///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_dialogue.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for map mode dialogue
***
*** \todo There is A LOT of indexing and id referencing in this code and in the
*** MapSprite's dialogue-related code. Currently very little is done to warn of
*** bad references (a sprite referencing a dialogue via an invalid ID) or bad
*** indexing (indexing a line of dialogue that does not exist/out of bounds errors).
*** The reason why things are this way right now is because sprites and dialogues
*** can be created in any order (and can even be created after the map has been loaded
*** and is in play). Because of this, the code may be referencing things that are
*** not yet created but will be shortly, so we don't want to print warnings about
*** those types of circumstances. What I think we should do is write a "CheckWarnings"
*** function or something similar that can be called after it is determined that
*** everything *should* be created and referenced properly.
*** ***************************************************************************/

#ifndef __MAP_DIALOGUE_HEADER__
#define __MAP_DIALOGUE_HEADER__

// Allacrost utilities
#include "utils.h"
#include "defs.h"

// Allacrost engines
#include "script.h"
#include "video.h"
#include "gui.h"

// Local map mode headers
#include "map_utils.h"

namespace hoa_battle {
class BattleMode;
}

namespace hoa_map {

namespace private_map {

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
	//! \param id The id number to represent the dialogue by (should be unique to other dialogue ids)
	MapDialogue(uint32 id);

	~MapDialogue();

	/** \brief Adds a new line of text and to the dialogue
	*** \param text The text to show on the screen
	*** \param speaker_id The object ID of the speaker of this line of text
	*** \param next_line The line of dialogue which should follow this one (a negative value indicates to end the dialogue)
	*** \param time The maximum time in milliseconds to show the line of dialogue (default == infinite)
	*** \param event The ID of an event to enact after selecting the option. A zero value indicates that no event is to occur.
	***
	*** \todo This should take a ustring instead of a std::string, but we need better support
	*** for ustring in scripts to do that first.
	**/
	void AddText(std::string text, uint32 speaker_id, int32 next_line, uint32 event = 0, bool display_timer = false);

	/** \brief Adds an option to the most recently added line of text
	*** \param text The text for this particular option
	*** \param next_line The index value of the next line of dialogue to display should this option be selected
	*** (a negative value indicates to end the dialogue immediately after the option is selected)
	*** \param event The ID of an event to enact after selecting the option. A zero value indicates that no event is to occur.
	***
	*** \todo This should take a ustring instead of a std::string, but we need better support
	*** for ustring in scripts to do that first.
	**/
	void AddOption(std::string text, int32 next_line, uint32 event = 0);

	/** \brief Proceeds the dialogue forward to display the next line
	*** \param line Index value of the next line of dialogue to read. A negative value indicates
	*** that there is no proceeding line and that the dialogue should finish.
	*** \return False if the dialogue is finished, true otherwise.
	**/
	bool ReadNextLine(int32 line);

	/** \brief Returns the string of the dialogue's event name as it would be stored in the saved game file
	*** \return The event string in the standard format of "dialogue#ID", where ID is the dialogue ID value
	**/
	std::string GetEventName()
		{ return std::string("dialogue#" + hoa_utils::NumberToString(_dialogue_id)); }

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
		{ return (_times_seen != 0); }

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

	//! \brief Returns the integer ID of the event that will be invoked after the current line of dialogue completes
	uint32 GetCurrentEvent()
		{ return _events[_current_line]; }

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

	//! \brief Returns the ID of the event to execute after a specific line
	uint32 GetLineEvent(uint32 line)
		{ if (line > _events.size()) return 0; else return _events[line]; }

	//! \name Class Member Access Functions
	//@{
	uint32 GetDialogueID() const
		{ return _dialogue_id; }

	int32 GetMaxViews() const
		{ return _max_views; }

	int32 GetTimesSeen() const
		{ return _times_seen; }

	uint32 GetLineCount() const
		{ return _line_count; }

	uint32 GetCurrentLine()
		{ return _current_line;}

	bool GetSaveState() const
		{ return _save_state; }

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

	void SetSaveState(bool state)
		{ _save_state = state; }
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

	//! \brief If true, dialogue will ignore user input and instead execute independently
	bool _blocked;

	//! \brief If true, the status of map sprites will be reset after the dialogue completes
	bool _save_state;

	//! \brief The event name for this dialogue that is stored in the saved game file, of the form "dialogue#"
	std::string _event_name;

	//! \brief The text of the conversation, split up into multiple lines
	std::vector<hoa_utils::ustring> _text;

	//! \brief A list of object ID numbers that declare the speaker of each line
	std::vector<uint32> _speakers;

	//! \brief The maximum display time for each line in the dialogue. A negative value indicates infinite time
	std::vector<int32> _display_times;

	//! \brief Holds indeces pointing to which line should follow each line of text. A negative value indicates that the dialogue should end.
	std::vector<int32> _next_lines;

	//! \brief A set of dialogue options indexed according to the line of dialogue that they belong to
	std::vector<MapDialogueOptions*> _options;

	//! \brief An optional MapEvent that may occur after each line
	std::vector<uint32> _events;
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

	~MapDialogueOptions()
		{}

	/** \brief Adds a new option to the OptionBox
	*** \param text The text for the new option
	*** \param next_line An integer index of the next line of dialogue should this option be selected.
	*** \param event The ID of an event to enact after selecting the option. Zero indicates that no event is to occur.
	***/
	void AddOption(hoa_utils::ustring text, int32 next_line, uint32 event = 0);

private:
	//! \brief Contains the text of the dialogue, where each entry represents a single line
	std::vector<hoa_utils::ustring> _text;

	/** \brief A index containing the next line of dialogue that should follow each option
	*** This is an index into the lines container for the MapDialogue object that is using this set of options.
	**/
	std::vector<int32> _next_lines;

	//! \brief An optional MapEvent that may occur as a result of selecting each option
	std::vector<uint32> _events;
}; // class MapDialogueOptions


/** ****************************************************************************
*** \brief A display window for all GUI controls and graphics necessary to execute a dialogue
***
*** This class, inheriting from the MenuWindow class, handles all visual control
*** and placement of a dialgoue. It serves primarily as a container class for
*** dialogue graphics.
*** ***************************************************************************/
class DialogueWindow : public hoa_gui::MenuWindow {
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
	//! \brief A parchment paper image embedded within the dialogue window
	hoa_video::StillImage _parchment_image;

	//! \brief The nameplate image used along with the dialogue box image
	hoa_video::StillImage _nameplate_image;

	//! \brief The textbox used for rendering the dialogue text
	hoa_gui::TextBox _display_textbox;

	//! \brief The option box used for rendering dialogue options where applicable
	hoa_gui::OptionBox _display_options;
}; // class DialogueWindow : public hoa_video::MenuWindow


/** ****************************************************************************
*** \brief Manages dialogue execution on maps
***
*** The MapMode class creates an instance of this class to handle all dialogue
*** processing that occurs on maps. This includes containing the dialogue objects,
*** handling user input, processing of script events, and display timing of the
*** dialogue.
***
*** \todo Add support so that the player may backtrack through lines in a
*** dialogue (without re-processing selected options or previous script events).
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

	/** \brief Adds a reference of a sprite to a dialogue
	*** \param dialogue_id The ID number of the dialogue that the sprite wishes to reference
	*** \param sprite_id The ID number of the sprite requesting to be referenced
	***
	*** Sprites reference a dialogue so that when the dialogue's status is updated (view count incremented, etc),
	*** the sprite will be informed that the dialogue has changed.
	**/
	void AddSpriteReference(uint32 dialogue_id, uint32 sprite_id);

	/** \brief Prepares the dialogue manager to begin processing a new dialogue
	*** \param dialogue_id The id number of the dialogue to begin
	**/
	void BeginDialogue(uint32 dialogue_id);

	/** \brief Prepares the dialogue manager to begin processing a new dialogue
	*** \param sprite A pointer to the map sprite that references the dialogue to begin processing
	***
	*** This function operates the same as the other BeginDialogue function with one exception. It also
	*** handles the calls necessary to update the map sprite. Specifically, making sure the sprite references a
	*** valid dialogue and increments its next dialogue pointer.
	**/
	void BeginDialogue(MapSprite* sprite);

	//! \brief Immediately ends any dialogue that is taking place
	void EndDialogue();

	/** \brief Returns a pointer to the MapDialogue with the requested ID value
	*** \param dialogue_id The identification number of the dialogue to retrieve
	*** \return A pointer to the dialogue requested, or NULL if no such dialogue was found
	**/
	MapDialogue* GetDialogue(uint32 dialogue_id);

	/** \brief Called whenever a map dialogue object's status is updated
	*** \param dialogue_id The ID number of the dialogue which was updated
	***
	*** The purpose of this function is to inform all map sprites which reference this dialogue
	*** that it has been updated, and that they should update their associated data accordingly. For example,
	*** it allows the sprite to re-examine whether or not it references any dialogue that has not been read by the player.
	*** This function is called automatically by the class every time that this class ends a dialogue that is taking place.
	**/
	void AnnounceDialogueUpdate(uint32 dialogue_id);

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

	/** \brief A container that stores map sprite IDs that are referenced with map dialogues
	*** The map key is the MapDialogue ID and the vector of unsigned integers is each sprite that references the dialogue.
	***
	*** \note The reason why these references are stored in this class rather than in the MapDialogue class is because
	*** it would require that a MapDialogue object exist before a sprite could create a reference to it. This would require
	*** an unnecessary dependency about which class objects are created first in the map script which should be avoided.
	**/
	std::map<uint32, std::vector<uint32> > _sprite_references;

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

	//! \brief Restores sprites to their states before this dialogue started
	void _RestoreSprites();
}; // class DialogueSupervisor

} // namespace private_map

} // namespace hoa_map

#endif // __MAP_DIALOGUE_HEADER__
