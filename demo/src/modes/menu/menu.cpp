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
*** \author  Andy Gardner chopperdave@allacrost.org
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
	//_location_picture.SetFilename("img/menus/locations/desert_cave.png");
	//_location_picture.SetDimensions(500, 125);
	//_location_picture.SetStatic(true);
	//VideoManager->LoadImage(_location_picture);

	// FIXME: remove this eventually
	GlobalManager->SetFunds(4236);
	_current_window = WIN_INVENTORY;

	GlobalParty & characters = *GlobalManager->GetActiveParty();

	// Setup character windows based on active party size
	switch (characters.GetPartySize()) {
		case 4: _character_window3.SetCharacter(dynamic_cast<GlobalCharacter*>(characters.GetActor(3)));
		case 3: _character_window2.SetCharacter(dynamic_cast<GlobalCharacter*>(characters.GetActor(2)));
		case 2: _character_window1.SetCharacter(dynamic_cast<GlobalCharacter*>(characters.GetActor(1)));
		case 1: _character_window0.SetCharacter(dynamic_cast<GlobalCharacter*>(characters.GetActor(0)));
			break;
		default: cerr << "MENU ERROR: no characters in party!" << endl;
			exit(1);
	}

	//////////// Setup the menu windows
	uint32 start_x = (1024 - 800) / 2 - 40;
	uint32 start_y = (768 - 600) / 2 + 15;
	uint32 win_width = 208;
//	uint32 win_height = 600 - 192;

	//The bottom window for the menu
	_bottom_window.Create(static_cast<float>(win_width * 4 + 16), 140 + 16, VIDEO_MENU_EDGE_ALL);
	_bottom_window.SetPosition(static_cast<float>(start_x),static_cast<float>(start_y) + 442);

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

	//Set up the skills window
	_skills_window.Create(static_cast<float>(win_width * 4 + 16), 448,
		VIDEO_MENU_EDGE_ALL);
	_skills_window.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y) + 10);

	//Set up the equipment window
	_equip_window.Create(static_cast<float>(win_width * 4 + 16), 448,
		VIDEO_MENU_EDGE_ALL);
	_equip_window.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y) + 10);

	// Set up the inventory window
	_inventory_window.Create(static_cast<float>(win_width * 4 + 16), 448,
		VIDEO_MENU_EDGE_ALL);
	_inventory_window.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y) + 10);

	//FIXME: Set up the formation window
	_formation_window.Create(static_cast<float>(win_width * 4 + 16), 448,
		VIDEO_MENU_EDGE_ALL);
	_formation_window.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y) + 10);


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

// Release and destroy everything
MenuMode::~MenuMode() {
	if (MENU_DEBUG) cout << "MENU: MenuMode destructor invoked." << endl;

	// Remove saved images
	VideoManager->DeleteImage(_saved_screen);

	// Unload location picture
	//VideoManager->DeleteImage(_location_picture);

	// Destroy all menu windows
	_bottom_window.Destroy();
	_character_window0.Destroy();
	_character_window1.Destroy();
	_character_window2.Destroy();
	_character_window3.Destroy();
	_inventory_window.Destroy();
	_status_window.Destroy();
	_skills_window.Destroy();
	_main_options_window.Destroy();
	_equip_window.Destroy();
	_formation_window.Destroy();

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

	// Show all windows (make them visible)
	_bottom_window.Show();
	_main_options_window.Show();
	_character_window0.Show();
	_character_window1.Show();
	_character_window2.Show();
	_character_window3.Show();
	_inventory_window.Show();
	_status_window.Show();
	_skills_window.Show();
	_equip_window.Show();
	_formation_window.Show();

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
	ob->SetCursorOffset(-52.0f, -20.0f);
}



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

	// Disable unused options
	_main_options.EnableOption(1, false);
	_main_options.EnableOption(2, false);
	_main_options.EnableOption(4, false);
}



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

	// Add strings and set default selection.
	_menu_inventory.SetOptions(options);
	_menu_inventory.SetSelection(INV_USE);
}


void MenuMode::_SetupSkillsOptionBox()
{
	// Setup the option box
	this->_SetupOptionBoxCommonSettings(&_menu_skills);
	_menu_skills.SetSize(SKILLS_SIZE, 1);

	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeUnicodeString("Use"));
	options.push_back(MakeUnicodeString("Cancel"));

	// Add strings and set default selection.
	_menu_skills.SetOptions(options);
	_menu_skills.SetSelection(SKILLS_USE);
}



void MenuMode::_SetupEquipOptionBox()
{
	// Setup the status option box
	this->_SetupOptionBoxCommonSettings(&_menu_equip);
	_menu_equip.SetCellSize(150.0f, 50.0f);
	_menu_equip.SetSize(EQUIP_SIZE, 1);

	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeUnicodeString("Equip"));
	options.push_back(MakeUnicodeString("Remove"));
	options.push_back(MakeUnicodeString("Cancel"));

	// Add strings and set default selection.
	_menu_equip.SetOptions(options);
	_menu_equip.SetSelection(EQUIP_EQUIP);

	// Disable unused options
	_menu_equip.EnableOption(EQUIP_REMOVE, false);
}

void MenuMode::_SetupStatusOptionBox()
{
	// Setup the status option box
	this->_SetupOptionBoxCommonSettings(&_menu_status);
	_menu_status.SetSize(STATUS_SIZE, 1);

	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeUnicodeString("View"));
	options.push_back(MakeUnicodeString("Cancel"));

	// Add strings and set default selection.
	_menu_status.SetOptions(options);
	_menu_status.SetSelection(STATUS_VIEW);
}



void MenuMode::_SetupOptionsOptionBox()
{
	// Setup the options option box
	this->_SetupOptionBoxCommonSettings(&_menu_options);
	_menu_options.SetSize(OPTIONS_SIZE, 1);

	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeUnicodeString("Edit"));
	options.push_back(MakeUnicodeString("Save"));
	options.push_back(MakeUnicodeString("Cancel"));

	// Add strings and set default selection.
	_menu_options.SetOptions(options);
	_menu_options.SetSelection(OPTIONS_EDIT);
}



void MenuMode::_SetupSaveOptionBox()
{
	// setup the save options box
	this->_SetupOptionBoxCommonSettings(&_menu_save);
	_menu_save.SetSize(SAVE_SIZE, 1);

	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeUnicodeString("Save"));
	options.push_back(MakeUnicodeString("Cancel"));

	// Add strings and set default selection.
	_menu_save.SetOptions(options);
	_menu_save.SetSelection(SAVE_SAVE);
}

////////////////////////////////////////////////////////////////////////////////
// MenuMode class -- Update Code
////////////////////////////////////////////////////////////////////////////////

void MenuMode::Update()
{
	// See if inventory window is active
	//FIX ME: Use a var to track active window and switch?

	if (_inventory_window.IsActive()) {
		//Update inventory window
		_inventory_window.Update();
		return;
	}
	else if (_status_window.IsActive()) {
		// Update status window.
		_status_window.Update();
		return;
	}
	else if (_skills_window.IsActive()) {
		// Update skills window
		_skills_window.Update();
		return;
	}
	else if (_equip_window.IsActive()) {
		// Update equipment window
		_equip_window.Update();
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
			_current_menu_showing = SHOW_MAIN;
			_current_menu = &_main_options;
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

	// Get the latest event from the current menu
	int32 event = _current_menu->GetEvent();

	// If confirm was pressed
	if (event == VIDEO_OPTION_CONFIRM)
	{
		// Handle options for the current menu
		switch (_current_menu_showing)
		{
			case SHOW_MAIN:
				this->_HandleMainMenu();
				break;
			case SHOW_INVENTORY:
				this->_HandleInventoryMenu();
				break;
			case SHOW_SKILLS:
				this->_HandleSkillsMenu();
				break;
			case SHOW_STATUS:
				this->_HandleStatusMenu();
				break;
			case SHOW_EQUIP:
				this->_HandleEquipMenu();
				break;
			/*case SHOW_OPTIONS:
				this->_HandleOptionsMenu();
				break;
			case SHOW_SAVE:
				this->_HandleSaveMenu();
				break;*/
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
	// Draw the saved screen background
	// For that, set the system coordinates to the size of the window (same with the save-screen)
	int32 width = VideoManager->GetWidth();
	int32 height = VideoManager->GetHeight();
	VideoManager->SetCoordSys (0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	Color grayed(0.35f, 0.35f, 0.35f, 1.0f);
	VideoManager->Move(0, 0);
	VideoManager->DrawImage(_saved_screen);

	// Restore the Coordinate system (that one is menu mode coodinate system)
	VideoManager->SetCoordSys (0.0f, 1024.0f, 768.0f, 0.0f);


	uint32 drawwindow;

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);

	// Move to the top left corner
	VideoManager->Move(0,0);

	// Set the text colour to white
	VideoManager->SetTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));

	//FIX ME:  Test
	_DrawBottomMenu();
	_main_options_window.Draw();

	// Detects which option is highlighted in main menu choices and sets that to the current window
	// to draw
	if (_current_menu_showing == SHOW_MAIN) {
		drawwindow = _current_menu->GetSelection() + 1;
	}
	else {
		drawwindow = _current_menu_showing;
	}

	// Draw the chosen window
	switch (drawwindow)
	{
		case SHOW_MAIN: break;

		case SHOW_INVENTORY:
		{
			_inventory_window.Draw();
			break;
		}
		case SHOW_STATUS:
		{
			_status_window.Draw();
			break;
		}
		case SHOW_SKILLS:
		{
			_skills_window.Draw();
			break;
		}
		case SHOW_EQUIP:
		{
			_equip_window.Draw();
			break;
		}
		/*case SHOW_OPTIONS:
		{
			//this->_HandleOptionsMenu();
			break;
		}*/
		//FIXME
		case SHOW_FORMATION:
		case SHOW_EXIT:
		{
			_formation_window.Draw();
			break;
		}
		/*case SHOW_SAVE:
		{
			//this->_HandleSaveMenu();
			break;
		}*/
	}

	// Draw character windows
	_character_window0.Draw();
	_character_window1.Draw();
	_character_window2.Draw();
	_character_window3.Draw();

	// Draw currently active options box
	_current_menu->Draw();
} // void MenuMode::Draw()


//FIX ME:  Make dynamic, move category id and select state enums to this class
void MenuMode::_DrawBottomMenu() {
	_bottom_window.Draw();

	/*VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->Move(150, 577);

	if (_current_menu_showing == SHOW_INVENTORY || _current_menu_showing == SHOW_EQUIP
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
	}


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
	std::string money = std::string("Drunes: ") + os_money.str();
	VideoManager->MoveRelative(0, 30);
	if (!VideoManager->DrawText(MakeUnicodeString(money)))
		cerr << "MENU: ERROR > Couldn't draw text!" << endl;

	//VideoManager->SetDrawFlags(VIDEO_X_RIGHT, VIDEO_Y_BOTTOM, 0);

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);

	VideoManager->Move(390, 685);
	VideoManager->DrawImage(_location_picture);*/
} // void MenuMode::_DrawBottomMenu()



//FIX ME:  Adjust for new layout
void MenuMode::_HandleMainMenu()
{

	// Change the based on which option was selected.
	switch (_main_options.GetSelection())
	{
		case MAIN_INVENTORY:
		{
			_current_menu_showing = SHOW_INVENTORY;
			_current_menu = &_menu_inventory;
			break;
		}
		case MAIN_SKILLS:
		{
			_current_menu_showing = SHOW_SKILLS;
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
			_current_menu_showing = SHOW_STATUS;
			_current_menu = &_menu_status;
			break;
		}
		case MAIN_EQUIP:
		{
			_current_menu_showing = SHOW_EQUIP;
			_current_menu = &_menu_equip;
			break;
		}
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

// Handles the status menu
void MenuMode::_HandleStatusMenu() {
	switch (_menu_status.GetSelection()) {
		case STATUS_VIEW:
			_status_window.Activate(true);
			break;
		case STATUS_CANCEL:
			_current_menu_showing = SHOW_MAIN;
			_current_menu = &_main_options;
			break;
		default:
			cerr << "MENU: ERROR: Invalid option in MenuMode::HandleStatusMenu()!" << endl;
			break;
	}
}

// Handles the inventory menu
void MenuMode::_HandleInventoryMenu()
{
	switch (_menu_inventory.GetSelection())
	{
		case INV_USE:
			// FIXME: Reenable
			// Make sure we have some items in the inventory.
			//if (GlobalManager->GetInventory().size() == 0)
			//	return;
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
			_current_menu = &_main_options;
			break;
		}
		default:
			cerr << "MENU: ERROR: Invalid option in MenuMode::HandleInventoryMenu()!" << endl;
			break;
	}
}

// Handles the skills menu
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
			_skills_window.Activate(true);
			_current_menu->SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
			break;
		}
		default:
			cerr << "MENU: ERROR: Invalid option in MenuMode::HandleSkillsMenu()!" << endl;
			break;
	}
}

// Handles the equipment menu
void MenuMode::_HandleEquipMenu()
{
	switch (_menu_equip.GetSelection())
	{
		case EQUIP_EQUIP:
			_equip_window.Activate(true);
			_current_menu->SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
			break;
		case EQUIP_REMOVE:
			// TODO: Handle the remove command
			cout << "MENU: Remove command!" << endl;
			break;
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
}

// Handles the options menu
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
}

// Handles the save menu
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
}

} // namespace hoa_menu
