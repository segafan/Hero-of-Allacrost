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
*** \brief A display for managing and displaying dialogue on maps
*** 
*** The MapMode class creates an instance of this class to handle all dialogue
*** processing. This includes the visual display of dialogue as well as handling
*** user input and processing of any scripted sequences that should appear with
*** the dialogue.
*** ***************************************************************************/
class DialogueManager : public hoa_video::MenuWindow {
public:
	DialogueManager();

	~DialogueManager();

	//! \brief Updates the state of the conversation
	void Update();

	//! \brief Draws the dialogue window, text, portraits, and other related visuals to the screen
	void Draw();

	void SetCurrentDialogue(MapDialogue* dialogue)
		{ _current_dialogue = dialogue; }

	void ClearDialogue()
		{ _current_dialogue = NULL; }

	MapDialogue* GetCurrentDialogue() const
		{ return _current_dialogue; }

private:
	//! \brief A pointer to the current piece of dialogue that is active
	MapDialogue* _current_dialogue;

	//! \brief A background image used in map dialogue
	hoa_video::StillImage _background_image;

	//! \brief The nameplate image used along with the dialogue box image.
	hoa_video::StillImage _nameplate_image;

	//! \brief The textbox used for rendering the dialogue text
	hoa_video::TextBox _display_textbox;
}; // class DialogueManager : public hoa_video::MenuWindow


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
	MapDialogue(bool save_state = true);

	~MapDialogue();

	//! \brief This resets the counter that keeps track of how many times the dialogue has been seen.
	void ClearSeenDialogue()
		{ _seen = 0; }

	//! \brief Indicates if this dialogue has been seen by the player.
	bool IsSeenDialogue() const
		{ return _seen == 0 ? false : true; }

	//! \brief This increments the counter that keeps track of how many times the dialogue has been seen.
	void SetSeenDialogue()
		{ _seen++; }

	//! \brief This returns the number of times that this dialogue has been seen by the player.
	int32 GetSeenCount() const
		{ return _seen; }

	//! \brief Returns the maximum time in milliseconds that the current line of dialogue should be displayed.
	int32 GetLineTime() const
		{ return _time[_current_line]; }

	//! \brief Returns a bool that indicates whether a dialogue is blocked (ignores user input)
	bool IsBlocked() const
		{ return _blocked; }

	//! \brief This method controls if the dialogue should ignore user input (true) or not (false).
	void SetBlock(bool b)
		{ _blocked = b; }

	//! \brief Returns true if a dialogue should load the saved state of the dialogue speakers at the end of the dialogue.
	bool IsSaving() const
		{ return _save_state; }

	//! \brief This returns the number of line of the dialogue.
	uint32 GetNumLines() const
		{ return _speakers.size(); }

	//! \brief Returns the object ID of the speaker of the current line of dialogue.
	uint32 GetCurrentSpeaker() const
		{ return _speakers[_current_line]; }

	//! \brief Returns a reference to the unicode text string of the current line of dialogue.
	hoa_utils::ustring GetCurrentLine() const
		{ return _text[_current_line]; }

	//! \brief Returns a pointer to the ScriptObject that will be invoked after the current line of dialogue completes
	ScriptObject* GetCurrentAction()
		{ return _actions[_current_line]; }

	//! \brief Returns the object id of the speaker of a line.
	uint32 GetLineSpeaker(uint32 line) const
		{ if (line > _speakers.size()) return -1; else return _speakers[line]; }

	//! \brief Returns the text of a specific line.
	hoa_utils::ustring GetLineText(uint32 line) const
		{ if (line > _text.size()) return hoa_utils::ustring(); else return _text[line]; }

	//! \brief Returns the actions of a specific line.
	ScriptObject* GetLineAction(uint32 line)
		{ if (line > _actions.size()) return NULL; else return _actions[line]; }

	/** \brief This method adds a new line of text and optionally an action to the dialogue.
	*** \param text The text to show on the screen
	*** \param speaker_id The object ID of the speaker of this line of text (the ID should correspond to a VirtualSprite or dervied class)
	*** \param time The maximum time in milliseconds to show the line of dialogue. DIALOGUE_INFINITE (-1)
	*** means that it won't disappear unless the user gives an input.
	*** \param action An integer key to the map_functions table in the map file that contains the script function to execute when this line
	*** completes. A value less than zero indicates that no action is to occur.
	*** 
	*** \todo text should eventually take a ustring instead of a std::string, but we need better support for ustring in scripts to do that
	**/
	void AddText(std::string text, uint32 speaker_id, int32 time = DIALOGUE_INFINITE, int32 action = -1);

	/** \brief This method will update the current line of the dialogue.
	*** \return False if the dialogue is finished, true otherwise.
	**/
	bool ReadNextLine();

private:
	//! \brief The text of the conversation, split up into multiple lines.
	std::vector<hoa_utils::ustring> _text;

	//! \brief A list of object ID numbers that declare who speaks which lines.
	std::vector<uint32> _speakers;

	//! \brief A list of optional events that may occur after each line.
	std::vector<ScriptObject*> _actions;

	//! \brief The maximum time of each line in the dialogue.
	std::vector<int32> _time;

	//! \brief This counts the number of time a player has seen this dialogue.
	uint32 _seen;

	//! \brief An index to the current line to read.
	uint32 _current_line;

	//! \brief When this member is set to true, dialogues will ignore user input and instead execute independently.
	bool _blocked;

	//! \brief Declares whether or not to reset the status of map sprites after the dialogue completes.
	bool _save_state;
}; // class MapDialogue

} // namespace private_map

} // namespace hoa_map

#endif
