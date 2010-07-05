///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
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

// Allacrost globals
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

namespace hoa_map {

namespace private_map {

///////////////////////////////////////////////////////////////////////////////
// MapDialogue Class Functions
///////////////////////////////////////////////////////////////////////////////

MapDialogue::MapDialogue(uint32 id) :
	_dialogue_id(id),
	_times_seen(0),
	_max_views(-1),
	_line_count(0),
	_current_line(0),
	_blocked(false),
	_save_state(true),
	_event_name("")
{
	// Look up the event for this dialogue to see whether it has already been read before or not
	// Either create the event or retrieve the number of times the dialogue has been seen.
	_event_name = GetEventName();
	GlobalEventGroup& event_group = *(MapMode::CurrentInstance()->GetMapEventGroup());

	if (event_group.DoesEventExist(_event_name) == false) {
		event_group.AddNewEvent(_event_name, 0);
	}
	else {
		SetTimesSeen(event_group.GetEvent(_event_name));
	}
}


MapDialogue::~MapDialogue() {
	for (uint32 i = 0; i < _options.size(); i++) {
		if (_options[i] != NULL) {
			delete _options[i];
		}
	}
}



void MapDialogue::AddText(std::string text, uint32 speaker_id, int32 next_line, uint32 event, bool display_timer) {
	_text.push_back(MakeUnicodeString(text));
	_speakers.push_back(speaker_id);
	_next_lines.push_back(next_line);
	_options.push_back(NULL);
	_events.push_back(event);
	_line_count++;

	if (display_timer == true) {
		// TODO: replace 5000 with a function call that will calculate the display time based on text length and player's speed setting
		_display_times.push_back(5000);
	}
	else {
		_display_times.push_back(-1);
	}
}



void MapDialogue::AddOption(string text, int32 next_line, uint32 event) {
	int32 current_line = _line_count - 1; // Current line that options will belong to.

	// If the line the options will be added to currently has no options, create a new instance of the MapDialogueOptions class to store the options in.
	if (_options[current_line] == NULL) {
		MapDialogueOptions* option = new MapDialogueOptions();
		_options[current_line] = option;
	}

	_options[current_line]->AddOption(MakeUnicodeString(text), next_line, event);
}



bool MapDialogue::ReadNextLine(int32 line) {
	bool end_dialogue = false;

	// If argument is negative, there is no next line to read so end the dialogue
	if (line < 0) {
		end_dialogue = true;
	}
	// End the dialogue in this case as well to avoid crashing
	else if (line >= static_cast<int32>(_line_count)) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function argument exceeded number of lines in dialogue: " << line << endl;
		end_dialogue = true;
	}
	else {
		_current_line = line;
	}

	if (end_dialogue == true) {
		_current_line = 0;
		IncrementTimesSeen();
		MapMode::CurrentInstance()->GetMapEventGroup()->SetEvent(_event_name, _times_seen);
		return false;
	}
	else {
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
// MapDialogueOptions Functions
///////////////////////////////////////////////////////////////////////////////

void MapDialogueOptions::AddOption(ustring text, int32 next_line, uint32 event) {
	if (_text.size() >= MAX_OPTIONS) {
		IF_PRINT_WARNING(MAP_DEBUG) << "dialogue option box already contains too many options. The new option will not be added." << endl;
		return;
	}

	_text.push_back(text);
	_next_lines.push_back(next_line);
	_events.push_back(event);
}

///////////////////////////////////////////////////////////////////////////////
// DialogueWindow class methods
///////////////////////////////////////////////////////////////////////////////

DialogueWindow::DialogueWindow() {
	if (_parchment_image.Load("img/menus/black_sleet_parch.png") == false)
		cerr << "MAP ERROR: failed to load image: " << _parchment_image.GetFilename() << endl;

	if (_nameplate_image.Load("img/menus/dialogue_nameplate.png") == false)
		cerr << "MAP ERROR: failed to load image: " << _nameplate_image.GetFilename() << endl;

	VideoManager->PushState();
	VideoManager->SetCoordSys(0, 1024, 768, 0);

	_display_textbox.SetDisplaySpeed(30);
	_display_textbox.SetPosition(260.0f, 596.0f);
	_display_textbox.SetDimensions(700.0f, 126.0f);
	_display_textbox.SetTextStyle(TextStyle("text20", Color::black, VIDEO_TEXT_SHADOW_LIGHT));
	_display_textbox.SetDisplayMode(VIDEO_TEXT_FADECHAR);
	_display_textbox.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_display_textbox.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);

	_display_options.SetPosition(300.0f, 630.0f);
	_display_options.SetDimensions(660.0f, 90.0f, 1, 255, 1, 3);
	_display_options.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_display_options.SetTextStyle(TextStyle("title20", Color::black, VIDEO_TEXT_SHADOW_LIGHT));
	_display_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_display_options.SetCursorOffset(-55.0f, -25.0f);
	_display_options.SetVerticalWrapMode(VIDEO_WRAP_MODE_NONE);
	_display_options.SetSelection(0);

	VideoManager->PopState();
}



DialogueWindow::~DialogueWindow() {
	MenuWindow::Destroy();
}



void DialogueWindow::Initialize() {
	// FIXME: Should this be here?  I don't think so, but if it does, get it working properly.
	// currently, it does nothing except flood the debug output!
//	MenuWindow::Show();
}



void DialogueWindow::Reset() {
//	MenuWindow::Hide();
	_display_textbox.ClearText();
	_display_options.ClearOptions();
}



void DialogueWindow::Draw(ustring* name, StillImage* portrait) {
//	MenuWindow::Draw();

	// Temporarily change the coordinate system to 1024x768 and draw the contents of the dialogue window
	VideoManager->PushState();
	VideoManager->SetCoordSys(0.0f, 1024.0f, 768.0f, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);

	VideoManager->Move(18.0f, 744.0f);
	_parchment_image.Draw();

// 	VideoManager->Move(47.0f, 726.0f);
// 	if (name != NULL)
// 		_nameplate_image.Draw();

	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, 0);
	VideoManager->MoveRelative(120.0f, -20.0f);

	if (name != NULL)
		VideoManager->Text()->Draw(*name, TextStyle("title22", Color::black, VIDEO_TEXT_SHADOW_LIGHT));

	if (portrait != NULL) {
		VideoManager->MoveRelative(0.0f, -20.0f);
		portrait->Draw();
	}

	_display_textbox.Draw();
	_display_options.Draw();

	VideoManager->PopState();
}

///////////////////////////////////////////////////////////////////////////////
// DialogueSupervisor class methods
///////////////////////////////////////////////////////////////////////////////

DialogueSupervisor::DialogueSupervisor() :
	_state(DIALOGUE_STATE_LINE),
	_current_dialogue(NULL),
	_current_options(NULL),
	_line_timer(-1),
	_dialogue_window()
{}



DialogueSupervisor::~DialogueSupervisor() {
	// Update the times seen count before deleting each dialogue
	for (map<uint32, MapDialogue*>::iterator i = _all_dialogues.begin(); i != _all_dialogues.end(); i++) {
		MapMode::CurrentInstance()->GetMapEventGroup()->SetEvent(i->second->GetEventName(), i->second->GetTimesSeen());
		delete i->second;
	}
	_all_dialogues.clear();
}



void DialogueSupervisor::AddDialogue(MapDialogue* dialogue) {
	if (dialogue == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function argument was NULL" << endl;
		return;
	}

	if (GetDialogue(dialogue->GetDialogueID()) != NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "a dialogue was already registered with this ID: " << dialogue->GetDialogueID() << endl;
		delete dialogue;
		return;
	}
	else {
		_all_dialogues.insert(make_pair(dialogue->GetDialogueID(), dialogue));
	}
}



void DialogueSupervisor::AddSpriteReference(uint32 dialogue_id, uint32 sprite_id) {
	map<uint32, vector<uint32> >::iterator entry = _sprite_references.find(dialogue_id);

	if (entry == _sprite_references.end()) {
		vector<uint32> new_entry(1, sprite_id);
		_sprite_references.insert(make_pair(dialogue_id, new_entry));
	}
	else {
		entry->second.push_back(sprite_id);
	}
}



void DialogueSupervisor::BeginDialogue(uint32 dialogue_id) {
	MapDialogue* dialogue = GetDialogue(dialogue_id);

	if (dialogue == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "could not begin dialogue because none existed for id# " << dialogue_id << endl;
		return;
	}

	if (_current_dialogue != NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "beginning a new dialogue while another dialogue is still active" << endl;
	}

	_current_dialogue = dialogue;
	_current_options = _current_dialogue->GetCurrentOptions();
	_line_timer = _current_dialogue->GetCurrentTime();
	_dialogue_window.Initialize();
	_dialogue_window._display_textbox.SetDisplayText(_current_dialogue->GetCurrentText());
	MapMode::CurrentInstance()->PushState(STATE_DIALOGUE);
}



void DialogueSupervisor::BeginDialogue(MapSprite* sprite) {
	if (sprite == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "NULL argument passed to function" << endl;
		return;
	}

	if (sprite->HasAvailableDialogue() == false) {
		IF_PRINT_WARNING(MAP_DEBUG) << "sprite argument had no available dialogue" << endl;
		return;
	}

	MapDialogue* next_dialogue = GetDialogue(sprite->GetNextDialogueID());
	if (next_dialogue == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "the next dialogue referenced by the sprite argument was invalid" << endl;
		return;
	}

	if (next_dialogue->IsAvailable() == false) {
		IF_PRINT_WARNING(MAP_DEBUG) << "the next dialogue referenced by the sprite was not available" << endl;
		return;
	}

	// Prepare the state of the sprite and map camera for the dialogue
	sprite->SaveState();
	sprite->moving = false;
	sprite->SetDirection(CalculateOppositeDirection(MapMode::CurrentInstance()->GetCamera()->GetDirection()));
	sprite->IncrementNextDialogue();
	// TODO: Is the line below necessary to do? Shouldn't the camera stop on its own (if its pointing to the player's character)?
	MapMode::CurrentInstance()->GetCamera()->moving = false;
	BeginDialogue(next_dialogue->GetDialogueID());
}



void DialogueSupervisor::EndDialogue() {
	if (_current_dialogue == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "tried to end a dialogue when none was active" << endl;
		return;
	}

	AnnounceDialogueUpdate(_current_dialogue->GetDialogueID());

	_dialogue_window.Reset();
	_current_dialogue = NULL;
	_current_options = NULL;
	_line_timer = -1;
	MapMode::CurrentInstance()->PopState();
}



MapDialogue* DialogueSupervisor::GetDialogue(uint32 dialogue_id) {
	if (_all_dialogues.find(dialogue_id) != _all_dialogues.end())
		return _all_dialogues[dialogue_id];
	else
		return NULL;
}



void DialogueSupervisor::AnnounceDialogueUpdate(uint32 dialogue_id) {
	map<uint32, vector<uint32> >::iterator entry = _sprite_references.find(dialogue_id);

	// Note that we don't print a warning if no entry was found, because the case where a dialogue exists
	// but is not referenced by any sprites is a valid one
	if (entry == _sprite_references.end())
		return;

	// Update the dialogue status of all sprites that reference this dialogue
	for (uint32 i = 0; i < entry->second.size(); i++) {
		MapSprite* referee = static_cast<MapSprite*>(MapMode::CurrentInstance()->GetObjectSupervisor()->GetObject(entry->second[i]));
		if (referee == NULL) {
			IF_PRINT_WARNING(MAP_DEBUG) << "map sprite: " << entry->second[i] << " references dialogue: " << dialogue_id << " but sprite object did not exist"<< endl;
		}
		else {
			referee->UpdateDialogueStatus();
		}
	}
}



void DialogueSupervisor::Update() {
	if (_current_dialogue == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted to update dialogue supervisor when no dialogue was active" << endl;
		return;
	}

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

	// FIXME: This is disabled to prevent problems where a dialogue is necessary or has other things attached.
	// For instance: in the opening map it was possible to cancel the dialogue and be stuck there.
	// Possible fix: advancing to a 'necessary part' of the dialogue
	// Possible fix: allowing dialogues to be specified as 'non-cancelable'
	if (0 && InputManager->CancelPress()) {
		_state = DIALOGUE_STATE_LINE;
		_RestoreSprites();
		EndDialogue();
	}

} // void DialogueSupervisor::Update()



void DialogueSupervisor::Draw() {
	if (_current_dialogue == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted to draw dialogue window when no dialogue was active" << endl;
		return;
	}

	// TODO: Check if speaker ID is 0 and if so, call Draw function with NULL arguments
	MapSprite* speaker = reinterpret_cast<MapSprite*>(MapMode::CurrentInstance()->GetObjectSupervisor()->GetObject(_current_dialogue->GetCurrentSpeaker()));
	_dialogue_window.Draw(&speaker->GetName(), speaker->GetFacePortrait());
} // void DialogueSupervisor::Draw()



void DialogueSupervisor::_UpdateLine() {
	_dialogue_window._display_textbox.Update();

	// TODO: there is potential for dead-lock here. Lines that have (or do not have) a display time, have player options,
	// and/or have the input blocking property set can cause a lock-up.

	// Update the display timer if it is enabled for this dialogue
	if (_line_timer > 0) {
		_line_timer -= SystemManager->GetUpdateTime();

		if (_line_timer <= 0) {
			if (_current_options != NULL) {
				_state = DIALOGUE_STATE_OPTION;
				_ConstructOptions();
			}
			else {
				_FinishLine(_current_dialogue->GetCurrentNextLine());
			}
		}
	}

	// If this dialogue does not allow user input, we are finished
	if (_current_dialogue->IsBlocked() == true)
		return;

	if (InputManager->ConfirmPress()) {
		// If the line is not yet finished displaying, display the rest of the text
		if (_dialogue_window._display_textbox.IsFinished() == false) {
			_dialogue_window._display_textbox.ForceFinish();
		}
		// Proceed to option selection if the line has options
		else if (_current_dialogue->CurrentLineHasOptions() == true) {
			_state = DIALOGUE_STATE_OPTION;
			_ConstructOptions();
		}
		else {
			_FinishLine(_current_dialogue->GetCurrentNextLine());
		}
	}

	// TODO: Handle cancel presses to allow backtracking through the dialogue
} // void DialogueSupervisor::_UpdateLine()



void DialogueSupervisor::_UpdateOptions() {
	_dialogue_window._display_options.Update();

	// Execute the event for the current selection if applicable, then return the next line of dialogue for this selection
	if (InputManager->ConfirmPress()) {
		_dialogue_window._display_options.InputConfirm();

		int32 selected_option = _dialogue_window._display_options.GetSelection();

		if (_current_options->_events[selected_option] != 0) {
			MapMode::CurrentInstance()->GetEventSupervisor()->StartEvent(_current_options->_events[selected_option]);
		}

		_FinishLine(_current_options->_next_lines[selected_option]);
	}

	// TODO: handle cancel press to return to previous lines

	else if (InputManager->UpPress()) {
		_dialogue_window._display_options.InputUp();
	}

	else if (InputManager->DownPress()) {
		_dialogue_window._display_options.InputDown();
	}
} // void DialogueSupervisor::_UpdateOptions()



void DialogueSupervisor::_ConstructOptions() {
	for (vector<ustring>::iterator i = _current_options->_text.begin(); i != _current_options->_text.end(); i++) {
		_dialogue_window._display_options.AddOption(*i);
	}
	_dialogue_window._display_options.SetSelection(0);
}



void DialogueSupervisor::_FinishLine(int32 next_line) {
	_dialogue_window._display_textbox.ClearText();
	_dialogue_window._display_options.ClearOptions();
	_state = DIALOGUE_STATE_LINE;

	// Execute any scripted events that should occur after this line of dialogue has finished
	if (_current_dialogue->GetCurrentEvent() != 0) {
		MapMode::CurrentInstance()->GetEventSupervisor()->StartEvent(_current_dialogue->GetCurrentEvent());
	}

	// Check if there are more lines of dialogue and continue on to the next line if available
	if (_current_dialogue->ReadNextLine(next_line) == true) {
		_current_options = _current_dialogue->GetCurrentOptions();
		_line_timer = _current_dialogue->GetCurrentTime();
		_dialogue_window._display_textbox.SetDisplayText(_current_dialogue->GetCurrentText());
		return;
	}

	// If this point in the function is reached, the last line of dialogue has ben read
	// Restore the status of the sprites that participated in this dialogue if necessary
	if (_current_dialogue->IsSaveState()) {
		_RestoreSprites();
	}

	EndDialogue();
} // void DialogueSupervisor::_FinishLine()



void DialogueSupervisor::_RestoreSprites() {
	// We only want to call the RestoreState function *once* for each speaker, so first we have to construct a list of pointers
	// for all speakers without duplication (i.e. the case where a speaker spoke more than one line of dialogue).

	set<MapSprite*> participants;
	for (uint32 i = 0; i < _current_dialogue->GetLineCount(); i++) {
		participants.insert(static_cast<MapSprite*>(MapMode::CurrentInstance()->GetObjectSupervisor()->GetObject(_current_dialogue->GetLineSpeaker(i))));
	}

	for (set<MapSprite*>::iterator i = participants.begin(); i != participants.end(); i++) {
		if ((*i)->IsStateSaved() == true)
			(*i)->RestoreState();
	}
}

} // namespace private_map

} // namespace hoa_map
