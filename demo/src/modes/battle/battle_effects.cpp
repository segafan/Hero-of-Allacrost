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

#include "global.h"
#include "script.h"
#include "battle_effects.h"
#include "battle_utils.h"

using namespace std;

using namespace hoa_utils;

using namespace hoa_script;

using namespace hoa_global;

namespace hoa_battle {

////////////////////////////////////////////////////////////////////////////////
// BattleStatusEffect class
////////////////////////////////////////////////////////////////////////////////

BattleStatusEffect::BattleStatusEffect(GLOBAL_STATUS type, GLOBAL_INTENSITY intensity) :
	GlobalStatusEffect(type, intensity)
{
// 	// TODO: Replace the "5" numeric below with a constant representing the maximum skill ID number
// 	if (_id == 0 || _id > 5) {
// 		IF_PRINT_WARNING(GLOBAL_DEBUG) << "constructor received an invalid id argument: " << id << endl;
// 		return;
// 	}
// 	ReadScriptDescriptor& script_file = GlobalManager->GetStatusEffectsScript();
//
// 	if (script_file.DoesTableExist(_id) == false) {
// 		IF_PRINT_WARNING(GLOBAL_DEBUG) << "no valid data for status effect in definition file: " << id << endl;
// 		return;
// 	}
//
// 	// Load the item data from the script
// 	script_file.OpenTable(_id);
// 	_name = MakeUnicodeString(script_file.ReadString("name"));
//
// 	if (script_file.DoesFunctionExist("Init")
// 		&& script_file.DoesFunctionExist("Update")
// 		&& script_file.DoesFunctionExist("Remove"))
// 	{
// 		_init = new ScriptObject();
// 		(*_init) = script_file.ReadFunctionPointer("Init");
// 		_update = new ScriptObject();
// 		(*_update) = script_file.ReadFunctionPointer("Update");
// 		_remove = new ScriptObject();
// 		(*_remove) = script_file.ReadFunctionPointer("Remove");
// 	}
// 	else {
// 		PRINT_WARNING << "functions missing in status effect definition file: " << id << endl;
// 		return;
// 	}
//
// 	script_file.CloseTable();
// 	if (script_file.IsErrorDetected()) {
// 		if (GLOBAL_DEBUG) {
// 			PRINT_WARNING << "one or more errors occurred while reading status effect data - they are listed below" << endl;
// 			cerr << script_file.GetErrorMessages() << endl;
// 		}
// 		return;
// 	}
}


BattleStatusEffect::~BattleStatusEffect(){
	if (_update != NULL)
		delete _update;
	_update = NULL;
}

} // namespace hoa_battle
