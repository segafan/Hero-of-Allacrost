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

/** \name GlobalItem and GlobalSkill Alignment values
*** \brief Enum values used for telling us whether they target friends, foes, or both
**/
enum GLOBAL_ALIGNMENT {
	GLOBAL_ALIGNMENT_INVALID = -1,
	GLOBAL_ALIGNMENT_GOOD    =  0,
	GLOBAL_ALIGNMENT_BAD	 =  1,
	GLOBAL_ALIGNMENT_NEUTRAL =  2,
	GLOBAL_ALIGNMENT_TOTAL   =  3
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
	GlobalAttackPoint()
		{}

	~GlobalAttackPoint()
		{}

	GLOBAL_TARGET GetTargetType()
		{ return GLOBAL_TARGET_ATTACK_POINT; }


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
	bool LoadData(hoa_script::ReadScriptDescriptor& script);

	//! \name Class Member Access Functions
	//@{
	hoa_utils::ustring GetName() const
		{ return _name; }

	uint16 GetXPosition() const
		{ return _x_position; }

	uint16 GetYPosition() const
		{ return _y_position; }

	uint32 GetFortitudeBonus() const
		{ return _fortitude_bonus; }

	uint32 GetProtectionBonus() const
		{ return _protection_bonus; }

	float GetEvadeBonus() const
		{ return _evade_bonus; }
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

	/** \brief The defense and evasion bonuses for this attack point
	***
	*** These are called "bonuses" because they are added to the fortitude, protection, and evade ratings of the
	*** actor which the attack point is a part of. These members may only be zero or positive -- never negative.
	*** They are percentages which are applied to the actor's native defense and evasion ratings. For example, 
	*** setting the fortitude bonus to 25.0f increases the actor's fortitude on this attack point by 25%.
	**/
	//@{
	uint32 _fortitude_bonus;
	uint32 _protection_bonus;
	float _evade_bonus;
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
}; // class GlobalAttackPoint


/** ****************************************************************************
*** \brief Represents an actor that can participate in battles
***
*** This is an abstract parent class that both playable characters and enemies 
*** inherit from to provide a consistent interface.
***
*** \note This class does not include any GlobalSkill objects. That is reserved
*** for the inherited class to implement as appropriate.
*** ***************************************************************************/
class GlobalActor : public GlobalTarget {
public:
	GlobalActor()
		{}

	virtual ~GlobalActor()
		{}

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

	// TEMP
	void SetName(hoa_utils::ustring na)
		{ _name = na; }

	//! \name Class member access functions
	//@{
	uint32 GetID() const
		{ return _id; }

	hoa_utils::ustring GetName() const
		{ return _name; }

	uint32 GetHitPoints() const
		{ return _hit_points; }

	uint32 GetMaxHitPoints() const
		{ return _max_hit_points; }

	uint32 GetSkillPoints() const
		{ return _skill_points; }

	uint32 GetMaxSkillPoints() const
		{ return _max_skill_points; }

	std::map<uint32, GlobalSkill*> GetSkills() const
		{ return _skills; }

	uint32 GetExperienceLevel() const
		{ return _experience_level; }

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

	uint32 GetPhysicalAttackRating() const
		{ return _physical_attack_rating; }

	uint32 GetMetaphysicalAttackRating() const
		{ return _metaphysical_attack_rating; }

	GlobalWeapon* GetWeaponEquipped() const
		{ return _weapon_equipped; }

	std::vector<GlobalArmor*> GetArmorEquipped() const
		{ return _armor_equipped; }

	std::vector<GlobalAttackPoint*> GetAttackPoints() const
		{ return _attack_points; }

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

	void SetHitPoints(uint32 hp)
		{ if (hp > _max_hit_points) _hit_points = _max_hit_points; else _hit_points = hp; }

	void AddHitPoints(uint32 hp)
		{ _hit_points += hp; if (_hit_points > _max_hit_points) _hit_points = _max_hit_points; }

	void SetSkillPoints(uint32 sp)
		{ if (sp > _max_skill_points) _skill_points = _max_skill_points; else _skill_points = sp; }

	void AddSkillPoints(uint32 sp)
		{ _skill_points += sp; if (_skill_points > _max_skill_points) _skill_points = _max_skill_points; }

	void SetMaxHitPoints(uint32 hp)
		{ _max_hit_points = hp; }

	void SetMaxSkillPoints(uint32 sp)
		{ _max_skill_points = sp; }

	void SetExperienceLevel(uint32 xp_level)
		{ _experience_level = xp_level; }

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

	/** \brief Determines if the actor is "alive" and able to take action in battle
	*** \return True if the character has a non-zero amount of hit points, otherwise false.
	**/
	bool IsAlive() const
		{ return (_hit_points != 0); }

	/** \brief Equips a new weapon on the actor
	*** \param weapon The new weapon to equip on the actor
	***
	*** This function will also automatically re-calculate all attack ratings, elemental, and status bonuses.
	*** NULL will be returned if no weapon is already equipped.
	**/
	void EquipWeapon(GlobalWeapon* weapon);

	/** \brief Equips a new armor on the actor
	*** \param armor The piece of armor to equip
	***
	*** This function will also automatically re-calculate all defense ratings, elemental, and status bonuses.
	*** If the attack point index is invalid, the function will return the armor argument. NULL will be returned
	*** if no armor is already equipped on the attack point.
	**/
	void EquipArmor(GlobalArmor* armor);

protected:
	//! \brief An identification number to represent the actor.
	uint32 _id;

	//! \brief The name of the actor as it will be displayed upon the screen.
	hoa_utils::ustring _name;

	//! \brief The filename that the actor's images use (no extension)
	//hoa_utils::ustring _filename;

	//! \name Base Actor Statistics
	//@{
	//! \brief The current experience level of the actor.
	uint32 _experience_level;

	//! \brief The current number of hit points that the actor has.
	uint32 _hit_points;

	//! \brief The maximum number of hit points that the actor may have.
	uint32 _max_hit_points;

	//! \brief The current number of skill points that the actor has.
	uint32 _skill_points;

	//! \brief The maximum number of skill points that the actor may have.
	uint32 _max_skill_points;

	//! \brief The strength index of the actor, used to determine physical attack rating.
	uint32 _strength;

	//! \brief The vigor index of the actor, used to determine metaphysical attack rating.
	uint32 _vigor;

	//! \brief The fortitude index of the actor, used to determine physical defense ratings.
	uint32 _fortitude;

	//! \brief The protection index of the actor, used in part to determine metaphysical defense ratings.
	uint32 _protection;

	//! \brief The agility of the actor, used to calculate the time it takes to recover stamina.
	uint32 _agility;

	//! \brief The attack evade percentage of the actor, ranged from 0.0 to 1.0
	float _evade;
	//@}

	//! \brief The sum of the character's strength and their weapon's physical attack
	uint32 _physical_attack_rating;

	//! \brief The sum of the character's vigor and their weapon's metaphysical attack
	uint32 _metaphysical_attack_rating;


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
	//@}


	/** \brief Defense and evade rating totals for all attack points
	*** \note The size of these vectors will always be equal to the number of attack points on the actor.
	***
	*** These members hold the total sums of physical defense, metaphyiscal defense, and evade percentage for
	*** each attack point. The sums are created from three sources: the actor's base defense and evade ratings,
	*** the bonuses that each attack point affords, and the defense ratings provided by any armor that is
	*** equipped on the attack point.
	**/
	//@{
	std::vector<uint32> _physical_defense_ratings;
	std::vector<uint32> _metaphysical_defense_ratings;
	std::vector<float>  _evade_ratings;
	//@}

	/** \brief The attack points that are located on the actor
	***
	*** All actors must have at least one attack point.
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
	*** and places the result in the classes two attack ratings members.
	**/
	void _CalculateAttackRatings();

	/** \brief Calculates the physical and metaphysical defense ratings for each attack point
	*** This function is called whenever the fortitude or protection members are modified.
	*** It will update the defense ratings of each attack point that belongs to the actor.
	**/
	void _CalculateDefenseRatings();

	/** \brief Calculates the evade rating for each attack point
	*** This function is called whenever the evade member is modified. It updates the evade
	*** ratings of all attack points that belong to the actor.
	**/
	void _CalculateEvadeRatings();
}; // class GlobalActor



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
public:
	GlobalCharacter(uint32 id);

	virtual ~GlobalCharacter();

	bool IsCharacter()
		{ return true; }

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	std::string GetFilename() const
		{ return _filename; }

	uint32 GetExperienceForNextLevel() const
		{ return _experience_next_level; }

	/*GlobalWeapon* GetWeapon() const
		{ return _weapon_equipped; }*/

	GlobalArmor* GetEquippedHeadArmor()
		{ return _armor_equipped[0]; }

	GlobalArmor* GetEquippedTorsoArmor()
		{ return _armor_equipped[1]; }

	GlobalArmor* GetEquippedArmsArmor()
		{ return _armor_equipped[2]; }

	GlobalArmor* GetEquippedLegArmor()
		{ return _armor_equipped[3]; }

	std::vector<GlobalSkill*>* GetAttackSkills()
		{ return &_attack_skills; }

	std::vector<GlobalSkill*>* GetDefenseSkills()
		{ return &_defense_skills; }

	std::vector<GlobalSkill*>* GetSupportSkills()
		{ return &_support_skills; }

	void SetExperienceNextLevel(uint32 xp_next)
		{ _experience_next_level = xp_next; }
	//@}

	/*
	* \brief Adds the given number of levels to the character
	* \param the number of levels to increase the character by
	*/
	void AddExperienceLevel(uint32 lvl = 1);

	/*
	* \brief Adds experience to the character
	* \param xp the amount of experience to add
	* \return true if he leveled up, flase if he did not
	*/
	bool AddXP(uint32 xp);

	void AddSkill(uint32 skill_id);

	void AddBattleAnimation(const std::string & name, hoa_video::AnimatedImage anim)
		{ _battle_animation[name] = anim; }

	hoa_video::AnimatedImage* RetrieveBattleAnimation(const std::string & name)
		{ return &_battle_animation[name]; }

	std::vector<hoa_video::StillImage>* GetBattlePortraits()
		{ return &_battle_portraits; }

protected:

	//! \brief The name used to retrieve the characters's data and other information from various sources
	std::string _filename;

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

	//! \brief The number of experience points needed to reach the next experience level.
	uint32 _experience_next_level;

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
*** In terms of the operation of this class, an enemy starts at level 1 with
*** various base statistics and is then grown to match a level close to the
*** current level of the player's characters, before it actually appears on
*** the field of battle. Furthermore, the enemy is grown using gaussian random
*** values to provide an element of uncertainty and to make the enemies that the
*** player encounters more unique and less static. This class may be needed to
*** be used as a parent class if a game mode has specific methods or data related
*** to the enemy that is not already provided here.
*** ***************************************************************************/
class GlobalEnemy : public GlobalActor {
public:
	GlobalEnemy(uint32 id);

	virtual ~GlobalEnemy();

	bool IsCharacter()
		{ return false; }

	//! \name Class member access functions
	//@{
	uint32 GetExperiencePoints() const
		{ return _experience_points; }

	uint32 GetMoney() const
		{ return _money; }

	uint32 GetItemDropped() const
		{ return _item_dropped; }

	float GetChanceToDrop() const
		{ return _chance_to_drop; }

	std::string GetFilename() const
		{ return _filename; }
	//@}

	//! \brief Returns the enemy's sprite frames
	std::vector<hoa_video::StillImage>* GetSpriteFrames()
		{ return &_sprite_frames; }

	//! \brief Returns the height of the enemy
	int GetHeight() const
	{ return static_cast<int>(_sprite_frames[0].GetHeight()); }

	/** \brief Simulates the growth of the enemy from the base experience level.
	*** \param level The experience level to set the enemey to
	***
	*** This function "simulates" the growth of an enemy from a base experience level (level 0) to
	*** the specified level in the argument. It does this by starting with the basic enemy statistics
	*** (maximum hit points, etc.). Every statistic has a related "growth" member, which indicates how
	*** much that statistic grows per-level on average. The growth member is multiplied by the function's
	*** level argument, and this result is applied to the GaussianRandomValue function (located in the
	*** utils code) with a standard deviation of 10% of the result. The GaussianRandomValue function returns
	*** the number to set the enemy's statistic to.
	**/
	void LevelSimulator(uint32 level);

protected:
	//! \brief The name used to retrieve the enemy's data and other information
	std::string _filename;

	//! \brief The number of experience points that the party is given when the enemy is defeated
	uint32 _experience_points;

	//! \brief The item the enemy drops when defeated
	uint32 _item_dropped;

	//! \brief Amount of money the enemy drops
	uint32 _money;

	//! \brief The chance an enemy has to drop an item
	float _chance_to_drop;

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
	//@}

	/** \brief The dimensions of the enemy sprite
	*** The units are in number of 64 pixels. For example, a width of 1 and a height of 2 is equal to a
	*** sprite that is 64 pixels wide by 128 pixels tall. These units are for simplification, since there
	*** exists a "virtual" grid in the battle scene, and all enemy sprite sizes must be in number of
	*** battle grid units. These members help the battle code to determine how and where to place all of
	*** the enemy battle sprites.
	**/
	uint32 _sprite_width, _sprite_height;

	/** \brief The battle sprite frames for the enemy
	*** Each enemy has four frames representing damage levels of 0%, 33%, 66%, and 100%. Thus, this vector
	*** always has four elements contained within it, where the 0th element is 0% damage, the 1st element
	*** is 33% damage, etc. This vector is used to load the multi-image containing the four sprite frames.
	**/
	std::vector<hoa_video::StillImage> _sprite_frames;
}; // class GlobalEnemy : public GlobalActor


/** ****************************************************************************
*** \brief Represents a party of actors
***
*** This class is a container for a group or "party" of actors. The purpose of
*** this class is tied mostly to convience for the GameGlobal class, as characters
*** may need to be organized into groups, and also because multiple actors can serve as
*** a target for items or skills.
***
*** \note When this class is destroyed, the characters contained within the class are
*** <i>not</i> destroyed. Do not assume otherwise.
*** ***************************************************************************/
class GlobalParty : public GlobalTarget {
public:
	GlobalParty()
		{}

	~GlobalParty()
		{}

	GLOBAL_TARGET GetTargetType()
		{ return GLOBAL_TARGET_PARTY; }

	//! \name Class member access functions
	//@{
	bool IsPartyEmpty() const
		{ return (_actors.size() == 0); }

	uint32 GetPartySize() const
		{ return _actors.size(); }

	GlobalActor* GetActor(uint32 index) const
		{ if (index >= _actors.size() || _actors.size() == 0) return NULL; else return _actors[index]; }

	std::vector<GlobalActor*> GetAllActors() const
		{ return _actors; }
	//@}

	/** \brief Adds an actor to the party
	*** \param character A pointer to the character to add
	***
	*** Note that duplicates of actors are not checked here.
	**/
	void AddActor(GlobalActor* actor)
		{ _actors.push_back(actor); }

	/** \brief Removes a character from the party
	*** \param character The index of the character to remove
	*** \return A pointer to the character that was removed, or NULL if the character was not found in the party.
	*** \note 
	**/
	GlobalActor* RemoveActor(uint32 index);

	/** \brief Clears the internally stored actor pointers
	*** Note that this function does not return the actors, so if you wish to get the GlobalActor pointers make
	*** sure you do so prior to invoking this call.
	**/
	void RemoveAllActors()
		{ _actors.clear(); }

private:
	/** \brief The vector of actors that are in this party
	*** The class will not create nor destroy GlobalActor pointers stored in this data structure.
	**/
	std::vector<GlobalActor*> _actors;
}; // class GlobalActorParty : public GlobalTarget

} // namespace hoa_global

#endif // __GLOBAL_ACTORS_HEADER__
