///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    map_actions.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Created: December 20th, 2005
 * \brief   Source file for map mode actions.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include "map.h"
#include "map_actions.h"
#include "map_objects.h"
#include "map_dialogue.h"
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

// ******************** ActionPathMove ***********************

void ActionPathMove::Load(uint32 table_key) {
	ReadDataDescriptor *read_data = &(sprite->current_map->_map_data);

	read_data->OpenTable(table_key);
	destination.row = read_data->ReadInt("row");
	destination.col = read_data->ReadInt("col");
	destination.altitude = read_data->ReadInt("alt");
	read_data->CloseTable();

	if (read_data->GetError() != DATA_NO_ERRORS) {
		if (MAP_DEBUG) cerr << "MAP ERROR: Failed to load data for an ActionPathMove object" << endl;
	}
}

void ActionPathMove::Process() {
	// Check if we already have a previously computed path and, if it is still valid, use it.
	if (path.empty()) {
		// Find a new path from scratch.
		TileNode start;
		start.row = sprite->row_position;
		start.col = sprite->col_position;
		start.altitude = sprite->altitude;
		start.f_score = 0;
		start.g_score = 0;
		start.h_score = 0;
		start.parent = NULL;
		path.push_back(start);
		sprite->current_map->_FindPath(destination, path);
		
		cout << ">>> FOUND PATH <<<" << endl;
		for (uint32 i = 0; i < path.size(); i++) {
			cout << "[" << path[i].col << ", " << path[i].row << "] ";
		}
		cout << endl;
	}
	
	if (sprite->row_position > path[current_node].row) {
		if (sprite->col_position > path[current_node].col) {
			sprite->Move(NORTHWEST);
		}
		else if (sprite->col_position < path[current_node].col) {
			sprite->Move(NORTHEAST);
		}
		else {
			sprite->Move(NORTH);
		}
	}
	else if (sprite->row_position < path[current_node].row) {
		if (sprite->col_position > path[current_node].col) {
			sprite->Move(SOUTHWEST);
		}
		else if (sprite->col_position < path[current_node].col) {
			sprite->Move(SOUTHEAST);
		}
		else {
			sprite->Move(SOUTH);
		}
	}
	else if (sprite->col_position < path[current_node].col) {
		sprite->Move(EAST);
	}
	else if (sprite->col_position > path[current_node].col) {
		sprite->Move(WEST);
	}
	// The else case should never happen. If it does, the sprite will stop moving
	
	// Check if move was successful and if so, update the current_node for the path
	if (sprite->status & IN_MOTION) {
		current_node++;
		// Check if this is the final node and if so, update the sprite's action index
		if (current_node >= path.size()) {
			current_node = 0;
			sprite->current_action = sprite->current_action + 1;
			if (sprite->current_action >= sprite->actions.size()) {
				sprite->current_action = 0;
			}
		}
	}
}



} // namespace private_map

} // namespace hoa_map
