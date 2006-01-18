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

#include <iostream>
#include "global.h"
#include "utils.h"
#include "video.h"

//<<<<<<< global.cpp
#include "battle_actions.h"
#include <string>
//=======

//>>>>>>> 1.22

using namespace std;
using namespace hoa_video;
using namespace hoa_audio;
using namespace hoa_utils;
using namespace hoa_map;

namespace hoa_global {

GameGlobal *GlobalManager = NULL;
bool GLOBAL_DEBUG = false;
SINGLETON_INITIALIZE(GameGlobal);

// ****************************************************************************
// ******************************* GlobalObject ************************************
// ****************************************************************************

GlobalObject::GlobalObject(string name, uint8 type, uint32 usable, uint32 id, uint32 count) {
	obj_name = name;
	obj_type = type;
	usable_by = usable;
	obj_id = id;
	obj_count = count;
}



GlobalObject::GlobalObject() {
	obj_name = "unknown";
	obj_type = GLOBAL_DUMMY_OBJ;
	usable_by = GLOBAL_NO_CHARACTERS;
	obj_id = 0;
	obj_count = 0;
}



GlobalObject::~GlobalObject() {
	obj_count = 0;
}

// ****************************************************************************
// ******************************** GlobalItem *************************************
// ****************************************************************************

GlobalItem::GlobalItem(string name, uint8 use, uint32 usable, uint32 id, uint32 count) :
	GlobalObject(name, GLOBAL_ITEM, usable, id, count) {
	_use_case = use;
}



GlobalItem::GlobalItem() :
	GlobalObject() {
	_use_case = GLOBAL_UNUSABLE_ITEM;
}



GlobalItem::~GlobalItem() {}

// ****************************************************************************
// ******************************* GlobalWeapon ************************************
// ****************************************************************************

GlobalWeapon::GlobalWeapon(string name, uint32 usable, uint32 id, uint32 count) :
	GlobalObject(name, GLOBAL_WEAPON, usable, id, count) {
}



GlobalWeapon::GlobalWeapon() :
	GlobalObject() {
	obj_type = GLOBAL_WEAPON;
}



GlobalWeapon::~GlobalWeapon() {}

// ****************************************************************************
// ******************************** GlobalArmor ************************************
// ****************************************************************************

GlobalArmor::GlobalArmor(string name, uint8 type, uint32 usable, uint32 id, uint32 count) :
	GlobalObject(name, type, usable, id, count) {
}



GlobalArmor::GlobalArmor() :
	GlobalObject() {
}



GlobalArmor::~GlobalArmor() {}

// ****************************************************************************
// ******************************** GlobalSkill ************************************
// ****************************************************************************

GlobalSkill::GlobalSkill(string name, uint32 sp) {
	_skill_name = name;
	_sp_usage = sp;
}



GlobalSkill::GlobalSkill() {
	_skill_name = "unknown";
	_sp_usage = 0;
}



GlobalSkill::~GlobalSkill() {
	for(unsigned int i = 0; i < _actions.size(); i++) {
		delete _actions[i];
	}
}

void GlobalSkill::PerformSkill(hoa_battle::Actor *a, std::vector<hoa_battle::Actor *> args) {
	_host = a;
	//add all the actions to the player to perform
	for(int i = 0; i < _arguments.size(); i++) {
		_actions[i]->Initialize(this, a, args);
		a->AddBattleAction(_actions[i]);
	}
	//this will destruct the skill
	hoa_battle::FinishSkill *fa = new hoa_battle::FinishSkill();
	fa->Initialize(this, a, args);
	a->AddBattleAction(fa);
}

void GlobalSkill::AddBattleAction(hoa_battle::BattleAction *bsa) {
	_actions.push_back(bsa);
}

uint32 GlobalSkill::GetCooldownTime() {
	return _coolDownTime;
}

uint32 GlobalSkill::GetWarmupTime() {
	return _warmUpTime;
}

// ****************************************************************************
// ****************************** GlobalAttackPoint ********************************
// ****************************************************************************

GlobalAttackPoint::GlobalAttackPoint(float x, float y, uint32 def, uint32 eva, uint8 elem_weak,
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



GlobalAttackPoint::GlobalAttackPoint() {
	_x_position = 0;
	_y_position = 0;
	_defense = 0;
	_evade = 0;
	_elemental_weakness = GLOBAL_NO_ELEMENTAL;
	_elemental_resistance = GLOBAL_NO_ELEMENTAL;
	_status_weakness = GLOBAL_NO_STATUS;
	_status_resistance = GLOBAL_NO_STATUS;
}



GlobalAttackPoint::~GlobalAttackPoint() {}

// ****************************************************************************
// ******************************** GlobalEnemy ************************************
// ****************************************************************************

GlobalEnemy::GlobalEnemy() { }

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
}


// Remove all frame images upon destruction
GlobalCharacter::~GlobalCharacter() {
	if (GLOBAL_DEBUG) cout << "GLOBAL: GlobalCharacter destructor invoked" << endl;
	GameVideo *VideoManager = GameVideo::GetReference();
	for (uint32 i = 0; i < _battle_frames.size(); i++) {
		VideoManager->DeleteImage(_battle_frames[i]);
	}
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

/*
<<<<<<< global.cpp

void GlobalCharacter::LoadFrames() {
	GameVideo *VideoManager = GameVideo::GetReference();
	StillImage imd;
	string full_name = "img/sprites/map/" + _filename;
	imd.SetDimensions(1.0f, 2.0f);

	imd.SetFilename(full_name + "_d1.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_d2.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_d3.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_d4.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_d5.jpg");
	_map_frames.push_back(imd);

	imd.SetFilename(full_name + "_u1.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_u2.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_u3.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_u4.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_u5.jpg");
	_map_frames.push_back(imd);

	imd.SetFilename(full_name + "_l1.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_l2.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_l3.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_l4.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_l5.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_l6.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_l7.jpg");
	_map_frames.push_back(imd);

	imd.SetFilename(full_name + "_r1.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_r2.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_r3.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_r4.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_r5.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_r6.jpg");
	_map_frames.push_back(imd);
	imd.SetFilename(full_name + "_r7.jpg");
	_map_frames.push_back(imd);

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < _map_frames.size(); i++) {
		VideoManager->LoadImage(_map_frames[i]);
	}
	VideoManager->EndImageLoadBatch();
}

=======
*/
//>>>>>>> 1.22
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
