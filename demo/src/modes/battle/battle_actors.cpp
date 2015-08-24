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

#include "input.h"
#include "script.h"

#include "battle.h"
#include "battle_actions.h"
#include "battle_actors.h"
#include "battle_effects.h"
#include "battle_indicators.h"
#include "battle_utils.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_script;

namespace hoa_battle {

namespace private_battle {

////////////////////////////////////////////////////////////////////////////////
// BattleActor class
////////////////////////////////////////////////////////////////////////////////

BattleActor::BattleActor(GlobalActor* actor) :
	GlobalActor(*actor),
	_state(ACTOR_STATE_INVALID),
	_global_actor(actor),
	_action(NULL),
	_x_origin(0.0f),
	_y_origin(0.0f),
	_x_location(0.0f),
	_y_location(0.0f),
	_execution_finished(false),
	_idle_state_time(0),
	_effects_supervisor(new EffectsSupervisor(this)),
	_indicator_supervisor(new IndicatorSupervisor(this))
{
	if (actor == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "constructor received NULL argument" << endl;
		return;
	}

	// TODO: I have concerns about the copy constructor for GlobalActor. Currently it creates a copy
	// of every single attack point, weapon, armor, and skill. I wonder if perhaps we should only
	// create a copy of the attack point
}



BattleActor::~BattleActor() {
	// If the actor did not get a chance to execute their action, delete it
	if (_action != NULL) {
		delete _action;
		_action = NULL;
	}

	delete _effects_supervisor;
	delete _indicator_supervisor;
}



void BattleActor::ChangeState(ACTOR_STATE new_state) {
	if (_state == new_state) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "actor was already in new state: " << new_state << endl;
		return;
	}

	_state = new_state;
	_state_timer.Reset();
	switch (_state) {
		case ACTOR_STATE_IDLE:
			if (_action != NULL) {
				delete _action;
				_action = NULL;
			}
			_state_timer.Initialize(_idle_state_time);
			_state_timer.Run();
			break;
		case ACTOR_STATE_WARM_UP:
			if (_action == NULL) {
				IF_PRINT_WARNING(BATTLE_DEBUG) << "no action available during state change: " << _state << endl;
			}
			else {
				_state_timer.Initialize(_action->GetWarmUpTime());
				_state_timer.Run();
			}
			break;
		case ACTOR_STATE_READY:
			if (_action == NULL) {
				IF_PRINT_WARNING(BATTLE_DEBUG) << "no action available during state change: " << _state << endl;
			}
			else {
				BattleMode::CurrentInstance()->NotifyActorReady(this);
			}
			break;
		case ACTOR_STATE_COOL_DOWN:
			_execution_finished = false;
			if (_action == NULL) {
				IF_PRINT_WARNING(BATTLE_DEBUG) << "no action available during state change: " << _state << endl;
			}
			else {
				_state_timer.Initialize(_action->GetCoolDownTime());
				_state_timer.Run();
			}
			break;
		case ACTOR_STATE_DEAD:
			BattleMode::CurrentInstance()->NotifyActorDeath(this);
			break;
		default:
			break;
	}
}



void BattleActor::RegisterDamage(uint32 amount) {
	if (amount == 0) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function called with a zero value argument" << endl;
		RegisterMiss();
		return;
	}
	if (_state == ACTOR_STATE_DEAD) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function called when actor state was dead" << endl;
		RegisterMiss();
		return;
	}

	SubtractHitPoints(amount);
	_indicator_supervisor->AddDamageIndicator(amount);

	if (GetHitPoints() == 0)
		ChangeState(ACTOR_STATE_DEAD);
}



void BattleActor::RegisterHealing(uint32 amount) {
	if (amount == 0) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function called with a zero value argument" << endl;
		RegisterMiss();
		return;
	}
	if (_state == ACTOR_STATE_DEAD) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function called when actor state was dead" << endl;
		RegisterMiss();
		return;
	}

	AddHitPoints(amount);
	_indicator_supervisor->AddHealingIndicator(amount);
}



void BattleActor::RegisterMiss() {
	_indicator_supervisor->AddMissIndicator();
}



void BattleActor::RegisterStatusChange(GLOBAL_STATUS status, GLOBAL_INTENSITY intensity) {
	GLOBAL_INTENSITY old_intensity = GLOBAL_INTENSITY_INVALID;
	GLOBAL_INTENSITY new_intensity = intensity;

	// TODO: determine if an opposite status effect is active and account for it

	old_intensity = _effects_supervisor->ChangeStatus(status, new_intensity);

	// If the return value was invalid, no status was changed so do not create any indicator
	if (old_intensity == GLOBAL_INTENSITY_INVALID) {
		return;
	}
	_indicator_supervisor->AddStatusIndicator(status, old_intensity, status, new_intensity);
}



void BattleActor::ChangeSkillPoints(int32 amount) {
	uint32 unsigned_amount = static_cast<uint32>(amount);

	// Modify actor's skill points accordingly
	if (amount > 0)
		AddSkillPoints(unsigned_amount);
	else if (amount < 0)
		SubtractSkillPoints(unsigned_amount);

	// TODO: SP change text needs to be implemented
}



void BattleActor::Update() {
	_state_timer.Update();
	_effects_supervisor->Update();
	_indicator_supervisor->Update();

	if (_state == ACTOR_STATE_IDLE) {
		if (_state_timer.IsFinished() == true)
			ChangeState(ACTOR_STATE_COMMAND);
	}
	else if (_state == ACTOR_STATE_WARM_UP) {
		if (_state_timer.IsFinished() == true)
			ChangeState(ACTOR_STATE_READY);
	}
	else if (_state == ACTOR_STATE_COOL_DOWN) {
		if (_state_timer.IsFinished() == true)
			ChangeState(ACTOR_STATE_IDLE);
	}
}



void BattleActor::DrawIndicators() const {
	_indicator_supervisor->Draw();
}



void BattleActor::SetAction(BattleAction* action) {
	if (action == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received NULL argument" << endl;
		return;
	}
	if (_state != ACTOR_STATE_COMMAND) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "actor was not in the command state when function was called" << endl;
		delete action;
		return;
	}
	if (_action != NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "actor already had another action set -- overridding" << endl;
		delete _action;
	}

	_action = action;
}



uint32 BattleActor::TotalPhysicalDefense() {
	uint32 phys_defense = 0;

	for (uint32 i = 0; i < _attack_points.size(); i++)
		phys_defense += _attack_points[i]->GetTotalPhysicalDefense();
	phys_defense /= _attack_points.size();

	return phys_defense;
}



uint32 BattleActor::TotalMetaphysicalDefense() {
	uint32 meta_defense = 0;

	for (uint32 i = 0; i < _attack_points.size(); i++)
		meta_defense += _attack_points[i]->GetTotalMetaphysicalDefense();
	meta_defense /= _attack_points.size();

	return meta_defense;
}



float BattleActor::TotalEvadeRating() {
	float evade = 0.0f;

	for (uint32 i = 0; i < _attack_points.size(); i++)
		evade += _attack_points[i]->GetTotalEvadeRating();
	evade /= static_cast<float>(_attack_points.size());

	return evade;
}

////////////////////////////////////////////////////////////////////////////////
// BattleCharacter class
////////////////////////////////////////////////////////////////////////////////

BattleCharacter::BattleCharacter(GlobalCharacter* character) :
	BattleActor(character),
	_global_character(character),
	_sprite_animation_alias("idle"),
	_animation_timer(0)
{
	if (_stamina_icon.Load("img/icons/actors/characters/" + character->GetFilename() + ".png", 45, 45) == false)
		PRINT_ERROR << "unable to load stamina icon for character: " << character->GetFilename() << endl;

	_name_text.SetStyle(TextStyle("title22"));
	_name_text.SetText(GetName());
	_hit_points_text.SetStyle(TextStyle("text20"));
	_hit_points_text.SetText(NumberToString(GetHitPoints()));
	_skill_points_text.SetStyle(TextStyle("text20"));
	_skill_points_text.SetText(NumberToString(GetSkillPoints()));
}



BattleCharacter::~BattleCharacter() {
	// If character was about to use an item before being destructed, restore it to inventory
	if ((_action != NULL) && (_action->IsItemAction() == true)) {
		// TODO: not sure if this is necessary/safe to do.
// 		ItemAction* item_action = dynamic_cast<ItemAction*>(_action);
// 		item_action->GetItem()->IncrementCount(1);
	}
}



void BattleCharacter::ChangeState(ACTOR_STATE new_state) {
	BattleActor::ChangeState(new_state);

	switch (_state) {
		case ACTOR_STATE_COMMAND:
			BattleMode::CurrentInstance()->NotifyCharacterCommand(this);
			break;
		case ACTOR_STATE_ACTING:
			// TODO: reset state timer?
			break;
		case ACTOR_STATE_DEAD:
			ChangeSpriteAnimation("idle");
			_global_character->RetrieveBattleAnimation("idle")->GetCurrentFrame()->EnableGrayScale();
			break;
		default:
			break;
	}
}



void BattleCharacter::Update() {
	BattleActor::Update();
	_animation_timer.Update();

	// Update the active sprite animation
	if (IsAlive() == true) {
		_global_character->RetrieveBattleAnimation(_sprite_animation_alias)->Update();
	}

	// If the character is executing their action,
	if (_state == ACTOR_STATE_ACTING) {
		if (_action->Execute() == true) {
			ChangeState(ACTOR_STATE_COOL_DOWN);
		}
	}
}



void BattleCharacter::DrawSprite() {
	// Draw the character sprite
	VideoManager->Move(_x_location, _y_location);

	if (_sprite_animation_alias == "idle") {
		// no need to do anything
	}
	else if (_animation_timer.IsFinished()) {
		_sprite_animation_alias = "idle";
	}
	else {
		uint32 dist = 120 * _animation_timer.GetTimeExpired() / _animation_timer.GetDuration();
		VideoManager->MoveRelative(dist, 0.0f);
	}

	_global_character->RetrieveBattleAnimation(_sprite_animation_alias)->Draw();
} // void BattleCharacter::DrawSprite()




void BattleCharacter::ChangeSpriteAnimation(const std::string& alias) {
	_sprite_animation_alias = alias;
	_global_character->RetrieveBattleAnimation(_sprite_animation_alias)->ResetAnimation();
	_animation_timer.Reset();
	_animation_timer.SetDuration(300);
	_animation_timer.Run();
}



void BattleCharacter::DrawPortrait() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(48.0f, 9.0f);

	vector<StillImage>& portrait_frames = *(_global_character->GetBattlePortraits());
	float hp_percent =  static_cast<float>(GetHitPoints()) / static_cast<float>(GetMaxHitPoints());

	if (GetHitPoints() == GetMaxHitPoints()) {
		portrait_frames[0].Draw();
	}
	else if (GetHitPoints() == 0) {
		portrait_frames[4].Draw();
	}
	else if (hp_percent > 0.75f) {
		portrait_frames[0].Draw();
		float alpha = 1.0f - ((hp_percent - 0.75f) * 4.0f);
		portrait_frames[1].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else if (hp_percent > 0.50f) {
		portrait_frames[1].Draw();
		float alpha = 1.0f - ((hp_percent - 0.50f) * 4.0f);
		portrait_frames[2].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else if (hp_percent > 0.25f) {
		portrait_frames[2].Draw();
		float alpha = 1.0f - ((hp_percent - 0.25f) * 4.0f);
		portrait_frames[3].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else { // (hp_precent > 0.0f)
		portrait_frames[3].Draw();
		float alpha = 1.0f - (hp_percent * 4.0f);
		portrait_frames[4].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
}



void BattleCharacter::DrawStatus(uint32 order) {
	// Used to determine where to draw the character's status
	float y_offset = 0.0f;

	// Determine what vertical order the character is in and set the y_offset accordingly
	switch (order) {
		case 0:
			y_offset = 0.0f;
			break;
		case 1:
			y_offset = -25.0f;
			break;
		case 2:
			y_offset = -50.0f;
			break;
		case 3:
			y_offset = -75.0f;
			break;
		default:
			IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid order argument: " << order << endl;
			y_offset = 0.0f;
	}

	// Draw the character's name
	VideoManager->SetDrawFlags(VIDEO_X_RIGHT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(280.0f, 82.0f + y_offset);
	_name_text.Draw();

	// If the swap key is being held down, draw status icons
	if (InputManager->SwapState()) {
		VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
		VideoManager->MoveRelative(10.0f, 0.0f);
		_effects_supervisor->Draw();
	}

	// Otherwise, draw the HP and SP bars (bars are 90 pixels wide and 6 pixels high)
	else {
		float bar_size;
		VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_NO_BLEND, 0);

		// Draw HP bar in green
		bar_size = static_cast<float>(90 * GetHitPoints()) / static_cast<float>(GetMaxHitPoints());
		VideoManager->Move(312.0f, 90.0f + y_offset);

		if (GetHitPoints() > 0) {
			VideoManager->DrawRectangle(bar_size, 6, Color(0.133f, 0.455f, 0.133f, 1.0f));
		}

		// Draw SP bar in blue
		bar_size = static_cast<float>(90 * GetSkillPoints()) / static_cast<float>(GetMaxSkillPoints());
		VideoManager->Move(420.0f, 90.0f + y_offset);

		if (GetSkillPoints() > 0) {
			VideoManager->DrawRectangle(bar_size, 6, Color(0.129f, 0.263f, 0.451f, 1.0f));
		}

		// Draw the cover image over the top of the bar
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->Move(293.0f, 84.0f + y_offset);
		BattleMode::CurrentInstance()->GetCharacterBarCovers().Draw();

		// TODO: The SetText calls below should not be done here. They should be made whenever the character's HP/SP
		// is modified. This re-renders the text every frame regardless of whether or not the HP/SP changed so its
		// not efficient

		VideoManager->SetDrawFlags(VIDEO_X_CENTER, 0);
		// Draw the character's current health on top of the middle of the HP bar
		VideoManager->Move(355.0f, 90.0f + y_offset);
		_hit_points_text.SetText(NumberToString(GetHitPoints()));
		_hit_points_text.Draw();

		// Draw the character's current skill points on top of the middle of the SP bar
		VideoManager->MoveRelative(110.0f, 0.0f);
		_skill_points_text.SetText(NumberToString(GetSkillPoints()));
		_skill_points_text.Draw();
	}
} // void BattleCharacter::DrawStatus()

// /////////////////////////////////////////////////////////////////////////////
// BattleEnemy class
// /////////////////////////////////////////////////////////////////////////////

BattleEnemy::BattleEnemy(GlobalEnemy* enemy) :
	BattleActor(enemy),
	_global_enemy(enemy)
{
	if (_stamina_icon.Load("img/icons/actors/enemies/" + _global_actor->GetFilename() + ".png", 45, 45) == false)
		PRINT_ERROR << "failed to load enemy stamina icon: " << _global_actor->GetFilename() << endl;
}



BattleEnemy::~BattleEnemy() {
	delete _global_actor;
}



// Compares the Y-coordinates of the actors, used for sorting the actors up-down when drawing
bool BattleEnemy::operator<(const BattleEnemy & other) const {
	// NOTE: this code is currently not working correctly
	//if ((_y_location - ((*GetActor()).GetHeight())) > (other.GetYLocation() - (*(other.GetActor()).GetHeight())))
	//	return true;
	return false;
}



void BattleEnemy::ChangeState(ACTOR_STATE new_state) {
	BattleActor::ChangeState(new_state);

	vector<StillImage>& sprite_frames = *(_global_enemy->GetBattleSpriteFrames());
	switch (_state) {
		case ACTOR_STATE_COMMAND:
			_DecideAction();
			ChangeState(ACTOR_STATE_WARM_UP);
			break;
		case ACTOR_STATE_ACTING:
			_state_timer.Initialize(400); // TEMP: 400ms is a random time for the enemy sprite to move
			_state_timer.Run();
			break;
		case ACTOR_STATE_DEAD:
			sprite_frames[3].EnableGrayScale();
			break;
		default:
			break;
	}
}



void BattleEnemy::Update() {
	BattleActor::Update();

	if (_state == ACTOR_STATE_ACTING) {
		if (_execution_finished == false)
			_execution_finished = _action->Execute();

		if ((_execution_finished == true) && (_state_timer.IsFinished() == true))
			ChangeState(ACTOR_STATE_COOL_DOWN);
	}
}



void BattleEnemy::DrawSprite() {
	vector<StillImage>& sprite_frames = *(_global_enemy->GetBattleSpriteFrames());

	// Draw the sprite's final damage frame, which should have grayscale enabled
	if (_state == ACTOR_STATE_DEAD) {
		VideoManager->Move(_x_location, _y_location);
		sprite_frames[3].Draw();
		return;
	}

	// TEMP: when the actor is acting, change its x draw position to show it move forward and then
	// backward one tile as it completes its execution. In the future this functionality should be
	// replaced by modifying the enemy's draw location members directly
	uint32 enemy_draw_offset = 0;
	if (_state == ACTOR_STATE_ACTING) {
		if (_state_timer.PercentComplete() <= 0.50f)
			enemy_draw_offset = TILE_SIZE * (2.0f * _state_timer.PercentComplete());
		else
			enemy_draw_offset = TILE_SIZE * (2.0f - 2.0f * _state_timer.PercentComplete());
	}

	// Draw the enemy's damage-blended sprite frames
	VideoManager->Move(_x_location - enemy_draw_offset, _y_location);

	float hp_percent = static_cast<float>(GetHitPoints()) / static_cast<float>(GetMaxHitPoints());

	// Alpha will range from 1.0 to 0.0 in the following calculations
	if (GetHitPoints() == GetMaxHitPoints()) {
		sprite_frames[0].Draw();
	}
	else if (GetHitPoints() == 0) {
		sprite_frames[3].Draw();
	}
	else if (hp_percent > 0.666f) {
		sprite_frames[0].Draw();
		float alpha = 1.0f - ((hp_percent - 0.666f) * 3.0f);
		sprite_frames[1].Draw(Color (1.0f, 1.0f, 1.0f, alpha));
	}
	else if (hp_percent >  0.333f) {
		sprite_frames[1].Draw();
		float alpha = 1.0f - ((hp_percent - 0.333f) * 3.0f);
		sprite_frames[2].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
	else { // (hp_precent > 0.0f)
		sprite_frames[2].Draw();
		float alpha = 1.0f - (hp_percent * 3.0f);
		sprite_frames[3].Draw(Color(1.0f, 1.0f, 1.0f, alpha));
	}
} // void BattleEnemy::DrawSprite()



void BattleEnemy::_DecideAction() {
	if (_global_enemy->GetSkills().empty() == true) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "enemy had no usable skills" << endl;
		ChangeState(ACTOR_STATE_IDLE);
	}

	// TODO: this method is mostly temporary and makes no intelligent decisions about what action to
	// take or on what target to select. Currently this method does the following.
	//
	// (1): select the first skill that the enemy has available
	// (2): select a random character that is not in the dead state to target
	// (3): select a random attack point on the selected character target
	//
	// Therefore, only skills that target attack points on enemies are valid. No party or actor targets
	// will work. Obviously these needs will be addressed eventually.

	// TEMP: select a random skill to use
	GlobalSkill* skill = NULL;
	skill = _global_enemy->GetSkills().begin()->second;

	// TEMP: select a random living character in the party for the target
	BattleTarget target;

	deque<BattleCharacter*> alive_characters = BattleMode::CurrentInstance()->GetCharacterActors();
	deque<BattleCharacter*>::iterator character_iterator = alive_characters.begin();
	while (character_iterator != alive_characters.end()) {
		if ((*character_iterator)->IsAlive() == false)
			character_iterator = alive_characters.erase(character_iterator);
		else
			character_iterator++;
	}

	if (alive_characters.empty() == true) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "no characters were alive when enemy was selecting a target" << endl;
		ChangeState(ACTOR_STATE_IDLE);
		return;
	}

	uint32 point_target = 0;
	BattleActor* actor_target = NULL;

	// TEMP: select a random alive character
	if (alive_characters.size() == 1)
		actor_target = alive_characters[0];
	else
		actor_target = alive_characters[RandomBoundedInteger(0, alive_characters.size() - 1)];

	// TEMP: select a random attack point on the target character
	uint32 num_points = actor_target->GetAttackPoints().size();
	if (num_points == 1)
		point_target = 0;
	else
		point_target = RandomBoundedInteger(0, num_points - 1);

	// TEMP: Should not statically assign to target a foe point. Examine the selected skill's target type
	target.SetPointTarget(GLOBAL_TARGET_FOE_POINT, point_target, actor_target);

	SetAction(new SkillAction(this, target, skill));
}

} // namespace private_battle

} // namespace hoa_battle
