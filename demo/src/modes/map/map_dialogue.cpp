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
#include "mode_manager.h"
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
using namespace hoa_mode_manager;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_menu;

namespace hoa_map {

namespace private_map {

// ****************************************************************************
// ***** TreasureMenu class methods
// ****************************************************************************

TreasureMenu::TreasureMenu() :
	_treasure(NULL)
{
	_action_window.Create(512, 64, ~VIDEO_MENU_EDGE_BOTTOM);
	_action_window.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_TOP);
	_action_window.SetPosition(512, 488);
	_action_window.SetDisplayMode(VIDEO_MENU_INSTANT);

	_list_window.Create(512, 192);
	_list_window.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_TOP);
	_list_window.SetPosition(512, 544);
	_list_window.SetDisplayMode(VIDEO_MENU_INSTANT);

	_action_options.AddOption(MakeUnicodeString("Return"));
	_action_options.AddOption(MakeUnicodeString("View details"));
	_action_options.AddOption(MakeUnicodeString("Open menu"));
	_action_options.SetCellSize(150.0f, 32.0f);
	_action_options.SetSize(3, 1);
	_action_options.SetPosition(20.0f, 20.0f);
	_action_options.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_action_options.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_action_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_action_options.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_action_options.SetCursorOffset(-50.0f, -25.0f);
	_action_options.SetFont("default");
	_action_options.SetSelection(0);
	_action_options.SetOwner(&_action_window);

	_list_options.SetCellSize(470.0f, 32.0f);
	_list_options.SetSize(1, 6);
	_list_options.SetPosition(20.0f, 20.0f);
	_list_options.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_list_options.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_list_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_list_options.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_list_options.SetCursorOffset(-50.0f, -25.0f);
	_list_options.SetFont("default");
	_list_options.SetOwner(&_list_window);
	// NOTE: this currently does not work. It will show no text
// 	_list_options.Scissoring(true, true);

	_detail_textbox.SetPosition(20.0f, 92.0f);
	_detail_textbox.SetDimensions(470.0f, 100.0f);
	_detail_textbox.SetDisplaySpeed(50);
	_detail_textbox.SetTextStyle(TextStyle());
	_detail_textbox.SetDisplayMode(VIDEO_TEXT_REVEAL);
	_detail_textbox.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_detail_textbox.SetOwner(&_list_window);
}



TreasureMenu::~TreasureMenu() {
	_action_window.Destroy();
	_list_window.Destroy();

	if (_treasure != NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "_treasure member was not NULL upon class destruction. Treasure contents may not have "
			<< "been added to player's inventory" << endl;
	}
}



void TreasureMenu::Initialize(MapTreasure* treasure) {
	if (treasure == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function argument was NULL" << endl;
		return;
	}

	if (_treasure != NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "_treasure member was not NULL when method was called" << endl;
		return;
	}

	_treasure = treasure;

	if (_treasure->_drunes != 0) {
		_list_options.AddOption(MakeUnicodeString(NumberToString(_treasure->_drunes) + " drunes"));
	}

	for (uint32 i = 0; i < _treasure->_objects_list.size(); i++) {
		_list_options.AddOption(_treasure->_objects_list[i]->GetName() + MakeUnicodeString("<R>x") +
			MakeUnicodeString(NumberToString(_treasure->_objects_list[i]->GetCount())));
	}
	_list_options.SetSelection(0);
	_action_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	_list_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

	_selection = ACTION_SELECTED;
	_action_window.Show();
	_list_window.Show();

	// Add the drunes to the player's inventory
	GlobalManager->AddDrunes(_treasure->_drunes);

	// Add the items to the player's inventory. Note that the AddToInventory call will delete the
	// pointer that it is given if that type of object already exists in the inventory. Because we
	// still require all of the object pointers to remain in memory while the menu is being displayed,
	// we check if an object exists in the inventory, increment the inventory count if it does, and
	// keep a record that we must delete the object once the menu is closed
	for (uint32 i = 0; i < _treasure->_objects_list.size(); i++) {
		GlobalObject* obj = _treasure->_objects_list[i];
		if (GlobalManager->IsObjectInInventory(obj->GetID() == true)) {
			GlobalManager->IncrementObjectCount(obj->GetID(), obj->GetCount());
			_objects_to_delete.push_back(obj);
		}
		else {
			GlobalManager->AddToInventory(_treasure->_objects_list[i]);
		}
	}
}



void TreasureMenu::Reset() {
	for (uint32 i = 0; i < _objects_to_delete.size(); i++) {
		delete _objects_to_delete[i];
	}
	_objects_to_delete.clear();

	_treasure->_empty = true;
	_treasure->_drunes = 0;
	_treasure->_objects_list.clear();
	_treasure = NULL;

	_action_window.Hide();
	_list_window.Hide();
	_list_options.ClearOptions();
}



void TreasureMenu::Update() {
	_action_window.Update();
	_list_window.Update();
	_action_options.Update();
	_list_options.Update();
	_detail_textbox.Update();

	// Don't process user input until after the treasure opening animation is finished
	if (_treasure->current_animation != MapTreasure::OPEN_ANIM) {
		_treasure->Update();
		return;
	}

	if (_selection == ACTION_SELECTED)
		_UpdateAction();
	else if (_selection == LIST_SELECTED)
		_UpdateList();
	else if (_selection == DETAIL_SELECTED)
		_UpdateDetail();
	else
		IF_PRINT_WARNING(MAP_DEBUG) << "unknown selection state: " << _selection << endl;
}



void TreasureMenu::_UpdateAction() {
	if (InputManager->ConfirmPress()) {
		if (_action_options.GetSelection() == 0) { // "Return" action
			Reset();
		}
		else if (_action_options.GetSelection() == 1) { // "View details" action
			_selection = LIST_SELECTED;
			_action_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_list_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		}
		else if (_action_options.GetSelection() == 2) { // "Open menu" action
			MenuMode* MM = new MenuMode(MapMode::_current_map->_map_name, MapMode::_current_map->_location_graphic.GetFilename());
			ModeManager->Push(MM);
			return;
		}
		else
			IF_PRINT_WARNING(MAP_DEBUG) << "unhandled action selection in OptionBox: " << _action_options.GetSelection() << endl;
	}

	else if (InputManager->LeftPress())
		_action_options.HandleLeftKey();

	else if (InputManager->RightPress())
		_action_options.HandleRightKey();
}



void TreasureMenu::_UpdateList() {
	// TODO: Implement treasure detail window
	if (InputManager->ConfirmPress()) {
		_selection = DETAIL_SELECTED;
		_list_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

		uint32 list_selection = _list_options.GetSelection();
		if (_treasure->_drunes != 0 && list_selection == 0) { // If true, the drunes are selected
			_detail_textbox.SetDisplayText(MakeUnicodeString("With the additional " + NumberToString(_treasure->_drunes) + " drunes found in this treasure added," +
				"the party now holds a total of " + NumberToString(GlobalManager->GetDrunes()) + " drunes."));
		}
		else { // Otherwise, a GlobalObject is selected
			if (_treasure->_drunes != 0)
				list_selection--;
			_detail_textbox.SetDisplayText(_treasure->_objects_list[list_selection]->GetDescription());
		}
	}

	else if (InputManager->CancelPress()) {
		_selection = ACTION_SELECTED;
		_action_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		_list_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	}

	else if (InputManager->UpPress()) {
		_list_options.HandleUpKey();
	}

	else if (InputManager->DownPress()) {
		_list_options.HandleDownKey();
	}
}



void TreasureMenu::_UpdateDetail() {
	if (InputManager->ConfirmPress() || InputManager->CancelPress()) {
		if (_detail_textbox.IsFinished() == false) {
			_detail_textbox.ForceFinish();
		}
		else {
			_selection = LIST_SELECTED;
			_list_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		}
	}
}



void TreasureMenu::Draw() {
	// We wait until the treasure is fully open before displaying the menu. The chest will eventually move to the open animation
	// so long as the Update() method continues to be called on it (which MapMode should always do)
	if (_treasure->current_animation != MapTreasure::OPEN_ANIM) {
		return;
	}

	VideoManager->PushState();
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);

	_action_window.Draw();

	VideoManager->Move(280.0f, 500.0f);
	TextManager->Draw("Treasure Contents");

	_action_options.Draw();

	_list_window.Draw();

	if (_selection == DETAIL_SELECTED) {
		uint32 list_selection = _list_options.GetSelection();
		bool drunes_selected = (_treasure->_drunes != 0 && list_selection == 0);

		// Decrement list selection if we have drunes so that it can be used to index the treasure object list
		if (_treasure->_drunes != 0)
			list_selection--;

		// Move to the upper left corner and draw either "Drunes" or the name of the selected object
		VideoManager->Move(280.0f, 590.0f);
		if (drunes_selected)
			TextManager->Draw("Drunes");
		else
			TextManager->Draw(_treasure->_objects_list[list_selection]->GetName());

		// Move to the upper right corner and draw the object icon
		if (drunes_selected == false) {
			VideoManager->Move(680.0f, 620.0f);
			_treasure->_objects_list[list_selection]->GetIconImage().Draw();
		}
		_detail_textbox.Draw();
	}
	else {
		_list_options.Draw();
	}

	VideoManager->PopState();
}

// ****************************************************************************
// ***** DialogueManager class methods
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
	_display_textbox.SetTextStyle(TextStyle("map"));
	_display_textbox.SetDisplayMode(VIDEO_TEXT_FADECHAR);
	_display_textbox.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_display_textbox.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);

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
	VideoManager->Text()->Draw(speaker->name, TextStyle("map"));
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

		// TODO Add support for actions

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
		// Place out global table on top of the stack
		MapMode::_loading_map->_map_script.OpenTable(MapMode::_loading_map->_map_namespace, true);
		MapMode::_loading_map->_map_script.OpenTable("map_functions");
		ScriptObject* new_action = new ScriptObject();
		*new_action = MapMode::_loading_map->_map_script.ReadFunctionPointer(action);
		// clean up our additions to the stack
		MapMode::_loading_map->_map_script.CloseTable();
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
