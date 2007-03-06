////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    global_objects.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for global game objects.
*** ***************************************************************************/

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
// ***** GlobalItem Class
// ****************************************************************************

void GlobalItem::_Load() {
	if (_id == 0 || _id > 10000) {
		_type = GLOBAL_OBJECT_INVALID;
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: GlobalItem::_Load has an invalid id value: " << _id << endl;
		return;
	}


	// Load the item data from the script
	GlobalManager->_items_script.ReadOpenTable(_id);
	_name = MakeUnicodeString(GlobalManager->_items_script.ReadString("name"));
	_description = MakeUnicodeString(GlobalManager->_items_script.ReadString("description"));
	_icon_image.SetFilename(GlobalManager->_items_script.ReadString("icon"));
	_usage = static_cast<GLOBAL_ITEM_USE>(GlobalManager->_items_script.ReadInt("usage"));
	_target_type = static_cast<GLOBAL_TARGET>(GlobalManager->_items_script.ReadInt("target_type"));
	_function = GlobalManager->_items_script.ReadFunctionPointer("use_function");
	GlobalManager->_items_script.ReadCloseTable();

	if (GlobalManager->_items_script.GetErrorCode() != SCRIPT_NO_ERRORS) {
		return;
	}
	if (_icon_image.Load() == false) {
		return;
	}
} // void GlobalItem::_Load()



void GlobalItem::Use(GlobalTarget* target) {
	if (_count == 0) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: tried to use item " << MakeStandardString(_name) << " which had a count of zero" << endl;
		return;
	}

	ScriptCallFunction<void>(_function);
	_count--;
}

// ****************************************************************************
// ***** GlobalWeapon Class
// ****************************************************************************

void GlobalWeapon::_Load() {
	if (_id < 10001 || _id > 20000) {
		_type = GLOBAL_OBJECT_INVALID;
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: GlobalWeapon::_Load has an invalid id value: " << _id << endl;
		return;
	}

	// Load the weapon data from the script
	GlobalManager->_weapons_script.ReadOpenTable(_id);

	_name = MakeUnicodeString(GlobalManager->_weapons_script.ReadString("name"));
	_description = MakeUnicodeString(GlobalManager->_weapons_script.ReadString("description"));
	_icon_image.SetFilename(GlobalManager->_weapons_script.ReadString("icon"));
	_usable_by = static_cast<uint32>(GlobalManager->_weapons_script.ReadInt("usable_by"));
	_physical_attack = GlobalManager->_weapons_script.ReadInt("physical_attack");
	_metaphysical_attack = GlobalManager->_weapons_script.ReadInt("metaphysical_attack");

	GlobalManager->_weapons_script.ReadCloseTable();

	if (GlobalManager->_weapons_script.GetErrorCode() != SCRIPT_NO_ERRORS) {
		return;
	}
	if (_icon_image.Load() == false) {
		return;
	}
} // void GlobalWeapon::_Load()

// ****************************************************************************
// ***** GlobalArmor Class
// ****************************************************************************

void GlobalArmor::_Load() {
	if (_id < 20001 || _id > 60000) {
		_type = GLOBAL_OBJECT_INVALID;
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: GlobalArmor::_Load has an invalid id value: " << _id << endl;
		return;
	}

	// Set the _type member according to the id value
	if (_id <= 30000) {
		_type = GLOBAL_OBJECT_HEAD_ARMOR;
	}
	else if (_id <= 40000) {
		_type = GLOBAL_OBJECT_TORSO_ARMOR;
	}
	else if (_id <= 50000) {
		_type = GLOBAL_OBJECT_ARM_ARMOR;
	}
	else if (_id <= 60000) {
		_type = GLOBAL_OBJECT_LEG_ARMOR;
	}
	else {
		_type = GLOBAL_OBJECT_INVALID;
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: GlobalArmor::_Load has an unknown id range: " << _id << endl;
		return;
	}

	GlobalManager->_armor_script.ReadOpenTable(_id);

	_name = MakeUnicodeString(GlobalManager->_armor_script.ReadString("name"));
	_description = MakeUnicodeString(GlobalManager->_armor_script.ReadString("description"));
	_icon_image.SetFilename(GlobalManager->_armor_script.ReadString("icon"));
	_usable_by = static_cast<uint32>(GlobalManager->_armor_script.ReadInt("usable_by"));
	_physical_defense = GlobalManager->_armor_script.ReadInt("physical_defense");
	_metaphysical_defense = GlobalManager->_armor_script.ReadInt("metaphysical_defense");

	GlobalManager->_armor_script.ReadCloseTable();

	if (GlobalManager->_armor_script.GetErrorCode() != SCRIPT_NO_ERRORS) {
		return;
	}
	if (_icon_image.Load() == false) {
		return;
	}
	return;
} // void GlobalArmor::_Load()

} // namespace hoa_global
