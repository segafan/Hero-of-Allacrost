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
 * \date    Last Updated: August 20th, 2005
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
/*
void ActionPathMove::Load(uint32 table_key) {
	ReadDataDescriptor &read_data = _sprite->_current_map->_map_data;
	
	read_data->OpenTable(table_key);
	_destination.row = read_data->ReadInt(row);
	_destination.col = read_data->ReadInt(col;
	_destination.altitude = read_data->ReadInt(altitude);
	read_data->CloseTable();
	
	if (read_data->GetError() != DATA_NO_ERRORS) {
		if (MAP_DEBUG)
			cerr << "MAP ERROR: Failed to load data for an ActionPathMove object" << endl;
	}
}

void ActionPathMove::Process() {
	// Check if we already have a previously computed path and, if it is still valid, use it.
	if (!path.empty()) {
		
	}
	
	// Find a new path from scratch.
	
}*/



} // namespace private_map

} // namespace hoa_map
