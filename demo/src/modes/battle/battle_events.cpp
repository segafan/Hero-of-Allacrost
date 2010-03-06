////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_events.cpp
*** \author  Jacob Rudolph, rujasu@allacrost.org
*** \brief   Source file for battle events.
*** ***************************************************************************/

#include "global.h"
#include "script.h"
#include "battle_events.h"
#include "battle_utils.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_global;
using namespace hoa_script;

namespace hoa_battle {

BattleEvent::BattleEvent(uint32 id) : _id(id), _name(NULL) {
	if (_id == 0 || _id > 5) {
		cerr << "BATTLE ERROR: BattleEvent constructor failed due to an invalid id assignment: " << _id << endl;
		exit(1);
	}
	ReadScriptDescriptor& script_file = *(GlobalManager->GetBattleEventScript());

	if (script_file.DoesTableExist(_id) == false) {
		cerr << "BATTLE ERROR: BattleEvent constructor failed because the table containing the "
			<< "event definition did not exist for event id: " << _id << endl;
		exit(1);
	}

	// Load the item data from the script
	script_file.OpenTable(_id);
	_name = MakeUnicodeString(script_file.ReadString("name"));

	if (	script_file.DoesFunctionExist("Before")
		&& script_file.DoesFunctionExist("During")
		&& script_file.DoesFunctionExist("After")	)
	{
		_before = new ScriptObject();
		*_before = script_file.ReadFunctionPointer("Before");
		_during = new ScriptObject();
		*_during = script_file.ReadFunctionPointer("During");
		_after = new ScriptObject();
		*_after = script_file.ReadFunctionPointer("After");
	}
	else {
		cerr << "BATTLE ERROR: BattleEvent constructor" << endl;
		exit(1);
	}

	script_file.CloseTable();

	if (script_file.IsErrorDetected()) {
		if (GLOBAL_DEBUG) {
			cerr << "BATTLE WARNING: BattleEvent constructor incurred script reading errors. They are as follows: " << endl;
			cerr << script_file.GetErrorMessages() << endl;
		}
		return;
	}
}

BattleEvent::~BattleEvent() {
	delete _before;
	_before = NULL;
	delete _during;
	_during = NULL;
	delete _after;
	_after = NULL;
}

} // namespace hoa_battle
