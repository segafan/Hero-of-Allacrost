////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    global_skills.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \brief   Source file for global game skills.
 *****************************************************************************/

#include <iostream>

#include "utils.h"
#include "video.h"
#include "script.h"

#include "global.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_script;



namespace hoa_global {

// ****************************************************************************
// ***** GlobalElementalEffect
// ****************************************************************************

// ****************************************************************************
// ***** GlobalStatusEffect
// ****************************************************************************

bool GlobalStatusEffect::IncrementIntensity(uint8 amount) {
	// Intensity can not be increased beyond the upper bound "extreme"
	if (_intensity == GLOBAL_INTENSITY_POS_EXTREME) {
		return false;
	}

	if (amount == 0) {
		if (GLOBAL_DEBUG) fprintf(stderr, "WARNING: passed 0 for amount argument to increase intensity of status effect\n");
		return false;
	}

// Raging_Hog: changed these _intensity -variables to _intesity_level -variables
	if (amount < 10) {
		// _intensity += amount;
		if (_intensity > GLOBAL_INTENSITY_POS_EXTREME) {
			_intensity = GLOBAL_INTENSITY_POS_EXTREME;
			return false;
		}
		else {
			return true;
		}
	}
	// This is done to protect against the possibility of an overflow condition
	else {
		if (GLOBAL_DEBUG) fprintf(stderr, "WARNING: amount argument was > 10 to increase intensity of status effect\n");

		if (_intensity != GLOBAL_INTENSITY_POS_EXTREME) {
			_intensity = GLOBAL_INTENSITY_POS_EXTREME;
		}
		return false;
	}
} // bool GlobalStatusEffect::IncrementIntensity(uint8 amount)



bool GlobalStatusEffect::DecrementIntensity(uint8 amount) {
	if (_intensity == GLOBAL_INTENSITY_INVALID) {
		return false;
	}

	if (amount == 0) {
		if (GLOBAL_DEBUG) fprintf(stderr, "WARNING: passed 0 for amount argument to decrease intensity of status effect\n");
		return false;
	}

	if (amount <= _intensity) {
		// _intensity -= amount;
		return true;
	}
	// This is done to protect against the possibility of an overflow condition
	else {
		if (_intensity != GLOBAL_INTENSITY_NEUTRAL) {
			_intensity = GLOBAL_INTENSITY_NEUTRAL;
		}
		return false;
	}
} // bool GlobalStatusEffect::DecrementIntensity(uint8 amount)

// ****************************************************************************
// ***** GlobalSkill
// ****************************************************************************

GlobalSkill::GlobalSkill(uint32 id) : _id(id) {
	// TODO: Use the id to look up the approrpriate script and load the skill data

	// TEMP: Only one type of skill is defined: Sword Slash
	_name = MakeUnicodeString("Sword Slash");
	_type = GLOBAL_SKILL_ATTACK;
	_target_type = GLOBAL_TARGET_ATTACK_POINT;
	_sp_required = 0;
	_warmup_time = 0;
	_cooldown_time = 0;
	_level_required = 1;
}



GlobalSkill::~GlobalSkill() {
	// TODO: Close the Lua script referenced by the _function member?

	for (uint32 i = 0; i < _elemental_effects.size(); i++) {
		delete _elemental_effects[i];
	}
	_elemental_effects.empty();

	for (uint32 i = 0; i < _status_effects.size(); i++) {
		delete _status_effects[i].second;
	}
	_status_effects.empty();
}

} // namespace hoa_global
