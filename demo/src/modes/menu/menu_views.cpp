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

CharacterWindow::CharacterWindow() : _char_id(GLOBAL_CHARACTER_INVALID) {
}



CharacterWindow::~CharacterWindow() {
	// Delete the character's portrait
	VideoManager->DeleteImage(_portrait);
}



void CharacterWindow::SetCharacter(GlobalCharacter *character) {
	_char_id = character->GetID();

	_portrait.SetFilename("img/portraits/map/" + character->GetFilename() + ".png");
	_portrait.SetStatic(true);
	_portrait.SetDimensions(100, 100);
	VideoManager->LoadImage(_portrait);
} // void CharacterWindow::SetCharacter(GlobalCharacter *character)



// Draw the window to the screen
void CharacterWindow::Draw() {
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
// InventoryWindow Class
////////////////////////////////////////////////////////////////////////////////


InventoryWindow::InventoryWindow() : _active_box(ITEM_ACTIVE_NONE) {
	_InitCategory();
	_InitInventoryItems();
	_InitCharSelect();

	//Initializes the description textbox for the bottom window
	_description.SetOwner(this);
	_description.SetPosition(30.0f, 525.0f);
	_description.SetDimensions(800.0f, 80.0f);
	_description.SetDisplaySpeed(30);
	_description.SetFont("default");
	_description.SetDisplayMode(VIDEO_TEXT_INSTANT);
	_description.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);

} // void InventoryWindow::InventoryWindow

InventoryWindow::~InventoryWindow()
{
}

//Initializes the list of items
void InventoryWindow::_InitInventoryItems() {
	// Set up the inventory option box
	_inventory_items.SetCellSize(400.0f, 60.0f);

	_inventory_items.SetPosition(500.0f, 170.0f);
	_inventory_items.SetFont("default");
	_inventory_items.SetCursorOffset(-52.0f, -20.0f);
	_inventory_items.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_inventory_items.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_inventory_items.SetSize( 1, 6 );
	_inventory_items.Scissoring( true, false );
	//_inventory_items.TEMP_OverideScissorring(true);

	// Update the item text
	_UpdateItemText();
	if (_inventory_items.GetNumberOptions() > 0) {
		_inventory_items.SetSelection(0);
	}
	VideoManager->MoveRelative(-65, 20);
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
	_item_categories.SetCellSize(56.0f,30.0f);
	_item_categories.SetPosition(458.0f, 120.0f);
	_item_categories.SetFont("default");
	_item_categories.SetSize(ITEM_CATEGORY_SIZE,1);

	_item_categories.SetCursorOffset(-52.0f, -20.0f);
	_item_categories.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_item_categories.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_item_categories.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);

	vector<ustring> options;
	options.push_back(MakeUnicodeString("All"));
	options.push_back(MakeUnicodeString("Itm"));
	options.push_back(MakeUnicodeString("Wpn"));
	options.push_back(MakeUnicodeString("Hlm"));
	options.push_back(MakeUnicodeString("Tor"));
	options.push_back(MakeUnicodeString("Arm"));
	options.push_back(MakeUnicodeString("Leg"));
	options.push_back(MakeUnicodeString("Key"));

	_item_categories.SetOptions(options);
	_item_categories.SetSelection(ITEM_ALL);
	_item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}

// Activates/deactivates inventory window
void InventoryWindow::Activate(bool new_status) {
	// Set new status
	if (_inventory_items.GetNumberOptions() > 0 && new_status) {
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
	active_option->Update();
	// Handle confirm/cancel presses differently for each window
	switch (_active_box) {
		case ITEM_ACTIVE_NONE:
			break;

		case ITEM_ACTIVE_CATEGORY:
		{
			// Activate the item list for this category
			if (event == VIDEO_OPTION_CONFIRM) {
				if (_inventory_items.GetNumberOptions() > 0) {
					_inventory_items.SetSelection(0);
					_item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					_description.SetDisplayText( _item_objects[ 0 ]->GetDescription() );
					_active_box = ITEM_ACTIVE_LIST;
					MenuMode::_instance->_menu_sounds["confirm"].PlaySound();
				} // if _inventory_items.GetNumberOptions() > 0
			} // if VIDEO_OPTION_CONFIRM
			// Deactivate inventory
			else if (event == VIDEO_OPTION_CANCEL) {
				MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
				_item_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				Activate(false);
			} // if VIDEO_OPTION_CANCEL
			break;
		} // case ITEM_ACTIVE_CATEGORY

		case ITEM_ACTIVE_LIST:
		{
			// Activate the character select for application
			if (event == VIDEO_OPTION_CONFIRM) {
				_active_box = ITEM_ACTIVE_CHAR;
				_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
				_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				MenuMode::_instance->_menu_sounds["confirm"].PlaySound();
			} // if VIDEO_OPTION_CONFIRM
			// Return to category selection
			else if (event == VIDEO_OPTION_CANCEL) {
				_active_box = ITEM_ACTIVE_CATEGORY;
				_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				_item_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
			} // else if VIDEO_OPTION_CANCEL
			else if ( event == VIDEO_OPTION_BOUNDS_UP || VIDEO_OPTION_BOUNDS_DOWN ) {
				_description.SetDisplayText( _item_objects[ _inventory_items.GetSelection() ]->GetDescription() );
			} // else if VIDEO_OPTION_BOUNDS_UP
			break;
		} // case ITEM_ACTIVE_LIST

		case ITEM_ACTIVE_CHAR:
		{
			// Use the item on the chosen character
			if (event == VIDEO_OPTION_CONFIRM) {
				GlobalObject* obj = _item_objects[ _inventory_items.GetSelection() ];
				GlobalCharacter *ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_select.GetSelection()));
				if (obj->GetObjectType() == GLOBAL_OBJECT_ITEM) {
					GlobalItem *item = (GlobalItem*)GlobalManager->RetrieveFromInventory(obj->GetID());
					item->MenuUse(ch);
				}
			} // if VIDEO_OPTION_CONFIRM
			// Return to item selection
			else if (event == VIDEO_OPTION_CANCEL) {
				_active_box = ITEM_ACTIVE_LIST;
				_inventory_items.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
			} // if VIDEO_OPTION_CANCEL
			break;
		} // case ITEM_ACTIVE_CHAR
	} // switch (_active_box)

	// Update the item list
	_UpdateItemText();
} // void InventoryWindow::Update()

// Updates the item list
void InventoryWindow::_UpdateItemText() {
	_item_objects.clear();
	_inventory_items.ClearOptions();

	switch (_item_categories.GetSelection()) {
		case ITEM_ALL:
		{
			std::map<uint32, GlobalObject*>* inv = GlobalManager->GetInventory();
			for (std::map<uint32, GlobalObject*>::iterator i = inv->begin(); i != inv->end(); i++) {
				_item_objects.push_back( i->second );
			}
		}
			break;

		case ITEM_ITEM:
			_item_objects = _GetItemVector(GlobalManager->GetInventoryItems());
			break;

		case ITEM_WEAPONS:
			_item_objects = _GetItemVector(GlobalManager->GetInventoryWeapons());
			break;

		case ITEM_HEAD_ARMOR:
			_item_objects = _GetItemVector(GlobalManager->GetInventoryHeadArmor());
			break;

		case ITEM_TORSO_ARMOR:
			_item_objects = _GetItemVector(GlobalManager->GetInventoryTorsoArmor());
			break;

		case ITEM_ARM_ARMOR:
			_item_objects = _GetItemVector(GlobalManager->GetInventoryArmArmor());
			break;

		case ITEM_LEG_ARMOR:
			_item_objects = _GetItemVector(GlobalManager->GetInventoryLegArmor());
			break;

		case ITEM_KEY:
			_item_objects = _GetItemVector(GlobalManager->GetInventoryKeyItems());
			break;
	}

	ustring text;
	std::vector<ustring> inv_names;

	for (size_t ctr = 0; ctr < _item_objects.size(); ctr++) {
		text = MakeUnicodeString("<" + _item_objects[ctr]->GetIconImage().GetFilename() + "><32>     ") + _item_objects[ctr]->GetName() + MakeUnicodeString("<R><350>" + NumberToString(_item_objects[ctr]->GetCount()) + "   ");
		inv_names.push_back(text);
	}

	_inventory_items.SetOptions(inv_names);
} // void InventoryWindow::UpdateItemText()



void InventoryWindow::Draw() {
	MenuWindow::Draw();

	// Update the item text in case the number of items changed.
	_UpdateItemText();

	// Draw char select option box
	_char_select.Draw();

	// Draw item categories option box
	_item_categories.Draw();

	// Draw item list
	_inventory_items.Draw();
} // bool InventoryWindow::Draw()


////////////////////////////////////////////////////////////////////////////////
// StatusWindow Class
////////////////////////////////////////////////////////////////////////////////

StatusWindow::StatusWindow() : _char_select_active(false) {
	// Get party size for iteration
	uint32 partysize = GlobalManager->GetActiveParty()->GetPartySize();
	StillImage portrait;
	GlobalCharacter* ch;

	// Set up the full body portrait
	for (uint32 i = 0; i < partysize; i++) {
		ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(i));
		portrait.SetFilename("img/portraits/menu/" + ch->GetFilename() + "_large.png");
		portrait.SetStatic(true);
		portrait.SetDimensions(150, 350);
		VideoManager->LoadImage(portrait);
		_full_portraits.push_back(portrait);
	}

	// Init char select option box
	_InitCharSelect();
} // StatusWindow::StatusWindow()



StatusWindow::~StatusWindow() {
	uint32 partysize = GlobalManager->GetActiveParty()->GetPartySize();

	for (uint32 i = 0; i < partysize; i++) {
		VideoManager->DeleteImage(_full_portraits[i]);
	}
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
	_char_select.Update();
} // void StatusWindow::Update()


// Draws the status window
void StatusWindow::Draw() {
	MenuWindow::Draw();

	GlobalCharacter* ch =  dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_select.GetSelection()));

	// Set drawing system
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);

	// window top corner is 432, 99
	VideoManager->Move(565, 130);

	//Draw character name and level
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, 0);
	VideoManager->DrawText(ch->GetName());

	VideoManager->MoveRelative(0, 25);
	ostringstream lvl;
	lvl << "Experience Level: " << ch->GetExperienceLevel();
	VideoManager->DrawText(MakeUnicodeString(lvl.str()));

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, 0);

	//Draw all character stats
	VideoManager->MoveRelative(-55, 60);

	ostringstream ohp;
	ohp << "HP: " << ch->GetHitPoints() << " (" << ch->GetMaxHitPoints() << ")";
	VideoManager->DrawText(MakeUnicodeString(ohp.str()));

	VideoManager->MoveRelative(0, 25);
	ostringstream osp;
	osp << "SP: " << ch->GetSkillPoints() << " (" << ch->GetMaxSkillPoints() << ")";
	VideoManager->DrawText(MakeUnicodeString(osp.str()));

	VideoManager->MoveRelative(0, 25);
	ostringstream next;
	next << "XP to Next: " << ch->GetExperienceForNextLevel();
	VideoManager->DrawText(MakeUnicodeString(next.str()));

	VideoManager->MoveRelative(0, 25);
	ostringstream ostr;
	ostr << "Strength: " << ch->GetStrength();
	VideoManager->DrawText(MakeUnicodeString(ostr.str()));

	VideoManager->MoveRelative(0, 25);
	ostringstream ovig;
	ovig << "Vigor: " << ch->GetVigor();
	VideoManager->DrawText(MakeUnicodeString(ovig.str()));

	VideoManager->MoveRelative(0, 25);
	ostringstream ofort;
	ofort << "XP to Next: " << ch->GetFortitude();
	VideoManager->DrawText(MakeUnicodeString(ofort.str()));

	VideoManager->MoveRelative(0, 25);
	ostringstream ores;
	ores << "Protection: " << ch->GetProtection();
	VideoManager->DrawText(MakeUnicodeString(ores.str()));

	VideoManager->MoveRelative(0, 25);
	ostringstream agl;
	agl << "Agility: " << ch->GetAgility();
	VideoManager->DrawText(MakeUnicodeString(agl.str()));

	VideoManager->MoveRelative(0, 25);
	ostringstream oeva;
	oeva << "Evade: " << ch->GetEvade() << "%";
	VideoManager->DrawText(MakeUnicodeString(oeva.str()));

	//Draw character full body portrait
	VideoManager->Move(735, 145);

	VideoManager->DrawImage(_full_portraits[_char_select.GetSelection()]);

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

	_description.SetOwner(this);
	_description.SetPosition(30.0f, 525.0f);
	_description.SetDimensions(800.0f, 80.0f);
	_description.SetDisplaySpeed(30);
	_description.SetFont("default");
	_description.SetDisplayMode(VIDEO_TEXT_INSTANT);
	_description.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);

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
	active_option->Update();
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
/*				_active_box = SKILL_ACTIVE_CHAR_APPLY;
				_skills_list.SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
				_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				MenuMode::_instance->_menu_sounds["confirm"].PlaySound();*/
				MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
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
				_skills_list.SetSelection(0);
				if (_skills_list.GetNumberOptions() > 0) {
					_active_box = SKILL_ACTIVE_LIST;
					_skills_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_skills_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					MenuMode::_instance->_menu_sounds["confirm"].PlaySound();
				}
				else {
					MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
				}
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

	if (_skills_list.GetNumberOptions() > 0 &&
	    _skills_list.GetSelection() >= 0 &&
	    _skills_list.GetNumberOptions() > _skills_list.GetSelection()) {
		GlobalCharacter* ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_select.GetSelection()));
		std::vector<hoa_global::GlobalSkill*>* skills = ch->GetAttackSkills();
		GlobalSkill* skill = skills->at(_skills_list.GetSelection());
		_description.SetDisplayText( skill->GetDescription() );
	}
} // void SkillsWindow::Update()



void SkillsWindow::_UpdateSkillList() {
	GlobalCharacter* ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_select.GetSelection()));
	std::vector<ustring> options;

	//FIX ME Need new categories
	std::vector<hoa_global::GlobalSkill*>* skills = ch->GetAttackSkills();
	uint32 skillsize = skills->size();

	string tempstr = "";

	switch (_skills_categories.GetSelection()) {
		case SKILL_ALL:
		case SKILL_BATTLE:
			_skills_list.SetSize(1,skillsize);

			for (uint32 i = 0; i < skillsize; i++) {
				tempstr = MakeStandardString(skills->at(i)->GetName()) + "		" + NumberToString(skills->at(i)->GetSPRequired()) + " SP";
				options.push_back(MakeUnicodeString(tempstr));
			}
		break;

		case SKILL_FIELD:
		default:
			_skills_list.SetSize(1,0);
	}

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
	GlobalCharacter* ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_select.GetSelection()));

	i.SetFilename(ch->GetWeaponEquipped()->GetIconImage().GetFilename());
	_equip_images.push_back(i);

	i.SetFilename(ch->GetHeadArmorEquipped()->GetIconImage().GetFilename());
	_equip_images.push_back(i);

	i.SetFilename(ch->GetTorsoArmorEquipped()->GetIconImage().GetFilename());
	_equip_images.push_back(i);

	i.SetFilename(ch->GetArmArmorEquipped()->GetIconImage().GetFilename());
	_equip_images.push_back(i);

	i.SetFilename(ch->GetLegArmorEquipped()->GetIconImage().GetFilename());
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
	if (_equip_list.GetNumberOptions() > 0) {
		_equip_list.SetSelection(0);
	}
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
	active_option->Update();
	switch (_active_box) {
		//Choose character
		case EQUIP_ACTIVE_CHAR:
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
		break;

		//Choose equipment to replace
		case EQUIP_ACTIVE_SELECT:
			if (event == VIDEO_OPTION_CONFIRM) {
				_active_box = EQUIP_ACTIVE_LIST;
				_UpdateEquipList();
				if (_equip_list.GetNumberOptions() > 0) {
					_equip_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
					_equip_list.SetSelection(0);
					_equip_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
					MenuMode::_instance->_menu_sounds["confirm"].PlaySound();
				}
				else {
					_active_box = EQUIP_ACTIVE_SELECT;
					MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
				}
			}
			else if (event == VIDEO_OPTION_CANCEL) {
				_active_box = EQUIP_ACTIVE_CHAR;
				_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				_equip_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
			}
		break;

		//Choose replacement
		case EQUIP_ACTIVE_LIST:
			if (event == VIDEO_OPTION_CONFIRM) {
				GlobalCharacter* ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_select.GetSelection()));
				uint32 id_num;

				switch ( _equip_select.GetSelection() ) {
					case EQUIP_WEAPON:
						id_num = GlobalManager->GetInventoryWeapons()->at(_equip_list.GetSelection())->GetID();
						GlobalManager->AddToInventory(ch->EquipWeapon((GlobalWeapon*)GlobalManager->RetrieveFromInventory(id_num)));
					break;

					case EQUIP_HEADGEAR:
						id_num = GlobalManager->GetInventoryHeadArmor()->at(_equip_list.GetSelection())->GetID();
						GlobalManager->AddToInventory(ch->EquipHeadArmor((GlobalArmor*)GlobalManager->RetrieveFromInventory(id_num)));
					break;

					case EQUIP_BODYARMOR:
						id_num = GlobalManager->GetInventoryTorsoArmor()->at(_equip_list.GetSelection())->GetID();
						GlobalManager->AddToInventory(ch->EquipTorsoArmor((GlobalArmor*)GlobalManager->RetrieveFromInventory(id_num)));
					break;

					case EQUIP_OFFHAND:
						id_num = GlobalManager->GetInventoryArmArmor()->at(_equip_list.GetSelection())->GetID();
						GlobalManager->AddToInventory(ch->EquipArmArmor((GlobalArmor*)GlobalManager->RetrieveFromInventory(id_num)));
					break;

					case EQUIP_LEGGINGS:
						id_num = GlobalManager->GetInventoryLegArmor()->at(_equip_list.GetSelection())->GetID();
						GlobalManager->AddToInventory(ch->EquipLegArmor((GlobalArmor*)GlobalManager->RetrieveFromInventory(id_num)));
					break;

					default:
						cout << "MENU ERROR: _equip_select.GetSelection value is invalid: " << _equip_select.GetSelection() << endl;
					break;
				} // switch _equip_select.GetSelection()

				_active_box = EQUIP_ACTIVE_SELECT;
				_equip_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				_equip_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				MenuMode::_instance->_menu_sounds["confirm"].PlaySound();
			} // if VIDEO_OPTION_CONFIRM
			else if (event == VIDEO_OPTION_CANCEL) {
				_active_box = EQUIP_ACTIVE_SELECT;
				MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
				_equip_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				_equip_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			} // else if VIDEO_OPTION_CANCEL
		break;
	} // switch _active_box

	_UpdateEquipList();
} // void EquipWindow::Update()



void EquipWindow::_UpdateEquipList() {
	GlobalCharacter* ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_select.GetSelection()));
	std::vector<ustring> options;

	if (_active_box == EQUIP_ACTIVE_LIST) {
		uint32 gearsize = 0;
//		vector<hoa_global::GlobalWeapon*> weapons;
//		vector<hoa_global::GlobalArmor*> armor;

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

		i.SetFilename(ch->GetHeadArmorEquipped()->GetIconImage().GetFilename());
		_equip_images.push_back(i);

		i.SetFilename(ch->GetTorsoArmorEquipped()->GetIconImage().GetFilename());
		_equip_images.push_back(i);

		i.SetFilename(ch->GetArmArmorEquipped()->GetIconImage().GetFilename());
		_equip_images.push_back(i);

		i.SetFilename(ch->GetLegArmorEquipped()->GetIconImage().GetFilename());
		_equip_images.push_back(i);

		for (uint32 i = 0; i < EQUIP_CATEGORY_SIZE; i++) {
			_equip_images[i].SetDimensions(60, 60);
			VideoManager->LoadImage(_equip_images[i]);
		}

		// Now, update the NAMES of the equipped items

		options.push_back(ch->GetWeaponEquipped()->GetName());
		options.push_back(ch->GetHeadArmorEquipped()->GetName());
		options.push_back(ch->GetTorsoArmorEquipped()->GetName());
		options.push_back(ch->GetArmArmorEquipped()->GetName());
		options.push_back(ch->GetLegArmorEquipped()->GetName());

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


FormationWindow::FormationWindow() : _active_box(FORM_ACTIVE_NONE) {
	_InitCharSelect();
}


FormationWindow::~FormationWindow() {
}


void FormationWindow::_InitCharSelect() {
	//character selection set up
	std::vector<ustring> options;
	uint32 size = GlobalManager->GetActiveParty()->GetPartySize();

	_char_select.SetCursorOffset(-50.0f, -6.0f);
	_char_select.SetFont("default");
	_char_select.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_char_select.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_char_select.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_char_select.SetSize(1, ((size >= 4) ? 4 : size));
	_char_select.SetCellSize(360, 108);
	_char_select.SetPosition(72.0f, 109.0f);

	_second_char_select.SetCursorOffset(-50.0f, -6.0f);
	_second_char_select.SetFont("default");
	_second_char_select.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
	_second_char_select.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_second_char_select.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_second_char_select.SetSize(1, ((size >= 4) ? 4 : size));
	_second_char_select.SetCellSize(360, 108);
	_second_char_select.SetPosition(72.0f, 109.0f);

	// Use blank string so cursor can point somewhere
	for (uint32 i = 0; i < size; i++) {
		options.push_back(MakeUnicodeString(" "));
	}

	_char_select.SetOptions(options);
	_char_select.SetSelection(0);
	_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

	_second_char_select.SetOptions(options);
	_second_char_select.SetSelection(0);
	_second_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

}


void FormationWindow::Update() {
	// Points to the active option box
	OptionBox *active_option = NULL;

	//choose correct menu
	switch (_active_box) {
		case FORM_ACTIVE_CHAR:
			active_option = &_char_select;
			break;
		case FORM_ACTIVE_SECOND:
			active_option = &_second_char_select;
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
	active_option->Update();

	switch (_active_box) {
		case FORM_ACTIVE_CHAR:
			if (event == VIDEO_OPTION_CONFIRM) {
				_active_box = FORM_ACTIVE_SECOND;
				_char_select.SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
				_second_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				MenuMode::_instance->_menu_sounds["confirm"].PlaySound();
			}
			else if (event == VIDEO_OPTION_CANCEL) {
				Activate(false);
				MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
			}
			break;

		case FORM_ACTIVE_SECOND:
			if (event == VIDEO_OPTION_CONFIRM) {
				// TODO: Implement Character Switch
				_active_box = FORM_ACTIVE_CHAR;
				_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				_second_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			}
			else if (event == VIDEO_OPTION_CANCEL) {
				_active_box = FORM_ACTIVE_CHAR;
				_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				_second_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				MenuMode::_instance->_menu_sounds["cancel"].PlaySound();
			}
			break;
	} // switch
	_char_select.Update();
}


void FormationWindow::Draw() {
	MenuWindow::Draw();
	_char_select.Draw();
	_second_char_select.Draw();
}


void FormationWindow::Activate(bool new_status) {
	if (new_status) {
		_active_box = FORM_ACTIVE_CHAR;
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	}
	else {
		_active_box = FORM_ACTIVE_NONE;
		_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		_second_char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	}
}

} // namespace private_menu

} // namespace hoa_menu
