////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    global_skills.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for global game skills.
***
*** This file contains the class implementation for "skills", which are the
*** actions that characters or enemies may take during a battle. It also contains
*** other classes that are not directly related to skills, such as the definition
*** of attack points.
*** ***************************************************************************/

#ifndef __GLOBAL_SKILLS_HEADER__
#define __GLOBAL_SKILLS_HEADER__

#include "defs.h"
#include "utils.h"

#include "global_actors.h"
#include "global_objects.h"

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

/** \name Status Effect Types
*** \brief Used to identify all of the numerous types of status effects
**/
enum GLOBAL_STATUS {
	GLOBAL_STATUS_INVALID    = -1,
	GLOBAL_STATUS_POISON     = 0,
	GLOBAL_STATUS_SLOW       = 1,
	GLOBAL_STATUS_TOTAL      = 2
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

/** \name Skill Types
*** \brief Enum values used to identify the type of a skill.
**/
enum GLOBAL_SKILL {
	GLOBAL_SKILL_INVALID  = -1,
	GLOBAL_SKILL_ATTACK   =  0,
	GLOBAL_SKILL_DEFEND   =  1,
	GLOBAL_SKILL_SUPPORT  =  2,
	GLOBAL_SKILL_TOTAL    =  3
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
	GlobalStatusEffect(GLOBAL_STATUS type, GLOBAL_INTENSITY intensity = GLOBAL_INTENSITY_NEUTRAL) :
		_type(type), _intensity(intensity) {}

	~GlobalStatusEffect()
		{}

	// TODO: Return a pointer from image stored in GameGlobal
	// hoa_video::StillImage* GetIconImage();

	//! \brief Class Member Access Functions
	//@{
	GLOBAL_STATUS GetType() const
		{ return _type; }

	GLOBAL_INTENSITY GetIntensity() const
		{ return _intensity; }

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

private:
	/** \brief The type (identifier) of status that the object represents
	*** Refer to the Status Effect Types for a list of the valid types and values that
	*** this member may be.
	**/
	GLOBAL_STATUS _type;

	/** \brief The level of intensity of the status effect
	*** There are four levels of intensity, as indicated by the Status Effect Intensities constants.
	*** This member should only ever equal one of those values
	**/
	GLOBAL_INTENSITY _intensity;
}; // class GlobalStatusEffect


/** ****************************************************************************
*** \brief A class for representing skills used in the game.
***
*** Skills are one representation of actions that a character or enemy may take
*** in battle. The actual execution of a skill is done by a Lua function, which
*** this class makes reference to. Typically skills are not shared between
*** characters and enemies, primarily because characters are fully animated when
*** executing actions, while enemies are not animated.
*** 
*** There are three types of skills: attack, defend, and support. The way that 
*** this class is initialized is by defining the "script name" of the skill.
*** When this information is known, it seeks a Lua file that contains the
*** data to set the members of this class as well as the script functions that
*** actually describe how to execute the skill.
*** ***************************************************************************/
class GlobalSkill {
public:
	GlobalSkill(uint32 id);

	~GlobalSkill();

	/** \brief Calls the script function which executes the skill for battles
	*** \param target A pointer to the target of the skill
	*** \param instigator A pointer to the instigator whom is executing the skill
	**/
	void BattleExecute(hoa_battle::private_battle::BattleActor* target, hoa_battle::private_battle::BattleActor* instigator);

	/** \brief Calls the script function which executes the skill for menus
	*** \param target A pointer to the target of the skill
	*** \param instigator A pointer to the instigator whom is executing the skill
	**/
	void MenuExecute(hoa_global::GlobalCharacter* target, hoa_global::GlobalCharacter* instigator);

	/** \name Class member access functions
	*** \note No set functiosn are defined because the class members should only be defined
	*** by the Lua script.
	**/
	//@{
	hoa_utils::ustring GetName() const
		{ return _name; }

	uint8 GetType() const
		{ return _type; }

	uint32 GetSPRequired() const
		{ return _sp_required; }

	uint32 GetWarmupTime() const
		{ return _warmup_time; }

	uint32 GetCooldownTime() const
		{ return _cooldown_time; }

	uint32 GetLevelRequired() const
		{ return _level_required; }

	GLOBAL_TARGET GetTargetType() const
		{ return _target_type; }

	GLOBAL_ALIGNMENT GetTargetAlignment() const
		{ return _target_alignment; }
		
// 	std::vector<GlobalElementalEffect*>& GetElementalEffects() const
// 		{ return _elemental_effects; }

// 	std::vector<std::pair<float, GlobalStatusEffect*> >& GetStatusEffects() const
// 		{ return _status_effects; }
	//@}

private:
	//! \brief The name of the skill as it will be displayed on the screen.
	hoa_utils::ustring _name;

	//! \brief A short description of the skill
	hoa_utils::ustring _description;

	//! \brief The unique identifier number of the skill.
	uint32 _id;

	/** \brief The type identifier for the skill
	*** Refer to the Skill Types constants defined in this file.
	**/
	GLOBAL_SKILL _type;

	//! \brief The type of target that the skill is executed upon.
	GLOBAL_TARGET _target_type;

	/** \brief Whose side the skill is on.
	*** Can either target friendlies, enemies, or both
	**/
	GLOBAL_ALIGNMENT _target_alignment;

	/** \brief Where the skill can be used
	*** Can either target friendlies, enemies, or both
	**/
	GLOBAL_USE _usage;

	/** \brief The amount of skill points (SP) that the skill requires to be used
	*** Zero is a valid value for this member and simply means that no skill points are required
	*** to use the skill. These are called "innate skills".
	**/
	uint32 _sp_required;

	/** \brief The amount of time that must expire before a skill can be used from when it is selected
	*** When a character or enemy is determined to use a skill, this member tells how many milliseconds must
	*** pass before the skill can be used (in otherwords, before it can be placed in the action queue in a battle).
	*** It is acceptable for this member to be zero.
	**/
	uint32 _warmup_time;

	/** \brief The amount of time that must expire after a skill can be used before the actor can regain their stamina
	*** After a character or enemy uses a skill, this member tells how many milliseconds must pass before
	*** the invoker can recover and begin re-filling their stamina bar. It is acceptable for this member to be zero.
	**/
	uint32 _cooldown_time;

	/** \brief The experience level required to use the skill
	*** If the actor wishing to use the skill does not have the minimum experience level required, then the
	*** existance of the skill may not be acknowledged at all depending upon the game code context which uses
	*** it. For example, skills that can not yet be used would not be show in the list of skills to the player.
	*** Zero is a valid member for this class as well and simply indicates that no specific experience level is required.
	**/
	uint32 _level_required;

	/** \brief A vector containing all elemental effects that are defined by the skill
	*** This vector contains only the elementals that have non-zero strength (in other words, it does not
	*** contain every single type of elemental regardless of whether it is actually used or not). Therefore,
	*** it is very possible that this vector may be empty.
	**/
	std::vector<GlobalElementalEffect*> _elemental_effects;

	/** \brief A vector containing all status effects and their likelihood of success that are defined by the skill
	*** This vector contains only the status effects that have a non-zero chance of affecting their target. Therefore,
	*** it is very possible that this vector may be empty. The first element in the pair is a floating point value from 0.0
	*** to 1.0 that indicates the likelihood of success that the status effect has on a target. Note that this likelihood
	*** does not take into account that the target may have a particular defense or immunity against the status effect.
	**/
	std::vector<std::pair<float, GlobalStatusEffect*> > _status_effects;

	//! \brief A pointer to the skill's execution function for battles
	ScriptObject _battle_execute_function;

	//! \brief Loads the skill's data from a file and sets the members of the class
	void _Load();

}; // class GlobalSkill

} // namespace hoa_global

#endif // __GLOBAL_SKILLS_HEADER__
