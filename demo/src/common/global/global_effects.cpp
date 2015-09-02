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

#include "system.h"

#include "global_effects.h"
#include "global.h"

using namespace std;

using namespace hoa_utils;

using namespace hoa_system;

namespace hoa_global {

string GetElementName(GLOBAL_ELEMENTAL type) {
	switch (type) {
		case GLOBAL_ELEMENTAL_FIRE:
			return Translate("Fire");
		case GLOBAL_ELEMENTAL_WATER:
			return Translate("Water");
		case GLOBAL_ELEMENTAL_VOLT:
			return Translate("Volt");
		case GLOBAL_ELEMENTAL_EARTH:
			return Translate("Earth");
		case GLOBAL_ELEMENTAL_SLICING:
			return Translate("Slicing");
		case GLOBAL_ELEMENTAL_SMASHING:
			return Translate("Smashing");
		case GLOBAL_ELEMENTAL_MAULING:
			return Translate("Mauling");
		case GLOBAL_ELEMENTAL_PIERCING:
			return Translate("Piercing");
		default:
			return Translate("Invalid Elemental");
	}
}



string GetStatusName(GLOBAL_STATUS type) {
	switch (type) {
		case GLOBAL_STATUS_HP_BOOST:
			return Translate("HP Boost");
		case GLOBAL_STATUS_HP_DRAIN:
			return Translate("HP Drain");
		case GLOBAL_STATUS_SP_BOOST:
			return Translate("SP Boost");
		case GLOBAL_STATUS_SP_DRAIN:
			return Translate("SP Drain");
		case GLOBAL_STATUS_STRENGTH_BOOST:
			return Translate("Strength Boost");
		case GLOBAL_STATUS_STRENGTH_DRAIN:
			return Translate("Strength Drain");
		case GLOBAL_STATUS_VIGOR_BOOST:
			return Translate("Vigor Boost");
		case GLOBAL_STATUS_VIGOR_DRAIN:
			return Translate("Vigor Drain");
		case GLOBAL_STATUS_FORTITUDE_BOOST:
			return Translate("Fortitude Boost");
		case GLOBAL_STATUS_FORTITUDE_DRAIN:
			return Translate("Fortitude Drain");
		case GLOBAL_STATUS_PROTECTION_BOOST:
			return Translate("Protection Boost");
		case GLOBAL_STATUS_PROTECTION_DRAIN:
			return Translate("Protection Drain");
		case GLOBAL_STATUS_AGILITY_BOOST:
			return Translate("Agility Boost");
		case GLOBAL_STATUS_AGILITY_DRAIN:
			return Translate("Agility Drain");
		case GLOBAL_STATUS_EVADE_BOOST:
			return Translate("Evade Boost");
		case GLOBAL_STATUS_EVADE_DRAIN:
			return Translate("Evade Drain");
		case GLOBAL_STATUS_PARALYSIS:
			return Translate("Paralysis");
		default:
			return Translate("Invalid Status");
	}
}

////////////////////////////////////////////////////////////////////////////////
// GlobalElementalEffect class
////////////////////////////////////////////////////////////////////////////////

void GlobalElementalEffect::IncrementIntensity(uint8 amount) {
	hoa_global::IncrementIntensity(_intensity, amount);
}



void GlobalElementalEffect::DecrementIntensity(uint8 amount) {
	hoa_global::DecrementIntensity(_intensity, amount);
}

////////////////////////////////////////////////////////////////////////////////
// GlobalStatusEffect class
////////////////////////////////////////////////////////////////////////////////

bool GlobalStatusEffect::IncrementIntensity(uint8 amount) {
	return hoa_global::IncrementIntensity(_intensity, amount);
}



bool GlobalStatusEffect::DecrementIntensity(uint8 amount) {
	GLOBAL_INTENSITY previous_intensity = _intensity;
	bool intensity_modified = hoa_global::DecrementIntensity(_intensity, amount);

	if (intensity_modified == true) {
		if (_intensity < GLOBAL_INTENSITY_NEUTRAL) {
			IF_PRINT_WARNING(GLOBAL_DEBUG) << "attempted to decrement intensity below neutral level" << endl;
			_intensity = GLOBAL_INTENSITY_NEUTRAL;
			if (_intensity == previous_intensity)
				intensity_modified = false;
		}
	}
	return intensity_modified;
}

} // namespace hoa_global
