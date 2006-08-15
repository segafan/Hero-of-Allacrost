///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    menu_views.cpp
 * \author  Daniel Steuernol steu@allacrost.org
 * \brief   Source file for menu mode interface.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include <sstream>
#include "menu.h"
#include "audio.h"
#include "video.h"
#include "global.h"
#include "input.h"
#include "data.h"
#include "menu_views.h"

using namespace std;
using namespace hoa_menu::private_menu;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_global;
using namespace hoa_input;
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
	VideoManager->MoveRelative(95, 0);
	// Get the char's lvl
	std::ostringstream os_level;
	os_level << character->GetXPLevel();
	std::string xp_level = std::string("Level: ") + os_level.str();
	if (!VideoManager->DrawText(MakeWideString(xp_level)))
		cerr << "CHARACTERWINDOW: ERROR: > Couldn't draw xp level" << endl;
	
	
	// Draw Portrait
	VideoManager->Move(x + 16 + 95 - (_portrait.GetWidth() / 2), y + 80);
	VideoManager->DrawImage(_portrait);
	
	
	// Draw Health
	VideoManager->Move(x + 34, y + 300);
	// convert to std::string
	ostringstream os_health;
	os_health << character->GetHP() << " / " << character->GetMaxHP();
	std::string health = std::string("Health: ") + os_health.str();
	if (!VideoManager->DrawText(MakeWideString(health)))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw health!" << endl;
		
	// Draw skill
	VideoManager->MoveRelative(0, 40);
	
	// convert to std::string
	ostringstream os_skill;
	os_skill << character->GetSP() << " / " << character->GetMaxSP();
	std::string skill = std::string("Skill: ") + os_skill.str();
	if (!VideoManager->DrawText(MakeWideString(skill)))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw skill!" << endl;
	
	// Draw xp
	VideoManager->MoveRelative(0, 40);
	
	// Convert to std::string
	ostringstream os_xp;
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
	this->_portrait.SetFilename("img/portraits/menu/" + character->GetFilename() + ".png");
	this->_portrait.SetStatic(true);
	this->_portrait.SetDimensions(150, 200);
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
	GlobalItem *new_item = new GlobalItem(GLOBAL_HP_RECOVERY_ITEM, GLOBAL_ALL_CHARACTERS, HP_POTION, 1);
	new_item->SetRecoveryAmount(20);
	GlobalManager->AddItemToInventory(new_item);
	//GlobalManager->AddItemToInventory(new GlobalWeapon("Broadsword", GLOBAL_ALL_CHARACTERS, 2, 4, "img/icons/sword.png"));
	//GlobalManager->AddItemToInventory(new GlobalArmor("Breastplate", GLOBAL_BODY_ARMOR, GLOBAL_ALL_CHARACTERS, 3, 7, "img/icons/breastplate.png"));
	////////////////////////////////////////////////////////
	/////////////// DELETE THIS ////////////////////////////
	// ONCE INVENTORY IS ADDING THROUGH THE RIGHT SPOT /////
	////////////////////////////////////////////////////////
	
	// Set up the inventory option box
	_inventory_items.SetCellSize(180.0f, 50.0f);
	// This is dependant on the number of inventory items?
	// or maybe have blank items.
	//_inventory_items.SetSize(2, 10);
	_inventory_items.SetPosition((1024-800)/2.0f, (768-600)/2.0f + 40.0f);
	_inventory_items.SetFont("default");
	//_inventory_items.SetSelectMode(VIDEO_SELECT_SINGLE);
	_inventory_items.SetCursorOffset(-35.0f, -4.0f);
	_inventory_items.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_inventory_items.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_inventory_items.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	// Update the item text
	UpdateItemText();
	
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
	
	// Update the item text in case the number of items changed.
	UpdateItemText();

	// Draw the inventory text
	_inventory_items.Draw();

	// Draw the char window
	_char_window.Draw();
	
	return true;
}

//-------------------------------------
// InventoryWindow::UpdateItemText()
//-------------------------------------
void InventoryWindow::UpdateItemText()
{
	// Get the inventory items
	vector<GlobalObject *> inv = GlobalManager->GetInventory();
				
	// Set the size of the option box
	// Calculate the number of rows, this is dividing by row_width, and if there is a remainder > 0
	// add one more row for the remainder.
	uint32 row_width = 4;
	_inventory_items.SetSize(row_width, inv.size() / row_width + ((inv.size() % row_width) > 0 ? 1 : 0));
	
	vector<ustring> inv_names;
	
	for (uint32 i = 0; i < inv.size(); ++i)
	{
		// Create the item text
		ostringstream os_obj_count;
		os_obj_count << inv[i]->GetCount();
		string inv_item_str = string("<") + inv[i]->GetIconPath() + string("><32>") + inv[i]->GetName() + string("<R>") + os_obj_count.str() + string("   ");
		inv_names.push_back(MakeWideString(inv_item_str));
	}
	
	_inventory_items.SetOptions(inv_names);
	// Make sure we have at least one item before setting selection
	// Constants are not used because the size of the inventory is changed
	// dynamically
	if (inv.size() > 0)
		_inventory_items.SetSelection(0);
}

//-------------------------------------
// InventoryWindow::Update()
//-------------------------------------
void InventoryWindow::Update()
{
	// Load sound effects
	SoundDescriptor confirm;
	SoundDescriptor cancel;
	if (confirm.LoadSound("confirm") == false) 
	{
		cerr << "MENUMODE::UPDATE - Unable to load confirm sound effect!" << endl;
	}
	if (cancel.LoadSound("cancel") == false) 
	{
		cerr << "MENUMODE::UPDATE - Unable to load cancel sound effect!" << endl;
	}

	// Character window is active, no processing is needed for 
	// inventory window.
	if (_char_window.IsActive())
	{
		// char select window is now cancelled.
		if (InputManager->CancelPress())
		{
			// deactivate it
			if (_char_window.IsActive())
			{
				cancel.PlaySound();
				_char_window.Activate(false);
				_char_window.Hide();
				// update item text.
				UpdateItemText();

				return;
			}
		}

		_char_window.Update();
		return;
	}

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
	if (event == VIDEO_OPTION_CONFIRM)
	{
		int item_selected = _inventory_items.GetSelection();

		if (GlobalManager->GetInventory()[item_selected]->GetSubClassType() == "GlobalItem")
		{
			GlobalItem *item = (GlobalItem *)GlobalManager->GetInventory()[item_selected];
			if (((item->GetUseCase() & GLOBAL_HP_RECOVERY_ITEM) == GLOBAL_HP_RECOVERY_ITEM) ||
				((item->GetUseCase() & GLOBAL_SP_RECOVERY_ITEM) == GLOBAL_SP_RECOVERY_ITEM))
			{
				// Play Sound
				confirm.PlaySound();
				// Activate char select window
				_char_window.Show();
				_char_window.Activate(true);
				_char_window.SetSelectedIndex(item_selected);
			}
		}
	}
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

//-------------------------------------------
// InventoryWindow::CanCancel()
//------------------------------------------
bool InventoryWindow::CanCancel()
{
	return !_char_window.IsActive();
}

//--------------------------------------------------------
// MiniCharacterSelectwindow::MiniCharacterSelectWindow()
//--------------------------------------------------------
MiniCharacterSelectWindow::MiniCharacterSelectWindow() : _char_window_active(false), _current_char_selected(0)
{
	MenuWindow::Create(300, 472);
	MenuWindow::SetPosition(724, 150);
}

//--------------------------------------------------------
// MiniCharacterSelectwindow::~MiniCharacterSelectWindow()
//--------------------------------------------------------
MiniCharacterSelectWindow::~MiniCharacterSelectWindow()
{
}

//------------------------------------------------------
// MiniCharacterSelectWindow::Draw()
//------------------------------------------------------
bool MiniCharacterSelectWindow::Draw()
{
	if (MenuWindow::Draw() == false)
		return false;

	if (!_char_window_active)
		return true;

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);

	// Draw Portraits of the party
	for (uint32 i = 0; i < GlobalManager->GetParty().size(); ++i)
	{
		VideoManager->Move(765, 180);

		GlobalCharacter *current = GlobalManager->GetParty()[i];
		// Get the portrait from the character eventually
		StillImage portrait;
		// TODO: This needs optimization, move the LoadImage - DeleteImage calls into constructor/destructor
		portrait.SetFilename(string("img/sprites/map/") + string(current->GetFilename()) + string("_d0.png"));
		portrait.SetDimensions(32, 64);
		portrait.SetStatic(true);
		VideoManager->LoadImage(portrait);
		VideoManager->MoveRelative(0.0f, (float)(i * 116));		
		VideoManager->DrawImage(portrait);
		VideoManager->DeleteImage(portrait);

		// Draw Text
		// draw name
		VideoManager->MoveRelative(65, -10);
		if (!VideoManager->DrawText(current->GetName()))
			cerr << "MINICHARACTERWINDOW: Unable to draw character's name" << endl;
		
		// Draw health
		VideoManager->MoveRelative(0, 30);
		ostringstream os_health;
		os_health << current->GetHP() << " / " << current->GetMaxHP();
		std::string health = std::string("Health: ") + os_health.str();
		if (!VideoManager->DrawText(MakeWideString(health)))
			cerr << "MINICHARACTERWINDOW: ERROR > Couldn't draw health!" << endl;

		// Draw skill points
		VideoManager->MoveRelative(0, 30);
		ostringstream os_skill;
		os_skill << current->GetSP() << " / " << current->GetMaxSP();
		std::string skill = std::string("Skill: ") + os_skill.str();
		if (!VideoManager->DrawText(MakeWideString(skill)))
			cerr << "CHARACTERWINDOW: ERROR > Couldn't draw skill!" << endl;

		// This is the char the cursor is currently on, draw it
		if (i == _current_char_selected)
		{
			VideoManager->Move(730.0f, (float)(207 + (i * 116)));
			VideoManager->DrawImage(*(VideoManager->GetDefaultCursor()));
		}
	}

	return true;
}

//------------------------------------------------------
// MiniCharacterSelectWindow::Active(bool)
//------------------------------------------------------
void MiniCharacterSelectWindow::Activate(bool new_status)
{
	// Set new status
	_char_window_active = new_status;
}

//------------------------------------------------------
// MiniCharacterSelectWindow::Update()
//------------------------------------------------------
void MiniCharacterSelectWindow::Update()
{
	// Load sound effects
	SoundDescriptor confirm;
	SoundDescriptor bump;
	if (confirm.LoadSound("obtain") == false) 
	{
		cerr << "MINICHARWINDOW::UPDATE - Unable to load confirm sound effect!" << endl;
	}
	if (bump.LoadSound("bump") == false) 
	{
		cerr << "MINICHARWINDOW::UPDATE - Unable to load bump sound effect!" << endl;
	}

	// Check input values
	if (InputManager->ConfirmPress())
	{
		// Use the passed in item, to update values
		GlobalItem *selected = static_cast<GlobalItem *>(GlobalManager->GetInventory()[_selected_item_index]);
		GlobalCharacter *ch = GlobalManager->GetParty()[_current_char_selected];

		if (selected->GetCount() == 0)
		{
			// no more items to use
			bump.PlaySound();
			return;
		}

		// Play Sound
		confirm.PlaySound();

		// increase hp
		uint8 temp = selected->GetUseCase();
		printf("Use Case:%x\tGLOBAL_HP_RECOVERY_ITEM:%x", temp, GLOBAL_HP_RECOVERY_ITEM);
		if ((selected->GetUseCase() & GLOBAL_HP_RECOVERY_ITEM) == GLOBAL_HP_RECOVERY_ITEM)
		{	
			uint32 new_hp = ch->GetHP();
			new_hp += selected->GetRecoveryAmount();
			if (new_hp > ch->GetMaxHP())
				new_hp = ch->GetMaxHP();
			ch->SetHP(new_hp);
		}

		// increase sp
		printf("Use Case:%x\tGLOBAL_SP_RECOVERY_ITEM:%x", temp, GLOBAL_SP_RECOVERY_ITEM);
		if ((selected->GetUseCase() & GLOBAL_SP_RECOVERY_ITEM) == GLOBAL_SP_RECOVERY_ITEM)
		{
			uint32 new_sp = ch->GetSP();
			new_sp += selected->GetRecoveryAmount();
			if (new_sp > ch->GetMaxSP())
				new_sp = ch->GetMaxSP();
			ch->SetSP(new_sp);
		}

		// decrease item count
		if (selected->GetCount() > 0)
			selected->DecCount(1);
	}
	else if (InputManager->LeftPress())
	{
		
	}
	else if (InputManager->RightPress())
	{
		
	}
	else if (InputManager->UpPress())
	{
		if (_current_char_selected > 0)
			_current_char_selected--;
		else
			_current_char_selected = GlobalManager->GetParty().size() - 1;
	}
	else if (InputManager->DownPress())
	{
		if (_current_char_selected < GlobalManager->GetParty().size() - 1)
			_current_char_selected++;
		else
			_current_char_selected = 0;
	}
}

//----------------------------------
// StatusWindow::StatusWindow()
//----------------------------------
StatusWindow::StatusWindow() : _active(false), _cursor_x(588.0f), _cursor_y(324.0f)
{
	// Get the current character
	_current_char = GlobalManager->GetParty()[0];
	// Set up the head picture
	//string path = string("img/sprites/map/") + string(_current_char->GetName()) + string("_d0.png");
	string head_path = string("img/portraits/map/") + _current_char->GetFilename() + string(".png");
	_head_portrait.SetFilename(head_path);
	_head_portrait.SetStatic(true);
	_head_portrait.SetDimensions(200, 200);
	// Set up the full body portrait
	string full_path = string("img/portraits/menu/") + _current_char->GetFilename() + string("_large.png");
	_full_portrait.SetFilename(full_path);
	_full_portrait.SetStatic(true);
	_full_portrait.SetDimensions(224,350);
	// Load image
	VideoManager->LoadImage(_head_portrait);
	VideoManager->LoadImage(_full_portrait);
}

//----------------------------------
// StatusWindow::~StatusWindow()
//----------------------------------
StatusWindow::~StatusWindow()
{
	VideoManager->DeleteImage(_head_portrait);
	VideoManager->DeleteImage(_full_portrait);
}

//-----------------------------------
// StatusWindow::Draw
//-----------------------------------
bool StatusWindow::Draw()
{
	if (!MenuWindow::Draw())
		return false;

	// Set drawing system
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);

	// window top corner is 72, 84
	VideoManager->Move(92, 104);
	VideoManager->DrawImage(_head_portrait);

	VideoManager->MoveRelative(10, 220);
	VideoManager->DrawText(_current_char->GetName());

	VideoManager->MoveRelative(0, 24);
	// Draw Health
	ostringstream os_health;
	os_health << _current_char->GetHP() << " / " << _current_char->GetMaxHP();
	string health = string("Health: ") + os_health.str();
	if (!VideoManager->DrawText(MakeWideString(health)))
		cerr << "STATUSWINDOW: ERROR > Couldn't draw health!" << endl;
		
	// Draw skill
	VideoManager->MoveRelative(0, 24);
	
	// convert to std::string
	ostringstream os_skill;
	os_skill << _current_char->GetSP() << " / " << _current_char->GetMaxSP();
	string skill = string("Skill: ") + os_skill.str();
	if (!VideoManager->DrawText(MakeWideString(skill)))
		cerr << "STATUSWINDOW: ERROR > Couldn't draw skill!" << endl;
	
	// Draw xp
	VideoManager->MoveRelative(0, 24);
	
	// Convert to std::string
	ostringstream os_xp;
	os_xp << _current_char->GetXPForNextLevel();
	string xp = string("XP Remaining: ") + os_xp.str();
	if (!VideoManager->DrawText(MakeWideString(xp)))
		cerr << "STATUSWINDOW: ERROR > Couldn't draw xp!" << endl;

	// Draw stremgth
	VideoManager->MoveRelative(160, -48);
	ostringstream os_str;
	os_str << _current_char->GetStrength();
	string str = string("Strength: ") + os_str.str();
	if (!VideoManager->DrawText(MakeWideString(str)))
		cerr << "STATUSWINDOW: ERROR > Couldn't draw character strength!" << endl;

	// Draw Intelligence
	VideoManager->MoveRelative(0, 24);
	ostringstream os_int;
	os_int << _current_char->GetIntelligence();
	string intelligence = string("Intelligence: ") + os_int.str();
	if (!VideoManager->DrawText(MakeWideString(intelligence)))
		cerr << "STATUSWINDOW: ERROR > Couldn't draw character intelligence!" << endl;

	// Draw Agility
	VideoManager->MoveRelative(0, 24);
	ostringstream os_agi;
	os_agi << _current_char->GetAgility();
	string agi = string("Agility: ") + os_agi.str();
	if (!VideoManager->DrawText(MakeWideString(agi)))
		cerr << "STATUSWINDOW: ERROR > Couldn't draw character agility!" << endl;

	VideoManager->Move(400, 104);
	VideoManager->DrawImage(_full_portrait);

	// Draw Equipment
	VideoManager->MoveRelative(220, 220);

	// Draw Head Armor
	string head_armour = string("Head Armor: "); // Currently doesn't work + _current_char->GetHeadArmor()->GetName();
	if (!VideoManager->DrawText(MakeWideString(head_armour)))
		cerr << "STATUSWINDOW: ERROR > Couldn't draw head armor text!" << endl;

	// Draw body Armor
	VideoManager->MoveRelative(0, 24);
	string chest_armor = string("Body Armor: ");
	if (!VideoManager->DrawText(MakeWideString(chest_armor)))
		cerr << "STATUSWINDOW: ERROR > Couldn't draw chest armor text!" << endl;

	// Draw Arms Armor
	VideoManager->MoveRelative(0, 24);
	string arms_armor = string("Arms Armor: ");
	if (!VideoManager->DrawText(MakeWideString(arms_armor)))
		cerr << "STATUSWINDOW: ERROR > Couldn't draw arms armor text!" << endl;

	// Draw legs Armor
	VideoManager->MoveRelative(0, 24);
	string legs_armor = string("Legs Armor: ");
	if (!VideoManager->DrawText(MakeWideString(legs_armor)))
		cerr << "STATUSWINDOW: ERROR > Couldn't draw legs armor text!" << endl;

	// Draw weapon
	VideoManager->MoveRelative(0, 24);
	string weapon = string("Weapon: ");
	if (!VideoManager->DrawText(MakeWideString(weapon)))
		cerr << "STATUSWINDOW: ERROR > Couldn't draw weapon text!" << endl;

	if (_active)
	{
		VideoManager->Move(_cursor_x, _cursor_y);
		VideoManager->DrawImage(*(VideoManager->GetDefaultCursor()));
	}

	return true;
}

//-----------------------------------
// StatusWindow::Activate
//-----------------------------------
void StatusWindow::Activate(bool new_value)
{
	_active = new_value;
}

//-----------------------------------
// StatusWindow::Activate
//-----------------------------------
void StatusWindow::Update()
{
	// Load sound effects
	SoundDescriptor confirm;
	SoundDescriptor cancel;
	if (confirm.LoadSound("confirm") == false) 
	{
		cerr << "STATUSWINDOW::UPDATE - Unable to load confirm sound effect!" << endl;
	}
	if (cancel.LoadSound("cancel") == false) 
	{
		cerr << "STATUSWINDOW::UPDATE - Unable to load cancel sound effect!" << endl;
	}

	// check input values
	if (InputManager->UpPress())
	{
	}
	else if (InputManager->DownPress())
	{
	}
	else if (InputManager->ConfirmPress())
	{
	}
}

} // end namespace hoa_menu
