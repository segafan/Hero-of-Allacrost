///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    map_actions.cpp
 * \author  Tyler Olsen, roots@allacrost.org
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
#include "system.h"
#include "script.h"
#include "battle.h"
#include "menu.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_global;
using namespace hoa_system;
using namespace hoa_script;
using namespace hoa_battle;
using namespace hoa_menu;
using namespace luabind;

namespace hoa_map {

namespace private_map {

// ******************** ActionPathMove ***********************
void ActionPathMove::BindToLua()
{
	module(ScriptManager->GetGlobalState())
	[
		class_<ActionPathMove>("ActionPathMove")
		.def(constructor<>())
		.def("SetDestination", &ActionPathMove::SetDestination)
		.def("SetSprite", &ActionPathMove::SetSprite)
	];
}

void ActionPathMove::Load(uint32 table_key) {
	ScriptDescriptor *read_data = &(sprite->current_map->_map_data);

	read_data->OpenTable(table_key);
	destination.row = read_data->ReadInt("row");
	destination.col = read_data->ReadInt("col");
	read_data->CloseTable();

	if (read_data->GetError() != DATA_NO_ERRORS) {
		if (MAP_DEBUG) cerr << "MAP ERROR: Failed to load data for an ActionPathMove object" << endl;
	}
}

void ActionPathMove::Process() {
	// Check if we already have a previously computed path and,
	// if it is still valid, use it.
	if (path.empty()) {
		// Find a new path from scratch.
		TileNode start;
		start.row = sprite->row_position;
		start.col = sprite->col_position;
		start.f_score = 0;
		start.g_score = 0;
		start.h_score = 0;
		start.parent = NULL;
		path.push_back(start);
		sprite->current_map->_FindPath(destination, path, sprite);
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
	// Else case should never happen. If it does, the sprite will stop moving
	
	// Check if move was successful; if so update the current_node for the path
	if (sprite->status & IN_MOTION) {
		current_node++;
	}
	else {
		// Move was unsuccessful, meaning the sprite has been blocked most
		// likely by another sprite. So, clear the remaining, already
		// calculated, path to the destination, and recompute it. Then add it
		// back to the original path, creating a new way to get to the
		// destination from somewhere in the middle of the path.
		TileNode middle;
		middle.row = sprite->row_position;
		middle.col = sprite->col_position;
		middle.f_score = 0;
		middle.g_score = 0;
		middle.h_score = 0;
		middle.parent = NULL;
		vector<TileNode> new_path;
		new_path.push_back(middle);
		sprite->current_map->_FindPath(destination, new_path, sprite);
		path.erase(path.begin() + current_node, path.end());
		for (vector<TileNode>::iterator it = new_path.begin();
	    	 it != new_path.end(); it++)
			path.push_back(*it);
	}

	// Check if this is the final node; if so update sprite's action index
	if (current_node >= path.size()) {
		// Push the destination onto the end of the path if it had been
		// occupied and no longer is
		if (!MapObject::current_map->
			_tile_layers[destination.row][destination.col].occupied &&
			(sprite->row_position != destination.row ||
			 sprite->col_position != destination.col)) {
			path.push_back(destination);
		}
		else {
			path.clear(); // Results in recalculating the path every single time
			current_node = 0;
			sprite->current_action = sprite->current_action + 1;
			if (sprite->current_action >= sprite->actions.size()) {
				sprite->current_action = 0;
			}
		}
	}
} // void ActionPathMove::Process()


void ActionFrameDisplay::Load(uint32 table_key) {
	//  FIXME: why is this a pointer???
	//	ScriptDescriptor *read_data; // Make this point to map data later
	ScriptDescriptor read_data;

	read_data.OpenTable(table_key);
	display_time = read_data.ReadInt("display_time");
	frame_index = read_data.ReadInt("frame_index");
	read_data.CloseTable();

	if (read_data.GetError() != DATA_NO_ERRORS) {
		if (MAP_DEBUG) cerr << "MAP ERROR: Failed to load data for an ActionFrameDisplay object" << endl;
	}
} // void ActionFrameDisplay::Load(uint32 table_key)


void ActionFrameDisplay::Process() {

	sprite->direction = frame_index;
	remaining_time -= SystemManager->GetUpdateTime();
	if (remaining_time <= 0) {
		remaining_time = display_time;

		sprite->current_action = sprite->current_action + 1;
		if (sprite->current_action >= sprite->actions.size()) {
			sprite->current_action = 0;
		}
	}
} // void ActionFrameDisplay::Process()

} // namespace private_map

} // namespace hoa_map
