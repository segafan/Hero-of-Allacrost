///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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

#include <iostream>

#include "utils.h"

#include "system.h"

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
// **************************** ActionPathMove *********************************
// *****************************************************************************

void ActionPathMove::Execute() {
	// TODO: Check if we already have a previously computed path and if it is still valid, use it.
	if (path.empty()) {
		MapMode::_current_map->_FindPath(_sprite, path, destination);
	}

	if (!path.empty()) {
		_sprite->moving = true;
		if (_sprite->y_position > path[current_node].row) { // Need to move north
			if (_sprite->x_position > path[current_node].col) { // Need to move northwest
				_sprite->SetDirection(NORTHWEST);
			}
			else {
				if (_sprite->x_position < path[current_node].col) { // Need to move northeast
					_sprite->SetDirection(NORTHEAST);
				}
				else { // Just move north
					_sprite->SetDirection(NORTH);
				}
			}
		}
		else if (_sprite->y_position < path[current_node].row) { // Need to move south
			if (_sprite->x_position > path[current_node].col) // Need to move southwest
				_sprite->SetDirection(SOUTHWEST);
			else if (_sprite->x_position < path[current_node].col) // Need to move southeast
				_sprite->SetDirection(SOUTHEAST);
			else // Just move south
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
				_finished = true;
				_sprite->moving = false;
			}
		}
	}
} // void ActionPathMove::Execute()

// *****************************************************************************
// **************************** ActionRandomMove *******************************
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
		_finished = true;
		_sprite->moving = false;
	}
}

// *****************************************************************************
// ***************************** ActionAnimate *********************************
// *****************************************************************************

void ActionAnimate::Execute() {
	timer += SystemManager->GetUpdateTime();

	if (timer > display_times[current_frame]) {
		timer = 0;
		current_frame++;

		// If we've finished displaying the final frame...
		if (current_frame >= frames.size()) {
			current_frame = 0;

			// If this animation is not infinitely looped, increment the loop counter
			if (loops >= 0) {
				loop_count++;
				if (loop_count > loops) {
					_finished = true;
					loop_count = 0;
					return;
				 }
			}
		}

		dynamic_cast<MapSprite*>(_sprite)->SetCurrentAnimation(frames[current_frame]);
	}

} // void ActionAnimate::Execute()

} // namespace private_map

} // namespace hoa_map
