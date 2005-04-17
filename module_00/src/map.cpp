/* 
 * map.cpp
 *	Code for Hero of Allacrost map mode
 *	(C) 2004 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */
 
/*
 * The code in this file is for handling the maps. This code is used whenever the player is walking around on
 *	a map (like a town or a dungeon). This includes handling tile images, sprite images, and events that
 *	occur on the map.
 *
 */
	
#include <iostream>
#include "map.h"
//#include "battle.h"
//#include "menu.h"

using namespace std;
using namespace hoa_map::local_map;
using namespace hoa_global;
using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_audio;
//using namespace hoa_battle;

namespace hoa_map {

// ***************** ObjectLayer Class Functions *******************

bool ObjectLayer::operator>(const ObjectLayer& obj) const {
	return (row_pos > obj.row_pos);
}

bool ObjectLayer::operator<(const ObjectLayer& obj) const {
	return (row_pos < obj.row_pos);
}

bool ObjectLayer::operator>=(const ObjectLayer& obj) const {
	return (row_pos >= obj.row_pos);
}

bool ObjectLayer::operator<=(const ObjectLayer& obj) const {
	return (row_pos <= obj.row_pos);
}

bool ObjectLayer::operator==(const ObjectLayer& obj) const {
	return (row_pos == obj.row_pos);
}

bool ObjectLayer::operator!=(const ObjectLayer& obj) const {
	return (row_pos != obj.row_pos);
}



// ******************* MapSprite Class Functions *********************

int MapSprite::FindFrame() {
	int draw_frame; // The frame index that we should draw
	
	// Depending on the direction the sprite is facing and the step_count, select the correct frame to draw
	switch (status & FACE_MASK) {
		case SOUTH:
		case SOUTH_SW:
		case SOUTH_SE:
			if (step_count < (0.25 * TILE_STEPS)) {
				draw_frame = DOWN_STANDING;
			}
			else if (step_count < (0.50 * TILE_STEPS)) {
				if (status & STEP_SWAP) 
					draw_frame = DOWN_RSTEP1;
				else 
					draw_frame = DOWN_LSTEP1;
			}
			else if (step_count < (0.75 * TILE_STEPS)) {
				if (status & STEP_SWAP) 
					draw_frame = DOWN_RSTEP2;
				else 
					draw_frame = DOWN_LSTEP2;
			}
			else { // (step_count < TILE_STEPS) == true
				if (status & STEP_SWAP) 
					draw_frame = DOWN_RSTEP3;
				else 
					draw_frame = DOWN_LSTEP3;
			}
			break;
		case NORTH:
		case NORTH_NW:
		case NORTH_NE:
			if (step_count < (0.25 * TILE_STEPS)) {
				draw_frame = UP_STANDING;
			}
			else if (step_count < (0.50 * TILE_STEPS)) {
				if (status & STEP_SWAP) 
					draw_frame = UP_RSTEP1;
				else 
					draw_frame = UP_LSTEP1;
			}
			else if (step_count < (0.75 * TILE_STEPS)) {
				if (status & STEP_SWAP) 
					draw_frame = UP_RSTEP2;
				else 
					draw_frame = UP_LSTEP2;
			}
			else { // (step_count < TILE_STEPS) == true
				if (status & STEP_SWAP) 
					draw_frame = UP_RSTEP3;
				else 
					draw_frame = UP_LSTEP3;
			}
			break;
		case WEST:
		case WEST_NW:
		case WEST_SW:
			if (step_count < (0.25 * TILE_STEPS)) {
				draw_frame = LEFT_STANDING;
			}
			else if (step_count < (0.50 * TILE_STEPS)) {
				if (status & STEP_SWAP) 
					draw_frame = LEFT_RSTEP1;
				else 
					draw_frame = LEFT_LSTEP1;
			}
			else if (step_count < (0.75 * TILE_STEPS)) {
				if (status & STEP_SWAP) 
					draw_frame = LEFT_RSTEP2;
				else 
					draw_frame = LEFT_LSTEP2;
			}
			else { // (step_count < TILE_STEPS) == true
				if (status & STEP_SWAP) 
					draw_frame = LEFT_RSTEP3;
				else 
					draw_frame = LEFT_LSTEP3;
			}
			break;
		case EAST:
		case EAST_NE:
		case EAST_SE:
			if (step_count < (0.25 * TILE_STEPS)) {
				draw_frame = RIGHT_STANDING;
			}
			else if (step_count < (0.50 * TILE_STEPS)) {
				if (status & STEP_SWAP) 
					draw_frame = RIGHT_RSTEP1;
				else 
					draw_frame = RIGHT_LSTEP1;
			}
			else if (step_count < (0.75 * TILE_STEPS)) {
				if (status & STEP_SWAP) 
					draw_frame = RIGHT_RSTEP2;
				else 
					draw_frame = RIGHT_LSTEP2;
			}
			else { // (step_count < TILE_STEPS) == true
				if (status & STEP_SWAP) 
					draw_frame = RIGHT_RSTEP3;
				else 
					draw_frame = RIGHT_LSTEP3;
			}
			break;
	}
	
	return draw_frame;
}



// Draw the appropriate sprite frame on the correct position on the screen
void MapSprite::Draw(MapFrame& mf) {
	float x_pos, y_pos; // The x and y cursor position to draw the sprite to 
	int draw_frame;     // The sprite frame index to draw
	
	// Set our default x and y position (true positions when sprite is not in motion)
	x_pos = mf.c_pos + (col_pos - mf.c_start);
	y_pos = mf.r_pos + (mf.r_start - row_pos);
	
	// When we are in motion, we have to off-set the step positions
	if (status & IN_MOTION) {
		switch (status & FACE_MASK) {
			case EAST:
				x_pos -= (float)(TILE_STEPS - step_count) / (float)TILE_STEPS;
				break;
			case WEST:
				x_pos += (float)(TILE_STEPS - step_count) / (float)TILE_STEPS;
				break;
			case NORTH:
				y_pos -= (float)(TILE_STEPS - step_count) / (float)TILE_STEPS;
				break;
			case SOUTH:
				y_pos += (float)(TILE_STEPS - step_count) / (float)TILE_STEPS;
				break;
			case NORTH_NW:
			case WEST_NW:
				x_pos += (float)(TILE_STEPS - step_count) / (float)TILE_STEPS;
				y_pos -= (float)(TILE_STEPS - step_count) / (float)TILE_STEPS;
				break;
			case SOUTH_SW:
			case WEST_SW:
				x_pos += (float)(TILE_STEPS - step_count) / (float)TILE_STEPS;
				y_pos += (float)(TILE_STEPS - step_count) / (float)TILE_STEPS;
				break;
			case NORTH_NE:
			case EAST_NE:
				x_pos -= (float)(TILE_STEPS - step_count) / (float)TILE_STEPS;
				y_pos -= (float)(TILE_STEPS - step_count) / (float)TILE_STEPS;
				break;
			case SOUTH_SE:
			case EAST_SE:
				x_pos -= (float)(TILE_STEPS - step_count) / (float)TILE_STEPS;
				y_pos += (float)(TILE_STEPS - step_count) / (float)TILE_STEPS;
				break;
		}
	}
	
	draw_frame = FindFrame();
	
	//cout << "Attempting to draw frame #" << draw_frame << " at: {" << x_pos << ", " << y_pos << '}' << endl;
	VideoManager->Move(x_pos, y_pos);
	VideoManager->DrawImage(frames[draw_frame]);
}



// ****************** PlayerSprite Class Functions *******************

// NOTE: This is all temporary code here
PlayerSprite::PlayerSprite() {
	cerr << "DEBUG: PlayerSprite's constructor invoked." << endl;
	object_type = PLAYER_SPRITE;
	row_pos = 16;
	col_pos = 12;
	step_count = 0;
	status = (VISIBLE | SOUTH);
	
	ImageDescriptor imd;
	imd.width = 1;
	imd.height = 2;
	
	imd.filename = "img/sprite/claudius_d1.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_d2.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_d3.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_d4.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_d5.png";
	frames.push_back(imd);
	
	imd.filename = "img/sprite/claudius_u1.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_u2.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_u3.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_u4.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_u5.png";
	frames.push_back(imd);
	
	imd.filename = "img/sprite/claudius_l1.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_l2.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_l3.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_l4.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_l5.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_l6.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_l7.png";
	frames.push_back(imd);
	
	imd.filename = "img/sprite/claudius_r1.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_r2.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_r3.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_r4.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_r5.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_r6.png";
	frames.push_back(imd);
	imd.filename = "img/sprite/claudius_r7.png";
	frames.push_back(imd);
	
	for (int i = 0; i < frames.size(); i++) {
		VideoManager->LoadImage(frames[i]);
	}
}



// The destructor frees all of the loaded images
PlayerSprite::~PlayerSprite() {
	cerr << "DEBUG: PlayerSprite's destructor invoked." << endl;
	for (int i = 0; i < frames.size(); i++) {
		VideoManager->DeleteImage(frames[i]);
	}
}


// ******************* MapMode Class Functions **********************

// ********************* GENERAL FUNCTIONS **************************

MapMode::MapMode(int new_map_id) {
	cerr << "DEBUG: MapMode's constructor invoked." << endl;
	
	AudioManager = GameAudio::_GetReference();
	VideoManager = GameVideo::_GetReference();
	//DataManager = GameData::_GetReference();
	ModeManager = GameModeManager::_GetReference();
	SettingsManager = GameSettings::_GetReference();
	
	mtype = map_m;
	input = &(SettingsManager->InputStatus);
	map_state = EXPLORE;
	map_id = new_map_id;
	
	// DataManger->LoadNewMap(map_id);
	// This is all temporary code until I get a function to call that loads this data
	
	random_encounters = true;
	encounter_rate = 12;
	steps_till_encounter = GaussianValue(encounter_rate, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	animation_rate = 200; // update frames every 0.2 seconds
	animation_counter = 0;
	
	tile_count = 16;	
	rows_count = 60;
	cols_count = 80;
	
	// Load in all tile images from memory
	ImageDescriptor imd;
	imd.width = 1;
	imd.height = 1;
	
	imd.filename = "img/tile/test_01.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_02.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_03.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_04.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_05.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_06.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_07.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_08.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_09.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_10.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_11.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_12.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_13.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_14.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_15.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_16.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_16a.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_16b.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_16d.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_16d.png";
	map_tiles.push_back(imd);
	imd.filename = "img/tile/test_16e.png";
	map_tiles.push_back(imd);
	
	for (int i = 0; i < map_tiles.size(); i++) { 
		VideoManager->LoadImage(map_tiles[i]);
	} 
	
	// Setup tile frame pointers for animation
	TileFrame *tf;
	for (int i = 0; i < tile_count - 1; i++) {
		tf = new TileFrame;
	  tf->frame = i;
		tf->next = tf;
		tile_frames.push_back(tf);
	}
	
	// Setup our final animated frame tile
	TileFrame *tmp;
	tf = new TileFrame;
	tf->frame = 15;
	tf->next = NULL;
	tile_frames.push_back(tf);
	
	tmp = new TileFrame;
	tf->next = tmp;
	tmp->frame = 16; // a
	tf = tmp;
	
	tmp = new TileFrame;
	tf->next = tmp;
	tmp->frame = 17; // b
	tf = tmp;
	
	tmp = new TileFrame;
	tf->next = tmp;
	tmp->frame = 18; // c
	tf = tmp;
	
	tmp = new TileFrame;
	tf->next = tmp;
	tmp->frame = 19; // d
	tf = tmp;
	
	tmp = new TileFrame;
	tf->next = tmp;
	tmp->frame = 20; // e
	tmp->next = tile_frames[15]; // Makes the linked list circular now
	
	
	
	// Setup our image map
	MapTile tmp_tile;
	tmp_tile.upper_layer = -1; // No upper layer in this test
	for (int r = 0; r < rows_count; r++) {
		map_layers.push_back(vector <MapTile>());
		for (int c = 0; c < cols_count; c++) {
			tmp_tile.lower_layer = (RandomNum(0, 16 - 1)); // Build our lower layer from random tiles
			if (tmp_tile.lower_layer == 15)
				tmp_tile.event_mask = NOT_WALKABLE; // We can not walk on the water tiles
			else
				tmp_tile.event_mask = 0x0000;
			map_layers[r].push_back(tmp_tile);
		}
	} 
	
	// Load player sprite and rest of map objects
	player_sprite = new PlayerSprite();
	object_layer.push_back(player_sprite);

	// The is temporary code for our screen shots. It handles the setting up of NPC sprites
	//ImageDescriptor imd;
	imd.height = 2;
	imd.width = 1;
	imd.filename = "img/sprite/boy_d1.png";

// 	NPCSprite *npc_sprite = new NPCSprite();
// 	npc_sprite->row_pos = 4;
// 	npc_sprite->col_pos = 6;
// 	npc_sprite->step_count = 0;
// 	npc_sprite->frame = imd;
// 	VideoManager->LoadImage(npc_sprite->frame);
// 	object_layer.push_back(npc_sprite);
// 	
// 	imd.filename = "img/sprite/girl_d1.png";
// 	npc_sprite = new NPCSprite();
// 	npc_sprite->row_pos = 8;
// 	npc_sprite->col_pos = 10;
// 	npc_sprite->step_count = 0;
// 	npc_sprite->frame = imd;
// 	VideoManager->LoadImage(npc_sprite->frame);
// 	object_layer.push_back(npc_sprite);
// 	
// 	imd.filename = "img/sprite/squire_d1.png";
// 	npc_sprite = new NPCSprite();
// 	npc_sprite->row_pos = 2;
// 	npc_sprite->col_pos = 20;
// 	npc_sprite->step_count = 0;
// 	npc_sprite->frame = imd;
// 	VideoManager->LoadImage(npc_sprite->frame);
// 	object_layer.push_back(npc_sprite);
// 	
// 	imd.filename = "img/sprite/mysticknight1_r3.png";
// 	npc_sprite = new NPCSprite();
// 	npc_sprite->row_pos = 14;
// 	npc_sprite->col_pos = 6;
// 	npc_sprite->step_count = 16;
// 	npc_sprite->frame = imd;
// 	VideoManager->LoadImage(npc_sprite->frame);
// 	object_layer.push_back(npc_sprite);
// 	
// 	imd.filename = "img/sprite/mysticknight2_l3.png";
// 		npc_sprite = new NPCSprite();
// 	npc_sprite->row_pos = 9;
// 	npc_sprite->col_pos = 16;
// 	npc_sprite->step_count = 16;
// 	npc_sprite->frame = imd;
// 	VideoManager->LoadImage(npc_sprite->frame);
// 	object_layer.push_back(npc_sprite);
// 	
// 	imd.filename = "img/sprite/aristocrat_l1.png";
// 		npc_sprite = new NPCSprite();
// 	npc_sprite->row_pos = 12;
// 	npc_sprite->col_pos = 12;
// 	npc_sprite->step_count = 0;
// 	npc_sprite->frame = imd;
// 	VideoManager->LoadImage(npc_sprite->frame);
// 	object_layer.push_back(npc_sprite);
// 	
// 	imd.filename = "img/sprite/laila_r1.png";
// 		npc_sprite = new NPCSprite();
// 	npc_sprite->row_pos = 15;
// 	npc_sprite->col_pos = 15;
// 	npc_sprite->step_count = 0;
// 	npc_sprite->frame = imd;
// 	VideoManager->LoadImage(npc_sprite->frame);
// 	object_layer.push_back(npc_sprite);

	
	// Setup our coordinate system
	VideoManager->SetCoordSys(-SCREEN_COLS/2, SCREEN_COLS/2, -SCREEN_ROWS/2, SCREEN_ROWS/2, 1);
}



MapMode::~MapMode() {
	cerr << "DEBUG: MapMode's destructor invoked." << endl;
	
	// Delete all of our tile images
	for (int i = 0; i < tile_count; i++) {
		VideoManager->DeleteImage(map_tiles[i]);
	}
	
	// Free up all our frame linked lists
	TileFrame *tf_del;
	TileFrame *tf_tmp;
	for (int i = 0; i < tile_frames.size(); i++) {
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
	
	// Delete all of our objects
	list<ObjectLayer*>::iterator obj_iter;
	for(obj_iter = object_layer.begin(); obj_iter != object_layer.end(); obj_iter++) {
		delete(*obj_iter);
	}
}



// Returns whether a sprite can move to a tile or not
inline bool MapMode::TileMoveable(int row, int col) {
	// First check that the player isn't trying to move outside the map boundaries
	if (row < 0 || col < 0 || row >= rows_count || col >= cols_count) {
		return false;
	}
	
	// Check if the tile is not walkable or occupied by another object
	if (map_layers[row][col].event_mask & (NOT_WALKABLE | OCCUPIED)) {
		return false;
	}
	
	cout << "Moving to new tile: {" << col << ',' << row << '}' << endl;
	return true;
}

// ************************ UPDATE FUNCTIONS ******************************

// Updates the game state when in map mode. Called from the main game loop.
void MapMode::Update(Uint32 time_elapsed) {
	int tzero;
	int tsize;
	animation_counter += time_elapsed;
	
	// Update our animation frames if needed
	if (animation_counter >= animation_rate) {
		// Update all tile frames
		for (int i = 0; i < tile_frames.size(); i++) {
			tile_frames[i] = tile_frames[i]->next;
		}
		
		// Update the frames for all dynamic map objects
//		 list<ObjectLayer*>::iterator obj_iter;
//		 for(obj_iter = object_layer.begin(); obj_iter != object_layer.end(); obj_iter++ ) {
//			 if (obj_iter->object_type == DYNAMIC_OBJECT) {
//				 obj_iter->frame_count = (obj_iter->frame_count + 1) % obj_iter->frames.size();
//			 }
//		 }

		animation_counter -= animation_rate;
	}
	
	switch (map_state) {
		case EXPLORE:
			UpdateExploreState(time_elapsed);
			break;
		case DIALOGUE:
			UpdateDialogueState();
			break;
		case SCRIPT_EVENT:
			UpdateScriptState();
			break;
	}	
}



// Updates the game status when MapMode is in the 'explore' state
void MapMode::UpdateExploreState(Uint32 time_elapsed) {
	int r_check, c_check; // Variables for holding the position of a tile to check.
	
	// ********** (1) Update sprite movement, if player sprite is in motion **********
	if (player_sprite->status & IN_MOTION) {
		//cout << "inc step_count: " << player_sprite->step_count << " + " << ((float)time_elapsed / (float)FAST_SPEED) << endl;
		player_sprite->step_count += (float)time_elapsed / (float)FAST_SPEED; // Modify the denomitor to change walking speed
		
		
		// Check whether we've reached a new tile
		if (player_sprite->step_count >= TILE_STEPS) { 
			//cout << "TEMP: Player sprite has finished moving to a new tile!" << endl;
			player_sprite->step_count -= TILE_STEPS;
			
			// IN_MOTION may get set again when UpdatePlayerMovement() called
			player_sprite->status = player_sprite->status & ~IN_MOTION; 
			// This flips the step_swap bit. Bit-wise XOR
			player_sprite->status = player_sprite->status ^ STEP_SWAP; 
			
			steps_till_encounter--;
			// If we moved diagonally, we need to decrease the steps_till_encounter by two, not one
			if (player_sprite->status & !(WEST | EAST | SOUTH | NORTH))
			  steps_till_encounter--;
			
			// Check if a random encounter has occured
			if (random_encounters && (steps_till_encounter <= 0)) {
				player_sprite->step_count = 0; // we need to be centered on a tile when we return from battle
				steps_till_encounter = GaussianValue(encounter_rate, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
				
				cout << "TEMP: Oh noes! A random encounter!!! Next encounter in: " << steps_till_encounter 
				<< " steps."<< endl;
				// Play a scary random encounter sound
				// Do a little cool script animation or something...
				
				// >>>Then once the script animation finishes, put this code at the end of the script<<<
				// Decide what enemies/enemy group to encounter here...?
				// BattleMode *BAM = new BattleMode(TEH_ENEMIEZ!!!);
				// ModeManager->Push(BAM);
				return;
			}
			
			UpdatePlayerMovement(); // This is called to see if the player wants to move to a new location
		}
		UpdateNPCMovement(time_elapsed);
		return;
	}
	
	// ********** If we arrive at the following code, the player is stopped on a tile **********
	
	// Handle menu press command
	if (input->menu_press) { // Push MenuMode onto the stack
// 		//MenuMode *MenuM = new MenuMode();
// 		//ModeManager->Push(MenuM);
		return;
	}
	
	// Handle confirm command.
	else if (input->confirm_press) {
		
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
		
		// Here check for treasure, sprite, etc interaction at postion [row_check, col_check]
		return;
	}
	
	// ******** (3) Handle any new movement commands from user and update NPCs ********
	
	UpdatePlayerMovement();
	UpdateNPCMovement(time_elapsed);
}



// Updates the game status when MapMode is in the 'dialogue' state
void MapMode::UpdateDialogueState() {
	cout << "TEMP: UpdateDialogueState()" << endl;
	static bool print_done; // User can only continue/exit dialogue when this is set to true
	
	// Handle other user input only if text printing is finished.
	if (print_done) {
		if (input->confirm_press) {
			//if (more_dialgoue) {
				// Send new dialogue to text renderer
			//}
			//else {
				map_state = EXPLORE;
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



// Updates the position and heading of the player sprite
void MapMode::UpdatePlayerMovement() {
	int new_r, new_c;       // New row and column position we may move to
	bool user_move = false; // Set to true if the user attempted to move the player sprite
	
	// Handle west, northwest, and southwest movement	
	if (input->left_state || input->left_press) {
		user_move = true;
		if (input->up_state || input->right_press) { // Moving northwest
			new_r = player_sprite->row_pos - 1;
			new_c = player_sprite->col_pos - 1;
			
			if (player_sprite->status & (NORTH_NW | NORTH | NORTH_NE | EAST_NE | EAST | EAST_SE))
				player_sprite->status = (player_sprite->status & RESET_FACE) | NORTH_NW;
			else
				player_sprite->status = (player_sprite->status & RESET_FACE) | WEST_NW; 
		}
		
		else if (input->down_state || input->down_press) { // Moving southwest
			new_r = player_sprite->row_pos + 1;
			new_c = player_sprite->col_pos - 1;
			
			if (player_sprite->status & (SOUTH_SW | SOUTH | SOUTH_SE | EAST_SE | EAST | EAST_NE))
				player_sprite->status = (player_sprite->status & RESET_FACE) | SOUTH_SW;
			else
				player_sprite->status = (player_sprite->status & RESET_FACE) | WEST_SW;	
		}
		
		else { // Moving west
			new_r = player_sprite->row_pos;
			new_c = player_sprite->col_pos - 1;
			player_sprite->status = (player_sprite->status & RESET_FACE) | WEST;
		}
	}
	
	// Handle east, northeast, and southeast movement
	else if (input->right_state || input->right_press) {
		user_move = true;
		if (input->up_state || input->up_press) { // Moving northeast
			new_r = player_sprite->row_pos - 1;
			new_c = player_sprite->col_pos + 1;
			
			if (player_sprite->status & (NORTH_NE | NORTH | NORTH_NW | WEST_NW | WEST | WEST_SW))
				player_sprite->status = (player_sprite->status & RESET_FACE) | NORTH_NE;
			else
				player_sprite->status = (player_sprite->status & RESET_FACE) | EAST_NE;
		}
		
		else if (input->down_state || input->down_press) { // Moving southeast
			new_r = player_sprite->row_pos + 1;
			new_c = player_sprite->col_pos + 1;
			
			if (player_sprite->status & (SOUTH_SE | SOUTH | SOUTH_SW | WEST_SW | WEST | WEST_NW))
				player_sprite->status = (player_sprite->status & RESET_FACE) | SOUTH_SE;
			else
				player_sprite->status = (player_sprite->status & RESET_FACE) | EAST_SE;		
		}
		
		else { // Moving east
			new_r = player_sprite->row_pos;
			new_c = player_sprite->col_pos + 1;
			player_sprite->status = (player_sprite->status & RESET_FACE) | EAST;
		}
	}
	
	// Handle north movement
	else if (input->up_state || input->up_press) {
		user_move = true;
		new_r = player_sprite->row_pos - 1;
		new_c = player_sprite->col_pos;
		player_sprite->status = (player_sprite->status & RESET_FACE) | NORTH;
	}
	
	// Handle south movement
	else if (input->down_state || input->down_press) {
		user_move = true;
		new_r = player_sprite->row_pos + 1;
		new_c = player_sprite->col_pos;
		player_sprite->status = (player_sprite->status & RESET_FACE) | SOUTH;
	}
	
	// Now check if we can actualy move to the tile the user requested to move to
	if (user_move) {
		if (TileMoveable(new_r, new_c)) {
			player_sprite->status = player_sprite->status | IN_MOTION;
			player_sprite->row_pos = new_r;
			player_sprite->col_pos = new_c;
		}
		else {
			player_sprite->status = player_sprite->status & ~IN_MOTION; // Sets the motion flag to false
			player_sprite->step_count = 0;      // Reset the step count
		}
	}
}



// Updates the position and heading of all NPC sprites
void MapMode::UpdateNPCMovement(Uint32 time_elapsed) {
// 	list<ObjectLayer*>::iterator obj_iter;
// 	for(obj_iter = object_layer.begin(); obj_iter != object_layer.end(); obj_iter++ ) {
// 		if ((*obj_iter)->object_type == NPC_SPRITE) {
// 			cout << "TEMP: Update a NPC sprite!" << endl;
// 		}
// 	}
// 	
// 	object_layer.sort();
}

// ********************* DRAWING FUNCTIONS **************************

// Determines things like our starting tiles
void MapMode::GetDrawInfo(MapFrame& mf) {
	// ************* (1) Calculate our default drawing positions for the tiles ****************
	
	// We draw from the top left corner
	mf.c_pos = -SCREEN_COLS / 2 - 0.5;
	mf.r_pos = SCREEN_ROWS / 2 - 0.5;
	
	// Set the default col and row tile count
	mf.c_draw = SCREEN_COLS + 1;
	mf.r_draw = SCREEN_ROWS + 1;
	
	// These are the default starting positions
	mf.c_start = player_sprite->col_pos - SCREEN_COLS / 2;
	mf.r_start = player_sprite->row_pos - SCREEN_ROWS / 2;
	
	// *********************** (2) Calculate our drawing information **************************
	
	if (player_sprite->status & IN_MOTION) {
		if (player_sprite->step_count <= (TILE_STEPS / 2)) {
			// We are not more than half-way moving west, so make adjustments
			if (player_sprite->status & (WEST | NORTH_NW | WEST_NW | SOUTH_SW | WEST_SW)) { 
				mf.c_pos += (float)player_sprite->step_count / (float)TILE_STEPS;
				mf.c_start++;
			}
			// We are not more than half-way moving east, so make adjustments
			else if (player_sprite->status & (EAST | NORTH_NE | EAST_NE | SOUTH_SE | EAST_SE)) {
				mf.c_pos -= (float)player_sprite->step_count / (float)TILE_STEPS;
				mf.c_start--;
			}
			
			// We are not more than half-way moving north, so make adjustments
			if (player_sprite->status & (NORTH | WEST_NW | NORTH_NW | EAST_NE | NORTH_NE)) { 
				mf.r_pos -= (float)player_sprite->step_count / (float)TILE_STEPS;
				mf.r_start++;
			}
			// We are not more than half-way moving south, so make adjustments
			else if (player_sprite->status & (SOUTH | WEST_SW | SOUTH_SW | EAST_SE | SOUTH_SE)) {
				mf.r_pos += (float)player_sprite->step_count / (float)TILE_STEPS;
				mf.r_start--;
			}
		}
		
		// NOTE: Draw code should never see a step_count >= 32. Update() takes care of that
		else { // (player_sprite->step_count > (TILE_STEPS / 2))
			// We are at least half-way moving west, so make adjustments
			if (player_sprite->status & (WEST | NORTH_NW | WEST_NW | SOUTH_SW | WEST_SW)) {
				mf.c_pos -= (float)(TILE_STEPS - player_sprite->step_count) / (float)TILE_STEPS;
			}
			// We are at least half-way moving east, so make adjustments
			else if (player_sprite->status & (EAST | NORTH_NE | EAST_NE | SOUTH_SE | EAST_SE)) {
				mf.c_pos += (float)(TILE_STEPS - player_sprite->step_count) / (float)TILE_STEPS;
			}
			
			// We are at least half-way moving north, so make adjustments
			if (player_sprite->status & (NORTH | WEST_NW | NORTH_NW | EAST_NE | NORTH_NE)) {
				mf.r_pos += (float)(TILE_STEPS - player_sprite->step_count) / (float)TILE_STEPS;
			}
			// We are at least half-way moving south, so make adjustments
			else if (player_sprite->status & (SOUTH | WEST_SW | SOUTH_SW | EAST_SE | SOUTH_SE)) {
				mf.r_pos -= (float)(TILE_STEPS - player_sprite->step_count) / (float)TILE_STEPS;
			}
		}
	}
	
	// *********************** (3) Check for special conditions **************************
	
	// Usually "the map moves around player", but when we encounter the edges of the map we 
	// need "the player to move around the map" 
	
	if (mf.c_start < 0) { // We exceed the far-left side of the map
		mf.c_start = 0;
		mf.c_pos = -SCREEN_COLS / 2;
	}
	else if (mf.c_start > cols_count - SCREEN_COLS - 1) { // We exceed the far-right side of the map
		mf.c_start = cols_count - SCREEN_COLS;
		mf.c_pos = -SCREEN_COLS / 2;
	}
	
	// If our column position is exactly on the left edge of the screen, we draw one less column of tiles
	if (mf.c_pos == -SCREEN_COLS / 2) {
		mf.c_draw--;
	}
	
	if (mf.r_start < 0) { // We exceed the far-north side of the map
		mf.r_start = 0;
		mf.r_pos = SCREEN_ROWS / 2 - 1;
	}
	else if (mf.r_start > rows_count - SCREEN_ROWS - 1) { // We exceed the far-south side of the map
		mf.r_start = rows_count - SCREEN_ROWS;
		mf.r_pos = SCREEN_ROWS / 2 - 1;
	}
	
	// If our row position is exactly on the top of the screen, we draw one less row of tiles
	if (mf.r_pos == SCREEN_ROWS / 2 - 1) {
		mf.r_draw--;
	}
	
	
}


// Public draw function called by the main game loop
void MapMode::Draw() { 
	MapFrame mf; // Contains all the information we need to know to draw the map
	
	GetDrawInfo(mf); // Get all the information we need for drawing this map frame
	
	// ************** (1) Draw the Lower Layer *************
	
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_NO_BLEND, 0);
	VideoManager->SelectLayer(1);
	VideoManager->Move(mf.c_pos, mf.r_pos);
	for (int r = mf.r_start; r < mf.r_start + mf.r_draw; r++) {
		for (int c = mf.c_start; c < mf.c_start + mf.c_draw; c++) {
			if (map_layers[r][c].lower_layer >= 0) // Then a lower layer tile exists and we should draw it
				VideoManager->DrawImage(map_tiles[tile_frames[map_layers[r][c].lower_layer]->frame]);
			VideoManager->MoveRel(1,0);
		}
		VideoManager->MoveRel(-mf.c_draw, -1);
		
	}
	
	// ************** (2) Draw the Object Layer *************
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	list<ObjectLayer*>::iterator obj_iter;
	for(obj_iter = object_layer.begin(); obj_iter != object_layer.end(); obj_iter++ ) {
		(*obj_iter)->Draw(mf);
	}
	 
	// ************** (3) Draw the Upper Layer *************

	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->SelectLayer(1);
	VideoManager->Move(mf.c_pos, mf.r_pos);
	for (int r = mf.r_start; r < mf.r_start + mf.r_draw; r++) {
		for (int c = mf.c_start; c < mf.c_start + mf.c_draw; c++) {
			if (map_layers[r][c].upper_layer >= 0) // Then an upper layer tile exists and we should draw it
				VideoManager->DrawImage(map_tiles[tile_frames[map_layers[r][c].upper_layer]->frame]);
			VideoManager->MoveRel(1,0);
		}
		VideoManager->MoveRel(-mf.c_draw, -1);
		
	}
	
	// ************** (4) Draw the Dialoge box, if needed *************
	// if (map_state = DIALOGUE) {
	// }
	return; 
}



int MapMode::GetTiles() {
	return tile_count;
}



int MapMode::GetRows() {
	return rows_count;
}



int MapMode::GetCols() {
	return cols_count;
}



std::vector<std::vector<MapTile> > MapMode::GetMapLayers() {
	return map_layers;
}



std::vector<hoa_video::ImageDescriptor> MapMode::GetMapTiles() {
	return map_tiles;
}



void MapMode::SetTiles(int num_tiles) {
	tile_count = num_tiles;
}



void MapMode::SetRows(int num_rows) {
	rows_count = num_rows;
}



void MapMode::SetCols(int num_cols) {
	cols_count = num_cols;
}



void MapMode::SetMapLayers(std::vector<std::vector<MapTile> > layers) {
	map_layers = layers;
}



void MapMode::SetMapTiles(std::vector<hoa_video::ImageDescriptor> tiles) {
	map_tiles = tiles;
} 


} // namespace hoa_map
