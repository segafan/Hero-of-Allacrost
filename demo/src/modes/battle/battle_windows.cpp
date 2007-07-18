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
using namespace hoa_system;
using namespace hoa_global;

namespace hoa_battle {

namespace private_battle {

// *****************************************************************************
// ActionWindow class
// *****************************************************************************

ActionWindow::ActionWindow() {
	// TODO: declare the MenuSkin to be used
	if (MenuWindow::Create(512.0f, 128.0f) == false) {
		cerr << "BATTLE ERROR: In ActionWindow constructor, the call to MenuWindow::Create() failed" << endl;
	}
	MenuWindow::SetPosition(512.0f, 128.0f);
	MenuWindow::SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);


	// NOTE: may need to set the dimensions of these images to 45, 45
	_action_category_icons.resize(4);
	_action_category_icons[0].SetFilename("img/icons/battle/attack.png");
	_action_category_icons[1].SetFilename("img/icons/battle/defend.png");
	_action_category_icons[2].SetFilename("img/icons/battle/support.png");
	_action_category_icons[3].SetFilename("img/icons/battle/item.png");
	for (uint32 i = 0; i < 4; i++) {
		if (VideoManager->LoadImage(_action_category_icons[i]) == false) {
			cerr << "BATTLE ERROR: In ActionWindow constructor, failed to load action category icon: "
				<< _action_category_icons[i].GetFilename() << endl;
			return;
		}
	}

	// Setup options for VIEW_ACTION_CATEGORY
	vector<ustring> category_options;
	category_options.push_back(MakeUnicodeString("<img/icons/battle/attack.png>\n\nAttack"));
	category_options.push_back(MakeUnicodeString("<img/icons/battle/defend.png>\n\nDefend"));
	category_options.push_back(MakeUnicodeString("<img/icons/battle/support.png>\n\nSupport"));
	category_options.push_back(MakeUnicodeString("<img/icons/battle/item.png>\n\nItem"));

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

	// Setup options for VIEW_ACTION_SELECTION
	_action_selection_list.SetPosition(128.0f, 120.0f);
	_action_selection_list.SetCursorOffset(-50.0f, 25.0f);
	_action_selection_list.SetCellSize(300.0f, 35.0f);
	_action_selection_list.SetFont("battle");
	_action_selection_list.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_action_selection_list.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_action_selection_list.SetSelectMode(VIDEO_SELECT_SINGLE);
	_action_selection_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_action_selection_list.SetOwner(this);

	// Setup options for VIEW_TARGET_SELECTION

	// Setup options for VIEW_ACTION_INFORMATION

	// Setup rendered text
	_skill_selection_header.SetAlignment(RenderedText::ALIGN_LEFT);
	_item_selection_header.SetAlignment(RenderedText::ALIGN_LEFT);
	_action_information.SetAlignment(RenderedText::ALIGN_LEFT);
	_target_information.SetAlignment(RenderedText::ALIGN_LEFT);
	
	VideoManager->SetFont("battle");
	VideoManager->SetTextColor(Color(1.0f, 1.0f, 0.0f, 0.8f)); // 80% translucent yellow text
	_skill_selection_header.SetText("Skill                          SP");
	_item_selection_header.SetText("Item                          QTY");

	Reset();
} // ActionWindow::ActionWindow()



ActionWindow::~ActionWindow() {
	MenuWindow::Destroy();

	for (uint32 i = 0; i < _action_category_icons.size(); i++) {
		VideoManager->DeleteImage(_action_category_icons[i]);
	}
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
	if (_character->GetActor()->GetAttackSkills()->empty())
		_action_category_list.EnableOption(0, false);
	if (_character->GetActor()->GetDefenseSkills()->empty())
		_action_category_list.EnableOption(1, false);
	if (_character->GetActor()->GetSupportSkills()->empty())
		_action_category_list.EnableOption(2, false);
	if (GlobalManager->GetInventoryItems()->empty())
		_action_category_list.EnableOption(3, false);

	_action_selection_list.ClearOptions();
} // void ActionWindow::Initialize(BattleCharacter* character)



void ActionWindow::Reset() {
	MenuWindow::Hide();
	_state = VIEW_INVALID;
	_character = NULL;
	_selected_action_category = 0;
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
	if (InputManager->LeftPress()) {
		_action_category_list.HandleLeftKey();
	}
	if (InputManager->RightPress()) {
		_action_category_list.HandleRightKey();
	}
	
	if (InputManager->ConfirmPress() ) {
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
	// TODO: implement cancel press ... what should it do?
	// else if (InputManager->CancelPress()) { ... }
}



void ActionWindow::_UpdateActionSelection() {
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
			// how many are available to use. If the current chararacter uses the item, then the decrement stays.
			// If count == 0, then it's removed from inventory...if item is not used (i.e. battle ends before use),
			// it is incremented back.
			item->DecrementCount(1);
			new_event = new ItemAction(current_battle->_selected_character, current_battle->_selected_target, item,
				attack_points->at(current_battle->_selected_attack_point));
		}
		current_battle->AddBattleActionToQueue(new_event);
		current_battle->_selected_character->SetState(ACTOR_WARM_UP);
		current_battle->_selected_target = NULL;
		current_battle->_selected_character = NULL;
		current_battle->_selected_character_index = current_battle->GetIndexOfFirstIdleCharacter();
		current_battle->_selected_attack_point = 0;

		Reset();
	}

	// If the target is a party type, then ignore arrow key presses since they are not processed
	if (_action_target_type == GLOBAL_TARGET_PARTY) {
		return;
	}

	if (InputManager->UpPress() || InputManager->DownPress()) {
		BattleActor* previous_target = current_battle->_selected_target;
		current_battle->_SelectNextTarget(InputManager->UpPress());
		if (previous_target != current_battle->_selected_target) {
			_ConstructTargetInformation();
		}
	}
	else if (InputManager->LeftPress() || InputManager->RightPress()) {
		uint32 previous_ap = current_battle->_selected_attack_point;
		current_battle->_SelectNextAttackPoint(InputManager->RightPress());
		if (previous_ap != current_battle->_selected_attack_point) {
			_ConstructTargetInformation();
		}
	}
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
	VideoManager->DrawImage(_action_category_icons[_selected_action_category]);
	VideoManager->MoveRelative(0.0f, -40.0f);
	VideoManager->SetDrawFlags(VIDEO_Y_CENTER, 0);

	switch (_selected_action_category) {
		case ACTION_TYPE_ATTACK:
			VideoManager->DrawText("Attack");
			break;
		case ACTION_TYPE_DEFEND:
			VideoManager->DrawText("Defend");
			break;
		case ACTION_TYPE_SUPPORT:
			VideoManager->DrawText("Support");
			break;
		case ACTION_TYPE_ITEM:
			VideoManager->DrawText("Item");
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
	if (_selected_action_category != ACTION_TYPE_ITEM) {
		_skill_selection_header.Draw();
	}
	else {
		_item_selection_header.Draw();
	}

	// Draw the list of actions
	_action_selection_list.Draw();
} // void ActionWindow::_DrawActionSelection()



void ActionWindow::_DrawTargetSelection() {
	VideoManager->Move(640.0f, 125.0f);
	VideoManager->SetTextColor(Color(1.0f, 1.0f, 0.0f, 0.8f)); // 80% translucent yellow text
	VideoManager->DrawText(MakeUnicodeString("Target Information"));
	VideoManager->MoveRelative(0.0f, -40.0f);
	_target_information.Draw();
}



void ActionWindow::_DrawActionInformation() {
	VideoManager->Move(640.0f, 125.0f);
	VideoManager->SetTextColor(Color(1.0f, 1.0f, 0.0f, 0.8f)); // 80% translucent yellow text
	VideoManager->DrawText(MakeUnicodeString("Action Information"));
	VideoManager->MoveRelative(0.0f, -40.0f);
	_action_information.Draw();
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

	// TODO: call a method here on the GlobalActor target that constructs the information text about the target

	// TEMP: to be removed once target information is properly constructed
	if (_action_target_type == GLOBAL_TARGET_ATTACK_POINT) {
		target_text = MakeUnicodeString("Attack Point Targeted.");
	}
	else if (_action_target_type == GLOBAL_TARGET_ACTOR) {
		target_text = MakeUnicodeString("Actor Targeted.");
	}
	else if (_action_target_type == GLOBAL_TARGET_PARTY) {
		target_text = MakeUnicodeString("Party Targeted.");
	}

	VideoManager->SetTextColor(Color::white);
	_target_information.SetText(target_text);
} // void ActionWindow::_ConstructTargetInformation()



void ActionWindow::_ConstructActionInformation() {
	ustring action_text;
	_action_information.Clear();

	// TODO: need to do a more complete job of filling out action information
	if (_selected_action_category == ACTION_TYPE_ITEM) {
		action_text = MakeUnicodeString("Name: ") + GetSelectedItem()->GetName() +
			MakeUnicodeString("\nCurrent Quantity: " + NumberToString(GetSelectedItem()->GetCount())) +
			MakeUnicodeString("\nTarget Type: TODO");
	}

	else if (_selected_action_category == ACTION_TYPE_ATTACK
			|| _selected_action_category == ACTION_TYPE_DEFEND
			|| _selected_action_category == ACTION_TYPE_SUPPORT)
	{
		// TODO: add warm-up and cool-down times (in seconds), and description
		action_text = MakeUnicodeString("Name: ") + GetSelectedSkill()->GetName() +
			MakeUnicodeString("\nSP Required: " + NumberToString(GetSelectedSkill()->GetSPRequired())) +
			MakeUnicodeString("\nTarget Type: TODO");
	}

	VideoManager->SetTextColor(Color::white);
	_action_information.SetText(action_text);
} // void ActionWindow::_ConstructActionInformation()

// /////////////////////////////////////////////////////////////////////////////
// FinishWindow class
// /////////////////////////////////////////////////////////////////////////////

FinishWindow::FinishWindow() {
	// TODO: declare the MenuSkin to be used
	if (MenuWindow::Create(512.0f, 256.0f) == false) {
		cerr << "BATTLE ERROR: In ActionWindow constructor, the call to MenuWindow::Create() failed" << endl;
	}
	MenuWindow::SetPosition(512.0f, 384.0f);
	MenuWindow::SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	MenuWindow::Hide();

	_state = FINISH_INVALID;

	vector<ustring> lose_text;
	lose_text.push_back(MakeUnicodeString("Retry  <R>the battle"));
	lose_text.push_back(MakeUnicodeString("Load   <R>from last save point"));
	lose_text.push_back(MakeUnicodeString("Return <R>to main menu"));
	lose_text.push_back(MakeUnicodeString("Exit   <R>the game"));
	_lose_options.SetOptions(lose_text);
	_lose_options.SetCellSize(128.0f, 50.0f);
	_lose_options.SetPosition(530.0f, 380.0f);
	_lose_options.SetSize(1, 4);
	_lose_options.SetFont("battle");
	_lose_options.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_lose_options.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_lose_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_lose_options.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_lose_options.SetCursorOffset(-60.0f, 25.0f);
	_lose_options.SetSelection(0);
	_lose_options.SetOwner(this);
}



FinishWindow::~FinishWindow() {
	MenuWindow::Destroy();
}



void FinishWindow::Initialize(bool victory) {
	MenuWindow::Show();

	if (victory) {
		_state = FINISH_ANNOUNCE_WIN;
		current_battle->AddMusic("mus/Allacrost_Fanfare.ogg");
		current_battle->_battle_music.back().PlayMusic();
	}
	else {
		_state = FINISH_ANNOUNCE_LOSE;
		current_battle->AddMusic("mus/Allacrost_Intermission.ogg");
		current_battle->_battle_music.back().PlayMusic();
	}
}



void FinishWindow::Update() {
	// TODO: This is temporary... need to properly allow player

	if (_state == FINISH_ANNOUNCE_WIN) {
		if (InputManager->ConfirmPress()) {
			AudioManager->PlaySound("snd/confirm.wav");
			current_battle->_ShutDown();
		}
	}

	else if (_state == FINISH_ANNOUNCE_LOSE) {
		if (InputManager->ConfirmPress()) {
			
		}
	}
}



void FinishWindow::Draw() {
	VideoManager->DisableSceneLighting();
	MenuWindow::Draw();

	// TODO: This all needs to be re-written so that it is formatted nicely and fits on the menu window
	if (_state == FINISH_ANNOUNCE_WIN) {
		VideoManager->Move(520.0f, 384.0f);
		VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
		VideoManager->SetTextColor(Color::white);

		ustring text = MakeUnicodeString("Your party is victorious!\n\n");
// 		text += MakeUnicodeString("XP: ") + MakeUnicodeString(NumberToString(current_battle->_victory_xp) + "\n\n");
// 		text += MakeUnicodeString("SP: ") + MakeUnicodeString(NumberToString(current_battle->_victory_sp) + "\n\n");
// 		text += MakeUnicodeString("Drunes: ") + MakeUnicodeString(NumberToString(current_battle->_victory_money) + "\n\n");
// 		if (current_battle->_victory_level) {
// 			text += MakeUnicodeString("Experience Level Gained\n\n");
// 		}
// 		if (current_battle->_victory_skill) {
// 			text += MakeUnicodeString("New Skill Learned\n\n");
// 		}

// 		if (current_battle->_victory_items.size() > 0) {
// 			text += MakeUnicodeString("Items: ");
// 			std::map<string, uint32>::iterator it;
// 			for (it = current_battle->_victory_items.begin(); it != current_battle->_victory_items.end(); ++it) {
// 				text += MakeUnicodeString(it->first);
// 				text += MakeUnicodeString(" x" + NumberToString(it->second) + "\n\n");
// 			}
// 		}
		VideoManager->DrawText(text);
	}

	else if (_state == FINISH_ANNOUNCE_LOSE) {
		VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
		VideoManager->Move(520.0f, 430.0f);
		VideoManager->DrawText("Your party has been defeated!");
	}
}

} // namespace private_battle

} // namespace hoa_battle
