///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
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

#include <sstream>
#include <iostream>
#include "grid.h"
#include "editor.h"

using namespace hoa_script;
using namespace hoa_map::private_map;
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
	_context = 0;           // context is set to default base context
	context_names << "Base";
	_changed = false;       // map has not yet been modified
	_initialized = false;   // prevents drawing the map until after it has been completely loaded
	_grid_on = true;        // grid lines default to on
	_select_on = false;     // selection rectangle default to off
	_ll_on = true;          // lower layer default to on
	_ml_on = false;         // middle layer default to off
	_ul_on = false;         // upper layer default to off
	_ol_on = true;			// object layer default to off

	// Set mouse tracking
	setMouseTracking(true);

	// Initialize layers
	vector<int> vect;
	for (int i = 0; i < _width * _height; i++)
	{
		vect.push_back(-1);
		_select_layer.push_back(-1);
		_object_layer.push_back(-1);
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
	for (list<MapSprite*>::iterator it = sprites.begin();it != sprites.end();
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

void Grid::SetInitialized(bool ready)
{
	_initialized = ready;
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

void Grid::SetOLOn(bool value)
{
	_ol_on = value;
	updateGL();
} // SetOLOn(...)

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
		case OBJECT_LAYER:
			return _object_layer;
		case INVALID_LAYER:
			/* FALLTHRU */
		case TOTAL_LAYER:
			/* FALLTHRU */
		default:
			return _lower_layer[context];
	} // switch on the current layer
} // GetLayer(...)

void Grid::CreateNewContext(int inherit_context)
{
	// Assumes all 3 layers have the same number of contexts.

	_lower_layer.push_back(_lower_layer[inherit_context]);
	_middle_layer.push_back(_middle_layer[inherit_context]);
	_upper_layer.push_back(_upper_layer[inherit_context]);
} // CreateNewContext()

void Grid::LoadMap()
{
	ReadScriptDescriptor read_data;
	vector<int32> vect;             // used to read in vectors from the file

	if (read_data.OpenFile(string(_file_name.toAscii()), true) == false)
		QMessageBox::warning(this, "Loading File...", QString("ERROR: could not open %1 for reading!").arg(_file_name));

	read_data.OpenTable(string(_file_name.section('/', -1).remove(".lua").toAscii()));

	music_files.clear();
	tileset_names.clear();
	tilesets.clear();
	_lower_layer.clear();
	_middle_layer.clear();
	_upper_layer.clear();
	_object_layer.clear();

	int num_contexts = read_data.ReadInt("num_map_contexts");
	_height = read_data.ReadInt("num_tile_rows");
	_width  = read_data.ReadInt("num_tile_cols");
	resize(_width * TILE_WIDTH, _height * TILE_HEIGHT);

	// Create selection layer
	for (int i = 0; i < _width * _height; i++)
		_select_layer.push_back(-1);

	// Base context is default and not saved in the map file.
	read_data.OpenTable("context_names");
	uint32 table_size = read_data.GetTableSize();
	for (uint32 i = 1; i <= table_size; i++)
		context_names.append(QString(read_data.ReadString(i).c_str()));
	read_data.CloseTable();

	read_data.OpenTable("tileset_filenames");
	table_size = read_data.GetTableSize();
	for (uint32 i = 1; i <= table_size; i++)
		tileset_names.append(QString(read_data.ReadString(i).c_str()));
	read_data.CloseTable();

	// Load music
	read_data.OpenTable("music_filenames");
	table_size = read_data.GetTableSize();
	// Remove first 4 characters in the string ("mus/")
	for (uint32 i = 1; i <= table_size; i++)
		music_files << QString(read_data.ReadString(i).c_str()).remove(0,4);
	read_data.CloseTable();
	
	// Loading the tileset images using LoadMultiImage is done in editor.cpp in
	// FileOpen via creation of the TilesetTable(s)

	vector<int32> row_vect;
	_lower_layer.push_back(row_vect);
	_middle_layer.push_back(row_vect);
	_upper_layer.push_back(row_vect);

	read_data.OpenTable("lower_layer");
	for (int32 i = 0; i < _height; i++)
	{
		read_data.ReadIntVector(i, vect);
		for (vector<int32>::iterator it = vect.begin(); it != vect.end(); it++)
			_lower_layer.begin()->push_back(*it);
		vect.clear();
	} // iterate through the rows of the lower layer
	read_data.CloseTable();

	row_vect.clear();
	read_data.OpenTable("middle_layer");
	for (int32 i = 0; i < _height; i++)
	{
		read_data.ReadIntVector(i, vect);
		for (vector<int32>::iterator it = vect.begin(); it != vect.end(); it++)
			_middle_layer.begin()->push_back(*it);
		vect.clear();
	} // iterate through the rows of the middle layer
	read_data.CloseTable();

	row_vect.clear();
	read_data.OpenTable("upper_layer");
	for (int32 i = 0; i < _height; i++)
	{
		read_data.ReadIntVector(i, vect);
		for (vector<int32>::iterator it = vect.begin(); it != vect.end(); it++)
			_upper_layer.begin()->push_back(*it);
		vect.clear();
	} // iterate through the rows of the upper layer
	read_data.CloseTable();

	// Load sprites
	/*
	read_data.OpenTable("sprites");
	vector<int32> keys;
	read_data.ReadTableKeys(keys);

	// Create empty sprites
	for( uint32 i = 0; i < keys.size(); i++ ) {
		_object_layer.push_back( keys[i] );
		sprites.push_back(new MapSprite());
	}

	std::list<MapSprite* >::iterator it=sprites.begin();
	for (uint32 i = 0; i < keys.size(); i++)
	{
		// Read lua spites and write to sprites vector
		read_data.OpenTable( keys[i] );
		(*it)->SetObjectID( read_data.ReadInt("object_id") );
		(*it)->SetName( read_data.ReadString("name") );
		(*it)->SetContext( read_data.ReadInt("context") );
		(*it)->SetXPosition( read_data.ReadInt("x_position"), read_data.ReadFloat("x_position_offset") );
		(*it)->SetYPosition( read_data.ReadInt("y_position"), read_data.ReadFloat("y_position_offset") );
		(*it)->SetCollHalfWidth( read_data.ReadFloat("col_half_width") );
		(*it)->SetCollHeight( read_data.ReadFloat("col_height") );
		(*it)->SetImgHalfWidth( read_data.ReadFloat("img_half_width") );
		(*it)->SetImgHeight( read_data.ReadFloat("img_height") );
		(*it)->SetMovementSpeed( read_data.ReadFloat("movement_speed") );
		(*it)->SetDirection( read_data.ReadInt("direction") );
		(*it)->LoadStandardAnimations( read_data.ReadString("standard_animations") );
		(*it)->LoadRunningAnimations( read_data.ReadString("running_animations") );
		(*it)->SetFacePortrait( read_data.ReadString("face_portrait") );

		if( it != sprites.end() )
			it++;
		read_data.CloseTable();
	}
	read_data.CloseTable();
*/
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

	// Load any existing context data
	for (int i = 1; i < num_contexts; i++)
	{
		stringstream context;
		context << "context_";
		if (i < 10)
			context << "0";
		context << i;

		// Initialize this context
		_lower_layer.push_back(_lower_layer[0]);
		_middle_layer.push_back(_middle_layer[0]);
		_upper_layer.push_back(_upper_layer[0]);

		// Read the table corresponding to this context and modify each tile accordingly.
		// The context table is an array of integer data. The size of this array should be divisible by four, as every consecutive group of four integers in
		// this table represent one tile context element. The first integer corresponds to the tile layer (0 = lower, 1 = middle, 2 = upper), the second
		// and third represent the row and column of the tile respectively, and the fourth value indicates which tile image should be used for this context.
		// So if the first four entries in the context table were {0, 12, 26, 180}, this would set the lower layer tile at position (12, 26) to the tile
		// index 180.
		vector<int32> context_data;
		read_data.ReadIntVector(context.str(), context_data);
		for (int j = 0; j < static_cast<int>(context_data.size()); j += 4) {
			switch (context_data[j]) {
				case 0: // lower layer
					_lower_layer[i][context_data[j+1] * _width + context_data[j+2]] =
						context_data[j+3];
					break;
				case 1: // middle layer
					_middle_layer[i][context_data[j+1] * _width + context_data[j+2]] =
						context_data[j+3];
					break;
				case 2: // upper layer
					_upper_layer[i][context_data[j+1] * _width + context_data[j+2]] =
						context_data[j+3];
					break;
				default:
					qWarning("unknown tile layer index reference when loading map context tiles");
					break;
			} // switch on the layers
		} // iterate through all tiles in this context
	} // iterate through all existing contexts

	read_data.CloseTable();
} // LoadMap()

void Grid::SaveMap()
{
	const int32 BUFFER_SIZE = 1024;
	char buffer[BUFFER_SIZE];
	int i;
	WriteScriptDescriptor write_data;
	int tileset_index;
	int tile_index;

	ifstream file;
	string before_text;
	string after_text;
	const char * BEFORE_TEXT_MARKER = "-- Allacrost map editor begin. Do not edit this line. --";
	const char * AFTER_TEXT_MARKER =  "-- Allacrost map editor end. Do not edit this line. --";

	// First, get the non-editor data (such as map scripting) from the file to save, so we don't clobber it.
	file.open(_file_name.toAscii(), ifstream::in);
	if (file.is_open()) {
		// Put all text before BEFORE_TEXT_MARKER into before_text string
		while(!file.eof()) {
			file.clear();
			file.getline(buffer, BUFFER_SIZE);
			if (strstr(buffer, BEFORE_TEXT_MARKER))
				break;
			else {
				before_text.append(buffer);
				before_text.push_back('\n');
			}
		}

		// Search for AFTER_TEXT_MARKER
		while (!file.eof()) {
			file.clear();
			file.getline(buffer, BUFFER_SIZE);
			if (strstr(buffer, AFTER_TEXT_MARKER))
				break;
		}

		// Put all text after AFTER_TEXT_MARKER into after_text string
		while (!file.eof()) {
			file.clear();
			file.getline(buffer, BUFFER_SIZE);
			if (!file.eof()) {
				after_text.append(buffer);
				after_text.push_back('\n');
			}
		}

		file.close();
	}

	if (write_data.OpenFile(string(_file_name.toAscii())) == false) {
		QMessageBox::warning(this, "Saving File...", QString("ERROR: could not open %1 for writing!").arg(_file_name));
		return;
	}

	if (!before_text.empty())
		write_data.WriteLine(before_text, false);

	write_data.WriteLine(BEFORE_TEXT_MARKER);
	write_data.InsertNewLine();
	write_data.WriteComment("A reference to the C++ MapMode object that was created with this file");
	write_data.WriteLine("map = {}\n");

	write_data.WriteComment("The number of contexts, rows, and columns that compose the map");
	write_data.WriteInt("num_map_contexts", context_names.size());
	write_data.WriteInt("num_tile_cols", _width);
	write_data.WriteInt("num_tile_rows", _height);
	write_data.InsertNewLine();

	write_data.WriteComment("The sound files used on this map.");
	write_data.BeginTable("sound_filenames");
	// TODO: currently sound_filenames table is not populated with sounds
	write_data.EndTable();
	write_data.InsertNewLine();

	write_data.WriteComment("The music files used as background music on this map.");
	write_data.BeginTable("music_filenames");
	i = 0;
	for (QStringList::Iterator qit = music_files.begin(); qit != music_files.end(); ++qit)
	{
		i++;
		// Readd the directory name
		write_data.WriteString(i, ((*qit).prepend("mus/")).ascii());
	} // iterate through context_names writing each element
	write_data.EndTable();
	write_data.InsertNewLine();

	write_data.WriteComment("The names of the contexts used to improve Editor user-friendliness");
	write_data.BeginTable("context_names");
	i = 0;
	// First entry is the default base context. Every map has it, so no need to save it.
	QStringList::Iterator qit = context_names.begin();
	qit++;
	for (; qit != context_names.end(); ++qit)
	{
		i++;
		write_data.WriteString(i, (*qit).ascii());
	} // iterate through context_names writing each element
	write_data.EndTable();
	write_data.InsertNewLine();

	write_data.WriteComment("The names of the tilesets used, with the path and file extension omitted");
	write_data.BeginTable("tileset_filenames");
	i = 0;
	for (QStringList::Iterator qit = tileset_names.begin(); qit != tileset_names.end(); ++qit)
	{
		i++;
		write_data.WriteString(i, (*qit).ascii());
	} // iterate through tileset_names writing each element
	write_data.EndTable();
	write_data.InsertNewLine();

	write_data.WriteComment("The map grid to indicate walkability. The size of the grid is 4x the size of the tile layer tables");
	write_data.WriteComment("Walkability status of tiles for 32 contexts. Zero indicates walkable. Valid range: [0:2^32-1]");
	write_data.BeginTable("map_grid");
	vector<int32> ll_vect;
	vector<int32> ml_vect;
	vector<int32> ul_vect;
	// Used to save the northern walkability info of tiles in all layers of
	// all contexts; initialize to walkable.
	vector<int32> map_row_north(_width*2, 0);
	// Used to save the southern walkability info of tiles in all layers of
	// all contexts; initialize to walkable.
	vector<int32> map_row_south(_width*2, 0);
	for (int row = 0; row < _height; row++)
	{
		// Iterate through all contexts of all layers, column by column,
		// row by row.
		for (int context = 0; context < static_cast<int>(_lower_layer.size());
		     context++)
		{
			for (int32 col = row * _width; col < row * _width + _width; col++)
			{
				// Used to know if any tile at all on all combined layers exists.
				bool missing_tile = true;
				
				// Get walkability for lower layer tile.
				tileset_index = _lower_layer[context][col] / 256;
				if (tileset_index == 0)
					tile_index = _lower_layer[context][col];
				else  // Don't divide by 0
					tile_index = _lower_layer[context][col] %
						(tileset_index * 256);
				if (tile_index == -1)
				{
					ll_vect.push_back(0);
					ll_vect.push_back(0);
					ll_vect.push_back(0);
					ll_vect.push_back(0);
				}
				else
				{
					missing_tile = false;
					ll_vect = tilesets[tileset_index]->walkability[tile_index];
				}

				// Get walkability for middle layer tile.
				tileset_index = _middle_layer[context][col] / 256;
				if (tileset_index == 0)
					tile_index = _middle_layer[context][col];
				else  // Don't divide by 0
					tile_index = _middle_layer[context][col] %
						(tileset_index * 256);
				if (tile_index == -1)
				{
					ml_vect.push_back(0);
					ml_vect.push_back(0);
					ml_vect.push_back(0);
					ml_vect.push_back(0);
				}
				else
				{
					missing_tile = false;
					ml_vect = tilesets[tileset_index]->walkability[tile_index];
				}

				// Get walkability for upper layer tile.
				tileset_index = _upper_layer[context][col] / 256;
				if (tileset_index == 0)
					tile_index = _upper_layer[context][col];
				else  // Don't divide by 0
					tile_index = _upper_layer[context][col] %
						(tileset_index * 256);
				if (tile_index == -1)
				{
					ul_vect.push_back(0);
					ul_vect.push_back(0);
					ul_vect.push_back(0);
					ul_vect.push_back(0);
				}
				else
				{
					missing_tile = false;
					ul_vect = tilesets[tileset_index]->walkability[tile_index];
				}

				if (missing_tile == true)
				{
					// NW corner
					map_row_north[col % _width * 2]     |= 1 << context;
					// NE corner
					map_row_north[col % _width * 2 + 1] |= 1 << context;
					// SW corner
					map_row_south[col % _width * 2]     |= 1 << context;
					// SE corner
					map_row_south[col % _width * 2 + 1] |= 1 << context;
				} // no tile exists at current location
				else
				{
					// NW corner
					map_row_north[col % _width * 2]     |=
						((ll_vect[0] | ml_vect[0] | ul_vect[0]) << context);
					// NE corner
					map_row_north[col % _width * 2 + 1] |=
						((ll_vect[1] | ml_vect[1] | ul_vect[1]) << context);
					// SW corner
					map_row_south[col % _width * 2]     |=
						((ll_vect[2] | ml_vect[2] | ul_vect[2]) << context);
					// SE corner
					map_row_south[col % _width * 2 + 1] |=
						((ll_vect[3] | ml_vect[3] | ul_vect[3]) << context);
				} // a real tile exists at current location

				ll_vect.clear();
				ml_vect.clear();
				ul_vect.clear();
			} // iterate through the columns of the layers
		} // iterate through each context

		write_data.WriteIntVector(row*2,   map_row_north);
		write_data.WriteIntVector(row*2+1, map_row_south);
		map_row_north.assign(_width*2, 0);
		map_row_south.assign(_width*2, 0);
	} // iterate through the rows of the layers
	write_data.EndTable();
	write_data.InsertNewLine();

	write_data.WriteComment("The lower tile layer. The numbers are indeces to the tile_mappings table.");
	write_data.BeginTable("lower_layer");
	vector<int32>::iterator it;    // used to iterate through the layers
	vector<int32> layer_row;       // one row of a layer
	it = _lower_layer[0].begin();
	for (int row = 0; row < _height; row++)
	{
		for (int col = 0; col < _width; col++)
		{
			layer_row.push_back(*it);
			it++;
		} // iterate through the columns of the lower layer
		write_data.WriteIntVector(row, layer_row);
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
		write_data.WriteIntVector(row, layer_row);
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
		write_data.WriteIntVector(row, layer_row);
		layer_row.clear();
	} // iterate through the rows of the upper layer
	write_data.EndTable();
	write_data.InsertNewLine();

	write_data.WriteComment("All, if any, existing contexts follow.");
	vector<int32>::iterator base_it;  // used to iterate through the base layers
	vector<int32> context_data;  // one vector of ints contains all the context info
	// Iterate through all contexts of all layers.
	for (int i = 1; i < static_cast<int>(_lower_layer.size()); i++)
	{
		base_it = _lower_layer[0].begin();
		it      = _lower_layer[i].begin();
		for (int row = 0; row < _height; row++)
		{
			for (int col = 0; col < _width; col++)
			{
				if (*it != *base_it)
				{
					context_data.push_back(0);    // lower layer = 0
					context_data.push_back(row);
					context_data.push_back(col);
					context_data.push_back(*it);
				} // a valid tile exists so record it

				base_it++;
				it++;
			} // iterate through the columns of the lower layer
		} // iterate through the rows of the lower layer

		base_it = _middle_layer[0].begin();
		it      = _middle_layer[i].begin();
		for (int row = 0; row < _height; row++)
		{
			for (int col = 0; col < _width; col++)
			{
				if (*it != *base_it)
				{
					context_data.push_back(1);    // middle layer = 1
					context_data.push_back(row);
					context_data.push_back(col);
					context_data.push_back(*it);
				} // a valid tile exists so record it

				base_it++;
				it++;
			} // iterate through the columns of the middle layer
		} // iterate through the rows of the middle layer

		base_it = _upper_layer[0].begin();
		it      = _upper_layer[i].begin();
		for (int row = 0; row < _height; row++)
		{
			for (int col = 0; col < _width; col++)
			{
				if (*it != *base_it)
				{
					context_data.push_back(2);    // upper layer = 2
					context_data.push_back(row);
					context_data.push_back(col);
					context_data.push_back(*it);
				} // a valid tile exists so record it

				base_it++;
				it++;
			} // iterate through the columns of the upper layer
		} // iterate through the rows of the upper layer

		if (context_data.empty() == false)
		{
			stringstream context;
			context << "context_";
			if (i < 10)
				context << "0";
			context << i;

			write_data.WriteIntVector(context.str(), context_data);
			write_data.InsertNewLine();
			context_data.clear();
		} // write the vector if it has data in it
	} // iterate through all contexts of all layers, assuming all layers have same number of contexts

	write_data.WriteLine(AFTER_TEXT_MARKER);

	// Write the "after data" if this file is overwriting another
	if (!after_text.empty())
		write_data.WriteLine(after_text, false);

	write_data.CloseFile();

	_changed = false;
} // SaveMap()

void Grid::InsertRow(int tile_index)
{
#if !defined(WIN32)
	int row = tile_index / _width;

	for (int i = 0; i < static_cast<int>(context_names.size()); i++)
	{
		_lower_layer[i].insert(_lower_layer[i].begin() + row * _width, _width, -1);
		_middle_layer[i].insert(_middle_layer[i].begin() + row * _width, _width, -1);
		_upper_layer[i].insert(_upper_layer[i].begin() + row * _width, _width, -1);
	} // iterate through all contexts

	_height++;
	resize(_width * TILE_WIDTH, _height * TILE_HEIGHT);
#endif
} // InsertRow(...)

void Grid::InsertCol(int tile_index)
{
#if !defined(WIN32)
	int col = tile_index % _width;

	for (int i = 0; i < static_cast<int>(context_names.size()); i++)
	{
		vector<int32>::iterator it = _lower_layer[i].begin() + col;
		for (int row = 0; row < _height; row++)
		{
			it = _lower_layer[i].insert(it, -1);
			it += _width + 1;
		} // iterate through the rows of the lower layer

		it = _middle_layer[i].begin() + col;
		for (int row = 0; row < _height; row++)
		{
			it = _middle_layer[i].insert(it, -1);
			it += _width + 1;
		} // iterate through the rows of the middle layer

		it = _upper_layer[i].begin() + col;
		for (int row = 0; row < _height; row++)
		{
			it = _upper_layer[i].insert(it, -1);
			it += _width + 1;
		} // iterate through the rows of the upper layer
	} // iterate through all contexts

	_width++;
	resize(_width * TILE_WIDTH, _height * TILE_HEIGHT);
#endif
} // InsertCol(...)

void Grid::DeleteRow(int tile_index)
{
#if !defined(WIN32)
	int row = tile_index / _width;

	for (int i = 0; i < static_cast<int>(context_names.size()); i++)
	{
		_lower_layer[i].erase(_lower_layer[i].begin() + row * _width,
			_lower_layer[i].begin() + row * _width + _width);
		_middle_layer[i].erase(_middle_layer[i].begin() + row * _width,
			_middle_layer[i].begin() + row * _width + _width);
		_upper_layer[i].erase(_upper_layer[i].begin() + row * _width,
			_upper_layer[i].begin() + row * _width + _width);
	} // iterate through all contexts

	_height--;
	resize(_width * TILE_WIDTH, _height * TILE_HEIGHT);
#endif
} // DeleteRow(...)

void Grid::DeleteCol(int tile_index)
{
#if !defined(WIN32)
	int col = tile_index % _width;

	for (int i = 0; i < static_cast<int>(context_names.size()); i++)
	{
		vector<int32>::iterator it = _lower_layer[i].begin() + col;
		for (int row = 0; row < _height; row++)
		{
			it = _lower_layer[i].erase(it);
			it += _width - 1;
		} // iterate through the rows of the lower layer

		it = _middle_layer[i].begin() + col;
		for (int row = 0; row < _height; row++)
		{
			it = _middle_layer[i].erase(it);
			it += _width - 1;
		} // iterate through the rows of the middle layer

		it = _upper_layer[i].begin() + col;
		for (int row = 0; row < _height; row++)
		{
			it = _upper_layer[i].erase(it);
			it += _width - 1;
		} // iterate through the rows of the upper layer
	} // iterate through all contexts

	_width--;
	resize(_width * TILE_WIDTH, _height * TILE_HEIGHT);
#endif
} // DeleteCol(...)



// ********** Protected functions **********

void Grid::initializeGL()
{
	// Destroy the video engine
	VideoManager->SingletonDestroy();
	// Recreate the video engine's singleton
	VideoManager = VideoEngine::SingletonCreate();
	VideoManager->SetTarget(VIDEO_TARGET_QT_WIDGET);

	VideoManager->SingletonInitialize();

	VideoManager->ApplySettings();
	VideoManager->FinalizeInitialization();
	VideoManager->ToggleFPS();
} // initializeGL()

void Grid::paintGL()
{
	int32 col, row;                  // tile array loop index
	vector<int32>::iterator it;      // used to iterate through tile arrays
	int tileset_index;               // index into the tileset_names vector
	int tile_index;                  // ranges from 0-255

	if (_initialized == false)
		return;

	// Setup drawing parameters
	VideoManager->SetCoordSys(0.0f, VideoManager->GetScreenWidth() / TILE_WIDTH,
		VideoManager->GetScreenHeight() / TILE_HEIGHT, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);
	VideoManager->Clear(Color::black);

	// Draw lower layer
	if (_ll_on)
	{
		// Calculate which tiles are visible
		// Set left, right, and width
		int sbval = _ed_scrollview->horizontalScrollBar()->value();
		int viewwidth = _ed_scrollview->width();
		int num_tiles_width = ((viewwidth - 21) / 32) + 1;
		
		int left_tile = sbval / 32;
		int right_tile = left_tile + num_tiles_width;
		right_tile = (right_tile < _width) ? right_tile : _width - 1;
		
		// Set top, bottom, and height
		sbval = _ed_scrollview->verticalScrollBar()->value();
		int viewheight = _ed_scrollview->height();
		int num_tiles_height = ((viewheight - 21) / 32) + 1;
		
		int top_tile = sbval / 32;
		int bottom_tile = top_tile + num_tiles_height;
		bottom_tile = (bottom_tile < _height) ? bottom_tile : _height - 1;


		VideoManager->Move(left_tile, top_tile);

		int layer_index;
		col = left_tile;
		row = top_tile;
		int i = 0;
		while (row <= bottom_tile)
		{
			layer_index = _lower_layer[_context][row * _width + col];
			if (layer_index != -1)
			{
				tileset_index = layer_index / 256;
				if (tileset_index == 0)
					tile_index = layer_index;
				else  // Don't divide by 0
					tile_index = layer_index % (tileset_index * 256);
				tilesets[tileset_index]->tiles[tile_index].Draw();
			} // a tile exists to draw
			if (col == right_tile) {
				col = left_tile;
				row++;
				VideoManager->MoveRelative(-(right_tile - left_tile), 1.0f);
			}
			else {
				col++;
				VideoManager->MoveRelative(1.0f, 0.0f);
			}
			i++;
			
		} // iterate through lower layer
//		std::cout << "Drew this many tiles: " << i << std::endl;
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

	// Draw object layer
	if (_ol_on)
	{
		for ( std::list<MapSprite* >::iterator sprite = sprites.begin(); sprite != sprites.end(); sprite++ )
		{
			if ( (*sprite) != NULL )
				if ( (*sprite)->GetContext() == static_cast<uint32>(_context) )
				{
					VideoManager->Move( (*sprite)->ComputeDrawXLocation() - 0.2f,
										(*sprite)->ComputeDrawYLocation() + (*sprite)->img_height*3/8 - 0.4f );
					(*sprite)->DrawSelection();
					VideoManager->Move( (*sprite)->ComputeDrawXLocation(), (*sprite)->ComputeDrawYLocation() );
					(*sprite)->Draw();
					(*sprite)->Update();

				} // a sprite exists to draw
		} // iterate through object layer
	} // object layer must be viewable

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

