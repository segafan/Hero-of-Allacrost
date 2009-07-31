////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
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

#include "global_skills.h"
#include "global_actors.h"
#include "global_effects.h"

namespace hoa_battle {

namespace private_battle {

//! \brief Represents the possible states that a BattleActor may be in
enum ACTOR_STATE {
	ACTOR_INVALID       = -1,
	//! Actor is recovering stamina so they can execute another action
	ACTOR_IDLE          =  0,
	ACTOR_AWAITING_TURN =  1,
	//! Actor has selected an action and is preparing to execute it
	ACTOR_WARM_UP       =  2,
	//! Actor is prepared to execute action and is waiting their turn to act
	ACTOR_READY         =  3,
	//! Actor is in the process of executing their selected action
	ACTOR_ACTING        =  4,
	//! Actor is finished with action execution and recovering
	ACTOR_COOL_DOWN     =  5,
	//! Actor has perished and is inactive in battle
	ACTOR_DEAD          =  6,
	//! Actor is in some state of paralysis and can not act nor recover stamina
	ACTOR_PARALYZED     =  7,
	ACTOR_TOTAL         =  8
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
*** \brief An abstract class for representing an acting entity in the battle
***
*** ***************************************************************************/
class BattleActor {
public:
	BattleActor(hoa_global::GlobalActor* actor, float x_origin, float y_origin);

	virtual ~BattleActor();

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

	//! \brief Sets actor's current sprite animation type
	virtual void PlayAnimation(std::string alias) = 0;

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

	virtual uint32 GetCombatAgility();

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

	std::vector<hoa_global::GlobalStatusEffect*> GetActorEffects()
		{ return _actor_effects; }

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

	//! \brief Adds a battle effect to the actor's vector
	void AddEffect(hoa_global::GlobalStatusEffect* new_effect);

	//! \brief Adds a battle effect to the actor's vector
	void AddNewEffect(int id);

	//! \brief Affect stat modifiers
	void AddHitPoints(int32 hp);
	void AddStrength(int32 str)     {_actor->AddStrength(str);}
	void AddFortitude(int32 frt)    {_actor->AddFortitude(frt);}
	void AddAgility(int32 agi)      {_actor->AddAgility(agi);}
	void AddVigor(int32 vig)        {_actor->AddVigor(vig);}

	virtual std::string GetAnimationString() = 0;

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

	//! \brief The actor's icon for the stamina meter
	hoa_video::StillImage _stamina_icon;

	//! \brief Amount of time character spends in the idle phase
	//FIX ME for now, will also be used for cool down times?
	hoa_system::SystemTimer _wait_time;

	//! \brief Recalculates wait time if agility has changed	
	void _RecalculateWaitTime();

	//! \brief Contains this actor's battle effects
	std::vector<hoa_global::GlobalStatusEffect*> _actor_effects;
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

	//! \brief Sets character's current sprite animation type
	void PlayAnimation(std::string alias);

	//! \brief Draws the character's damage-blended face portrait
	void DrawPortrait();

	//! \brief Draws the character's status in the bottom area of the screen
	void DrawStatus();

	hoa_global::GlobalCharacter* GetActor()
		{ return dynamic_cast<hoa_global::GlobalCharacter*>(_actor); }

	virtual std::string GetAnimationString()
		{ return _animation_string; }

protected:
	//! \brief Contains alias of current animation
	std::string _animation_string;

	//! \brief Contains countdown timer of current animation
	hoa_system::SystemTimer _animation_time;
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

	//! \brief Sets actor's current sprite animation type
	void PlayAnimation(std::string alias);

	hoa_global::GlobalEnemy* GetActor()
		{ return dynamic_cast<hoa_global::GlobalEnemy*>(_actor); }

	//! \brief Compares the Y-coordinates of the actors, used for sorting the actors up-down when drawing
	bool operator<(const BattleEnemy & other) const;

	virtual std::string GetAnimationString()
		{ return _animation_string; }

protected:
	//! \brief Contains alias of current animation
	std::string _animation_string;

	//! \brief Contains countdown timer of current animation
	hoa_system::SystemTimer _animation_time;

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
