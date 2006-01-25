///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    menu_views.cpp
 * \author  Daniel Steuernol steu@allacrost.org
 * \date    Last Updated: January 24th, 2006
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
#include "menu_views.h"

using namespace std;
using namespace hoa_menu::private_menu;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_engine;
using namespace hoa_global;
using namespace hoa_data;

namespace hoa_menu {
	
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
	
	// Get the instance manager
	GlobalCharacter *character = GlobalManager->GetCharacter(_char_id);
	
	// Draw name first
	VideoManager->Move(x + 34, y + 40);
	if (!VideoManager->DrawText(character->GetName()))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw Character Name!" << endl;
	
	// Draw Level
	VideoManager->MoveRelative(150, 0);
	// Get the char's lvl
	std::ostringstream os_level;
	os_level << character->GetXPLevel();
	std::string xp_level = std::string("Level: ") + os_level.str();
	if (!VideoManager->DrawText(MakeWideString(xp_level)))
		cerr << "CHARACTERWINDOW: ERROR: > Couldn't draw xp level" << endl;
	
	
	// Draw Portrait
	VideoManager->Move(x + 16 + 118 - (_portrait.GetWidth() / 2), y + 80);
	VideoManager->DrawImage(_portrait);
	
	
	// Draw Health
	VideoManager->Move(x + 34, y + 450);
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
	VideoManager->DeleteImage(this->_portrait);
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
	this->_portrait.SetDimensions(200, 350);
	//this->_portrait.SetDimensions(Get_The_Dimensions);
	// Load image into VideoManager
	VideoManager->LoadImage(this->_portrait);
}

//-------------------------------------
// InventoryWindow::InventoryWindow
//-------------------------------------
InventoryWindow::InventoryWindow() : _inventory_active(false)
{
	////////////////////////////////////////////////////////
	/////////////// DELETE THIS ////////////////////////////
	// ONCE INVENTORY IS ADDING THROUGH THE RIGHT SPOT /////
	////////////////////////////////////////////////////////
	GlobalManager->AddItemToInventory(new GlobalItem("Item 1", GLOBAL_ALL_CHARACTERS, GLOBAL_RECOVERY_ITEM, 1, 1));
	GlobalManager->AddItemToInventory(new GlobalItem("Item 2", GLOBAL_ALL_CHARACTERS, GLOBAL_RECOVERY_ITEM, 2, 4));
	GlobalManager->AddItemToInventory(new GlobalItem("Item 3", GLOBAL_ALL_CHARACTERS, GLOBAL_RECOVERY_ITEM, 3, 7));
	////////////////////////////////////////////////////////
	/////////////// DELETE THIS ////////////////////////////
	// ONCE INVENTORY IS ADDING THROUGH THE RIGHT SPOT /////
	////////////////////////////////////////////////////////
	
	// Set up the inventory option box
	_inventory_items.SetCellSize(150.0f, 50.0f);
	// This is dependant on the number of inventory items?
	// or maybe have blank items.
	//_inventory_items.SetSize(2, 10);
	_inventory_items.SetPosition(40.0f, 40.0f);
	_inventory_items.SetFont("default");
	//_inventory_items.SetSelectMode(VIDEO_SELECT_SINGLE);
	_inventory_items.SetCursorOffset(-35.0f, -4.0f);
	_inventory_items.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_inventory_items.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	// Get the inventory items
	vector<GlobalObject *> inv = GlobalManager->GetInventory();
	
	// Set the size of the option box
	// Calculate the number of rows, this is dividing by 6, and if there is a remainder > 0
	// add one more row for the remainder.
	_inventory_items.SetSize(6, inv.size() / 6 + ((inv.size() % 6) > 0 ? 1 : 0));
	
	vector<ustring> inv_names;
	
	for (int i = 0; i < inv.size(); ++i)
	{
		// TODO: Eventually include the icon for the item.
		ostringstream os_obj_count;
		os_obj_count << inv[i]->GetCount();
		// Add in a bunch of spaces on the end to provide some spacing between columns
		string inv_item_str = string("<L>") + inv[i]->GetName() + string("<R>") + os_obj_count.str() + string("                       ");		
		inv_names.push_back(MakeWideString(inv_item_str));
	}
	
	_inventory_items.SetOptions(inv_names);
	// Make sure we have at least one item before setting selection
	// Constants are not used because the size of the inventory is changed
	// dynamically
	if (inv.size() > 0)
		_inventory_items.SetSelection(0);
	
	// Initially hide the cursor
	_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	
}

//-------------------------------------
// InventoryWindow::~InventoryWindow
//-------------------------------------
InventoryWindow::~InventoryWindow()
{
}

//-------------------------------------
// InventoryWindow::Draw()
//-------------------------------------
bool InventoryWindow::Draw()
{
	if (MenuWindow::Draw() == false)
		return false;
	
	// Draw the inventory text
	_inventory_items.Draw();
	
	return true;
}

//-------------------------------------
// InventoryWindow::Update()
//-------------------------------------
void InventoryWindow::Update()
{
	// Check input values
	if (InputManager->ConfirmPress())
	{
		_inventory_items.HandleConfirmKey();
	}
	else if (InputManager->LeftPress())
	{
		_inventory_items.HandleLeftKey();
	}
	else if (InputManager->RightPress())
	{
		_inventory_items.HandleRightKey();
	}
	else if (InputManager->UpPress())
	{
		_inventory_items.HandleUpKey();
	}
	else if (InputManager->DownPress())
	{
		_inventory_items.HandleDownKey();
	}
	
	// Handle Event from option box
	int32 event = _inventory_items.GetEvent();
	
	// Take appropriate action based on event!
}

//-------------------------------------------
// InventoryWindow::Activate()
//------------------------------------------
void InventoryWindow::Activate(bool new_status)
{
	// Set new status
	_inventory_active = new_status; 
	// update cursor state
	if (_inventory_active)
		_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	else
		_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}

} // end namespace hoa_menu
