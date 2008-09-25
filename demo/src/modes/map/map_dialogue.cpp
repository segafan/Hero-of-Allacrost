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

	// If the line the options will be added to currently has no options, create a new instance of the DialogueOptionBox class to store the options in.
	if (_options[current_line] == NULL) {
		DialogueOptionBox* option = new DialogueOptionBox();
		_options[current_line] = option;
	}

	_options[current_line]->AddOption(text, next_line, action);
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
// ********** DialogueOptionBox Functions
// ******************************************************************************

DialogueOptionBox::DialogueOptionBox() {
	// Initialize the option box attributes.
	_options.SetCellSize(500.0f, 25.0f);
	_options.SetSize(1, 4);
	_options.SetPosition(325.0f, 620.0f);
	_options.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_options.SetFont("map");
	_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_options.SetCursorOffset(-55.0f, -25.0f);
	_options.SetVerticalWrapMode(VIDEO_WRAP_MODE_NONE);
	_options.SetSelection(0);
}



DialogueOptionBox::~DialogueOptionBox() {
	for (uint32 i = 0; i < _actions.size(); i++) {
		if (_actions[i] != NULL)
			delete _actions[i];
	}
}



void DialogueOptionBox::AddOption(string text, int32 next_line, int32 action) {
	// TODO: we need to figure out what is the real max limit number of options we can have, make a constant for it, and use that here
	if (_options.GetNumberOptions() >= 4) {
		IF_PRINT_WARNING(MAP_DEBUG) << "dialogue option box already contains too many options. The new option will not be added." << endl;
		return;
	}

	_options.AddOption(MakeUnicodeString(text));
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

	// TODO: removing this line causes no option to be selected (even though the constructor set the selection to zero). This is a problem with
	// the option box class that needs to be fixed, and then this line should be removed
	_options.SetSelection(0);
}



int32 DialogueOptionBox::Update() {
	_options.Update();

	// Execute any action for the current selection, then return the next line of dialogue for this selection
	if (InputManager->ConfirmPress()) {
		_options.HandleConfirmKey();

		if (_actions[_options.GetSelection()] != NULL) {
			try {
				ScriptCallFunction<void>(*(_actions[_options.GetSelection()]));
			} catch (luabind::error& e) {
				ScriptManager->HandleLuaError(e);
			}
		}

		int32 next_line = _next_lines[_options.GetSelection()];
		_options.SetSelection(0); // Reset the selection to the top option
		return next_line;
	}

	if (InputManager->UpPress()) {
		_options.HandleUpKey();
	}

	if (InputManager->DownPress()) {
		_options.HandleDownKey();
	}

	return -1;
}



void DialogueOptionBox::Draw() {
	_options.Draw();
}

// ****************************************************************************
// ***** DialogueWindow class methods
// ****************************************************************************

DialogueWindow::DialogueWindow() :
	_state(DIALOGUE_STATE_NORMAL),
	_current_dialogue(NULL),
	_current_options(NULL),
	_display_time(0)
{
	VideoManager->PushState();
	VideoManager->SetCoordSys(0, 1024, 768, 0);
// 	MenuWindow::Create(1024.0f, 256.0f);
// 	MenuWindow::SetPosition(0.0f, 512.0f);
// 	MenuWindow::SetDisplayMode(VIDEO_MENU_EXPAND_FROM_CENTER);

	if (_background_image.Load("img/menus/dialogue_box.png") == false)
		cerr << "MAP ERROR: failed to load image: " << _background_image.GetFilename() << endl;

	if (_nameplate_image.Load("img/menus/dialogue_nameplate.png") == false)
		cerr << "MAP ERROR: failed to load image: " << _nameplate_image.GetFilename() << endl;

	_display_textbox.SetDisplaySpeed(30);
	_display_textbox.SetPosition(300.0f, 768.0f - 180.0f);
	_display_textbox.SetDimensions(1024.0f - 300.0f - 60.0f, 180.0f - 70.0f);
	_display_textbox.SetTextStyle(TextStyle("map", Color::black, VIDEO_TEXT_SHADOW_LIGHT));
	_display_textbox.SetDisplayMode(VIDEO_TEXT_FADECHAR);
	_display_textbox.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_display_textbox.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);

	VideoManager->PopState();
}


DialogueWindow::~DialogueWindow() {
// 	MenuWindow::Destroy();
//_display_textbox.SetDisplayText(_current_dialogue->GetLine());
}



void DialogueWindow::BeginDialogue(MapDialogue* dialogue) {
	if (dialogue == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function argument was NULL" << endl;
		return;
	}

	if (_current_dialogue != NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "beginning a new dialogue while another dialogue is still active" << endl;
	}

	_current_dialogue = dialogue;
	_current_options = NULL; // This will get set to appropriate dialogue options in the Update method
	_display_time = _current_dialogue->GetCurrentTime();
	_display_textbox.SetDisplayText(_current_dialogue->GetCurrentText());
}



void DialogueWindow::EndDialogue() {
	_current_dialogue = NULL;
	_current_options = NULL;
	_display_time = -1;
	MapMode::_current_map->_map_state = EXPLORE;
}



void DialogueWindow::Update() {
	bool line_finished = false; // When set to true, indicates to move on to the next line of dialogue
	int32 next_line = -1; // Stores an index to the next line of dialogue to be displayed.

	if (_current_dialogue == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted to update dialogue manager when no dialogue was active" << endl;
		return;
	}

	if (_state == DIALOGUE_STATE_NORMAL) {
		_display_textbox.Update(MapMode::_current_map->_time_elapsed);

		// TODO: there is potential for dead-lock here. Lines that have (or do not have) a display time, have player options,
		// and/or have the input blocking property set can cause a lock-up.

		// Update the display timer if it is enabled for this dialogue
		if (_display_time > 0) {
			_display_time -= MapMode::_current_map->_time_elapsed;
			
			if (_display_time <= 0) {
				if (_current_dialogue->CurrentLineHasOptions() == true) {
					_current_options = _current_dialogue->GetCurrentOptions();
					_state = DIALOGUE_STATE_OPTION;
				}
				else {
					line_finished = true;
					next_line = _current_dialogue->GetCurrentNextLine();
				}
			}
		}
		
		// Check for user input only if this dialogue is non-blocking
		if (_current_dialogue->IsBlocked() == false) {
			if (InputManager->ConfirmPress()) {
				// If the line is not yet finished displaying, display the rest of the text
				if (_display_textbox.IsFinished() == false) {
					_display_textbox.ForceFinish();
				}
				// Proceed to option selection if the line has options
				else if (_current_dialogue->CurrentLineHasOptions() == true) {
					_current_options = _current_dialogue->GetCurrentOptions();
					_state = DIALOGUE_STATE_OPTION;
				}
				else {
					line_finished = true;
					next_line = _current_dialogue->GetCurrentNextLine();
				}
			}
		}

		// TODO: Handle cancel presses to allow backtracking through the dialogue
	}

	else if (_state == DIALOGUE_STATE_OPTION) {
		// If the update function returns a non-negative number, a selection has been made
		next_line = _current_options->Update();
		if (next_line >= 0)
			line_finished = true;
	}

	// If the line has finished, process the post-line action if it exists and move on to the next line
	if (line_finished == true) {
		_state = DIALOGUE_STATE_NORMAL;

		// Execute any scripted actions that should occur after this line of dialogue has finished
		if (_current_dialogue->GetCurrentAction() != NULL) {
			try {
				ScriptCallFunction<void>(*(_current_dialogue->GetCurrentAction()));
			} catch (luabind::error& e) {
				ScriptManager->HandleLuaError(e);
			}
		}

		// If there are more lines of dialogue, continue on to the next line
		if (_current_dialogue->ReadNextLine(next_line) == true) {
			_display_time = _current_dialogue->GetCurrentTime();
			_display_textbox.SetDisplayText(_current_dialogue->GetCurrentText());
			_state = DIALOGUE_STATE_NORMAL;
		}
		// The last line of dialogue has ben read. Restore all necessary game state data
		else {
			// Restore the status of the sprites that participated in this dialogue
			if (_current_dialogue->IsSaveState()) {
				for (uint32 i = 0; i < _current_dialogue->GetLineCount(); i++) {
					static_cast<VirtualSprite*>(MapMode::_current_map->_object_manager->GetObject(_current_dialogue->GetLineSpeaker(i)))->RestoreState();
				}
			}
			EndDialogue();
		}
	}
} // void DialogueWindow::Update()



void DialogueWindow::Draw() {
	// Temporarily change the coordinate system to 1024x768, then draw the dialogue background, nameplate, speaker's name, and speaker's face portrait
	VideoManager->PushState();
	VideoManager->SetCoordSys(0.0f, 1024.0f, 768.0f, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);

	VideoManager->Move(0.0f, 768.0f);
	_background_image.Draw();
	VideoManager->MoveRelative(47.0f, -42.0f);
	_nameplate_image.Draw();

	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, 0);
	VideoManager->MoveRelative(120.0f, -10.0f);

	VirtualSprite* speaker = reinterpret_cast<VirtualSprite*>(MapMode::_current_map->_object_manager->GetObject(_current_dialogue->GetCurrentSpeaker()));
	if (_state == DIALOGUE_STATE_NORMAL) {
		_display_textbox.Draw();
	}
	else if (_state == DIALOGUE_STATE_OPTION) {
		_display_textbox.Draw();
		_current_options->Draw();
	}
	VideoManager->Text()->Draw(speaker->name, TextStyle("map", Color::black, VIDEO_TEXT_SHADOW_LIGHT));

	if (speaker->face_portrait != NULL) {
		VideoManager->MoveRelative(0.0f, -26.0f);
		speaker->face_portrait->Draw();
	}
	VideoManager->PopState();
} // void DialogueWindow::Draw()

} // namespace private_map

} // namespace hoa_map
