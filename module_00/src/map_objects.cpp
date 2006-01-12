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
#include "map.h"
#include "map_objects.h"
#include "map_dialogue.h"
#include "map_actions.h"
#include "audio.h"
#include "video.h"
#include "global.h"
#include "data.h"
#include "battle.h"
#include "menu.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_engine;
using namespace hoa_global;
using namespace hoa_data;
using namespace hoa_battle;
using namespace hoa_menu;

namespace hoa_map {

namespace private_map {

// Initialize static class members
MapMode* MapObject::current_map = NULL;

// ****************************************************************************
// *********************** MapObject Class Functions ************************
// ****************************************************************************

MapObject::MapObject() {
	object_type = EMPTY_OBJECT;
	row_position = -1;
	col_position = -1;
	altitude = 0;
	status = 0;
}



MapObject::~MapObject() {}

// ****************************************************************************
// ************************ MapSprite Class Functions *************************
// ****************************************************************************

// Constructor for critical class members. Other members are initialized via support functions
MapSprite::MapSprite() {
	if (MAP_DEBUG) cout << "MAP: MapSprite constructor invoked" << endl;
	step_speed = NORMAL_SPEED;
	step_count = 0.0f;
	delay_time = NORMAL_DELAY;
	wait_time = 0;

}


// Free all the loaded sprite frames
MapSprite::~MapSprite() {
	if (MAP_DEBUG) cout << "MAP: MapSprite destructor invoked" << endl;

	for (uint32 i = 0; i < frames.size(); i++) {
		VideoManager->DeleteImage(frames[i]);
	}
}


// Load the appropriate number of image frames for the sprite
void MapSprite::LoadFrames() {
	StillImage imd;

	// Prepare standard sprite animation frames (24 count)
	imd.SetDimensions(1.0f, 2.0f);
	imd.SetFilename(filename + "_d0.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_d1.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_d2.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_d3.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_d4.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_d5.png");
	frames.push_back(imd);

	imd.SetFilename(filename + "_u0.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_u1.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_u2.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_u3.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_u4.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_u5.png");
	frames.push_back(imd);

	imd.SetFilename(filename + "_l0.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_l1.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_l2.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_l3.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_l4.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_l5.png");
	frames.push_back(imd);

	imd.SetFilename(filename + "_r0.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_r1.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_r2.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_r3.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_r4.png");
	frames.push_back(imd);
	imd.SetFilename(filename + "_r5.png");
	frames.push_back(imd);

	// TODO: Load any special frames specified for the sprite

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < frames.size(); i++) {
		VideoManager->LoadImage(frames[i]);
	}
	VideoManager->EndImageLoadBatch();
}



void MapSprite::SaveState() {
	saved_status = status;
	saved_frame = frame;
}



void MapSprite::RestoreState() {
	status = saved_status;
	frame = saved_frame;
}


void MapSprite::AddDialogue(std::vector<std::string> new_dia) {
	MapDialogue new_dialogue;
	//new_dialogue.SetLines(new_dia);
	//_dialogues.push_back(new_dialogue);
	seen_all_dialogue = false;
}


// Find the frame image that should be drawn
void MapSprite::FindFrame() {
	if (!(status & IN_MOTION)) {
		switch (direction) {
			case SOUTH:
			case SW_SOUTH:
			case SE_SOUTH:
				frame = DOWN_STANDING;
				break;
			case NORTH:
			case NW_NORTH:
			case NE_NORTH:
				frame = UP_STANDING;
				break;
			case WEST:
			case NW_WEST:
			case SW_WEST:
				frame = LEFT_STANDING;
				break;
			case EAST:
			case NE_EAST:
			case SE_EAST:
				frame = RIGHT_STANDING;
				break;
			default:
				frame = DOWN_STANDING;
				break;
		}
		return;
	}

	// Depending on the direction the sprite is facing and the step_count, select the correct frame to draw
	float progress = step_count / step_speed;
	switch (direction) {
		case SOUTH:
		case SW_SOUTH:
		case SE_SOUTH:
			if (progress < 0.33f) {
				frame = DOWN_NEUTRAL;
				return;
			}
			else if (progress < 0.66f) {
				if (status & STEP_SWAP)
					frame = DOWN_LSTEP1;
				else
					frame = DOWN_RSTEP1;
				return;
			}
			else { // (progress < 1.00f)
				if (status & STEP_SWAP)
					frame = DOWN_LSTEP2;
				else
					frame = DOWN_RSTEP2;
				return;
			}
			return;
		case NORTH:
		case NW_NORTH:
		case NE_NORTH:
			if (progress < 0.33f) {
				frame = UP_NEUTRAL;
				return;
			}
			else if (progress < 0.66f) {
				if (status & STEP_SWAP)
					frame = UP_LSTEP1;
				else
					frame = UP_RSTEP1;
				return;
			}
			else { // (progress < 1.00f)
				if (status & STEP_SWAP)
					frame = UP_LSTEP2;
				else
					frame = UP_RSTEP2;
				return;
			}
			return;
		case WEST:
		case NW_WEST:
		case SW_WEST:
			if (progress < 0.33f) {
				frame = LEFT_NEUTRAL;
				return;
			}
			else if (progress < 0.66f) {
				if (status & STEP_SWAP)
					frame = LEFT_LSTEP1;
				else
					frame = LEFT_RSTEP1;
				return;
			}
			else { // (progress < 1.00f)
				if (status & STEP_SWAP)
					frame = LEFT_LSTEP2;
				else
					frame = LEFT_RSTEP2;
				return;
			}
			return;
		case EAST:
		case NE_EAST:
		case SE_EAST:
			if (progress < 0.33f) {
				frame = RIGHT_NEUTRAL;
				return;
			}
			else if (progress < 0.66f) {
				if (status & STEP_SWAP)
					frame = RIGHT_LSTEP1;
				else
					frame = RIGHT_RSTEP1;
				return;
			}
			else { // (progress < 1.00f)
				if (status & STEP_SWAP)
					frame = RIGHT_LSTEP2;
				else
					frame = RIGHT_RSTEP2;
				return;
			}
			return;
		default:
			break;
	}

	cerr << "MAP ERROR: Sprite direction was not set in call to MapSprite::_FindFrame()" << endl;
	frame = DOWN_STANDING; // To avoid any seg faults from occuring.
	return;
} // MapSprite::_FindFrame()


// Move a sprite to a tile in the given direction, if possible.
void MapSprite::Move(uint16 move_direction) {
	TileCheck tcheck;

	switch (move_direction) {
		case NORTH:
			tcheck.row = row_position - 1;
			tcheck.col = col_position;
			direction = NORTH;
			break;
		case SOUTH:
			tcheck.row = row_position + 1;
			tcheck.col = col_position;
			direction = SOUTH;
			break;
		case WEST:
			tcheck.row = row_position;
			tcheck.col = col_position - 1;
			direction = WEST;
			break;
		case EAST:
			tcheck.row = row_position;
			tcheck.col = col_position + 1;
			direction = EAST;
			break;
		case NW_NORTH:
		case NW_WEST:
			tcheck.row = row_position - 1;
			tcheck.col = col_position - 1;
			if (direction & (NW_NORTH | NORTH | NE_NORTH | NE_EAST | EAST | SE_EAST))
				direction = NW_NORTH;
			else
				direction = NW_WEST;
			break;
		case SW_SOUTH:
		case SW_WEST:
			if (direction & (SW_SOUTH | SOUTH | SE_SOUTH | SE_EAST | EAST | NE_EAST))
				direction = SW_SOUTH;
			else
				direction = SW_WEST;
			tcheck.row = row_position + 1;
			tcheck.col = col_position - 1;
			break;
		case NE_NORTH:
		case NE_EAST:
			if (direction & (NE_NORTH | NORTH | NW_NORTH | NW_WEST | WEST | SW_WEST))
				direction = NE_NORTH;
			else
				direction = NE_EAST;
			tcheck.row = row_position - 1;
			tcheck.col = col_position + 1;
			break;
		case SE_SOUTH:
		case SE_EAST:
			if (direction & (SE_SOUTH | SOUTH | SW_SOUTH | SW_WEST | WEST | NW_WEST))
				direction = SE_SOUTH;
			else
				direction = SE_EAST;
			tcheck.row = row_position + 1;
			tcheck.col = col_position + 1;
			break;
		default:
			if (MAP_DEBUG) cerr << "MAP: WARNING: MapSprite::Move() called with invalid direction" << endl;
			return;
	}

	tcheck.altitude = altitude;
	tcheck.direction = direction;

	if (current_map->_TileMoveable(tcheck)) {
		// ************************ Check For Tile Departure Event ***********************
		if (current_map->_tile_layers[row_position][col_position].depart_event != 255) {
			// Look-up and process the event associated with the tile.
			cout << "Tile had a departure event." << endl;
		}

		status |= IN_MOTION; // Set the sprite's motion flag

		// For the tile the sprite is moving off of, clear off the occupied bit.
		current_map->_tile_layers[row_position][col_position].occupied &= ~altitude;

		// Change the new tile row and column coordinates of the sprite.
		row_position = tcheck.row;
		col_position = tcheck.col;

		// Set the occuped bit for the tile the sprite is moving on to.
		current_map->_tile_layers[row_position][col_position].occupied |= altitude;
	}
	else {
		status &= ~IN_MOTION;
	}
}

// Updates the status of the sprite
void MapSprite::Update() {
	if (status & IN_MOTION) {
		step_count += static_cast<float>(current_map->_time_elapsed);

		// Check whether we've reached a new tile and if so, update accordingly.
		if (step_count >= step_speed) {
			step_count -= step_speed;
			status &= ~IN_MOTION;
			status ^= STEP_SWAP; // This flips the step_swap bit

			// ************************ Check For Tile Arrival Event ***********************
			if (current_map->_tile_layers[row_position][col_position].arrive_event != 255) {
				// Look-up and process the event associated with the tile.
				cout << "Tile has an arrival event." << endl;
			}

			// Process the next action, which may be another tile move.
			if (!actions.empty()) {
				actions[current_action]->Process();
			}

			// If the sprite isn't going to move to another tile, reset the step offset
			if (!(status & IN_MOTION)) {
				step_count = 0.0f;
			}
		}
	}

	// Sprite is not in motion, process the sprite's action.
	else {
		if (!actions.empty()) {
			actions[current_action]->Process();
		}
	}
}

// Draw the appropriate sprite frame on the correct position on the screen
void MapSprite::Draw() {
	if (!(status & VISIBLE))
		return;

	float x_draw = 0.0;  // The x and y cursor position to draw the sprite to
	float y_draw = 0.0;
	uint32 draw_frame = 0; // The sprite frame index to draw


	// Find the x and y position (true positions when sprite is not in motion)
	x_draw = current_map->_draw_info.c_pos + (static_cast<float>(col_position) - current_map->_draw_info.c_start);
	y_draw = current_map->_draw_info.r_pos + (static_cast<float>(row_position) - current_map->_draw_info.r_start);

	// When the sprite is in motion, we have to off-set the step positions
	if (status & IN_MOTION) {
		float position_offset = step_count / step_speed - 1.0f;
		switch (direction) {
			case EAST:
				x_draw += position_offset;
				break;
			case WEST:
				x_draw -= position_offset;
				break;
			case NORTH:
				y_draw -= position_offset;
				break;
			case SOUTH:
				y_draw += position_offset;
				break;
			case NW_NORTH:
			case NW_WEST:
				x_draw -= position_offset;
				y_draw -= position_offset;
				break;
			case SW_SOUTH:
			case SW_WEST:
				x_draw -= position_offset;
				y_draw += position_offset;
				break;
			case NE_NORTH:
			case NE_EAST:
				x_draw += position_offset;
				y_draw -= position_offset;
				break;
			case SE_SOUTH:
			case SE_EAST:
				x_draw += position_offset;
				y_draw += position_offset;
				break;
		}
	}

		// TODO: Determine if the sprite is off-screen and if so, don't draw it.
	VideoManager->Move(x_draw, y_draw);
	FindFrame();
	VideoManager->DrawImage(frames[frame]);
}

} // namespace private_map

} // namespace hoa_map
