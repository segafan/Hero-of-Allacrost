////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_windows.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for battle menu windows
*** ***************************************************************************/

#include "utils.h"
#include "defs.h"

#include "audio.h"
#include "video.h"
#include "input.h"
#include "mode_manager.h"
#include "system.h"
#include "global.h"

#include "battle.h"
#include "battle_actors.h"
#include "battle_windows.h"

using namespace std;

using namespace hoa_utils;

using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_input;
using namespace hoa_mode_manager;
using namespace hoa_system;
using namespace hoa_global;

namespace hoa_battle {

namespace private_battle {

// *****************************************************************************
// ActionWindow class
// *****************************************************************************

ActionWindow::ActionWindow() {
	//CD: Like FinishWindow, we should move all this to Initialize()

	// TODO: declare the MenuSkin to be used
	if (MenuWindow::Create(512.0f, 128.0f) == false) {
		cerr << "BATTLE ERROR: In ActionWindow constructor, the call to MenuWindow::Create() failed" << endl;
	}
	MenuWindow::SetPosition(512.0f, 128.0f);
	MenuWindow::SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);	

	// Setup options for VIEW_ACTION_CATEGORY
	_InitActionCategoryList();

	// Setup options for VIEW_ACTION_SELECTION
	_InitActionSelectionList();

	// Setup options for VIEW_TARGET_SELECTION and VIEW_ACTION_INFORMATION
	_InitSelectionHeaders();

	// Setup rendered text
	_InitInformationText();

	Reset();
} // ActionWindow::ActionWindow()


ActionWindow::~ActionWindow() {
	MenuWindow::Destroy();
}

void ActionWindow::_InitActionCategoryList()
{
	// NOTE: may need to set the dimensions of these images to 45, 45
	_action_category_icons.resize(4);
	bool success = true;
	success &= _action_category_icons[0].Load("img/icons/battle/attack.png");
	success &= _action_category_icons[1].Load("img/icons/battle/defend.png");
	success &= _action_category_icons[2].Load("img/icons/battle/support.png");
	success &= _action_category_icons[3].Load("img/icons/battle/item.png");
	if (success == false) {
		cerr << "BATTLE ERROR: In ActionWindow constructor, failed to load an action category icon" << endl;
	}

	vector<ustring> category_options;
	category_options.push_back(MakeUnicodeString("<img/icons/battle/attack.png>\nAttack"));
	category_options.push_back(MakeUnicodeString("<img/icons/battle/defend.png>\nDefend"));
	category_options.push_back(MakeUnicodeString("<img/icons/battle/support.png>\nSupport"));
	category_options.push_back(MakeUnicodeString("<img/icons/battle/item.png>\nItem"));

	_action_category_list.SetOptions(category_options);
	_action_category_list.SetPosition(50.0f, 120.0f);
	_action_category_list.SetCursorOffset(-20.0f, 25.0f);
	_action_category_list.SetCellSize(100.0f, 100.0f);
	_action_category_list.SetSize(4, 1);
	_action_category_list.SetFont("battle");
	_action_category_list.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_action_category_list.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_action_category_list.SetSelectMode(VIDEO_SELECT_SINGLE);
	_action_category_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_action_category_list.SetSelection(0);
	_action_category_list.SetOwner(this);
}


void ActionWindow::_InitActionSelectionList()
{
	_action_selection_list.SetPosition(128.0f, 120.0f);
	_action_selection_list.SetCursorOffset(-50.0f, 25.0f);
	_action_selection_list.SetCellSize(300.0f, 35.0f);
	_action_selection_list.SetFont("battle");
	_action_selection_list.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_action_selection_list.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_action_selection_list.SetSelectMode(VIDEO_SELECT_SINGLE);
	_action_selection_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_action_selection_list.SetOwner(this);
}
	

void ActionWindow::_InitSelectionHeaders()
{
	TextStyle battle_style("battle", Color(1.0f, 1.0f, 0.0f, 0.8f));

	_skill_selection_header.SetStyle(battle_style);
	_item_selection_header.SetStyle(battle_style);
	_skill_selection_header.SetText("Skill                                                  SP");
	_item_selection_header.SetText("Item                                                  Qty");
}


void ActionWindow::_InitInformationText()
{
	TextStyle battle_style("battle", Color::white);

	_action_information.SetStyle(battle_style);

	_target_information.SetStyle(battle_style);
}

void ActionWindow::Initialize(BattleCharacter* character) {
	_character = character;
	if (character == NULL) {
		if (BATTLE_DEBUG)
			cerr << "BATTLE WARNING: In ActionWindow::Initialize(), a NULL character pointer was passed" << endl;
		_state = VIEW_INVALID;
		return;
	}

	MenuWindow::Show();
	_state = VIEW_ACTION_CATEGORY;

	// Disable action categories which have no skills, or where the inventory is empty
	// Enable action categories which have skills. Enable inventory if player has items.
	_action_category_list.EnableOption(0, !_character->GetActor()->GetAttackSkills()->empty());
	_action_category_list.EnableOption(1, !_character->GetActor()->GetDefenseSkills()->empty());
	_action_category_list.EnableOption(2, !_character->GetActor()->GetSupportSkills()->empty());

	//We do this if in case someone is already queued to use the last item in our inventory
	_action_category_list.EnableOption(3, false);
	std::vector<GlobalItem*>* items = GlobalManager->GetInventoryItems();
	for (uint32 i = 0; i < items->size(); ++i)
	{
		if((*items)[i]->GetCount())
		{
			_action_category_list.EnableOption(3, true);
			break;
		}
	}

	_action_selection_list.ClearOptions();
} // void ActionWindow::Initialize(BattleCharacter* character)



void ActionWindow::Reset() {
	MenuWindow::Hide();
	_state = VIEW_INVALID;
	_character = NULL;
	_selected_action_category = 0;
	_action_category_list.SetSelection(0);
	_selected_action = 0;
	_action_target_type = GLOBAL_TARGET_INVALID;
	_action_target_ally = false;
	_item_list.clear();
	_skill_list = NULL;
}

// ----- UPDATE METHODS

void ActionWindow::Update() {
	MenuWindow::Update(SystemManager->GetUpdateTime());

	switch (_state) {
		case VIEW_ACTION_CATEGORY:
			_UpdateActionCategory();
			break;
		case VIEW_ACTION_SELECTION:
			_UpdateActionSelection();
			break;
		case VIEW_TARGET_SELECTION:
			_UpdateTargetSelection();
			break;
		case VIEW_ACTION_INFORMATION:
			_UpdateActionInformation();
			break;
		case VIEW_INVALID:
		case VIEW_TOTAL:
		default:
			if (BATTLE_DEBUG)
				cerr << "BATTLE ERROR: In ActionWindow::Update(), the window state was invalid: " << _state << endl;
			return;
	}
}



void ActionWindow::_UpdateActionCategory() {
	_action_category_list.Update(SystemManager->GetUpdateTime());

	if (InputManager->LeftPress()) {
		_action_category_list.HandleLeftKey();
	}
	else if (InputManager->RightPress()) {
		_action_category_list.HandleRightKey();
	}
	
	else if (InputManager->ConfirmPress() ) {
		_action_category_list.HandleConfirmKey();
		if (_action_category_list.GetEvent() == VIDEO_OPTION_CONFIRM) {
			_selected_action_category = static_cast<uint32>(_action_category_list.GetSelection());
			_ConstructActionSelectionList();
			_state = VIEW_ACTION_SELECTION;
		}
		else {
			// TODO: Play a sound to indicate the selection was invalid
		}
	}
	// Pick the next charcter who is not the current character
	else if (InputManager->CancelPress())
	{
		Reset();
		current_battle->_ActivateNextCharacter();
	}
}



void ActionWindow::_UpdateActionSelection() {
	_action_selection_list.Update(SystemManager->GetUpdateTime());

	if (InputManager->UpPress()) {
		_action_selection_list.HandleUpKey();
	}
	if (InputManager->DownPress()) {
		_action_selection_list.HandleDownKey();
	}

	if (InputManager->ConfirmPress()) {
		// TODO: make a call to battle mode so it can select the appropriate target
		_selected_action = _action_selection_list.GetSelection();
		_state = VIEW_TARGET_SELECTION;

		// TODO: if the target of the action is an entire party, display a list of all enemies in the window when in the VIEW_TARGET_SELECTION state

		if (_selected_action_category == ACTION_TYPE_ATTACK ||
			_selected_action_category == ACTION_TYPE_DEFEND ||
			_selected_action_category == ACTION_TYPE_SUPPORT)
		{
			_action_target_type = _skill_list->at(_selected_action)->GetTargetType();
			_action_target_ally = _skill_list->at(_selected_action)->IsTargetAlly();
			current_battle->_SetInitialTarget();
		}

		else if (_selected_action_category == ACTION_TYPE_ITEM) {
			_action_target_type = _item_list[_selected_action]->GetTargetType();
			_action_target_ally = _item_list[_selected_action]->IsTargetAlly();
			current_battle->_SetInitialTarget();
		}

		else {
			if (BATTLE_DEBUG)
				cerr << "BATTLE WARNING: In ActionWindow::_UpdateActionSelection(), selected action category was invalid" << endl;
		}

		_ConstructTargetInformation();
	}
	else if (InputManager->MenuPress()) {
		_selected_action = _action_selection_list.GetSelection();
		_ConstructActionInformation();
		_state = VIEW_ACTION_INFORMATION;
	}
	else if (InputManager->CancelPress()) {
		_state = VIEW_ACTION_CATEGORY;
		_skill_list = NULL;
		_item_list.clear();
	}
} // void ActionWindow::_UpdateActionSelection()



void ActionWindow::_UpdateTargetSelection() {
	if (InputManager->CancelPress()) {
		_target_information.Clear();
		_state = VIEW_ACTION_SELECTION;
		current_battle->_selected_target = NULL;
		return;
	}
	else if (InputManager->ConfirmPress()) {
		vector<GlobalAttackPoint*>* attack_points = current_battle->_selected_target->GetActor()->GetAttackPoints();
		BattleAction* new_event;
		if (_selected_action_category != ACTION_TYPE_ITEM) {
			new_event = new SkillAction(current_battle->_selected_character, current_battle->_selected_target, GetSelectedSkill(),
				attack_points->at(current_battle->_selected_attack_point));
		}
		else {
			GlobalItem* item = GetSelectedItem();
			// NOTE: Don't know if decrementing the item count is the best approach to use here.
			// We decrement the count now so that if the next character wants to use items, they know
			// how many are available to use. If the current character uses the item, then the decrement stays.
			// If count == 0, then it's removed from inventory...if item is not used (i.e. battle ends before use),
			// it is incremented back.
			item->DecrementCount(1);
			new_event = new ItemAction(current_battle->_selected_character, current_battle->_selected_target, item,
				attack_points->at(current_battle->_selected_attack_point));
		}
		current_battle->AddBattleActionToQueue(new_event);
		current_battle->_selected_character->SetState(ACTOR_WARM_UP);

		current_battle->RemoveFromTurnQueue(current_battle->_selected_character);
		current_battle->_selected_target = NULL;
		current_battle->_selected_character = NULL;
		current_battle->_selected_character_index = current_battle->GetIndexOfNextIdleCharacter();
		current_battle->_selected_attack_point = 0;

		Reset();
	}

	// If the target is a party type, then ignore arrow key presses since they are not processed
	if (_action_target_type == GLOBAL_TARGET_PARTY) {
		return;
	}

	//CD NOTE: We've removed the _ConstructTargetInformation calls from here
	//because we need to do it every frame.  If we have a character selected
	//as our target for using something like a healing potion, and he gets hit,
	//we need to update his HP to reflect what it is after the hit.  With the
	//below logic, his info won't be refreshed until we change targets
	if (InputManager->UpPress() || InputManager->DownPress()) {
//		BattleActor* previous_target = current_battle->_selected_target;
		current_battle->_SelectNextTarget(InputManager->UpPress());
		/*if (previous_target != current_battle->_selected_target) {
			_ConstructTargetInformation();
		}*/
	}
	else if (InputManager->LeftPress() || InputManager->RightPress()
		&& _action_target_type == GLOBAL_TARGET_ATTACK_POINT) {
//		uint32 previous_ap = current_battle->_selected_attack_point;
		current_battle->_SelectNextAttackPoint(InputManager->RightPress());
		/*if (previous_ap != current_battle->_selected_attack_point) {
			_ConstructTargetInformation();
		}*/
	}

	_ConstructTargetInformation();
} // void ActionWindow::_UpdateTargetSelection()



void ActionWindow::_UpdateActionInformation() {
	if (InputManager->MenuPress() || InputManager->CancelPress()) {
		_action_information.Clear();
		_state = VIEW_ACTION_SELECTION;
	}
}

// ----- DRAW METHODS

void ActionWindow::Draw() {
	MenuWindow::Draw();

	switch (_state) {
		case VIEW_ACTION_CATEGORY:
			_DrawActionCategory();
			break;
		case VIEW_ACTION_SELECTION:
			_DrawActionSelection();
			break;
		case VIEW_TARGET_SELECTION:
			_DrawTargetSelection();
			break;
		case VIEW_ACTION_INFORMATION:
			_DrawActionInformation();
			break;
		case VIEW_INVALID:
		case VIEW_TOTAL:
		default:
			if (BATTLE_DEBUG)
				cerr << "BATTLE ERROR: In ActionWindow::Draw(), the window state was invalid: " << _state << endl;
			return;
	}
}



void ActionWindow::_DrawActionCategory() {
	_action_category_list.Draw();
}



void ActionWindow::_DrawActionSelection() {
	// Draw the selected action category and name
	VideoManager->Move(570.0f, 80.0f);
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	_action_category_icons[_selected_action_category].Draw();
	VideoManager->MoveRelative(0.0f, -40.0f);
	VideoManager->SetDrawFlags(VIDEO_Y_CENTER, 0);

	switch (_selected_action_category) {
		case ACTION_TYPE_ATTACK:
			VideoManager->Text()->Draw("Attack");
			break;
		case ACTION_TYPE_DEFEND:
			VideoManager->Text()->Draw("Defend");
			break;
		case ACTION_TYPE_SUPPORT:
			VideoManager->Text()->Draw("Support");
			break;
		case ACTION_TYPE_ITEM:
			VideoManager->Text()->Draw("Item");
			break;
		default:
			if (BATTLE_DEBUG)
				cerr << "BATTLE ERROR: In ActionWindow::_DrawActionSelection(), unknown action category was selected: "
					<< _selected_action_category << endl;
			return;
	}

	// Draw the action list header text
	VideoManager->Move(640.0f, 125.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, 0);
	VideoManager->Text()->SetDefaultTextColor(Color(1.0f, 1.0f, 0.0f, 0.8f)); // 80% translucent yellow text
	if (_selected_action_category != ACTION_TYPE_ITEM) {
		//_skill_selection_header.Draw();
		VideoManager->Text()->Draw(_skill_selection_header.GetString());
	}
	else {
		//_item_selection_header.Draw();
		VideoManager->Text()->Draw(_item_selection_header.GetString());
	}

	// Draw the list of actions
	_action_selection_list.Draw();
} // void ActionWindow::_DrawActionSelection()



void ActionWindow::_DrawTargetSelection() {
	VideoManager->Move(640.0f, 125.0f);
	VideoManager->Text()->SetDefaultTextColor(Color(1.0f, 1.0f, 0.0f, 0.8f)); // 80% translucent yellow text
	VideoManager->Text()->Draw(MakeUnicodeString("Target Information"));
	VideoManager->MoveRelative(120.0f, -30.0f);
	//_target_information.Draw();
	VideoManager->Text()->Draw(_target_information.GetString());
}



void ActionWindow::_DrawActionInformation() {
	VideoManager->Move(640.0f, 125.0f);
	VideoManager->Text()->SetDefaultTextColor(Color(1.0f, 1.0f, 0.0f, 0.8f)); // 80% translucent yellow text
	VideoManager->Text()->Draw(MakeUnicodeString("Action Information"));
	VideoManager->MoveRelative(120.0f, -30.0f);
	//_action_information.Draw();
	VideoManager->Text()->Draw(_action_information.GetString());
}

// ----- OTHER METHODS

void ActionWindow::_ConstructActionSelectionList() {
	if (_selected_action_category == ACTION_TYPE_ATTACK ||
		_selected_action_category == ACTION_TYPE_DEFEND ||
		_selected_action_category == ACTION_TYPE_SUPPORT)
	{
		// Setup the battle skill list
		if (_selected_action_category == ACTION_TYPE_ATTACK) {
			_skill_list = _character->GetActor()->GetAttackSkills();
		}
		else if (_selected_action_category == ACTION_TYPE_DEFEND) {
			_skill_list = _character->GetActor()->GetDefenseSkills();
		}
		else if (_selected_action_category == ACTION_TYPE_SUPPORT) {
			_skill_list = _character->GetActor()->GetSupportSkills();
		}
		
		if (_skill_list->empty()) {
			if (BATTLE_DEBUG)
				cerr << "BATTLE ERROR: In ActionWindow::ConstructActionSelectionList(), the character had no skills to list" << endl;
			return;
		}

		// Now use the skill_list to construction the text for each skill
		vector<ustring> skill_text;
		for (uint32 i = 0; i < _skill_list->size(); i++) {
			skill_text.push_back(MakeUnicodeString("<L>") + _skill_list->at(i)->GetName() + MakeUnicodeString("<R>") +
				MakeUnicodeString(NumberToString(_skill_list->at(i)->GetSPRequired())));
		}

		// Add the options to the list
		_action_selection_list.SetOptions(skill_text);
		_action_selection_list.SetSize(1, skill_text.size());
		_action_selection_list.SetSelection(0);

		// Disable any options for which the character does not have a sufficient amount of SP to execute
		for (uint32 i = 0; i < _skill_list->size(); i++) {
			if (_skill_list->at(i)->GetSPRequired() > _character->GetActor()->GetSkillPoints())
				_action_selection_list.EnableOption(i, false);
		}
		return;
	} // if (_action_category_selected == ACTION_TYPE_ATTACK || ...)

	if (_selected_action_category == ACTION_TYPE_ITEM) {
		// A temporary pointer to all the items to check
		vector<GlobalItem*>* temp_item_list;
		
		temp_item_list = GlobalManager->GetInventoryItems();
		if (temp_item_list->empty()) {
			if (BATTLE_DEBUG)
				cerr << "BATTLE ERROR: In ActionWindow::ConstructActionSelectionList(), there were no items in the inventory" << endl;
			return;
		}

		_item_list.clear();
		// Contains the text for the items as they will appear on the screen
		vector<ustring> items_text;
		GlobalItem *item = NULL;

		// Only add items to the list which have a count greater than zero and are usable in battle
		// NOTE: We check for GetCount() because if a character is preparing to use an item, we
		// we temporarily decrement the count. If the item doesn't get used (e.g. because the character died, etc.),
		// then we increment the item count back. So when this point in the code is reached, you could potentially have
		// items which are still part of the inventory, but are temporarily unavailable for selection until we
		// see if a another character gets to use an item or not.
 		for (uint32 i = 0; i < temp_item_list->size(); ++i) {
 			item = temp_item_list->at(i);

			if (item->GetUsage() >= GLOBAL_USE_BATTLE && item->GetCount() > 0) {
				_item_list.push_back(item);
 				items_text.push_back(MakeUnicodeString("<L>") + item->GetName()
 					+ MakeUnicodeString("<R>") + MakeUnicodeString(NumberToString(item->GetCount())));
			}
 		}

		// Calculate the number of rows, this is dividing by 6, and if there is a remainder > 0, add one more row for the remainder
 		_action_selection_list.SetSize(1, _item_list.size() / 6 + ((_item_list.size() % 6) > 0 ? 1 : 0));

		_action_selection_list.SetOptions(items_text);
		_action_selection_list.SetSize(1, items_text.size());
		_action_selection_list.SetSelection(0);
	} // if (_action_category_selected == ACTION_TYPE_ITEM)

	else {
		if (BATTLE_DEBUG)
			cerr << "BATTLE ERROR: In ActionWindow::ConstructActionSelectionList(), the action category selected was invalid" << endl;
	}
} // void ActionWindow::ConstructActionSelectionList()



void ActionWindow::_ConstructTargetInformation() {
	ustring target_text;
	_target_information.Clear();

	if (_action_target_type == GLOBAL_TARGET_ATTACK_POINT)
		current_battle->_selected_target->ConstructInformation(target_text, current_battle->_selected_attack_point);
	else if (_action_target_type == GLOBAL_TARGET_ACTOR)
		current_battle->_selected_target->ConstructInformation(target_text, -1);
	else {
		// TODO: construct a list of all characters or enemies depending upon the type
	}

	//VideoManager->Text()->SetDefaultTextColor(Color::white);
	_target_information.SetText(target_text);
} // void ActionWindow::_ConstructTargetInformation()



void ActionWindow::_ConstructActionInformation() {
	ustring action_text;
	_action_information.Clear();

	// TODO: need to do a more complete job of filling out action information
	if (_selected_action_category == ACTION_TYPE_ITEM) {
		action_text = MakeUnicodeString("Name: ") + GetSelectedItem()->GetName() +
			MakeUnicodeString("\nCurrent Quantity: " + NumberToString(GetSelectedItem()->GetCount())) +
			MakeUnicodeString("\nTarget Type: ") + MakeUnicodeString(GetTargetTypeText(GetSelectedItem()->GetTargetType(), GetSelectedItem()->IsTargetAlly()));
	}

	else/* if (_selected_action_category == ACTION_TYPE_ATTACK
			|| _selected_action_category == ACTION_TYPE_DEFEND
			|| _selected_action_category == ACTION_TYPE_SUPPORT)*/
	{
		// TODO: add warm-up and cool-down times (in seconds), and description
		action_text = MakeUnicodeString("Name: ") + GetSelectedSkill()->GetName() +
			MakeUnicodeString("\nSP Required: " + NumberToString(GetSelectedSkill()->GetSPRequired())) +
			MakeUnicodeString("\nTarget Type: ") + MakeUnicodeString(GetTargetTypeText(GetSelectedSkill()->GetTargetType(), GetSelectedSkill()->IsTargetAlly()));
	}

	//VideoManager->Text()->SetDefaultTextColor(Color::white);
	_action_information.SetText(action_text);
} // void ActionWindow::_ConstructActionInformation()

// /////////////////////////////////////////////////////////////////////////////
// FinishWindow class
// /////////////////////////////////////////////////////////////////////////////

FinishWindow::FinishWindow()
{
	//CD: We should really move all this to Initialize() instead

	// TODO: declare the MenuSkin to be used
	//Just like the ones in Menu Mode
	float start_x = (1024 - 800) / 2 + 144;
	float start_y = 768 - ((768 - 600) / 2 + 15);
	
	if (!MenuWindow::Create(480.0f, 560.0f))
		cerr << "BATTLE ERROR: In FinishWindow constructor, the call to MenuWindow::Create() failed" << endl;

	MenuWindow::SetPosition(start_x, start_y);

	for (int32 i = 0; i < 4; ++i)
	{
		for (int32 j = 0; j < 8; ++j)
			_growth_gained[i][j] = 0;
	}

	_state = FINISH_INVALID;

	//Create character windows
	_InitCharacterWindows(start_x, start_y);
	//Create items and xp & money window
	_InitSpoilsWindows(start_x, start_y);
	//Initalize victory text (but don't set the string yet)
	_InitVictoryText();
	//Retry, quit, etc.
	_InitLoseOptions();
}



FinishWindow::~FinishWindow() {
	_character_window[0].Destroy();
	_character_window[1].Destroy();
	_character_window[2].Destroy();
	_character_window[3].Destroy();

	_xp_and_money_window.Destroy();
	_items_window.Destroy();

	MenuWindow::Destroy();
}



void FinishWindow::Initialize(bool victory) {
	MenuWindow::Show();

	_victory_money = 0;
	_victory_xp = 0;
	_victory_items.clear();

	for (uint32 i = 0; i < current_battle->_character_actors.size(); i++) {
		_characters.push_back(dynamic_cast<GlobalCharacter*>(current_battle->_character_actors[i]->GetActor()));
		_character_growths.push_back(_characters[i]->GetGrowth());
		_char_portraits[i].Load("img/portraits/map/" + current_battle->_character_actors[i]->GetActor()->GetFilename() + ".png", 100.0f, 100.0f);
	}

	if (victory) {
		_state = FINISH_WIN_ANNOUNCE;
		_finish_outcome.SetDisplayText("The heroes are victorious!");
		_TallyXPMoneyAndItems();
	}
	else {
		_state = FINISH_LOSE_ANNOUNCE;
		_finish_outcome.SetDisplayText("The heroes have been defeated...");
	}
}

void FinishWindow::_InitCharacterWindows(float start_x, float start_y)
{
	_character_window[0].Create(480.0f, 140.0f, ~VIDEO_MENU_EDGE_BOTTOM, VIDEO_MENU_EDGE_BOTTOM);
	_character_window[0].SetPosition(start_x, start_y - 12.0f);
	_character_window[0].Show();

	_character_window[1].Create(480.0f, 140.0f, ~VIDEO_MENU_EDGE_BOTTOM, VIDEO_MENU_EDGE_BOTTOM);
	_character_window[1].SetPosition(start_x, start_y - 12.0f - 140.0f);
	_character_window[1].Show();

	_character_window[2].Create(480.0f, 140.0f, ~VIDEO_MENU_EDGE_BOTTOM, VIDEO_MENU_EDGE_BOTTOM);
	_character_window[2].SetPosition(start_x, start_y - 11.0f - 140.0f * 2.0f);
	_character_window[2].Show();

	_character_window[3].Create(480.0f, 140.0f, VIDEO_MENU_EDGE_ALL, ~VIDEO_MENU_EDGE_ALL);//~VIDEO_MENU_EDGE_BOTTOM, VIDEO_MENU_EDGE_BOTTOM);
	_character_window[3].SetPosition(start_x, start_y - 10.0f - 140.0f * 3.0f);
	_character_window[3].Show();
}


void FinishWindow::_InitSpoilsWindows(float start_x, float start_y)
{
	_xp_and_money_window.Create(480.0f, 72.0f, VIDEO_MENU_EDGE_ALL, ~VIDEO_MENU_EDGE_ALL);
	_xp_and_money_window.SetPosition(start_x, start_y + 50.0f);
	_xp_and_money_window.Show();

	_items_window.Create(480.0f, 560.0f, ~VIDEO_MENU_EDGE_TOP, VIDEO_MENU_EDGE_TOP);
	_items_window.SetPosition(start_x, start_y - 13.0f);
	_items_window.Show();
}


void FinishWindow::_InitLoseOptions()
{
	vector<ustring> lose_text;
	lose_text.push_back(MakeUnicodeString("Retry the battle"));
	lose_text.push_back(MakeUnicodeString("Load from last save point"));
	lose_text.push_back(MakeUnicodeString("Return to main menu"));
	lose_text.push_back(MakeUnicodeString("Exit the game"));
	_lose_options.SetOptions(lose_text);
	_lose_options.SetCellSize(128.0f, 50.0f);
	_lose_options.SetPosition(270.0f, 130.0f);
	_lose_options.SetSize(1, 4);
	_lose_options.SetFont("battle");
	_lose_options.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_lose_options.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_lose_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_lose_options.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_lose_options.SetCursorOffset(-60.0f, 25.0f);
	_lose_options.SetSelection(0);
	_lose_options.SetOwner(this);
	_lose_options.EnableOption(0, false);
}


void FinishWindow::_InitVictoryText()
{
	_finish_outcome.SetPosition(512, 0);
	_finish_outcome.SetDimensions(400, 100);
	_finish_outcome.SetDisplaySpeed(30);
	_finish_outcome.SetTextStyle(TextStyle());
	_finish_outcome.SetDisplayMode(VIDEO_TEXT_REVEAL);
	_finish_outcome.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
}

// ----- Tallies all the stuff we've won (xp, money, items)

void FinishWindow::_TallyXPMoneyAndItems()
{
	GlobalEnemy* ge;
	vector<GlobalObject*> objects;
	std::map<GlobalObject*, int32>::iterator iter;

	for (uint32 i = 0; i < current_battle->GetNumberOfEnemies(); ++i)
	{
		ge = current_battle->GetEnemyActorAt(i)->GetActor();
		_victory_money += ge->GetDrunesDropped();
		_victory_xp += ge->GetExperiencePoints();
		ge->DetermineDroppedObjects(objects);

		for (uint32 j = 0; j < objects.size(); ++j)
		{
			iter = _victory_items.find(objects[j]);
			if (iter != _victory_items.end())
			{
				iter->second++;
			}
			else
			{
				_victory_items.insert(make_pair(objects[j], 1));
			}
		}
	}

	uint32 num_alive_characters = 0;
	for (uint32 i = 0; i < current_battle->GetNumberOfCharacters(); ++i)
	{
		if (current_battle->GetPlayerCharacterAt(i)->IsAlive())
		{
			++num_alive_characters;
		}
	}
	_victory_xp /= num_alive_characters;
}

void FinishWindow::_ClearLearnedSkills()
{
	for (uint32 i = 0; i < _characters.size(); ++i)
	{
		_character_growths[i]->GetSkillsLearned()->clear();
	}
}

// ----- UPDATE METHODS

void FinishWindow::Update() {
	MenuWindow::Update(SystemManager->GetUpdateTime());

	switch (_state) {
		case FINISH_WIN_ANNOUNCE:
			_UpdateAnnounceWin();
			break;
		case FINISH_WIN_SHOW_GROWTH:
		case FINISH_WIN_RESOLVE_GROWTH:
		case FINISH_WIN_SHOW_SKILLS:
		case FINISH_WIN_SHOW_SPOILS:
		case FINISH_WIN_RESOLVE_SPOILS:
			_UpdateWinWaitForOK();
			break;
		case FINISH_WIN_COUNTDOWN_GROWTH:
			_UpdateWinGrowth();
			break;
		case FINISH_WIN_COUNTDOWN_SPOILS:
			_UpdateWinSpoils();
			break;
		case FINISH_WIN_COMPLETE:
			current_battle->_ShutDown();
			break;
		case FINISH_LOSE_ANNOUNCE:
			_UpdateAnnounceLose();
			break;
		case FINISH_LOSE_CONFIRM:
			_UpdateLoseConfirm();
			break;
		case FINISH_INVALID:
		case FINISH_TOTAL:
		default:
			if (BATTLE_DEBUG)
				cerr << "BATTLE ERROR: In FinishWindow::Update(), the window state was invalid: " << _state << endl;
			return;
	}
} // void FinishWindow::Update()



void FinishWindow::_UpdateAnnounceWin() {
//	This block is for gradual text. Currently, battle mode uses full text.
// This block causes the game to freeze at the end of battle.
/*	if (_finish_outcome.IsFinished() == false) {
		_finish_outcome.Update(SystemManager->GetUpdateTime());
		
		if (InputManager->ConfirmPress())
			_finish_outcome.ForceFinish();
		return;
	} */

	if (InputManager->ConfirmPress())
		_state = FINISH_WIN_SHOW_GROWTH;
}

// If OK was pressed, just move to the next state
void FinishWindow::_UpdateWinWaitForOK()
{
	if (InputManager->ConfirmPress())
	{
		switch (_state)
		{
			case FINISH_WIN_SHOW_GROWTH:
				_state = FINISH_WIN_COUNTDOWN_GROWTH;
				break;
			case FINISH_WIN_RESOLVE_GROWTH:
				_state = FINISH_WIN_SHOW_SKILLS;
				break;
			case FINISH_WIN_SHOW_SKILLS:
				_state = FINISH_WIN_SHOW_SPOILS;
				_ClearLearnedSkills(); //so we don't render them every battle
				break;
			case FINISH_WIN_SHOW_SPOILS:
				_state = FINISH_WIN_COUNTDOWN_SPOILS;
				break;
			case FINISH_WIN_RESOLVE_SPOILS:
				_state = FINISH_WIN_COMPLETE;
				break;
			default:
				if (BATTLE_DEBUG)
					cerr << "BATTLE ERROR: In FinishWindow::_UpdateWinWaitForOK(), the window state was invalid: " << _state << endl;
				return;
		}
	}
}

void FinishWindow::_UpdateWinGrowth() {
	static uint32 time_of_next_update = SDL_GetTicks();
	uint32 xp_to_add = 1;

	if (InputManager->ConfirmPress())
	{
		xp_to_add = _victory_xp;
		_victory_xp = 0;
	}
	else if (SDL_GetTicks() < time_of_next_update)
	{
		return;
	}
	else
	{
		--_victory_xp;
	}

	for (uint32 i = 0; i < _characters.size(); ++i)
	{
		if (_characters[i]->IsAlive())
		{
			if (_characters[i]->AddExperiencePoints(xp_to_add))
			{
				do {
					//Record growth stats for each character for rendering
					//HP
					_growth_gained[i][0] += _character_growths[i]->GetHitPointsGrowth();
					//SP
					_growth_gained[i][1] += _character_growths[i]->GetSkillPointsGrowth();
					//STR
					_growth_gained[i][2] += _character_growths[i]->GetStrengthGrowth();
					//VIG
					_growth_gained[i][3] += _character_growths[i]->GetVigorGrowth();
					//FOR
					_growth_gained[i][4] += _character_growths[i]->GetFortitudeGrowth();
					//PRO
					_growth_gained[i][5] += _character_growths[i]->GetProtectionGrowth();
					//AGI
					_growth_gained[i][6] += _character_growths[i]->GetAgilityGrowth();
					//EVD
					_growth_gained[i][7] += _character_growths[i]->GetEvadeGrowth();

					if (_character_growths[i]->IsExperienceLevelGained())
					{
						//Play Sound
					}
					_character_growths[i]->AcknowledgeGrowth();
				} while(_character_growths[i]->IsGrowthDetected());
			}
		}
	}

	//We've allocated all the XP
	if (!_victory_xp)
		_state = FINISH_WIN_RESOLVE_GROWTH;

	//Every 50 milliseconds we update
	time_of_next_update += 50;
}

void FinishWindow::_UpdateWinSpoils() {
	static uint32 time_of_next_update = SDL_GetTicks();
	uint32 money_to_add = 1;

	if (InputManager->ConfirmPress())
	{
		money_to_add = _victory_money;
		_victory_money = 0;
	}
	else if (SDL_GetTicks() < time_of_next_update)
	{
		return;
	}
	else
	{
		--_victory_money;
	}

	GlobalManager->AddDrunes(money_to_add);

	if (!_victory_money)
	{
		std::map<GlobalObject*, int32>::iterator iter;

		for (iter = _victory_items.begin(); iter != _victory_items.end(); ++iter)
		{
			GlobalManager->AddToInventory(iter->first->GetID(), iter->second);
		}

		_state = FINISH_WIN_RESOLVE_SPOILS;
	}

	//Every 50 milliseconds we update
	time_of_next_update += 50;
}



void FinishWindow::_UpdateAnnounceLose() {
	_lose_options.Update();

//	This block is for gradual text. Currently, battle mode uses full text.
// This block causes the game to freeze at the end of battle.
/*	if (_finish_outcome.IsFinished() == false) {
		_finish_outcome.Update(SystemManager->GetUpdateTime());
		
		if (InputManager->ConfirmPress())
			_finish_outcome.ForceFinish();
		return;
	} */

	if (InputManager->UpPress()) {
		_lose_options.HandleUpKey();
	}
	else if (InputManager->DownPress()) {
		_lose_options.HandleDownKey();
	}
	else if (InputManager->ConfirmPress()) {
		switch (_lose_options.GetSelection()) {
			case 0: // Retry the battle
				// TODO
				break;
			case 1: // Load from last save point
				// TODO
				break;
			case 2: // Return to main menu
			case 3: // Exit game
				_state = FINISH_LOSE_CONFIRM;
				break;
		}
	}
	
}


void FinishWindow::_UpdateLoseConfirm() {
	if (_lose_options.GetSelection() == 2) {
		ModeManager->SingletonInitialize(); // Removes all game modes and returns to boot mode
	}
	else {
		SystemManager->ExitGame();
	}
}

// ----- DRAW METHODS

void FinishWindow::Draw() {
	VideoManager->DisableSceneLighting();
	//VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);
	//TEMP!
	//Two different window arrangements for win and lose would be best
	//Win has all the elaborate windows, lose just has the game over options
	//MenuWindow::Draw();
	//_items_window.Draw();

	//TEMP!!!
	//Just so everyone has an idea of the potential setup for the finishwindow

	switch (_state) {
		case FINISH_WIN_ANNOUNCE:
			_DrawAnnounceWin();
			break;
		case FINISH_WIN_SHOW_GROWTH:
		case FINISH_WIN_COUNTDOWN_GROWTH:
		case FINISH_WIN_RESOLVE_GROWTH:
			_character_window[0].Draw();
			_character_window[1].Draw();
			_character_window[2].Draw();
			_character_window[3].Draw();

			_xp_and_money_window.Draw();
			_DrawWinGrowth();
			break;
		case FINISH_WIN_SHOW_SKILLS:
			_character_window[0].Draw();
			_character_window[1].Draw();
			_character_window[2].Draw();
			_character_window[3].Draw();

			_xp_and_money_window.Draw();
			_DrawWinSkills();
			break;
		case FINISH_WIN_SHOW_SPOILS:
		case FINISH_WIN_COUNTDOWN_SPOILS:
		case FINISH_WIN_RESOLVE_SPOILS:
			_items_window.Draw();
			_xp_and_money_window.Draw();
			_DrawWinSpoils();
			break;
		case FINISH_LOSE_ANNOUNCE:
			_DrawAnnounceLose();
			break;
		case FINISH_LOSE_CONFIRM:
			_DrawLoseConfirm();
			break;
		case FINISH_WIN_COMPLETE:
			break;
		case FINISH_INVALID:
		case FINISH_TOTAL:
		default:
			if (BATTLE_DEBUG)
				cerr << "BATTLE ERROR: In FinishWindow::Draw(), the window state was invalid: " << _state << endl;
			return;
	}
}


void FinishWindow::_DrawAnnounceWin() {
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->Move(512.0f, 384.0f);
	VideoManager->Text()->Draw("VICTORY!!");
}



void FinishWindow::_DrawWinGrowth() {
	//Draw XP Earned
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->Move(496, 683);
	VideoManager->Text()->Draw(MakeUnicodeString("XP Gained: ") + MakeUnicodeString(NumberToString(_victory_xp)));

	//Now draw char info
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_CENTER, 0);
	//VideoManager->Move(265, 580);
	VideoManager->Move(270, 595);

	ustring display_text;
	for (uint32 i = 0; i < _characters.size(); ++i)
	{
		//Portraits
		_char_portraits[i].Draw();

		VideoManager->MoveRelative(5,-55);
		VideoManager->Text()->Draw(MakeUnicodeString("Lv. ") +
			MakeUnicodeString(NumberToString(_characters[i]->GetExperienceLevel())));
		VideoManager->MoveRelative(0, -15);
		VideoManager->Text()->Draw(MakeUnicodeString("XP To Next: ") +
			MakeUnicodeString(NumberToString(_characters[i]->GetExperienceForNextLevel() - _characters[i]->GetExperiencePoints())));

		//First column
		//VideoManager->MoveRelative(150, 40);
		//VideoManager->MoveRelative(150, 25);
		VideoManager->MoveRelative(140, 105);

		//HP
		display_text = MakeUnicodeString("HP: ") + 
			MakeUnicodeString(NumberToString(_characters[i]->GetMaxHitPoints()));
		if (_growth_gained[i][0])
		{
			display_text += MakeUnicodeString(" (") + 
			MakeUnicodeString(NumberToString(_growth_gained[i][0])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		//SP
		VideoManager->MoveRelative(0, -26);
		display_text = MakeUnicodeString("SP: ") + 
			MakeUnicodeString(NumberToString(_characters[i]->GetMaxSkillPoints()));
		if (_growth_gained[i][1])
		{
			display_text += MakeUnicodeString(" (") + 
			MakeUnicodeString(NumberToString(_growth_gained[i][1])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		//STR
		VideoManager->MoveRelative(0, -26);
		display_text = MakeUnicodeString("STR: ") + 
			MakeUnicodeString(NumberToString(_characters[i]->GetStrength()));
		if (_growth_gained[i][2])
		{
			display_text += MakeUnicodeString(" (") + 
			MakeUnicodeString(NumberToString(_growth_gained[i][2])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		//VIG
		VideoManager->MoveRelative(0, -26);
		display_text = MakeUnicodeString("VIG: ") + 
			MakeUnicodeString(NumberToString(_characters[i]->GetVigor()));
		if (_growth_gained[i][3])
		{
			display_text += MakeUnicodeString(" (") + 
			MakeUnicodeString(NumberToString(_growth_gained[i][3])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		//Second Column
		//FOR
		VideoManager->MoveRelative(155, 78);
		display_text = MakeUnicodeString("FOR: ") + 
			MakeUnicodeString(NumberToString(_characters[i]->GetStrength()));
		if (_growth_gained[i][4])
		{
			display_text += MakeUnicodeString(" (") + 
			MakeUnicodeString(NumberToString(_growth_gained[i][4])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		//PRO
		VideoManager->MoveRelative(0, -26);
		display_text = MakeUnicodeString("PRO: ") + 
			MakeUnicodeString(NumberToString(_characters[i]->GetProtection()));
		if (_growth_gained[i][5])
		{
			display_text += MakeUnicodeString(" (") + 
			MakeUnicodeString(NumberToString(_growth_gained[i][5])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		//AGI
		VideoManager->MoveRelative(0, -26);
		display_text = MakeUnicodeString("AGI: ") + 
			MakeUnicodeString(NumberToString(_characters[i]->GetAgility()));
		if (_growth_gained[i][6])
		{
			display_text += MakeUnicodeString(" (") + 
			MakeUnicodeString(NumberToString(_growth_gained[i][6])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		//EVD
		VideoManager->MoveRelative(0, -26);
		display_text = MakeUnicodeString("EVD: ") + 
			MakeUnicodeString(NumberToString(_characters[i]->GetEvade()));
		if (_growth_gained[i][7])
		{
			display_text += MakeUnicodeString(" (") + 
			MakeUnicodeString(NumberToString(_growth_gained[i][7])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		VideoManager->MoveRelative(-300,-140 + 43);
	}
}

void FinishWindow::_DrawWinSkills()
{
	//Draw XP Earned
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->Move(496, 683);
	VideoManager->Text()->Draw(MakeUnicodeString("XP Gained: ") + MakeUnicodeString(NumberToString(_victory_xp)));

	//Now draw char info
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_CENTER, 0);
	//VideoManager->Move(265, 580);
	VideoManager->Move(270, 595);

	std::vector<GlobalSkill*>* skills_learned = NULL;
	ustring display_text;
	for (uint32 i = 0; i < _characters.size(); ++i)
	{
		//Portrait
		_char_portraits[i].Draw();
		//TEMP
		VideoManager->MoveRelative(140, 35);
		VideoManager->Text()->Draw("Skills Learned");
		VideoManager->MoveRelative(50, -30);
		
		skills_learned = _character_growths[i]->GetSkillsLearned();

		for (uint32 j = 0; j < skills_learned->size(); ++j)
		{
			VideoManager->Text()->Draw(skills_learned->at(j)->GetName());
			VideoManager->MoveRelative(0, -20);
		}

		VideoManager->MoveRelative(-190, -5 + (20 * (float)(skills_learned->size())) - 140);
	}
}

void FinishWindow::_DrawWinSpoils() 
{
	//VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_CENTER, 0);	
	//VideoManager->Move(496, 683);
	//VideoManager->Move(96, 683);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_CENTER, 0);
	VideoManager->Move(280, 683);
	VideoManager->Text()->Draw(MakeUnicodeString("Drunes: ") + MakeUnicodeString(NumberToString(_victory_money)));

	VideoManager->SetDrawFlags(VIDEO_X_RIGHT, VIDEO_Y_CENTER, 0);
	VideoManager->Move(712, 683);
	VideoManager->Text()->Draw(MakeUnicodeString("$ ") + MakeUnicodeString(NumberToString(GlobalManager->GetDrunes())));

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
	//VideoManager->Move(700, 640);
	VideoManager->Move(475, 640);
	VideoManager->Text()->Draw("Items");
	//VideoManager->MoveRelative(-140, -25);
	VideoManager->MoveRelative(-200, -35);

	std::map<GlobalObject*, int32>::iterator iter;

	for (iter = _victory_items.begin(); iter != _victory_items.end(); ++iter)
	{
		VideoManager->Text()->Draw(iter->first->GetName());
		VideoManager->SetDrawFlags(VIDEO_X_RIGHT, VIDEO_Y_TOP, 0);
		VideoManager->MoveRelative(425, 0);
		VideoManager->Text()->Draw(MakeUnicodeString(NumberToString(iter->second)));
		VideoManager->MoveRelative(-425, -25);
		VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
	}
}



void FinishWindow::_DrawAnnounceLose() {
	
	_lose_options.Draw();

}



void FinishWindow::_DrawLoseConfirm() {

}

} // namespace private_battle

} // namespace hoa_battle
