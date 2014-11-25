///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2011 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    tile_layers.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for tile layer data classes
*** **************************************************************************/

#include "script.h"
#include "editor_utils.h"
#include "tile_layers.h"

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

} // namespace hoa_editor
