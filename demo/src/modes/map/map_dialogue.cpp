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
	_display_textbox.SetFont("default");
	_display_textbox.SetDisplayMode(VIDEO_TEXT_FADECHAR);
	_display_textbox.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);

	VideoManager->PopState();
}


DialogueManager::~DialogueManager() {
// 	MenuWindow::Destroy();
//_display_textbox.SetDisplayText(_current_dialogue->GetLine());
}


void DialogueManager::Update() {
	static int32 timeleft = 0; //Time left to the curent line
	static MapDialogue* last_dialogue = NULL; //Used to detect if the dialogue changed

	if (_current_dialogue == NULL)
		return;

	if (_current_dialogue != last_dialogue) {
		timeleft = _current_dialogue->LineTime();
		last_dialogue = _current_dialogue;
	}

	_display_textbox.Update(MapMode::_current_map->_time_elapsed);
	// Only update if it has some time left
	if (timeleft > 0) {
		timeleft -= MapMode::_current_map->_time_elapsed;
		// If it get below 0, clip it to 0 as -1 means infinite
		if (timeleft < 0)
			timeleft = 0;
	}

	// Get the actions
	std::vector<SpriteAction*>* actions = &_current_dialogue->GetActions();
	for (std::vector<SpriteAction*>::iterator i = actions->begin(); i != actions->end(); ++i) {
		// Note order is important here, check if the pointer is valid
		// then check if the action is not finished
		if ((*i) && !(*i)->IsFinished()) {
			//Unfinished, then execute the action.
			(*i)->Execute();
			//If the action is forced, return now to ignore user input.
			if( (*i)->IsForced() )
				return;
		}
	}

	// If the dialogue is blocked, ignore user input
	if (_current_dialogue->IsBlocked()) {
		if (timeleft <= 0) {
			if (_current_dialogue->ReadNextLine()) {
				// There is no time elft, change line
				timeleft = _current_dialogue->LineTime();
				_display_textbox.SetDisplayText(_current_dialogue->GetLine());
			}
			else {
				// The dialogue is over
				MapMode::_current_map->_map_state = EXPLORE;

				// Restore the status of the map sprites if the dialogue should reset them.
				if (_current_dialogue->IsSaving()) {
					for (uint32 i = 0; i < _current_dialogue->GetNumLines(); i++) {
						static_cast<VirtualSprite*>(MapMode::_current_map->_all_objects[_current_dialogue->GetSpeaker(i)])->LoadState();
					}
				}
				_current_dialogue = NULL;
				last_dialogue = NULL;
			}
		}
	}
	else {
		if (timeleft != 0) {
			if (InputManager->ConfirmPress()) {
				// The line isn't finished, but user sent an input
				if (!_display_textbox.IsFinished()) {
					// Force the text to show completly
					_display_textbox.ForceFinish();
				}
				else {
					// IF the text is already show, change line
					if (_current_dialogue->ReadNextLine()) {
						timeleft = _current_dialogue->LineTime();
						_display_textbox.SetDisplayText(_current_dialogue->GetLine());
					}
					else {
						// The is no more line, the dialogue is over
						MapMode::_current_map->_map_state = EXPLORE;
						// Restore the status of the map sprites
						if (_current_dialogue->IsSaving()) {
							for (uint32 i = 0; i < _current_dialogue->GetNumLines(); i++) {
								static_cast<VirtualSprite*>( MapMode::_current_map->_all_objects[_current_dialogue->GetSpeaker(i)])->LoadState();
							}
						}
						_current_dialogue = NULL;
						last_dialogue = NULL;
					}
				}
			}
		}
		else {
			// There is no more time left, too bad we change line
			if (_current_dialogue->ReadNextLine()) {
				timeleft = _current_dialogue->LineTime();
				_display_textbox.SetDisplayText(_current_dialogue->GetLine());
			}
			else {
				// No more line, the dialogue is over
				MapMode::_current_map->_map_state = EXPLORE;
				if (_current_dialogue->IsSaving()) {
					// Restore the status of the map sprites
					for (uint32 i = 0; i < _current_dialogue->GetNumLines(); i++) {
						static_cast<VirtualSprite*>(MapMode::_current_map->_all_objects[_current_dialogue->GetSpeaker(i)])->LoadState();
					}
				}
				_current_dialogue = NULL;
				last_dialogue = NULL;
			}
		} // if (timeleft == 0)
	} // if (!_current_dialogue->IsBlocked())
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
	VideoManager->SetTextColor(Color(Color::black));
	VideoManager->SetFont("map");
	VideoManager->MoveRelative(120.0f, -10.0f);
	VirtualSprite* speaker = reinterpret_cast<VirtualSprite*>(MapMode::_current_map->_all_objects[ _current_dialogue->GetSpeaker()]);
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

MapDialogue::MapDialogue( const bool save_state ) :
	_seen(0),
	_current_line(0),
	_blocked(false)
{
	_save_state = save_state;
}



MapDialogue::~MapDialogue() {
	for (uint32 i = 0; i < _actions.size(); ++i) {
		for (uint32 j = 0; j < _actions[i].size(); ++j) {
			delete _actions[i][j];
		}
	}
}



bool MapDialogue::ReadNextLine() {
	if (++_current_line >= _text.size()) {
		_current_line = 0;
		SetSeenDialogue();
		return false;
	}
	return true;
}



void MapDialogue::AddText(uint32 speaker_id, hoa_utils::ustring text, int32 time, SpriteAction* action) {
	_speakers.push_back(speaker_id);
	_text.push_back(text);
	_time.push_back(time);
	_actions.resize(_actions.size() + 1);
	_actions.back().push_back(action);
}



void MapDialogue::AddTextActions(uint32 speaker_id, hoa_utils::ustring text, std::vector<SpriteAction*>& actions, int32 time) {
	_speakers.push_back(speaker_id);
	_text.push_back(text);
	_time.push_back(time);
	_actions.push_back(actions);
}

} // namespace private_map

} // namespace hoa_map
