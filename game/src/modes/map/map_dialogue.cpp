///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_dialogue.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for map mode dialogue.
*** ***************************************************************************/

// Allacrost utilities
#include "utils.h"

// Allacrost engines
#include "audio.h"
#include "input.h"
#include "mode_manager.h"

// Allacrost common
#include "dialogue.h"
#include "global.h"

// Other game mode headers
#include "menu.h"

// Local map mode headers
#include "map.h"
#include "map_dialogue.h"
#include "map_events.h"
#include "map_objects.h"
#include "map_sprites.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_gui;
using namespace hoa_input;
using namespace hoa_mode_manager;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_menu;
using namespace hoa_common;

namespace hoa_map {

namespace private_map {

// Used to indicate that no event is to take place for a particular dialogue line or option
const uint32 NO_DIALOGUE_EVENT = 0;

///////////////////////////////////////////////////////////////////////////////
// MapDialogueEventData Class Functions
///////////////////////////////////////////////////////////////////////////////

void MapDialogueEventData::AddEvent(uint32 event_id, bool launch_at_start, uint32 launch_timing) {
	if (event_id == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted to add an event with an invalid ID (0). The event was not added" << endl;
		return;
	}

	_event_ids.push_back(event_id);
	_launches_at_start.push_back(launch_at_start);
	_launch_timings.push_back(launch_timing);
}


void MapDialogueEventData::ProcessEvents(bool at_start) const {
	for (uint32 i = 0; i < _event_ids.size(); ++i) {
		if (_launches_at_start[i] == at_start) {
			if (_launch_timings[i] == 0) {
				MapMode::CurrentInstance()->GetEventSupervisor()->StartEvent(_event_ids[i]);
			}
			else {
				MapMode::CurrentInstance()->GetEventSupervisor()->StartEvent(_event_ids[i], _launch_timings[i]);
			}
		}
	}
}


bool MapDialogueEventData::ValidateEvents() const {
	bool return_value = true;

	for (uint32 i = 0; i < _event_ids.size(); ++i) {
		if (MapMode::CurrentInstance()->GetEventSupervisor()->GetEvent(_event_ids[i]) == NULL) {
			return_value = false;
			IF_PRINT_WARNING(MAP_DEBUG) << "no event was registered for the event ID: " << _event_ids[i] << endl;
		}
	}

	return return_value;
}

///////////////////////////////////////////////////////////////////////////////
// MapDialogue Class Functions
///////////////////////////////////////////////////////////////////////////////

MapDialogue::MapDialogue(uint32 id) :
	CommonDialogue(id),
	_input_blocked(false),
	_restore_state(true),
	_dialogue_name("dialogue#" + hoa_utils::NumberToString(id))
{}



MapDialogue* MapDialogue::Create(uint32 id) {
	MapDialogue* dialogue = new MapDialogue(id);
	MapMode::CurrentInstance()->GetDialogueSupervisor()->RegisterDialogue(dialogue);
	return dialogue;
}



void MapDialogue::AddLine(string text, uint32 speaker) {
	AddLine(text, speaker, COMMON_DIALOGUE_NEXT_LINE);
}



void MapDialogue::AddLine(string text, uint32 speaker, int32 next_line) {
	CommonDialogue::AddLine(text, next_line);
	_speakers.push_back(speaker);
	_line_events.push_back(MapDialogueEventData());
}

void MapDialogue::AddLine(string text){
	AddLine(text,NO_SPRITE, COMMON_DIALOGUE_NEXT_LINE);
}

void MapDialogue::AddLineTiming(uint32 display_time) {
	if (_line_count == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function called when dialogue contained no lines" << endl;
		return;
	}

	_display_times[_line_count - 1] = display_time;
}



void MapDialogue::AddLineTiming(uint32 display_time, uint32 line) {
	if (_line_count == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function called when dialogue contained no lines" << endl;
		return;
	}

	if (line >= _line_count) {
		IF_PRINT_WARNING(MAP_DEBUG) << "invalid line index requested: " << line << endl;
		return;
	}

	_display_times[line] = display_time;
}



void MapDialogue::AddLineEventAtStart(uint32 event_id) {
	AddLineEventAtStart(event_id, 0);
}



void MapDialogue::AddLineEventAtStart(uint32 event_id, uint32 delay_ms) {
	if (_line_count == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function called when dialogue contained no lines" << endl;
		return;
	}

	_line_events[_line_count - 1].AddEvent(event_id, true, delay_ms);
}



void MapDialogue::AddLineEventAtEnd(uint32 event_id) {
	AddLineEventAtEnd(event_id, 0);
}



void MapDialogue::AddLineEventAtEnd(uint32 event_id, uint32 delay_ms) {
	if (_line_count == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function called when dialogue contained no lines" << endl;
		return;
	}

	_line_events[_line_count - 1].AddEvent(event_id, false, delay_ms);
}




void MapDialogue::AddOption(string text) {
	AddOption(text, COMMON_DIALOGUE_NEXT_LINE);
}



void MapDialogue::AddOption(string text, int32 next_line) {
	if (_line_count == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Attempted to add an option to a dialogue with no lines" << endl;
		return;
	}

	uint32 current_line = _line_count - 1;

	// If the line the options will be added to currently has no options, create a new instance of the MapDialogueOptions class to store the options in.
	if (_options[current_line] == NULL) {
		_options[current_line] = new MapDialogueOptions();
	}

	MapDialogueOptions* options = dynamic_cast<MapDialogueOptions*>(_options[current_line]);
	options->AddOption(text, next_line);
}



void MapDialogue::AddOptionEvent(uint32 event_id) {
	AddOptionEvent(event_id, 0);
}



void MapDialogue::AddOptionEvent(uint32 event_id, uint32 delay_ms) {
	if (_line_count == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Attempted to add an option to a dialogue with no lines" << endl;
		return;
	}

	if (_options[_line_count - 1] == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Attempted to add an option event to a line that contained no options" << endl;
		return;
	}

	MapDialogueOptions* options = dynamic_cast<MapDialogueOptions*>(_options[_line_count - 1]);
	options->AddOptionEvent(event_id, delay_ms);
}



bool MapDialogue::Validate() {
	if (CommonDialogue::Validate() == false) {
		// The CommonDialogue::Validate() call will print the appropriate warning if debugging is enabled (common code debugging that is)
		return false;
	}

	// Construct containers that hold all unique sprite and event ids for this dialogue
	set<uint32> sprite_ids;
	set<uint32> event_ids;
	for (uint32 i = 0; i < _line_count; i++) {
		sprite_ids.insert(_speakers[i]);
	}

	// Check that all sprites and events referrenced by the dialogue exist
	for (set<uint32>::iterator i = sprite_ids.begin(); i != sprite_ids.end(); i++) {
		if (MapMode::CurrentInstance()->GetObjectSupervisor()->GetSprite(*i) == NULL) {
			IF_PRINT_WARNING(MAP_DEBUG) << "Validation failed for dialogue #" << _dialogue_id
				<< ": dialogue referenced invalid sprite with id: " << *i << endl;
			return false;
		}
	}

	for (uint32 i = 0; i < _line_events.size(); i++) {
		if (_line_events[i].ValidateEvents() == false) {
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// MapDialogueOptions Class Functions
///////////////////////////////////////////////////////////////////////////////

void MapDialogueOptions::AddOption(string text) {
	AddOption(text, COMMON_DIALOGUE_NEXT_LINE);
}



void MapDialogueOptions::AddOption(string text, int32 next_line) {
	CommonDialogueOptions::AddOption(text, next_line);
	_option_events.push_back(MapDialogueEventData());
}



void MapDialogueOptions::AddOptionEvent(uint32 event_id) {
	AddOptionEvent(event_id, 0);
}



void MapDialogueOptions::AddOptionEvent(uint32 event_id, uint32 delay_ms) {
	if (_text.empty() == true) {
		IF_PRINT_WARNING(MAP_DEBUG) << "Attempted to add an option event when no options were available" << endl;
		return;
	}

	_option_events[_option_events.size() - 1].AddEvent(event_id, false, delay_ms);
}

///////////////////////////////////////////////////////////////////////////////
// DialogueSupervisor Class Functions
///////////////////////////////////////////////////////////////////////////////

DialogueSupervisor::DialogueSupervisor() :
	_state(DIALOGUE_STATE_INACTIVE),
	_current_dialogue(NULL),
	_current_options(NULL),
	_line_timer(),
	_line_counter(0),
	_dialogue_window()
{
	_dialogue_window.SetPosition(512.0f, 760.0f);
}



DialogueSupervisor::~DialogueSupervisor() {
	_current_dialogue = NULL;
	_current_options = NULL;

	// Delete all dialogues
	for (map<uint32, MapDialogue*>::iterator i = _dialogues.begin(); i != _dialogues.end(); i++) {
		delete i->second;
	}
	_dialogues.clear();
}



void DialogueSupervisor::Update() {
	if (_current_dialogue == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted to update when no dialogue was active" << endl;
		return;
	}

	_line_timer.Update();

	switch (_state) {
		case DIALOGUE_STATE_LINE:
			_UpdateLine();
			break;
		case DIALOGUE_STATE_OPTION:
			_UpdateOptions();
			break;
		default:
			IF_PRINT_WARNING(MAP_DEBUG) << "dialogue supervisor was in an unknown state: " << _state << endl;
			_state = DIALOGUE_STATE_LINE;
			break;
	}
}



void DialogueSupervisor::Draw() {
	_dialogue_window.Draw();
}



void DialogueSupervisor::RegisterDialogue(MapDialogue* dialogue) {
	if (dialogue == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function received NULL argument" << endl;
		return;
	}

	if (GetDialogue(dialogue->GetDialogueID()) != NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "a dialogue was already registered with this ID: " << dialogue->GetDialogueID() << endl;
		delete dialogue;
		return;
	}
	else {
		_dialogues.insert(make_pair(dialogue->GetDialogueID(), dialogue));
	}
}



void DialogueSupervisor::BeginDialogue(uint32 dialogue_id) {
	MapDialogue* dialogue = GetDialogue(dialogue_id);

	if (dialogue == NULL) {
		IF_PRINT_WARNING(COMMON_DEBUG) << "could not begin dialogue because none existed for id# " << dialogue_id << endl;
		return;
	}

	if (_current_dialogue != NULL) {
		IF_PRINT_WARNING(COMMON_DEBUG) << "beginning a new dialogue while another dialogue is still active" << endl;
	}

	_line_counter = 0;
	_current_dialogue = dialogue;
	_BeginLine();
	MapMode::CurrentInstance()->PushState(STATE_DIALOGUE);
}



void DialogueSupervisor::EndDialogue() {
	if (_current_dialogue == NULL) {
		IF_PRINT_WARNING(COMMON_DEBUG) << "tried to end a dialogue when there was no dialogue active" << endl;
		return;
	}

	_current_dialogue->IncrementTimesSeen();
	if (MapMode::CurrentInstance()->GetMapEventGroup()->DoesEventExist(_current_dialogue->GetDialogueName()) == false) {
		MapMode::CurrentInstance()->GetMapEventGroup()->AddNewEvent(_current_dialogue->GetDialogueName(), _current_dialogue->GetTimesSeen());
	}
	else {
		MapMode::CurrentInstance()->GetMapEventGroup()->SetEvent(_current_dialogue->GetDialogueName(), _current_dialogue->GetTimesSeen());
	}

	// We only want to call the RestoreState function *once* for each speaker, so first we have to construct a list of pointers
	// for all speakers without duplication (i.e. the case where a speaker spoke more than one line of dialogue).

	// Get a unique set of all sprites that participated in the dialogue
	set<MapSprite*> speakers;
	for (uint32 i = 0; i < _current_dialogue->GetLineCount(); i++) {
		if(_current_dialogue->GetLineSpeaker(i) != NO_SPRITE) {
			speakers.insert(dynamic_cast<MapSprite*>(MapMode::CurrentInstance()->GetObjectSupervisor()->GetObject(_current_dialogue->GetLineSpeaker(i))));
		}else {
			speakers.insert(NULL);
		}
			}

	for (set<MapSprite*>::iterator i = speakers.begin(); i != speakers.end(); i++) {
		// Each sprite needs to know that this dialogue completed so that they can update their data accordingly
		if(*i != NULL) {
			(*i)->UpdateDialogueStatus();

			// Restore the state (orientation, animation, etc.) of all speaker sprites if necessary
			if (_current_dialogue->IsRestoreState() == true) {
				if ((*i)->IsStateSaved() == true)
					(*i)->RestoreState();
			}
		}
	}

	_current_dialogue = NULL;
	_current_options = NULL;
	MapMode::CurrentInstance()->PopState();
}



MapDialogue* DialogueSupervisor::GetDialogue(uint32 dialogue_id) {
	map<uint32, MapDialogue*>::iterator location = _dialogues.find(dialogue_id);
	if (location == _dialogues.end()) {
		return NULL;
	}
	else {
		return location->second;
	}
}



void DialogueSupervisor::_UpdateLine() {
	_dialogue_window.GetDisplayTextBox().Update();

	// If the line has a valid display time and the timer is finished, move on to the next line
	if ((_line_timer.GetDuration() > 0) && (_line_timer.IsFinished() == true)) {
		_EndLine();
		return;
	}

	// Set the correct indicator
	if (_current_dialogue->IsInputBlocked() || _current_options != NULL || _dialogue_window.GetDisplayTextBox().IsFinished() == false) {
		_dialogue_window.SetIndicator(COMMON_DIALOGUE_NO_INDICATOR);
	}
	else if (_line_counter == _current_dialogue->GetLineCount()-1) {
		_dialogue_window.SetIndicator(COMMON_DIALOGUE_LAST_INDICATOR);
	}
	else {
		_dialogue_window.SetIndicator(COMMON_DIALOGUE_NEXT_INDICATOR);
	}

	// If this dialogue does not allow user input, we are finished
	if (_current_dialogue->IsInputBlocked() == true) {
		return;
	}

	if (InputManager->ConfirmPress()) {
		// If the line is not yet finished displaying, display the rest of the text
		if (_dialogue_window.GetDisplayTextBox().IsFinished() == false) {
			_dialogue_window.GetDisplayTextBox().ForceFinish();
		}
		// Proceed to option selection if the line has options
		else if (_current_options != NULL) {
			_state = DIALOGUE_STATE_OPTION;
		}
		else {
			_EndLine();
		}
	}
}



void DialogueSupervisor::_UpdateOptions() {
	if (InputManager->ConfirmPress()) {
		_dialogue_window.GetDisplayOptionBox().InputConfirm();
		_EndLine();
	}

	else if (InputManager->UpPress()) {
		_dialogue_window.GetDisplayOptionBox().InputUp();
	}

	else if (InputManager->DownPress()) {
		_dialogue_window.GetDisplayOptionBox().InputDown();
	}
}



void DialogueSupervisor::_BeginLine() {
	_state = DIALOGUE_STATE_LINE;
	_current_options = dynamic_cast<MapDialogueOptions*>(_current_dialogue->GetLineOptions(_line_counter));

	// Execute any map events that should occur when this line of dialogue has started
	MapDialogueEventData* events = _current_dialogue->GetLineEventData(_line_counter);
	if (events != NULL) {
		events->ProcessEvents(true);
	}

	// Initialize the line timer
	if (_current_dialogue->GetLineDisplayTime(_line_counter) >= 0) {
		_line_timer.Initialize(_current_dialogue->GetLineDisplayTime(_line_counter));
		_line_timer.Run();
	}
	// If the line has no timer specified, set the line time to zero and put the timer in the finished state
	else {
		_line_timer.Initialize(0);
		_line_timer.Finish();
	}

	// Setup the text and graphics for the dialogue window
	_dialogue_window.Clear();
	_dialogue_window.GetDisplayTextBox().SetDisplayText(_current_dialogue->GetLineText(_line_counter));

	if (_current_options != NULL) {
		for (uint32 i = 0; i < _current_options->GetNumberOptions(); i++) {
			_dialogue_window.GetDisplayOptionBox().AddOption(_current_options->GetOptionText(i));
		}

		_dialogue_window.GetDisplayOptionBox().SetSelection(0);
	}

	// Executes normal code if the speaker is an actual sprite i.e speaker id is not NO_SPRITE
	if(_current_dialogue->GetLineSpeaker(_line_counter) != NO_SPRITE) {
		MapObject* object = MapMode::CurrentInstance()->GetObjectSupervisor()->GetObject(_current_dialogue->GetLineSpeaker(_line_counter));
		if (object == NULL) {
			IF_PRINT_WARNING(MAP_DEBUG) << "dialogue #" << _current_dialogue->GetDialogueID()
				<< " referenced a sprite that did not exist with id: " << _current_dialogue->GetLineSpeaker(_line_counter) << endl;
			return;
		}
		else if (object->GetType() != SPRITE_TYPE) {
			IF_PRINT_WARNING(MAP_DEBUG) << "dialogue #" << _current_dialogue->GetDialogueID()
				<< " referenced a map object which was not a sprite with id: " << _current_dialogue->GetLineSpeaker(_line_counter) << endl;
			return;
		}
		else {
			MapSprite* speaker = dynamic_cast<MapSprite*>(object);
			_dialogue_window.GetNameText().SetText(speaker->GetName());
			_dialogue_window.SetPortraitImage(speaker->GetFacePortrait());
		}
	}else {
		_dialogue_window.GetNameText().SetText("");
		_dialogue_window.SetPortraitImage(NULL);
	}


}



void DialogueSupervisor::_EndLine() {
	// Execute any map events that should occur after this line of dialogue has finished
	MapDialogueEventData* line_events = _current_dialogue->GetLineEventData(_line_counter);
	if (line_events != NULL) {
		line_events->ProcessEvents(false);
	}

	// Execute any map events that were tied to the selected dialogue option
	if (_current_options != NULL) {
		uint32 selected_option = _dialogue_window.GetDisplayOptionBox().GetSelection();
		MapDialogueEventData* option_events = _current_options->GetOptionEventData(selected_option);
		if (option_events != NULL) {
			option_events->ProcessEvents(false);
		}
	}

	// Determine the next line to read
	int32 next_line = _current_dialogue->GetLineNextLine(_line_counter);
	// If this line had options, the selected option next line overrides the line's next line that we set above
	if (_current_options != NULL) {
		uint32 selected_option = _dialogue_window.GetDisplayOptionBox().GetSelection();
		next_line = _current_options->GetOptionNextLine(selected_option);
	}

	// --- Case 1: Explicitly setting the next line. Warn and end the dialogue if the line to move to is invalid
	if (next_line >= 0) {
		if (static_cast<uint32>(next_line) >= _current_dialogue->GetLineCount()) {
			IF_PRINT_WARNING(MAP_DEBUG) << "dialogue #" << _current_dialogue->GetDialogueID()
				<< " tried to set dialogue to invalid line. Current/next line values: {" << _line_counter
				<< ", " << next_line << "}" << endl;
			next_line = COMMON_DIALOGUE_END;
		}
	}
	// --- Case 2: Request to incrementing the current line. If we're incrementing past the last line, end the dialogue
	else if (next_line == COMMON_DIALOGUE_NEXT_LINE) {
		next_line = _line_counter + 1;
		if (static_cast<uint32>(next_line) >= _current_dialogue->GetLineCount())
			next_line = COMMON_DIALOGUE_END;
	}
	// --- Case 3: Request to end the current dialogue
	else if (next_line == COMMON_DIALOGUE_END) {
		// Do nothing
	}
	// --- Case 4: Unknown negative value. Warn and end dialogue
	else {
		IF_PRINT_WARNING(MAP_DEBUG) << "dialogue #" << _current_dialogue->GetDialogueID()
			<< " unknown next line control value: " << next_line << endl;
		next_line = COMMON_DIALOGUE_END;
	}

	// Now either end the dialogue or move on to the next line
	if (next_line == COMMON_DIALOGUE_END) {
		EndDialogue();
	}
	else {
		_line_counter = next_line;
		_BeginLine();
	}
}

} // namespace private_map

} // namespace hoa_map
