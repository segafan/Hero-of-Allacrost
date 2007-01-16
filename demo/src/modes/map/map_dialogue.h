///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_dialogue.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for map mode dialogue.
*** ***************************************************************************/

#ifndef __MAP_DIALOGUE_HEADER__
#define __MAP_DIALOGUE_HEADER__

#include "utils.h"
#include "defs.h"
#include "video.h"

namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief Retains and manages dialogues between characters on a map.
*** ***************************************************************************/
class MapDialogue {
public:
	MapDialogue();

	~MapDialogue();

	//! \brief The text of the conversation, split up into multiple lines.
	std::vector<hoa_utils::ustring> text;

	//! \brief A list of sprite ID numbers for who speaks what lines.
	std::vector<uint32> speakers;

	// A list of events that may occur after each line.
	// std::vector<SpriteAction*> events

	//! \brief True if the player has already read this dialogue.
	bool seen;

	//! \brief An index to the current line to read.
	uint32 current_line;

	const bool IsSeenDialogue()
		{ return seen; }

	void SetSeenDialogue()
		{ seen = true; }

	void ClearSeenDialogue()
		{ seen = false; }

	const bool ReadNextLine();
}; // class MapDialogue

} // namespace private_map

} // namespace hoa_map

#endif
