///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    menu_views.cpp
 * \author  Daniel Steuernol steu@allacrost.org
 * \author  Andy Gardner chopperdave@allacrost.org
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
	_char_id = GLOBAL_CHARACTER_INVALID;
}



CharacterWindow::~CharacterWindow()
{
	// Delete the character's portrait
	VideoManager->DeleteImage(_portrait);
}



void CharacterWindow::SetCharacter(GlobalCharacter *character)
{
	_char_id = character->GetID();

	// TODO: Load the portrait
	_portrait.SetFilename("img/portraits/map/" + character->GetFilename() + ".png");
	_portrait.SetStatic(true);
	_portrait.SetDimensions(100, 100);
	VideoManager->LoadImage(_portrait);
} // void CharacterWindow::SetCharacter(GlobalCharacter *character)



// Draw the window to the screen
void CharacterWindow::Draw()
{
	// Call parent Draw method, if failed pass on fail result
	MenuWindow::Draw();

	// check to see if this window is an actual character
	if (_char_id == hoa_global::GLOBAL_CHARACTER_INVALID)
		// no more to do here
		return;

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);

	// Get the window metrics
	float x, y, w, h;
	GetPosition(x,y);
	GetDimensions(w,h);
/*<<<<<<< .mine
=======

	// check to see if this window is an actual character
	if (_char_id == hoa_global::GLOBAL_CHARACTER_INVALID)
		// no more to do here
		return;
>>>>>>> .r528*/

	GlobalCharacter *character = GlobalManager->GetCharacter(_char_id);

	//Draw character portrait
	VideoManager->Move(x + 12, y + 8);
	VideoManager->DrawImage(_portrait);

	// Write character name
	VideoManager->MoveRelative(150, 0);
	if (!VideoManager->DrawText(character->GetName()))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw Character Name!" << endl;

	// Level
	VideoManager->MoveRelative(0,20);
	std::ostringstream os_level;
	os_level << character->GetExperienceLevel();
	std::string xp_level = std::string("Lv: ") + os_level.str();
	if (!VideoManager->DrawText(MakeUnicodeString(xp_level)))
		cerr << "CHARACTERWINDOW: ERROR: > Couldn't draw xp level" << endl;

	// HP
	VideoManager->MoveRelative(0,20);
	ostringstream os_health;
	os_health << character->GetHitPoints() << " / " << character->GetMaxHitPoints();
	std::string health = std::string("HP: ") + os_health.str();
	if (!VideoManager->DrawText(MakeUnicodeString(health)))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw health!" << endl;

	// SP
	VideoManager->MoveRelative(0,20);
	ostringstream os_skill;
	os_skill << character->GetSkillPoints() << " / " << character->GetMaxSkillPoints();
	std::string skill = std::string("SP: ") + os_skill.str();
	if (!VideoManager->DrawText(MakeUnicodeString(skill)))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw skill!" << endl;

	// XP to level up
	VideoManager->MoveRelative(0, 20);
	ostringstream os_xp;
	os_xp << character->GetExperienceForNextLevel();
	std::string xp = std::string("XP To Next: ") + os_xp.str();
	if (!VideoManager->DrawText(MakeUnicodeString(xp)))
		cerr << "CHARACTERWINDOW: ERROR > Couldn't draw xp!" << endl;

	return;
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
void MiniCharacterSelectWindow::Draw()
{
	MenuWindow::Draw();

	if (!_char_window_active)
		return;

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);

	// Draw Portraits of the party
	for (uint32 i = 0; i < GlobalManager->GetActiveParty()->GetPartySize(); ++i)
	{
		VideoManager->Move(765, 180);

		GlobalCharacter *current = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActor(i));
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

	return;
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
		GlobalItem *selected = (*GlobalManager->GetInventoryItems())[_selected_item_index];
		GlobalCharacter *ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActor(_current_char_selected));

		if (selected->GetCount() == 0)
		{
			// no more items to use
			MenuMode::_instance->_menu_sounds["bump"].PlaySound();
			return;
		}

		// check character hp
		if (ch->GetHitPoints() == ch->GetMaxHitPoints())
		{
			// don't use item we're full
			MenuMode::_instance->_menu_sounds["bump"].PlaySound();
			return;
		}

		// Play Sound
// 		_menu_sounds["potion"].PlaySound();

		// increase hp FIXME
		if (selected->GetUsage() == GLOBAL_USE_MENU)
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
			Activate(false);
			Hide();
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


InventoryWindow::InventoryWindow() : _active_box(ITEM_ACTIVE_NONE)
{
	_InitCategory();
	_InitInventoryItems();
	_InitCharSelect();

/*	None of this is useful... do we need any of it anymore?
	//Load char portraits for bottom menu
	StillImage i;

	//FIX ME: Make dynamic based on char names
	i.SetFilename("img/portraits/battle/claudius.png");

	i.SetDimensions(100.0f, 100.0f);
	_portraits.push_back(i);
	cout << "Warning Here?" << endl;
	VideoManager->LoadImage(_portraits[0]);
	cout << "Warning Here?" << endl;
*/
	_location_graphic.SetFilename("img/menus/locations/desert_cave.png");
	_location_graphic.SetDimensions(500.0f, 125.0f);
	VideoManager->LoadImage(_location_graphic);

	//Initializes the description textbox for the bottom window
	_description.SetOwner(this);
	_description.SetPosition(30.0f, 525.0f);
	_description.SetDimensions(800.0f, 80.0f);
	_description.SetDisplaySpeed(30);
	_description.SetFont("default");
	_description.SetDisplayMode(VIDEO_TEXT_INSTANT);
	_description.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);

}// void InventoryWindow::InventoryWindow

InventoryWindow::~InventoryWindow()
{
	// Delete portraits
//	VideoManager->DeleteImage(_portraits[0]);
	VideoManager->DeleteImage(_location_graphic);
}

//Initializes the list of items
void InventoryWindow::_InitInventoryItems() {
	// Set up the inventory option box
	_inventory_items.SetCellSize(400.0f, 60.0f);

	_inventory_items.SetPosition(500.0f, 170.0f);
	_inventory_items.SetFont("default");
	_inventory_items.SetCursorOffset(-52.0f, -20.0f);
	_inventory_items.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_inventory_items.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_inventory_items.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_inventory_items.TEMP_OverideScissorring(true);

	// Update the item text
	_UpdateItemText();
	_inventory_items.SetSelection(0);		VideoManager->MoveRelative(-65, 20);
	// Initially hide the cursor
	_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}

//Initalizes character select
void InventoryWindow::_InitCharSelect() {
	//character selection set up
	vector<ustring> options;
	uint32 size = GlobalManager->GetActiveParty()->GetPartySize();

	_char_select.SetCursorOffset(-50.0f, -6.0f);
	_char_select.SetFont("default");
	_char_select.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_char_select.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_char_select.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_char_select.SetSize(1, ((size >= 4) ? 4 : size));
	_char_select.SetSize(1, 4);
	_char_select.SetCellSize(360, 108);
	_char_select.SetPosition(72.0f, 109.0f);

	//Use a blank string so the cursor has somewhere to point
	//String is overdrawn by char portraits, so no matter
	for (uint32 i = 0; i < size; i++) {
		options.push_back(MakeUnicodeString(" "));
	}

	_char_select.SetOptions(options);
	_char_select.SetSelection(0);
	_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}

//Initalizes the available item categories
void InventoryWindow::_InitCategory() {
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

	//FIXME: Reenable later
	_item_categories.EnableOption(ITEM_KEY,false);
}

// Activates/deactivates inventory window
void InventoryWindow::Activate(bool new_status)
{
	// Set new status
	if (_inventory_items.GetNumOptions() > 0 && new_status) {
		_active_box = ITEM_ACTIVE_CATEGORY;
		// Update cursor state
		_item_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	}
	else {
		//FIX ME: Play N/A noise
		_active_box = ITEM_ACTIVE_NONE;
		_item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	}
}

// Updates the window
void InventoryWindow::Update() {

	//bool cancel = false;
	if ( GlobalManager->GetInventory()->size() == 0 )
	{
		// no more items in inventory, exit inventory window
		Activate(false);
		return;
	}

	// Points to the active option box
	OptionBox *active_option = NULL;

	_inventory_items.Update( SystemManager->GetUpdateTime() ); //For scrolling

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

	// Handle confirm/cancel presses differently for each window
	switch (_active_box) {
		case ITEM_ACTIVE_NONE: break;

		case ITEM_ACTIVE_CATEGORY:
			{
				// Activate the item list for this category
				if (event == VIDEO_OPTION_CONFIRM) {
					if (_inventory_items.GetNumOptions() > 0) {
						_item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
						_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
						_description.SetDisplayText( _item_objects[ 0 ]->GetDescription() );
						_active_box = ITEM_ACTIVE_LIST;
						MenuMode::_instance->_menu_sounds["confirm"].PlaySound();
					}
				}
				// Deactivate inventory
				else if (event == VIDEO_OPTION_CANCEL) {
					MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
					_item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					Activate(false);
				}
			}
			break;

		case ITEM_ACTIVE_LIST:
			{
				// Activate the character select for application
				if (event == VIDEO_OPTION_CONFIRM) {
					_active_box = ITEM_ACTIVE_CHAR;
					_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
					_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					MenuMode::_instance->_menu_sounds["confirm"].PlaySound();
				}
				// Return to category selection
				else if (event == VIDEO_OPTION_CANCEL) {
					_active_box = ITEM_ACTIVE_CATEGORY;
					MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
					_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_item_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				}
				else if ( event == VIDEO_OPTION_BOUNDS_UP || VIDEO_OPTION_BOUNDS_DOWN )
				{
					_description.SetDisplayText( _item_objects[ _inventory_items.GetSelection() ]->GetDescription() );
				}
			}
			break;

		case ITEM_ACTIVE_CHAR:
			{
				// Use the item on the chosen character
				if (event == VIDEO_OPTION_CONFIRM) {
					GlobalObject* obj = _item_objects[ _inventory_items.GetSelection() ];
					GlobalCharacter *ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActor(_char_select.GetSelection()));
					if (obj->GetType() == GLOBAL_OBJECT_ITEM) {
						GlobalItem *item = (GlobalItem*)obj;
						item->MenuUse(ch);
						if (item->GetCount() <= 0) {
							GlobalManager->RemoveFromInventory(item->GetID());
						} // if
					} // if
					//_menu_sounds["confirm"].PlaySound();
				}
				// Return to item selection
				else if (event == VIDEO_OPTION_CANCEL) {
					_active_box = ITEM_ACTIVE_LIST;
					_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
				}
			}
			break;
	}

//<<<<<<< .mine
	// Update the item list
	_UpdateItemText();
//=======
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
	/*UpdateItemText();
>>>>>>> .r528*/

} // void InventoryWindow::Update()

// FIXME Temp function
void InventoryWindow::_TEMP_ApplyItem() {

	// Use the passed in item to update values
	GlobalItem *selected = (*GlobalManager->GetInventoryItems())[_inventory_items.GetSelection()];
	GlobalCharacter *ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActor(_char_select.GetSelection()));

	if (selected->GetCount() == 0)
	{
		// no more items to use
		MenuMode::_instance->_menu_sounds["bump"].PlaySound();
		return;
	}

	// check character hp
	if (ch->GetHitPoints() == ch->GetMaxHitPoints())
	{
		// don't use item we're full
		MenuMode::_instance->_menu_sounds["bump"].PlaySound();
		return;
	}

	// Play Sound
// 	_menu_sounds["potion"].PlaySound();

	// increase hp
	if (selected->GetUsage() == GLOBAL_USE_MENU)
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
		if (GlobalManager->GetInventoryItems()->size() > 0) {
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
		//Activate(false);
		//Hide();
	}
}

// Updates the item list
void InventoryWindow::_UpdateItemText()
{
	// Get the inventory items
	std::map<uint32, GlobalObject*>* inv = GlobalManager->GetInventory();
	std::vector<GlobalItem*>* invItems = GlobalManager->GetInventoryItems();
	std::vector<GlobalWeapon*>* invWeapons = GlobalManager->GetInventoryWeapons();
	std::vector<GlobalArmor*>* invTorsoArmor = GlobalManager->GetInventoryTorsoArmor();
	std::vector<GlobalArmor*>* invHeadArmor = GlobalManager->GetInventoryHeadArmor();
	std::vector<GlobalArmor*>* invArmArmor = GlobalManager->GetInventoryArmArmor();
	std::vector<GlobalArmor*>* invLegArmor = GlobalManager->GetInventoryLegArmor();
	std::vector<GlobalKeyItem*>* invKeyItems = GlobalManager->GetInventoryKeyItems();


	// For item names
	std::vector<ustring> inv_names;
	// For iterating through items
	std::map<uint32, GlobalObject*>::iterator i;
	std::vector<GlobalItem*>::iterator i_item;
	std::vector<GlobalWeapon*>::iterator i_weapon;
	std::vector<GlobalArmor*>::iterator i_armor;
	std::vector<GlobalKeyItem*>::iterator i_key;

	// temporary object storage variables
	GlobalObject* obj;
	GlobalItem* item;
	GlobalWeapon* weapon;
	GlobalArmor* armor;
	GlobalKeyItem* key_item;

	_item_objects.clear();

	// Temp var to hold option
	string text;

	//FIX ME - When video engine is fixed, take out MakeStandardString
	switch (_item_categories.GetSelection()) {

		//Index all items
		case ITEM_ALL:
			for (i = inv->begin(); i != inv->end(); i++) {
				obj = i->second;
				text = "<" + obj->GetIconImage().GetFilename() + "><32>     " + MakeStandardString(obj->GetName()) + "<R><350>" + NumberToString(obj->GetCount()) + "   ";
				inv_names.push_back(MakeUnicodeString(text));
				_item_objects.push_back( obj );
			}
			break;


		//Index menu items only
		case ITEM_FIELD:
			for (i_item = invItems->begin(); i_item != invItems->end(); i_item++) {
				item = *i_item;
				if( item->GetUsage() == GLOBAL_USE_MENU || item->GetUsage() == GLOBAL_USE_ALL  ) {
					text = "<" + item->GetIconImage().GetFilename() + "><32>     " + MakeStandardString(item->GetName()) + "<R><350>" + NumberToString(item->GetCount()) + "   ";
					inv_names.push_back(MakeUnicodeString(text));
					_item_objects.push_back( item );
				}
			}
			break;

		//Index battle items only
		case ITEM_BATTLE:
			for (i_item = invItems->begin(); i_item != invItems->end(); i_item++) {
				item = *i_item;
				if( item->GetUsage() == GLOBAL_USE_BATTLE ) {
					text = "<" + item->GetIconImage().GetFilename() + "><32>     " + MakeStandardString(item->GetName()) + "<R><350>" + NumberToString(item->GetCount()) + "   ";
					inv_names.push_back(MakeUnicodeString(text));
					_item_objects.push_back( item );
				}
			}
			break;

		//Index equipment only
		case ITEM_EQUIPMENT:
			for (i_weapon = invWeapons->begin(); i_weapon != invWeapons->end(); i_weapon++) {
				weapon = *i_weapon;
				text = "<" + weapon->GetIconImage().GetFilename() + "><32>     " + MakeStandardString(weapon->GetName()) + "<R><350>" + NumberToString(weapon->GetCount()) + "   ";
				inv_names.push_back(MakeUnicodeString(text));
				_item_objects.push_back( weapon );
			}

			for (i_armor = invHeadArmor->begin(); i_armor != invHeadArmor->end(); i_armor++) {
				armor = *i_armor;
				text = "<" + armor->GetIconImage().GetFilename() + "><32>     " + MakeStandardString(armor->GetName()) + "<R><350>" + NumberToString(armor->GetCount()) + "   ";
				inv_names.push_back(MakeUnicodeString(text));
				_item_objects.push_back( armor );
			}

			for (i_armor = invTorsoArmor->begin(); i_armor != invTorsoArmor->end(); i_armor++) {
				armor = *i_armor;
				text = "<" + armor->GetIconImage().GetFilename() + "><32>     " + MakeStandardString(armor->GetName()) + "<R><350>" + NumberToString(armor->GetCount()) + "   ";
				inv_names.push_back(MakeUnicodeString(text));
				_item_objects.push_back( armor );
			}

			for (i_armor = invArmArmor->begin(); i_armor != invArmArmor->end(); i_armor++) {
				armor = *i_armor;
				text = "<" + armor->GetIconImage().GetFilename() + "><32>     " + MakeStandardString(armor->GetName()) + "<R><350>" + NumberToString(armor->GetCount()) + "   ";
				inv_names.push_back(MakeUnicodeString(text));
				_item_objects.push_back( armor );
			}

			for (i_armor = invLegArmor->begin(); i_armor != invLegArmor->end(); i_armor++) {
				armor = *i_armor;
				text = "<" + armor->GetIconImage().GetFilename() + "><32>     " + MakeStandardString(armor->GetName()) + "<R><350>" + NumberToString(armor->GetCount()) + "   ";
				inv_names.push_back(MakeUnicodeString(text));
				_item_objects.push_back( armor );
			}

			break;

		case ITEM_KEY:
			for (i_key = invKeyItems->begin(); i_key != invKeyItems->end(); i_key++) {
				key_item = *i_key;
				text = "<" + key_item->GetIconImage().GetFilename() + "><32>     " + MakeStandardString(key_item->GetName()) + "<R><350>" + NumberToString(key_item->GetCount()) + "   ";
				inv_names.push_back(MakeUnicodeString(text));
				_item_objects.push_back( key_item );
			}
			break;
	}

	_inventory_items.SetSize(1,6);
	_inventory_items.SetOptions(inv_names);
} // void InventoryWindow::UpdateItemText()



void InventoryWindow::Draw()
{
	MenuWindow::Draw();

	// This is part of the bottom menu, but there's really no way to run it from menu.cpp...
	if (_active_box == ITEM_ACTIVE_LIST) {
		GlobalObject* obj = _item_objects[ _inventory_items.GetSelection() ];

		VideoManager->SetDrawFlags(VIDEO_X_LEFT,VIDEO_Y_CENTER,0);

		VideoManager->Move(100, 600);
		VideoManager->DrawImage(obj->GetIconImage() );
		VideoManager->MoveRelative(65, 0);
		VideoManager->DrawText(obj->GetName());
		VideoManager->SetDrawFlags(VIDEO_X_LEFT,VIDEO_Y_BOTTOM,0);
		_description.Draw();
	} // if

	// Update the item text in case the number of items changed.
	_UpdateItemText();

	// Draw char select option box
	_char_select.Draw();

	// Draw item categories option box
	_item_categories.Draw();

	// Draw item list
	_inventory_items.Draw();

	return;
/*<<<<<<< .mine
=======
	// Draw the char window
	// _char_window.Draw();
>>>>>>> .r528*/

} // bool InventoryWindow::Draw()


////////////////////////////////////////////////////////////////////////////////
// StatusWindow Class
////////////////////////////////////////////////////////////////////////////////

StatusWindow::StatusWindow() : _char_select_active(false)
{
	// Get party size for iteration
	uint32 partysize = GlobalManager->GetActiveParty()->GetPartySize();
	StillImage portrait;

	// Set up the full body portrait
	for (uint32 i = 0; i < partysize; i++) {
		// FIXME: Make applicable for other characters
		string full_path = string("img/portraits/menu/claudius_large.png");
		portrait.SetFilename(full_path);
		portrait.SetStatic(true);
		portrait.SetDimensions(150, 350);
		VideoManager->LoadImage(portrait);
		_full_portraits.push_back(portrait);
	}

// 	// FIXME Init the location picture
// 	_location_graphic.SetFilename("img/menus/locations/desert_cave.png");
// 	_location_graphic.SetDimensions(500, 125);
// 	_location_graphic.SetStatic(true);
// 	VideoManager->LoadImage(_location_graphic);
//
// 	//Init char select option box
	_InitCharSelect();
//
// 	// Load sounds
// 	SoundDescriptor confirm;
// 	SoundDescriptor bump;
// 	SoundDescriptor potion;
// 	SoundDescriptor cancel;
// 	if (confirm.LoadSound("snd/obtain.wav") == false)
// 	{
// 		cerr << "INVENTORYWINDOW::UPDATE - Unable to load confirm sound effect!" << endl;
// 	}
// 	if (bump.LoadSound("snd/bump.wav") == false)
// 	{
// 		cerr << "INVENTORYWINDOW::UPDATE - Unable to load bump sound effect!" << endl;
// 	}
// 	if (potion.LoadSound("snd/potion_drink.wav") == false)
// 	{
// 		cerr << "INVENTORYWINDOW::UPDATE - Unable to load potion drink sound effect!" << endl;
// 	}
// 	if (cancel.LoadSound("snd/cancel.wav") == false)
// 	{
// 		cerr << "INVENTORYWINDOW::UPDATE - Unable to load cancel sound effect!" << endl;
// 	}
// 	_menu_sounds["confirm"] = confirm;
// 	_menu_sounds["bump"] = bump;
// 	_menu_sounds["potion"] = potion;
// 	_menu_sounds["cancel"] = cancel;

	_current_char = GlobalManager->GetCharacter(GLOBAL_CHARACTER_CLAUDIUS);
} // StatusWindow::StatusWindow()



StatusWindow::~StatusWindow()
{
	uint32 partysize = GlobalManager->GetActiveParty()->GetPartySize();

	for (uint32 i = 0; i < partysize; i++) {
		VideoManager->DeleteImage(_full_portraits[i]);
	}
//
// 	VideoManager->DeleteImage(_location_graphic);
//
// 	// Clear sounds
// 	_menu_sounds["confirm"].FreeSound();
// 	_menu_sounds["bump"].FreeSound();
// 	_menu_sounds["potion"].FreeSound();
// 	_menu_sounds["cancel"].FreeSound();
}

// Activate/deactivate window
void StatusWindow::Activate(bool new_value) {
	_char_select_active = new_value;

	if (_char_select_active)
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	else
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}

void StatusWindow::_InitCharSelect() {
	//character selection set up
	vector<ustring> options;
	uint32 size = GlobalManager->GetActiveParty()->GetPartySize();

	_char_select.SetCursorOffset(-50.0f, -6.0f);
	_char_select.SetFont("default");
	_char_select.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_char_select.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_char_select.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_char_select.SetSize(1, ((size >= 4) ? 4 : size));
	_char_select.SetCellSize(360, 108);
	_char_select.SetPosition(72.0f, 109.0f);

	// Use blank string so cursor can point somewhere
	for (uint32 i = 0; i < size; i++) {
		options.push_back(MakeUnicodeString(" "));
	}

	_char_select.SetOptions(options);
	_char_select.SetSelection(0);
	_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}

// Updates the status window
void StatusWindow::Update() {
	//FIX ME: Handle StatusWindow OptionBox

	// check input values
	if (InputManager->UpPress())
	{
		_char_select.HandleUpKey();
	}
	else if (InputManager->DownPress())
	{
		_char_select.HandleDownKey();
	}
	else if (InputManager->CancelPress())
	{
		_char_select.HandleCancelKey();
	}

	if (_char_select.GetEvent() == VIDEO_OPTION_CANCEL) {
		Activate(false);
		MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
	}

} // void StatusWindow::Update()


// Draws the status window
void StatusWindow::Draw() {
	MenuWindow::Draw();

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
	ostringstream ostr;
	ostr << "Strength: " << _current_char->GetStrength();
	VideoManager->DrawText(MakeUnicodeString(ostr.str()));

	VideoManager->MoveRelative(0, 25);
	ostringstream ovig;
	ovig << "Vigor: " << _current_char->GetVigor();
	VideoManager->DrawText(MakeUnicodeString(ovig.str()));

	VideoManager->MoveRelative(0, 25);
	ostringstream ofort;
	ofort << "XP to Next: " << _current_char->GetFortitude();
	VideoManager->DrawText(MakeUnicodeString(ofort.str()));

	VideoManager->MoveRelative(0, 25);
	ostringstream ores;
	ores << "Protection: " << _current_char->GetProtection();
	VideoManager->DrawText(MakeUnicodeString(ores.str()));

	VideoManager->MoveRelative(0, 25);
	ostringstream agl;
	agl << "Agility: " << _current_char->GetAgility();
	VideoManager->DrawText(MakeUnicodeString(agl.str()));

	VideoManager->MoveRelative(0, 25);
	ostringstream oeva;
	oeva << "Evade: " << _current_char->GetEvade() << "%";
	VideoManager->DrawText(MakeUnicodeString(oeva.str()));

	//Draw character full body portrait
	VideoManager->Move(735, 145);

	VideoManager->DrawImage(_full_portraits[0]);

	_char_select.Draw();
} // void StatusWindow::Draw()

////////////////////////////////////////////////////////////////////////////////
// SkillsWindow Class
////////////////////////////////////////////////////////////////////////////////

SkillsWindow::SkillsWindow() : _active_box(SKILL_ACTIVE_NONE) {
	// Init option boxes
	_InitCharSelect();
	_InitSkillsList();
	_InitSkillsCategories();
} // SkillsWindow::SkillsWindow()



void SkillsWindow::Activate(bool new_status) {
	// Activate window and first option box...or deactivate both
	if (new_status) {
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		_active_box = SKILL_ACTIVE_CHAR;
	}
	else {
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		_active_box = SKILL_ACTIVE_NONE;
	}
}



void SkillsWindow::_InitSkillsList() {
	// Set up the inventory option box
	_skills_list.SetCellSize(180.0f, 30.0f);
	_skills_list.SetPosition(500.0f, 170.0f);
	_skills_list.SetFont("default");
	_skills_list.SetCursorOffset(-52.0f, -20.0f);
	_skills_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_skills_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_skills_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);

	_UpdateSkillList();
	_skills_list.SetSelection(0);
	_skills_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}



void SkillsWindow::_InitCharSelect() {
	//character selection set up
	vector<ustring> options;
	uint32 size = GlobalManager->GetActiveParty()->GetPartySize();

	_char_select.SetCursorOffset(-50.0f, -6.0f);
	_char_select.SetFont("default");
	_char_select.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_char_select.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_char_select.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_char_select.SetSize(1, ((size >= 4) ? 4 : size));
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
}



void SkillsWindow::_InitSkillsCategories() {
	_skills_categories.SetCellSize(105.0f,30.0f);
	_skills_categories.SetPosition(510.0f, 120.0f);
	_skills_categories.SetFont("default");
	_skills_categories.SetSize(SKILL_CATEGORY_SIZE,1);
	_skills_categories.SetCursorOffset(-52.0f, -20.0f);
	_skills_categories.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_skills_categories.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_skills_categories.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);

	// Create options
	vector<ustring> options;
	options.push_back(MakeUnicodeString("All"));
	options.push_back(MakeUnicodeString("Field"));
	options.push_back(MakeUnicodeString("Battle"));

	// Set options and default selection
	_skills_categories.SetOptions(options);
	_skills_categories.SetSelection(SKILL_ALL);
	_skills_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
} // void SkillsWindow::InitSkillsCategories()



void SkillsWindow::Update() {
	//FIXME: Need support for skills
	return;

	OptionBox *active_option = NULL;

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
			// Handle skill application
			if (event == VIDEO_OPTION_CONFIRM) {
				//TODO Use Skill
				MenuMode::_instance->_menu_sounds["confirm"].PlaySound();
			}
			else if (event == VIDEO_OPTION_CANCEL) {
				_active_box = SKILL_ACTIVE_LIST;
				_skills_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
			}
			break;

		case SKILL_ACTIVE_CHAR:
			// Choose character for skillset
			if (event == VIDEO_OPTION_CONFIRM) {
				_active_box = SKILL_ACTIVE_CATEGORY;
				_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				_skills_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				_char_skillset = _char_select.GetSelection();
				MenuMode::_instance->_menu_sounds["confirm"].PlaySound();
			}
			else if (event == VIDEO_OPTION_CANCEL) {
				Activate(false);
				_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
			}
			break;

		case SKILL_ACTIVE_LIST:
			// Choose skill
			if (event == VIDEO_OPTION_CONFIRM) {
				_active_box = SKILL_ACTIVE_CHAR_APPLY;
				_skills_list.SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
				_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				MenuMode::_instance->_menu_sounds["confirm"].PlaySound();
			}
			else if (event == VIDEO_OPTION_CANCEL) {
				_active_box = SKILL_ACTIVE_CATEGORY;
				MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
				_skills_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				_skills_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			}
			break;

		case SKILL_ACTIVE_CATEGORY:
			// Choose skill type
			if (event == VIDEO_OPTION_CONFIRM) {
				_active_box = SKILL_ACTIVE_LIST;
				_skills_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				_skills_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				MenuMode::_instance->_menu_sounds["confirm"].PlaySound();
			}
			else if (event == VIDEO_OPTION_CANCEL) {
				_active_box = SKILL_ACTIVE_CHAR;
				MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
				_skills_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				_char_select.SetSelection(_char_skillset);
			}
			break;
	}

	_UpdateSkillList();
} // void SkillsWindow::Update()



void SkillsWindow::_UpdateSkillList() {
	//uint32 partysize = GlobalManager->GetActiveParty().GetPartySize();

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
	/*GlobalCharacterParty* party = GlobalManager->GetActiveParty();
	GlobalCharacter* ch = party->GetCharacters()[_char_select.GetSelection()];

	switch (_skills_categories.GetSelection()) {
		case SKILL_ALL:
			{*/

	_skills_list.SetSize(1,12);

	_skills_list.SetOptions(options);
}



void SkillsWindow::Draw() {
	MenuWindow::Draw();

	//Draw option boxes
	_char_select.Draw();
	_skills_categories.Draw();
	_skills_list.Draw();
}

////////////////////////////////////////////////////////////////////////////////
// EquipWindow Class
////////////////////////////////////////////////////////////////////////////////

EquipWindow::EquipWindow() : _active_box(EQUIP_ACTIVE_NONE) {
	// Initialize option boxes
	_InitCharSelect();
	_InitEquipmentSelect();
	_InitEquipmentList();

	StillImage i;
	GlobalActor* actor = GlobalManager->GetActiveParty()->GetActor(_char_select.GetSelection());
	GlobalCharacter* ch = (GlobalCharacter*)(actor);

	i.SetFilename(ch->GetWeaponEquipped()->GetIconImage().GetFilename());
	_equip_images.push_back(i);

	i.SetFilename(ch->GetEquippedHeadArmor()->GetIconImage().GetFilename());
	_equip_images.push_back(i);

	i.SetFilename(ch->GetEquippedTorsoArmor()->GetIconImage().GetFilename());
	_equip_images.push_back(i);

	i.SetFilename(ch->GetEquippedArmsArmor()->GetIconImage().GetFilename());
	_equip_images.push_back(i);

	i.SetFilename(ch->GetEquippedLegArmor()->GetIconImage().GetFilename());
	_equip_images.push_back(i);

	for (uint32 i = 0; i < EQUIP_CATEGORY_SIZE; i++) {
		_equip_images[i].SetDimensions(60, 60);
		VideoManager->LoadImage(_equip_images[i]);
	}

}



EquipWindow::~EquipWindow() {
	for (uint32 i = 0; i < EQUIP_CATEGORY_SIZE; i++) {
		VideoManager->DeleteImage(_equip_images[i]);
	}
}



void EquipWindow::Activate(bool new_status) {

	//Activate window and first option box...or deactivate both
	if (new_status) {
		_active_box = EQUIP_ACTIVE_CHAR;
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	}
	else {
		_active_box = EQUIP_ACTIVE_NONE;
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	}
}



void EquipWindow::_InitEquipmentList() {
	// Set up the inventory option box
	_equip_list.SetCellSize(180.0f, 30.0f);

	_equip_list.SetPosition(500.0f, 170.0f);
	_equip_list.SetFont("default");

	_equip_list.SetCursorOffset(-52.0f, -20.0f);
	_equip_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_equip_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_equip_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	// Update the equipment list
	_UpdateEquipList();
	_equip_list.SetSelection(0);
	// Initially hide the cursor
	_equip_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}



void EquipWindow::_InitCharSelect() {
	//character selection set up
	vector<ustring> options;
	uint32 size = GlobalManager->GetActiveParty()->GetPartySize();

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



void EquipWindow::_InitEquipmentSelect() {
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
	_UpdateEquipList();
	_equip_select.SetSelection(EQUIP_WEAPON);


} // void EquipWindow::_InitEquipmentSelect()



void EquipWindow::Update() {

	// Points to the active option box
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
					MenuMode::_instance->_menu_sounds["confirm"].PlaySound();
				}
				else if (event == VIDEO_OPTION_CANCEL) {
					Activate(false);
					MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
				}
			}
			break;

		//Choose equipment to replace
		case EQUIP_ACTIVE_SELECT:
			{
				if (event == VIDEO_OPTION_CONFIRM) {
					_active_box = EQUIP_ACTIVE_LIST;
					_equip_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_equip_list.SetSelection(0);
					_equip_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					MenuMode::_instance->_menu_sounds["confirm"].PlaySound();
				}
				else if (event == VIDEO_OPTION_CANCEL) {
					_active_box = EQUIP_ACTIVE_CHAR;
					_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					_equip_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
				}
			}
			break;

		//Choose replacement
		case EQUIP_ACTIVE_LIST:
			{
				if (event == VIDEO_OPTION_CONFIRM) {
					GlobalActor* actor = GlobalManager->GetActiveParty()->GetActor(_char_select.GetSelection());
					GlobalCharacter* ch = (GlobalCharacter*)(actor);

					if (_equip_select.GetSelection() == EQUIP_WEAPON) {
						GlobalManager->AddToInventory(ch->GetWeaponEquipped()->GetID());
						ch->EquipWeapon(GlobalManager->GetInventoryWeapons()->at(_equip_list.GetSelection()));
						GlobalManager->DecrementObjectCount(ch->GetWeaponEquipped()->GetID(), 1);
					}
					else if (_equip_select.GetSelection() == EQUIP_HEADGEAR) {
						GlobalManager->AddToInventory(ch->GetEquippedHeadArmor()->GetID());ch->EquipArmor(GlobalManager->GetInventoryHeadArmor()->at(_equip_list.GetSelection()));
						GlobalManager->DecrementObjectCount(ch->GetEquippedHeadArmor()->GetID(), 1);
					}
					else if (_equip_select.GetSelection() == EQUIP_BODYARMOR) {
						GlobalManager->AddToInventory(ch->GetEquippedTorsoArmor()->GetID());ch->EquipArmor(GlobalManager->GetInventoryTorsoArmor()->at(_equip_list.GetSelection()));
						GlobalManager->DecrementObjectCount(ch->GetEquippedTorsoArmor()->GetID(), 1);
					}
					else if (_equip_select.GetSelection() == EQUIP_OFFHAND) {
						GlobalManager->AddToInventory(ch->GetEquippedArmsArmor()->GetID());ch->EquipArmor(GlobalManager->GetInventoryArmArmor()->at(_equip_list.GetSelection()));
						GlobalManager->DecrementObjectCount(ch->GetEquippedArmsArmor()->GetID(), 1);
					}
					else if (_equip_select.GetSelection() == EQUIP_LEGGINGS) {
						GlobalManager->AddToInventory(ch->GetEquippedLegArmor()->GetID());ch->EquipArmor(GlobalManager->GetInventoryLegArmor()->at(_equip_list.GetSelection()));
						GlobalManager->DecrementObjectCount(ch->GetEquippedLegArmor()->GetID(), 1);
					}
					else {
						cerr << "This shouldn't happen!" << endl;
					}

					_active_box = EQUIP_ACTIVE_SELECT;
					_equip_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_equip_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					MenuMode::_instance->_menu_sounds["confirm"].PlaySound();
				}
				else if (event == VIDEO_OPTION_CANCEL) {
					_active_box = EQUIP_ACTIVE_SELECT;
					MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
					_equip_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_equip_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				}
			}
			break;
	}

	_UpdateEquipList();
} // void EquipWindow::Update()



void EquipWindow::_UpdateEquipList() {
	GlobalActor* actor = GlobalManager->GetActiveParty()->GetActor(_char_select.GetSelection());
	GlobalCharacter* ch = (GlobalCharacter*)(actor);
	std::vector<ustring> options;

	if (_active_box == EQUIP_ACTIVE_LIST) {
		uint32 gearsize = 0;
		vector<hoa_global::GlobalWeapon*> weapons;
		vector<hoa_global::GlobalArmor*> armor;

		switch (_equip_select.GetSelection()) {
			case EQUIP_WEAPON:
				gearsize = GlobalManager->GetInventoryWeapons()->size();

				for (uint32 j = 0; j < gearsize; j++) {
					options.push_back(GlobalManager->GetInventoryWeapons()->at(j)->GetName());
				}

				break;

			case EQUIP_HEADGEAR:
				gearsize = GlobalManager->GetInventoryHeadArmor()->size();

				for (uint32 j = 0; j < gearsize; j++) {
					options.push_back(GlobalManager->GetInventoryHeadArmor()->at(j)->GetName());
				}

				break;

			case EQUIP_BODYARMOR:
				gearsize = GlobalManager->GetInventoryTorsoArmor()->size();

				for (uint32 j = 0; j < gearsize; j++) {
					options.push_back(GlobalManager->GetInventoryTorsoArmor()->at(j)->GetName());
				}

				break;

			case EQUIP_OFFHAND:
				gearsize = GlobalManager->GetInventoryArmArmor()->size();

				for (uint32 j = 0; j < gearsize; j++) {
					options.push_back(GlobalManager->GetInventoryArmArmor()->at(j)->GetName());
				}

				break;

			case EQUIP_LEGGINGS:
				gearsize = GlobalManager->GetInventoryLegArmor()->size();

				for (uint32 j = 0; j < gearsize; j++) {
					options.push_back(GlobalManager->GetInventoryLegArmor()->at(j)->GetName());
				}

				break;
		} // switch
		_equip_list.SetSize(1, gearsize);
		_equip_list.SetOptions(options);
	} // if EQUIP_ACTIVE_LIST

	else {
		// First, update the IMAGES of the equipped items
		_equip_images.clear();
		StillImage i;

		i.SetFilename(ch->GetWeaponEquipped()->GetIconImage().GetFilename());
		_equip_images.push_back(i);

		i.SetFilename(ch->GetEquippedHeadArmor()->GetIconImage().GetFilename());
		_equip_images.push_back(i);

		i.SetFilename(ch->GetEquippedTorsoArmor()->GetIconImage().GetFilename());
		_equip_images.push_back(i);

		i.SetFilename(ch->GetEquippedArmsArmor()->GetIconImage().GetFilename());
		_equip_images.push_back(i);

		i.SetFilename(ch->GetEquippedLegArmor()->GetIconImage().GetFilename());
		_equip_images.push_back(i);

		for (uint32 i = 0; i < EQUIP_CATEGORY_SIZE; i++) {
			_equip_images[i].SetDimensions(60, 60);
			VideoManager->LoadImage(_equip_images[i]);
		}

		// Now, update the NAMES of the equipped items

		options.push_back(ch->GetWeaponEquipped()->GetName());
		options.push_back(ch->GetEquippedHeadArmor()->GetName());
		options.push_back(ch->GetEquippedTorsoArmor()->GetName());
		options.push_back(ch->GetEquippedArmsArmor()->GetName());
		options.push_back(ch->GetEquippedLegArmor()->GetName());

		_equip_select.SetOptions(options);
	}

} // void EquipWindow::UpdateEquipList()



void EquipWindow::Draw() {
	MenuWindow::Draw();

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

} // void EquipWindow::Draw()


FormationWindow::FormationWindow() {
}

FormationWindow::~FormationWindow() {
}

void FormationWindow::Draw() {
	MenuWindow::Draw();
}

} // namespace private_menu

} // namespace hoa_menu
