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
#include "audio.h"
#include "video.h"
#include "data.h"
//#include "battle.h"
//#include "menu.h"

using namespace std;
using namespace hoa_map::local_map;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_engine;
using namespace hoa_data;
//using namespace hoa_battle;
//using namespace hoa_menu;

namespace hoa_map {

// ****************************************************************************
// ************************ ObjectLayer Class Functions ***********************
// ****************************************************************************


ObjectLayer::ObjectLayer() {
	VideoManager = hoa_video::GameVideo::_GetReference();
}

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


// ****************************************************************************
// ************************ MapSprite Class Functions *************************
// ****************************************************************************


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
	
	// Set the default x and y position (true positions when sprite is not in motion)
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


// ****************************************************************************
// ************************ PlayerSprite Class Functions **********************
// ****************************************************************************


// NOTE: This is all temporary code here
PlayerSprite::PlayerSprite() {
	cerr << "DEBUG: PlayerSprite's constructor invoked." << endl;
	object_type = PLAYER_SPRITE;
	row_pos = 16;
	col_pos = 12;
	step_count = 0;
	step_speed = NORMAL_SPEED;
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


// ****************************************************************************
// ************************* NPCSprite Class Functions ************************
// ****************************************************************************

// NOTE: This is all temporary code here
NPCSprite::NPCSprite(string name) {
	cerr << "DEBUG: NPCSprite's constructor invoked." << endl;
	object_type = NPC_SPRITE;
	row_pos = 4;
	col_pos = 6;
	step_count = 0;
	step_speed = 40;
	status = (VISIBLE | SOUTH);
	DelayedMovement(250);
	
	string path_name = "img/sprite/" + name;
	
	ImageDescriptor imd;
	imd.width = 1;
	imd.height = 2;
	
	imd.filename = path_name + "_d1.png";
	frames.push_back(imd);
	imd.filename = path_name + "_d2.png";
	frames.push_back(imd);
	imd.filename = path_name + "_d3.png";
	frames.push_back(imd);
	imd.filename = path_name + "_d4.png";
	frames.push_back(imd);
	imd.filename = path_name + "_d5.png";
	frames.push_back(imd);
	
	imd.filename = path_name + "_u1.png";
	frames.push_back(imd);
	imd.filename = path_name + "_u2.png";
	frames.push_back(imd);
	imd.filename = path_name + "_u3.png";
	frames.push_back(imd);
	imd.filename = path_name + "_u4.png";
	frames.push_back(imd);
	imd.filename = path_name + "_u5.png";
	frames.push_back(imd);
	
	imd.filename = path_name + "_l1.png";
	frames.push_back(imd);
	imd.filename = path_name + "_l2.png";
	frames.push_back(imd);
	imd.filename = path_name + "_l3.png";
	frames.push_back(imd);
	imd.filename = path_name + "_l4.png";
	frames.push_back(imd);
	imd.filename = path_name + "_l5.png";
	frames.push_back(imd);
	imd.filename = path_name + "_l6.png";
	frames.push_back(imd);
	imd.filename = path_name + "_l7.png";
	frames.push_back(imd);
	
	imd.filename = path_name + "_r1.png";
	frames.push_back(imd);
	imd.filename = path_name + "_r2.png";
	frames.push_back(imd);
	imd.filename = path_name + "_r3.png";
	frames.push_back(imd);
	imd.filename = path_name + "_r4.png";
	frames.push_back(imd);
	imd.filename = path_name + "_r5.png";
	frames.push_back(imd);
	imd.filename = path_name + "_r6.png";
	frames.push_back(imd);
	imd.filename = path_name + "_r7.png";
	frames.push_back(imd);
	
	for (int i = 0; i < frames.size(); i++) {
		VideoManager->LoadImage(frames[i]);
	}
}


// The destructor frees all of the loaded images
NPCSprite::~NPCSprite() {
	cerr << "DEBUG: NPCSprite's destructor invoked." << endl;
	for (int i = 0; i < frames.size(); i++) {
		VideoManager->DeleteImage(frames[i]);
	}
}

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
	
	tile_count = 16;	
	row_count = 60;
	col_count = 80;
	
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
	for (int r = 0; r < row_count; r++) {
		map_layers.push_back(vector <MapTile>());
		for (int c = 0; c < col_count; c++) {
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
	
	NPCSprite *npc_sprite = new NPCSprite("laila");
	object_layer.push_back(npc_sprite);
}



MapMode::MapMode(int new_map_id) {
	cerr << "DEBUG: MapMode's constructor invoked." << endl;
	
	mtype = map_m;
	map_state = EXPLORE;
	map_id = new_map_id;
	
	// Load the map from the Lua data file
	DataManager->LoadMap(this, map_id);
	
	// Temporary function that creates a random map
	//TempCreateMap();
	
	// Setup the coordinate system
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
	// First check that the object in question isn't trying to move outside the map boundaries
	if (row < 1 || col < 0 || row >= row_count || col >= col_count) {
		return false;
	}
	
	// Check if the tile is not walkable or occupied by another object
	if (map_layers[row][col].event_mask & (NOT_WALKABLE | OCCUPIED)) {
		return false;
	}
	
	//cout << "Moving to new tile: {" << col << ',' << row << '}' << endl;
	return true;
}



// Simple functions for the MapEditor code
std::vector<std::vector<MapTile> > MapMode::GetMapLayers() { return map_layers; }
std::vector<hoa_video::ImageDescriptor> MapMode::GetMapTiles() { return map_tiles; }
void MapMode::SetTiles(int num_tiles) { tile_count = num_tiles; }
void MapMode::SetRows(int num_rows) { row_count = num_rows; }
void MapMode::SetCols(int num_cols) { col_count = num_cols; }
void MapMode::SetMapLayers(std::vector<std::vector<MapTile> > layers) { map_layers = layers; }
void MapMode::SetMapTiles(std::vector<hoa_video::ImageDescriptor> tiles) { map_tiles = tiles; }
int MapMode::GetTiles() { return tile_count; }
int MapMode::GetRows() { return row_count; }
int MapMode::GetCols() { return col_count; }


// ****************************************************************************
// **************************** UPDATE FUNCTIONS ******************************
// ****************************************************************************


// Updates the game state when in map mode. Called from the main game loop.
void MapMode::Update(Uint32 new_time_elapsed) {
	time_elapsed = new_time_elapsed; 
	animation_counter += time_elapsed;
	
	// *********** (!) Update the tile animation frames if needed ***********
	
	if (animation_counter >= ANIMATION_RATE) {
		// Update all tile frames
		for (int i = 0; i < tile_frames.size(); i++) {
			tile_frames[i] = tile_frames[i]->next;
		}
		
		// Update the frames for all dynamic map objects....???
//		 list<ObjectLayer*>::iterator obj_iter;
//		 for(obj_iter = object_layer.begin(); obj_iter != object_layer.end(); obj_iter++ ) {
//			 if (obj_iter->object_type == DYNAMIC_OBJECT) {
//				 obj_iter->frame_count = (obj_iter->frame_count + 1) % obj_iter->frames.size();
//			 }
//		 }

		animation_counter -= ANIMATION_RATE;
	}
	
	// ***************** (2) Update the map based on what state we are in **************
	switch (map_state) {
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
	object_layer.sort();
}



// Updates the game status when MapMode is in the 'explore' state
void MapMode::UpdateExploreState() {
	UpdatePlayerExplore(); // Updates the player's sprite and processes user input
	
	// Update all game objects (??? Or only non-playable sprites ???)
	list<ObjectLayer*>::iterator obj_iter;
	for(obj_iter = object_layer.begin(); obj_iter != object_layer.end(); obj_iter++ ) {
		switch ((*obj_iter)->object_type) {
			case PLAYER_SPRITE:
				//UpdatePlayerExplore(dynamic_cast<PlayerSprite*>(*obj_iter));
				break;
			case NPC_SPRITE:
				UpdateNPCExplore(dynamic_cast<NPCSprite*>(*obj_iter));
				break;
			default:
				cout << "Object not a sprite!" << endl;
				break;
		}
	}
}


// Updates the player sprite and processes user input while in the 'explore' state
void MapMode::UpdatePlayerExplore() {
	int r_check, c_check;   // Variables for holding the position of a tile to check.
	bool user_move = false; // Set to true if the user attempted to move the player sprite
	
	// *********** (!) Handle updates for the player sprite when in motion ************
	if (player_sprite->status & IN_MOTION) {
		player_sprite->step_count += (float)time_elapsed / (float)player_sprite->step_speed;
		
		// Check whether we've reached a new tile
		if (player_sprite->step_count >= TILE_STEPS) { 
			player_sprite->step_count -= TILE_STEPS;
			
			player_sprite->status &= ~IN_MOTION; // IN_MOTION may get set again shortly
			player_sprite->status ^= STEP_SWAP; // This flips the step_swap bit. Bit-wise XOR
			
			if (random_encounters) {
				steps_till_encounter--;
				if (player_sprite->status & !(WEST | EAST | SOUTH | NORTH))
					steps_till_encounter--; // Decrease count again if player moved diagonally
				
				if (random_encounters && (steps_till_encounter <= 0)) { // then a random encounter has occured
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
			}
		}
	}
	
	// ********* (2) Handle updates for the player sprite when not in motion **********
	if (!(player_sprite->status & IN_MOTION)) {
		
		// Handle west, northwest, and southwest movement	
		if (InputManager->LeftState() || InputManager->LeftPress()) {
			user_move = true;
			if (InputManager->UpState() || InputManager->RightPress()) { // Moving northwest
				r_check = player_sprite->row_pos - 1;
				c_check = player_sprite->col_pos - 1;
				
				if (player_sprite->status & (NORTH_NW | NORTH | NORTH_NE | EAST_NE | EAST | EAST_SE))
					player_sprite->status = (player_sprite->status & RESET_FACE) | NORTH_NW;
				else
					player_sprite->status = (player_sprite->status & RESET_FACE) | WEST_NW; 
			}
			
			else if (InputManager->DownState() || InputManager->DownPress()) { // Moving southwest
				r_check = player_sprite->row_pos + 1;
				c_check = player_sprite->col_pos - 1;
				
				if (player_sprite->status & (SOUTH_SW | SOUTH | SOUTH_SE | EAST_SE | EAST | EAST_NE))
					player_sprite->status = (player_sprite->status & RESET_FACE) | SOUTH_SW;
				else
					player_sprite->status = (player_sprite->status & RESET_FACE) | WEST_SW;	
			}
			
			else { // Moving west
				r_check = player_sprite->row_pos;
				c_check = player_sprite->col_pos - 1;
				player_sprite->status = (player_sprite->status & RESET_FACE) | WEST;
			}
		}
		
		// Handle east, northeast, and southeast movement
		else if (InputManager->RightState() || InputManager->RightPress()) {
			user_move = true;
			if (InputManager->UpState() || InputManager->UpPress()) { // Moving northeast
				r_check = player_sprite->row_pos - 1;
				c_check = player_sprite->col_pos + 1;
				
				if (player_sprite->status & (NORTH_NE | NORTH | NORTH_NW | WEST_NW | WEST | WEST_SW))
					player_sprite->status = (player_sprite->status & RESET_FACE) | NORTH_NE;
				else
					player_sprite->status = (player_sprite->status & RESET_FACE) | EAST_NE;
			}
			
			else if (InputManager->DownState() || InputManager->DownPress()) { // Moving southeast
				r_check = player_sprite->row_pos + 1;
				c_check = player_sprite->col_pos + 1;
				
				if (player_sprite->status & (SOUTH_SE | SOUTH | SOUTH_SW | WEST_SW | WEST | WEST_NW))
					player_sprite->status = (player_sprite->status & RESET_FACE) | SOUTH_SE;
				else
					player_sprite->status = (player_sprite->status & RESET_FACE) | EAST_SE;		
			}
			
			else { // Moving east
				r_check = player_sprite->row_pos;
				c_check = player_sprite->col_pos + 1;
				player_sprite->status = (player_sprite->status & RESET_FACE) | EAST;
			}
		}
		
		// Handle north movement
		else if (InputManager->UpState() || InputManager->UpPress()) {
			user_move = true;
			r_check = player_sprite->row_pos - 1;
			c_check = player_sprite->col_pos;
			player_sprite->status = (player_sprite->status & RESET_FACE) | NORTH;
		}
		
		// Handle south movement
		else if (InputManager->DownState() || InputManager->DownPress()) {
			user_move = true;
			r_check = player_sprite->row_pos + 1;
			c_check = player_sprite->col_pos;
			player_sprite->status = (player_sprite->status & RESET_FACE) | SOUTH;
		}
		
		// Now check if we can actualy move the sprite to the tile the user requested to move to
		if (user_move) {
			if (TileMoveable(r_check, c_check)) {
				player_sprite->status |= IN_MOTION; // Set motion flag
				map_layers[player_sprite->row_pos][player_sprite->col_pos].event_mask &= ~OCCUPIED;
				player_sprite->row_pos = r_check;
				player_sprite->col_pos = c_check;
				map_layers[player_sprite->row_pos][player_sprite->col_pos].event_mask |= OCCUPIED;
			}
			// Regardless of whether the move was successful or not, refuse to process additional commands
			//  from the user.
			return; 
		}
	}
		
	if (InputManager->MenuPress()) { // Push MenuMode onto the stack
		//MenuMode *MenuM = new MenuMode();
		//ModeManager->Push(MenuM);
		return;
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
		
		// TODO: Here check for treasure, sprite, etc interaction at postion [row_check, col_check]
		return;
	}
}



// Updates the NPC sprites while in the 'explore' state
void MapMode::UpdateNPCExplore(NPCSprite *npc) {
	if (!(npc->status & IN_MOTION)) {
		// Process either scripted movement or random movement
		if (npc->wait_time > 0) {
			npc->wait_time -= time_elapsed; // Decrement the wait timer
		}
		
		else {
			npc->status &= IN_MOTION;
			switch (RandomNum(0, 3)) {
				case 0:
					npc->status = (npc->status & RESET_FACE) | NORTH;
					if (TileMoveable(npc->row_pos - 1, npc->col_pos)) {
						npc->status |= IN_MOTION;
						map_layers[npc->row_pos][npc->col_pos].event_mask &= ~OCCUPIED;
						npc->row_pos = npc->row_pos - 1;
						map_layers[npc->row_pos][npc->col_pos].event_mask |= OCCUPIED;
					}
					break;
				case 1:
					npc->status = (npc->status & RESET_FACE) | SOUTH;
					if (TileMoveable(npc->row_pos + 1, npc->col_pos)) {
						npc->status |= IN_MOTION;
						map_layers[npc->row_pos][npc->col_pos].event_mask &= ~OCCUPIED;
						npc->row_pos = npc->row_pos + 1;
						map_layers[npc->row_pos][npc->col_pos].event_mask |= OCCUPIED;
					}
					break;
				case 2:
					npc->status = (npc->status & RESET_FACE) | WEST;
					if (TileMoveable(npc->row_pos, npc->col_pos - 1)) {
						npc->status |= IN_MOTION;
						map_layers[npc->row_pos][npc->col_pos].event_mask &= ~OCCUPIED;
						npc->col_pos = npc->col_pos - 1;
						map_layers[npc->row_pos][npc->col_pos].event_mask |= OCCUPIED;
					}
					break;
				case 3:
					npc->status = (npc->status & RESET_FACE) | EAST;
					if (TileMoveable(npc->row_pos, npc->col_pos + 1)) {
						npc->status |= IN_MOTION;
						map_layers[npc->row_pos][npc->col_pos].event_mask &= ~OCCUPIED;
						npc->col_pos = npc->col_pos + 1;
						map_layers[npc->row_pos][npc->col_pos].event_mask |= OCCUPIED;
					}
					break;
				default: cout << "wtf!?" << endl; 
					break;
			}
			
			if (npc->delay_time != 0) {
				npc->wait_time = GaussianValue(npc->delay_time, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
				cout << "Set new wait time!!! " << npc->wait_time << endl;
			}
			
			return;
		}
	}
	
	// Update sprites that are in motion
	npc->step_count += (float)time_elapsed / (float)npc->step_speed;
	
	// Check whether we've reached a new tile
	if (npc->step_count >= TILE_STEPS) { 
		npc->step_count = 0;
		//npc->step_count -= TILE_STEPS;
		
		npc->status &= ~IN_MOTION;
		npc->status ^= STEP_SWAP; // This flips the step_swap bit. Bit-wise XOR
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




// ****************************************************************************
// **************************** DRAW FUNCTIONS ********************************
// ****************************************************************************


// Determines things like our starting tiles
void MapMode::GetDrawInfo(MapFrame& mf) {
	// ************* (1) Calculate the default drawing positions for the tiles ****************
	
	// We draw from the top left corner
	mf.c_pos = -SCREEN_COLS / 2 - 0.5;
	mf.r_pos = SCREEN_ROWS / 2 - 0.5;
	
	// Set the default col and row tile count
	mf.c_draw = SCREEN_COLS + 1;
	mf.r_draw = SCREEN_ROWS + 1;
	
	// These are the default starting positions
	mf.c_start = player_sprite->col_pos - SCREEN_COLS / 2;
	mf.r_start = player_sprite->row_pos - SCREEN_ROWS / 2;
	
	// *********************** (2) Calculate the drawing information **************************
	
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
	
	// Usually the map "moves around the player", but when we encounter the edges of the map we 
	// need the player to "move around the map".
	
	if (mf.c_start < 0) { // We exceed the far-left side of the map
		mf.c_start = 0;
		mf.c_pos = -SCREEN_COLS / 2;
	}
	else if (mf.c_start > col_count - SCREEN_COLS - 1) { // We exceed the far-right side of the map
		mf.c_start = col_count - SCREEN_COLS;
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
	else if (mf.r_start > row_count - SCREEN_ROWS - 1) { // We exceed the far-south side of the map
		mf.r_start = row_count - SCREEN_ROWS;
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


} // namespace hoa_map
