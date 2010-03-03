///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_utils.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for battle mode utility code
***
*** This file contains utility code that is shared among the various battle mode
*** classes.
*** ***************************************************************************/

#include "defs.h"
#include "utils.h"

#include "global.h"

#include "battle.h"
#include "battle_actors.h"
#include "battle_utils.h"

using namespace std;

using namespace hoa_utils;

using namespace hoa_global;

namespace hoa_battle {

namespace private_battle {

float timer_multiplier = 1.0f;

bool wait;

////////////////////////////////////////////////////////////////////////////////
// BattleTarget class
////////////////////////////////////////////////////////////////////////////////

BattleTarget::BattleTarget() :
	_type(GLOBAL_TARGET_INVALID),
	_point(0),
	_actor(NULL),
	_party(NULL)
{}



void BattleTarget::InvalidateTarget() {
	_type = GLOBAL_TARGET_INVALID;
	_point = 0;
	_actor = NULL;
	_party = NULL;
}



void BattleTarget::SetInitialTarget(BattleActor* user, GLOBAL_TARGET type) {
	InvalidateTarget();

	if (user == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received NULL argument" << endl;
		return;
	}
	if ((type <= GLOBAL_TARGET_INVALID) || (type >= GLOBAL_TARGET_TOTAL)) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid target type argument: " << type << endl;
		return;
	}

	// Determine what party the initial target will exist in
	deque<BattleActor*>* target_party;
	if ((type == GLOBAL_TARGET_ALLY_POINT) || (type == GLOBAL_TARGET_ALLY) || (type == GLOBAL_TARGET_ALL_ALLIES)) {
		if (user->IsEnemy() == false)
			target_party = &BattleMode::CurrentInstance()->GetCharacterParty();
		else
			target_party = &BattleMode::CurrentInstance()->GetEnemyParty();
	}
	else if ((type == GLOBAL_TARGET_FOE_POINT) || (type == GLOBAL_TARGET_FOE) || (type == GLOBAL_TARGET_ALL_FOES)) {
		if (user->IsEnemy() == false)
			target_party = &BattleMode::CurrentInstance()->GetEnemyParty();
		else
			target_party = &BattleMode::CurrentInstance()->GetCharacterParty();
	}
	else {
		target_party = NULL;
	}

	// Set the actor/party according to the target type
	switch (type) {
		case GLOBAL_TARGET_SELF_POINT:
		case GLOBAL_TARGET_SELF:
			_actor = user;
			break;
		case GLOBAL_TARGET_ALLY_POINT:
		case GLOBAL_TARGET_FOE_POINT:
		case GLOBAL_TARGET_ALLY:
		case GLOBAL_TARGET_FOE:
			_actor = target_party->at(0);
			break;
		case GLOBAL_TARGET_ALL_ALLIES:
		case GLOBAL_TARGET_ALL_FOES:
			_party = target_party;
			break;
		default:
			IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid type: " << type << endl;
			return;
	}

	_type = type;

	// If the target is not a party and not the user themselves, select the first valid actor
	if ((_actor != NULL) && (_actor != user)) {
		if (IsValid() == false) {
			if (SelectNextActor(user, true, true) == false)
				IF_PRINT_WARNING(BATTLE_DEBUG) << "could not find an initial actor that was a valid target" << endl;
		}
	}
}



void BattleTarget::SetPointTarget(GLOBAL_TARGET type, uint32 attack_point, BattleActor* actor) {
	if (IsTargetPoint(type) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received invalid type argument: " << type << endl;
		return;
	}
	if ((actor == NULL) && (_actor == NULL)) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "attempted to set an attack point with no valid actor selected" << endl;
		return;
	}
	else if ((actor == NULL) && (attack_point >= _actor->GetAttackPoints().size())) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "attack point index was out-of-range: " << attack_point << endl;
		return;
	}
	else if ((_actor == NULL) && (attack_point >= actor->GetAttackPoints().size())) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "attack point index was out-of-range: " << attack_point << endl;
		return;
	}

	_type = type;
	_point = attack_point;
	if (actor != NULL)
		_actor = actor;
	_party = NULL;
}



void BattleTarget::SetActorTarget(GLOBAL_TARGET type, BattleActor* actor) {
	if (IsTargetActor(type) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received invalid type argument: " << type << endl;
		return;
	}
	if (actor == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received NULL argument" << endl;
		return;
	}

	_type = type;
	_point = 0;
	_actor = actor;
	_party = NULL;
}



void BattleTarget::SetPartyTarget(GLOBAL_TARGET type, deque<BattleActor*>* party) {
	if (IsTargetParty(type) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received invalid type argument: " << type << endl;
		return;
	}
	if (party == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received NULL argument" << endl;
		return;
	}

	_type = type;
	_point = 0;
	_actor = NULL;
	_party = party;
}



bool BattleTarget::IsValid() {
	if (IsTargetPoint(_type) == true) {
		if (_actor == NULL)
			return false;
		else if (_point >= _actor->GetAttackPoints().size())
			return false;
		else if (_actor->IsAlive() == false)
			return false;
		else
			return true;
	}
	else if (IsTargetActor(_type) == true) {
		if (_actor == NULL)
			return false;
		else if (_actor->IsAlive() == false)
			return false;
		else
			return true;
	}
	else if (IsTargetParty(_type) == true) {
		if (_party == NULL)
			return false;
		else
			return true;
	}
	else {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid target type: " << _type << endl;
		return false;
	}
}



bool BattleTarget::SelectNextPoint(BattleActor* user, bool direction, bool valid_criteria) {
	if (user == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received NULL argument" << endl;
		return false;
	}
	if (IsTargetPoint(_type) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid target type: " << _type << endl;
		return false;
	}
	if (_actor == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "no valid actor target" << endl;
		return false;
	}

	// First check for the case where we need to select a new actor
	if (valid_criteria == true && IsValid() == false) {
		_point = 0;
		return SelectNextActor(user, direction, valid_criteria);
	}

	// If the actor has only a single attack point, there's no way to select another attack point
	uint32 num_points = _actor->GetAttackPoints().size();
	if (num_points == 1) {
		return false;
	}

	if (direction == true) {
		_point++;
		if (_point >= num_points)
			_point = 0;
	}
	else {
		if (_point == 0)
			_point = num_points - 1;
		else
			_point--;
	}
	return true;
}



bool BattleTarget::SelectNextActor(BattleActor* user, bool direction, bool valid_criteria) {
	if (user == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received NULL argument" << endl;
		return false;
	}
	if ((IsTargetPoint(_type) == false) && (IsTargetActor(_type) == false)) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid target type: " << _type << endl;
		return false;
	}
	if (_actor == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "no valid actor target" << endl;
		return false;
	}

	// ----- (1): Retrieve the proper party container that contains the actors we would like to select from
	deque<BattleActor*>* target_party = NULL;
	if ((_type == GLOBAL_TARGET_SELF_POINT) || (_type == GLOBAL_TARGET_SELF)) {
		return false; // Self type targets do not have multiple actors to select from
	}
	else if ((_type == GLOBAL_TARGET_ALLY_POINT) || (_type == GLOBAL_TARGET_ALLY)) {
		if (user->IsEnemy() == false)
			target_party = &BattleMode::CurrentInstance()->GetCharacterParty();
		else
			target_party = &BattleMode::CurrentInstance()->GetEnemyParty();
	}
	else if ((_type == GLOBAL_TARGET_FOE_POINT) || (_type == GLOBAL_TARGET_FOE)) {
		if (user->IsEnemy() == false)
			target_party = &BattleMode::CurrentInstance()->GetEnemyParty();
		else
			target_party = &BattleMode::CurrentInstance()->GetCharacterParty();
	}
	else {
		// This should never be reached because the target type was already determined to be a point or actor above
		IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid target type: " << _type << endl;
		return false;
	}

	// ----- (2): Check the target party for early exit conditions
	if (target_party->empty() == true) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "actor target's party was empty" << endl;
		return false;
	}
	if (target_party->size() == 1) {
		return false; // No more actors to select from in the party
	}

	// ----- (3): Determine the index of the current actor in the target party
	uint32 original_target_index = 0xFFFFFFFF; // Initially set to an impossibly high index for error checking
	for (uint32 i = 0; i < target_party->size(); i++) {
		if (target_party->at(i) == _actor) {
			original_target_index = i;
			break;
		}
	}
	if (original_target_index == 0xFFFFFFFF) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "actor target was not found in party" << endl;
		return false;
	}

	// ----- (4): Starting from the index of the original actor, select the next available actor
	BattleActor* original_actor = _actor;
	uint32 new_target_index = original_target_index;
	while (true) {
		// Increment or decrement the target index based on the direction argument
		if (direction == true) {
			new_target_index = (new_target_index >= target_party->size() - 1) ? 0 : new_target_index + 1;
		}
		else {
			new_target_index = (new_target_index == 0) ? target_party->size() - 1 : new_target_index - 1;
		}

		// If we've reached the original target index then we were unable to select another actor target
		if (new_target_index == original_target_index) {
			_actor = original_actor;
			return false;
		}

		// Set the new actor target and if required, ascertain the new target's validity. If the new target
		// must be valid and this new actor is not, the loop will continue and will try again with the next actor
		_actor = target_party->at(new_target_index);
		if (valid_criteria == false) {
			return true;
		}
		else if (IsValid() == true){
			return true;
		}
	}
} // bool BattleTarget::SelectNextActor(BattleActor* user, bool direction, bool valid_criteria)

////////////////////////////////////////////////////////////////////////////////
// BattleItem class
////////////////////////////////////////////////////////////////////////////////

BattleItem::BattleItem(hoa_global::GlobalItem item) :
	_item(item),
	_available_count(item.GetCount())
{
	if (item.GetID() == 0)
		IF_PRINT_WARNING(BATTLE_DEBUG) << "constructor received invalid item argument" << endl;
}



BattleItem::~BattleItem() {
	if (_available_count != _item.GetCount())
		IF_PRINT_WARNING(BATTLE_DEBUG) << "actual count was not equal to available count upon destruction" << endl;
}



void BattleItem::IncrementAvailableCount() {
	_available_count++;
	if (_available_count > _item.GetCount()) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "attempted to increment available count above actual count: " << _available_count << endl;
		_available_count--;
	}
}



void BattleItem::DecrementAvailableCount() {
	if (_available_count == 0) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "attempted to decrement available count below zero" << endl;
		return;
	}
	_available_count--;
}



void BattleItem::IncrementCount() {
	_item.IncrementCount();
	_available_count++;
}



void BattleItem::DecrementCount() {
	if (_item.GetCount() == 0) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "item count was zero when function was called" << endl;
		return;
	}

	_item.DecrementCount();

	if (_available_count > _item.GetCount()) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "available count was greater than actual count: " << _available_count  << endl;
		_available_count = _item.GetCount();
	}
}

} // namespace private_shop

} // namespace hoa_shop
