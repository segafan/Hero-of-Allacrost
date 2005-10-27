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

void MapMode::_TEMP_CreateMap() {
	_random_encounters = true;
	_encounter_rate = 12;
	_steps_till_encounter = GaussianValue(_encounter_rate, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	_animation_counter = 0;

	_row_count = 60;
	_col_count = 80;
	
	// Setup GUI items in 1024x768 coordinate system
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

	// Load in all tile images from memory
	StaticImage imd;
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
		_tile_layers.push_back(vector <MapTile>());
		for (uint32 c = 0; c < _col_count; c++) {
			tmp_tile.lower_layer = static_cast<int16>(RandomNumber(0, 16 - 1)); // Build the lower layer from random tiles
			if (tmp_tile.lower_layer == 15) { // Set water tile properties
				tmp_tile.not_walkable = ALTITUDE_1;
				tmp_tile.properties = CONFIRM_EVENT;
			}
			else {
				tmp_tile.not_walkable = 0x00;
				tmp_tile.properties = 0x00;
			}
			tmp_tile.occupied = 0x00;
			_tile_layers[r].push_back(tmp_tile);
		}
	}
	
	MapSprite::VideoManager = VideoManager;
	MapSprite::CurrentMap = this;
	
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
	npc_sprite->AddDialogue(std::vector<std::string>(1, "I'm a hottie!"));
	npc_sprite->AddDialogue(std::vector<std::string>(1, "But, I'm also your sister..."));
	npc_sprite->AddDialogue(std::vector<std::string>(1, "Is this really okay?"));
	_ground_objects.push_back(npc_sprite);

	// If the _focused_object is ever NULL, the game will exit with a seg fault :(
	_focused_object = player;
}



MapMode::MapMode(uint32 new_map_id) {
	if (MAP_DEBUG) cout << "MAP: MapMode constructor invoked" << endl;

	mode_type = ENGINE_MAP_MODE;
	_map_state = EXPLORE;
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
	
	// Free up the dialogue menu
	_dialogue_window.Destroy();
}


// Resets appropriate class members.
void MapMode::Reset() {
	_dialogue_text = NULL;
	
	// Set video engine properties
	VideoManager->SetCoordSys(-SCREEN_COLS/2.0f, SCREEN_COLS/2.0f, -SCREEN_ROWS/2.0f, SCREEN_ROWS/2.0f);
	if(!VideoManager->SetFont("default")) 
    cerr << "MAP: ERROR > Couldn't set map font!" << endl;
	
	// Set MapObject Static Pointers
	MapObject::VideoManager = VideoManager;
	MapObject::CurrentMap = this;
}


// Returns true if an object can be moved to the tile.
bool MapMode::_TileMoveable(const private_map::TileCheck& tcheck) {
	// Check that the row and col indeces are valid and not outside the map
	// (By the way, the top row is never walkable on a map so we don't check that)
	if (tcheck.row < 1 || tcheck.col < 0 || tcheck.row >= _row_count || tcheck.col >= _col_count) {
		return false;
	}

	// Check if the tile is not walkable at this altitude
	if (_tile_layers[tcheck.row][tcheck.col].not_walkable & tcheck.altitude) {
		return false;
	}
	
	// Don't allow diagonal movement if any component tiles (top/bottom/left/right) are unwalkable
	switch (tcheck.direction) {
		case NORTH:
		case SOUTH:
		case WEST:
		case EAST:
			break;
		case NORTH_NW:
		case WEST_NW:
			if ((_tile_layers[tcheck.row][tcheck.col + 1].not_walkable & tcheck.altitude) ||
			    (_tile_layers[tcheck.row + 1][tcheck.col].not_walkable & tcheck.altitude)) {
				return false;
			}
			break;
		case SOUTH_SW:
		case WEST_SW:
			if ((_tile_layers[tcheck.row][tcheck.col + 1].not_walkable & tcheck.altitude) ||
			    (_tile_layers[tcheck.row - 1][tcheck.col].not_walkable & tcheck.altitude)) {
				return false;
			}
			break;
		case NORTH_NE:
		case EAST_NE:
			if ((_tile_layers[tcheck.row][tcheck.col - 1].not_walkable & tcheck.altitude) ||
			    (_tile_layers[tcheck.row + 1][tcheck.col].not_walkable & tcheck.altitude)) {
				return false;
			}
			break;
		case SOUTH_SE:
		case EAST_SE:
			if ((_tile_layers[tcheck.row][tcheck.col - 1].not_walkable & tcheck.altitude) ||
			    (_tile_layers[tcheck.row - 1][tcheck.col].not_walkable & tcheck.altitude)) {
				return false;
			}
			break;
		default:
			if (MAP_DEBUG) cerr << "MAP: WARNING: Called MapMode::_TileMoveable() with an invalid direction" << endl;
			return false;
	}
	
	// Check that no other objects occupy this tile
	if (_tile_layers[tcheck.row][tcheck.col].occupied & tcheck.altitude) {
		return false;
	}

	return true;
}


// Checks if there is an interaction with a sprite at the specified tile.
uint32 MapMode::_CheckInteraction(const private_map::TileCheck& tcheck) {
	// Check that the row and col indeces are valid and not outside the map
	if (tcheck.row < 1 || tcheck.col < 0 || tcheck.row >= _row_count || tcheck.col >= _col_count) {
		cerr << "MAP: WARNING: Called _CheckInteraction() with out of bounds row and column" << endl;
		return NO_INTERACTION;
	}
	
	// Check if an object occupies the tile
	if (_tile_layers[tcheck.row][tcheck.col].occupied && tcheck.altitude) {
		// Look up the object that occupies this tile at this altitude
		return SPRITE_INTERACTION;
	}
	
	return NO_INTERACTION;
}



MapObject* MapMode::_FindTileOccupant(const private_map::TileCheck& tcheck) {
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		if (_ground_objects[i]->_row_pos == tcheck.row && 
				_ground_objects[i]->_col_pos == tcheck.col &&
				_ground_objects[i]->_altitude == tcheck.altitude) {
			return _ground_objects[i];
		}  
	}
	return NULL;
} // MapMode::_FindTileOccupant()



void MapMode::_FindPath(const MapSprite* sprite, const private_map::TileNode& destination) {
	// NOTE: This algorithm is just in its primitive stages now and only checks for non-movable tiles.
	// It will also check for occupied tiles at a later time.
	
	// The source tile that the sprite is currently occupying
	TileNode source;
	// 
	vector<TileNode> open_list;
	// The nodes which have already been visited once.
	vector<TileNode> closed_list;
	
	source.row = sprite->_row_pos;
	source.col = sprite->_col_pos;

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
				move_direction = NORTH_NW;
			}
			else if (InputManager->DownState() || InputManager->DownPress()) {
				move_direction = SOUTH_SW;
			}
			else {
				move_direction = WEST;
			}
		}

		// Handle east, northeast, and southeast movement
		else if (InputManager->RightState() || InputManager->RightPress()) {
			user_move = true;
			if (InputManager->UpState() || InputManager->UpPress())
				move_direction = NORTH_NE;
			else if (InputManager->DownState() || InputManager->DownPress())
				move_direction = SOUTH_SE;
			else
				move_direction = EAST;
		}

		// Handle north movement
		else if (InputManager->UpState() || InputManager->UpPress()) {
			user_move = true;
			move_direction = NORTH;
		}

		// Handle south movement
		else if (InputManager->DownState() || InputManager->DownPress()) {
			user_move = true;
			move_direction = SOUTH;
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
		_animation_counter -= ANIMATION_RATE;
	}

	// ***************** (2) Update the map based on what state we are in **************
	switch (_map_state) {
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
	
	// ***************** (3) Update all objects on the map **************
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		switch ((_ground_objects[i])->_object_type) {
			case NPC_SPRITE:
				dynamic_cast<MapSprite *>(_ground_objects[i])->Update();
				break;
			default:
				break;
		}
	}

	// ************ (4) Sort the objects so they are in the correct draw order ********
	// Note: this sorting algorithm will be optimized at a later time
	for (uint32 i = 1; i < _ground_objects.size(); i++) {
		MapObject *tmp = _ground_objects[i];
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
	if (_focused_object->_object_type == CHARACTER_SPRITE) {
		_UpdatePlayer(_focused_object);
	}
	if (_focused_object == _virtual_sprite)
		_UpdateVirtualSprite();
}


// Updates the player-controlled sprite and also processes user input.
void MapMode::_UpdatePlayer(MapSprite *player_sprite) {
	bool user_move = false; // Set to true if the user attempted to move the player sprite
	uint16 move_direction = 0;

	// *********** (1) Handle updates for the player sprite when in motion ************
	if (player_sprite->_status & IN_MOTION) {
		player_sprite->_step_count += (float)_time_elapsed / player_sprite->_step_speed;

		// Check whether we've reached a new tile
		if (player_sprite->_step_count >= player_sprite->_step_speed) {
			player_sprite->_step_count -= player_sprite->_step_speed;

			player_sprite->_status &= ~IN_MOTION; // IN_MOTION may get set again shortly
			player_sprite->_status ^= STEP_SWAP;  // This flips the step_swap bit. Bit-wise XOR.

			if (_random_encounters) {
				_steps_till_encounter--;
				// Decrease count again if player moved diagonally
				if (player_sprite->_direction & DIAGONAL)
					_steps_till_encounter--; 

				if (_steps_till_encounter <= 0) { // then a random encounter has occured
					player_sprite->_step_count = 0; // we need to be centered on a tile when we return from battle
					_steps_till_encounter = GaussianValue(_encounter_rate, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);

					// Play a scary random encounter sound
					// Fade the screen out, or some other visual effect
					// BattleMode *BAM = new BattleMode(_list_of_enemies_);
					// ModeManager->Push(BAM);
					return;
				}
			}
		}
	}
	
	// ********* (2) Handle updates for the player sprite when not in motion **********
	if (!(player_sprite->_status & IN_MOTION)) {

		// Handle west, northwest, and southwest movement
		if (InputManager->LeftState()) {
			user_move = true;
			if (InputManager->UpState()) {
				move_direction = NORTH_NW;
			}
			else if (InputManager->DownState()) {
				move_direction = SOUTH_SW;
			}
			else {
				move_direction = WEST;
			}
		}

		// Handle east, northeast, and southeast movement
		else if (InputManager->RightState()) {
			user_move = true;
			if (InputManager->UpState())
				move_direction = NORTH_NE;
			else if (InputManager->DownState())
				move_direction = SOUTH_SE;
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

		// Now check if we can actualy move the sprite to the tile the user requested to move to
		if (user_move) {
			player_sprite->GroundMove(move_direction);
			// If the move was successful, don't process any other input
			if (player_sprite->_status & IN_MOTION) {
				return;
			}
		}
	}

	if (InputManager->MenuPress()) { // Push MenuMode onto the stack
		MenuMode *MenuM = new MenuMode();
		ModeManager->Push(MenuM);
	}

	// Handle confirm command.
	else if (InputManager->ConfirmPress()) {
		TileCheck tcheck;

		if (player_sprite->_direction & (WEST | WEST_NW | WEST_SW)) {
			tcheck.row = player_sprite->_row_pos;
			tcheck.col = player_sprite->_col_pos - 1;
		}
		else if (player_sprite->_direction & (EAST | EAST_NE | EAST_SE)) {
			tcheck.row = player_sprite->_row_pos;
			tcheck.col = player_sprite->_col_pos + 1;
		}
		else if (player_sprite->_direction & (NORTH | NORTH_NW | NORTH_NE)) {
			tcheck.row = player_sprite->_row_pos - 1;
			tcheck.col = player_sprite->_col_pos;
		}
		else { // then => (player_sprite->_direction & (SOUTH | SOUTH_SW | SOUTH_SE)) == true
			tcheck.row = player_sprite->_row_pos + 1;
			tcheck.col = player_sprite->_col_pos;
		}
		
		tcheck.altitude = player_sprite->_altitude;
		tcheck.direction = player_sprite->_direction;

		// Check the tile the player is facing for events or other objects that can be interacted with.
		uint32 interaction = _CheckInteraction(tcheck);
		if (interaction == SPRITE_INTERACTION) {
			MapObject* obj = _FindTileOccupant(tcheck);
			if (obj != NULL) {
				switch (obj->_object_type) {
					case NPC_SPRITE:
					{
							MapSprite* spr = dynamic_cast<MapSprite*>(obj);
							spr->SaveState();
							spr->_status &= ~UPDATEABLE;
							_dialogue_window.Show();
							if (tcheck.direction & (NORTH | NORTH_NW | NORTH_NE)) {
								spr->_direction = SOUTH;
							}
							else if (tcheck.direction & (SOUTH | SOUTH_SW | SOUTH_SE)) {
								spr->_direction = NORTH;
							}
							else if (tcheck.direction & (EAST | EAST_NE | EAST_SE)) {
								spr->_direction = WEST;
							}
							else if (tcheck.direction & (WEST | WEST_NW | WEST_SW)) {
								spr->_direction = EAST;
							}
							_dialogue_speakers.push_back(spr);
							_dialogue_text = &(spr->_dialogues[spr->_next_dialogue]._lines);
							_dialogue_line = 0;
							_dialogue_textbox.ShowText(MakeWideString((*_dialogue_text)[0]));
							_map_state = DIALOGUE;
						break;
					}
					default:
						break;
				}
			}
			else {
				cerr << "MAP: WARNING: No occupant found in call to _FindTileOccupant()" << endl;
			}
		}
		return;
	}
}



// Updates the game status when MapMode is in the 'dialogue' state
void MapMode::_UpdateDialogueState() {
	_dialogue_textbox.Update(_time_elapsed);
	
	// User is done reading the dialogue if they press confirm
	if (InputManager->ConfirmPress()) {
		_dialogue_line++;
		// Check if we've passed the last line of dialogue, and if so exit the dialogue
		if (_dialogue_line >= _dialogue_text->size()) {
				// Remove the dialogue state from the map state stack
			_map_state = EXPLORE;
		
			// Restore the status of map sprites
			for (uint32 i = 0; i < _dialogue_speakers.size(); i++) {
				if (_ground_objects[i] != _focused_object) {
					_dialogue_speakers[i]->RestoreState();
					_dialogue_speakers[i]->FinishedDialogue();
				}
			}
			
			// Clean out the dialogue speakers
			_dialogue_speakers.clear();
			_dialogue_window.Hide();
		}
		else {
			_dialogue_textbox.ShowText(MakeWideString((*_dialogue_text)[_dialogue_line]));
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
	_draw_info.c_pos = (-SCREEN_COLS / 2.0f) - 0.5f;
	_draw_info.r_pos = (SCREEN_ROWS / 2.0f) - 0.5f;

	// Set the default col and row tile counts
	_draw_info.c_draw = static_cast<uint8>(SCREEN_COLS) + 1;
	_draw_info.r_draw = static_cast<uint8>(SCREEN_ROWS) + 1;

	// These are the default starting positions
	_draw_info.c_start = static_cast<int16>(_focused_object->_col_pos - (static_cast<int32>(SCREEN_COLS) / 2));
	_draw_info.r_start = static_cast<int16>(_focused_object->_row_pos - (static_cast<int32>(SCREEN_ROWS) / 2));

	// *********************** (2) Calculate the drawing information **************************
	if (_focused_object->_status & IN_MOTION) {
		if (_focused_object->_step_count <= (_focused_object->_step_speed / 2.0f)) {
			// We are not more than half-way moving west, so make adjustments
			if (_focused_object->_direction & (WEST | NORTH_NW | WEST_NW | SOUTH_SW | WEST_SW)) {
				_draw_info.c_pos += _focused_object->_step_count / _focused_object->_step_speed;
				_draw_info.c_start++;
			}
			// We are not more than half-way moving east, so make adjustments
			else if (_focused_object->_direction & (EAST | NORTH_NE | EAST_NE | SOUTH_SE | EAST_SE)) {
				_draw_info.c_pos -= _focused_object->_step_count / _focused_object->_step_speed;
				_draw_info.c_start--;
			}

			// We are not more than half-way moving north, so make adjustments
			if (_focused_object->_direction & (NORTH | WEST_NW | NORTH_NW | EAST_NE | NORTH_NE)) {
				_draw_info.r_pos -= _focused_object->_step_count / _focused_object->_step_speed;
				_draw_info.r_start++;
			}
			// We are not more than half-way moving south, so make adjustments
			else if (_focused_object->_direction & (SOUTH | WEST_SW | SOUTH_SW | EAST_SE | SOUTH_SE)) {
				_draw_info.r_pos += _focused_object->_step_count / _focused_object->_step_speed;
				_draw_info.r_start--;
			}
		}

		// NOTE: Draw code should never see a step_count >= 32. Update() takes care of that
		else { // (_focused_object->_step_count > (TILE_STEPS / 2))
			// We are at least half-way moving west, so make adjustments
			if (_focused_object->_direction & (WEST | NORTH_NW | WEST_NW | SOUTH_SW | WEST_SW)) {
				_draw_info.c_pos -= (_focused_object->_step_speed - _focused_object->_step_count) / 
				                    _focused_object->_step_speed;
			}
			// We are at least half-way moving east, so make adjustments
			else if (_focused_object->_direction & (EAST | NORTH_NE | EAST_NE | SOUTH_SE | EAST_SE)) {
				_draw_info.c_pos += (_focused_object->_step_speed - _focused_object->_step_count) / 
				                    _focused_object->_step_speed;
			}

			// We are at least half-way moving north, so make adjustments
			if (_focused_object->_direction & (NORTH | WEST_NW | NORTH_NW | EAST_NE | NORTH_NE)) {
				_draw_info.r_pos += (_focused_object->_step_speed - _focused_object->_step_count) / 
				                    _focused_object->_step_speed;
			}
			// We are at least half-way moving south, so make adjustments
			else if (_focused_object->_direction & (SOUTH | WEST_SW | SOUTH_SW | EAST_SE | SOUTH_SE)) {
				_draw_info.r_pos -= (_focused_object->_step_speed - _focused_object->_step_count) / 
				                    _focused_object->_step_speed;
			}
		}
	}

	// *********************** (3) Check for special conditions **************************

	// Usually the map "moves around the player", but when we encounter the edges of the map we
	// need the player to "move around the map".

	// Exceeds the far-left side of the map
	if (_draw_info.c_start < 0) { 
		_draw_info.c_start = 0;
		_draw_info.c_pos = -(SCREEN_COLS / 2.0f);
	}
	// Exceeds the far-right side of the map
	else if (_draw_info.c_start > _col_count - static_cast<int32>(SCREEN_COLS) - 1) { 
		_draw_info.c_start = static_cast<int16>(_col_count - static_cast<int32>(SCREEN_COLS));
		_draw_info.c_pos = -(SCREEN_COLS / 2.0f);
	}

	// If our column position is exactly on the left edge of the screen, we draw one less column of tiles
	if (_draw_info.c_pos == -(SCREEN_COLS / 2.0f)) {
		_draw_info.c_draw--;
	}

	// Exceeds the far-north side of the map
	if (_draw_info.r_start < 0) { 
		_draw_info.r_start = 0;
		_draw_info.r_pos = (SCREEN_ROWS / 2.0f) - 1.0f;
	}
	// Exceeds the far-south side of the map
	else if (_draw_info.r_start > _row_count - static_cast<int32>(SCREEN_ROWS) - 1) { 
		_draw_info.r_start = static_cast<int16>(_row_count - static_cast<int32>(SCREEN_ROWS));
		_draw_info.r_pos = (SCREEN_ROWS / 2.0f) - 1.0f;
	}

	// If the row position is exactly on the top of the screen, draw one less row of tiles
	if (_draw_info.r_pos == (SCREEN_ROWS / 2.0f) - 1.0f) {
		_draw_info.r_draw--;
	}
	
} // MapMode::_GetDrawInfo()


// Public draw function called by the main game loop
void MapMode::Draw() {
	// Calculate all the information we need for drawing this map frame
	_GetDrawInfo(); 

	// ************** (1) Draw the Lower Layer *************
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_NO_BLEND, 0);
	VideoManager->Move(_draw_info.c_pos, _draw_info.r_pos);
	for (uint32 r = static_cast<uint32>(_draw_info.r_start); 
	     r < static_cast<uint32>(_draw_info.r_start) + _draw_info.r_draw; r++) {
		for (uint32 c = static_cast<uint32>(_draw_info.c_start); 
		     c < static_cast<uint32>(_draw_info.c_start) + _draw_info.c_draw; c++) {
			if (_tile_layers[r][c].lower_layer >= 0) { // Then a lower layer tile exists and we should draw it
				VideoManager->DrawImage(_map_tiles[_tile_frames[_tile_layers[r][c].lower_layer]->frame]);
			}
			VideoManager->MoveRelative(1.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(_draw_info.c_draw), -1.0f);

	}

	// ************** (2) Draw the Object Layer *************
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		// The following function call only draws the object if it is visible on the screen.
		(_ground_objects[i])->Draw();
	}

	// ************** (3) Draw the Upper Layer *************
	VideoManager->Move(_draw_info.c_pos, _draw_info.r_pos);
	for (int32 r = _draw_info.r_start; r < _draw_info.r_start + _draw_info.r_draw; r++) {
		for (int32 c = _draw_info.c_start; c < _draw_info.c_start + _draw_info.c_draw; c++) {
			if (_tile_layers[r][c].upper_layer >= 0) // Then an upper layer tile exists and we should draw it;
				VideoManager->DrawImage(_map_tiles[_tile_frames[_tile_layers[r][c].upper_layer]->frame]);
			VideoManager->MoveRelative(1.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(_draw_info.c_draw), -1.0f);
	}
	
	
	// ************** (4) Draw the Dialogue menu and text *************
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
