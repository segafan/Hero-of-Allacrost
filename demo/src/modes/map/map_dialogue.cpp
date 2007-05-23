///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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

#include "utils.h"
#include "map.h"
#include "map_objects.h"
#include "map_dialogue.h"
#include "audio.h"
#include "video.h"
#include "input.h"
#include "script.h"
#include "global.h"
#include "menu.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_input;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_menu;
using namespace luabind;

namespace hoa_map {

namespace private_map {

// ****************************************************************************
// *********************** MapDialogue Class Functions ************************
// ****************************************************************************

DialogueManager::DialogueManager() {
	VideoManager->PushState();
	VideoManager->SetCoordSys(0, 1024, 768, 0);
// 	MenuWindow::Create(1024.0f, 256.0f);
// 	MenuWindow::SetPosition(0.0f, 512.0f);
// 	MenuWindow::SetDisplayMode(VIDEO_MENU_EXPAND_FROM_CENTER);

	_background_image.SetFilename("img/menus/dialogue_box.png");
	if (_background_image.Load() == false)
		cerr << "MAP ERROR: failed to load image: " << _background_image.GetFilename() << endl;

	_nameplate_image.SetFilename("img/menus/dialogue_nameplate.png");
	if (_nameplate_image.Load() == false)
		cerr << "MAP ERROR: failed to load image: " << _nameplate_image.GetFilename() << endl;

	_display_textbox.SetDisplaySpeed(30);
	_display_textbox.SetPosition(300.0f, 768.0f - 180.0f);
	_display_textbox.SetDimensions(1024.0f - 300.0f - 60.0f, 180.0f - 70.0f);
	_display_textbox.SetFont("map");
	_display_textbox.SetTextColor(Color::black);
	_display_textbox.SetDisplayMode(VIDEO_TEXT_FADECHAR);
	_display_textbox.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);

	VideoManager->PopState();
}


DialogueManager::~DialogueManager() {
// 	MenuWindow::Destroy();
//_display_textbox.SetDisplayText(_current_dialogue->GetLine());
}


void DialogueManager::Update() {
	static int32 time_remaining = 0;          // The time that remains for the display of the current line
	static MapDialogue* last_dialogue = NULL; // Used to detect if this is the first update to a new piece of dialogue
	bool finish_line = false;                 // When set to true, indicates that the current line of dialogue is finished

	if (_current_dialogue == NULL)
		return;

	if (_current_dialogue != last_dialogue) {
		time_remaining = _current_dialogue->GetCurrentTime();
		_display_textbox.SetDisplayText(_current_dialogue->GetCurrentText());
		last_dialogue = _current_dialogue;
	}

	_display_textbox.Update(MapMode::_current_map->_time_elapsed);

	// Update the dialogue timer
	if (time_remaining > 0) {
		time_remaining -= MapMode::_current_map->_time_elapsed;
		// If it get below 0, clip it to 0 as -1 means infinite
		if (time_remaining < 0) {
			time_remaining = 0;
			finish_line = true;
		}
	}

	// Check for user input only if this dialogue is non-blocking
	if (_current_dialogue->IsBlocked() == false) {
		if (InputManager->ConfirmPress()) {
			// If the line is not yet finished displaying, display the rest of the text
			if (!_display_textbox.IsFinished()) {
				_display_textbox.ForceFinish();
			}
			// Otherwise, finish this line
			else {
				finish_line = true;
			}
		}
	}

	// If the line has been finished, process the post-line action if it exists and move on to the next line
	if (finish_line == true) {
		if (_current_dialogue->GetCurrentAction() != NULL) {
			try {
				ScriptCallFunction<void>(*(_current_dialogue->GetCurrentAction()));
			} catch (luabind::error& e) {
				ScriptManager->HandleLuaError(e);
			}
		}
	
		// Move to the next line of dialogue
		if (_current_dialogue->ReadNextLine() == true) {
			time_remaining = _current_dialogue->GetCurrentTime();
			_display_textbox.SetDisplayText(_current_dialogue->GetCurrentText());
		}

		// This dialogue is finished, restore game state as necessary
		else {
			// The is no more line, the dialogue is over
			MapMode::_current_map->_map_state = EXPLORE;
			// Restore the status of the map sprites
			if (_current_dialogue->IsSaving()) {
				for (uint32 i = 0; i < _current_dialogue->GetNumLines(); i++) {
					static_cast<VirtualSprite*>(MapMode::_current_map->_all_objects[_current_dialogue->GetLineSpeaker(i)])->LoadState();
				}
			}
			_current_dialogue = NULL;
			last_dialogue = NULL;
		}
	}
} // void DialogueManager::Update()



void DialogueManager::Draw() {
	VideoManager->PushState();
	VideoManager->SetCoordSys(0.0f, 1024.0f, 768.0f, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->Move(0.0f, 768.0f);
	_background_image.Draw();
	VideoManager->MoveRelative(47.0f, -42.0f);
	_nameplate_image.Draw();

	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, 0);
	VideoManager->SetFont("map");
	VideoManager->SetTextColor(Color(Color::black));
	VideoManager->MoveRelative(120.0f, -10.0f);
	VirtualSprite* speaker = reinterpret_cast<VirtualSprite*>(MapMode::_current_map->_all_objects[_current_dialogue->GetCurrentSpeaker()]);
	VideoManager->DrawText(speaker->name);
	if (speaker->face_portrait != NULL) {
		VideoManager->MoveRelative(0.0f, -26.0f);
		speaker->face_portrait->Draw();
	}
	_display_textbox.Draw();
	VideoManager->PopState();
} // void DialogueManager::Draw()

// ****************************************************************************
// *********************** MapDialogue Class Functions ************************
// ****************************************************************************

MapDialogue::MapDialogue(bool save_state) :
	_seen(0),
	_current_line(0),
	_blocked(false)
{
	_save_state = save_state;
}



MapDialogue::~MapDialogue() {
	for (uint32 i = 0; i < _actions.size(); ++i) {
		if (_actions[i] != NULL) {
			delete _actions[i];
		}
	}
}



bool MapDialogue::ReadNextLine() {
	// Determine if the dialogue is finished
	if (++_current_line >= _text.size()) {
		_current_line = 0;
		IncrementTimesSeen();
		if (_owner != NULL) {
			_owner->UpdateSeenDialogue();
		}
		return false;
	}
	// Return true if the dialogue has additional lines to read
	return true;
}



void MapDialogue::AddText(std::string text, uint32 speaker_id, int32 time, int32 action) {
	_text.push_back(MakeUnicodeString(text));
	_speakers.push_back(speaker_id);
	_time.push_back(time);
	if (action >= 0) {
		MapMode::_loading_map->_map_script.ReadOpenTable("map_functions");
		ScriptObject* new_action = new ScriptObject();
		*new_action = MapMode::_loading_map->_map_script.ReadFunctionPointer(action);
		MapMode::_loading_map->_map_script.ReadCloseTable();
		_actions.push_back(new_action);
	}
	else {
		_actions.push_back(NULL);
	}
}

} // namespace private_map

} // namespace hoa_map
