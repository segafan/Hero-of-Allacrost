///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_dialogue.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for map mode dialogue.
*** ***************************************************************************/

#include "utils.h"
#include "map.h"
#include "map_objects.h"
#include "map_dialogue.h"
#include "audio.h"
#include "video.h"
#include "global.h"
#include "script.h"
//#include "battle.h"
#include "menu.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_global;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_menu;
using namespace luabind;

namespace hoa_map {

namespace private_map {

// ****************************************************************************
// *********************** MapDialogue Class Functions ************************
// ****************************************************************************

MapDialogue::MapDialogue( const bool save_state ) :
	_seen(0),
	_current_line(0),
	_blocked(false)
{
	if (MAP_DEBUG)
		cout << "MAP: MapDialogue constructor invoked" << endl;
	_save_state = save_state;
}

MapDialogue::~MapDialogue() {
	if (MAP_DEBUG)
		cout << "MAP: MapDialogue destructor invoked" << endl;

	for( uint32 i = 0; i < _actions.size(); ++i )
	{
		for( uint32 j = 0; j < _actions[i].size(); ++j )
		{
			delete _actions[i][j];
		}
	}
}

bool MapDialogue::ReadNextLine() {
	if ( ++_current_line >= _text.size() ) {
		_current_line = 0;
		SetSeenDialogue();
		return false;
	}
	return true;
}

void MapDialogue::AddText( const uint32 speaker_id, const hoa_utils::ustring text, const int32 time, SpriteAction* action )
{
	_speakers.push_back( speaker_id );
	_text.push_back( text );
	_time.push_back( time );
	_actions.resize( _actions.size() + 1 );
	_actions.back().push_back( action );
}

void MapDialogue::AddTextActions(const uint32 speaker_id, const hoa_utils::ustring text, const std::vector<SpriteAction*> & actions, const int32 time )
{
	_speakers.push_back( speaker_id );
	_text.push_back( text );
	_time.push_back( time );
	_actions.push_back( actions );
}

} // namespace private_map

} // namespace hoa_map
