///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_objects.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for map mode objects.
*** ***************************************************************************/

#include "utils.h"
#include "map.h"
#include "map_objects.h"
#include "map_dialogue.h"
#include "map_actions.h"
#include "audio.h"
#include "video.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;

namespace hoa_map {

namespace private_map {

// *****************************************************************************
// ************************ MapObject Class Functions **************************
// *****************************************************************************

MapObject::MapObject() {
	context = 0;
	object_id = -1;
	x_position = -1;
	y_position = -1;
	x_offset = 0.0f;
	y_offset = 0.0f;
	coll_half_width = 0.0f;
	coll_height = 0.0f;
}

// ****************************************************************************
// ************************ MapSprite Class Functions *************************
// ****************************************************************************

// Constructor for critical class members. Other members are initialized via support functions
MapSprite::MapSprite() {
	if (MAP_DEBUG)
		cout << "MAP: MapSprite constructor invoked" << endl;

	direction = SOUTH;
	movement_speed = NORMAL_SPEED;
	current_animation = ANIM_STANDING_SOUTH;
	face_portrait = NULL;
	updatable = true;
	visible = true;
}


// Free all allocated images and other structures
MapSprite::~MapSprite() {
	if (MAP_DEBUG)
		cout << "MAP: MapSprite destructor invoked" << endl;

	if (face_portrait != NULL) {
		VideoManager->DeleteImage(*face_portrait);
		face_portrait = NULL;
	}

	for (uint32 i = 0; i < animations.size(); i++) {
		VideoManager->DeleteImage(animations[i]);
	}
	animations.clear();

// 	for (uint32 i = 0; i < dialogues.size(); i++) {
// 		delete(dialogues[i]);
// 	}
}


// Load in the appropriate images and other data for the sprite
bool MapSprite::Load() {
	AnimatedImage img;
	uint32 frame_speed = static_cast<uint32>(movement_speed / 20.0f);

	// TEMP
	img.Clear();
	img.AddFrame(string("img/sprites/map/claudius_d0.png"), frame_speed);
	img.SetDimensions(2.0f, 4.0f);
	animations.push_back(img);

	img.Clear();
	img.AddFrame(string("img/sprites/map/claudius_u0.png"), frame_speed);
	img.SetDimensions(2.0f, 4.0f);
	animations.push_back(img);

	img.Clear();
	img.AddFrame(string("img/sprites/map/claudius_l0.png"), frame_speed);
	img.SetDimensions(2.0f, 4.0f);
	animations.push_back(img);

	img.Clear();
	img.AddFrame(string("img/sprites/map/claudius_r0.png"), frame_speed);
	img.SetDimensions(2.0f, 4.0f);
	animations.push_back(img);

	img.Clear();
	img.AddFrame(string("img/sprites/map/claudius_d1.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_d2.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_d3.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_d1.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_d4.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_d5.png"), frame_speed);
	img.SetDimensions(2.0f, 4.0f);
	animations.push_back(img);

	img.Clear();
	img.AddFrame(string("img/sprites/map/claudius_u1.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_u2.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_u3.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_u1.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_u4.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_u5.png"), frame_speed);
	img.SetDimensions(2.0f, 4.0f);
	animations.push_back(img);

	img.Clear();
	img.AddFrame(string("img/sprites/map/claudius_l1.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_l2.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_l3.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_l1.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_l4.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_l5.png"), frame_speed);
	img.SetDimensions(2.0f, 4.0f);
	animations.push_back(img);

	img.Clear();
	img.AddFrame(string("img/sprites/map/claudius_r1.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_r2.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_r3.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_r1.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_r4.png"), frame_speed);
	img.AddFrame(string("img/sprites/map/claudius_r5.png"), frame_speed);
	img.SetDimensions(2.0f, 4.0f);
	animations.push_back(img);

	for (uint32 i = 0; i < animations.size(); i++) {
		if (animations[i].Load() == false) {
			cerr << "MAP ERROR: failed to load sprite animation" << endl;
			return false;
		}
	}
	return true;
} // bool MapSprite::Load()


// Updates the state of the sprite
void MapSprite::Update() {
	if (updatable == false || animations.size() == 0) {
		return;
	}

	// Set the sprite's animation to the standing still position
	if (!moving & was_moving) {
		// Set the movement animation to zero progress
		animations[current_animation].SetTimeProgress(0);

		// Determine the correct standing frame to display
		if (direction & FACING_NORTH) {
			current_animation = ANIM_STANDING_NORTH;
		}
		else if (direction & FACING_SOUTH) {
			current_animation = ANIM_STANDING_SOUTH;
		}
		else if (direction & FACING_WEST) {
			current_animation = ANIM_STANDING_WEST;
		}
		else if (direction & FACING_EAST) {
			current_animation = ANIM_STANDING_EAST;
		}
		else {
			cerr << "MAP ERROR: could not find proper standing animation to draw" << endl;
		}
	} // if (!moving & was_moving)

	if (moving) {
		// Save the previous state
		uint8 last_animation = current_animation;
		float tmp_x = x_offset;
		float tmp_y = y_offset;
		float distance_moved = 2.0f * static_cast<float>(MapMode::_current_map->_time_elapsed) / movement_speed;

		// Move the sprite the appropriate distance in the appropriate direction
		switch (direction) {
			case NORTH:
				y_offset -= distance_moved;
				break;
			case SOUTH:
				y_offset += distance_moved;
				break;
			case WEST:
				x_offset -= distance_moved;
				break;
			case EAST:
				x_offset += distance_moved;
				break;
			case NW_NORTH:
			case NW_WEST:
				x_offset -= distance_moved;
				y_offset -= distance_moved;
				break;
			case SW_SOUTH:
			case SW_WEST:
				x_offset -= distance_moved;
				y_offset += distance_moved;
				break;
			case NE_NORTH:
			case NE_EAST:
				x_offset += distance_moved;
				y_offset -= distance_moved;
				break;
			case SE_SOUTH:
			case SE_EAST:
				x_offset += distance_moved;
				y_offset += distance_moved;
				break;
			default:
				cerr << "MAP ERROR: map sprite trying to move in an invalid direction" << endl;
		} // switch (direction)

		// Determine the correct animation to display
		if (direction & FACING_NORTH) {
			current_animation = ANIM_WALKING_NORTH;
		}
		else if (direction & FACING_SOUTH) {
			current_animation = ANIM_WALKING_SOUTH;
		}
		else if (direction & FACING_WEST) {
			current_animation = ANIM_WALKING_WEST;
		}
		else if (direction & FACING_EAST) {
			current_animation = ANIM_WALKING_EAST;
		}
		else {
			cerr << "MAP ERROR: could not find proper movement animation to draw" << endl;
		}

		// If the direction of movement changed in mid-flight, update the animation timer on the
		// new animated image to reflect the old, so the walking animations do not appear to 
		// "start and stop" whenever the direction is changed.
		if (current_animation != last_animation) {
			animations[current_animation].SetTimeProgress(animations[last_animation].GetTimeProgress());
			animations[last_animation].SetTimeProgress(0);
		}

		animations[current_animation].Update();

		// TODO: Call collision detection routine here

		// TEMP: Detect out-of-map boundaries
		float x_location = static_cast<float>(x_position) + x_offset;
		float y_location = static_cast<float>(y_position) + y_offset;

		if (x_location - coll_half_width < 0.0f) {
			x_offset = tmp_x;
		}
		else if (x_location + coll_half_width >= static_cast<float>(MapMode::_current_map->_num_tile_cols * 2)) {
			x_offset = tmp_x;
		}

		if (y_location + coll_height < 0.0f) {
			y_offset = tmp_y;
		}
		else if (y_location >= static_cast<float>(MapMode::_current_map->_num_tile_rows * 2)) {
			y_offset = tmp_y;
		}

		// Roll-over position offsets if necessary
		while (x_offset < 0.0f) {
			x_position -= 1;
			x_offset += 1.0f;
		}
		while (x_offset > 1.0f) {
			x_position += 1;
			x_offset -= 1.0f;
		}
		while (y_offset < 0.0f) {
			y_position -= 1;
			y_offset += 1.0f;
		}
		while (y_offset > 1.0f) {
			y_position += 1;
			y_offset -= 1.0f;
		}
	} // if (moving)

	was_moving = moving;
} // void MapSprite::Update()


// Draw the appropriate sprite frame on the correct position on the screen
void MapSprite::Draw() {
	if (visible == false)
		return;

	// Store the full x and y position coordinates of the sprite in a single pair of variables
	float x_pos = static_cast<float>(x_position) + x_offset;
	float y_pos = static_cast<float>(y_position) + y_offset;

	// ---------- (1) Determine if the sprite is off-screen and if so, don't draw it.

	if (x_pos + half_width < MapMode::_current_map->_draw_info.left_edge  ||
		x_pos - half_width > MapMode::_current_map->_draw_info.right_edge ||
		y_pos + height > MapMode::_current_map->_draw_info.bottom_edge    ||
		y_pos < MapMode::_current_map->_draw_info.top_edge) {
// 		if (x_pos + half_width < MapMode::_current_map->_draw_info.left_edge)
// 			printf("Too far left:  %f < %f\n", x_pos + half_width, MapMode::_current_map->_draw_info.left_edge);
// 		if (x_pos - half_width > MapMode::_current_map->_draw_info.right_edge)
// 			printf("Too far right: %f > %f\n", x_pos - half_width, MapMode::_current_map->_draw_info.right_edge);
// 		if (y_pos + height < MapMode::_current_map->_draw_info.bottom_edge)
// 			printf("Too far below: %f < %f\n", y_pos + height, MapMode::_current_map->_draw_info.bottom_edge);
// 		if (y_pos > MapMode::_current_map->_draw_info.top_edge)
// 			printf("Too far above: %f > %f\n", y_pos, MapMode::_current_map->_draw_info.top_edge);
		return;
	}

	// ---------- (2) Calculate the drawing coordinates, move the drawing cursor, and draw the sprite

	VideoManager->Move(x_pos - MapMode::_current_map->_draw_info.left_edge, y_pos - MapMode::_current_map->_draw_info.top_edge);
	VideoManager->DrawImage(animations[current_animation]);
} // void MapSprite::Draw()

} // namespace private_map

} // namespace hoa_map
