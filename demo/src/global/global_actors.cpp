////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
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

GlobalAttackPoint::GlobalAttackPoint() {
	_name = MakeUnicodeString("");
	_x_position = 0;
	_y_position = 0;
}



GlobalAttackPoint::GlobalAttackPoint(const hoa_utils::ustring & name, uint16 x, uint16 y) :
	_name(name),
	_x_position(x),
	_y_position(y)
{}



GlobalAttackPoint::~GlobalAttackPoint() {
	for (uint32 i = 0; i < _status_weaknesses.size(); i++) {
		//delete _status_weaknesses[i];
	}
	_status_weaknesses.clear();
}



// ****************************************************************************
// ***** GlobalActor
// ****************************************************************************
GlobalActor::GlobalActor()
{
}


GlobalActor::~GlobalActor()
{
}


GlobalWeapon* GlobalActor::EquipWeapon(GlobalWeapon* weapon) {
	// TODO
	return NULL;
}



GlobalArmor* GlobalActor::EquipArmor(GlobalArmor* armor, uint32 ap_index) {
	// TODO
	return NULL;
}



void GlobalActor::_CalculateAttackRatings() {
	// TODO
}



void GlobalActor::_CalculateDefenseRatings() {
	// TODO
}



void GlobalActor::_CalculateEvadeRatings() {
	// TODO
}

// ****************************************************************************
// ***** GlobalEnemy
// ****************************************************************************

GlobalEnemy::GlobalEnemy(const std::string & file_name) :
	_filename(file_name)
{
	// TODO: This is unfinished code
	ScriptDescriptor read_data;
	string fileName = "dat/enemies/" + _filename + ".lua";
	if (!read_data.OpenFile(fileName.c_str(), READ)) {
		cerr << "GLOBAL ERROR: failed to load enemy file: " << _filename << endl;
		return;
	}
	
	_enemy_id = read_data.ReadInt("id");
	_sprite_width = read_data.ReadInt("width");
	_sprite_height = read_data.ReadInt("height");
	uint32 numSkills = read_data.ReadInt("number_of_skills");
	for (uint32 i = 0; i < numSkills; i++) {
		_enemy_skills.push_back(new GlobalSkill(read_data.ReadString(("skill_" + i))));
	}
	
	// Load damage frames for the enemy. TODO: Do these images need to be deleted as well somewhere?!
	StillImage i;
	i.SetFilename("img/sprites/battle/enemies/" + _filename + ".png");
	i.SetStatic(true);
	i.SetDimensions(static_cast<float>(_sprite_width), static_cast<float>(_sprite_height));
	VideoManager->LoadImage(i);
	_sprite_frames.push_back(i);

	i.SetFilename("img/sprites/battle/enemies/" + _filename + "_hp66.png");
	i.SetStatic(true);
	i.SetDimensions(static_cast<float>(_sprite_width), static_cast<float>(_sprite_height));
	VideoManager->LoadImage(i);
	_sprite_frames.push_back(i);

	i.SetFilename("img/sprites/battle/enemies/" + _filename + "_hp33.png");
	i.SetStatic(true);
	i.SetDimensions(static_cast<float>(_sprite_width), static_cast<float>(_sprite_height));
	VideoManager->LoadImage(i);
	_sprite_frames.push_back(i);

	i.SetFilename("img/sprites/battle/enemies/" + _filename + "_hp00.png");
	i.SetStatic(true);
	i.SetDimensions(static_cast<float>(_sprite_width), static_cast<float>(_sprite_height));
	VideoManager->LoadImage(i);
	_sprite_frames.push_back(i);

	_movement_speed = read_data.ReadInt("movement_speed");
	_base_hit_points = read_data.ReadInt("base_hit_points");
	_base_skill_points = read_data.ReadInt("base_skill_points");
	_base_experience_points = read_data.ReadInt("base_experience_points");
	_base_strength = read_data.ReadInt("base_strength");
	_base_intelligence = read_data.ReadInt("base_intelligence");
	_base_agility = read_data.ReadInt("base_agility");
	_growth_hit_points = static_cast<float>(read_data.ReadInt("growth_hit_points"));
	_growth_skill_points = static_cast<float>(read_data.ReadInt("growth_skill_points"));
	_growth_experience_points = static_cast<float>(read_data.ReadInt("growth_experience_points"));
	_growth_strength = static_cast<float>(read_data.ReadInt("growth_strength"));
	_growth_intelligence = static_cast<float>(read_data.ReadInt("growth_intelligence"));
	_growth_agility = static_cast<float>(read_data.ReadInt("growth_agility"));
	_max_hit_points = _growth_hit_points;
	_max_skill_points = _growth_skill_points;
		
	int32 num_maps = read_data.ReadInt("number_of_maps");
	for (int32 i = 1; i <= num_maps; i++) {
		std::ostringstream os;
		os << "map_x_" << i;
		float x = read_data.ReadFloat(os.str().c_str());
		os.str("");
		os << "map_y_" << i;
		float y = read_data.ReadFloat(os.str().c_str());
		os.str("");
		os << "map_name_" << i;
		std::string name = read_data.ReadString(os.str().c_str());
		
		GlobalAttackPoint *ap = new GlobalAttackPoint(hoa_utils::MakeUnicodeString(name), (uint16)x, (uint16)y);
		_attack_points.push_back(ap);
	}
} // GlobalEnemy::~GlobalEnemy()



GlobalEnemy::~GlobalEnemy() { 
	// TODO: delete all attack points, skills, and other dynamically allocated data
}



void GlobalEnemy::LevelSimulator(uint32 level) {
	// This case should never occur, but we check for it just in case
	if (level == 0) {
		return;
	}
	
	_experience_level = level;

	_max_hit_points += static_cast<uint32>(_growth_hit_points * level);
	_max_skill_points += static_cast<uint32>(_growth_skill_points * level);
	_strength += static_cast<uint32>(_growth_strength * level);
	_vigor += static_cast<uint32>(_growth_vigor * level);
	_fortitude += static_cast<uint32>(_growth_fortitude * level);
	_resistance += static_cast<uint32>(_growth_resistance * level);
	_agility += static_cast<uint32>(_growth_agility * level);
	_evade += static_cast<float>(_growth_evade * level);
	_experience_points += static_cast<uint32>(_growth_experience_points * level);

	// Randomize the stats using a guassian random variable, with the inital stats as the means and a standard deviation of 10% of the mean
	_max_hit_points = GaussianRandomValue(_max_hit_points, _max_hit_points / 10.0f);
	_max_skill_points = GaussianRandomValue(_max_skill_points, _max_skill_points / 10.0f);
	_strength = GaussianRandomValue(_strength, _strength / 10.0f);
	_vigor = GaussianRandomValue(_strength, _strength / 10.0f);
	_fortitude = GaussianRandomValue(_fortitude, _fortitude / 10.0f);
	_resistance = GaussianRandomValue(_resistance, _resistance / 10.0f);
	_agility = GaussianRandomValue(_agility, _agility / 10.0f);

	// TODO CHECK Raging_Hog: _evade is between 0.0 to 1.0 and GaussianRandomValue expects integer??
	// I fixed it with a kludge, please fix. Don't think evasion works right now too well...
	//_evade = GaussianRandomValue(_evade, _evade / 10);
	_evade = static_cast<float>(GaussianRandomValue(static_cast<int32>(_evade), _evade / 10.0f));
	_experience_points = GaussianRandomValue(_experience_points, _experience_points / 10.0f);

	// The current hit points and skill points are automatically set to their new maximum value
	_hit_points = _max_hit_points;
	_skill_points = _max_skill_points;

	// TODO: Determine what skills, elemental bonuses, and status bonuses are unlocked at this new level
}


// ****************************************************************************
// ***** GlobalCharacter
// ****************************************************************************
// TODO CHECK Raging_Hog: _name doesn't exist and isn't needed anywhere? I'm taking it out damn it.
/*
GlobalCharacter::GlobalCharacter(hoa_utils::ustring name, std::string filename, uint32 id) :
	_name(name),
	_filename(filename),
	_id(id)
*/
GlobalCharacter::GlobalCharacter(const hoa_utils::ustring & name, const std::string & filename, uint32 id)
{
	_name = name;
	_filename = filename;
	_id = id;

	// TODO: Use the character's id or filename to look up the character's stats in a Lua file

// TODO CHECK Raging_Hog: were these functions just typed wrong? Some are also missing from global_actors.h

	// TEMP: Set character's stats
	/*
	SetMaxHP(300);
	SetHP(300);
	SetMaxSP(200);
	SetSP(200);
	SetXP(35);
	SetXPNextLevel(156);
	SetXPLevel(100);
	SetAgility(56);
	SetIntelligence(67);
	SetStrength(120);
	*/
	SetMaxHitPoints(300);
	SetHitPoints(300);
	SetMaxSkillPoints(200);
	SetSkillPoints(200);
	SetExperienceLevel(35);
	SetAgility(56);
	SetStrength(120);

	// TEMP: Add new skills
	AddAttackSkill(new GlobalSkill("sword_slash"));

	// Load the character's standard set of map sprite frames
	StillImage imd;
	imd.SetDimensions(1.0f, 2.0f);
	imd.SetFilename("img/sprites/map/" + _filename + "_d0.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_d1.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_d2.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_d3.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_d4.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_d5.png");
	_map_frames_standard.push_back(imd);

	imd.SetFilename("img/sprites/map/" + _filename + "_u0.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_u1.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_u2.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_u3.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_u4.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_u5.png");
	_map_frames_standard.push_back(imd);

	imd.SetFilename("img/sprites/map/" + _filename + "_l0.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_l1.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_l2.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_l3.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_l4.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_l5.png");
	_map_frames_standard.push_back(imd);

	imd.SetFilename("img/sprites/map/" + _filename + "_r0.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_r1.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_r2.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_r3.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_r4.png");
	_map_frames_standard.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_r5.png");
	_map_frames_standard.push_back(imd);

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < _map_frames_standard.size(); i++) {
		if (VideoManager->LoadImage(_map_frames_standard[i]) == false) {
			exit(1);
		}
	}
	VideoManager->EndImageLoadBatch();

	// Load the character's standard map portrait
	_map_portrait_standard.SetFilename("img/portraits/map/" + _filename + ".png");
	if (VideoManager->LoadImage(_map_portrait_standard) == false) {
		exit(1);
	}

	// Load the character's standard set of battle sprite frames
	vector<StillImage> idle_frames;
	AnimatedImage idle;
	imd.SetDimensions(64, 128);
	imd.SetFilename("img/sprites/battle/characters/" + _filename + "_idle_fr0.png");
	idle_frames.push_back(imd);
	imd.SetFilename("img/sprites/battle/characters/" + _filename + "_idle_fr1.png");
	idle_frames.push_back(imd);
	imd.SetFilename("img/sprites/battle/characters/" + _filename + "_idle_fr2.png");
	idle_frames.push_back(imd);
	imd.SetFilename("img/sprites/battle/characters/" + _filename + "_idle_fr3.png");
	idle_frames.push_back(imd);
	imd.SetFilename("img/sprites/battle/characters/" + _filename + "_idle_fr4.png");
	idle_frames.push_back(imd);
	imd.SetFilename("img/sprites/battle/characters/" + _filename + "_idle_fr5.png");
	idle_frames.push_back(imd);
		
	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < idle_frames.size(); i++) {
		if (!VideoManager->LoadImage(idle_frames[i])) cerr << "Failed to load battle sprite." << endl;
			
	}
	VideoManager->EndImageLoadBatch();

	for (uint32 i = 0; i < idle_frames.size(); i++) { idle.AddFrame(idle_frames[i], 10); }
	idle.SetFrameIndex(0);
	_battle_animation["idle"] = idle;

	// Load the character's battle portrait frames
	imd.SetDimensions(100, 100);
	imd.SetFilename("img/portraits/battle/" + _filename + ".png");
	_battle_portraits.push_back(imd);
	imd.SetFilename("img/portraits/battle/" + _filename + "_hp75.png");
	_battle_portraits.push_back(imd);
	imd.SetFilename("img/portraits/battle/" + _filename + "_hp50.png");
	_battle_portraits.push_back(imd);
	imd.SetFilename("img/portraits/battle/" + _filename + "_hp25.png");
	_battle_portraits.push_back(imd);
	imd.SetFilename("img/portraits/battle/" + _filename + "_hp00.png");
	_battle_portraits.push_back(imd);

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < _battle_portraits.size(); i++) {
		if (VideoManager->LoadImage(_battle_portraits[i]) == false)
			exit(1);
	}
	VideoManager->EndImageLoadBatch();

	// Load the character's menu portrait
	_menu_portrait.SetFilename("img/portraits/menu/" + _filename + ".png");
	if (VideoManager->LoadImage(_menu_portrait) == false) {
		exit(1);
	}
} // GlobalCharacter::GlobalCharacter(hoa_utils::ustring name, std::string filename, uint32 id)



// Remove all frame images upon destruction
GlobalCharacter::~GlobalCharacter() {
	// TODO: delete all dynamically allocated data
}

// ****************************************************************************
// ***** GlobalCharacterParty
// ****************************************************************************

void GlobalCharacterParty::AddCharacter(GlobalCharacter* character) {
	// Make sure that the character is not already in the party
	for (uint32 i = 0; i < _characters.size(); i++) {
		if (_characters[i] == character)
			return;
	}

	_characters.push_back(character);
} // void GlobalCharacterParty::AddCharacter(GlobalCharacter* character)



GlobalCharacter* GlobalCharacterParty::RemoveCharacter(GlobalCharacter* character) {
	GlobalCharacter* removed = NULL;

	for (uint32 i = 0; i < _characters.size(); i++) {
		if (_characters[i] == character) {
			removed = _characters[i];
		}

		// Shift all other characters in the vector if the character to remove has already been found
		if (removed != NULL) {
			_characters[i-1] = _characters[i];
		}
	}

	// If the removed character was found, remove the last element from the vector
	if (removed != NULL) {
		_characters.pop_back();
	}

	return removed;

} // GlobalCharacterParty* GlobalCharacterParty::RemoveCharacter(GlobalCharacter* character)

} // namespace hoa_global
