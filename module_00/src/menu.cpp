///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    menu.cpp
 * \author  Daniel Steuernol steu@allacrost.org
 * \date    Last Updated: January 15th, 2006
 * \brief   Source file for menu mode interface.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include <sstream>
#include "menu.h"
#include "audio.h"
#include "video.h"
#include "engine.h"
#include "global.h"
#include "data.h"

using namespace std;
using namespace hoa_menu::private_menu;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_engine;
using namespace hoa_global;
using namespace hoa_data;



namespace hoa_menu {

bool MENU_DEBUG = false;

//--------------------------
// MenuMode::MenuMode
//--------------------------
MenuMode::MenuMode() 
{
	if (MENU_DEBUG) cout << "MENU: MenuMode constructor invoked." << endl;

	// Save the currently drawn screen
	if (!VideoManager->CaptureScreen(_saved_screen)) {
		cerr << "MENU: ERROR: Couldn't save the screen!" << endl;
	}
	
	// DELETE THIS TOO!!
	GlobalCharacter *laila = new GlobalCharacter("Laila", "laila", GLOBAL_LAILA);
	GlobalManager->AddCharacter(laila);
	
	vector<GlobalCharacter *> characters = GlobalManager->GetParty();
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
	
	// Set the default menu to the main option box
	_current_menu_showing = SHOW_MAIN;
	_current_menu = &_main_options;
	
	// Set Font
	_font_name = "default";
	
	// DELETE this when we have real data.
	GlobalManager->GetCharacter(hoa_global::GLOBAL_CLAUDIUS)->SetHP(80);
	GlobalManager->GetCharacter(hoa_global::GLOBAL_CLAUDIUS)->SetMaxHP(340);
	GlobalManager->GetCharacter(hoa_global::GLOBAL_CLAUDIUS)->SetSP(35);
	GlobalManager->GetCharacter(hoa_global::GLOBAL_CLAUDIUS)->SetMaxSP(65);
	GlobalManager->GetCharacter(hoa_global::GLOBAL_CLAUDIUS)->SetXP(35);
	GlobalManager->GetCharacter(hoa_global::GLOBAL_CLAUDIUS)->SetXPNextLevel(156);
	GlobalManager->GetCharacter(hoa_global::GLOBAL_CLAUDIUS)->SetXPLevel(100);
	GlobalManager->GetCharacter(hoa_global::GLOBAL_LAILA)->SetHP(300);
	GlobalManager->GetCharacter(hoa_global::GLOBAL_LAILA)->SetMaxHP(440);
	GlobalManager->GetCharacter(hoa_global::GLOBAL_LAILA)->SetSP(300);
	GlobalManager->GetCharacter(hoa_global::GLOBAL_LAILA)->SetMaxSP(370);
	GlobalManager->GetCharacter(hoa_global::GLOBAL_LAILA)->SetXP(124);
	GlobalManager->GetCharacter(hoa_global::GLOBAL_LAILA)->SetXPNextLevel(357);
	GlobalManager->GetCharacter(hoa_global::GLOBAL_LAILA)->SetXPLevel(75);
	GlobalManager->SetMoney(4236);
}


MenuMode::~MenuMode() {
	if (MENU_DEBUG) cout << "MENU: MenuMode destructor invoked." << endl;
	
	// Remove saved images
	VideoManager->DeleteImage(_saved_screen);
	
	for (uint32 i = 0; i < _menu_images.size(); i++) {
		VideoManager->DeleteImage(_menu_images[i]);
	}
	
	// Destroy menu windows
	_character_window0.Destroy();
	_character_window1.Destroy();
	_character_window2.Destroy();
	_character_window3.Destroy();
	_bottom_window.Destroy();
}

//-----------------------------------------
// MenuMode::SetupOptionBoxCommonSettings
//-----------------------------------------
void MenuMode::_SetupOptionBoxCommonSettings(OptionBox *ob)
{
	// Set all the default options
	ob->SetFont(_font_name);
	ob->SetCellSize(128.0f, 50.0f);
	ob->SetPosition(30.0f, 625.0f);
	ob->SetAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	ob->SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	ob->SetSelectMode(VIDEO_SELECT_SINGLE);
	ob->SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	ob->SetCursorOffset(-35.0f, 5.0f);
}

//-------------------------------
// MenuMode::SetupMainOptionBox
//-------------------------------
void MenuMode::_SetupMainOptionBox()
{
	// Setup the main options box
	this->_SetupOptionBoxCommonSettings(&_main_options);
	_main_options.SetSize(MAIN_SIZE, 1);
	
	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeWideString("Inventory"));
	options.push_back(MakeWideString("Skills"));
	options.push_back(MakeWideString("Equipment"));
	options.push_back(MakeWideString("Status"));
	options.push_back(MakeWideString("Options"));
	options.push_back(MakeWideString("Save"));
	options.push_back(MakeWideString("Exit"));
	
	// Add strings and set default selection.
	_main_options.SetOptions(options);
	_main_options.SetSelection(MAIN_INVENTORY);
}

//-------------------------------------
// MenuMode::SetupInventoryOptionBox
//-------------------------------------
void MenuMode::_SetupInventoryOptionBox()
{
	// Setup the option box
	this->_SetupOptionBoxCommonSettings(&_menu_inventory);
	_menu_inventory.SetSize(INV_SIZE, 1);
	
	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeWideString("Use"));
	options.push_back(MakeWideString("Sort"));
	options.push_back(MakeWideString("Cancel"));
	
	_menu_inventory.SetOptions(options);
	_menu_inventory.SetSelection(INV_USE);
}

//------------------------------------
// MenuMode::SetupSkillsOptionBox
//------------------------------------
void MenuMode::_SetupSkillsOptionBox()
{
	// setup the option box
	this->_SetupOptionBoxCommonSettings(&_menu_skills);
	_menu_skills.SetSize(SKILLS_SIZE, 1);
	
	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeWideString("Cancel"));
	
	_menu_skills.SetOptions(options);
	_menu_skills.SetSelection(SKILLS_CANCEL);
}

//------------------------------------
// MenuMode::SetupEquipmentOptionBox
//------------------------------------
void MenuMode::_SetupEquipmentOptionBox()
{
	// setup the option box
	this->_SetupOptionBoxCommonSettings(&_menu_equipment);
	_menu_equipment.SetSize(EQUIP_SIZE, 1);
	
	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeWideString("Equip"));
	options.push_back(MakeWideString("Remove"));
	options.push_back(MakeWideString("Cancel"));
	
	_menu_equipment.SetOptions(options);
	_menu_equipment.SetSelection(EQUIP_EQUIP);
}

//------------------------------------
// MenuMode::SetupStatusOptionBox
//------------------------------------
void MenuMode::_SetupStatusOptionBox()
{
	// setup the status option box
	this->_SetupOptionBoxCommonSettings(&_menu_status);
	_menu_status.SetSize(STATUS_SIZE, 1);
	
	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeWideString("Next Character"));
	options.push_back(MakeWideString("Prev Character"));
	options.push_back(MakeWideString("Cancel"));
	
	_menu_status.SetOptions(options);
	_menu_status.SetSelection(STATUS_NEXT);
}

//-------------------------------------
// MenuMode::SetupOptionsOptionBox
//-------------------------------------
void MenuMode::_SetupOptionsOptionBox()
{
	// setup the options option box
	this->_SetupOptionBoxCommonSettings(&_menu_options);
	_menu_options.SetSize(OPTIONS_SIZE, 1);
	
	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeWideString("Edit"));
	options.push_back(MakeWideString("Save"));
	options.push_back(MakeWideString("Cancel"));
	
	_menu_options.SetOptions(options);
	_menu_options.SetSelection(OPTIONS_EDIT);
}

//-------------------------------------
// MenuMode::SetupSaveOptionBox
//-------------------------------------
void MenuMode::_SetupSaveOptionBox()
{
	// setup the save options box
	this->_SetupOptionBoxCommonSettings(&_menu_save);
	_menu_save.SetSize(SAVE_SIZE, 1);
	
	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeWideString("Save"));
	options.push_back(MakeWideString("Cancel"));
	
	_menu_save.SetOptions(options);
	_menu_save.SetSelection(SAVE_SAVE);
}

// Resets appropriate class members
void MenuMode::Reset() {
	VideoManager->SetCoordSys(0, 1024, 768, 0); // Top left corner coordinates are (0,0)
	if(!VideoManager->SetFont(_font_name)) 
		cerr << "MAP: ERROR > Couldn't set menu font!" << endl;
	
	// Setup the menu windows
	_character_window0.Create(256, 576, ~VIDEO_MENU_EDGE_RIGHT);
	_character_window0.SetPosition(0, 0);
	_character_window0.Show();
	_character_window1.Create(256, 576, ~VIDEO_MENU_EDGE_RIGHT, VIDEO_MENU_EDGE_LEFT);
	_character_window1.SetPosition(256, 0);
	_character_window1.Show();
	_character_window2.Create(256, 576, ~VIDEO_MENU_EDGE_RIGHT, VIDEO_MENU_EDGE_LEFT);
	_character_window2.SetPosition(512, 0);
	_character_window2.Show();
	_character_window3.Create(256, 576, VIDEO_MENU_EDGE_ALL, VIDEO_MENU_EDGE_LEFT);
	_character_window3.SetPosition(768, 0);
	_character_window3.Show();
	_bottom_window.Create(1024, 192);
	_bottom_window.SetPosition(0, 576);
	_bottom_window.Show();
	
	// Setup OptionBoxes
	this->_SetupMainOptionBox();
	this->_SetupInventoryOptionBox();
	this->_SetupSkillsOptionBox();
	this->_SetupEquipmentOptionBox();
	this->_SetupStatusOptionBox();
	this->_SetupOptionsOptionBox();
	this->_SetupSaveOptionBox();
}

//------------------------------------------
// MenuMode::Update
//-------------------------------------------
void MenuMode::Update(uint32 time_elapsed) {

	if (InputManager->CancelPress()) 
	{
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
			case SHOW_EQUIPMENT:
				this->_HandleEquipmentMenu();
				break;
			case SHOW_SKILLS:
				this->_HandleSkillsMenu();
				break;
			case SHOW_STATUS:
				this->_HandleStatusMenu();
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
}

//-------------------------------
// MenuMode::HandleMainMenu
//-------------------------------
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
		case MAIN_EQUIPMENT:
		{
			_current_menu_showing = SHOW_EQUIPMENT;
			_current_menu = &_menu_equipment;
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
		case MAIN_STATUS:
		{
			_current_menu_showing = SHOW_STATUS;
			_current_menu = &_menu_status;
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
}

//--------------------------------
// MenuMode::HandleInventoryMenu
//--------------------------------
void MenuMode::_HandleInventoryMenu()
{
	switch (_menu_inventory.GetSelection())
	{
		case INV_USE:
			// TODO: Handle the use inventory command
			cout << "MENU: Inventory Use command!" << endl;
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

//--------------------------------
// MenuMode::HandleEquipmentMenu
//--------------------------------
void MenuMode::_HandleEquipmentMenu()
{
	switch (_menu_equipment.GetSelection())
	{
		case EQUIP_EQUIP:
			// TODO: Handle the equip equipment command
			cout << "MENU: Equipment Equip command!" << endl;
			break;
		case EQUIP_REMOVE:
			// TODO: Handle the remove equipment command
			cout << "MENU: Equipment Remove command!" << endl;
			break;
		case EQUIP_CANCEL:
		{
			_current_menu_showing = SHOW_MAIN;
			_current_menu = &_main_options;
			break;
		}
		default:
			cerr << "MENU: ERROR: Invalid option in MenuMode::HandleEquipmentMenu()!" << endl;
			break;
	}
}

//--------------------------------
// MenuMode::HandleSkillsMenu
//--------------------------------
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
}

//----------------------------------
// MenuMode::HandleStatusMenu
//----------------------------------
void MenuMode::_HandleStatusMenu()
{
	switch (_menu_status.GetSelection())
	{
		case STATUS_NEXT:
			// TODO: Handle the status - next command.
			cout << "MENU: Status Next command!" << endl;
			break;
		case STATUS_PREV:
			// TODO: Handle the status - prev command.
			cout << "MENU: Status Prev command!" << endl;
			break;
		case STATUS_CANCEL:
		{
			_current_menu_showing = SHOW_MAIN;
			_current_menu = &_main_options;
			break;
		}
		default:
			cerr << "MENU: ERROR: Invalid option in MenuMode::HandleStatusMenu()!" << endl;
			break;
	}
}

//----------------------------------
// MenuMode::HandleOptionsMenu
//----------------------------------
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

//---------------------------------
// MenuMode::HandleSaveMenu
//---------------------------------
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

//----------------------
// MenuMode::Draw
//----------------------
void MenuMode::Draw() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);
	// Move to the top left corner
	VideoManager->Move(0,0);
	
	// Draw the saved screen as the menu background
	VideoManager->DrawImage(_saved_screen); 
	
	// Draw the four character menus
	_character_window0.Draw();
	_character_window1.Draw();
	_character_window2.Draw();
	_character_window3.Draw();
	_bottom_window.Draw();
	
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	
	// Draw currently active options box
	_current_menu->Draw();
	
	
	// Draw 2nd menu text
	VideoManager->Move(30, 700);
	if (!VideoManager->DrawText("Time: 00:24:35"))
		cerr << "MENU: ERROR > Couldn't draw text!" << endl;
	
	std::ostringstream os_money;
	os_money << GlobalManager->GetMoney();
	std::string money = std::string("Bling:") + os_money.str() + "B";
	VideoManager->MoveRelative(0, 24);
	if (!VideoManager->DrawText(MakeWideString(money)))
		cerr << "MENU: ERROR > Couldn't draw text!" << endl;
	
}

//--------------------------------
// Draw the window to the screen
//-------------------------------
bool CharacterWindow::Draw()
{
	// Call parent Draw method, if failed pass on fail result
	if (MenuWindow::Draw() == false)
		return false;
	// Window is hidden return true
	if (MenuWindow::GetState() == hoa_video::VIDEO_MENU_STATE_HIDDEN)
		return true;
	
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
	
	// Get the window metrics
	float x, y, w, h;
	this->GetPosition(x,y);
	this->GetDimensions(w,h);

	// check to see if this window is an actual character
	if (_char_id == hoa_global::GLOBAL_NO_CHARACTERS)
		// no more to do here
		return true;
	
	// Menu border fudge
	x += 8;
	
	// Get the instance manager
	GlobalCharacter *character = GlobalManager->GetCharacter(_char_id);
	
	// Draw name first
	VideoManager->Move(x + 32, y + 40);
	if (!VideoManager->DrawText(character->GetName()))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw Character Name!" << endl;
	
	// Draw Level
	VideoManager->MoveRelative(140, 0);
	// Get the char's lvl
	std::ostringstream os_level;
	os_level << character->GetXPLevel();
	std::string xp_level = std::string("Level: ") + os_level.str();
	if (!VideoManager->DrawText(MakeWideString(xp_level)))
		cerr << "CHARACTERWINDOW: ERROR: > Couldn't draw xp level" << endl;
	
	
	// Draw Portrait
	VideoManager->Move(x + 128 - (_portrait.GetWidth() / 2), y + 80);
	VideoManager->DrawImage(_portrait);
	
	
	// Draw Health
	VideoManager->Move(x + 32, y + 400);
	// convert to std::string
	std::ostringstream os_health;
	os_health << character->GetHP() << " / " << character->GetMaxHP();
	std::string health = std::string("Health: ") + os_health.str();
	if (!VideoManager->DrawText(MakeWideString(health)))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw health!" << endl;
		
	// Draw skill
	VideoManager->MoveRelative(0, 40);
	
	// convert to std::string
	std::ostringstream os_skill;
	os_skill << character->GetSP() << " / " << character->GetMaxSP();
	std::string skill = std::string("Skill: ") + os_skill.str();
	if (!VideoManager->DrawText(MakeWideString(skill)))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw skill!" << endl;
		
	// Draw xp
	VideoManager->MoveRelative(0, 40);
	
	// Convert to std::string
	std::ostringstream os_xp;
	os_xp << character->GetXPForNextLevel();
	std::string xp = std::string("XP Remaining: ") + os_xp.str();
	if (!VideoManager->DrawText(MakeWideString(xp)))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw xp!" << endl;
	
	return true;
}

//------------------------------------
// CharacterWindow Constructor
//------------------------------------
CharacterWindow::CharacterWindow()
{ 
	_char_id = GLOBAL_NO_CHARACTERS;
}

//-------------------------------------
// CharacterWindow::~CharacterWindow
//-------------------------------------
CharacterWindow::~CharacterWindow()
{ 
	// Delete Our portrait
	//VideoManager->DeleteImage(this->_portrait);
}

//-------------------------------------
// CharacterWindow::SetCharacter
//-------------------------------------
void CharacterWindow::SetCharacter(GlobalCharacter *character)
{
	this->_char_id = character->GetID();
	
	// TODO: Load the portrait
	this->_portrait.SetFilename("img/menus/blank.png");
	this->_portrait.SetStatic(true);
	this->_portrait.SetDimensions(200, 300);
	//this->_portrait.SetDimensions(Get_The_Dimensions);
	// Load image into VideoManager
	VideoManager->LoadImage(this->_portrait);
}

} // namespace hoa_menu
