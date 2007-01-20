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

	const bool IsSeenDialogue()
		{ return _seen; }

	void SetSeenDialogue()
		{ _seen = true; }

	void ClearSeenDialogue()
		{ _seen = false; }

	void AddText( const uint32 speaker_id, const hoa_utils::ustring text, SpriteAction* action = 0 );

	uint32 GetSpeaker() const
		{ return _speakers[ _current_line ]; }

	hoa_utils::ustring GetLine() const
		{ return _text[ _current_line ]; }

	SpriteAction* GetAction() const
		{ return _actions[ _current_line ]; }

	//TODO: These are unsafe, might have to add checks
	uint32 GetSpeaker( uint32 line ) const
		{ return _speakers[ line ]; }

	hoa_utils::ustring GetLine( uint32 line ) const
		{ return _text[ line ]; }

	SpriteAction* GetAction( uint32 line ) const
		{ return _actions[ line ]; }

	const bool ReadNextLine();

	const uint32 GetNumLines() const
		{ return _speakers.size(); }

private:
	//! \brief The text of the conversation, split up into multiple lines.
	std::vector<hoa_utils::ustring> _text;

	//! \brief A list of object ID numbers for who speaks what lines.
	std::vector<uint32> _speakers;

	// A list of events that may occur after each line.
	std::vector<SpriteAction*> _actions;

	//! \brief True if the player has already read this dialogue.
	bool _seen;

	//! \brief An index to the current line to read.
	uint32 _current_line;

	bool _blocked;

	uint32 _time_left;
}; // class MapDialogue

} // namespace private_map

} // namespace hoa_map

#endif
