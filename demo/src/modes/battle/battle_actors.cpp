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
*** \author  Viljami Korhonen, mindflayer@allacrost.org
*** \author  Corey Hoffstein, visage@allacrost.org
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
// BattleCharacterActor class
// *****************************************************************************
BattleCharacterActor::BattleCharacterActor(GlobalCharacter * character, float XLocation, float YLocation) :
GlobalCharacter(character->GetName(), character->GetFilename(), character->GetID()),
global_character_(character),
_x_location(XLocation),
_y_location(YLocation),
_x_origin(_x_location),
_y_origin(_y_location),
_total_time_damaged(0),
_damage_dealt(0),
_is_queued_to_perform(false)
{
}


BattleCharacterActor::~BattleCharacterActor() {
	// TODO: !
}


// Updates the state of the character. Must be called every frame!
void BattleCharacterActor::Update() {
	if (!IsAlive()) {
		current_battle->RemoveScriptedEventsForActor(this);
		}
	global_character_->RetrieveBattleAnimation("idle")->Update();
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
		global_character_->RetrieveBattleAnimation("idle")->Draw();
		

		// TEMP: determine if character sprite needs red damage numbers drawn next to it
		if (_total_time_damaged > 0) {
			_total_time_damaged += SystemManager->GetUpdateTime();
			VideoManager->SetTextColor(Color::red);
			VideoManager->Move(GetXLocation() + 100.0f, GetYLocation() + 70.0f);
			VideoManager->DrawText(NumberToString(_damage_dealt));

			if (_total_time_damaged > 3000) { // Show it for three seconds
				_total_time_damaged = 0;
				current_battle->SetPerformingScript (false);
			}
		}
	}
	else {
		// TODO: draw the "incapacitated" character here
	}
}


// Draws the character's damage-blended face portrait
void BattleCharacterActor::DrawPortrait() {
	std::vector<StillImage> portrait_frames = global_character_->GetBattlePortraits();
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(48, 9);

	float hp_percent =  static_cast<float>(GetHitPoints()) / static_cast<float>(GetMaxHitPoints());

	if (GetHitPoints() == 0) {
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

	VideoManager->SetTextColor(Color::white);
	
	// Draw the character's name
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(225.0f, 90.0f + y_offset);
 	VideoManager->DrawText(GetName());

	// TODO: HP, SP, and ST stamina bars

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



////////////////////////////////////////////////////////////////////////////////
// EnemyActor class
////////////////////////////////////////////////////////////////////////////////
BattleEnemyActor::BattleEnemyActor(const std::string & filename, float XLocation, float YLocation) :
GlobalEnemy(filename),
_x_location(XLocation),
_y_location(YLocation),
_x_origin(_x_location),
_y_origin(_y_location),
_total_time_damaged(0),
_damage_dealt(0),
_is_queued_to_perform(false)
{
	// TODO
}



BattleEnemyActor::~BattleEnemyActor() {
	// TODO
}


// Updates the action status of the enemy
void BattleEnemyActor::Update() {
	static uint32 next_attack = 0;
	static uint32 last_attack = 0;

	// make sure the enemy isn't queued to perform.  set the next attack time
	if (next_attack == 0 && !IsQueuedToPerform()) {
		next_attack = RandomBoundedInteger(5000, 30000);
		last_attack = 0;
		SetXLocation(GetXOrigin()); // Always attack from the starting location
	}

	last_attack += SystemManager->GetUpdateTime();

	if ( last_attack > next_attack && !IsQueuedToPerform() && IsAlive()) {
		//we can perform another attack
		std::deque<GlobalActor*> final_targets;
		std::deque<BattleCharacterActor*> targets = current_battle->ReturnCharacters();

		for (uint8 i = 0; i < targets.size(); i++) {
			final_targets.push_back(dynamic_cast<GlobalActor*>(targets[i]));
		}

		// okay, we can perform another attack.  set us up as queued to perform.
		SetQueuedToPerform(true);
		current_battle->AddScriptEventToQueue(ScriptEvent(dynamic_cast<GlobalActor*>(this), final_targets, "sword_swipe"));
		SetXLocation(GetXOrigin()); // Always attack from the starting location

		last_attack = 0;
		next_attack = 0;
	}

	// If we're attacking, update the offset a little
	if (IsAttacking())
		_x_location -= 0.8f * static_cast<float>(SystemManager->GetUpdateTime());
	else
		SetXLocation(GetXOrigin()); // Restore original place

}


// Draws the damage-blended enemy sprite
void BattleEnemyActor::DrawSprite() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	
	// Draw the sprite's final damage frame in grayscale and return
	if (!IsAlive()) {
		VideoManager->Move(_x_location, _y_location);
		std::vector<StillImage> & sprite_frames = *GetSpriteFrames();
		sprite_frames[3].EnableGrayScale();
		VideoManager->DrawImage(sprite_frames[3]);
		sprite_frames[3].DisableGrayScale();
		return;
	}
	else {
		// Draw the actor selector image over the currently selected enemy
		if (this == current_battle->_selected_enemy) {
			VideoManager->Move(_x_location - 20.0f, _y_location - 20.0f);
			VideoManager->DrawImage(current_battle->_actor_selection_image);
		}

		// Draw the enemy's damage-blended sprite frames
		std::vector<StillImage> & sprite_frames = *GetSpriteFrames();
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
		if (this == current_battle->_selected_enemy && current_battle->_cursor_state == CURSOR_SELECT_ATTACK_POINT) {
			VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
			std::vector<GlobalAttackPoint*> attack_points = GetAttackPoints();
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

		VideoManager->SetTextColor(Color::red);
		VideoManager->Move(GetXLocation() + 100.0f, GetYLocation() + 70.0f);
		VideoManager->DrawText(NumberToString(_damage_dealt));

		if (_total_time_damaged > 3000) {
			_total_time_damaged = 0;
			current_battle->SetPerformingScript(false);
		}
	}
}


// Draws the enemy's status information
void BattleEnemyActor::DrawStatus() {
	// Draw the enemy's name
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->SetTextColor(Color::white);
	VideoManager->Move(920, 84);
	VideoManager->DrawText(GetName());

	// Draw the name of the enemy's currently selected attack point
	if (current_battle->_cursor_state == CURSOR_SELECT_ATTACK_POINT) {
		std::vector<GlobalAttackPoint*> attack_points = GetAttackPoints();
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


// Is the monster attacking right now
bool BattleEnemyActor::IsAttacking() const
{
	if (current_battle && !current_battle->_script_queue.empty())
	{
		GlobalActor * first_attacker = (current_battle->_script_queue.front()).GetSource();
		if (IsQueuedToPerform() && (this == first_attacker))
			return true;
	}

	return false;
}


} // namespace private_battle

} // namespace hoa_battle
