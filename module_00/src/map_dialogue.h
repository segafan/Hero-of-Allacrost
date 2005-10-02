///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    map_dialogue.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 20th, 2005
 * \brief   Header file for map mode dialogue.
 *****************************************************************************/

#ifndef __MAP_DIALOGUE_HEADER__
#define __MAP_DIALOGUE_HEADER__

#include "utils.h"
#include <string>
#include <vector>
#include "defs.h"
#include "engine.h"
#include "video.h"

//! All calls to map mode are wrapped in this namespace.
namespace hoa_map {

//! An internal namespace to be used only within the boot code. Don't use this namespace anywhere else!
namespace private_map {

} // namespace private_map

/*!****************************************************************************
 * \brief A class for retaining and managing a sprite's dialogue.
 *
 * Dialogues in map mode are rather complex. We would like to have dialogues
 * between a character and an NPC, dialogues between multiple NPCs, etc. This
 * class is still in its infant stages and support for some of the more advanced
 * dialogue types has yet to be implemented.
 *****************************************************************************/
class MapDialogue {
private:
	//! An index to the next line to read.
	uint32 _next_line;
	//! The dialogue itself, broken into individual lines.
	std::vector<std::string> _lines;
	//! A vector indicating whom is the speaker for a section of dialogue.
	std::vector<uint32> _speakers;
	//! True if the player has already seen this particular dialogue
	bool _seen;

	friend class MapMode;
	friend class MapSprite;
public:
	MapDialogue();
	~MapDialogue();

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	void SetLines(std::vector<std::string> txt) { _lines = txt; _seen = false; }
	void AddLine(std::string txt) { _lines.push_back(txt); _seen = false; }
	bool SeenDialogue() { return _seen; }
	void ReadDialogue() { _seen = true; }
	//@}
	
}; // class MapDialogue

} // namespace hoa_map

#endif
