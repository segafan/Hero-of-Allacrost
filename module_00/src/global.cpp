///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    global.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 12th, 2005
 * \brief   Source file for the global game components.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include "global.h"
#include "video.h"
#include <string>

using namespace std;
using namespace hoa_video;
using namespace hoa_audio;
using namespace hoa_utils;

namespace hoa_global {

bool GLOBAL_DEBUG = false;
SINGLETON_INITIALIZE(GameInstance);

// ****************************************************************************
// ******************************* GObject ************************************
// ****************************************************************************

GObject::GObject(string name, uint8 type, uint32 usable, uint32 id, uint32 count) {
	obj_name = name;
	obj_type = type;
	usable_by = usable;
	obj_id = id;
	obj_count = count;
}



GObject::GObject() {
	obj_name = "unknown";
	obj_type = GLOBAL_DUMMY_OBJ;
	usable_by = GLOBAL_NO_CHARACTERS;
	obj_id = 0;
	obj_count = 0;
}



GObject::~GObject() {
	obj_count = 0;
}

// ****************************************************************************
// ******************************** GItem *************************************
// ****************************************************************************

GItem::GItem(string name, uint8 use, uint32 usable, uint32 id, uint32 count) :
	GObject(name, GLOBAL_ITEM, usable, id, count) {
	_use_case = use;
}



GItem::GItem() :
	GObject() {
	_use_case = GLOBAL_UNUSABLE_ITEM;
}



GItem::~GItem() {}

// ****************************************************************************
// ****************************** GSkillBook **********************************
// ****************************************************************************


GSkillBook::GSkillBook(string name, uint8 use, uint32 usable, uint32 id, uint32 count) :
	GObject(name, GLOBAL_SKILL_BOOK, usable, id, count) {
}



GSkillBook::GSkillBook() :
	GObject() {
	obj_type = GLOBAL_SKILL_BOOK;
}



GSkillBook::~GSkillBook() {}

// ****************************************************************************
// ******************************* GWeapon ************************************
// ****************************************************************************

GWeapon::GWeapon(string name, uint32 usable, uint32 id, uint32 count) :
	GObject(name, GLOBAL_WEAPON, usable, id, count) {
}



GWeapon::GWeapon() :
	GObject() {
	obj_type = GLOBAL_WEAPON;
}



GWeapon::~GWeapon() {}

// ****************************************************************************
// ******************************** GArmor ************************************
// ****************************************************************************

GArmor::GArmor(string name, uint8 type, uint32 usable, uint32 id, uint32 count) :
	GObject(name, type, usable, id, count) {
}



GArmor::GArmor() :
	GObject() {
}



GArmor::~GArmor() {}

// ****************************************************************************
// ******************************** GSkill ************************************
// ****************************************************************************

GSkill::GSkill(string name, uint32 sp) {
	_skill_name = name;
	_sp_usage = sp;
}



GSkill::GSkill() {
	_skill_name = "unknown";
	_sp_usage = 0;
}



GSkill::~GSkill() {}

// ****************************************************************************
// ****************************** GAttackPoint ********************************
// ****************************************************************************

GAttackPoint::GAttackPoint(float x, float y, uint32 def, uint32 eva, uint8 elem_weak,
                           uint8 elem_res, uint8 stat_weak, uint8 stat_res) {
	_x_position = x;
	_y_position = y;
	_defense = def;
	_evade = eva;
	_elemental_weakness = elem_weak;
	_elemental_resistance = elem_res;
	_status_weakness = stat_weak;
	_status_resistance = stat_res;
}



GAttackPoint::GAttackPoint() {
	_x_position = 0;
	_y_position = 0;
	_defense = 0;
	_evade = 0;
	_elemental_weakness = GLOBAL_NO_ELEMENTAL;
	_elemental_resistance = GLOBAL_NO_ELEMENTAL;
	_status_weakness = GLOBAL_NO_STATUS;
	_status_resistance = GLOBAL_NO_STATUS;
}



GAttackPoint::~GAttackPoint() {}

// ****************************************************************************
// ******************************** GEnemy ************************************
// ****************************************************************************

GEnemy::GEnemy() { }

GEnemy::~GEnemy() { }

// Simulate the leveling up of experience for the enemy from its base stats.
void GEnemy::LevelSimulator(uint32 lvl) {
	_experience_level = lvl;
	
	// Assign the initial values of the stats (== base + growth * lvl)
	_max_hit_points = _base_hit_points + (_growth_hit_points * lvl);
	_experience_points = _base_experience_points + (_growth_experience_points * lvl);
	_strength = _base_strength + (_growth_strength * lvl);
	_intelligence = _base_intelligence + (_growth_intelligence * lvl);
	_agility = _base_agility + (_growth_agility * lvl);

	// Randomize the stats using a guassian random variable, with the inital stats as the means
	_max_hit_points = GaussianValue(_max_hit_points, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	_experience_points = GaussianValue(_experience_points, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	_strength = GaussianValue(_strength, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	_intelligence = GaussianValue(_intelligence, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	_agility = GaussianValue(_agility, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
}

// ****************************************************************************
// ****************************** GCharacter **********************************
// ****************************************************************************

//
GCharacter::GCharacter(std::string na, std::string fn, uint32 id) {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GCharacter constructor invoked" << endl;
	_name = na;
	_filename = fn;
	_char_id = id;

	LoadFrames();
}


// Remove all frame images upon destruction
GCharacter::~GCharacter() {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GCharacter destructor invoked" << endl;
	GameVideo *VideoManager = GameVideo::GetReference();
	for (uint32 i = 0; i < _map_frames.size(); i++) {
		VideoManager->DeleteImage(_map_frames[i]);
	}
	for (uint32 i = 0; i < _battle_frames.size(); i++) {
		VideoManager->DeleteImage(_battle_frames[i]);
	}
}


void GCharacter::LoadFrames() {
	GameVideo *VideoManager = GameVideo::GetReference();
	ImageDescriptor imd;
	string full_name = "img/sprites/map/" + _filename;
	imd.SetDimensions(1.0f, 2.0f);

	imd.SetFilename(full_name + "_d1.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_d2.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_d3.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_d4.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_d5.png");
	_map_frames.push_back(imd);

	imd.SetFilename(full_name + "_u1.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_u2.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_u3.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_u4.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_u5.png");
	_map_frames.push_back(imd);

	imd.SetFilename(full_name + "_l1.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_l2.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_l3.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_l4.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_l5.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_l6.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_l7.png");
	_map_frames.push_back(imd);

	imd.SetFilename(full_name + "_r1.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_r2.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_r3.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_r4.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_r5.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_r6.png");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_r7.png");
	_map_frames.push_back(imd);

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < _map_frames.size(); i++) {
		VideoManager->LoadImage(_map_frames[i]);
	}
	VideoManager->EndImageLoadBatch();
}

// ****************************************************************************
// ***************************** GameInstance *********************************
// ****************************************************************************

GameInstance::GameInstance() {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GameInstance constructor invoked" << endl;
}

GameInstance::~GameInstance() {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GameInstance destructor invoked" << endl;
	for (uint32 i = 0; i < _characters.size(); i++) {
		delete _characters[i];
	}
}

// Initialize GameInstance members
bool GameInstance::Initialize() {
// 	VideoManager = GameVideo::GetReference();
	return true;
}

void GameInstance::AddCharacter(GCharacter *ch) {
	if (GLOBAL_DEBUG) cout << "GLOBAL: Adding new character to party: " << ch->GetName() << endl;
	_characters.push_back(ch);
}

GCharacter* GameInstance::GetCharacter(uint32 id) {
	for (uint32 i = 0; i < _characters.size(); i++) {
		if (_characters[i]->GetID() == id) {
			return _characters[i];
		}
	}
	if (GLOBAL_DEBUG) cerr << "GLOBAL WARNING: No character matching id #" << id << " found in party" << endl;
	return NULL;
}


}// namespace hoa_global
