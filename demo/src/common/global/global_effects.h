////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    global_effects.h
*** \author  Jacob Rudolph, rujasu@allacrost.org
*** \brief   Header file for global game effects
***
*** This file contains the class implementation for status and elemental effects.
*** Status effects are certain states that characters and enemies may fall in
*** to while in battle, such as being poisoned or confused. Elemental effects
*** are special properties that allow an aggressor to take advantage of to
*** expose a weakness on a target.
*** ***************************************************************************/

#ifndef __GLOBAL_EFFECTS_HEADER__
#define __GLOBAL_EFFECTS_HEADER__

#include "defs.h"
#include "utils.h"
#include "system.h"

#include "video.h"
#include "script.h"

#include "global_actors.h"

namespace hoa_global {

/** \name Elemental Effect Types
*** \brief Used to identify the eight different types of elementals
*** There are a total of four physical and four metaphysical elemental effects
**/
enum GLOBAL_ELEMENTAL {
	GLOBAL_ELEMENTAL_INVALID    = -1,
	GLOBAL_ELEMENTAL_FIRE       =  0,
	GLOBAL_ELEMENTAL_WATER      =  1,
	GLOBAL_ELEMENTAL_VOLT       =  2,
	GLOBAL_ELEMENTAL_EARTH      =  3,
	GLOBAL_ELEMENTAL_SLICING    =  4,
	GLOBAL_ELEMENTAL_SMASHING   =  5,
	GLOBAL_ELEMENTAL_MAULING    =  6,
	GLOBAL_ELEMENTAL_PIERCING   =  7,
	GLOBAL_ELEMENTAL_TOTAL      =  8
};

/** \name Status Effect Types
*** \brief Used to identify the various types of status effects
**/
enum GLOBAL_STATUS {
	GLOBAL_STATUS_INVALID    = -1,
	GLOBAL_STATUS_TOTAL      =  0
};

/** \name Effect Intensity Levels
*** \brief Used to reflect the potency of elemental and status effects
*** There are nine valid intensity levels. Four negative, four positive, and one neutral.
*** The neutral intensity level essentially equates to "no effect".
**/
enum GLOBAL_INTENSITY {
	GLOBAL_INTENSITY_INVALID       = -5,
	GLOBAL_INTENSITY_NEG_EXTREME   = -4,
	GLOBAL_INTENSITY_NEG_GREATER   = -3,
	GLOBAL_INTENSITY_NEG_MODERATE  = -2,
	GLOBAL_INTENSITY_NEG_LESSER    = -1,
	GLOBAL_INTENSITY_NEUTRAL       =  0,
	GLOBAL_INTENSITY_POS_LESSER    =  1,
	GLOBAL_INTENSITY_POS_MODERATE  =  2,
	GLOBAL_INTENSITY_POS_GREATER   =  3,
	GLOBAL_INTENSITY_POS_EXTREME   =  4,
	GLOBAL_INTENSITY_TOTAL         =  5
};

/** \brief Increments an intensity enumerated value
*** \param intensity A reference to the intensity data to modify
*** \param amount The number of levels to increase the intensity by (default == 1)
*** \return True if the intensity data was modified or false if it was left unchanged
*** \note The intensity will not be allowed to increase beyond the valid intensity range
**/
bool IncrementIntensity(GLOBAL_INTENSITY& intensity, uint8 amount = 1);

/** \brief Decrements an intensity enumerated value
*** \param intensity A reference to the intensity data to modify
*** \param amount The number of levels to decrease the intensity by (default == 1)
*** \return True if the intensity data was modified or false if it was left unchanged
*** \note The intensity will not be allowed to decrease beyond the valid intensity range
**/
bool DecrementIntensity(GLOBAL_INTENSITY& intensity, uint8 amount = 1);


/** ****************************************************************************
*** \brief Represents an elemental effect in the game
***
*** This class is a simple container of two enumerated values: an elemental type
*** and an intensity. Elemental effects provide for special types of attack and
*** defense bonuses. There are really two types of elemental effects: physical
*** and metaphysical, the same as the two attack damage types. Whether the elemental
*** effect represented by objects of this class are meant to serve as a defensive boost
*** or an offensive boost is determined by the context in which the class object is used.
***
*** \todo Explain any differences between how physical versus metaphyiscal elements
*** function in the game once that decision has been reached.
*** ***************************************************************************/
class GlobalElementalEffect {
public:
	/** \param type The elemental type that this class object should represent
	*** \param intensity The intensity of the elemental (default value == GLOBAL_INTENSITY_NEUTRAL)
	**/
	GlobalElementalEffect(GLOBAL_ELEMENTAL type, GLOBAL_INTENSITY intensity = GLOBAL_INTENSITY_NEUTRAL) :
		_type(type), _intensity(intensity) {}

	~GlobalElementalEffect()
		{}

	/** \brief Class Member Access Functions
	*** \note The "Set" functions may also change the _icon_image member of this class
	**/
	//@{
	GLOBAL_ELEMENTAL GetType() const
		{ return _type; }

	GLOBAL_INTENSITY GetIntensity() const
		{ return _intensity; }

	void SetIntensity(GLOBAL_INTENSITY intensity)
		{ _intensity = intensity; }
	//@}

	/** \brief Increments the elemental effect's intensity
	*** \param amount The number of levels to increase the intensity by (default = 1)
	*** \note The intensity will not be allowed to increase beyond the valid intensity range
	**/
	void IncrementIntensity(uint8 amount = 1);

	/** \brief Decrements the elemental effect's intensity
	*** \param amount The number of levels to decrease the intensity by (default = 1)
	*** \note The intensity will not be allowed to decrease beyond the valid intensity range
	**/
	void DecrementIntensity(uint8 amount = 1);

private:
	//! \brief The type of elemental that the object represents
	GLOBAL_ELEMENTAL _type;

	//! \brief The intensity level of this elemental effect
	GLOBAL_INTENSITY _intensity;
}; // class GlobalElementalEffect


/** ****************************************************************************
*** \brief Represents a status effect in the game
***
*** Status effects can be either aiding or ailing to the actor with the active
*** status. (TODO: provide a more accurate description once this class structure
*** is in a more-or-less final state).
***
*** \todo I think this class needs big changes. First, it should only be used to
*** represent a status effect I feel (as a property on weapons or armor) property
*** similar to the GlobalElementalEffect class. The battle code should create a
*** derived class to achieve that. Perhaps these classes should be renamed
*** Global*Attribute instead in to better represent what these classes really are?
***
*** Second, all of these modifier methods are unnecessary IMO. Instead, the derived
*** class should simply have a pointer to the actor the status is inflicted on and
*** the script can modify the properties of the actor as appropriate. I'm leaving
*** things the way they are now, but I really think these changes should be done.
*** -- Roots
*** ***************************************************************************/
class GlobalStatusEffect {
public:
	/** \param id The unique id value that determines the status effect that this class object should represent
	*** \param intensity The intensity of the status (default value == GLOBAL_INTENSITY_NEUTRAL)
	**/
	GlobalStatusEffect(uint32 id, GLOBAL_INTENSITY intensity = GLOBAL_INTENSITY_NEUTRAL);

	virtual ~GlobalStatusEffect();

	//! \brief Resets and starts the status effect's timer
	void StartTimer()
		{ _timer->Reset(); _timer->Run(); }

	/** \brief Sets how long the timer should run for
	*** \param n The number of milliseconds that the timer should run for
	**/
	void SetDuration(uint32 n)
		{ _timer->Initialize(n); }

	/** \brief Increments the status effect intensity by a positive amount
	*** \param amount The number of intensity levels to increase the status effect by
	*** \return True if the intensity level was modified
	**/
	bool IncrementIntensity(uint8 amount);

	/** \brief Decrements the status effect intensity by a negative amount
	*** \param amount The number of intensity levels to decrement the status effect by
	*** \return True if the intensity level was modified
	**/
	bool DecrementIntensity(uint8 amount);

	//! \brief Class Member Access Functions
	//@{
	const hoa_utils::ustring GetEffectName() const
		{ return _name; }

	GLOBAL_INTENSITY GetIntensity() const
		{ return _intensity; }

	hoa_system::SystemTimer* GetTimer() const
		{ return _timer; }

	float GetStrModifier() const
		{ return _str_modifier; }

	float GetVigModifier() const
		{ return _vig_modifier; }

	float GetForModifier() const
		{ return _for_modifier; }

	float GetProModifier() const
		{ return _pro_modifier; }

	float GetAgiModifier() const
		{ return _agi_modifier; }

	float GetEvaModifier() const
		{ return _eva_modifier; }

	bool IsStunEffect() const
		{ return _stun; }

	ScriptObject* GetInitFunction()
		{ return _init; }

	ScriptObject* GetUpdateFunction()
		{ return _update; }

	ScriptObject* GetRemoveFunction()
		{ return _remove; }

	// TODO: Return a pointer from image stored in GameGlobal
	hoa_video::StillImage* GetIconImage();

	void SetIntensity(GLOBAL_INTENSITY intensity)
		{ _intensity = intensity; }

	void SetStrModifier(float n)
		{ _str_modifier = n; }

	void SetVigModifier(float n)
		{ _vig_modifier = n; }

	void SetForModifier(float n)
		{ _for_modifier = n; }

	void SetProModifier(float n)
		{ _pro_modifier = n; }

	void SetAgiModifier(float n)
		{ _agi_modifier = n; }

	void SetEvaModifier(float n)
		{ _eva_modifier = n; }

	void SetStunEffect(bool n)
		{ _stun = n; }
	//@}

private:
	//! \brief An ID number that identifies the type of effect
	uint32 _id;

	//! \brief The name of the effect
	hoa_utils::ustring _name;

	//! \brief The intensity level of this status effect
	GLOBAL_INTENSITY _intensity;

	//! \brief Percentage modifiers for various actor stats
	//@{
	float _str_modifier;
	float _vig_modifier;
	float _for_modifier;
	float _pro_modifier;
	float _agi_modifier;
	float _eva_modifier;
	//@}

	//! \brief If true the status effect will prevent an inflicted actor from taking any action
	bool _stun;

	//! \brief A timer used to determine how long the status effect lasts
	hoa_system::SystemTimer* _timer;

	//! \brief A pointer to the initialization script function
	ScriptObject* _init;

	//! \brief A pointer to the update script function
	ScriptObject* _update;

	//! \brief A pointer to the remove script function
	ScriptObject* _remove;
}; // class GlobalStatusEffect

} // namespace hoa_global

#endif // __GLOBAL_EFFECTS_HEADER__
