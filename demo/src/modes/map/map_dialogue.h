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
*** Dialogues should be retained by the DialogueWindow class and sprites should
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
	*** \param next_line The index value of the next line of dialogue to display should this option be selected
	*** (default value of -1 indicates to just continue on to the standard next line)
	*** \param action An integer key to the map_functions table in the map file that contains the
	*** script function to execute should this option be selected
	*** (default value of -1 indicates that no action is to occur.)
	***/
	void AddOption(std::string text, int32 next_line = -1, int32 action = -1);

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
*** \brief A wrapper around the OptionBox class specific to the needs of dialogues
***
*** When the player reads a dialogue he or she may be presented at some point with a
*** small number of options, one of which the player must select. The selected option
*** determines the path that the dialogue will take, which may include an entire series
*** of scripted events and sequences. The presentation of that list of options in a
*** dialogue is the responsibility of this class.
***
*** Instances of this class are populated as needed within the MapDialogue class. The
*** class acts as a high level wrapper for the OptionBox class (part of the video engine's
*** GUI system). For each option, the class also contains an index to the next line of
*** dialogue that should be read and an optional pointer to a script function to execute.
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

	/** \brief Adds a new option to the OptionBox
	*** \param text The text for the new option (should be brief)
	*** \param next_line An integer index of the next line of dialogue should this option be selected.
	*** \param action An integer key to the map_functions table in the map file that contains the script
	*** function to execute when this option is select. A negative value indicates no action is to occur.
	***/
	void AddOption(std::string text, int32 next_line, int32 action = -1);

	//! \brief Processes user input and updates the state of the option box (cursor blinking or option scrolling)
	int32 Update();

	//! \brief Draws the OptionBox object
	void Draw();

	//! \brief Returns the option that is currently selected in the option box (-1 if nothing selected)
	int32 GetSelectedOption() const
		{ return _options.GetSelection(); }

private:
	//! \brief The instance of the OptionBox class
	hoa_video::OptionBox _options;

	/** \brief A index containing the next line of dialogue that should follow each option
	*** This is an index into the lines container for the MapDialogue object that is using this set of options.
	**/
	std::vector<int32> _next_lines;

	/** \brief An optional script event that may occur after each line
	*** A NULL entry indicates that no option is to occur.
	**/
	std::vector<ScriptObject*> _actions;
}; // class DialogueOptionBox


/** ****************************************************************************
*** \brief Manages and displays dialogue on maps
***
*** The MapMode class creates an instance of this class to handle all dialogue
*** processing that occurs on maps.. This includes containing the dialogue objects,
*** handling user input, processing of script actions, and visual display of the
*** dialogue.
***
*** \todo Replace the background image member with just the parchment paper image
*** and use the inherited menu window object to replace the background image.
***
*** \todo Modify so that the current line and line options are displayed in the
*** same window.
***
*** \todo Add support so that the player may backtrack through lines in a
*** dialogue (without re-processing selected options or previous script actions).
*** ***************************************************************************/
class DialogueWindow : public hoa_video::MenuWindow {
public:
	DialogueWindow();

	~DialogueWindow();

	/** \brief Prepares the dialogue manager to begin processing a new dialogue
	*** \param dialogue A pointer to the dialogue to begin
	**/
	void BeginDialogue(MapDialogue* dialogue);

	//! \brief Immediately ends any dialogue that is taking place
	void EndDialogue();

	//! \brief Updates the state of the conversation
	void Update();

	//! \brief Draws the dialogue window, text, portraits, and other related visuals to the screen
	void Draw();

	//! \name Class member access functions
	//@{
	DIALOGUE_STATE GetDialogueState() const
		{ return _state; }

	MapDialogue* GetCurrentDialogue() const
		{ return _current_dialogue; }

	DialogueOptionBox* GetCurrentOptions() const
		{ return _current_options; }
	
	int32 GetDisplayTime() const
		{ return _display_time; }
	//@}

private:
	//! \brief Retains the current state of the dialogue
	DIALOGUE_STATE _state;

	//! \brief A pointer to the current piece of dialogue that is active
	MapDialogue* _current_dialogue;

	//! \brief A pointer to the current set of options
	DialogueOptionBox* _current_options;

	//! \brief A pointer to the current speaker.
	int32 _display_time;

	//! \brief Contains every single dialogue that can occur on this map
	std::vector<MapDialogue*> _dialogues;

	//! \brief A background image used in map dialogue
	hoa_video::StillImage _background_image;

	//! \brief The nameplate image used along with the dialogue box image
	hoa_video::StillImage _nameplate_image;

	//! \brief The textbox used for rendering the dialogue text
	hoa_video::TextBox _display_textbox;
}; // class DialogueWindow : public hoa_video::MenuWindow

} // namespace private_map

} // namespace hoa_map

#endif // __MAP_DIALOGUE_HEADER__
