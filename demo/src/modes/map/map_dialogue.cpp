///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
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
#include "map_objects.h"
#include "map_sprites.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_input;
using namespace hoa_mode_manager;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_menu;

namespace hoa_map {

namespace private_map {

// ****************************************************************************
// ********** MapDialogue Class Functions
// ****************************************************************************

MapDialogue::MapDialogue(bool save_state) :
	_times_seen(0),
	_max_views(-1),
	_line_count(0),
	_current_line(0),
	_blocked(false),
	_save_state(save_state),
	_event_name(""),
	_owner(NULL)
{}


MapDialogue::~MapDialogue() {
	for (uint32 i = 0; i < _options.size(); i++ ) {
		if (_options[i] != NULL) {
			delete _options[i];
		}
	}

	for (uint32 i = 0; i < _actions.size(); ++i) {
		if (_actions[i] != NULL) {
			delete _actions[i];
		}
	}
}



void MapDialogue::AddText(std::string text, uint32 speaker_id, int32 time, int32 action) {
	_text.push_back(MakeUnicodeString(text));
	_speakers.push_back(speaker_id);
	_display_times.push_back(time);
	_next_lines.push_back(-1);
	_options.push_back(NULL);
	_line_count++;

	if (action >= 0) {
		MapMode::_loading_map->_map_script.OpenTable(MapMode::_loading_map->_map_tablespace, true);
		MapMode::_loading_map->_map_script.OpenTable("map_functions");

		ScriptObject* new_action = new ScriptObject();
		*new_action = MapMode::_loading_map->_map_script.ReadFunctionPointer(action);
		_actions.push_back(new_action);

		MapMode::_loading_map->_map_script.CloseTable();
		MapMode::_loading_map->_map_script.CloseTable();
	}
	else {
		_actions.push_back(NULL);
	}
}



void MapDialogue::AddOption(string text, int32 next_line, int32 action) {
	int32 current_line = _line_count - 1; // Current line that options will belong to.

	// If the line the options will be added to currently has no options, create a new instance of the MapDialogueOptions class to store the options in.
	if (_options[current_line] == NULL) {
		MapDialogueOptions* option = new MapDialogueOptions();
		_options[current_line] = option;
	}

	_options[current_line]->AddOption(MakeUnicodeString(text), next_line, action);
}



bool MapDialogue::ReadNextLine(int32 line) {
	bool ignore_argument = false;
	if (line >= 0) {
		if (line >= static_cast<int32>(_line_count)) {
			IF_PRINT_WARNING(MAP_DEBUG) << "function argument exceeded dialogue lines bound: " << line << endl;
			ignore_argument = true;
		}
		else {
			_current_line = line;
		}
	}
	else {
		ignore_argument = true;
	}

	if (ignore_argument == true) {
		if (_next_lines[_current_line] >= 0)
			_current_line = _next_lines[_current_line];
		else
			++_current_line;
	}

	// Determine if the dialogue is now finished
	if (_current_line >= _text.size()) {
		_current_line = 0;
		IncrementTimesSeen();
		MapMode::_current_map->_map_event_group->SetEvent(_event_name, _times_seen);

		if (_owner != NULL) {
			_owner->UpdateSeenDialogue();
			_owner->UpdateActiveDialogue();
		}
		return false;
	}
	else {
		return true;
	}
}

// ******************************************************************************
// ********** MapDialogueOptions Functions
// ******************************************************************************

MapDialogueOptions::~MapDialogueOptions() {
	for (uint32 i = 0; i < _actions.size(); i++) {
		if (_actions[i] != NULL)
			delete _actions[i];
	}
}



void MapDialogueOptions::AddOption(ustring text, int32 next_line, int32 action) {
	if (_text.size() >= MAX_OPTIONS) {
		IF_PRINT_WARNING(MAP_DEBUG) << "dialogue option box already contains too many options. The new option will not be added." << endl;
		return;
	}

	_text.push_back(text);
	_next_lines.push_back(next_line);

	if (action < 0) {
		_actions.push_back(NULL);
	}
	else {
		MapMode::_loading_map->_map_script.OpenTable(MapMode::_loading_map->_map_tablespace, true);
		MapMode::_loading_map->_map_script.OpenTable("map_functions");

		ScriptObject* new_action = new ScriptObject();
		*new_action = MapMode::_loading_map->_map_script.ReadFunctionPointer(action);
		_actions.push_back(new_action);

		MapMode::_loading_map->_map_script.CloseTable();
		MapMode::_loading_map->_map_script.CloseTable();
	}
}

// ****************************************************************************
// ********** DialogueWindow class methods
// ****************************************************************************

DialogueWindow::DialogueWindow() {
	if (_background_image.Load("img/menus/dialogue_box.png") == false)
		cerr << "MAP ERROR: failed to load image: " << _background_image.GetFilename() << endl;

	if (_nameplate_image.Load("img/menus/dialogue_nameplate.png") == false)
		cerr << "MAP ERROR: failed to load image: " << _nameplate_image.GetFilename() << endl;

	VideoManager->PushState();
	VideoManager->SetCoordSys(0, 1024, 768, 0);
// 	MenuWindow::Create(1024.0f, 256.0f);
// 	MenuWindow::SetPosition(0.0f, 512.0f);
// 	MenuWindow::SetDisplayMode(VIDEO_MENU_EXPAND_FROM_CENTER);

	_display_textbox.SetDisplaySpeed(30);
	_display_textbox.SetPosition(300.0f, 768.0f - 180.0f);
	_display_textbox.SetDimensions(1024.0f - 300.0f - 60.0f, 180.0f - 70.0f);
	_display_textbox.SetTextStyle(TextStyle("map", Color::black, VIDEO_TEXT_SHADOW_LIGHT));
	_display_textbox.SetDisplayMode(VIDEO_TEXT_FADECHAR);
	_display_textbox.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_display_textbox.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);

	_display_options.SetCellSize(500.0f, 25.0f);
	_display_options.SetSize(1, 4);
	_display_options.SetPosition(325.0f, 620.0f);
	_display_options.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_display_options.SetFont("map");
	_display_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_display_options.SetCursorOffset(-55.0f, -25.0f);
	_display_options.SetVerticalWrapMode(VIDEO_WRAP_MODE_NONE);
	_display_options.SetSelection(0);

	VideoManager->PopState();
}



DialogueWindow::~DialogueWindow() {
// 	MenuWindow::Destroy();
}



void DialogueWindow::Initialize() {
//	MenuWindow::Show();
}



void DialogueWindow::Reset() {
//	MenuWindow::Hide();
	_display_textbox.ClearText();
	_display_options.ClearOptions();
}



void DialogueWindow::Draw(ustring* name, StillImage* portrait) {
	// Temporarily change the coordinate system to 1024x768, then draw the dialogue background, nameplate, speaker's name, and speaker's face portrait
	VideoManager->PushState();
	VideoManager->SetCoordSys(0.0f, 1024.0f, 768.0f, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);

//	MenuWindow::Draw();

	VideoManager->Move(0.0f, 768.0f); // Bottom right corner of screen
	_background_image.Draw();

	VideoManager->MoveRelative(47.0f, -42.0f);
	if (name != NULL)
		_nameplate_image.Draw();

	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, 0);
	VideoManager->MoveRelative(120.0f, -10.0f);

	_display_textbox.Draw();
	_display_options.Draw();

	if (name != NULL)
		VideoManager->Text()->Draw(*name, TextStyle("map", Color::black, VIDEO_TEXT_SHADOW_LIGHT));

	if (portrait != NULL) {
		VideoManager->MoveRelative(0.0f, -26.0f);
		portrait->Draw();
	}
	VideoManager->PopState();
}

// ****************************************************************************
// ********** DialogueSupervisor class methods
// ****************************************************************************

DialogueSupervisor::DialogueSupervisor() :
	_state(DIALOGUE_STATE_LINE),
	_current_dialogue(NULL),
	_current_options(NULL),
	_line_timer(-1),
	_dialogue_window()
{}



DialogueSupervisor::~DialogueSupervisor() {
	for (map<uint32, MapDialogue*>::iterator i = _all_dialogues.begin(); i != _all_dialogues.end(); i++) {
		delete i->second;
	}
	_all_dialogues.clear();
}



void DialogueSupervisor::BeginDialogue(MapDialogue* dialogue) {
	if (dialogue == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function argument was NULL" << endl;
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
}



void DialogueSupervisor::EndDialogue() {
	_dialogue_window.Reset();
	_current_dialogue = NULL;
	_current_options = NULL;
	_line_timer = -1;
	MapMode::_current_map->_map_state = EXPLORE;
}



MapDialogue* DialogueSupervisor::GetDialogue(uint32 dialogue_id) {
	if (_all_dialogues.find(dialogue_id) != _all_dialogues.end())
		return _all_dialogues[dialogue_id];
	else
		return NULL;
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
} // void DialogueSupervisor::Update()



void DialogueSupervisor::Draw() {
	if (_current_dialogue == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted to draw dialogue window when no dialogue was active" << endl;
		return;
	}

	// TODO: Check if speaker ID is 0 and if so, call Draw function with NULL arguments
	MapSprite* speaker = reinterpret_cast<MapSprite*>(MapMode::_current_map->_object_manager->GetObject(_current_dialogue->GetCurrentSpeaker()));
	_dialogue_window.Draw(&speaker->name, speaker->face_portrait);
} // void DialogueSupervisor::Draw()



void DialogueSupervisor::_UpdateLine() {
		_dialogue_window._display_textbox.Update();

		// TODO: there is potential for dead-lock here. Lines that have (or do not have) a display time, have player options,
		// and/or have the input blocking property set can cause a lock-up.

		// Update the display timer if it is enabled for this dialogue
		if (_line_timer > 0) {
			_line_timer -= MapMode::_current_map->_time_elapsed;
			
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

	// Execute any action for the current selection, then return the next line of dialogue for this selection
	if (InputManager->ConfirmPress()) {
		_dialogue_window._display_options.HandleConfirmKey();

		int32 selected_option = _dialogue_window._display_options.GetSelection();

		if (_current_options->_actions[selected_option] != NULL) {
			try {
				ScriptCallFunction<void>(*(_current_options->_actions[selected_option]));
			} catch (luabind::error& e) {
				ScriptManager->HandleLuaError(e);
			}
		}

		_FinishLine(_current_options->_next_lines[selected_option]);
	}

	// TODO: handle cancel press to return to previous lines

	else if (InputManager->UpPress()) {
		_dialogue_window._display_options.HandleUpKey();
	}

	else if (InputManager->DownPress()) {
		_dialogue_window._display_options.HandleDownKey();
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

	// Execute any scripted actions that should occur after this line of dialogue has finished
	if (_current_dialogue->GetCurrentAction() != NULL) {
		try {
			ScriptCallFunction<void>(*(_current_dialogue->GetCurrentAction()));
		} catch (luabind::error& e) {
			ScriptManager->HandleLuaError(e);
		}
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
		// We only want to call the RestoreState function *once* for each speaker, so first we have to construct a list of pointers
		// for all speakers without duplication (i.e. the case where a speaker spoke more than one line of dialogue).
		set<MapSprite*> participants;
		for (uint32 i = 0; i < _current_dialogue->GetLineCount(); i++) {
			participants.insert(static_cast<MapSprite*>(MapMode::_current_map->_object_manager->GetObject(_current_dialogue->GetLineSpeaker(i))));
		}

		for (set<MapSprite*>::iterator i = participants.begin(); i != participants.end(); i++) {
			if ((*i)->IsStateSaved() == true)
				(*i)->RestoreState();
		}
	}

	EndDialogue();
} // void DialogueSupervisor::_FinishLine()

} // namespace private_map

} // namespace hoa_map
