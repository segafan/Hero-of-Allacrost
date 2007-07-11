////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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
#include "battle_actors.h"

#include "global.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_script;

namespace hoa_global {

using namespace private_global;

GlobalObject* GlobalCreateNewObject(uint32 id, uint32 count) {
	GlobalObject* new_object = NULL;

	if (id == 0) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalCreateNewObject() was given a zero id argument" << endl;
		return NULL;
	}
	else if (id <= MAX_ITEM_ID)
		new_object = new GlobalItem(id, count);
	else if (id <= MAX_WEAPON_ID)
		new_object = new GlobalWeapon(id, count);
	else if (id <= MAX_LEG_ARMOR_ID)
		new_object = new GlobalArmor(id, count);
	else if (id <= MAX_SHARD_ID)
		new_object = new GlobalShard(id, count);
	else if (id <= MAX_KEY_ITEM_ID)
		new_object = new GlobalKeyItem(id, count);
	else {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalCreateNewObject() was given an invalid id argument" << endl;
	}

	return new_object;
}

// -----------------------------------------------------------------------------
// GlobalItem class
// -----------------------------------------------------------------------------

GlobalItem::GlobalItem(uint32 id, uint32 count) :
	GlobalObject(id, count)
{
	if (_id == 0 || _id > MAX_ITEM_ID) {
		cerr << "GLOBAL ERROR: GlobalItem constructor failed due to an invalid id assignment: " << _id << endl;
		exit(1);
	}

	ReadScriptDescriptor& script_file = GlobalManager->_items_script;

	if (script_file.DoesTableExist(_id) == false) {
		cerr << "GLOBAL ERROR: GlobalItem constructor failed because the table containing the "
			<< "item definition did not exist for item id: " << _id << endl;
		exit(1);
	}

	// Load the item data from the script
	script_file.OpenTable(_id);
	_name = MakeUnicodeString(script_file.ReadString("name"));
	_description = MakeUnicodeString(script_file.ReadString("description"));
	_icon_image.SetFilename(script_file.ReadString("icon"));
	_target_type = static_cast<GLOBAL_TARGET>(script_file.ReadInt("target_type"));
	_target_ally = script_file.ReadBool("target_ally");
	_price = script_file.ReadUInt("standard_price");

	if (script_file.DoesFunctionExist("BattleUse")) {
		_battle_use_function = new ScriptObject();
		*_battle_use_function = script_file.ReadFunctionPointer("BattleUse");
	}
	if (script_file.DoesFunctionExist("MenuUse")) {
		_menu_use_function = new ScriptObject();
		*_menu_use_function = script_file.ReadFunctionPointer("MenuUse");
	}

	// Determine the items's usage based on which use functions are available
	if (_battle_use_function != NULL && _menu_use_function != NULL)
		_usage = GLOBAL_USE_ALL;
	else if (_battle_use_function != NULL && _menu_use_function == NULL)
		_usage = GLOBAL_USE_BATTLE;
	else if (_battle_use_function == NULL && _menu_use_function != NULL)
		_usage = GLOBAL_USE_MENU;
	else
		_usage = GLOBAL_USE_INVALID;

	if (_icon_image.Load() == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalItem constructor failed to load the icon image for the item: " << _id << endl;
	}

	script_file.CloseTable();

	if (script_file.IsErrorDetected()) {
		if (GLOBAL_DEBUG) {
			cerr << "GLOBAL WARNING: GlobalItem constructor incurred script reading errors. They are as follows: " << endl;
			cerr << script_file.GetErrorMessages() << endl;
		}
		return;
	}
} // void GlobalItem::GlobalItem(uint32 id, uint32 count = 1)



GlobalItem::~GlobalItem() {
	if (_battle_use_function != NULL) {
		delete _battle_use_function;
		_battle_use_function = NULL;
	}
	if (_menu_use_function != NULL) {
		delete _menu_use_function;
		_menu_use_function = NULL;
	}
} // void GlobalItem::~GlobalItem()



GlobalItem::GlobalItem(const GlobalItem& copy) {
	_id = copy._id;
	_name = copy._name;
	_description = copy._description;
	_count = copy._count;
	_price = copy._price;
	_icon_image = copy._icon_image;
	_usage = copy._usage;
	_target_type = copy._target_type;
	_target_ally = copy._target_ally;

	// Make copies of valid ScriptObject function pointers
	if (copy._battle_use_function == NULL)
		_battle_use_function = NULL;
	else
		_battle_use_function = new ScriptObject(*copy._battle_use_function);

	if (copy._menu_use_function == NULL)
		_menu_use_function = NULL;
	else
		_menu_use_function = new ScriptObject(*copy._menu_use_function);
} // GlobalItem::GlobalItem(const GlobalItem& copy)



GlobalItem& GlobalItem::operator=(const GlobalItem& copy) {
	if (this == &copy) // Handle self-assignment case
		return *this;

	_id = copy._id;
	_name = copy._name;
	_description = copy._description;
	_count = copy._count;
	_price = copy._price;
	_icon_image = copy._icon_image;
	_usage = copy._usage;
	_target_type = copy._target_type;
	_target_ally = copy._target_ally;

	// Make copies of valid ScriptObject function pointers
	if (copy._battle_use_function == NULL)
		_battle_use_function = NULL;
	else
		_battle_use_function = new ScriptObject(*copy._battle_use_function);

	if (copy._menu_use_function == NULL)
		_menu_use_function = NULL;
	else
		_menu_use_function = new ScriptObject(*copy._menu_use_function);

	return *this;
} // GlobalItem& GlobalItemoperator=(const GlobalItem& copy)



void GlobalItem::BattleUse(hoa_battle::private_battle::BattleActor* target, hoa_battle::private_battle::BattleActor* instigator) {
	if (IsUsableInBattle() == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalItem::BattleUse() failed because battle usage "
				<< "was not supported by the item: " << _id << endl;
		return;
	}

	if (_count == 0) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalItem::BattleUse() failed because the count of the item "
				<< "was set to zero for item: " << _id << endl;
		return;
	}

	ScriptCallFunction<void>(*_battle_use_function, target, instigator);
	_count--;
} // void GlobalItem::BattleUse(hoa_battle::private_battle::BattleActor* target, hoa_battle::private_battle::BattleActor* instigator)



void GlobalItem::MenuUse(GlobalCharacter* target) {
	if (IsUsableInMenu() == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalItem::MenuUse() failed because menu usage "
				<< "was not supported by the item: " << _id << endl;
		return;
	}

	if (_count == 0) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalItem::MenuUse() failed because the count of the item "
				<< "was set to zero for item: " << _id << endl;
		return;
	}

	ScriptCallFunction<void>(*_menu_use_function, target);
	_count--;
} // void GlobalItem::MenuUse(GlobalCharacter* target)

// -----------------------------------------------------------------------------
// GlobalWeapon class
// -----------------------------------------------------------------------------

GlobalWeapon::GlobalWeapon(uint32 id, uint32 count) :
	GlobalObject(id, count)
{
	if (_id <= MAX_ITEM_ID || _id > MAX_WEAPON_ID) {
		cerr << "GLOBAL ERROR: GlobalWeapon constructor failed due to an invalid id assignment: " << _id << endl;
		exit(1);
	}

	ReadScriptDescriptor& script_file = GlobalManager->_weapons_script;

	if (script_file.DoesTableExist(_id) == false) {
		cerr << "GLOBAL ERROR: GlobalWeapon constructor failed because the table containing the "
			<< "weapon definition did not exist for weapon id: " << _id << endl;
		exit(1);
	}

	// Load the weapon data from the script
	script_file.OpenTable(_id);
	_name = MakeUnicodeString(script_file.ReadString("name"));
	_description = MakeUnicodeString(script_file.ReadString("description"));
	_icon_image.SetFilename(script_file.ReadString("icon"));
	_physical_attack = script_file.ReadUInt("physical_attack");
	_metaphysical_attack = script_file.ReadUInt("metaphysical_attack");
	_price = script_file.ReadUInt("standard_price");
	_usable_by = script_file.ReadUInt("usable_by");

	if (_icon_image.Load() == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalWeapon constructor failed to load the icon image for the weapon: " << _id << endl;
	}

	script_file.CloseTable();

	if (script_file.IsErrorDetected()) {
		if (GLOBAL_DEBUG) {
			cerr << "GLOBAL WARNING: GlobalWeapon constructor incurred script reading errors. They are as follows: " << endl;
			cerr << script_file.GetErrorMessages() << endl;
		}
		return;
	}
} // void GlobalWeapon::GlobalWeapon(uint32 id, uint32 count = 1)

// -----------------------------------------------------------------------------
// GlobalArmor class
// -----------------------------------------------------------------------------

GlobalArmor::GlobalArmor(uint32 id, uint32 count) :
	GlobalObject(id, count)
{
	if (_id <= MAX_WEAPON_ID || _id > MAX_LEG_ARMOR_ID) {
		cerr << "GLOBAL ERROR: GlobalArmor constructor failed due to an invalid id assignment: " << _id << endl;
		exit(1);
	}

	// Figure out the appropriate script reference to grab based on the id value
	ReadScriptDescriptor* script_file;
	if (_id <= MAX_HEAD_ARMOR_ID) {
		script_file = &(GlobalManager->_head_armor_script);
	}
	else if (_id <= MAX_TORSO_ARMOR_ID) {
		script_file = &(GlobalManager->_torso_armor_script);
	}
	else if (_id <= MAX_ARM_ARMOR_ID) {
		script_file = &(GlobalManager->_arm_armor_script);
	}
	else {
		script_file = &(GlobalManager->_leg_armor_script);
	}

	if (script_file->DoesTableExist(_id) == false) {
		cerr << "GLOBAL ERROR: GlobalArmor constructor failed because the table containing the "
			<< "armor definition did not exist for armor id: " << _id << endl;
		exit(1);
	}

	// Load the armor data from the script
	script_file->OpenTable(_id);
	_name = MakeUnicodeString(script_file->ReadString("name"));
	_description = MakeUnicodeString(script_file->ReadString("description"));
	_icon_image.SetFilename(script_file->ReadString("icon"));
	_physical_defense = script_file->ReadUInt("physical_defense");
	_metaphysical_defense = script_file->ReadUInt("metaphysical_defense");
	_price = script_file->ReadUInt("standard_price");
	_usable_by = script_file->ReadUInt("usable_by");

	if (_icon_image.Load() == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalArmor constructor failed to load the icon image for the armor: " << _id << endl;
	}

	script_file->CloseTable();

	if (script_file->IsErrorDetected()) {
		if (GLOBAL_DEBUG) {
			cerr << "GLOBAL WARNING: GlobalArmor constructor incurred script reading errors. They are as follows: " << endl;
			cerr << script_file->GetErrorMessages() << endl;
		}
		return;
	}
} // void GlobalArmor::GlobalArmor(uint32 id, uint32 count = 1)



GLOBAL_OBJECT GlobalArmor::GetObjectType() const {
	if (_id <= MAX_HEAD_ARMOR_ID)
		return GLOBAL_OBJECT_HEAD_ARMOR;
	else if (_id <= MAX_TORSO_ARMOR_ID)
		return GLOBAL_OBJECT_TORSO_ARMOR;
	else if (_id <= MAX_ARM_ARMOR_ID)
		return GLOBAL_OBJECT_ARM_ARMOR;
	else
		return GLOBAL_OBJECT_LEG_ARMOR;
}

} // namespace hoa_global
