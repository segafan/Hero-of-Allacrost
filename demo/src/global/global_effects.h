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
*** \brief   Header file for global game effects.
***
*** This file contains the class implementation for status and elemental effects.
*** ***************************************************************************/

#ifndef __GLOBAL_EFFECTS_HEADER__
#define __GLOBAL_EFFECTS_HEADER__

#include "defs.h"
#include "utils.h"

#include "video.h"
#include "script.h"

#include "global_actors.h"

namespace hoa_global {

/** \name Elemental Effect Types
*** \brief Constants used to identify the various elementals
*** There are a total of eight elementals: four physical and four metaphysical.
**/
enum GLOBAL_ELEMENTAL {
	GLOBAL_ELEMENTAL_INVALID    = 0,
	GLOBAL_ELEMENTAL_FIRE       = 1,
	GLOBAL_ELEMENTAL_WATER      = 2,
	GLOBAL_ELEMENTAL_VOLT       = 3,
	GLOBAL_ELEMENTAL_EARTH      = 4,
	GLOBAL_ELEMENTAL_SLICING    = 5,
	GLOBAL_ELEMENTAL_SMASHING   = 6,
	GLOBAL_ELEMENTAL_MAULING    = 7,
	GLOBAL_ELEMENTAL_PIERCING   = 8,
	GLOBAL_ELEMENTAL_TOTAL      = 9
};

/** \name EffectIntensity Levels
*** \brief Used to reflect the potency of elemental and status effects
***
***
**/
enum GLOBAL_INTENSITY {
	GLOBAL_INTENSITY_INVALID       = -5,
	GLOBAL_INTENSITY_NEG_EXTREME   = -4,
	GLOBAL_INTENSITY_NEG_GREATER   = -3,
	GLOBAL_INTENSITY_NEG_MODERATE  = -2,
	GLOBAL_INTENSITY_NEG_LESSER    = -1,
	GLOBAL_INTENSITY_NEUTRAL       = 0,
	GLOBAL_INTENSITY_POS_LESSER    = 1,
	GLOBAL_INTENSITY_POS_MODERATE  = 2,
	GLOBAL_INTENSITY_POS_GREATER   = 3,
	GLOBAL_INTENSITY_POS_EXTREME   = 4,
	GLOBAL_INTENSITY_TOTAL         = 5
};

/** ****************************************************************************
*** \brief Represents an elemental effect on an actor or other object
***
*** Elemental effects are special types of attack and defense bonuses. They do
*** not apply on individual attack points, but rather on the whole of a
*** character or enemy. There are really two types of elemental effects: physical
*** and metaphysical (the same as the two types of attacks). The difference
*** between physical and metaphysical attacks is the relationship of elemental
*** strengths and weaknesses. For example, equpping an armor with the metaphysical
*** "fire" elemental makes the bearer weak against water, but strong against earth.
*** On the contrary, an armor with the phyiscal "piercing" elemental makes the bearer
*** strong against piercing attacks.
*** ***************************************************************************/
class GlobalElementalEffect {
public:
	GlobalElementalEffect() :
		_type(GLOBAL_ELEMENTAL_INVALID), _intensity(GLOBAL_INTENSITY_NEUTRAL) {}

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
	**/
	void IncrementIntensity(uint8 amount = 1);
	
	/** \brief Decrements the elemental effect's intensity
	*** \param amount The number of levels to decrease the intensity by (default = 1)
	**/
	void DecrementIntensity(uint8 amount = 1);

private:
	/** \brief The type (identifier) of elemental that the object represents
	*** Refer to the Elemental Effect Types for a list of the valid types and values that
	*** this member may be.
	**/
	GLOBAL_ELEMENTAL _type;

	/** \brief The intensity of the elemental effect has
	*** Note that this member only includes positive values since it is an unsigned integer. This is
	*** done for simplicity. Whether the elemental effect is a defensive boost or an offensive boost
	*** is not determined by this class, but rather the context in which the class object is used.
	**/
	GLOBAL_INTENSITY _intensity;
}; // class GlobalElementalEffect



/** ****************************************************************************
*** \brief Represents a status effect on an actor or other object
***
*** Status effects are either aiding (boost to strength) or ailing (poisoned).
*** An object of this class represents a single status effect (not mulitple).
*** A feature unique to Allacrost is that status effects have different levels
*** of intensity, four to be exact.
***
*** \todo Determine how we wish to handle status effects which are polar
*** opposites of one another (i.e., increase in speed versus decrease in speed).
***
*** \todo Add a pointer to the script function to execute (if any) to apply the
*** status effect. Note that some status effects will require periodic updates
*** (e.g. poison) while others only need to be applied one time (e.g. increase
*** in strength).
*** ***************************************************************************/
class GlobalStatusEffect {
public:
	GlobalStatusEffect(uint32 id, GLOBAL_INTENSITY intensity = GLOBAL_INTENSITY_NEUTRAL);

	virtual ~GlobalStatusEffect();

	// TODO: Return a pointer from image stored in GameGlobal
	hoa_video::StillImage* GetIconImage();

	//! \brief Class Member Access Functions
	//@{
	GLOBAL_INTENSITY GetIntensity() const
		{ return _intensity; }

	const hoa_utils::ustring GetEffectName() const
		{ return _name; }

	void SetIntensity(GLOBAL_INTENSITY intensity)
		{ _intensity = intensity; }
	//@}

	/** \brief Increments the status effect intensity by a positive amount
	*** \param amount The number of intensity levels to increase the status effect by
	*** \return False if the intensity level could not fully be increased by the amount specified (upper bound limit)
	***
	*** Usually the return value of this function can be safely ignored. What happens is, for example, when the original
	*** intensity level is equal to "greater intensity level" and the function is given a parameter of "2", it will
	*** increment the intensity by one level (not two) to "extreme" (the maximum upper limit). It then returns false
	*** to indicate that the intensity could not fully be increased by the amount specified.
	***
	*** \note If this function does indeed change the intensity level, the _icon_image pointer will also be changed to
	*** reflect this.
	**/
	bool IncrementIntensity(uint8 amount);
	
	/** \brief Decrements the status effect intensity by a specified amount
	*** \param amount The number of intensity levels to decrement the status effect by
	*** \return False if the intensity level reaches GLOBAL_INTENSITY_INVALID (zero intensity)
	***
	*** Unlike the IncrementIntensity function, the user is advised to always check the return value of this function.
	*** When the intensity reaches zero, the user may wish to delete the StatusEffect object class as it no longer has
	*** any effect on the actor.
	**/
	bool DecrementIntensity(uint8 amount);

	//! \brief Script Function Accessors
	//@{
	ScriptObject* GetInitFunction()		{ return _init; }
	ScriptObject* GetUpdateFunction()	{ return _update; }
	ScriptObject* GetRemoveFunction()	{ return _remove;}
	//@}

private:
	//! \brief unique ID number of effect
	uint32 _id;

	//! \brief The name of the effect
	hoa_utils::ustring _name;

	/** \brief The level of intensity of the status effect
	*** There are four levels of intensity, as indicated by the Status Effect Intensities constants.
	*** This member should only ever equal one of those values
	**/
	GLOBAL_INTENSITY _intensity;

	//! \brief Initialization script for object
	ScriptObject* _init;
	//! \brief Update script for object
	ScriptObject* _update;
	//! \brief Removal script for object
	ScriptObject* _remove;
}; // class GlobalStatusEffect

} // namespace hoa_global

#endif
