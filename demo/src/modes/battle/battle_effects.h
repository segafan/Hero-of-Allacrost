////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2009 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_effects.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for battle actor effects.
***
*** This file contains the code that manages effects that influence an actor's
*** behavior and properties.
*** ***************************************************************************/

#ifndef __BATTLE_EFFECTS_HEADER__
#define __BATTLE_EFFECTS_HEADER__

#include "defs.h"
#include "utils.h"

#include "script.h"
#include "system.h"

#include "global_effects.h"

namespace hoa_battle {

namespace private_battle {

/** ****************************************************************************
*** \brief An abstract class for representing an actor in the battle
***
***
*** ***************************************************************************/
class BattleStatusEffect : public hoa_global::GlobalStatusEffect {
public:
	/** \param type The status type that this class object should represent
	*** \param intensity The intensity of the status
	*** \param actor A pointer to the actor affected by the status
	**/
	BattleStatusEffect(hoa_global::GLOBAL_STATUS type, hoa_global::GLOBAL_INTENSITY intensity, BattleActor* actor);

	~BattleStatusEffect();

	/** \brief Increments the status effect intensity by a positive amount
	*** \param amount The number of intensity levels to increase the status effect by
	*** \return True if the intensity level was modified
	**/
	bool IncrementIntensity(uint8 amount);

	/** \brief Decrements the status effect intensity by a negative amount
	*** \param amount The number of intensity levels to decrement the status effect by
	*** \return True if the intensity level was modified
	*** \note Intensity will not be decremented below GLOBAL_INTENSITY_NEUTRAL
	**/
	bool DecrementIntensity(uint8 amount);

	//! \brief Class Member Access Functions
	//@{
	//! \note This will cause the timer to reset and also
	void SetIntensity(hoa_global::GLOBAL_INTENSITY intensity);

	const std::string& GetName() const
		{ return _name; }

	BattleActor* GetAffectedActor() const
		{ return _affected_actor; }

	//! \note Returns a pointer instead of a reference so that Lua functions can access the timer
	hoa_system::SystemTimer* GetTimer()
		{ return &_timer; }
	//@}

private:
	//! \brief Holds the translated name of the status effect
	std::string _name;

//	//! \brief The opposte type for the status
// 	hoa_global::GLOBAL_STATUS _opposite_type;

	//! \brief A pointer to the actor that is affected by this status
	BattleActor* _affected_actor;

	//! \brief A timer used to determine how long the status effect lasts
	hoa_system::SystemTimer _timer;

	//! \brief A pointer to the update script function
	ScriptObject* _apply_function;

	/** \brief Applies the change in the status intensity to the affected actor
	*** This will call the Lua function pointed to by the _apply_function member.
	*** This will also reset the timer.
	**/
	void _ApplyChange();
}; // class BattleStatusEffect : public hoa_global::GlobalStatusEffect


/** ****************************************************************************
*** \brief Manages all elemental and status elements for an actor
***
***
***
*** \todo Elemental effects are not yet implemented or supported by this class.
*** They should be added when elemental effects in battle are ready to be used.
*** ***************************************************************************/
class EffectsSupervisor {
public:
	//! \param actor A valid pointer to the actor object that this class is responsible for
	EffectsSupervisor(BattleActor* actor);

	~EffectsSupervisor();

	//! \brief Updates the timers and state of any effects
	void Update();

	//! \brief Draws the element and status effect icons to the bottom status menu
	void Draw();

	/** \brief
	***
	**/
	hoa_global::GLOBAL_INTENSITY ChangeStatus(hoa_global::GLOBAL_STATUS status, hoa_global::GLOBAL_INTENSITY& intensity);

// 	hoa_global::GLOBAL_INTENSITY RemoveStatus(hoa_global::GLOBAL_STATUS status);

	/** \brief Returns true if the requested status is active on the managed actor
	*** \param status The type of status to check for
	**/
	bool IsStatusActive(hoa_global::GLOBAL_STATUS status)
		{ return (_status_effects.find(status) != _status_effects.end()); }

private:
	//! \brief A pointer to the actor that this class supervises effects for
	BattleActor* _actor;

	// TODO: support for elemental effects will be added at a later time
//	//! \brief Contains all active element effects
// 	std::map<hoa_global::GLOBAL_ELEMENTAL, BattleElementEffect*> _element_effects;

	//! \brief Contains all active status effects
	std::map<hoa_global::GLOBAL_STATUS, BattleStatusEffect*> _status_effects;
}; // class EffectsSupervisor

} // namespace private_battle

} // namespace hoa_battle

#endif // __BATTLE_EFFECTS_HEADER__
