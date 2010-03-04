////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
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

#include "defs.h"
#include "utils.h"

#include "global_skills.h"
#include "global_actors.h"
#include "global_effects.h"

#include "battle_utils.h"

namespace hoa_battle {

namespace private_battle {

/** ****************************************************************************
*** \brief An abstract class for representing an actor in the battle
***
*** An "actor" is a term used to represent both characters and enemies in battle.
*** This abstract class contains members and methods that are common to both types of
*** actors. As such, many of the implemented methods in this class are virtual.
***
*** The BattleActor class contains a pointer to a GlobalActor object that represents
*** the character or enemy. BattleActor contains its own members for all actor stats such
*** as HP, strength, evade rating, etc. There are two reasons why BattleActor uses its own
*** members instead of directly accessing and modifying the members of the GlobalActor pointer.
*** First, various effects can occur in battle which can modify otherwise static stats such as
*** agility. We need the ability to restore each stat to its base value, and the GlobalActor
*** class retains that unaltered value. Second, if the player loses the battle and chooses to
*** retry, we need to restore all actors back to their original state before the battle began.
*** Retreiving the values of the GlobalActor class allows us to do so.
***
*** Throughout the battle, actors progress through a series of states. The
*** standard set of states that an actor cylces through while they are "alive"
*** and participating in the battle are as follows.
***
*** -# ACTOR_STATE_IDLE
*** -# ACTOR_STATE_COMMAND
*** -# ACTOR_STATE_WARM_UP
*** -# ACTOR_STATE_READY
*** -# ACTOR_STATE_ACTING
*** -# ACTOR_STATE_COOL_DOWN
***
*** Throughout each cycle, the actor will select or be given an action to execute.
*** This action may be to attack an actor on the opposing team, heal a teammate,
*** use an item, or perform some sort of skill. Each actor is responsible for the
*** management of the action that they intend to take.
*** ***************************************************************************/
class BattleActor : public hoa_global::GlobalActor {
public:
	BattleActor(hoa_global::GlobalActor* actor);

	virtual ~BattleActor();

	//! \brief Returns true if the actor is considered an enemy of the character party
	virtual bool IsEnemy() const = 0;

	//! \brief Returns true so long as the actor is not in the "dead" state
	bool IsAlive() const
		{ return (_state != ACTOR_STATE_DEAD); }

	//! \brief Empty method. Required because this is a pure virtual method of GlobalActor
	void AddSkill(uint32 skill_id)
		{}

	//! \brief Adds a battle effect to the actor's vector
	void AddEffect(hoa_global::GlobalStatusEffect* new_effect);

	//! \brief Adds a battle effect to the actor's vector
	void AddNewEffect(uint32 id);

	/** \brief Changes the state of the actor and modifies the actor's properties accordingly
	*** \param new_state The state to set the actor to
	**/
	virtual void ChangeState(ACTOR_STATE new_state);

	/** \brief Deals damage to the actor by reducing its hit points by a certain amount
	*** \param amount The number of hit points to decrease on the actor
	***
	*** If the state of the actor is ACTOR_STATE_DEAD, this function will print a warning and change nothing.
	*** If the amount of damage dealt is greater than the actor's current hit points, the actor will be placed
	*** in the ACTOR_STATE_DEAD state.
	**/
	void RegisterDamage(uint32 amount);

	/** \brief Heals the actor by restoring a certain amount of hit points
	*** \param amount The number of hit points to add to the actor
	***
	*** If the state of the actor is ACTOR_STATE_DEAD, this function will print a warning and change nothing.
	*** The number of hit points on the actor are not allowed to increase beyond the actor's maximum hit
	*** points.
	**/
	void RegisterHealing(uint32 amount);

	//! \brief Indicates that an action failed to connect on this target
	void RegisterMiss();

	/** \brief Increases or decreases the current skill points of the actor
	*** \param amount The number of skill points to increase or decrease
	***
	*** If the actor is dead, no change will take place. If the amount is positive, the actor will
	*** not be allowed to exceed above their maximum skill points.
	***
	*** Any non-zero change in skill points will be reflected via increase/decrease text that will
	*** be drawn to the screen near the actor's sprite. If the value of the amount argument is zero,
	*** the word "Miss" will be drawn instead;
	**/
	void ChangeSkillPoints(int32 amount);

	//! \brief Updates the state of the actor
	virtual void Update();

	//! \brief Draws the actor's current sprite animation frame
	virtual void DrawSprite() = 0;

	//! \brief Draws all active indicator text and graphics for the actor
	void DrawIndicators() const;

	/** \brief Sets the action that the actor should execute next
	*** \param action A pointer to the action that the actor should execute
	***
	*** The actor assumes responsibility for the memory management of the action that is given to it with
	*** this method and will delete the object at the appropriate time. You should only call the method
	*** when the actor is in the state ACTOR_STATE_COMMAND. Invoking it at any other time will result in a
	*** warning and no operation, and the action object will be deleted immediately. A warning is also
	*** printed in the case where the actor has another action prepared.
	**/
	void SetAction(BattleAction* action);

	//! \brief Resets actor stats to their original values
	//@{
	void ResetHitPoints(uint32 hp)
		{ SetHitPoints(_global_actor->GetHitPoints()); }

	void ResetMaxHitPoints(uint32 hp)
		{ SetMaxHitPoints(_global_actor->GetMaxHitPoints()); }

	void ResetSkillPoints(uint32 sp)
		{ SetSkillPoints(_global_actor->GetSkillPoints()); }

	void ResetMaxSkillPoints(uint32 sp)
		{ SetMaxSkillPoints(_global_actor->GetMaxSkillPoints()); }

	void ResetStrength(uint32 st)
		{ SetStrength(_global_actor->GetStrength()); }

	void ResetVigor(uint32 vi)
		{ SetVigor(_global_actor->GetVigor()); }

	void ResetFortitude(uint32 fo)
		{ SetFortitude(_global_actor->GetFortitude()); }

	void ResetProtection(uint32 pr)
		{ SetProtection(_global_actor->GetProtection()); }

	void ResetAgility(uint32 ag)
		{ SetAgility(_global_actor->GetAgility()); }

	void ResetEvade(float ev)
		{ SetEvade(_global_actor->GetEvade()); }
	//@}

	//! \brief Returns the average defense/evasion totals of all of the actor's attack points
	//@{
	uint32 TotalPhysicalDefense();

	uint32 TotalMetaphysicalDefense();

	float TotalEvadeRating();
	//@}

	//! \name Class member access methods
	//@{
	ACTOR_STATE GetState() const
		{ return _state; }

	hoa_global::GlobalActor* GetGlobalActor()
		{ return _global_actor; }

	BattleAction* GetAction()
		{ return _action; }

	float GetXOrigin() const
		{ return _x_origin; }

	float GetYOrigin() const
		{ return _y_origin; }

	float GetXLocation() const
		{ return _x_location; }

	float GetYLocation() const
		{ return _y_location; }

	uint32 GetIdleStateTime() const
		{ return _idle_state_time; }

	hoa_video::StillImage& GetStaminaIcon()
		{ return _stamina_icon; }

	std::vector<hoa_global::GlobalStatusEffect*> GetActorEffects()
		{ return _actor_effects; }

	hoa_system::SystemTimer& GetStateTimer()
		{ return _state_timer; }

	void SetXOrigin(float x_origin)
		{ _x_origin = x_origin; }

	void SetYOrigin(float y_origin)
		{ _y_origin = y_origin; }

	void SetXLocation(float x_location)
		{ _x_location = x_location; }

	void SetYLocation(float y_location)
		{ _y_location = y_location; }

	//! \note If the actor is in the idle state, this will not affect the state timer
	 void SetIdleStateTime(uint32 time)
		{ _idle_state_time = time; }
	//@}

protected:
	//! \brief The state that the actor is currently in
	ACTOR_STATE _state;

	//! \brief A pointer to the global actor object which the battle actor represents
	hoa_global::GlobalActor* _global_actor;

	//! \brief A pointer to the action that the actor is preparing to perform or is currently performing
	BattleAction* _action;

	//! \brief The "home" coordinates for the actor's default location on the battle field
	float _x_origin, _y_origin;

	//! \brief The x and y coordinates of the actor's current location on the battle field
	float _x_location, _y_location;

	//! \brief Set to true when the actor is in the ACTING state and the execution of the action is complete
	bool _execution_finished;

	//! \brief The amount of time (in milliseconds) that the actor needs to wait to pass through the idle state
	uint32 _idle_state_time;

	//! \brief A timer used as the character progresses through the standard series of actor states
	hoa_system::SystemTimer _state_timer;

	//! \brief The actor's icon for the stamina meter
	hoa_video::StillImage _stamina_icon;

	//! \brief Contains this actor's battle effects
	std::vector<hoa_global::GlobalStatusEffect*> _actor_effects;

	//! \brief An assistant class to the actor that manages all the actor's indicator text and graphics
	IndicatorSupervisor* _indicator_supervisor;
}; // class BattleActor


/** ****************************************************************************
*** \brief Represents a player-controlled character in the battle
***
*** Character actors have a series of animated images that reflect their current
*** state and actions. Each character also has a custom set of progressive damage
*** battle portraits (5 in total) that are drawn when the character is selected.
*** ***************************************************************************/
class BattleCharacter : public BattleActor {
public:
	BattleCharacter(hoa_global::GlobalCharacter* character);

	~BattleCharacter();

	bool IsEnemy() const
		{ return false; }

	void ChangeState(ACTOR_STATE new_state);

	/** \brief Changes the actor's current sprite animation image
	*** \param alias The alias text used to identify the animation to change
	**/
	void ChangeSpriteAnimation(const std::string& alias);

	//! \brief Updates the state of the character. Must be called every frame!
	void Update();

	//! \brief Draws the character's current sprite animation frame
	void DrawSprite();

	//! \brief Draws the character's damage-blended face portrait
	void DrawPortrait();

	/** \brief Draws the character's status in the bottom area of the screen
	*** \param order The order position of the character [0-3] used to determine draw positions
	**/
	void DrawStatus(uint32 order);

	hoa_global::GlobalCharacter* GetGlobalCharacter()
		{ return _global_character; }

	const std::string& GetSpriteAnimationAlias() const
		{ return _sprite_animation_alias; }

protected:
	//! \brief A pointer to the global character object which the battle character represents
	hoa_global::GlobalCharacter* _global_character;

	//! \brief Contains the identifier text of the current sprite animation
	std::string _sprite_animation_alias;

	//! \brief Contains countdown timer of current animation
	hoa_system::SystemTimer _animation_timer;

	//! \brief Rendered text of the character's name
	hoa_video::TextImage _name_text;

	//! \brief Rendered text of the character's current hit points
	hoa_video::TextImage _hit_points_text;

	//! \brief Rendered text of the character's current skill points
	hoa_video::TextImage _skill_points_text;
}; // class BattleCharacter


/** ****************************************************************************
*** \brief Represents the entity for an enemy in the battle
***
*** This class is a wrapper around a GlobalEnemy object.
*** ***************************************************************************/
class BattleEnemy : public BattleActor {
public:
	BattleEnemy(hoa_global::GlobalEnemy* enemy);

	~BattleEnemy();

	bool IsEnemy() const
		{ return true; }

	void ChangeState(ACTOR_STATE new_state);

	void Update();

	//! \brief Draws the damage blended enemy sprite image on to the battle field
	void DrawSprite();

	hoa_global::GlobalEnemy* GetGlobalEnemy()
		{ return _global_enemy; }

	//! \brief Compares the Y-coordinates of the actors, used for sorting the actors up-down when drawing
	bool operator<(const BattleEnemy & other) const;

protected:
	//! \brief A pointer to the global enemy object which the battle enemy represents
	hoa_global::GlobalEnemy* _global_enemy;

	/** \brief Decides what action that the enemy should execute and the target
	*** \todo This function is extremely rudimentary right now. Later, it should be given a more complete
	*** AI decision making algorithm
	**/
	void _DecideAction();
}; // class BattleEnemy

} // namespace private_battle

} // namespace hoa_battle

#endif // __BATTLE_ACTORS_HEADER__
