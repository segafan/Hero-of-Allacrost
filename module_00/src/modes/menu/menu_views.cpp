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
 * \brief   Source file for various menu views.
 *****************************************************************************/

#include <iostream>
#include <sstream>

#include "utils.h"

#include "audio.h"
#include "video.h"
#include "global.h"
#include "input.h"

#include "menu.h"
#include "menu_views.h"

using namespace std;
using namespace hoa_menu::private_menu;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_global;
using namespace hoa_input;

namespace hoa_menu {

namespace private_menu {

////////////////////////////////////////////////////////////////////////////////
// CharacterWindow Class
////////////////////////////////////////////////////////////////////////////////

CharacterWindow::CharacterWindow()
{
	_char_id = GLOBAL_NO_CHARACTERS;
}



CharacterWindow::~CharacterWindow()
{
	// Delete the character's portrait
	VideoManager->DeleteImage(this->_portrait);
}



void CharacterWindow::SetCharacter(GlobalCharacter *character)
{
	this->_char_id = character->GetID();
	
	// TODO: Load the portrait
	this->_portrait.SetFilename("img/portraits/menu/" + character->GetFilename() + ".png");
	this->_portrait.SetStatic(true);
	this->_portrait.SetDimensions(150, 200);
	VideoManager->LoadImage(this->_portrait);
} // void CharacterWindow::SetCharacter(GlobalCharacter *character)



// Draw the window to the screen
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
	if (!VideoManager->DrawText(MakeUnicodeString(xp_level)))
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
	if (!VideoManager->DrawText(MakeUnicodeString(health)))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw health!" << endl;
		
	// Draw skill
	VideoManager->MoveRelative(0, 40);
	
	// convert to std::string
	ostringstream os_skill;
	os_skill << character->GetSP() << " / " << character->GetMaxSP();
	std::string skill = std::string("Skill: ") + os_skill.str();
	if (!VideoManager->DrawText(MakeUnicodeString(skill)))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw skill!" << endl;
	
	// Draw xp
	VideoManager->MoveRelative(0, 40);
	
	// Convert to std::string
	ostringstream os_xp;
	os_xp << character->GetXPForNextLevel();
	std::string xp = std::string("XP Remaining: ") + os_xp.str();
	if (!VideoManager->DrawText(MakeUnicodeString(xp)))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw xp!" << endl;
	
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// MiniCharacterSelectWindow Class
////////////////////////////////////////////////////////////////////////////////








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
		if (!VideoManager->DrawText(MakeUnicodeString(health)))
			cerr << "MINICHARACTERWINDOW: ERROR > Couldn't draw health!" << endl;

		// Draw skill points
		VideoManager->MoveRelative(0, 30);
		ostringstream os_skill;
		os_skill << current->GetSP() << " / " << current->GetMaxSP();
		std::string skill = std::string("Skill: ") + os_skill.str();
		if (!VideoManager->DrawText(MakeUnicodeString(skill)))
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
	if (confirm.LoadSound("snd/obtain.wav") == false) 
	{
		cerr << "MINICHARWINDOW::UPDATE - Unable to load confirm sound effect!" << endl;
	}
	if (bump.LoadSound("snd/bump.wav") == false) 
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
		if (selected->GetCount() != 0) {
			selected->DecCount(1);
		}
		else {
			// GlobalManager->RemoveFromInventory(selected);
		}
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

////////////////////////////////////////////////////////////////////////////////
// InventoryWindow Class
////////////////////////////////////////////////////////////////////////////////

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



InventoryWindow::~InventoryWindow()
{
}



void InventoryWindow::Activate(bool new_status)
{
	// Set new status
	_inventory_active = new_status; 
	// Update cursor state
	if (_inventory_active)
		_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	else
		_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}



bool InventoryWindow::CanCancel()
{
	return !_char_window.IsActive();
}


void InventoryWindow::Update()
{
	// Load sound effects
	SoundDescriptor confirm;
	SoundDescriptor cancel;
	if (confirm.LoadSound("snd/confirm.wav") == false) 
	{
		cerr << "MENUMODE::UPDATE - Unable to load confirm sound effect!" << endl;
	}
	if (cancel.LoadSound("snd/cancel.wav") == false) 
	{
		cerr << "MENUMODE::UPDATE - Unable to load cancel sound effect!" << endl;
	}

	// When the character select window is active, no processing is needed for the inventory window.
	if (_char_window.IsActive())
	{
		// User requested to exit the character select window
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
		else {
			_char_window.Update();
		}
		return;
	} // if (_char_window.IsActive())

	// Handle the appropriate input events
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

	// Activate the character select window upon a confirm event on the option box
	if (_inventory_items.GetEvent() == VIDEO_OPTION_CONFIRM)
	{
		int item_selected = _inventory_items.GetSelection();

		GlobalItem *item = (GlobalItem*)(GlobalManager->GetInventory()[item_selected]);
		if ((item->GetUseCase() & GLOBAL_HP_RECOVERY_ITEM) || (item->GetUseCase() & GLOBAL_SP_RECOVERY_ITEM))
		{
			confirm.PlaySound();
			// Activate the character select window
			_char_window.Show();
			_char_window.Activate(true);
			_char_window.SetSelectedIndex(item_selected);
		}
	} // if (event == VIDEO_OPTION_CONFIRM)
} // void InventoryWindow::Update()



void InventoryWindow::UpdateItemText()
{
	// Get the inventory items
	vector<GlobalObject*> inv = GlobalManager->GetInventory();
				
	// Set the size of the option box
	// Calculate the number of rows, this is dividing by row_width, and if there is a remainder > 0
	// add one more row for the remainder.
	uint32 row_width = 4;
	_inventory_items.SetSize(row_width, inv.size() / row_width + ((inv.size() % row_width) > 0 ? 1 : 0));

	// Construct the option names using the item icon image, the item name, and the item count
	vector<ustring> inv_names;
	for (uint32 i = 0; i < inv.size(); ++i) {
		inv_names.push_back(MakeUnicodeString("<" + inv[i]->GetIconPath() + "><32>"
			+ inv[i]->GetName() + "<R>" + NumberToString(inv[i]->GetCount()) + "   "));
	}
	
	_inventory_items.SetOptions(inv_names);

	// Make sure we have at least one item before setting selection
	if (inv.size() > 0) {
		_inventory_items.SetSelection(0);
	}
} // void InventoryWindow::UpdateItemText()



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
} // bool InventoryWindow::Draw()

////////////////////////////////////////////////////////////////////////////////
// StatusWindow Class
////////////////////////////////////////////////////////////////////////////////

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
	_full_portrait.SetDimensions(150, 350);
	// Load image
	VideoManager->LoadImage(_head_portrait);
	VideoManager->LoadImage(_full_portrait);

	StillImage item;
	item.SetDimensions(60, 60);
	item.SetFilename("img/icons/weapons/karlate_sword.png");
	images.push_back(item);
	item.SetFilename("img/icons/armor/karlate_helmet.png");
	images.push_back(item);
	item.SetFilename("img/icons/armor/karlate_breastplate.png");
	images.push_back(item);
	item.SetFilename("img/icons/armor/karlate_shield.png");
	images.push_back(item);
	item.SetFilename("img/icons/armor/karlate_greaves.png");
	images.push_back(item);
	
	if (!VideoManager->LoadImage(images[0]))
		exit(1);
	if (!VideoManager->LoadImage(images[1]))
		exit(1);
	if (!VideoManager->LoadImage(images[2]))
		exit(1);
	if (!VideoManager->LoadImage(images[3]))
		exit(1);
	if (!VideoManager->LoadImage(images[4]))
		exit(1);
}



StatusWindow::~StatusWindow()
{
	VideoManager->DeleteImage(_head_portrait);
	VideoManager->DeleteImage(_full_portrait);
}



void StatusWindow::Update()
{
	// Load sound effects
	SoundDescriptor confirm;
	SoundDescriptor cancel;
	if (confirm.LoadSound("snd/confirm.wav") == false) 
	{
		cerr << "STATUSWINDOW::UPDATE - Unable to load confirm sound effect!" << endl;
	}
	if (cancel.LoadSound("snd/cancel.wav") == false) 
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
} // void StatusWindow::Update()



bool StatusWindow::Draw()
{
	if (!MenuWindow::Draw())
		return false;

	// Set drawing system
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);

	// window top corner is 72, 84
	VideoManager->Move(95, 150);
	VideoManager->DrawImage(_head_portrait);

	VideoManager->SetDrawFlags(VIDEO_X_CENTER, 0);
	VideoManager->MoveRelative(100, 220);
	VideoManager->DrawText(_current_char->GetName());

	VideoManager->MoveRelative(0, 25);
	VideoManager->DrawText(MakeUnicodeString("Experience Level: 4"));

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, 0);
	
	VideoManager->MoveRelative(120, -240);
	VideoManager->DrawText(MakeUnicodeString("HP: 246 (330)"));
	VideoManager->MoveRelative(0, 25);
	VideoManager->DrawText(MakeUnicodeString("SP: 189 (200)"));
	VideoManager->MoveRelative(0, 25);
	VideoManager->DrawText(MakeUnicodeString("XP to Level: 273"));
	VideoManager->MoveRelative(0, 25);
	VideoManager->DrawText(MakeUnicodeString("Strength: 105"));
	VideoManager->MoveRelative(0, 25);
	VideoManager->DrawText(MakeUnicodeString("Vigor: 72"));
	VideoManager->MoveRelative(0, 25);
	VideoManager->DrawText(MakeUnicodeString("Fortitude: 106"));
	VideoManager->MoveRelative(0, 25);
	VideoManager->DrawText(MakeUnicodeString("Resistance: 48"));
	VideoManager->MoveRelative(0, 25);
	VideoManager->DrawText(MakeUnicodeString("Agility: 26"));
	VideoManager->MoveRelative(0, 25);
	VideoManager->DrawText(MakeUnicodeString("Evade: 3%"));

	VideoManager->Move(500, 105);
	VideoManager->DrawImage(_full_portrait);

	VideoManager->MoveRelative(175, 5);
	VideoManager->DrawImage(images[0]);
	VideoManager->MoveRelative(0, 70);
	VideoManager->DrawImage(images[1]);
	VideoManager->MoveRelative(0, 70);
	VideoManager->DrawImage(images[2]);
	VideoManager->MoveRelative(0, 70);
	VideoManager->DrawImage(images[3]);
	VideoManager->MoveRelative(0, 70);
	VideoManager->DrawImage(images[4]);

	VideoManager->MoveRelative(75, -280 + 25);
	VideoManager->DrawText(MakeUnicodeString("Weapon"));
	VideoManager->MoveRelative(0, 70);
	VideoManager->DrawText(MakeUnicodeString("Head Armor"));
	VideoManager->MoveRelative(0, 70);
	VideoManager->DrawText(MakeUnicodeString("Torso Armor"));
	VideoManager->MoveRelative(0, 70);
	VideoManager->DrawText(MakeUnicodeString("Arm Armor"));
	VideoManager->MoveRelative(0, 70);
	VideoManager->DrawText(MakeUnicodeString("Leg Armor"));


	
// 	// Draw Health
// 	ostringstream os_health;
// 	os_health << _current_char->GetHP() << " / " << _current_char->GetMaxHP();
// 	string health = string("Health: ") + os_health.str();
// 	if (!VideoManager->DrawText(MakeUnicodeString(health)))
// 		cerr << "STATUSWINDOW: ERROR > Couldn't draw health!" << endl;
// 		
// 	// Draw skill
// 	VideoManager->MoveRelative(0, 24);
// 	
// 	// convert to std::string
// 	ostringstream os_skill;
// 	os_skill << _current_char->GetSP() << " / " << _current_char->GetMaxSP();
// 	string skill = string("Skill: ") + os_skill.str();
// 	if (!VideoManager->DrawText(MakeUnicodeString(skill)))
// 		cerr << "STATUSWINDOW: ERROR > Couldn't draw skill!" << endl;
// 	
// 	// Draw xp
// 	VideoManager->MoveRelative(0, 24);
	
// 	// Convert to std::string
// 	ostringstream os_xp;
// 	os_xp << _current_char->GetXPForNextLevel();
// 	string xp = string("XP Remaining: ") + os_xp.str();
// 	if (!VideoManager->DrawText(MakeUnicodeString(xp)))
// 		cerr << "STATUSWINDOW: ERROR > Couldn't draw xp!" << endl;
// 
// 	// Draw stremgth
// 	VideoManager->MoveRelative(160, -48);
// 	ostringstream os_str;
// 	os_str << _current_char->GetStrength();
// 	string str = string("Strength: ") + os_str.str();
// 	if (!VideoManager->DrawText(MakeUnicodeString(str)))
// 		cerr << "STATUSWINDOW: ERROR > Couldn't draw character strength!" << endl;
// 
// 	// Draw Intelligence
// 	VideoManager->MoveRelative(0, 24);
// 	ostringstream os_int;
// 	os_int << _current_char->GetIntelligence();
// 	string intelligence = string("Intelligence: ") + os_int.str();
// 	if (!VideoManager->DrawText(MakeUnicodeString(intelligence)))
// 		cerr << "STATUSWINDOW: ERROR > Couldn't draw character intelligence!" << endl;
// 
// 	// Draw Agility
// 	VideoManager->MoveRelative(0, 24);
// 	ostringstream os_agi;
// 	os_agi << _current_char->GetAgility();
// 	string agi = string("Agility: ") + os_agi.str();
// 	if (!VideoManager->DrawText(MakeUnicodeString(agi)))
// 		cerr << "STATUSWINDOW: ERROR > Couldn't draw character agility!" << endl;



	// Draw Equipment
// 	VideoManager->MoveRelative(220, 220);
// 
// 	VideoManager->MoveRelative(0, 24);
// 	VideoManager->DrawText(MakeUnicodeString("Weapon"));
// 
// 	VideoManager->MoveRelative(0, 24);
// 	VideoManager->DrawText(MakeUnicodeString("Head Armor"));
// 
// 	VideoManager->MoveRelative(0, 24);
// 	VideoManager->DrawText(MakeUnicodeString("Torso Armor"));
// 
// 	VideoManager->MoveRelative(0, 24);
// 	VideoManager->DrawText(MakeUnicodeString("Arm Armor"));
// 
// 	VideoManager->MoveRelative(0, 24);
// 	VideoManager->DrawText(MakeUnicodeString("Leg Armor"));

	if (_active) {
		VideoManager->Move(_cursor_x, _cursor_y);
		VideoManager->DrawImage(*(VideoManager->GetDefaultCursor()));
	}

	return true;
} // bool StatusWindow::Draw()

} // namespace private_menu

} // namespace hoa_menu
