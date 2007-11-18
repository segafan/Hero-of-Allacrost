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


/** ***************************************************************************************
*** \brief Displays the contents of a discovered treasure in a menu window
***
*** An instance of this class is defined in the MapMode class. Typically you should only
*** need to operate on this instance and not create any additional instances of the treasure
*** menu. Upon opening a treasure chest or other treasure-containing map object, this menu
*** should appear and list the amount of drunes found, if any, a list of the icon and name of
*** each GlobalObject found (items, equipment, etc), a smaller sub-window for displaying
*** detailed information about highlighted entries, and a confirmation option so that the
*** user may exit the menu. For certain 
***
*** To use this class do the following steps:
***
*** -# Call the Initialize method to show the menu with the treasure argument passed
*** -# Call the Update method to process user input and update the menu's appearance
*** -# Call the Draw method to draw the menu to the screen
*** -# Call the Reset method to hide the menu and add the treasure's contents to the player's
***    inventory.
***
*** \todo Allow the player to use or equip selected treasure objects directly from the
*** action menu.
*** **************************************************************************************/
class TreasureMenu {
public:
	//! \brief The possible sub-windows that are selected. Used for determining how to process user input.
	enum SELECTION {
		ACTION_SELECTED = 0,
		LIST_SELECTED = 1,
		DETAIL_SELECTED = 2
	};

	TreasureMenu();

	~TreasureMenu();

	/** \brief Un-hides the menu window and initializes it to display the contents of a new treasure
	*** \param treasure A pointer to the treasure to display the contents of
	**/
	void Initialize(MapTreasure* treasure);

	//! \brief Hides the window and adds the treasure's contents to the player's inventory
	void Reset();

	//! \brief Returns true if the treasure menu is active and user input should be processed through it
	bool IsActive() const
		{ return (_treasure != NULL); }

	//! \brief Processes input events from the user and updates the showing/hiding progress of the window
	void Update();

	/** \brief Draws the window to the screen
	*** \note If the Initialize method has not been called with a valid treasure pointer beforehand, this
	*** method will print a warning and it will not draw anything to the screen.
	**/
	void Draw();

private:
	//! \brief A window which contains options for viewing, using, or equipping inventory, or for exiting the menu
	hoa_video::MenuWindow _action_window;

	//! \brief The window which lists all of the drunes and inventory objects contained in the treasure
	hoa_video::MenuWindow _list_window;

	//! \brief A smaller window for displaying detailed information about the selected entry in the _list_otpions
	hoa_video::MenuWindow _detail_window;

	//! \brief The available actions that a user can currently take. Displayed in the _action_window.
	hoa_video::OptionBox _action_options;

	//! \brief The icon + name of all drunes and inventory objects earned. Displayed in the _list_window
	hoa_video::OptionBox _list_options;

	//! \brief A pointer to the treasure object to display the contents of
	MapTreasure* _treasure;

	//! \brief The currently selected sub-window for processing user input
	SELECTION _selection;

	// ---------- Private methods

	//! \brief Processes user input when the action sub-window is selected
	void _UpdateAction();

	//! \brief Processes user input when the list sub-window is selected
	void _UpdateList();

	//! \brief Processes user input when the detail sub-window is selected
	void _UpdateDetail();
}; // class TreasureMenu


/** ***************************************************************************************
*** \brief Stores a single OptionBox instance and contains methods and members to update/draw it.
***
*** This class is used only by the MapDialogue class. It creates an instance of 
*** the OptionBox class which is located in the video engine GUI. Using the 
*** AddOption method, the OptionBox is populated. There are also methods to 
*** update the OptionBox(check for selections, key presses, etc) and to draw 
*** the OptionBox to the screen, both of these are defined by the OptionBox class.
***
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
*** Dialogues may also have a set of options attached to it. The options are 
*** created and displayed using the OptionBox class in the video engine GUI. The options are 
*** stored in a vector of DialogueOptionBox object pointers. The vector is indexed by
*** the lines of dialogue, so options for line 3 would be stored in _options[3]. 
*** A null value would mean there are no options for that line of dialogue.
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

	/** \brief This method adds an option to the current line of text
	*** \param text The text of this particular option.
	*** \param speaker_id The object ID of the speaker of the set of options. (Should correspond to a VirtualSprite or derived class.)
	*** \param next_line An integer value of the next line of dialogue should this option be selected. 
	*** \param action An integer key to the map_functions table in the map file that contains the script function to execute when this line complese. (Less than 0 indicates no action is to occur.)
	***/
	void AddOption(std::string text, uint32 speaker_id, int32 next_line = -1, int32 action = -1);
	
	//! \brief This method returns the currently loaded option. 
	DialogueOptionBox* GetCurrentOption() const {return _options[_current_line];};

	//! \brief This method will return true if the current line contains options.
	bool HasOptions() 
	{ if(_options[_current_line] != NULL) return true; else return false; }

	//! \brief This method returns an integer value of the next line of dialogue to be displayed. 
	int32 GetNextLine();
	
	/** \brief This method will update the current line of the dialogue.
	*** \param line Integer value of the next line of dialogue. This value defaults to -1 which indicates that the next line of dialogue is immediately after the current line. 
	*** \return False if the dialogue is finished, true otherwise.
	**/
	bool ReadNextLine(int32 line = -1);

	/** \brief This method skips to an indicated line of dialogue.
	*** \param line Integer value of the line of dialogue to skip to.
	**/
	void GoToLine(int32 next_line) { _next_line_index[_next_line_index.size()-1] = next_line; }

	//! \brief This method ends the current dialogue, by setting the next line to an impossibly high value.
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

	/** \brief This method sets the owner of the sprite.
	*** \param sprite The VirtualSprite you are setting as the owner. 
	**/
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

	//! \brief Returns the owner of the VirtualSprite object which owns the current dialogue.
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

	//! \brief Returns the current line of the dialogue.
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

	//! \brief Stores the amount of lines in the dialogue.
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
	
	//! \brief Each element of the vector represents a line of dialogue, and the value stored within that element is the line the dialogue will progress to next. 
	std::vector<int32> _next_line_index;
}; // class MapDialogue

} // namespace private_map

} // namespace hoa_map

#endif
