////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
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

#include "global_objects.h"
#include "global_skills.h"

namespace hoa_global {

//! \name Game Character Types
//@{
const uint32 GLOBAL_CHARACTER_NONE        = 0x00000000;
const uint32 GLOBAL_CHARACTER_CLAUDIUS    = 0x00000001;
const uint32 GLOBAL_CHARACTER_LAILA       = 0x00000002;
const uint32 GLOBAL_CHARACTER_ALL         = 0xFFFFFFFF;
//@}



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
class GlobalAttackPoint {
public:
	GlobalAttackPoint();
	GlobalAttackPoint(const hoa_utils::ustring & name, uint16 x, uint16 y);

	~GlobalAttackPoint();

	//! \name Class Member Access Functions
	//@{
	hoa_utils::ustring GetName() const
		{ return _name; }
	uint16 GetXPosition() const
		{ return _x_position; }
	uint16 GetYPosition() const
		{ return _y_position; }
	float GetEvade() const
		{ return _evade; }
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
	//@{
	uint16 _x_position;
	uint16 _y_position;
	//@}

	/** \brief The defense ratings that the attack point has
	*** The values of these members may also include the defense ratings from an equipped piece of armor.
	**/
	//@{
	uint32 _physical_defense;
	uint32 _metaphysical_defense;
	//@}

	/** \brief The attack evade percentage for the attack point
	*** This member ranges from 0.0 to 1.0 and determines how likely it is that an attempt to attack the
	*** attack point will fail. The higher the evasion rate, the more likely it is that an aggressor's
	*** attack will miss this target.
	**/
	float _evade;

	/** \brief A vector containing all status effects that may be triggered by attacking the point
	*** This vector contains only the status effects that have a non-zero chance of affecting their target. Therefore,
	*** it is very possible that this vector may be empty. The first element in the pair is a floating point value from 0.0
	*** to 1.0 that indicates the likelihood of success that should the attack point be successfully attacked by an opponent,
	*** the status effect becomes triggered on the enemy. Note that this likelihood
	*** does not take into account that the target may have a particular defense or immunity against the status effect.
	**/
	std::vector<std::pair<float, GlobalStatusEffect*> > _status_weaknesses;

	//! \brief A pointer to the actor which the attack point belongs to
	GlobalActor* _actor;

	/** \brief The armor that is protecting the attack point, if any
	*** The armor equipped provides changes to the physical and metaphysical defense members. When it is
	*** unequipped, the amount of protection that it provided is subtracted from the defense members.
	*** Usually enemies will not have armor equipped (instead their attack points are just given high defense
	*** values), but characters will almost always have armor equipped.
	**/
	GlobalArmor* _armor_equipped;
}; // class GlobalAttackPoint



/** ****************************************************************************
*** \brief Represents an actor that can participate in battles
***
*** This is a parent class that both playable characters and enemies inherit from to
*** provide a consistent interface between the two and to effectively re-use code.
*** It is intended to be an abstract class and therefmore an instance of this class
*** should never exist on its own.
*** 
*** \note The constructor for this class is in the protected space because the class
*** is abstract, and should only be able to be created by the classes which inherit
*** from it.
***
*** \note This class does not include any GlobalSkill objects. That is reserved 
*** for the inherited class to implement as appropriate.
*** ***************************************************************************/
class GlobalActor {
public:
	virtual ~GlobalActor();

	//! \name Class member access functions
	//@{
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
	uint32 GetStrength() const
		{ return _strength; }
	uint32 GetVigor() const
		{ return _vigor; }
	uint32 GetFortitude() const
		{ return _fortitude; }
	uint32 GetResistance() const
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
	std::vector<GlobalElementalEffect*>& GetElementalAttackBonuses()
		{ return _elemental_attack_bonuses; }
	std::vector<std::pair<float, GlobalStatusEffect*> >& GetStatusAttackBonuses()
		{ return _status_attack_bonuses; }
	std::vector<GlobalElementalEffect*>& GetElementalDefenseBonuses()
		{ return _elemental_defense_bonuses; }
	std::vector<std::pair<float, GlobalStatusEffect*> >& GetStatusDefenseBonuses()
		{ return _status_defense_bonuses; }

	hoa_utils::ustring GetName() const
		{ return _name; }

	void SetHitPoints(uint32 hp)
		{ if (hp > _max_hit_points) _hit_points = _max_hit_points; else _hit_points = hp; }
	void SetSkillPoints(uint32 sp)
		{ if (sp > _max_skill_points) _skill_points = _max_skill_points; else _skill_points = sp; }
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
	void SetName(const hoa_utils::ustring & name)
		{ _name = name; }
	//@}

	/** \brief Determines if a character is "alive" and can fight in battles
	*** \return True if the character has a non-zero amount of hit points, otherwise false.
	**/
	bool IsAlive() const
		{ return (_hit_points != 0); }

	//! \name Class member add and sub functions
	//@{
	void AddHitPoints(uint32 hp)
		{ _hit_points += hp; if (_hit_points > _max_hit_points) _hit_points = _max_hit_points; }
	void AddSkillPoints(uint32 sp)
		{ _skill_points += sp; if (_skill_points > _max_skill_points) _skill_points = _max_skill_points; }
	void AddMaxHitPoints(uint32 hp)
		{ _max_hit_points += hp; }
	void AddMaxSkillPoints(uint32 sp)
		{ _max_skill_points += sp; }
	void AddExperienceLevel(uint32 xp_level)
		{ _experience_level += xp_level; }
	void AddStrength(uint32 st)
		{ _strength += st; }
	void AddVigor(uint32 vi)
		{ _vigor += vi; }
	void AddFortitude(uint32 fo)
		{ _fortitude += fo; }
	void AddProtection(uint32 pr)
		{ _protection += pr; }
	void AddAgility(uint32 ag)
		{ _agility += ag; }
	//@}

	/** \brief Equips a new weapon on the actor
	*** \param weapon The new weapon to equip on the actor
	*** \return The weapon that was previously equipped on the actor
	*** This function will also automatically re-calculate all attack ratings, elemental, and status bonuses.
	*** NULL will be returned if no weapon is already equipped.
	**/
	GlobalWeapon* EquipWeapon(GlobalWeapon* weapon);
	/** \brief Equips a new armor on the actor
	*** \param armor The piece of armor to equip
	*** \param ap_index The actor's attack point index to equip the armor on
	*** \return The armor that was previously equipped on the attack point
	*** This function will also automatically re-calculate all defense ratings, elemental, and status bonuses.
	*** If the attack point index is invalid, the function will return the armor argument. NULL will be returned
	*** if no armor is already equipped on the attack point.
	**/
	GlobalArmor* EquipArmor(GlobalArmor* armor, uint32 ap_index);

protected:
	GlobalActor();

	//! \brief The name of the actor as it will be displayed upon the screen
	hoa_utils::ustring _name;

	//! \name Basic Actor Stats
	//@{
	//! \brief The current number of hit points that the actor has
	uint32 _hit_points;
	//! \brief The maximum number of hit points that the actor may have
	uint32 _max_hit_points;
	//! \brief The current number of skill points that the actor has
	uint32 _skill_points;
	//! \brief The maximum number of skill points that the actor may have
	uint32 _max_skill_points;
	//! \brief The current experience level of the actor
	uint32 _experience_level;
	//! \brief The strength index of the actor, used in part to determine physical attack rating
	uint32 _strength;
	//! \brief The vigor index of the actor, used in part to determine metaphysical attack rating
	uint32 _vigor;
	//! \brief The fortitude index of the actor, used in part to determine physical defense ratings
	uint32 _fortitude;
	//! \brief The protection index of the actor, used in part to determine metaphysical defense ratings.
	uint32 _protection;
	//! \brief The agility of the actor, used to calculate the speed in which stamina recovers
	uint32 _agility;
	//! \brief The attack evade percentage of the actor, ranged from 0.0 to 1.0
	float _evade;
	//@}

	//! \brief The sum of the character's strength and their weapon's physical attack
	uint32 _physical_attack_rating;
	//! \brief The sum of the character's vigor and their weapon's metaphysical attack
	uint32 _metaphysical_attack_rating;

	/** \brief The weapon that the actor has equipped
	*** Actors are not required to have weapons equipped, and indeed most enemies will probably not have any
	*** weapons explicitly equipped. The various bonuses to attack ratings, elemental attacks, and status
	*** attacks are automatically added to the appropriate members of this class when the weapon is equipped,
	*** and likewise those bonuses are removed when the weapon is unequipped.
	**/
	GlobalWeapon* _weapon_equipped;
	/** \brief The various armors that the actor has equipped
	*** Again, actors are not required to have armor of any sort equipped. Note that the defense bonuses that
	*** are afforded by the armors are not directly applied to the character's defense ratings, but rather to
	*** the defense ratings of their attack points. However the elemental and status bonuses of the armor are
	*** applied to the character as a whole. The armor must be equipped on one of the actor's attack points to
	*** really afford any kind of defensive bonus.
	*** 
	*** \note The size of this vector is always equal to the size of the _attack_points vector
	**/
	std::vector<GlobalArmor*> _armor_equipped;
	/** \brief The attack points that are located on the actor
	*** Every actor <b>must have at least one</b> attack point, otherwise it is impossible for that actor to
	*** be harmed in battle. The attack points carry the defense bonuses from the armor that the actor has
	*** equipped, if any.
	**/
	std::vector<GlobalAttackPoint*> _attack_points;

	/** \brief The elemental effects added to the actor's attack
	*** Actors may carry various elemental attack bonuses, or they may carry none. These bonuses include
	*** those that are brought upon by the weapon that the character may have equipped.
	**/
	std::vector<GlobalElementalEffect*> _elemental_attack_bonuses;
	/** \brief The status effects added to the actor's attack
	*** Actors may carry various status attack bonuses, or they may carry none. These bonuses include
	*** those that are brought upon by the weapon that the character may have equipped. The first member
	*** in the pair is the likelihood (between 0.0 and 1.0) that the actor has of inflicting that status
	*** effect upon a targeted foe.
	**/
	std::vector<std::pair<float, GlobalStatusEffect*> > _status_attack_bonuses;
	/** \brief The elemental effects added to the actor's defense
	*** Actors may carry various elemental defense bonuses, or they may carry none. These bonuses include
	*** those that are brought upon by all of the armors that the character may have equipped.
	**/
	std::vector<GlobalElementalEffect*> _elemental_defense_bonuses;
	/** \brief The status effects added to the actor's defense
	*** Actors may carry various status defense bonuses, or they may carry none. These bonuses include
	*** those that are brought upon by the armors that the character may have equipped. The first member
	*** in the pair is the reduction in the likelihood (between 0.0 and 1.0) that the actor has of
	*** repelling an attack with a status effect.
	**/
	std::vector<std::pair<float, GlobalStatusEffect*> > _status_defense_bonuses;

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
	*** ratings of all attack poitns that belong to the actor.
	**/
	void _CalculateEvadeRatings();
}; // class GlobalActor



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
	GlobalEnemy(const std::string & file_name);
	virtual ~GlobalEnemy();

	//! \name Class member access functions
	//@{
	std::string GetFilename() const
		{ return _filename; }
	uint32 GetExperiencePoints() const
		{ return _experience_points; }
	std::vector<GlobalSkill*> GetSkills() const
		{ return _enemy_skills; }
	//@}

	//! Returns the enemy's sprite frames
	std::vector<hoa_video::StillImage> * GetSpriteFrames()
	{ return &_sprite_frames; }

	//! \brief Gives the enemy a new skill to use in battle
	void AddSkill(GlobalSkill *skill)
		{ if (skill != NULL) _enemy_skills.push_back(skill); }

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
	//! \brief An unique identification number for the enemy type.
	uint32 _enemy_id;

	/** \brief The dimensions of the enemy sprite
	*** The units are in numbers of 64 pixels. For example, a width of 1 and a height of 2 is equal to a
	*** sprite that is 64 pixels wide by 128 pixels tall. These units are for simplification, since there
	*** exists a "virtual" grid in the battle scene, and all enemy sprite sizes must be in number of
	*** battle grid units. These members help the battle code to determine how and where to place all of
	*** the enemy battle sprites.
	**/
	//@{
	uint32 _sprite_width;
	uint32 _sprite_height;
	//@}
	
	// TODO CHECK Raging_Hog: These variables demanded by compiler. No clue what they do.
	// Please check especially the data type!
	//std::string _sprite_animations[];
	
	uint32 _movement_speed;
	uint32 _base_hit_points;
	uint32 _base_skill_points;
	uint32 _base_experience_points;
	uint32 _base_strength;
	uint32 _base_intelligence;
	uint32 _base_agility;

	/** \brief The set of skills that the enemy may choose from to take actions.
	*** Unlike with characters, there is no need to discern between the various types of skills for enemies.
	*** An enemy must have <b>at least</b> one skill in order to do anything useful in battle.
	**/
	std::vector<GlobalSkill*> _enemy_skills;
	/** \brief The battle sprite frames for the enemy
	*** Each enemy has four frames representing damage levels of 0%, 33%, 66%, and 100%. Thus, this vector
	*** always has four elements contained within it, where the 0th element is 0% damage, the 1st element
	*** is 33% damage, etc.
	**/
	std::vector<hoa_video::StillImage> _sprite_frames;

	//! \brief The number of experience points that the party is given when the enemy is defeated
	uint32 _experience_points;

	/** \name Growth Statistics
	*** \brief The average increase for statistics between experience levels is stored by these members
	*** Note that even though the normal statistics members are integers, these are floating point values. This
	*** is so because it allows us a finer granularity of control over how much a particular statistic grows
	*** with time.
	**/
	//@{
	float _growth_max_hit_points;
	float _growth_max_skill_points;
	float _growth_strength;
	float _growth_vigor;
	float _growth_fortitude;
	float _growth_resistance;
	float _growth_agility;
	float _growth_evade;
	float _growth_experience_points;

	// TODO CHECK Raging_Hog: Added these. Not sure if they're the same as _growth_max_hit_points or _growth_max_skill_points.
	//
	float _growth_hit_points;
	float _growth_skill_points;

	float _growth_intelligence;

	// TODO CHECK Raging_Hog: Should this be _growth_resistance?
	uint32 _resistance;
	//@}
}; // class GlobalEnemy : public GlobalActor



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
	GlobalCharacter(const hoa_utils::ustring & name, const std::string & filename, uint32 id);
	virtual ~GlobalCharacter();

	//@{
	//! \brief Get the equipment on the character
	//! \return the equipped weapon or armor

	//@}

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	std::string GetFilename() const
		{ return _filename; }
	uint32 GetID() const
		{ return _id; }
	uint32 GetExperienceForNextLevel() const
		{ return _experience_next_level; }

	GlobalWeapon* GetWeapon() const
		{ return _equipped_weapon; }
	GlobalArmor* GetEquippedHeadArmor()
		{ return _armor_equipped[0]; }
	GlobalArmor* GetEquippedTorsoArmor()
		{ return _armor_equipped[1]; }
	GlobalArmor* GetEquippedArmsArmor()
		{ return _armor_equipped[2]; }
	GlobalArmor* GetEquippedLegArmor()
		{ return _armor_equipped[3]; }

	std::vector<GlobalSkill*> GetAttackSkills() const
		{ return _attack_skills; }
	std::vector<GlobalSkill*> GetDefenseSkills() const
		{ return _defense_skills; }
	std::vector<GlobalSkill*> GetSupportSkills() const
		{ return _support_skills; }

	std::vector<hoa_video::StillImage> * GetBattlePortraits()
		{ return &_battle_portraits; }
	void SetExperienceNextLevel(uint32 xp_next)
		{ _experience_next_level = xp_next; }
	//@}

	void AddAttackSkill(GlobalSkill* skill)
		{ if (skill != NULL) _attack_skills.push_back(skill); }
	void AddDefenseSkill(GlobalSkill* skill)
		{ if (skill != NULL) _defense_skills.push_back(skill); }
	void AddSupportSkill(GlobalSkill* skill)
		{ if (skill != NULL) _support_skills.push_back(skill); }

	void AddBattleAnimation(const std::string & name, hoa_video::AnimatedImage anim)
		{ _battle_animation[name] = anim; }
	hoa_video::AnimatedImage * RetrieveBattleAnimation(const std::string & name)
		{ return &_battle_animation[name]; }

protected:
	//! \brief The name used to retrieve the characters's data and other information from various sources
	std::string _filename;
	
	/** \brief An identification number for the character
	*** Refer to the Game Character Type constants at the top of this file
	**/
	uint32 _id;
	
	GlobalWeapon* _equipped_weapon;
	GlobalArmor* _equipped_armor[4];

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
*** \brief Represents a party of characters
***
*** This class is a container for a group or "party" of characters. The purpose of 
*** this class is tied mostly to convience for the GameGlobal class, as characters
*** may need to be organized into groups. For example, the characters that are in the 
*** active party versus the "reserved" party, or in the case of when the characters split
*** into multiple parties according with the story or to achieve a particular goal.
***
*** \note When this class is destroyed, the characters contained within the class are
*** <i>not</i> destroyed. Do not assume otherwise.
***
*** \todo Perhaps this class should be placed in the private_global namespace since
*** the only need for it seems to be in the GameGlobal class?
*** ***************************************************************************/
class GlobalCharacterParty {
public:
	GlobalCharacterParty()
		{}
	~GlobalCharacterParty()
		{}

	//! \name Class member access functions
	//@{
	std::vector<GlobalCharacter*> GetCharacters() const
		{ return _characters; }
	uint32 GetPartySize() const
		{ return _characters.size(); }
	bool IsPartyEmpty() const
		{ return (_characters.size() == 0); }
	//@}

	/** \brief Adds a character to the party
	*** \param character A pointer to the character to add
	*** Note that if the character is found to already be in the party, the character will <b>not</b>
	*** be added a second time.
	**/
	void AddCharacter(GlobalCharacter* character);
	/** \brief Removes a character from the party
	*** \param character A pointer to the character to remove
	*** \return A pointer to the character that was removed, or NULL if the character was not found in the party
	**/
	GlobalCharacter* RemoveCharacter(GlobalCharacter* character);
private:
	//! \brief The characters that are in this party
	std::vector<GlobalCharacter*> _characters;
}; // class GlobalCharacterParty

} // namespace hoa_global

#endif // __GLOBAL_ACTORS_HEADER__
