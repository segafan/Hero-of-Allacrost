///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
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

// Allacrost utilities
#include "utils.h"
#include "defs.h"

// Allacrost engines
#include "script.h"
#include "video.h"

namespace hoa_map {

namespace private_map {

//! \brief Used to indicate that a line of dialogue can stay on the screen for an infinite amount of time
const int32 DIALOGUE_INFINITE = -1;

//! \brief Indicates the maximum number of options that a line of dialogue can present
const uint32 MAX_OPTIONS = 5;

//! \brief Defines the different states the dialogue can be in.
enum DIALOGUE_STATE {
	DIALOGUE_STATE_NORMAL   =  0, //!< standard text presented in dialogue window
	DIALOGUE_STATE_OPTION   =  1, //!< player-selectable options presented in dialogue window
};


/** ****************************************************************************
*** \brief Represents dialogues between characters on a map
***
*** Dialogues consist of multiple lines. Each line of a dialogue contains the
*** following information:
***
*** -# The text of the line
*** -# An object ID that indicates who is currently speaking the line
*** -# A value that indicates the maximum time that the line should be displayed
*** -# A pointer to a script function to execute after the line is finished
***
*** Dialogues may also have a set of options attached to it. Each set of options
*** in a dialogue are represented by an instance of the DialogueOptionBox class.
*** The options are stored in a vector of DialogueOptionBox object pointers. This
*** vector is indexed by the lines of dialogue, so options for line 3 would be stored
*** in _options[3]. A null value would mean there are no options for that line of
*** dialogue.
***
*** Both the display time value and the script function pointer are optional and
*** do not need to be set for every line of dialogue. Dialogues may also be "blocked",
*** which means that they ignore the user's input while the dialogue is executing.
*** The map file retains the number of times each dialogue in the map has been
*** seen by the player so that subsequent visits to the map do not falsely display
*** sprites as having new dialogue.
***
*** The standard order of lines in a dialogue begin with the first line and end
*** with the last (as stored in the lines container). However this need not be
*** the case and quite often is not. After reading a line, you may proceed to any
*** other line in a dialogue. The next line can be chosen either by selecting a
*** particular option presented on a line, or looking up the next line value in a
*** class container. It can also be explicity set when calling the ReadNextLine
*** method to finish reading the current line, although this is usually only done
*** by the dialogue manager when processing a selected dialogue option.
***
*** When a dialogue is finished, usually the state of all speaker sprites is
*** restored (uch as the direction they were facing prior to the dialogue) so that
*** they can resume their prior activity. Also for dialogues which are "owned" by
*** a sprite (where owned simply means that the dialogue instance is retained in
*** the VirtualSprite#_dialogues container), the sprite is informed that the
*** dialogue has finished so that the sprite may re-check whether or not all
*** dialogues that it contains have been seen by the player.
***
*** \todo MapDialogues need to be made more generic. They should not require a
*** speaker ID (ie we can have a "narrator" of sorts), they should not require
*** a portrait, and they should not be contained within the VirtualSprite class.
*** Dialogues should be retained by the DialogueManager class and sprites should
*** reference these dialogues via a dialogue ID as appropriate.
*** ***************************************************************************/
class MapDialogue {
public:
	/** \param save_state If true, the state of the speakers should be reset when
	*** the dialogue finishes (default == true)
	**/
	MapDialogue(bool save_state = true);

	~MapDialogue();

	/** \brief Adds a new line of text and to the dialogue
	*** \param text The text to show on the screen
	*** \param speaker_id The object ID of the speaker of this line of text
	*** \param time The maximum time in milliseconds to show the line of dialogue (default == infinite)
	*** \param action An integer key to the map_functions table in the map file that contains the script function
	*** to execute when this line completes. A negative value indicates that no action is to occur.
	***
	*** \todo This should take a ustring instead of a std::string, but we need better support
	*** for ustring in scripts to do that first.
	**/
	void AddText(std::string text, uint32 speaker_id, int32 time = DIALOGUE_INFINITE, int32 action = -1);

	/** \brief Adds an option to the most recently added line of text
	*** \param text The text for this particular option
	*** \param speaker_id The object ID of the speaker of the set of options
	*** \param next_line The index value of the next line of dialogue to display should this option be selected
	*** (default value of -1 indicates to just continue on to the standard next line)
	*** \param action An integer key to the map_functions table in the map file that contains the
	*** script function to execute should this option be selected
	*** (default value of -1 indicates that no action is to occur.)
	***/
	void AddOption(std::string text, uint32 speaker_id, int32 next_line = -1, int32 action = -1);

	/** \brief Proceeds the dialogue forward to display the next line
	*** \param line Index value of the next line of dialogue to read. This value defaults to -1,
	*** which indicates that the next line of dialogue is immediately after the current line.
	*** \return False if the dialogue is finished, true otherwise.
	**/
	bool ReadNextLine(int32 line = -1);

	//! \brief Return true if this dialogue is available to be viewed (_times_seen is still less than _max_views)
	bool IsAvailable() const
		{ if (_max_views < 0) return true; else return (static_cast<int32>(_times_seen) < _max_views); }

	//! \brief Resets the _times_seen counter to zero
	void ResetTimesSeen()
		{ _times_seen = 0; }

	//! \brief Increments the number of times this dialogue has been seen by the player
	void IncrementTimesSeen()
		{ _times_seen++; }

	//! \brief Indicates if this dialogue has already been seen by the player.
	bool HasAlreadySeen() const
		{ return (_times_seen == 0) ? false : true; }

	/** \brief Sets the next line for a specified line of dialogue
	*** \param set_line The line of dialogue to change its next line value
	*** \param next_line The line of dialogue to skip to. A negative value indicates no
	**/
	void SetNextLine(uint32 set_line, int32 next_line)
		{ _next_lines[_next_lines.size() - 1] = next_line; }

	/** \brief Ends the current dialogue by setting the next line to an unlikely high line value
	*** \todo I don't think this should be necessary to end a dialogue. Find a better way and then eliminate this function
	**/
	void EndDialogue()
		{ _next_lines[_next_lines.size() - 1] = 9999; }

	// ----- Methods: retrieval of properties of the current line

	//! \brief Returns true if the current line contains options.
	bool CurrentLineHasOptions() const
		{ if (_options[_current_line] != NULL) return true; else return false; }

	//! \brief Returns the set of options for the current line (will be NULL if no options exist for this line)
	DialogueOptionBox* GetCurrentOptions() const
		{ return _options[_current_line]; }

	//! \brief Returns an integer value of the next line of dialogue to be displayed for the current line
	int32 GetCurrentNextLine() const
		{ return _next_lines[_current_line]; }

	//! \brief Returns a reference to the unicode text string of the current line of dialogue
	const hoa_utils::ustring& GetCurrentText() const
		{ return _text[_current_line]; }

	//! \brief Returns the object ID of the speaker of the current line of dialogue
	uint32 GetCurrentSpeaker() const
		{ return _speakers[_current_line]; }

	//! \brief Returns the display time of the current line of dialogue
	int32 GetCurrentTime() const
		{ return _display_times[_current_line]; }

	//! \brief Returns a pointer to the ScriptObject that will be invoked after the current line of dialogue completes
	ScriptObject* GetCurrentAction()
		{ return _actions[_current_line]; }

	// ----- Methods: retrieval of properties of a specific line

	//! \brief Returns the text of a specific line
	hoa_utils::ustring GetLineText(uint32 line) const
		{ if (line > _text.size()) return hoa_utils::ustring(); else return _text[line]; }

	//! \brief Returns the object id of the speaker of a specific line
	uint32 GetLineSpeaker(uint32 line) const
		{ if (line > _speakers.size()) return 0; else return _speakers[line]; }

	//! \brief Returns the display time of a specific line
	int32 GetLineTime(uint32 line) const
		{ if (line > _display_times.size()) return -1; else return _display_times[line]; }

	//! \brief Returns the actions to follow a specific line
	ScriptObject* GetLineAction(uint32 line)
		{ if (line > _actions.size()) return NULL; else return _actions[line]; }

	//! \name Class Member Access Functions
	//@{
	int32 GetMaxViews() const
		{ return _max_views; }

	int32 GetTimesSeen() const
		{ return _times_seen; }

	VirtualSprite* GetOwner() const
		{ return _owner; }

	uint32 GetLineCount() const
		{ return _line_count; }

	uint32 GetCurrentLine()
		{ return _current_line;}

	bool IsBlocked() const
		{ return _blocked; }

	bool IsSaveState() const
		{ return _save_state; }

	void SetTimesSeen(uint32 times)
		{ _times_seen = times; }

	void SetMaxViews(int32 views)
		{ _max_views = views; }

	void SetBlocked(bool block)
		{ _blocked = block; }

	void SetOwner(VirtualSprite* sprite)
		{ _owner = sprite; }

	//! \todo This function should be eliminated once the dialogues are changed to not be contained within map sprites
	void SetEventName(std::string name)
		{ _event_name = name; }
	//@}

private:
	//! \brief Counts the number of time a player has seen this dialogue.
	uint32 _times_seen;

	//! \brief Declares the max number of times that this dialogue can be viewed (negative value indicates no limit)
	int32 _max_views;

	//! \brief Stores the amount of lines in the dialogue.
	uint32 _line_count;

	//! \brief An index to the current line to read.
	uint32 _current_line;

	//! \brief If true, dialogues will ignore user input and instead execute independently
	bool _blocked;

	//! \brief If true, the status of map sprites will be reset after the dialogue completes
	bool _save_state;

	//! \brief The event name for this dialogue that is stored in the saved game file, usually of the form "s##_d##"
	std::string _event_name;

	//! \brief The sprite, if any, which "owns" this dialogue (the dialogue can only be initiated by talking to the owner)
	VirtualSprite* _owner;

	//! \brief The text of the conversation, split up into multiple lines
	std::vector<hoa_utils::ustring> _text;

	//! \brief A list of object ID numbers that declare the speaker of each line
	std::vector<uint32> _speakers;

	//! \brief The maximum display time for each line in the dialogue. A negative value indicates infinite time
	std::vector<int32> _display_times;

	/** \brief Holds indeces pointing to which line should follow each line of dialogue
	*** Usually this vector is populated with negative values, which indicate that the next line should simply
	*** be at the next highest index
	**/
	std::vector<int32> _next_lines;

	//! \brief A set of dialogue options indexed according to the line of dialogue that they belong to
	std::vector<DialogueOptionBox*> _options;

	//! \brief A list of optional events that may occur after each line
	std::vector<ScriptObject*> _actions;
}; // class MapDialogue


/** ***************************************************************************************
*** \brief Stores a single OptionBox instance and contains methods and members to update/draw it.
***
*** This class is used only by the MapDialogue class. It creates an instance of
*** the OptionBox class which is located in the video engine GUI. Using the
*** AddOption method, the OptionBox is populated. There are also methods to
*** update the OptionBox(check for selections, key presses, etc) and to draw
*** the OptionBox to the screen, both of these are defined by the OptionBox class.
*** **************************************************************************************/
class DialogueOptionBox {
public:
	DialogueOptionBox();
	~DialogueOptionBox();

	/** \brief This method adds an option to the OptionBox
	*** \param text The text of this particular option.
	*** \param speaker_id The object ID of the speaker of the set of options. (Should correspond to a VirtualSprite or derived class.)
	*** \param next_line An integer value of the next line of dialogue should this option be selected.
	*** \param action An integer key to the map_functions table in the map file that contains the script function to execute when this line complese. (Less than 0 indicates no action is to occur.)
	***/
	bool AddOption(std::string text, uint32 speaker_id, int32 next_line, int32 action = -1);

	//! \brief Calls upon the OptionBox update to check for key presses/selections.
	int32 Update();

	//! \brief Calls upon the OptionBox draw function.
	void Draw();

	//! \brief Returns the speaker who owns the Options
	uint32 GetCurrentSpeaker() { return _speaker; }

	//! \brief Sets the dialogue that the option belongs to.
	void SetCurrentDialogue(MapDialogue* dialogue) {_current_dialogue = dialogue;}

private:
	//! \brief Stores the dialogue that the options belong to.
	MapDialogue* _current_dialogue;

	//! \brief The size of the option box(number of options.)
	uint32 _size;

	//! \brief The speaker of the options
	uint32 _speaker;

	//! \brief Instance of the OptionBox class.
	hoa_video::OptionBox _options;

	//! \brief A list of optional events that may occur after each line.
	std::vector<ScriptObject*> _actions;

	//! \brief A index containing the line of dialogue each option is directed to.
	std::vector<int32> _next_line_index;
}; // class DialogueOptionBox


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

	//! \brief Updates the current dialogue.
	void SetCurrentDialogue(MapDialogue* dialogue)
		{ _current_dialogue = dialogue; }

	//! \brief Clears the current dialogue.
	void ClearDialogue()
		{ _current_dialogue = NULL; }

	//! \brief Returns the current dialogue.
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

} // namespace private_map

} // namespace hoa_map

#endif // __MAP_DIALOGUE_HEADER__
