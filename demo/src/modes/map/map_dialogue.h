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

//! \brief This constant indicates the maximum amount of options a line of dialogue can have.
const uint32 MAX_OPTIONS = 5;

//! \brief This enum defines the different states the dialogue can be in. 
enum DIALOGUE_STATE {
	DIALOGUE_STATE_NORMAL		=  0,
	DIALOGUE_STATE_OPTION		=  1,
};



class DialogueOptionBox {
public:
	DialogueOptionBox();
	~DialogueOptionBox();

	bool AddOption(std::string text, uint32 speaker_id, int32 next_line, int32 action = -1);
	
	int32 Update();
	
	void Draw();

	uint32 GetCurrentSpeaker() { return _speaker; }

	void SetCurrentDialogue(MapDialogue* dialogue) {_current_dialogue = dialogue;}

private:
	MapDialogue* _current_dialogue;
	uint32 _size;
	uint32 _speaker;
	hoa_video::OptionBox _options;
	std::vector<ScriptObject*> _actions;
	std::vector<int32> _next_line_index;
};

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
	
	//! \brief Sets the dialogue state. 
	void SetDialogueState(DIALOGUE_STATE state)
		{ _state = state; }

	//! \brief Returns the state the dialogue is currently in.
	DIALOGUE_STATE GetDialogueState()
		{ return _state; }

	void SetCurrentDialogue(MapDialogue* dialogue)
		{ _current_dialogue = dialogue; }

	void ClearDialogue()
		{ _current_dialogue = NULL; }

	MapDialogue* GetCurrentDialogue() const
		{ return _current_dialogue; }


private:
	//! \brief Keeps track of whether dialogue is in text mode or option mode. 
	DIALOGUE_STATE _state;
	
	//! \brief A pointer to the current set of options
	DialogueOptionBox* _current_option;

	//! \brief A pointer to the current speaker.
	VirtualSprite* _current_speaker;

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
*** 
*** Dialogues consist of multiple lines. Each line of a dialogue contains the 
*** following information:
***
*** -# The text of the line
*** -# An object ID that indicates who is currently speaking the line
*** -# A value that indicates the maximum time that the line should be displayed
*** -# A pointer to a script function to execute after the line is finished
***
*** Both the time value and the script function pointer are optional and do not
*** need to be set for every line of dialogue. Dialogues may also be "blocked",
*** which means that they ignore the user's input while the dialogue is executing.
***
*** When a dialogue is finished, usually the state of all speaker sprites (status 
*** such as the direction they were facing prior to the dialogue) is restored so
*** that they can continue. Also for dialogues which are "owned" by a sprite (where
*** owned simply means that the dialogue instance is retained in the VirtualSprite#_dialogues
*** container), the sprite is informed that the dialogue has finished so that the
*** sprite may re-check whether or not all dialogues that it contains have been
*** seen by the player.
*** ***************************************************************************/
class MapDialogue {
public:
	/** \brief This is the default contructor.
	*** It can take a bool parameter to indicate weither or not the dialogue should
	*** reset the speakers to the state at which they were before the dialogue.
	*** This parameter is set to true (reset) by default.
	**/
	MapDialogue(bool save_state = true);

	~MapDialogue();

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

	//! \brief This method adds an option to the current line of text
	void AddOption(std::string text, uint32 speaker_id, int32 next_line = -1, int32 action = -1);
	
	DialogueOptionBox* GetCurrentOption() const {return _options[_current_line];};

	//! \brief This method will return true if the current line contains options.
	bool HasOptions() 
	{ if(_options[_current_line] != NULL) return true; else return false; }

	int32 GetNextLine();
	
	/** \brief This method will update the current line of the dialogue.
	*** \return False if the dialogue is finished, true otherwise.
	**/
	bool ReadNextLine(int32 line = -1);

	void GoToLine(int32 next_line) { _next_line_index[_next_line_index.size()-1] = next_line; }

	void EndDialogue() { _next_line_index[_next_line_index.size()-1] = 9999; }

	//! \name Class Member Access Functions
	//@{
	//! \brief This resets the counter that keeps track of how many times the dialogue has been seen.
	void ResetTimesSeen()
		{ _seen = 0; }

	//! \brief Indicates if this dialogue has already been seen by the player.
	bool HasAlreadySeen() const
		{ return (_seen == 0) ? false : true; }

	//! \brief This increments the counter that keeps track of how many times the dialogue has been seen by the player
	void IncrementTimesSeen()
		{ _seen++; }

	//! \brief This sets the max number of times a dialogue can be viewed by the player.
	void SetMaxViews(int32 views)
		{ _max_views = views; }

	//! \brief This returns the number of views that this dialogue can be seen by the player.
	int32 GetMaxViews() const
		{ return _max_views; }

	//! \brief This method controls if the dialogue should ignore user input (true) or not (false).
	void SetBlock(bool b)
		{ _blocked = b; }

	void SetOwner(VirtualSprite* sprite)
		{ _owner = sprite; }

	//! \brief This returns the number of times that this dialogue has been seen by the player.
	int32 GetTimesSeen() const
		{ return _seen; }

	//! \brief Return true if this dialogue is active.(IE _seen is still less than _max_views)
	bool isActive() const
		{ return _active; }

	//! \brief Returns a bool that indicates whether a dialogue is blocked (ignores user input)
	bool IsBlocked() const
		{ return _blocked; }

	//! \brief Returns true if a dialogue should load the saved state of the dialogue speakers at the end of the dialogue.
	bool IsSaving() const
		{ return _save_state; }

	VirtualSprite* GetOwner() const
		{ return _owner; }

	//! \brief This returns the number of line of the dialogue.
	uint32 GetNumLines() const
		{ return _speakers.size(); }

	//! \brief Returns a reference to the unicode text string of the current line of dialogue.
	hoa_utils::ustring GetCurrentText() const
		{ return _text[_current_line]; }

	//! \brief Returns the object ID of the speaker of the current line of dialogue.
	uint32 GetCurrentSpeaker() const
		{ return _speakers[_current_line]; }

	//! \brief Returns the display time of the current line of dialogue.
	int32 GetCurrentTime() const
		{ return _time[_current_line]; }

	//! \brief Returns a pointer to the ScriptObject that will be invoked after the current line of dialogue completes
	ScriptObject* GetCurrentAction()
		{ return _actions[_current_line]; }

	//! \brief Returns the text of a specific line.
	hoa_utils::ustring GetLineText(uint32 line) const
		{ if (line > _text.size()) return hoa_utils::ustring(); else return _text[line]; }

	uint32 GetCurrentLine() {return _current_line;}

	//! \brief Returns the object id of the speaker of a line.
	uint32 GetLineSpeaker(uint32 line) const
		{ if (line > _speakers.size()) return 0; else return _speakers[line]; }

	//! \brief Returns the maximum time in milliseconds that the current line of dialogue should be displayed.
	int32 GetLineTime(uint32 line) const
		{ if (line > _time.size()) return -1; else return _time[line]; }

	//! \brief Returns the actions of a specific line.
	ScriptObject* GetLineAction(uint32 line)
		{ if (line > _actions.size()) return NULL; else return _actions[line]; }

private:
	//! \brief This counts the number of time a player has seen this dialogue.
	uint32 _seen;

	//! \brief Declares the max number of times this dialogue can be viewed.
	int32 _max_views;

	uint32 _line_count;

	//! \brief An index to the current line to read.
	uint32 _current_line;

	//! \brief Declares whether or not the dialogue is still active, which is determined by _max_views.
	bool _active;

	//! \brief When this member is set to true, dialogues will ignore user input and instead execute independently.
	bool _blocked;

	//! \brief Declares whether or not to reset the status of map sprites after the dialogue completes.
	bool _save_state;

	//! \brief The sprite, if any, which "owns" this dialogue (ie the dialogue can only be initiated by talking to the owner)
	VirtualSprite* _owner;

	//! \brief The text of the conversation, split up into multiple lines.
	std::vector<hoa_utils::ustring> _text;

	//! \brief A list of object ID numbers that declare who speaks which lines.
	std::vector<uint32> _speakers;

	//! \brief The maximum display time of each line in the dialogue. A time less than zero indicates infinite time.
	std::vector<int32> _time;

	//! \brief A list of DialogueOptions indexed according to the line of dialogue they belong to. 
	std::vector<DialogueOptionBox*> _options;
	//@}

	//! \brief A list of optional events that may occur after each line.
	std::vector<ScriptObject*> _actions;
	
	//! \brief 
	std::vector<int32> _next_line_index;
}; // class MapDialogue

} // namespace private_map

} // namespace hoa_map

#endif
