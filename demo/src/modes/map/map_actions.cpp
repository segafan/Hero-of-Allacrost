///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_actions.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for map mode actions.
*** ***************************************************************************/

// Allacrost utilities
#include "utils.h"

// Allacrost engines
#include "system.h"

// Local map mode headers
#include "map.h"
#include "map_actions.h"
#include "map_dialogue.h"
#include "map_objects.h"
#include "map_sprites.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_system;

namespace hoa_map {

namespace private_map {

// *****************************************************************************
// ********** ActionPathMove
// *****************************************************************************

void ActionPathMove::SetDestination(int16 x, int16 y) {
	// TODO: should x and y be checked to make sure they are within the map boundaries?
	destination.col = x;
	destination.row = y;
	path.clear();
}


void ActionPathMove::Execute() {
	// TODO: Check if we already have a previously computed path and if it is still valid, use it.
	// The code below automatically re-uses a path if there is one without checking if the source
	// node is the same as the sprite's current position

	if (path.empty() == true) {
		MapMode::_current_map->_object_manager->FindPath(_sprite, path, destination);

		// If no path could be found, there's nothing more that can be done here
		if (path.empty() == true)
			return;
	}

	// TODO: the code below needs to be optimized. We should only be doing the directional
	// readjustment after the sprite has reached the next node

	_sprite->moving = true;
	if (_sprite->y_position > path[current_node].row) { // Need to move toward the north
		if (_sprite->x_position > path[current_node].col)
			_sprite->SetDirection(NORTHWEST);
		else if (_sprite->x_position < path[current_node].col)
			_sprite->SetDirection(NORTHEAST);
		else
			_sprite->SetDirection(NORTH);
	}
	else if (_sprite->y_position < path[current_node].row) { // Need to move toward the south
		if (_sprite->x_position > path[current_node].col)
			_sprite->SetDirection(SOUTHWEST);
		else if (_sprite->x_position < path[current_node].col)
			_sprite->SetDirection(SOUTHEAST);
		else
			_sprite->SetDirection(SOUTH);
	}
	else if (_sprite->x_position > path[current_node].col) { // Need to move west
		_sprite->SetDirection(WEST);
	}
	else if (_sprite->x_position < path[current_node].col) { // Need to move east
		_sprite->SetDirection(EAST);
	}
	else { // The x and y position have reached the node, update to the next node
		current_node++;
		if (current_node >= path.size()) { // Destination has been reached
			current_node = 0;
			// TODO: _finished is never set to false in this class...
			_finished = true;
			_sprite->moving = false;
		}
	}
} // void ActionPathMove::Execute()

// *****************************************************************************
// ********** ActionRandomMove
// *****************************************************************************

void ActionRandomMove::Execute() {
	_sprite->moving = true;
	direction_timer += SystemManager->GetUpdateTime();
	movement_timer += SystemManager->GetUpdateTime();

	// Check if we should change the sprite's direction
	if (direction_timer >= total_direction_time) {
		direction_timer -= total_direction_time;
		_sprite->SetRandomDirection();
	}

	if (movement_timer >= total_movement_time) {
		movement_timer = 0;
		// TODO: _finished is never set to false in this class...
		_finished = true;
		_sprite->moving = false;
	}
}

// *****************************************************************************
// ********** ActionAnimate
// *****************************************************************************

void ActionAnimate::Reset() {
	display_timer = 0;
	current_frame = 0;
	loop_count = 0;
	_finished = false;
}



void ActionAnimate::Execute() {
	display_timer += SystemManager->GetUpdateTime();

	if (display_timer > frame_times[current_frame]) {
		display_timer = 0;
		current_frame++;

		// Check if we are past the final frame to display in the loop
		if (current_frame >= frames.size()) {
			current_frame = 0;

			// If this animation is not infinitely looped, increment the loop counter
			if (number_loops >= 0) {
				loop_count++;
				if (loop_count > number_loops) {
					_finished = true;
					loop_count = 0;
					return;
				 }
			}
		}

		dynamic_cast<MapSprite*>(_sprite)->SetCurrentAnimation(static_cast<uint8>(frames[current_frame]));
	}

} // void ActionAnimate::Execute()

} // namespace private_map

} // namespace hoa_map
