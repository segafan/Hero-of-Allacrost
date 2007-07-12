////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    global_actors.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for global game actors.
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

// -----------------------------------------------------------------------------
// GlobalAttackPoint class
// -----------------------------------------------------------------------------

bool GlobalAttackPoint::_LoadData(ReadScriptDescriptor& script) {
	if (script.IsFileOpen() == false) {
		return false;
	}

	_name = MakeUnicodeString(script.ReadString("name"));
	_x_position = script.ReadInt("x_position");
	_y_position = script.ReadInt("y_position");
	_fortitude_modifier = script.ReadFloat("fortitude_modifier");
	_protection_modifier = script.ReadFloat("protection_modifier");
	_evade_modifier = script.ReadFloat("evade_modifier");

	if (script.IsErrorDetected()) {
		if (GLOBAL_DEBUG) {
			cerr << "GLOBAL ERROR: GlobalAttackPoint::LoadData() failed due to script reading errors. "
				<< "They are as follows:" << endl;
			cerr << script.GetErrorMessages() << endl;
		}
		return false;
	}

	return true;
} // bool GlobalAttackPoint::_LoadData(ReadScriptDescriptor& script)

// -----------------------------------------------------------------------------
// GlobalActor class
// -----------------------------------------------------------------------------

GlobalActor::~GlobalActor() {
	// Delete all attack points
	for (uint32 i = 0; i < _attack_points.size(); i++) {
		delete _attack_points[i];
	}
	_attack_points.clear();

	// Delete all equipment
	if (_weapon_equipped != NULL)
		delete _weapon_equipped;
	for (uint32 i = 0; i < _armor_equipped.size(); i++) {
		if (_armor_equipped[i] != NULL)
			delete _armor_equipped[i];
	}
	_armor_equipped.clear();

	// Delete all skills
	for (map<uint32, GlobalSkill*>::iterator i = _skills.begin(); i != _skills.end(); i++) {
		delete i->second;
	}
	_skills.clear();
}



GlobalActor::GlobalActor(const GlobalActor& copy) {
	_id = copy._id;
	_name = copy._name;
	_filename = copy._filename;
	_experience_level = copy._experience_level;
	_experience_points = copy._experience_points;
	_hit_points = copy._hit_points;
	_max_hit_points = copy._max_hit_points;
	_skill_points = copy._skill_points;
	_max_skill_points = copy._max_skill_points;
	_strength = copy._strength;
	_vigor = copy._vigor;
	_fortitude = copy._fortitude;
	_protection = copy._protection;
	_agility = copy._agility;
	_evade = copy._evade;
	_total_physical_attack = copy._total_physical_attack;
	_total_metaphysical_attack = copy._total_metaphysical_attack;

	// Copy all attack points
	for (uint32 i = 0; i < copy._attack_points.size(); i++) {
		_attack_points.push_back(new GlobalAttackPoint(*copy._attack_points[i]));
		_attack_points[i]->_actor_owner = this;
	}

	// Copy all equipment that is not NULL
	if (copy._weapon_equipped == NULL)
		_weapon_equipped = NULL;
	else
		_weapon_equipped = new GlobalWeapon(*copy._weapon_equipped);

	for (uint32 i = 0; i < _armor_equipped.size(); i++) {
		if (_armor_equipped[i] == NULL)
			_armor_equipped.push_back(NULL);
		else
			_armor_equipped.push_back(new GlobalArmor(*copy._armor_equipped[i]));
	}

	// Copy all skills
	for (map<uint32, GlobalSkill*>::const_iterator i = copy._skills.begin(); i != copy._skills.end(); i++) {
		_skills.insert(make_pair(i->first, new GlobalSkill(*(i->second))));
	}
} // GlobalActor::GlobalActor(const GlobalActor& copy)



GlobalActor& GlobalActor::operator=(const GlobalActor& copy) {
	cout << "GlobalActor assignment operator" << endl;
	if (this == &copy) // Handle self-assignment case
		return *this;

	_id = copy._id;
	_name = copy._name;
	_filename = copy._filename;
	_experience_level = copy._experience_level;
	_experience_points = copy._experience_points;
	_hit_points = copy._hit_points;
	_max_hit_points = copy._max_hit_points;
	_skill_points = copy._skill_points;
	_max_skill_points = copy._max_skill_points;
	_strength = copy._strength;
	_vigor = copy._vigor;
	_fortitude = copy._fortitude;
	_protection = copy._protection;
	_agility = copy._agility;
	_evade = copy._evade;
	_total_physical_attack = copy._total_physical_attack;
	_total_metaphysical_attack = copy._total_metaphysical_attack;

	// Copy all attack points
	for (uint32 i = 0; i < copy._attack_points.size(); i++) {
		_attack_points.push_back(new GlobalAttackPoint(*_attack_points[i]));
		_attack_points[i]->_actor_owner = this;
	}

	// Copy all equipment that is not NULL
	if (copy._weapon_equipped == NULL)
		_weapon_equipped = NULL;
	else
		_weapon_equipped = new GlobalWeapon(*copy._weapon_equipped);

	for (uint32 i = 0; i < _armor_equipped.size(); i++) {
		if (_armor_equipped[i] == NULL)
			_armor_equipped.push_back(NULL);
		else
			_armor_equipped.push_back(new GlobalArmor(*copy._armor_equipped[i]));
	}

	// Copy all skills
	for (map<uint32, GlobalSkill*>::const_iterator i = copy._skills.begin(); i != copy._skills.end(); i++) {
		_skills.insert(make_pair(i->first, new GlobalSkill(*(i->second))));
	}
	return *this;
} // GlobalActor& GlobalActor::operator=(const GlobalActor& copy)



uint32 GlobalActor::GetTotalPhysicalDefense(uint32 index) const {
	if (index >= _attack_points.size()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalActor::GetTotalPhysicalDefense() was called with an invalid "
				<< "index argument: " << index << endl;
		return 0;
	}
	else {
		return _attack_points[index]->GetTotalPhysicalDefense();
	}
}



uint32 GlobalActor::GetTotalMetaphysicalDefense(uint32 index) const {
	if (index >= _attack_points.size()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalActor::GetTotalMetaphysicalDefense() was called with an invalid "
				<< "index argument: " << index << endl;
		return 0;
	}
	else {
		return _attack_points[index]->GetTotalMetaphysicalDefense();
	}
}



float GlobalActor::GetTotalEvadeRating(uint32 index) const {
	if (index >= _attack_points.size()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalActor::GetTotalEvadeRating() was called with an invalid "
				<< "index argument: " << index << endl;
		return 0.0f;
	}
	else {
		return _attack_points[index]->GetTotalEvadeRating();
	}
}



GlobalArmor* GlobalActor::GetArmorEquipped(uint32 index) const {
	if (index >= _armor_equipped.size()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalActor::GetArmorEquipped() was called with an invalid "
				<< "index argument: " << index << endl;
		return NULL;
	}
	else {
		return _armor_equipped[index];
	}
}



GlobalAttackPoint* GlobalActor::GetAttackPoint(uint32 index) const {
	if (index >= _attack_points.size()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalActor::GetAttackPoint() was called with an invalid "
				<< "index argument: " << index << endl;
		return NULL;
	}
	else {
		return _attack_points[index];
	}
}



GlobalSkill* GlobalActor::GetSkill(uint32 skill_id) const {
	map<uint32, GlobalSkill*>::const_iterator skill_location = _skills.find(skill_id);
	if (skill_location != _skills.end())
		return skill_location->second;
	else
		return NULL;
}



GlobalSkill* GlobalActor::GetSkill(const GlobalSkill* skill) const {
	if (skill == NULL) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalActor::GetSkill() was called with a NULL skill argument" << endl;
		return NULL;
	}
	return GetSkill(skill->GetID());
}



void GlobalActor::_CalculateAttackRatings() {
	_total_physical_attack = _strength;
	_total_metaphysical_attack = _vigor;

	if (_weapon_equipped != NULL) {
		_total_physical_attack += _weapon_equipped->GetPhysicalAttack();
		_total_metaphysical_attack += _weapon_equipped->GetMetaphysicalAttack();
	}
}



void GlobalActor::_CalculateDefenseRatings() {
	// Re-calculate the defense ratings for all attack points
	for (uint32 i = 0; i < _attack_points.size(); i++) {
		// If the modifier is -100% or less, set the total defense to zero
		_attack_points[i]->_total_physical_defense = 0;
		if (_attack_points[i]->_fortitude_modifier > -1.0f)
			_attack_points[i]->_total_physical_defense = _fortitude + static_cast<uint32>(_fortitude * _attack_points[i]->_fortitude_modifier);

		_attack_points[i]->_total_metaphysical_defense = 0;
		if (_attack_points[i]->_protection_modifier > -1.0f)
			_attack_points[i]->_total_metaphysical_defense = _protection + static_cast<uint32>(_protection * _attack_points[i]->_protection_modifier);

		// If there's armor equipped on this attack point add its defensive properties to the defense totals
		if (_armor_equipped[i] != NULL) {
			_attack_points[i]->_total_physical_defense += _armor_equipped[i]->GetPhysicalDefense();
			_attack_points[i]->_total_metaphysical_defense += _armor_equipped[i]->GetMetaphysicalDefense();
		}
	}
} // void GlobalActor::_CalculateDefenseRatings()



void GlobalActor::_CalculateEvadeRatings() {
	// Re-calculate the evade ratings for all attack points
	for (uint32 i = 0; i < _attack_points.size(); i++) {
		// If the modifier is -100% or less, set the evade rating to zero
		_attack_points[i]->_total_evade_rating = 0;
		if (_attack_points[i]->_evade_modifier > -1.0f)
			_attack_points[i]->_total_evade_rating = _evade + static_cast<uint32>(_evade * _attack_points[i]->_evade_modifier);
	}
} // void GlobalActor::_CalculateEvadeRatings()



GlobalWeapon* GlobalActor::EquipWeapon(GlobalWeapon* weapon) {
	GlobalWeapon* old_weapon = _weapon_equipped;
	_weapon_equipped = weapon;
	_CalculateAttackRatings();
	return old_weapon;
}



GlobalArmor* GlobalActor::EquipArmor(GlobalArmor* armor, uint32 index) {
	if (index >= _armor_equipped.size()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: GlobalActor::EquipArmor() was given an invalid index: " << index
				<< ", the armor was not equipped" << endl;
		return armor;
	}

	GlobalArmor* old_armor = _armor_equipped[index];
	_armor_equipped[index] = armor;

	if (old_armor != NULL && armor != NULL) {
		if (old_armor->GetObjectType() != armor->GetObjectType()) {
			if (GLOBAL_DEBUG)
				cerr << "GLOBAL WARNING: GlobalActor::EquipArmor() replaced the old armor "
					<< "with a different type of armor" << endl;
		}
	}
	_CalculateDefenseRatings();
	return old_armor;
}

// -----------------------------------------------------------------------------
// GlobalCharacterGrowth class
// -----------------------------------------------------------------------------

GlobalCharacterGrowth::~GlobalCharacterGrowth() {
	for (list<GlobalSkill*>::iterator i = _skills_learned.begin(); i != _skills_learned.end();) {
		delete (*i);
		i = _skills_learned.erase(i);
	}
}



void GlobalCharacterGrowth::AcknowledgeGrowth() {
	if (_growth_detected == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalCharacterGrowth::AcknowledgeGrowth() was invoked when there was no character "
				<< "growth detected" << endl;
		return;
	}

	_growth_detected = false;
	_hit_points_growth = 0;
	_skill_points_growth = 0;
	_strength_growth = 0;
	_vigor_growth = 0;
	_fortitude_growth = 0;
	_protection_growth = 0;
	_agility_growth = 0;
	_evade_growth = 0.0f;

	// If a new experience level has been gained, we must retrieve the growth data for the new experience level
	if (_experience_level_gained) {
		_character_owner->_experience_level += 1;
		_experience_level_gained = false;
		_DetermineNextLevelExperience();

		ReadScriptDescriptor character_script;
		if (character_script.OpenFile("dat/characters.lua") == false) {
			if (GLOBAL_DEBUG)
				cerr << "GLOBAL ERROR: GlobalCharacterGrowth::AcknowledgeGrowth() failed to open the script file containing the "
					<< "character's definition when the character reached a new experience level" << endl;
			return;
		}

		try {
			ScriptCallFunction<void>(character_script.GetLuaState(), "DetermineGrowth", this);
			_ConstructPeriodicGrowth();
			_CheckForGrowth();
		}
		catch (luabind::error e) {
			ScriptManager->HandleLuaError(e);
		}
		catch (luabind::cast_failed e) {
			ScriptManager->HandleCastError(e);
		}
	} // if (_experience_level_gained)
} // void GlobalCharacterGrowth::AcknowledgeGrowth()



void GlobalCharacterGrowth::_AddSkill(uint32 skill_id) {
	if (skill_id == 0) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalCharacterGrowth::_AddSkill() failed because an invalid skill id was passed to it (0)" << endl;
		return;
	}

	// Make sure we don't add a skill to learn more than once
	for (list<GlobalSkill*>::iterator i = _skills_learned.begin(); i != _skills_learned.end(); i++) {
		if (skill_id == (*i)->GetID()) {
			if (GLOBAL_DEBUG)
				cerr << "GLOBAL WARNING: GlobalCharacterGrowth::_AddSkill() failed because the skill to be added was already "
					<< "present in the list of skills to earn. The skill ID in question was: " << skill_id << endl;
			return;
		}
	}

	GlobalSkill* skill = new GlobalSkill(skill_id);
	if (skill->GetID() == 0) { // Indicates that the skill failed to load successfully
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalCharacterGrowth::_AddSkill() failed because the skill failed to load" << endl;
		delete skill;
	}
	else {
		_skills_learned.push_back(skill);
	}
} // void GlobalCharacterGrowth::_AddSkill(uint32 skill_id)



void GlobalCharacterGrowth::_CheckForGrowth() {
	// ----- (1): If a new experience level is gained, empty the periodic growth containers into the growth members
	if (_character_owner->GetExperiencePoints() >= _experience_for_next_level) {
		_experience_level_gained = true;
		_growth_detected = true;

		for (uint32 i = 0; i < _hit_points_periodic_growth.size(); i++)
			_hit_points_growth += _hit_points_periodic_growth[i].second;
		_hit_points_periodic_growth.clear();

		for (uint32 i = 0; i < _skill_points_periodic_growth.size(); i++)
			_skill_points_growth += _skill_points_periodic_growth[i].second;
		_skill_points_periodic_growth.clear();

		for (uint32 i = 0; i < _strength_periodic_growth.size(); i++)
			_strength_growth += _strength_periodic_growth[i].second;
		_strength_periodic_growth.clear();

		for (uint32 i = 0; i < _vigor_periodic_growth.size(); i++)
			_vigor_growth += _vigor_periodic_growth[i].second;
		_vigor_periodic_growth.clear();

		for (uint32 i = 0; i < _fortitude_periodic_growth.size(); i++)
			_fortitude_growth += _fortitude_periodic_growth[i].second;
		_fortitude_periodic_growth.clear();

		for (uint32 i = 0; i < _protection_periodic_growth.size(); i++)
			_protection_growth += _protection_periodic_growth[i].second;
		_protection_periodic_growth.clear();

		for (uint32 i = 0; i < _agility_periodic_growth.size(); i++)
			_agility_growth += _agility_periodic_growth[i].second;
		_agility_periodic_growth.clear();

		for (uint32 i = 0; i < _evade_periodic_growth.size(); i++)
			_evade_growth += _evade_periodic_growth[i].second;
		_evade_periodic_growth.clear();

		// Make a run through the skills to learn and make sure that the character does not already know that skill
		for (list<GlobalSkill*>::iterator i = _skills_learned.begin(); i != _skills_learned.end();) {
			if (_character_owner->GetSkill(*i) == NULL) { // If true then the skill to learn is not already known by the character
				i++;
			}
			else {
				if (GLOBAL_DEBUG)
					cerr << "GLOBAL WARNING: GlobalCharacterGrowth::_CheckForGrowth() detected that the character already knew the skill "
						<< "that was to be learned on the next experience level. The ID of that skill was: " << (*i)->GetID() << endl;
				delete *i;
				i = _skills_learned.erase(i);
			}
		}
		
		return;
	} // if (_actor_ower->GetExperiencePoints() >= _experience_for_next_level)

	// ----- (2): If there is no growth detected, check all periodic growth containers
	switch (_growth_detected) { // switch statement used instead of if statement here so we can break out of it early
		case true:
			break;
		case false:
			if (_hit_points_periodic_growth.empty() == false) {
				if (_character_owner->GetExperiencePoints() >= _hit_points_periodic_growth.front().first) {
					_growth_detected = true;
					break;
				}
			}

			if (_skill_points_periodic_growth.empty() == false) {
				if (_character_owner->GetExperiencePoints() >= _skill_points_periodic_growth.front().first) {
					_growth_detected = true;
					break;
				}
			}

			if (_strength_periodic_growth.empty() == false) {
				if (_character_owner->GetExperiencePoints() >= _strength_periodic_growth.front().first) {
					_growth_detected = true;
					break;
				}
			}

			if (_vigor_periodic_growth.empty() == false) {
				if (_character_owner->GetExperiencePoints() >= _vigor_periodic_growth.front().first) {
					_growth_detected = true;
					break;
				}
			}

			if (_fortitude_periodic_growth.empty() == false) {
				if (_character_owner->GetExperiencePoints() >= _fortitude_periodic_growth.front().first) {
					_growth_detected = true;
					break;
				}
			}

			if (_protection_periodic_growth.empty() == false) {
				if (_character_owner->GetExperiencePoints() >= _protection_periodic_growth.front().first) {
					_growth_detected = true;
					break;
				}
			}

			if (_agility_periodic_growth.empty() == false) {
				if (_character_owner->GetExperiencePoints() >= _agility_periodic_growth.front().first) {
					_growth_detected = true;
					break;
				}
			}

			if (_evade_periodic_growth.empty() == false) {
				if (_character_owner->GetExperiencePoints() >= _evade_periodic_growth.front().first) {
					_growth_detected = true;
					break;
				}
			}
			break;
	} // switch (_growth_detected)

	// ----- (3): If there is no growth detected update all periodic growth containers
	if (_growth_detected == true) {
		while (_hit_points_periodic_growth.begin() != _hit_points_periodic_growth.end()) {
			if (_character_owner->GetExperiencePoints() >= _hit_points_periodic_growth.begin()->first) {
				_hit_points_growth += _hit_points_periodic_growth.begin()->second;
				_hit_points_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}

		while (_skill_points_periodic_growth.begin() != _skill_points_periodic_growth.end()) {
			if (_character_owner->GetExperiencePoints() >= _skill_points_periodic_growth.begin()->first) {
				_skill_points_growth += _skill_points_periodic_growth.begin()->second;
				_skill_points_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}

		while (_strength_periodic_growth.begin() != _strength_periodic_growth.end()) {
			if (_character_owner->GetExperiencePoints() >= _strength_periodic_growth.begin()->first) {
				_strength_growth += _strength_periodic_growth.begin()->second;
				_strength_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}

		while (_vigor_periodic_growth.begin() != _vigor_periodic_growth.end()) {
			if (_character_owner->GetExperiencePoints() >= _vigor_periodic_growth.begin()->first) {
				_vigor_growth += _vigor_periodic_growth.begin()->second;
				_vigor_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}

		while (_fortitude_periodic_growth.begin() != _fortitude_periodic_growth.end()) {
			if (_character_owner->GetExperiencePoints() >= _fortitude_periodic_growth.begin()->first) {
				_fortitude_growth += _fortitude_periodic_growth.begin()->second;
				_fortitude_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}

		while (_protection_periodic_growth.begin() != _protection_periodic_growth.end()) {
			if (_character_owner->GetExperiencePoints() >= _protection_periodic_growth.begin()->first) {
				_protection_growth += _protection_periodic_growth.begin()->second;
				_protection_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}

		while (_agility_periodic_growth.begin() != _agility_periodic_growth.end()) {
			if (_character_owner->GetExperiencePoints() >= _agility_periodic_growth.begin()->first) {
				_agility_growth += _agility_periodic_growth.begin()->second;
				_agility_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}

		while (_evade_periodic_growth.begin() != _evade_periodic_growth.end()) {
			if (_character_owner->GetExperiencePoints() >= _evade_periodic_growth.begin()->first) {
				_evade_growth += _evade_periodic_growth.begin()->second;
				_evade_periodic_growth.pop_front();
			}
			else {
				break;
			}
		}
	} // if (_growth_detected == true)
} // void GlobalCharacterGrowth::_CheckForGrowth()



void GlobalCharacterGrowth::_ConstructPeriodicGrowth() {
	// TODO: Implement a gradual growth algorithm

	// TEMP: all growth is done when the experience level is gained
	_hit_points_periodic_growth.push_back(make_pair(_experience_for_next_level, _hit_points_growth));
	_skill_points_periodic_growth.push_back(make_pair(_experience_for_next_level, _skill_points_growth));
	_strength_periodic_growth.push_back(make_pair(_experience_for_next_level, _strength_growth));
	_vigor_periodic_growth.push_back(make_pair(_experience_for_next_level, _vigor_growth));
	_fortitude_periodic_growth.push_back(make_pair(_experience_for_next_level, _fortitude_growth));
	_protection_periodic_growth.push_back(make_pair(_experience_for_next_level, _protection_growth));
	_agility_periodic_growth.push_back(make_pair(_experience_for_next_level, _agility_growth));
	_evade_periodic_growth.push_back(make_pair(_experience_for_next_level, _evade_growth));

	_hit_points_growth = 0;
	_skill_points_growth = 0;
	_strength_growth = 0;
	_vigor_growth = 0;
	_fortitude_growth = 0;
	_protection_growth = 0;
	_agility_growth = 0;
	_evade_growth = 0.0f;
} // void GlobalCharacterGrowth::_ConstructPeriodicGrowth()



void GlobalCharacterGrowth::_DetermineNextLevelExperience() {
	uint32 new_xp = 0; // Temporary variable for holding the new experience milestone

	// TODO: implement a real algorithm for determining the next experience goal
	new_xp = _experience_for_last_level + 50;

	_experience_for_last_level = _experience_for_next_level;
	_experience_for_next_level = new_xp;
} // void GlobalCharacterGrowth::_DetermineNextLevelExperience()

// -----------------------------------------------------------------------------
// GlobalCharacter class
// -----------------------------------------------------------------------------

GlobalCharacter::GlobalCharacter(uint32 id, bool initial) :
	_growth(this)
{
	_id = id;

	// (1): Attempt to open the characters script file
	ReadScriptDescriptor char_script;
	if (char_script.OpenFile("dat/actors/characters.lua") == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: GlobalCharacter constructor could not create a new character because "
				<< "the script file dat/actors/character.lua did not open succesfully" << endl;
		// Set the character's ID to zero since this is now an invalid character object
		_id = 0;
		return;
	}

	// (2): Open up the table containing the character's data and retrieve their basic properties
	char_script.OpenTable("characters");
	char_script.OpenTable(_id);
	_name = MakeUnicodeString(char_script.ReadString("name"));
	_filename = char_script.ReadString("filename");

	// (3): Constructing the character from their initial stats if requested
	if (initial) {
		char_script.OpenTable("initial_stats");
		_experience_level = char_script.ReadUInt("experience_level");
		_experience_points = char_script.ReadUInt("experience_points");
		_max_hit_points = char_script.ReadUInt("max_hit_points");
		_hit_points = _max_hit_points;
		_max_skill_points = char_script.ReadUInt("max_skill_points");
		_skill_points = _max_skill_points;
		_strength = char_script.ReadUInt("strength");
		_vigor = char_script.ReadUInt("vigor");
		_fortitude = char_script.ReadUInt("fortitude");
		_protection = char_script.ReadUInt("protection");
		_agility = char_script.ReadUInt("agility");
		_evade = char_script.ReadFloat("evade");

		// Add the character's initial equipment. If any equipment ids are zero, that indicates nothing is to be equipped.
		uint32 equipment_id = 0;
		equipment_id = char_script.ReadUInt("weapon");
		if (equipment_id != 0) {
			_weapon_equipped = new GlobalWeapon(equipment_id);
		} else {
			_weapon_equipped = NULL;
		}

		equipment_id = char_script.ReadUInt("head_armor");
		if (equipment_id != 0) {
			_armor_equipped.push_back(new GlobalArmor(equipment_id));
		} else {
			_armor_equipped.push_back(NULL);
		}

		equipment_id = char_script.ReadUInt("torso_armor");
		if (equipment_id != 0) {
			_armor_equipped.push_back(new GlobalArmor(equipment_id));
		} else {
			_armor_equipped.push_back(NULL);
		}

		equipment_id = char_script.ReadUInt("arm_armor");
		if (equipment_id != 0) {
			_armor_equipped.push_back(new GlobalArmor(equipment_id));
		} else {
			_armor_equipped.push_back(NULL);
		}

		equipment_id = char_script.ReadUInt("leg_armor");
		if (equipment_id != 0) {
			_armor_equipped.push_back(new GlobalArmor(equipment_id));
		} else {
			_armor_equipped.push_back(NULL);
		}
		char_script.CloseTable();

		if (char_script.IsErrorDetected()) {
			if (GLOBAL_DEBUG) {
				cerr << "GLOBAL WARNING: GlobalCharacter constructor had errors in reading the character's "
					<< "initial stats. They are as follows:" << endl;
				cerr << char_script.GetErrorMessages() << endl;
			}
		}
	} // if (initial)

	// (4): Setup the character's attack points
	char_script.OpenTable("attack_points");
	for (uint32 i = GLOBAL_POSITION_HEAD; i < GLOBAL_POSITION_LEGS; i++) {
		_attack_points.push_back(new GlobalAttackPoint(this));
		char_script.OpenTable(i);
		if (_attack_points[i]->_LoadData(char_script) == false) {
			if (GLOBAL_DEBUG) {
				cerr << "GLOBAL WARNING: GlobalCharacter constructor failed to succesfully load data for "
					<< "attack point number: " << i << endl;
			}
		}
		char_script.CloseTable();
	}
	char_script.CloseTable();

	if (char_script.IsErrorDetected()) {
		if (GLOBAL_DEBUG) {
			cerr << "GLOBAL WARNING: GlobalCharacter constructor had errors in reading the character's "
				<< "attack points. They are as follows:" << endl;
			cerr << char_script.GetErrorMessages() << endl;
		}
	}

	// (5): Create the character's initial skill set, if requested
	if (initial) {
		// The keys to the skills table indicate the skill id, the value is the level required to add the skill
		vector<int32> skill_ids;
		uint32 level_required;
		char_script.OpenTable("skills");
		char_script.ReadTableKeys(skill_ids);

		// Only add the skills for which the experience level requirements are met
		for (uint32 i = 0; i < skill_ids.size(); i++) {
			level_required = char_script.ReadUInt(skill_ids[i]);
			if (level_required <= _experience_level) {
				AddSkill(static_cast<uint32>(skill_ids[i]));
			}
		}

		char_script.CloseTable();

		if (char_script.IsErrorDetected()) {
			if (GLOBAL_DEBUG) {
				cerr << "GLOBAL WARNING: GlobalCharacter constructor had errors in reading the character's "
					<< "initial skill set. They are as follows:" << endl;
				cerr << char_script.GetErrorMessages() << endl;
			}
		}
	} // if (initial)

	char_script.CloseTable(); // "characters[id]"
	char_script.CloseTable(); // "characters"

	// (6): Determine the character's initial growth, if requested
	if (initial) {
		// Initialize the experience level milestones
		_growth._experience_for_last_level = _experience_points;
		_growth._experience_for_next_level = _experience_points;
		_growth._DetermineNextLevelExperience();
		try {
			ScriptCallFunction<void>(char_script.GetLuaState(), "DetermineGrowth", this);
			_growth._ConstructPeriodicGrowth();
		}
		catch (luabind::error e) {
			ScriptManager->HandleLuaError(e);
		}
		catch (luabind::cast_failed e) {
			ScriptManager->HandleCastError(e);
		}
	}

	if (char_script.IsErrorDetected()) {
		if (GLOBAL_DEBUG) {
			cerr << "GLOBAL WARNING: GlobalCharacter constructor had script read errors remaining when "
				<< "it was finished with the file. They are as follows: " << endl;
			cerr << char_script.GetErrorMessages() << endl;
		}
	}
	
	char_script.CloseFile();

	// TEMP TEMP TEMP: Load the character's idle animation
	AnimatedImage idle;
	StillImage img;
	img.SetDimensions(64, 128);
	for (uint32 i = 0; i < 6; i ++) {
		idle.AddFrame(img, 10);
	}
	if (VideoManager->LoadAnimatedImageFromNumberElements(idle, "img/sprites/battle/characters/" + _filename + "_idle.png", 1, 6) == false) {
		exit(1);
	}
	idle.SetFrameIndex(0);
	_battle_animation["idle"] = idle;

	// Load the character's battle portraits from a multi image
	_battle_portraits.assign(5, StillImage());
	for (uint32 i = 0; i < _battle_portraits.size(); i++) {
		_battle_portraits[i].SetDimensions(100, 100);
	}
	if (VideoManager->LoadMultiImageFromNumberElements(_battle_portraits, "img/portraits/battle/" + _filename + "_damage.png", 1, 5) == false)
		exit(1);
} // GlobalCharacter::GlobalCharacter(uint32 id, bool initial)



GlobalCharacter::~GlobalCharacter() {
	// TODO: Remove all references to loaded image files
}



void GlobalCharacter::AddSkill(uint32 skill_id) {
	if (skill_id == 0) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalCharacter::AddSkill() failed because an invalid skill id was passed to it (0)" << endl;
		return;
	}

	if (_skills.find(skill_id) != _skills.end()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalCharacter::AddSkill() failed because the character had already learned the skill "
				<< "with the skill id value of: " << skill_id << endl;
	}

	GlobalSkill* skill = new GlobalSkill(skill_id);
	if (skill->GetID() == 0) { // Indicates that the skill failed to load successfully
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalCharacter::AddSkill() failed because the skill failed to load" << endl;
		delete skill;
		return;
	}

	// Insert the pointer to the new skill inside of the global skills map and the skill type vector
	_skills.insert(make_pair(skill_id, skill));
	switch (skill->GetType()) {
		case GLOBAL_SKILL_ATTACK:
			_attack_skills.push_back(skill);
			break;
		case GLOBAL_SKILL_DEFEND:
			_defense_skills.push_back(skill);
			break;
		case GLOBAL_SKILL_SUPPORT:
			_support_skills.push_back(skill);
			break;
		default:
			if (GLOBAL_DEBUG)
				cerr << "GLOBAL WARNING: GlobalCharacter::AddSkill() loaded a new skill with an unknown skill type: " << skill->GetType() << endl;
			break;
	}
} // void GlobalCharacter::AddSkill(uint32 skill_id)



bool GlobalCharacter::AddExperiencePoints(uint32 xp) {
	_experience_points += xp;
	_growth._CheckForGrowth();
	return _growth.IsGrowthDetected();
}

// -----------------------------------------------------------------------------
// GlobalEnemy class
// -----------------------------------------------------------------------------

GlobalEnemy::GlobalEnemy(uint32 id) {
	_id = id;
	_experience_level = 0;
	_weapon_equipped = NULL;
	_armor_equipped.clear();

	// (1): Use the id member to determine the name of the data file that the enemy is defined in
	string file_ext;
	if (_id < 1) {
		cerr << "GLOBAL ERROR: invalid id for loading enemy data: " << _id << endl;
	} else if (_id < 101) {
		file_ext = "01";
	} else if (_id < 201) {
		file_ext = "02";
	}
	string filename = "dat/actors/enemies_set_" + file_ext + ".lua";

	// (2): Open the script file and table that store the enemy data
	ReadScriptDescriptor enemy_data;
	if (enemy_data.OpenFile(filename) == false) {
		cerr << "GLOBAL ERROR: failed to load enemy data file: " << _filename << endl;
		_id = 0;
		return;
	}
	enemy_data.OpenTable("enemies");
	enemy_data.OpenTable(_id);

	// (3): Load the enemy's name and sprite data
	_name = MakeUnicodeString(enemy_data.ReadString("name"));
	_filename = enemy_data.ReadString("filename");
	_sprite_width = enemy_data.ReadInt("sprite_width");
	_sprite_height = enemy_data.ReadInt("sprite_height");

	// (4): Attempt to load the MultiImage for the sprite's frames, which should contain one row and four columns of images
	_battle_sprite_frames.assign(4, StillImage());
	string sprite_filename = "img/sprites/battle/enemies/" + _filename + ".png";
	if (VideoManager->LoadMultiImageFromNumberElements(_battle_sprite_frames, sprite_filename, 1, 4) == false) {
		cerr << "GLOBAL WARNING: failed to load sprite frames for enemy: " << sprite_filename << endl;
	}

	// (5): Load the enemy's initial stats
	enemy_data.OpenTable("initial_stats");
	_max_hit_points = enemy_data.ReadUInt("hit_points");
	_hit_points = _max_hit_points;
	_max_skill_points = enemy_data.ReadUInt("skill_points");
	_skill_points = _max_skill_points;
	_experience_points = enemy_data.ReadUInt("experience_points");
	_strength = enemy_data.ReadUInt("strength");
	_vigor = enemy_data.ReadUInt("vigor");
	_fortitude = enemy_data.ReadUInt("fortitude");
	_protection = enemy_data.ReadUInt("protection");
	_agility = enemy_data.ReadUInt("agility");
	_evade = enemy_data.ReadFloat("evade");
	_drunes_dropped = enemy_data.ReadUInt("drunes");
	enemy_data.CloseTable();

	// (6): Load the enemy's growth statistics
	enemy_data.OpenTable("growth_stats");
	_growth_hit_points = enemy_data.ReadFloat("hit_points");
	_growth_skill_points = enemy_data.ReadFloat("skill_points");
	_growth_experience_points = enemy_data.ReadFloat("experience_points");
	_growth_strength = enemy_data.ReadFloat("strength");
	_growth_vigor = enemy_data.ReadFloat("vigor");
	_growth_fortitude = enemy_data.ReadFloat("fortitude");
	_growth_protection = enemy_data.ReadFloat("protection");
	_growth_agility = enemy_data.ReadFloat("agility");
	_growth_evade = enemy_data.ReadFloat("evade");
	_growth_drunes = enemy_data.ReadFloat("drunes");
	enemy_data.CloseTable();

	// (7): Create the attack points for the enemy
	enemy_data.OpenTable("attack_points");
	uint32 ap_size = enemy_data.GetTableSize();
	for (uint32 i = 1; i <= ap_size; i++) {
		_attack_points.push_back(new GlobalAttackPoint(this));
		enemy_data.OpenTable(i);
		if (_attack_points.back()->_LoadData(enemy_data) == false) {
			cerr << "GLOBAL ERROR: GlobalEnemy constructor was unable to load data for an attack point" << endl;
			exit(1);
		}
		enemy_data.CloseTable();
	}
	enemy_data.CloseTable();

	// (8): Add the set of skills to the enemy
	vector<int32> skill_ids;
	enemy_data.OpenTable("skills");
	enemy_data.ReadTableKeys(skill_ids);
	for (uint32 i = 0; i < skill_ids.size(); i++) {
		uint32 skill = static_cast<uint32>(skill_ids[i]);
		if (_skill_set.find(skill) == _skill_set.end()) {
			_skill_set.insert(make_pair(skill, enemy_data.ReadUInt(skill)));
		}
		else {
			if (GLOBAL_DEBUG)
				cerr << "GLOBAL WARNING: GlobalEnemy constructor tried to add a skill to the "
					<< "skill set multiple times" << endl;
		}
	}
	enemy_data.CloseTable();


	// (9): Load the possible items that the enemy may drop
	enemy_data.OpenTable("drop_objects");
	for (uint32 i = 1; i <= enemy_data.GetTableSize(); i++) {
		enemy_data.OpenTable(i);
		_dropped_objects.push_back(enemy_data.ReadUInt(1));
		_dropped_chance.push_back(enemy_data.ReadFloat(2));
		_dropped_level_required.push_back(enemy_data.ReadUInt(3));
		enemy_data.CloseTable();
	}
	enemy_data.CloseTable();

	enemy_data.CloseTable(); // enemies[_id]
	enemy_data.CloseTable(); // enemies

	if (enemy_data.IsErrorDetected()) {
		if (GLOBAL_DEBUG) {
			cerr << "GLOBAL WARNING: GlobalEnemy constructor had script read errors remaining when "
				<< "it was finished with the file. They are as follows: " << endl;
			cerr << enemy_data.GetErrorMessages() << endl;
		}
	}

	enemy_data.CloseFile();
} // GlobalEnemy::~GlobalEnemy()



GlobalEnemy::~GlobalEnemy() {
	// TODO: dereference all loaded image files
}



void GlobalEnemy::AddSkill(uint32 skill_id) {
	if (skill_id == 0) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalEnemy::AddSkill() failed because an invalid skill id was passed to it (0)" << endl;
		return;
	}

	if (_skills.find(skill_id) != _skills.end()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalEnemy::AddSkill() failed because the enemy had already learned the skill "
				<< "with the skill id value of: " << skill_id << endl;
	}

	GlobalSkill* skill = new GlobalSkill(skill_id);
	if (skill->GetID() == 0) { // Indicates that the skill failed to load successfully
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalCharacter::AddSkill() failed because the skill failed to load" << endl;
		delete skill;
		return;
	}

	// Insert the pointer to the new skill inside of the global skills map and the skill type vector
	_skills.insert(make_pair(skill_id, skill));
}



void GlobalEnemy::Initialize(uint32 xp_level) {
	if (xp_level == 0) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalEnemy::Initialize() was called with an xp_level argument of 0" << endl;
		return;
	}

	if (_skills.empty() == false) { // Indicates that the enemy has already been initialized
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalEnemy::Initialize() was invoked for an already initialized enemy" << endl;
		return;
	}

	_experience_level = xp_level;

	// ----- (1): Add all new skills that should be available at the current experience level
	for (map<uint32, uint32>::iterator i = _skill_set.begin(); i != _skill_set.end(); i++) {
		if (_experience_level >= i->second)
			AddSkill(i->first);
	}

	if (_skills.empty()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalEnemy::Initialize() did not add any skills for the enemy" << endl;
	}

	// ----- (2): Add the new stats to the growth rate multiplied by the experience level
	_max_hit_points    += static_cast<uint32>(_growth_hit_points * _experience_level);
	_max_skill_points  += static_cast<uint32>(_growth_skill_points * _experience_level);
	_experience_points += static_cast<uint32>(_growth_experience_points * _experience_level);
	_strength          += static_cast<uint32>(_growth_strength * _experience_level);
	_vigor             += static_cast<uint32>(_growth_vigor * _experience_level);
	_fortitude         += static_cast<uint32>(_growth_fortitude * _experience_level);
	_protection        += static_cast<uint32>(_growth_protection * _experience_level);
	_agility           += static_cast<uint32>(_growth_agility * _experience_level);
	_evade             += static_cast<float>(_growth_evade * _experience_level);
	_drunes_dropped    += static_cast<uint32>(_growth_drunes * _experience_level);

	// ----- (3): Randomize the new stats by using a guassian random variable
	// Use the inital stats as the means and a standard deviation of 10% of the mean
	_max_hit_points     = GaussianRandomValue(_max_hit_points, _max_hit_points / 10.0f);
	_max_skill_points   = GaussianRandomValue(_max_skill_points, _max_skill_points / 10.0f);
	_experience_points  = GaussianRandomValue(_experience_points, _experience_points / 10.0f);
	_strength           = GaussianRandomValue(_strength, _strength / 10.0f);
	_vigor              = GaussianRandomValue(_strength, _strength / 10.0f);
	_fortitude          = GaussianRandomValue(_fortitude, _fortitude / 10.0f);
	_protection         = GaussianRandomValue(_protection, _protection / 10.0f);
	_agility            = GaussianRandomValue(_agility, _agility / 10.0f);
	// TODO: need a gaussian random var function that takes a float arg
	//_evade              = static_cast<float>(GaussianRandomValue(_evade, _evade / 10.0f));
	_drunes_dropped     = GaussianRandomValue(_drunes_dropped, _drunes_dropped / 10.0f);

	// ----- (4): Set the current hit points and skill points to their new maximum values
	_hit_points = _max_hit_points;
	_skill_points = _max_skill_points;
} // void GlobalEnemy::Initialize(uint32 xp_level)



void GlobalEnemy::DetermineDroppedObjects(vector<GlobalObject*>& objects) {
	objects.clear();

	for (uint32 i = 0; i < _dropped_objects.size(); i++) {
		if (_experience_level >= _dropped_level_required[i]) {
			if (_dropped_chance[i] < RandomFloat()) {
				objects.push_back(GlobalCreateNewObject(_dropped_objects[i]));
			}
		}
	}
}

// -----------------------------------------------------------------------------
// GlobalParty class
// -----------------------------------------------------------------------------

void GlobalParty::AddActor(GlobalActor* actor, int32 index) {
	if (actor == NULL) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::AddActor() was passed a NULL actor argument" << endl;
		return;
	}

	if (_allow_duplicates == false) {
		// Check that this actor is not already in the party
		for (uint32 i = 0; i < _actors.size(); i++) {
			if (actor->GetID() == _actors[i]->GetID()) {
				if (GLOBAL_DEBUG)
					cerr << "GLOBAL WARNING: GlobalParty::AddActor() attempted to add a duplicate actor "
						<< "when duplicates were not allowed: " << actor->GetID() << endl;
				return;
			}
		}
	}

	if (index < 0) {
		_actors.push_back(actor);
	}
	else if (static_cast<uint32>(index) >= _actors.size()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::AddActor() was given an index argument that exceeded "
				<< "the current party size: " << index << endl;
		return;
	}
	else {
		vector<GlobalActor*>::iterator position = _actors.begin();
		for (int32 i = 0; i < index; i++, position++);
		_actors.insert(position, actor);
	}
} // void GlobalParty::AddActor(GlobalActor* actor, int32 index)



GlobalActor* GlobalParty::RemoveActorAtIndex(uint32 index) {
	if (index >= _actors.size()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::RemoveActorAtIndex() was called with an out-of-bounds "
				<< "index argument: " << index << endl;
		return NULL;
	}

	GlobalActor* removed_actor = _actors[index];
	vector<GlobalActor*>::iterator position = _actors.begin();
	for (uint32 i = 0; i < index; i++, position++);
	_actors.erase(position);

	return removed_actor;
} // GlobalActor* GlobalParty::RemoveActorAtIndex(uint32 index)



GlobalActor* GlobalParty::RemoveActorByID(uint32 id) {
	if (_allow_duplicates) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::RemoveActorByID() was called when duplicate actors "
				<< "were allowed in the party: " << id << endl;
		return NULL;
	}

	GlobalActor* removed_actor = NULL;
	for (vector<GlobalActor*>::iterator position = _actors.begin(); position != _actors.end(); position++) {
		if (id == (*position)->GetID()) {
			removed_actor = *position;
			_actors.erase(position);
			break;
		}
	}

	if (removed_actor == NULL) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::RemoveActorByID() failed to find an actor in the party "
				<< "with the requested id: " << id << endl;
	}

	return removed_actor;
} // GlobalActor* GlobalParty::RemoveActorByID(uint32 id)



void GlobalParty::SwapActorsByIndex(uint32 first_index, uint32 second_index) {
	if (first_index == second_index) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::SwapActorsByIndex() was called with both the first and "
				<< "second index arguments equal to one another: " << first_index << endl;
		return;
	}

	if (first_index >= _actors.size()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::SwapActorsByIndex() was called with an out-of-bounds "
				<< "first index argument: " << first_index << endl;
		return;
	}

	if (second_index >= _actors.size()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::SwapActorsByIndex() was called with an out-of-bounds "
				<< "second index argument: " << second_index << endl;
		return;
	}

	GlobalActor* tmp = _actors[first_index];
	_actors[first_index] = _actors[second_index];
	_actors[second_index] = tmp;
} // void GlobalParty::SwapActorsByIndex(uint32 first_index, uint32 second_index)



void GlobalParty::SwapActorsByID(uint32 first_id, uint32 second_id) {
	if (first_id == second_id) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::SwapActorsByID() was called with both the first and "
				<< "second id arguments equal to one another: " << first_id << endl;
		return;
	}

	if (_allow_duplicates) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::SwapActorsByID() was called when duplicate actors "
				<< "were allowed in the party" << endl;
		return;
	}

	vector<GlobalActor*>::iterator first_position;
	vector<GlobalActor*>::iterator second_position;
	for (first_position = _actors.begin(); first_position != _actors.end(); first_position++) {
		if ((*first_position)->GetID() == first_id)
			break;
	}
	for (second_position = _actors.begin(); second_position != _actors.end(); second_position++) {
		if ((*second_position)->GetID() == second_id)
			break;
	}

	if (first_position == _actors.end()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::SwapActorsByID() failed because there did not exist "
				<< "an actor with an id equal to the first_id argument: " << first_id << endl;
		return;
	}
	if (second_position == _actors.end()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::SwapActorsByID() failed because there did not exist "
				<< "an actor with an id equal to the second_id argument: " << second_id << endl;
		return;
	}

	GlobalActor* tmp = *first_position;
	*first_position = *second_position;
	*second_position = tmp;
} // void GlobalParty::SwapActorsByID(uint32 first_id, uint32 second_id)



GlobalActor* GlobalParty::ReplaceActorByIndex(uint32 index, GlobalActor* new_actor) {
	if (new_actor == NULL) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::ReplaceActorByIndex() was passed a NULL actor argument" << endl;
		return NULL;
	}

	if (index >= _actors.size()) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::ReplaceActorByIndex() was called with an out-of-bounds "
				<< "index argument: " << index << endl;
		return NULL;
	}

	GlobalActor* tmp = _actors[index];
	_actors[index] = new_actor;
	return tmp;
} // GlobalActor* GlobalParty::ReplaceActorByIndex(uint32 index, GlobalActor* new_actor)



GlobalActor* GlobalParty::ReplaceActorByID(uint32 id, GlobalActor* new_actor) {
	if (_allow_duplicates) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::ReplaceActorByID() was called when duplicate actors "
				<< "were allowed in the party: " << id << endl;
		return NULL;
	}

	if (new_actor == NULL) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::ReplaceActorByID() was passed a NULL actor argument" << endl;
		return NULL;
	}

	GlobalActor* removed_actor = NULL;
	for (vector<GlobalActor*>::iterator position = _actors.begin(); position != _actors.end(); position++) {
		if (id == (*position)->GetID()) {
			removed_actor = *position;
			*position = new_actor;
			break;
		}
	}

	if (removed_actor == NULL) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::ReplaceActorByID() failed to find an actor in the party "
				<< "with the requested id: " << id << endl;
	}

	return removed_actor;
} // GlobalActor* GlobalParty::ReplaceActorByID(uint32 id, GlobalActor* new_actor)



float GlobalParty::AverageExperienceLevel() const {
	if (_actors.empty())
		return 0.0f;

	float xp_level_sum = 0.0f;
	for (uint32 i = 0; i < _actors.size(); i++)
		xp_level_sum += static_cast<float>(_actors[i]->GetExperienceLevel());
	return (xp_level_sum / static_cast<float>(_actors.size()));
} // float GlobalParty::AverageExperienceLevel()



GlobalActor* GlobalParty::GetActorByID(uint32 id) const {
	if (_allow_duplicates) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL WARNING: GlobalParty::GetActorByID() was called when duplicate actors "
				<< "were allowed in the party: " << id << endl;
		return NULL;
	}

	for (uint32 i = 0; i < _actors.size(); i++) {
		if (_actors[i]->GetID() == id) {
			return _actors[i];
		}
	}

	if (GLOBAL_DEBUG)
		cerr << "GLOBAL WARNING: GlobalParty::GetActorByID() failed to find an actor in the party "
				<< "with the requested id: " << id << endl;
	return NULL;
} // float GlobalParty::AverageExperienceLevel()

} // namespace hoa_global
