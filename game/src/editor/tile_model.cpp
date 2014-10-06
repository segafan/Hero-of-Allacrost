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
		return false;
	if (_tiles[0].empty() == true)
		return false;

	// Check that the coordinates do not exceed the bounds of the layer
	if (y >= _tiles.size())
		return false;
	if (x >= _tiles[0].size())
		return false;

	return _tiles[y][x];
}



void TileLayer::SetTile(uint32 x, uint32 y, int32 value) {
	// Make sure that there is at least one column and one row
	if (_tiles.empty() == true)
		return false;
	if (_tiles[0].empty() == true)
		return false;

	// Check that the coordinates do not exceed the bounds of the layer
	if (y >= _tiles.size())
		return false;
	if (x >= _tiles[0].size())
		return false;

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
		_tiles[row_index] = NO_TILE;
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
	uint32 length = GetLength();

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
	_tiles.resize(height, NO_TILE);
	for (uint32 y = 0; y < height; ++y) {
		_tiles[y].resize(length, NO_TILE);
	}
}

///////////////////////////////////////////////////////////////////////////////
// TileDataModel class
///////////////////////////////////////////////////////////////////////////////

TileDataModel::TileDataModel() :
	_tile_layer_count(0),
	_selected_tile_layer(NULL),
	_tile_context_count(0),
	_selected_tile_context(NULL),
	_all_tile_contexts(MAX_CONTEXTS, NULL)
{}



bool TileDataModel::InitializeData(uint32 map_length, uint32 map_height) {
	if (IsInitialized() == true) {
		return false;
	}

	// Create a single TileContext called "Base" containing a single tile layer called "Ground"
	_empty_tile_layer._ResizeLayer(map_length, map_height);

	TileContext* new_context = new TileContext(1); // Given an ID of 1 since no other contexts exist yet
	new_context->_AddTileLayer(_empty_tile_layer);

	_tile_context_count = 1;
	_tile_layer_count = 1;
	_tile_layer_visible.push_back(true);
	_tile_layer_collision.push_back(true);
	_tile_layer_names.append("Ground");

	_all_tile_contexts[0] = new_context;
	_tile_context_names.append("Base");

	_selected_tile_context = new_context;
	_selected_tile_layer = new_context->GetTileLayer(0);
}



void TileDataModel::DestroyData() {
	_tile_layer_count = 0;
	_tile_layer_visible.clear();
	_tile_layer_collision.clear();
	_tile_layer_names = QStringList();

	_tile_context_count = 0;
	_tile_context_names = QStringList();

	for (uint32 i = 0; i < _all_tile_contexts.size(); ++i) {
		if (_all_tile_contexts[i] == NULL)
			break;

		delete _all_tile_contexts[i];
		_all_tile_contexts[i] = NULL;
	}

	_selected_tile_context = NULL;
	_selected_tile_layer = NULL;
}



bool TileDataModel::LoadMap(ReadScriptDescriptor& data_file) {
	if (IsInitialized() == true) {
		return false;
	}

	// TODO: save number of contexts, number of layers, context names, layer names
	// TODO: save context inheriting information, layer collision properties
	// TODO: save collision data
	// TODO: for each context, save layers

	return true;
}



bool TileDataModel::SaveMap(WriteScriptDescriptor& data_file) {
	if (IsInitialized() == false) {
		return false;
	}

	// TODO: save number of contexts, number of layers, context names, layer names
	// TODO: save context inheriting information, layer collision properties
	// TODO: save collision data
	// TODO: for each context, save layers

	return true;
}



void TileDataModel::AddTileLayer(QString name, bool collision_on) {
	// TODO: Make sure that the name is unique among existing layer names

	_tile_layer_count += 1;
	_tile_layer_names.push_back(name);
	_tile_layer_visible.push_back(true);
	_tile_layer_collision.push_back(collision_on);

	for (uint32 i = 0; i < _tile_context_count; ++i) {
		_tile_context[i]._AddTileLayer(_empty_tile_layer);
	}
}



bool TileDataModel::DeleteTileLayer(uint32 layer_index) {
	if (layer_index > _tile_layer_count) {
		return;
	}

	// TODO
}



bool TileDataModel::RenameTileLayer(uint32 layer_index, QString new_name) {
	if (layer_index > _tile_layer_count) {
		return;
	}

	// TODO: Make sure the name being added does not already exist in the layer name list
}



void TileDataModel::MoveTileLayerUp(uint32 layer_index) {
	if (layer_index > _tile_layer_count) {
		return;
	}

	// TODO
}



void TileDataModel::MoveTileLayerDown(uint32 layer_index) {
	if (layer_index > _tile_layer_count) {
		return;
	}

	// TODO
}



TileContext* TileDataModel::AddTileContext(QString name, int32 inherting_context_id) {
	// Check all conditions where we would not be able to create the new context

	// Already have the maximum number of contexts allowed
	if (_all_tile_contexts.back() != NULL)
		return NULL;
	// A context with the requested name already exists
	if (FindTileContextByName(name) != NULL)
		return NULL;
	// Invalid range for inherting context ID
	if (inherting_context_id <= 0 || inherting_context_id > MAX_CONTEXTS)
		return NULL;
	// No context exists for the requested inherting ID
	if (_all_tile_contexts[inherting_context_id - 1] == NULL)
		return NULL;

	// Create the new context and add it to the bottom of the context list
	TileContext* new_context = new TileContext(); // TODO
}



bool TileDataModel::DeleteTileContext(TileContext* context) {
	// Check all conditions where we would not be able to create the new context

	// Invalid argument
	if (context == NULL)
		return false;
	if (FindContextByID(context->GetContextID()) != context) {
		PRINT_WARNING(EDITOR_DEBUG) << "function received a pointer to a TileContext object that is not managed by this class" << endl;
		return false;
	}
	// Context is the final base context remaining

	// Other contexts inherit from this context


	// Move the context all the way to the bottom of the context list and then delete it
	while (MoveContextDown(context) == true);
	uint32 remove_index = GetNumberOfContexts() - 1;
	delete _all_tile_contexts[remove_index];
	_all_tile_contexts[remove_index] = NULL;
}



bool TileDataModel::RenameTileContext(uint32 context_index, QString new_name) {
	// TODO
}



QStringList TileDataModel::GetInheritedTileContextNames() const {
	QStringList name_list;

	// TODO
	return name_list;
}



TileContext* TileDataModel::FindTileContextByID(int32 context_id) const {
	if (context_id <= 0 || context_id > MAX_CONTEXTS)
		return NULL;

	return _all_tile_contexts[context_id - 1];
}



TileContext* TileDataModel::FindTileContextByName(QString context_name) const {
	for (uint32 i = 0; i < _all_tile_contexts.size(); ++i) {
		if (_all_tile_contexts[i] == NULL)
			break;

		if (_all_tile_contexts[i]->GetName() == context_name)
			return _all_tile_contexts[i];
	}

	return NULL;
}



TileContext* TileDataModel::FindTileContextByIndex(uint32 context_index) const {
	if (context_index >= _all_tile_contexts.size())
		return NULL;

	return _all_tile_contexts[context_index];
}



bool TileDataModel::MoveTileContextUp(TileContext* context) {
	if (context == NULL)
		return false;

	uint32 index = _GetTileContextIndex(context);

	// Make sure that we didn't receive a pointer to an unmanaged TileContext
	if (context != _all_tile_contexts[index]) {
		PRINT_WARNING(EDITOR_DEBUG) << "function received a pointer to a TileContext object that is not managed by this class" << endl;
		return false;
	}

	// If the context is already at the top, we can't move it any further up the list
	if (index == 0)
		return false;

	_SwapTileContexts(context, _all_tile_contexts[index - 1]);
}



bool TileDataModel::MoveTileContextDown(TileContext* context) {
	if (context == NULL)
		return false;

	uint32 index = _GetTileContextIndex(context);

	// Make sure that we didn't receive a pointer to an unmanaged TileContext
	if (context != _all_tile_contexts[index]) {
		PRINT_WARNING(EDITOR_DEBUG) << "function received a pointer to a TileContext object not managed by this class" << endl;
		return false;
	}

	// If the context is already at the bottom, we can't move it any further down the list
	if (index == _all_tile_contexts.size() - 1)
		return false;
	else if (_all_tile_contexts[index + 1] == NULL)
		return false;

	_SwapTileContexts(context, _all_tile_contexts[index + 1]);
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

} // namespace hoa_editor
