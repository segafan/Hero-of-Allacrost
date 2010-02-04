////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
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
#include <string>

#include "input.h"
#include "script.h"

#include "battle.h"

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
	_actor_effects()
{
}


BattleActor::~BattleActor() {
	ScriptObject* remove;

	for (uint32 ctr = 0; ctr < _actor_effects.size(); ctr++) {
		remove = _actor_effects[ctr]->GetRemoveFunction();
		ScriptCallFunction<void>(*remove, this);
		delete(_actor_effects[ctr]);
	}
}

void BattleActor::ConstructInformation(hoa_utils::ustring& info, int32 ap_index) {
	info.clear();

	// Add character's name and attack point name if viable
	info += GetActor()->GetName();
	if (ap_index >= 0 && static_cast<uint32>(ap_index) < GetActor()->GetAttackPoints().size()) {
		info += MakeUnicodeString(" - ") + GetActor()->GetAttackPoints().at(ap_index)->GetName();
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
	uint32 atk = GetActor()->GetTotalPhysicalAttack();
	for (uint32 i = 0; i < _actor_effects.size(); i++) {
		atk += atk * _actor_effects[i]->GetStrModifier();
	}
	return atk;
}

uint32 BattleActor::GetPhysicalDefense() {
	uint32 pdef = GetActor()->GetTotalPhysicalDefense(0); /// @todo : fix the parameter to a correct attack point index!
	for (uint32 i = 0; i < _actor_effects.size(); i++) {
		pdef += pdef * _actor_effects[i]->GetForModifier();
	}
	return pdef;
}

float BattleActor::GetCombatEvade() {
	uint32 eva = GetActor()->GetEvade();
	for (uint32 i = 0; i < _actor_effects.size(); i++) {
		eva += eva * _actor_effects[i]->GetEvaModifier();
	}
	return eva;
}

uint32 BattleActor::GetCombatAgility() {
	uint32 agi = GetActor()->GetAgility();
	for (uint32 i = 0; i < _actor_effects.size(); i++) {
		agi += agi * _actor_effects[i]->GetAgiModifier();
	}
	return agi;
}



void BattleActor::ResetWaitTime() {
	_wait_time.Reset();
	_wait_time.Run();
	_stamina_icon_location = STAMINA_LOCATION_BOTTOM;
}



void BattleActor::TakeDamage(int32 damage) {
	//_total_time_damaged = 1;
	uint32 damage_dealt = 0;
	ustring text;

	if (damage <= 0)
	{
		//_damage_dealt = RandomBoundedInteger(1, 5);
		text = MakeUnicodeString("Miss");
	}
	else
	{
		damage_dealt = damage + static_cast<uint32>(RandomBoundedInteger(0, 4));
		text = MakeUnicodeString(NumberToString(damage_dealt));
	}

	current_battle->AddDamageText(text, 3000, GetXLocation() + 40.0f, GetYLocation() + 100.0f);

	// Was it a killing blow?
	if (/*static_cast<uint32>(_damage_dealt)*/damage_dealt >= GetActor()->GetHitPoints())
	{
		GetActor()->SetHitPoints(0);
		GetWaitTime()->Reset();
		_state = ACTOR_DEAD;

		//FIXME: This is going to change when we switch to map sprites
		//CD: We need to consolidate the ways enemy and character battle
		//anims are retrieved and updated.  There's no reason we should be
		//doing two different things for each.
		if (IsEnemy())
		{
			BattleEnemy* enemy = (BattleEnemy*)(this);
			std::vector<StillImage>& sprite_frames = *(enemy->GetActor()->GetBattleSpriteFrames());
			sprite_frames[3].EnableGrayScale();
		}
		else
		{
			BattleCharacter* character = (BattleCharacter*)(this);
			character->PlayAnimation("idle");
			character->GetActor()->RetrieveBattleAnimation("idle")->GetCurrentFrame()->EnableGrayScale();
		}

		current_battle->NotifyOfActorDeath(this);
	}
	else
	{
		GetActor()->SubtractHitPoints(damage_dealt);
	}
}

void BattleActor::AddEffect(hoa_global::GlobalStatusEffect* new_effect) {
	_actor_effects.push_back(new_effect);
	ScriptObject* init = new_effect->GetInitFunction();
	ScriptCallFunction<void>(*init, new_effect, this);
}

void BattleActor::AddNewEffect(int id) {
	hoa_global::GlobalStatusEffect* new_effect = new hoa_global::GlobalStatusEffect(id);
	AddEffect(new_effect);
}

void BattleActor::AddHitPoints(int32 hp) {
	if (hp < 0)
		return;

	GetActor()->AddHitPoints(hp);
}

// *****************************************************************************
// BattleCharacter class
// *****************************************************************************

BattleCharacter::BattleCharacter(GlobalCharacter* character, float x_origin, float y_origin) :
	BattleActor(character, x_origin, y_origin),
	_animation_string("idle"),
	_animation_time(0)
{
	if (_stamina_icon.Load("img/icons/actors/characters/" + character->GetFilename() + ".png", 45, 45) == false)
		PRINT_ERROR << "Unable to load stamina icon for " << character->GetFilename() << endl;

	_state = ACTOR_IDLE;

	_name_text.SetStyle(TextStyle("title22"));
	_name_text.SetText(GetActor()->GetName());
	_hit_points_text.SetStyle(TextStyle("text20"));
	_hit_points_text.SetText(NumberToString(GetActor()->GetHitPoints()));
	_skill_points_text.SetStyle(TextStyle("text20"));
	_skill_points_text.SetText(NumberToString(GetActor()->GetSkillPoints()));
} // BattleCharacter::BattleCharacter(GlobalCharacter* character, float x_origin, float y_origin)



BattleCharacter::~BattleCharacter() {
}



void BattleCharacter::Update() {
	bool paused = false;

	for (uint32 i = 0; i < GetActorEffects().size(); i++) {
		if (GetActorEffects().at(i)->IsStunEffect()) {
			_wait_time.Pause();
			paused = true;
		}
		if (GetActorEffects().at(i)->GetTimer()->IsFinished()) {
			_wait_time.Run();
			paused = false;
		}
	}

	if (_state == ACTOR_IDLE && !paused) {
		if (_wait_time.IsRunning())
		{
			_stamina_icon_location += SystemManager->GetUpdateTime() * ((STAMINA_LOCATION_SELECT - STAMINA_LOCATION_BOTTOM) / _wait_time.GetDuration());
		}
		else if (_wait_time.IsFinished())
		{
			_state = ACTOR_AWAITING_TURN;
			current_battle->AddToTurnQueue(this);
		}
	}

	//If he's dead, we freeze him on his last frame
	if (IsAlive())
		GetActor()->RetrieveBattleAnimation(_animation_string)->Update();

// 	ScriptObject* remove;

	for (uint32 i = 0; i < GetActorEffects().size(); i++) {
		if (GetActorEffects().at(i)->GetTimer()->IsFinished()) {
			// FIXME: remove functions do not work, though we don't really use them anyway
			// remove = _actor_effects[i]->GetRemoveFunction();
			// ScriptCallFunction<void>(*remove, this); // status effect's time is up
			delete(_actor_effects.at(i));
			_actor_effects.at(i) = new GlobalStatusEffect(1); // replace it with a dummy
		}
	}
}



void BattleCharacter::DrawSprite() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);

	// Draw the character sprite
	VideoManager->Move(_x_location, _y_location);

	if (_animation_string == "idle") {
		// no need to do anything
	}
	else if (_animation_time.IsFinished()) {
		_animation_string = "idle";
	}
	else {
		uint32 dist = 120 * _animation_time.GetTimeExpired() / _animation_time.GetDuration();
		VideoManager->MoveRelative(dist, 0);
	}

	GetActor()->RetrieveBattleAnimation(_animation_string)->Draw();


	if (IsAlive()) {
		// Draw the actor selector image if this character is currently selected
		if (this == current_battle->_selected_character) {
			VideoManager->Move(_x_location - 20.0f, _y_location - 20.0f);
			current_battle->_actor_selection_image.Draw();
		}

		if (this == current_battle->_selected_target) {
			VideoManager->Move(_x_location - 20.0f, _y_location - 20.0f);
			current_battle->_actor_selection_image.Draw();
		}
	}
} // void BattleCharacter::DrawSprite()

void BattleCharacter::PlayAnimation(std::string alias) {
	_animation_string = alias;
	GetActor()->RetrieveBattleAnimation(_animation_string)->ResetAnimation();
	_animation_time.Reset();
	_animation_time.SetDuration(300);
	_animation_time.Run();
}


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

	// Draw the highlighted background if the character is selected
	if (current_battle->_selected_character == this) {
		VideoManager->Move(149.0f, 84.0f + y_offset);
		current_battle->_character_selection.Draw();
	}

	// Draw the character's name
	VideoManager->SetDrawFlags(VIDEO_X_RIGHT, 0);
	VideoManager->Move(280.0f, 82.0f + y_offset);
	_name_text.Draw();

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
		VideoManager->Move(312.0f, 90.0f + y_offset);

		if (GetActor()->GetHitPoints() > 0) {
			VideoManager->DrawRectangle(bar_size, 6, Color(0.133f, 0.455f, 0.133f, 1.0f));
		}

		// Draw SP bar in blue
		bar_size = static_cast<float>(90 * GetActor()->GetSkillPoints()) / static_cast<float>(GetActor()->GetMaxSkillPoints());
		VideoManager->Move(420.0f, 90.0f + y_offset);

		if (GetActor()->GetSkillPoints() > 0) {
			VideoManager->DrawRectangle(bar_size, 6, Color(0.129f, 0.263f, 0.451f, 1.0f));
		}

		// Draw the cover image over the top of the bar
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->Move(293.0f, 84.0f + y_offset);
		current_battle->_character_bars.Draw();

		// TODO: The SetText calls below should not be done here. They should be made whenever the character's HP/SP
		// is modified. This re-renders the text every frame regardless of whether or not the HP/SP changed so its
		// not efficient

		VideoManager->SetDrawFlags(VIDEO_X_CENTER, 0);
		// Draw the character's current health on top of the middle of the HP bar
		VideoManager->Move(355.0f, 90.0f + y_offset);
		_hit_points_text.SetText(NumberToString(GetActor()->GetHitPoints()));
		_hit_points_text.Draw();

		// Draw the character's current skill points on top of the middle of the SP bar
		VideoManager->MoveRelative(110, 0);
		_skill_points_text.SetText(NumberToString(GetActor()->GetSkillPoints()));
		_skill_points_text.Draw();
	}
} // void BattleCharacter::DrawStatus()

// /////////////////////////////////////////////////////////////////////////////
// BattleEnemy class
// /////////////////////////////////////////////////////////////////////////////

BattleEnemy::BattleEnemy(GlobalEnemy* enemy, float x_origin, float y_origin) :
	BattleActor(enemy, x_origin, y_origin),
	_animation_string("idle"),
	_animation_time(0)
{
	if (_stamina_icon.Load("img/icons/actors/enemies/" + _actor->GetFilename() + ".png", 45, 45) == false)
		cerr << "ERROR: In BattleEnemy constructor" << endl;

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
	bool paused = false;

	for (uint32 i = 0; i < GetActorEffects().size(); i++) {
		if (GetActorEffects().at(i)->IsStunEffect()) {
			_wait_time.Pause();
			paused = true;
		}
		if (GetActorEffects().at(i)->GetTimer()->IsFinished()) {
			_wait_time.Run();
			paused = false;
		}
	}

	if (_state == ACTOR_IDLE && !paused) {
		if (_wait_time.IsFinished()) { // Indicates that the idle state is now finished
			_stamina_icon_location = STAMINA_LOCATION_SELECT;
			_state = ACTOR_WARM_UP;
			//Stop the timer!!
			_wait_time.Pause();
			_DecideAction();
		}
		else { // If still in IDLE state, update the stamina icon's location
			_stamina_icon_location += SystemManager->GetUpdateTime() * (300.0f / _wait_time.GetDuration());
		}
		return;
	}
}



void BattleEnemy::DrawSprite() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	std::vector<StillImage>& sprite_frames = *(GetActor()->GetBattleSpriteFrames());

	uint32 enemy_draw_offset = 0;

	if (_animation_string == "idle") {
		// no need to do anything
	}
	else if (_animation_time.IsFinished()) {
		_animation_string = "idle";
	}
	else {
		enemy_draw_offset = 80 * _animation_time.GetTimeExpired() / _animation_time.GetDuration();
	}

	// Draw the sprite's final damage frame in grayscale and return
	if (_state == ACTOR_DEAD) {
		VideoManager->Move(_x_location, _y_location);
		sprite_frames[3].Draw();
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
		VideoManager->Move(_x_location - enemy_draw_offset, _y_location);

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
		if (this == current_battle->_selected_target && current_battle->_action_window.GetActionTargetType() == GLOBAL_TARGET_ATTACK_POINT) {
			VideoManager->PushState();
			VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
			const vector<GlobalAttackPoint*>& attack_points = GetActor()->GetAttackPoints();

			VideoManager->Move(GetXLocation() + attack_points[current_battle->_selected_attack_point]->GetXPosition(),
				GetYLocation() + attack_points[current_battle->_selected_attack_point]->GetYPosition());
			current_battle->_attack_point_indicator.Draw();

			// Reset default X and Y draw orientation
			VideoManager->PopState();
		}
	}
} // void BattleEnemy::DrawSprite()


void BattleEnemy::PlayAnimation(std::string alias) {
	_animation_string = alias;
	_animation_time.SetDuration(270);
	_animation_time.Reset();
	_animation_time.Run();
}

void BattleEnemy::_DecideAction() {
	// TEMP: this selects the first skill the enemy has and the first character as a target. Needs to be changed
	// changed to choose random character
	// changed again so laila get's attacked less.
	// this is a hack and needs to be fixed, won't work when there is more people in the party
	uint32 rand = RandomBoundedInteger(0, 100);
	int32 target;
	if (rand < 70)
		target = 0;
	else
	{
		if (GlobalManager->GetActiveParty()->GetPartySize() > 1)
			target = 1;
		else
			target = 0;
	}
	GlobalSkill* skill = GetActor()->GetSkills().begin()->second;
	BattleAction* action = new SkillAction(this, current_battle->GetPlayerCharacterAt(target), skill);
	current_battle->AddBattleActionToQueue(action);
	SetXLocation(GetXOrigin()); // Always attack from the starting location
} // void BattleEnemy::_DecideAction()

} // namespace private_battle

} // namespace hoa_battle
