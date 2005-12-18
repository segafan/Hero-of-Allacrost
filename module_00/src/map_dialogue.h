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

/*!****************************************************************************
 * \brief Container for the information necessary to process a single sprite conversation.
 *
 * NPC map sprites typically have multiple things to say to the player. This class represents
 * a single conversation of that sprite. 
 * 
 * \note This class has no functions because the only class that modifies and manages this
 * information is the SpriteDialogue class.
 * 
 * \note This class is under-going evolutionary stages and currently is far from developed. In
 * particular, this class is lacking the ability to process the following types of dialogues.
 * 
 * -# The ability to have multiple speakers in a dialogue, either between other NPCs or playable
 *    characters.
 * -# The ability to keep the sprite in motion during the dialogue.
 * -# The ability to have scripted, non-standard dialogue sequences. For example, displaying special
 *    sprite frames during the dialogue to illustrate emotion.
 *****************************************************************************/
class SpriteText {
	friend class SpriteDialogue;
private:
	//! The entire text for this conversation, split up into multiple lines.
	std::vector<hoa_utils::ustring> _text;
	//! A vector indicating whom is the speaker for a section of dialogue.
	uint32 _next_text;
	//! True if the player has already read this dialogue.
	bool _seen;
public:
	SpriteText() { _next_text = 0; _seen = false; }
	~SpriteText() {}
};

} // namespace private_map

/*!****************************************************************************
 * \brief Retains and manages all of a sprite's dialogue.
 *
 * This class manages the dialogue of 
 *
 * Dialogues in map mode are rather complex. We would like to have dialogues
 * between a character and an NPC, dialogues between multiple NPCs, etc. This
 * class is still in its infant stages and support for some of the more advanced
 * dialogue types has yet to be implemented.
 *****************************************************************************/
class SpriteDialogue {
	friend class MapMode;
	friend class MapSprite;
	friend class SpriteText;
private:
	//! All of the sprite's individual dialogues.
	std::vector<private_map::SpriteText> _lines;
	//! An index to the next set of lines to read.
	uint32 _next_line;
	//! True if the player has already read every dialogue from the sprite.
	bool _seen_all;
public:
	SpriteDialogue();
	~SpriteDialogue();
	
	//! Adds a simple new dialogue with \c only a single line of text.
	void AddLine(hoa_utils::ustring txt);
	
	//! \name Public Member Access Functions
	//! \brief Used for setting and retrieving the values of the various class members.
	//@{
	bool SeenAllDialogue() { return _seen_all; }
	void SetSeenAllDialogue() { _seen_all = true; }
	//@}
	
}; // class SpriteDialogue


/*!****************************************************************************
 * \brief A class for retaining and managing a scripted map dialogue.
 *
 * The MapDialogue class manages dialogues that take place from a scripted map
 * sequence. These dialogues do not "belong" to any one sprite and are almost  
 * always read only once by the player during a scripted sequence.
 * 
 * \note Obviously, this class has a lot of work to be done. I'm mostly waiting
 * on support from the DataManager side of things before I implement this class.
 *****************************************************************************/
class MapDialogue {
	friend class MapMode;
private:

public:
	MapDialogue();
	~MapDialogue();
}; // class MapDialogue

} // namespace hoa_map

#endif
