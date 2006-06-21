///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    global.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \brief   Source file for the global game components.
 *****************************************************************************/

#include <iostream>
#include "global.h"
#include "utils.h"
#include "video.h"
#include "data.h"

#include "battle.h"

#include <string>


using namespace std;
using namespace hoa_video;
using namespace hoa_audio;
using namespace hoa_utils;
using namespace hoa_map;
using namespace hoa_battle;
using namespace hoa_data;

namespace hoa_global {

GameGlobal *GlobalManager = NULL;
bool GLOBAL_DEBUG = false;
SINGLETON_INITIALIZE(GameGlobal);

// ****************************************************************************
// ******************************* GlobalObject ************************************
// ****************************************************************************

GlobalObject::GlobalObject(string name, uint8 type, uint32 usable, uint32 id, uint32 count, string icon_path) {
	obj_name = name;
	obj_type = type;
	usable_by = usable;
	obj_id = id;
	obj_count = count;
	_icon_path = icon_path;
}



GlobalObject::GlobalObject() {
	obj_name = "unknown";
	obj_type = GLOBAL_DUMMY_OBJ;
	usable_by = GLOBAL_NO_CHARACTERS;
	obj_id = 0;
	obj_count = 0;
	_sub_class_type = "GlobalObject";
}



GlobalObject::~GlobalObject() {
	obj_count = 0;
}

// ****************************************************************************
// ******************************** GlobalItem *************************************
// ****************************************************************************

GlobalItem::GlobalItem(string name, uint8 use, uint32 usable, uint32 id, uint32 count, string icon_path) :
	GlobalObject(name, GLOBAL_ITEM, usable, id, count, icon_path) {
	_use_case = use;
	this->_sub_class_type = "GlobalItem";
}



GlobalItem::GlobalItem() :
	GlobalObject() {
	_use_case = GLOBAL_UNUSABLE_ITEM;
	this->_sub_class_type = "GlobalItem";
}



GlobalItem::~GlobalItem() {}

// ****************************************************************************
// ******************************* GlobalWeapon ************************************
// ****************************************************************************

GlobalWeapon::GlobalWeapon(string name, uint32 usable, uint32 id, uint32 count, string icon_path) :
	GlobalObject(name, GLOBAL_WEAPON, usable, id, count, icon_path) {
        
        _damage_amount = new BattleStatTypes();
        
        ReadDataDescriptor read_data;
        string fileName = "dat/objects/" + name + ".lua";
	if (!read_data.OpenFile(fileName.c_str())) {
		cout << "GLOBAL ERROR: failed to load weapon file: " << name << endl;
	}
        else {
                _damage_amount->volt = read_data.ReadInt("volt_damage");
                _damage_amount->earth = read_data.ReadInt("earth_damage");
                _damage_amount->water = read_data.ReadInt("water_damage");
                _damage_amount->fire = read_data.ReadInt("fire_damage");
                _damage_amount->piercing = read_data.ReadInt("piercing_damage");
                _damage_amount->slashing = read_data.ReadInt("slashing_damage");
                _damage_amount->bludgeoning = read_data.ReadInt("bludgeoning_damage");
        }
}


GlobalWeapon::GlobalWeapon() :
	GlobalObject() {
	obj_type = GLOBAL_WEAPON;
        _damage_amount = 0;
		this->_sub_class_type = "GlobalWeapon";
}



GlobalWeapon::~GlobalWeapon() {
        if(_damage_amount != 0)
                delete _damage_amount;
        _damage_amount = 0;
}

// ****************************************************************************
// ******************************** GlobalArmor ************************************
// ****************************************************************************

GlobalArmor::GlobalArmor(string name, uint8 type, uint32 usable, uint32 id, uint32 count, string icon_path) :
	GlobalObject(name, type, usable, id, count, icon_path) {
        
        /*
        float x, float y, uint32 volt, uint32 earth, uint32 water, uint32 fire, 
                                        uint32 piercing, uint32 slashing, uint32 bludgeoning
        */
        ReadDataDescriptor read_data;
        string fileName = "dat/objects/" + name + ".lua";
	if (!read_data.OpenFile(fileName.c_str())) {
		cout << "GLOBAL ERROR: failed to load weapon file: " << name << endl;
	}
        else {
                uint32 numAttackPoints = read_data.ReadInt("number_of_attack_points");
                for(uint32 i = 0; i < numAttackPoints; i++) {
                        float x = read_data.ReadFloat(("x_"+i));
                        float y = read_data.ReadFloat(("y_"+i));
                        uint32 volt = read_data.ReadInt(("volt_defense_"+i));
                        uint32 earth = read_data.ReadInt(("earth_defense_"+i));
                        uint32 water = read_data.ReadInt(("water_defense_"+i));
                        uint32 fire = read_data.ReadInt(("fire_defense_"+i));
                        uint32 piercing = read_data.ReadInt(("piercing_defense_"+i));
                        uint32 slashing = read_data.ReadInt(("slashing_defense_"+i));
                        uint32 bludgeoning = read_data.ReadInt(("bludgeoning_defense_"+i));
                        
                        GlobalAttackPoint ap(x,y,volt,earth,water,fire,piercing,slashing,bludgeoning);
                        _attack_points.push_back(ap);
                }
        }
}



GlobalArmor::GlobalArmor() :
	GlobalObject() {
		this->_sub_class_type = "GlobalArmor";
}



GlobalArmor::~GlobalArmor() {}

// ****************************************************************************
// ******************************** GlobalSkill ************************************
// ****************************************************************************

GlobalSkill::GlobalSkill(string script_name) {
	_script_name = script_name;
        
        ReadDataDescriptor read_data;
        string fileName = "dat/skills/" + _script_name + ".lua";
	if (!read_data.OpenFile(fileName.c_str())) {
		cerr << "GLOBAL ERROR: failed to load skill file: " << _script_name << endl;
	}
        else { 
                _skill_name = read_data.ReadString("skill_name");
                string type = read_data.ReadString("skill_type");

                if(type == "ATTACK")
                        _skill_type = ATTACK;
                else if(type == "DEFENSE")
                        _skill_type = DEFENSE;
                else if(type == "SUPPORT")
                        _skill_type = SUPPORT;
                else {
                        cerr << "GLOBAL ERROR: Unknown type for skill: " << _script_name << endl;
                }
                              
                _sp_usage = read_data.ReadInt("sp_usage");
                _warmup_time = read_data.ReadInt("warmup_time");
                _cooldown_time = read_data.ReadInt("cooldown_time");  
                _level_required = read_data.ReadInt("level_required");
                _num_arguments = read_data.ReadInt("num_arguments");
                 
                _stats = new BattleStatTypes();
                _stats->volt = read_data.ReadInt("volt_level");
                _stats->earth = read_data.ReadInt("earth_level");
                _stats->water = read_data.ReadInt("water_level");
                _stats->fire = read_data.ReadInt("fire_level");
                _stats->piercing = read_data.ReadInt("piercing_level");
                _stats->slashing = read_data.ReadInt("slashing_level");
                _stats->bludgeoning = read_data.ReadInt("bludgeoning_level");
        }
}



GlobalSkill::GlobalSkill() {
	_skill_name = "unknown";
	_sp_usage = 0;
}



GlobalSkill::~GlobalSkill() {
        if(_stats)
                delete _stats;
        _stats = 0;
}

// ****************************************************************************
// ****************************** GlobalAttackPoint ********************************
// ****************************************************************************

GlobalAttackPoint::GlobalAttackPoint(float x, float y, uint32 volt, uint32 earth, uint32 water, uint32 fire, 
                                        uint32 piercing, uint32 slashing, uint32 bludgeoning) {
	_x_position = x;
	_y_position = y;
	
        _resistance = new BattleStatTypes();
        _resistance->volt = volt;
        _resistance->earth = earth;
        _resistance->water = water;
        _resistance->fire = fire;
        _resistance->piercing = piercing;
        _resistance->slashing = slashing;
        _resistance->bludgeoning = bludgeoning;
}



GlobalAttackPoint::GlobalAttackPoint() {
	_x_position = 0;
	_y_position = 0;
	
        _resistance = 0;
}



GlobalAttackPoint::~GlobalAttackPoint() {
        if(_resistance != 0)
                delete _resistance;
        _resistance = 0;
}

// ****************************************************************************
// ******************************** GlobalEnemy ************************************
// ****************************************************************************

GlobalEnemy::GlobalEnemy(string file_name) {
        _file_name = file_name;
        
         ReadDataDescriptor read_data;
        string fileName = "dat/enemies/" + _file_name + ".lua";
	if (!read_data.OpenFile(fileName.c_str())) {
		cout << "GLOBAL ERROR: failed to load enemy file: " << _file_name << endl;
	}
        else {
                _enemy_id = read_data.ReadInt("id");
                _enemy_width = read_data.ReadInt("width");
                _enemy_height = read_data.ReadInt("height");
                uint32 numSkills = read_data.ReadInt("number_of_skills");
                for(uint32 i = 0; i < numSkills; i++) {
                        _enemy_skills.push_back(new GlobalSkill(read_data.ReadString(("skill_" + i))));
                }
                
                uint32 numAnimations = read_data.ReadInt("number_of_animations");
                for(uint32 i = 0; i < numAnimations; i++) {
                        string animationName = read_data.ReadString(("animation_name_" + i));
                        vector<StillImage> animations;
                        uint32 numFrames = read_data.ReadInt(("num_frames_" + i));
                        for(uint32 j = 0; j < numFrames; j++) {
                                string fileNameString = "file_name_" + i + '_' + j;
                                string x_dimensionString = "x_dimension_" + i + '_' + j;
                                string y_dimensionString = "y_dimension_" + i + '_' + j;
                                string fileName = read_data.ReadString(fileNameString.c_str());
                                uint32 x_dimension = read_data.ReadInt(x_dimensionString.c_str());
                                uint32 y_dimension = read_data.ReadInt(y_dimensionString.c_str());
                                
                                StillImage i;
                                i.SetFilename("img/sprites/battle/"+fileName);
                                i.SetStatic(true);
                                i.SetDimensions(x_dimension, y_dimension);

                                VideoManager->LoadImage(i);
                                animations.push_back(i);
                        }
                        _sprite_animations[animationName] = animations;
                }
                
                _movement_speed = read_data.ReadInt("movement_speed");
                _base_hit_points = read_data.ReadInt("base_hit_points");
                _base_skill_points = read_data.ReadInt("base_skill_points");
                _base_experience_points = read_data.ReadInt("base_experience_points");
                _base_strength = read_data.ReadInt("base_strength");
                _base_intelligence = read_data.ReadInt("base_intelligence");
                _base_agility = read_data.ReadInt("base_agility");
                _growth_hit_points = read_data.ReadInt("growth_hit_points");
                _growth_skill_points = read_data.ReadInt("growth_skill_points");
                _growth_experience_points = read_data.ReadInt("growth_experience_points");
                _growth_strength = read_data.ReadInt("growth_strength");
                _growth_intelligence = read_data.ReadInt("growth_intelligence");
                _growth_agility = read_data.ReadInt("growth_agility");
        }
}

GlobalEnemy::~GlobalEnemy() { }

// Simulate the leveling up of experience for the enemy from its base stats.
void GlobalEnemy::LevelSimulator(uint32 lvl) {
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
	
	_hit_points = _max_hit_points;
	//_movement_speed = 5 + _agility%5;
}

// ****************************************************************************
// ****************************** GlobalCharacter **********************************
// ****************************************************************************

//
GlobalCharacter::GlobalCharacter(std::string na, std::string fn, uint32 id) {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GlobalCharacter constructor invoked" << endl;
	_name = na;
	_filename = fn;
	_char_id = id;

	// Prepare standard sprite animation frames (24 count)
	StillImage imd;
	imd.SetDimensions(1.0f, 2.0f);
	imd.SetFilename("img/sprites/map/" + _filename + "_d0.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_d1.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_d2.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_d3.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_d4.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_d5.png");
	_map_frames.push_back(imd);

	imd.SetFilename("img/sprites/map/" + _filename + "_u0.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_u1.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_u2.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_u3.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_u4.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_u5.png");
	_map_frames.push_back(imd);

	imd.SetFilename("img/sprites/map/" + _filename + "_l0.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_l1.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_l2.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_l3.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_l4.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_l5.png");
	_map_frames.push_back(imd);

	imd.SetFilename("img/sprites/map/" + _filename + "_r0.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_r1.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_r2.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_r3.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_r4.png");
	_map_frames.push_back(imd);
	imd.SetFilename("img/sprites/map/" + _filename + "_r5.png");
	_map_frames.push_back(imd);

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < _map_frames.size(); i++) {
		VideoManager->LoadImage(_map_frames[i]);
	}
	VideoManager->EndImageLoadBatch();
        
        /*
        std::vector<hoa_video::StillImage> claudiusAnimation;
        StillImage si;
        si.SetDimensions(64, 128);
        si.SetFilename("img/sprites/battle/claudius_idle.png");
        claudiusAnimation.push_back(si);

        if(!VideoManager->LoadImage(claudiusAnimation[0]))
                cerr << "Failed to load claudius image." << endl; //failed to laod image
        else
                 AddAnimation("IDLE", claudiusAnimation);
	
        std::cout << this << std::endl;
        std::cout << _battle_animation.size() << std::endl;
        */
        
       _movement_speed = 5;
}


// Remove all frame images upon destruction
GlobalCharacter::~GlobalCharacter() {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GlobalCharacter destructor invoked" << endl;
	/*for (uint32 i = 0; i < _battle_frames.size(); i++) {
		VideoManager->DeleteImage(_battle_frames[i]);
	}*/
	
	//std::map<std::string, hoa_video::AnimatedImage>::iterator it = _battle_animation.begin();
	//for(; it != _battle_animation.end(); it++) {
		//VideoManager->DeleteImage((*it->second));
	//}
}

// Add xp and increase lvl if necessary
void GlobalCharacter::AddXP(uint32 xp)
{
	_experience_points += xp;
	_experience_next_level -= xp;
	// TODO: Consult lvl chart and take appropriate action
	// Replace amount_required variables with the right values
	//if (_experience_points >= amount_required_to_level_up)
	//{
	//	_IncreaseCharacterAttributes();
	//	_experience_points = _experience_points - amount_required_to_level_up;
	//	_experience_next_level = amount_required_for_next_level - _experience_points;
	//}
}

// ****************************************************************************
// ***************************** GameGlobal *********************************
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
}

// Initialize GameGlobal members
bool GameGlobal::Initialize() {
// 	VideoManager = GameVideo::GetReference();
	return true;
}

void GameGlobal::AddCharacter(GlobalCharacter *ch) {
	if (GLOBAL_DEBUG) cout << "GLOBAL: Adding new character to party: " << ch->GetName() << endl;
	_characters.push_back(ch);
	// Check size of active party if less then 4, add to party
	if (_party.GetPartySize() < 4)
		_party.AddCharacter(ch->GetID());
}

GlobalCharacter* GameGlobal::GetCharacter(uint32 id) {
	for (uint32 i = 0; i < _characters.size(); i++) {
		if (_characters[i]->GetID() == id) {
			return _characters[i];
		}
	}
	if (GLOBAL_DEBUG) cerr << "GLOBAL WARNING: No character matching id #" << id << " found in party" << endl;
	return NULL;
}

//-------------------------------
// GameGlobal::GetMoney
//-------------------------------
uint32 GameGlobal::GetMoney()
{
	return _money;
}

//------------------------------
// GameGlobal::SetMoney
//------------------------------
void GameGlobal::SetMoney(uint32 amount)
{
	_money = amount;
}

//-----------------------------
// GameGlobal::AddMoney
//-----------------------------
void GameGlobal::AddMoney(uint32 amount)
{
	_money += amount;
}

//------------------------------
// GameGlobal::SubtractMoney
//------------------------------
void GameGlobal::SubtractMoney(uint32 amount)
{
	// check to make amount is less then current amount of money
	if (amount <= _money)
		_money -= amount;
	else
		if (GLOBAL_DEBUG) cerr << "GLOBAL: SubtractMoney tried to subtract more money then we had! Current amount left alone." << endl;
}

//----------------------------------------------
// GameGlobal::GetParty
//----------------------------------------------
vector<GlobalCharacter *> GameGlobal::GetParty()
{
	vector<uint32> characters = _party.GetCharacters();
	vector<GlobalCharacter *> ret;
	for (vector<uint32>::iterator p = characters.begin(); p != characters.end(); ++p)
		ret.push_back(GetCharacter((*p)));
	
	return ret;
}

//-----------------------
// GlobalParty::GlobalParty
//-----------------------
GlobalParty::GlobalParty()
{
	// Nothing to do here yet.
}

//-----------------------
// GlobalParty::~GlobalParty
//-----------------------
GlobalParty::~GlobalParty()
{
	// Nothing to do here yet.
}

//-------------------------------------
// GlobalParty::AddCharacter
//-------------------------------------
void GlobalParty::AddCharacter(uint32 char_id)
{
	// Only add the new char if there is less then 4 members in the party.
	if (_characters.size() < 4)
		_characters.push_back(char_id);
	else
		cerr << "GLOBAL: Unable to add another char to party, it is already at 4 members!" << endl;
}

void GlobalParty::RemoveCharacter(uint32 char_id)
{
	// search for id and remove it
	for (vector<uint32>::iterator p = _characters.begin(); p != _characters.end(); ++p)
	{
		if ((*p) == char_id)
		{
			_characters.erase(p);
			return;
		}
	}
	if (GLOBAL_DEBUG) cerr << "GLOBAL: No Character matching " << char_id << " found!" << endl;
}

}// namespace hoa_global
