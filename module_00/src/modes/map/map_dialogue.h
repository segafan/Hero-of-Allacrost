///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    map_dialogue.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \brief   Header file for map mode dialogue.
 *****************************************************************************/

#ifndef __MAP_DIALOGUE_HEADER__
#define __MAP_DIALOGUE_HEADER__

#include "utils.h"
#include "defs.h"
#include "video.h"

//! All calls to map mode are wrapped in this namespace.
namespace hoa_map {

//! An internal namespace to be used only within the boot code. Don't use this namespace anywhere else!
namespace private_map {

/*!****************************************************************************
 * \brief A class for retaining and managing map dialogues.
 *
 * This 
 * 
 * \note The text, speakers, and events vectors are all the same size.
 *
 *****************************************************************************/
class MapDialogue {
public:
	MapDialogue();
	~MapDialogue();

	//! The text of the conversation, split up into multiple lines.
	std::vector<hoa_utils::ustring> text;
	//! A list of sprite ID numbers for who speaks what lines.
	std::vector<uint32> speakers;
	// A list of events that may occur after each line.
	// std::vector<SpriteAction*> events

	//! True if the player has already read this dialogue.
	bool seen;
	//! An index to the current line to read.
	uint32 current_line;

	const bool IsSeenDialogue()
		{ return seen; }
	void SetSeenDialogue()
		{ seen = true; }
	void ClearSeenDialogue()
		{ seen = false; }
		
	const bool ReadNextLine();
}; // class MapDialogue



/*!****************************************************************************
 * \brief Retains and manages all of a sprite's dialogue.
 *
 * Dialogues in map mode are rather complex. We would like to have dialogues
 * between a character and an NPC, dialogues between multiple NPCs, etc. This
 * class is still in its infant stages and support for some of the more advanced
 * dialogue types has yet to be implemented.
 *****************************************************************************/
class SpriteDialogue : public MapDialogue {
public:
	//! The action that the sprite takes when it is spoken to by the player.
	//! \note The default action (NULL) is to turn and face the player.
	SpriteAction* speaking_action;
	
	SpriteDialogue();
	~SpriteDialogue();
}; // class SpriteDialogue

} // namespace private_map

} // namespace hoa_map

#endif
