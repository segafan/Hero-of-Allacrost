///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    menu.cpp
*** \author  Daniel Steuernol steu@allacrost.org
*** \brief   Source file for menu mode interface.
*** ***************************************************************************/

#include <iostream>
#include <sstream>

#include "utils.h"

#include "menu.h"
#include "audio.h"
#include "video.h"
#include "mode_manager.h"
#include "system.h"
#include "input.h"
#include "global.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_system;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_global;

using namespace hoa_menu::private_menu;

namespace hoa_menu {

bool MENU_DEBUG = false;

////////////////////////////////////////////////////////////////////////////////
// MenuMode class -- Initialization and Destruction Code
////////////////////////////////////////////////////////////////////////////////
MenuMode::MenuMode()
{
	if (MENU_DEBUG) cout << "MENU: MenuMode constructor invoked." << endl;

	// Save the currently drawn screen
	if (!VideoManager->CaptureScreen(_saved_screen)) {
		cerr << "MENU: ERROR: Couldn't save the screen!" << endl;
	}
	
	// Init the location picture
	_location_picture.SetFilename("img/menus/locations/desert_cave.png");
	_location_picture.SetDimensions(500, 125);
	_location_picture.SetStatic(true);
	VideoManager->LoadImage(_location_picture);

	// TEMP: remove this eventually
	GlobalManager->SetMoney(4236);
	_current_window = WIN_INVENTORY;
		
	vector<GlobalCharacter*> characters = GlobalManager->GetParty();

	switch (characters.size()) {
		case 4: _character_window3.SetCharacter(characters[3]);
		case 3: _character_window2.SetCharacter(characters[2]);
		case 2: _character_window1.SetCharacter(characters[1]);
		case 1: _character_window0.SetCharacter(characters[0]);
			break;
		default: cerr << "MENU ERROR: no characters in party!" << endl;
			exit(1);
	}
	/*
	if (characters.size() == 4)
	{
		_character_window0.SetCharacter(characters[0]);
		_character_window1.SetCharacter(characters[1]);
		_character_window2.SetCharacter(characters[2]);
		_character_window3.SetCharacter(characters[3]);
	}
	else if (characters.size() == 3)
	{
		_character_window0.SetCharacter(characters[0]);
		_character_window1.SetCharacter(characters[1]);
		_character_window2.SetCharacter(characters[2]);
	}
	else if (characters.size() == 2)
	{
		_character_window0.SetCharacter(characters[0]);
		_character_window1.SetCharacter(characters[1]);
	}
	else if (characters.size() == 1)
	{
		_character_window0.SetCharacter(characters[0]);
	}
	else
	{
		cerr << "MENU ERROR: no characters in party!" << endl;
		exit(1);
	}*/

	//////////// Setup the menu windows
	uint32 start_x = (1024 - 800) / 2 - 40;
	uint32 start_y = (768 - 600) / 2 + 15;
	uint32 win_width = 208;
//	uint32 win_height = 600 - 192;

	//FIX ME: Make bottom window dynamic
	_bottom_window.Create(static_cast<float>(win_width * 4 + 16), 140 + 16, VIDEO_MENU_EDGE_ALL);
	_bottom_window.SetPosition(static_cast<float>(start_x),static_cast<float>(start_y) + 442);//static_cast<float>(start_y) + static_cast<float>(win_height) - 16);


	// Width of each character window is 360 px.
	// Each char window will have an additional 16 px for the left border 
	// The 4th (last) char window will have another 16 px for the right border
	// Height of the char window is 98 px.
	// The bottom window in the main view is 192 px high, and the full width which will be 216 * 4 + 16
	_character_window0.Create(360, 98,~VIDEO_MENU_EDGE_BOTTOM,
		VIDEO_MENU_EDGE_BOTTOM);
	_character_window0.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y) + 10);

	_character_window1.Create(360, 98,~VIDEO_MENU_EDGE_BOTTOM,
		VIDEO_MENU_EDGE_BOTTOM | VIDEO_MENU_EDGE_TOP);
	_character_window1.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y) + 118);

	_character_window2.Create(360, 98,~VIDEO_MENU_EDGE_BOTTOM,
		VIDEO_MENU_EDGE_BOTTOM | VIDEO_MENU_EDGE_TOP);
	_character_window2.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y) + 226);

	_character_window3.Create(360, 98,~VIDEO_MENU_EDGE_BOTTOM,
		VIDEO_MENU_EDGE_TOP | VIDEO_MENU_EDGE_BOTTOM);
	_character_window3.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y) + 334);

	_main_options_window.Create(static_cast<float>(win_width * 4 + 16), 60,
		~VIDEO_MENU_EDGE_BOTTOM, VIDEO_MENU_EDGE_BOTTOM);
	_main_options_window.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y) - 50);

	// Set up the status window
	_status_window.Create(static_cast<float>(win_width * 4 + 16), 448,
		VIDEO_MENU_EDGE_ALL);
	_status_window.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y) + 10);

	// Set up the item list header window
	/*_item_list_header_window.Create(static_cast<float>(win_width * 4), 40,
		~(VIDEO_MENU_EDGE_LEFT | VIDEO_MENU_EDGE_RIGHT),VIDEO_MENU_EDGE_LEFT | VIDEO_MENU_EDGE_RIGHT);
	_item_list_header_window.SetPosition(static_cast<float>(start_x) + 10, static_cast<float>(start_y) + 10);*/
	// Set up the inventory window
	_inventory_window.Create(static_cast<float>(win_width * 4 + 16), 448,
		VIDEO_MENU_EDGE_ALL);
	_inventory_window.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y) + 10);

	/*_character_window0.Create(static_cast<float>(win_width) + 16, static_cast<float>(win_height),
		~VIDEO_MENU_EDGE_RIGHT, VIDEO_MENU_EDGE_RIGHT);
	_character_window0.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y));

	_character_window1.Create(static_cast<float>(win_width) + 16, static_cast<float>(win_height),
		~VIDEO_MENU_EDGE_RIGHT, VIDEO_MENU_EDGE_LEFT | VIDEO_MENU_EDGE_RIGHT);
	_character_window1.SetPosition(static_cast<float>(start_x) + static_cast<float>(win_width), static_cast<float>(start_y));
	
	_character_window2.Create(static_cast<float>(win_width) + 16, static_cast<float>(win_height),
		~VIDEO_MENU_EDGE_RIGHT, VIDEO_MENU_EDGE_LEFT | VIDEO_MENU_EDGE_RIGHT);
	_character_window2.SetPosition(static_cast<float>(start_x) + static_cast<float>(2 * win_width), static_cast<float>(start_y));
	
	_character_window3.Create(static_cast<float>(win_width) + 16, static_cast<float>(win_height),
		VIDEO_MENU_EDGE_ALL, VIDEO_MENU_EDGE_LEFT);
	_character_window3.SetPosition(static_cast<float>(start_x) + static_cast<float>(3 * win_width), static_cast<float>(start_y));
*/
	// Setup the inventory window
	/*_inventory_window.Create(static_cast<float>(win_width * 4 + 16), static_cast<float>(win_height),
		VIDEO_MENU_EDGE_ALL, VIDEO_MENU_EDGE_BOTTOM);
	_inventory_window.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y));

	// Setup the status window
	_status_window.Create(static_cast<float>(win_width * 4 + 16), static_cast<float>(win_height),
		VIDEO_MENU_EDGE_ALL, VIDEO_MENU_EDGE_BOTTOM);
	_status_window.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y));*/

	
	// Set the menu to show the main options
	
	_current_menu_showing = SHOW_MAIN;
	_current_menu = &_main_options;
	
	// Load sounds
	SoundDescriptor confirm;
	SoundDescriptor bump;
	SoundDescriptor potion;
	SoundDescriptor cancel;
	if (confirm.LoadSound("snd/obtain.wav") == false) 
	{
		cerr << "MINICHARWINDOW::UPDATE - Unable to load confirm sound effect!" << endl;
	}
	if (bump.LoadSound("snd/bump.wav") == false) 
	{
		cerr << "MINICHARWINDOW::UPDATE - Unable to load bump sound effect!" << endl;
	}
	if (potion.LoadSound("snd/potion_drink.wav") == false)
	{
		cerr << "MINICHARWINDOW::UPDATE - Unable to load potion drink sound effect!" << endl;
	}
	if (cancel.LoadSound("snd/cancel.wav") == false)
	{
		cerr << "MINICHARWINDOW::UPDATE - Unable to load cancel sound effect!" << endl;
	}
	_menu_sounds["confirm"] = confirm;
	_menu_sounds["bump"] = bump;
	_menu_sounds["potion"] = potion;
	_menu_sounds["cancel"] = cancel;

} // MenuMode::MenuMode()


MenuMode::~MenuMode() {
	if (MENU_DEBUG) cout << "MENU: MenuMode destructor invoked." << endl;
	
	// Remove saved images
	VideoManager->DeleteImage(_saved_screen);
	
	for (uint32 i = 0; i < _menu_images.size(); i++) {
		VideoManager->DeleteImage(_menu_images[i]);
	}
	
	// Unload location picture
	VideoManager->DeleteImage(_location_picture);
	
	// Destroy all menu windows
	_bottom_window.Destroy();
	_character_window0.Destroy();
	_character_window1.Destroy();
	_character_window2.Destroy();
	_character_window3.Destroy();
	_inventory_window.Destroy();
	_status_window.Destroy();
	_main_options_window.Destroy();
	//_item_list_header_window.Destroy();

	// Clear sounds
	_menu_sounds["confirm"].FreeSound();
	_menu_sounds["bump"].FreeSound();
	_menu_sounds["potion"].FreeSound();
	_menu_sounds["cancel"].FreeSound();
} // MenuMode::~MenuMode()


// Resets configuration/data for the class as appropriate
void MenuMode::Reset() {
	// Top left corner coordinates in menu mode are always (0,0)
	VideoManager->SetCoordSys(0, 1024, 768, 0);

	if (!VideoManager->SetFont("default")) {
		cerr << "MAP: ERROR > Couldn't set menu font!" << endl;
		exit(1);
	}

	_bottom_window.Show();
	_main_options_window.Show();
	_character_window0.Show();
	_character_window1.Show();
	_character_window2.Show();
	_character_window3.Show();
	_inventory_window.Show();
	_status_window.Show();
	//_item_list_header_window.Show();
	
	// Setup OptionBoxes
	this->_SetupMainOptionBox();
	this->_SetupInventoryOptionBox();
	this->_SetupSkillsOptionBox();
	this->_SetupStatusOptionBox();
	this->_SetupOptionsOptionBox();
	this->_SetupSaveOptionBox();
	this->_SetupEquipOptionBox();
} // void MenuMode::Reset()



void MenuMode::_SetupOptionBoxCommonSettings(OptionBox *ob)
{
	// Set all the default options
	ob->SetFont("default");
	ob->SetCellSize(115.0f, 50.0f);
	ob->SetPosition(142.0f, 85.0f);
	ob->SetAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	ob->SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	ob->SetSelectMode(VIDEO_SELECT_SINGLE);
	ob->SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	ob->SetCursorOffset(-50.0f, -28.0f);
} // void MenuMode::_SetupOptionBoxCommonSettings(OptionBox *ob)



void MenuMode::_SetupMainOptionBox()
{
	// Setup the main options box
	this->_SetupOptionBoxCommonSettings(&_main_options);
	_main_options.SetSize(MAIN_SIZE, 1);
	
	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeUnicodeString("Inventory"));
	options.push_back(MakeUnicodeString("Skills"));
	options.push_back(MakeUnicodeString("Equip"));
	options.push_back(MakeUnicodeString("Status"));
	options.push_back(MakeUnicodeString("Formation"));
	options.push_back(MakeUnicodeString("Exit"));
	
	// Add strings and set default selection.
	_main_options.SetOptions(options);
	_main_options.SetSelection(MAIN_INVENTORY);

	// disable unused options 
	_main_options.EnableOption(1, false);
	_main_options.EnableOption(2, false);
	_main_options.EnableOption(4, false);
} // void MenuMode::_SetupMainOptionBox()



void MenuMode::_SetupInventoryOptionBox()
{
	// Setup the option box
	this->_SetupOptionBoxCommonSettings(&_menu_inventory);
	_menu_inventory.SetSize(INV_SIZE, 1);
	
	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeUnicodeString("Use"));
	options.push_back(MakeUnicodeString("Sort"));
	options.push_back(MakeUnicodeString("Cancel"));
	
	_menu_inventory.SetOptions(options);
	_menu_inventory.SetSelection(INV_USE);
} // void MenuMode::_SetupInventoryOptionBox()


void MenuMode::_SetupSkillsOptionBox()
{
	// setup the option box
	this->_SetupOptionBoxCommonSettings(&_menu_skills);
	_menu_skills.SetSize(SKILLS_SIZE, 1);
	
	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeUnicodeString("Use"));
	options.push_back(MakeUnicodeString("Cancel"));
	
	_menu_skills.SetOptions(options);
	_menu_skills.SetSelection(SKILLS_USE);
} // void MenuMode::_SetupSkillsOptionBox()



void MenuMode::_SetupEquipOptionBox()
{
	// setup the status option box
	this->_SetupOptionBoxCommonSettings(&_menu_equip);
	_menu_equip.SetCellSize(150.0f, 50.0f);
	_menu_equip.SetSize(EQUIP_SIZE, 1);
	
	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeUnicodeString("Equip"));
	options.push_back(MakeUnicodeString("Remove"));
	//options.push_back(MakeUnicodeString("Next"));
	//options.push_back(MakeUnicodeString("Previous"));
	options.push_back(MakeUnicodeString("Cancel"));
	
	_menu_equip.SetOptions(options);
	_menu_equip.SetSelection(EQUIP_EQUIP);
// 	_menu_status_equip.SetCursor(
} // void MenuMode::_SetupEquipOptionBox()

void MenuMode::_SetupStatusOptionBox()
{
	// setup the status option box
	this->_SetupOptionBoxCommonSettings(&_menu_status);
	//_menu_status.SetCellSize(150.0f, 50.0f);
	_menu_status.SetSize(STATUS_SIZE, 1);
	
	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeUnicodeString("View"));
	//options.push_back(MakeUnicodeString("Remove"));
	//options.push_back(MakeUnicodeString("Next"));
	//options.push_back(MakeUnicodeString("Previous"));
	options.push_back(MakeUnicodeString("Cancel"));
	
	_menu_status.SetOptions(options);
	_menu_status.SetSelection(STATUS_VIEW);
// 	_menu_status_equip.SetCursor(
} // void MenuMode::_SetupEquipOptionBox()



void MenuMode::_SetupOptionsOptionBox()
{
	// setup the options option box
	this->_SetupOptionBoxCommonSettings(&_menu_options);
	_menu_options.SetSize(OPTIONS_SIZE, 1);
	
	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeUnicodeString("Edit"));
	options.push_back(MakeUnicodeString("Save"));
	options.push_back(MakeUnicodeString("Cancel"));
	
	_menu_options.SetOptions(options);
	_menu_options.SetSelection(OPTIONS_EDIT);
} // void MenuMode::_SetupOptionsOptionBox()



void MenuMode::_SetupSaveOptionBox()
{
	// setup the save options box
	this->_SetupOptionBoxCommonSettings(&_menu_save);
	_menu_save.SetSize(SAVE_SIZE, 1);
	
	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeUnicodeString("Save"));
	options.push_back(MakeUnicodeString("Cancel"));
	
	_menu_save.SetOptions(options);
	_menu_save.SetSelection(SAVE_SAVE);
} // void MenuMode::_SetupSaveOptionBox()

////////////////////////////////////////////////////////////////////////////////
// MenuMode class -- Update Code
////////////////////////////////////////////////////////////////////////////////

void MenuMode::Update() 
{	
	// See if inventory window is active
	if (_inventory_window.IsActive()) {
		// See if cancel was pressed, duplicate code, but not really sure of an
		// elegant way to do this.
		/*if (_inventory_window.CanCancel() && InputManager->CancelPress())
		{
			// Play sound
			_menu_sounds["cancel"].PlaySound();
			_inventory_window.Activate(false);
			_current_menu->SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			return;
		}*/
		
		_inventory_window.Update();
		return;
	}
	else if (_status_window.IsActive())
	{
		// Update status window.
		_status_window.Update();
		return;
	}

		
	if (InputManager->CancelPress()) 
	{
		// Play sound.
		_menu_sounds["cancel"].PlaySound();
		// If in main menu, return to previous Mode, else return to main menu.
		if (_current_menu_showing == SHOW_MAIN)
			ModeManager->Pop();
		else
		{
			//_menu_queue.pop_back();
			//_menu_showing_queue.pop_back();
			//if (_menu_queue.size() == 1)
			_current_menu_showing = SHOW_MAIN;
			_current_menu = &_main_options;
			//_current_menu
		}
	}
	else if (InputManager->ConfirmPress())
	{
		// Play Sound
		if (_current_menu->IsEnabled(_current_menu->GetSelection()))
			_menu_sounds["confirm"].PlaySound();
		_current_menu->HandleConfirmKey();
	}
	else if (InputManager->LeftPress())
	{
		// Play Sound
		_current_menu->HandleLeftKey();
	}
	else if (InputManager->RightPress())
	{
		// Play Sound
		_current_menu->HandleRightKey();
	}
	else if (InputManager->UpPress() && _current_menu_showing == SHOW_MAIN)
	{
		// Up was pressed
		//_current_menu->SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	}
	
	// Get the latest event from the current menu
	int32 event = _current_menu->GetEvent();
	
	if (event == VIDEO_OPTION_CONFIRM)
	{
		//FIX ME:  vector of function pointers?

		switch (_current_menu_showing)
		{
			case SHOW_MAIN:
				this->_HandleMainMenu();
				break;
			case SHOW_INVENTORY:
				this->_HandleInventoryMenu();
				break;
			/*case SHOW_ITEM_LIST:
				this->_HandleItemListMenu();
				break;*/
			case SHOW_SKILLS:
				//this->_HandleSkillsMenu();
				break;
			case SHOW_STATUS:
				this->_HandleStatusMenu();
				break;
			case SHOW_EQUIP:
				this->_HandleEquipMenu();
				break;
			case SHOW_OPTIONS:
				this->_HandleOptionsMenu();
				break;
			case SHOW_SAVE:
				this->_HandleSaveMenu();
				break;
			default:
				cerr << "MENU: ERROR: Invalid menu showing!" << endl;
				break;
		}
	}
} // void MenuMode::Update()

////////////////////////////////////////////////////////////////////////////////
// MenuMode class -- Draw Code
////////////////////////////////////////////////////////////////////////////////

void MenuMode::Draw() {
	uint32 drawwindow;

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);

	// Move to the top left corner
	VideoManager->Move(0,0);

	// Set the text colour to white
	VideoManager->SetTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
	
	// Draw the saved screen as the menu background
	VideoManager->DrawImage(_saved_screen);
	
	//FIX ME:  Test
	_DrawBottomMenu();
	_main_options_window.Draw();
		
	//_main_options_window.Draw();
	if (_current_menu_showing == SHOW_MAIN) {
		drawwindow = _current_menu->GetSelection() + 1;
	}
	else {
		drawwindow = _current_menu_showing;
	}

	// Draw the four character menus
	switch (drawwindow)
	{
		case SHOW_MAIN: 
			//_status_window.Draw();
			
			//_inventory_window.Draw();
			//_DrawItemListHeader();
			
			break;
		/*case SHOW_ITEM_LIST:
			switch (_menu_showing_queue[1]) {
				case SHOW_INVENTORY:
					_inventory_window.Draw();
					_DrawItemListHeader();
					break;
				case SHOW_EQUIP:
					break;
				case SHOW_SKILLS:
					break;
			}
			break;*/
		case SHOW_INVENTORY:
		{
			_inventory_window.Draw();
			//_DrawItemListHeader();
			//_menu_item_list.Draw();
			break;
		}
		case SHOW_STATUS:
		{
			_status_window.Draw();
			break;
		}
		case SHOW_SKILLS:
		{
			//this->_HandleSkillsMenu();
			break;
		}
		case SHOW_EQUIP:
		{
			//this->_HandleEquipMenu();
			break;
		}
		case SHOW_OPTIONS:
		{
			//this->_HandleOptionsMenu();
			break;
		}
		case SHOW_SAVE:
		{
			//this->_HandleSaveMenu();
			break;
		}
	}

	
	_character_window0.Draw();
	_character_window1.Draw();
	_character_window2.Draw();
	_character_window3.Draw();
	_current_menu->Draw();
	// Draw currently active options box
	
} // void MenuMode::Draw()


/*void MenuMode::_DrawItemListHeader() {

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);

	VideoManager->Move(465, 120);
	VideoManager->DrawText(MakeUnicodeString("All"));

	VideoManager->MoveRelative(70, 0);
	VideoManager->DrawText(MakeUnicodeString("Field"));

	VideoManager->MoveRelative(75, 0);
	VideoManager->DrawText(MakeUnicodeString("Battle"));

	VideoManager->MoveRelative(100, 0);
	VideoManager->DrawText(MakeUnicodeString("Equipment"));

	VideoManager->MoveRelative(125, 0);
	VideoManager->DrawText(MakeUnicodeString("Key"));
	
}*/


//FIX ME:  Make dynamic, move category id and select state enums to this class
void MenuMode::_DrawBottomMenu() {
	_bottom_window.Draw();

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->Move(150, 577);

	/*if (_current_menu_showing == SHOW_INVENTORY || _current_menu_showing == SHOW_EQUIP
		|| _current_menu_showing == SHOW_SKILLS) {
			VideoManager->DrawText(MakeUnicodeString("STR: 105 (+1)"));

			VideoManager->MoveRelative(0, 20);
			VideoManager->DrawText(MakeUnicodeString("VGR: 72 (+0)"));

			VideoManager->MoveRelative(0, 20);
			VideoManager->DrawText(MakeUnicodeString("FRT: 106 (-2)"));

			VideoManager->MoveRelative(0, 20);
			VideoManager->DrawText(MakeUnicodeString("RES: 48 (+0)"));

			VideoManager->MoveRelative(0, 20);
			VideoManager->DrawText(MakeUnicodeString("AGI: 25 (+0)"));

			VideoManager->MoveRelative(0, 20);
			VideoManager->DrawText(MakeUnicodeString("EVD: 3% (+1)"));

			VideoManager->SetDrawFlags(VIDEO_X_CENTER,VIDEO_Y_BOTTOM,0);

			VideoManager->MoveRelative(370, -80);
			VideoManager->DrawText(MakeUnicodeString("Health Potion"));

			VideoManager->MoveRelative(0, 60);
			VideoManager->DrawText(MakeUnicodeString("HP +30.  Single Ally"));

			VideoManager->SetDrawFlags(VIDEO_X_LEFT,VIDEO_Y_BOTTOM,0);
			StillImage i;
			i.SetFilename("img/icons/items/health_potion_large.png");
			VideoManager->LoadImage(i);
			VideoManager->MoveRelative(260, 30);
			VideoManager->DrawImage(i);
	}*/
	

	// Display Location
	
	if (!VideoManager->DrawText(MakeUnicodeString("Desert Cave")))
		cerr << "MENU: ERROR > Couldn't draw location!" << endl;

	// Draw Played Time
	VideoManager->MoveRelative(-40, 60);
	std::ostringstream os_time;
	uint8 hours = SystemManager->GetPlayHours();
	uint8 minutes = SystemManager->GetPlayMinutes();
	uint8 seconds = SystemManager->GetPlaySeconds();
	os_time << (hours < 10 ? "0" : "") << (uint32)hours << ":";
	os_time << (minutes < 10 ? "0" : "") << (uint32)minutes << ":";
	os_time << (seconds < 10 ? "0" : "") << (uint32)seconds;

	std::string time = std::string("Time: ") + os_time.str();
	if (!VideoManager->DrawText(MakeUnicodeString(time)))
		cerr << "MENU: ERROR > Couldn't draw text!" << endl;
	
	// Get the money of the party
	std::ostringstream os_money;
	os_money << GlobalManager->GetMoney();
	std::string money = std::string("Dorrun: ") + os_money.str();
	VideoManager->MoveRelative(0, 30);
	if (!VideoManager->DrawText(MakeUnicodeString(money)))
		cerr << "MENU: ERROR > Couldn't draw text!" << endl;
	
	//VideoManager->SetDrawFlags(VIDEO_X_RIGHT, VIDEO_Y_BOTTOM, 0);
		
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
		
	VideoManager->Move(390, 685);
	VideoManager->DrawImage(_location_picture);
} // void MenuMode::_DrawBottomMenu()



//FIX ME:  Adjust for new layout
void MenuMode::_HandleMainMenu()
{
	
	// Change the based on which option was selected.
	switch (_main_options.GetSelection())
	{
		case MAIN_INVENTORY:
		{
			//_menu_showing_queue.push_back(SHOW_INVENTORY);
			_current_menu_showing = SHOW_INVENTORY;
			//_menu_queue.push_back(&_menu_inventory);
			_current_menu = &_menu_inventory;
			break;
		}
		case MAIN_SKILLS:
		{
			//_menu_showing_queue.push_back(SHOW_SKILLS);
			_current_menu_showing = SHOW_SKILLS;
			//_menu_queue.push_back(&_menu_skills);
			_current_menu = &_menu_skills;
			break;
		}
		/*case MAIN_OPTIONS:
		{
			_current_menu_showing = SHOW_OPTIONS;
			_current_menu = &_menu_options;
			break;
		}*/
		/*case MAIN_FORMATION:
		{
			_current_menu_showing = SHOW_FORMATION;
			_current_menu = &_menu_formation;
			break;
		}*/
		case MAIN_STATUS:
		{
			//_menu_showing_queue.push_back(SHOW_STATUS);
			_current_menu_showing = SHOW_STATUS;
			//_menu_queue.push_back(&_menu_char_select);
			_current_menu = &_menu_status;
			break;
		}
		/*case MAIN_EQUIP:
		{
			_current_menu_showing = SHOW_EQUIP;
			_current_menu = &_menu_equip;
			break;
		}*/
		/*case MAIN_SAVE:
		{
			_current_menu_showing = SHOW_SAVE;
			_current_menu = &_menu_save;
			break;
		}*/
		case MAIN_EXIT:
		{
			ModeManager->Pop();
			break;
		}
		default:
		{
			cerr << "MENU: ERROR: Invalid option in MenuMode::HandleMainMenu()!" << endl;
			break;
		}
	}
} // void MenuMode::_HandleMainMenu()


void MenuMode::_HandleStatusMenu() {
	switch (_menu_status.GetSelection()) {
		case STATUS_VIEW:
			_status_window.Activate(true);
			break;
		case STATUS_CANCEL:
			_current_menu_showing = SHOW_MAIN;
			_current_menu = &_main_options;
			//_status_window.Activate(false);
			break;
		default:
			cerr << "MENU: ERROR: Invalid option in MenuMode::HandleStatusMenu()!" << endl;
			break;
	}
} // void MenuMode::_HandleStatusMenu()

void MenuMode::_HandleInventoryMenu()
{
	switch (_menu_inventory.GetSelection())
	{
		case INV_USE:
			// Make sure we have some items in the inventory.
			//if (GlobalManager->GetInventory().size() == 0)
			//	return;
			
			// TODO: Handle the use inventory command
			_inventory_window.Activate(true);
			
			_current_menu->SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
			break;
		case INV_SORT:
			// TODO: Handle the sort inventory comand
			cout << "MENU: Inventory sort command!" << endl;
			break;
		case INV_CANCEL:
		{
			_current_menu_showing = SHOW_MAIN;
			//_menu_queue.pop_back();
			//_current_menu = _menu_queue[_menu_queue.size() - 1];
			_current_menu = &_main_options;
			//_inventory_window.Activate(false);
			break;
		}
		default:
			cerr << "MENU: ERROR: Invalid option in MenuMode::HandleInventoryMenu()!" << endl;
			break;
	}
} // void MenuMode::_HandleInventoryMenu()



void MenuMode::_HandleSkillsMenu()
{
	switch (_menu_skills.GetSelection())
	{
		case SKILLS_CANCEL:
		{
			_current_menu_showing = SHOW_MAIN;
			_current_menu = &_main_options;
			break;
		}
		case SKILLS_USE:
		{
			//TODO
			break;
		}
		default:
			cerr << "MENU: ERROR: Invalid option in MenuMode::HandleSkillsMenu()!" << endl;
			break;
	}
} // void MenuMode::_HandleSkillsMenu()



void MenuMode::_HandleEquipMenu()
{
	switch (_menu_equip.GetSelection())
	{
		case EQUIP_EQUIP:
			// TODO: Handle the equip command
			cout << "MENU: Equip command!" << endl;
			break;
		case EQUIP_REMOVE:
			// TODO: Handle the remove command
			cout << "MENU: Remove command!" << endl;
			break;
		/*case STATUS_EQUIP_NEXT:
			// TODO: Handle the status - next command.
			cout << "MENU: Status Next command!" << endl;
			break;
		case STATUS_EQUIP_PREV:
			// TODO: Handle the status - prev command.
			cout << "MENU: Status Prev command!" << endl;
			break;*/
		case EQUIP_CANCEL:
		{
			_current_menu_showing = SHOW_MAIN;
			_current_menu = &_main_options;
			break;
		}
		default:
			cerr << "MENU: ERROR: Invalid option in MenuMode::HandleEquipMenu()!" << endl;
			break;
	}
} // void MenuMode::_HandleEquipMenu()



void MenuMode::_HandleOptionsMenu()
{
	switch (_menu_options.GetSelection())
	{
		case OPTIONS_EDIT:
			// TODO: Handle the Options - Edit command
			cout << "MENU: Options - Edit command!" << endl;
			break;
		case OPTIONS_SAVE:
			// TODO: Handle the Options - Save command
			cout << "MENU: Options - Save command!" << endl;
			break;
		case OPTIONS_CANCEL:
		{
			_current_menu_showing = SHOW_MAIN;
			_current_menu = &_main_options;
			break;
		}
		default:
			cerr << "MENU: ERROR: Invalid option in MenuMode::HandleOptionsMenu()!" << endl;
			break;
	}
} // void MenuMode::_HandleOptionsMenu()



void MenuMode::_HandleSaveMenu()
{
	switch (_menu_save.GetSelection())
	{
		case SAVE_SAVE:
			// TODO: Handle Save - Save command
			cout << "MENU: Save - Save command!" << endl;
			break;
		case SAVE_CANCEL:
		{
			_current_menu_showing = SHOW_MAIN;
			_current_menu = &_main_options;
			break;
		}
		default:
			cerr << "MENU: ERROR: Invalid option in MenuMode::HandleSaveMenu()!" << endl;
	}
} // void MenuMode::_HandleSaveMenu()

} // namespace hoa_menu
