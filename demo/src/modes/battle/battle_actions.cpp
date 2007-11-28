////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_action.cpp
*** \author  Viljami Korhonen, mindflayer@allacrost.org
*** \author  Andy Gardner, chopperdave@allacrost.org
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for actions that occur in battles.
*** ***************************************************************************/

#include <iostream>
#include <sstream>

#include "utils.h"
#include "system.h"
#include "script.h"
#include "battle.h"
#include "battle_actions.h"
#include "battle_actors.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_script;

using namespace hoa_battle::private_battle;

namespace hoa_battle {

namespace private_battle {

////////////////////////////////////////////////////////////////////////////////
// BattleAction class
////////////////////////////////////////////////////////////////////////////////

BattleAction::BattleAction(BattleActor* source, BattleActor* target, GlobalAttackPoint* attack_point) :
	_source(source),
	_target(target),
	_attack_point(attack_point)
{
	if ((source == NULL || target == NULL) && BATTLE_DEBUG) {
		cerr << "BATTLE ERROR: BattleAction constructor recieved NULL source and/or target" << endl;
	}
}



void BattleAction::Update() {
	if (_warm_up_time.IsRunning()) {
		//float offset = SystemManager->GetUpdateTime() * (107.f / _warm_up_time.GetDuration());
		float offset = SystemManager->GetUpdateTime() * ((STAMINA_LOCATION_READY - STAMINA_LOCATION_SELECT) / _warm_up_time.GetDuration());
		_source->SetStaminaIconLocation(_source->GetStaminaIconLocation() + offset);
	}

	// TODO: Any warm up animations
}

void BattleAction::VerifyValidTarget(BattleActor* &target)
{
	if (target->IsAlive() || !target->IsEnemy())
		return;

	uint32 index = current_battle->GetIndexOfNextAliveEnemy(true);
	
	if (index == INVALID_BATTLE_ACTOR_INDEX)
	{
		cerr << "BATTLE ERROR: BattleAction::VerifyValidTarget could not find a valid target.  How did we get to this stage?" << endl;
		return;
	}

	target = current_battle->GetEnemyActorAt(index);
}

////////////////////////////////////////////////////////////////////////////////
// SkillAction class
////////////////////////////////////////////////////////////////////////////////

SkillAction::SkillAction(BattleActor* source, BattleActor* target, GlobalSkill* skill, GlobalAttackPoint* attack_point) :
	BattleAction(source, target, attack_point),
	_skill(skill)
{
	_warm_up_time.Initialize(skill->GetWarmupTime(), 0, current_battle);
	_warm_up_time.Run();

	if (skill == NULL && BATTLE_DEBUG) {
		cerr << "BATTLE ERROR: SkillAction constructor recieved NULL skill argument" << endl;
	}
}



void SkillAction::RunScript() {
	VerifyValidTarget(_target);

	if (_skill->GetTargetType() == GLOBAL_TARGET_PARTY) {
		if (_target->IsEnemy()) {
			BattleEnemy* enemy;
			//Loop through all enemies and apply the item
			for (uint32 i = 0; i < current_battle->GetNumberOfEnemies(); i++) {
				enemy = current_battle->GetEnemyActorAt(i);
				_skill->BattleExecute(enemy, _source);
			}
		}
		else { // Target is a character
			BattleCharacter* character;
			//Loop through all party members and apply
			for (uint32 i = 0; i < current_battle->GetNumberOfCharacters(); i++) {
				character = current_battle->GetPlayerCharacterAt(i);
				_skill->BattleExecute(character, _source);
			}
		}
	} // if (_skill->GetTargetType() == GLOBAL_TARGET_PARTY)

	else {
		//CD: We don't check for alive or dead here...what if it's a resurrect spell?
	//	if (_target->IsAlive()) {
		_skill->BattleExecute(_target, _source);
	//	}

		// TODO: what to do if the target is dead? Find a new target? Cancel?
		//else {

		//}
	}

	_source->GetActor()->SubtractSkillPoints(_skill->GetSPRequired());

	// FIX ME temporary code!!!
	if (_source) {
		//_source->TEMP_ResetAttackTimer();
		_source->ResetWaitTime();
	}
} // void SkillAction::RunScript()

////////////////////////////////////////////////////////////////////////////////
// ItemAction class
////////////////////////////////////////////////////////////////////////////////

ItemAction::ItemAction(BattleActor* source, BattleActor* target, GlobalItem* item, GlobalAttackPoint* attack_point) :
	BattleAction(source, target, attack_point),
	_item(item)
{
	_warm_up_time.Initialize(ITEM_WARM_UP_TIME, 0, current_battle);
	_warm_up_time.Run();

	if (item == NULL && BATTLE_DEBUG) {
		cerr << "BATTLE ERROR: ItemAction constructor recieved NULL item argument" << endl;
	}
}



void ItemAction::RunScript() {
	VerifyValidTarget(_target);

	if (_item->GetTargetType() == GLOBAL_TARGET_PARTY) {
		if (_target->IsEnemy()) {
			BattleEnemy* enemy;
			//Loop through enemies and apply the item to each target
			for (uint32 i = 0; i < current_battle->GetNumberOfEnemies(); i++) {
				enemy = current_battle->GetEnemyActorAt(i);
				if (enemy->IsAlive()) {
					_item->BattleUse(enemy, _source);
				}
			}
		}
	
		else {
			BattleCharacter* character;
			//Loop through all party members and apply
			for (uint32 i = 0; i < current_battle->GetNumberOfCharacters(); i++) {
				character = current_battle->GetPlayerCharacterAt(i);
				_item->BattleUse(character, _source);
			}
		}
	} // if (_item->GetTargetType() == GLOBAL_TARGET_PARTY)

	else {
		_item->BattleUse(_target, _source);
	}

	if (_source->IsEnemy() == false) {
		if (_item->GetCount() == 0)
			GlobalManager->RemoveFromInventory(_item->GetID());
	}

	// FIX ME temporary code!!!
	if (_source) {
		//_source->TEMP_ResetAttackTimer();
		_source->ResetWaitTime();
	}
} // void ItemAction::RunScript()

} // namespace private_battle

} // namespace hoa_battle
