////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
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
// ***** GlobalStatusEffect
// ****************************************************************************

GlobalStatusEffect::GlobalStatusEffect() :
	_type(GLOBAL_STATUS_INVALID),
	_intensity_level(GLOBAL_INTENSITY_NEUTRAL),
	_icon_image(NULL)
{}



GlobalStatusEffect::GlobalStatusEffect(uint8 type, GLOBAL_INTENSITY intensity_level) :
	_type(type),
	_intensity_level(intensity_level)
{
	_CreateIconImage();
}



GlobalStatusEffect::~GlobalStatusEffect() {
	if (_icon_image != NULL) {
		delete _icon_image;
	}
}



void GlobalStatusEffect::SetType(uint8 type) {
	// TODO: this function needs to check if the status argument it received is a valid status type
	if (_type != type) {
		_type = type;
		_CreateIconImage();
	}
}



void GlobalStatusEffect::SetIntensityLevel(GLOBAL_INTENSITY intensity) {
	if (intensity <= GLOBAL_INTENSITY_POS_EXTREME) {
		if (_intensity_level != intensity) {
			_intensity_level = intensity;
			// TODO Raging_Hog: Whatever his function should do,
			// it should be first declared in global_skills.h
			//_UpdateIconImage();
		}
	}
	
	// To make sure that the intensity level does not exceed the maximum upper bound
	else {
		if (GLOBAL_DEBUG) fprintf(stderr, "WARNING: Tried to set status effect intensity level above maximum\n");
		if (_intensity_level != GLOBAL_INTENSITY_POS_EXTREME) {
			_intensity_level = GLOBAL_INTENSITY_POS_EXTREME;
			//_UpdateIconImage();
		}
	}
}



bool GlobalStatusEffect::IncrementIntensity(uint8 amount) {
	// Intensity can not be increased beyond the upper bound "extreme"
	if (_intensity_level == GLOBAL_INTENSITY_POS_EXTREME) {
		return false;
	}

	if (amount == 0) {
		if (GLOBAL_DEBUG) fprintf(stderr, "WARNING: passed 0 for amount argument to increase intensity of status effect\n");
		return false;
	}

// Raging_Hog: changed these _intensity -variables to _intesity_level -variables
	if (amount < 10) {
		// _intensity_level += amount;
		if (_intensity_level > GLOBAL_INTENSITY_POS_EXTREME) {
			_intensity_level = GLOBAL_INTENSITY_POS_EXTREME;
			_CreateIconImage();
			return false;
		}
		else {
			_CreateIconImage();
			return true;
		}
	}
	// This is done to protect against the possibility of an overflow condition
	else {
		if (GLOBAL_DEBUG) fprintf(stderr, "WARNING: amount argument was > 10 to increase intensity of status effect\n");

		if (_intensity_level != GLOBAL_INTENSITY_POS_EXTREME) {
			_intensity_level = GLOBAL_INTENSITY_POS_EXTREME;
			_CreateIconImage();
		}
		return false;
	}
} // bool GlobalStatusEffect::IncrementIntensity(uint8 amount)



bool GlobalStatusEffect::DecrementIntensity(uint8 amount) {
	if (_intensity_level == GLOBAL_INTENSITY_INVALID) {
		return false;
	}

	if (amount == 0) {
		if (GLOBAL_DEBUG) fprintf(stderr, "WARNING: passed 0 for amount argument to decrease intensity of status effect\n");
		return false;
	}

	if (amount <= _intensity_level) {
		// _intensity_level -= amount;
		_CreateIconImage();
		return true;
	}
	// This is done to protect against the possibility of an overflow condition
	else {
		if (_intensity_level != GLOBAL_INTENSITY_NEUTRAL) {
			_intensity_level = GLOBAL_INTENSITY_NEUTRAL;
			_CreateIconImage();
		}
		return false;
	}
} // bool GlobalStatusEffect::DecrementIntensity(uint8 amount)


// ?? This is defined about 100 rows from here with more meat
/*
static bool CheckValidType(uint8 type) {
	switch (type) {
		case GLOBAL_STATUS_POISON:
		case GLOBAL_STATUS_SLOW:
			return true;
		case GLOBAL_STATUS_NONE:
		default:
			return false;
	}
} // static bool CheckValidType(uint8 type)
*/



void GlobalStatusEffect::_CreateIconImage() {
	// The old icon image, if it exists, is always deleted whenever this function is called
	if (_icon_image != NULL) {
		delete _icon_image;
		_icon_image = NULL;
	}

// 	if (_type == GLOBAL_STATUS_INVALID) {
// 		return;
// 	}

	_icon_image = new StillImage();
	_icon_image->SetDimensions(25, 25); // All icon images are 25x25 pixels
	
	// TODO: Here construct the first part of the image based on the intensity
// 	switch (_intensity_level) {
// 		case GLOBAL_INTENSITY_EXTREME:
// 			// Draw a red image border
// 			break;
// 		case GLOBAL_INTENSITY_GREATER:
// 			// Draw an orange image border
// 			break;
// 		case GLOBAL_INTENSITY_MODERATE:
// 			// Draw a yellow image border
// 			break;
// 		case GLOBAL_INTENSITY_LESSER:
// 			// Draw a green image border
// 			break;
// 
// 		// For the default case (including GLOBAL_INTENSITY_NONE) nothing is drawn.
// 		// No warning message is issued since this is a valid case
// 	}

	// TODO: Here draw the second part of the image based on the type
//	switch (_type) {
// 		case GLOBAL_STATUS_SOMETHING:
// 			break;
//		default:
			// This is not a good case to happen because we'll end up with a blank icon
	//		if (GLOBAL_DEBUG) fprintf(stderr, "WARNING: could not determine icon image to draw for status effect\n");
//	}

	// TODO: Perhaps here the green up-arrow or red down-arrow will be drawn, if we decide not to include those arrows
	// in the status images themselves
} // void GlobalStatusEffect::_CreateIconImage()

// ****************************************************************************
// ***** GlobalElementalEffect
// ****************************************************************************

GlobalElementalEffect::GlobalElementalEffect() :
	_type(GLOBAL_ELEMENTAL_INVALID),
	_strength(0),
	_icon_image(NULL)
{}



GlobalElementalEffect::GlobalElementalEffect(GLOBAL_ELEMENTAL type, uint32 strength) :
	_type(type),
	_strength(strength)
{
	if (CheckValidType(_type) == false) {
		fprintf(stderr, "ERROR: Invalid type in GlobalElementalEffect constructor\n");
		_type = GLOBAL_ELEMENTAL_INVALID;
	}
	_SetIconImage();
}



GlobalElementalEffect::~GlobalElementalEffect()
{
}


void GlobalElementalEffect::SetType(uint8 type) {
	if (_type == type) {
		return;
	}

	// Ascertain that the numeric argument is a valid elemental type


}

// These are already defined
//void GlobalElementalEffect::IncrementStrength(uint32 amount)

//void GlobalElementalEffect::DecrementStrength(uint32 amount)

bool GlobalElementalEffect::CheckValidType(GLOBAL_ELEMENTAL type) {
	switch (type) {
		case GLOBAL_ELEMENTAL_FIRE:
		case GLOBAL_ELEMENTAL_WATER:
		case GLOBAL_ELEMENTAL_VOLT:
		case GLOBAL_ELEMENTAL_EARTH:
		case GLOBAL_ELEMENTAL_SLICING:
		case GLOBAL_ELEMENTAL_SMASHING:
		case GLOBAL_ELEMENTAL_MAULING:
		case GLOBAL_ELEMENTAL_PIERCING:
			return true;
		default:
			return false;
	}
} // static bool CheckValidType(uint8 type)



void GlobalElementalEffect::_SetIconImage() {
	if (_type == GLOBAL_ELEMENTAL_INVALID) {
		_icon_image = NULL;
		return;
	}

	// GetElementalIcon commented out. Not defined in global_skills.h
	// The type is not checked before this call, but it is checked by the following GameGlobal function
	//_icon_image = GlobalManager->GetElementalIcon(_type);
}

// ****************************************************************************
// ***** GlobalSkill
// ****************************************************************************

GlobalSkill::GlobalSkill() {
	// TEMP: Only one type of skill is defined: Sword Slash
	_skill_name = MakeUnicodeString("Sword Slash");
	_skill_type = GLOBAL_SKILL_ATTACK;
	_sp_usage = 10;
	_skill_points_required = 10;
	_warmup_time = 0;
	_cooldown_time = 0;
	_level_required = 1;
	_number_targets = 1;
}



GlobalSkill::GlobalSkill(string script_name) {
	_script_name = script_name;

	// TODO: Decide on structure of skills and read information from a file
// 	ReadDataDescriptor read_data;
// 	string fileName = "dat/skills/" + _script_name + ".lua";
// 
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
