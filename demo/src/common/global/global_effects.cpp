////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    global_effects.cpp
*** \author  Jacob Rudolph, rujasu@allacrost.org
*** \brief   Source file for global game effects
*** ***************************************************************************/

#include "script.h"
#include "video.h"

#include "global_effects.h"
#include "global.h"

using namespace std;

using namespace hoa_utils;

using namespace hoa_script;
using namespace hoa_video;

namespace hoa_global {

bool IncrementIntensity(GLOBAL_INTENSITY& intensity, uint8 amount) {
	if (amount == 0)
		return false;
	if ((intensity <= GLOBAL_INTENSITY_INVALID) || (intensity >= GLOBAL_INTENSITY_POS_EXTREME))
		return false;

	// This check protects against overflow conditions
	if (amount > (GLOBAL_INTENSITY_TOTAL * 2)) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "attempted to increment intensity by an excessive amount: " << amount << endl;
		if (intensity == GLOBAL_INTENSITY_POS_EXTREME) {
			return false;
		}
		else {
			intensity = GLOBAL_INTENSITY_POS_EXTREME;
			return true;
		}
	}

	// TODO: compiler does not allow this, figure out another way
// 	intensity += amount;
	if (intensity >= GLOBAL_INTENSITY_TOTAL)
		intensity = GLOBAL_INTENSITY_POS_EXTREME;
	return true;
}



bool DecrementIntensity(GLOBAL_INTENSITY& intensity, uint8 amount) {
	if (amount == 0)
		return false;
	if ((intensity <= GLOBAL_INTENSITY_NEG_EXTREME) || (intensity >= GLOBAL_INTENSITY_TOTAL))
		return false;

	// This check protects against overflow conditions
	if (amount > (GLOBAL_INTENSITY_TOTAL * 2)) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "attempted to decrement intensity by an excessive amount: " << amount << endl;
		if (intensity == GLOBAL_INTENSITY_NEG_EXTREME) {
			return false;
		}
		else {
			intensity = GLOBAL_INTENSITY_NEG_EXTREME;
			return true;
		}
	}

	// TODO: compiler does not allow this, figure out another way
// 	intensity -= amount;
	if (intensity <= GLOBAL_INTENSITY_INVALID)
		intensity = GLOBAL_INTENSITY_NEG_EXTREME;
	return true;
}

// -----------------------------------------------------------------------------
// GlobalElementalEffect class
// -----------------------------------------------------------------------------

void GlobalElementalEffect::IncrementIntensity(uint8 amount) {
	hoa_global::IncrementIntensity(_intensity, amount);
}



void GlobalElementalEffect::DecrementIntensity(uint8 amount) {
	hoa_global::DecrementIntensity(_intensity, amount);
}

// -----------------------------------------------------------------------------
// GlobalStatusEffect class
// -----------------------------------------------------------------------------

GlobalStatusEffect::GlobalStatusEffect(uint32 id, GLOBAL_INTENSITY intensity) :
	_id(id),
	_intensity(intensity),
	_str_modifier(0.0f),
	_vig_modifier(0.0f),
	_for_modifier(0.0f),
	_pro_modifier(0.0f),
	_agi_modifier(0.0f),
	_eva_modifier(0.0f),
	_stun(false),
	_timer(NULL),
	_init(NULL),
	_update(NULL),
	_remove(NULL)
{
	// TODO: Replace the "5" numeric below with a constant representing the maximum skill ID number
	if (_id == 0 || _id > 5) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "constructor received an invalid id argument: " << id << endl;
		return;
	}
	ReadScriptDescriptor& script_file = GlobalManager->GetStatusEffectsScript();

	if (script_file.DoesTableExist(_id) == false) {
		IF_PRINT_WARNING(GLOBAL_DEBUG) << "no valid data for status effect in definition file: " << id << endl;
		return;
	}

	// Load the item data from the script
	script_file.OpenTable(_id);
	_name = MakeUnicodeString(script_file.ReadString("name"));

	if (script_file.DoesFunctionExist("Init")
		&& script_file.DoesFunctionExist("Update")
		&& script_file.DoesFunctionExist("Remove"))
	{
		_init = new ScriptObject();
		(*_init) = script_file.ReadFunctionPointer("Init");
		_update = new ScriptObject();
		(*_update) = script_file.ReadFunctionPointer("Update");
		_remove = new ScriptObject();
		(*_remove) = script_file.ReadFunctionPointer("Remove");
	}
	else {
		PRINT_WARNING << "functions missing in status effect definition file: " << id << endl;
		return;
	}

	script_file.CloseTable();
	if (script_file.IsErrorDetected()) {
		if (GLOBAL_DEBUG) {
			PRINT_WARNING << "one or more errors occurred while reading status effect data - they are listed below" << endl;
			cerr << script_file.GetErrorMessages() << endl;
		}
		return;
	}

	_timer = new hoa_system::SystemTimer();
} // GlobalStatusEffect::GlobalStatusEffect(uint32 id, GLOBAL_INTENSITY intensity)



GlobalStatusEffect::~GlobalStatusEffect() {
	if (_timer != NULL)
		delete _timer;
	_timer = NULL;

	if (_init != NULL)
		delete _init;
	_init = NULL;

	if (_update != NULL)
		delete _update;
	_update = NULL;

	if (_remove != NULL)
		delete _remove;
	_remove = NULL;
}



bool GlobalStatusEffect::IncrementIntensity(uint8 amount) {
	return hoa_global::IncrementIntensity(_intensity, amount);
}



bool GlobalStatusEffect::DecrementIntensity(uint8 amount) {
	return hoa_global::DecrementIntensity(_intensity, amount);
}

} // namespace hoa_global
