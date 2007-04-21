///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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

	// Default properties
	_changed = false;       // map has not yet been modified
	_grid_on = true;        // grid lines default to on
	_ll_on = true;          // lower layer default to on
	_ml_on = false;         // middle layer default to off
	_ul_on = false;         // upper layer default to off

	// Initialize layers
	for (int i = 0; i < _width * _height; i++)
	{
		_lower_layer.push_back(-1);
		_middle_layer.push_back(-1);
		_upper_layer.push_back(-1);
	} // -1 is used for no tiles
	// -1 is used for no tiles
	for (int i = 0; i < _width * _height * 4; i++)
		indiv_walkable.push_back(-1);

	// Create the video engine's singleton
	VideoManager = GameVideo::SingletonCreate();
} // Grid constructor

Grid::~Grid()
{
	// FIXME
//	VideoManager->DeleteImage();
	GameVideo::SingletonDestroy();
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

std::vector<int32>& Grid::GetLayer(LAYER_TYPE layer) 
{
	switch(layer) {
		case LOWER_LAYER:
			return _lower_layer;
		case MIDDLE_LAYER:
			return _middle_layer;
		case UPPER_LAYER:
			return _upper_layer;
		case INVALID_LAYER:
			/* FALLTHRU */
		case TOTAL_LAYER:
			/* FALLTHRU */
		default:
			return _lower_layer;
	} // switch on the current layer
	//return _lower_layer;
} // GetLayer(...)

void Grid::SetMusic(const QString& music_file) 
{
	_music_file = music_file;
} // SetMusic(...)

const QString& Grid::GetMusic() const 
{
	return _music_file;
} // GetMusic()

void Grid::LoadMap()
{
	ScriptDescriptor read_data;
	vector<int32> vect;             // used to read in vectors from the file

	if (!read_data.OpenFile(_file_name.toStdString(), SCRIPT_READ))
		QMessageBox::warning(this, "Loading File...", QString("ERROR: could not open %1 for reading!").arg(_file_name));

	tileset_names.clear();
	tilesets.clear();
	_lower_layer.clear();
	_middle_layer.clear();
	_upper_layer.clear();
	
	_height = read_data.ReadInt("num_tile_rows");
	_width  = read_data.ReadInt("num_tile_cols");
	resize(_width * TILE_WIDTH, _height * TILE_HEIGHT);

	read_data.ReadOpenTable("tileset_filenames");
	uint32 table_size = read_data.ReadGetTableSize();
	for (uint32 i = 1; i <= table_size; i++)
		tileset_names.append(QString::fromStdString(read_data.ReadString(i)));
	read_data.ReadCloseTable();

	// Loading the tileset images using LoadMultiImage is done in editor.cpp in FileOpen via
	// creation of the TilesetTable(s)

	read_data.ReadOpenTable("lower_layer");
	for (int32 i = 0; i < _height; i++)
	{
		read_data.ReadIntVector(i, vect);
		for (vector<int32>::iterator it = vect.begin(); it != vect.end(); it++)
			_lower_layer.push_back(*it);
		vect.clear();
	} // iterate through the rows of the lower layer
	read_data.ReadCloseTable();

	read_data.ReadOpenTable("middle_layer");
	for (int32 i = 0; i < _height; i++)
	{
		read_data.ReadIntVector(i, vect);
		for (vector<int32>::iterator it = vect.begin(); it != vect.end(); it++)
			_middle_layer.push_back(*it);
		vect.clear();
	} // iterate through the rows of the lower layer
	read_data.ReadCloseTable();

	read_data.ReadOpenTable("upper_layer");
	for (int32 i = 0; i < _height; i++)
	{
		read_data.ReadIntVector(i, vect);
		for (vector<int32>::iterator it = vect.begin(); it != vect.end(); it++)
			_upper_layer.push_back(*it);
		vect.clear();
	} // iterate through the rows of the lower layer
	read_data.ReadCloseTable();
	
	// The map_grid is 4x as big as the map: 2x in the width and 2x in the height. Starting
	// with row 0 (and doing the same thing for every row that is a multiple of 2), we take the first 2 entries,
	// bit-wise AND them, and put them into a temporary vector called walk_temp. We keep doing this for every 2
	// entries until the row is exhausted. walk_temp should now be the same length or size as the width of the map.
	//
	// Now we go on to the next row (and we do the same thing here for every other row, or rows that are not
	// multiples of 2. We do the same thing that we did for the multiples-of-2 rows, except now we bit-wise AND the
	// 2 entries with their corresponding entry in the walk_temp vector. We have now reconstructed the walkability
	// for each tile.
	read_data.ReadOpenTable("map_grid");
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
	read_data.ReadCloseTable();

	// Load music
	read_data.ReadOpenTable("music_filenames");
	if (read_data.GetErrorCode() == SCRIPT_NO_ERRORS)
	{
		int size = read_data.ReadGetTableSize();
		if (size == 0)
			_music_file = "None";
		else
			_music_file = QString::fromStdString(read_data.ReadString(1));
		read_data.ReadCloseTable();
	}

	_grid_on = true;        // grid lines default to on
	_ll_on   = true;        // lower layer default to on
	_ml_on   = true;        // middle layer default to off
	_ul_on   = true;        // upper layer default to off
	_changed = false;       // map has not been changed yet
} // LoadMap()

void Grid::SaveMap()
{
	char buffer[5]; // used for converting an int to a string with sprintf
	int i;      // Lua table index / Loop counter variable
	vector<int32>::iterator it;  // used to iterate through the layers
	vector<int32> layer_row;     // one row of a layer
	ScriptDescriptor write_data;
	
	if (write_data.OpenFile(_file_name.toStdString(), SCRIPT_WRITE) == false) {
		QMessageBox::warning(this, "Saving File...", QString("ERROR: could not open %1 for writing!").arg(_file_name));
		return;
	}

	write_data.WriteComment(_file_name);
	write_data.WriteInsertNewLine();

	write_data.WriteComment("A reference to the C++ MapMode object that was created with this file");
	write_data.WriteLine("map = {}\n");

	write_data.WriteComment("The number of rows and columns of tiles that compose the map");
	write_data.WriteInt("num_tile_cols", _width);
	write_data.WriteInt("num_tile_rows", _height);
	write_data.WriteInsertNewLine();

	write_data.WriteComment("The sound files used on this map.");
	write_data.WriteBeginTable("sound_filenames");
	// TEMP: currently sound_filenames table is not populated with sounds
	write_data.WriteEndTable();
	write_data.WriteInsertNewLine();

	write_data.WriteComment("The music files used as background music on this map.");
	write_data.WriteBeginTable("music_filenames");
	if (_music_file != "None")
		write_data.WriteString(1, _music_file);
	write_data.WriteEndTable();
	write_data.WriteInsertNewLine();

	write_data.WriteComment("The names of the tilesets used, with the path and file extension omitted");
	write_data.WriteBeginTable("tileset_filenames");
	i = 0;
	for (QStringList::Iterator qit = tileset_names.begin(); qit != tileset_names.end(); ++qit)
	{
		i++;
		write_data.WriteString(i, (*qit).ascii());
	} // iterate through tileset_list writing each element
	write_data.WriteEndTable();
	write_data.WriteInsertNewLine();

	// Create vector of 0s.
	vector<int32> vect_0s(_width, 0);

	write_data.WriteComment("The map grid to indicate walkability. The size of the grid is 4x the size of the tile layer tables");
	write_data.WriteComment("Walkability status of tiles for 32 contexts. Non-zero indicates walkable. Valid range: [0:2^32-1]");
	write_data.WriteBeginTable("map_grid");
	for (int row = 0; row < _height * 2; row++)
	{
		for (int col = 0; col < _width * 2; col++)
		{
			// Individual tile property supersedes anything else.
			if (indiv_walkable[row / 2 * _width + col] != -1)
			{
				if (row % 2 == 0)
				{
					layer_row.push_back(indiv_walkable[row / 2 * _width + col] & 1);  // gets NW corner
					layer_row.push_back((indiv_walkable[row / 2 * _width + col] >> 1) & 1);  // gets NE corner
				} // no remainder means top half of the tile
				else
				{
					layer_row.push_back((indiv_walkable[row / 2 * _width + col] >> 2) & 1);  // gets SW corner
					layer_row.push_back((indiv_walkable[row / 2 * _width + col] >> 3) & 1);  // gets SE corner
				} // remainder means bottom half of the tile
			} // this tile has been modified from it's default
			else
			{
				/*ScriptDescriptor read_data;
				if (!read_data.OpenFile("dat/tilesets/tiles_database.lua", SCRIPT_READ))
					QMessageBox::warning(this, "Tiles Database",
						QString("ERROR: could not open dat/tilesets/tiles_database.lua for reading!"));
				QString temp;// = file_name_list[_lower_layer[row / 2 * _width + col]];
				temp.remove(".png").remove("img/tiles/");
				read_data.ReadOpenTable("tile_filenames");
				uint32 table_size = read_data.ReadGetTableSize();
				uint32 index = 0;
				QString filename = "";
				while (filename != temp && index < table_size)
				{
					index++;
					filename = QString::fromStdString(read_data.ReadString(index));
				} // find index of current tile in the database
				read_data.ReadCloseTable();
				read_data.CloseFile();

				if (filename == temp)
				{
					read_data.ReadOpenTable("tile_properties");
					uint32 walk_prop = read_data.ReadInt(index);
					if (row % 2 == 0)
					{
						layer_row.push_back(walk_prop & 1);  // gets NW corner
						layer_row.push_back(walk_prop & 2);  // gets NE corner
					} // no remainder means top half of the tile
					else
					{
						layer_row.push_back(walk_prop & 4);  // gets SW corner
						layer_row.push_back(walk_prop & 8);  // gets SE corner
					} // remainder means bottom half of the tile
					read_data.ReadCloseTable();
				} // tile exists in the database*/
			} // falls back to global property in tile database
		} // iterate through the columns of the lower layer
		sprintf(buffer, "%d", row);
		write_data.WriteIntVector(buffer, layer_row);
		layer_row.clear();
	} // iterate through the rows of the lower layer
	write_data.WriteEndTable();
	write_data.WriteInsertNewLine();

	write_data.WriteComment("The lower tile layer. The numbers are indeces to the tile_mappings table.");
	write_data.WriteBeginTable("lower_layer");
	it = _lower_layer.begin();
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
	write_data.WriteEndTable();
	write_data.WriteInsertNewLine();

	write_data.WriteComment("The middle tile layer. The numbers are indeces to the tile_mappings table.");
	write_data.WriteBeginTable("middle_layer");
	it = _middle_layer.begin();
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
	write_data.WriteEndTable();
	write_data.WriteInsertNewLine();

	write_data.WriteComment("The upper tile layer. The numbers are indeces to the tile_mappings table.");
	write_data.WriteBeginTable("upper_layer");
	it = _upper_layer.begin();
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
	write_data.WriteEndTable();
	write_data.WriteInsertNewLine();

	write_data.CloseFile();

	_changed = false;
} // SaveMap()



// ********** Protected slots **********

void Grid::initializeGL()
{
	VideoManager->SetTarget(VIDEO_TARGET_QT_WIDGET);
	VideoManager->SingletonInitialize();
	VideoManager->ToggleFPS();
} // initializeGL()

void Grid::paintGL()
{
	int32 col;                       // tile array loop index
	vector<int32>::iterator it;      // used to iterate through tile arrays
	int tileset_index;               // index into the tileset_names vector
	int tile_index;                  // ranges from 0-255

	// Setup drawing parameters
	VideoManager->SetCoordSys(0.0f, VideoManager->GetWidth() / TILE_WIDTH,
		VideoManager->GetHeight() / TILE_HEIGHT, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);
	VideoManager->Clear(Color::white);

	// Draw lower layer
	if (_ll_on)
	{
		VideoManager->Move(0.0f, 0.0f);
		col = 0;
		for (it = _lower_layer.begin(); it != _lower_layer.end(); it++)
		{
			if (*it != -1)
			{
				tileset_index = *it / 256;
				if (tileset_index == 0)
					tile_index = *it;
				else  // Don't divide by 0
					tile_index = *it / (tileset_index * 256);
				VideoManager->DrawImage(tilesets[tileset_index]->tiles[tile_index]);
			} // a tile exists to draw
			col = ++col % _width;
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
		for (it = _middle_layer.begin(); it != _middle_layer.end(); it++)
		{
			if (*it != -1)
			{
				tileset_index = *it / 256;
				if (tileset_index == 0)
					tile_index = *it;
				else  // Don't divide by 0
					tile_index = *it / (tileset_index * 256);
				VideoManager->DrawImage(tilesets[tileset_index]->tiles[tile_index]);
			} // a tile exists to draw
			col = ++col % _width;
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
		for (it = _upper_layer.begin(); it != _upper_layer.end(); it++)
		{
			if (*it != -1)
			{
				tileset_index = *it / 256;
				if (tileset_index == 0)
					tile_index = *it;
				else  // Don't divide by 0
					tile_index = *it / (tileset_index * 256);
				VideoManager->DrawImage(tilesets[tileset_index]->tiles[tile_index]);
			} // a tile exists to draw
			col = ++col % _width;
			if (col == 0)
				VideoManager->MoveRelative(-_width + 1, 1.0f);
			else
				VideoManager->MoveRelative(1.0f, 0.0f);
		} // iterate through upper layer
	} // upper layer must be viewable

	// If grid is toggled on, draw it
	if (_grid_on)
		VideoManager->DrawGrid(0.0f, 0.0f, 1.0f, 1.0f, Color::black);
} // paintGL()

void Grid::resizeGL(int w, int h)
{
	VideoManager->SetResolution(w, h);
	VideoManager->ApplySettings();
} // resizeGL(...)
