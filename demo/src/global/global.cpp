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

#include "utils.h"
#include "global.h"

#include "video.h"
#include "script.h"
#include "system.h"

using namespace std;

using namespace hoa_utils;

using namespace hoa_video;
using namespace hoa_script;
using namespace hoa_system;

template<> hoa_global::GameGlobal* Singleton<hoa_global::GameGlobal>::_singleton_reference = 0;

namespace hoa_global {

GameGlobal* GlobalManager = NULL;
bool GLOBAL_DEBUG = false;

// -----------------------------------------------------------------------------
// GlobalEventGroup class
// -----------------------------------------------------------------------------

void GlobalEventGroup::AddNewEvent(const std::string& event_name, int32 event_value) {
	if (DoesEventExist(event_name) == true) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalEventGroup::AddNewEvent() could not add the event because an event "
				<< "with the name \"" << event_name  << "\" already existed in event group: " << _group_name << endl;
		return;
	}
	_events.insert(make_pair(event_name, event_value));
}



int32 GlobalEventGroup::GetEvent(const std::string& event_name) {
	map<string, int32>::iterator event_iter = _events.find(event_name);
	if (event_iter == _events.end()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalEventGroup::GetEvent() could not retrieve the event value because "
				<< "the event name \"" << event_name  << "\" did not exist in event group: " << _group_name << endl;
		return GLOBAL_BAD_EVENT;
	}
	return event_iter->second;
}



void GlobalEventGroup::SetEvent(const std::string& event_name, int32 event_value) {
	map<string, int32>::iterator event_iter = _events.find(event_name);
	if (event_iter == _events.end()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalEventGroup::SetEvent() could not set the event value because "
				<< "the event name \"" << event_name  << "\" did not exist in event group: " << _group_name << endl;
		return;
	}
	event_iter->second = event_value;
}

// -----------------------------------------------------------------------------
// GameGlobal class - Initialization and Destruction
// -----------------------------------------------------------------------------

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
	if (_items_script.OpenFile("dat/objects/items.lua") == false) {
		return false;
	}
	_items_script.OpenTable("items");

	if (_weapons_script.OpenFile("dat/objects/weapons.lua") == false) {
		return false;
	}
	_weapons_script.OpenTable("weapons");

	if (_armor_script.OpenFile("dat/objects/armor.lua") == false) {
		return false;
	}
	_armor_script.OpenTable("armor");

	if (_attack_skills_script.OpenFile("dat/skills/attack.lua") == false) {
		return false;
	}
	_attack_skills_script.OpenTable("skills");

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
	_character_order.clear();
	_active_party.RemoveAllActors();

	// Delete all event groups
	for (map<string, GlobalEventGroup*>::iterator i = _event_groups.begin(); i != _event_groups.end(); i++) {
		delete (i->second);
	}
	_event_groups.clear();
} // void GameGlobal::ClearAllData()

// -----------------------------------------------------------------------------
// GameGlobal class - Character Functions
// -----------------------------------------------------------------------------

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



void GameGlobal::AddCharacter(GlobalCharacter* ch) {
	if (_characters.find(ch->GetID()) != _characters.end()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: attempted to add a character that already existed" << endl;
		return;
	}

	_characters.insert(make_pair(ch->GetID(), ch));

	// Add the new character to the active party if the active party contains less than four characters
	if (_character_order.size() < 4)
		_active_party.AddActor(ch);

	_character_order.push_back(ch);
} // void GameGlobal::AddCharacter(GlobalCharacter* ch)



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
	if (ch == _characters.end()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: No character matching id #" << id << " found in party" << endl;
		return NULL;
	}

	return (ch->second);

}

// -----------------------------------------------------------------------------
// GameGlobal class - Inventory Functions
// -----------------------------------------------------------------------------

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
		_inventory.insert(make_pair(obj_id, new_obj));
		_inventory_items.push_back(new_obj);
	} else if (obj_id < 20000) { // Weapon
		GlobalWeapon *new_obj = new GlobalWeapon(obj_id, obj_count);
		_inventory.insert(make_pair(obj_id, new_obj));
		_inventory_weapons.push_back(new_obj);
	} else if (obj_id < 30000) { // Head Armor
		GlobalArmor *new_obj = new GlobalArmor(obj_id, obj_count);
		_inventory.insert(make_pair(obj_id, new_obj));
		_inventory_head_armor.push_back(new_obj);
	} else if (obj_id < 40000) { // Torso Armor
		GlobalArmor *new_obj = new GlobalArmor(obj_id, obj_count);
		_inventory.insert(make_pair(obj_id, new_obj));
		_inventory_torso_armor.push_back(new_obj);
	} else if (obj_id < 50000) { // Arm Armor
		GlobalArmor *new_obj = new GlobalArmor(obj_id, obj_count);
		_inventory.insert(make_pair(obj_id, new_obj));
		_inventory_arm_armor.push_back(new_obj);
	} else if (obj_id < 60000) { // Leg Armor
		GlobalArmor *new_obj = new GlobalArmor(obj_id, obj_count);
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

	// Use the id value to figure out what type of object it is, and remove it from the object vector
	if (obj_id == 0) {
		cerr << "GLOBAL WARNING: attempted to remove invalid object to inventory with id: " << obj_id << endl;
	} else if (obj_id < 10000) { // Item
		for (vector<GlobalItem*>::iterator i = _inventory_items.begin(); i != _inventory_items.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_items.erase(i);
				delete _inventory[obj_id];
				_inventory.erase(obj_id);
				return;
			}
		}
		cerr << "GLOBAL WARNING: object to remove was not found in _inventory_items vector" << endl;
	} else if (obj_id < 20000) { // Weapon
		for (vector<GlobalWeapon*>::iterator i = _inventory_weapons.begin(); i != _inventory_weapons.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_weapons.erase(i);
				delete _inventory[obj_id];
				_inventory.erase(obj_id);
				return;
			}
		}
		cerr << "GLOBAL WARNING: object to remove was not found in _inventory_weapons vector" << endl;
	} else if (obj_id < 30000) { // Head Armor
		for (vector<GlobalArmor*>::iterator i = _inventory_head_armor.begin(); i != _inventory_head_armor.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_head_armor.erase(i);
				delete _inventory[obj_id];
				_inventory.erase(obj_id);
				return;
			}
		}
		cerr << "GLOBAL WARNING: object to remove was not found in _inventory_head_armor vector" << endl;
	} else if (obj_id < 40000) { // Torso Armor
		for (vector<GlobalArmor*>::iterator i = _inventory_torso_armor.begin(); i != _inventory_torso_armor.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_torso_armor.erase(i);
				delete _inventory[obj_id];
				_inventory.erase(obj_id);
				return;
			}
		}
		cerr << "GLOBAL WARNING: object to remove was not found in _inventory_torso_armor vector" << endl;
	} else if (obj_id < 50000) { // Arm Armor
		for (vector<GlobalArmor*>::iterator i = _inventory_arm_armor.begin(); i != _inventory_arm_armor.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_arm_armor.erase(i);
				delete _inventory[obj_id];
				_inventory.erase(obj_id);
				return;
			}
		}
		cerr << "GLOBAL WARNING: object to remove was not found in _inventory_arm_armor vector" << endl;
	} else if (obj_id < 60000) { // Leg Armor
		for (vector<GlobalArmor*>::iterator i = _inventory_leg_armor.begin(); i != _inventory_leg_armor.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_leg_armor.erase(i);
				delete _inventory[obj_id];
				_inventory.erase(obj_id);
				return;
			}
		}
		cerr << "GLOBAL WARNING: object to remove was not found in _inventory_leg_armor vector" << endl;
	} else if (obj_id < 70000) { // Shard
		for (vector<GlobalShard*>::iterator i = _inventory_shards.begin(); i != _inventory_shards.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_shards.erase(i);
				delete _inventory[obj_id];
				_inventory.erase(obj_id);
				return;
			}
		}
		cerr << "GLOBAL WARNING: object to remove was not found in _inventory_shards vector" << endl;
	} else if (obj_id < 80000) { // Key Item
		for (vector<GlobalKeyItem*>::iterator i = _inventory_key_items.begin(); i != _inventory_key_items.end(); i++) {
			if ((*i)->GetID() == obj_id) {
				_inventory_key_items.erase(i);
				delete _inventory[obj_id];
				_inventory.erase(obj_id);
				return;
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

// -----------------------------------------------------------------------------
// GameGlobal class - Event Group Functions
// -----------------------------------------------------------------------------

void GameGlobal::AddNewEventGroup(const std::string& group_name) {
	if (DoesEventGroupExist(group_name) == true) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GameGlobal::AddNewEventGroup() failed because there was already an event group "
				<< "name that existed for the requested group name: " << group_name << endl;
		return;
	}

	GlobalEventGroup* geg = new GlobalEventGroup(group_name);
	_event_groups.insert(make_pair(group_name, geg));
}



GlobalEventGroup* GameGlobal::GetEventGroup(const std::string& group_name) {
	map<string, GlobalEventGroup*>::iterator group_iter = _event_groups.find(group_name);
	if (group_iter == _event_groups.end()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GameGlobal::GetEventGroup() could not retrieve the event group because "
				<< "there was no group event corresponding to the group name: " << group_name << endl;
		return NULL;
	}
	return (group_iter->second);
}

// -----------------------------------------------------------------------------
// GameGlobal class - Other Functions
// -----------------------------------------------------------------------------

uint32 GameGlobal::AveragePartyLevel() {
	uint32 xp_level_sum = 0;
	for (uint32 i = 0; i < _active_party.GetPartySize(); i++) {
		GlobalActor* actor = _active_party.GetActor(i);
		xp_level_sum += actor->GetExperienceLevel();
	}
	xp_level_sum /= _active_party.GetPartySize();
	return xp_level_sum;
} // uint32 GameGlobal::AveragePartyLevel()



bool GameGlobal::SaveGame(string& filename) {
	WriteScriptDescriptor file;
	if (file.OpenFile(filename) == false) {
		return false;
	}

	// ----- (1) Save simple play data
	file.InsertNewLine();
	file.WriteUInt("funds", _funds);
	file.WriteUInt("play_hours", SystemManager->GetPlayHours());
	file.WriteUInt("play_minutes", SystemManager->GetPlayMinutes());
	file.WriteUInt("play_seconds", SystemManager->GetPlaySeconds());

	// ----- (2) Save the inventory (object id + object count pairs)
	// NOTE: This does not save any weapons/armor that are equipped on the characters. That data
	// is stored alongside the character data when it is saved
	_SaveInventory(file, "items", _inventory_items);
	_SaveInventory(file, "weapons", _inventory_weapons);
	_SaveInventory(file, "head_armor", _inventory_head_armor);
	_SaveInventory(file, "torso_armor", _inventory_torso_armor);
	_SaveInventory(file, "arm_armor", _inventory_arm_armor);
	_SaveInventory(file, "leg_armor", _inventory_leg_armor);
	_SaveInventory(file, "shards", _inventory_shards);
	_SaveInventory(file, "key_items", _inventory_key_items);

	// ----- (3) Save character data
	file.InsertNewLine();
	file.WriteLine("characters = {");
	// First save the order of the characters in the party
	file.WriteLine("\t[\"order\"] = {");
	for (uint32 i = 0; i < _character_order.size(); i++) {
		if (i == 0)
			file.WriteLine("\t\t" + NumberToString(_character_order[i]->GetID()), false);
		else
			file.WriteLine(", " + NumberToString(_character_order[i]->GetID()), false);
	}
	file.WriteLine("\n\t},");

	// Now save each individual character's data
	for (uint32 i = 0; i < _character_order.size(); i++) {
		if ((i + 1) == _character_order.size())
			_SaveCharacter(file, _character_order[i], true);
		else
			_SaveCharacter(file, _character_order[i], false);
	}
	file.WriteLine("}");

	// ----- (4) Save event data
	file.InsertNewLine();
	file.WriteLine("event_groups = {");
	for (map<string, GlobalEventGroup*>::iterator i = _event_groups.begin(); i != _event_groups.end(); i++) {
		_SaveEvents(file, i->second);
	}
	file.WriteLine("}");

	file.InsertNewLine();
	file.CloseFile();
	return true;
} // bool GameGlobal::SaveGame(string& filename)



bool GameGlobal::LoadGame(const string& filename) {
	ReadScriptDescriptor file;
	if (file.OpenFile(filename) == false) {
		return false;
	}

	// ----- (1) Load play data
	_funds = file.ReadUInt("funds");
	uint8 hours, minutes, seconds;
	hours = file.ReadUInt("play_hours");
	minutes = file.ReadUInt("play_minutes");
	seconds = file.ReadUInt("play_seconds");
	SystemManager->SetPlayTime(hours, minutes, seconds);

	// ----- (2) Load inventory
	_LoadInventory(file, "items");
	_LoadInventory(file, "weapons");
	_LoadInventory(file, "head_armor");
	_LoadInventory(file, "torso_armor");
	_LoadInventory(file, "arm_armor");
	_LoadInventory(file, "leg_armor");
	_LoadInventory(file, "shards");
	_LoadInventory(file, "key_items");

	// ----- (3) Load characters into the party in the correct order
	file.OpenTable("characters");
	vector<uint32> char_ids;
	file.ReadUIntVector("order", char_ids);
	for (uint32 i = 0; i < char_ids.size(); i++) {
		_LoadCharacter(file, char_ids[i]);
	}
	file.CloseTable();

	// ----- (4) Load event data
	vector<string> group_names;
	file.OpenTable("event_groups");
	file.ReadTableKeys(group_names);
	for (uint32 i = 0; i < group_names.size(); i++)
		_LoadEvents(file, group_names[i]);
	file.CloseTable();

	// ----- (5) Report any errors detected from the previous read operations
	if (file.IsErrorDetected()) {
		if (GLOBAL_DEBUG) {
			cerr << "GLOBAL WARNING: GameGlobal::LoadGame ran into errors when reading the game file. They are as follows:" << endl;
			cerr << file.GetErrorMessages() << endl;
			file.ClearErrors();
		}
	}

	file.CloseFile();
	
	return true;
} // bool GameGlobal::LoadGame(string& filename)

// -----------------------------------------------------------------------------
// GameGlobal class - Private Methods
// -----------------------------------------------------------------------------

void GameGlobal::_SaveCharacter(WriteScriptDescriptor& file, GlobalCharacter* character, bool last) {
	if (file.IsFileOpen() == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GameGlobal::_SaveCharacter() failed because the file argument was not an open file" << endl;
		return;
	}
	if (character == NULL) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GameGlobal::_SaveCharacter() failed because the character argument was NULL" << endl;
		return;
	}

	file.WriteLine("\t[" + NumberToString(character->GetID()) + "] = {");

	// ----- (1): Write out the character's stats
	file.WriteLine("\t\texperience_level = " + NumberToString(character->GetExperienceLevel()) + ",");
	file.WriteLine("\t\texperience_points = " + NumberToString(character->GetExperiencePoints()) + ",");
	file.WriteLine("\t\texperience_points_next = " + NumberToString(character->GetExperienceForNextLevel()) + ", ");

	file.WriteLine("\t\tmax_hit_points = " + NumberToString(character->GetMaxHitPoints()) + ",");
	file.WriteLine("\t\thit_points = " + NumberToString(character->GetHitPoints()) + ",");
	file.WriteLine("\t\tmax_skill_points = " + NumberToString(character->GetMaxSkillPoints()) + ",");
	file.WriteLine("\t\tskill_points = " + NumberToString(character->GetSkillPoints()) + ",");

	file.WriteLine("\t\tstrength = " + NumberToString(character->GetStrength()) + ",");
	file.WriteLine("\t\tvigor = " + NumberToString(character->GetVigor()) + ",");
	file.WriteLine("\t\tfortitude = " + NumberToString(character->GetFortitude()) + ",");
	file.WriteLine("\t\tprotection = " + NumberToString(character->GetProtection()) + ",");
	file.WriteLine("\t\tagility = " + NumberToString(character->GetAgility()) + ",");
	file.WriteLine("\t\tevade = " + NumberToString(character->GetEvade()) + ",");

	// ----- (2): Write out the character's equipment
	uint32 weapon_id = 0;
	uint32 head_id = 0;
	uint32 torso_id = 0;
	uint32 arm_id = 0;
	uint32 leg_id = 0;
	GlobalObject *obj_tmp = NULL;

	obj_tmp = character->GetWeaponEquipped();
	if (obj_tmp != NULL)
		weapon_id = obj_tmp->GetID();

	obj_tmp = character->GetHeadArmorEquipped();
	if (obj_tmp != NULL)
		head_id = obj_tmp->GetID();

	obj_tmp = character->GetTorsoArmorEquipped();
	if (obj_tmp != NULL)
		torso_id = obj_tmp->GetID();

	obj_tmp = character->GetArmArmorEquipped();
	if (obj_tmp != NULL)
		arm_id = obj_tmp->GetID();

	obj_tmp = character->GetLegArmorEquipped();
	if (obj_tmp != NULL)
		leg_id = obj_tmp->GetID();

	file.InsertNewLine();
	file.WriteLine("\t\tequipment = {");
	file.WriteLine("\t\t\tweapon = " + NumberToString(weapon_id) + ",");
	file.WriteLine("\t\t\thead_armor = " + NumberToString(head_id) + ",");
	file.WriteLine("\t\t\ttorso_armor = " + NumberToString(torso_id) + ",");
	file.WriteLine("\t\t\tarm_armor = " + NumberToString(arm_id) + ",");
	file.WriteLine("\t\t\tleg_armor = " + NumberToString(leg_id));
	file.WriteLine("\t\t},");

	// ----- (3): Write out the character's skills
	std::vector<GlobalSkill*>* skill_vector;

	file.InsertNewLine();
	file.WriteLine("\t\tattack_skills = {");
	skill_vector = character->GetAttackSkills();
	for (uint32 i = 0; i < skill_vector->size(); i++) {
		if (i == 0)
			file.WriteLine("\t\t\t", false);
		else
			file.WriteLine(", ", false);
		file.WriteLine(NumberToString(skill_vector->at(i)->GetID()), false);
	}
	file.WriteLine("\n\t\t},");

	file.InsertNewLine();
	file.WriteLine("\t\tdefense_skills = {");
	skill_vector = character->GetDefenseSkills();
	for (uint32 i = 0; i < skill_vector->size(); i++) {
		if (i == 0)
			file.WriteLine("\t\t\t", false);
		else
			file.WriteLine(", ", false);
		file.WriteLine(NumberToString(skill_vector->at(i)->GetID()), false);
	}
	file.WriteLine("\n\t\t},");

	file.InsertNewLine();
	file.WriteLine("\t\tsupport_skills = {");
	skill_vector = character->GetSupportSkills();
	for (uint32 i = 0; i < skill_vector->size(); i++) {
		if (i == 0)
			file.WriteLine("\t\t\t", false);
		else
			file.WriteLine(", ", false);
		file.WriteLine(NumberToString(skill_vector->at(i)->GetID()), false);
	}
	file.WriteLine("\n\t\t}");

	if (last)
		file.WriteLine("\t}");
	else
		file.WriteLine("\t},");
} // void GameGlobal::_SaveCharacter(WriteScriptDescriptor& file, GlobalCharacter* character, bool last)



void GameGlobal::_SaveEvents(hoa_script::WriteScriptDescriptor& file, GlobalEventGroup* event_group) {
	if (file.IsFileOpen() == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GameGlobal::_SaveEvents() failed because the file passed to it was not open" << endl;
		return;
	}

	if (event_group == NULL) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GameGlobal::_SaveEvents() failed because the a NULL event group was passed to it" << endl;
		return;
	}

	file.WriteLine("\t" + event_group->GetGroupName() + " = {");

	for (map<string, int32>::iterator i = event_group->_events.begin(); i != event_group->_events.end(); i++) {
		if (i == event_group->_events.begin())
			file.WriteLine("\t\t", false);
		else
			file.WriteLine(", ", false);
		file.WriteLine("[\"" + i->first + "\"] = " + NumberToString(i->second), false);
	}
	file.WriteLine("\t}");
} // GameGlobal::_SaveEvents(hoa_script::WriteScriptDescriptor& file, GlobalEventGroup* event_group)



void GameGlobal::_LoadInventory(hoa_script::ReadScriptDescriptor& file, std::string category_name) {
	if (file.IsFileOpen() == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GameGlobal::_LoadInventory() failed because the file passed to it was not open" << endl;
		return;
	}

	vector<uint32> object_ids;

	// The table keys are the inventory object ID numbers. The value of each key is the count of that object
	file.OpenTable(category_name);
	file.ReadTableKeys(object_ids);
	for (uint32 i = 0; i < object_ids.size(); i++) {
		AddToInventory(object_ids[i], file.ReadUInt(object_ids[i]));
	}
	file.CloseTable();
} // void GameGlobal::_LoadInventory(hoa_script::ReadScriptDescriptor& file, std::string category_name)



void GameGlobal::_LoadCharacter(hoa_script::ReadScriptDescriptor& file, uint32 id) {
	if (file.IsFileOpen() == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GameGlobal::_LoadCharacter() failed because the file passed to it was not open" << endl;
		return;
	}

	// ----- (1): Create a new GlobalCharacter object using the provided id
	// This loads all of the character's "static" data, such as their name, etc.
	GlobalCharacter* character = new GlobalCharacter(id);

	// This function assumes that the characters table in the saved game file is already open.
	// So all we need to open is the character's table
	file.OpenTable(id);

	// ----- (2): Read in all of the character's stats data
	character->SetExperienceLevel(file.ReadUInt("experience_level"));
	character->SetExperiencePoints(file.ReadUInt("experience_points"));
	character->SetExperienceNextLevel(file.ReadUInt("experience_points_next"));

	character->SetMaxHitPoints(file.ReadUInt("max_hit_points"));
	character->SetHitPoints(file.ReadUInt("hit_points"));
	character->SetMaxSkillPoints(file.ReadUInt("max_skill_points"));
	character->SetSkillPoints(file.ReadUInt("skill_points"));

	character->SetStrength(file.ReadUInt("strength"));
	character->SetVigor(file.ReadUInt("vigor"));
	character->SetFortitude(file.ReadUInt("fortitude"));
	character->SetProtection(file.ReadUInt("protection"));
	character->SetAgility(file.ReadUInt("agility"));
	character->SetEvade(file.ReadUInt("evade"));

	// ----- (3): Read the character's equipment and load it onto the character
	file.OpenTable("equipment");
	uint32 equip_id;

	// Equip the objects on the character as long as valid equipment IDs were read
	equip_id = file.ReadUInt("weapon");
	if (equip_id != 0) {
		character->EquipWeapon(new GlobalWeapon(equip_id));
	}

	equip_id = file.ReadUInt("head_armor");
	if (equip_id != 0) {
		character->EquipArmor(new GlobalArmor(equip_id));
	}

	equip_id = file.ReadUInt("torso_armor");
	if (equip_id != 0) {
		character->EquipArmor(new GlobalArmor(equip_id));
	}

	equip_id = file.ReadUInt("arm_armor");
	if (equip_id != 0) {
		character->EquipArmor(new GlobalArmor(equip_id));
	}

	equip_id = file.ReadUInt("leg_armor");
	if (equip_id != 0) {
		character->EquipArmor(new GlobalArmor(equip_id));
	}

	file.CloseTable();

	// ----- (4): Read the character's skills and pass those onto the character object
	vector<uint32> skill_ids;

	skill_ids.clear();
	file.ReadUIntVector("attack_skills", skill_ids);
	for (uint32 i = 0; i < skill_ids.size(); i++) {
		character->AddSkill(skill_ids[i]);
	}

	skill_ids.clear();
	file.ReadUIntVector("defense_skills", skill_ids);
	for (uint32 i = 0; i < skill_ids.size(); i++) {
		character->AddSkill(skill_ids[i]);
	}

	skill_ids.clear();
	file.ReadUIntVector("support_skills", skill_ids);
	for (uint32 i = 0; i < skill_ids.size(); i++) {
		character->AddSkill(skill_ids[i]);
	}

	file.CloseTable();

	AddCharacter(character);
} // void GameGlobal::_LoadCharacter(hoa_script::ReadScriptDescriptor& file, uint32 id);



void GameGlobal::_LoadEvents(hoa_script::ReadScriptDescriptor& file, const std::string& group_name) {
	if (file.IsFileOpen() == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GameGlobal::_LoadEvents() failed because the file passed to it was not open" << endl;
		return;
	}

	AddNewEventGroup(group_name);
	GlobalEventGroup* new_group = GetEventGroup(group_name); // new_group is guaranteed not to be NULL

	vector<string> event_names;

	file.OpenTable(group_name);
	file.ReadTableKeys(event_names);
	for (uint32 i = 0; i < event_names.size(); i++) {
		new_group->AddNewEvent(event_names[i], file.ReadInt(event_names[i]));
	}
	file.CloseTable();
} // void GameGlobal::_LoadEvents(hoa_script::ReadScriptDescriptor& file, const std::string& group_name)

} // namespace hoa_global
