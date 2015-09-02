
////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_command.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for battle menu windows
*** ***************************************************************************/

#include "input.h"
#include "system.h"
#include "video.h"

#include "battle.h"
#include "battle_actions.h"
#include "battle_actors.h"
#include "battle_command.h"
#include "battle_utils.h"

using namespace std;

using namespace hoa_utils;

using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_gui;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;

namespace hoa_battle {

namespace private_battle {

const float HEADER_POSITION_X = 140.0f;
const float HEADER_POSITION_Y = 140.0f;
const float HEADER_SIZE_X = 350.0f;
const float HEADER_SIZE_Y = 30.0f;

const float LIST_POSITION_X = 140.0f;
const float LIST_POSITION_Y = 115.0f;
const float LIST_SIZE_X = 350.0f;
const float LIST_SIZE_Y = 120.0f;

////////////////////////////////////////////////////////////////////////////////
// CharacterCommandSettings class
////////////////////////////////////////////////////////////////////////////////

CharacterCommandSettings::CharacterCommandSettings(BattleCharacter* character, MenuWindow& window) :
	_character(character),
	_last_category(CATEGORY_ATTACK),
	_last_item(0),
	_last_self_target(BattleTarget()),
	_last_character_target(BattleTarget()),
	_last_enemy_target(BattleTarget())
{
	_attack_list.SetOwner(&window);
	_attack_list.SetPosition(LIST_POSITION_X, LIST_POSITION_Y);
	_attack_list.SetDimensions(LIST_SIZE_X, LIST_SIZE_Y, 1, 255, 1, 4);
	_attack_list.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_attack_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_attack_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_attack_list.SetTextStyle(TextStyle("text20"));
	_attack_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	_attack_list.SetCursorOffset(-50.0f, 25.0f);

	_defend_list.SetOwner(&window);
	_defend_list.SetPosition(LIST_POSITION_X, LIST_POSITION_Y);
	_defend_list.SetDimensions(LIST_SIZE_X, LIST_SIZE_Y, 1, 255, 1, 4);
	_defend_list.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_defend_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_defend_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_defend_list.SetTextStyle(TextStyle("text20"));
	_defend_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	_defend_list.SetCursorOffset(-50.0f, 25.0f);

	_support_list.SetOwner(&window);
	_support_list.SetPosition(LIST_POSITION_X, LIST_POSITION_Y);
	_support_list.SetDimensions(LIST_SIZE_X, LIST_SIZE_Y, 1, 255, 1, 4);
	_support_list.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_support_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_support_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_support_list.SetTextStyle(TextStyle("text20"));
	_support_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	_support_list.SetCursorOffset(-50.0f, 25.0f);

	if (_character == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "constructor received NULL character pointer" << endl;
		return;
	}

	// Construct the attack, defend, and support skill lists for the character
	vector<GlobalSkill*>* skill_list = NULL;

	skill_list = _character->GetGlobalCharacter()->GetAttackSkills();
	for (uint32 i = 0; i < skill_list->size(); i++) {
		_attack_list.AddOption(ustring());
		_attack_list.AddOptionElementText(i, skill_list->at(i)->GetName());
		_attack_list.AddOptionElementAlignment(i, VIDEO_OPTION_ELEMENT_RIGHT_ALIGN);
		_attack_list.AddOptionElementText(i, MakeUnicodeString(NumberToString(skill_list->at(i)->GetSPRequired())));
		if (skill_list->at(i)->GetSPRequired() > _character->GetGlobalCharacter()->GetSkillPoints()) {
			_attack_list.EnableOption(i, false);
		}
	}
	if (skill_list->empty() == false)
		_attack_list.SetSelection(0);

	skill_list = _character->GetGlobalCharacter()->GetDefenseSkills();
	for (uint32 i = 0; i < skill_list->size(); i++) {
		_defend_list.AddOption(ustring());
		_defend_list.AddOptionElementText(i, skill_list->at(i)->GetName());
		_defend_list.AddOptionElementAlignment(i, VIDEO_OPTION_ELEMENT_RIGHT_ALIGN);
		_defend_list.AddOptionElementText(i, MakeUnicodeString(NumberToString(skill_list->at(i)->GetSPRequired())));
		if (skill_list->at(i)->GetSPRequired() > _character->GetGlobalCharacter()->GetSkillPoints()) {
			_attack_list.EnableOption(i, false);
		}
	}
	if (skill_list->empty() == false)
		_defend_list.SetSelection(0);

	skill_list = _character->GetGlobalCharacter()->GetSupportSkills();
	for (uint32 i = 0; i < skill_list->size(); i++) {
		_attack_list.AddOption(ustring());
		_attack_list.AddOptionElementText(i, skill_list->at(i)->GetName());
		_attack_list.AddOptionElementAlignment(i, VIDEO_OPTION_ELEMENT_RIGHT_ALIGN);
		_attack_list.AddOptionElementText(i, MakeUnicodeString(NumberToString(skill_list->at(i)->GetSPRequired())));
		if (skill_list->at(i)->GetSPRequired() > _character->GetGlobalCharacter()->GetSkillPoints()) {
			_attack_list.EnableOption(i, false);
		}
	}
	if (skill_list->empty() == false)
		_support_list.SetSelection(0);
} // CharacterCommandSettings::CharacterCommandSettings(BattleCharacter* character, MenuWindow& window)



void CharacterCommandSettings::RefreshLists() {
	uint32 require_sp = 0xFFFFFFFF;
	uint32 current_sp = _character->GetSkillPoints();
	vector<GlobalSkill*>* skill_list = NULL;

	skill_list = _character->GetGlobalCharacter()->GetAttackSkills();
	for (uint32 i = 0; i < skill_list->size(); i++) {
		require_sp = skill_list->at(i)->GetSPRequired();
		if (require_sp > current_sp)
			_attack_list.EnableOption(i, false);
		else
			_attack_list.EnableOption(i, true);
	}

	skill_list = _character->GetGlobalCharacter()->GetDefenseSkills();
	for (uint32 i = 0; i < skill_list->size(); i++) {
		require_sp = skill_list->at(i)->GetSPRequired();
		if (require_sp > current_sp)
			_defend_list.EnableOption(i, false);
		else
			_defend_list.EnableOption(i, true);
	}

	skill_list = _character->GetGlobalCharacter()->GetSupportSkills();
	for (uint32 i = 0; i < skill_list->size(); i++) {
		require_sp = skill_list->at(i)->GetSPRequired();
		if (require_sp > current_sp)
			_support_list.EnableOption(i, false);
		else
			_support_list.EnableOption(i, true);
	}
}



void CharacterCommandSettings::SaveLastTarget(BattleTarget& target) {
	switch (target.GetType()) {
		case GLOBAL_TARGET_SELF_POINT:
		case GLOBAL_TARGET_SELF:
			_last_self_target = target;
			break;
		case GLOBAL_TARGET_ALLY_POINT:
		case GLOBAL_TARGET_ALLY:
			_last_character_target = target;
			break;
		case GLOBAL_TARGET_FOE_POINT:
		case GLOBAL_TARGET_FOE:
			_last_enemy_target = target;
			break;
		case GLOBAL_TARGET_ALL_ALLIES:
		case GLOBAL_TARGET_ALL_FOES:
			break; // Party type targets are not retained
		default:
			IF_PRINT_WARNING(BATTLE_DEBUG) << "target argument was an invalid type: " << target.GetType() << endl;
			break;
	}
}



void CharacterCommandSettings::SetLastSelfTarget(BattleTarget& target) {
	if ((target.GetType() != GLOBAL_TARGET_SELF_POINT) && (target.GetType() != GLOBAL_TARGET_SELF)) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "target argument was an invalid type: " << target.GetType() << endl;
		return;
	}

	_last_self_target = target;
}



void CharacterCommandSettings::SetLastCharacterTarget(BattleTarget& target) {
	if ((target.GetType() != GLOBAL_TARGET_ALLY_POINT) && (target.GetType() != GLOBAL_TARGET_ALLY)) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "target argument was an invalid type: " << target.GetType() << endl;
		return;
	}

	_last_character_target = target;
}



void CharacterCommandSettings::SetLastEnemyTarget(BattleTarget& target) {
	if ((target.GetType() != GLOBAL_TARGET_FOE_POINT) && (target.GetType() != GLOBAL_TARGET_FOE)) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "target argument was an invalid type: " << target.GetType() << endl;
		return;
	}

	_last_enemy_target = target;
}

////////////////////////////////////////////////////////////////////////////////
// ItemCommand class
////////////////////////////////////////////////////////////////////////////////

ItemCommand::ItemCommand(MenuWindow& window) {
	_item_header.SetOwner(&window);
	_item_header.SetPosition(HEADER_POSITION_X, HEADER_POSITION_Y);
	_item_header.SetDimensions(HEADER_SIZE_X, HEADER_SIZE_Y, 1, 1, 1, 1);
	_item_header.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_item_header.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_item_header.SetTextStyle(TextStyle("title22"));
	_item_header.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	_item_header.AddOption(UTranslate("Item<R>Count"));

	_item_list.SetOwner(&window);
	_item_list.SetPosition(LIST_POSITION_X, LIST_POSITION_Y);
	_item_list.SetDimensions(LIST_SIZE_X, LIST_SIZE_Y, 1, 255, 1, 4);
	_item_list.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_item_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_item_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_item_list.SetTextStyle(TextStyle("text20"));
	_item_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	_item_list.SetCursorOffset(-50.0f, 25.0f);

	vector<GlobalItem*>* all_items = GlobalManager->GetInventoryItems();
	for (uint32 i = 0; i < all_items->size(); i++) {
		if (all_items->at(i)->IsUsableInBattle() == true) {
			if (all_items->at(i)->GetCount() == 0) {
				IF_PRINT_WARNING(BATTLE_DEBUG) << "discovered item in inventory with a zero count" << endl;
			}

			_items.push_back(BattleItem(GlobalItem(*all_items->at(i))));
		}
	}
	_item_mappings.resize(_items.size(), -1);

	_ReconstructList();
}



void ItemCommand::Initialize(uint32 item_index) {
	if (item_index >= _items.size()) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "item_index argument was out-of-range: " << item_index << endl;
		return;
	}

	// If the item is in the list, set the list selection to that item
	if (_item_mappings[item_index] >= 0) {
		_item_list.SetSelection(_item_mappings[item_index]);
		return;
	}

	// Otherwise find the nearest item to the desired item that is in the list
	uint32 next_item_index = 0xFFFFFFFF;
	uint32 prev_item_index = 0xFFFFFFFF;

	for (uint32 i = item_index + 1; i < _items.size(); i++) {
		if (_item_mappings[i] >= 0) {
			next_item_index = i;
			break;
		}
	}
	for (uint32 i = item_index - 1; i >= 0; i--) {
		if (_item_mappings[i] >= 0) {
			prev_item_index = i;
			break;
		}
	}

	// If this case is true there are no items in the list. This should not happen because the item
	// command should not be used if no items exist
	if ((next_item_index == 0xFFFFFFFF) && (prev_item_index == 0xFFFFFFFF)) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "no items were in the list" << endl;
		return;
	}
	else if ((next_item_index - item_index) <= (item_index - prev_item_index)) {
		_item_list.SetSelection(_item_mappings[next_item_index]);
	}
	else {
		_item_list.SetSelection(_item_mappings[prev_item_index]);
	}
}



BattleItem* ItemCommand::GetSelectedItem() {
	uint32 index = GetItemIndex();
	if (index == 0xFFFFFFFF)
		return NULL;
	else
		return &(_items[index]);
}



uint32 ItemCommand::GetItemIndex() {
	if (_item_list.GetSelection() < 0) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid selection in item list" << endl;
		return 0xFFFFFFFF;
	}

	int32 selection = _item_list.GetSelection();
	for (uint32 i = 0; i < _items.size(); i++) {
		if (_item_mappings[i] == selection) {
			return static_cast<uint32>(selection);
		}
	}

	// Execution should never reach this line
	IF_PRINT_WARNING(BATTLE_DEBUG) << "could not find index for item list selection: " << _item_list.GetSelection() << endl;
	return 0xFFFFFFFF;
}



void ItemCommand::UpdateList() {
	_item_list.Update();

	if (InputManager->UpPress())
		_item_list.InputUp();
	else if (InputManager->DownPress())
		_item_list.InputDown();
}



void ItemCommand::UpdateInformation() {
	// TODO
}



void ItemCommand::DrawList() {
	_item_header.Draw();
	_item_list.Draw();
}



void ItemCommand::DrawInformation() {
	// TODO
}



void ItemCommand::CommitInventoryChanges() {
	for (uint32 i = 0; i < _items.size(); i++) {
		if (_items[i].GetAvailableCount() != _items[i].GetCount()) {
			IF_PRINT_WARNING(BATTLE_DEBUG) << "" << endl;
		}

		// TODO
	}
}



void ItemCommand::_ReconstructList() {
	_item_list.ClearOptions();

	uint32 option_index = 0;
	for (uint32 i = 0; i < _items.size(); i++) {
		// Don't add any items with a non-zero count
		if (_items[i].GetAvailableCount() == 0) {
			_item_mappings[i] = -1;
			continue;
		}

		ustring option_text = _items[i].GetItem().GetName();
		option_text += MakeUnicodeString("<R>×" + NumberToString(_items[i].GetAvailableCount()));
		_item_list.AddOption(option_text);
		_item_mappings[i] = option_index;
		option_index++;
	}

	if (_item_list.GetNumberOptions() == 0)
		_item_list.SetSelection(-1);
	else
		_item_list.SetSelection(0);
}



void ItemCommand::_RefreshEntry(uint32 entry) {
	if (entry >= _item_list.GetNumberOptions()) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "entry argument was out-of-range: " << entry << endl;
		return;
	}

	// Determine which item corresponds to the list entry
	int32 item_index = 0;
	for (uint32 i = 0; i < _item_mappings.size(); i++) {
		if (_item_mappings[i] == static_cast<int32>(entry)) {
			item_index = _item_mappings[i];
			break;
		}
	}

	ustring option_text = _items[item_index].GetItem().GetName();
	option_text += MakeUnicodeString("<R>×" + NumberToString(_items[item_index].GetAvailableCount()));
	_item_list.SetOptionText(entry, option_text);
}

////////////////////////////////////////////////////////////////////////////////
// SkillCommand class
////////////////////////////////////////////////////////////////////////////////

SkillCommand::SkillCommand(MenuWindow& window) :
	_skills(NULL),
	_skill_list(NULL)
{
	_skill_header.SetOwner(&window);
	_skill_header.SetPosition(HEADER_POSITION_X, HEADER_POSITION_Y);
	_skill_header.SetDimensions(HEADER_SIZE_X, HEADER_SIZE_Y, 1, 1, 1, 1);
	_skill_header.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_skill_header.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_skill_header.SetTextStyle(TextStyle("title22"));
	_skill_header.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	_skill_header.AddOption(UTranslate("Skill<R>SP"));
}



void SkillCommand::Initialize(vector<GlobalSkill*>* skills, OptionBox* skill_list) {
	if (skills == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received NULL skills argument" << endl;
		return;
	}
	if (skill_list == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received NULL skill_list argument" << endl;
		return;
	}

	_skills = skills;
	_skill_list = skill_list;
}



GlobalSkill* SkillCommand::GetSelectedSkill() {
	if ((_skills == NULL) || (_skill_list == NULL))
		return NULL;

	uint32 selection = _skill_list->GetSelection();
	if (_skill_list->IsOptionEnabled(selection) == false)
		return NULL;
	else
		return _skills->at(selection);
}



void SkillCommand::UpdateList() {
	if (_skill_list == NULL)
		return;

	_skill_list->Update();

	if (InputManager->UpPress())
		_skill_list->InputUp();
	else if (InputManager->DownPress())
		_skill_list->InputDown();
}



void SkillCommand::UpdateInformation() {
	// TODO
}



void SkillCommand::DrawList() {
	if (_skill_list == NULL)
		return;

	_skill_header.Draw();
	_skill_list->Draw();
}



void SkillCommand::DrawInformation() {
	// TODO
}

////////////////////////////////////////////////////////////////////////////////
// CommandSupervisor class
////////////////////////////////////////////////////////////////////////////////

CommandSupervisor::CommandSupervisor() :
	_state(COMMAND_STATE_INVALID),
	_active_settings(NULL),
	_selected_skill(NULL),
	_selected_item(NULL),
	_item_command(_command_window),
	_skill_command(_command_window)
{
	if (_command_window.Create(512.0f, 128.0f) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "failed to create menu window" << endl;
	}
	_command_window.SetPosition(512.0f, 128.0f);
	_command_window.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_command_window.Show();

	_category_icons.resize(4, StillImage());
	if (_category_icons[0].Load("img/icons/battle/attack.png") == false)
		PRINT_ERROR << "failed to load category icon" << endl;
	if (_category_icons[1].Load("img/icons/battle/defend.png") == false)
		PRINT_ERROR << "failed to load category icon" << endl;
	if (_category_icons[2].Load("img/icons/battle/support.png") == false)
		PRINT_ERROR << "failed to load category icon" << endl;
	if (_category_icons[3].Load("img/icons/battle/item.png") == false)
		PRINT_ERROR << "failed to load category icon" << endl;

	_category_text.resize(4, TextImage("", TextStyle("title22")));
	_category_text[0].SetText(Translate("Attack"));
	_category_text[1].SetText(Translate("Defend"));
	_category_text[2].SetText(Translate("Support"));
	_category_text[3].SetText(Translate("Item"));

	vector<ustring> category_options;
	category_options.push_back(MakeUnicodeString("<img/icons/battle/attack.png>\n") + UTranslate("Attack"));
	category_options.push_back(MakeUnicodeString("<img/icons/battle/defend.png>\n") + UTranslate("Defend"));
	category_options.push_back(MakeUnicodeString("<img/icons/battle/support.png>\n") + UTranslate("Support"));
	category_options.push_back(MakeUnicodeString("<img/icons/battle/item.png>\n") + UTranslate("Item"));

	_category_list.SetOwner(&_command_window);
	_category_list.SetPosition(256.0f, 75.0f);
	_category_list.SetDimensions(400.0f, 80.0f, 4, 1, 4, 1);
	_category_list.SetCursorOffset(-20.0f, 25.0f);
	_category_list.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_category_list.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_category_list.SetTextStyle(TextStyle("title22"));
	_category_list.SetSelectMode(VIDEO_SELECT_SINGLE);
	_category_list.SetOptions(category_options);
	_category_list.SetSelection(0);

	_window_header.SetStyle(TextStyle("title22"));
	_window_text.SetStyle(TextStyle("text20"));
}



CommandSupervisor::~CommandSupervisor() {
	_command_window.Destroy();
}



void CommandSupervisor::ConstructCharacterSettings() {
	deque<BattleCharacter*>& characters = BattleMode::CurrentInstance()->GetCharacterActors();
	for (uint32 i = 0; i < characters.size(); i++)
		_CreateCharacterSettings(characters[i]);
}



void CommandSupervisor::Initialize(BattleCharacter* character) {
	if (character == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function recieved NULL pointer argument" << endl;
		_state = COMMAND_STATE_INVALID;
		return;
	}

	if (_HasCharacterSettings(character) == false)
		_CreateCharacterSettings(character);

	_ChangeState(COMMAND_STATE_CATEGORY);
	_active_settings = &(_character_settings.find(character)->second);
	_category_list.SetSelection(_active_settings->GetLastCategory());

	// Determine which categories should be enabled or disabled
	if (_active_settings->GetAttackList()->GetNumberOptions() == 0)
		_category_list.EnableOption(0, false);
	else
		_category_list.EnableOption(0, true);
	if (_active_settings->GetDefendList()->GetNumberOptions() == 0)
		_category_list.EnableOption(1, false);
	else
		_category_list.EnableOption(1, true);
	if (_active_settings->GetSupportList()->GetNumberOptions() == 0)
		_category_list.EnableOption(2, false);
	else
		_category_list.EnableOption(2, true);
	if (_item_command.GetNumberListOptions() == 0)
		_category_list.EnableOption(3, false);
	else
		_category_list.EnableOption(3, true);

	// Warn if there are no enabled options in the category list
	for (uint32 i = 0; i < _category_list.GetNumberOptions(); i++) {
		if (_category_list.IsOptionEnabled(i) == true)
			return;
	}

	IF_PRINT_WARNING(BATTLE_DEBUG) << "no options in category list were enabled" << endl;
}



void CommandSupervisor::Update() {
	switch (_state) {
		case COMMAND_STATE_CATEGORY:
			_UpdateCategory();
			break;
		case COMMAND_STATE_ACTION:
			_UpdateAction();
			break;
		case COMMAND_STATE_TARGET:
			_UpdateTarget();
			break;
		case COMMAND_STATE_INFORMATION:
			_UpdateInformation();
			break;
		default:
			IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid/unknown command state: " << _state << endl;
			_ChangeState(COMMAND_STATE_CATEGORY);
			return;
	}
}



void CommandSupervisor::Draw() {
	_command_window.Draw();

	switch (_state) {
		case COMMAND_STATE_CATEGORY:
			_DrawCategory();
			break;
		case COMMAND_STATE_ACTION:
			_DrawAction();
			break;
		case COMMAND_STATE_TARGET:
			_DrawTarget();
			break;
		case COMMAND_STATE_INFORMATION:
			_DrawInformation();
			break;
		default:
			IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid/unknown command state: " << _state << endl;
			_ChangeState(COMMAND_STATE_CATEGORY);
			return;
	}
}



void CommandSupervisor::NotifyActorDeath(BattleActor* actor) {
	if (_state == COMMAND_STATE_INVALID) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function called when class was in invalid state" << endl;
		return;
	}

	if (GetCommandCharacter() == actor) {
		_ChangeState(COMMAND_STATE_INVALID);
		return;
	}

	// TODO: update the selected target if the target is the actor who just deceased
	// if (_selected_target.GetActor() == actor)
}



bool CommandSupervisor::_IsSkillCategorySelected() const {
	uint32 category = _category_list.GetSelection();
	if ((category == CATEGORY_ATTACK) || (category == CATEGORY_DEFEND) || (category == CATEGORY_SUPPORT))
		return true;
	else
		return false;
}



bool CommandSupervisor::_IsItemCategorySelected() const {
	uint32 category = _category_list.GetSelection();
	if (category == CATEGORY_ITEM)
		return true;
	else
		return false;
}



void CommandSupervisor::_ChangeState(COMMAND_STATE new_state) {
	if (_state == new_state) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "class was already in state to change to: " << new_state << endl;
		return;
	}

	if (new_state == COMMAND_STATE_INVALID) {
		_active_settings = NULL;
		_selected_skill = NULL;
		_selected_item = NULL;
	}
	else if (new_state == COMMAND_STATE_CATEGORY) {
		// Nothing to do here. The Initialize() function performs all necessary actions when entering this state.
	}
	else if ((new_state == COMMAND_STATE_ACTION) && (_state == COMMAND_STATE_CATEGORY)) {
		switch (_category_list.GetSelection()) {
			case CATEGORY_ATTACK:
				_skill_command.Initialize(GetCommandCharacter()->GetGlobalCharacter()->GetAttackSkills(), _active_settings->GetAttackList());
				break;
			case CATEGORY_DEFEND:
				_skill_command.Initialize(GetCommandCharacter()->GetGlobalCharacter()->GetDefenseSkills(), _active_settings->GetDefendList());
				break;
			case CATEGORY_SUPPORT:
				_skill_command.Initialize(GetCommandCharacter()->GetGlobalCharacter()->GetSupportSkills(), _active_settings->GetSupportList());
				break;
			case CATEGORY_ITEM:
				_item_command.Initialize(_active_settings->GetLastItem());
				break;
			default:
				IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid category selection: " << _category_list.GetSelection() << endl;
				_category_list.SetSelection(0);
				return;
		}
	}
	else if (new_state == COMMAND_STATE_TARGET) {
		BattleActor* user = GetCommandCharacter();
		GLOBAL_TARGET target_type = GLOBAL_TARGET_INVALID;

		if (_IsSkillCategorySelected() == true)
			target_type = _skill_command.GetSelectedSkill()->GetTargetType();
		else if (_IsItemCategorySelected() == true)
			target_type = _item_command.GetSelectedItem()->GetTargetType();

		// Retrieved the saved target depending on the type, or set the target if the target type is a party
		if (IsTargetParty(target_type) == true)
			_selected_target.SetInitialTarget(user, target_type);
		else if (IsTargetSelf(target_type) == true)
			_selected_target = _active_settings->GetLastSelfTarget();
		else if (IsTargetAlly(target_type) == true)
			_selected_target = _active_settings->GetLastCharacterTarget();
		else if (IsTargetFoe(target_type) == true)
			_selected_target = _active_settings->GetLastEnemyTarget();
		else
			IF_PRINT_WARNING(BATTLE_DEBUG) << "no conditions met for invalid target type: " << target_type << endl;


		// If the target type is invalid that means that there is no previous target so grab the initial target
		if (_selected_target.GetType() == GLOBAL_TARGET_INVALID) {
			_selected_target.SetInitialTarget(user, target_type);
		}
		// Otherwise if the last target is no longer valid, select the next valid target
		else if (_selected_target.IsValid() == false) {
			// Party targets should always be valid and attack points on actors do not disappear, so only the actor
			// must be invalid
			if (_selected_target.SelectNextActor(user) == false) {
				IF_PRINT_WARNING(BATTLE_DEBUG) << "no valid targets found" << endl;
			}
		}

		_CreateTargetText();
	}
	else if (new_state == COMMAND_STATE_INFORMATION) {
		_CreateInformationText();
	}

	_state = new_state;
}



void CommandSupervisor::_UpdateCategory() {
	_category_list.Update();

	if (InputManager->ConfirmPress()) {
		if (_category_list.IsOptionEnabled(_category_list.GetSelection()) == true) {
			_active_settings->SetLastCategory(_category_list.GetSelection());
			_ChangeState(COMMAND_STATE_ACTION);
		}
		else {
			// TODO: play an "invalid" sound?
		}
	}

	else if (InputManager->LeftPress()) {
		_category_list.InputLeft();
	}

	else if (InputManager->RightPress()) {
		_category_list.InputRight();
	}
}



void CommandSupervisor::_UpdateAction() {
	if (InputManager->CancelPress()) {
		_ChangeState(COMMAND_STATE_CATEGORY);
		return;
	}

	if (_IsSkillCategorySelected() == true) {
		if (InputManager->ConfirmPress()) {
			_selected_skill = _skill_command.GetSelectedSkill();
			if (_selected_skill != NULL) {
				_ChangeState(COMMAND_STATE_TARGET);
			}
			else {
				// TODO: play "invalid" sound here?
			}
		}

		else if (InputManager->MenuPress()) {
			// TODO
		}

		else {
			_skill_command.UpdateList();
		}
	}
	else if (_IsItemCategorySelected() == true) {
		if (InputManager->ConfirmPress()) {
			_selected_item = _item_command.GetSelectedItem();
			if (_selected_item != NULL) {
				_ChangeState(COMMAND_STATE_TARGET);
			}
			else {
				// TODO: play "invalid" sound here?
			}
		}

		else if (InputManager->MenuPress()) {
			// TODO
		}

		else {
			_item_command.UpdateList();
		}
	}
	else {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid category selection: " << _category_list.GetSelection() << endl;
		_state = COMMAND_STATE_CATEGORY;
		_category_list.SetSelection(0);
	}
}



void CommandSupervisor::_UpdateTarget() {
	if (InputManager->CancelPress()) {
		_state = COMMAND_STATE_ACTION;
	}

	else if (InputManager->ConfirmPress()) {
		_FinalizeCommand();
	}

	else if (InputManager->UpPress() || InputManager->DownPress()) {
		if ((IsTargetPoint(_selected_target.GetType()) == true) || (IsTargetActor(_selected_target.GetType()) == true)) {
			_selected_target.SelectNextActor(GetCommandCharacter(), InputManager->UpPress());
			_CreateTargetText();
		}
	}

	else if (InputManager->RightPress() || InputManager->LeftPress()) {
		if (IsTargetPoint(_selected_target.GetType()) == true) {
			_selected_target.SelectNextPoint(GetCommandCharacter(), InputManager->RightPress());
			_CreateTargetText();
		}
	}
}



void CommandSupervisor::_UpdateInformation() {
	if (InputManager->ConfirmPress() || InputManager->CancelPress())
		_state = COMMAND_STATE_ACTION;
}



void CommandSupervisor::_DrawCategory() {
	_category_list.Draw();
}



void CommandSupervisor::_DrawAction() {
	uint32 category_index = _category_list.GetSelection();

	// Draw the corresponding category icon and text to the left side of the window
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->Move(570.0f, 75.0);
	_category_icons[category_index].Draw();
	VideoManager->MoveRelative(0.0f, -35.0f);
	_category_text[category_index].Draw();

	// Draw the header and list for either the skills or items to the right side of the window
	if (_IsSkillCategorySelected() == true) {
		_skill_command.DrawList();
	}
	else if (_IsItemCategorySelected() == true) {
		_item_command.DrawList();
	}
}



void CommandSupervisor::_DrawTarget() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(560.0f, 110.0f);
	_window_header.Draw();
	VideoManager->Move(560.0f, 85.0f);
	_window_text.Draw();
}



void CommandSupervisor::_DrawInformation() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(580.0f, 100.0f);
	_window_header.Draw();
	VideoManager->Move(600.0f, 80.0f);
	_window_text.Draw();
}



void CommandSupervisor::_CreateTargetText() {
	_window_header.SetText("Select Target");

	ustring target_text;
	if (IsTargetPoint(_selected_target.GetType()) == true) {
		BattleActor* actor = _selected_target.GetActor();
		uint32 point = _selected_target.GetPoint();

		// Add target actor's name and attack point on the first line and actor's HP/SP on the second and third lines
		target_text = actor->GetName();
		target_text += MakeUnicodeString(" — ") + actor->GetAttackPoints().at(point)->GetName() + MakeUnicodeString("\n");
		target_text += MakeUnicodeString("HP: ") + MakeUnicodeString(NumberToString(actor->GetHitPoints())) +
			MakeUnicodeString(" / ") + MakeUnicodeString(NumberToString(actor->GetMaxHitPoints())) + MakeUnicodeString("\n");
		target_text += MakeUnicodeString("SP: ") + MakeUnicodeString(NumberToString(actor->GetSkillPoints())) +
			MakeUnicodeString(" / ") + MakeUnicodeString(NumberToString(actor->GetMaxSkillPoints())) + MakeUnicodeString("\n");
	}
	else if (IsTargetActor(_selected_target.GetType()) == true) {
		BattleActor* actor = _selected_target.GetActor();

		// Add target actor's name on the first line and actor's HP/SP on the second and third lines
		target_text = actor->GetName() + MakeUnicodeString("\n");
		target_text += MakeUnicodeString("HP: ") + MakeUnicodeString(NumberToString(actor->GetHitPoints())) +
			MakeUnicodeString(" / ") + MakeUnicodeString(NumberToString(actor->GetMaxHitPoints())) + MakeUnicodeString("\n");
		target_text += MakeUnicodeString("SP: ") + MakeUnicodeString(NumberToString(actor->GetSkillPoints())) +
			MakeUnicodeString(" / ") + MakeUnicodeString(NumberToString(actor->GetMaxSkillPoints())) + MakeUnicodeString("\n");
	}
	else if (IsTargetParty(_selected_target.GetType()) == true) {
		target_text = MakeUnicodeString("All");
	}
	else {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid target type: " << _selected_target.GetType() << endl;
	}

	_window_text.SetText(target_text);
}



void CommandSupervisor::_CreateInformationText() {
	_window_header.SetText("Action Information");

	ustring info_text;
	if (_IsSkillCategorySelected() == true) {
		info_text = UTranslate("Name: ") + _selected_skill->GetName() + MakeUnicodeString("\n");
		info_text += UTranslate("Required SP: " + NumberToString(_selected_skill->GetSPRequired())) + MakeUnicodeString("\n");
		info_text += UTranslate("Target Type: ") + MakeUnicodeString(GetTargetText(_selected_skill->GetTargetType()));
	}
	else if (_IsItemCategorySelected() == true) {
		info_text = UTranslate("Name: ") + _selected_item->GetItem().GetName() + MakeUnicodeString("\n");
		info_text += UTranslate("Current Quantity: " + NumberToString(_selected_item->GetCount())) + MakeUnicodeString("\n");
		info_text += UTranslate("Target Type: ") + MakeUnicodeString(GetTargetText(_selected_item->GetItem().GetTargetType()));
	}
	else {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "unknown category selected: " << _category_list.GetSelection() << endl;
	}

	_window_text.SetText(info_text);
}



void CommandSupervisor::_FinalizeCommand() {
	BattleAction* new_action = NULL;
	BattleCharacter* character = GetCommandCharacter();

	_active_settings->SaveLastTarget(_selected_target);

	if (_IsSkillCategorySelected() == true) {
		new_action = new SkillAction(character, _selected_target, _selected_skill);
	}
	else if (_IsItemCategorySelected() == true) {
		new_action = new ItemAction(character, _selected_target, _selected_item);
	}
	else {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "did not create action for character, unknown category selected: " << _category_list.GetSelection() << endl;
	}
	character->SetAction(new_action);

	_ChangeState(COMMAND_STATE_INVALID);
	BattleMode::CurrentInstance()->NotifyCharacterCommandComplete(character);
}

} // namespace private_battle

} // namespace hoa_battle
