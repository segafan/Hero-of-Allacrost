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
*** \author  Viljami Korhonen, mindflayer@allacrost.org
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

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_script;

using namespace hoa_battle::private_battle;

namespace hoa_battle {

namespace private_battle {

////////////////////////////////////////////////////////////////////////////////
// BattleActorEffect class
////////////////////////////////////////////////////////////////////////////////

// TODO: This class is currently not implemented nor used

////////////////////////////////////////////////////////////////////////////////
// BattleActor class
////////////////////////////////////////////////////////////////////////////////

BattleActor::BattleActor(uint32 x, uint32 y):
	_x_origin(x),
	_y_origin(y),
	_x_location(static_cast<float>(x)),
	_y_location(static_cast<float>(y)),
	_max_skill_points(0),
	_current_skill_points(0),
	_is_move_capable(true),
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



BattleActor::~BattleActor() {
	// TODO: Need to implement
}




void BattleActor::UpdateEffects() {
	// TODO: ActorEffects not yet ready to be implemented
// 	for (uint32 i = 0; i < _effects.size(); i++) {
// 		_effects[i].Update(SystemManager->GetUpdateTime());
// 	}
}



void BattleActor::TEMP_Deal_Damage(uint32 damage) {
	_TEMP_damage_dealt = damage;
	_TEMP_total_time_damaged = 1;

	if (_TEMP_damage_dealt >= GetHealthPoints()) {
		SetHealthPoints(0);
		current_battle->RemoveScriptedEventsForActor(this);
	}
	else {
		SetHealthPoints(GetHealthPoints() - _TEMP_damage_dealt);
	}
}

// *****************************************************************************
// CharacterActor class
// *****************************************************************************

CharacterActor::CharacterActor(GlobalCharacter* const character, uint32 x, uint32 y) :
	BattleActor(x, y),
	_wrapped_character(character)
{
	 _current_animation = _wrapped_character->GetAnimation("IDLE");
}



CharacterActor::~CharacterActor() {
	// TODO
}



void CharacterActor::Update() {
	if (IsAlive()) {
		if (GetHealthPoints() <= 0) {
			current_battle->RemoveScriptedEventsForActor(this);
		}
		_current_animation.Update();
	}
}



void CharacterActor::DrawSprite() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	
	if (IsAlive()) {
		// Draw the actor selector graphic if this character is currently selected
		if (this == current_battle->_selected_character && current_battle->_cursor_state != CURSOR_IDLE) {
			VideoManager->Move(GetXLocation() - 20, GetYLocation() - 20);
			VideoManager->DrawImage(current_battle->_actor_selection_image);
		}

		// Draw the frame for the sprite's current animation
		VideoManager->Move(GetXLocation(), GetYLocation());
		_current_animation.Draw();

		// TEMP: determine if character sprite needs red damage numbers drawn next to it
		if (_TEMP_total_time_damaged > 0) {
			_TEMP_total_time_damaged += SystemManager->GetUpdateTime();
			VideoManager->SetTextColor(Color::red);
			VideoManager->Move(GetXLocation() + 100, GetYLocation() + 70);
			VideoManager->DrawText(NumberToString(_TEMP_damage_dealt));

			if (_TEMP_total_time_damaged > 3000) {
				_TEMP_total_time_damaged = 0;
				current_battle->SetPerformingScript (false);
			}
		}
	}
	else {
		// TODO: draw the "incapacitated" character here
	}
} // void CharacterActor::DrawSprite()



void CharacterActor::DrawPortrait() {
	vector<StillImage> portrait_frames = _wrapped_character->GetBattlePortraits();
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(48, 9);

	float hp_percent = GetHealthPoints() / static_cast<float>(GetMaxHealthPoints());


	if (GetHealthPoints() == 0) {
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
} // void CharacterActor::DrawPortrait()



void CharacterActor::DrawStatus() {
	// Used to determine where to draw the character's status
	int32 y_offset = 0;
	
	// Determine what vertical order the character is in and set the y_offset accordingly
	if (current_battle->_character_actors[0] == this) {
		y_offset = 0;
	} else if (current_battle->_character_actors[1] == this) {
		y_offset = -25;
	} else if (current_battle->_character_actors[2] == this) {
		y_offset = -50;
	} else if (current_battle->_character_actors[3] == this) {
		y_offset = -75;
	}

	VideoManager->SetTextColor(Color::white);
	
	// Draw the character's name
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(225.0f, 90.0f + y_offset);
 	VideoManager->DrawText(GetName());

	// Draw the character's HP, SP, and ST stamina bars
	// TODO

	// Draw the character's current health on top of the middle of the HP bar
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);

	VideoManager->Move(355.0f, 90.0f + y_offset);
	VideoManager->DrawText(NumberToString(GetHealthPoints()));

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
} // void CharacterActor::DrawStatus()

////////////////////////////////////////////////////////////////////////////////
// EnemyActor class
////////////////////////////////////////////////////////////////////////////////

EnemyActor::EnemyActor (GlobalEnemy enemy, uint32 x, uint32 y) :
	BattleActor(x, y),
	_wrapped_enemy(enemy)
{
	// TODO
}



EnemyActor::~EnemyActor() {
	// TODO
}


// NOTE: This function is almost entirely composed of temporarily code right now
void EnemyActor::Update() {
	static uint32 next_attack = 0;
	static uint32 last_attack = 0;

	// make sure the enemy isn't queued to perform.  set the next attack time
	if (next_attack == 0 && !IsQueuedToPerform()) {
		next_attack = RandomBoundedInteger(5000, 30000);
		last_attack = 0;
		SetXLocation(static_cast<float>(GetXOrigin())); // Always attack from the starting location
	}

	last_attack += SystemManager->GetUpdateTime();

	if ( last_attack > next_attack && !IsQueuedToPerform() && IsAlive()) {
		//we can perform another attack
		deque<BattleActor*> final_targets;
		deque<CharacterActor*> targets = current_battle->ReturnCharacters();

		for (uint8 i = 0; i < targets.size(); i++) {
			final_targets.push_back(dynamic_cast<BattleActor*>(targets[i]));
		}

		// okay, we can perform another attack.  set us up as queued to perform.
		SetQueuedToPerform(true);
		current_battle->AddScriptEventToQueue(ScriptEvent(dynamic_cast<BattleActor*>(this), final_targets, "sword_swipe"));
		SetXLocation(static_cast<float>(GetXOrigin())); // Always attack from the starting location

		last_attack = 0;
		next_attack = 0;
	}

	// If we're attacking, update the offset a little
	if (IsAttacking())
		SetXLocation(GetXLocation() - 0.8f * static_cast<float>(SystemManager->GetUpdateTime()));
	else
		SetXLocation(static_cast<float>(GetXOrigin())); // Restore original place

} // void EnemyActor::Update()



void EnemyActor::DrawSprite() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	
	// Draw the sprite's final damage frame (it will be in gray scale) and return
	if (!IsAlive()) {
		VideoManager->Move(GetXLocation(), GetYLocation());
		vector<StillImage> sprite_frames = _wrapped_enemy.GetAnimation("IDLE");
		sprite_frames[3].EnableGrayScale();
		VideoManager->DrawImage(sprite_frames[3]);
		sprite_frames[3].DisableGrayScale();
	}
	else {
		// Draw the actor selector image over the currently selected enemy
		if (this == current_battle->_selected_enemy) {
			VideoManager->Move(GetXLocation() - 20, GetYLocation() - 20);
			VideoManager->DrawImage(current_battle->_actor_selection_image);
		}

		// Draw the enemy's damage-blended sprite frames
		vector<StillImage> sprite_frames = _wrapped_enemy.GetAnimation("IDLE");
		VideoManager->Move(GetXLocation(), GetYLocation());
		float hp_percent = GetHealthPoints() / static_cast<float>(GetMaxHealthPoints());

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
		if (this == current_battle->_selected_enemy && current_battle->_cursor_state == CURSOR_SELECT_ATTACK_POINT) {
			VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
			vector<GlobalAttackPoint*> attack_points = GetAttackPoints();
			VideoManager->Move(GetXLocation() + attack_points[current_battle->_attack_point_selected]->GetXPosition(),
				GetYLocation() + attack_points[current_battle->_attack_point_selected]->GetYPosition());
			VideoManager->DrawImage(current_battle->_attack_point_indicator);

			// Reset default X and Y draw orientation
			VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
		}
	}

	// TEMP: Determine if enemy needs to have red damage text drawn next to it
	if (_TEMP_total_time_damaged > 0) {
		_TEMP_total_time_damaged += SystemManager->GetUpdateTime();

		VideoManager->SetTextColor(Color::red);
		VideoManager->Move(GetXLocation() + 100, GetYLocation() + 70);
		VideoManager->DrawText(NumberToString(_TEMP_damage_dealt));

		if (_TEMP_total_time_damaged > 3000) {
			_TEMP_total_time_damaged = 0;
			current_battle->SetPerformingScript(false);
		}
	}
} // void EnemyActor::DrawSprite()



void EnemyActor::DrawStatus() {
	// Draw the enemy's name
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->SetTextColor(Color::white);
	VideoManager->Move(920, 84);
	VideoManager->DrawText(GetName());

	// Draw the name of the enemy's selected attack point
	if (current_battle->_cursor_state == CURSOR_SELECT_ATTACK_POINT) {
		vector<GlobalAttackPoint*> attack_points = GetAttackPoints();
		VideoManager->MoveRelative(0, -25);
		ustring attack_point = MakeUnicodeString("(" + attack_points[current_battle->_attack_point_selected]->GetName() + ")");
		VideoManager->DrawText(attack_point);
	}

	// TODO Draw the icons for any status afflictions that the enemy has
	//VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	//VideoManager->Move(818, 12);
// 	for (uint8 i = 0; i < _effects; i++) {
// 		VideoManager->DrawImage(_effects[i].image);
// 		VideoManager->MoveRel(25, 0);
// 	}
} // void EnemyActor::DrawStatus()

//! Is the monster attacking right now
const bool EnemyActor::IsAttacking() const
{
	if (current_battle && !current_battle->_script_queue.empty())
	{
		BattleActor * first_attacker = (current_battle->_script_queue.front()).GetSource();
		if (IsQueuedToPerform() && (this == first_attacker))
			return true;
	}

	return false;
}


} // namespace private_battle

} // namespace hoa_battle
