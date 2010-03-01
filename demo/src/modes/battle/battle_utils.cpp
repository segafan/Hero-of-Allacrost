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
	_attack_point(0),
	_actor(NULL),
	_party(NULL)
{}



void BattleTarget::SetAttackPointTarget(uint32 attack_point, BattleActor* actor) {
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

	_type = GLOBAL_TARGET_ATTACK_POINT;
	_attack_point = attack_point;
	if (actor != NULL)
		_actor = actor;
	_party = NULL;
}



void BattleTarget::SetActorTarget(BattleActor* actor) {
	if (actor == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received NULL argument" << endl;
		return;
	}

	_type = GLOBAL_TARGET_ACTOR;
	_attack_point = 0;
	_actor = actor;
	_party = NULL;
}



void BattleTarget::SetPartyTarget(set<BattleActor*>* party) {
	if (party == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received NULL argument" << endl;
		return;
	}

	_type = GLOBAL_TARGET_PARTY;
	_attack_point = 0;
	_actor = NULL;
	_party = party;
}



bool BattleTarget::IsValid() {
	if (_type == GLOBAL_TARGET_ATTACK_POINT) {
		if (_actor == NULL)
			return false;
		else if (_attack_point >= _actor->GetAttackPoints().size())
			return false;
		else if (_actor->IsAlive() == false)
			return false;
		else
			return true;
	}
	else if (_type == GLOBAL_TARGET_ACTOR) {
		if (_actor == NULL)
			return false;
		else if (_actor->IsAlive() == false)
			return false;
		else
			return true;
	}
	else if (_type == GLOBAL_TARGET_PARTY) {
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



bool BattleTarget::SelectNextAttackPoint(bool direction, bool standard_criteria) {
	if (_type != GLOBAL_TARGET_ATTACK_POINT) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid target type: " << _type << endl;
		return false;
	}
	if (_actor == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "no valid actor target" << endl;
		return false;
	}

	// First check for the case where we need to select a new actor
	if (standard_criteria == true && IsValid() == false) {
		_attack_point = 0;
		return SelectNextActor(direction, standard_criteria);
	}

	// If the actor has only a single attack point, there's no way to select another attack point
	uint32 num_attack_points = _actor->GetAttackPoints().size();
	if (num_attack_points == 1) {
		return false;
	}

	if (direction == true) {
		_attack_point++;
		if (_attack_point >= num_attack_points)
			_attack_point = 0;
	}
	else {
		if (_attack_point == 0)
			_attack_point = num_attack_points - 1;
		else
			_attack_point--;
	}
	return true;
}



bool BattleTarget::SelectNextActor(bool direction, bool valid_criteria) {
	if ((_type != GLOBAL_TARGET_ATTACK_POINT) && (_type != GLOBAL_TARGET_ACTOR)) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid target type: " << _type << endl;
		return false;
	}
	if (_actor == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "no valid actor target" << endl;
		return false;
	}

	// TODO: get character or enemy actor container and cycle through
	return true;
}

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
