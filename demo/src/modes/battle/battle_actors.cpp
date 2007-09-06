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
#include "input.h"
#include "system.h"
#include "global.h"
#include "script.h"
#include "battle.h"
#include "battle_actors.h"
#include "battle_actions.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_script;

using namespace hoa_battle::private_battle;

namespace hoa_battle {

namespace private_battle {

// *****************************************************************************
// BattleActor class
// *****************************************************************************

BattleActor::BattleActor(GlobalActor* actor, float x_origin, float y_origin) :
	_actor(actor),
	_state(ACTOR_INVALID),
	_x_origin(x_origin),
	_y_origin(y_origin),
	_x_location(x_origin),
	_y_location(y_origin),
	_total_time_damaged(0),
	_damage_dealt(0)
{
	// Reset attack timer, TEMP CODE!!!!
	_TEMP_attack_animation_timer.Initialize(0);
	_TEMP_attack_animation_timer.Run();
}



void BattleActor::ConstructInformation(hoa_utils::ustring& info, int32 ap_index) {
	info.clear();

	// Add character's name and attack point name if viable
	info += GetActor()->GetName();
	if (ap_index > 0 && static_cast<uint32>(ap_index) < GetActor()->GetAttackPoints()->size()) {
		info += MakeUnicodeString(" - ") + GetActor()->GetAttackPoints()->at(ap_index)->GetName();
	}
	info += MakeUnicodeString("\n");

	// Add character's current and max HP/SP amounts
	info += MakeUnicodeString("HP: ") + MakeUnicodeString(NumberToString(GetActor()->GetHitPoints())) +
		MakeUnicodeString(" / ") + MakeUnicodeString(NumberToString(GetActor()->GetMaxHitPoints())) + MakeUnicodeString("\n");
	info += MakeUnicodeString("SP: ") + MakeUnicodeString(NumberToString(GetActor()->GetSkillPoints())) +
		MakeUnicodeString(" / ") + MakeUnicodeString(NumberToString(GetActor()->GetMaxSkillPoints())) + MakeUnicodeString("\n");
} // void BattleActor::ConstructInformation(hoa_utils::ustring& info, int32 ap_index)



void BattleActor::DrawStaminaIcon(bool is_selected) {
	//if (IsAlive()) {
		VideoManager->Move(995, _stamina_icon_location);
		_stamina_icon.Draw();
		if (is_selected)
			current_battle->_stamina_icon_selected.Draw();
	//}
}


uint32 BattleActor::GetPhysicalAttack() {
	return GetActor()->GetTotalPhysicalAttack();
}

uint32 BattleActor::GetPhysicalDefense() {
	return GetActor()->GetTotalPhysicalDefense(0); /// @todo : fix the parameter to a correct attack point index!
}

float BattleActor::GetCombatEvade() {
	return GetActor()->GetEvade();
}



void BattleActor::ResetWaitTime() {
	_wait_time.Reset();
	_wait_time.Run();
	_stamina_icon_location = STAMINA_LOCATION_BOTTOM;
}



void BattleActor::TakeDamage(int32 damage) {
	_total_time_damaged = 1;

	if (damage <= 0) {
		_damage_dealt = RandomBoundedInteger(1, 5);
	}
	else {
		_damage_dealt = damage + static_cast<uint32>(RandomBoundedInteger(0, 4));
	}

	if (static_cast<uint32>(_damage_dealt) >= GetActor()->GetHitPoints()) { // Was it a killing blow?
		GetActor()->SetHitPoints(0);
		GetWaitTime()->Reset();
		current_battle->RemoveActionsForActor(this);
		_state = ACTOR_DEAD;
	}
	else {
		GetActor()->SubtractHitPoints(_damage_dealt);
	}
}



void BattleActor::TEMP_ResetAttackTimer() {
	_TEMP_attack_animation_timer.Initialize(1000);
	_TEMP_attack_animation_timer.Run();
}

// *****************************************************************************
// BattleCharacter class
// *****************************************************************************

BattleCharacter::BattleCharacter(GlobalCharacter* character, float x_origin, float y_origin) :
	BattleActor(character, x_origin, y_origin)
{
	if (_stamina_icon.Load("img/icons/actors/characters/" + character->GetFilename() + ".png", 45, 45) == false)
		cerr << "oh noes" << endl;

	_state = ACTOR_IDLE;
} // BattleCharacter::BattleCharacter(GlobalCharacter* character, float x_origin, float y_origin)



BattleCharacter::~BattleCharacter() {
}



void BattleCharacter::Update() {
	if (_state == ACTOR_IDLE && GetWaitTime()->IsRunning())
		_stamina_icon_location += SystemManager->GetUpdateTime() * (300.0f / _wait_time.GetDuration());
		//_stamina_icon_location += SystemManager->GetUpdateTime() * (405.0f / _wait_time.GetDuration());

	if (_state == ACTOR_ACTING) {
		if ((_x_location - _x_origin) < 50)
			_x_location += 0.8f * static_cast<float>(SystemManager->GetUpdateTime());
	}
	else
		SetXLocation(GetXOrigin()); // Restore original place

	GetActor()->RetrieveBattleAnimation("idle")->Update();
}



void BattleCharacter::DrawSprite() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);

	if (IsAlive()) {
		// Draw the actor selector image if this character is currently selected
		if (this == current_battle->_selected_character) {
			VideoManager->Move(_x_location - 20.0f, _y_location - 20.0f);
			current_battle->_actor_selection_image.Draw();
		}
		// Draw the character sprite
		VideoManager->Move(_x_location, _y_location);
		GetActor()->RetrieveBattleAnimation("idle")->Draw();

		if (this == current_battle->_selected_target) {
			VideoManager->Move(_x_location - 20.0f, _y_location - 20.0f);
			current_battle->_actor_selection_image.Draw();
		}

		// TEMP: determine if character sprite needs red damage numbers drawn next to it
		if (_total_time_damaged > 0) {
			_total_time_damaged += SystemManager->GetUpdateTime();
			VideoManager->SetFont("battle_dmg");
			VideoManager->SetTextColor(Color::red);
			VideoManager->Move(GetXLocation() + 40.0f, GetYLocation() + ( _total_time_damaged / 35.0f ) + 100.0f);
			VideoManager->DrawText(NumberToString(_damage_dealt));
			VideoManager->SetFont("battle");

			if (_total_time_damaged > 3000) { // Show it for three seconds
				_total_time_damaged = 0;
			}
		}
	}
	else {
		// TODO: draw the "incapacitated" character here
	}
} // void BattleCharacter::DrawSprite()



void BattleCharacter::DrawPortrait() {
	std::vector<StillImage> & portrait_frames = *(GetActor()->GetBattlePortraits());
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(48, 9);

	float hp_percent =  static_cast<float>(GetActor()->GetHitPoints()) / static_cast<float>(GetActor()->GetMaxHitPoints());

	if (GetActor()->GetHitPoints() == 0) {
		portrait_frames[4].Draw();
	}
	// The blend alpha will range from 1.0 to 0.0 in the following calculations
	else if (hp_percent < 0.25f) {
		portrait_frames[4].Draw();
		float alpha = (hp_percent) * 4;
		portrait_frames[3].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
  else if (hp_percent < 0.50f) {
		portrait_frames[3].Draw();
		float alpha = (hp_percent - 0.25f) * 4;
		portrait_frames[2].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else if (hp_percent < 0.75f) {
		portrait_frames[2].Draw();
		float alpha = (hp_percent - 0.50f) * 4;
		portrait_frames[1].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else if (hp_percent < 1.00f) {
		portrait_frames[1].Draw();
		float alpha = (hp_percent - 0.75f) * 4;
		portrait_frames[0].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else { // Character is at full health
		portrait_frames[0].Draw();
	}
} // void BattleCharacter::DrawPortrait()



void BattleCharacter::DrawStatus() {
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

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->SetTextColor(Color::white);

	// Draw the highlighted background if the character is selected
	if (current_battle->_selected_character == this) {
		VideoManager->Move(149, 84.0f + y_offset);
		current_battle->_character_selection.Draw();
	}

	// Draw the character's name
	VideoManager->SetDrawFlags(VIDEO_X_RIGHT, 0);
	VideoManager->Move(280.0f, 90.0f + y_offset);
 	VideoManager->DrawText(GetActor()->GetName());

	// If the swap key is being held down, draw status icons
	if (InputManager->SwapState()) {
		// TODO: draw status icons and information for actor
	}

	// Otherwise, draw the HP and SP bars (bars are 90 pixels wide and 6 pixels high)
	else {
		float bar_size;
		VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_NO_BLEND, 0);

		// Draw HP bar in green
		bar_size = static_cast<float>(90 * GetActor()->GetHitPoints()) / static_cast<float>(GetActor()->GetMaxHitPoints());
		VideoManager->Move(312, 90 + y_offset);

		if (GetActor()->GetHitPoints() > 0) {
			VideoManager->DrawRectangle(bar_size, 6, Color(0.133f, 0.455f, 0.133f, 1.0f));
		}

		// Draw SP bar in blue
		bar_size = static_cast<float>(90 * GetActor()->GetSkillPoints()) / static_cast<float>(GetActor()->GetMaxSkillPoints());
		VideoManager->Move(420, 90 + y_offset);

		if (GetActor()->GetSkillPoints() > 0) {
			VideoManager->DrawRectangle(bar_size, 6, Color(0.129f, 0.263f, 0.451f, 1.0f));
		}

		// Draw the cover image over the top of the bar
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->Move(293.0f, 84.0f + y_offset);
		current_battle->_character_bars.Draw();

		VideoManager->SetDrawFlags(VIDEO_X_CENTER, 0);
		// Draw the character's current health on top of the middle of the HP bar
		VideoManager->Move(355.0f, 94.0f + y_offset);
		VideoManager->DrawText(NumberToString(GetActor()->GetHitPoints()));

		// Draw the character's current skill points on top of the middle of the SP bar
		VideoManager->MoveRelative(110, 0);
		VideoManager->DrawText(NumberToString(GetActor()->GetSkillPoints()));
	}
} // void BattleCharacter::DrawStatus()

// /////////////////////////////////////////////////////////////////////////////
// BattleEnemy class
// /////////////////////////////////////////////////////////////////////////////

BattleEnemy::BattleEnemy(GlobalEnemy* enemy, float x_origin, float y_origin) :
	BattleActor(enemy, x_origin, y_origin)
{
	if (_stamina_icon.Load("img/icons/actors/enemies/" + _actor->GetFilename() + ".png", 45, 45) == false)
		cerr << "oh noes" << endl;

	_state = ACTOR_IDLE;
}



BattleEnemy::~BattleEnemy() {
	delete _actor;
}



// Compares the Y-coordinates of the actors, used for sorting the actors up-down when drawing
bool BattleEnemy::operator<(const BattleEnemy & other) const {
	// NOTE: this code is currently not working correctly
	//if ((_y_location - ((*GetActor()).GetHeight())) > (other.GetYLocation() - (*(other.GetActor()).GetHeight())))
	//	return true;
	return false;
}



void BattleEnemy::Update() {
	if (_state == ACTOR_IDLE) {
		if (_wait_time.IsFinished()) { // Indicates that the idle state is now finished
			_stamina_icon_location = STAMINA_LOCATION_SELECT;
			_state = ACTOR_WARM_UP;
			//Stop the timer!!
			_wait_time.Pause();
			_DecideAction();
		}
		else { // If still in IDLE state, update the stamina icon's location
			//_stamina_icon_location += SystemManager->GetUpdateTime() * (405.0f / _wait_time.GetDuration());
			_stamina_icon_location += SystemManager->GetUpdateTime() * (300.0f / _wait_time.GetDuration());
		}
		return;
	}

	// TEMP: while the enemy is attacking, update their location to show a little jolting horizontal movement
	if (_state == ACTOR_ACTING) {
		if ((_x_origin - _x_location) < 50)
			_x_location -= 0.8f * static_cast<float>(SystemManager->GetUpdateTime());
	}
	else {
		SetXLocation(GetXOrigin()); // Restore actor to original location
	}
}



void BattleEnemy::DrawSprite() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	std::vector<StillImage>& sprite_frames = *(GetActor()->GetBattleSpriteFrames());

	// Draw the sprite's final damage frame in grayscale and return
	if (_state == ACTOR_DEAD) {
		VideoManager->Move(_x_location, _y_location);
		sprite_frames[3].EnableGrayScale();
		sprite_frames[3].Draw();
		sprite_frames[3].DisableGrayScale();
	}
	else {
		// Draw the actor selector image over the currently selected enemy
		if (this == current_battle->_selected_target) {
			VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
			VideoManager->Move(_x_location + GetActor()->GetSpriteWidth() / 2, _y_location - 25);
			current_battle->_actor_selection_image.Draw();
			VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
		}

		// Draw the enemy's damage-blended sprite frames	
		VideoManager->Move(_x_location, _y_location);
		float hp_percent = static_cast<float>(GetActor()->GetHitPoints()) / static_cast<float>(GetActor()->GetMaxHitPoints());

		// Alpha will range from 1.0 to 0.0 in the following calculations
		if (hp_percent < 0.33f) {
			sprite_frames[3].Draw();
			float alpha = (hp_percent) * 3;
			sprite_frames[2].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
		}
		else if (hp_percent < 0.66f) {
			sprite_frames[2].Draw();
			float alpha = (hp_percent - 0.33f) * 3;
			sprite_frames[1].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
		}
		else if (hp_percent < 1.00f) {
			sprite_frames[1].Draw();
			float alpha = (hp_percent - 0.66f) * 3;
			sprite_frames[0].Draw(Color (1.0f, 1.0f, 1.0f, alpha));
		}
		else { // Enemy is at full health
			sprite_frames[0].Draw();
		}

		// Draw the attack point indicator if necessary
		if (this == current_battle->_selected_target && current_battle->_action_window->GetActionTargetType() == GLOBAL_TARGET_ATTACK_POINT) {
			VideoManager->PushState();
			VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
			std::vector<GlobalAttackPoint*>& attack_points = *(GetActor()->GetAttackPoints());

			VideoManager->Move(GetXLocation() + attack_points[current_battle->_selected_attack_point]->GetXPosition(),
				GetYLocation() + attack_points[current_battle->_selected_attack_point]->GetYPosition());
			current_battle->_attack_point_indicator.Draw();

			// Reset default X and Y draw orientation
			VideoManager->PopState();
		}
	}

	// TEMP: Determine if enemy needs to have red damage text drawn next to it
	if (_total_time_damaged > 0) {
		_total_time_damaged += SystemManager->GetUpdateTime();

		VideoManager->SetFont("battle_dmg");
		VideoManager->SetTextColor(Color::red);
		VideoManager->Move(GetXLocation() + 25.0f, GetYLocation() + ( _total_time_damaged / 35.0f ) + 80.0f);
		VideoManager->DrawText(NumberToString(_damage_dealt));
		VideoManager->SetFont("battle");

		if (_total_time_damaged > 3000) {
			_total_time_damaged = 0;
		}
	}
} // void BattleEnemy::DrawSprite()



void BattleEnemy::_DecideAction() {
	// TEMP: this selects the first skill the enemy has and the first character as a target. Needs to be changed
	GlobalSkill* skill = GetActor()->GetSkills()->begin()->second;
	BattleAction* action = new SkillAction(this, current_battle->GetPlayerCharacterAt(0), skill);
	current_battle->AddBattleActionToQueue(action);
	SetXLocation(GetXOrigin()); // Always attack from the starting location
} // void BattleEnemy::_DecideAction()

} // namespace private_battle

} // namespace hoa_battle
