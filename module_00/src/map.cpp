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
#include "audio.h"
#include "video.h"
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

void MapMode::_TempCreateMap() {
	_random_encounters = true;
	_encounter_rate = 12;
	_steps_till_encounter = GaussianValue(_encounter_rate, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	_animation_counter = 0;

	_row_count = 60;
	_col_count = 80;

	// Load in all tile images from memory
	ImageDescriptor imd;
	imd.width = 1;
	imd.height = 1;

	imd.filename = "img/tiles/test_01.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_02.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_03.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_04.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_05.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_06.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_07.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_08.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_09.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_10.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_11.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_12.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_13.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_14.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_15.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_16a.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_16b.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_16d.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_16d.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_16e.png";
	_map_tiles.push_back(imd);
	imd.filename = "img/tiles/blue_40.png";
	_map_tiles.push_back(imd);

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < _map_tiles.size(); i++) {
		VideoManager->LoadImage(_map_tiles[i]);
	}
	VideoManager->EndImageLoadBatch();

	// Setup tile frame pointers for animation
	TileFrame *tf;
	for (uint32 i = 0; i < 15; i++) {
		tf = new TileFrame;
	  tf->frame = i;
		tf->next = tf;
		_tile_frames.push_back(tf);
	}

	// Setup the final animated frame tile
	TileFrame *tmp;
	tf = new TileFrame;
	tf->frame = 15; // a
	tf->next = NULL;
	_tile_frames.push_back(tf);

	tmp = new TileFrame;
	tf->next = tmp;
	tmp->frame = 16; // b
	tf = tmp;

	tmp = new TileFrame;
	tf->next = tmp;
	tmp->frame = 17; // c
	tf = tmp;

	tmp = new TileFrame;
	tf->next = tmp;
	tmp->frame = 18; // d
	tf = tmp;

	tmp = new TileFrame;
	tf->next = tmp;
	tmp->frame = 19; // e
	tmp->next = _tile_frames[15]; // Makes the linked list circular now

	tf = new TileFrame;
	tf->frame = 20;
	tf->next = tf;
	_tile_frames.push_back(tf);


	// Setup the image map
	MapTile tmp_tile;
	tmp_tile.upper_layer = -1; // No upper layer in this test
	for (uint32 r = 0; r < _row_count; r++) {
		_map_layers.push_back(vector <MapTile>());
		for (uint32 c = 0; c < _col_count; c++) {
			tmp_tile.lower_layer = (RandomNum(0, 16 - 1)); // Build the lower layer from random tiles
			tmp_tile.upper_layer = 16;
			if (tmp_tile.lower_layer == 15)
				tmp_tile.properties = Z_LVL1; // We can not walk on the water tiles
			else
				tmp_tile.properties = 0x0000;
			_map_layers[r].push_back(tmp_tile);
		}
	}
	
	// Load player sprite and rest of map objects
	MapSprite *p_sprite = new MapSprite(PLAYER_SPRITE, 2, 2, (CONTROLABLE | VISIBLE | SOUTH | Z_LVL1));
	p_sprite->LoadCharacterInfo(GLOBAL_CLAUDIUS);
	_ground_objects.push_back(p_sprite);

	MapSprite *npc_sprite = new MapSprite(NPC_SPRITE, 4, 6, (VISIBLE | EAST | Z_LVL1));
	npc_sprite->SetName("Laila");
	npc_sprite->SetFilename("img/sprites/map/laila");
	npc_sprite->SetSpeed(FAST_SPEED);
	npc_sprite->SetDelay(LONG_DELAY);
	npc_sprite->LoadFrames();
	npc_sprite->_speech->AddDialogue("Hey there hot stuff", GLOBAL_LAILA);
	npc_sprite->_speech->AddDialogue("Wanna do it again?", GLOBAL_LAILA);
	_ground_objects.push_back(npc_sprite);

	// If the _focused_object is ever NULL, the game will exit with a seg fault :(
	_focused_object = p_sprite;
}



MapMode::MapMode(uint32 new_map_id) {
	if (MAP_DEBUG) cout << "MAP: MapMode constructor invoked" << endl;

	mode_type = ENGINE_MAP_MODE;
	_map_state.push_back(EXPLORE);
	_map_id = new_map_id;

	// Load the map from the Lua data file
	//DataManager->LoadMap(this, map_id);

	// Setup the coordinate system
	//VideoManager->SetCoordSys(-SCREEN_COLS/2.0, SCREEN_COLS/2.0, -SCREEN_ROWS/2.0, SCREEN_ROWS/2.0);

	_virtual_sprite = new MapSprite(VIRTUAL_SPRITE, 20, 20, 0x0);
	_virtual_sprite->SetSpeed(VERY_FAST_SPEED);
	_virtual_sprite->SetDelay(NO_DELAY);

	// Temporary function that creates a random map
	_TempCreateMap();
}



MapMode::~MapMode() {
	if (MAP_DEBUG) cout << "MAP: MapMode destructor invoked" << endl;

	// Delete all of the tile images
	for (uint32 i = 0; i < _map_tiles.size(); i++) {
		VideoManager->DeleteImage(_map_tiles[i]);
	}

	// Free up all the frame linked lists
	TileFrame *tf_del;
	TileFrame *tf_tmp;
	for (uint32 i = 0; i < _tile_frames.size(); i++) {
		tf_del = _tile_frames[i];
		tf_tmp = tf_del->next;
		delete tf_del;

		while (tf_tmp != _tile_frames[i]) {
			tf_del = tf_tmp;
			tf_tmp = tf_del->next;
			delete tf_del;
		}

		_tile_frames[i] = NULL;
	}

	// Delete all of the objects
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		delete(_ground_objects[i]);
	}
}



// Returns true if the player is able to move to the tile.
inline bool MapMode::_TileMoveable(uint32 row, uint32 col, uint32 z_occupied) {
	// First check that the object in question isn't trying to move outside the map boundaries
	if (row < 1 || col < 0 || row >= _row_count || col >= _col_count) {
		return false;
	}

	// Check if the tile is not walkable or occupied by another object on the same z level
	if (_map_layers[row][col].properties & z_occupied) {
		return false;
	}

	return true;
}


// Moves the sprite in some direction, if possible.
void MapMode::_SpriteMove(uint32 direction, MapSprite *sprite) {
	uint32 r_check, c_check; // Variables for holding the position of a tile to check.

	// Set the sprite's facing direction and tile coordinates it wishes to move to
	switch (direction) {
		case MOVE_NORTH:
			sprite->_status = (sprite->_status & ~FACE_MASK) | NORTH;
			r_check = sprite->_row_pos - 1;
			c_check = sprite->_col_pos;
			break;
		case MOVE_SOUTH:
			sprite->_status = (sprite->_status & ~FACE_MASK) | SOUTH;
			r_check = sprite->_row_pos + 1;
			c_check = sprite->_col_pos;
			break;
		case MOVE_WEST:
			sprite->_status = (sprite->_status & ~FACE_MASK) | WEST;
			r_check = sprite->_row_pos;
			c_check = sprite->_col_pos - 1;
			break;
		case MOVE_EAST:
			sprite->_status = (sprite->_status & ~FACE_MASK) | EAST;
			r_check = sprite->_row_pos;
			c_check = sprite->_col_pos + 1;
			break;
		case MOVE_NW:
			if (sprite->_status & (NORTH_NW | NORTH | NORTH_NE | EAST_NE | EAST | EAST_SE))
				sprite->_status = (sprite->_status & ~FACE_MASK) | NORTH_NW;
			else
				sprite->_status = (sprite->_status & ~FACE_MASK) | WEST_NW;
			r_check = sprite->_row_pos - 1;
			c_check = sprite->_col_pos - 1;
			break;
		case MOVE_SW:
			if (sprite->_status & (SOUTH_SW | SOUTH | SOUTH_SE | EAST_SE | EAST | EAST_NE))
				sprite->_status = (sprite->_status & ~FACE_MASK) | SOUTH_SW;
			else
				sprite->_status = (sprite->_status & ~FACE_MASK) | WEST_SW;
			r_check = sprite->_row_pos + 1;
			c_check = sprite->_col_pos - 1;
			break;
		case MOVE_NE:
			if (sprite->_status & (NORTH_NE | NORTH | NORTH_NW | WEST_NW | WEST | WEST_SW))
				sprite->_status = (sprite->_status & ~FACE_MASK) | NORTH_NE;
			else
				sprite->_status = (sprite->_status & ~FACE_MASK) | EAST_NE;
			r_check = sprite->_row_pos - 1;
			c_check = sprite->_col_pos + 1;
			break;
		case MOVE_SE:
			if (sprite->_status & (SOUTH_SE | SOUTH | SOUTH_SW | WEST_SW | WEST | WEST_NW))
				sprite->_status = (sprite->_status & ~FACE_MASK) | SOUTH_SE;
			else
				sprite->_status = (sprite->_status & ~FACE_MASK) | EAST_SE;
			r_check = sprite->_row_pos + 1;
			c_check = sprite->_col_pos + 1;
			break;
	}

	// If the tile is moveable, set the motion flag and update the sprite coordinates
	if (_TileMoveable(r_check, c_check, (sprite->_status & Z_MASK))) {
		sprite->_status |= IN_MOTION;
		_map_layers[sprite->_row_pos][sprite->_col_pos].properties &= ~(sprite->_status & Z_MASK);
		sprite->_row_pos = r_check;
		sprite->_col_pos = c_check;
		// TODO: Check for map event here, change sprite's Z_LVL if necessary
		_map_layers[sprite->_row_pos][sprite->_col_pos].properties |= (sprite->_status & Z_MASK);
	}
	else {
		sprite->_status &= ~IN_MOTION;
		sprite->_wait_time = GaussianValue(sprite->_delay_time, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	}
}


// Processes user update and camera movement. Only called when the map is focused on the virtual sprite
void MapMode::_UpdateVirtualSprite() {
	bool user_move;
	uint32 move_direction;

	// *********** (1) Handle updates for the player sprite when in motion ************
	if (_virtual_sprite->_status & IN_MOTION) {
		_virtual_sprite->_step_count += (float)_time_elapsed / _virtual_sprite->_step_speed;

		// Check whether we've reached a new tile
		if (_virtual_sprite->_step_count >= _virtual_sprite->_step_speed) {
			_virtual_sprite->_step_count -= _virtual_sprite->_step_speed;
			_virtual_sprite->_status &= ~IN_MOTION; // IN_MOTION may get set again shortly
		}
	}

	// ********* (2) Handle updates for the player sprite when not in motion **********
	if (!(_virtual_sprite->_status & IN_MOTION)) {

		// Handle west, northwest, and southwest movement
		if (InputManager->LeftState() || InputManager->LeftPress()) {
			user_move = true;
			if (InputManager->UpState() || InputManager->RightPress()) {
				move_direction = MOVE_NW;
			}
			else if (InputManager->DownState() || InputManager->DownPress()) {
				move_direction = MOVE_SW;
			}
			else {
				move_direction = MOVE_WEST;
			}
		}

		// Handle east, northeast, and southeast movement
		else if (InputManager->RightState() || InputManager->RightPress()) {
			user_move = true;
			if (InputManager->UpState() || InputManager->UpPress())
				move_direction = MOVE_NE;
			else if (InputManager->DownState() || InputManager->DownPress())
				move_direction = MOVE_SE;
			else
				move_direction = MOVE_EAST;
		}

		// Handle north movement
		else if (InputManager->UpState() || InputManager->UpPress()) {
			user_move = true;
			move_direction = MOVE_NORTH;
		}

		// Handle south movement
		else if (InputManager->DownState() || InputManager->DownPress()) {
			user_move = true;
			move_direction = MOVE_SOUTH;
		}

		// Now check if we can actualy move the sprite to the tile the user requested to move to
		if (user_move) {
			_SpriteMove(move_direction, _virtual_sprite); // The move will always be successful here
			// A user can't move and do another command, so exit
		}
	}

	if (InputManager->CancelPress()) {
		// TEMP: Switch focus back to player sprite
		cout << "Set focused object from Virtual sprite back to Player sprite" << endl;
		if (_ground_objects[0]->_object_type == PLAYER_SPRITE)
			_focused_object = dynamic_cast<MapSprite*>(_ground_objects[0]);
		else
			_focused_object = dynamic_cast<MapSprite*>(_ground_objects[1]);
	}
}

// ****************************************************************************
// **************************** UPDATE FUNCTIONS ******************************
// ****************************************************************************

// Updates the game state when in map mode. Called from the main game loop.
void MapMode::Update(uint32 new_time_elapsed) {
	_time_elapsed = new_time_elapsed;
	_animation_counter += _time_elapsed;

	// *********** (1) Update the tile animation frames if needed ***********

	if (_animation_counter >= ANIMATION_RATE) {
		// Update all tile frames
		for (uint32 i = 0; i < _tile_frames.size(); i++) {
			_tile_frames[i] = _tile_frames[i]->next;
		}

		// Update the frames for all dynamic map objects....???
//		 list<ObjectLayer*>::iterator obj_iter;
//		 for(obj_iter = _ground_objects.begin(); obj_iter != _ground_objects.end(); obj_iter++ ) {
//			 if (obj_iter->object_type == DYNAMIC_OBJECT) {
//				 obj_iter->frame_count = (obj_iter->frame_count + 1) % obj_iter->frames.size();
//			 }
//		 }

		_animation_counter -= ANIMATION_RATE;
	}

	// ***************** (2) Update the map based on what state we are in **************
	switch (_map_state.back()) {
		case EXPLORE:
			_UpdateExploreState();
			break;
		case DIALOGUE:
			_UpdateDialogueState();
			break;
		case SCRIPT_EVENT:
			_UpdateScriptState();
			break;
	}

	// ************ (3) Sort the objects so they are in the correct draw order ********
	for (uint32 i = 1; i < _ground_objects.size(); i++) {
		ObjectLayer *tmp = _ground_objects[i];
		int32 j = static_cast<int>(i) - 1;
		while (j >= 0 && (_ground_objects[j])->_row_pos > tmp->_row_pos) {
			_ground_objects[j+1] = _ground_objects[j];
			j--;
		}
		_ground_objects[j+1] = tmp;
	}
}



// Updates the game status when MapMode is in the 'explore' state
void MapMode::_UpdateExploreState() {
	// Update all game objects (??? Or only non-playable sprites ???)
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		switch ((_ground_objects[i])->_object_type) {
			case PLAYER_SPRITE:
			{
				ObjectLayer *pSprite = _ground_objects[i];
				_UpdatePlayerExplore(dynamic_cast<MapSprite *>(_ground_objects[i]));
				break;
			}
			case NPC_SPRITE:
				_UpdateNPCExplore((MapSprite*)(_ground_objects[i]));
				break;
			default:
				break;
		}
	}

	if (_focused_object == _virtual_sprite)
		_UpdateVirtualSprite();
}


// Updates the player-controlled sprite and processes user input while in the 'explore' state
void MapMode::_UpdatePlayerExplore(MapSprite *player_sprite) {
	int32 r_check, c_check;   // Variables for saving tile coordinates
	int32 move_direction;     // The direction the sprite may be set to move in
	bool user_move = false; // Set to true if the user attempted to move the player sprite

	// *********** (!) Handle updates for the player sprite when in motion ************
	if (player_sprite->_status & IN_MOTION) {
		player_sprite->_step_count += (float)_time_elapsed / player_sprite->_step_speed;

		// Check whether we've reached a new tile
		if (player_sprite->_step_count >= player_sprite->_step_speed) {
			player_sprite->_step_count -= player_sprite->_step_speed;

			player_sprite->_status &= ~IN_MOTION; // IN_MOTION may get set again shortly
			player_sprite->_status ^= STEP_SWAP; // This flips the step_swap bit. Bit-wise XOR

			if (_random_encounters) {
				_steps_till_encounter--;
				if (player_sprite->_status & !(WEST | EAST | SOUTH | NORTH))
					_steps_till_encounter--; // Decrease count again if player moved diagonally

				if (_random_encounters && (_steps_till_encounter <= 0)) { // then a random encounter has occured
					player_sprite->_step_count = 0; // we need to be centered on a tile when we return from battle
					_steps_till_encounter = GaussianValue(_encounter_rate, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);

					if (MAP_DEBUG) cout << "MAP: Oh noes! A random encounter!!! Next encounter in: " << _steps_till_encounter
					<< " steps."<< endl;
					// Play a scary random encounter sound
					// Do a little cool script animation or something...

					// >>>Then once the script animation finishes, put this code at the end of the script<<<
					// Decide what enemies/enemy group to encounter here...?
					// BattleMode *BAM = new BattleMode(TEH_ENEMIEZ!!!);
					// ModeManager->Push(BAM);
					return;
				}
			}
		}
	}

	// If the player sprite is not currently the focused object, we have nothing more to do here
	if (player_sprite != _focused_object)
		return;

	// ********* (2) Handle updates for the player sprite when not in motion **********
	if (!(player_sprite->_status & IN_MOTION)) {

		// Handle west, northwest, and southwest movement
		if (InputManager->LeftState() || InputManager->LeftPress()) {
			user_move = true;
			if (InputManager->UpState() || InputManager->RightPress()) {
				move_direction = MOVE_NW;
			}
			else if (InputManager->DownState() || InputManager->DownPress()) {
				move_direction = MOVE_SW;
			}
			else {
				move_direction = MOVE_WEST;
			}
		}

		// Handle east, northeast, and southeast movement
		else if (InputManager->RightState() || InputManager->RightPress()) {
			user_move = true;
			if (InputManager->UpState() || InputManager->UpPress())
				move_direction = MOVE_NE;
			else if (InputManager->DownState() || InputManager->DownPress())
				move_direction = MOVE_SE;
			else
				move_direction = MOVE_EAST;
		}

		// Handle north movement
		else if (InputManager->UpState() || InputManager->UpPress()) {
			user_move = true;
			move_direction = MOVE_NORTH;
		}

		// Handle south movement
		else if (InputManager->DownState() || InputManager->DownPress()) {
			user_move = true;
			move_direction = MOVE_SOUTH;
		}

		// Now check if we can actualy move the sprite to the tile the user requested to move to
		if (user_move) {
			_SpriteMove(move_direction, player_sprite);
			// Regardless of whether the move was successful or not, refuse to process additional commands
			//  from the user.
			return;
		}
	}

	if (InputManager->MenuPress()) { // Push MenuMode onto the stack
		MenuMode *MenuM = new MenuMode();
		ModeManager->Push(MenuM);
// 		TEMP: Switch the focused object to a virtual sprite
// 		if (MAP_DEBUG) cout << "MAP: Set focused object from Player sprite to Virtual sprite" << endl;
// 		_focused_object = _virtual_sprite;
// 		return;
	}

	// Handle confirm command.
	else if (InputManager->ConfirmPress()) {

		if (player_sprite->_status & (WEST | WEST_NW | WEST_SW)) {
			r_check = player_sprite->_row_pos;
			c_check = player_sprite->_col_pos - 1;
		}
		else if (player_sprite->_status & (EAST | EAST_NE | EAST_SE)) {
			r_check = player_sprite->_row_pos;
			c_check = player_sprite->_col_pos + 1;
		}
		else if (player_sprite->_status & (NORTH | NORTH_NW | NORTH_NE)) {
			r_check = player_sprite->_row_pos - 1;
			c_check = player_sprite->_col_pos;
		}
		else { // then => (player_sprite->_status & (SOUTH | SOUTH_SW | SOUTH_SE)) == true
			r_check = player_sprite->_row_pos + 1;
			c_check = player_sprite->_col_pos;
		}

		//CheckTile(row_check, col_check, player_sprite->);
		return;
	}
}



// Updates the NPC sprites while in the 'explore' state
void MapMode::_UpdateNPCExplore(MapSprite *npc) {

	if (npc->_status & IN_MOTION) {
		npc->_step_count += (float)_time_elapsed / npc->_step_speed;

		// Check whether we've reached a new tile
		if (npc->_step_count >= npc->_step_speed) {
			npc->_step_count -= npc->_step_speed;
			npc->_status &= ~IN_MOTION;
			npc->_status ^= STEP_SWAP; // This flips the step_swap bit

			if (npc->_delay_time != 0) { // Stop the sprite for now and set a new wait time
				npc->_wait_time = GaussianValue(npc->_delay_time, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
				npc->_step_count = 0;
			}
			else { // Keep the sprite moving
				_SpriteMove(RandomNum(0,7), npc);
			}
		}
		return;
	}

	else { // Sprite is not in motion
		// Process either scripted movement or random movement
		if (npc->_wait_time > 0) {
			npc->_wait_time -= static_cast<int32>(_time_elapsed); // Decrement the wait timer
		}

		else {
			npc->_status |= IN_MOTION;
			_SpriteMove(RandomNum(0,7), npc);
		}
	}
}


// Updates the game status when MapMode is in the 'dialogue' state
void MapMode::_UpdateDialogueState() {
	cout << "TEMP: UpdateDialogueState()" << endl;
	static bool print_done; // User can only continue/exit dialogue when this is set to true

	// Handle other user input only if text printing is finished.
	if (print_done) {
		if (InputManager->ConfirmPress()) {
			//if (more_dialgoue) {
				// Send new dialogue to text renderer
			//}
			//else {
				_map_state.push_back(EXPLORE);
			//}
			print_done = false; // Reset our print_done so that dialogue can be printed next time
		}
	}

	//UpdateNPCMovement(); // NPCs can still walk around while we are in dialogue mode
}



// Updates the game status when MapMode is in the 'script_event' state
void MapMode::_UpdateScriptState() {
	cout << "TEMP: UpdateScriptState()" << endl;
}




// ****************************************************************************
// **************************** DRAW FUNCTIONS ********************************
// ****************************************************************************


// Determines things like our starting tiles
void MapMode::_GetDrawInfo(MapFrame& mf) {
	// ************* (1) Calculate the default drawing positions for the tiles ****************
	// Draw from the top left corner
	mf.c_pos = (-SCREEN_COLS / 2.0f) - 0.5f;
	mf.r_pos = (SCREEN_ROWS / 2.0f) - 0.5f;

	// Set the default col and row tile counts
	mf.c_draw = static_cast<uint32>(SCREEN_COLS) + 1;
	mf.r_draw = static_cast<uint32>(SCREEN_ROWS) + 1;

	// These are the default starting positions
	mf.c_start = _focused_object->_col_pos - (static_cast<int32>(SCREEN_COLS) / 2);
	mf.r_start = _focused_object->_row_pos - (static_cast<int32>(SCREEN_ROWS) / 2);

	// *********************** (2) Calculate the drawing information **************************
	if (_focused_object->_status & IN_MOTION) {
		if (_focused_object->_step_count <= (_focused_object->_step_speed / 2.0f)) {
			// We are not more than half-way moving west, so make adjustments
			if (_focused_object->_status & (WEST | NORTH_NW | WEST_NW | SOUTH_SW | WEST_SW)) {
				mf.c_pos += _focused_object->_step_count / _focused_object->_step_speed;
				mf.c_start++;
			}
			// We are not more than half-way moving east, so make adjustments
			else if (_focused_object->_status & (EAST | NORTH_NE | EAST_NE | SOUTH_SE | EAST_SE)) {
				mf.c_pos -= _focused_object->_step_count / _focused_object->_step_speed;
				mf.c_start--;
			}

			// We are not more than half-way moving north, so make adjustments
			if (_focused_object->_status & (NORTH | WEST_NW | NORTH_NW | EAST_NE | NORTH_NE)) {
				mf.r_pos -= _focused_object->_step_count / _focused_object->_step_speed;
				mf.r_start++;
			}
			// We are not more than half-way moving south, so make adjustments
			else if (_focused_object->_status & (SOUTH | WEST_SW | SOUTH_SW | EAST_SE | SOUTH_SE)) {
				mf.r_pos += _focused_object->_step_count / _focused_object->_step_speed;
				mf.r_start--;
			}
		}

		// NOTE: Draw code should never see a step_count >= 32. Update() takes care of that
		else { // (_focused_object->_step_count > (TILE_STEPS / 2))
			// We are at least half-way moving west, so make adjustments
			if (_focused_object->_status & (WEST | NORTH_NW | WEST_NW | SOUTH_SW | WEST_SW)) {
				mf.c_pos -= (_focused_object->_step_speed - _focused_object->_step_count) / _focused_object->_step_speed;
			}
			// We are at least half-way moving east, so make adjustments
			else if (_focused_object->_status & (EAST | NORTH_NE | EAST_NE | SOUTH_SE | EAST_SE)) {
				mf.c_pos += (_focused_object->_step_speed - _focused_object->_step_count) / _focused_object->_step_speed;
			}

			// We are at least half-way moving north, so make adjustments
			if (_focused_object->_status & (NORTH | WEST_NW | NORTH_NW | EAST_NE | NORTH_NE)) {
				mf.r_pos += (_focused_object->_step_speed - _focused_object->_step_count) / _focused_object->_step_speed;
			}
			// We are at least half-way moving south, so make adjustments
			else if (_focused_object->_status & (SOUTH | WEST_SW | SOUTH_SW | EAST_SE | SOUTH_SE)) {
				mf.r_pos -= (_focused_object->_step_speed - _focused_object->_step_count) / _focused_object->_step_speed;
			}
		}
	}

	// *********************** (3) Check for special conditions **************************

	// Usually the map "moves around the player", but when we encounter the edges of the map we
	// need the player to "move around the map".

	// Exceeds the far-left side of the map
	if (mf.c_start < 0) { 
		mf.c_start = 0;
		mf.c_pos = -(SCREEN_COLS / 2.0f);
	}
	// Exceeds the far-right side of the map
	else if (mf.c_start > _col_count - static_cast<int32>(SCREEN_COLS) - 1) { 
		mf.c_start = _col_count - static_cast<int32>(SCREEN_COLS);
		mf.c_pos = -(SCREEN_COLS / 2.0f);
	}

	// If our column position is exactly on the left edge of the screen, we draw one less column of tiles
	if (mf.c_pos == -(SCREEN_COLS / 2.0f)) {
		mf.c_draw--;
	}

	// Exceeds the far-north side of the map
	if (mf.r_start < 0) { 
		mf.r_start = 0;
		mf.r_pos = (SCREEN_ROWS / 2.0f) - 1.0f;
	}
	// Exceeds the far-south side of the map
	else if (mf.r_start > _row_count - static_cast<int32>(SCREEN_ROWS) - 1) { 
		mf.r_start = _row_count - static_cast<int32>(SCREEN_ROWS);
		mf.r_pos = (SCREEN_ROWS / 2.0f) - 1.0f;
	}

	// If the row position is exactly on the top of the screen, draw one less row of tiles
	if (mf.r_pos == (SCREEN_ROWS / 2.0f) - 1.0f) {
		mf.r_draw--;
	}
} // MapMode::GetDrawInfo(MapFrame& mf)


// Public draw function called by the main game loop
void MapMode::Draw() {
	MapFrame mf; // Contains all the information we need to know to draw the map

	// Make sure the coordinate system is set properly
	VideoManager->SetCoordSys(-SCREEN_COLS/2.0f, SCREEN_COLS/2.0f, -SCREEN_ROWS/2.0f, SCREEN_ROWS/2.0f);

	_GetDrawInfo(mf); // Get all the information we need for drawing this map frame


	// ************** (1) Draw the Lower Layer *************
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_NO_BLEND, 0);
	VideoManager->Move(mf.c_pos, mf.r_pos);
	for (uint32 r = static_cast<uint32>(mf.r_start); r < static_cast<uint32>(mf.r_start) + mf.r_draw; r++) {
		for (uint32 c = static_cast<uint32>(mf.c_start); c < static_cast<uint32>(mf.c_start) + mf.c_draw; c++) {
			if (_map_layers[r][c].lower_layer >= 0) // Then a lower layer tile exists and we should draw it
				VideoManager->DrawImage(_map_tiles[_tile_frames[_map_layers[r][c].lower_layer]->frame]);
			VideoManager->MoveRel(1.0f, 0.0f);
		}
		VideoManager->MoveRel(-static_cast<float>(mf.c_draw), -1.0f);

	}

	// ************** (2) Draw the Object Layer *************
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		(_ground_objects[i])->Draw(mf);
	}

	// ************** (3) Draw the Upper Layer *************
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->Move(mf.c_pos, mf.r_pos);
	for (uint32 r = mf.r_start; r < mf.r_start + mf.r_draw; r++) {
		for (uint32 c = mf.c_start; c < mf.c_start + mf.c_draw; c++) {
			if (_map_layers[r][c].upper_layer >= 0) // Then an upper layer tile exists and we should draw it
				VideoManager->DrawImage(_map_tiles[_tile_frames[_map_layers[r][c].upper_layer]->frame]);
			VideoManager->MoveRel(1.0f, 0.0f);
		}
		VideoManager->MoveRel(-static_cast<float>(mf.c_draw), -1.0f);
	}

	// ************** (4) Draw the Dialoge box, if needed *************
	// if (_map_state.back() = DIALOGUE) {
	// }
	return;
}


} // namespace hoa_map
