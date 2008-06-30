////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
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
#include "global_objects.h"

namespace hoa_global {

namespace private_global {

/** \name Skill ID Range Constants
*** These constants set the maximum valid ID ranges for each skill category.
*** The full valid range for each skill category ID is:
*** - Attack:        1-10000
*** - Defend:    10001-20000
*** - Support:   20001-30000
**/
//@{
const uint32 MAX_ATTACK_ID   = 10000;
const uint32 MAX_DEFEND_ID   = 20000;
const uint32 MAX_SUPPORT_ID  = 30000;
//@}

} // namespace private_global

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
*** \brief A class for representing skills used in the game.
***
*** Skills are one representation of actions that a character or enemy may take
*** in battle. The actual execution of a skill is done by a Lua function, which
*** this class manages pointers to. Typically skills are not shared between
*** characters and enemies, primarily because characters are fully animated when
*** executing actions, while enemies are not animated. Skills can be used in
*** contexts other than battles. For instance, using a healing skill on the
*** party from the character management menu.
***
*** There are three types of skills: attack, defend, and support. The way that 
*** this class is initialized is by defining the "script ID" of the skill.
*** When this information is known, it seeks a Lua file that contains the
*** data to set the members of this class as well as the script functions that
*** actually perform the execution of the skill.
***
*** \todo Skill script functions should take abstract target type class pointer,
*** arguments, not actor pointers.
*** ***************************************************************************/
class GlobalSkill {
public:
	//! \param id The identification number of the skill to construct
	GlobalSkill(uint32 id);

	~GlobalSkill();

	GlobalSkill(const GlobalSkill& copy);

	GlobalSkill& operator=(const GlobalSkill& copy);

	//! \brief Returns true if the skill can be executed in battles
	bool IsExecutableInBattle() const
		{ return (_battle_execute_function != NULL); }

	//! \brief Returns true if the skill can be executed in menus
	bool IsExecutableInMenu() const
		{ return (_menu_execute_function != NULL); }

	/** \name Class member access functions
	*** \note No set functions are defined because the class members should only be defined
	*** by the Lua script.
	**/
	//@{
	hoa_utils::ustring GetName() const
		{ return _name; }

	hoa_utils::ustring GetDescription() const
		{ return _description; }

	uint32 GetID() const
		{ return _id; }

	uint8 GetType() const
		{ return _type; }

	uint32 GetSPRequired() const
		{ return _sp_required; }

	uint32 GetWarmupTime() const
		{ return _warmup_time; }

	uint32 GetCooldownTime() const
		{ return _cooldown_time; }

	GLOBAL_TARGET GetTargetType() const
		{ return _target_type; }

	bool IsTargetAlly() const
		{ return _target_ally; }

	/** \brief Returns a pointer to the ScriptObject of the battle execution function
	*** \note This function will return NULL if the skill is not executable in battle
	**/
	const ScriptObject* GetBattleExecuteFunction() const
		{ return _battle_execute_function; }

	/** \brief Returns a pointer to the ScriptObject of the menu execution function
	*** \note This function will return NULL if the skill is not executable in menus
	**/
	const ScriptObject* GetMenuExecuteFunction() const
		{ return _menu_execute_function; }
		
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

	//! \brief The type of target that the skill is executed upon.
	GLOBAL_TARGET _target_type;

	//! \brief If set to true, the target should be an ally. If not, the target should be a foe.
	bool _target_ally;

	/** \brief A vector containing all elemental effects that are defined by the skill
	*** This vector contains only the elementals that have non-zero strength (in other words, it does not
	*** contain every single type of elemental regardless of whether it is actually used or not). Therefore,
	*** it is very possible that this vector may be empty.
	**/
// 	std::vector<GlobalElementalEffect*> _elemental_effects;

	/** \brief A vector containing all status effects and their likelihood of success that are defined by the skill
	*** This vector contains only the status effects that have a non-zero chance of affecting their target. Therefore,
	*** it is very possible that this vector may be empty. The first element in the pair is a floating point value from 0.0
	*** to 1.0 that indicates the likelihood of success that the status effect has on a target. Note that this likelihood
	*** does not take into account that the target may have a particular defense or immunity against the status effect.
	**/
// 	std::vector<std::pair<float, GlobalStatusEffect*> > _status_effects;

	//! \brief A pointer to the skill's execution function for battles
	ScriptObject* _battle_execute_function;

	//! \brief A pointer to the skill's execution function for menus
	ScriptObject* _menu_execute_function;
}; // class GlobalSkill

} // namespace hoa_global

#endif // __GLOBAL_SKILLS_HEADER__
