////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_actors.h
*** \author  Corey Hoffstein, visage@allacrost.org
*** \brief   Header file for actors present in battles.
***
*** This code contains the implementation of battle actors (characters and
*** enemies) whom are represented on the field of battle.
*** ***************************************************************************/

#ifndef __BATTLE_ACTORS_HEADER__
#define __BATTLE_ACTORS_HEADER__

#include "utils.h"
#include "defs.h"
#include "mode_manager.h"
#include "global.h"
#include "video.h"
#include "audio.h"

#include "battle.h"

namespace hoa_battle {

namespace private_battle {

/** \brief Actor Effects affect the stats of an Actor, be it burn, sleep, frozen, poison, et cetera.
***
**/
class ActorEffect {
public:
	ActorEffect(private_battle::BattleActor* const AHost, std::string AEffectName, StatusSeverity AHowSevere,
		uint32 ATTL, bool ACanMove, uint32 AHealthModifier, uint32 ASkillPointModifier, uint32 AStrengthModifier,
		uint32 AIntelligenceModifier, uint32 AAgilityModifier, uint32 AUpdateLength);

	virtual ~ActorEffect()
		{}

	//! Update the effect
	void Update(uint32 ATimeElapsed)
		{}

	//@{
	uint32 GetTTL () const
		{ return _TTL; }
	BattleActor * const GetHost () const
		{ return _host; }
	std::string GetEffectName () const
		{ return _effect_name; }
	uint32 GetUpdateLength () const
		{ return _update_length; }
	uint32 GetLastUpdate () const
		{ return _last_update; }
	bool CanMove () const
		{ return _can_move; }
	uint32 GetHealthModifier () const
		{ return _health_modifier; }
	uint32 GetSkillPointModifier () const
		{ return _skill_point_modifier; }
	uint32 GetStrengthModifier () const
		{ return _strength_modifier; }
	uint32 GetIntelligenceModifier () const
		{ return _intelligence_modifier; }
	uint32 GetAgilityModifier () const
		{ return _agility_modifier; }
	//@}

	void SetLastUpdate (const uint32 ALastUpdate)
		{ _last_update = ALastUpdate; }

	//! Undo the effect on the host
	void UndoEffect () const
		{}

private:
	//! Who we are effecting
	private_battle::BattleActor* _host;
	//! The name of the effect
	std::string _effect_name;
	//! The length the effect will last
	uint32 _TTL;

	StatusSeverity _severeness;

	//! How often the effect does something -1 for update once
	bool _can_move;

	uint32 _health_modifier;
	uint32 _skill_point_modifier;
	uint32 _strength_modifier;
	uint32 _intelligence_modifier;
	uint32 _agility_modifier;

	//how often to update the effect
	uint32 _update_length;
	//! How old the effect is
	uint32 _age;
	//! When the last update was
	uint32 _last_update;
	//! How many times this effect updated on the player
	uint32 _times_updated;

	void _SubtractTTL (uint32 dt);
}; // class ActorEffect


/** \brief Actor is the general entity partaking in battle.
*** It will be inherited by player actors and enemy actors.
**/
class BattleActor {
public:
	BattleActor(BattleMode * ABattleMode, uint32 AXLocation, uint32 AYLocation);
	virtual ~BattleActor();

	virtual void Update(uint32 ATimeElapsed) = 0;
	virtual void Draw() = 0;

	void Die();
	const bool IsAlive() const
		{ return _is_alive; }

	//! \brief Get the mode we are currently fighting in
	BattleMode *GetOwnerBattleMode() const
		{ return _owner_battle_mode; }

	//! \brief Manipulate the effects that the player is affected by
	//@{
	void UpdateEffects(uint32 ATimeElapsed);
	void PushEffect(const ActorEffect AEffect);
	//@}

	//! \brief BattleActor class member access functions
	//@{
	bool IsMoveCapable() const
		{ return _is_move_capable; }
	bool IsQueuedToPerform() const
		{ return _is_queued_to_perform; }
	bool IsWarmingUp() const
		{ return _warmup_time != 0; }
	bool IsInDefensiveMode() const
		{ return _defensive_mode_bonus != 0; }

	void SetMoveCapable(bool AMoveCapable)
		{ _is_move_capable = AMoveCapable; }
	void SetQueuedToPerform(bool AQueuedToPerform)
		{ _is_queued_to_perform = AQueuedToPerform; }
	void SetWarmupTime(uint32 AWarmupTime)
		{ _warmup_time = AWarmupTime; }
	void SetDefensiveBonus(uint32 ADefensiveBonus)
		{ _defensive_mode_bonus = ADefensiveBonus; }

	const uint32 GetXOrigin() const
		{ return _x_origin; }
	const uint32 GetYOrigin() const
		{ return _y_origin; }
	const float GetXLocation() const
		{ return _x_location; }
	const float GetYLocation() const
		{ return _y_location; }

	void SetXOrigin(uint32 x)
		{ _x_origin = x; }
	void SetYOrigin(uint32 y)
		{ _y_origin = y; }
	void SetXLocation(float x)
		{ _x_location = x; }
	void SetYLocation(float y)
		{ _y_location = y; }

	void SetTotalStrengthModifier(uint32 AStrengthModifier)
		{ _total_strength_modifier = AStrengthModifier; }
	void SetTotalIntelligenceModifier(uint32 AIntelligenceModifier)
		{ _total_intelligence_modifier = AIntelligenceModifier; }
	void SetTotalAgilityModifier(uint32 AAgilityModifier)
		{ _total_agility_modifier = AAgilityModifier; }

	uint32 GetTotalStrengthModifier()
		{ return _total_strength_modifier; }
	uint32 GetTotalAgilityModifier()
		{ return _total_agility_modifier; }
	uint32 GetTotalIntelligenceModifier()
		{ return _total_intelligence_modifier; }

	// The following accessor functions are meant for the inherited class to implement
	virtual const std::string GetName() const = 0;
	virtual const std::vector<hoa_global::GlobalAttackPoint*> GetAttackPoints() const = 0;
	virtual uint32 GetHealth() const = 0;
	virtual uint32 GetMaxHealth() const = 0;
	virtual uint32 GetSkillPoints() const = 0;
	virtual uint32 GetMaxSkillPoints() const = 0;
	virtual uint32 GetStrength() const = 0;
	virtual uint32 GetIntelligence() const = 0;
	virtual uint32 GetAgility() const = 0;
	virtual uint32 GetMovementSpeed() const = 0;

	virtual void SetHealth(uint32 hp) = 0;
	virtual void SetSkillPoints(uint32 sp) = 0;
	virtual void SetAnimation(std::string AAnimation) {}
	//@}

	void TEMP_Deal_Damage(uint32 damage);

protected:
	uint32 _TEMP_total_time_damaged;
	uint32 _TEMP_damage_dealt;

private:
	//! The mode we belong to
	BattleMode *_owner_battle_mode;
	//! The original X location of the actor
	uint32 _x_origin;
	//! The original Y location of the actor
	uint32 _y_origin;
	//! The X location of the actor on the battle grid
	float _x_location;
	//! The Y location of the actor on the battle grid
	float _y_location;
	//! A list of effects and ailments on the character
	std::deque<ActorEffect> _effects;
	//! The maximum stamina
	uint32 _max_skill_points;
	//! The remaining level of stamina
	uint32 _current_skill_points;
	//! Tells whether the character can move (frozen, burned, et cetera)
	bool _is_move_capable;
	//! Tells if the character is alive or dead
	bool _is_alive;
	//! Is the character attacking or queued to?
	bool _is_queued_to_perform;
	//! Are we warming up for the action?
	uint32 _warmup_time;
	//! Are we cooling down from an action?
	uint32 _cooldown_time;
	//! Do we have a defensive mode bonus?  how much?
	uint32 _defensive_mode_bonus;

	//! The sum of all modifiers from effects
	uint32 _total_strength_modifier;
	uint32 _total_agility_modifier;
	uint32 _total_intelligence_modifier;
}; // class BattleActor


class PlayerActor : public BattleActor {
public:
	PlayerActor(hoa_global::GlobalCharacter* const AWrapped, BattleMode* const ABattleMode, uint32 AXLoc, uint32 AYLoc);
	~PlayerActor();
	void Update(uint32 ATimeElapsed);
	void Draw();

	//! \brief PlayerActor class member access functions
	//@{
	std::vector<hoa_global::GlobalSkill*> GetAttackSkills() const
		{ return _wrapped_character->GetAttackSkills(); }
	std::vector<hoa_global::GlobalSkill*> GetDefenseSkills() const
		{ return _wrapped_character->GetDefenseSkills(); }
	std::vector<hoa_global::GlobalSkill*> GetSupportSkills() const
		{ return _wrapped_character->GetSupportSkills(); }
	const std::string GetName() const
		{ return _wrapped_character->GetName(); }
	const std::vector<hoa_global::GlobalAttackPoint*> GetAttackPoints() const
		{ return _wrapped_character->GetAttackPoints(); }
	uint32 GetHealth() const
		{ return _wrapped_character->GetHP(); }
	uint32 GetMaxHealth() const
		{ return _wrapped_character->GetMaxHP(); }
	uint32 GetSkillPoints() const
		{ return _wrapped_character->GetSP(); }
	uint32 GetMaxSkillPoints() const
		{ return _wrapped_character->GetMaxSP(); }
	uint32 GetStrength() const
		{ return _wrapped_character->GetStrength(); }
	uint32 GetIntelligence() const
		{ return _wrapped_character->GetIntelligence(); }
	uint32 GetAgility() const
		{ return _wrapped_character->GetAgility(); }
	uint32 GetMovementSpeed() const
		{ return _wrapped_character->GetMovementSpeed(); }

	void SetHealth(uint32 AHealth)
		{ _wrapped_character->SetHP(AHealth); }
	void SetSkillPoints(uint32 ASkillPoints)
		{ _wrapped_character->SetSP(ASkillPoints); }
	void SetAnimation(std::string ACurrentAnimation)
		{ _current_animation = _wrapped_character->GetAnimation(ACurrentAnimation); }
	//@}

private:
	//! The global character we have wrapped around
	hoa_global::GlobalCharacter * _wrapped_character;
	//! The current animation to draw
	hoa_video::AnimatedImage _current_animation;
}; // class PlayerActor

class EnemyActor : public BattleActor {
public:
	EnemyActor(hoa_global::GlobalEnemy AGlobalEnemy, BattleMode* const ABattleMode, uint32 AXLoc, uint32 AYLoc);
	~EnemyActor();
	void Update(uint32 ATimeElapsed);
	void Draw();

	//! \brief Has the GlobalEnemy level up to average_level
	void LevelUp(uint32 AAverageLevel);
	//! \todo
	void DoAI();

	//! \brief EnemyActor class member access functions
	//@{
	const std::vector<hoa_global::GlobalSkill*> GetSkills() const
		{ return _wrapped_enemy.GetSkills(); }
	const std::string GetName() const
		{ return _wrapped_enemy.GetName(); }
	const std::vector <hoa_global::GlobalAttackPoint*> GetAttackPoints() const
		{ return _wrapped_enemy.GetAttackPoints(); }
	uint32 GetHealth() const
		{ return _wrapped_enemy.GetHP(); }
	uint32 GetMaxHealth() const
		{ return _wrapped_enemy.GetMaxHP(); }
	uint32 GetSkillPoints() const
		{ return _wrapped_enemy.GetSP(); }
	uint32 GetMaxSkillPoints() const
		{ return _wrapped_enemy.GetMaxSP(); }
	uint32 GetStrength() const
		{ return _wrapped_enemy.GetStrength(); }
	uint32 GetIntelligence() const
		{ return _wrapped_enemy.GetIntelligence(); }
	uint32 GetAgility() const
		{ return _wrapped_enemy.GetAgility(); }
	uint32 GetMovementSpeed() const
		{ return _wrapped_enemy.GetMovementSpeed(); }

	void SetHealth(uint32 AHealth)
		{ _wrapped_enemy.SetHP(AHealth); }
	void SetSkillPoints(uint32 ASkillPoints)
		{ _wrapped_enemy.SetSP(ASkillPoints); }
	//@}
private:
	//! The enemy we have wrapped around
	hoa_global::GlobalEnemy _wrapped_enemy;
}; // class EnemyActor

} // namespace private_battle

} // namespace hoa_battle

#endif // __BATTLE_ACTORS_HEADER__


