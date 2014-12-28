///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2011 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    map_data.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for map data class
*** **************************************************************************/

#include "script.h"
#include "common.h"

#include "editor_utils.h"
#include "map_data.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_script;
using namespace hoa_common;


namespace hoa_editor {

///////////////////////////////////////////////////////////////////////////////
// MapData class -- General Functions
///////////////////////////////////////////////////////////////////////////////

MapData::MapData() :
	_map_filename(""),
	_map_name(""),
	_map_designers(""),
	_map_description(""),
	_map_length(0),
	_map_height(0),
	_map_modified(false),
	_tile_layer_count(0),
	_tile_context_count(0),
	_selected_tile_layer(NULL),
	_selected_tile_context(NULL),
	_all_tile_contexts(MAX_CONTEXTS, NULL),
	_error_message("")
{}



bool MapData::CreateData(uint32 map_length, uint32 map_height) {
	if (IsInitialized() == true) {
		return false;
	}

	_map_length = map_length;
	_map_height = map_height;
	_empty_tile_layer._ResizeLayer(map_length, map_height);
	_empty_tile_layer.FillLayer(NO_TILE);

	// Create three tile layers, the last of which has no collision enabled initially
	_tile_layer_properties.push_back(TileLayerProperties(QString("Ground"), true, true));
	_tile_layer_properties.push_back(TileLayerProperties(QString("Detail"), true, true));
	_tile_layer_properties.push_back(TileLayerProperties(QString("Sky"), true, false));
	_tile_layer_count = 3;

	// Create a single TileContext called "Base"
	TileContext* new_context = new TileContext(1, "Base"); // Give an ID of 1 since no other contexts exist yet
	new_context->_AddTileLayer(_empty_tile_layer);
	new_context->_AddTileLayer(_empty_tile_layer);
	new_context->_AddTileLayer(_empty_tile_layer);
	_all_tile_contexts[0] = new_context;
	_tile_context_count = 1;

	_selected_tile_context = new_context;
	_selected_tile_layer = new_context->GetTileLayer(0);
	return true;
}



void MapData::DestroyData() {
	_map_filename = "";
	_map_name = "";
	_map_length = 0;
	_map_height = 0;
	_tile_layer_count = 0;
	_tile_layer_properties.clear();

	for (uint32 i = 0; i < _tilesets.size(); ++i) {
		delete _tilesets[i];
	}
	_tilesets.clear();

	for (uint32 i = 0; i < _all_tile_contexts.size(); ++i) {
		if (_all_tile_contexts[i] == NULL)
			break;

		delete _all_tile_contexts[i];
		_all_tile_contexts[i] = NULL;
	}

	_selected_tile_context = NULL;
	_selected_tile_layer = NULL;
	_tile_context_count = 0;

	_error_message = "";
}



bool MapData::LoadData(QString filename) {
	if (IsInitialized() == true) {
		return false;
	}

	// ---------- (1): Open the file and open the tablespace table, then clear any existing data before reading begins
	ReadScriptDescriptor data_file;
	if (data_file.OpenFile(string(filename.toAscii()), true) == false) {
		_error_message = "Could not open file " + filename + " for reading.";
		return false;
	}

	string tablespace = DetermineLuaFileTablespaceName(filename.toStdString());
	if (data_file.DoesTableExist(tablespace) == false) {
		_error_message = "Map file " + filename.toAscii() + " did not have the expected namespace table " + tablespace.c_str();
		return false;
	}

	data_file.OpenTable(tablespace);

	DestroyData();
	_map_filename = filename;

	// ---------- (2): Read the basic map data properties
	_map_name = QString::fromStdString(data_file.ReadString("map_name"));
	_map_designers = QString::fromStdString(data_file.ReadString("map_designers"));
	_map_description = QString::fromStdString(data_file.ReadString("map_description"));
	_map_length = data_file.ReadUInt("map_length");
	_map_height = data_file.ReadUInt("map_height");
 	uint32 number_tilesets = data_file.ReadUInt("number_tilesets");
	_tile_layer_count = data_file.ReadUInt("number_tile_layers");
	_tile_context_count = data_file.ReadUInt("number_map_contexts");
	_empty_tile_layer._ResizeLayer(_map_length, _map_height);
	_empty_tile_layer.FillLayer(NO_TILE);

	if (_map_length < MINIMUM_MAP_LENGTH) {
		_error_message = QString("Error when loading map file. Map was smaller (%1) than the minimum length.").arg(_map_length);
		data_file.CloseTable();
		data_file.CloseFile();
		DestroyData();
		return false;
	}
	if (_map_height < MINIMUM_MAP_HEIGHT) {
		_error_message = QString("Error when loading map file. Map was smaller (%1) than the minimum height.").arg(_map_height);
		data_file.CloseTable();
		data_file.CloseFile();
		DestroyData();
		return false;
	}
	if (_tile_layer_count == 0) {
		_error_message = QString("Error when loading map file. Map did not have any tile layers.");
		data_file.CloseTable();
		data_file.CloseFile();
		DestroyData();
		return false;
	}
	if (_tile_context_count == 0) {
		_error_message = QString("Error when loading map file. Map did not have any contexts.");
		data_file.CloseTable();
		data_file.CloseFile();
		DestroyData();
		return false;
	}

	// ---------- (3): Construct each tileset object for the map
	vector<string> tileset_filenames;
	data_file.ReadStringVector("tileset_filenames", tileset_filenames);
	if (tileset_filenames.empty() == true) {
		_error_message = QString("Error when loading map file. Map did use any tile contexts.");
		data_file.CloseTable();
		data_file.CloseFile();
		DestroyData();
		return false;
	}

	for (uint32 i = 0; i < number_tilesets; ++i) {
		Tileset* tileset = new Tileset();
		QString tileset_qname = QString::fromStdString(tileset_filenames[i].c_str());
		if (tileset->Load(tileset_qname) == false) {
			_error_message = QString("Failed to load tileset file ") + tileset_qname + QString(" during loading of map file ") + _map_filename;
			delete tileset;
			return false;
		}
		AddTileset(tileset);
	}

	// ---------- (4): Read in the properties of tile layers and tile contexts
	vector<string> tile_layer_names;
	vector<bool> tile_layer_collision_enabled;
	data_file.ReadStringVector("tile_layer_names", tile_layer_names);
	data_file.ReadBoolVector("tile_layer_collision_enabled", tile_layer_collision_enabled);

	vector<string> tile_context_names;
	vector<int32> tile_context_inheritance;
	data_file.ReadStringVector("map_context_names", tile_context_names);
	data_file.ReadIntVector("map_context_inheritance", tile_context_inheritance);

	// ---------- (5): Construct each tile context and layer and initialize it with empty data
	for (uint32 i = 0; i < _tile_layer_count; ++i) {
		_tile_layer_properties.push_back(TileLayerProperties(QString::fromStdString(tile_layer_names[i]), true, tile_layer_collision_enabled[i]));
	}

	for (uint32 i = 0; i < _tile_context_count; ++i) {
		TileContext* new_context = new TileContext(i + 1, QString::fromStdString(tile_context_names[i]));
		if (tile_context_inheritance[i] != NO_CONTEXT) {
			new_context->_SetInheritingContext(tile_context_inheritance[i]);
		}
		for (uint32 j = 0; j < _tile_layer_count; ++j) {
			new_context->_AddTileLayer(_empty_tile_layer);
		}

		_all_tile_contexts[i] = new_context;
	}

	_selected_tile_context = _all_tile_contexts[0];
	_selected_tile_layer = _selected_tile_context->GetTileLayer(0);

	// ---------- (6): Read in the collision grid data
	_collision_data.resize(_map_height * 2);
	for (uint32 y = 0; y < _map_height * 2; ++y) {
		_collision_data[y].reserve(_map_length * 2);
	}

	data_file.OpenTable("collision_grid");
	for (uint32 y = 0; y < _map_height * 2; ++y) {
		data_file.ReadUIntVector(y, _collision_data[y]);
	}
	data_file.CloseTable();

	// ---------- (7): Read the map tile data into the appropriate layers of each tile context
	vector<int32> tile_data; // Container used to read in all the data for a tile corresponding to one X, Y coordinate
	tile_data.reserve(_tile_context_count * _tile_layer_count);

	vector<vector<std::vector<int32> >* > layer_tiles; // Holds pointers to the tile vectors within each context and layer
	for (uint32 c = 0; c < _tile_context_count; ++c) {
		for (uint32 l = 0; l < _tile_layer_count; ++l) {
			layer_tiles.push_back(&_all_tile_contexts[c]->GetTileLayer(l)->GetTiles());
		}
	}

	data_file.OpenTable("map_tiles");
	for (uint32 y = 0; y < _map_height; ++y) {
		data_file.OpenTable(y);
		for (uint32 x = 0; x < _map_length; ++x) {
			tile_data.clear();
			data_file.ReadIntVector(x, tile_data);
			for (uint32 t = 0; t < tile_data.size(); ++t) {
				(*layer_tiles[t])[y][x] = tile_data[t];
			}
		}
		data_file.CloseTable();
	}
	data_file.CloseTable();

	if (data_file.IsErrorDetected()) {
		_error_message = QString("One or more errors were detected when reading in the map file:\n") + QString::fromStdString(data_file.GetErrorMessages());
		data_file.CloseTable();
		data_file.CloseFile();
		_map_modified = false;
		return false;
	}

	data_file.CloseTable();
	data_file.CloseFile();
	_map_modified = false;
	return true;
} // bool MapData::LoadData(QString filename)



bool MapData::SaveData(QString filename) {
	if (IsInitialized() == false) {
		return false;
	}

	// ---------- (1): Open the file and write the tablespace header and map header information
	WriteScriptDescriptor data_file;
	if (data_file.OpenFile(filename.toStdString()) == false) {
		_error_message = "Could not open file for writing: " + filename;
		return false;
	}

	data_file.WriteNamespace(DetermineLuaFileTablespaceName(filename.toStdString()));
	data_file.InsertNewLine();

	data_file.WriteString("map_name", _map_name.toStdString());
	data_file.WriteString("map_designers", _map_designers.toStdString());
	data_file.WriteString("map_description", _map_description.toStdString());
	data_file.InsertNewLine();

	// ---------- (2): Write the basic map data properties
	data_file.WriteUInt("map_length", _map_length);
	data_file.WriteUInt("map_height", _map_height);
 	data_file.WriteUInt("number_tilesets", _tilesets.size());
	data_file.WriteUInt("number_tile_layers", _tile_layer_count);
	data_file.WriteUInt("number_map_contexts", _tile_context_count);
	data_file.InsertNewLine();

	// ---------- (3): Write properties of tilesets, tile layers, and map contexts
	data_file.BeginTable("tileset_filenames");
	for (uint32 i = 0; i < _tilesets.size(); ++i) {
		data_file.WriteString((i+1), _tilesets[i]->GetTilesetDefinitionFilename().toStdString());
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	data_file.BeginTable("tile_layer_names");
	QStringList layer_names = GetTileLayerNames();
	for (int32 i = 0; i < layer_names.size(); ++i) {
		data_file.WriteString((i+1), layer_names[i].toStdString());
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	data_file.BeginTable("tile_layer_collision_enabled");
	for (uint32 i = 0; i < _tile_layer_properties.size(); ++i) {
		data_file.WriteBool((i+1), _tile_layer_properties[i].IsCollisionEnabled());
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	data_file.BeginTable("map_context_names");
	QStringList context_names = GetTileContextNames();
	for (int32 i = 0; i < context_names.size(); ++i) {
		data_file.WriteString((i+1), context_names[i].toStdString());
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	data_file.BeginTable("map_context_inheritance");
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		data_file.WriteInt((i+1), _all_tile_contexts[i]->GetInheritedContextID());
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	// ---------- (4): Write collision grid data
	data_file.BeginTable("collision_grid");
	_ComputeCollisionData();
	for (uint32 i = 0; i < _collision_data.size(); ++i) {
		data_file.WriteUIntVector(i, _collision_data[i]);
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	// ---------- (5): For each tile, write the tile value for each layer and each context
	vector<int32> tiles(_tile_context_count * _tile_layer_count, NO_TILE);
	data_file.BeginTable("map_tiles");
	for (uint32 y = 0; y < _map_height; ++y) {
		data_file.DeclareTable(y);
	}
	for (uint32 y = 0; y < _map_height; ++y) {
		data_file.OpenTable(y);
		for (uint32 x = 0; x < _map_length; ++x) {
			for (uint32 c = 0; c < _tile_context_count; ++c) {
				for (uint32 l = 0; l < _tile_layer_count; ++l) {
					tiles[(c * _tile_layer_count) + l] = _all_tile_contexts[c]->GetTileLayer(l)->GetTile(x, y);
				}
			}
			data_file.WriteIntVector(x, tiles);
		}
		data_file.EndTable();
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	if (data_file.IsErrorDetected()) {
		_error_message = "One or more errors occurred when writing map file:\n" + QString::fromStdString(data_file.GetErrorMessages());
		data_file.CloseFile();
		_map_modified = false;
		return false;
	}

	data_file.CloseFile();
	_map_modified = false;
	return true;
} // bool MapData::SaveData(QString filename)



void MapData::ResizeMap(uint32 number_cols, uint32 number_rows) {
	// TODO: modify the size of all layers in each context
}

///////////////////////////////////////////////////////////////////////////////
// MapData class -- Tileset Functions
///////////////////////////////////////////////////////////////////////////////

bool MapData::AddTileset(Tileset* new_tileset) {
	if (new_tileset == NULL) {
		_error_message = "ERROR: function received NULL pointer argument";
		return false;
	}

	if (new_tileset->IsInitialized() == false) {
		_error_message = "ERROR: function received uninitialized tileset object";
		return false;
	}

	for (uint32 i = 0; i < _tilesets.size(); ++i) {
		if (_tilesets[i] == new_tileset) {
			_error_message = "ERROR: tileset was already added to map data";
			return false;
		}

		if (_tilesets[i]->GetTilesetDefinitionFilename() == new_tileset->GetTilesetDefinitionFilename()) {
			_error_message = "ERROR: a tileset with the same definition file already exists within the map data";
			return false;
		}
	}

	_tilesets.push_back(new_tileset);
	_tileset_names.push_back(new_tileset->GetTilesetName());
	return true;
}



void MapData::RemoveTileset(uint32 tileset_index) {
	if (tileset_index >= _tilesets.size()) {
		_error_message = "ERROR: no tileset exists at index " + QString(NumberToString(tileset_index).c_str());
		return;
	}

	delete _tilesets[tileset_index];

	// Shift all remaining tilesets over, then remove the last entry in the list
	for (uint32 i = tileset_index + 1; i < _tilesets.size(); ++i) {
		_tilesets[i-1] = _tilesets[i];
		_tileset_names[i-1] = _tileset_names[i];
	}
	_tilesets.pop_back();
	_tileset_names.pop_back();
}



void MapData::MoveTilesetUp(uint32 tileset_index) {
	if (tileset_index >= _tilesets.size()) {
		_error_message = "ERROR: no tileset exists at index " + QString(NumberToString(tileset_index).c_str());
		return;
	}

	if (tileset_index == 0) {
		_error_message = "WARN: tileset could not be moved further down at index " + QString(NumberToString(tileset_index).c_str());
		return;
	}

	Tileset* temp_tileset = _tilesets[tileset_index - 1];
	_tilesets[tileset_index - 1] = _tilesets[tileset_index];
	_tilesets[tileset_index] = temp_tileset;

	QString temp_name = _tileset_names[tileset_index - 1];
	_tileset_names[tileset_index - 1] = _tileset_names[tileset_index];
	_tileset_names[tileset_index] = temp_name;
}



void MapData::MoveTilesetDown(uint32 tileset_index) {
	if (tileset_index >= _tilesets.size()) {
		_error_message = "ERROR: no tileset exists at index " + QString(NumberToString(tileset_index).c_str());
		return;
	}

	if (tileset_index == _tilesets.size() - 1) {
		_error_message = "WARN: tileset could not be moved further up at index " + QString(NumberToString(tileset_index).c_str());
		return;
	}

	Tileset* temp_tileset = _tilesets[tileset_index + 1];
	_tilesets[tileset_index + 1] = _tilesets[tileset_index];
	_tilesets[tileset_index] = temp_tileset;

	QString temp_name = _tileset_names[tileset_index + 1];
	_tileset_names[tileset_index + 1] = _tileset_names[tileset_index];
	_tileset_names[tileset_index] = temp_name;
}

///////////////////////////////////////////////////////////////////////////////
// MapData class -- Tile Layer Functions
///////////////////////////////////////////////////////////////////////////////

TileLayer* MapData::ChangeSelectedTileLayer(uint32 layer_index) {
	if (layer_index >= _tile_layer_count) {
		IF_PRINT_WARNING(EDITOR_DEBUG) << "could not change selection because no layer with this index exists: " << layer_index << endl;
		return NULL;
	}

	_selected_tile_layer = _selected_tile_context->GetTileLayer(layer_index);
	return _selected_tile_layer;
}



QStringList MapData::GetTileLayerNames() const {
	QStringList layer_names;
	for (uint32 i = 0; i < _tile_layer_count; ++i) {
		layer_names.append(_tile_layer_properties[i].GetName());
	}

	return layer_names;
}



void MapData::ToggleTileLayerVisibility(uint32 layer_index) {
	if (layer_index > _tile_layer_count)
		return;

	bool visible = _tile_layer_properties[layer_index].IsVisible();
	_tile_layer_properties[layer_index].SetVisible(!visible);
}



void MapData::ToggleTileLayerCollision(uint32 layer_index) {
	if (layer_index > _tile_layer_count)
		return;

	bool collisions = _tile_layer_properties[layer_index].IsCollisionEnabled();
	_tile_layer_properties[layer_index].SetCollisionEnabled(!collisions);
}



bool MapData::AddTileLayer(QString name, bool collision_on) {
	// Check that the name will be unique among all existing tile layers before adding
	QStringList layer_names = GetTileLayerNames();
	int32 name_index = layer_names.indexOf(name);
	if (name_index != -1) {
		_error_message = "ERROR: a tile layer with this name already exists";
		return false;
	}

	_tile_layer_count += 1;
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		_all_tile_contexts[i]->_AddTileLayer(_empty_tile_layer);
	}
	_tile_layer_properties.push_back(TileLayerProperties(name, true, collision_on));

	return true;
}



bool MapData::DeleteTileLayer(uint32 layer_index) {
	if (layer_index >= _tile_layer_count) {
		_error_message = "ERROR: no tile layer exists at this index";
		return false;
	}

	// Delete the layer from each context
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		_all_tile_contexts[i]->_RemoveTileLayer(layer_index);
	}

	// Remove the corresponding entry from the layer properties
	for (uint32 i = layer_index; i < _tile_layer_count - 1; ++i) {
		_tile_layer_properties[i] = _tile_layer_properties[i+1];
	}
	_tile_layer_properties.pop_back();

	_tile_layer_count--;
	return true;
}



bool MapData::RenameTileLayer(uint32 layer_index, QString new_name) {
	if (layer_index > _tile_layer_count) {
		_error_message = "ERROR: no tile layer exists at this index";
		return false;
	}

	// Check for the case where the name doesn't actually change
	if (_tile_layer_properties[layer_index].GetName() == new_name) {
		return true;
	}

	// Check that the name will be unique among all existing tile layers before renaming
	QStringList layer_names = GetTileLayerNames();
	if (layer_names.indexOf(new_name) != -1) {
		_error_message = "ERROR: a tile layer with this name already exists";
		return false;
	}

	_tile_layer_properties[layer_index].SetName(new_name);
	return true;
}



bool MapData::SwapTileLayers(uint32 index_one, uint32 index_two) {
	if (index_one == index_two) {
		_error_message = "WARN: tried to use same index to swap two tile layers";
		return false;
	}
	if (index_one >= _tile_layer_count) {
		_error_message = "ERROR: no tile layer exists at first layer index";
		return false;
	}
	if (index_two >= _tile_layer_count) {
		_error_message = "ERROR: no tile layer exists at second layer index";
		return false;
	}

	for (uint32 i = 0; i < _tile_context_count; ++i) {
		_all_tile_contexts[i]->_SwapTileLayers(index_one, index_two);
	}

	// Move the layer properties up
	TileLayerProperties swap = _tile_layer_properties[index_two];
	_tile_layer_properties[index_two] = _tile_layer_properties[index_one];
	_tile_layer_properties[index_one] = _tile_layer_properties[index_two];

	return true;
}



void MapData::InsertTileLayerRows(uint32 row_index, uint32 row_count) {
	if (row_index >= _map_height) {
		return;
	}

	if (row_count == 0) {
		return;
	}

	for (uint32 i = 0; i < _tile_context_count; ++i) {
		vector<TileLayer>& layers = _all_tile_contexts[i]->GetTileLayers();
		for (uint32 j = 0; j < _tile_layer_count; ++j) {
			layers[j]._AddLayerRow(row_index, NO_TILE);
		}
	}

	_map_height = _map_height + row_count;
}



void MapData::RemoveTileLayerRows(uint32 row_index, uint32 row_count) {
	if (row_index >= _map_height) {
		return;
	}

	if (row_count == 0) {
		return;
	}

	if (row_count > (_map_height - MINIMUM_MAP_HEIGHT)) {
		return;
	}

	for (uint32 i = 0; i < _tile_context_count; ++i) {
		vector<TileLayer>& layers = _all_tile_contexts[i]->GetTileLayers();
		for (uint32 j = 0; j < _tile_layer_count; ++j) {
			layers[j]._AddLayerRow(row_index, NO_TILE);
		}
	}

	_map_height = _map_height - row_count;
}



void MapData::InsertTileLayerColumns(uint32 col_index, uint32 col_count) {
	if (col_index >= _map_length) {
		return;
	}

	if (col_count == 0) {
		return;
	}

	for (uint32 i = 0; i < _tile_context_count; ++i) {
		vector<TileLayer>& layers = _all_tile_contexts[i]->GetTileLayers();
		for (uint32 j = 0; j < _tile_layer_count; ++j) {
			layers[j]._AddLayerCol(col_index, NO_TILE);
		}
	}

	_map_length = _map_length + col_count;
}



void MapData::RemoveTileLayerColumns(uint32 col_index, uint32 col_count) {
	if (col_index >= _map_length) {
		return;
	}

	if (col_count == 0) {
		return;
	}

	if (col_count > (_map_length - MINIMUM_MAP_LENGTH)) {
		return;
	}

	for (uint32 i = 0; i < _tile_context_count; ++i) {
		vector<TileLayer>& layers = _all_tile_contexts[i]->GetTileLayers();
		for (uint32 j = 0; j < _tile_layer_count; ++j) {
			layers[j]._DeleteLayerCol(col_index);
		}
	}

	_map_length = _map_length - col_count;
}

///////////////////////////////////////////////////////////////////////////////
// MapData class -- Tile Context Functions
///////////////////////////////////////////////////////////////////////////////

TileContext* MapData::ChangeSelectedTileContext(int32 context_id) {
	if (context_id <= 0) {
		return NULL;
	}
	if (static_cast<uint32>(context_id) > _tile_context_count) {
		IF_PRINT_WARNING(EDITOR_DEBUG) << "could not change selection because no context with this ID exists: " << context_id << endl;
		return NULL;
	}

	// Before changing the context, figure out the index of the selected tile layer for the current context
	uint32 layer_index = 0;
	for (uint32 i = 0; i < _tile_layer_count; ++i) {
		if (_selected_tile_context->GetTileLayer(i) == _selected_tile_layer) {
			layer_index = i;
			break;
		}
	}
	_selected_tile_context = _all_tile_contexts[context_id - 1];
	ChangeSelectedTileLayer(layer_index);
	return _selected_tile_context;
}



QStringList MapData::GetTileContextNames() const {
	QStringList context_names;
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		if (_all_tile_contexts[i] == NULL)
			break;

		context_names.append(_all_tile_contexts[i]->GetContextName());
	}

	return context_names;
}



QStringList MapData::GetInheritedTileContextNames() const {
	QStringList name_list;

	// Contexts that do not inherit have an empty string placed in the name list
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		if (_all_tile_contexts[i]->IsInheritingContext() == false) {
			name_list.append("");
		}
		else {
			uint32 inherit_index = _all_tile_contexts[i]->GetInheritedContextID();
			name_list.append(_all_tile_contexts[inherit_index]->GetContextName());
		}
	}

	return name_list;
}



TileContext* MapData::AddTileContext(QString name, int32 inheriting_context_id) {
	// Check all conditions where we would not be able to create the new context

	if (_all_tile_contexts.back() != NULL) {
		_error_message = "ERROR: could not add new context as the maximum number of contexts has been reached";
		return NULL;
	}
	if (name.isEmpty() == true) {
		_error_message = "ERROR: tile context must have a name";
		return NULL;
	}
	if (FindTileContextByName(name) != NULL) {
		_error_message = "ERROR: a context with this name already exists";
		return NULL;
	}
	if (inheriting_context_id != NO_CONTEXT) {
		if (inheriting_context_id <= 0 || static_cast<uint32>(inheriting_context_id) > MAX_CONTEXTS) {
			_error_message = "ERROR: invalid value for inhertiting context ID";
			return NULL;
		}
		if (_all_tile_contexts[inheriting_context_id - 1] == NULL) {
			_error_message = "ERROR: no context exists for the requested inheriting context ID";
			return NULL;
		}
	}

	// Create the new context and add it to the bottom of the context list
	uint32 new_id = _tile_context_count + 1;
	TileContext* new_context = new TileContext(new_id, name, inheriting_context_id);
	for (uint32 i = 0; i < _tile_layer_count; ++i) {
		new_context->_AddTileLayer(_empty_tile_layer);
	}
	_all_tile_contexts[_tile_context_count] = new_context;
	_tile_context_count++;

	return new_context;
}



bool MapData::DeleteTileContext(int32 context_id) {
	TileContext* context = FindTileContextByID(context_id);
	// Check all conditions where we would not be able to delete the context
	if (context == NULL) {
		_error_message = "ERROR: received invalid context ID";
		return false;
	}
	if (_tile_context_count <= 1) {
		_error_message = "ERROR: can not delete the last remaining context for the map";
		return false;
	}
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		if (context_id == _all_tile_contexts[i]->GetInheritedContextID()) {
			_error_message = "ERROR: could not delete context as it is being inherited by one or more additional contexts";
			return false;
		}
	}

	// Move the context to delete to the end of the list
	for (uint32 i = static_cast<uint32>(context_id); i < _tile_context_count; ++i) {
		SwapTileContexts(i, i + 1);
	}

	delete context;
	_all_tile_contexts[_tile_context_count - 1] = NULL;
	_tile_context_count--;

	return true;
}



bool MapData::RenameTileContext(int32 context_id, QString new_name) {
	if (context_id <= 0) {
		return false;
	}
	if (static_cast<uint32>(context_id) > _tile_context_count) {
		_error_message = "ERROR: context_id exceeds size of context list";
		return false;
	}

	if (_all_tile_contexts[context_id - 1]->GetContextName() == new_name) {
		return true;
	}

	QStringList context_names = GetTileContextNames();
	if (context_names.indexOf(new_name) != -1) {
		_error_message = "ERROR: a context with this name already exists";
		return false;
	}

	_all_tile_contexts[context_id - 1]->SetContextName(new_name);
	return true;
}



bool MapData::SwapTileContexts(int32 first_id, int32 second_id) {
	if (first_id <= 0 || second_id <= 0) {
		_error_message = "ERROR: invalid context ID passed when trying to swap context positions";
		return false;
	}
	if (first_id == second_id) {
		_error_message = "ERROR: tried to swap two contexts with the same ID";
		return false;
	}
	if (static_cast<uint32>(first_id) > _tile_context_count) {
		_error_message = "ERROR: no tile context exists at first context ID";
		return false;
	}
	if (static_cast<uint32>(second_id) > _tile_layer_count) {
		_error_message = "ERROR: no tile context exists at second context ID";
		return false;
	}

	uint32 first_index = static_cast<uint32>(first_id - 1);
	uint32 second_index = static_cast<uint32>(second_id - 1);

	// Perform the swap and update each context's ID to match it's new position in the container
	TileContext* swap = _all_tile_contexts[first_index];
	_all_tile_contexts[first_index] = _all_tile_contexts[second_index];
	_all_tile_contexts[first_index]->_SetContextID(first_id);
	_all_tile_contexts[second_index] = swap;
	_all_tile_contexts[second_index]->_SetContextID(second_id);

	// Go through each context and see if it inherited from either the first or the second context. Update these values appropriately
	for (uint32 i = 0; i < _all_tile_contexts.size(); ++i) {
		if (_all_tile_contexts[i] == NULL)
			break;

		int32 inherited_id = _all_tile_contexts[i]->GetInheritedContextID();
		if (inherited_id == first_id)
			_all_tile_contexts[i]->_SetInheritingContext(second_id);
		else if (inherited_id == second_id)
			_all_tile_contexts[i]->_SetInheritingContext(first_id);
	}

	return true;
}



TileContext* MapData::FindTileContextByID(int32 context_id) const {
	if (context_id <= 0 || static_cast<uint32>(context_id) > MAX_CONTEXTS) {
		return NULL;
	}

	if (context_id > static_cast<int32>(_tile_context_count)) {
		return NULL;
	}

	return _all_tile_contexts[context_id - 1];
}



TileContext* MapData::FindTileContextByName(QString context_name) const {
	for (uint32 i = 0; i < _all_tile_contexts.size(); ++i) {
		if (_all_tile_contexts[i] == NULL)
			break;

		if (_all_tile_contexts[i]->GetContextName() == context_name)
			return _all_tile_contexts[i];
	}

	return NULL;
}



TileContext* MapData::FindTileContextByIndex(uint32 context_index) const {
	if (context_index >= _all_tile_contexts.size())
		return NULL;

	return _all_tile_contexts[context_index];
}



void MapData::_ComputeCollisionData() {
	// Resize the container to hold each grid element that will be computed
	_collision_data.resize(_map_height * 2);
	for (uint32 i = 0; i < _map_height * 2; ++i) {
		_collision_data[i].resize(_map_length * 2, 0);
	}

	// Holds the indexes of only the tile layers that have their collision data enabled
	vector<uint32> collision_layers;
	for (uint32 i = 0; i < _tile_layer_properties.size(); ++i) {
		if (_tile_layer_properties[i].IsCollisionEnabled() == true)
			collision_layers.push_back(i);
	}

	// The context being processed
	TileContext* context = NULL;
	// A bit-mask used to put the collision data into the proper bit based on the context's ID
	uint32 context_mask = 0;

	// The value of the current tile being processed
	int32 tile = 0;
	// The indexes to the proper tileset and that tileset's collision data for the tile being processed
	uint32 tileset_index = 0;
	uint32 tileset_collision_index = 0;

	// Holds the indexes to the _collision_data for each of the four collision quadrants of a single tile
	uint32 north_index = 0;
	uint32 south_index = 0;
	uint32 west_index = 0;
	uint32 east_index = 0;
	for (uint32 c = 0; c < _tile_context_count; ++c) {
		// This mask is used to set the appropriate bit for this context
		context_mask = 0x00000001 << c;
		context = _all_tile_contexts[c];

		// Iterate through each tile in the map and extract the collision data from each
		for (uint32 y = 0; y < _map_height; ++y) {
			north_index = y * 2;
			south_index = north_index + 1;
			for (uint32 x = 0; x < _map_length; ++x) {
				west_index = x * 2;
				east_index = west_index + 1;
				for (uint32 l = 0; l < collision_layers.size(); ++l) {
					tile = context->GetTileLayer(collision_layers[l])->GetTile(x, y);
					// TODO: if this tile comes from an inherited context, do we want to include the collision data from that inherited tile?
					// Or do we want to leave out all collision information entirely, as we do here currently?
					if (tile < 0) {
						continue;
					}

					// Determine the tileset that this tile belongs to and the location of the tile within that set
					tileset_index = tile / TILESET_NUM_TILES;
					tile = tile % TILESET_NUM_TILES;
					tileset_collision_index = tile * TILE_NUM_QUADRANTS;

					if (_tilesets[tileset_index]->GetQuadrantCollision(tileset_collision_index) != 0)
						_collision_data[north_index][west_index] |= context_mask;
					if (_tilesets[tileset_index]->GetQuadrantCollision(tileset_collision_index + 1) != 0)
						_collision_data[north_index][east_index] |= context_mask;
					if (_tilesets[tileset_index]->GetQuadrantCollision(tileset_collision_index + 2) != 0)
						_collision_data[south_index][west_index] |= context_mask;
					if (_tilesets[tileset_index]->GetQuadrantCollision(tileset_collision_index + 3) != 0)
						_collision_data[south_index][east_index] |= context_mask;
				}
			}
		}
	}
} // void MapData::_ComputeCollisionData()

} // namespace hoa_editor
