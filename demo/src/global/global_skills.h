////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
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



namespace hoa_global {

/** \name Status Effect Types
*** \brief Used to identify all of the numerous types of status effects
**/
//@{
const uint8 GLOBAL_STATUS_NONE   = 0;
const uint8 GLOBAL_STATUS_POISON = 1;
const uint8 GLOBAL_STATUS_SLOW   = 2;
//@}

/** \name Status Effect Intensity Levels
*** \brief Used to reflect the stengeth/potency of status effects
*** Each valid intensity level has a different color associated with it.
**/
//@{
const uint8 GLOBAL_INTENSITY_NONE      = 0;
const uint8 GLOBAL_INTENSITY_LESSER    = 1;
const uint8 GLOBAL_INTENSITY_MODERATE  = 2;
const uint8 GLOBAL_INTENSITY_GREATER   = 3;
const uint8 GLOBAL_INTENSITY_EXTREME   = 4;
//@}

/** \name Elemental Effect Types
*** \brief Constants used to identify the various elementals
**/
//@{
const uint8 GLOBAL_ELEMENTAL_NONE      = 0x00;
const uint8 GLOBAL_ELEMENTAL_FIRE      = 0x01;
const uint8 GLOBAL_ELEMENTAL_WATER     = 0x02;
const uint8 GLOBAL_ELEMENTAL_VOLT      = 0x04;
const uint8 GLOBAL_ELEMENTAL_EARTH     = 0x08;
const uint8 GLOBAL_ELEMENTAL_SLICING   = 0x10;
const uint8 GLOBAL_ELEMENTAL_SMASHING  = 0x20;
const uint8 GLOBAL_ELEMENTAL_MAULING   = 0x40;
const uint8 GLOBAL_ELEMENTAL_PIERCING  = 0x80;
//@}

/** \name Skill Types
*** \brief Constants used to identify the type of a skills
**/
//@{
const uint8 GLOBAL_SKILL_NONE      = 0x00;
const uint8 GLOBAL_SKILL_ATTACK    = 0x01;
const uint8 GLOBAL_SKILL_DEFEND    = 0x02;
const uint8 GLOBAL_SKILL_SUPPORT   = 0x04;
//@}

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
	/** \brief No-arg constructor sets all class members to invalid/uninitialized states
	*** Usually you do not want to create an object of this class using its no-arg destructor,
	*** however it is here just in case.
	**/
	GlobalStatusEffect();
	GlobalStatusEffect(uint8 type, uint8 intensity_level = GLOBAL_INTENSITY_NONE);
	~GlobalStatusEffect();

	/** \brief Class Member Access Functions
	*** \note The "Set" functions may also change the _icon_image member of this class
	**/
	//@{
	uint8 GetType() const
		{ return _type; }
	uint8 GetIntensityLevel() const
		{ return _intensity_level; }
	//! \note This function may return NULL if the _type member is not properly initialized
	const hoa_video::StillImage* GetIconImage() const
		{ return _icon_image; }

	void SetType(uint8 type);
	void SetIntensityLevel(uint8 intensity_level);
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

	/** \brief Checks that the argument is a valid status type
	*** \return True if the type is valid, false if it is not recognized or is equal to GLOBAL_STATUS_NONE
	**/
	static bool CheckValidType(uint8 type);

private:
	/** \brief The type (identifier) of status that the object represents
	*** Refer to the Status Effect Types for a list of the valid types and values that
	*** this member may be.
	**/
	uint8 _type;

	/** \brief The intensity level of the effect
	*** There are four levels of intensity, as indicated by the Status Effect Intensities constants.
	*** This member should only ever equal one of those values
	**/
	uint8 _intensity_level;

	/** \brief A pointer to an icon image that represents the status effect
	*** The _icon_image is not a single image file, but rather a conglomeration of different images.
	*** This member is automatically updated whenever the type or intensity of the status effect changes.
	*** This image is a visual depiction of both the type and intensity of the status effect. The intensity
	*** is represented by a background color, and the type is represented by an image. If the _intensity member
	*** is invalid, then no background color is drawn. If the type member is invalid, then the image remains NULL. 
	***
	*** \note Each icon is 25x25 pixels in dimension.
	**/
	hoa_video::StillImage *_icon_image;

	/** \brief Creates an icon image to reflect the current type and intensity
	*** This function is called whenever it is determined that the _type or _intensity member has been
	*** changed.
	**/
	void _CreateIconImage();
}; // class GlobalStatusEffect



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
	/** \brief No-arg constructor sets all class members to invalid/uninitialized states
	*** Usually you do not want to create an object of this class using its no-arg destructor,
	*** however it is here just in case.
	**/
	GlobalElementalEffect();
	GlobalElementalEffect(uint8 type, uint32 value = 0);
	~GlobalElementalEffect()
		{}

	/** \brief Class Member Access Functions
	*** \note The "Set" functions may also change the _icon_image member of this class
	**/
	//@{
	uint8 GetType() const
		{ return _type; }
	uint32 GetStrength() const
		{ return _strength; }
	//! \note This function may return NULL if the _type member is not properly initialized
	const hoa_video::StillImage* GetIconImage() const
		{ return _icon_image; }

	void SetType(uint8 type);
	void SetStrength(uint32 strength)
		{ _strength = strength; }
	//@}

	/** \brief Increments the elemental effect's strength
	*** \param amount The number to increase the _strength member by
	**/
	void IncrementStrength(uint32 amount);
	
	/** \brief Decrements the elemental effect's strength
	*** \param amount The number to decrease the _strength member by
	**/
	void DecrementStrength(uint32 amount);

	/** \brief Checks that the argument is a valid elemental type
	*** \return True if the type is valid, false if it is not recognized or is equal to GLOBAL_ELEMENTAL_NONE
	**/
	static bool CheckValidType(uint8 type);

private:
	/** \brief The type (identifier) of elemental that the object represents
	*** Refer to the Elemental Effect Types for a list of the valid types and values that
	*** this member may be.
	**/
	uint8 _type;

	/** \brief The amount of strength that the elemental effect has
	*** Note that this member only includes positive values since it is an unsigned integer. This is
	*** done for simplicity. Whether the elemental effect is a defensive boost or an offensive boost
	*** is not determined by this class, but rather the context in which the class object is used.
	**/
	uint32 _strength;

	/** \brief A pointer to an icon image that represents the elemental effect
	*** This member is a pointer to the image of the elemental that is stored in the GameGlobal singleton class.
	*** The StillImage object that is pointed to by this member is never created nor destroyed by this class.
	***
	*** \note Each icon is 25x25 pixels in dimension.
	**/
	hoa_video::StillImage *_icon_image;

	/** \brief Sets the image that corresponds to the _type member
	*** This function sets up the _icon_image member to reflect what the type of the elemental
	*** effect is.
	**/
	void _SetIconImage();
}; // class GlobalElementalEffect



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
	/** \brief No-arg constructor sets all class members to invalid/uninitialized states
	*** Usually you do not want to create an object of this class using its no-arg destructor,
	*** however it is here just in case.
	**/
	GlobalSkill();
	GlobalSkill(std::string script);
	~GlobalSkill();

	/** \name Class member access functions
	*** \note No set functiosn are defined because the class members should only be defined
	*** by the Lua script.
	**/
	//@{
	hoa_utils::ustring GetSkillName() const
		{ return _skill_name; }
	uint8 GetSkillType() const
		{ return _skill_type; }
	uint32 GetSkillPointsRequired() const
		{ return _skill_points_required; }
	uint32 GetWarmupTime() const
		{ return _warmup_time; }
	uint32 GetCooldownTime() const
		{ return _cooldown_time; }
	uint32 GetLevelRequired() const
		{ return _level_required; }
	uint32 GetNumberTargets() const
		{ return _number_targets; }
// 	std::vector<GlobalElementalEffect*>& GetElementalEffects() const
// 		{ return _elemental_effects; }
// 	std::vector<std::pair<float, GlobalStatusEffect*> >& GetStatusEffects() const
// 		{ return _status_effects; }
	//@}

private:
	//! \brief The name of the skill as it will be displayed on the screen.
	hoa_utils::ustring _skill_name;
	/** \brief The type identifier for the skill
	*** Refer to the Skill Types constants defined in this file.
	**/
	uint8 _skill_type;
	/** \brief The amount of skill points (SP) that the skill requires to be used
	*** Zero is a valid value for this member and simply means that no skill points are required
	*** to use the skill. These are called "innate skills".
	**/
	uint8 _sp_usage;
	/** \brief Don't know for sure. My best guess would be the amount of skill points lost 
	 */
	uint32 _skill_points_required;
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
	/** \brief The number of targets that the skill will effect when it is used
	*** A target could be either an attack point (for a sword strike) or an entire actor (for a heal spell).
	*** The number of targets does <b>not</b> include the invoker if the skill does nothing more than it would normally
	*** be expected to do (reduce the actor's current number of skill points, etc.).
	**/
	uint32 _number_targets;

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

	//! \brief The name of the skill as it is used to reference its information in a Lua script
	std::string _script_name;
	// TODO: Add a pointer to the skill's execution script function
	// ScriptFunction _function;
}; // class GlobalSkill

} // namespace hoa_global

#endif // __GLOBAL_SKILLS_HEADER__
