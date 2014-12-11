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
	_map_length = data_file.ReadUInt("map_length");
	_map_height = data_file.ReadUInt("map_height");
 	uint32 number_tilesets = data_file.ReadUInt("number_tilesets");
	_tile_layer_count = data_file.ReadUInt("number_tile_layers");
	_tile_context_count = data_file.ReadUInt("number_map_contexts");

	// ---------- (3): Construct the tileset, tile layers, and tile context objects
	vector<string> tileset_names;
	data_file.ReadStringVector("tileset_names", tileset_names);
	for (uint32 i = 0; i < number_tilesets; ++i) {
		Tileset* tileset = new Tileset();
		QString tileset_qname = QString(tileset_names[i].c_str());
		if (tileset->Load(tileset_qname) == false) {
			_error_message = QString("Failed to load tileset file ") + tileset_qname + QString(" during loading of map file ") + _map_filename;
			delete tileset;
			return false;
		}
		AddTileset(tileset);
	}

	vector<string> tile_layer_names;
	vector<bool> tile_layer_collision_enabled;
	data_file.ReadStringVector("tile_layer_names", tile_layer_names);
	data_file.ReadBoolVector("tile_layer_collision_enabled", tile_layer_collision_enabled);

	// TODO: construct layers

	vector<string> tile_context_names;
	vector<int32> tile_context_inheritance;
	data_file.ReadStringVector("map_context_names", tile_context_names);
	data_file.ReadIntVector("map_context_inheritance", tile_context_inheritance);

	// TODO: construct tile contexts

	// ---------- (4): Read collision grid data

	// TODO: Currently collision data is only computed and written when the map file is saved. In the future,
	// the collision data should always be available and optionally visible on the map view.

	// ---------- (5): Read the tile value for each layer and each context


	if (data_file.IsErrorDetected()) {
		_error_message = QString("Data read failure occurred for global map variables. Error messages:\n") + QString(data_file.GetErrorMessages().c_str());
		return false;
	}

	// Create selection layer
	// TODO: resize select layer here

	data_file.CloseTable();
	data_file.CloseFile();

	return true;
} // bool MapData::LoadData(QString filename)



bool MapData::SaveData(QString filename) {
	if (IsInitialized() == false) {
		return false;
	}

	// ---------- (1): Open the file and write the tablespace header and map header comment
	WriteScriptDescriptor data_file;
	if (data_file.OpenFile(filename.toStdString()) == false) {
		_error_message = "Could not open file for writing: " + filename;
		return false;
	}

	data_file.WriteNamespace(DetermineLuaFileTablespaceName(filename.toStdString()));
	data_file.InsertNewLine();

	// TODO: add this information at a later time when the user has the ability to add this
	// data to the map properties in the editor.
	data_file.WriteComment("------------------------------------------------------------");
	data_file.WriteComment("Map Name: ");
	data_file.WriteComment("Map Designer(s): ");
	data_file.WriteComment("Description:");
	data_file.WriteComment("------------------------------------------------------------");
	data_file.InsertNewLine();

	// ---------- (2): Write the basic map data properties
	data_file.WriteUInt("map_length", _map_length);
	data_file.WriteUInt("map_height", _map_height);
 	data_file.WriteUInt("number_tilesets", _tilesets.size());
	data_file.WriteUInt("number_tile_layers", _tile_layer_count);
	data_file.WriteUInt("number_map_contexts", _tile_context_count);
	data_file.InsertNewLine();

	// ---------- (3): Write properties of tilesets, tile layers, and map contexts
	data_file.BeginTable("tileset_names");
	for (uint32 i = 0; i < _tilesets.size(); ++i) {
		data_file.WriteString((i+1), _tileset_names[i].toStdString());
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
	vector<vector<uint32> > collision_grid;
	_ComputeCollisionData(collision_grid);
	for (uint32 i = 0; i < collision_grid.size(); ++i) {
		data_file.WriteUIntVector(static_cast<int32>(i+1), collision_grid[i]);
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	// ---------- (5): For each tile, write the tile value for each layer and each context
	data_file.BeginTable("map_tiles");
	for (uint32 y = 0; y < _map_height; ++y) {
		for (uint32 x = 0; x < _map_length; ++x) {
			string tile_id = "map_tiles[" + NumberToString(y) + "][" + NumberToString(x) + "] = ";
			vector<int32> tiles;
			for (uint32 c = 0; c < _tile_context_count; ++c) {
				for (uint32 l = 0; l < _tile_layer_count; ++l) {
					// TODO: push context/layer data into tiles vecotr
				}
			}
			// TODO: write tiles vector
		}
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	if (data_file.IsErrorDetected()) {
		_error_message = "One or more errors occurred when writing map file. Error messages:\n" + QString(data_file.GetErrorMessages().c_str());
		return false;
	}
	data_file.CloseFile();

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
	if (layer_index > _tile_layer_count) {
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



bool MapData::MoveTileLayerUp(uint32 layer_index) {
	if (layer_index >= _tile_layer_count) {
		_error_message = "ERROR: no tile layer exists at this index";
		return false;
	}
	if (layer_index == 0) {
		_error_message = "WARN: tile layer could not be moved further up";
		return false;
	}

	uint32 swap_index = layer_index - 1;

	// Move the tile layer up across all contexts
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		_all_tile_contexts[i]->_SwapTileLayers(layer_index, swap_index);
	}

	// Move the layer properties up
	TileLayerProperties swap = _tile_layer_properties[swap_index];
	_tile_layer_properties[swap_index] = _tile_layer_properties[layer_index];
	_tile_layer_properties[layer_index] = _tile_layer_properties[swap_index];

	return true;
}



bool MapData::MoveTileLayerDown(uint32 layer_index) {
	if (layer_index >= _tile_layer_count) {
		_error_message = "ERROR: no tile layer exists at this index";
		return false;
	}
	if (layer_index == (_tile_layer_count - 1)) {
		_error_message = "WARN: tile layer could not be moved further down";
		return false;
	}

	uint32 swap_index = layer_index + 1;

	// Move the tile layer up across all contexts
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		_all_tile_contexts[i]->_SwapTileLayers(layer_index, swap_index);
	}

	// Move the layer properties up
	TileLayerProperties swap = _tile_layer_properties[swap_index];
	_tile_layer_properties[swap_index] = _tile_layer_properties[layer_index];
	_tile_layer_properties[layer_index] = _tile_layer_properties[swap_index];

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



bool MapData::DeleteTileContext(TileContext* context) {
	// Check all conditions where we would not be able to create the new context
	if (context == NULL) {
		_error_message = "ERROR: received NULL context argument";
		return false;
	}
	if (FindTileContextByID(context->GetContextID()) != context) {
		_error_message = "function received a pointer to a TileContext object that is not managed";
		return false;
	}
	// TODO case: Context is the final base context remaining
	// TODO case: Other contexts inherit from this context


	// Move the context all the way to the bottom of the context list and then delete it
	int32 remove_index = static_cast<int32>(_tile_context_count - 1);
	// This loop works because MoveTileContextDown updates the context ID to its new position
	while (context->GetContextID() < remove_index) {
		MoveTileContextDown(context);
	}

	delete _all_tile_contexts[remove_index];
	_all_tile_contexts[remove_index] = NULL;
	return true;
}



bool MapData::RenameTileContext(uint32 context_index, QString new_name) {
	if (context_index >= _tile_context_count) {
		_error_message = "ERROR: context_index exceeds size of context list";
		return false;
	}

	if (_all_tile_contexts[context_index]->GetContextName() == new_name) {
		return true;
	}

	QStringList context_names = GetTileContextNames();
	if (context_names.indexOf(new_name) != -1) {
		_error_message = "ERROR: a context with this name already exists";
		return false;
	}

	_all_tile_contexts[context_index]->SetContextName(new_name);
	return true;
}



bool MapData::MoveTileContextUp(TileContext* context) {
	if (context == NULL)
		return false;

	uint32 index = _GetTileContextIndex(context);

	// Make sure that we didn't receive a pointer to an unmanaged TileContext
	if (context != _all_tile_contexts[index]) {
		IF_PRINT_WARNING(EDITOR_DEBUG) << "function received a pointer to a TileContext object that is not managed by this class" << endl;
		return false;
	}

	// If the context is already at the top, we can't move it any further up the list
	if (index == 0)
		return false;

	_SwapTileContexts(context, _all_tile_contexts[index - 1]);
	return true;
}



bool MapData::MoveTileContextDown(TileContext* context) {
	if (context == NULL)
		return false;

	uint32 index = _GetTileContextIndex(context);

	// Make sure that we didn't receive a pointer to an unmanaged TileContext
	if (context != _all_tile_contexts[index]) {
		IF_PRINT_WARNING(EDITOR_DEBUG) << "function received a pointer to a TileContext object not managed by this class" << endl;
		return false;
	}

	// If the context is already at the bottom, we can't move it any further down the list
	if (index == _all_tile_contexts.size() - 1)
		return false;
	else if (_all_tile_contexts[index + 1] == NULL)
		return false;

	_SwapTileContexts(context, _all_tile_contexts[index + 1]);
	return true;
}



TileContext* MapData::FindTileContextByID(int32 context_id) const {
	if (context_id <= 0 || static_cast<uint32>(context_id) > MAX_CONTEXTS) {
		return NULL;
	}

	if (context_id >= static_cast<int32>(_tile_context_count)) {
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



void MapData::_SwapTileContexts(TileContext* first, TileContext* second) {
	int32 first_id = first->GetContextID();
	int32 second_id = second->GetContextID();
	uint32 first_index = static_cast<uint32>(first_id - 1);
	uint32 second_index = static_cast<uint32>(second_id - 1);

	// Perform the swap and update each context's ID to match it's new position in the container
	_all_tile_contexts[first_index] = second;
	second->_SetContextID(first_id);
	_all_tile_contexts[second_index] = first;
	first->_SetContextID(second_id);

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
}



void MapData::_ComputeCollisionData(std::vector<std::vector<uint32> >& data) {
	// TODO: this function needs to be rewritten from scratch to account for multiple
	// tile layers, each with their own collision data toggability

// 	// Used to save the northern walkability info of tiles in all layers of
// 	// all contexts; initialize to walkable.
// 	vector<int32> map_row_north(_length * 2, 0);
// 	// Used to save the southern walkability info of tiles in all layers of
// 	// all contexts; initialize to walkable.
// 	vector<int32> map_row_south(_length * 2, 0);
// 	for (uint32 row = 0; row < _height; row++) {
// 		// Iterate through all contexts of all layers, column by column,
// 		// row by row.
// 		for (int context = 0; context < static_cast<int>(_lower_layer.size()); context++) {
// 			for (uint32 col = row * _length; col < row * _length + _length; col++) {
// 				// Used to know if any tile at all on all combined layers exists.
// 				bool missing_tile = true;
//
// 				// Get walkability for lower layer tile.
// 				tileset_index = _lower_layer[context][col] / 256;
// 				if (tileset_index == 0)
// 					tile_index = _lower_layer[context][col];
// 				else  // Don't divide by 0
// 					tile_index = _lower_layer[context][col] %
// 						(tileset_index * 256);
// 				if (tile_index == -1) {
// 					ll_vect.push_back(0);
// 					ll_vect.push_back(0);
// 					ll_vect.push_back(0);
// 					ll_vect.push_back(0);
// 				}
// 				else {
// 					missing_tile = false;
// 					ll_vect = tilesets[tileset_index]->walkability[tile_index];
// 				}
//
// 				// Get walkability for middle layer tile.
// 				tileset_index = _middle_layer[context][col] / 256;
// 				if (tileset_index == 0)
// 					tile_index = _middle_layer[context][col];
// 				else  // Don't divide by 0
// 					tile_index = _middle_layer[context][col] %
// 						(tileset_index * 256);
// 				if (tile_index == -1) {
// 					ml_vect.push_back(0);
// 					ml_vect.push_back(0);
// 					ml_vect.push_back(0);
// 					ml_vect.push_back(0);
// 				}
// 				else {
// 					missing_tile = false;
// 					ml_vect = tilesets[tileset_index]->walkability[tile_index];
// 				}
//
// 				// Get walkability for upper layer tile.
// 				tileset_index = _upper_layer[context][col] / 256;
// 				if (tileset_index == 0)
// 					tile_index = _upper_layer[context][col];
// 				else  // Don't divide by 0
// 					tile_index = _upper_layer[context][col] %
// 						(tileset_index * 256);
// 				if (tile_index == -1) {
// 					ul_vect.push_back(0);
// 					ul_vect.push_back(0);
// 					ul_vect.push_back(0);
// 					ul_vect.push_back(0);
// 				}
// 				else {
// 					missing_tile = false;
// 					ul_vect = tilesets[tileset_index]->walkability[tile_index];
// 				}
//
// 				if (missing_tile == true) {
// 					// NW corner
// 					map_row_north[col % _length * 2]     |= 1 << context;
// 					// NE corner
// 					map_row_north[col % _length * 2 + 1] |= 1 << context;
// 					// SW corner
// 					map_row_south[col % _length * 2]     |= 1 << context;
// 					// SE corner
// 					map_row_south[col % _length * 2 + 1] |= 1 << context;
// 				}
// 				else {
// 					// NW corner
// 					map_row_north[col % _length * 2]     |=
// 						((ll_vect[0] | ml_vect[0] | ul_vect[0]) << context);
// 					// NE corner
// 					map_row_north[col % _length * 2 + 1] |=
// 						((ll_vect[1] | ml_vect[1] | ul_vect[1]) << context);
// 					// SW corner
// 					map_row_south[col % _length * 2]     |=
// 						((ll_vect[2] | ml_vect[2] | ul_vect[2]) << context);
// 					// SE corner
// 					map_row_south[col % _length * 2 + 1] |=
// 						((ll_vect[3] | ml_vect[3] | ul_vect[3]) << context);
// 				} // a real tile exists at current location
//
// 				ll_vect.clear();
// 				ml_vect.clear();
// 				ul_vect.clear();
// 			} // iterate through the columns of the layers
// 		} // iterate through each context
//
// 		write_data.WriteIntVector(row*2,   map_row_north);
// 		write_data.WriteIntVector(row*2+1, map_row_south);
// 		map_row_north.assign(_length*2, 0);
// 		map_row_south.assign(_length*2, 0);
// 	} // iterate through the rows of the layers
// 	write_data.EndTable();
// 	write_data.InsertNewLine();
} // void MapData::_ComputeCollisionData(std::vector<std::vector<uint32> >& data)

} // namespace hoa_editor
