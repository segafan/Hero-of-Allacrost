////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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
	_attack_skills_script.CloseFile();
}



bool GameGlobal::SingletonInitialize() {
	// Open up the persistent script files
	if (_items_script.OpenFile("dat/objects/items.lua", SCRIPT_READ) == false) {
		return false;
	}
	_items_script.ReadOpenTable("items");

	if (_weapons_script.OpenFile("dat/objects/weapons.lua", SCRIPT_READ) == false) {
		return false;
	}
	_weapons_script.ReadOpenTable("weapons");

	if (_armor_script.OpenFile("dat/objects/armor.lua", SCRIPT_READ) == false) {
		return false;
	}
	_armor_script.ReadOpenTable("armor");

	if (_attack_skills_script.OpenFile("dat/skills/attack.lua", SCRIPT_READ) == false) {
		return false;
	}
	_attack_skills_script.ReadOpenTable("skills");

	return true;
}



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
	_inventory_shards.clear();
	_inventory_key_items.clear();

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

	// Add the new character to the active party if the active party contains less than four characters
	if (_character_order.size() < 4)
		_active_party.AddActor(ch);

	_character_order.push_back(ch);
} // void GameGlobal::AddCharacter(uint32 id)



void GameGlobal::RemoveCharacter(uint32 id) {
	map<uint32, GlobalCharacter*>::iterator ch = _characters.find(id);

	if (ch == _characters.end()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: attempted to remove a character that did not exist" << endl;
		return;
	}

	delete(ch->second);
	_characters.erase(ch);

	for (vector<GlobalCharacter*>::iterator i = _character_order.begin(); i != _character_order.end(); i++) {
		if ((*i)->GetID() == id) {
			_character_order.erase(i);

			// Reform the active party, in case the removed character was a member of it
			_active_party.RemoveAllActors();
			for (uint32 j = 0; j < 4 || j >= _character_order.size(); j++) {
				_active_party.AddActor(_character_order[j]);
			}
		}
	}
} // void GameGlobal::RemoveCharacter(uint32 id)



GlobalCharacter* GameGlobal::GetCharacter(uint32 id) {
	map<uint32, GlobalCharacter*>::iterator ch = _characters.find(id);
	if (ch != _characters.end()) {
		return (ch->second);
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
		for (vector<GlobalItem*>::iterator i = _inventory_items.begin(); i != _inventory_items.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_items.erase(i);
				return;
			}
		}
		cerr << "GLOBAL WARNING: object to remove was not found in _inventory_items vector" << endl;
	} else if (obj_id < 20000) { // Weapon
		for (vector<GlobalWeapon*>::iterator i = _inventory_weapons.begin(); i != _inventory_weapons.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_weapons.erase(i);
				return;
			}
		}
		cerr << "GLOBAL WARNING: object to remove was not found in _inventory_weapons vector" << endl;
	} else if (obj_id < 30000) { // Head Armor
		for (vector<GlobalArmor*>::iterator i = _inventory_head_armor.begin(); i != _inventory_head_armor.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_head_armor.erase(i);
				return;
			}
		}
		cerr << "GLOBAL WARNING: object to remove was not found in _inventory_head_armor vector" << endl;
	} else if (obj_id < 40000) { // Torso Armor
		for (vector<GlobalArmor*>::iterator i = _inventory_torso_armor.begin(); i != _inventory_torso_armor.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_torso_armor.erase(i);
				return;
			}
		}
		cerr << "GLOBAL WARNING: object to remove was not found in _inventory_torso_armor vector" << endl;
	} else if (obj_id < 50000) { // Arm Armor
		for (vector<GlobalArmor*>::iterator i = _inventory_arm_armor.begin(); i != _inventory_arm_armor.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_arm_armor.erase(i);
				return;
			}
		}
		cerr << "GLOBAL WARNING: object to remove was not found in _inventory_arm_armor vector" << endl;
	} else if (obj_id < 60000) { // Leg Armor
		for (vector<GlobalArmor*>::iterator i = _inventory_leg_armor.begin(); i != _inventory_leg_armor.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_leg_armor.erase(i);
				return;
			}
		}
		cerr << "GLOBAL WARNING: object to remove was not found in _inventory_leg_armor vector" << endl;
	} else if (obj_id < 70000) { // Shard
		for (vector<GlobalShard*>::iterator i = _inventory_shards.begin(); i != _inventory_shards.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_shards.erase(i);
				return;
			}
		}
		cerr << "GLOBAL WARNING: object to remove was not found in _inventory_shards vector" << endl;
	} else if (obj_id < 80000) { // Key Item
		for (vector<GlobalKeyItem*>::iterator i = _inventory_key_items.begin(); i != _inventory_key_items.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_key_items.erase(i);
				break;
			}
		}
		cerr << "GLOBAL WARNING: object to remove was not found in _inventory_key_items vector" << endl;
	} else {
		cerr << "GLOBAL WARNING: attempted to remove invalid object from inventory with id: " << obj_id << endl;
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
