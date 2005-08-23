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

void MapMode::TempCreateMap() {
	random_encounters = true;
	encounter_rate = 12;
	steps_till_encounter = GaussianValue(encounter_rate, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	animation_counter = 0;

	row_count = 60;
	col_count = 80;

	// Load in all tile images from memory
	ImageDescriptor imd;
	imd.width = 1;
	imd.height = 1;

	imd.filename = "img/tiles/test_01.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_02.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_03.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_04.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_05.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_06.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_07.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_08.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_09.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_10.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_11.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_12.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_13.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_14.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_15.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_16a.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_16b.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_16d.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_16d.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/test_16e.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tiles/blue_40.png";
	map_tiles.push_back(imd);

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < map_tiles.size(); i++) {
		VideoManager->LoadImage(map_tiles[i]);
	}
	VideoManager->EndImageLoadBatch();

	// Setup tile frame pointers for animation
	TileFrame *tf;
	for (uint32 i = 0; i < 15; i++) {
		tf = new TileFrame;
	  tf->frame = i;
		tf->next = tf;
		tile_frames.push_back(tf);
	}

	// Setup the final animated frame tile
	TileFrame *tmp;
	tf = new TileFrame;
	tf->frame = 15; // a
	tf->next = NULL;
	tile_frames.push_back(tf);

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
	tmp->next = tile_frames[15]; // Makes the linked list circular now

	tf = new TileFrame;
	tf->frame = 20;
	tf->next = tf;
	tile_frames.push_back(tf);


	// Setup the image map
	MapTile tmp_tile;
	tmp_tile.upper_layer = -1; // No upper layer in this test
	for (uint32 r = 0; r < row_count; r++) {
		map_layers.push_back(vector <MapTile>());
		for (uint32 c = 0; c < col_count; c++) {
			tmp_tile.lower_layer = (RandomNum(0, 16 - 1)); // Build the lower layer from random tiles
			tmp_tile.upper_layer = 16;
			if (tmp_tile.lower_layer == 15)
				tmp_tile.properties = Z_LVL1; // We can not walk on the water tiles
			else
				tmp_tile.properties = 0x0000;
			map_layers[r].push_back(tmp_tile);
		}
	}

	// Load player sprite and rest of map objects
	MapSprite *p_sprite = new MapSprite(PLAYER_SPRITE, 2, 2, (CONTROLABLE | VISIBLE | SOUTH | Z_LVL1));
	p_sprite->LoadCharacterInfo(GLOBAL_CLAUDIUS);
	ground_objects.push_back(p_sprite);

	MapSprite *npc_sprite = new MapSprite(NPC_SPRITE, 4, 6, (VISIBLE | EAST | Z_LVL1));
	npc_sprite->SetName("Laila");
	npc_sprite->SetFilename("img/sprites/map/laila");
	npc_sprite->SetSpeed(FAST_SPEED);
	npc_sprite->SetDelay(LONG_DELAY);
	npc_sprite->LoadFrames();
	npc_sprite->speech->AddDialogue("Hey there hot stuff", GLOBAL_LAILA);
	npc_sprite->speech->AddDialogue("Wanna do it again?", GLOBAL_LAILA);
	ground_objects.push_back(npc_sprite);

	// If the focused_object is ever NULL, the game will exit with a seg fault :(
	focused_object = p_sprite;
}



MapMode::MapMode(uint32 new_map_id) {
	if (MAP_DEBUG) cout << "MAP: MapMode constructor invoked" << endl;

	mode_type = ENGINE_MAP_MODE;
	map_state.push_back(EXPLORE);
	map_id = new_map_id;

	// Load the map from the Lua data file
	//DataManager->LoadMap(this, map_id);

	// Setup the coordinate system
	//VideoManager->SetCoordSys(-SCREEN_COLS/2.0, SCREEN_COLS/2.0, -SCREEN_ROWS/2.0, SCREEN_ROWS/2.0);

	virtual_sprite = new MapSprite(VIRTUAL_SPRITE, 20, 20, 0x0);
	virtual_sprite->SetSpeed(VERY_FAST_SPEED);
	virtual_sprite->SetDelay(NO_DELAY);

	// Temporary function that creates a random map
	TempCreateMap();
}



MapMode::~MapMode() {
	if (MAP_DEBUG) cout << "MAP: MapMode destructor invoked" << endl;

	// Delete all of the tile images
	for (uint32 i = 0; i < map_tiles.size(); i++) {
		VideoManager->DeleteImage(map_tiles[i]);
	}

	// Free up all the frame linked lists
	TileFrame *tf_del;
	TileFrame *tf_tmp;
	for (uint32 i = 0; i < tile_frames.size(); i++) {
		tf_del = tile_frames[i];
		tf_tmp = tf_del->next;
		delete tf_del;

		while (tf_tmp != tile_frames[i]) {
			tf_del = tf_tmp;
			tf_tmp = tf_del->next;
			delete tf_del;
		}

		tile_frames[i] = NULL;
	}

	// Delete all of the objects
	for (uint32 i = 0; i < ground_objects.size(); i++) {
		delete(ground_objects[i]);
	}
}



// Returns true if the player is able to move to the tile.
inline bool MapMode::TileMoveable(uint32 row, uint32 col, uint32 z_occupied) {
	// First check that the object in question isn't trying to move outside the map boundaries
	if (row < 1 || col < 0 || row >= row_count || col >= col_count) {
		return false;
	}

	// Check if the tile is not walkable or occupied by another object on the same z level
	if (map_layers[row][col].properties & z_occupied) {
		return false;
	}

	return true;
}


// Moves the sprite in some direction, if possible.
void MapMode::SpriteMove(uint32 direction, MapSprite *sprite) {
	uint32 r_check, c_check; // Variables for holding the position of a tile to check.

	// Set the sprite's facing direction and tile coordinates it wishes to move to
	switch (direction) {
		case MOVE_NORTH:
			sprite->status = (sprite->status & ~FACE_MASK) | NORTH;
			r_check = sprite->row_pos - 1;
			c_check = sprite->col_pos;
			break;
		case MOVE_SOUTH:
			sprite->status = (sprite->status & ~FACE_MASK) | SOUTH;
			r_check = sprite->row_pos + 1;
			c_check = sprite->col_pos;
			break;
		case MOVE_WEST:
			sprite->status = (sprite->status & ~FACE_MASK) | WEST;
			r_check = sprite->row_pos;
			c_check = sprite->col_pos - 1;
			break;
		case MOVE_EAST:
			sprite->status = (sprite->status & ~FACE_MASK) | EAST;
			r_check = sprite->row_pos;
			c_check = sprite->col_pos + 1;
			break;
		case MOVE_NW:
			if (sprite->status & (NORTH_NW | NORTH | NORTH_NE | EAST_NE | EAST | EAST_SE))
				sprite->status = (sprite->status & ~FACE_MASK) | NORTH_NW;
			else
				sprite->status = (sprite->status & ~FACE_MASK) | WEST_NW;
			r_check = sprite->row_pos - 1;
			c_check = sprite->col_pos - 1;
			break;
		case MOVE_SW:
			if (sprite->status & (SOUTH_SW | SOUTH | SOUTH_SE | EAST_SE | EAST | EAST_NE))
				sprite->status = (sprite->status & ~FACE_MASK) | SOUTH_SW;
			else
				sprite->status = (sprite->status & ~FACE_MASK) | WEST_SW;
			r_check = sprite->row_pos + 1;
			c_check = sprite->col_pos - 1;
			break;
		case MOVE_NE:
			if (sprite->status & (NORTH_NE | NORTH | NORTH_NW | WEST_NW | WEST | WEST_SW))
				sprite->status = (sprite->status & ~FACE_MASK) | NORTH_NE;
			else
				sprite->status = (sprite->status & ~FACE_MASK) | EAST_NE;
			r_check = sprite->row_pos - 1;
			c_check = sprite->col_pos + 1;
			break;
		case MOVE_SE:
			if (sprite->status & (SOUTH_SE | SOUTH | SOUTH_SW | WEST_SW | WEST | WEST_NW))
				sprite->status = (sprite->status & ~FACE_MASK) | SOUTH_SE;
			else
				sprite->status = (sprite->status & ~FACE_MASK) | EAST_SE;
			r_check = sprite->row_pos + 1;
			c_check = sprite->col_pos + 1;
			break;
	}

	// If the tile is moveable, set the motion flag and update the sprite coordinates
	if (TileMoveable(r_check, c_check, (sprite->status & Z_MASK))) {
		sprite->status |= IN_MOTION;
		map_layers[sprite->row_pos][sprite->col_pos].properties &= ~(sprite->status & Z_MASK);
		sprite->row_pos = r_check;
		sprite->col_pos = c_check;
		// TODO: Check for map event here, change sprite's Z_LVL if necessary
		map_layers[sprite->row_pos][sprite->col_pos].properties |= (sprite->status & Z_MASK);
	}
	else {
		sprite->status &= ~IN_MOTION;
		sprite->wait_time = GaussianValue(sprite->delay_time, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	}
}


// Processes user update and camera movement. Only called when the map is focused on the virtual sprite
void MapMode::UpdateVirtualSprite() {
	bool user_move;
	uint32 move_direction;

	// *********** (1) Handle updates for the player sprite when in motion ************
	if (virtual_sprite->status & IN_MOTION) {
		virtual_sprite->step_count += (float)time_elapsed / virtual_sprite->step_speed;

		// Check whether we've reached a new tile
		if (virtual_sprite->step_count >= virtual_sprite->step_speed) {
			virtual_sprite->step_count -= virtual_sprite->step_speed;
			virtual_sprite->status &= ~IN_MOTION; // IN_MOTION may get set again shortly
		}
	}

	// ********* (2) Handle updates for the player sprite when not in motion **********
	if (!(virtual_sprite->status & IN_MOTION)) {

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
			SpriteMove(move_direction, virtual_sprite); // The move will always be successful here
			// A user can't move and do another command, so exit
		}
	}

	if (InputManager->CancelPress()) {
		// TEMP: Switch focus back to player sprite
		cout << "Set focused object from Virtual sprite back to Player sprite" << endl;
		if (ground_objects[0]->object_type == PLAYER_SPRITE)
			focused_object = dynamic_cast<MapSprite*>(ground_objects[0]);
		else
			focused_object = dynamic_cast<MapSprite*>(ground_objects[1]);
	}
}

// ****************************************************************************
// **************************** UPDATE FUNCTIONS ******************************
// ****************************************************************************

// Updates the game state when in map mode. Called from the main game loop.
void MapMode::Update(uint32 new_time_elapsed) {
	time_elapsed = new_time_elapsed;
	animation_counter += time_elapsed;

	// *********** (1) Update the tile animation frames if needed ***********

	if (animation_counter >= ANIMATION_RATE) {
		// Update all tile frames
		for (uint32 i = 0; i < tile_frames.size(); i++) {
			tile_frames[i] = tile_frames[i]->next;
		}

		// Update the frames for all dynamic map objects....???
//		 list<ObjectLayer*>::iterator obj_iter;
//		 for(obj_iter = ground_objects.begin(); obj_iter != ground_objects.end(); obj_iter++ ) {
//			 if (obj_iter->object_type == DYNAMIC_OBJECT) {
//				 obj_iter->frame_count = (obj_iter->frame_count + 1) % obj_iter->frames.size();
//			 }
//		 }

		animation_counter -= ANIMATION_RATE;
	}

	// ***************** (2) Update the map based on what state we are in **************
	switch (map_state.back()) {
		case EXPLORE:
			UpdateExploreState();
			break;
		case DIALOGUE:
			UpdateDialogueState();
			break;
		case SCRIPT_EVENT:
			UpdateScriptState();
			break;
	}

	// ************ (3) Sort the objects so they are in the correct draw order ********
	for (uint32 i = 1; i < ground_objects.size(); i++) {
		ObjectLayer *tmp = ground_objects[i];
		int32 j = static_cast<int>(i) - 1;
		while (j >= 0 && (ground_objects[j])->row_pos > tmp->row_pos) {
			ground_objects[j+1] = ground_objects[j];
			j--;
		}
		ground_objects[j+1] = tmp;
	}
}



// Updates the game status when MapMode is in the 'explore' state
void MapMode::UpdateExploreState() {
	// Update all game objects (??? Or only non-playable sprites ???)
	for (uint32 i = 0; i < ground_objects.size(); i++) {
		switch ((ground_objects[i])->object_type) {
			case PLAYER_SPRITE:
			{
				ObjectLayer *pSprite = ground_objects[i];
				UpdatePlayerExplore(dynamic_cast<MapSprite *>(ground_objects[i]));
				break;
			}
			case NPC_SPRITE:
				UpdateNPCExplore((MapSprite*)(ground_objects[i]));
				break;
			default:
				break;
		}
	}

	if (focused_object == virtual_sprite)
		UpdateVirtualSprite();
}


// Updates the player-controlled sprite and processes user input while in the 'explore' state
void MapMode::UpdatePlayerExplore(MapSprite *player_sprite) {
	int32 r_check, c_check;   // Variables for saving tile coordinates
	int32 move_direction;     // The direction the sprite may be set to move in
	bool user_move = false; // Set to true if the user attempted to move the player sprite

	// *********** (!) Handle updates for the player sprite when in motion ************
	if (player_sprite->status & IN_MOTION) {
		player_sprite->step_count += (float)time_elapsed / player_sprite->step_speed;

		// Check whether we've reached a new tile
		if (player_sprite->step_count >= player_sprite->step_speed) {
			player_sprite->step_count -= player_sprite->step_speed;

			player_sprite->status &= ~IN_MOTION; // IN_MOTION may get set again shortly
			player_sprite->status ^= STEP_SWAP; // This flips the step_swap bit. Bit-wise XOR

			if (random_encounters) {
				steps_till_encounter--;
				if (player_sprite->status & !(WEST | EAST | SOUTH | NORTH))
					steps_till_encounter--; // Decrease count again if player moved diagonally

				if (random_encounters && (steps_till_encounter <= 0)) { // then a random encounter has occured
					player_sprite->step_count = 0; // we need to be centered on a tile when we return from battle
					steps_till_encounter = GaussianValue(encounter_rate, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);

					if (MAP_DEBUG) cout << "MAP: Oh noes! A random encounter!!! Next encounter in: " << steps_till_encounter
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
	if (player_sprite != focused_object)
		return;

	// ********* (2) Handle updates for the player sprite when not in motion **********
	if (!(player_sprite->status & IN_MOTION)) {

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
			SpriteMove(move_direction, player_sprite);
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
// 		focused_object = virtual_sprite;
// 		return;
	}

	// Handle confirm command.
	else if (InputManager->ConfirmPress()) {

		if (player_sprite->status & (WEST | WEST_NW | WEST_SW)) {
			r_check = player_sprite->row_pos;
			c_check = player_sprite->col_pos - 1;
		}
		else if (player_sprite->status & (EAST | EAST_NE | EAST_SE)) {
			r_check = player_sprite->row_pos;
			c_check = player_sprite->col_pos + 1;
		}
		else if (player_sprite->status & (NORTH | NORTH_NW | NORTH_NE)) {
			r_check = player_sprite->row_pos - 1;
			c_check = player_sprite->col_pos;
		}
		else { // then => (player_sprite->status & (SOUTH | SOUTH_SW | SOUTH_SE)) == true
			r_check = player_sprite->row_pos + 1;
			c_check = player_sprite->col_pos;
		}

		//CheckTile(row_check, col_check, player_sprite->);
		return;
	}
}



// Updates the NPC sprites while in the 'explore' state
void MapMode::UpdateNPCExplore(MapSprite *npc) {

	if (npc->status & IN_MOTION) {
		npc->step_count += (float)time_elapsed / npc->step_speed;

		// Check whether we've reached a new tile
		if (npc->step_count >= npc->step_speed) {
			npc->step_count -= npc->step_speed;
			npc->status &= ~IN_MOTION;
			npc->status ^= STEP_SWAP; // This flips the step_swap bit

			if (npc->delay_time != 0) { // Stop the sprite for now and set a new wait time
				npc->wait_time = GaussianValue(npc->delay_time, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
				npc->step_count = 0;
			}
			else { // Keep the sprite moving
				SpriteMove(RandomNum(0,7), npc);
			}
		}
		return;
	}

	else { // Sprite is not in motion
		// Process either scripted movement or random movement
		if (npc->wait_time > 0) {
			npc->wait_time -= static_cast<int32>(time_elapsed); // Decrement the wait timer
		}

		else {
			npc->status |= IN_MOTION;
			SpriteMove(RandomNum(0,7), npc);
		}
	}
}


// Updates the game status when MapMode is in the 'dialogue' state
void MapMode::UpdateDialogueState() {
	cout << "TEMP: UpdateDialogueState()" << endl;
	static bool print_done; // User can only continue/exit dialogue when this is set to true

	// Handle other user input only if text printing is finished.
	if (print_done) {
		if (InputManager->ConfirmPress()) {
			//if (more_dialgoue) {
				// Send new dialogue to text renderer
			//}
			//else {
				map_state.push_back(EXPLORE);
			//}
			print_done = false; // Reset our print_done so that dialogue can be printed next time
		}
	}

	//UpdateNPCMovement(); // NPCs can still walk around while we are in dialogue mode
}



// Updates the game status when MapMode is in the 'script_event' state
void MapMode::UpdateScriptState() {
	cout << "TEMP: UpdateScriptState()" << endl;
}




// ****************************************************************************
// **************************** DRAW FUNCTIONS ********************************
// ****************************************************************************


// Determines things like our starting tiles
void MapMode::GetDrawInfo(MapFrame& mf) {
	// ************* (1) Calculate the default drawing positions for the tiles ****************
	// Draw from the top left corner
	mf.c_pos = (-SCREEN_COLS / 2.0f) - 0.5f;
	mf.r_pos = (SCREEN_ROWS / 2.0f) - 0.5f;

	// Set the default col and row tile counts
	mf.c_draw = static_cast<uint32>(SCREEN_COLS) + 1;
	mf.r_draw = static_cast<uint32>(SCREEN_ROWS) + 1;

	// These are the default starting positions
	mf.c_start = focused_object->col_pos - (static_cast<int32>(SCREEN_COLS) / 2);
	mf.r_start = focused_object->row_pos - (static_cast<int32>(SCREEN_ROWS) / 2);
	
// 	cout << ">>> RAWR (1) <<<" << endl;
// 	cout << -(SCREEN_COLS / -2.0f) - 0.5f << ", " << (SCREEN_ROWS / 2.0f) << endl;
// 	cout << "> Starting draw row: " << mf.r_start << endl;
// 	cout << "> Starting draw column: " << mf.c_start << endl;
// 	cout << "> Position draw row: " << mf.r_pos << endl;
// 	cout << "> Position draw column: " << mf.c_pos << endl;

	// *********************** (2) Calculate the drawing information **************************
	if (focused_object->status & IN_MOTION) {
		if (focused_object->step_count <= (focused_object->step_speed / 2.0f)) {
			// We are not more than half-way moving west, so make adjustments
			if (focused_object->status & (WEST | NORTH_NW | WEST_NW | SOUTH_SW | WEST_SW)) {
				mf.c_pos += focused_object->step_count / focused_object->step_speed;
				mf.c_start++;
			}
			// We are not more than half-way moving east, so make adjustments
			else if (focused_object->status & (EAST | NORTH_NE | EAST_NE | SOUTH_SE | EAST_SE)) {
				mf.c_pos -= focused_object->step_count / focused_object->step_speed;
				mf.c_start--;
			}

			// We are not more than half-way moving north, so make adjustments
			if (focused_object->status & (NORTH | WEST_NW | NORTH_NW | EAST_NE | NORTH_NE)) {
				mf.r_pos -= focused_object->step_count / focused_object->step_speed;
				mf.r_start++;
			}
			// We are not more than half-way moving south, so make adjustments
			else if (focused_object->status & (SOUTH | WEST_SW | SOUTH_SW | EAST_SE | SOUTH_SE)) {
				mf.r_pos += focused_object->step_count / focused_object->step_speed;
				mf.r_start--;
			}
		}

		// NOTE: Draw code should never see a step_count >= 32. Update() takes care of that
		else { // (focused_object->step_count > (TILE_STEPS / 2))
			// We are at least half-way moving west, so make adjustments
			if (focused_object->status & (WEST | NORTH_NW | WEST_NW | SOUTH_SW | WEST_SW)) {
				mf.c_pos -= (focused_object->step_speed - focused_object->step_count) / focused_object->step_speed;
			}
			// We are at least half-way moving east, so make adjustments
			else if (focused_object->status & (EAST | NORTH_NE | EAST_NE | SOUTH_SE | EAST_SE)) {
				mf.c_pos += (focused_object->step_speed - focused_object->step_count) / focused_object->step_speed;
			}

			// We are at least half-way moving north, so make adjustments
			if (focused_object->status & (NORTH | WEST_NW | NORTH_NW | EAST_NE | NORTH_NE)) {
				mf.r_pos += (focused_object->step_speed - focused_object->step_count) / focused_object->step_speed;
			}
			// We are at least half-way moving south, so make adjustments
			else if (focused_object->status & (SOUTH | WEST_SW | SOUTH_SW | EAST_SE | SOUTH_SE)) {
				mf.r_pos -= (focused_object->step_speed - focused_object->step_count) / focused_object->step_speed;
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
	else if (mf.c_start > col_count - static_cast<int32>(SCREEN_COLS) - 1) { 
		mf.c_start = col_count - static_cast<int32>(SCREEN_COLS);
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
	else if (mf.r_start > row_count - static_cast<int32>(SCREEN_ROWS) - 1) { 
		mf.r_start = row_count - static_cast<int32>(SCREEN_ROWS);
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

	GetDrawInfo(mf); // Get all the information we need for drawing this map frame

// 	cout << ">>> DEBUGGER <<<" << endl;
// 	cout << "> Starting draw row: " << mf.r_start << endl;
// 	cout << "> Starting draw column: " << mf.c_start << endl;
// 	printf("> Position row: %f\n", mf.r_pos);
// 	printf("> Position column: %f\n", mf.c_pos);
// 	
// 	SDL_Delay(200);
	
	// ************** (1) Draw the Lower Layer *************
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_NO_BLEND, 0);
	VideoManager->Move(mf.c_pos, mf.r_pos);
	for (uint32 r = static_cast<uint32>(mf.r_start); r < static_cast<uint32>(mf.r_start) + mf.r_draw; r++) {
		for (uint32 c = static_cast<uint32>(mf.c_start); c < static_cast<uint32>(mf.c_start) + mf.c_draw; c++) {
			if (map_layers[r][c].lower_layer >= 0) // Then a lower layer tile exists and we should draw it
				VideoManager->DrawImage(map_tiles[tile_frames[map_layers[r][c].lower_layer]->frame]);
			VideoManager->MoveRel(1.0f, 0.0f);
		}
		VideoManager->MoveRel(-static_cast<float>(mf.c_draw), -1.0f);

	}

	// ************** (2) Draw the Object Layer *************
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	for (uint32 i = 0; i < ground_objects.size(); i++) {
		(ground_objects[i])->Draw(mf);
	}

	// ************** (3) Draw the Upper Layer *************
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->Move(mf.c_pos, mf.r_pos);
	for (uint32 r = mf.r_start; r < mf.r_start + mf.r_draw; r++) {
		for (uint32 c = mf.c_start; c < mf.c_start + mf.c_draw; c++) {
			if (map_layers[r][c].upper_layer >= 0) // Then an upper layer tile exists and we should draw it
				VideoManager->DrawImage(map_tiles[tile_frames[map_layers[r][c].upper_layer]->frame]);
			VideoManager->MoveRel(1.0f, 0.0f);
		}
		VideoManager->MoveRel(-static_cast<float>(mf.c_draw), -1.0f);
	}

	// ************** (4) Draw the Dialoge box, if needed *************
	// if (map_state.back() = DIALOGUE) {
	// }
	return;
}


} // namespace hoa_map
