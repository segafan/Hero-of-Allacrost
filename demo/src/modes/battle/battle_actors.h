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
*** \author  Viljami Korhonen, mindflayer@allacrost.org
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
#include "global_actors.h"
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
*** \brief Represents the player-controlled character in the battle
*** ***************************************************************************/
class BattleCharacterActor : public hoa_global::GlobalCharacter {
public:
	BattleCharacterActor(const hoa_utils::ustring & name, const std::string & filename, uint32 id, float XLocation, float YLocation);

	virtual ~BattleCharacterActor();

	//! Updates the state of the character. Must be called every frame!
	void Update();

	//! Draws the character's current sprite animation frame
	void DrawSprite();

	//! Draws the character's damage-blended face portrait
	void DrawPortrait();

	//! Draws the character's status information
	void DrawStatus();

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

	
private:
	//! Character's X-coordinate on the screen
	float _x_location;

	//! Character's Y-coordinate on the screen
	float _y_location;

	//! Starting X coordinate of the character
	float _x_origin;

	//! Starting Y coordinate of the character
	float _y_origin;

	//! Variable for tracking time (ms) on how long to show the damage text
	uint32 _total_time_damaged;

	//! How much damage was dealt on the last strike
	uint32 _damage_dealt;

	//! Is the character queued to attack?
	bool _is_queued_to_perform;
}; // end BattleCharacterActor


/** ****************************************************************************
*** \brief Represents the entity for an enemy in the battle
***
*** This class is a wrapper around a GlobalEnemy object.
*** ***************************************************************************/
class BattleEnemyActor : public hoa_global::GlobalEnemy {
public:
	BattleEnemyActor(const std::string & file_name, float XLocation, float YLocation);

	virtual ~BattleEnemyActor();

	//! Updates the action status of the enemy
	void Update();

	//! Draws the damage-blended enemy sprite
	void DrawSprite();

	//! Draws the enemy's status information
	void DrawStatus();

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
	uint32 _total_time_damaged;

	//! How much damage was dealt on the last strike
	uint32 _damage_dealt;

	//! Is the enemy queued to attack?
	bool _is_queued_to_perform;
}; // end BattleEnemyActor

} // namespace private_battle

} // namespace hoa_battle

#endif // __BATTLE_ACTORS_HEADER__


