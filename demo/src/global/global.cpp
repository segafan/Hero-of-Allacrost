////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    global.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for the global game manager
*** ***************************************************************************/

#include <iostream>

#include "global.h"
#include "utils.h"
#include "script.h"
#include "video.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_script;
using namespace hoa_video;

namespace hoa_global {

GameGlobal *GlobalManager = NULL;
bool GLOBAL_DEBUG = false;
SINGLETON_INITIALIZE(GameGlobal);

// ****************************************************************************
// ***** GameGlobal class - Initialization and Destruction
// ****************************************************************************

GameGlobal::GameGlobal() {
	if (GLOBAL_DEBUG)
		cout << "GLOBAL: GameGlobal constructor invoked" << endl;
}



GameGlobal::~GameGlobal() {
	if (GLOBAL_DEBUG)
		cout << "GLOBAL: GameGlobal destructor invoked" << endl;

	ClearAllData();

	// Close all persistent script files
	_items_script.CloseFile();
	_weapons_script.CloseFile();
	_armor_script.CloseFile();
}



bool GameGlobal::SingletonInitialize() {
	// Open up the persistent script files
	if (_items_script.OpenFile("dat/objects/items.lua", SCRIPT_READ) == false) {
		cerr << "GLOBAL ERROR: could not open script: dat/items.lua" << endl;
		return false;
	}
	_items_script.ReadOpenTable("items");

	if (_weapons_script.OpenFile("dat/objects/weapons.lua", SCRIPT_READ) == false) {
		cerr << "GLOBAL ERROR: could not open script: dat/weapons.lua" << endl;
		return false;
	}
	_weapons_script.ReadOpenTable("weapons");

	if (_armor_script.OpenFile("dat/objects/armor.lua", SCRIPT_READ) == false) {
		cerr << "GLOBAL ERROR: could not open script: dat/armor.lua" << endl;
		return false;
	}
	_armor_script.ReadOpenTable("armor");

	return true;
}



void GameGlobal::BindToLua() {
	using namespace luabind;

	module(ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GameGlobal>("GameGlobal")
			.def("AddCharacter", &GameGlobal::AddCharacter)
			.def("GetCharacter", &GameGlobal::GetCharacter)
			.def("GetFunds", &GameGlobal::GetFunds)
			.def("SetFunds", &GameGlobal::SetFunds)
			.def("AddFunds", &GameGlobal::AddFunds)
			.def("SubtractFunds", &GameGlobal::SubtractFunds)
			.def("AddToInventory", &GameGlobal::AddToInventory)
			.def("RemoveFromInventory", &GameGlobal::RemoveFromInventory)
			.def("IncrementObjectCount", &GameGlobal::IncrementObjectCount)
			.def("DecrementObjectCount", &GameGlobal::DecrementObjectCount)

			// Namespace constants
			.enum_("constants") [
				// Character type constants
				value("GLOBAL_CHARACTER_CLAUDIUS", GLOBAL_CHARACTER_CLAUDIUS),
				// Object type constants
				value("GLOBAL_OBJECT_INVALID", GLOBAL_OBJECT_INVALID),
				value("GLOBAL_OBJECT_ITEM", GLOBAL_OBJECT_ITEM),
				value("GLOBAL_OBJECT_WEAPON", GLOBAL_OBJECT_WEAPON),
				value("GLOBAL_OBJECT_HEAD_ARMOR", GLOBAL_OBJECT_HEAD_ARMOR),
				value("GLOBAL_OBJECT_TORSO_ARMOR", GLOBAL_OBJECT_TORSO_ARMOR),
				value("GLOBAL_OBJECT_ARM_ARMOR", GLOBAL_OBJECT_ARM_ARMOR),
				value("GLOBAL_OBJECT_LEG_ARMOR", GLOBAL_OBJECT_LEG_ARMOR),
				value("GLOBAL_OBJECT_JEWEL", GLOBAL_OBJECT_JEWEL),
				value("GLOBAL_OBJECT_KEY_ITEM", GLOBAL_OBJECT_KEY_ITEM),
				// Item usage constants
				value("GLOBAL_ITEM_USE_MENU", GLOBAL_ITEM_USE_INVALID),
				value("GLOBAL_ITEM_USE_MENU", GLOBAL_ITEM_USE_MENU),
				value("GLOBAL_ITEM_USE_BATTLE", GLOBAL_ITEM_USE_BATTLE),
				value("GLOBAL_ITEM_USE_ALL", GLOBAL_ITEM_USE_ALL),
				// Elemental type constants
				value("GLOBAL_ELEMENTAL_FIRE", GLOBAL_ELEMENTAL_FIRE),
				value("GLOBAL_ELEMENTAL_WATER", GLOBAL_ELEMENTAL_WATER),
				value("GLOBAL_ELEMENTAL_VOLT", GLOBAL_ELEMENTAL_VOLT),
				value("GLOBAL_ELEMENTAL_EARTH", GLOBAL_ELEMENTAL_EARTH),
				value("GLOBAL_ELEMENTAL_SLICING", GLOBAL_ELEMENTAL_SLICING),
				value("GLOBAL_ELEMENTAL_SMASHING", GLOBAL_ELEMENTAL_SMASHING),
				value("GLOBAL_ELEMENTAL_MAULING", GLOBAL_ELEMENTAL_MAULING),
				value("GLOBAL_ELEMENTAL_PIERCING", GLOBAL_ELEMENTAL_PIERCING),
				// Target constants
				value("GLOBAL_TARGET_INVALID", GLOBAL_TARGET_INVALID),
				value("GLOBAL_TARGET_ATTACK_POINT", GLOBAL_TARGET_ATTACK_POINT),
				value("GLOBAL_TARGET_ACTOR", GLOBAL_TARGET_ACTOR),
				value("GLOBAL_TARGET_PARTY", GLOBAL_TARGET_PARTY)
			]
	];

	module(ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalAttackPoint>("GlobalAttackPoint")
	];

	module(ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalActor>("GlobalActor")
			.def("GetName", &GlobalActor::GetName)
			.def("GetHitPoints", &GlobalActor::GetHitPoints)
			.def("GetMaxHitPoints", &GlobalActor::GetMaxHitPoints)
			.def("GetSkillPoints", &GlobalActor::GetSkillPoints)
			.def("GetMaxSkillPoints", &GlobalActor::GetMaxSkillPoints)
			.def("GetExperienceLevel", &GlobalActor::GetExperienceLevel)
			.def("GetStrength", &GlobalActor::GetStrength)
			.def("GetVigor", &GlobalActor::GetVigor)
			.def("GetFortitude", &GlobalActor::GetFortitude)
			.def("GetResistance", &GlobalActor::GetResistance)
			.def("GetAgility", &GlobalActor::GetAgility)
			.def("GetEvade", &GlobalActor::GetEvade)
			.def("GetPhysicalAttackRating", &GlobalActor::GetPhysicalAttackRating)
			.def("GetMetaphysicalAttackRating", &GlobalActor::GetMetaphysicalAttackRating)
			.def("GetWeaponEquipped", &GlobalActor::GetWeaponEquipped)
			.def("GetArmorEquipped", &GlobalActor::GetArmorEquipped)
			.def("GetAttackPoints", &GlobalActor::GetAttackPoints)
// 			.def("GetElementalAttackBonuses", &GlobalActor::GetElementalAttackBonuses)
// 			.def("GetStatusAttackBonuses", &GlobalActor::GetStatusAttackBonuses)
// 			.def("GetElementalDefenseBonuses", &GlobalActor::GetElementalDefenseBonuses)
// 			.def("GetStatusDefenseBonuses", &GlobalActor::GetStatusDefenseBonuses)

			.def("SetHitPoints", &GlobalActor::SetHitPoints)
			.def("SetSkillPoints", &GlobalActor::SetSkillPoints)
			.def("SetMaxHitPoints", &GlobalActor::SetMaxHitPoints)
			.def("SetMaxSkillPoints", &GlobalActor::SetMaxSkillPoints)
			.def("SetExperienceLevel", &GlobalActor::SetExperienceLevel)
			.def("SetStrength", &GlobalActor::SetStrength)
			.def("SetVigor", &GlobalActor::SetVigor)
			.def("SetFortitude", &GlobalActor::SetFortitude)
			.def("SetProtection", &GlobalActor::SetProtection)
			.def("SetAgility", &GlobalActor::SetAgility)
			.def("SetEvade", &GlobalActor::SetEvade)

			.def("IsAlive", &GlobalActor::IsAlive)
// 			.def("EquipWeapon", &GlobalActor::EquipWeapon)
// 			.def("EquipArmor", &GlobalActor::EquipArmor)
	];

	module(ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalEnemy, GlobalActor>("GlobalEnemy")
	];

	module(ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalCharacter, GlobalActor>("GlobalCharacter")
	];

	module(ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalParty>("GlobalParty")
	];

	module(ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalObject>("GlobalObject")
			.def("GetID", &GlobalObject::GetID)
			.def("GetName", &GlobalObject::GetName)
			.def("GetType", &GlobalObject::GetType)
			.def("GetUsableBy", &GlobalObject::GetUsableBy)
			.def("GetCount", &GlobalObject::GetCount)
			.def("IncrementCount", &GlobalObject::IncrementCount)
			.def("DecrementCount", &GlobalObject::DecrementCount)
	];

	module(ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalItem, GlobalObject>("GlobalItem")
// 			.def(constructor<>(uint32, uint32))
	];

	module(ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalWeapon, GlobalObject>("GlobalWeapon")
// 			.def(constructor<>(uint32, uint32))
	];

	module(ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalArmor, GlobalObject>("GlobalArmor")
// 			.def(constructor<>(uint32, uint32))
	];

	module(ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalStatusEffect>("GlobalStatusEffect")
	];

	module(ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalElementalEffect>("GlobalElementalEffect")
	];

	module(ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalSkill>("GlobalSkill")
	];
} // void GameGlobal::BindToLua()



void GameGlobal::ClearAllData() {
	// Delete all inventory objects
	for (map<uint32, GlobalObject*>::iterator i = _inventory.begin(); i != _inventory.end(); i++) {
		delete i->second;
	}
	_inventory.clear();
	_inventory_items.clear();
	_inventory_weapons.clear();
	_inventory_head_armor.clear();
	_inventory_torso_armor.clear();
	_inventory_arm_armor.clear();
	_inventory_leg_armor.clear();
// 	_inventory_shards.clear();
// 	_inventory_key_items.clear();

	// Delete all characters
	for (map<uint32, GlobalCharacter*>::iterator i = _characters.begin(); i != _characters.end(); i++) {
		delete i->second;
	}
	_characters.clear();
} // void GameGlobal::ClearAllData()

// ****************************************************************************
// ***** GameGlobal class - Character and Party Manipulations
// ****************************************************************************

void GameGlobal::AddCharacter(uint32 id) {
	if (_characters.find(id) != _characters.end()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: attempted to add a character that already existed" << endl;
		return;
	}

	GlobalCharacter *ch = new GlobalCharacter(id);
	_characters.insert(make_pair(id, ch));
}



GlobalCharacter* GameGlobal::GetCharacter(uint32 id) {
	for (uint32 i = 0; i < _characters.size(); i++) {
		if (_characters[i] != 0 && _characters[i]->GetID() == id) {
			return _characters[i];
		}
	}

	if (GLOBAL_DEBUG)
		cerr << "GLOBAL WARNING: No character matching id #" << id << " found in party" << endl;
	return NULL;
}

// ****************************************************************************
// ***** GameGlobal class - Inventory Manipulations
// ****************************************************************************

void GameGlobal::AddToInventory(uint32 obj_id, uint32 obj_count) {
	// If the object is already in the inventory, increment the count of the object 
	if (_inventory.find(obj_id) != _inventory.end()) {
		_inventory[obj_id]->IncrementCount(obj_count);
		return;
	}

	// Otherwise create a new object instance and add it to the inventory
	// Use the id value to figure out what type of object it is
	if (obj_id == 0) {
		cerr << "GLOBAL ERROR: Attempted to add invalid object to inventory with id: " << obj_id << endl;
	} else if (obj_id < 10000) { // Item
		GlobalItem *new_obj = new GlobalItem(obj_id, obj_count);
		new_obj->IncrementCount(obj_count);
		_inventory.insert(make_pair(obj_id, new_obj));
		_inventory_items.push_back(new_obj);
	} else if (obj_id < 20000) { // Weapon
		GlobalWeapon *new_obj = new GlobalWeapon(obj_id, obj_count);
		new_obj->IncrementCount(obj_count);
		_inventory.insert(make_pair(obj_id, new_obj));
		_inventory_weapons.push_back(new_obj);
	} else if (obj_id < 30000) { // Head Armor
		GlobalArmor *new_obj = new GlobalArmor(obj_id, obj_count);
		new_obj->IncrementCount(obj_count);
		_inventory.insert(make_pair(obj_id, new_obj));
		_inventory_head_armor.push_back(new_obj);
	} else if (obj_id < 40000) { // Torso Armor
		GlobalArmor *new_obj = new GlobalArmor(obj_id, obj_count);
		new_obj->IncrementCount(obj_count);
		_inventory.insert(make_pair(obj_id, new_obj));
		_inventory_torso_armor.push_back(new_obj);
	} else if (obj_id < 50000) { // Arm Armor
		GlobalArmor *new_obj = new GlobalArmor(obj_id, obj_count);
		new_obj->IncrementCount(obj_count);
		_inventory.insert(make_pair(obj_id, new_obj));
		_inventory_arm_armor.push_back(new_obj);
	} else if (obj_id < 60000) { // Leg Armor
		GlobalArmor *new_obj = new GlobalArmor(obj_id, obj_count);
		new_obj->IncrementCount(obj_count);
		_inventory.insert(make_pair(obj_id, new_obj));
		_inventory_leg_armor.push_back(new_obj);
	} else if (obj_id < 70000) { // Shard
		// TODO
	} else if (obj_id < 80000) { // Key Item
		// TODO
	} else {
		cerr << "GLOBAL ERROR: Attempted to add invalid object to inventory with id: " << obj_id << endl;
	}
} // void GameGlobal::AddToInventory(uint32 obj_id)



void GameGlobal::RemoveFromInventory(uint32 obj_id) {
	if (_inventory.find(obj_id) == _inventory.end()) {
		cerr << "GLOBAL WARNING: attempted to delete an object from inventory that didn't exist, with id: " << obj_id << endl;
	}

	delete _inventory[obj_id];
	_inventory.erase(obj_id);

	// Use the id value to figure out what type of object it is, and remove it from the object vector
	if (obj_id == 0) {
		cerr << "GLOBAL WARNING: attempted to remove invalid object to inventory with id: " << obj_id << endl;
	} else if (obj_id < 10000) { // Item
		// TODO
	} else if (obj_id < 20000) { // Weapon
		// TODO
	} else if (obj_id < 30000) { // Head Armor
		// TODO
	} else if (obj_id < 40000) { // Torso Armor
		// TODO
	} else if (obj_id < 50000) { // Arm Armor
		// TODO
	} else if (obj_id < 60000) { // Leg Armor
		// TODO
	} else if (obj_id < 70000) { // Shard
		// TODO
	} else if (obj_id < 80000) { // Key Item
		// TODO
	} else {
		cerr << "GLOBAL WARNING: attempted to remove invalid object to inventory with id: " << obj_id << endl;
	}
} // void GameGlobal::RemoveFromInventory(uint32 obj_id)



void GameGlobal::IncrementObjectCount(uint32 obj_id, uint32 count) {
	// Do nothing if the item does not exist in the inventory
	if (_inventory.find(obj_id) == _inventory.end()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: attempted to increment object count for an object that wasn't in the inventory, id: " << obj_id << endl;
		return;
	}

	_inventory[obj_id]->IncrementCount(count);
} // void GameGlobal::IncrementObjectCount(uint32 obj_id, uint32 count)



void GameGlobal::DecrementObjectCount(uint32 obj_id, uint32 count) {
	// Do nothing if the item does not exist in the inventory
	if (_inventory.find(obj_id) == _inventory.end()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: attempted to decrement object count for an object that wasn't in the inventory, id: " << obj_id << endl;
		return;
	}

	// Decrement the number of objects so long as the number to decrement by does not equal or exceed the count
	if (count < _inventory[obj_id]->GetCount()) {
		_inventory[obj_id]->DecrementCount(count);
	}
	// Remove the object from the inventory otherwise
	else {
		RemoveFromInventory(obj_id);
	}
} // void GameGlobal::DecrementObjectCount(uint32 obj_id, uint32 count)

} // namespace hoa_global
