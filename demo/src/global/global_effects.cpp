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
*** \brief   Source file for global game effects.
***
*** This file contains the class implementation for status and elemental effects.
*** ***************************************************************************/

#include <iostream>

#include "video.h"
#include "global.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_script;

namespace hoa_global {

// -----------------------------------------------------------------------------
// GlobalElementalEffect class
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// GlobalStatusEffect class
// -----------------------------------------------------------------------------

GlobalStatusEffect::GlobalStatusEffect(uint32 id, GLOBAL_INTENSITY intensity) :
	_id(id),
	_name(NULL),
	_intensity(intensity)
{
	if (_id == 0 || _id > 5) {
		cerr << "GLOBAL ERROR: GlobalStatusEffect constructor failed due to an invalid id assignment: " << _id << endl;
		exit(1);
	}
	ReadScriptDescriptor& script_file = GlobalManager->_effects_script;

	if (script_file.DoesTableExist(_id) == false) {
		cerr << "GLOBAL ERROR: GlobalStatusEffect constructor failed because the table containing the "
			<< "effect definition did not exist for status effect id: " << _id << endl;
		exit(1);
	}

	// Load the item data from the script
	script_file.OpenTable(_id);
	_name = MakeUnicodeString(script_file.ReadString("name"));
	
	if (	script_file.DoesFunctionExist("Init") 
		&& script_file.DoesFunctionExist("Update")
		&& script_file.DoesFunctionExist("Remove")	)
	{
		_init = new ScriptObject();
		*_init = script_file.ReadFunctionPointer("Init");
		_update = new ScriptObject();
		*_update = script_file.ReadFunctionPointer("Update");
		_remove = new ScriptObject();
		*_remove = script_file.ReadFunctionPointer("Remove");
	}
	else {
		cerr << "GLOBAL ERROR: GlobalStatusEffect constructor" << endl;
		exit(1);
	}

	script_file.CloseTable();

	if (script_file.IsErrorDetected()) {
		if (GLOBAL_DEBUG) {
			cerr << "GLOBAL WARNING: GlobalStatusEffect constructor incurred script reading errors. They are as follows: " << endl;
			cerr << script_file.GetErrorMessages() << endl;
		}
		return;
	}
}

GlobalStatusEffect::~GlobalStatusEffect() {
	delete _init;
	_init = NULL;
	delete _update;
	_update = NULL;
	delete _remove;
	_remove = NULL;
}

bool GlobalStatusEffect::IncrementIntensity(uint8 amount) {
	// Intensity can not be increased beyond the upper bound "extreme"
	if (_intensity == GLOBAL_INTENSITY_POS_EXTREME) {
		return false;
	}

	if (amount == 0) {
		if (GLOBAL_DEBUG) fprintf(stderr, "WARNING: passed 0 for amount argument to increase intensity of status effect\n");
		return false;
	}

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

} // namespace hoa_global
