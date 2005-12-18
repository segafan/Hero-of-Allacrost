///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    map.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 12th, 2005
 * \brief   Source file for map mode interface.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include "map.h"
#include "map_objects.h"
#include "map_dialogue.h"
#include "audio.h"
#include "video.h"
#include "gui.h"
#include "global.h"
#include "data.h"
#include "battle.h"
#include "menu.h"

using namespace std;
using namespace hoa_map::private_map;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_engine;
using namespace hoa_global;
using namespace hoa_data;
using namespace hoa_battle;
using namespace hoa_menu;

namespace hoa_map {

bool MAP_DEBUG = false;

// ****************************************************************************
// ************************** MapMode Class Functions *************************
// ****************************************************************************
// ***************************** GENERAL FUNCTIONS ****************************
// ****************************************************************************

MapMode::MapMode() {
	if (MAP_DEBUG) cout << "MAP: MapMode constructor invoked" << endl;

	mode_type = ENGINE_MAP_MODE;
	_map_state = EXPLORE;
	
	_map_camera = new MapSprite();
	_map_camera->SetObjectType(MAP_CAMERA);
	_map_camera->SetRowPosition(20);
	_map_camera->SetColPosition(20);
	_map_camera->SetAltitude(0);
	_map_camera->SetStatus(0);
	_map_camera->SetStepSpeed(NORMAL_SPEED);

	_ground_objects.push_back(_map_camera);
	// Loads all the map data
	LoadMap();
}



MapMode::~MapMode() {
	if (MAP_DEBUG) cout << "MAP: MapMode destructor invoked" << endl;

	// Delete all of the tile images
	for (uint32 i = 0; i < _tile_images.size(); i++) {
		VideoManager->DeleteImage(*(_tile_images[i]));
	}

	// Delete all of the objects
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		delete(_ground_objects[i]);
	}
	
	// Free up the dialogue menu
	_dialogue_window.Destroy();
}


// Resets appropriate class members.
void MapMode::Reset() {
	_dialogue_text = NULL;
	
	// Reset active video engine properties
	VideoManager->SetCoordSys(0.0f, SCREEN_COLS, SCREEN_ROWS, 0.0f);
	//VideoManager->SetCoordSys(-SCREEN_COLS/2.0f, SCREEN_COLS/2.0f, -SCREEN_ROWS/2.0f, SCREEN_ROWS/2.0f);
	if(!VideoManager->SetFont("default")) 
    cerr << "MAP: ERROR > Couldn't set map font!" << endl;
	
	// Let all map objects know that this is the current map
	MapObject::CurrentMap = this;
}


// Loads the map from a Lua file.
void MapMode::LoadMap() {
	// *********** (1) Setup GUI items in 1024x768 coordinate system ************
	VideoManager->PushState();
	VideoManager->SetCoordSys(0, 1024, 768, 0);
	_dialogue_window.Create(1024.0f, 128.0f);
	_dialogue_window.SetPosition(0.0f, 768.0f);
	
	_dialogue_textbox.SetDisplaySpeed(30);
	_dialogue_textbox.SetPosition(0.0f + 32.0f, 768.0f - 32.0f);
	_dialogue_textbox.SetDimensions(1024.0f - 64.0f, 128.0f - 64.0f);
	_dialogue_textbox.SetFont("default");
	_dialogue_textbox.SetDisplayMode(VIDEO_TEXT_REVEAL);
	_dialogue_textbox.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	VideoManager->PopState();
	
	// ************* (2) Open data file and begin processing data ***************
	_map_data.OpenFile("dat/maps/test_01.lua");
	_random_encounters = _map_data.ReadBool("random_encounters");
	if (_random_encounters) {
		_encounter_rate = _map_data.ReadInt("encounter_rate");
		_steps_till_encounter = GaussianValue(_encounter_rate, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	}
	else {
		// Set some decent default values, just in case a script turns random encounters on later
		_encounter_rate = 10;
		_steps_till_encounter = 10;
	}

	_row_count = _map_data.ReadInt("row_count");
	_col_count = _map_data.ReadInt("col_count");
	
	// ********************** (3) Load in tile filenames ************************
	vector<string> tile_filenames;
	_map_data.FillStringVector("tile_filenames", tile_filenames);
	for (uint32 i = 0; i < tile_filenames.size(); i++) {
		// Prepend the pathname and append the file extension for all the file names
		tile_filenames[i] = "img/tiles/" + tile_filenames[i] + ".png";
	}
	
	// ******************** (4) Setup tile image mappings ***********************
	vector<int32> tile_mappings;
	_map_data.OpenTable("tile_mappings");
	int32 mapping_count = tile_filenames.size(); // TMP
	//int32 mapping_count = _map_data.GetTableSize("tile_mappings");
	//cout << "mapping_count == " << mapping_count << endl;
	for (uint32 i = 0; i < mapping_count; i++) {
		_map_data.FillIntVector(i, tile_mappings);
		
		if (tile_mappings.size() == 1) { // Then add a new static image
			StillImage *static_tile = new StillImage();
			static_tile->SetDimensions(1.0f, 1.0f);
			static_tile->SetFilename(tile_filenames[tile_mappings[0]]);
			_tile_images.push_back(static_tile);
		}
		else { // Create a new dynamic image
			//_tile_images.push_back(&animate_tile);
			for (uint32 j = 0; j < tile_mappings.size(); j += 2) {
//				(dynamic_cast<AnimatedImage>(_tile_images.back())).AddFrame
//				 (tile_filenames[tile_mappings[j]], tile_mappings[j+1]);
				// NOTE: Find a cleaner way to do this later...
			}
		}
		tile_mappings.clear();
	}
	_map_data.CloseTable();
	
	// **************** (5) Load all tile images from memory ********************
	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < _tile_images.size(); i++) {
		if (!_tile_images[i]->Load()) {
			cerr << "MAP ERROR: Failed to load tile image " << endl;
		}
	}
	VideoManager->EndImageLoadBatch();
	
	// ******************** (6) Create the 2D tile map *************************
	MapTile tmp_tile;
	for (uint32 r = 0; r < _row_count; r++) {
		_tile_layers.push_back(vector <MapTile>(_col_count));
	}
	
	vector<int32> properties;
	_map_data.OpenTable("lower_layer");
	for (uint32 r = 0; r < _row_count; r++) {
		_map_data.FillIntVector(r, properties);
		
		for (uint32 c = 0; c < _col_count; c++) {
			_tile_layers[r][c].lower_layer = static_cast<int16>(properties[c]);
		}

		properties.clear();
	}
	_map_data.CloseTable();
	
	_map_data.OpenTable("middle_layer");
	for (uint32 r = 0; r < _row_count; r++) {
		_map_data.FillIntVector(r, properties);
		
		for (uint32 c = 0; c < _col_count; c++) {
			_tile_layers[r][c].middle_layer = static_cast<int16>(properties[c]);
		}
		
		properties.clear();
	}
	_map_data.CloseTable();
	
	_map_data.OpenTable("upper_layer");
	for (uint32 r = 0; r < _row_count; r++) {
		_map_data.FillIntVector(r, properties);
		
		for (uint32 c = 0; c < _col_count; c++) {
			_tile_layers[r][c].upper_layer = static_cast<int16>(properties[c]);
		}
		
		properties.clear();
	}
	_map_data.CloseTable();
	
	_map_data.OpenTable("tile_walkable");
	for (uint32 r = 0; r < _row_count; r++) {
		_map_data.FillIntVector(r, properties);
		
		for (uint32 c = 0; c < _col_count; c++) {
			_tile_layers[r][c].walkable = static_cast<uint8>(properties[c]);
		}
		
		properties.clear();
	}
	_map_data.CloseTable();
	
// 	_map_data.OpenTable("tile_properties");
// 	for (uint32 r = 0; r < _row_count; r++) {
// 		_map_data.FillIntVector(r, properties);
// 		
// 		for (uint32 c = 0; c < _col_count; c++) {
// 			_tile_layers[r][c].properties = static_cast<uint8>(properties[c]);
// 		}
// 		
// 		properties.clear();
// 	}
// 	_map_data.CloseTable();
	
	// The occupied member of tiles are not set until we place map objects
	
	_map_data.OpenTable("tile_events");
	for (uint32 r = 0; r < _row_count; r++) {
		_map_data.FillIntVector(r, properties);
		
		for (uint32 c = 0; c < _col_count; c++) {
			_tile_layers[r][c].arrive_event = static_cast<int16>(properties[c]);
			_tile_layers[r][c].depart_event = static_cast<int16>(properties[c]);
			_tile_layers[r][c].confirm_event = static_cast<int16>(properties[c]);
		}
		
		properties.clear();
	}
	_map_data.CloseTable();
	_map_data.CloseFile();
	
	if (_map_data.GetError() != DATA_NO_ERRORS) {
		cout << "MAP ERROR: some error occured during reading of map file" << endl;
	}
	
	// Load player sprite and rest of map objects
	MapSprite *player = new MapSprite();
	player->SetObjectType(PLAYER_SPRITE);
	player->SetRowPosition(18);
	player->SetColPosition(18);
	player->SetStepSpeed(NORMAL_SPEED);
	player->SetAltitude(ALTITUDE_1);
	player->SetStatus(UPDATEABLE | VISIBLE | ALWAYS_IN_CONTEXT);
	player->SetFilename("img/sprites/map/claudius");
	player->SetDirection(SOUTH);
	player->LoadFrames();
	_ground_objects.push_back(player);
	
	// If the _focused_object is ever NULL, the game will exit with a seg fault :(
	_focused_object = player;
} // _LoadMap()



// Returns true if an object can be moved to the tile.
bool MapMode::_TileMoveable(const private_map::TileCheck& tcheck) {
	// Check that the row and col indeces are valid and not outside the map
	// (By the way, the top row is never walkable on a map so we don't check that)
	if (tcheck.row < 1 || tcheck.col < 0 || tcheck.row >= _row_count || tcheck.col >= _col_count) {
		return false;
	}
	
	// If the focused object is the virtual sprite, there's nothing left to check
	if (_focused_object == _map_camera) {
		return true;
	}

	// Check that the tile is walkable at this altitude
	if (!(_tile_layers[tcheck.row][tcheck.col].walkable & tcheck.altitude)) {
		return false;
	}
	
	// Don't allow diagonal movement if any nearby component x, y tiles are unwalkable
	switch (tcheck.direction) {
		case NORTH:
		case SOUTH:
		case WEST:
		case EAST:
			break;
		case NW_NORTH:
		case NW_WEST:
			if ( !((_tile_layers[tcheck.row][tcheck.col + 1].walkable & tcheck.altitude) ||
			      (_tile_layers[tcheck.row + 1][tcheck.col].walkable & tcheck.altitude)) ) {
				return false;
			}
			break;
		case SW_SOUTH:
		case SW_WEST:
			if ( !((_tile_layers[tcheck.row][tcheck.col + 1].walkable & tcheck.altitude) ||
			      (_tile_layers[tcheck.row - 1][tcheck.col].walkable & tcheck.altitude)) ) {
				return false;
			}
			break;
		case NE_NORTH:
		case NE_EAST:
			if ( !((_tile_layers[tcheck.row][tcheck.col - 1].walkable & tcheck.altitude) ||
			      (_tile_layers[tcheck.row + 1][tcheck.col].walkable & tcheck.altitude)) ) {
				return false;
			}
			break;
		case SE_SOUTH:
		case SE_EAST:
			if ( !((_tile_layers[tcheck.row][tcheck.col - 1].walkable & tcheck.altitude) ||
			      (_tile_layers[tcheck.row - 1][tcheck.col].walkable & tcheck.altitude)) ) {
				return false;
			}
			break;
		default:
			if (MAP_DEBUG) cerr << "MAP: WARNING: Called MapMode::_TileMoveable() with an invalid direction" << endl;
			return false;
	}
	
	// Check that no other objects occupy this tile at this altitude
	if (_tile_layers[tcheck.row][tcheck.col].occupied & tcheck.altitude) {
		return false;
	}

	return true;
}


// Checks if there is an interaction with a map object or event at the specified tile.
// This function is used when the user presses the "confirm" key in the map exploration state.
// uint32 MapMode::_CheckInteraction(const private_map::TileCheck& tcheck) {
// 	// Check that the row and col indeces are valid and not outside the map
// 	if (tcheck.row < 1 || tcheck.col < 0 || tcheck.row >= _row_count || tcheck.col >= _col_count) {
// 		cerr << "MAP: WARNING: Called _CheckInteraction() with out of bounds row and column" << endl;
// 		return NO_INTERACTION;
// 	}
// 	
// 	// Check if the tile in question has a confirm event registered to it.
// 	if (_tile_layers[tcheck.row][tcheck.col].properties & CONFIRM_EVENT) {
// 		return TILE_INTERACTION;
// 	}
// 	
// 	// Check if an object occupies the tile
// 	if (_tile_layers[tcheck.row][tcheck.col].occupied && tcheck.altitude) {
// 		// TODO: Look up the object that occupies this tile at this altitude
// 		return OBJECT_INTERACTION;
// 	}
// 	
// 	return NO_INTERACTION;
// }


// Searches the list of map objects to find the object occupying a tile.
MapObject* MapMode::_FindTileOccupant(const private_map::TileCheck& tcheck) {
	// TODO: Use a more sophisticated search algorithm here
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		if (_ground_objects[i]->_row_position == tcheck.row && 
				_ground_objects[i]->_col_position == tcheck.col &&
				_ground_objects[i]->_altitude == tcheck.altitude) {
			return _ground_objects[i];
		}  
	}
	return NULL;
} // MapMode::_FindTileOccupant()



// Finds a path for a sprite to take, using the A* algorithm.
void MapMode::_FindPath(const MapSprite* sprite, 
                        const private_map::TileNode& destination,
                        vector<TileNode> &path) {
	// The source tile that the sprite is currently occupying
	TileNode source;
	// The tiles that we are considering for the next move
	vector<TileNode> open_list;
	// The tiles which have already been visited once.
	vector<TileNode> closed_list;
	
	// Set the source properties
	source.row = sprite->_row_position;
	source.col = sprite->_col_position;
	source.altitude = sprite->_altitude;
	
	closed_list.push_back(source);
	
	// Find a path until the current node is equal to the destination
	while (source.row != destination.row && source.col != destination.col) {
		
	}
	
}


// Processes user update and camera movement. Only called when the map is focused on the virtual sprite
void MapMode::_UpdateVirtualSprite() {

} // MapMode::_UpdateVirtualSprite()

// ****************************************************************************
// **************************** UPDATE FUNCTIONS ******************************
// ****************************************************************************

// Updates the game state when in map mode. Called from the main game loop.
void MapMode::Update(uint32 new_time_elapsed) {
	_time_elapsed = new_time_elapsed;

	// ***************** (1) Update all objects on the map **************
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		if ((_ground_objects[i])->_status & UPDATEABLE) {
			_ground_objects[i]->Update();
		}
	}
	
	// ***************** (2) Process user input **************
	switch (_map_state) {
		case EXPLORE:
			_UpdateExplore();
			break;
		case DIALOGUE:
			_UpdateDialogue();
			break;
		default:
			_UpdateExplore();
			break;
	}
	
	// ************ (3) Sort the objects so they are in the correct draw order ********
	// Note: this sorting algorithm will be optimized at a later time
	for (uint32 i = 1; i < _ground_objects.size(); i++) {
		MapObject *tmp = _ground_objects[i];
		int32 j = static_cast<int32>(i) - 1;
		while (j >= 0 && (_ground_objects[j])->_row_position > tmp->_row_position) {
			_ground_objects[j+1] = _ground_objects[j];
			j--;
		}
		_ground_objects[j+1] = tmp;
	}
}



// Updates the game status when MapMode is in the 'explore' state
void MapMode::_UpdateExplore() {
	bool user_move = false;
	uint32 move_direction;
	
	// (1) Check if the focused object is moving. If so, only process swap events from the user
	if (_focused_object->_status & IN_MOTION) {
// 		if (InputManager->SwapPress()) {
// 			// Change the character representing the player's party
// 		
// 		}
		return;
	}
	
	// (2) Give highest priority to confirm events from the user
	if (InputManager->ConfirmPress()) {
		// Check for a sprite present on the adjacent tile or a confirm event registerd to the tile
		TileCheck tcheck;
		if (_focused_object->_direction & (WEST | NW_WEST | SW_WEST)) {
			tcheck.row = _focused_object->_row_position;
			tcheck.col = _focused_object->_col_position - 1;
		}
		else if (_focused_object->_direction & (EAST | NE_EAST | SE_EAST)) {
			tcheck.row = _focused_object->_row_position;
			tcheck.col = _focused_object->_col_position + 1;
		}
		else if (_focused_object->_direction & (NORTH | NW_NORTH | NE_NORTH)) {
			tcheck.row = _focused_object->_row_position - 1;
			tcheck.col = _focused_object->_col_position;
		}
		else { // then => (_focused_object->_direction & (SOUTH | SW_SOUTH | SE_SOUTH)) == true
			tcheck.row = _focused_object->_row_position + 1;
			tcheck.col = _focused_object->_col_position;
		}
		tcheck.altitude = _focused_object->_altitude;
		tcheck.direction = _focused_object->_direction;

		// Check the tile the player is facing for events or other objects that can be interacted with.
		// uint8 interaction = _CheckInteraction(tcheck);
		return;
	}
	
	// (3) Check for menu press events
	if (InputManager->MenuPress()) {
		MenuMode *MM = new MenuMode();
		ModeManager->Push(MM);
		return;
	}
	
	// (4) Handle movement input from user
	// Handle west, northwest, and southwest movement
	if (InputManager->LeftState()) {
		user_move = true;
		if (InputManager->UpState()) {
			move_direction = NW_NORTH;
		}
		else if (InputManager->DownState()) {
			move_direction = SW_SOUTH;
		}
		else {
			move_direction = WEST;
		}
	}
	// Handle east, northeast, and southeast movement
	else if (InputManager->RightState()) {
		user_move = true;
		if (InputManager->UpState())
			move_direction = NE_NORTH;
		else if (InputManager->DownState())
			move_direction = SE_SOUTH;
		else
			move_direction = EAST;
	}
	// Handle north movement
	else if (InputManager->UpState()) {
		user_move = true;
		move_direction = NORTH;
	}
	// Handle south movement
	else if (InputManager->DownState()) {
		user_move = true;
		move_direction = SOUTH;
	}
		
	if (user_move) {
		_focused_object->Move(move_direction);
		// The move may be successful, or it may not be.
	}
}



// Updates the game status when MapMode is in the 'dialogue' state
void MapMode::_UpdateDialogue() {
// 	_dialogue_textbox.Update(_time_elapsed);
// 	
// 	// User is done reading the dialogue if they press confirm
// 	if (InputManager->ConfirmPress()) {
// 		_dialogue_line++;
// 		// Check if we've passed the last line of dialogue, and if so exit the dialogue
// 		if (_dialogue_line >= _dialogue_text->size()) {
// 				// Remove the dialogue state from the map state stack
// 			_map_state = EXPLORE;
// 		
// 			// Restore the status of map sprites
// 			for (uint32 i = 0; i < _dialogue_speakers.size(); i++) {
// 				if (_ground_objects[i] != _focused_object) {
// 					_dialogue_speakers[i]->RestoreState();
// 					_dialogue_speakers[i]->FinishedDialogue();
// 				}
// 			}
// 			
// 			// Clean out the dialogue speakers
// 			_dialogue_speakers.clear();
// 			_dialogue_window.Hide();
// 		}
// 		else {
// 			_dialogue_textbox.ShowText(MakeWideString((*_dialogue_text)[_dialogue_line]));
// 		}
// 	}
}




// ****************************************************************************
// **************************** DRAW FUNCTIONS ********************************
// ****************************************************************************


// Determines things like our starting tiles
void MapMode::_GetDrawInfo() {
	// ************* (1) Set the default drawing positions for the tiles ****************
	// Begin drawing from the top left corner
	_draw_info.c_pos = -0.5f;
	_draw_info.r_pos = 0.5f;

	// By default draw 32 + 1 columns and 24 + 1 rows
	_draw_info.c_draw = static_cast<uint8>(SCREEN_COLS) + 1;
	_draw_info.r_draw = static_cast<uint8>(SCREEN_ROWS) + 1;

	// The default starting tile row and column is relative to the focused sprite's current position.
	_draw_info.c_start = static_cast<int16>(_focused_object->_col_position 
	                     - (static_cast<int32>(SCREEN_COLS) / 2));
	_draw_info.r_start = static_cast<int16>(_focused_object->_row_position 
	                     - (static_cast<int32>(SCREEN_ROWS) / 2));

	// *** (2) Modify drawing positions if focused sprite is currently moving ***
	
	if (_focused_object->_status & IN_MOTION) {
		float offset = _focused_object->_step_count / _focused_object->_step_speed;
		if (_focused_object->_direction & (WEST | NW_NORTH | NW_WEST | SW_SOUTH | SW_WEST)) {
			if (offset < 0.5f) {
				_draw_info.c_pos += offset;
				_draw_info.c_start++;
			}
			else {
				_draw_info.c_pos -= 1.0f - offset;
			}
		}
		else if (_focused_object->_direction & (EAST | NE_NORTH | NE_EAST | SE_SOUTH | SE_EAST)) {
			if (offset < 0.5f) {
				_draw_info.c_pos -= offset;
				_draw_info.c_start--;
			}
			else {
				_draw_info.c_pos += 1.0f - offset;
			}
		}
		
		if (_focused_object->_direction & (NORTH | NW_WEST | NW_NORTH | NE_EAST | NE_NORTH)) {
			if (offset < 0.5f) {
				_draw_info.r_pos += offset;
				_draw_info.r_start++;
			}
			else {
				_draw_info.r_pos -= 1.0f - offset;
			}
		}
		else if (_focused_object->_direction & (SOUTH | SW_WEST | SW_SOUTH | SE_EAST | SE_SOUTH)) {
			if (offset < 0.5f) {
				_draw_info.r_pos -= offset;
				_draw_info.r_start--;
			}
			else {
				_draw_info.r_pos += 1.0f - offset;
			}
		}
	}

	// *********************** (3) Check for special conditions **************************

	// Usually the map moves around the player, but when we encounter the edges of the map we
	// want the player to move around the map.

	// Exceeds the far-left side of the map
	if (_draw_info.c_start < 0) { 
		_draw_info.c_start = 0;
		_draw_info.c_pos = 0.0f;
	}
	// Exceeds the far-right side of the map
	else if (_draw_info.c_start >= _col_count - static_cast<int32>(SCREEN_COLS)) { 
		_draw_info.c_start = static_cast<int16>(_col_count - static_cast<int32>(SCREEN_COLS));
		_draw_info.c_pos = 0.0f;
	}
	// If our column position is exactly on the left edge of the screen, we draw one less column of tiles
	if (_draw_info.c_pos == 0.0f) {
		_draw_info.c_draw--;
	}

	// Exceeds the far-north side of the map
	if (_draw_info.r_start < 0) { 
		_draw_info.r_start = 0;
		_draw_info.r_pos = 1.0f;
	}
	// Exceeds the far-south side of the map
	else if (_draw_info.r_start >= _row_count - static_cast<int32>(SCREEN_ROWS)) {
		_draw_info.r_start = static_cast<int16>(_row_count - static_cast<int32>(SCREEN_ROWS));
		_draw_info.r_pos = 1.0f;
	}
	// If the row position is exactly on the top of the screen, draw one less row of tiles
	if (_draw_info.r_pos == 1.0f) {
		_draw_info.r_draw--;
	}
} // MapMode::_GetDrawInfo()


// Public draw function called by the main game loop
void MapMode::Draw() {
	// Calculate all the information we need for drawing this map frame
	_GetDrawInfo(); 
	
	// ************** (1) Draw the lower tile layer *************
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_NO_BLEND, 0);
	VideoManager->Move(_draw_info.c_pos, _draw_info.r_pos);
// 	printf("moved cursor to: %1.2f, %1.2f\n", _draw_info.c_pos, _draw_info.r_pos);
// 	printf("start tile: %d, %d\n", _draw_info.c_start, _draw_info.r_start);
// 	printf("sprite pos: %d, %d\n", _focused_object->_col_position, _focused_object->_row_position);
	for (uint32 r = static_cast<uint32>(_draw_info.r_start); 
	     r < static_cast<uint32>(_draw_info.r_start) + _draw_info.r_draw; r++) {
		for (uint32 c = static_cast<uint32>(_draw_info.c_start); 
		     c < static_cast<uint32>(_draw_info.c_start) + _draw_info.c_draw; c++) {
			if (_tile_layers[r][c].lower_layer >= 0) { // Then a lower layer tile exists and we should draw it
				_tile_images[_tile_layers[r][c].lower_layer]->Draw();
			}
			VideoManager->MoveRelative(1.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(_draw_info.c_draw), 1.0f);
	}
	
	// ************** (2) Draw the middle tile layer *************
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->Move(_draw_info.c_pos, _draw_info.r_pos);
	for (uint32 r = static_cast<uint32>(_draw_info.r_start); 
	     r < static_cast<uint32>(_draw_info.r_start) + _draw_info.r_draw; r++) {
		for (uint32 c = static_cast<uint32>(_draw_info.c_start); 
		     c < static_cast<uint32>(_draw_info.c_start) + _draw_info.c_draw; c++) {
			if (_tile_layers[r][c].middle_layer >= 0) { // Then a middle layer tile exists and we should draw it
				_tile_images[_tile_layers[r][c].middle_layer]->Draw();
			}
			VideoManager->MoveRelative(1.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(_draw_info.c_draw), 1.0f);
	}

	// ************** (3) Draw the ground object layer (first pass) *************
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		// Only draw objects that are visible
		if (_ground_objects[i]->_status & VISIBLE) {
			(_ground_objects[i])->Draw();
		}
	}
	
	// ************** (4) Draw the middle object layer *************
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	for (uint32 i = 0; i < _middle_objects.size(); i++) {
		// Only draw objects that are visible
		if (_middle_objects[i]->_status & VISIBLE) {
			(_middle_objects[i])->Draw();
		}
	}
	
	// ************** (5) Draw the ground object layer (second pass) *************
// 	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
// 	for (uint32 i = 0; i < _ground_objects.size(); i++) {
// 		// The following function call only draws the object if it is visible on the screen.
// 		(_ground_objects[i])->Draw();
// 	}

	// ************** (6) Draw the upper tile layer *************
	VideoManager->Move(_draw_info.c_pos, _draw_info.r_pos);
	for (int32 r = _draw_info.r_start; r < _draw_info.r_start + _draw_info.r_draw; r++) {
		for (int32 c = _draw_info.c_start; c < _draw_info.c_start + _draw_info.c_draw; c++) {
			if (_tile_layers[r][c].upper_layer >= 0) // Then an upper layer tile exists and we should draw it;
				_tile_images[_tile_layers[r][c].upper_layer]->Draw();
			VideoManager->MoveRelative(1.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(_draw_info.c_draw), 1.0f);
	}
	
	// ************* (7) Draw the sky object layer **********
	VideoManager->SetDrawFlags(VIDEO_NO_BLEND, 0);
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	for (uint32 i = 0; i < _sky_objects.size(); i++) {
		// Only draw objects that are visible
		if (_sky_objects[i]->_status & VISIBLE) {
			(_sky_objects[i])->Draw();
		}
	}
	
	// ************** (8) Draw the dialogue menu and text *************
	if (_map_state == DIALOGUE) {
		VideoManager->PushState();
		VideoManager->SetCoordSys(0, 1024, 768, 0);
		_dialogue_window.Draw();
		_dialogue_textbox.Draw();
		VideoManager->PopState();
	}
		
	return;
} // MapMode::_Draw()

} // namespace hoa_map
