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
#include "gui.h"

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

void MapMode::_TEMP_CreateMap() {
	_random_encounters = true;
	_encounter_rate = 12;
	_steps_till_encounter = GaussianValue(_encounter_rate, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	_animation_counter = 0;

	_row_count = 60;
	_col_count = 80;

	// Load in all tile images from memory
	ImageDescriptor imd;
	imd.SetDimensions(1.0f, 1.0f);

	imd.SetFilename("img/tiles/test_01.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_02.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_03.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_04.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_05.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_06.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_07.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_08.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_09.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_10.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_11.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_12.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_13.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_14.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_15.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_16a.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_16b.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_16c.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_16d.png");
	_map_tiles.push_back(imd);
	imd.SetFilename("img/tiles/test_16e.png");
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

	// Setup the image map
	MapTile tmp_tile;
	tmp_tile.upper_layer = -1; // No upper layer in this test
	for (uint32 r = 0; r < _row_count; r++) {
		_map_layers.push_back(vector <MapTile>());
		for (uint32 c = 0; c < _col_count; c++) {
			tmp_tile.lower_layer = (RandomNum(0, 16 - 1)); // Build the lower layer from random tiles
			if (tmp_tile.lower_layer == 15) { // Set water tile properties
				tmp_tile.not_walkable = ALTITUDE_1;
				tmp_tile.properties = CONFIRM_EVENT;
			}
			else {
				tmp_tile.not_walkable = 0x00;
				tmp_tile.properties = 0x00;
			}
			tmp_tile.occupied = 0x00;
			_map_layers[r].push_back(tmp_tile);
		}
	}
	
	// Load player sprite and rest of map objects
	MapSprite *player = new MapSprite(CHARACTER_SPRITE, 2, 2, ALTITUDE_1, (UPDATEABLE | VISIBLE | IN_CONTEXT));
	player->LoadCharacterInfo(GLOBAL_CLAUDIUS);
	player->_direction = SOUTH;
	_ground_objects.push_back(player);

	MapSprite *npc_sprite = new MapSprite(NPC_SPRITE, 4, 6, ALTITUDE_1, (UPDATEABLE | VISIBLE | IN_CONTEXT));
	npc_sprite->SetName("Laila");
	npc_sprite->_direction = EAST;
	npc_sprite->SetFilename("img/sprites/map/laila");
	npc_sprite->SetSpeed(VERY_SLOW_SPEED);
	npc_sprite->SetDelay(LONG_DELAY);
	npc_sprite->LoadFrames();
	npc_sprite->_speech->AddDialogue("I'm a hottie!");
	npc_sprite->_speech->AddDialogue("But, I'm also your sister...");
	npc_sprite->_speech->AddDialogue("Is this really okay?");
	_ground_objects.push_back(npc_sprite);

	// If the _focused_object is ever NULL, the game will exit with a seg fault :(
	_focused_object = player;
}



MapMode::MapMode(uint32 new_map_id) {
	if (MAP_DEBUG) cout << "MAP: MapMode constructor invoked" << endl;

	mode_type = ENGINE_MAP_MODE;
	_map_state.push_back(EXPLORE);
	_map_id = new_map_id;

	// Load the map from the Lua data file
	//DataManager->LoadMap(this, map_id);

	_virtual_sprite = new MapSprite(VIRTUAL_SPRITE, 20, 20, ALTITUDE_0, 0x0);
	_virtual_sprite->SetSpeed(VERY_FAST_SPEED);
	_virtual_sprite->SetDelay(NO_DELAY);

	// Temporary function that creates a random map
	_TEMP_CreateMap();
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


// Resets appropriate class members.
void MapMode::Reset() {
	_dialogue_text = "";
	
	// Set video engine properties
	VideoManager->SetCoordSys(-SCREEN_COLS/2.0f, SCREEN_COLS/2.0f, -SCREEN_ROWS/2.0f, SCREEN_ROWS/2.0f);
	if(!VideoManager->SetFont("default")) 
    cerr << "MAP: ERROR > Couldn't set map font!" << endl;
}



// Returns true if the player is able to move to the tile.
inline bool MapMode::_TileMoveable(int32 row, int32 col, uint8 altitude_level) {
	// First check that the object in question isn't trying to move outside the map boundaries
	if (row < 1 || col < 0 || row >= _row_count || col >= _col_count) {
		return false;
	}

	// Check if the tile is not walkable at this altitude
	if (_map_layers[row][col].not_walkable & altitude_level) {
		return false;
	}
	
	// Check that no other objects occupy this tile
	if (_map_layers[row][col].occupied & altitude_level) {
		return false;
	}

	return true;
}

// Checks if there is an interaction with the 'confirm' command at a specified tile.
void MapMode::_CheckInteraction(int32 row, int32 col, uint8 altitude_level) {
	// First check that the object in question isn't trying to move outside the map boundaries
	if (row < 1 || col < 0 || row >= _row_count || col >= _col_count) {
		return;
	}
	
	// Check if an object occupies the tile
	if (_map_layers[row][col].occupied && altitude_level) {
		// Look up the object that occupies this tile at this altitude
		cout << "OCCUPIED TILE" << endl;
		// ****** TEMPORARY PROOF OF CONCEPT CODE ********
		// On the next update, we want to go to the dialogue state
		_map_state.push_back(DIALOGUE);
		// Save the status of both sprites so they can be set back after the dialogue is finished
		for (uint32 i = 0; i < _ground_objects.size(); i++) {
			switch ((_ground_objects[i])->_object_type) {
				case CHARACTER_SPRITE:
				case NPC_SPRITE:
				{
					MapSprite *tmp_sprite = dynamic_cast<MapSprite *>(_ground_objects[i]);
					// Save the sprite's current status
					tmp_sprite->_speech->_saved_status = tmp_sprite->_status;
					tmp_sprite->_speech->_saved_direction = tmp_sprite->_direction;
					// Turn off its update function so it doesn't move around while the dialogue is occuring
					tmp_sprite->_status &= (~IN_MOTION | ~UPDATEABLE);
					
					// We need to get the sprite on the recieving end of the player to face the player's sprite
					if (_ground_objects[i] != _focused_object) {
						if (_focused_object->_direction & (NORTH | NORTH_NW | NORTH_NE)) {
							tmp_sprite->_direction = SOUTH;
						}
						else if (_focused_object->_direction & (SOUTH | SOUTH_SW | SOUTH_SE)) {
							tmp_sprite->_direction = NORTH;
						}
						else if (_focused_object->_direction & (EAST | EAST_NE | EAST_SE)) {
							tmp_sprite->_direction = WEST;
						}
						else { // (_focused_object->_direction & (WEST | WEST_NW | WEST_SW))
							tmp_sprite->_direction = EAST;
						}
					}
					break;
				}
				default:
					break;
			}
		}
		return;
	}
	
	// Finally, check if the tile has a confirm event associated with it.
	if (_map_layers[row][col].properties && CONFIRM_EVENT) {
		// Look up the event corresponding to this tile in the event list.
		cout << "RAWR! I'm a tile with a confirm event!" << endl;
		return;
	}

}

// Attempts to move a sprite in the ground object layer in the given direction.
void MapMode::_GroundSpriteMove(uint32 direction, MapSprite *sprite) {
	uint32 r_check, c_check; // Variables for holding the position of a tile to check.

	// Set the sprite's facing direction and tile coordinates it wishes to move to
	switch (direction) {
		case MOVE_NORTH:
			sprite->_direction = NORTH;
			r_check = sprite->_row_pos - 1;
			c_check = sprite->_col_pos;
			break;
		case MOVE_SOUTH:
			sprite->_direction = SOUTH;
			r_check = sprite->_row_pos + 1;
			c_check = sprite->_col_pos;
			break;
		case MOVE_WEST:
			sprite->_direction = WEST;
			r_check = sprite->_row_pos;
			c_check = sprite->_col_pos - 1;
			break;
		case MOVE_EAST:
			sprite->_direction = EAST;
			r_check = sprite->_row_pos;
			c_check = sprite->_col_pos + 1;
			break;
		case MOVE_NW:
			if (sprite->_direction & (NORTH_NW | NORTH | NORTH_NE | EAST_NE | EAST | EAST_SE))
				sprite->_direction = NORTH_NW;
			else
				sprite->_direction = WEST_NW;
			r_check = sprite->_row_pos - 1;
			c_check = sprite->_col_pos - 1;
			break;
		case MOVE_SW:
			if (sprite->_direction & (SOUTH_SW | SOUTH | SOUTH_SE | EAST_SE | EAST | EAST_NE))
				sprite->_direction = SOUTH_SW;
			else
				sprite->_direction = WEST_SW;
			r_check = sprite->_row_pos + 1;
			c_check = sprite->_col_pos - 1;
			break;
		case MOVE_NE:
			if (sprite->_direction & (NORTH_NE | NORTH | NORTH_NW | WEST_NW | WEST | WEST_SW))
				sprite->_direction = NORTH_NE;
			else
				sprite->_direction = EAST_NE;
			r_check = sprite->_row_pos - 1;
			c_check = sprite->_col_pos + 1;
			break;
		case MOVE_SE:
			if (sprite->_direction & (SOUTH_SE | SOUTH | SOUTH_SW | WEST_SW | WEST | WEST_NW))
				sprite->_direction = SOUTH_SE;
			else
				sprite->_direction = EAST_SE;
			r_check = sprite->_row_pos + 1;
			c_check = sprite->_col_pos + 1;
			break;
	}

	// Check if the sprite can move to the new tile and change the game state accordingly
	if (_TileMoveable(r_check, c_check, sprite->_altitude)) {
		// First set the sprite's motion flag
		sprite->_status |= IN_MOTION;
		// For the tile the sprite is moving off of, negate the occupied bit.
		_map_layers[sprite->_row_pos][sprite->_col_pos].occupied &= ~sprite->_altitude;
		
		// ************************ Check For Tile Departure Event ***********************
		if (_map_layers[r_check][c_check].properties & DEPART_EVENT) {
			// Look-up and process the event associated with the tile.
			cout << "Tile had a departure event." << endl;
		}
		
		sprite->_row_pos = r_check;
		sprite->_col_pos = c_check;
		// Set the occuped bit for the tile the sprite is moving on to.
		_map_layers[sprite->_row_pos][sprite->_col_pos].occupied |= sprite->_altitude;
		
		// ************************ Check For Tile Departure Event ***********************
		if (_map_layers[r_check][c_check].properties & ARRIVE_EVENT) {
			// Look-up and process the event associated with the tile.
			cout << "Tile has an arrival event." << endl;
		}
	}
	else {
		sprite->_status &= ~IN_MOTION;
		// TODO: Only modify the wait time if the sprite is a random NPC mover, not scripted and player sprites.
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

		//if (user_move) {
			// Check that the tile the player wishes to move to isn't off the map
			
			// A user can't move and do another command, so exit
		//}
	}
} // MapMode::_UpdateVirtualSprite()

// ****************************************************************************
// **************************** UPDATE FUNCTIONS ******************************
// ****************************************************************************

// Updates the game state when in map mode. Called from the main game loop.
void MapMode::Update(uint32 new_time_elapsed) {
	_time_elapsed = new_time_elapsed;
	_animation_counter += _time_elapsed;

	// *********** (1) Update the tile animation frames if needed ***********
	// NOTE: This section will become defunct once animation is supported in the video engine
	
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
		int32 j = static_cast<int32>(i) - 1;
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
			case CHARACTER_SPRITE:
			{
				ObjectLayer *pSprite = _ground_objects[i];
				_UpdatePlayer(dynamic_cast<MapSprite *>(_ground_objects[i]));
				break;
			}
			case NPC_SPRITE:
				_UpdateNPC((MapSprite*)(_ground_objects[i]));
				break;
			default:
				break;
		}
	}

	if (_focused_object == _virtual_sprite)
		_UpdateVirtualSprite();
}


// Updates the player-controlled sprite and also processes user input.
void MapMode::_UpdatePlayer(MapSprite *player_sprite) {
	int32 r_check, c_check;   // Variables for saving tile coordinates
	int32 move_direction;     // The direction the sprite may be set to move in
	bool user_move = false; // Set to true if the user attempted to move the player sprite

	// *********** (1) Handle updates for the player sprite when in motion ************
	if (player_sprite->_status & IN_MOTION) {
		player_sprite->_step_count += (float)_time_elapsed / player_sprite->_step_speed;

		// Check whether we've reached a new tile
		if (player_sprite->_step_count >= player_sprite->_step_speed) {
			player_sprite->_step_count -= player_sprite->_step_speed;

			player_sprite->_status &= ~IN_MOTION; // IN_MOTION may get set again shortly
			player_sprite->_status ^= STEP_SWAP; // This flips the step_swap bit. Bit-wise XOR

			if (_random_encounters) {
				_steps_till_encounter--;
				if (player_sprite->_direction & !(WEST | EAST | SOUTH | NORTH))
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
			_GroundSpriteMove(move_direction, player_sprite);
			// Regardless of whether the move was successful or not, refuse to process additional commands
			//  from the user.
			return;
		}
	}

	if (InputManager->MenuPress()) { // Push MenuMode onto the stack
		MenuMode *MenuM = new MenuMode();
		ModeManager->Push(MenuM);
	}

	// Handle confirm command.
	else if (InputManager->ConfirmPress()) {

		if (player_sprite->_direction & (WEST | WEST_NW | WEST_SW)) {
			r_check = player_sprite->_row_pos;
			c_check = player_sprite->_col_pos - 1;
		}
		else if (player_sprite->_direction & (EAST | EAST_NE | EAST_SE)) {
			r_check = player_sprite->_row_pos;
			c_check = player_sprite->_col_pos + 1;
		}
		else if (player_sprite->_direction & (NORTH | NORTH_NW | NORTH_NE)) {
			r_check = player_sprite->_row_pos - 1;
			c_check = player_sprite->_col_pos;
		}
		else { // then => (player_sprite->_direction & (SOUTH | SOUTH_SW | SOUTH_SE)) == true
			r_check = player_sprite->_row_pos + 1;
			c_check = player_sprite->_col_pos;
		}

		// Check the tile the player is facing for events or other objects that can be interacted with.
		_CheckInteraction(r_check, c_check, player_sprite->_altitude);
		return;
	}
}



// Updates the NPC sprites
void MapMode::_UpdateNPC(MapSprite *npc) {

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
				_GroundSpriteMove(RandomNum(0,7), npc);
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
			_GroundSpriteMove(RandomNum(0,7), npc);
		}
	}
}


// Updates the game status when MapMode is in the 'dialogue' state
void MapMode::_UpdateDialogueState() {
// 	cout << "UpdateDialogueState: setting text" << endl;
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		if (_ground_objects[i] != _focused_object) {
			MapSprite *tmp_sprite = dynamic_cast<MapSprite*>(_ground_objects[i]);
			_dialogue_text = tmp_sprite->_speech->_conversations[tmp_sprite->_speech->_next_read][0];
		}
	}
	
	// User is done reading the dialogue if they press confirm
	if (InputManager->ConfirmPress()) {
		cout << "UpdateDialogueState: processing confirm press" << endl;
		// Remove the dialogue state from the map state stack
		_map_state.pop_back();
		
		// Restore the status of map sprites
		for (uint32 i = 0; i < _ground_objects.size(); i++) {
			if (_ground_objects[i] != _focused_object) {
				MapSprite *tmp_sprite = dynamic_cast<MapSprite*>(_ground_objects[i]);
				tmp_sprite->_status = tmp_sprite->_speech->_saved_status;
				tmp_sprite->_direction = tmp_sprite->_speech->_saved_direction;
				if (_ground_objects[i] != _focused_object) {
					tmp_sprite->_speech->FinishedConversation();
				}
			}
		}
	}
}



// Updates the game status when MapMode is in the 'script_event' state
void MapMode::_UpdateScriptState() {
	cout << "TEMP: UpdateScriptState()" << endl;
}




// ****************************************************************************
// **************************** DRAW FUNCTIONS ********************************
// ****************************************************************************


// Determines things like our starting tiles
void MapMode::_GetDrawInfo() {
	// ************* (1) Calculate the default drawing positions for the tiles ****************
	// Draw from the top left corner
	_map_info.c_pos = (-SCREEN_COLS / 2.0f) - 0.5f;
	_map_info.r_pos = (SCREEN_ROWS / 2.0f) - 0.5f;

	// Set the default col and row tile counts
	_map_info.c_draw = static_cast<uint32>(SCREEN_COLS) + 1;
	_map_info.r_draw = static_cast<uint32>(SCREEN_ROWS) + 1;

	// These are the default starting positions
	_map_info.c_start = _focused_object->_col_pos - (static_cast<int32>(SCREEN_COLS) / 2);
	_map_info.r_start = _focused_object->_row_pos - (static_cast<int32>(SCREEN_ROWS) / 2);

	// *********************** (2) Calculate the drawing information **************************
	if (_focused_object->_status & IN_MOTION) {
		if (_focused_object->_step_count <= (_focused_object->_step_speed / 2.0f)) {
			// We are not more than half-way moving west, so make adjustments
			if (_focused_object->_direction & (WEST | NORTH_NW | WEST_NW | SOUTH_SW | WEST_SW)) {
				_map_info.c_pos += _focused_object->_step_count / _focused_object->_step_speed;
				_map_info.c_start++;
			}
			// We are not more than half-way moving east, so make adjustments
			else if (_focused_object->_direction & (EAST | NORTH_NE | EAST_NE | SOUTH_SE | EAST_SE)) {
				_map_info.c_pos -= _focused_object->_step_count / _focused_object->_step_speed;
				_map_info.c_start--;
			}

			// We are not more than half-way moving north, so make adjustments
			if (_focused_object->_direction & (NORTH | WEST_NW | NORTH_NW | EAST_NE | NORTH_NE)) {
				_map_info.r_pos -= _focused_object->_step_count / _focused_object->_step_speed;
				_map_info.r_start++;
			}
			// We are not more than half-way moving south, so make adjustments
			else if (_focused_object->_direction & (SOUTH | WEST_SW | SOUTH_SW | EAST_SE | SOUTH_SE)) {
				_map_info.r_pos += _focused_object->_step_count / _focused_object->_step_speed;
				_map_info.r_start--;
			}
		}

		// NOTE: Draw code should never see a step_count >= 32. Update() takes care of that
		else { // (_focused_object->_step_count > (TILE_STEPS / 2))
			// We are at least half-way moving west, so make adjustments
			if (_focused_object->_direction & (WEST | NORTH_NW | WEST_NW | SOUTH_SW | WEST_SW)) {
				_map_info.c_pos -= (_focused_object->_step_speed - _focused_object->_step_count) / 
				                    _focused_object->_step_speed;
			}
			// We are at least half-way moving east, so make adjustments
			else if (_focused_object->_direction & (EAST | NORTH_NE | EAST_NE | SOUTH_SE | EAST_SE)) {
				_map_info.c_pos += (_focused_object->_step_speed - _focused_object->_step_count) / 
				                    _focused_object->_step_speed;
			}

			// We are at least half-way moving north, so make adjustments
			if (_focused_object->_direction & (NORTH | WEST_NW | NORTH_NW | EAST_NE | NORTH_NE)) {
				_map_info.r_pos += (_focused_object->_step_speed - _focused_object->_step_count) / 
				                    _focused_object->_step_speed;
			}
			// We are at least half-way moving south, so make adjustments
			else if (_focused_object->_direction & (SOUTH | WEST_SW | SOUTH_SW | EAST_SE | SOUTH_SE)) {
				_map_info.r_pos -= (_focused_object->_step_speed - _focused_object->_step_count) / 
				                    _focused_object->_step_speed;
			}
		}
	}

	// *********************** (3) Check for special conditions **************************

	// Usually the map "moves around the player", but when we encounter the edges of the map we
	// need the player to "move around the map".

	// Exceeds the far-left side of the map
	if (_map_info.c_start < 0) { 
		_map_info.c_start = 0;
		_map_info.c_pos = -(SCREEN_COLS / 2.0f);
	}
	// Exceeds the far-right side of the map
	else if (_map_info.c_start > _col_count - static_cast<int32>(SCREEN_COLS) - 1) { 
		_map_info.c_start = _col_count - static_cast<int32>(SCREEN_COLS);
		_map_info.c_pos = -(SCREEN_COLS / 2.0f);
	}

	// If our column position is exactly on the left edge of the screen, we draw one less column of tiles
	if (_map_info.c_pos == -(SCREEN_COLS / 2.0f)) {
		_map_info.c_draw--;
	}

	// Exceeds the far-north side of the map
	if (_map_info.r_start < 0) { 
		_map_info.r_start = 0;
		_map_info.r_pos = (SCREEN_ROWS / 2.0f) - 1.0f;
	}
	// Exceeds the far-south side of the map
	else if (_map_info.r_start > _row_count - static_cast<int32>(SCREEN_ROWS) - 1) { 
		_map_info.r_start = _row_count - static_cast<int32>(SCREEN_ROWS);
		_map_info.r_pos = (SCREEN_ROWS / 2.0f) - 1.0f;
	}

	// If the row position is exactly on the top of the screen, draw one less row of tiles
	if (_map_info.r_pos == (SCREEN_ROWS / 2.0f) - 1.0f) {
		_map_info.r_draw--;
	}
} // MapMode::_GetDrawInfo()


// Public draw function called by the main game loop
void MapMode::Draw() {
	// Calculate all the information we need for drawing this map frame
	_GetDrawInfo(); 

	// ************** (1) Draw the Lower Layer *************
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_NO_BLEND, 0);
	VideoManager->Move(_map_info.c_pos, _map_info.r_pos);
	for (uint32 r = static_cast<uint32>(_map_info.r_start); 
	     r < static_cast<uint32>(_map_info.r_start) + _map_info.r_draw; r++) {
		for (uint32 c = static_cast<uint32>(_map_info.c_start); 
		     c < static_cast<uint32>(_map_info.c_start) + _map_info.c_draw; c++) {
			if (_map_layers[r][c].lower_layer >= 0) { // Then a lower layer tile exists and we should draw it
				VideoManager->DrawImage(_map_tiles[_tile_frames[_map_layers[r][c].lower_layer]->frame]);
			}
			VideoManager->MoveRelative(1.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(_map_info.c_draw), -1.0f);

	}

	// ************** (2) Draw the Object Layer *************
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		(_ground_objects[i])->Draw(_map_info);
	}

	// ************** (3) Draw the Upper Layer *************
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->Move(_map_info.c_pos, _map_info.r_pos);
	for (uint32 r = _map_info.r_start; r < _map_info.r_start + _map_info.r_draw; r++) {
		for (uint32 c = _map_info.c_start; c < _map_info.c_start + _map_info.c_draw; c++) {
			if (_map_layers[r][c].upper_layer >= 0) // Then an upper layer tile exists and we should draw it;
				VideoManager->DrawImage(_map_tiles[_tile_frames[_map_layers[r][c].upper_layer]->frame]);
			VideoManager->MoveRelative(1.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(_map_info.c_draw), -1.0f);
	}


	// ************** (4) Draw the Dialoge menu and text *************
	if (_map_state.back() == DIALOGUE) {
// 		cout << _dialogue_text << endl;

		//---Raj added some sample code here for textbox display---

		float dialogueWidth = 1024.0f;
		float dialogueHeight = 150.0f;

		ImageDescriptor menu;
		VideoManager->CreateMenu(menu, dialogueWidth, dialogueHeight);
		
		VideoManager->PushState();
		VideoManager->SetCoordSys(CoordSys(0, 1024, 0, 768));
		VideoManager->Move(0,0);
		VideoManager->DrawImage(menu);
		VideoManager->DeleteImage(menu);
		VideoManager->PopState();


		VideoManager->PushState();		
		VideoManager->SetCoordSys(CoordSys(0,1024,0,768));		
	
		// Roots: here's a BASIC example of using a textbox. Note, in reality you wouldn't
		//        just create a textbox every frame, but this is just for an example:
	
		TextBox box;
		box.SetDisplaySpeed(10);
		box.SetPosition(0,0);
		box.SetDimensions(dialogueWidth, dialogueHeight);
		box.SetFont("default");
		box.SetDisplayMode(VIDEO_TEXT_INSTANT);
		box.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
		box.ShowText(_dialogue_text);
		box.Draw();
		
		VideoManager->PopState();				
		//---End Raj's Sample code------------------

	}
	
	return;
} // MapMode::_Draw()


} // namespace hoa_map
