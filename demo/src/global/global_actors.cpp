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


// ****************************************************************************
// ***** GlobalAttackPoint
// ****************************************************************************

bool GlobalAttackPoint::LoadData(ReadScriptDescriptor& script) {
	if (script.GetAccessMode() != SCRIPT_READ) {
		return false;
	}

	_name = MakeUnicodeString(script.ReadString("name"));
	_x_position = script.ReadInt("x_position");
	_y_position = script.ReadInt("y_position");
	_fortitude_bonus = script.ReadInt("fortitude_bonus");
	_protection_bonus = script.ReadInt("protection_bonus");
	_evade_bonus = script.ReadFloat("evade_bonus");

	// TODO: Check if any script read errors occurred

	return true;
}

// ****************************************************************************
// ***** GlobalActor
// ****************************************************************************

void GlobalActor::_CalculateAttackRatings() {
	// TODO
}



void GlobalActor::_CalculateDefenseRatings() {
	// TODO
}



void GlobalActor::_CalculateEvadeRatings() {
	// TODO
}



void GlobalActor::EquipWeapon(GlobalWeapon* weapon) {
	delete _weapon_equipped;
	_weapon_equipped = new GlobalWeapon(weapon->GetID());
}



void GlobalActor::EquipArmor(GlobalArmor* armor) {
	switch (armor->GetType()) {
		case GLOBAL_OBJECT_HEAD_ARMOR:
		delete _armor_equipped[0];
		_armor_equipped[0] = new GlobalArmor(armor->GetID());
		break;

		case GLOBAL_OBJECT_TORSO_ARMOR:
		delete _armor_equipped[1];
		_armor_equipped[1] = new GlobalArmor(armor->GetID());
		break;

		case GLOBAL_OBJECT_ARM_ARMOR:
		delete _armor_equipped[2];
		_armor_equipped[2] = new GlobalArmor(armor->GetID());
		break;

		case GLOBAL_OBJECT_LEG_ARMOR:
		delete _armor_equipped[3];
		_armor_equipped[3] = new GlobalArmor(armor->GetID());
		break;

		default:
		cerr << "ERROR: Trying to equip an armor that isn't an armor!" << endl;
		break;
	}
}

// ****************************************************************************
// ***** GlobalCharacter
// ****************************************************************************

GlobalCharacter::~GlobalCharacter() {
	// TODO: delete all dynamically allocated data

}


GlobalCharacter::GlobalCharacter(uint32 id) {
	_id = id;

	// (1): Attempt to open the characters script file
	ReadScriptDescriptor char_script;
	if (char_script.OpenFile("dat/actors/characters.lua") == false) {
		if (GLOBAL_DEBUG)
			cerr << "GLOBAL ERROR: could not create new character" << endl;
		return;
	}

	// (2): Open up the table containing the character's data
	char_script.OpenTable("characters");
	char_script.OpenTable(_id);

	// (3): Read the character's basic stats
	_name = MakeUnicodeString(char_script.ReadString("name"));
	_filename = char_script.ReadString("filename");

	char_script.OpenTable("base_stats");
	_max_hit_points = char_script.ReadInt("max_hit_points");
	_hit_points = _max_hit_points;
	_max_skill_points = char_script.ReadInt("max_skill_points");
	_skill_points = _max_skill_points;
	_experience_level = 1;
	_strength = char_script.ReadInt("strength");
	_vigor = char_script.ReadInt("vigor");
	_fortitude = char_script.ReadInt("fortitude");
	_protection = char_script.ReadInt("protection");
	_agility = char_script.ReadInt("agility");
	_evade = char_script.ReadFloat("evade");
	_experience_next_level = char_script.ReadInt("experience_points");
	char_script.CloseTable();

	// (3.5): Read the character's growth stats
	char_script.OpenTable("growth_stats");
	_growth_hit_points = char_script.ReadFloat("hit_points");
	_growth_skill_points = char_script.ReadFloat("skill_points");
	_growth_experience_points = char_script.ReadFloat("experience_points");
	_growth_strength = char_script.ReadFloat("strength");
	_growth_vigor = char_script.ReadFloat("vigor");
	_growth_fortitude = char_script.ReadFloat("fortitude");
	_growth_protection = char_script.ReadFloat("protection");
	_growth_agility = char_script.ReadFloat("agility");
	_growth_evade = char_script.ReadFloat("evade");
	char_script.CloseTable();

	// (4): Setup the character's attack points
	char_script.OpenTable("attack_points");
	for (uint32 i = 0; i < 4; i++) {
		_attack_points.push_back(new GlobalAttackPoint());
		char_script.OpenTable(i+1);
		_attack_points[i]->LoadData(char_script);
		char_script.CloseTable();
	}
	char_script.CloseTable();

	// (5): Create the character's initial equipment
	uint32 equipment_id;

	// If the equipment id is zero, that means that there is no initial equipment
	equipment_id = char_script.ReadInt("initial_weapon");
	if (equipment_id != 0) {
		_weapon_equipped = new GlobalWeapon(equipment_id, 1);
	} else {
		_weapon_equipped = NULL;
	}

	equipment_id = char_script.ReadInt("initial_head_armor");
	if (equipment_id != 0) {
		_armor_equipped.push_back(new GlobalArmor(equipment_id, 1));
	} else {
		_armor_equipped.push_back(NULL);
	}

	equipment_id = char_script.ReadInt("initial_torso_armor");
	if (equipment_id != 0) {
		_armor_equipped.push_back(new GlobalArmor(equipment_id, 1));
	} else {
		_armor_equipped.push_back(NULL);
	}

	equipment_id = char_script.ReadInt("initial_arm_armor");
	if (equipment_id != 0) {
		_armor_equipped.push_back(new GlobalArmor(equipment_id, 1));
	} else {
		_armor_equipped.push_back(NULL);
	}

	equipment_id = char_script.ReadInt("initial_leg_armor");
	if (equipment_id != 0) {
		_armor_equipped.push_back(new GlobalArmor(equipment_id, 1));
	} else {
		_armor_equipped.push_back(NULL);
	}

	// (6): Create the character's initial skill set
	vector<int32> skill_ids;
	char_script.ReadIntVector("initial_skills", skill_ids);

	GlobalSkill *skill = NULL;

	for (uint32 i = 0; i < skill_ids.size(); i++) {
		skill = new GlobalSkill(skill_ids[i]);
		_skills.insert(make_pair(skill_ids[i], skill));

		switch (skill->GetType())
		{
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
				break;
		}
	}

	char_script.CloseTable();
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

	// TEMP: Add new skills
	// AddAttackSkill(new GlobalSkill()); // removed text "sword_slash" because that constructor isn't implemented yet -MF
} // GlobalCharacter::GlobalCharacter(uint32 id)

bool GlobalCharacter::AddXP(uint32 xp)
{
	int32 temp_xp = static_cast<int32>(_experience_next_level - xp);
	int32 xp_to_next_level = 0;

	// (1): Attempt to open the characters script file
	ReadScriptDescriptor char_script;
	char_script.OpenFile("dat/actors/characters.lua");
	//FIXME - no error checking code here!

	if (temp_xp <= 0) {
		//next lines are essentially SetExperienceNextLevel(GetXPForNextLevel() + _experience_next_level);
		xp_to_next_level = ScriptCallFunction<int32>(char_script.GetLuaState(), "XPToNextLevel", (_experience_level + 1)); //add one to calculate based on the level we're going to; simplifies the lua function a bit.
		SetExperienceNextLevel(xp_to_next_level + temp_xp);  //if we're here, temp_xp will be negative
		
		AddExperienceLevel(1);  //FIXME - if the character gets enough XP to gain multiple levels, they shouldn't only get one

		char_script.CloseFile();
		return true;
	}
	else {
		SetExperienceNextLevel(temp_xp);
	}
	char_script.CloseFile();
	return false;
}

void GlobalCharacter::AddSkill(uint32 skill_id) {
	GlobalSkill* skill = new GlobalSkill(skill_id);

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
			break;
	}
}

void GlobalCharacter::AddExperienceLevel(uint32 lvl)
{
	_experience_level += lvl;

	// Adds the growth stats, plus a small random value, to the character's stats on levelling.

	_strength += static_cast<uint32>(_growth_strength + RandomBoundedInteger(1, 3));
	_vigor += static_cast<uint32>(_growth_vigor + RandomBoundedInteger(1, 3));
	_fortitude += static_cast<uint32>(_growth_fortitude + RandomBoundedInteger(1, 3));
	_protection += static_cast<uint32>(_growth_protection + RandomBoundedInteger(1, 2));
	_agility += static_cast<uint32>(_growth_agility + RandomBoundedInteger(1, 3));

	_max_hit_points += static_cast<uint32>(_growth_hit_points + RandomBoundedInteger(1, 3));
	_max_skill_points += static_cast<uint32>(_growth_skill_points + RandomBoundedInteger(1, 3));
	_evade += static_cast<uint32>(_growth_evade + RandomBoundedInteger(1, 3));
}

// ****************************************************************************
// ***** GlobalEnemy
// ****************************************************************************

GlobalEnemy::GlobalEnemy(uint32 id) {
	_id = id;
	_experience_level = 0;

	_armor_equipped.clear();
	_weapon_equipped = NULL;

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
	if (enemy_data.OpenFile(filename.c_str()) == false) {
		cerr << "GLOBAL ERROR: failed to load enemy data file: " << _filename << endl;
		_id = 0;
		return;
	}
	enemy_data.OpenTable("enemies");
	enemy_data.OpenTable(_id);

	// (3): Load the enemy's name and sprite data
	_name = MakeUnicodeString(enemy_data.ReadString("name"));
	string sprite_filename = enemy_data.ReadString("sprite_filename");
	_sprite_width = enemy_data.ReadInt("sprite_width");
	_sprite_height = enemy_data.ReadInt("sprite_height");
	_filename = enemy_data.ReadString("filename");

	// Make four dummy images for LoadMultiImage(). TODO: Can't we do these in LoadMultiImage() instead :(
	_sprite_frames.assign(4, StillImage());
	// Attempt to load the MultiImage, which should contain one row and four columns of images
	if (VideoManager->LoadMultiImageFromNumberElements(_sprite_frames, sprite_filename, 1, 4) == false) {
		cerr << "GLOBAL ERROR: failed to load sprite frames for enemy: " << sprite_filename << endl;
		_id = 0;
		return;
	}

	// (4): Load the base statistics
	enemy_data.OpenTable("base_stats");
	_max_hit_points = enemy_data.ReadInt("hit_points");
	_hit_points = _max_hit_points;
	_max_skill_points = enemy_data.ReadInt("skill_points");
	_skill_points = _max_skill_points;
	_experience_points = enemy_data.ReadInt("experience_points");
	_strength = enemy_data.ReadInt("strength");
	_vigor = enemy_data.ReadInt("vigor");
	_fortitude = enemy_data.ReadInt("fortitude");
	_protection = enemy_data.ReadInt("protection");
	_agility = enemy_data.ReadInt("agility");
	_evade = enemy_data.ReadFloat("evade");
	enemy_data.CloseTable();

	// (5): Load the rewards
	enemy_data.OpenTable("rewards");
	_item_dropped = enemy_data.ReadInt("item_dropped");
	_chance_to_drop = enemy_data.ReadFloat("chance_to_drop");
	_money = enemy_data.ReadInt("money");
	enemy_data.CloseTable();

	// (6): Load the growth statistics
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
	enemy_data.CloseTable();

	// (7): Create and initialize the attack points for the enemy
	enemy_data.OpenTable("attack_points");
	uint32 ap_size = enemy_data.GetTableSize();
	for (uint32 i = 1; i <= ap_size; i++) {
		_attack_points.push_back(new GlobalAttackPoint());
		enemy_data.OpenTable(i);
		if (_attack_points.back()->LoadData(enemy_data) == false) {
			cerr << "GLOBAL ERROR: GlobalEnemy constructor was unable to load data for an attack point" << endl;
		}
		enemy_data.CloseTable();
	}
	enemy_data.CloseTable();

	// (8): Add the set of skills to the enemy
	vector<int32> skill_ids;
	enemy_data.ReadIntVector("skills", skill_ids);
	for (uint32 i = 0; i < skill_ids.size(); i++) {
		// TODO: Use the skill ids to load all of the GlobalSkills for the enemy
		_skills.insert(make_pair(skill_ids[i], new GlobalSkill(static_cast<uint32>(skill_ids[i]))));
	}

	enemy_data.CloseFile();
} // GlobalEnemy::~GlobalEnemy()



GlobalEnemy::~GlobalEnemy() {
	// TODO: delete all attack points, skills, and other dynamically allocated data
}



void GlobalEnemy::LevelSimulator(uint32 level) {
	if (level == 0) {
		return;
	}

	_experience_level = level;

	// Set the new stats to be the growth rate multiplied by the new experience level
	_max_hit_points += static_cast<uint32>(_growth_hit_points * level);
	_max_skill_points += static_cast<uint32>(_growth_skill_points * level);
	_strength += static_cast<uint32>(_growth_strength * level);
	_vigor += static_cast<uint32>(_growth_vigor * level);
	_fortitude += static_cast<uint32>(_growth_fortitude * level);
	_protection += static_cast<uint32>(_growth_protection * level);
	_agility += static_cast<uint32>(_growth_agility * level);
	_evade += static_cast<float>(_growth_evade * level);
	_experience_points += static_cast<uint32>(_growth_experience_points * level);

	// Randomize the new stats by using a guassian random variable, with the inital stats as the means and a standard deviation of 10% of the mean
	_max_hit_points = GaussianRandomValue(_max_hit_points, _max_hit_points / 10.0f);
	_max_skill_points = GaussianRandomValue(_max_skill_points, _max_skill_points / 10.0f);
	_strength = GaussianRandomValue(_strength, _strength / 10.0f);
	_vigor = GaussianRandomValue(_strength, _strength / 10.0f);
	_fortitude = GaussianRandomValue(_fortitude, _fortitude / 10.0f);
	_protection = GaussianRandomValue(_protection, _protection / 10.0f);
	_agility = GaussianRandomValue(_agility, _agility / 10.0f);
	// TODO: need a gaussian random var function that takes a float arg
	// _evade = static_cast<float>(GaussianRandomValue(_evade, _evade / 10.0f));
	_experience_points = GaussianRandomValue(_experience_points, _experience_points / 10.0f);

	// The current hit points and skill points are automatically set to their new maximum value
	_hit_points = _max_hit_points;
	_skill_points = _max_skill_points;

	// TODO: Determine what skills, elemental bonuses, and status bonuses are unlocked at this new level
} // void GlobalEnemy::LevelSimulator(uint32 level)

// ****************************************************************************
// ***** GlobalParty
// ****************************************************************************

GlobalActor* GlobalParty::RemoveActor(uint32 index) {
	if (index >= _actors.size() || _actors.size() == 0) {
		return NULL;
	}

	GlobalActor* removed_actor = _actors[index];

	// Shift all of the actors appropriately and remove the last element of the _actors vector
	for (uint32 i = index + 1; i < _actors.size(); i++) {
		_actors[i-1] = _actors[i];
	}
	_actors.pop_back();

	return removed_actor;
} // GlobalActor* GlobalParty::RemoveActor(uint32 index)

} // namespace hoa_global
