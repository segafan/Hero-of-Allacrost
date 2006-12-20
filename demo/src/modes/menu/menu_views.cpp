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
 * \author  Daniel Steuernol steu@allacrost.org, Andy Gardner chopperdave@allacrost.org
 * \brief   Source file for various menu views.
 *****************************************************************************/

#include <iostream>
#include <sstream>

#include "utils.h"

#include "audio.h"
#include "video.h"
#include "global.h"
#include "input.h"
#include "system.h"

#include "menu.h"
#include "menu_views.h"

using namespace std;
using namespace hoa_menu::private_menu;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_global;
using namespace hoa_input;
using namespace hoa_system;

namespace hoa_menu {

namespace private_menu {


////////////////////////////////////////////////////////////////////////////////
// CharacterWindow Class
////////////////////////////////////////////////////////////////////////////////

CharacterWindow::CharacterWindow()
{
	_char_id = GLOBAL_CHARACTER_NONE;
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
	this->_portrait.SetFilename("img/portraits/map/" + character->GetFilename() + "_small.png");
	this->_portrait.SetStatic(true);
	this->_portrait.SetDimensions(100, 100);
	VideoManager->LoadImage(this->_portrait);
} // void CharacterWindow::SetCharacter(GlobalCharacter *character)



// Draw the window to the screen
bool CharacterWindow::Draw()
{
	// Call parent Draw method, if failed pass on fail result
	if (MenuWindow::Draw() == false)
		return false;
	// Window is hidden return true
	/*if (MenuWindow::GetState() == hoa_video::VIDEO_MENU_STATE_HIDDEN)
		return true;*/
	
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
	
	// Get the window metrics
	float x, y, w, h;
	this->GetPosition(x,y);
	this->GetDimensions(w,h);

	// check to see if this window is an actual character
	if (_char_id == hoa_global::GLOBAL_CHARACTER_NONE)
		// no more to do here
		return true;
	
	GlobalCharacter *character = GlobalManager->GetCharacter(_char_id);
	
	VideoManager->Move(x + 12, y + 8);
	VideoManager->DrawImage(_portrait);

	VideoManager->MoveRelative(150, 0);
	if (!VideoManager->DrawText(character->GetName()))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw Character Name!" << endl;

	VideoManager->MoveRelative(0,20);
	std::ostringstream os_level;
	os_level << character->GetExperienceLevel();
	std::string xp_level = std::string("Lv: ") + os_level.str();
	if (!VideoManager->DrawText(MakeUnicodeString(xp_level)))
		cerr << "CHARACTERWINDOW: ERROR: > Couldn't draw xp level" << endl;

	VideoManager->MoveRelative(0,20);
	ostringstream os_health;
	os_health << character->GetHitPoints() << " / " << character->GetMaxHitPoints();
	std::string health = std::string("HP: ") + os_health.str();
	if (!VideoManager->DrawText(MakeUnicodeString(health)))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw health!" << endl;

	VideoManager->MoveRelative(0,20);
	ostringstream os_skill;
	os_skill << character->GetSkillPoints() << " / " << character->GetMaxSkillPoints();
	std::string skill = std::string("SP: ") + os_skill.str();
	if (!VideoManager->DrawText(MakeUnicodeString(skill)))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw skill!" << endl;

	VideoManager->MoveRelative(0, 20);
	ostringstream os_xp;
	os_xp << character->GetExperienceForNextLevel();
	std::string xp = std::string("XP To Next: ") + os_xp.str();
	if (!VideoManager->DrawText(MakeUnicodeString(xp)))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw xp!" << endl;
/*
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
	*/
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
}

//--------------------------------------------------------
// MiniCharacterSelectwindow::~MiniCharacterSelectWindow()
//--------------------------------------------------------
MiniCharacterSelectWindow::~MiniCharacterSelectWindow()
{
	// Clear sounds
	_menu_sounds["confirm"].FreeSound();
	_menu_sounds["bump"].FreeSound();
	_menu_sounds["potion"].FreeSound();
	_menu_sounds["cancel"].FreeSound();
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
	for (uint32 i = 0; i < GlobalManager->GetActiveParty()->GetPartySize(); ++i)
	{
		VideoManager->Move(765, 180);

		GlobalCharacter *current = GlobalManager->GetActiveParty()->GetCharacters()[i];
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
		os_health << current->GetHitPoints() << " / " << current->GetMaxHitPoints();
		std::string health = std::string("Health: ") + os_health.str();
		if (!VideoManager->DrawText(MakeUnicodeString(health)))
			cerr << "MINICHARACTERWINDOW: ERROR > Couldn't draw health!" << endl;

		// Draw skill points
		VideoManager->MoveRelative(0, 30);
		ostringstream os_skill;
		os_skill << current->GetSkillPoints() << " / " << current->GetMaxSkillPoints();
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
	// Check input values
	if (InputManager->ConfirmPress())
	{
		// Use the passed in item, to update values
		GlobalItem *selected = static_cast<GlobalItem *>(GlobalManager->GetInventory()[_selected_item_index]);
		GlobalCharacter *ch = GlobalManager->GetActiveParty()->GetCharacters()[_current_char_selected];

		if (selected->GetCount() == 0)
		{
			// no more items to use
			_menu_sounds["bump"].PlaySound();
			return;
		}

		// check character hp
		if (ch->GetHitPoints() == ch->GetMaxHitPoints())
		{
			// don't use item we're full
			_menu_sounds["bump"].PlaySound();
			return;
		}

		// Play Sound
		_menu_sounds["potion"].PlaySound();

		// increase hp FIXME
		if (selected->GetUsage() == GLOBAL_MENU_USAGE)
		{	
			uint32 new_hp = ch->GetHitPoints();
			//new_hp += selected->GetRecoveryAmount();
			if (new_hp > ch->GetMaxHitPoints())
				new_hp = ch->GetMaxHitPoints();
			ch->SetHitPoints(new_hp);
		}

		// increase sp
		/*if ((selected->GetUsage() & GLOBAL_SP_RECOVERY_ITEM) == GLOBAL_SP_RECOVERY_ITEM)
		{
			uint32 new_sp = ch->GetSP();
			new_sp += selected->GetRecoveryAmount();
			if (new_sp > ch->GetMaxSP())
				new_sp = ch->GetMaxSP();
			ch->SetSP(new_sp);
		}*/

		// decrease item count
		if (selected->GetCount() > 1) {
			selected->DecrementCount(1);
		}
		else {
			GlobalManager->RemoveFromInventory(selected->GetID());
			this->Activate(false);
			this->Hide();
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
			_current_char_selected = GlobalManager->GetActiveParty()->GetPartySize() - 1;
	}
	else if (InputManager->DownPress())
	{
		if (_current_char_selected < GlobalManager->GetActiveParty()->GetPartySize() - 1)
			_current_char_selected++;
		else
			_current_char_selected = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
// InventoryWindow Class
////////////////////////////////////////////////////////////////////////////////


InventoryWindow::InventoryWindow() : /*_inventory_active(false), _char_select_active(false), 
									_item_categories_active(false)*/_active_box(ITEM_ACTIVE_NONE)
{
	InitInventoryItems();
	InitCharSelect();
	InitCategory();

	//_item_categories_active = false;
	//_inventory_active = false;
	//_char_select_active = false;
	
	// Load sounds
	SoundDescriptor confirm;
	SoundDescriptor bump;
	SoundDescriptor potion;
	SoundDescriptor cancel;
	if (confirm.LoadSound("snd/obtain.wav") == false) 
	{
		cerr << "INVENTORYWINDOW::UPDATE - Unable to load confirm sound effect!" << endl;
	}
	if (bump.LoadSound("snd/bump.wav") == false) 
	{
		cerr << "INVENTORYWINDOW::UPDATE - Unable to load bump sound effect!" << endl;
	}
	if (potion.LoadSound("snd/potion_drink.wav") == false)
	{
		cerr << "INVENTORYWINDOW::UPDATE - Unable to load potion drink sound effect!" << endl;
	}
	if (cancel.LoadSound("snd/cancel.wav") == false)
	{
		cerr << "INVENTORYWINDOW::UPDATE - Unable to load cancel sound effect!" << endl;
	}
	_menu_sounds["confirm"] = confirm;
	_menu_sounds["bump"] = bump;
	_menu_sounds["potion"] = potion;
	_menu_sounds["cancel"] = cancel;

	//Load char portraits for bottom menu
	StillImage i;

	//FIX ME: Make dynamic based on char names
	i.SetFilename("img/portraits/battle/claudius.png");
	i.SetDimensions(100.0f, 100.0f);
	_portraits.push_back(i);
	VideoManager->LoadImage(_portraits[0]);

	_location_picture.SetFilename("img/menus/locations/desert_cave.png");
	_location_picture.SetDimensions(500.0f, 125.0f);
	VideoManager->LoadImage(_location_picture);
	
}// void InventoryWindow::InventoryWindow

InventoryWindow::~InventoryWindow()
{
	// Clear sounds
	_menu_sounds["confirm"].FreeSound();
	_menu_sounds["bump"].FreeSound();
	_menu_sounds["potion"].FreeSound();
	_menu_sounds["cancel"].FreeSound();

	VideoManager->DeleteImage(_portraits[0]);
	VideoManager->DeleteImage(_location_picture);
}

void InventoryWindow::InitInventoryItems() {
	// Set up the inventory option box
	_inventory_items.SetCellSize(180.0f, 30.0f);
	// This is dependant on the number of inventory items?
	// or maybe have blank items.
	//_inventory_items.SetSize(2, 10);
	_inventory_items.SetPosition(500.0f, 170.0f);
	_inventory_items.SetFont("default");
	//_inventory_items.SetSelectMode(VIDEO_SELECT_SINGLE);
	_inventory_items.SetCursorOffset(-52.0f, -20.0f);
	_inventory_items.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_inventory_items.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_inventory_items.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	// Update the item text
	UpdateItemText();
	_inventory_items.SetSelection(0);
	// Initially hide the cursor
	_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}

void InventoryWindow::InitCharSelect() {
	//character selection set up
	//float w, h;
	vector<ustring> options;
	uint32 size = GlobalManager->GetActiveParty()->GetPartySize();
	//_character_window0.GetDimensions(w, h);
	
	_char_select.SetCursorOffset(-50.0f, -6.0f);
	_char_select.SetFont("default");
	_char_select.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_char_select.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_char_select.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_char_select.SetSize(1, ((size >= 4) ? 4 : size));
	_char_select.SetSize(1, 4);
	_char_select.SetCellSize(360, 108);
	_char_select.SetPosition(72.0f, 109.0f);

	for (uint32 i = 0; i < size; i++) {
		options.push_back(MakeUnicodeString(" "));
	}

	_char_select.SetOptions(options);
	_char_select.SetSelection(0);
	_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	//character selection set up
}

void InventoryWindow::InitCategory() {
	_item_categories.SetCellSize(85.0f,30.0f);
	_item_categories.SetPosition(458.0f, 120.0f);
	_item_categories.SetFont("default");
	_item_categories.SetSize(ITEM_CATEGORY_SIZE,1);
	
	_item_categories.SetCursorOffset(-52.0f, -20.0f);
	_item_categories.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_item_categories.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_item_categories.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);

	vector<ustring> options;
	options.push_back(MakeUnicodeString("All"));
	options.push_back(MakeUnicodeString("Field"));
	options.push_back(MakeUnicodeString("Battle"));
	options.push_back(MakeUnicodeString("Gear"));
	options.push_back(MakeUnicodeString("Key"));
		

	_item_categories.SetOptions(options);
	_item_categories.SetSelection(ITEM_ALL);
	_item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

	/*VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);

	VideoManager->Move(465, 120);
	VideoManager->DrawText(MakeUnicodeString("All"));

	VideoManager->MoveRelative(70, 0);
	VideoManager->DrawText(MakeUnicodeString("Field"));

	VideoManager->MoveRelative(75, 0);
	VideoManager->DrawText(MakeUnicodeString("Battle"));

	VideoManager->MoveRelative(100, 0);
	VideoManager->DrawText(MakeUnicodeString("Equipment"));

	VideoManager->MoveRelative(125, 0);
	VideoManager->DrawText(MakeUnicodeString("Key"));*/
}


void InventoryWindow::Activate(bool new_status)
{
	// Set new status
	if (_inventory_items.GetNumOptions() > 0 && new_status) {
		_active_box = ITEM_ACTIVE_CATEGORY;
		// Update cursor state
		//if (_item_categories_active)
			_item_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		/*else
			_item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);*/
	}
	else {
		//FIX ME: Play N/A noise
		_active_box = ITEM_ACTIVE_NONE;
		_item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	}
}



/*bool InventoryWindow::CanCancel()
{
	return !_char_window.IsActive();
}*/


void InventoryWindow::Update() {

	//bool cancel = false;
	if (_inventory_items.GetNumOptions() == 0)
	{
		// no more items in inventory, exit inventory window
		Activate(false);
		return;
	}

	OptionBox *active_option = NULL;

	// When the character select window is active, no processing is needed for the inventory window.
	/*if (_char_window.IsActive())
	{
		// User requested to exit the character select window
		if (InputManager->CancelPress())
		{
			// deactivate it
			if (_char_window.IsActive())
			{
				_menu_sounds["cancel"].PlaySound();
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
	}*/ // if (_char_window.IsActive())

	switch (_active_box) {
		case ITEM_ACTIVE_CATEGORY:
			active_option = &_item_categories;
			break;
		case ITEM_ACTIVE_CHAR:
			active_option = &_char_select;
			break;
		case ITEM_ACTIVE_LIST:
			active_option = &_inventory_items;
			break;
	}

	/*if (_item_categories_active) {
		active_option = &_item_categories;
	}
	else if (_char_select_active) {
		active_option = &_char_select;
	}
	else if (_inventory_active) {
		active_option = &_inventory_items;
	}*/
	
	// Handle the appropriate input events
	if (InputManager->ConfirmPress())
	{
		active_option->HandleConfirmKey();
		
	}
	else if (InputManager->CancelPress())
	{
		active_option->HandleCancelKey();
	}
	else if (InputManager->LeftPress())
	{
		active_option->HandleLeftKey();
	}
	else if (InputManager->RightPress())
	{
		active_option->HandleRightKey();
	}
	else if (InputManager->UpPress())
	{
		active_option->HandleUpKey();
	}
	else if (InputManager->DownPress())
	{
		active_option->HandleDownKey();
	}

	uint32 event = active_option->GetEvent();

	//if (_char_select_active) {
	switch (_active_box) {
		case ITEM_ACTIVE_NONE: break;

		case ITEM_ACTIVE_CATEGORY:
			{
				if (event == VIDEO_OPTION_CONFIRM) {
					//_item_categories_active = false;
					_item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					//_inventory_active = true;
					_active_box = ITEM_ACTIVE_LIST;
					_menu_sounds["confirm"].PlaySound();
				}
				else if (event == VIDEO_OPTION_CANCEL) {
					_menu_sounds["cancel"].PlaySound();
					_item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					Activate(false);
				}
			}
			break;

		case ITEM_ACTIVE_LIST:
			{
				if (event == VIDEO_OPTION_CONFIRM) {
					//_char_select_active = true;
					//_inventory_active = false;
					_active_box = ITEM_ACTIVE_CHAR;
					_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
					_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					_menu_sounds["confirm"].PlaySound();
				}
				else if (event == VIDEO_OPTION_CANCEL) {
					//_inventory_active = false;
					_active_box = ITEM_ACTIVE_CATEGORY;
					_menu_sounds["cancel"].PlaySound();
					_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_item_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					//_item_categories_active = true;
				}
			}
			break;

		case ITEM_ACTIVE_CHAR:
			{
				if (event == VIDEO_OPTION_CONFIRM) {
					//TODO Use Item
					/*GlobalItem *item = (GlobalItem*)(GlobalManager->GetInventory()[item_selected]);
					if ((item->GetUseCase() & GLOBAL_HP_RECOVERY_ITEM) || (item->GetUseCase() & GLOBAL_SP_RECOVERY_ITEM))
					{*/
						ApplyItem();
					//}
					//_menu_sounds["confirm"].PlaySound();
				}
				else if (event == VIDEO_OPTION_CANCEL) {
					//_char_select_active = false;
					//_inventory_active = true;
					_active_box = ITEM_ACTIVE_LIST;
					_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_menu_sounds["cancel"].PlaySound();
					//cancel = false;
				}
			}
			break;
	}

		/*if (event == VIDEO_OPTION_CONFIRM) {
			//TODO Use Item
			GlobalItem *item = (GlobalItem*)(GlobalManager->GetInventory()[item_selected]);
			if ((item->GetUseCase() & GLOBAL_HP_RECOVERY_ITEM) || (item->GetUseCase() & GLOBAL_SP_RECOVERY_ITEM))
			{
				ApplyItem();
			//}
			//_menu_sounds["confirm"].PlaySound();
		}
		else if (event == VIDEO_OPTION_CANCEL) {
			//_char_select_active = false;
			//_inventory_active = true;
			_active_box = ITEM_LIST_ACTIVE;
			_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_menu_sounds["cancel"].PlaySound();
			//cancel = false;
		}
	}
	// Activate the character select window upon a confirm event on the option box
	else if (_inventory_active)
	{
		
		if (event == VIDEO_OPTION_CONFIRM) {
			//int item_selected = _inventory_items.GetSelection();
			_char_select_active = true;
			_inventory_active = false;
			_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
			_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_menu_sounds["confirm"].PlaySound();
			//GlobalItem *item = (GlobalItem*)(GlobalManager->GetInventory()[item_selected]);
			//if ((item->GetUseCase() & GLOBAL_HP_RECOVERY_ITEM) || (item->GetUseCase() & GLOBAL_SP_RECOVERY_ITEM))
			//{
			
				// Activate the character select window
				//_char_window.Show();
				//_char_window.Activate(true);
				//_char_window.SetSelectedIndex(item_selected);
		}
		else if (event == VIDEO_OPTION_CANCEL) {
			_inventory_active = false;
			_menu_sounds["cancel"].PlaySound();
			//Activate(false);
			//cancel = false;
			_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_item_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_item_categories_active = true;
		}
	}
	else if (_item_categories_active) {
		if (event == VIDEO_OPTION_CONFIRM) {
			_item_categories_active = false;
			_item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_inventory_active = true;
			_menu_sounds["confirm"].PlaySound();
		}
		else if (event == VIDEO_OPTION_CANCEL) {
			_menu_sounds["cancel"].PlaySound();
			_item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			Activate(false);
		}
	}*/
	UpdateItemText();

} // void InventoryWindow::Update()

void InventoryWindow::ApplyItem() {
// Use the passed in item, to update values
	GlobalItem *selected = static_cast<GlobalItem *>(GlobalManager->GetInventory()[_inventory_items.GetSelection()]);
	GlobalCharacter *ch = GlobalManager->GetActiveParty()->GetCharacters()[_char_select.GetSelection()];

	if (selected->GetCount() == 0)
	{
		// no more items to use
		_menu_sounds["bump"].PlaySound();
		return;
	}

	// check character hp
	if (ch->GetHitPoints() == ch->GetMaxHitPoints())
	{
		// don't use item we're full
		_menu_sounds["bump"].PlaySound();
		return;
	}

	// Play Sound
	_menu_sounds["potion"].PlaySound();

	// increase hp
	if (selected->GetUsage() == GLOBAL_MENU_USAGE)
	{	
		uint32 new_hp = ch->GetHitPoints();
		new_hp += 180;//selected->->GetRecoveryAmount();
		if (new_hp > ch->GetMaxHitPoints())
			new_hp = ch->GetMaxHitPoints();
		ch->SetHitPoints(new_hp);
	}

	// increase sp
	/*if ((selected->GetUseCase() & GLOBAL_SP_RECOVERY_ITEM) == GLOBAL_SP_RECOVERY_ITEM)
	{
		uint32 new_sp = ch->GetSP();
		new_sp += selected->GetRecoveryAmount();
		if (new_sp > ch->GetMaxSP())
			new_sp = ch->GetMaxSP();
		ch->SetSP(new_sp);
	}*/

	// decrease item count
	if (selected->GetCount() > 1) {
		selected->DecrementCount(1);
	}
	else {
		//GlobalManager->RemoveItemFromInventory(HP_POTION);
		GlobalManager->RemoveFromInventory(selected->GetID());
		//_char_select_active = false;
		if (GlobalManager->GetInventory().size() > 0) {
			//_inventory_active = true;
			_active_box = ITEM_ACTIVE_LIST;
			_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		}
		else {
			//_char_select_active = false;
			//_inventory_active = false;
			//_item_categories_active = false;
			_active_box = ITEM_ACTIVE_NONE;
			_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		}
		//this->Activate(false);
		//this->Hide();
	}
}


void InventoryWindow::UpdateItemText()
{
	//FIX ME:  use _item_categories.GetSelection() to filter item list

	// Get the inventory items
	
	map<uint32, GlobalObject*> inv = GlobalManager->GetInventory();
				
	// Set the size of the option box
	// Calculate the number of rows, this is dividing by row_width, and if there is a remainder > 0
	// add one more row for the remainder.
	//uint32 row_width = 4;
	//_inventory_items.SetSize(row_width, inv.size() / row_width + ((inv.size() % row_width) > 0 ? 1 : 0));

	_inventory_items.SetSize(1,inv.size());

	// Construct the option names using the item icon image, the item name, and the item count

	vector<ustring> inv_names;
	map<uint32,GlobalObject*>::iterator i;
	GlobalObject* item;
	string text;

	//FIX ME - When video engine is fixed, take out MakeStandardString
	for (i = inv.begin(); i != inv.end(); i++) {
		item = i->second;
		text ="<" + item->GetIconPath() + "><32>" + MakeStandardString(item->GetName()) +
			"<R><350>" + NumberToString(item->GetCount()) + "   ";
		inv_names.push_back(MakeUnicodeString(text));
	}
	/*for (uint32 i = 0; i < inv.size(); ++i) {
		inv_names.push_back(MakeUnicodeString("<" + inv[i]->GetIconPath() + "><32>" + 
			inv[i]->GetName() + "<R><350>" + NumberToString(inv[i]->GetCount()) + "   "));
	}*/
	
	_inventory_items.SetOptions(inv_names);

	// Make sure we have at least one item before setting selection
	//if (inv.size() > 0) {
		_inventory_items.SetSelection(0);
	//}

	//FIX ME: TEST CODE
	/*vector<ustring> inv_names;
	_inventory_items.SetSize(1, 12);
	for (uint32 j = 0; j < 12; j++) {
		inv_names.push_back(MakeUnicodeString("Health Potion                 1"));
	}*/

	//_inventory_items.SetOptions(inv_names);
} // void InventoryWindow::UpdateItemText()



bool InventoryWindow::Draw()
{
	if (MenuWindow::Draw() == false)
		return false;
	
	// Update the item text in case the number of items changed.
	

	// Draw the inventory text
	//if (_inventory_active)
	//	_inventory_items.Draw();
	//if (_char_select_active)
	_char_select.Draw();
	
	_item_categories.Draw();

	_inventory_items.Draw();

	DrawBottomMenu();
	
	return true;
	// Draw the char window
	///_char_window.Draw();
	

	//VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);

	/*VideoManager->Move(530, 120);
	VideoManager->DrawText(MakeUnicodeString("NAME"));

	VideoManager->MoveRelative(300, 0);
	VideoManager->DrawText(MakeUnicodeString("QTY"));*/

	/*VideoManager->Move(500, 170);
	VideoManager->DrawText(MakeUnicodeString("Health Potion"));
	VideoManager->MoveRelative(340, 0);
	VideoManager->DrawText(MakeUnicodeString("1"));

	VideoManager->MoveRelative(-340, 25);
	VideoManager->DrawText(MakeUnicodeString("Antidote"));
	VideoManager->MoveRelative(340, 0);
	VideoManager->DrawText(MakeUnicodeString("1"));

	VideoManager->MoveRelative(-340, 25);
	VideoManager->DrawText(MakeUnicodeString("Love Potion #9"));
	VideoManager->MoveRelative(340, 0);
	VideoManager->DrawText(MakeUnicodeString("1"));*/
	
} // bool InventoryWindow::Draw()

void InventoryWindow::DrawBottomMenu() {

	GlobalCharacter* _current_char;
	GlobalItem* _current_item;
	ostringstream agl, iteminfo;

	switch (_active_box) {
		case ITEM_ACTIVE_CHAR:
			_current_char = GlobalManager->GetActiveParty()->GetCharacters()[_char_select.GetSelection()];
			VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
			VideoManager->Move(115, 670);

			VideoManager->DrawImage(_portraits[0]);
			VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
			VideoManager->Move(277,559);

			//VideoManager->MoveRelative(0, 25);
			VideoManager->DrawText(MakeUnicodeString("Strength: 106"));

			VideoManager->MoveRelative(0, 20);
			VideoManager->DrawText(MakeUnicodeString("Vigor: 72"));

			VideoManager->MoveRelative(0, 20);
			VideoManager->DrawText(MakeUnicodeString("Fortitude: 106"));

			VideoManager->MoveRelative(0, 20);
			VideoManager->DrawText(MakeUnicodeString("Resistance: 48"));

			VideoManager->MoveRelative(0, 20);
			//ostringstream agl;
			//agl.flush();
			agl << "Agility: " << _current_char->GetAgility();
			VideoManager->DrawText(MakeUnicodeString(agl.str()));

			VideoManager->MoveRelative(0, 20);
			VideoManager->DrawText(MakeUnicodeString("Evade: 3%"));

		case ITEM_ACTIVE_LIST:
			_current_item = static_cast<GlobalItem*>(GlobalManager->GetInventory()[_inventory_items.GetSelection()]);
			VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
			VideoManager->Move(670,577);

			VideoManager->DrawText(_current_item->GetName());
			VideoManager->MoveRelative(0, 50);
			//stat.flush();
			iteminfo << "HP + 180";// << _current_item->GetRecoveryAmount();
			VideoManager->DrawText(MakeUnicodeString(iteminfo.str()));
			break;

		default:		
			VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
			VideoManager->Move(150, 577);

			//FIX ME: Make dynamic
			//Display Location
			
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
			os_money << GlobalManager->GetFunds();
			std::string money = std::string("Dorrun: ") + os_money.str();
			VideoManager->MoveRelative(0, 30);
			if (!VideoManager->DrawText(MakeUnicodeString(money)))
				cerr << "MENU: ERROR > Couldn't draw text!" << endl;

			VideoManager->Move(390, 685);
			VideoManager->DrawImage(_location_picture);

			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// StatusWindow Class
////////////////////////////////////////////////////////////////////////////////

StatusWindow::StatusWindow() : _char_select_active(false)//, _cursor_x(588.0f), _cursor_y(324.0f)
{
	// Get the current character
	//_current_char = GlobalManager->GetParty()[0];
	uint32 partysize = GlobalManager->GetActiveParty()->GetCharacters().size();
	StillImage portrait;
	// Set up the head picture
	//string path = string("img/sprites/map/") + string(_current_char->GetName()) + string("_d0.png");
	//string head_path = string("img/portraits/map/") + _current_char->GetFilename() + string(".png");
	//_head_portrait.SetFilename(head_path);
	//_head_portrait.SetStatic(true);
	//_head_portrait.SetDimensions(200, 200);
	// Set up the full body portrait
	for (uint32 i = 0; i < partysize; i++) {
		_current_char = GlobalManager->GetActiveParty()->GetCharacters()[i];
		string full_path = string("img/portraits/menu/") + _current_char->GetFilename() + string("_large.png");
		portrait.SetFilename(full_path);
		portrait.SetStatic(true);
		portrait.SetDimensions(150, 350);
		VideoManager->LoadImage(portrait);
		_full_portraits.push_back(portrait);
	}

	// Init the location picture
	_location_picture.SetFilename("img/menus/locations/desert_cave.png");
	_location_picture.SetDimensions(500, 125);
	_location_picture.SetStatic(true);
	VideoManager->LoadImage(_location_picture);

	//Init char select option box
	InitCharSelect();

	// Load sounds
	SoundDescriptor confirm;
	SoundDescriptor bump;
	SoundDescriptor potion;
	SoundDescriptor cancel;
	if (confirm.LoadSound("snd/obtain.wav") == false) 
	{
		cerr << "INVENTORYWINDOW::UPDATE - Unable to load confirm sound effect!" << endl;
	}
	if (bump.LoadSound("snd/bump.wav") == false) 
	{
		cerr << "INVENTORYWINDOW::UPDATE - Unable to load bump sound effect!" << endl;
	}
	if (potion.LoadSound("snd/potion_drink.wav") == false)
	{
		cerr << "INVENTORYWINDOW::UPDATE - Unable to load potion drink sound effect!" << endl;
	}
	if (cancel.LoadSound("snd/cancel.wav") == false)
	{
		cerr << "INVENTORYWINDOW::UPDATE - Unable to load cancel sound effect!" << endl;
	}
	_menu_sounds["confirm"] = confirm;
	_menu_sounds["bump"] = bump;
	_menu_sounds["potion"] = potion;
	_menu_sounds["cancel"] = cancel;
	// Load image
	//VideoManager->LoadImage(_head_portrait);
	

	/*StillImage item;
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
	
	for (uint32 i = 0; i < images.size(); i++) {
		if (!VideoManager->LoadImage(images[i]))
			exit(1);
	}*/
	/*if (!VideoManager->LoadImage(images[0]))
		exit(1);
	if (!VideoManager->LoadImage(images[1]))
		exit(1);
	if (!VideoManager->LoadImage(images[2]))
		exit(1);
	if (!VideoManager->LoadImage(images[3]))
		exit(1);
	if (!VideoManager->LoadImage(images[4]))
		exit(1);*/
}



StatusWindow::~StatusWindow()
{
	//VideoManager->DeleteImage(_head_portrait);
	uint32 partysize = GlobalManager->GetActiveParty()->GetCharacters().size();
	for (uint32 i = 0; i < partysize; i++) {
		VideoManager->DeleteImage(_full_portraits[i]);
	}

	VideoManager->DeleteImage(_location_picture);

	// Clear sounds
	_menu_sounds["confirm"].FreeSound();
	_menu_sounds["bump"].FreeSound();
	_menu_sounds["potion"].FreeSound();
	_menu_sounds["cancel"].FreeSound();
	/*for (uint32 i = 0; i < images.size(); i++) {
		VideoManager->DeleteImage(images[i]);
	}*/
}

void StatusWindow::Activate(bool new_value) {
	_char_select_active = new_value;

	if (_char_select_active)
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	else
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}

void StatusWindow::InitCharSelect() {
	//character selection set up
	//float w, h;
	vector<ustring> options;
	uint32 size = GlobalManager->GetActiveParty()->GetCharacters().size();
	//_character_window0.GetDimensions(w, h);
	
	_char_select.SetCursorOffset(-50.0f, -6.0f);
	_char_select.SetFont("default");
	_char_select.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_char_select.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_char_select.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_char_select.SetSize(1, ((size >= 4) ? 4 : size));
	//_char_select.SetSize(1, 4);
	_char_select.SetCellSize(360, 108);
	_char_select.SetPosition(72.0f, 109.0f);

	for (uint32 i = 0; i < size; i++) {
		options.push_back(MakeUnicodeString(" "));
	}

	_char_select.SetOptions(options);
	_char_select.SetSelection(0);
	_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	//character selection set up
}

void StatusWindow::Update()
{
	//FIX ME: Handle StatusWindow OptionBox
	
	// check input values
	if (InputManager->UpPress())
	{
		_char_select.HandleUpKey();
	}
	else if (InputManager->DownPress())
	{
		_char_select.HandleUpKey();
	}
	else if (InputManager->CancelPress())
	{
		_char_select.HandleCancelKey();
	}

	if (_char_select.GetEvent() == VIDEO_OPTION_CANCEL) {
		Activate(false);
		_menu_sounds["cancel"].PlaySound();
	}

} // void StatusWindow::Update()



bool StatusWindow::Draw()
{
	if (!MenuWindow::Draw())
		return false;

	// Set drawing system
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);

	// window top corner is 432, 99
	VideoManager->Move(565, 130);

	//Draw character name and level
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, 0);
	VideoManager->DrawText(_current_char->GetName());

	VideoManager->MoveRelative(0, 25);
	ostringstream lvl;
	lvl << "Experience Level: " << _current_char->GetExperienceLevel();
	VideoManager->DrawText(MakeUnicodeString(lvl.str()));

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, 0);
	
	//Draw all character stats
	VideoManager->MoveRelative(-55, 60);
	
	ostringstream ohp;
	ohp << "HP: " << _current_char->GetHitPoints() << " (" << _current_char->GetMaxHitPoints() << ")";
	VideoManager->DrawText(MakeUnicodeString(ohp.str()));

	VideoManager->MoveRelative(0, 25);
	ostringstream osp;
	osp << "SP: " << _current_char->GetSkillPoints() << " (" << _current_char->GetMaxSkillPoints() << ")";
	VideoManager->DrawText(MakeUnicodeString(osp.str()));

	VideoManager->MoveRelative(0, 25);
	ostringstream next;
	next << "XP to Next: " << _current_char->GetExperienceForNextLevel();
	VideoManager->DrawText(MakeUnicodeString(next.str()));

	VideoManager->MoveRelative(0, 25);
	VideoManager->DrawText(MakeUnicodeString("Strength: 106"));

	VideoManager->MoveRelative(0, 25);
	VideoManager->DrawText(MakeUnicodeString("Vigor: 72"));

	VideoManager->MoveRelative(0, 25);
	VideoManager->DrawText(MakeUnicodeString("Fortitude: 106"));

	VideoManager->MoveRelative(0, 25);
	VideoManager->DrawText(MakeUnicodeString("Resistance: 48"));

	VideoManager->MoveRelative(0, 25);
	ostringstream agl;
	agl << "Agility: " << _current_char->GetAgility();
	VideoManager->DrawText(MakeUnicodeString(agl.str()));

	VideoManager->MoveRelative(0, 25);
	VideoManager->DrawText(MakeUnicodeString("Evade: 3%"));

	//Draw character full body portrait
	VideoManager->Move(735, 145);
	VideoManager->DrawImage(_full_portraits[_char_select.GetSelection()]);

	_char_select.Draw();

	DrawBottomMenu();

	/*VideoManager->MoveRelative(0, 50);
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
	VideoManager->DrawText(MakeUnicodeString("Leg Armor"));*/


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

	
	return true;
} // bool StatusWindow::Draw()

void StatusWindow::DrawBottomMenu() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->Move(150, 577);

	//FIX ME: Make dynamic
	//Display Location
	
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
	os_money << GlobalManager->GetFunds();
	std::string money = std::string("Dorrun: ") + os_money.str();
	VideoManager->MoveRelative(0, 30);
	if (!VideoManager->DrawText(MakeUnicodeString(money)))
		cerr << "MENU: ERROR > Couldn't draw text!" << endl;
	
	//VideoManager->SetDrawFlags(VIDEO_X_RIGHT, VIDEO_Y_BOTTOM, 0);
		
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
		
	VideoManager->Move(390, 685);
	VideoManager->DrawImage(_location_picture);
}


SkillsWindow::SkillsWindow()  : _active_box(SKILL_ACTIVE_NONE)/*_char_select_active(false),
									_char_select_apply_active(false),
									_skills_categories_active(false), 
									_skills_list_active(false)*/
{
	InitCharSelect();
	InitSkillsList();
	InitSkillsCategories();
	
	// Load sounds
	SoundDescriptor confirm;
	SoundDescriptor cancel;

	if (confirm.LoadSound("snd/obtain.wav") == false) 
	{
		cerr << "SKILLSWINDOW::UPDATE - Unable to load confirm sound effect!" << endl;
	}
	
	if (cancel.LoadSound("snd/cancel.wav") == false)
	{
		cerr << "SKILLSWINDOW::UPDATE - Unable to load cancel sound effect!" << endl;
	}

	_menu_sounds["confirm"] = confirm;
	_menu_sounds["cancel"] = cancel;
	
}// SkillsWindow::SkillsWindow()

SkillsWindow::~SkillsWindow()
{
	// Clear sounds
	_menu_sounds["confirm"].FreeSound();
	_menu_sounds["cancel"].FreeSound();

}// SkillsWindow::~SkillsWindow()


void SkillsWindow::Activate(bool new_status) {
	//Activate window and first option box...or deactivate both
	//_char_select_active = new_status;

	if (new_status) {
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		_active_box = SKILL_ACTIVE_CHAR;
	}
	else {
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		_active_box = SKILL_ACTIVE_NONE;
	}
}


void SkillsWindow::InitSkillsList() {
	// Set up the inventory option box
	_skills_list.SetCellSize(180.0f, 30.0f);
		
	_skills_list.SetPosition(500.0f, 170.0f);
	_skills_list.SetFont("default");
	
	_skills_list.SetCursorOffset(-52.0f, -20.0f);
	_skills_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_skills_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_skills_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	// Update the skills list
	UpdateSkillList();
	_skills_list.SetSelection(0);
	// Initially hide the cursor
	_skills_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

} // void SkillsWindow::InitInventoryItems()

void SkillsWindow::InitCharSelect() {
	//character selection set up
	vector<ustring> options;
	uint32 size = GlobalManager->GetActiveParty()->GetCharacters().size();
	
	_char_select.SetCursorOffset(-50.0f, -6.0f);
	_char_select.SetFont("default");
	_char_select.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_char_select.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_char_select.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_char_select.SetSize(1, ((size >= 4) ? 4 : size));
	//_char_select.SetSize(1, 4);
	_char_select.SetCellSize(360, 108);
	_char_select.SetPosition(72.0f, 109.0f);

	//Use blank strings....won't be seen anyway
	for (uint32 i = 0; i < size; i++) {
		options.push_back(MakeUnicodeString(" "));
	}

	//Set options, selection and cursor state
	_char_select.SetOptions(options);
	_char_select.SetSelection(0);
	_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	
} //SkillsWindow::InitCharSelect()

void SkillsWindow::InitSkillsCategories() {
	//Set params
	_skills_categories.SetCellSize(105.0f,30.0f);
	_skills_categories.SetPosition(510.0f, 120.0f);
	_skills_categories.SetFont("default");
	_skills_categories.SetSize(SKILL_CATEGORY_SIZE,1);
	
	_skills_categories.SetCursorOffset(-52.0f, -20.0f);
	_skills_categories.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_skills_categories.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_skills_categories.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);

	//Create options
	vector<ustring> options;
	options.push_back(MakeUnicodeString("All"));
	options.push_back(MakeUnicodeString("Field"));
	options.push_back(MakeUnicodeString("Battle"));
		
	//Set options and default selection
	_skills_categories.SetOptions(options);
	_skills_categories.SetSelection(SKILL_ALL);
	_skills_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

	
} // void SkillsWindow::InitSkillsCategories()

void SkillsWindow::Update() {

	//FIX ME: Need support for skills
	return;

	OptionBox *active_option;

	//choose correct menu
	switch (_active_box) {
		case SKILL_ACTIVE_CATEGORY:
			active_option = &_skills_categories;
			break;
		case SKILL_ACTIVE_CHAR_APPLY:
		case SKILL_ACTIVE_CHAR:
			active_option = &_char_select;
			break;
		case SKILL_ACTIVE_LIST:
			active_option = &_skills_list;
			break;
	}
	/*
	if (_char_select_active || _char_select_apply_active) {
		_active_option = &_char_select;
	}
	else if (_skills_categories_active) {
		_active_option = &_skills_categories;
	}
	else if (_skills_list_active) {
		_active_option = &_skills_list;
	}*/
	
	// Handle the appropriate input events
	if (InputManager->ConfirmPress())
	{
		active_option->HandleConfirmKey();
	}
	else if (InputManager->CancelPress())
	{
		active_option->HandleCancelKey();
	}
	else if (InputManager->LeftPress())
	{
		active_option->HandleLeftKey();
	}
	else if (InputManager->RightPress())
	{
		active_option->HandleRightKey();
	}
	else if (InputManager->UpPress())
	{
		active_option->HandleUpKey();
	}
	else if (InputManager->DownPress())
	{
		active_option->HandleDownKey();
	}

	uint32 event = active_option->GetEvent();

	switch (_active_box) {
		case SKILL_ACTIVE_CHAR_APPLY:
			//Handle skill application
			{
				if (event == VIDEO_OPTION_CONFIRM) {
					//TODO Use Skill
					_menu_sounds["confirm"].PlaySound();
				}
				else if (event == VIDEO_OPTION_CANCEL) {
					//_char_select_apply_active = false;
					//_skills_list_active = true;
					_active_box = SKILL_ACTIVE_LIST;
					_skills_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_menu_sounds["cancel"].PlaySound();
				}
			}
			break;

		case SKILL_ACTIVE_CHAR:
			//Choose character for skillset
			{
				if (event == VIDEO_OPTION_CONFIRM) {
					//_char_select_active = false;
					//_skills_categories_active = true;
					_active_box = SKILL_ACTIVE_CATEGORY;
					_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_skills_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					_char_skillset = _char_select.GetSelection();
					_menu_sounds["confirm"].PlaySound();
				}
				else if (event == VIDEO_OPTION_CANCEL) {
					Activate(false);
					_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_menu_sounds["cancel"].PlaySound();
				}
			}
			break;

		case SKILL_ACTIVE_LIST:
			//Choose skill
			{
				if (event == VIDEO_OPTION_CONFIRM) {
					//_char_select_apply_active = true;
					//_skills_list_active = false;
					_active_box = SKILL_ACTIVE_CHAR_APPLY;
					_skills_list.SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
					_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					_menu_sounds["confirm"].PlaySound();
				}
				else if (event == VIDEO_OPTION_CANCEL) {
					//_skills_list_active = false;
					_active_box = SKILL_ACTIVE_CATEGORY;
					_menu_sounds["cancel"].PlaySound();
					_skills_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_skills_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					//_skills_categories_active = true;
				}
			}
			break;

		case SKILL_ACTIVE_CATEGORY:
			//Choose skill type
			{
				if (event == VIDEO_OPTION_CONFIRM) {
					//_skills_categories_active = false;
					_active_box = SKILL_ACTIVE_LIST;
					_skills_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_skills_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					//_skills_list_active = true;
					_menu_sounds["confirm"].PlaySound();
				}
				else if (event == VIDEO_OPTION_CANCEL) {
					//_skills_categories_active = false;
					//_char_select_active = true;
					_active_box = SKILL_ACTIVE_CHAR;
					_menu_sounds["cancel"].PlaySound();
					_skills_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					_char_select.SetSelection(_char_skillset);
				}
			}
			break;
	}

	/*//Handle skill application
	if (_char_select_apply_active) {
		if (event == VIDEO_OPTION_CONFIRM) {
			//TODO Use Skill
			_menu_sounds["confirm"].PlaySound();
		}
		else if (event == VIDEO_OPTION_CANCEL) {
			_char_select_apply_active = false;
			_skills_list_active = true;
			_skills_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_menu_sounds["cancel"].PlaySound();
		}
	}

	//Choose character for skillset
	else if (_char_select_active) {

		if (event == VIDEO_OPTION_CONFIRM) {
			_char_select_active = false;
			_skills_categories_active = true;
			_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_skills_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_char_skillset = _char_select.GetSelection();
			_menu_sounds["confirm"].PlaySound();
		}
		else if (event == VIDEO_OPTION_CANCEL) {
			Activate(false);
			_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_menu_sounds["cancel"].PlaySound();
		}
	}
	
	//Choose skill
	else if (_skills_list_active)
	{
		
		if (event == VIDEO_OPTION_CONFIRM) {
			_char_select_apply_active = true;
			_skills_list_active = false;
			_skills_list.SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
			_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_menu_sounds["confirm"].PlaySound();
		}
		else if (event == VIDEO_OPTION_CANCEL) {
			_skills_list_active = false;
			_menu_sounds["cancel"].PlaySound();
			_skills_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_skills_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_skills_categories_active = true;
		}
	}

	//Choose skill type
	else if (_skills_categories_active) {
		if (event == VIDEO_OPTION_CONFIRM) {
			_skills_categories_active = false;
			_skills_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_skills_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_skills_list_active = true;
			_menu_sounds["confirm"].PlaySound();
		}
		else if (event == VIDEO_OPTION_CANCEL) {
			_skills_categories_active = false;
			_char_select_active = true;
			_menu_sounds["cancel"].PlaySound();
			_skills_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_char_select.SetSelection(_char_skillset);
		}
	}*/

	UpdateSkillList();
} // void SkillsWindow::Update()

void SkillsWindow::UpdateSkillList() {
	//uint32 partysize = GlobalManager->GetParty().size();

	//hoa_global::GlobalCharacter* ch = GlobalManager->GetCharacter(_char_select.GetSelection());
	std::vector<ustring> options;

	//FIX ME Need new categories
	/*std::vector<hoa_global::GlobalSkill*> skills = ch->GetAttackSkills();
	uint32 skillsize = skills.size();

	

	_skills_list.SetSize(1,skillsize);
	

	for (uint32 i = 0; i < skillsize; i++) {
		os.clear();
		os << skills[i]->GetName() << "              " << skills[i]->GetSPUsage() << "SP";
		options.push_back(MakeUnicodeString(os.str()));
	}*/

	//FIX ME: Test code
	/*for (uint32 i = 0; i < 12; i++) {
		options.push_back(MakeUnicodeString("Sword Strike                       30 SP"));
	}
*/
	_skills_list.SetSize(1,12);

	_skills_list.SetOptions(options);

}

bool SkillsWindow::Draw() {
	if (MenuWindow::Draw() == false)
		return false;

	//Draw option boxes
	_char_select.Draw();
	_skills_categories.Draw();
	_skills_list.Draw();

	return true;

}

void SkillsWindow::DrawBottomMenu() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->Move(150, 577);

	//FIX ME: Make dynamic
	//Display Location
	
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
	os_money << GlobalManager->GetFunds();
	std::string money = std::string("Dorrun: ") + os_money.str();
	VideoManager->MoveRelative(0, 30);
	if (!VideoManager->DrawText(MakeUnicodeString(money)))
		cerr << "MENU: ERROR > Couldn't draw text!" << endl;
}


EquipWindow::EquipWindow() : _active_box(EQUIP_ACTIVE_NONE)
{
	InitCharSelect();
	InitEquipmentSelect();
	InitEquipmentList();
	
	// Load sounds
	SoundDescriptor confirm;
	SoundDescriptor cancel;

	if (confirm.LoadSound("snd/obtain.wav") == false) 
	{
		cerr << "SKILLSWINDOW::UPDATE - Unable to load confirm sound effect!" << endl;
	}
	
	if (cancel.LoadSound("snd/cancel.wav") == false)
	{
		cerr << "SKILLSWINDOW::UPDATE - Unable to load cancel sound effect!" << endl;
	}

	_menu_sounds["confirm"] = confirm;
	_menu_sounds["cancel"] = cancel;

	StillImage i;

	i.SetFilename("img/icons/weapons/karlate_sword.png");
	_equip_images.push_back(i);

	i.SetFilename("img/icons/armor/karlate_helmet.png");
	_equip_images.push_back(i);

	i.SetFilename("img/icons/armor/karlate_breastplate.png");
	_equip_images.push_back(i);

	i.SetFilename("img/icons/armor/karlate_shield.png");
	_equip_images.push_back(i);

	i.SetFilename("img/icons/armor/karlate_greaves.png");
	_equip_images.push_back(i);

	for (uint32 i = 0; i < EQUIP_CATEGORY_SIZE; i++) {
		_equip_images[i].SetDimensions(60, 60);
		VideoManager->LoadImage(_equip_images[i]);
	}
	
}// EquipWindow::EquipWindow()

EquipWindow::~EquipWindow()
{
	// Clear sounds
	_menu_sounds["confirm"].FreeSound();
	_menu_sounds["cancel"].FreeSound();

	for (uint32 i = 0; i < EQUIP_CATEGORY_SIZE; i++) {
		VideoManager->DeleteImage(_equip_images[i]);
	}

}// EquipWindow::~EquipWindow()


void EquipWindow::Activate(bool new_status) {
	//Activate window and first option box...or deactivate both
	//_char_select_active = new_status;

	if (new_status) {
		_active_box = EQUIP_ACTIVE_CHAR;
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	}
	else {
		_active_box = EQUIP_ACTIVE_NONE;
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	}
}


void EquipWindow::InitEquipmentList() {
	// Set up the inventory option box
	_equip_list.SetCellSize(180.0f, 30.0f);
		
	_equip_list.SetPosition(500.0f, 170.0f);
	_equip_list.SetFont("default");
	
	_equip_list.SetCursorOffset(-52.0f, -20.0f);
	_equip_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_equip_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_equip_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	// Update the equipment list
	//UpdateEquipList();
	_equip_list.SetSelection(0);
	// Initially hide the cursor
	_equip_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

} // void EquipWindow::InitInventoryItems()

void EquipWindow::InitCharSelect() {
	//character selection set up
	vector<ustring> options;
	uint32 size = GlobalManager->GetActiveParty()->GetCharacters().size();
	
	_char_select.SetCursorOffset(-50.0f, -6.0f);
	_char_select.SetFont("default");
	_char_select.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_char_select.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_char_select.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_char_select.SetSize(1, ((size >= 4) ? 4 : size));
	//_char_select.SetSize(1, 4);
	_char_select.SetCellSize(360, 108);
	_char_select.SetPosition(72.0f, 109.0f);

	//Use blank strings....won't be seen anyway
	for (uint32 i = 0; i < size; i++) {
		options.push_back(MakeUnicodeString(" "));
	}

	//Set options, selection and cursor state
	_char_select.SetOptions(options);
	_char_select.SetSelection(0);
	_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	
} // void EquipWindow::InitCharSelect()

void EquipWindow::InitEquipmentSelect() {
	//Set params
	_equip_select.SetCellSize(105.0f,70.0f);
	_equip_select.SetPosition(680.0f, 145.0f);
	_equip_select.SetFont("default");
	_equip_select.SetSize(1,EQUIP_CATEGORY_SIZE);
	
	_equip_select.SetCursorOffset(-132.0f, -20.0f);
	_equip_select.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_equip_select.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_equip_select.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);

	//Set options and default selection
	
	_equip_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	UpdateEquipList();
	_equip_select.SetSelection(EQUIP_WEAPON);

	
} // void EquipWindow::InitEquipmentSelect()

void EquipWindow::Update() {
	// FIXME: may cause a seg. fault, however when not initialized gcc spits out a
	// warning saying it might be used uninitialized in this function.
	OptionBox *active_option = NULL;

	//choose correct menu
	switch (_active_box) {
		case EQUIP_ACTIVE_CHAR:
			active_option = &_char_select;
			break;
		case EQUIP_ACTIVE_SELECT:
			active_option = &_equip_select;
			break;
		case EQUIP_ACTIVE_LIST:
			active_option = &_equip_list;
			break;
	}

	/*if (_char_select_active) {
		active_option = &_char_select;
	}
	else if (_equip_select_active) {
		active_option = &_equip_select;
	}
	else if (_equip_list_active) {
		active_option = &_equip_list;
	}*/
	
	// Handle the appropriate input events
	if (InputManager->ConfirmPress())
	{
		active_option->HandleConfirmKey();
	}
	else if (InputManager->CancelPress())
	{
		active_option->HandleCancelKey();
	}
	else if (InputManager->LeftPress())
	{
		active_option->HandleLeftKey();
	}
	else if (InputManager->RightPress())
	{
		active_option->HandleRightKey();
	}
	else if (InputManager->UpPress())
	{
		active_option->HandleUpKey();
	}
	else if (InputManager->DownPress())
	{
		active_option->HandleDownKey();
	}

	uint32 event = active_option->GetEvent();

	switch (_active_box) {
		//Choose character
		case EQUIP_ACTIVE_CHAR:
			{
				if (event == VIDEO_OPTION_CONFIRM) {
					_active_box = EQUIP_ACTIVE_SELECT;
					_char_select.SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
					_equip_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					_menu_sounds["confirm"].PlaySound();
				}
				else if (event == VIDEO_OPTION_CANCEL) {
					Activate(false);
					_menu_sounds["cancel"].PlaySound();
				}
			}
			break;

		//Choose equipment to replace
		case EQUIP_ACTIVE_SELECT:
			{
				if (event == VIDEO_OPTION_CONFIRM) {
					_active_box = EQUIP_ACTIVE_LIST;
					_equip_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_equip_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					_menu_sounds["confirm"].PlaySound();
				}
				else if (event == VIDEO_OPTION_CANCEL) {
					_active_box = EQUIP_ACTIVE_CHAR;
					_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					_equip_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_menu_sounds["cancel"].PlaySound();
				}
			}
			break;

		//Choose replacement
		case EQUIP_ACTIVE_LIST:
			{
				if (event == VIDEO_OPTION_CONFIRM) {
					//TODO Change Equipment, handle removal
					_active_box = EQUIP_ACTIVE_SELECT;
					_equip_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_equip_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					_menu_sounds["confirm"].PlaySound();
				}
				else if (event == VIDEO_OPTION_CANCEL) {
					_active_box = EQUIP_ACTIVE_SELECT;
					_menu_sounds["cancel"].PlaySound();
					_equip_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_equip_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				}
			}
			break;
	}
	//Choose character
	/*if (_char_select_active) {
		if (event == VIDEO_OPTION_CONFIRM) {
			_char_select_active = false;
			_char_select.SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
			_equip_select_active = true;
			_equip_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_menu_sounds["confirm"].PlaySound();
		}
		else if (event == VIDEO_OPTION_CANCEL) {
			Activate(false);
			_menu_sounds["cancel"].PlaySound();
		}
	}

	//Choose equipment to replace
	else if (_equip_select_active) {

		if (event == VIDEO_OPTION_CONFIRM) {
			_equip_select_active = false;
			_equip_list_active = true;
			_equip_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_equip_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_menu_sounds["confirm"].PlaySound();
		}
		else if (event == VIDEO_OPTION_CANCEL) {
			_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_char_select_active = true;
			_equip_select_active = false;
			_equip_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_menu_sounds["cancel"].PlaySound();
		}
	}
	
	//Choose replacement
	else if (_equip_list_active)
	{
		
		if (event == VIDEO_OPTION_CONFIRM) {
			//TODO Change Equipment, handle removal
			_equip_select_active = true;
			_equip_list_active = false;
			_equip_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_equip_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			_menu_sounds["confirm"].PlaySound();
		}
		else if (event == VIDEO_OPTION_CANCEL) {
			//_skills_list_active = false;
			_menu_sounds["cancel"].PlaySound();
			_equip_select_active = true;
			_equip_list_active = false;
			_equip_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			_equip_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		}

	}*/

	//UpdateEquipList();
} // void EquipWindow::Update()

void EquipWindow::UpdateEquipList() {
	// FIXME: warning, unused variable
	//hoa_global::GlobalCharacter* ch = GlobalManager->GetCharacter(_char_select.GetSelection());
	std::vector<ustring> options;

	if (_active_box == EQUIP_ACTIVE_LIST) {
		uint32 gearsize;
		vector<hoa_global::GlobalWeapon*> weapons;
		vector<hoa_global::GlobalArmor*> armor;
		map<uint32, hoa_global::GlobalObject*> inv = GlobalManager->GetInventory();
		uint32 invsize = inv.size();

		switch (_equip_select.GetSelection()) {
			case EQUIP_WEAPON:
				for (uint32 i = 0; i < invsize; i++) {
					if (inv[i]->GetType() == GLOBAL_WEAPON) { // && usable by cur char
						weapons.push_back(static_cast<GlobalWeapon*>(inv[i]));
					}
				}

				gearsize = weapons.size();

				for (uint32 j = 0; j < gearsize; j++) {
					options.push_back(weapons[j]->GetName());
				}

				_equip_list.SetOptions(options);				
				break;

			case EQUIP_HEADGEAR:
				for (uint32 i = 0; i < invsize; i++) {
					if (inv[i]->GetType() == GLOBAL_HEAD_ARMOR) { // && usable by cur char
						armor.push_back(static_cast<GlobalArmor*>(inv[i]));
					}
				}

				gearsize = armor.size();

				for (uint32 j = 0; j < gearsize; j++) {
					options.push_back(armor[j]->GetName());
				}

				_equip_list.SetOptions(options);
				break;

			case EQUIP_BODYARMOR:
				for (uint32 i = 0; i < invsize; i++) {
					if (inv[i]->GetType() == GLOBAL_TORSO_ARMOR) { // && usable by cur char
						armor.push_back(static_cast<GlobalArmor*>(inv[i]));
					}
				}

				gearsize = armor.size();

				for (uint32 j = 0; j < gearsize; j++) {
					options.push_back(armor[j]->GetName());
				}

				_equip_list.SetOptions(options);
				break;

			case EQUIP_OFFHAND:
				for (uint32 i = 0; i < invsize; i++) {
					if (inv[i]->GetType() == GLOBAL_ARMS_ARMOR) { // && usable by cur char
						armor.push_back(static_cast<GlobalArmor*>(inv[i]));
					}
				}

				gearsize = armor.size();

				for (uint32 j = 0; j < gearsize; j++) {
					options.push_back(armor[j]->GetName());
				}

				_equip_list.SetOptions(options);
				break;

			case EQUIP_LEGGINGS:
				for (uint32 i = 0; i < invsize; i++) {
					if (inv[i]->GetType() == GLOBAL_LEGS_ARMOR) { // && usable by cur char
						armor.push_back(static_cast<GlobalArmor*>(inv[i]));
					}
				}

				gearsize = armor.size();

				for (uint32 j = 0; j < gearsize; j++) {
					options.push_back(armor[j]->GetName());
				}

				_equip_list.SetOptions(options);
				break;
		}
	}

	else {
		/*options.push_back(MakeUnicodeString(ch->GetWeapon()->GetName()));
		options.push_back(MakeUnicodeString(ch->GetHeadArmor()->GetName()));
		options.push_back(MakeUnicodeString(ch->GetBodyArmor()->GetName()));
		options.push_back(MakeUnicodeString(ch->GetArmsArmor()->GetName()));
		options.push_back(MakeUnicodeString(ch->GetLegArmor()->GetName()));*/
		options.push_back(MakeUnicodeString("Karlate Sword"));
		options.push_back(MakeUnicodeString("Karlate Helmet"));
		options.push_back(MakeUnicodeString("Karlate Breastplate"));
		options.push_back(MakeUnicodeString("Karlate Shield"));
		options.push_back(MakeUnicodeString("Karlate Greaves"));
		
		_equip_select.SetOptions(options);
	}

}

bool EquipWindow::Draw() {
	if (MenuWindow::Draw() == false)
		return false;

	//Draw option boxes
	_char_select.Draw();

	if (_active_box == EQUIP_ACTIVE_LIST) {
		_equip_list.Draw();
		VideoManager->Move(660.0f, 135.0f);
		VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
		switch (_equip_select.GetSelection()) {
			case EQUIP_WEAPON: 
				VideoManager->DrawText(MakeUnicodeString("Weapons"));
				break;
			case EQUIP_HEADGEAR:
				VideoManager->DrawText(MakeUnicodeString("Headgear"));
				break;
			case EQUIP_BODYARMOR:
				VideoManager->DrawText(MakeUnicodeString("Body Armor"));
				break;
			case EQUIP_OFFHAND:
				VideoManager->DrawText(MakeUnicodeString("Offhand"));
				break;
			case EQUIP_LEGGINGS:
				VideoManager->DrawText(MakeUnicodeString("Leggings"));
				break;
		}
	}
	else {
		_equip_select.Draw();

		// FIX ME: warning: unused variable
		//hoa_global::GlobalCharacter *ch = GlobalManager->GetParty()[_char_select.GetSelection()];

		//FIX ME: Use XML tags for formatting option boxes
		VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
		VideoManager->Move(450.0f, 170.0f);
		VideoManager->DrawText(MakeUnicodeString("Weapon"));
		VideoManager->MoveRelative(0.0f, 70.0f);
		VideoManager->DrawText(MakeUnicodeString("Headgear"));
		VideoManager->MoveRelative(0.0f, 70.0f);
		VideoManager->DrawText(MakeUnicodeString("Body Armor"));
		VideoManager->MoveRelative(0.0f, 70.0f);
		VideoManager->DrawText(MakeUnicodeString("Offhand"));
		VideoManager->MoveRelative(0.0f, 70.0f);
		VideoManager->DrawText(MakeUnicodeString("Leggings"));

		VideoManager->MoveRelative(150.0f, -370.0f);
		
		for (uint32 i = 0; i < _equip_images.size(); i++) {
			VideoManager->MoveRelative(0.0f, 70.0f);
			VideoManager->DrawImage(_equip_images[i]);
		}
	}

	return true;

} // bool EquipWindow::Draw()


FormationWindow::FormationWindow() {
}

FormationWindow::~FormationWindow() {
}

bool FormationWindow::Draw() {
	if (MenuWindow::Draw() == false) {
		return false;
	}
	return true;
}

} // namespace private_menu

} // namespace hoa_menu
