////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_actors.cpp
*** \author  Corey Hoffstein, visage@allacrost.org
*** \brief   Source file for actors present in battles.
*** ***************************************************************************/

#include <iostream>
#include <sstream>

#include "utils.h"
#include "audio.h"
#include "video.h"
#include "settings.h"
#include "global.h"
#include "data.h"

#include "battle.h"
#include "battle_actors.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_settings;
using namespace hoa_global;
using namespace hoa_data;

using namespace hoa_battle::private_battle;

namespace hoa_battle {

namespace private_battle {

// *****************************************************************************
// ActorEffect class
// *****************************************************************************

ActorEffect::ActorEffect(BattleActor* const AHost, std::string AEffectName, StatusSeverity AHowSevere, uint32 ATTL,
	bool ACanMove, uint32 AHealthModifier, uint32 ASkillPointModifier, uint32 AStrengthModifier,
	uint32 AIntelligenceModifier, uint32 AAgilityModifier, uint32 AUpdateLength) :
	_host(AHost),
	_effect_name(AEffectName),
	_TTL(ATTL),
	_severeness(AHowSevere),
	_can_move(ACanMove),
	_health_modifier(AHealthModifier),
	_skill_point_modifier(ASkillPointModifier),
	_strength_modifier(ASkillPointModifier),
	_intelligence_modifier(AIntelligenceModifier),
	_agility_modifier(AAgilityModifier),
	_update_length(AUpdateLength),
	_age(0),
	_times_updated(0) {
	_last_update = SettingsManager->GetUpdateTime();
}

// *****************************************************************************
// BattleActor class
// *****************************************************************************

BattleActor::BattleActor(BattleMode* ABattleMode, uint32 AXLocation, uint32 AYLocation):
	_owner_battle_mode(ABattleMode),
	_x_origin(AXLocation),
	_y_origin(AYLocation),
	_x_location(AXLocation),
	_y_location(AYLocation),
	_max_skill_points(0),
	_current_skill_points(0),
	_is_move_capable(true),
	_is_alive(true),
	_is_queued_to_perform(false),
	_warmup_time(0),
	_cooldown_time(0),
	_defensive_mode_bonus(0),
	_total_strength_modifier(0),
	_total_agility_modifier(0),
	_total_intelligence_modifier(0)
{
	_TEMP_total_time_damaged = 0;
}

BattleActor::~BattleActor(){
	// TODO: Need to implement
}

void BattleActor::Die() {
	_is_alive = false;
	// Remove any scripts that the actor is a part of
	GetOwnerBattleMode()->RemoveScriptedEventsForActor(this);
}

void BattleActor::UpdateEffects(uint32 ATimeElapsed) {
	for (uint32 i = 0; i < _effects.size(); i++) {
		_effects[i].Update(ATimeElapsed);
	}
}

void BattleActor::PushEffect(const ActorEffect AEffect) {
	_effects.push_back (AEffect);
}

void BattleActor::TEMP_Deal_Damage (uint32 damage) {
	_TEMP_damage_dealt = damage;
	_TEMP_total_time_damaged = 1;

	if (_TEMP_damage_dealt >= GetHealth()) {
	  SetHealth(0);
	  Die();
	}
	else {
		SetHealth (GetHealth() - _TEMP_damage_dealt);
	}
}

// *****************************************************************************
// PlayerActor class
// *****************************************************************************

PlayerActor::PlayerActor(GlobalCharacter* const AWrapped, BattleMode* const ABattleMode, uint32 AXLoc, uint32 AYLoc) :
	BattleActor(ABattleMode, AXLoc, AYLoc), _wrapped_character(AWrapped) {
	 _current_animation = _wrapped_character->GetAnimation("IDLE");
}

PlayerActor::~PlayerActor() {
	// TODO
}

void PlayerActor::Update (uint32 ATimeElapsed) {
	if (IsAlive()) {
		if (GetHealth() <= 0) {
			Die();
		}
		_current_animation.Update();
	}
}

void PlayerActor::Draw() {
	// Draw the blended character portrait
	std::vector<hoa_video::StillImage> head_shots = _wrapped_character->GetBattleHeadShots();
	VideoManager->Move(50, 10);
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	float percent = GetHealth() / static_cast<float>(GetMaxHealth());

	// Alpha will range from 1.0 to 0.0 in the following calculations
	if (GetHealth() == 0) {
	  VideoManager->DrawImage(head_shots[4]);
	}
	else if (percent < 0.25f) {
		VideoManager->DrawImage(head_shots[4]);
		float alpha = (percent) * 4;
		VideoManager->DrawImage(head_shots[3], Color(1.0f, 1.0f, 1.0f, alpha));
	}
  else if (percent < 0.50f) {
		VideoManager->DrawImage(head_shots[3]);
		float alpha = (percent - 0.25f) * 4;
		VideoManager->DrawImage(head_shots[2], Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else if (percent < 0.75f) {
		VideoManager->DrawImage(head_shots[2]);
		float alpha = (percent - 0.50f) * 4;
		VideoManager->DrawImage(head_shots[1], Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else if (percent < 1.00f) {
		VideoManager->DrawImage(head_shots[1]);
		float alpha = (percent - 0.75f) * 4;
		VideoManager->DrawImage(head_shots[0], Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else {
		VideoManager->DrawImage(head_shots[0]);
	}

	if (IsAlive()) {
		if (_TEMP_total_time_damaged > 0) {
			_TEMP_total_time_damaged += SettingsManager->GetUpdateTime();
			Color c ;
			string damage_amount("" + _TEMP_damage_dealt);
			VideoManager->SetTextColor(Color(1.0f, 0.0f, 0.0f, 1.0f));
			VideoManager->Move(GetXLocation() + 100, GetYLocation() + 70);
			VideoManager->DrawText(damage_amount);

			if (_TEMP_total_time_damaged > 3000) {
				_TEMP_total_time_damaged = 0;
				GetOwnerBattleMode()->SetPerformingScript (false);
			}
		}

		// Draw the sprite frame for the current animation
		VideoManager->Move(GetXLocation(), GetYLocation());
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		_current_animation.Draw();

// 		Color c ;
// 		ostringstream health_amount;
// 		health_amount << GetHealth();
// 		TEMP_Draw_Text (c, 320, 90, health_amount.str());
	} // if (IsAlive())

	else {
		// TODO: draw the "expired" character here
	}

	// Draw the character's health
	string battle_text("" + GetHealth());
	VideoManager->SetTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
	VideoManager->Move(310, 90);
	VideoManager->DrawText(battle_text);
} // void PlayerActor::Draw()

// *****************************************************************************
// EnemyActor class
// *****************************************************************************

EnemyActor::EnemyActor (GlobalEnemy AGlobalEnemy, BattleMode * const ABattleMode, uint32 AXLoc, uint32 AYLoc) :
	BattleActor(ABattleMode, AXLoc, AYLoc),
	_wrapped_enemy (AGlobalEnemy)
{}

EnemyActor::~EnemyActor() {
	// TODO
}

void EnemyActor::Update(uint32 ATimeElapsed) {
	/*
	static float totalHealthLost = 0;
	if (!IsAlive()) {
		return;
	}

	totalHealthLost += ATimeElapsed / 300.0f;

	float health = GetHealth() - totalHealthLost;
	if (health - (uint32)health > .5)
		health = (uint32)health + 1;
	else
		health = (uint32)health;

	if (GetHealth() > 0)
		SetHealth((uint32)health);
	else
		Die();

	if(totalHealthLost > .5)
		totalHealthLost = 0;
	*/
} // void EnemyActor::Update

void EnemyActor::Draw() {
	// Determine if enemy needs to have red damage text drawn next to it
	if (_TEMP_total_time_damaged > 0) {
		_TEMP_total_time_damaged += SettingsManager->GetUpdateTime();

		string damage_amount("" + _TEMP_damage_dealt);
		VideoManager->SetTextColor(Color(1.0f, 0.0f, 0.0f, 1.0f));
		VideoManager->Move(GetXLocation() + 100, GetYLocation() + 70);
		VideoManager->DrawText(damage_amount);

		if (_TEMP_total_time_damaged > 3000) {
			_TEMP_total_time_damaged = 0;
			GetOwnerBattleMode()->SetPerformingScript(false);
		}
	}

	if (!IsAlive()) {
		return;
	}

	// TEMP: Draws enemy's HP text
	// Color c(0.0f, 1.0f, 0.0f, 1.0f);
	// ostringstream health_amount;
	// health_amount << "HP: " << GetHealth() << " / " << GetMaxHealth();
	// TEMP_Draw_Text(c, GetXLocation()-20, GetYLocation()-20, health_amount.str());

	// Draw the blended enemy sprite frames
	float hp_percent = GetHealth() / static_cast<float>(GetMaxHealth());
	std::vector<hoa_video::StillImage> animations = _wrapped_enemy.GetAnimation("IDLE");
	VideoManager->Move(GetXLocation(), GetYLocation());
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);

	// Temporary code for enemies that don't have all damage frames
// 	if (animations.size() == 1) {
// 		VideoManager->DrawImage(animations[0]);
// 		return;
// 	}

	// Alpha will range from 1.0 to 0.0 in the following calculations
	if (hp_percent < 0.33f) {
		VideoManager->DrawImage(animations[3]);
		float alpha = (hp_percent) * 3;
		VideoManager->DrawImage(animations[2], Color(1.0f, 1.0f, 1.0f, alpha));
// 		VideoManager->DrawImage(animations[2]);
// 		float alpha = 1.0f - (hp_percent * 3);
// 		VideoManager->DrawImage(animations[3], Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else if (hp_percent < 0.66f) {
		VideoManager->DrawImage(animations[2]);
		float alpha = (hp_percent - 0.33f) * 3;
		VideoManager->DrawImage (animations[1], Color(1.0f, 1.0f, 1.0f, alpha));
// 		VideoManager->DrawImage(animations[1]);
// 		float alpha = 1.0f - ((hp_percent - 0.33f) * 3);
// 		VideoManager->DrawImage (animations[2], Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else if (hp_percent < 1.00f) {
		VideoManager->DrawImage(animations[1]);
		float alpha = (hp_percent - 0.66f) * 3;
		VideoManager->DrawImage (animations[0], Color (1.0f, 1.0f, 1.0f, alpha));
// 		VideoManager->DrawImage(animations[0]);
// 		float alpha = 1.0f - ((hp_percent - 0.66f) * 3);
// 		VideoManager->DrawImage (animations[1], Color (1.0f, 1.0f, 1.0f, alpha));
	}
	else {
		VideoManager->DrawImage(animations[0]);
	}
} // void EnemyActor::Draw()


void EnemyActor::LevelUp (uint32 AAverageLevel) {
	_wrapped_enemy.LevelSimulator(AAverageLevel);
}


void EnemyActor::DoAI() {
	static uint32 next_attack = 0;
	static uint32 last_attack = 0;

	//make sure the enemy isn't queued to perform.  set the next attack time
	if (next_attack == 0 && !IsQueuedToPerform()) {
		next_attack = rand() % 30000;
		last_attack = 0;
	}

	last_attack += SettingsManager->GetUpdateTime();

	if (last_attack > next_attack && !IsQueuedToPerform()) {
		//we can perform another attack
		std::deque<BattleActor*>final_targets;
		std::deque<PlayerActor*>targets = GetOwnerBattleMode()->ReturnCharacters();

		for (uint8 i = 0; i < targets.size(); i++) {
			final_targets.push_back(dynamic_cast<BattleActor*>(targets[i]));
		}

		//okay, we can perform another attack.  set us up as queued to perform.
		SetQueuedToPerform(true);
		GetOwnerBattleMode()->AddScriptEventToQueue(ScriptEvent(dynamic_cast<BattleActor*>(this), final_targets, "sword_swipe"));

		last_attack = 0;
		next_attack = 0;
	}
} // void EnemyActor::DoAI()

} // namespace private_battle

} // namespace hoa_battle
