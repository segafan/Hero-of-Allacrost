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
	DIALOGUE_STATE_LINE     =  0, //!< standard text presented in dialogue window
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
*** in a dialogue are represented by an instance of the MapDialogueOptions class.
*** The options are stored in a vector of MapDialogueOptions object pointers. This
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
*** the MapSprite#_dialogues container), the sprite is informed that the
*** dialogue has finished so that the sprite may re-check whether or not all
*** dialogues that it contains have been seen by the player.
***
*** \todo MapDialogues need to be made more generic. They should not require a
*** speaker ID (ie we can have a "narrator" of sorts), they should not require
*** a portrait, and they should not be contained within the MapSprite class.
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
	MapDialogueOptions* GetCurrentOptions() const
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

	MapSprite* GetOwner() const
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

	void SetOwner(MapSprite* sprite)
		{ _owner = sprite; }

	//! \todo This function should be eliminated once the dialogues are changed to not be contained within map sprites
	void SetEventName(std::string name)
		{ _event_name = name; }
	//@}

private:
	//! \brief A unique identification number that represents this dialogue
	uint32 _dialogue_id;

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

	//! \brief The event name for this dialogue that is stored in the saved game file, of the form "dialogue#"
	std::string _event_name;

	//! \brief The sprite, if any, which "owns" this dialogue (the dialogue can only be initiated by talking to the owner)
	MapSprite* _owner;

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
	std::vector<MapDialogueOptions*> _options;

	//! \brief A list of optional events that may occur after each line
	std::vector<ScriptObject*> _actions;
}; // class MapDialogue


/** ***************************************************************************************
*** \brief A container class for option sets presented in dialogue
***
*** When the player reads a dialogue he or she may be presented with a small number of options,
*** one of which the player must select. The selected option determines the path that the
*** dialogue will take, which may include an entire series of scripted events and sequences.
*** This class is responsible for containing all of the information necessary to make this
*** possible. It represents a set of options that the player must choose between.
***
*** Instances of this class are populated as needed by the MapDialogue class. For each option,
*** the class contains an index to the next line of dialogue that should be read and an optional
*** pointer to a script function to execute, should that particular option be selected.
*** **************************************************************************************/
class MapDialogueOptions {
	friend class DialogueSupervisor;

public:
	MapDialogueOptions()
		{}

	~MapDialogueOptions();

	/** \brief Adds a new option to the OptionBox
	*** \param text The text for the new option
	*** \param next_line An integer index of the next line of dialogue should this option be selected.
	*** \param action An integer key to the map_functions table in the map file that contains the script
	*** function to execute when this option is select. A negative value indicates no action is to occur.
	***/
	void AddOption(hoa_utils::ustring text, int32 next_line, int32 action = -1);

private:
	//! \brief Contains the text of the dialogue, where each entry represents a single line
	std::vector<hoa_utils::ustring> _text;

	/** \brief A index containing the next line of dialogue that should follow each option
	*** This is an index into the lines container for the MapDialogue object that is using this set of options.
	**/
	std::vector<int32> _next_lines;

	/** \brief An optional script event that may occur after each line
	*** A NULL entry indicates that no option is to occur.
	**/
	std::vector<ScriptObject*> _actions;
}; // class MapDialogueOptions


/** ****************************************************************************
*** \brief A display window for all GUI controls and graphics necessary to execute a dialogue
***
*** This class, inheriting from the MenuWindow class, handles all visual control
*** and placement of a dialgoue. It serves primarily as a container class for
*** dialogue graphics.
***
*** \todo Replace the background image member with just the parchment paper image
*** and use the inherited menu window object to replace the background image.
*** ***************************************************************************/
class DialogueWindow : public hoa_video::MenuWindow {
	friend class DialogueSupervisor;
public:
	DialogueWindow();

	~DialogueWindow();

	//! \brief Unhides the display window and prepares to begin a new dialogue display
	void Initialize();

	//! \brief Clears all GUI structures and hides the display window
	void Reset();

	/** \brief Draws the dialogue window and all other visuals
	*** \param name A pointer to the name of the current speaker
	*** \param portrait A pointer to the portrait image of the current speaker
	***
	*** It is valid for either function argument to be NULL. This indicates
	*** that the window should omit drawing this information.
	**/
	void Draw(hoa_utils::ustring* name, hoa_video::StillImage* portrait);

private:
	//! \brief A background image used in map dialogue
	hoa_video::StillImage _background_image;

	//! \brief The nameplate image used along with the dialogue box image
	hoa_video::StillImage _nameplate_image;

	//! \brief The textbox used for rendering the dialogue text
	hoa_video::TextBox _display_textbox;

	//! \brief The option box used for rendering dialogue options where applicable
	hoa_video::OptionBox _display_options;
}; // class DialogueWindow : public hoa_video::MenuWindow


/** ****************************************************************************
*** \brief Manages and dialogue operation on maps
***
*** The MapMode class creates an instance of this class to handle all dialogue
*** processing that occurs on maps. This includes containing the dialogue objects,
*** handling user input, processing of script actions, and display timing of the
*** dialogue.
***
*** \todo Add support so that the player may backtrack through lines in a
*** dialogue (without re-processing selected options or previous script actions).
*** ***************************************************************************/
class DialogueSupervisor {
public:
	DialogueSupervisor();

	~DialogueSupervisor();

	//! \brief Updates the state of visual elements such as scrolling text
	void Update();

	//! \brief Draws the dialogue window, text, portraits, and other related visuals to the screen
	void Draw();

	/** \brief Adds a new dialogue to be managed by the supervisor
	*** \param dialogue Pointer to a MapDialogue object that was created with the new operator
	***
	*** The dialogue to add must have a unique dialogue ID so that it can be added to the map.
	*** If a dialogue with the same ID is already found within the map, then the dialogue will
	*** not be added. All dialogues that are successfully added will be later deleted when this
	*** class' destructor is invoked, so make sure you only pass in MapDialogue's that were
	*** created with the "new" operator.
	**/
	void AddDialogue(MapDialogue* dialogue);

	/** \brief Prepares the dialogue manager to begin processing a new dialogue
	*** \param dialogue_id The id number of the dialogue to begin
	**/
	void BeginDialogue(MapDialogue* dialogue);

	//! \brief Immediately ends any dialogue that is taking place
	void EndDialogue();

	/** \brief Returns a pointer to the MapDialogue with the requested ID value
	*** \param dialogue_id The identification number of the dialogue to retrieve
	*** \return A pointer to the dialogue requested, or NULL if no such dialogue was found
	**/
	MapDialogue* GetDialogue(uint32 dialogue_id);

	//! \name Class member access functions
	//@{
	DIALOGUE_STATE GetDialogueState() const
		{ return _state; }

	MapDialogue* GetCurrentDialogue() const
		{ return _current_dialogue; }

	MapDialogueOptions* GetCurrentOptions() const
		{ return _current_options; }
	
	int32 GetLineTimer() const
		{ return _line_timer; }
	//@}

private:
	//! \brief Contains all dialogues used in the map in a std::map structure. The dialogue ID is the map key
	std::map<uint32, MapDialogue*> _all_dialogues;

	//! \brief Retains the current state of the dialogue
	DIALOGUE_STATE _state;

	//! \brief A pointer to the current piece of dialogue that is active
	MapDialogue* _current_dialogue;

	//! \brief A pointer to the current set of options for the active dialogue line
	MapDialogueOptions* _current_options;

	//! \brief A timer that is employed for dialogues which have a display time limit
	int32 _line_timer;

	//! \brief The window and associated GUI controls where the dialogue text and graphics should be displayed
	DialogueWindow _dialogue_window;

	// ---------- Private methods ----------

	//! \brief Updates the state of the dialogue when it is in the line state
	void _UpdateLine();

	//! \brief Updates the state of the dialogue when it is in the option state
	void _UpdateOptions();

	//! \brief Populates the dialogue window's option box with the current line option text
	void _ConstructOptions();

	/** \brief Finishes the current dialogue line and moves the dialogue forward to the next line
	*** \param next_line The index of the next line to read in the dialogue
	*** This function will automatically end the dialogue if no line follows the current line
	**/
	void _FinishLine(int32 next_line);
}; // class DialogueSupervisor

} // namespace private_map

} // namespace hoa_map

#endif // __MAP_DIALOGUE_HEADER__
