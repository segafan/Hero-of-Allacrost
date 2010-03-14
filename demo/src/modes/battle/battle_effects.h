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
*** \brief Manages all data related to a single status effect in battle
***
*** This class extends the GlobalStatusEffect class, which contains nothing
*** more than two enum members representing the status type and intensity. This
*** class provides a complete implementation of a status effect, including an
*** image icon, a timer, and script functions to implement the effect.
***
*** This class represents an active effect on a single actor. Objects of this
*** class are not shared on multiple actors in any form. Status effects only
*** have positive intensity values and will naturally decrease in intensity over
*** time until they reach the neutral intensity level. Some types of status
*** effects have an opposite type. For example, one status effect may boost the
*** actor's strength while another drains strength. We do not allow these two
*** statuses to co-exist on the same actor, thus the two have a cancelation effect
*** on each other and the stronger (more intense) effect will remain.
***
*** \todo Implement opposite types for status effects and possibly add a boolean
*** member to indicate whether the status is aiding or ailing.
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

	hoa_video::StillImage* GetIconImage() const
		{ return _icon_image; }
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

	//! \brief A pointer to the icon image that represents the status. Will be NULL if the status is invalid
	hoa_video::StillImage* _icon_image;

	/** \brief Applies the change in the status intensity to the affected actor
	*** This will call the Lua function pointed to by the _apply_function member.
	*** This will also reset the timer and update the icon image member.
	**/
	void _ApplyChange();
}; // class BattleStatusEffect : public hoa_global::GlobalStatusEffect


/** ****************************************************************************
*** \brief Manages all elemental and status elements for an actor
***
*** The class contains all of the active effects on an actor. These effects are
*** updated regularly by this class and are removed when their timers expire.
*** This class also contains draw functions which will display icons for all the
*** effects of an actor to the screen.
***
*** \todo The Draw function probably should be renamed to something more specific
*** and should check whether or not the actor is a character. Its intended to be
*** used only for character actors to draw on the bottom menu. There should also
*** probably be another draw function for drawing the status of an actor to the
*** command window.
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

	/** \brief Changes the intensity level of a status effect
	*** \param status The status effect type to change
	*** \param intensity A reference to the amount of intensity to increase or decrease the status effect by
	*** \return The previous intensity level of the status
	***
	*** This is the one and only function for performing any status changes to an actor. It will add status effects,
	*** modify the intensity of existing effects, or remove status effects depending on the current state of the
	*** status effect and the value of the intensity argument. If the intensity argument is positive, the status
	*** effect will be added at that intensity if it does not already exist. If it does exist, then this will increase
	*** the intensity of the effect. A negative intensity will either decrease the intensity of the effect if it exists,
	*** remove the effect if it exists and the negative intensity outweights the current intensity, or if the effect
	*** does not exist when intensity is negative, no action will be performed.
	***
	*** \note The intensity argument, which is a reference, is modified by the function to the new intensity value set
	*** for the status. Since the old intensity is the return value of this function, this means that both new and old
	*** intensity values for the status can be obtained after the change is made.
	***
	*** \note If any arguments are invalid the function will return GLOBAL_INTENSITY_INVALID.
	***
	*** \note To be absolutely certain that a status effect is removed from the actor regardless of its current intensity,
	*** use the value GLOBAL_INTENSITY_NEG_EXTREME for the intensity argument.
	***
	*** \note This function only changes the state of the status and does <i>not</i> display any visual or other indicator
	*** to the player that the status was modified. Typically you should invoke BattleActor::RegisterStatusChange(...)
	*** when you want to change the status of the actor. That method will call this one as well as activating the proper
	*** indicator based on the state change performed for the status.
	**/
	hoa_global::GLOBAL_INTENSITY ChangeStatus(hoa_global::GLOBAL_STATUS status, hoa_global::GLOBAL_INTENSITY& intensity);

	/** \brief Returns true if the requested status is active on the managed actor
	*** \param status The type of status to check for
	**/
	bool IsStatusActive(hoa_global::GLOBAL_STATUS status)
		{ return (_status_effects.find(status) != _status_effects.end()); }

private:
	//! \brief A pointer to the actor that this class supervises effects for
	BattleActor* _actor;

	// TODO: support for elemental effects may be added here at a later time
//	//! \brief Contains all active element effects
// 	std::map<hoa_global::GLOBAL_ELEMENTAL, BattleElementEffect*> _element_effects;

	//! \brief Contains all active status effects
	std::map<hoa_global::GLOBAL_STATUS, BattleStatusEffect*> _status_effects;
}; // class EffectsSupervisor

} // namespace private_battle

} // namespace hoa_battle

#endif // __BATTLE_EFFECTS_HEADER__
