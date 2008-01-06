///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_tiles.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for map mode tile management.
*** ***************************************************************************/

#include "script.h"
#include "video.h"
#include "map.h"
#include "map_tiles.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_script;
using namespace hoa_video;

namespace hoa_map {

namespace private_map {

TileManager::TileManager() :
	_num_tile_rows(0),
	_num_tile_cols(0)
{}



TileManager::~TileManager() {
	// Delete _tile_images but not _animated_tile_images since everything contained in the later should be within the former
	_tile_grid.clear();
	_animated_tile_images.clear();

	for (uint32 i = 0; i < _tile_images.size(); i++)
		delete(_tile_images[i]);
	_tile_images.clear();
}



void TileManager::Load(ReadScriptDescriptor& map_file) {
	// Contains all of the tileset filenames used (string does not contain path information or file extensions)
	vector<string> tileset_filenames;
	// A container to temporarily retain all tile images loaded for each tileset. Each inner vector contains 256 StillImages
	vector<vector<StillImage> > tileset_images;
	// Used to determine whether each tile is used by the map or not. An entry of -1 indicates that particular tile is not used
	vector<int16> tile_references;
	// Temporarily holds all animated tile images. The map key is the value of the tile index, before translation
	map<uint32, AnimatedImage> tile_animations;

	_num_tile_rows = map_file.ReadInt("num_tile_rows");
	_num_tile_cols = map_file.ReadInt("num_tile_cols");

	// Do some checking to make sure tables are of the proper size
	// NOTE: we only check that the number of rows are correct, but not the number of columns
	if (map_file.GetTableSize("lower_layer") != _num_tile_rows) {
		cerr << "MAP ERROR: In MapMode::_Load(), the lower_layer table had an incorrect number of rows" << endl;
		return;
	}
	if (map_file.GetTableSize("middle_layer") != _num_tile_rows) {
		cerr << "MAP ERROR: In MapMode::_Load(), the middle_layer table had an incorrect number of rows" << endl;
		return;
	}
	if (map_file.GetTableSize("upper_layer") != _num_tile_rows) {
		cerr << "MAP ERROR: In MapMode::_Load(), the upper_layer table had an incorrect number of rows" << endl;
		return;
	}

	// ---------- (1) Load in the map tileset images and initialize all map tiles

	map_file.ReadStringVector("tileset_filenames", tileset_filenames);

	// Note: each tileset image is 512x512 pixels, yielding 256 32x32 pixel tiles each
	for (uint32 i = 0; i < tileset_filenames.size(); i++) {
		// Construct the image filename from the tileset filename and create a new vector to use in the LoadMultiImage call
		string image_filename = "img/tilesets/" + tileset_filenames[i] + ".png";
		tileset_images.push_back(vector<StillImage>(TILES_PER_TILESET));

		for (uint32 j = 0; j < TILES_PER_TILESET; j++) {
			tileset_images[i][j].SetDimensions(2.0f, 2.0f);
		}

		if (ImageDescriptor::LoadMultiImageFromElementGrid(tileset_images[i], image_filename, 16, 16) == false) {
			cerr << "MAP ERROR: MapMode::_LoadTiles() failed to load tileset image:" << image_filename << endl;
			return;
		}
	}

	// ---------- (2) Read in the map tile indeces from all three tile layers

	// First create the properly sized 2D grid of map tiles, then read the tile indeces from the map file
	for (uint32 r = 0; r < _num_tile_rows; r++) {
		_tile_grid.push_back(vector<MapTile>(_num_tile_cols));
	}

	vector<int32> table_row; // Used to temporarily store a row of integer table data

	// Read the lower_layer
	map_file.OpenTable("lower_layer");
	for (uint32 r = 0; r < _num_tile_rows; r++) {
		table_row.clear();
		map_file.ReadIntVector(r, table_row);
		for (uint32 c = 0; c < _num_tile_cols; c++) {
			_tile_grid[r][c].lower_layer = table_row[c];
		}
	}
	map_file.CloseTable();

	// Read the middle_layer
	map_file.OpenTable("middle_layer");
	for (uint32 r = 0; r < _num_tile_rows; r++) {
		table_row.clear();
		map_file.ReadIntVector(r, table_row);
		for (uint32 c = 0; c < _num_tile_cols; c++) {
			_tile_grid[r][c].middle_layer = table_row[c];
		}
	}
	map_file.CloseTable();

	// Read the upper_layer
	map_file.OpenTable("upper_layer");
	for (uint32 r = 0; r < _num_tile_rows; r++) {
		table_row.clear();
		map_file.ReadIntVector(r, table_row);
		for (uint32 c = 0; c < _num_tile_cols; c++) {
			_tile_grid[r][c].upper_layer = table_row[c];
		}
	}
	map_file.CloseTable();

	// ---------- (3) Determine which tiles in each tileset are referenced in this map

	// Set size to be equal to the total number of tiles and initialize all entries to -1 (unreferenced)
	tile_references.assign(tileset_filenames.size() * TILES_PER_TILESET, -1);

	for (uint32 r = 0; r < _num_tile_rows; r++) {
		for (uint32 c = 0; c < _num_tile_cols; c++) {
			if (_tile_grid[r][c].lower_layer >= 0)
				tile_references[_tile_grid[r][c].lower_layer] = 0;
			if (_tile_grid[r][c].middle_layer >= 0)
				tile_references[_tile_grid[r][c].middle_layer] = 0;
			if (_tile_grid[r][c].upper_layer >= 0)
				tile_references[_tile_grid[r][c].upper_layer] = 0;
		}
	}

	// Here, we have to convert the original tile indeces defined in the map file into a new form. The original index
	// indicates the tileset where the tile is used and its location in that tileset. We need to convert those indecs
	// so that they serve as an index to the MapMode::_tile_images vector, where the tile images will soon be stored.

	// Keeps track of the next translated index number to assign
	uint32 next_index = 0;

	for (uint32 i = 0; i < tile_references.size(); i++) {
		if (tile_references[i] >= 0) {
			tile_references[i] = next_index;
			next_index++;
		}
	}

	// Now, go back and re-assign all lower, middle, and upper tile layer indeces with the translated indeces
	for (uint32 r = 0; r < _num_tile_rows; r++) {
		for (uint32 c = 0; c < _num_tile_cols; c++) {
			if (_tile_grid[r][c].lower_layer >= 0)
				_tile_grid[r][c].lower_layer = tile_references[_tile_grid[r][c].lower_layer];
			if (_tile_grid[r][c].middle_layer >= 0)
				_tile_grid[r][c].middle_layer = tile_references[_tile_grid[r][c].middle_layer];
			if (_tile_grid[r][c].upper_layer >= 0)
				_tile_grid[r][c].upper_layer = tile_references[_tile_grid[r][c].upper_layer];
		}
	}

	// ---------- (4) Parse all of the tileset definition files and create any animated tile images that are used

	ReadScriptDescriptor tileset_script; // Used to access the tileset definition file
	vector<uint32> animation_info;   // Temporarily retains the animation data (tile frame indeces and display times)

	for (uint32 i = 0; i < tileset_filenames.size(); i++) {
		if (tileset_script.OpenFile("dat/tilesets/" + tileset_filenames[i] + ".lua") == false) {
			cerr << "MAP ERROR: In MapMode::_LoadTiles(), the map failed to load because it could not open a tileset definition file: "
				<< tileset_script.GetFilename() << endl;
			return;
		}

		tileset_script.OpenTable(tileset_filenames[i]);
		tileset_script.OpenTable("animated_tiles");
		for (uint32 j = 1; j <= tileset_script.GetTableSize(); j++) {
			animation_info.clear();
			tileset_script.ReadUIntVector(j, animation_info);

			// The index of the first frame in the animation. (i * TILES_PER_TILESET) factors in which tileset the frame comes from
			uint32 first_frame_index = animation_info[0] + (i * TILES_PER_TILESET);

			// Check if this animation is referenced in the map by looking at the first tile frame index. If it is not, continue on to the next animation
			if (tile_references[first_frame_index] == -1) {
				continue;
			}

			AnimatedImage new_animation;
			new_animation.SetDimensions(2.0f, 2.0f);

			// Each pair of entries in the animation info indicate the tile frame index (k) and the time (k+1)
			for (uint32 k = 0; k < animation_info.size(); k += 2) {
				new_animation.AddFrame(tileset_images[i][animation_info[k]], animation_info[k+1]);
			}

			tile_animations.insert(make_pair(first_frame_index, new_animation));
		}
		tileset_script.CloseTable();
		tileset_script.CloseTable();
		tileset_script.CloseFile();
	} // for (uint32 i = 0; i < tileset_filenames.size(); i++)

	// ---------- (5) Add all referenced tiles to the _tile_images vector, in the proper order

	for (uint32 i = 0; i < tileset_images.size(); i++) {
		for (uint32 j = 0; j < TILES_PER_TILESET; j++) {
			uint32 reference = (i * TILES_PER_TILESET) + j;

			if (tile_references[reference] >= 0) {
				// Add the tile as a StillImage
				if (tile_animations.find(reference) == tile_animations.end()) {
					_tile_images.push_back(new StillImage(tileset_images[i][j]));
				}

				// Add the tile as an AnimatedImage
				else {
					AnimatedImage* new_animated_tile = new AnimatedImage(tile_animations[reference]);
					_tile_images.push_back(new_animated_tile);
					_animated_tile_images.push_back(new_animated_tile);
				}
			}
		}
	}

	// TODO: Retain tile images available in other contexts

	// Remove all tileset images. Any tiles which were not added to _tile_images will no longer exist in memory
	tileset_images.clear();
}



void TileManager::Update() {
	for (uint32 i = 0; i < _animated_tile_images.size(); i++) {
		_animated_tile_images[i]->Update();
	}
}



void TileManager::DrawLowerLayer(const MapFrame* const frame) {
	VideoManager->SetDrawFlags(VIDEO_NO_BLEND, 0);
	VideoManager->Move(frame->tile_x_start, frame->tile_y_start);
	for (uint32 r = static_cast<uint32>(frame->starting_row);
			r < static_cast<uint32>(frame->starting_row + frame->num_draw_rows); r++) {
		for (uint32 c = static_cast<uint32>(frame->starting_col);
				c < static_cast<uint32>(frame->starting_col + frame->num_draw_cols); c++) {
			if (_tile_grid[r][c].lower_layer >= 0) { // Draw a tile image if it exists at this location
				_tile_images[_tile_grid[r][c].lower_layer]->Draw();
			}
			VideoManager->MoveRelative(2.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(frame->num_draw_cols * 2), 2.0f);
	}
}



void TileManager::DrawMiddleLayer(const MapFrame* const frame) {
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->Move(frame->tile_x_start, frame->tile_y_start);
	for (uint32 r = static_cast<uint32>(frame->starting_row);
			r < static_cast<uint32>(frame->starting_row + frame->num_draw_rows); r++) {
		for (uint32 c = static_cast<uint32>(frame->starting_col);
				c < static_cast<uint32>(frame->starting_col + frame->num_draw_cols); c++) {
			if (_tile_grid[r][c].middle_layer >= 0) { // Draw a tile image if it exists at this location
				_tile_images[_tile_grid[r][c].middle_layer]->Draw();
			}
			VideoManager->MoveRelative(2.0f, 0.0f);
		}

		VideoManager->MoveRelative(-static_cast<float>(frame->num_draw_cols * 2), 2.0f);
	}
}



void TileManager::DrawUpperLayer(const MapFrame* const frame) {
	VideoManager->Move(frame->tile_x_start, frame->tile_y_start);
	for (uint32 r = static_cast<uint32>(frame->starting_row);
			r < static_cast<uint32>(frame->starting_row + frame->num_draw_rows); r++) {
		for (uint32 c = static_cast<uint32>(frame->starting_col);
				c < static_cast<uint32>(frame->starting_col + frame->num_draw_cols); c++) {
			if (_tile_grid[r][c].upper_layer >= 0) { // Draw a tile image if it exists at this location
				_tile_images[_tile_grid[r][c].upper_layer]->Draw();
			}
			VideoManager->MoveRelative(2.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(frame->num_draw_cols * 2), 2.0f);
	}
}

} // namespace private_map

} // namespace hoa_map
