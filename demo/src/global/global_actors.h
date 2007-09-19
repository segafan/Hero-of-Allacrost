////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    global_actors.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for global game actors.
***
*** This file contains the implementation of "actors", which are living entities
*** in the game (characters and enemies, but not NPCs). It also contains classes
*** that are closely related to the implementation of actors
*** ***************************************************************************/

#ifndef __GLOBAL_ACTORS_HEADER__
#define __GLOBAL_ACTORS_HEADER__

#include <utility>

#include "defs.h"
#include "utils.h"

namespace hoa_global {

/** \name Target Types
*** \brief Enum values used for declaring the type of target of items, skills, and actions.
**/
enum GLOBAL_TARGET {
	GLOBAL_TARGET_INVALID      = -1,
	GLOBAL_TARGET_ATTACK_POINT =  0,
	GLOBAL_TARGET_ACTOR        =  1,
	GLOBAL_TARGET_PARTY        =  2,
	GLOBAL_TARGET_TOTAL        =  3
};

/** \name GlobalItem and GlobalSkill Usage Cases
*** \brief Enum values used for identification of different game object types
**/
enum GLOBAL_USE {
	GLOBAL_USE_INVALID = -1,
	GLOBAL_USE_MENU    =  0,
	GLOBAL_USE_BATTLE  =  1,
	GLOBAL_USE_ALL     =  2,
	GLOBAL_USE_TOTAL   =  3
};

/** \name Character Attack Point Positions
*** \brief Intergers that represent the location of the four attack points on characters
**/
//@{
const uint32 GLOBAL_POSITION_HEAD  = 0;
const uint32 GLOBAL_POSITION_TORSO = 1;
const uint32 GLOBAL_POSITION_ARMS  = 2;
const uint32 GLOBAL_POSITION_LEGS  = 3;
//@}

/** \name Game Character IDs
*** \brief Integers that are used for identification of characters
*** These series of constants are used as bit-masks for determining things such as if the character
*** may use a certain item. Only one bit should be set for each character ID.
**/
//@{
const uint32 GLOBAL_CHARACTER_INVALID     = 0x00000000;
const uint32 GLOBAL_CHARACTER_CLAUDIUS    = 0x00000001;
const uint32 GLOBAL_CHARACTER_LAILA       = 0x00000002;
const uint32 GLOBAL_CHARACTER_ALL         = 0xFFFFFFFF;
//@}


/** ****************************************************************************
*** \brief Represents a target for an item, action, or skill
***
*** This is an abstract class that all three types of target classes inherit
*** from. Pointers to this class are used as parameters to script functions
*** that
*** ***************************************************************************/
class GlobalTarget {
public:
	GlobalTarget()
		{}

	virtual ~GlobalTarget()
		{}

	/** \brief A pure virtual function that returns the type of target
	*** \return The appropriate GLOBAL_TARGET enum for the target type in question
	***
	*** This method is necessary so that GlobalTarget can be an abstract class.
	*** Inheriting classes simply need to return a valid target type.
	**/
	virtual GLOBAL_TARGET GetTargetType() = 0;
}; // class GlobalTarget


/** ****************************************************************************
*** \brief Represents the "attack points" present on an actor
***
*** An attack point is a place where an actor may be attacked (it is <b>not</b> a numerical
*** quantity). Actors may have multiple attack points, each with their own resistances
*** and weaknesses. For example, the number of attack points on all character battle sprites
*** is four, and they are located on the head, torso, arms, and legs. An attack point may have
*** armor equipped to it, which would increase its defense ratings. Attack points may also have
*** "natural" weaknesses to certain status effects. For example, attacking the legs of an
*** actor may induce a status effect that reduces their agility
*** ***************************************************************************/
class GlobalAttackPoint : public GlobalTarget {
	friend class GlobalActor;
	friend class GlobalCharacter;
	friend class GlobalEnemy;

public:
	GlobalAttackPoint(GlobalActor* actor_owner) :
		_actor_owner(actor_owner) {}

	~GlobalAttackPoint()
		{}

	GLOBAL_TARGET GetTargetType()
		{ return GLOBAL_TARGET_ATTACK_POINT; }

	//! \name Class Member Access Functions
	//@{
	hoa_utils::ustring& GetName()
		{ return _name; }

	uint16 GetXPosition() const
		{ return _x_position; }

	uint16 GetYPosition() const
		{ return _y_position; }

	float GetFortitudeModifier() const
		{ return _fortitude_modifier; }

	float GetProtectionModifier() const
		{ return _protection_modifier; }

	float GetEvadeModifier() const
		{ return _evade_modifier; }

	GlobalActor* GetActorOwner() const
		{ return _actor_owner; }

	uint16 GetTotalPhysicalDefense() const
		{ return _total_physical_defense; }

	uint16 GetTotalMetaphysicalDefense() const
		{ return _total_metaphysical_defense; }

	float GetTotalEvadeRating() const
		{ return _total_evade_rating; }
	//@}

private:
	/** \brief The name of the attack point as is displayed on the screen
	*** Usually, this is simply the name of a body part such as "head" or "tail". More elaborate names
	*** may be chosen for special foes and bosses, however.
	**/
	hoa_utils::ustring _name;

	/** \brief The position of the physical attack point relative to the actor's battle sprite
	*** These members treat the bottom left corner of the sprite as the origin (0, 0) and increase in the
	*** right and upwards directions. The combination of these two members point to the center pinpoint
	*** location of the attack point. The units of these two members are in number of pixels.
	**/
	uint16 _x_position, _y_position;

	/** \brief The defense and evasion percentage modifiers for this attack point
	***
	*** These are called "modifiers" because they modify the value of fortitude, protection, and evade ratings of the
	*** actor. They represent percentage change from the base stat. So for example, a fortitude modifer that is 0.25f
	*** increases the fortitude of the attack point by 25%. If the base protection rating was 10 and the protection
	*** modifier was -0.30f, the resulting protection for the attack point would be: 10 + (10 * -0.30f) = 7.
	***
	*** The lower bound for each modifier is -1.0f (-100%), which will result in a value of zero for that stat. No 
	*** actor stats can be negative so even if the modifier drops below -1.0f, the resulting value will still be zero.
	*** There is no theoretical upper bound, but it is usually advised to keep it under 1.0f (100%).
	**/
	//@{
	float _fortitude_modifier;
	float _protection_modifier;
	float _evade_modifier;
	//@}

	//! \brief A pointer to the actor which "owns" this attack point (i.e., the attack point is a location on the actor)
	GlobalActor* _actor_owner;

	/** \brief The cumunalative defense and evade stats for this attack point
	*** These totals include the actor's base stat, the percentage modifier for the attack point, and the stats of any
	*** armor that is equipped on the attack point.
	**/
	//@{
	uint16 _total_physical_defense;
	uint16 _total_metaphysical_defense;
	float _total_evade_rating;
	//@}

	/** \brief A vector containing all status effects that may be triggered by attacking the point
	*** This vector contains only the status effects that have a non-zero chance of affecting their target. Therefore,
	*** it is very possible that this vector may be empty. The first element in the pair is a floating point value from 0.0
	*** to 1.0 that indicates the likelihood of success that should the attack point be successfully attacked by an opponent,
	*** the status effect becomes triggered on the enemy. Note that this likelihood
	*** does not take into account that the target may have a particular defense or immunity against the status effect.
	**/
	// TODO: Add status and elemental effects to attack points
	// std::vector<std::pair<float, GlobalStatusEffect*> > _status_weaknesses;

	/** \brief Reads in the attack point's data from a script file
	*** \param script A reference to the open script file where to retrieve the data from
	*** \return True upon success, false upon failure.
	***
	*** There are two requirements for using this function. First, the script file must already
	*** be opened for reading permissions. Second, the table which contains the attack point data
	*** must be opened <b>prior</b> to making this function call. This function will not close the
	*** table containing the attack point when it finishes loading the data, so the calling routine
	*** must remember to close the table after this call is made.
	**/
	bool _LoadData(hoa_script::ReadScriptDescriptor& script);
}; // class GlobalAttackPoint : public GlobalTarget


/** ****************************************************************************
*** \brief Represents an actor that can participate in battles
***
*** This is an abstract parent class that both playable characters and enemies 
*** inherit from in order to provide a consistent interface to the statistics
*** that characters and enemies share.
***
*** \todo The copy constructor and copy assignment operator need to written to
*** avoid having two objects with the same set of pointers (to skills, equipment,
*** attack points, etc). Deleting both the original and then the copy will cause
*** a segmentation fault.
*** ***************************************************************************/
class GlobalActor : public GlobalTarget {
public:
	GlobalActor()
		{}

	virtual ~GlobalActor();

	GlobalActor(const GlobalActor& copy);

	GlobalActor& operator=(const GlobalActor& copy);

	GLOBAL_TARGET GetTargetType()
		{ return GLOBAL_TARGET_ACTOR; }

	/** \brief A purely virtual function used for actor type identification
	*** \return True if the actor is a character, false if it is an enemy
	***
	*** This function is implemented primarily because GlobalActor requires at
	*** least one purely virtual function to be an abstract class.
	*** FIX ME Obsolete.  Moved to BattleActor and now called IsEnemy().
	**/
	virtual bool IsCharacter() = 0;

	/** \brief Determines if the actor is "alive" and able to perform actions
	*** \return True if the character has a non-zero amount of hit points.
	**/
	bool IsAlive() const
		{ return (_hit_points != 0); }

	/** \name Class member get functions
	*** Some of these functions take an index argument to retrieve a particular
	*** attack point stat or piece of armor. If an invalid index is given, a zero
	*** or NULL value will be returned.
	**/
	//@{
	uint32 GetID() const
		{ return _id; }

	hoa_utils::ustring& GetName()
		{ return _name; }

	std::string& GetFilename()
		{ return _filename; }

	uint32 GetHitPoints() const
		{ return _hit_points; }

	uint32 GetMaxHitPoints() const
		{ return _max_hit_points; }

	uint32 GetSkillPoints() const
		{ return _skill_points; }

	uint32 GetMaxSkillPoints() const
		{ return _max_skill_points; }

	uint32 GetExperienceLevel() const
		{ return _experience_level; }

	uint32 GetExperiencePoints() const
		{ return _experience_points; }

	uint32 GetStrength() const
		{ return _strength; }

	uint32 GetVigor() const
		{ return _vigor; }

	uint32 GetFortitude() const
		{ return _fortitude; }

	uint32 GetProtection() const
		{ return _protection; }

	uint32 GetAgility() const
		{ return _agility; }

	float GetEvade() const
		{ return _evade; }

	uint32 GetTotalPhysicalAttack() const
		{ return _total_physical_attack; }

	uint32 GetTotalMetaphysicalAttack() const
		{ return _total_metaphysical_attack; }

	uint32 GetTotalPhysicalDefense(uint32 index) const;

	uint32 GetTotalMetaphysicalDefense(uint32 index) const;

	float GetTotalEvadeRating(uint32 index) const;

	GlobalWeapon* GetWeaponEquipped() const
		{ return _weapon_equipped; }

	std::vector<GlobalArmor*>* GetArmorEquipped()
		{ return &_armor_equipped; }

	GlobalArmor* GetArmorEquipped(uint32 index) const;

	std::vector<GlobalAttackPoint*>* GetAttackPoints()
		{ return &_attack_points; }

	GlobalAttackPoint* GetAttackPoint(uint32 index) const;

	std::map<uint32, GlobalSkill*>* GetSkills()
		{ return &_skills; }

	/** \brief Used to detect if a character knows a certain skill.
	*** \return A pointer to the skill if it is found, or NULL if the skill was not found
	**/
	GlobalSkill* GetSkill(uint32 skill_id) const;

	//! \brief An alternative GetSkill call that takes a skill pointer as an argument
	GlobalSkill* GetSkill(const GlobalSkill* skill) const;

	// TODO: elemental and status effects not yet available in game
// 	std::vector<GlobalElementalEffect*>& GetElementalAttackBonuses()
// 		{ return _elemental_attack_bonuses; }
// 
// 	std::vector<std::pair<float, GlobalStatusEffect*> >& GetStatusAttackBonuses()
// 		{ return _status_attack_bonuses; }
// 
// 	std::vector<GlobalElementalEffect*>& GetElementalDefenseBonuses()
// 		{ return _elemental_defense_bonuses; }
// 
// 	std::vector<std::pair<float, GlobalStatusEffect*> >& GetStatusDefenseBonuses()
// 		{ return _status_defense_bonuses; }
	//@}

	/** \name Class member set functions
	*** Normally you should not need to directly set these members, but rather add or subtract
	*** an amount from the current value of the member. Total attack, defense, or evade ratings
	*** are re-calculated when an appropriately related stat is changed.
	**/
	void SetExperienceLevel(uint32 xp_level)
		{ _experience_level = xp_level; }

	void SetExperiencePoints(uint32 xp_points)
		{ _experience_points = xp_points; }

	void SetHitPoints(uint32 hp)
		{ if (hp > _max_hit_points) _hit_points = _max_hit_points; else _hit_points = hp; }

	void SetMaxHitPoints(uint32 hp)
		{ _max_hit_points = hp; if (_hit_points > _max_hit_points) _hit_points = _max_hit_points; }

	void SetSkillPoints(uint32 sp)
		{ if (sp > _max_skill_points) _skill_points = _max_skill_points; else _skill_points = sp; }

	void SetMaxSkillPoints(uint32 sp)
		{ _max_skill_points = sp; if (_skill_points > _max_skill_points) _skill_points = _max_skill_points; }

	void SetStrength(uint32 st)
		{ _strength = st; _CalculateAttackRatings(); }

	void SetVigor(uint32 vi)
		{ _vigor = vi; _CalculateAttackRatings(); }

	void SetFortitude(uint32 fo)
		{ _fortitude = fo; _CalculateDefenseRatings(); }

	void SetProtection(uint32 pr)
		{ _protection = pr; _CalculateDefenseRatings(); }

	void SetAgility(uint32 ag)
		{ _agility = ag; }

	void SetEvade(float ev)
		{ _evade = ev; _CalculateEvadeRatings(); }
	//@}

	/** \name Class member add and subtract functions
	*** These methods provide a means to easily add or subtract amounts off of certain stats, such
	*** as hit points or stength. Total attack, defense, or evade ratings are re-calculated when
	*** an appropriately related stat is changed.
	**/
	//@{
	//! \note The current hit points is prevented from exceeding the maximum hit points possible
	void AddHitPoints(uint32 hp)
		{ _hit_points += hp; if (_hit_points > _max_hit_points) _hit_points = _max_hit_points; }

	//! \note If an overflow condition is detected, hit points will be set to zero
	void SubtractHitPoints(uint32 hp)
		{ if (hp < _hit_points) _hit_points -= hp; else _hit_points = 0; }

	//! \note The current hit points is also increased by the same amount as the max hit points
	void AddMaxHitPoints(uint32 hp)
		{ _max_hit_points += hp; _hit_points += hp; }

	//! \note The current skill points is prevented from exceeding the maximum skill points possible
	void AddSkillPoints(uint32 sp)
		{ _skill_points += sp; if (_skill_points > _max_skill_points) _skill_points = _max_skill_points; }

	//! \note If an overflow condition is detected, skill points will be set to zero
	void SubtractSkillPoints(uint32 sp)
		{ if (sp < _skill_points) _skill_points -= sp; else _skill_points = 0; }

	//! \note The current skill points is also increased by the same amount as the max skill points
	void AddMaxSkillPoints(uint32 sp)
		{ _max_skill_points += sp; _skill_points += sp; }

	void AddStrength(uint32 st)
		{ _strength += st; _CalculateAttackRatings(); }

	void AddVigor(uint32 vi)
		{ _vigor += vi; _CalculateAttackRatings(); }

	void AddFortitude(uint32 fo)
		{ _fortitude += fo; _CalculateDefenseRatings(); }

	void AddProtection(uint32 pr)
		{ _protection += pr; _CalculateDefenseRatings(); }

	void AddAgility(uint32 ag)
		{ _agility += ag; }

	void AddEvade(float ev)
		{ _evade += ev; _CalculateEvadeRatings(); }
	//@}

	/** \brief Equips a new weapon on the actor
	*** \param weapon The new weapon to equip on the actor
	*** \return A pointer to the weapon that was previouslly equipped, or NULL if no weapon was equipped.
	***
	*** This function will also automatically re-calculate all attack ratings, elemental, and status bonuses.
	**/
	GlobalWeapon* EquipWeapon(GlobalWeapon* weapon);

	/** \brief Equips a new armor on the actor
	*** \param armor The piece of armor to equip
	*** \param index The index into the _armor_equippd vector where to equip the armor
	*** \return A pointer to the armor that was previously equipped, or NULL if no armor was equipped
	***
	*** This function will also automatically re-calculate all defense ratings, elemental, and status bonuses.
	*** If the index argument is invalid (out-of-bounds), the function will return the armor argument.
	**/
	GlobalArmor* EquipArmor(GlobalArmor* armor, uint32 index);

	/** \brief Adds a new skill to the actor's skill set
	*** \param skill_id The id number of the skill to add
	***
	*** No skill may be added more than once. If this case is detected or an error occurs when trying
	*** to load the skill data, it will not be added.
	**/
	virtual void AddSkill(uint32 skill_id) = 0;

protected:
	//! \brief An identification number to represent the actor.
	uint32 _id;

	//! \brief The name of the actor as it will be displayed upon the screen.
	hoa_utils::ustring _name;

	//! \brief The filename based used to look up an actors image files and other data
	std::string _filename;

	//! \name Base Actor Statistics
	//@{
	//! \brief The current experience level of the actor.
	uint32 _experience_level;

	//! \brief The number of experience points the actor has earned.
	uint32 _experience_points;

	//! \brief The current number of hit points that the actor has.
	uint32 _hit_points;

	//! \brief The maximum number of hit points that the actor may have.
	uint32 _max_hit_points;

	//! \brief The current number of skill points that the actor has.
	uint32 _skill_points;

	//! \brief The maximum number of skill points that the actor may have.
	uint32 _max_skill_points;

	//! \brief Used to determine the actor's physical attack rating.
	uint32 _strength;

	//! \brief Used to determine the actor's metaphysical attack rating.
	uint32 _vigor;

	//! \brief Used to determine the actor's physical defense rating.
	uint32 _fortitude;

	//! \brief Used to determine the actor's metaphysical defense rating.
	uint32 _protection;

	//! \brief Used to calculate the time it takes to recover stamina in battles.
	uint32 _agility;

	//! \brief The attack evade percentage of the actor, ranged from 0.0 to 1.0
	float _evade;
	//@}

	//! \brief The sum of the character's strength and their weapon's physical attack
	uint32 _total_physical_attack;

	//! \brief The sum of the character's vigor and their weapon's metaphysical attack
	uint32 _total_metaphysical_attack;

	/** \brief The attack points that are located on the actor
	*** \note All actors must have at least one attack point.
	**/
	std::vector<GlobalAttackPoint*> _attack_points;

	/** \brief The weapon that the actor has equipped
	*** \note If no weapon is equipped, this member will be equal to NULL.
	***
	*** Actors are not required to have weapons equipped, and indeed most enemies will probably not have any
	*** weapons explicitly equipped. The various bonuses to attack ratings, elemental attacks, and status
	*** attacks are automatically added to the appropriate members of this class when the weapon is equipped,
	*** and likewise those bonuses are removed when the weapon is unequipped.
	**/
	GlobalWeapon* _weapon_equipped;

	/** \brief The various armors that the actor has equipped
	*** \note The size of this vector will always be equal to the number of attack points on the actor.
	***
	*** Actors are not required to have armor of any sort equipped. Note that the defense bonuses that
	*** are afforded by the armors are not directly applied to the character's defense ratings, but rather to
	*** the defense ratings of their attack points. However the elemental and status bonuses of the armor are
	*** applied to the character as a whole. The armor must be equipped on one of the actor's attack points to
	*** really afford any kind of defensive bonus.
	**/
	std::vector<GlobalArmor*> _armor_equipped;

	/** \brief A map containing all skills that the actor can use
	*** Unlike with characters, there is no need to hold the various types of skills in seperate containers
	*** for enemies. An enemy must have <b>at least</b> one skill in order to do anything useful in battle.
	**/
	std::map<uint32, GlobalSkill*> _skills;

	/** \brief The elemental effects added to the actor's attack
	*** Actors may carry various elemental attack bonuses, or they may carry none. These bonuses include
	*** those that are brought upon by the weapon that the character may have equipped.
	**/
// 	std::vector<GlobalElementalEffect*> _elemental_attack_bonuses;

	/** \brief The status effects added to the actor's attack
	*** Actors may carry various status attack bonuses, or they may carry none. These bonuses include
	*** those that are brought upon by the weapon that the character may have equipped. The first member
	*** in the pair is the likelihood (between 0.0 and 1.0) that the actor has of inflicting that status
	*** effect upon a targeted foe.
	**/
// 	std::vector<std::pair<float, GlobalStatusEffect*> > _status_attack_bonuses;

	/** \brief The elemental effects added to the actor's defense
	*** Actors may carry various elemental defense bonuses, or they may carry none. These bonuses include
	*** those that are brought upon by all of the armors that the character may have equipped.
	**/
// 	std::vector<GlobalElementalEffect*> _elemental_defense_bonuses;

	/** \brief The status effects added to the actor's defense
	*** Actors may carry various status defense bonuses, or they may carry none. These bonuses include
	*** those that are brought upon by the armors that the character may have equipped. The first member
	*** in the pair is the reduction in the likelihood (between 0.0 and 1.0) that the actor has of
	*** repelling an attack with a status effect.
	**/
// 	std::vector<std::pair<float, GlobalStatusEffect*> > _status_defense_bonuses;

	/** \brief Calculates an actor's physical and metaphysical attack ratings
	*** This function simply sums the actor's strength/vigor with their weapon's attack ratings
	*** and places the result in total physical/metaphysical attack members
	**/
	void _CalculateAttackRatings();

	/** \brief Calculates the physical and metaphysical defense ratings for each attack point
	*** This function is called whenever the fortitude or protection members are modified or
	*** armor equipment is changed. It will update the total defense members of each attack
	*** point that belongs to the actor.
	**/
	void _CalculateDefenseRatings();

	/** \brief Calculates the evade rating for each attack point
	*** This function is called whenever the evade member is modified. It updates the evade
	*** ratings of all attack points that belong to the actor.
	**/
	void _CalculateEvadeRatings();
}; // class GlobalActor : public GlobalTarget


/** ****************************************************************************
*** \brief A container class for tracking the growth of a character
***
*** This class is essentially an extension of the GlobalCharacter class that
*** manages and updates the character's growth. The primary reason that this
*** class exists is to provide an interface for external code to determine
*** when character growth occurs, inform the player of the growth, and
*** acknowleged the growth.
***
*** The recommended proceedure for processing character growth is as follows:
*** -# If the return value of GlobalCharacter::AddExperiencePoints is true, growth
***    has occured and should be processed.
*** -# Call GlobalCharacter::GetGrowth() to retrieve a pointer to this object
*** -# Call IsExperienceLevel() to determine whether the type growth is a new
***    experience level, or simply gradual growth.
*** -# If the growth type is gradual, call the various Growth() methods and
***    report any non-zero values to the player. Then call AcknoledgeGrowth()
*** -# Otherwise if the growth type is a new level, report growth plus any skills
***    learned and call AcknoledgeGrowth() (*see note)
***
*** \note When an experience level is gained, after the call to AcknowledgeGrowth()
*** there may be new growth available (because the character gained multiple 
*** experience levels or met the requirements for additional gradual growth for
*** the new experience level to gain). Thus, you should strongly consider calling
*** the IsGrowthDetected() method after AcknowledgeGrowth() to report any further
*** character growth that occured after the character reached a new level.
*** ***************************************************************************/
class GlobalCharacterGrowth {
	friend class GameGlobal;
	friend class GlobalCharacter;
	friend void hoa_defs::BindEngineToLua();

public:
	GlobalCharacterGrowth(GlobalCharacter* owner);

	~GlobalCharacterGrowth();

	/** \brief Processes any growth that has occured by modifier the character's stats
	*** If an experience level is gained, this function will open up the script file that contains
	*** the character's definition and get new growth stats for the next experience level.
	**/
	void AcknowledgeGrowth();

	//! \name Class member access functions
	//@{
	bool IsExperienceLevelGained() const
		{ return _experience_level_gained; }

	bool IsGrowthDetected() const
		{ return _growth_detected; }

	uint32 GetHitPointsGrowth() const
		{ return _hit_points_growth; }

	uint32 GetSkillPointsGrowth() const
		{ return _skill_points_growth; }

	uint32 GetStrengthGrowth() const
		{ return _strength_growth; }

	uint32 GetVigorGrowth() const
		{ return _vigor_growth; }

	uint32 GetFortitudeGrowth() const
		{ return _fortitude_growth; }

	uint32 GetProtectionGrowth() const
		{ return _protection_growth; }

	uint32 GetAgilityGrowth() const
		{ return _agility_growth; }

	float GetEvadeGrowth() const
		{ return _evade_growth; }

	std::vector<GlobalSkill*>* GetSkillsLearned()
		{ return &_skills_learned; }
	//@}

private:
	//! \brief A pointer to the character which this growth belongs to
	GlobalCharacter* _character_owner;

	//! \brief Set to true when it is detected that a new experience level has been reached
	bool _experience_level_gained;

	//! \brief Set to true when it is detected that sufficient experience for at least one stat to grow has been reached
	bool _growth_detected;

	//! \brief The experience points required to reach the next experience level
	uint32 _experience_for_next_level;

	//! \brief The experience points that were required to reach the previous experience level
	uint32 _experience_for_last_level;

	/** \brief The amount of growth that should be added to each of the character's stats
	*** These members are incremented by the _UpdateGrowth() function, which detects when a character
	*** has enough experience points to meet a growth requirement. They are all cleared to zero after
	*** a call to AcknowledgeGrowth()
	***
	*** \note These members are given read/write access in Lua so that Lua may use them to hold new
	*** growth amounts when a character reaches a new level. Refer to the function DetermineGrowth(character)
	*** defined in dat/actors/characters.lua
	**/
	//@{
	uint32 _hit_points_growth;
	uint32 _skill_points_growth;
	uint32 _strength_growth;
	uint32 _vigor_growth;
	uint32 _fortitude_growth;
	uint32 _protection_growth;
	uint32 _agility_growth;
	float _evade_growth;
	//@}

	/** \brief The periodic growth of the stats as a function of experience points
	*** The purpose of these containers is to support the gradual growth of characters.
	*** The first member in each pair is the experience points required for that growth
	*** to occur, while the second member is the value of the growth. Each entry in the
	*** deques are ordered from lowest (front) to highest (back) XP requirements. The
	*** final entry in each deque should be the growth for when the next experience
	*** level is reached. Note that these structures do not need to contain any entries
	*** (ie, a stat does not need to grow on every level).
	***
	*** These containers are emptied when a new experience level occurs, and are also
	*** re-constructed after the experience level gain has been acknowledged.
	**/
	//@{
	std::deque<std::pair<uint32, uint32> > _hit_points_periodic_growth;
	std::deque<std::pair<uint32, uint32> > _skill_points_periodic_growth;
	std::deque<std::pair<uint32, uint32> > _strength_periodic_growth;
	std::deque<std::pair<uint32, uint32> > _vigor_periodic_growth;
	std::deque<std::pair<uint32, uint32> > _fortitude_periodic_growth;
	std::deque<std::pair<uint32, uint32> > _protection_periodic_growth;
	std::deque<std::pair<uint32, uint32> > _agility_periodic_growth;
	std::deque<std::pair<uint32, float> > _evade_periodic_growth;
	//@}

	/** \brief Contains any and all skills that are to be learned when the next experience level is reached
	*** These are automatically added to the character by this class after the new experience level growth has been
	*** acknowledged.
	**/
	std::vector<GlobalSkill*> _skills_learned;

	/** \brief Adds a new skill for the character to learn at the next experience level gained
	*** \param skill_id The ID number of the skill to add
	*** \note This function is bound to and invoked by Lua to add all of the skills to be learned
	**/
	void _AddSkill(uint32 skill_id);

	/** \brief Examines if any growth has occured as a result of the character's experience points
	*** This is called by GlobalCharacter whenever the character's experience points change. If any growth is
	*** detected, the _growth_detected member is set and the various growth members of the class are incremented
	*** by the growth amount.
	**/
	void _CheckForGrowth();

	/** \brief Constructs the numerous periodic growth deques when growth stats for a new level are loaded in
	*** After new growth stats have been loaded in for a level, this function takes those values, breaks them
	*** apart, and spreads out their growth periodically. 50% of the growth is saved for when the character
	*** reaches a new level, while the other 50% are rewarded as the character's experience grows to values
	*** in between the previous experience level marker and the next.
	***
	*** \note The growth members should contain the total growth stats when this function is called. These
	*** members will be set back to zero by the time the function returns as their values will be split up
	*** and placed in numerous entries in the periodic_growth deques. All periodic_growth deques should be
	*** empty when this function is called.
	**/
	void _ConstructPeriodicGrowth();

	/** \brief An algorithm that computes how many experience points are needed to reach the next level
	*** This algorithm is a function of the current experience level and the experience points that
	*** were required to reach the current level. This function modifies the _experience_for_next_level
	*** and _experience_for_last_level members.
	**/
	void _DetermineNextLevelExperience();
}; // class GlobalCharacterGrowth


/** ****************************************************************************
*** \brief Represents playable game characters
***
*** This calls represents playable game characters only (those that you can control,
*** equip, and send into battle). It does not cover NPCs. This class also holds
*** numerous images that represent the character in a variety of contexts (maps, battle,
*** menus, etc.). This class may be used as a parent class by game modes that need
*** to add further data or methods to manipulate the character than is provided here.
***
*** \note A character has four, and only four, attack points. They are the head,
*** torso, arms, and legs of the character.
*** ***************************************************************************/
class GlobalCharacter : public GlobalActor {
	friend class GlobalCharacterGrowth;

public:
	/** \brief Constructs a new character from its definition in a script file
	*** \param id The integer ID of the character to create
	*** \param initial If true, the character's stats, equipment, and skills are set
	*** to the character's initial status
	*** \note If initial is set to false, the character's stats, equipment, and skills
	*** must be set by external code, otherwise they will remain 0/NULL/empty.
	**/
	GlobalCharacter(uint32 id, bool initial = true);

	virtual ~GlobalCharacter();

	bool IsCharacter()
		{ return true; }

	//! \name Public Member Access Functions
	//@{
	uint32 GetExperienceForNextLevel() const
		{ return _growth._experience_for_next_level; }

	GlobalCharacterGrowth* GetGrowth()
		{ return &_growth; }

	GlobalArmor* GetHeadArmorEquipped()
		{ return _armor_equipped[0]; }

	GlobalArmor* GetTorsoArmorEquipped()
		{ return _armor_equipped[1]; }

	GlobalArmor* GetArmArmorEquipped()
		{ return _armor_equipped[2]; }

	GlobalArmor* GetLegArmorEquipped()
		{ return _armor_equipped[3]; }

	std::vector<GlobalSkill*>* GetAttackSkills()
		{ return &_attack_skills; }

	std::vector<GlobalSkill*>* GetDefenseSkills()
		{ return &_defense_skills; }

	std::vector<GlobalSkill*>* GetSupportSkills()
		{ return &_support_skills; }
	//@}

	void AddSkill(uint32 skill_id);

	/** \brief Adds experience points to the character
	*** \param xp The amount of experience points to add
	*** \return True if the new experience points triggered character growth
	**/
	bool AddExperiencePoints(uint32 xp);

	GlobalArmor* EquipHeadArmor(GlobalArmor* armor)
		{ return EquipArmor(armor, GLOBAL_POSITION_HEAD); }

	GlobalArmor* EquipTorsoArmor(GlobalArmor* armor)
		{ return EquipArmor(armor, GLOBAL_POSITION_TORSO); }

	GlobalArmor* EquipArmArmor(GlobalArmor* armor)
		{ return EquipArmor(armor, GLOBAL_POSITION_ARMS); }

	GlobalArmor* EquipLegArmor(GlobalArmor* armor)
		{ return EquipArmor(armor, GLOBAL_POSITION_LEGS); }

	// TEMP: image accessor functions
	//@{
	void AddBattleAnimation(const std::string & name, hoa_video::AnimatedImage anim)
		{ _battle_animation[name] = anim; }

	hoa_video::AnimatedImage* RetrieveBattleAnimation(const std::string & name)
		{ return &_battle_animation[name]; }

	std::vector<hoa_video::StillImage>* GetBattlePortraits()
		{ return &_battle_portraits; }
	//@}

protected:
	/** \brief The skills that the character may use
	*** Skills are broken up into three types: attack, defense, and support. There is really no distinguishment
	*** between the various skill types, they just serve an organizational means and are used to identify a
	*** skill's general purpose/use. Characters keep their skills in these seperate containers because they
	*** are presented in this way to the player.
	**/
	//@{
	std::vector<GlobalSkill*> _attack_skills;
	std::vector<GlobalSkill*> _defense_skills;
	std::vector<GlobalSkill*> _support_skills;
	//@}

	/** \brief A manager object for monitoring the character's growth
	*** This object contains information such as what is required for the next level experience level to be reached,
	*** the amount that each stat will grow by on the next level, whether any new skills will be learned, etc. It
	*** is updated whenever the character's experience_points or experience_level are changed.
	**/
	GlobalCharacterGrowth _growth;

	/** \name Character Images
	*** \note Although many of the names of these members would imply that they are only used in one particular
	*** mode of operation (map, battle, etc.), these members may be freely used by different game modes for
	*** which they were not specifically designed for. The names are simply meant to indicate the primary game
	*** mode where the images are intended to be used.
	**/
	//@{
	/** \brief The standard frame images for the character's map sprite.
	*** This container holds the standard frames for the character's map sprite, which include standing and
	*** walking frames. This set includes 24 frames in total, 6 each for the down, up, left, and right
	*** orientations.
	**/
	std::vector<hoa_video::StillImage> _map_frames_standard;

	/** \brief The character's standard map portrait image
	*** The standard map portrait is ususally used in dialogues, but may also be used in other modes where
	*** appropriate. The size of the map portrait is 200x200 pixels.
	**/
	hoa_video::StillImage _map_portrait_standard;

	/** \brief The frame images for the character's battle sprite.
	*** This map container contains various animated images for the character's battle sprites. The key to the
	*** map is a simple string which describes the animation, such as "idle".
	**/
	std::map<std::string, hoa_video::AnimatedImage> _battle_animation;

	/** \brief The frame images for the character's battle portrait
	*** Each character has 5 battle portraits which represent the character's health with damage levels of 0%,
	*** 25%, 50%, 75%, and 100% (this is also the order in which the frames are stored, starting with the 0%
	*** frame at index 0). Thus, the size of this vector is always five elements. Each portait is 100x100
	*** pixels in size.
	**/
	std::vector<hoa_video::StillImage> _battle_portraits;

	/** \brief The character's full-body portrait image for use in menu mode
	*** This image is a detailed, full-scale portait of the character and is intended for use in menu mode.
	*** The size of the image is 150x350 pixels.
	**/
	hoa_video::StillImage _menu_portrait;
	//@}
}; // class GlobalCharacter : public GlobalActor


/** ****************************************************************************
*** \brief Represents those enemies that fight in battles
***
*** Allacrost handles enemies a little different than most RPGs. Instead of an
*** enemy always having the same statistics for health, strength, etc., enemies
*** are more closely matched to the player's experience levels and abilities.
*** In terms of the operation of this class, an enemy starts at level 0 with
*** various base statistics and is then grown to match a level close to the
*** current level of the player's characters, before it actually appears on
*** the field of battle. Furthermore, the enemy is grown using gaussian random
*** values to provide an element of uncertainty and to make the enemies that the
*** player encounters more unique and less static.
*** ***************************************************************************/
class GlobalEnemy : public GlobalActor {
public:
	GlobalEnemy(uint32 id);

	~GlobalEnemy();

	bool IsCharacter()
		{ return false; }

	/** \brief Enables the enemy to be able to use a new skill
	*** \param skill_id The integer ID of the skill to add to the enemy
	*** If the enemy already knows the skill to be added, it will not be added again
	*** and a warning message will be printed.
	*** \note Skills should <b>only</b> be added after the enemy has been initialized
	**/
	void AddSkill(uint32 skill_id);

	/** \brief Initializes the enemy and prepares it for battle
	*** \param xp_level The experience level to set the enemy to
	***
	*** This function sets the enemy's experience level, grows its stats to match this
	*** level, and adds any skills that the enemy should be capable of using at the
	*** level. Call this function once only, because after the enemy has skills enabled
	*** it will not be able to re-initialize. If you need to initialize the enemy once
	*** more, you'll have to create a brand new GlobalEnemy object and initialize that
	*** instead.
	**/
	void Initialize(uint32 xp_level);

	/** \brief Uses random variables to calculate which objects, if any, the enemy dropped
	*** \param objects A reference to a vector to hold the GlobalObject pointers
	***
	*** The objects vector is cleared immediately once this function is called so make sure
	*** that it does not hold anything meaningful. Any objects which are added to this
	*** vector are created with new GlobalObject(), so the object should be deleted or
	*** its pointer passed to the GlobalManager to later delete.
	**/
	void DetermineDroppedObjects(std::vector<GlobalObject*>& objects);

	//! \name Class member access functions
	//@{
	uint32 GetSpriteHeight() const
		{ return _sprite_height; }

	uint32 GetSpriteWidth() const
		{ return _sprite_width; }

	uint32 GetDrunesDropped() const
		{ return _drunes_dropped; }

	std::vector<hoa_video::StillImage>* GetBattleSpriteFrames()
		{ return &_battle_sprite_frames; }
	//@}

protected:
	//! \brief The dimensions of the enemy's battle sprite in pixels
	uint32 _sprite_width, _sprite_height;

	/** \name Growth Statistics
	*** \brief The average increase for statistics between experience levels is stored by these members
	***
	*** Note that even though the normal statistics members are integers, these are floating point values. This
	*** is so because it allows us a finer granularity of control over how much a particular statistic grows
	*** with time.
	**/
	//@{
	float _growth_hit_points;
	float _growth_skill_points;
	float _growth_experience_points;
	float _growth_strength;
	float _growth_vigor;
	float _growth_fortitude;
	float _growth_protection;
	float _growth_agility;
	float _growth_evade;
	float _growth_drunes;
	//@}

	//! \brief The amount of drunes that the enemy drops
	uint32 _drunes_dropped;

	/** \brief Dropped object vectors
	*** These three vectors are all of the same size. _dropped_objects contains the object IDs that the enemy
	*** may drop. _dropped_chance contains a value from 0.0f to 1.0f that determines the probability of the
	*** enemy dropping that object. And finally _dropped_level_required contains the minimum experience level
	*** that the enemy must be at in order to drop this object at all.
	**/
	//@{
	std::vector<uint32> _dropped_objects;
	std::vector<float> _dropped_chance;
	std::vector<uint32> _dropped_level_required;
	//@}

	/** \brief Contains all of the possible skills that the enemy may possess
	*** The map key is the id for the skill that the enemy may know. The value for each key is the
	*** experience level that the enemy is required to be at in order to effectively learn the skill.
	*** The elements in this map are added to the enemy's _skill vector of GlobalSkills when the 
	*** Initialize() function is called, and as long as the enemy meets the level requirements for
	*** adding the skill.
	**/
	std::map<uint32, uint32> _skill_set;

	/** \brief The battle sprite frames for the enemy
	*** Each enemy has four frames representing damage levels of 0%, 33%, 66%, and 100%. Thus, this vector
	*** always has four elements contained within it, where the 0th element is 0% damage, the 1st element
	*** is 33% damage, etc. This vector is used to load the multi-image containing the four sprite frames.
	**/
	std::vector<hoa_video::StillImage> _battle_sprite_frames;
}; // class GlobalEnemy : public GlobalActor


/** ****************************************************************************
*** \brief Represents a party of actors
***
*** This class is a container for a group or "party" of actors. A party is a type
*** of target for items and skills. The GameGlobal class also organizes characters
*** into parties for convienence. Note that an actor may be either an enemy or
*** a character, but you should avoid creating parties that contain both
*** characters and enemies, as it can lead to conflicts (for example, a character
*** and enemy which have the same ID value).
***
*** Parties may or may not allow duplicate actors (a duplicate actor is defined
*** as an actor that has the same _id member as another actor in the party).
*** This property is determined in the GlobalParty constructor
***
*** \note When this class is destroyed, the actors contained within the class are
*** <i>not</i> destroyed.
*** 
*** \note All methods which perform an operation by using an actor ID are
*** <b>only</b> valid to use if the party does not allow duplicates. 
*** ***************************************************************************/
class GlobalParty : public GlobalTarget {
public:
	/** \param allow_duplicates Determines whether or not the party allows duplicate
	*** actors to be added (default value == false)
	**/
	GlobalParty(bool allow_duplicates = false) :
		_allow_duplicates(allow_duplicates) {}

	~GlobalParty()
		{}

	/** \brief Adds an actor to the party
	*** \param actor A pointer to the actor to add to the party
	*** \param index The index where the actor should be inserted. If negative, actor is added to the end
	*** \note The actor will not be added if it is already in the party
	**/
	void AddActor(GlobalActor* actor, int32 index = -1);

	/** \brief Removes an actor from the party
	*** \param index The index of the actor in the party to remove
	*** \return A pointer to the actor that was removed, or NULL if the index provided was invalid
	**/
	GlobalActor* RemoveActorAtIndex(uint32 index);

	/** \brief Removes an actor from the party
	*** \param id The id value of the actor to remove
	*** \return A pointer to the actor that was removed, or NULL if the actor was not found in the party
	**/
	GlobalActor* RemoveActorByID(uint32 id);

	/** \brief Clears the internally stored actor pointers
	*** \note This function does not return the actor pointers, so if you wish to get the
	*** GlobalActors make sure you do so prior to invoking this call.
	**/
	void RemoveAllActors()
		{ _actors.clear(); }

	/** \brief Swaps the location of two actors in the party by their indeces
	*** \param first_index The index of the first actor to swap
	*** \param second_index The index of the second actor to swap
	**/
	void SwapActorsByIndex(uint32 first_index, uint32 second_index);

	/** \brief Swaps the location of two actors in the party by looking up their IDs
	*** \param first_id The id of the first actor to swap
	*** \param second_id The id of the second actor to swap
	**/
	void SwapActorsByID(uint32 first_id, uint32 second_id);

	/** \brief Replaces an actor in the party at a specified index with a new actor
	*** \param index The index of the actor to be replaced
	*** \param new_actor A pointer to the actor that will replace the existing actor
	*** \return A pointer to the replaced actor, or NULL if the operation did not take place
	**/
	GlobalActor* ReplaceActorByIndex(uint32 index, GlobalActor* new_actor);

	/** \brief Replaces an actor in the party with the specified id with a new actor
	*** \param id The id of the actor to be replaced
	*** \param new_actor A pointer to the actor that will replace the existing actor
	*** \return A pointer to the replaced actor, or NULL if the operation did not take place
	**/
	GlobalActor* ReplaceActorByID(uint32 id, GlobalActor* new_actor);

	/** \brief Computes the average experience level of all actors in the party
	*** \return A float representing the average experience level (0.0f if party is empty)
	**/
	float AverageExperienceLevel() const;

	/** \brief Retrieves a poitner to the actor in the party at a specified index
	*** \param index The index where the actor may be found in the party
	*** \return A pointer to the actor at the specified index, or NULL if the index argument was invalid
	**/
	GlobalActor* GetActorAtIndex(uint32 index) const
		{ if (index >= _actors.size()) return NULL; else return _actors[index]; }

	/** \brief Retrieves a poitner to the actor in the party with the spefified id
	*** \param id The id of the actor to return
	*** \return A pointer to the actor with the requested ID, or NULL if the actor was not found
	**/
	GlobalActor* GetActorByID(uint32 id) const;

	GLOBAL_TARGET GetTargetType()
		{ return GLOBAL_TARGET_PARTY; }

	bool IsPartyEmpty() const
		{ return (_actors.size() == 0); }

	bool IsAllowDuplicates() const
		{ return _allow_duplicates; }

	uint32 GetPartySize() const
		{ return _actors.size(); }

	std::vector<GlobalActor*>* GetAllActors()
		{ return &_actors; }

private:
	/** \brief The vector of actors that are in this party
	*** The class will not create nor destroy GlobalActor pointers stored in this data structure.
	**/
	std::vector<GlobalActor*> _actors;

	//! \brief If true, actors are allowed to be inserted into the party multiple times
	bool _allow_duplicates;
}; // class GlobalActorParty : public GlobalTarget

} // namespace hoa_global

#endif // __GLOBAL_ACTORS_HEADER__
