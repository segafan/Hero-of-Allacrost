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
*** \author  Viljami Korhonen, mindflayer@allacrost.org
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

/** ****************************************************************************
*** \brief Represents postive and negative afflictions that affect actors in battle
***
*** \note This class is place holder code right now and is not used. It will be
*** fleshed out at a later time.
*** ***************************************************************************/
class BattleActorEffect {
public:
	BattleActorEffect(private_battle::BattleActor* const host)
		{}
	virtual ~BattleActorEffect()
		{}

	//! Updates the effect's timer and other status
	void Update()
		{}
	//! Removes the effect from its host
	void CureEffect() const
		{}

	//! \name BattleActorEffect class member access functionss
	//@{
	BattleActor* const GetHost() const
		{ return _host; }
	hoa_utils::ustring GetEffectName() const
		{ return _effect_name; }

	const int32 GetModifiedHealthPoints() const
		{ return _health_points_modifier; }
	const int32 GetModifiedSkillPoints() const
		{ return _skill_points_modifier; }
	const int32 GetModifiedStrength() const
		{ return _strength_modifier; }
	const int32 GetModifiedIntelligence() const
		{ return _intelligence_modifier; }
	const int32 GetModifiedAgility() const
		{ return _agility_modifier; }
	//@}

private:
	//! A pointer to the actor that the effect is afflicting
	private_battle::BattleActor* _host;
	//! The name of the effect
	hoa_utils::ustring _effect_name;
	//! The number of milliseconds remaining until the effect is removed
	uint32 _time_till_expiration;
	// //! Enum value to determine how severe the effect is
	// hoa_global::GLOBAL_AFFLICTION_SEVERITY _severity;

	/** \name Status modification members
	*** \brief Members which modifies various properties of the host's status
	**/
	//@{
	int32 _health_points_modifier;
	int32 _skill_points_modifier;
	int32 _strength_modifier;
	int32 _intelligence_modifier;
	int32 _agility_modifier;
	//@}
}; // class BattleActorEffect

/** ****************************************************************************
*** \brief Represents the generic entity for an object taking part in a battle
***
*** This is a virtual class that must be inherited
*** ***************************************************************************/
class BattleActor {
public:
	BattleActor(uint32 x, uint32 y);
	virtual ~BattleActor();

	virtual void Update() = 0;
	virtual void DrawSprite() = 0;

	//! Update the effects that the player is affected by
	void UpdateEffects();

	//! \name BattleActor class member access functions
	//@{
	const bool IsAlive() const
		{ return (GetHealthPoints() != 0); }
	const bool IsMoveCapable() const
		{ return _is_move_capable; }
	const bool IsQueuedToPerform() const
		{ return _is_queued_to_perform; }
	const bool IsWarmingUp() const
		{ return (_warmup_time != 0); }
	const bool IsInDefensiveMode() const
		{ return (_defensive_mode_bonus != 0); }

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
	virtual const hoa_utils::ustring GetName() const = 0;
	virtual const std::vector<hoa_global::GlobalAttackPoint*> GetAttackPoints() const = 0;
	virtual const uint32 GetHealthPoints() const = 0;
	virtual const uint32 GetMaxHealthPoints() const = 0;
	virtual const uint32 GetSkillPoints() const = 0;
	virtual const uint32 GetMaxSkillPoints() const = 0;
	virtual const uint32 GetStrength() const = 0;
	virtual const uint32 GetIntelligence() const = 0;
	virtual const uint32 GetAgility() const = 0;
	virtual const uint32 GetMovementSpeed() const = 0;

	virtual void SetHealthPoints(uint32 hp) = 0;
	virtual void SetSkillPoints(uint32 sp) = 0;
	virtual void SetAnimation(std::string animation) {}
	//@}

	void TEMP_Deal_Damage(uint32 damage);

protected:
	uint32 _TEMP_total_time_damaged;
	uint32 _TEMP_damage_dealt;

private:
	//! A unique identification number for the actor
	uint8 _actor_id;
	//! The "home" X location for the actor
	uint16 _x_origin;
	//! The "home" Y location for the actor
	uint16 _y_origin;
	//! The actor's current X location on the battle grid
	float _x_location;
	//! The actor's current Y location on the battle grid
	float _y_location;

	//! The maximum stamina
	uint32 _max_skill_points;
	//! The remaining level of stamina
	uint32 _current_skill_points;
	//! Tells whether the character can move (frozen, burned, et cetera)
	bool _is_move_capable;
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

//	//! A list of effects and ailments on the character
// 	std::deque<ActorEffect> _effects;
}; // class BattleActor

/** ****************************************************************************
*** \brief Represents the entity for a character in the battle
***
*** This class is a wrapper around a GlobalCharacter object. 
*** ***************************************************************************/
class CharacterActor : public BattleActor {
public:
	CharacterActor(hoa_global::GlobalCharacter* const AWrapped, uint32 AXLoc, uint32 AYLoc);
	~CharacterActor();

	//! Updates the state of the character
	void Update();

	//! Draws the character's current sprite animation frame
	void DrawSprite();
	//! Draws the character's damage-blended face portrait
	void DrawPortrait();
	//! Draws the character's status information
	void DrawStatus();

	void SetAnimation(std::string animation)
		{ _current_animation = _wrapped_character->GetAnimation(animation); }

	//! \name CharacterActor class member access functions
	//@{
	const hoa_utils::ustring GetName() const
		{ return _wrapped_character->GetName(); }
		
	const uint32 GetHealthPoints() const
		{ return _wrapped_character->GetHP(); }
	const uint32 GetMaxHealthPoints() const
		{ return _wrapped_character->GetMaxHP(); }
	const uint32 GetSkillPoints() const
		{ return _wrapped_character->GetSP(); }
	const uint32 GetMaxSkillPoints() const
		{ return _wrapped_character->GetMaxSP(); }
	void SetHealthPoints(uint32 hp)
		{ _wrapped_character->SetHP(hp); }
	void SetSkillPoints(uint32 sp)
		{ _wrapped_character->SetSP(sp); }
		
	const uint32 GetStrength() const
		{ return _wrapped_character->GetStrength(); }
	const uint32 GetIntelligence() const
		{ return _wrapped_character->GetIntelligence(); }
	const uint32 GetAgility() const
		{ return _wrapped_character->GetAgility(); }
	const uint32 GetMovementSpeed() const
		{ return _wrapped_character->GetMovementSpeed(); }

	std::vector<hoa_global::GlobalSkill*> GetAttackSkills() const
		{ return _wrapped_character->GetAttackSkills(); }
	std::vector<hoa_global::GlobalSkill*> GetDefenseSkills() const
		{ return _wrapped_character->GetDefenseSkills(); }
	std::vector<hoa_global::GlobalSkill*> GetSupportSkills() const
		{ return _wrapped_character->GetSupportSkills(); }
	const std::vector<hoa_global::GlobalAttackPoint*> GetAttackPoints() const
		{ return _wrapped_character->GetAttackPoints(); }
	//@}

private:
	//! The global character we have wrapped around
	hoa_global::GlobalCharacter* _wrapped_character;
	//! The current sprite animation to draw
	hoa_video::AnimatedImage _current_animation;
}; // class CharacterActor : public BattleActor


/** ****************************************************************************
*** \brief Represents the entity for an enemy in the battle
***
*** This class is a wrapper around a GlobalEnemy object.
*** ***************************************************************************/
class EnemyActor : public BattleActor {
public:
	EnemyActor(hoa_global::GlobalEnemy enemy, uint32 x, uint32 y);
	~EnemyActor();

	//! Updates the action status of the enemy
	void Update();

	//! Draws the damage-blended enemy sprite
	void DrawSprite();
	//! Draws the enemy's status information
	void DrawStatus();

	//! \brief EnemyActor class member access functions
	//@{

	const hoa_utils::ustring GetName() const
		{ return _wrapped_enemy.GetName(); }

	const uint32 GetHealthPoints() const
		{ return _wrapped_enemy.GetHP(); }
	const uint32 GetMaxHealthPoints() const
		{ return _wrapped_enemy.GetMaxHP(); }
	const uint32 GetSkillPoints() const
		{ return _wrapped_enemy.GetSP(); }
	const uint32 GetMaxSkillPoints() const
		{ return _wrapped_enemy.GetMaxSP(); }

	void SetHealthPoints(uint32 hp)
		{ _wrapped_enemy.SetHP(hp); }
	void SetSkillPoints(uint32 sp)
		{ _wrapped_enemy.SetSP(sp); }

	const uint32 GetStrength() const
		{ return _wrapped_enemy.GetStrength(); }
	const uint32 GetIntelligence() const
		{ return _wrapped_enemy.GetIntelligence(); }
	const uint32 GetAgility() const
		{ return _wrapped_enemy.GetAgility(); }
	const uint32 GetMovementSpeed() const
		{ return _wrapped_enemy.GetMovementSpeed(); }

	const std::vector<hoa_global::GlobalSkill*> GetSkills() const
		{ return _wrapped_enemy.GetSkills(); }
	const std::vector <hoa_global::GlobalAttackPoint*> GetAttackPoints() const
		{ return _wrapped_enemy.GetAttackPoints(); }
	//@}

private:
	//! The GlobalEnemy object that this object is wrapped around
	hoa_global::GlobalEnemy _wrapped_enemy;
}; // class EnemyActor : public BattleActor

} // namespace private_battle

} // namespace hoa_battle

#endif // __BATTLE_ACTORS_HEADER__


