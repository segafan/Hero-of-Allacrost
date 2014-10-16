///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2011 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    tile_model.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for the tile data model classes
*** **************************************************************************/

#include "script.h"
#include "editor_utils.h"
#include "tile_model.h"

using namespace hoa_script;
using namespace std;

namespace hoa_editor {

///////////////////////////////////////////////////////////////////////////////
// TileLayer class
///////////////////////////////////////////////////////////////////////////////

int32 TileLayer::GetTile(uint32 x, uint32 y) const {
	// Make sure that there is at least one column and one row
	if (_tiles.empty() == true)
		return NO_TILE;
	if (_tiles[0].empty() == true)
		return NO_TILE;

	// Check that the coordinates do not exceed the bounds of the layer
	if (y >= _tiles.size())
		return NO_TILE;
	if (x >= _tiles[0].size())
		return NO_TILE;

	return _tiles[y][x];
}



void TileLayer::SetTile(uint32 x, uint32 y, int32 value) {
	// Make sure that there is at least one column and one row
	if (_tiles.empty() == true)
		return;
	if (_tiles[0].empty() == true)
		return;

	// Check that the coordinates do not exceed the bounds of the layer
	if (y >= _tiles.size())
		return;
	if (x >= _tiles[0].size())
		return;

	_tiles[y][x] = value;
}



void TileLayer::FillLayer(int32 tile_id) {
	for (uint32 y = 0; y < _tiles.size(); ++y) {
		for (uint32 x = 0; x < _tiles[y].size(); ++x) {
			_tiles[y][x] = tile_id;
		}
	}
}



void TileLayer::_AddLayerRow(uint32 row_index, int32 value) {
	uint32 old_height = GetHeight();
	uint32 length = GetLength();

	if (old_height == 0) {
		return;
	}
	if (row_index > old_height) {
		return;
	}

	// And an empty tile row to the bottom
	_tiles.push_back(vector<int32>(length, NO_TILE));
	// Shift the appropriate rows one position down to make room for the row to add
	for (uint32 i = _tiles.size() - 1; i > row_index; --i) {
		_tiles[i] = _tiles[i-1];
	}
	// Now set the tiles at the new row to NO_TILE
	for (uint32 i = 0; i < length; ++i) {
		_tiles[row_index][i] = NO_TILE;
	}
}



void TileLayer::_AddLayerCol(uint32 col_index, int32 value) {
	uint32 height = GetHeight();
	uint32 old_length = GetLength();

	if (height == 0) {
		return;
	}
	if (col_index > old_length) {
		return;
	}

	for (uint32 i = 0; i < height; ++i) {
		// Add an empty tile column to the back of each row.
		_tiles[i].push_back(NO_TILE);
		// Shift the columns one position right to make room for the column to add.
		for (uint32 j = _tiles[i].size() - 1; j > col_index; --j) {
			_tiles[i][j] = _tiles[i][j-1];
		}
		// Set the value in the new column to NO_TILE
		_tiles[i][col_index] = NO_TILE;
	}
}



void TileLayer::_DeleteLayerRow(uint32 row_index) {
	uint32 old_height = GetHeight();

	if (old_height == 0) {
		return;
	}
	if (row_index >= old_height) {
		return;
	}

	// Swap all rows below the index one position up to replace the deleted row
	for (uint32 i = row_index; i < _tiles.size() - 1; ++i) {
		_tiles[i] = _tiles[i+1];
	}
	// Now remove the last row
	_tiles.pop_back();
}



void TileLayer::_DeleteLayerCol(uint32 col_index) {
	uint32 height = GetHeight();
	uint32 old_length = GetLength();

	if (height == 0) {
		return;
	}
	if (col_index > old_length) {
		return;
	}

	// Swap all columns to the right of col_index one position left to replace the deleted column
	for (uint32 i = 0; i < height; ++i) {
		for (uint32 j = col_index; j < old_length - 1; ++j) {
			_tiles[i][j] = _tiles[i][j+1];
		}
		// Now remove the last column
		_tiles[i].pop_back();
	}
}



void TileLayer::_ResizeLayer(uint32 length, uint height) {
	_tiles.resize(height, vector<int32>(length));
	for (uint32 y = 0; y < height; ++y) {
		_tiles[y].resize(length, NO_TILE);
	}
}

///////////////////////////////////////////////////////////////////////////////
// TileContext class
///////////////////////////////////////////////////////////////////////////////

void TileContext::_AddTileLayer(TileLayer& layer) {
	if (layer.GetHeight() == 0 || layer.GetLength() == 0) {
		IF_PRINT_WARNING(EDITOR_DEBUG) << "could not add layer because one or both dimensions are zero" << endl;
		return;
	}

	// If no tile layers exist, we don't need to do any layer size checking
	if (_tile_layers.empty() == true) {
		_tile_layers.push_back(layer);
		return;
	}

	// Ensure that the height and length of the layer match an existing layer
	if (layer.GetHeight() != _tile_layers[0].GetHeight()) {
		IF_PRINT_WARNING(EDITOR_DEBUG) << "could not add layer because its height does not match the existing layers" << endl;
		return;
	}
	if (layer.GetLength() != _tile_layers[0].GetLength()) {
		IF_PRINT_WARNING(EDITOR_DEBUG) << "could not add layer because its length does not match the existing layers" << endl;
		return;
	}

	_tile_layers.push_back(layer);
}



void TileContext::_RemoveTileLayer(uint32 layer_index) {
	if (layer_index >= _tile_layers.size()) {
		IF_PRINT_WARNING(EDITOR_DEBUG) << "could not remove layer because the layer_index argument (" << layer_index
			<< ") exceeds the number of layers (" << layer_index << ")" << endl;
		return;
	}

	for (uint32 i = layer_index; i < _tile_layers.size() - 1; ++i) {
		_tile_layers[i] = _tile_layers[i+1];
	}
	_tile_layers.pop_back();
}



void TileContext::_SwapTileLayers(uint32 first_index, uint32 second_index) {
	if (first_index >= _tile_layers.size() || second_index >= _tile_layers.size()) {
		IF_PRINT_WARNING(EDITOR_DEBUG) << "could not remove layer because one or both index arguments (" << first_index
			<< ", " << second_index << ") exceeds the number of layers (" << _tile_layers.size() << ")" << endl;
		return;
	}

	// TODO: see if this can be replaced with a call to std::swap
	TileLayer swap = _tile_layers[first_index];
	_tile_layers[first_index] = _tile_layers[second_index];
	_tile_layers[second_index] = _tile_layers[first_index];
}

///////////////////////////////////////////////////////////////////////////////
// TileDataModel class
///////////////////////////////////////////////////////////////////////////////

TileDataModel::TileDataModel() :
	_tile_layer_count(0),
	_tile_context_count(0),
	_selected_tile_layer(NULL),
	_selected_tile_context(NULL),
	_all_tile_contexts(MAX_CONTEXTS, NULL)
{}



bool TileDataModel::CreateData(uint32 map_length, uint32 map_height) {
	if (IsInitialized() == true) {
		return false;
	}

	_empty_tile_layer._ResizeLayer(map_length, map_height);

	// Create a single tile layer called "Ground"
	_tile_layer_properties.push_back(TileLayerProperties(QString("Ground"), true, true));
	_tile_layer_count = 1;

	// Create a single TileContext called "Base"
	TileContext* new_context = new TileContext(1, "Base"); // Give an ID of 1 since no other contexts exist yet
	new_context->_AddTileLayer(_empty_tile_layer);
	_all_tile_contexts[0] = new_context;
	_tile_context_count = 1;

	_selected_tile_context = new_context;
	_selected_tile_layer = new_context->GetTileLayer(0);
	return true;
}



void TileDataModel::DestroyData() {
	_tile_layer_count = 0;
	_tile_layer_properties.clear();

	for (uint32 i = 0; i < _all_tile_contexts.size(); ++i) {
		if (_all_tile_contexts[i] == NULL)
			break;

		delete _all_tile_contexts[i];
		_all_tile_contexts[i] = NULL;
	}

	_selected_tile_context = NULL;
	_selected_tile_layer = NULL;
	_tile_context_count = 0;
}



bool TileDataModel::LoadData(ReadScriptDescriptor& data_file) {
	if (IsInitialized() == true) {
		return false;
	}

	// TODO: save number of contexts, number of layers, context names, layer names
	// TODO: save context inheriting information, layer collision properties
	// TODO: save collision data
	// TODO: for each context, save layers

	return true;
} // bool TileDataModel::LoadData(ReadScriptDescriptor& data_file)



bool TileDataModel::SaveData(WriteScriptDescriptor& data_file) {
	if (IsInitialized() == false) {
		return false;
	}

	if (data_file.IsFileOpen() == false) {
		return false;
	}

	// ---------- (1): Write basic map data properties
	data_file.WriteInt("map_length", _empty_tile_layer.GetLength());
	data_file.WriteInt("map_height", _empty_tile_layer.GetHeight());
// 	data_file.WriteInt("number_tilesets", _tileset_count);
	data_file.WriteInt("number_tile_layers", _tile_layer_count);
	data_file.WriteInt("number_map_contexts", _tile_context_count);
	data_file.InsertNewLine();

	// ---------- (2): Write properties of tilesets, tile layers, and contexts
// 	data_file.BeginTable("tileset_names");
// 	for (uint32 i = 0; i < _tile_layer_names.size(); ++i) {
// 		data_file.WriteString((i+1), _tileset_names[i].toAscii());
// 	}
// 	data_file.EndTable();
// 	data_file.InsertNewLine();

	data_file.BeginTable("tile_layer_names");
	QStringList layer_names = GetTileLayerNames();
	for (int32 i = 0; i < layer_names.size(); ++i) {
		data_file.WriteString((i+1), layer_names[i].toStdString());
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

	data_file.BeginTable("collision_grid");
	vector<vector<uint32> > collision_grid;
	_ComputeCollisionData(collision_grid);
	for (uint32 i = 0; i < collision_grid.size(); ++i) {
		data_file.WriteUIntVector(static_cast<int32>(i+1), collision_grid[i]);
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	data_file.BeginTable("map_context_inheritance");
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		data_file.WriteInt((i+1), _all_tile_contexts[i]->GetInheritedContextID());
	}
	data_file.EndTable();
	data_file.InsertNewLine();

	// ---------- (3): Compute and write layer collision data
	vector<vector<uint32> > collision_data;
	_ComputeCollisionData(collision_data);
	data_file.BeginTable("collision_grid");
	for (uint32 i = 0; i < collision_data.size(); ++i) {

	}
	data_file.EndTable();
	data_file.InsertNewLine();


	// ---------- (4): Write all layers for each context

	// TODO: save number of contexts, number of layers, context names, layer names
	// TODO: save context inheriting information, layer collision properties
	// TODO: save collision data
	// TODO: for each context, save layers

	return true;
} // bool TileDataModel::SaveData(WriteScriptDescriptor& data_file)



QStringList TileDataModel::GetTileLayerNames() const {
	QStringList layer_names;
	for (uint32 i = 0; i < _tile_layer_count; ++i) {
		layer_names.append(_tile_layer_properties[i].GetName());
	}

	return layer_names;
}



void TileDataModel::ToggleTileLayerVisibility(uint32 layer_index) {
	if (layer_index > _tile_layer_count)
		return;

	bool visible = _tile_layer_properties[layer_index].IsVisible();
	_tile_layer_properties[layer_index].SetVisible(!visible);
}



void TileDataModel::ToggleTileLayerCollision(uint32 layer_index) {
	if (layer_index > _tile_layer_count)
		return;

	bool collisions = _tile_layer_properties[layer_index].IsCollisionEnabled();
	_tile_layer_properties[layer_index].SetCollisionEnabled(!collisions);
}



bool TileDataModel::AddTileLayer(QString name, bool collision_on) {
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



bool TileDataModel::DeleteTileLayer(uint32 layer_index) {
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



bool TileDataModel::RenameTileLayer(uint32 layer_index, QString new_name) {
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



bool TileDataModel::MoveTileLayerUp(uint32 layer_index) {
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



bool TileDataModel::MoveTileLayerDown(uint32 layer_index) {
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



QStringList TileDataModel::GetTileContextNames() const {
	QStringList context_names;
	for (uint32 i = 0; i < _tile_context_count; ++i) {
		if (_all_tile_contexts[i] == NULL)
			break;

		context_names.append(_all_tile_contexts[i]->GetContextName());
	}

	return context_names;
}



QStringList TileDataModel::GetInheritedTileContextNames() const {
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


TileContext* TileDataModel::AddTileContext(QString name, int32 inheriting_context_id) {
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
		if (inheriting_context_id <= 0 || inheriting_context_id > MAX_CONTEXTS) {
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



bool TileDataModel::DeleteTileContext(TileContext* context) {
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



bool TileDataModel::RenameTileContext(uint32 context_index, QString new_name) {
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



bool TileDataModel::MoveTileContextUp(TileContext* context) {
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



bool TileDataModel::MoveTileContextDown(TileContext* context) {
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



TileContext* TileDataModel::FindTileContextByID(int32 context_id) const {
	if (context_id <= 0 || context_id > MAX_CONTEXTS) {
		return NULL;
	}

	if (context_id >= static_cast<int32>(_tile_context_count)) {
		return NULL;
	}

	return _all_tile_contexts[context_id - 1];
}



TileContext* TileDataModel::FindTileContextByName(QString context_name) const {
	for (uint32 i = 0; i < _all_tile_contexts.size(); ++i) {
		if (_all_tile_contexts[i] == NULL)
			break;

		if (_all_tile_contexts[i]->GetContextName() == context_name)
			return _all_tile_contexts[i];
	}

	return NULL;
}



TileContext* TileDataModel::FindTileContextByIndex(uint32 context_index) const {
	if (context_index >= _all_tile_contexts.size())
		return NULL;

	return _all_tile_contexts[context_index];
}







void TileDataModel::_SwapTileContexts(TileContext* first, TileContext* second) {
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



void TileDataModel::_ComputeCollisionData(std::vector<std::vector<uint32> >& data) {
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
} // void TileDataModel::_ComputeCollisionData(std::vector<std::vector<uint32> >& data)

} // namespace hoa_editor
