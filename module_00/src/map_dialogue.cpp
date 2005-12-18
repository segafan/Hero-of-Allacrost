///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    map_dialogue.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 20th, 2005
 * \brief   Source file for map mode dialogue.
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

// ****************************************************************************
// ********************** SpriteDialogue Class Functions **********************
// ****************************************************************************

SpriteDialogue::SpriteDialogue() {
	if (MAP_DEBUG) cout << "MAP: SpriteDialogue constructor invoked" << endl;
	_next_line = 0;
	// Don't set to false until we actually have some lines of dialogue
	_seen_all = true;
}



SpriteDialogue::~SpriteDialogue() {
	if (MAP_DEBUG) cout << "MAP: SpriteDialogue destructor invoked" << endl;
}



void SpriteDialogue::AddLine(hoa_utils::ustring txt) {
	SpriteText new_dialogue;
	//new_dialogue._text.push_back(txt);
	_lines.push_back(new_dialogue);
}

// ****************************************************************************
// *********************** MapDialogue Class Functions ************************
// ****************************************************************************

MapDialogue::MapDialogue() {
	if (MAP_DEBUG) cout << "MAP: MapDialogue constructor invoked" << endl;
}



MapDialogue::~MapDialogue() {
	if (MAP_DEBUG) cout << "MAP: MapDialogue destructor invoked" << endl;
}

} // namespace hoa_map
