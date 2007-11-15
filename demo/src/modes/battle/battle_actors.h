////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_actors.h
*** \author  Viljami Korhonen, mindflayer@allacrost.org
*** \author  Corey Hoffstein, visage@allacrost.org
*** \author  Andy Gardner, chopperdave@allacrost.org
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
#include "global_actors.h"
#include "video.h"
#include "audio.h"

namespace hoa_battle {

namespace private_battle {

//! \brief Represents the possible states that a BattleActor may be in
enum ACTOR_STATE {
	ACTOR_INVALID    = -1,
	//! Actor is recovering stamina so they can execute another action
	ACTOR_IDLE       =  0,
	//! Actor has selected an action and is preparing to execute it
	ACTOR_WARM_UP    =  1,
	//! Actor is prepared to execute action and is waiting their turn to act
	ACTOR_READY      =  2,
	//! Actor is in the process of executing their selected action
	ACTOR_ACTING     =  3,
	//! Actor is finished with action execution and recovering
	ACTOR_COOL_DOWN  =  4,
	//! Actor has perished and is inactive in battle
	ACTOR_DEAD       =  5,
	//! Actor is in some state of paralysis and can not act nor recover stamina
	ACTOR_PARALYZED  =  6,
	ACTOR_TOTAL      =  7
};

//! \brief Constants for the significant locations along the stamina meter
//@{
//! The bottom of the stamina bar
const float STAMINA_LOCATION_BOTTOM = 128.0f;
//! The yellow/orange transition area where the actor can select an action
const float STAMINA_LOCATION_SELECT = STAMINA_LOCATION_BOTTOM + 300.0f;
//! The orange/red transition area where the actor is warmed up and ready to execute
const float STAMINA_LOCATION_READY = STAMINA_LOCATION_SELECT + 100.0f;
//! The top of the stamina bar
const float STAMINA_LOCATION_TOP    = STAMINA_LOCATION_SELECT + 100.0f;
//@}

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

	//! \brief Updates the effect's timer and other status
	void Update()
		{}

	//! \brief Removes the effect from its host
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

	const int32 GetModifiedAgility() const
		{ return _agility_modifier; }
	//@}

private:
	//! \brief A pointer to the actor that the effect is afflicting
	private_battle::BattleActor* _host;

	//! \brief The name of the effect
	hoa_utils::ustring _effect_name;

	//! \brief The number of milliseconds remaining until the effect is removed
	uint32 _time_till_expiration;

	//! \brief Enum value to determine the strength of the effect
	hoa_global::GLOBAL_INTENSITY _intensity;

	/** \name Status modification members
	*** \brief Members which modifies various properties of the host's status
	**/
	//@{
	int32 _health_points_modifier;
	int32 _skill_points_modifier;
	int32 _strength_modifier;
	int32 _agility_modifier;
	//@}
}; // class BattleActorEffect


/** ****************************************************************************
*** \brief An abstract class for representing an acting entity in the battle
***
*** ***************************************************************************/
class BattleActor {
public:
	BattleActor(hoa_global::GlobalActor* actor, float x_origin, float y_origin);

	virtual ~BattleActor()
		{}

	/** \brief Deals a specific amount of damage to the actor
	*** \param damage The amount of damage to deal
	*** \todo Eventually should be made defunct or replaced with a better version
	**/
	virtual void TakeDamage(int32 damage);

	//! \brief Resets the wait time and time meter portrait
	void ResetWaitTime();

	//! \brief Returns true if the actor is considered an enemy of the character party
	virtual bool IsEnemy() = 0;

	/** \brief Constructs a string with the actor's vital information
	*** \param info A reference to the string which should hold the information
	*** \param ap_index The attack point to include information on, set to -1 if no AP info is needed
	*** This information is used by the ActionWindow class to display target information
	*** in the window.
	**/
	virtual void ConstructInformation(hoa_utils::ustring& info, int32 ap_index);

	//! \brief Draws the actor's current sprite animation frame
	virtual void DrawSprite() = 0;

	/** \brief Draws the actor's stamina icon at the appropriate location
	*** \param is_selected If true, the stamina icon will be drawn highlighted
	**/
	void DrawStaminaIcon(bool is_selected = false);

	bool IsAlive() const
		{ return (_state != ACTOR_DEAD); }

	//! \name Class member access functions
	//@{
	virtual hoa_global::GlobalActor* GetActor()
		{ return _actor; }

	virtual uint32 GetPhysicalAttack();

	virtual uint32 GetPhysicalDefense();

	virtual float GetCombatEvade();

	private_battle::ACTOR_STATE GetState() const
		{ return _state; }

	uint32 GetIndex() const
		{ return _index; }
	
	float GetXOrigin() const
		{ return _x_origin; }

	float GetYOrigin() const
		{ return _y_origin; }

	float GetXLocation() const
		{ return _x_location; }

	float GetYLocation() const
		{ return _y_location; }

	float GetStaminaIconLocation()
		{ return _stamina_icon_location; }

	hoa_system::SystemTimer* GetWaitTime()
		{ return &_wait_time; }

	void SetState(private_battle::ACTOR_STATE state)
		{ _state = state; }

	void SetIndex(uint32 index)
		{ _index = index; }

	void SetXOrigin(float x_origin)
		{ _x_origin = x_origin; }

	void SetYOrigin(float y_origin)
		{ _y_origin = y_origin; }

	void SetXLocation(float x_location)
		{ _x_location = x_location; }

	void SetYLocation(float y_location)
		{ _y_location = y_location; }

	void SetStaminaIconLocation(float location)
		{ _stamina_icon_location = location; }
	//@}

	//! \brief Restores some the given hp to the actor
	void BattleActor::AddHitPoints(int32 hp);

	//! \brief Resets the attack timer for the animation
	void TEMP_ResetAttackTimer();

protected:
	//! \brief A pointer to the global actor object which the battle actor represents
	hoa_global::GlobalActor* _actor;

	//! \brief The state that the actor is currently in
	ACTOR_STATE _state;

	//! \brief The numeric index of where the actor is stored in its BattleMode container
	uint32 _index;

	//! \brief The last time the actor was updated
	uint32 _last_update_time;

	//! \brief The "home" coordinates for the actor's default location on the battle field
	float _x_origin, _y_origin;

	//! \brief The x and y coordinates of the actor's current location on the battle field
	float _x_location, _y_location;

	//! \brief The y-value of it's location, since x is fixed
	float _stamina_icon_location;

	//! \brief Variable for tracking time (ms) on how long to show the damage text
	//FIX ME this has to go
	uint32 _total_time_damaged;

	//! \brief How much damage was dealt on the last strike
	//FIX ME this has to go unless we have good cause for it
	uint32 _damage_dealt;

	//! \brief The actor's icon for the stamina meter
	hoa_video::StillImage _stamina_icon;

	//! \name Actor timers
	//@{
	//! \brief Amount of time character spends in the idle phase
	//FIX ME for now, will also be used for cool down times?
	hoa_system::SystemTimer _wait_time;

	//! \brief Timer for the attack animation
	hoa_system::SystemTimer _TEMP_attack_animation_timer;
	//@}

	//! \brief Recalculates wait time if agility has changed	
	void _RecalculateWaitTime();
}; // class BattleActor


/** ****************************************************************************
*** \brief Represents a player-controlled character in the battle
*** ***************************************************************************/
class BattleCharacter : public BattleActor {
public:
	BattleCharacter(hoa_global::GlobalCharacter* character, float x_origin, float y_origin);

	~BattleCharacter();

	bool IsEnemy()
		{ return false; }

	//! \brief Updates the state of the character. Must be called every frame!
	void Update();

	//! \brief Draws the character's current sprite animation frame
	void DrawSprite();

	//! \brief Draws the character's damage-blended face portrait
	void DrawPortrait();

	//! \brief Draws the character's status in the bottom area of the screen
	void DrawStatus();

	hoa_global::GlobalCharacter* GetActor()
		{ return dynamic_cast<hoa_global::GlobalCharacter*>(_actor); }
}; // class BattleCharacter


/** ****************************************************************************
*** \brief Represents the entity for an enemy in the battle
***
*** This class is a wrapper around a GlobalEnemy object.
*** ***************************************************************************/
class BattleEnemy : public BattleActor {
public:
	BattleEnemy(hoa_global::GlobalEnemy* enemy, float x_origin, float y_origin);

	~BattleEnemy();

	bool IsEnemy()
		{ return true; }

	//! \brief Updates the action status of the enemy
	void Update();

	//! \brief Draws the damage-blended enemy sprite on the battle field
	void DrawSprite();

	hoa_global::GlobalEnemy* GetActor()
		{ return dynamic_cast<hoa_global::GlobalEnemy*>(_actor); }

	//! \brief Compares the Y-coordinates of the actors, used for sorting the actors up-down when drawing
	bool operator<(const BattleEnemy & other) const;

private:
	/** \brief Decides what action that the enemy should execute and the target
	*** \todo This function is extremely rudimentary right now. Later, it should be given a more complete
	*** AI decision making algorithm
	**/
	void _DecideAction();
}; // class BattleEnemy

} // namespace private_battle

} // namespace hoa_battle

#endif // __BATTLE_ACTORS_HEADER__
