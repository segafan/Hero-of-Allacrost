////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2009 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_effects.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for battle actor effects.
*** ***************************************************************************/

#include "script.h"
#include "system.h"
#include "video.h"

#include "global.h"

#include "battle.h"
#include "battle_actors.h"
#include "battle_effects.h"
#include "battle_utils.h"

using namespace std;

using namespace hoa_utils;

using namespace hoa_system;
using namespace hoa_script;
using namespace hoa_video;

using namespace hoa_global;

namespace hoa_battle {

namespace private_battle {

////////////////////////////////////////////////////////////////////////////////
// BattleStatusEffect class
////////////////////////////////////////////////////////////////////////////////

BattleStatusEffect::BattleStatusEffect(GLOBAL_STATUS type, GLOBAL_INTENSITY intensity, BattleActor* actor) :
	GlobalStatusEffect(type, intensity),
	_name(GetStatusName(type)),
	_affected_actor(actor),
	_timer(0),
	_apply_function(NULL),
	_icon_image(NULL)
{
	if ((type <= GLOBAL_STATUS_INVALID) || (type >= GLOBAL_STATUS_TOTAL)) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "constructor received an invalid type argument: " << type << endl;
		return;
	}
	if ((intensity <= GLOBAL_INTENSITY_INVALID) || (intensity >= GLOBAL_INTENSITY_TOTAL)) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "constructor received an invalid intensity argument: " << intensity << endl;
		return;
	}
	if (actor == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "constructor received NULL actor argument" << endl;
		return;
	}

	uint32 table_id = static_cast<uint32>(type);
	ReadScriptDescriptor& script_file = GlobalManager->GetStatusEffectsScript();
	if (script_file.DoesTableExist(table_id) == false) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "Lua definition file contained no entry for status effect: " << table_id << endl;
		return;
	}

	script_file.OpenTable(table_id);
	if (script_file.DoesFunctionExist("Apply")) {
		_apply_function = new ScriptObject();
		(*_apply_function) = script_file.ReadFunctionPointer("Apply");
	}
	else {
		PRINT_WARNING << "no apply function found in Lua definition file for status: " << table_id << endl;
	}
	script_file.CloseTable();

	if (script_file.IsErrorDetected()) {
		if (BATTLE_DEBUG) {
			PRINT_WARNING << "one or more errors occurred while reading status effect data - they are listed below" << endl;
			cerr << script_file.GetErrorMessages() << endl;
		}
	}

	_ApplyChange();
}



BattleStatusEffect::~BattleStatusEffect(){
	if (_apply_function != NULL)
		delete _apply_function;
	_apply_function = NULL;
}




void BattleStatusEffect::SetIntensity(hoa_global::GLOBAL_INTENSITY intensity) {
	if ((intensity < GLOBAL_INTENSITY_NEUTRAL) || (intensity >= GLOBAL_INTENSITY_TOTAL)) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "attempted to set status effect to invalid intensity: " << intensity << endl;
		return;
	}

	if (_intensity == intensity) {
		_timer.Reset();
		_timer.Run();
		return;
	}

	_intensity = intensity;
	_ApplyChange();
}




bool BattleStatusEffect::IncrementIntensity(uint8 amount) {
	bool change = GlobalStatusEffect::IncrementIntensity(amount);
	if (change == true)
		_ApplyChange();
	return change;
}



bool BattleStatusEffect::DecrementIntensity(uint8 amount) {
	bool change = GlobalStatusEffect::DecrementIntensity(amount);
	if (change == true)
		_ApplyChange();
	return change;
}



void BattleStatusEffect::_ApplyChange() {
	_icon_image = BattleMode::CurrentInstance()->GetStatusIcon(_type, _intensity);

	_timer.Reset();
	ScriptCallFunction<void>(*_apply_function, this);
	_timer.Run();
}

////////////////////////////////////////////////////////////////////////////////
// EffectsSupervisor class
////////////////////////////////////////////////////////////////////////////////

EffectsSupervisor::EffectsSupervisor(BattleActor* actor) :
	_actor(actor)
{
	if (actor == NULL)
		IF_PRINT_WARNING(BATTLE_DEBUG) << "contructor received NULL actor argument" << endl;
}



EffectsSupervisor::~EffectsSupervisor() {
	for (map<GLOBAL_STATUS, BattleStatusEffect*>::iterator i = _status_effects.begin(); i != _status_effects.end(); i++)
		delete (i->second);
	_status_effects.clear();
}



void EffectsSupervisor::Update() {
	// Update the timers for all active status effects
	SystemTimer* effect_timer = NULL;
	for (map<GLOBAL_STATUS, BattleStatusEffect*>::iterator i = _status_effects.begin(); i != _status_effects.end(); i++) {
		effect_timer = i->second->GetTimer();
		effect_timer->Update();

		// Decrease the intensity of the status by one level when its timer expires. This may result in
		// the status effect being removed from the actor if its intensity changes to the neutral level.
		if (effect_timer->IsFinished() == true) {
			_actor->RegisterStatusChange(i->first, GLOBAL_INTENSITY_NEG_LESSER);
		}

	}
}



void EffectsSupervisor::Draw() {
	for (map<GLOBAL_STATUS, BattleStatusEffect*>::iterator i = _status_effects.begin(); i != _status_effects.end(); i++) {
		i->second->GetIconImage()->Draw();
		VideoManager->MoveRelative(25.0f, 0.0f);
	}
}



GLOBAL_INTENSITY EffectsSupervisor::ChangeStatus(GLOBAL_STATUS status, GLOBAL_INTENSITY& intensity) {
	if ((status <= GLOBAL_STATUS_INVALID) || (status >= GLOBAL_STATUS_TOTAL)) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received invalid status argument: " << status << endl;
		return GLOBAL_INTENSITY_INVALID;
	}

	bool increase_intensity;
	if ((intensity >= GLOBAL_INTENSITY_NEG_EXTREME) && (intensity < GLOBAL_INTENSITY_NEUTRAL)) {
		increase_intensity = false;
	}
	else if ((intensity <= GLOBAL_INTENSITY_POS_EXTREME) && (intensity > GLOBAL_INTENSITY_NEUTRAL)) {
		increase_intensity = true;
	}
	else {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received invalid intensity argument: " << intensity << endl;
		return GLOBAL_INTENSITY_INVALID;
	}

	// TODO: account for opposite status effects. I think perhaps a recursive function call may be useful

	// The intensity of this status effect before this function call was made, used as the function return value
	GLOBAL_INTENSITY old_intensity = GLOBAL_INTENSITY_INVALID;
	// Holds the unsigned amount of change in intensity in either a positive or negative dgree
	uint8 change_amount = abs(static_cast<int8>(intensity));
	// An iterator to the status effect if it already exists on the actor
	map<GLOBAL_STATUS, BattleStatusEffect*>::iterator existing_entry = _status_effects.find(status);
	// Set to true if the status effect was already active on the actor prior to this function call
	bool status_active = (existing_entry != _status_effects.end());

	// Case 1: Decrease intensity when status was not active -- no change in status
	if ((status_active == false) && (increase_intensity == false)) {
		// No warning message is printed for the case where the status was not found. This is done because
		// certain skills/abilities want to remove status effects. It makes the implementation of those
		// abilities easier if they do not have to worry about checking whether or not the status effect is
		// active on the target.
		old_intensity = GLOBAL_INTENSITY_INVALID;
	}
	// Case 2: Increase intensity when status was not active -- add the new status
	else if ((status_active == false) && (increase_intensity == true)) {
		old_intensity = GLOBAL_INTENSITY_NEUTRAL;
		BattleStatusEffect* new_effect = new BattleStatusEffect(status, intensity, _actor);
		_status_effects.insert(make_pair(status, new_effect));
	}
	// Case 3: Decrease intensity when status was active -- decrease intensity and possibly remove status
	else if ((status_active == true) && (increase_intensity == false)) {
		old_intensity = existing_entry->second->GetIntensity();
		existing_entry->second->DecrementIntensity(change_amount);
		intensity = existing_entry->second->GetIntensity();

		if (intensity == GLOBAL_INTENSITY_NEUTRAL) {
			delete existing_entry->second;
			_status_effects.erase(existing_entry);
		}
	}
	// Case 4: Increase intensity when status was active --
	else { // ((status_active == true) && (increase_intensity == true))
		old_intensity = existing_entry->second->GetIntensity();
		existing_entry->second->IncrementIntensity(change_amount);
		intensity = existing_entry->second->GetIntensity();
	}

	return old_intensity;
} // GLOBAL_INTENSITY EffectsSupervisor::ChangeStatus(GLOBAL_STATUS status, GLOBAL_INTENSITY& intensity)

} // namespace private_battle

} // namespace hoa_battle
