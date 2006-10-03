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
#include "script.h"

#include "battle.h"

#include <string>
#include <sstream>


using namespace std;
using namespace hoa_video;
using namespace hoa_audio;
using namespace hoa_utils;
using namespace hoa_map;
using namespace hoa_battle;
using namespace hoa_script;

namespace hoa_global {

GameGlobal *GlobalManager = NULL;
bool GLOBAL_DEBUG = false;
SINGLETON_INITIALIZE(GameGlobal);

// ****************************************************************************
// ******************************* GlobalObject ************************************
// ****************************************************************************

GlobalObject::GlobalObject(uint8 type, uint32 usable, GameItemID id, uint32 count) 
{
	obj_name = GlobalManager->GetItemName(id);
	obj_type = type;
	usable_by = usable;
	obj_id = id;
	obj_count = count;
	_icon_path = GlobalManager->GetItemIconPath(id);
}

GlobalObject::~GlobalObject() {
	obj_count = 0;
}

// ****************************************************************************
// ******************************** GlobalItem *************************************
// ****************************************************************************

GlobalItem::GlobalItem(uint8 use, uint32 usable, GameItemID id, uint32 count) :
	GlobalObject(GLOBAL_ITEM, usable, id, count) {
	_use_case = use;
	this->_sub_class_type = "GlobalItem";
}

GlobalItem::~GlobalItem() 
{
}

// ****************************************************************************
// ******************************* GlobalWeapon ************************************
// ****************************************************************************

GlobalWeapon::GlobalWeapon(uint32 usable, GameItemID id, uint32 count) :
	GlobalObject(GLOBAL_WEAPON, usable, id, count) {
        
	_damage_amount = new GlobalStatusAfflictions();
        
	ScriptDescriptor read_data;
	string fileName = "dat/objects/" + GlobalManager->GetItemName(id) + ".lua";
	if (!read_data.OpenFile(fileName.c_str(), READ)) {
		cout << "GLOBAL ERROR: failed to load weapon file: " << GlobalManager->GetItemName(id) << endl;
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

GlobalWeapon::~GlobalWeapon() {
	if(_damage_amount != 0)
		delete _damage_amount;
	_damage_amount = 0;
}

// ****************************************************************************
// ******************************** GlobalArmor ************************************
// ****************************************************************************

GlobalArmor::GlobalArmor(uint8 type, uint32 usable, GameItemID id, uint32 count) :
	GlobalObject(type, usable, id, count) {
        
	/*
		float x, float y, uint32 volt, uint32 earth, uint32 water, uint32 fire, 
        uint32 piercing, uint32 slashing, uint32 bludgeoning
	*/
    ScriptDescriptor read_data;
    string fileName = "dat/objects/" + GlobalManager->GetItemName(id) + ".lua";
	if (!read_data.OpenFile(fileName.c_str(), READ)) {
		cout << "GLOBAL ERROR: failed to load weapon file: " << GlobalManager->GetItemName(id) << endl;
	}
    else {
		uint32 numAttackPoints = read_data.ReadInt("number_of_attack_points");
		for(uint32 i = 0; i < numAttackPoints; i++) {
			std::string s = read_data.ReadString(("name_"+i));
			float x = read_data.ReadFloat(("x_"+i));
			float y = read_data.ReadFloat(("y_"+i));
			uint32 volt = read_data.ReadInt(("volt_defense_"+i));
			uint32 earth = read_data.ReadInt(("earth_defense_"+i));
			uint32 water = read_data.ReadInt(("water_defense_"+i));
			uint32 fire = read_data.ReadInt(("fire_defense_"+i));
			uint32 piercing = read_data.ReadInt(("piercing_defense_"+i));
			uint32 slashing = read_data.ReadInt(("slashing_defense_"+i));
			uint32 bludgeoning = read_data.ReadInt(("bludgeoning_defense_"+i));
                        
			GlobalAttackPoint ap(s,x,y,volt,earth,water,fire,piercing,slashing,bludgeoning);
			_attack_points.push_back(ap);
		}
	}
}

GlobalArmor::~GlobalArmor() 
{
}

// ****************************************************************************
// ******************************** GlobalSkill ************************************
// ****************************************************************************

GlobalSkill::GlobalSkill(string script_name) {
	_script_name = script_name;
	
	ScriptDescriptor read_data;
	string fileName = "dat/skills/" + _script_name + ".lua";

	if (!read_data.OpenFile(fileName.c_str(), READ)) {
		cerr << "GLOBAL ERROR: failed to load skill file: " << _script_name << endl;
	}
	else {
		_skill_name = read_data.ReadString("skill_name");
		string type = read_data.ReadString("skill_type");

		if (type == "ATTACK")
			_skill_type = ATTACK;
		else if (type == "DEFENSE")
			_skill_type = DEFENSE;
		else if (type == "SUPPORT")
			_skill_type = SUPPORT;
		else {
			cerr << "GLOBAL ERROR: Unknown type for skill: " << _script_name << endl;
		}
		
		_sp_usage = read_data.ReadInt("sp_usage");
		_warmup_time = read_data.ReadInt("warmup_time");
		_cooldown_time = read_data.ReadInt("cooldown_time");
		_level_required = read_data.ReadInt("level_required");
		_num_arguments = read_data.ReadInt("num_arguments");
		
		_stats = new GlobalStatusAfflictions();
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

GlobalAttackPoint::GlobalAttackPoint(std::string name, float x, float y,
	uint32 volt, uint32 earth, uint32 water, uint32 fire,
	uint32 piercing, uint32 slashing, uint32 bludgeoning)
{
	_name = name;
	_x_position = x;
	_y_position = y;
	
	_resistance = new GlobalStatusAfflictions();
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

GlobalEnemy::GlobalEnemy(string file_name) :
	_file_name(file_name)
{
	ScriptDescriptor read_data;
	string fileName = "dat/enemies/" + _file_name + ".lua";
	if (!read_data.OpenFile(fileName.c_str(), READ)) {
		cerr << "GLOBAL ERROR: failed to load enemy file: " << _file_name << endl;
		return;
	}
	
	_enemy_id = read_data.ReadInt("id");
	_enemy_width = read_data.ReadInt("width");
	_enemy_height = read_data.ReadInt("height");
	uint32 numSkills = read_data.ReadInt("number_of_skills");
	for (uint32 i = 0; i < numSkills; i++) {
		_enemy_skills.push_back(new GlobalSkill(read_data.ReadString(("skill_" + i))));
	}
	
	uint32 numAnimations = read_data.ReadInt("number_of_animations");
	for (uint32 i = 0; i < numAnimations; i++) {
		string animationName = read_data.ReadString(("animation_name_" + i));
		vector<StillImage> animations;
		uint32 numFrames = read_data.ReadInt(("num_frames_" + i));
		for (uint32 j = 0; j < numFrames; j++) {
			string fileNameString = "file_name_" + i + '_' + j;
			string x_dimensionString = "x_dimension_" + i + '_' + j;
			string y_dimensionString = "y_dimension_" + i + '_' + j;
			string fileName = read_data.ReadString(fileNameString.c_str());
			uint32 x_dimension = read_data.ReadInt(x_dimensionString.c_str());
			uint32 y_dimension = read_data.ReadInt(y_dimensionString.c_str());
			
			StillImage i;
			i.SetFilename("img/sprites/battle/"+fileName);
			i.SetStatic(true);
			i.SetDimensions(static_cast<float>(x_dimension), static_cast<float>(y_dimension));

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
		
	int num_maps = read_data.ReadInt("number_of_maps");
	for (int i = 1; i <= num_maps; i++) {
		std::ostringstream os;
		os << "map_x_" << i;
		float x = read_data.ReadFloat(os.str().c_str());
		os.str("");
		os << "map_y_" << i;
		float y = read_data.ReadFloat(os.str().c_str());
		os.str("");
		os << "map_name_" << i;
		std::string name = read_data.ReadString(os.str().c_str());
		
		GlobalAttackPoint *gap = new GlobalAttackPoint(name,x,y,0,0,0,0,0,0,0);
		_attack_points.push_back(gap);
	}
} // GlobalEnemy::~GlobalEnemy()

GlobalEnemy::~GlobalEnemy() { 
        ///delete animations
        //delete global attack points
}

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
	_max_hit_points = GaussianRandomValue(_max_hit_points);
	_experience_points = GaussianRandomValue(_experience_points);
	_strength = GaussianRandomValue(_strength);
	_intelligence = GaussianRandomValue(_intelligence);
	_agility = GaussianRandomValue(_agility);
	
	_hit_points = _max_hit_points;

	// Make sure there's always at least one hit point!
	if (_hit_points <= 0) 
		_hit_points = 1;
	if (_max_hit_points <= 0) 
		_max_hit_points = 1;

	//_movement_speed = 5 + _agility%5;
}

// ****************************************************************************
// ****************************** GlobalCharacter **********************************
// ****************************************************************************

//
GlobalCharacter::GlobalCharacter(hoa_utils::ustring na, std::string fn, uint32 id) {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GlobalCharacter constructor invoked" << endl;
	_name = na;
	_filename = fn;
	_char_id = id;

	// Load the character's standard set of map sprite frames (24 count)
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

	// Load the character's standard map portrait
	_map_portrait.SetFilename("img/portraits/map/" + _filename + ".png");
	if (! VideoManager->LoadImage(_map_portrait)) exit(1);

	// Load the character's standard set of battle sprite frames (24 count)
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
		if (!VideoManager->LoadImage(idle_frames[i])) cerr << "Failed to load claudius image." << endl;
			
	}
	VideoManager->EndImageLoadBatch();

	for (uint32 i = 0; i < idle_frames.size(); i++) { idle.AddFrame(idle_frames[i], 10); }
	idle.SetFrameIndex(0);
	_battle_animation["IDLE"] = idle;

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
		if (!VideoManager->LoadImage(_battle_portraits[i]))
			cerr << "Failed to load battle portrait." << endl;
	}
	VideoManager->EndImageLoadBatch();

	// Load the character's menu portrait
	_menu_portrait.SetFilename("img/portraits/menu/" + _filename + ".png");
	if (! VideoManager->LoadImage(_menu_portrait)) exit(1);

	// TEMP: Set character's stats
	SetMaxHP(200);
	SetHP(200);
	SetMaxSP(200);
	SetSP(147);
	SetXP(35);
	SetXPNextLevel(156);
	SetXPLevel(100);
	SetAgility(56);
	SetIntelligence(67);
	SetStrength(120);

	// TEMP: Add new skills
	AddAttackSkill(new GlobalSkill("sword_swipe"));

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
	
	SetItemName(HP_POTION, "HP Potion");
	SetItemIconPath(HP_POTION, "img/icons/items/health_potion.png");
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
bool GameGlobal::SingletonInitialize() {
	return true;
}

void GameGlobal::AddCharacter(GlobalCharacter *ch) {
	if (GLOBAL_DEBUG) cout << "GLOBAL: Adding new character to party: " << MakeStandardString(ch->GetName()) << endl;
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

//-------------------------------------
// GlobalParty::RemoveCharacter
//-------------------------------------
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

//-------------------------------------
// GlobalParty::AddItemToInventory
//-------------------------------------
void GameGlobal::AddItemToInventory(GameItemID id)
{
	vector<GlobalObject *>::iterator i = _inventory.begin();
	for (; i != _inventory.end(); i++)
	{
		if ((*i)->GetID() == id)
		{
			(*i)->SetCount((*i)->GetCount() + 1);
			return;
		}
	}
	
	//////////////////////////////////////////////////
	// This needs to be a lua call to load the object
	//////////////////////////////////////////////////
	// hack to create potion for demo
	if (id == HP_POTION)
	{
		GlobalItem *potion = new GlobalItem(GLOBAL_HP_RECOVERY_ITEM, GLOBAL_ALL_CHARACTERS, HP_POTION, 1);
		potion->SetRecoveryAmount(180);
		_inventory.push_back(potion);
	}
}

//-------------------------------------
// GlobalParty::RemoveItemFromInventory
//-------------------------------------
void GameGlobal::RemoveItemFromInventory(GameItemID id)
{
	// search through inventory and remove item
	vector<GlobalObject *>::iterator i = _inventory.begin();
	for (; i != _inventory.end(); i++)
	{
		if ((*i)->GetID() == id)
			break;
	}

	_inventory.erase(i);
}

}// namespace hoa_global
