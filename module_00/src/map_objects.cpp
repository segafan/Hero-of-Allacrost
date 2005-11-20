///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    map_objects.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 20th, 2005
 * \brief   Source file for map mode objects.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include "map.h"
#include "map_objects.h"
#include "map_dialogue.h"
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

// Initialize static class members
MapMode* MapObject::CurrentMap = NULL;

// ****************************************************************************
// *********************** MapObject Class Functions ************************
// ****************************************************************************

// Initialize the members and setup the pointer to the GameVideo class
MapObject::MapObject(uint8 type, uint32 row, uint32 col, uint8 alt, uint16 stat) {
	_object_type = type;
	_row_pos = static_cast<int16>(row);
	_col_pos = static_cast<int16>(col);
	_altitude = alt;
	_status = stat;
}


// Destructor
MapObject::~MapObject() {}

// ****************************************************************************
// ************************ MapSprite Class Functions *************************
// ****************************************************************************

// Constructor for critical class members. Other members are initialized via support functions
MapSprite::MapSprite(uint8 type, uint32 row, uint32 col, uint8 alt, uint16 stat)
                     : MapObject(type, row, col, alt, stat) {
	if (MAP_DEBUG) cout << "MAP: MapSprite constructor invoked" << endl;
	_step_speed = NORMAL_SPEED;
	_step_count = 0;
	_delay_time = NORMAL_DELAY;
	_wait_time = 0;
	_name = "";
	_filename = "";
	_frames = NULL;
	_seen_all_dialogue = true;
}


// Free all the frames from memory
MapSprite::~MapSprite() {
	if (MAP_DEBUG) cout << "MAP: MapSprite destructor invoked" << endl;
	
	// Character sprite frames are kept globally, so don't delete them.
	if (_object_type == PLAYER_SPRITE) { 
		return;
	}

	for (uint32 i = 0; i < _frames->size(); i++) {
		VideoManager->DeleteImage((*_frames)[i]);
	}
	delete _frames;
}


// Load the appropriate number of image frames for the sprite
void MapSprite::LoadFrames() {
	StillImage imd;

	// Prepare standard sprite animation frames (24 count)
	_frames = new vector<StillImage>;
	imd.SetDimensions(1.0f, 2.0f);
	imd.SetFilename(_filename + "_d1.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_d2.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_d3.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_d4.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_d5.png");
	_frames->push_back(imd);

	imd.SetFilename(_filename + "_u1.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_u2.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_u3.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_u4.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_u5.png");
	_frames->push_back(imd);

	imd.SetFilename(_filename + "_l1.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_l2.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_l3.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_l4.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_l5.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_l6.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_l7.png");
	_frames->push_back(imd);

	imd.SetFilename(_filename + "_r1.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_r2.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_r3.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_r4.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_r5.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_r6.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_r7.png");
	_frames->push_back(imd);

//	// Prepare additional extra frames if the sprite is not a regular NPC
//	if (_status & (ADV_SPRITE)) {
//
//	}

	// Load a

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < _frames->size(); i++) {
		VideoManager->LoadImage((*_frames)[i]);
	}
	VideoManager->EndImageLoadBatch();
}


// Loads the frames and other info from the GameInstance singleton
void MapSprite::LoadCharacterInfo(uint32 character) {
	GCharacter *pchar = GameInstance::GetReference()->GetCharacter(character);
	_name = pchar->GetName();
	_filename = "img/sprite/" + pchar->GetFilename();
	_frames = pchar->GetMapFrames();
}

void MapSprite::SaveState() {
	_saved_status = _status;
	_saved_frame = _frame;
}

void MapSprite::RestoreState() {
	_status = _saved_status;
	_frame = _saved_frame;
}


void MapSprite::AddDialogue(std::vector<std::string> new_dia) {
	MapDialogue new_dialogue;
	new_dialogue.SetLines(new_dia);
	_dialogues.push_back(new_dialogue);
	_seen_all_dialogue = false;
}


// Find the frame image that should be drawn
void MapSprite::_FindFrame() {
	// Depending on the direction the sprite is facing and the step_count, select the correct frame to draw
	switch (_direction) {
		case SOUTH:
		case SOUTH_SW:
		case SOUTH_SE:
			if (_step_count < (0.25 * _step_speed)) {
				_frame = DOWN_STANDING;
				return;
			}
			else if (_step_count < (0.50 * _step_speed)) {
				if (_status & STEP_SWAP)
					_frame = DOWN_RSTEP1;
				else
					_frame = DOWN_LSTEP1;
				return;
			}
			else if (_step_count < (0.75 * _step_speed)) {
				if (_status & STEP_SWAP) 
					_frame = DOWN_RSTEP2;
				else 
					_frame = DOWN_LSTEP2;
				return;
			}
			else { // (_step_count < _step_speed) == true
				if (_status & STEP_SWAP) 
					_frame = DOWN_RSTEP3;
				else 
					_frame = DOWN_LSTEP3;
				return;
			}
			break;
		case NORTH:
		case NORTH_NW:
		case NORTH_NE:
			if (_step_count < (0.25 * _step_speed)) {
				_frame = UP_STANDING;
				return;
			}
			else if (_step_count < (0.50 * _step_speed)) {
				if (_status & STEP_SWAP)
					_frame = UP_RSTEP1;
				else
					_frame = UP_LSTEP1;
				return;
			}
			else if (_step_count < (0.75 * _step_speed)) {
				if (_status & STEP_SWAP)
					_frame = UP_RSTEP2;
				else
					_frame = UP_LSTEP2;
				return;
			}
			else { // (_step_count < _step_speed) == true
				if (_status & STEP_SWAP) 
					_frame = UP_RSTEP3;
				else 
					_frame = UP_LSTEP3;
				return;
			}
			break;
		case WEST:
		case WEST_NW:
		case WEST_SW:
			if (_step_count < (0.25 * _step_speed)) {
				_frame = LEFT_STANDING;
				return;
			}
			else if (_step_count < (0.50 * _step_speed)) {
				if (_status & STEP_SWAP)
					_frame = LEFT_RSTEP1;
				else
					_frame = LEFT_LSTEP1;
				return;
			}
			else if (_step_count < (0.75 * _step_speed)) {
				if (_status & STEP_SWAP)
					_frame = LEFT_RSTEP2;
				else
					_frame = LEFT_LSTEP2;
				return;
			}
			else { // (step_count < step_speed) == true
				if (_status & STEP_SWAP)
					_frame = LEFT_RSTEP3;
				else
					_frame = LEFT_LSTEP3;
				return;
			}
			break;
		case EAST:
		case EAST_NE:
		case EAST_SE:
			if (_step_count < (0.25 * _step_speed)) {
				_frame = RIGHT_STANDING;
				return;
			}
			else if (_step_count < (0.50 * _step_speed)) {
				if (_status & STEP_SWAP)
					_frame = RIGHT_RSTEP1;
				else
					_frame = RIGHT_LSTEP1;
				return;
			}
			else if (_step_count < (0.75 * _step_speed)) {
				if (_status & STEP_SWAP)
					_frame = RIGHT_RSTEP2;
				else
					_frame = RIGHT_LSTEP2;
				return;
			}
			else { // (_step_count < _step_speed) == true
				if (_status & STEP_SWAP)
					_frame = RIGHT_RSTEP3;
				else
					_frame = RIGHT_LSTEP3;
				return;
			}
			break;
		default:
			cerr << "MAP: ERROR: Sprite direction was not set in call to MapSprite::_FindFrame()" << endl;
			_frame = 0; // To avoid any seg faults from occuring.
			break;
	}
	
	return;
} // MapSprite::_FindFrame()


uint16 MapSprite::_RandomDirection() {
	int32 random_val;
	
	random_val = RandomNumber(0, 7);
	
	switch (random_val) {
		case 0:
			return NORTH;
		case 1:
			return SOUTH;
		case 2:
			return WEST;
		case 3:
			return EAST;
		case 4:
			return NORTH_NW;
		case 5:
			return SOUTH_SW;
		case 6:
			return NORTH_NE;
		case 7:
			return SOUTH_SE;
		default:
			if (MAP_DEBUG) cerr << "MAP: WARNING: RandomNumber() returned an out-of-range value" << endl;
			break;
	}
	
	return NORTH;
}

// Move a sprite across the ground, if possible.
void MapSprite::GroundMove(uint16 move_direction) {
	TileCheck tcheck;
	
	switch (move_direction) {
		case NORTH:
			tcheck.row = _row_pos - 1;
			tcheck.col = _col_pos;
			_direction = NORTH;
			break;
		case SOUTH:
			tcheck.row = _row_pos + 1;
			tcheck.col = _col_pos;
			_direction = SOUTH;
			break;
		case WEST:
			tcheck.row = _row_pos;
			tcheck.col = _col_pos - 1;
			_direction = WEST;
			break;
		case EAST:
			tcheck.row = _row_pos;
			tcheck.col = _col_pos + 1;
			_direction = EAST;
			break;
		case NORTH_NW:
		case WEST_NW:
			tcheck.row = _row_pos - 1;
			tcheck.col = _col_pos - 1;
			if (_direction & (NORTH_NW | NORTH | NORTH_NE | EAST_NE | EAST | EAST_SE))
				_direction = NORTH_NW;
			else
				_direction = WEST_NW;
			break;
		case SOUTH_SW:
		case WEST_SW:
			if (_direction & (SOUTH_SW | SOUTH | SOUTH_SE | EAST_SE | EAST | EAST_NE))
				_direction = SOUTH_SW;
			else
				_direction = WEST_SW;
			tcheck.row = _row_pos + 1;
			tcheck.col = _col_pos - 1;
			break;
		case NORTH_NE:
		case EAST_NE:
			if (_direction & (NORTH_NE | NORTH | NORTH_NW | WEST_NW | WEST | WEST_SW))
				_direction = NORTH_NE;
			else
				_direction = EAST_NE;
			tcheck.row = _row_pos - 1;
			tcheck.col = _col_pos + 1;
			break;
		case SOUTH_SE:
		case EAST_SE:
			if (_direction & (SOUTH_SE | SOUTH | SOUTH_SW | WEST_SW | WEST | WEST_NW))
				_direction = SOUTH_SE;
			else
				_direction = EAST_SE;
			tcheck.row = _row_pos + 1;
			tcheck.col = _col_pos + 1;
			break;
		default:
			if (MAP_DEBUG) cerr << "MAP: WARNING: MapSprite::GroundMove() called with invalid direction" << endl;
			return;
	}
	
	tcheck.direction = _direction;
	tcheck.altitude = _altitude;
	
	if (CurrentMap->_TileMoveable(tcheck)) {
		// ************************ Check For Tile Departure Event ***********************
		if (CurrentMap->_tile_layers[_row_pos][_col_pos].properties & DEPART_EVENT) {
			// Look-up and process the event associated with the tile.
			cout << "Tile had a departure event." << endl;
		}
		
		_status |= IN_MOTION; // Set the sprite's motion flag
		
		// For the tile the sprite is moving off of, clear the occupied bit.
		CurrentMap->_tile_layers[_row_pos][_col_pos].occupied &= ~_altitude;
		
		_row_pos = tcheck.row;
		_col_pos = tcheck.col;
		
		// Set the occuped bit for the tile the sprite is moving on to.
		CurrentMap->_tile_layers[_row_pos][_col_pos].occupied |= _altitude;
		
		// ************************ Check For Tile Arrival Event ***********************
		if (CurrentMap->_tile_layers[_row_pos][_col_pos].properties & ARRIVE_EVENT) {
			// Look-up and process the event associated with the tile.
			cout << "Tile has an arrival event." << endl;
		}
	}
	else {
		_status &= ~IN_MOTION;
		// TODO: Only modify the wait time if the sprite is a random mover
		_wait_time = GaussianValue(_delay_time, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	}
}

// Updates the status of the sprite
void MapSprite::Update() {
	if (!(_status & UPDATEABLE))
		return;
	
	if (_status & IN_MOTION) {
		_step_count += static_cast<float>(CurrentMap->_time_elapsed) / _step_speed;

		// Check whether we've reached a new tile
		if (_step_count >= _step_speed) {
			_step_count -= _step_speed;
			_status &= ~IN_MOTION;
			_status ^= STEP_SWAP; // This flips the step_swap bit

			if (_delay_time != 0) { // Stop the sprite for now and set a new wait time
				_wait_time = GaussianValue(_delay_time, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
				_step_count = 0;
			}
			else { // Keep the sprite moving
				GroundMove(_RandomDirection());
			}
		}
		return;
	}

	else { // Sprite is not in motion
		// Process either scripted movement or random movement
		if (_wait_time > 0) {
			_wait_time -= static_cast<int32>(CurrentMap->_time_elapsed); // Decrement the wait timer
		}

		else {
			_status |= IN_MOTION;
			GroundMove(_RandomDirection());
		}
	}
}

// Draw the appropriate sprite frame on the correct position on the screen
void MapSprite::Draw() {
	if (!(_status & VISIBLE))
		return;
	
	float x_pos = 0.0;  // The x and y cursor position to draw the sprite to
	float y_pos = 0.0;
	uint32 draw_frame = 0; // The sprite frame index to draw

	// Set the default x and y position (true positions when sprite is not in motion)
	x_pos = CurrentMap->_draw_info.c_pos + (static_cast<float>(_col_pos) - 
	        static_cast<float>(CurrentMap->_draw_info.c_start));
	y_pos = CurrentMap->_draw_info.r_pos + (static_cast<float>(CurrentMap->_draw_info.r_start) - 
	        static_cast<float>(_row_pos));

	// When the sprite is in motion, we have to off-set the step positions
	if (_status & IN_MOTION) {
		switch (_direction) {
			case EAST:
				x_pos -= (_step_speed - _step_count) / _step_speed;
				break;
			case WEST:
				x_pos += (_step_speed - _step_count) / _step_speed;
				break;
			case NORTH:
				y_pos -= (_step_speed - _step_count) / _step_speed;
				break;
			case SOUTH:
				y_pos += (_step_speed - _step_count) / _step_speed;
				break;
			case NORTH_NW:
			case WEST_NW:
				x_pos += (_step_speed - _step_count) / _step_speed;
				y_pos -= (_step_speed - _step_count) / _step_speed;
				break;
			case SOUTH_SW:
			case WEST_SW:
				x_pos += (_step_speed - _step_count) / _step_speed;
				y_pos += (_step_speed - _step_count) / _step_speed;
				break;
			case NORTH_NE:
			case EAST_NE:
				x_pos -= (_step_speed - _step_count) / _step_speed;
				y_pos -= (_step_speed - _step_count) / _step_speed;
				break;
			case SOUTH_SE:
			case EAST_SE:
				x_pos -= (_step_speed - _step_count) / _step_speed;
				y_pos += (_step_speed - _step_count) / _step_speed;
				break;
		}
	}
	
	VideoManager->Move(x_pos, y_pos);
	_FindFrame();
	VideoManager->DrawImage((*_frames)[_frame]);
}

} // namespace hoa_map
