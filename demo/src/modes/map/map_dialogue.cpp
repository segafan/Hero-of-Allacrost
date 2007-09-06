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

#include "audio.h"
#include "video.h"
#include "input.h"
#include "script.h"
#include "global.h"
#include "menu.h"

#include "map_dialogue.h"
#include "map.h"
#include "map_objects.h"
#include "map_sprites.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_input;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_menu;

namespace hoa_map {

namespace private_map {

// ****************************************************************************
// *********************** MapDialogue Class Functions ************************
// ****************************************************************************

DialogueManager::DialogueManager() {
	_state = DIALOGUE_STATE_NORMAL;
	
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
	bool finish_line = false;		  // When set to true, indicates that the current line of dialogue is finished
	int32 next_line = -1;			  // Used to store an index to the next line of dialogue to be displayed.

	if (_current_dialogue == NULL)
		return;

	if (_current_dialogue != last_dialogue) {
		time_remaining = _current_dialogue->GetCurrentTime();
		_display_textbox.SetDisplayText(_current_dialogue->GetCurrentText());
		last_dialogue = _current_dialogue;
	}
	
	// During option selection mode.
	if (_state == DIALOGUE_STATE_OPTION) {
		// Check for option selections. If a selection was made, Update() returns the index to the next line of dialogue, otherwise -1 indicates no selection was made. 
		next_line = _current_option->Update();
		// If next_line is greater than -1, a selection was made.
		if( next_line != -1 )
		{ 
			finish_line = true; 
		}
	}
	
	// During dialogue text display mode.
	if (_state == DIALOGUE_STATE_NORMAL) {
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
					// Check for dialogue options
					if(_current_dialogue->HasOptions()){
						// Load the options and set state to option mode
						_current_option = _current_dialogue->GetCurrentOption();
						_state = DIALOGUE_STATE_OPTION;
					}
					// Option selection is done
					else {
						finish_line = true;
						next_line = _current_dialogue->GetNextLine();
					}
				}
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
		if (_current_dialogue->ReadNextLine(next_line) == true) {
			time_remaining = _current_dialogue->GetCurrentTime();
			_display_textbox.SetDisplayText(_current_dialogue->GetCurrentText());
			_state = DIALOGUE_STATE_NORMAL;
		}
		
		// This dialogue is finished, restore game state as necessary
		else {
			//The is no more line, the dialogue is over
			MapMode::_current_map->_map_state = EXPLORE;
			// Restore the status of the map sprites
			if (_current_dialogue->IsSaving()) {
				for (uint32 i = 0; i < _current_dialogue->GetNumLines(); i++) {
					static_cast<VirtualSprite*>(MapMode::_current_map->_all_objects[_current_dialogue->GetLineSpeaker(i)])->LoadState();
				}
			}
			_current_dialogue = NULL;
			last_dialogue = NULL;
			_state = DIALOGUE_STATE_NORMAL; 
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
	
	VirtualSprite* speaker = NULL;
	if(_state == DIALOGUE_STATE_NORMAL) {
		_display_textbox.Draw(); // Display dialogue text
		speaker = reinterpret_cast<VirtualSprite*>(MapMode::_current_map->_all_objects[_current_dialogue->GetCurrentSpeaker()]);
	}
	if(_state == DIALOGUE_STATE_OPTION) {
		_current_option->Draw(); // Display options
		speaker = reinterpret_cast<VirtualSprite*>(MapMode::_current_map->_all_objects[_current_option->GetCurrentSpeaker()]); 
	}
	VideoManager->DrawText(speaker->name);
	if (speaker->face_portrait != NULL) {
		VideoManager->MoveRelative(0.0f, -26.0f);
		speaker->face_portrait->Draw();
	}
	VideoManager->PopState();
} // void DialogueManager::Draw()


	
	


// ******************************************************************************
// ************************* DialogueOptionBox Functions ************************
// ******************************************************************************

DialogueOptionBox::DialogueOptionBox() :
	_size(0), // Initialize as empty OptionBox
	_speaker(0)
{
	// Initialize the option box attributes.
	_options.SetCellSize(500.0f, 25.0f);
	_options.SetSize(1, 0);
	_options.SetPosition(325.0f, 574.0f);
	_options.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_options.SetFont("map");
	_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_options.SetCursorOffset(-55.0f, -25.0f);
	_options.SetVerticalWrapMode(VIDEO_WRAP_MODE_NONE);
}

DialogueOptionBox::~DialogueOptionBox()
{}

bool DialogueOptionBox::AddOption(std::string text, uint32 speaker_id, int32 next_line, int32 action)
{
	if(_options.AddOption(MakeUnicodeString(text))) {
		_next_line_index.push_back(next_line);
			
		if(action != static_cast<int32>(-1)) {/*TODO::put action function pointer into vector*/}
		
		_speaker = speaker_id;
		_size++; 
		_options.SetSize(1, _size);
		_options.SetSelection(0);

		return true;
	}
	else { return false; } //ERROR	
}

int32 DialogueOptionBox::Update()
{
	// clear OptionBox events
	_options.Update(); 

	// If the confirm key is pressed, return the selection information
	if (InputManager->ConfirmPress()) {
		_options.HandleConfirmKey();
		// Get selection and determine/return the next line of dialogue.
		int32 selection = _next_line_index[_options.GetSelection()];
		_options.SetSelection(0); // Reset selection
		return selection;
	}
	// If the cancel key is pressed, redisplay the current line. 
	if (InputManager->CancelPress()) {
		return _current_dialogue->GetCurrentLine();
	}
	
	if (InputManager->UpPress()) {
		_options.HandleUpKey();
	}

	if (InputManager->DownPress()) {
		_options.HandleDownKey();
	}
	return -1;		
}

void DialogueOptionBox::Draw()
{
	_options.Draw();
}
	

// ****************************************************************************
// *********************** MapDialogue Class Functions ************************
// ****************************************************************************

MapDialogue::MapDialogue(bool save_state) :
	_seen(0),
	_max_views(-1),
	_line_count(0),
	_current_line(0),
	_active(true),
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

	for (uint32 i = 0; i < _options.size(); i++ ) {
		if (_options[i] != NULL) {
			delete _options[i];
		}
	}
}

int32 MapDialogue::GetNextLine() { return _next_line_index[_current_line]; }



bool MapDialogue::ReadNextLine(int32 line) {
	// If line variable left at default, then just increment to next line.
	if( line == -1 ) ++_current_line;
	// Elsewise, go to designated line;
	else _current_line = line;
	// Determine if the dialogue is finished
	if (_current_line >= _text.size()) {
		_current_line = 0;
		IncrementTimesSeen();
		if((static_cast<int32>(_seen) >= _max_views) && (_max_views != -1)) {
			_active = false; 
		}
		if (_owner != NULL) {
			_owner->UpdateSeenDialogue();
			_owner->UpdateActiveDialogue();
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
	_next_line_index.push_back(-1);
	_options.push_back(NULL);
	_line_count++;
	if (action >= 0) {
		MapMode::_loading_map->_map_script.OpenTable("map_functions");
		ScriptObject* new_action = new ScriptObject();
		*new_action = MapMode::_loading_map->_map_script.ReadFunctionPointer(action);
		MapMode::_loading_map->_map_script.CloseTable();
		_actions.push_back(new_action);
	}
	else {
		_actions.push_back(NULL);
	}
}

void MapDialogue::AddOption(std::string text, uint32 speaker_id, int32 next_line, int32 action)
{	
	int32 current_line = _line_count -1; // Current line that options will belong to.

	// If the line the options will be added to currently has no options, create a new instance of the DialogueOptionBox class to store the options in.
	if (_options[current_line] == NULL )
	{
		DialogueOptionBox* option =  new DialogueOptionBox();
		option->SetCurrentDialogue(this);
		_options[current_line] = option;
		
	}
	
	_options[current_line]->AddOption(text, speaker_id, next_line, action);
}

} // namespace private_map

} // namespace hoa_map
