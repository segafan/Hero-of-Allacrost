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
using namespace luabind;

namespace hoa_global {

GameGlobal *GlobalManager = NULL;
bool GLOBAL_DEBUG = false;
SINGLETON_INITIALIZE(GameGlobal);

// ****************************************************************************
// ***** GameGlobal class - Initialization and Destruction
// ****************************************************************************

GameGlobal::GameGlobal() {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GameGlobal constructor invoked" << endl;
}



GameGlobal::~GameGlobal() {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GameGlobal destructor invoked" << endl;
	for (uint32 i = 0; i < _characters.size(); i++) {
		delete _characters[i];
	}

	// Clean up inventory items
	for (uint32 i = 0; i < _inventory.size(); ++i) {
		delete _inventory[i];
	}

	_items_script.CloseFile();
	_weapons_script.CloseFile();
	_armor_script.CloseFile();
}



bool GameGlobal::SingletonInitialize() {
	if (_items_script.OpenFile("dat/items.lua", SCRIPT_READ) == false) {
		cerr << "GLOBAL ERROR: could not open script: dat/items.lua" << endl;
		return false;
	}
	_items_script.ReadOpenTable("items");

	if (_weapons_script.OpenFile("dat/weapons.lua", SCRIPT_READ) == false) {
		cerr << "GLOBAL ERROR: could not open script: dat/weapons.lua" << endl;
		return false;
	}
	_weapons_script.ReadOpenTable("weapons");

	if (_armor_script.OpenFile("dat/armor.lua", SCRIPT_READ) == false) {
		cerr << "GLOBAL ERROR: could not open script: dat/armor.lua" << endl;
		return false;
	}
	_armor_script.ReadOpenTable("armor");

	return true;
}



void GameGlobal::BindToLua() {
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
			.def("GetElementalAttackBonuses", &GlobalActor::GetElementalAttackBonuses)
			.def("GetStatusAttackBonuses", &GlobalActor::GetStatusAttackBonuses)
			.def("GetElementalDefenseBonuses", &GlobalActor::GetElementalDefenseBonuses)
			.def("GetStatusDefenseBonuses", &GlobalActor::GetStatusDefenseBonuses)

			.def("SetName", &GlobalActor::SetName)
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
			.def("EquipWeapon", &GlobalActor::EquipWeapon)
			.def("EquipArmor", &GlobalActor::EquipArmor)
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
		class_<GlobalCharacterParty>("GlobalCharacterParty")
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
			.def(constructor<>())
	];

	module(ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalWeapon, GlobalObject>("GlobalWeapon")
			.def(constructor<>())
	];

	module(ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalArmor, GlobalObject>("GlobalArmor")
			.def(constructor<>())
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
}

// ****************************************************************************
// ***** GameGlobal class - Character and Party Manipulations
// ****************************************************************************

void GameGlobal::AddCharacter(GlobalCharacter *ch) {
	_characters[ch->GetID()] = ch;
// 	if (_active_party.GetPartySize() < 4) {
// 		_active_party.AddCharacter(ch);
// 	}
}



GlobalCharacter* GameGlobal::GetCharacter(uint32 id) {
	for (uint32 i = 0; i < _characters.size(); i++) {
		if (_characters[i] != 0 && _characters[i]->GetID() == id) {
			return _characters[i];
		}
	}

	if (GLOBAL_DEBUG) cerr << "GLOBAL WARNING: No character matching id #" << id << " found in party" << endl;
	return NULL;
}

// ****************************************************************************
// ***** GameGlobal class - Inventory Manipulations
// ****************************************************************************

void GameGlobal::AddToInventory(GlobalObject *obj) {
	// If the object isn't already in the inventory, insert it
	if (_inventory.find(obj->GetID()) == _inventory.end()) {
		_inventory.insert(make_pair(obj->GetID(), obj));
	}

	// Otherwise increment the count of the object instance already in the inventory
	else {
		_inventory[obj->GetID()]->IncrementCount(obj->GetCount());
		// Delete the object parameter since it is no longer valid
		delete(obj);
	}
} // void GameGlobal::AddToInventory(GlobalObject *obj)



void GameGlobal::RemoveFromInventory(uint32 item_id) {
	if (_inventory.find(item_id) != _inventory.end()) {
		// Delete the object pointer in the inventory and then remove it from the map
		delete _inventory[item_id];
		_inventory.erase(item_id);
	}
} // void GameGlobal::RemoveFromInventory(uint32 item_id)



void GameGlobal::IncrementObjectCount(uint32 item_id, uint32 count) {
	// Do nothing if the item does not exist in the inventory
	if (_inventory.find(item_id) == _inventory.end()) {
		return;
	}

	_inventory[item_id]->IncrementCount(count);
} // void GameGlobal::IncrementObjectCount(uint32 item_id, uint32 count)



void GameGlobal::DecrementObjectCount(uint32 item_id, uint32 count) {
	// Do nothing if the item does not exist in the inventory
	if (_inventory.find(item_id) == _inventory.end()) {
		return;
	}

	// Remove the object from the inventory if the count argument is greater than the number of objects
	if (count >= _inventory[item_id]->GetCount()) {
		delete _inventory[item_id];
		_inventory.erase(item_id);
	}

	// Otherwise, simply decrement the number of objects
	else {
		_inventory[item_id]->DecrementCount(count);
	}
} // void GameGlobal::DecrementObjectCount(uint32 item_id, uint32 count)

} // namespace hoa_global
