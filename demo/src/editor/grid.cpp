///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    grid.cpp
 * \author  Philip Vorsilak, gorzuate@allacrost.org
 * \brief   Source file for editor's grid, used for the OpenGL map portion
 *          where tiles are painted, edited, etc.
 *****************************************************************************/

#include <iostream>
#include "grid.h"

using namespace hoa_script;
using namespace hoa_editor;
using namespace hoa_video;
using namespace std;

namespace hoa_editor
{
	LAYER_TYPE& operator++(LAYER_TYPE& value, int dummy) 
	{
		value = static_cast<LAYER_TYPE>(static_cast<int>(value) + 1);
		return value;
	}
}

Grid::Grid(QWidget* parent, const QString& name, int width, int height)
	: QGLWidget(parent, (const char*) name)
{	
	_file_name = name;      // gives the map a name

	// Initial size
	_height = height;
	_width  = width;
	resize(_width * TILE_WIDTH, _height * TILE_HEIGHT);
	_context = 0;

	// Default properties
	_changed = false;       // map has not yet been modified
	_grid_on = true;        // grid lines default to on
	_select_on = false;     // selection rectangle default to off
	_ll_on = true;          // lower layer default to on
	_ml_on = false;         // middle layer default to off
	_ul_on = false;         // upper layer default to off

	// Initialize layers
	vector<int> vect;
	for (int i = 0; i < _width * _height; i++)
	{
		vect.push_back(-1);
		_select_layer.push_back(-1);
	} // -1 is used for no tiles
	_lower_layer.push_back(vect);
	_middle_layer.push_back(vect);
	_upper_layer.push_back(vect);
} // Grid constructor

Grid::~Grid()
{
	for (vector<Tileset*>::iterator it = tilesets.begin();it != tilesets.end();
	     it++)
		delete *it;
	VideoManager->SingletonDestroy();
} // Grid destructor



// ********** Public functions **********

void Grid::SetChanged(bool value)
{
	_changed = value;
} // SetChanged(...)

void Grid::SetFileName(QString filename)
{
	_file_name = filename;
} // SetFileName(...)

void Grid::SetHeight(int height)
{
	_height = height;
	_changed = true;
} // SetHeight(...)

void Grid::SetWidth(int width)
{
	_width = width;
	_changed = true;
} // SetWidth(...)

void Grid::SetContext(int context)
{
	_context = context;
} // SetContext(...)

void Grid::SetLLOn(bool value)
{
	_ll_on = value;
	updateGL();
} // SetLLOn(...)

void Grid::SetMLOn(bool value)
{
	_ml_on = value;
	updateGL();
} // SetMLOn(...)

void Grid::SetULOn(bool value)
{
	_ul_on = value;
	updateGL();
} // SetULOn(...)

void Grid::SetGridOn(bool value)
{
	_grid_on = value;
	updateGL();
} // SetGridOn(...)

void Grid::SetSelectOn(bool value)
{
	_select_on = value;
	updateGL();
} // SetSelectOn(...)

void Grid::SetTexturesOn(bool value)
{
	_textures_on = value;
	updateGL();
} // SetTexturesOn(...)

vector<int32>& Grid::GetLayer(LAYER_TYPE layer, int context) 
{
	switch(layer) {
		case LOWER_LAYER:
			return _lower_layer[context];
		case MIDDLE_LAYER:
			return _middle_layer[context];
		case UPPER_LAYER:
			return _upper_layer[context];
		case SELECT_LAYER:
			return _select_layer;
		case INVALID_LAYER:
			/* FALLTHRU */
		case TOTAL_LAYER:
			/* FALLTHRU */
		default:
			return _lower_layer[context];
	} // switch on the current layer
} // GetLayer(...)

void Grid::SetMusic(const QString& music_file) 
{
	_music_file = music_file;
} // SetMusic(...)

const QString& Grid::GetMusic() const 
{
	return _music_file;
} // GetMusic()

void Grid::CreateNewContext()
{
	// Assumes all 3 layers have the same number of contexts.
	if (static_cast<unsigned>(_context) >= _lower_layer.size())
	{	
		_context = _lower_layer.size();  // in case it's greater than
		vector<int> vect;
		for (int i = 0; i < _width * _height; i++)
			vect.push_back(-1);
		_lower_layer.push_back(vect);
		_middle_layer.push_back(vect);
		_upper_layer.push_back(vect);
	} // only if current context doesn't already exist
} // CreateNewContext()

void Grid::LoadMap()
{
	ReadScriptDescriptor read_data;
	vector<int32> vect;             // used to read in vectors from the file

	if (!read_data.OpenFile(string(_file_name.toAscii())))
		QMessageBox::warning(this, "Loading File...", QString("ERROR: could not open %1 for reading!").arg(_file_name));

	read_data.OpenTable(string(_file_name.section('/', -1).remove(".lua").toAscii()));

	tileset_names.clear();
	tilesets.clear();
	_lower_layer.clear();
	_middle_layer.clear();
	_upper_layer.clear();
	
	_height = read_data.ReadInt("num_tile_rows");
	_width  = read_data.ReadInt("num_tile_cols");
	resize(_width * TILE_WIDTH, _height * TILE_HEIGHT);

	read_data.OpenTable("tileset_filenames");
	uint32 table_size = read_data.GetTableSize();
	for (uint32 i = 1; i <= table_size; i++)
		tileset_names.append(QString(read_data.ReadString(i).c_str()));
	read_data.CloseTable();

	// Loading the tileset images using LoadMultiImage is done in editor.cpp in FileOpen via
	// creation of the TilesetTable(s)

	read_data.OpenTable("lower_layer");
	for (int32 i = 0; i < _height; i++)
	{
		read_data.ReadIntVector(i, vect);
		for (vector<int32>::iterator it = vect.begin(); it != vect.end(); it++)
			_lower_layer[0].push_back(*it);
		vect.clear();
	} // iterate through the rows of the lower layer
	read_data.CloseTable();

	read_data.OpenTable("middle_layer");
	for (int32 i = 0; i < _height; i++)
	{
		read_data.ReadIntVector(i, vect);
		for (vector<int32>::iterator it = vect.begin(); it != vect.end(); it++)
			_middle_layer[0].push_back(*it);
		vect.clear();
	} // iterate through the rows of the lower layer
	read_data.CloseTable();

	read_data.OpenTable("upper_layer");
	for (int32 i = 0; i < _height; i++)
	{
		read_data.ReadIntVector(i, vect);
		for (vector<int32>::iterator it = vect.begin(); it != vect.end(); it++)
			_upper_layer[0].push_back(*it);
		vect.clear();
	} // iterate through the rows of the lower layer
	read_data.CloseTable();
	
	// The map_grid is 4x as big as the map: 2x in the width and 2x in the height. Starting
	// with row 0 (and doing the same thing for every row that is a multiple of 2), we take the first 2 entries,
	// bit-wise AND them, and put them into a temporary vector called walk_temp. We keep doing this for every 2
	// entries until the row is exhausted. walk_temp should now be the same length or size as the width of the map.
	//
	// Now we go on to the next row (and we do the same thing here for every other row, or rows that are not
	// multiples of 2. We do the same thing that we did for the multiples-of-2 rows, except now we bit-wise AND the
	// 2 entries with their corresponding entry in the walk_temp vector. We have now reconstructed the walkability
	// for each tile.
	/*read_data.ReadOpenTable("map_grid");
	uint32 walk_west;
	vector<uint32> walk_temp;
	vector<uint32>::iterator wit;
	for (int32 i = 0; i < _height * 2; i++)
	{
		read_data.ReadIntVector(i, vect);
		wit = walk_temp.begin();
		for (vector<int32>::iterator it = vect.begin(); it != vect.end(); it++)
		{
			walk_west = *it;
			it++;
			if (i % 2 == 0)
				walk_temp.push_back(walk_west & *it);
			else
			{
				indiv_walkable.push_back(*wit & walk_west & *it);
				wit++;
			} // remainder means entire tile has been read
		} // iterate through the row
		vect.clear();
		if (i % 2 != 0)
			walk_temp.clear();
	} // iterate through the rows of the walkability table
	read_data.ReadCloseTable();*/

	// Load music
	read_data.OpenTable("music_filenames");
	if (read_data.IsErrorDetected() == false)
	{
		int size = read_data.GetTableSize();
		if (size == 0)
			_music_file = "None";
		else
			_music_file = QString(read_data.ReadString(1).c_str());
		read_data.CloseTable();
	}

	read_data.CloseTable();

	_grid_on = true;        // grid lines default to on
	_ll_on   = true;        // lower layer default to on
	_ml_on   = true;        // middle layer default to off
	_ul_on   = true;        // upper layer default to off
	_context = 1;           // context default to main context (#1)
	_changed = false;       // map has not been changed yet
} // LoadMap()

void Grid::SaveMap()
{
	char buffer[10];  // used for converting an int to a string with sprintf
	int i;           // Lua table index / Loop counter variable
	vector<int32>::iterator it;  // used to iterate through the layers
	vector<int32> layer_row;     // one row of a layer
	WriteScriptDescriptor write_data;
	int tileset_index;
	int tile_index;
	
	if (write_data.OpenFile(string(_file_name.toAscii())) == false) {
		QMessageBox::warning(this, "Saving File...", QString("ERROR: could not open %1 for writing!").arg(_file_name));
		return;
	}

	write_data.WriteComment("A reference to the C++ MapMode object that was created with this file");
	write_data.WriteLine("map = {}\n");

	write_data.WriteComment("The number of rows and columns of tiles that compose the map");
	write_data.WriteInt("num_tile_cols", _width);
	write_data.WriteInt("num_tile_rows", _height);
	write_data.InsertNewLine();

	write_data.WriteComment("The sound files used on this map.");
	write_data.BeginTable("sound_filenames");
	// TEMP: currently sound_filenames table is not populated with sounds
	write_data.EndTable();
	write_data.InsertNewLine();

	write_data.WriteComment("The music files used as background music on this map.");
	write_data.BeginTable("music_filenames");
	if (_music_file != "None")
		write_data.WriteString(1, string(_music_file.toAscii()));
	write_data.EndTable();
	write_data.InsertNewLine();

	write_data.WriteComment("The names of the tilesets used, with the path and file extension omitted");
	write_data.BeginTable("tileset_filenames");
	i = 0;
	for (QStringList::Iterator qit = tileset_names.begin(); qit != tileset_names.end(); ++qit)
	{
		i++;
		write_data.WriteString(i, (*qit).ascii());
	} // iterate through tileset_list writing each element
	write_data.EndTable();
	write_data.InsertNewLine();

	// Create vector of 0s.
	vector<int32> vect_0s(_width, 0);

	write_data.WriteComment("The map grid to indicate walkability. The size of the grid is 4x the size of the tile layer tables");
	write_data.WriteComment("Walkability status of tiles for 32 contexts. Zero indicates walkable. Valid range: [0:2^32-1]");
	write_data.BeginTable("map_grid");
	vector<int32> ll_vect;
	vector<int32> ml_vect;
	vector<int32> ul_vect;
	int row = 0;
	bool twice = false;
	while (row < _height)
	{
		for (int32 i = row * _width; i < row * _width + _width; i++)
		{
			// Get walkability for lower layer tile.
			tileset_index = _lower_layer[0][i] / 256;
			if (tileset_index == 0)
				tile_index = _lower_layer[0][i];
			else  // Don't divide by 0
				tile_index = _lower_layer[0][i] % (tileset_index * 256);
			if (tile_index == -1)
			{
				ll_vect.push_back(0);
				ll_vect.push_back(0);
				ll_vect.push_back(0);
				ll_vect.push_back(0);
			}
			else
				ll_vect = tilesets[tileset_index]->walkability[tile_index];

			// Get walkability for middle layer tile.
			tileset_index = _middle_layer[0][i] / 256;
			if (tileset_index == 0)
				tile_index = _middle_layer[0][i];
			else  // Don't divide by 0
				tile_index = _middle_layer[0][i] % (tileset_index * 256);
			if (tile_index == -1)
			{
				ml_vect.push_back(0);
				ml_vect.push_back(0);
				ml_vect.push_back(0);
				ml_vect.push_back(0);
			}
			else
				ml_vect = tilesets[tileset_index]->walkability[tile_index];

			// Get walkability for upper layer tile.
			tileset_index = _upper_layer[0][i] / 256;
			if (tileset_index == 0)
				tile_index = _upper_layer[0][i];
			else  // Don't divide by 0
				tile_index = _upper_layer[0][i] % (tileset_index * 256);
			if (tile_index == -1)
			{
				ul_vect.push_back(0);
				ul_vect.push_back(0);
				ul_vect.push_back(0);
				ul_vect.push_back(0);
			}
			else
				ul_vect = tilesets[tileset_index]->walkability[tile_index];

			if (twice)
			{
				layer_row.push_back(ll_vect[2] | ml_vect[2] | ul_vect[2]);  // SW corner
				layer_row.push_back(ll_vect[3] | ml_vect[3] | ul_vect[3]);  // SE corner
			}
			else
			{
				layer_row.push_back(ll_vect[0] | ml_vect[0] | ul_vect[0]);  // NW corner
				layer_row.push_back(ll_vect[1] | ml_vect[1] | ul_vect[1]);  // NE corner
			}
			
			ll_vect.clear();
			ml_vect.clear();
			ul_vect.clear();
		} // iterate through the columns of the layers

		if (twice)
			sprintf(buffer, "%d", row*2+1);
		else
			sprintf(buffer, "%d", row*2);
		write_data.WriteIntVector(buffer, layer_row);
		layer_row.clear();
		if (twice)
		{
			twice = false;
			row++;
		}
		else
			twice = true;
	} // iterate through the rows of the layers
	write_data.EndTable();
	write_data.InsertNewLine();

	write_data.WriteComment("The lower tile layer. The numbers are indeces to the tile_mappings table.");
	write_data.BeginTable("lower_layer");
	it = _lower_layer[0].begin();
	for (int row = 0; row < _height; row++)
	{
		for (int col = 0; col < _width; col++)
		{
			layer_row.push_back(*it);
			it++;
		} // iterate through the columns of the lower layer
		sprintf(buffer, "%d", row);
		write_data.WriteIntVector(buffer, layer_row);
		layer_row.clear();
	} // iterate through the rows of the lower layer
	write_data.EndTable();
	write_data.InsertNewLine();

	write_data.WriteComment("The middle tile layer. The numbers are indeces to the tile_mappings table.");
	write_data.BeginTable("middle_layer");
	it = _middle_layer[0].begin();
	for (int row = 0; row < _height; row++)
	{
		for (int col = 0; col < _width; col++)
		{
			layer_row.push_back(*it);
			it++;
		} // iterate through the columns of the middle layer
		sprintf(buffer, "%d", row);
		write_data.WriteIntVector(buffer, layer_row);
		layer_row.clear();
	} // iterate through the rows of the middle layer
	write_data.EndTable();
	write_data.InsertNewLine();

	write_data.WriteComment("The upper tile layer. The numbers are indeces to the tile_mappings table.");
	write_data.BeginTable("upper_layer");
	it = _upper_layer[0].begin();
	for (int row = 0; row < _height; row++)
	{
		for (int col = 0; col < _width; col++)
		{
			layer_row.push_back(*it);
			it++;
		} // iterate through the columns of the upper layer
		sprintf(buffer, "%d", row);
		write_data.WriteIntVector(buffer, layer_row);
		layer_row.clear();
	} // iterate through the rows of the upper layer
	write_data.EndTable();
	write_data.InsertNewLine();

	write_data.WriteComment("All, if any, existing contexts follow.");
	vector<int32> context_data;  // one vector of ints contains all the context info
	// Iterate through all contexts of all layers.
	for (int i = 1; i < static_cast<int>(_lower_layer.size()); i++)
	{
		it = _lower_layer[i].begin();
		for (int row = 0; row < _height; row++)
		{
			for (int col = 0; col < _width; col++)
			{
				if (*it != -1)
				{
					context_data.push_back(0);    // lower layer = 0
					context_data.push_back(row);
					context_data.push_back(col);
					context_data.push_back(*it);
				} // a valid tile exists so record it

				it++;
			} // iterate through the columns of the lower layer
		} // iterate through the rows of the lower layer

		it = _middle_layer[i].begin();
		for (int row = 0; row < _height; row++)
		{
			for (int col = 0; col < _width; col++)
			{
				if (*it != -1)
				{
					context_data.push_back(1);    // middle layer = 1
					context_data.push_back(row);
					context_data.push_back(col);
					context_data.push_back(*it);
				} // a valid tile exists so record it

				it++;
			} // iterate through the columns of the middle layer
		} // iterate through the rows of the middle layer

		it = _upper_layer[i].begin();
		for (int row = 0; row < _height; row++)
		{
			for (int col = 0; col < _width; col++)
			{
				if (*it != -1)
				{
					context_data.push_back(2);    // upper layer = 2
					context_data.push_back(row);
					context_data.push_back(col);
					context_data.push_back(*it);
				} // a valid tile exists so record it

				it++;
			} // iterate through the columns of the upper layer
		} // iterate through the rows of the upper layer

		if (context_data.empty() == false)
		{
			if (i < 11)
				sprintf(buffer, "context_0%d", i);
			else
				sprintf(buffer, "context_%d", i);
			write_data.WriteIntVector(buffer, context_data);
			write_data.InsertNewLine();
		} // write the vector if it has data in it
	} // iterate through all contexts of all layers, assuming all layers have same number of contexts

	write_data.CloseFile();

	_changed = false;
} // SaveMap()



// ********** Protected functions **********

void Grid::initializeGL()
{
	// Destroy the video engine
	VideoManager->SingletonDestroy();
	// Recreate the video engine's singleton
	VideoManager = GameVideo::SingletonCreate();
	VideoManager->SetTarget(VIDEO_TARGET_QT_WIDGET);
	
	VideoManager->SingletonInitialize();
		
	VideoManager->ApplySettings();
	VideoManager->FinalizeInitialization();
	VideoManager->ToggleFPS();
} // initializeGL()

void Grid::paintGL()
{
	int32 col;                       // tile array loop index
	vector<int32>::iterator it;      // used to iterate through tile arrays
	int tileset_index;               // index into the tileset_names vector
	int tile_index;                  // ranges from 0-255

	// Setup drawing parameters
	VideoManager->SetCoordSys(0.0f, VideoManager->GetScreenWidth() / TILE_WIDTH,
		VideoManager->GetScreenHeight() / TILE_HEIGHT, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);
	VideoManager->Clear(Color::black);

	// Draw lower layer
	if (_ll_on)
	{
		VideoManager->Move(0.0f, 0.0f);
		col = 0;
		for (it = _lower_layer[_context].begin(); it != _lower_layer[_context].end(); it++)
		{
			if (*it != -1)
			{
				tileset_index = *it / 256;
				if (tileset_index == 0)
					tile_index = *it;
				else  // Don't divide by 0
					tile_index = *it % (tileset_index * 256);
				//cerr << "grid tileset_index: " << tileset_index << endl;
				//cerr << "grid tile_index: " << tile_index << endl;
				tilesets[tileset_index]->tiles[tile_index].Draw();
			} // a tile exists to draw
			col++;
			col %= _width;
			if (col == 0)
				VideoManager->MoveRelative(-_width + 1, 1.0f);
			else
				VideoManager->MoveRelative(1.0f, 0.0f);
		} // iterate through lower layer
	} // lower layer must be viewable

	// Draw middle layer
	if (_ml_on)
	{
		VideoManager->Move(0.0f, 0.0f);
		col = 0;
		for (it = _middle_layer[_context].begin(); it != _middle_layer[_context].end(); it++)
		{
			if (*it != -1)
			{
				tileset_index = *it / 256;
				if (tileset_index == 0)
					tile_index = *it;
				else  // Don't divide by 0
					tile_index = *it % (tileset_index * 256);
				tilesets[tileset_index]->tiles[tile_index].Draw();
			} // a tile exists to draw
			col++;
			col %= _width;
			if (col == 0)
				VideoManager->MoveRelative(-_width + 1, 1.0f);
			else
				VideoManager->MoveRelative(1.0f, 0.0f);
		} // iterate through middle layer
	} // middle layer must be viewable

	// Draw upper layer
	if (_ul_on)
	{
		VideoManager->Move(0.0f, 0.0f);
		col = 0;
		for (it = _upper_layer[_context].begin(); it != _upper_layer[_context].end(); it++)
		{
			if (*it != -1)
			{
				tileset_index = *it / 256;
				if (tileset_index == 0)
					tile_index = *it;
				else  // Don't divide by 0
					tile_index = *it % (tileset_index * 256);
				tilesets[tileset_index]->tiles[tile_index].Draw();
			} // a tile exists to draw
			col++;
			col %= _width;
			if (col == 0)
				VideoManager->MoveRelative(-_width + 1, 1.0f);
			else
				VideoManager->MoveRelative(1.0f, 0.0f);
		} // iterate through upper layer
	} // upper layer must be viewable

	// If selection rectangle mode is on, draw it
	if (_select_on)
	{
		Color blue_selection(0.0f, 0.0f, 255.0f, 0.5f);
		VideoManager->Move(0.0f, 0.0f);
		col = 0;
		for (it = _select_layer.begin(); it != _select_layer.end(); it++)
		{
			// a tile exists to draw
			if (*it != -1)
				VideoManager->DrawRectangle(1.0f, 1.0f, blue_selection);
			col++;
			col %= _width;
			if (col == 0)
				VideoManager->MoveRelative(-_width + 1, 1.0f);
			else
				VideoManager->MoveRelative(1.0f, 0.0f);
		} // iterate through selection layer
	} // selection rectangle must be viewable

	// If grid is toggled on, draw it
	if (_grid_on)
		VideoManager->DrawGrid(0.0f, 0.0f, 1.0f, 1.0f, Color::black);

	if (_textures_on)
		VideoManager->Textures()->DEBUG_ShowTexSheet();
} // paintGL()

void Grid::resizeGL(int w, int h)
{
	VideoManager->SetResolution(w, h);
	VideoManager->ApplySettings();
} // resizeGL(...)

