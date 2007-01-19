///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
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

#include "utils.h"
#include <iostream>
#include "map.h"
#include "map_actions.h"
#include "map_objects.h"
#include "map_dialogue.h"

using namespace std;
using namespace hoa_utils;

namespace hoa_map {

namespace private_map {

// *****************************************************************************
// **************************** ActionPathMove *********************************
// *****************************************************************************

void ActionPathMove::Load(uint32 table_key) {
	// TODO
// 	ScriptDescriptor *read_data = &(sprite->current_map->_map_data);
//
// 	read_data->ReadOpenTable(table_key);
// 	destination.row = read_data->ReadInt("row");
// 	destination.col = read_data->ReadInt("col");
// 	read_data->ReadCloseTable();
//
// 	if (read_data->GetError() != SCRIPT_NO_ERRORS) {
// 		if (MAP_DEBUG) cerr << "MAP ERROR: Failed to load data for an ActionPathMove object" << endl;
// 	}

}



void ActionPathMove::Execute() {
	// TODO: Check if we already have a previously computed path and if it is still valid, use it.

	if (path.empty()) 
	{
		MapMode::_current_map->_FindPath(_sprite, path, destination);
	}

	if( !path.empty() && _count != 0 )
	{
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
				_count--;
				_sprite->moving = false;
			}
		}
	}
} // void ActionPathMove::Execute()

// *****************************************************************************
// ************************** ActionAnimate *******************************
// *****************************************************************************

// void ActionAnimate::Load(uint32 table_key) {
	// TODO
// 	//  FIXME: why is this a pointer???
// 	//	ScriptDescriptor *read_data; // Make this point to map data later
// 	ScriptDescriptor read_data;
//
// 	read_data.ReadOpenTable(table_key);
// 	display_time = read_data.ReadInt("display_time");
// 	frame_index = read_data.ReadInt("frame_index");
// 	read_data.ReadCloseTable();
//
// 	if (read_data.GetError() != SCRIPT_NO_ERRORS) {
// 		if (MAP_DEBUG) cerr << "MAP ERROR: Failed to load data for an ActionFrameDisplay object" << endl;
// 	}
// } // void ActionAnimate::Load(uint32 table_key)


// void ActionAnimate::Execute() {
	// TODO
// 	sprite->direction = frame_index;
// 	remaining_time -= SystemManager->GetUpdateTime();
// 	if (remaining_time <= 0) {
// 		remaining_time = display_time;
//
// 		sprite->current_action = sprite->current_action + 1;
// 		if (sprite->current_action >= sprite->actions.size()) {
// 			sprite->current_action = 0;
// 		}
// 	}
// } // void ActionAnimate::Execute()

} // namespace private_map

} // namespace hoa_map
