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
#include "battle.h"

namespace hoa_battle {

namespace private_battle {

// FIX ME
/** \brief Temporary enum used to determine which phase of the time meter a character/enemy
*** is in.
**/
/*enum TEMP_ACTION_STATE {
	ACTION_IDLE = 0,
	ACTION_WARM_UP = 1,
	ACTION_COOL_DOWN = 2
};*/

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
*** \brief This is an interface class for all the battle actors (eg. the player and the monsters)
***
*** \note This class is used because we no longer need try-catch blocks when converting
*** pointers from GlobalActor to either BattleCharacterActor or BattleEnemyActor.
*** Of course it helps to keep things a bit more generic and maintainable as well. ;)
*** ***************************************************************************/
class IBattleActor
{
public:
	IBattleActor() {}
	virtual ~IBattleActor() {}

	//! Gives a specific amount of damage for the object
	virtual void TakeDamage(uint32 damage) = 0;

	//! Sets queued to perform
	virtual void SetQueuedToPerform(bool QueuedToPerform) = 0;

	//! Gets a pointer to the GlobalActor
	virtual hoa_global::GlobalActor * GetActor() = 0;

	// \brief Resets the wait time and time meter portrait
	virtual void ResetWaitTime() = 0;

	// \brief Sets the location of the time meter portrait
	// \param new_val new value for the location
	virtual void SetTimePortraitLocation(float new_val) = 0;

	// \brief Gets the location of the time meter portrait
	// \return The location of the time portrait
	virtual float GetTimePortraitLocation() = 0;

	// \brief Gets the wait time
	// \return the wait time
	virtual hoa_system::Timer* GetWaitTime() = 0;
};


/** ****************************************************************************
*** \brief Represents the player-controlled character in the battle
*** ***************************************************************************/
class BattleCharacterActor : public hoa_global::GlobalCharacter, public IBattleActor {
public:
	BattleCharacterActor(hoa_global::GlobalCharacter * character, float x_location, float y_location);

	virtual ~BattleCharacterActor();

	//! Updates the state of the character. Must be called every frame!
	void Update();

	//! Draws the character's current sprite animation frame
	void DrawSprite();

	//! Draws the character's damage-blended face portrait
	void DrawPortrait();

	// \brief Draws the character's portrait for the time meter
	// \param is_selected If the enemy is selected for an action, highlight it
	void DrawTimePortrait(bool is_selected);

	//! Draws the character's status information
	void DrawStatus();

	//! Gives a specific amount of damage for the character
	void TakeDamage(uint32 damage);

	//! Is the character queued to attack?
	bool IsQueuedToPerform() const
		{ return _is_queued_to_perform; }

	// Sets queued to perform
	void SetQueuedToPerform(bool QueuedToPerform)
	{ _is_queued_to_perform = QueuedToPerform; }

	//! \name Getters and setters for the current and the original coordinates
	//@{
	float GetXLocation() const
		{ return _x_location; }

	float GetYLocation() const
		{ return _y_location; }

	float GetXOrigin() const
		{ return _x_origin; }

	float GetYOrigin() const
		{ return _y_origin; }

	void SetXLocation(float x_location)
		{ _x_location = x_location; }

	void SetYLocation(float y_location)
		{ _y_location = y_location; }

	void SetXOrigin(float x_origin)
		{ _x_origin = x_origin; }

	void SetYOrigin(float y_origin)
		{ _y_origin = y_origin; }
	//@}

	// Gets a pointer to the GlobalActor
	virtual hoa_global::GlobalCharacter * GetActor()
		{ return global_character_; }

	// \brief Sets the location of the time meter portrait
	// \param new_val new value for the location
	virtual void SetTimePortraitLocation(float new_val) { _time_portrait_location = new_val; }

	// \brief Gets the location of the time meter portrait
	// \return The location of the time portrait
	virtual float GetTimePortraitLocation() { return _time_portrait_location; }

	// \brief Gets the wait time
	// \return the wait time
	virtual hoa_system::Timer* GetWaitTime() { return &_wait_time; }

	// \brief Resets the wait time and time meter portrait
	void ResetWaitTime();
	//inline void SetWaitTime(uint32 wait_time) { _wait_time = wait_time; }
	
private:
	//! A Pointer to the 'real' GlobalCharacter. TODO: This is very bad design and probably should be fixed...
	hoa_global::GlobalCharacter * global_character_;

	//! Character's X-coordinate on the screen
	float _x_location;

	//! Character's Y-coordinate on the screen
	float _y_location;

	//! Starting X coordinate of the character
	float _x_origin;

	//! Starting Y coordinate of the character
	float _y_origin;

	//! Variable for tracking time (ms) on how long to show the damage text
	//FIX ME this has to go
	uint32 _total_time_damaged;

	//! How much damage was dealt on the last strike
	//FIX ME this has to go unless we have good cause for it
	uint32 _damage_dealt;

	//! Is the character queued to attack?
	bool _is_queued_to_perform;

	//! Portrait for the time meter
	hoa_video::StillImage _time_meter_portrait;

	//! The image used to highlight time portraits for selected actors
	hoa_video::StillImage _time_portrait_selected;

	//! Image of the cover of the bottom menu shrinking bar
	hoa_video::StillImage _status_bar_cover_image;

	//! Image of the menu status
	hoa_video::StillImage _status_menu_image;

	//! The y-value of it's location, since x is fixed
	float _time_portrait_location;

	//! Amount of time character spends in the idle phase
	//FIX ME for now, will also be used for cool down times?
	hoa_system::Timer _wait_time;

	//! Which action state the char is in for the time meter.  TEMPORARY!!!
	//uint8 _action_state;

}; // end BattleCharacterActor


/** ****************************************************************************
*** \brief Represents the entity for an enemy in the battle
***
*** This class is a wrapper around a GlobalEnemy object.
*** ***************************************************************************/
class BattleEnemyActor : public hoa_global::GlobalEnemy, public IBattleActor {
public:
	BattleEnemyActor(GlobalEnemy enemy, float x_location, float y_location);

	virtual ~BattleEnemyActor();

	//! Updates the action status of the enemy
	void Update();

	//! Draws the damage-blended enemy sprite
	void DrawSprite();

	// \brief Draws the enemy's portrait for the time meter
	// \param is_selected If the enemy is selected for an action, highlight it
	void DrawTimePortrait(bool is_selected);

	//! Draws the enemy's status information
	void DrawStatus();

	//! Gives a specific amount of damage for the enemy
	void TakeDamage(uint32 damage);

	//! Is the monster attacking right now
	bool IsAttacking() const;

	//! Is the enemy queued to attack?
	bool IsQueuedToPerform() const
		{ return _is_queued_to_perform; }

	// Sets queued to perform
	void SetQueuedToPerform(bool QueuedToPerform)
		{ _is_queued_to_perform = QueuedToPerform; }

	//! \name Getters and setters for the current and the original coordinates
	//@{
	float GetXLocation() const
		{ return _x_location; }

	float GetYLocation() const
		{ return _y_location; }

	float GetXOrigin() const
		{ return _x_origin; }

	float GetYOrigin() const
		{ return _y_origin; }

	void SetXLocation(float x_location)
		{ _x_location = x_location; }

	void SetYLocation(float y_location)
		{ _y_location = y_location; }

	void SetXOrigin(float x_origin)
		{ _x_origin = x_origin; }

	void SetYOrigin(float y_origin)
		{ _y_origin = y_origin; }
	//@}

	//! Gets a pointer to the GlobalActor
	virtual hoa_global::GlobalActor* GetActor()
		{ return this; }

	//! \brief Sets the location of the time meter portrait
	//! \param new_val new value for the location
	virtual void SetTimePortraitLocation(float new_val) { _time_portrait_location = new_val; }

	//! \brief Gets the location of the time meter portrait
	//! \return The location of the time portrait
	virtual float GetTimePortraitLocation() { return _time_portrait_location; }

	//! \brief Gets the wait time
	//! \return the wait time
	virtual hoa_system::Timer* GetWaitTime() { return &_wait_time; }

	//! \brief Resets the wait time and time meter portrait
	void ResetWaitTime();
	//inline void SetWaitTime(uint32 wait_time) { _wait_time = wait_time; }

	//! \brief Compares the Y-coordinates of the actors, used for sorting the actors up-down when drawing
	bool operator<(const BattleEnemyActor & other) const;

private:
	//! Enemy's X-coordinate on the screen
	float _x_location;

	//! Enemy's Y-coordinate on the screen
	float _y_location;

	//! Starting X coordinate of the enemy
	float _x_origin;

	//! Starting Y coordinate of the enemy
	float _y_origin;

	//! Variable for tracking time (ms) on how long to show the damage text
	//FIX ME this has to go
	uint32 _total_time_damaged;

	//! How much damage was dealt on the last strike
	//FIX ME this has to go unless we have good cause for it
	uint32 _damage_dealt;

	//! Is the enemy queued to attack?
	bool _is_queued_to_perform;

	//! Portrait for the time meter
	hoa_video::StillImage _time_meter_portrait;

	//! The image used to highlight time portraits for selected actors
	hoa_video::StillImage _time_portrait_selected;

	//! The y-value of it's location, since x is fixed
	float _time_portrait_location;

	//! Amount of time enemy spends in the idle phase
	//FIX ME for now, will also be used for cool down times?
	hoa_system::Timer _wait_time;

	//! Which action state the char is in for the time meter.  TEMPORARY!!!
	//uint8 _action_state;

}; // end BattleEnemyActor

} // namespace private_battle

} // namespace hoa_battle

#endif // __BATTLE_ACTORS_HEADER__


