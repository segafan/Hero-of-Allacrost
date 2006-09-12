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
	
	vector<GlobalCharacter*> characters = GlobalManager->GetParty();
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
	}

	//////////// Setup the menu windows
	uint32 start_x = (1024 - 800) / 2 - 40;
	uint32 start_y = (768 - 600) / 2;
	uint32 win_width = 208;
	uint32 win_height = 600 - 192;

	_bottom_window.Create(208 * 4 + 16, 192 + 16, ~VIDEO_MENU_EDGE_TOP);
	_bottom_window.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y) + static_cast<float>(win_height) - 16);


	// Width of each character window is 200 px.
	// Each char window will have an additional 16 px for the left border 
	// The 4th (last) char window will have another 16 px for the right border
	// Height of the char window is 408 px.
	// The bottom window in the main view is 192 px high, and the full width which will be 216 * 4 + 16
	_character_window0.Create(static_cast<float>(win_width) + 16, static_cast<float>(win_height),
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

	// Setup the inventory window
	_inventory_window.Create(static_cast<float>(win_width * 4 + 16), static_cast<float>(win_height),
		VIDEO_MENU_EDGE_ALL, VIDEO_MENU_EDGE_BOTTOM);
	_inventory_window.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y));

	// Setup the status window
	_status_window.Create(static_cast<float>(win_width * 4 + 16), static_cast<float>(win_height),
		VIDEO_MENU_EDGE_ALL, VIDEO_MENU_EDGE_BOTTOM);
	_status_window.SetPosition(static_cast<float>(start_x), static_cast<float>(start_y));

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
	_character_window0.Show();
	_character_window1.Show();
	_character_window2.Show();
	_character_window3.Show();
	_inventory_window.Show();
	_status_window.Show();
	
	// Setup OptionBoxes
	this->_SetupMainOptionBox();
	this->_SetupInventoryOptionBox();
	this->_SetupSkillsOptionBox();
	this->_SetupStatusEquipOptionBox();
	this->_SetupOptionsOptionBox();
	this->_SetupSaveOptionBox();
} // void MenuMode::Reset()



void MenuMode::_SetupOptionBoxCommonSettings(OptionBox *ob)
{
	// Set all the default options
	ob->SetFont("default");
	ob->SetCellSize(115.0f, 50.0f);
	ob->SetPosition(102.0f, 515.0f);
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
	options.push_back(MakeUnicodeString("Options"));
	options.push_back(MakeUnicodeString("Save"));
	options.push_back(MakeUnicodeString("Exit"));
	
	// Add strings and set default selection.
	_main_options.SetOptions(options);
	_main_options.SetSelection(MAIN_INVENTORY);

	// disable unused options 
	_main_options.EnableOption(1, false);
	_main_options.EnableOption(3, false);
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
	options.push_back(MakeUnicodeString("Cancel"));
	
	_menu_skills.SetOptions(options);
	_menu_skills.SetSelection(SKILLS_CANCEL);
} // void MenuMode::_SetupSkillsOptionBox()



void MenuMode::_SetupStatusEquipOptionBox()
{
	// setup the status option box
	this->_SetupOptionBoxCommonSettings(&_menu_status_equip);
	_menu_status_equip.SetCellSize(150.0f, 50.0f);
	_menu_status_equip.SetSize(STATUS_EQUIP_SIZE, 1);
	
	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeUnicodeString("Equip"));
	options.push_back(MakeUnicodeString("Remove"));
	options.push_back(MakeUnicodeString("Next"));
	options.push_back(MakeUnicodeString("Previous"));
	options.push_back(MakeUnicodeString("Cancel"));
	
	_menu_status_equip.SetOptions(options);
	_menu_status_equip.SetSelection(STATUS_EQUIP_EQUIP);
// 	_menu_status_equip.SetCursor(
} // void MenuMode::_SetupStatusEquipOptionBox()



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
		if (_inventory_window.CanCancel() && InputManager->CancelPress())
		{
			// Play sound
			_menu_sounds["cancel"].PlaySound();
			_inventory_window.Activate(false);
			_current_menu->SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			return;
		}
		
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
			case SHOW_STATUS_EQUIP:
				this->_HandleStatusEquipMenu();
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
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);

	// Move to the top left corner
	VideoManager->Move(0,0);

	// Set the text colour to white
	VideoManager->SetTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
	
	// Draw the saved screen as the menu background
	VideoManager->DrawImage(_saved_screen);
	
	_DrawBottomMenu();

	// Draw the four character menus
	switch (_current_menu_showing)
	{
		case SHOW_MAIN:
		{
			_character_window0.Draw();
			_character_window1.Draw();
			_character_window2.Draw();
			_character_window3.Draw();
			break;
		}
		case SHOW_INVENTORY:
		{
			_inventory_window.Draw();
			break;
		}
		case SHOW_STATUS_EQUIP:
		{
			_status_window.Draw();
			break;
		}
	}

	// Draw currently active options box
	_current_menu->Draw();
} // void MenuMode::Draw()



void MenuMode::_DrawBottomMenu() {
	_bottom_window.Draw();
	
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);

	// Display Location
	VideoManager->Move(150, 575);
	if (!VideoManager->DrawText("Desert Cave"))
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
	
	VideoManager->SetDrawFlags(VIDEO_X_RIGHT, VIDEO_Y_BOTTOM, 0);
	

	
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
		
	VideoManager->Move(350, 670);
	VideoManager->DrawImage(_location_picture);
} // void MenuMode::_DrawBottomMenu()



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
		case MAIN_OPTIONS:
		{
			_current_menu_showing = SHOW_OPTIONS;
			_current_menu = &_menu_options;
			break;
		}
		case MAIN_STATUS_EQUIP:
		{
			_current_menu_showing = SHOW_STATUS_EQUIP;
			_current_menu = &_menu_status_equip;
			break;
		}
		case MAIN_SAVE:
		{
			_current_menu_showing = SHOW_SAVE;
			_current_menu = &_menu_save;
			break;
		}
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



void MenuMode::_HandleInventoryMenu()
{
	switch (_menu_inventory.GetSelection())
	{
		case INV_USE:
			// Make sure we have some items in the inventory.
			if (GlobalManager->GetInventory().size() == 0)
				return;
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
			_current_menu = &_main_options;
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
		default:
			cerr << "MENU: ERROR: Invalid option in MenuMode::HandleSkillsMenu()!" << endl;
			break;
	}
} // void MenuMode::_HandleSkillsMenu()



void MenuMode::_HandleStatusEquipMenu()
{
	switch (_menu_status_equip.GetSelection())
	{
		case STATUS_EQUIP_EQUIP:
			// TODO: Handle the equip command
			cout << "MENU: Equip command!" << endl;
			break;
		case STATUS_EQUIP_REMOVE:
			// TODO: Handle the remove command
			cout << "MENU: Remove command!" << endl;
			break;
		case STATUS_EQUIP_NEXT:
			// TODO: Handle the status - next command.
			cout << "MENU: Status Next command!" << endl;
			break;
		case STATUS_EQUIP_PREV:
			// TODO: Handle the status - prev command.
			cout << "MENU: Status Prev command!" << endl;
			break;
		case STATUS_EQUIP_CANCEL:
		{
			_current_menu_showing = SHOW_MAIN;
			_current_menu = &_main_options;
			break;
		}
		default:
			cerr << "MENU: ERROR: Invalid option in MenuMode::HandleStatusMenu()!" << endl;
			break;
	}
} // void MenuMode::_HandleStatusEquipMenu()



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
