///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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

//! \brief This constant is used to indicate that a line of dialogue can stay an infinite time on the screen.
const int32 DIALOGUE_INFINITE = -1;

/** ****************************************************************************
*** \brief Retains and manages dialogues between characters on a map.
*** ***************************************************************************/
class MapDialogue {
public:
	/** \brief This is the default contructor.
	*** It can take a bool parameter to indicate weither or not the dialogue should
	*** reset the speakers to the state at which they were before the dialogue.
	*** This parameter is set to true ( reset ) by default.
	**/
	MapDialogue(const bool save_state = true);

	~MapDialogue();

	//! \brief Indicates if this dialogue has been seen by the player.
	bool IsSeenDialogue() const
		{ return _seen == 0 ? false : true; }

	//! \brief This increments the counter that keeps track of how many times the dialogue has been seen.
	void SetSeenDialogue()
		{ _seen++; }

	//! \brief This resets the counter that keeps track of how many times the dialogue has been seen.

	void ClearSeenDialogue()
		{ _seen = 0; }

	//! \brief This returns the number of times that this dialogue has been seen by the player.
	int32 GetSeenCount() const
		{ return _seen; }

	/** \brief This method adds a new line of text and actions to the dialogue.
	*** \param speaker_id The object ID of the speaker of the line of text ( must be a VirtualSprite )
	*** \param text The text to show on the screen
	*** \param actions A vector of SpriteActions to execute during this line of the dialogue.
	*** \param time The maximum time in milliseconds to show the line of dialogue. DIALOGUE_INFINITE (-1)
	*** means that it won't disappear unless the user gives an input.
	*** \note The actions received will be executed in order during the dialogue. If
	*** it is required to have multiple actors move at the same time, each action
	*** should have their force attribute set to false. If it is required to force
	*** the actions to finish before continuing the dialogue, the last action can have its
	*** force attribute set to true, but has to finish after the other actions.
	**/
	void AddTextActions(const uint32 speaker_id, const hoa_utils::ustring text,
		const std::vector<SpriteAction*> & actions, const int32 time = DIALOGUE_INFINITE);

	/** \brief This method adds a new line of text and action to the dialogue.
	*** \param speaker_id The object ID of the speaker of the line of text ( must be a VirtualSprite )
	*** \param text The text to show on the screen
	*** \param time The maximum time in milliseconds to show the line of dialogue. DIALOGUE_INFINITE (-1)
	*** means that it won't disappear unless the user gives an input.
	*** \param action A pointer to a SpriteAction that should be executed during this line of the dialogue.
	**/
	void AddText(const uint32 speaker_id, const hoa_utils::ustring text,
		const int32 time = DIALOGUE_INFINITE, SpriteAction* action = NULL);

	/** \brief This method will update the current line of the dialogue.
	*** \return false if the dialogue is over, true otherwise.
	**/
	bool ReadNextLine();

	//! \brief Returns the object ID of speaker of the current line of dialogue.
	uint32 GetSpeaker() const
		{ return _speakers[_current_line]; }

	//! \brief Returns the unicode string of the current line of dialogue.
	hoa_utils::ustring GetLine() const
		{ return _text[_current_line]; }

	//! \brief Returns the vector of SpriteAction* of the current line of dialogue.
	std::vector<SpriteAction*> & GetActions()
		{ return _actions[_current_line]; }

	//! \brief Returns the maximum time in milliseconds that the current line of dialogue should be displayed.
	int32 LineTime() const
		{ return _time[_current_line]; }

	/** \brief This method returns if a dialogue is blocked or not.
	*** A blocked dialogue cannot be skipped by the user. Each line
	*** of the dialogue will have to last the time specified.
	*** \return true if blocked, false otherwise.
	**/
	bool IsBlocked() const
		{ return _blocked; }

	//! \brief This method controls if the dialogue should ignore user input (true) or not (false).
	void SetBlock( const bool b )
		{ _blocked = b; }

	/** \brief This method returns if a dialogue should load the saved state
	*** of the dialogue speakers at the end of the dialogue.
	*** \return true if it will load the saved state, false otherwise.
	**/
	bool IsSaving() const
		{ return _save_state; }

	//! \brief This returns the number of line of the dialogue.
	uint32 GetNumLines() const
		{ return _speakers.size(); }

	//These are unsafe and might change in the future

	//! \brief returns the objec id of the speaker of a line.
	uint32 GetSpeaker(uint32 line) const
		{ return _speakers[line]; }

	//! \brief returns the text of a line.
	hoa_utils::ustring GetLine(uint32 line) const
		{ return _text[line]; }

	//! \brief returns the actions of a line.
	std::vector<SpriteAction*> & GetActions(uint32 line)
		{ return _actions[line]; }

private:
	//! \brief The text of the conversation, split up into multiple lines.
	std::vector<hoa_utils::ustring> _text;

	//! \brief A list of object ID numbers for who speaks what lines.
	std::vector<uint32> _speakers;

	//! \brief A list of events that may occur after each line.
	std::vector< std::vector<SpriteAction*> > _actions;

	//! \brief The maximum time of each line in the dialogue
	std::vector<int32> _time;

	//! \brief This counts the number of time a player has seen this dialogue.
	uint32 _seen;

	//! \brief An index to the current line to read.
	uint32 _current_line;

	//! \brief Contains the blocked status of the dialogue.
	bool _blocked;

	//! \brief Contains weither or not to reset the map sprites status
	bool _save_state;
}; // class MapDialogue

} // namespace private_map

} // namespace hoa_map

#endif
