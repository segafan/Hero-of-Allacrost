////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_actors.cpp
*** \author  Viljami Korhonen, mindflayer@allacrost.org
*** \author  Corey Hoffstein, visage@allacrost.org
*** \author  Andy Gardner, chopperdave@allacrost.org
*** \brief   Source file for actors present in battles.
*** ***************************************************************************/

#include <iostream>
#include <sstream>

#include "utils.h"
#include "audio.h"
#include "video.h"
#include "system.h"
#include "global.h"
#include "script.h"
#include "battle.h"
#include "battle_actors.h"

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_script;

using namespace hoa_battle::private_battle;

namespace hoa_battle {

namespace private_battle {

// *****************************************************************************
// BattleActor class
// *****************************************************************************

BattleActor::BattleActor() : _total_time_damaged(0),
	_damage_dealt(0),
	_is_queued_to_perform(false)
{
	//Load time portrait selector
	_time_portrait_selected.SetDimensions(45,45);
	_time_portrait_selected.SetFilename("img/menus/stamina_icon_selected.png");
	VideoManager->LoadImage(_time_portrait_selected);

	// Reset attack timer, TEMP CODE!!!!
	_TEMP_attack_animation_timer.SetDuration(0);
	_TEMP_attack_animation_timer.Reset();
	_TEMP_attack_animation_timer.Play();
}

BattleActor::~BattleActor()
{
	VideoManager->DeleteImage(_time_portrait_selected);
}

//Copies stats from the GlobalActor handle that is passed in
void BattleActor::InitBattleActorStats(hoa_global::GlobalActor* actor)
{
	SetMaxHitPoints(actor->GetMaxHitPoints());
	SetMaxSkillPoints(actor->GetMaxSkillPoints());
	SetHitPoints(actor->GetHitPoints());
	SetSkillPoints(actor->GetSkillPoints());
	SetStrength(actor->GetStrength());
	SetVigor(actor->GetVigor());
	SetFortitude(actor->GetFortitude());
	SetProtection(actor->GetProtection());
	SetAgility(actor->GetAgility());
	SetEvade(actor->GetEvade());

	CalcPhysicalAttack();
	CalcMetaPhysicalAttack();
	CalcPhysicalDefense();
	CalcMetaPhysicalDefense();
}

// Draws the character's portrait on the time meter
void BattleActor::DrawTimePortrait(bool is_selected) {
	if (IsAlive()) {
		VideoManager->Move(995, _time_portrait_location);

		VideoManager->DrawImage(_time_meter_portrait);

		if (is_selected)
			VideoManager->DrawImage(_time_portrait_selected);
	}
}

//Reset actor wait time
void BattleActor::ResetWaitTime()
{
	_wait_time.Reset();
	_wait_time.Play();

	//Sets time meter portrait position
	_time_portrait_location = 128.f;
}


//For when he dies
void BattleActor::OnDeath()
{
	GetWaitTime()->Reset();
}

//For when he is given new life! LIFE!
void BattleActor::OnLife()
{
	GetWaitTime()->Play();
}

// Gives a specific amount of damage for the actor
// Switched from uint to int to allow for nullified attacks
//(i.e. attacks where damage dealt is < defense)
void BattleActor::TakeDamage(int32 damage)
{
	_total_time_damaged = 1;

	if (damage <= 0)
	{
		_damage_dealt = RandomBoundedInteger(1, 4);
		return;
	}

	_damage_dealt = damage;

	if (static_cast<uint32>(damage) >= GetHitPoints()) // Was it a killing blow?
	{
		SetHitPoints(0);
		OnDeath();
		current_battle->RemoveScriptedEventsForActor(this);
	}
	else {
		SetHitPoints(GetHitPoints() - damage);
	}
}

void BattleActor::TEMP_ResetAttackTimer()
{
	_TEMP_attack_animation_timer.SetDuration(1000);
	_TEMP_attack_animation_timer.Reset();
	_TEMP_attack_animation_timer.Play();
}

// Is the actor attacking right now
bool BattleActor::TEMP_IsAttacking() const
{
	return !_TEMP_attack_animation_timer.HasExpired();
}

// *****************************************************************************
// BattleCharacterActor class
// *****************************************************************************
BattleCharacterActor::BattleCharacterActor(GlobalCharacter * character, float x_location, float y_location) :
	BattleActor(),
	_global_character(character)
	/*_x_location(x_location),
	_y_location(y_location),
	_x_origin(_x_location),
	_y_origin(_y_location)*/
	//_total_time_damaged(0),
	//_damage_dealt(0),
	//_is_queued_to_perform(false)
{
	//Cannot initalize protected members in init list for some reason
	_x_location = x_location;
	_y_location = y_location;
	_x_origin = _x_location;
	_y_origin = _y_location;
	//FIX ME
	_time_meter_portrait.SetFilename("img/icons/actors/characters/" + character->GetFilename() + ".png");
	_time_meter_portrait.SetDimensions(45,45);
	VideoManager->LoadImage(_time_meter_portrait);

	//Load time portrait selector
	/*_time_portrait_selected.SetDimensions(45,45);
	_time_portrait_selected.SetFilename("img/menus/stamina_icon_selected.png");
	VideoManager->LoadImage(_time_portrait_selected);*/

	//_time_portrait_location = 128;
	//FIX ME Use char stats to determine wait time
	//FIX ME #2 Do not initialize here.  Have BattleMode loop through all actors
	// and scale wait time based on slowest actor
	//_wait_time.SetDuration(5000);
	//ResetWaitTime();
	//_wait_time.Play();
	//_action_state = ACTION_IDLE;

	// Load images for the down menu
	_status_bar_cover_image.SetFilename("img/menus/bar_cover.png");
	VideoManager->LoadImage(_status_bar_cover_image);

	_status_menu_image.SetFilename("img/menus/battle_character_menu.png");
	VideoManager->LoadImage(_status_menu_image);
}


BattleCharacterActor::~BattleCharacterActor() {
	//FIX ME
	VideoManager->DeleteImage(_time_meter_portrait);
	//VideoManager->DeleteImage(_time_portrait_selected);
	VideoManager->DeleteImage(_status_bar_cover_image);
	VideoManager->DeleteImage(_status_menu_image);
}


/*void BattleCharacterActor::ResetWaitTime()
{
	_wait_time.Reset();// = 5000;
	_wait_time.Play();

	//Sets time meter portrait position
	_time_portrait_location = 128.f;
}*/

// Updates the state of the character. Must be called every frame!
void BattleCharacterActor::Update()
{	
	/*if (GetActor()->IsAlive() == false)
	{
		current_battle->RemoveScriptedEventsForActor(this);
	}*/
	if (!_wait_time.HasExpired() && IsAlive() && !IsQueuedToPerform() && _wait_time.IsPlaying())
		_time_portrait_location += SystemManager->GetUpdateTime() * (405.0f / _wait_time.GetDuration());

	GetActor()->RetrieveBattleAnimation("idle")->Update();

	if (TEMP_IsAttacking()) {
		if ((_x_location - _x_origin) < 50)
			_x_location += 0.8f * static_cast<float>(SystemManager->GetUpdateTime());
	}
	else
		SetXLocation(GetXOrigin()); // Restore original place

	//if (!_wait_time.HasExpired() && GetActor()->IsAlive() && !IsQueuedToPerform())
	//	_time_portrait_location += SystemManager->GetUpdateTime() * (405.0f / _wait_time.GetDuration());	
}

//Calculates the actor's base physical attack damage
void BattleCharacterActor::CalcPhysicalAttack()
{
	_physical_attack = _strength;

	if (GetActor()->GetWeaponEquipped())
		_physical_attack += GetActor()->GetWeaponEquipped()->GetPhysicalAttack();
}

//Calculates the actor's base metaphysical attack damage
void BattleCharacterActor::CalcMetaPhysicalAttack()
{
	_metaphysical_attack = _vigor;

	if (GetActor()->GetWeaponEquipped())
		_metaphysical_attack += GetActor()->GetWeaponEquipped()->GetMetaphysicalAttack();
}

//Calculates the actor's base physical defense
void BattleCharacterActor::CalcPhysicalDefense(hoa_global::GlobalAttackPoint* attack_point)
{
	_physical_defense = _fortitude;
	std::vector<GlobalArmor*> armor = GetActor()->GetArmorEquipped();

	for (uint32 i = 0; i < armor.size(); ++i)
	{
		_physical_defense += armor[i]->GetPhysicalDefense();
	}

	if (attack_point)
	{
		_physical_defense += attack_point->GetFortitudeBonus();
	}
}

//Calculates the actor's base metaphysical defense
void BattleCharacterActor::CalcMetaPhysicalDefense(hoa_global::GlobalAttackPoint* attack_point)
{
	_metaphysical_defense = _protection;

	std::vector<GlobalArmor*> armor = GetActor()->GetArmorEquipped();

	for (uint32 i = 0; i < armor.size(); ++i)
	{
		_metaphysical_defense += armor[i]->GetMetaphysicalDefense();
	}

	if (attack_point)
	{
		_metaphysical_defense += attack_point->GetProtectionBonus();
	}
}

//Calculates the actor's evade
void BattleCharacterActor::CalcEvade(hoa_global::GlobalAttackPoint* attack_point)
{
	_combat_evade = _evade;

	//std::vector<GlobalArmor*> armor = GetActor()->GetArmorEquipped();

	/*for (uint32 i = 0; i < armor.size(); ++i)
	{
		_metaphysical_defense += armor[i]->GetMetaphysicalDefense();
	}*/
	if (attack_point)
	{
		_combat_evade += attack_point->GetEvadeBonus();
	}
}

// Draws the character's current sprite animation frame
void BattleCharacterActor::DrawSprite() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);

	if (IsAlive()) {
		// Draw the actor selector image if this character is currently selected
		if (this == current_battle->_selected_character && current_battle->_cursor_state != CURSOR_IDLE) {
			VideoManager->Move(_x_location - 20.0f, _y_location - 20.0f);
			VideoManager->DrawImage(current_battle->_actor_selection_image);
		}
		// Draw the character sprite
		VideoManager->Move(_x_location, _y_location);
		GetActor()->RetrieveBattleAnimation("idle")->Draw();

		if (this == current_battle->_selected_target) {
			VideoManager->Move(_x_location - 20.0f, _y_location - 20.0f);
			VideoManager->DrawImage(current_battle->_actor_selection_image);
		}

		// TEMP: determine if character sprite needs red damage numbers drawn next to it
		if (_total_time_damaged > 0) {
			_total_time_damaged += SystemManager->GetUpdateTime();
			VideoManager->SetFont( "battle_dmg" );
			VideoManager->SetTextColor(Color::red);
			VideoManager->Move(GetXLocation() + 40.0f, GetYLocation() + ( _total_time_damaged / 35.0f ) + 100.0f);
			VideoManager->DrawText(NumberToString(_damage_dealt));
			VideoManager->SetFont( "battle" );

			if (_total_time_damaged > 3000) { // Show it for three seconds
				_total_time_damaged = 0;
			}
				//current_battle->SetPerformingScript (false);
			//}
		}
	}
	else {
		// TODO: draw the "incapacitated" character here
	}
}


// Draws the character's damage-blended face portrait
void BattleCharacterActor::DrawPortrait() {
	std::vector<StillImage> & portrait_frames = *(GetActor()->GetBattlePortraits());
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(48, 9);

	float hp_percent =  static_cast<float>(GetHitPoints()) / static_cast<float>(GetMaxHitPoints());

	if (GetActor()->GetHitPoints() == 0) {
	  VideoManager->DrawImage(portrait_frames[4]);
	}
	// The blend alpha will range from 1.0 to 0.0 in the following calculations
	else if (hp_percent < 0.25f) {
		VideoManager->DrawImage(portrait_frames[4]);
		float alpha = (hp_percent) * 4;
		VideoManager->DrawImage(portrait_frames[3], Color(1.0f, 1.0f, 1.0f, alpha));
	}
  else if (hp_percent < 0.50f) {
		VideoManager->DrawImage(portrait_frames[3]);
		float alpha = (hp_percent - 0.25f) * 4;
		VideoManager->DrawImage(portrait_frames[2], Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else if (hp_percent < 0.75f) {
		VideoManager->DrawImage(portrait_frames[2]);
		float alpha = (hp_percent - 0.50f) * 4;
		VideoManager->DrawImage(portrait_frames[1], Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else if (hp_percent < 1.00f) {
		VideoManager->DrawImage(portrait_frames[1]);
		float alpha = (hp_percent - 0.75f) * 4;
		VideoManager->DrawImage(portrait_frames[0], Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else { // Character is at full health
		VideoManager->DrawImage(portrait_frames[0]);
	}
}


// Draws the character's portrait on the time meter
/*void BattleCharacterActor::DrawTimePortrait(bool is_selected) {

	if (GetActor()->IsAlive()) {
		VideoManager->Move(995, _time_portrait_location);

		VideoManager->DrawImage(_time_meter_portrait);

		if (is_selected)
			VideoManager->DrawImage(_time_portrait_selected);
	}
}*/
// Draws the character's status information
void BattleCharacterActor::DrawStatus() {
	// Used to determine where to draw the character's status
	float y_offset = 0.0f;

	// Determine what vertical order the character is in and set the y_offset accordingly
	if (current_battle->_character_actors[0] == this) {
		y_offset = 0.0f;
	} else if (current_battle->_character_actors[1] == this) {
		y_offset = -25.0f;
	} else if (current_battle->_character_actors[2] == this) {
		y_offset = -50.0f;
	} else if (current_battle->_character_actors[3] == this) {
		y_offset = -75.0f;
	}

	// Shrinking bars (HP, SP) TODO: STAMINA
	float bar_size;

	// HP, green bar
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_NO_BLEND, 0);
	bar_size = static_cast<float>(83*GetHitPoints())/static_cast<float>(GetMaxHitPoints());
	VideoManager->Move(312, 90 + y_offset);

	if (GetHitPoints() > 0)		// Draw color bar (if needed)
	{
		VideoManager->DrawRectangle(bar_size,6,Color(0.133f,0.455f,0.133f,1.0f));
	}
	if (GetHitPoints() != GetMaxHitPoints())	// Draw black bar (if needed)
	{
		VideoManager->MoveRelative(bar_size, 0.0f);
		VideoManager->DrawRectangle(83.0f-bar_size,6,Color::black);
		VideoManager->Move(312, 90 + y_offset);
	}

	VideoManager->SetDrawFlags(VIDEO_BLEND_ADD, 0);
	VideoManager->DrawImage(_status_bar_cover_image);

	// SP, blue bar
	VideoManager->SetDrawFlags(VIDEO_NO_BLEND, 0);
	bar_size = static_cast<float>(84*GetSkillPoints())/static_cast<float>(GetMaxSkillPoints());
	VideoManager->Move(412, 90 + y_offset);

	if (GetSkillPoints() > 0)	// Draw color bar (if needed)
	{
		VideoManager->DrawRectangle(bar_size,6,Color(0.129f,0.263f,0.451f,1.0f));
	}
	if (GetHitPoints() != GetMaxHitPoints())	// Draw black bar (if needed)
	{
		VideoManager->MoveRelative(bar_size,0.0f);
		VideoManager->DrawRectangle(83.0f-bar_size,6,Color::black);
		VideoManager->Move(412, 90 + y_offset);
	}

	VideoManager->SetDrawFlags(VIDEO_BLEND_ADD, 0);
	VideoManager->DrawImage(_status_bar_cover_image);

	// Draw the background of the menu
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(149, 84.0f + y_offset);
	VideoManager->DrawImage(_status_menu_image);

	VideoManager->SetTextColor(Color::white);

	// Draw the character's name
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(225.0f, 90.0f + y_offset);
 	VideoManager->DrawText(GetActor()->GetName());

	// Draw the character's current health on top of the middle of the HP bar
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);

	VideoManager->Move(355.0f, 90.0f + y_offset);
	VideoManager->DrawText(NumberToString(GetHitPoints()));

	// Draw the character's current skill points on top of the middle of the SP bar
	VideoManager->MoveRelative(100, 0);
	VideoManager->DrawText(NumberToString(GetSkillPoints()));

	// Draw all of the character's current status afflictions
	// TODO: waiting for ActorEffects class to be implemented
	// VideoManager->MoveRel(152, 4);
	// for (uint8 i = 0; i < _effects.size(); i++) {
	// 	VideoManager->DrawImage(_effects[i].image);
	// 	VideoManager->MoveRel(25, 0);
	// }
}


// Gives a specific amount of damage for the character
/*void BattleCharacterActor::TakeDamage(uint32 damage)
{
	_total_time_damaged = 1;
	_damage_dealt = damage;

	if (damage >= GetActor()->GetHitPoints()) // Was it a killing blow?
	{
		GetActor()->SetHitPoints(0);
		current_battle->RemoveScriptedEventsForActor(this);
	}
	else {
		GetActor()->SetHitPoints(GetActor()->GetHitPoints() - damage);
	}
}*/

//For now, only update HP and SP
void BattleCharacterActor::UpdateGlobalActorStats()
{
	GetActor()->SetHitPoints(GetHitPoints());
	GetActor()->SetSkillPoints(GetSkillPoints());
}

////////////////////////////////////////////////////////////////////////////////
// EnemyActor class
////////////////////////////////////////////////////////////////////////////////
BattleEnemyActor::BattleEnemyActor(GlobalEnemy enemy, float x_location, float y_location) :
	BattleActor(),
	_global_enemy(enemy.GetID())
	/*_x_location(x_location),
	_y_location(y_location),
	_x_origin(_x_location),
	_y_origin(_y_location)*/
	/*_total_time_damaged(0),
	_damage_dealt(0),
	_is_queued_to_perform(false)*/
{
	//Cannot initalize protected members in init list for some reason
	_x_location = x_location;
	_y_location = y_location;
	_x_origin = _x_location;
	_y_origin = _y_location;

	//FIX ME
	_time_meter_portrait.SetFilename("img/icons/actors/enemies/" + _global_enemy.GetFilename() + ".png");
	//_time_meter_portrait.SetFilename("img/menus/stamina_icon.png");
	_time_meter_portrait.SetDimensions(45,45);
	VideoManager->LoadImage(_time_meter_portrait);

	//Load time portrait selector
	/*_time_portrait_selected.SetDimensions(45,45);
	_time_portrait_selected.SetFilename("img/menus/stamina_icon.png");
	VideoManager->LoadImage(_time_portrait_selected);*/

	//GetWaitTime()->SetDuration(2000);
	//ResetWaitTime();
}



BattleEnemyActor::~BattleEnemyActor() {
	// FIX ME
	VideoManager->DeleteImage(_time_meter_portrait);
	//VideoManager->DeleteImage(_time_portrait_selected);
}

//Calculates the actor's base physical attack damage
void BattleEnemyActor::CalcPhysicalAttack()
{
	_physical_attack = _strength;

	if (GetActor()->GetWeaponEquipped())
		_physical_attack += GetActor()->GetWeaponEquipped()->GetPhysicalAttack();
}

//Calculates the actor's base metaphysical attack damage
void BattleEnemyActor::CalcMetaPhysicalAttack()
{
	_metaphysical_attack = _vigor;

	if (GetActor()->GetWeaponEquipped())
		_metaphysical_attack += GetActor()->GetWeaponEquipped()->GetMetaphysicalAttack();
}

//Calculates the actor's base physical defense
void BattleEnemyActor::CalcPhysicalDefense(GlobalAttackPoint* attack_point)
{
	_physical_defense = _fortitude;

	if (attack_point)
	{
		_physical_defense += attack_point->GetFortitudeBonus();
	}
}

//Calculates the actor's base metaphysical defense
void BattleEnemyActor::CalcMetaPhysicalDefense(GlobalAttackPoint* attack_point)
{
	_metaphysical_defense = _protection;

	if (attack_point)
	{
		_metaphysical_defense += attack_point->GetProtectionBonus();
	}
}

//Calculates the actor's base evade
void BattleEnemyActor::CalcEvade(hoa_global::GlobalAttackPoint* attack_point)
{
	_combat_evade = _evade;

	//std::vector<GlobalArmor*> armor = GetActor()->GetArmorEquipped();

	/*for (uint32 i = 0; i < armor.size(); ++i)
	{
		_metaphysical_defense += armor[i]->GetMetaphysicalDefense();
	}*/
	if (attack_point)
	{
		_combat_evade += attack_point->GetEvadeBonus();
	}
}

/*void BattleEnemyActor::ResetWaitTime()
{
	_wait_time.Reset();
	_wait_time.Play();
	//Sets time meter portrait position

	_time_portrait_location = 128.f;
}*/

// Compares the Y-coordinates of the actors, used for sorting the actors up-down when drawing
// BROKEN!!! My bad -CD
bool BattleEnemyActor::operator<(const BattleEnemyActor & other) const
{
	//if ((_y_location - ((*GetActor()).GetHeight())) > (other.GetYLocation() - (*(other.GetActor()).GetHeight())))
	//	return true;
	
	return false;
}

// Updates the action status of the enemy
void BattleEnemyActor::Update() {

	//if (_wait_time)
	//	_wait_time -= SystemManager->GetUpdateTime();
	if (!_wait_time.HasExpired() && IsAlive() && !IsQueuedToPerform() && _wait_time.IsPlaying())
		_time_portrait_location += SystemManager->GetUpdateTime() * (405.0f / _wait_time.GetDuration());

	if (IsAlive() && !IsQueuedToPerform() && GetWaitTime()->HasExpired())
	{
		//if (_wait_time.HasExpired())
		//{
			//_wait_time = 0;
			//FIX ME Needs real AI decisions
			//we can perform another attack
			//MF: Bad bad bad.  We do not build a queue of targets.  Not anymore.
			//All we do is have a handle to either 1 enemy or char to see which
			//side we're attacking.  If it's a party skill, then the script just
			//loops over them.  Besides, we don't want to send dequeues to the
			//Lua stack.

			/*std::deque<BattleActor*> final_targets;
			std::deque<BattleCharacterActor*> targets = current_battle->GetCharacters();

			for (uint8 i = 0; i < targets.size(); i++) {
				final_targets.push_back(dynamic_cast<BattleActor*>(targets[i]));
			}*/

			// okay, we can perform another attack.  set us up as queued to perform.
			SetQueuedToPerform(true);
			GlobalSkill* skill = (GetActor()->GetSkills().begin()->second);
			//FIX ME Until we have AI, pick Claudius
			ScriptEvent *se = new ScriptEvent(this, current_battle->GetPlayerCharacterAt(0), skill);
			current_battle->AddScriptEventToQueue(se);

			//current_battle->AddScriptEventToQueue(new ScriptEvent(this, final_targets, "sword_swipe", 3000));
			SetXLocation(GetXOrigin()); // Always attack from the starting location
		//}
		//FIX ME have to use char stats
		/*else
			_time_portrait_location += SystemManager->GetUpdateTime() * (405.f / _wait_time.GetDuration());//.081f;*/
	}

	// If we're attacking, update the offset a little
	// FIX ME Let the script event handle this
	if (TEMP_IsAttacking()) {
		if ((_x_origin - _x_location) < 50)
			_x_location -= 0.8f * static_cast<float>(SystemManager->GetUpdateTime());
	}
	else
		SetXLocation(GetXOrigin()); // Restore original place

}


// Draws the damage-blended enemy sprite
void BattleEnemyActor::DrawSprite() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);

	// Draw the sprite's final damage frame in grayscale and return
	if (!IsAlive()) {
		VideoManager->Move(_x_location, _y_location);
		std::vector<StillImage> & sprite_frames = *(GetActor()->GetSpriteFrames());
		sprite_frames[3].EnableGrayScale();
		VideoManager->DrawImage(sprite_frames[3]);
		sprite_frames[3].DisableGrayScale();
	}
	else {
		// Draw the actor selector image over the currently selected enemy
		if (this == current_battle->_selected_target) {
			VideoManager->Move(_x_location - 20.0f, _y_location - 20.0f);
			VideoManager->DrawImage(current_battle->_actor_selection_image);
		}

		// Draw the enemy's damage-blended sprite frames
		std::vector<StillImage>& sprite_frames = *(GetActor()->GetSpriteFrames());
		VideoManager->Move(_x_location, _y_location);
		float hp_percent = static_cast<float>(GetHitPoints()) / static_cast<float>(GetMaxHitPoints());

		// Alpha will range from 1.0 to 0.0 in the following calculations
		if (hp_percent < 0.33f) {
			VideoManager->DrawImage(sprite_frames[3]);
			float alpha = (hp_percent) * 3;
			VideoManager->DrawImage(sprite_frames[2], Color(1.0f, 1.0f, 1.0f, alpha));
		}
		else if (hp_percent < 0.66f) {
			VideoManager->DrawImage(sprite_frames[2]);
			float alpha = (hp_percent - 0.33f) * 3;
			VideoManager->DrawImage(sprite_frames[1], Color(1.0f, 1.0f, 1.0f, alpha));
		}
		else if (hp_percent < 1.00f) {
			VideoManager->DrawImage(sprite_frames[1]);
			float alpha = (hp_percent - 0.66f) * 3;
			VideoManager->DrawImage(sprite_frames[0], Color (1.0f, 1.0f, 1.0f, alpha));
		}
		else { // Enemy is at full health
			VideoManager->DrawImage(sprite_frames[0]);
		}

		// Draw the attack point indicator if necessary
		if (this == current_battle->_selected_target && current_battle->_cursor_state == CURSOR_SELECT_ATTACK_POINT) {
			VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
			std::vector<GlobalAttackPoint*> attack_points = GetActor()->GetAttackPoints();

			VideoManager->Move(GetXLocation() + attack_points[current_battle->_attack_point_selected]->GetXPosition(),
				GetYLocation() + attack_points[current_battle->_attack_point_selected]->GetYPosition());
			VideoManager->DrawImage(current_battle->_attack_point_indicator);

			// Reset default X and Y draw orientation
			VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
		}
	}

	// Determine if enemy needs to have red damage text drawn next to it
	if (_total_time_damaged > 0) {
		_total_time_damaged += SystemManager->GetUpdateTime();

		VideoManager->SetFont( "battle_dmg" );
		VideoManager->SetTextColor(Color::red);
		VideoManager->Move(GetXLocation() + 25.0f, GetYLocation() + ( _total_time_damaged / 35.0f ) + 80.0f);
		VideoManager->DrawText(NumberToString(_damage_dealt));
		VideoManager->SetFont( "battle" );

		if (_total_time_damaged > 3000) {
			_total_time_damaged = 0;
			//current_battle->SetPerformingScript(false);
		}
	}
}

// Draws the enemy's time meter portrait
/*void BattleEnemyActor::DrawTimePortrait(bool is_selected)
{
	if (GetActor()->IsAlive()) {
		VideoManager->Move(995, _time_portrait_location);

		VideoManager->DrawImage(_time_meter_portrait);

		if (is_selected)
			VideoManager->DrawImage(_time_portrait_selected);
	}
}*/

// Draws the enemy's status information
void BattleEnemyActor::DrawStatus() {
	// Draw the enemy's name
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->SetTextColor(Color::white);
	VideoManager->Move(920, 84);
	VideoManager->DrawText(GetActor()->GetName());

	// Draw the name of the enemy's currently selected attack point
	if (current_battle->_cursor_state == CURSOR_SELECT_ATTACK_POINT) {
		std::vector<GlobalAttackPoint*> attack_points = GetActor()->GetAttackPoints();
		VideoManager->MoveRelative(0, -25);
		ustring attack_point = MakeUnicodeString("(") + attack_points[current_battle->_attack_point_selected]->GetName() + MakeUnicodeString(")");
		VideoManager->DrawText(attack_point);
	}

	// TODO Draw the icons for any status afflictions that the enemy has
	//VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	//VideoManager->Move(818, 12);
// 	for (uint8 i = 0; i < _effects; i++) {
// 		VideoManager->DrawImage(_effects[i].image);
// 		VideoManager->MoveRel(25, 0);
// 	}
}


// Gives a specific amount of damage for the enemy
/*void BattleEnemyActor::TakeDamage(uint32 damage)
{
	_total_time_damaged = 1;
	_damage_dealt = damage;
	if (damage >= GetHitPoints()) // Was it a killing blow?
	{
		SetHitPoints(0);
		current_battle->RemoveScriptedEventsForActor(this);
	}
	else {
		SetHitPoints(GetHitPoints() - damage);
	}
}*/


} // namespace private_battle

} // namespace hoa_battle
