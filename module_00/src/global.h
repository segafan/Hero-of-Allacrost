///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    global.h
 * \author  Tyler Olsen, roots@allacrost.org, Daniel Steuernol steu@allacrost.org
 * \date    Last Updated: January 19th, 2006
 * \brief   Header file for the global game components.
 *
 * This file contains several classes that need to be used "globally" by all
 * the inherited game mode classes. This includes classes that represent things
 * like items, playable characters, foes, and a game instance manager, which
 * serves to hold all the party's current status information (play time, item
 * inventory, etc).
 *
 * \note As of this moment, the code and structure in this file is pre-mature
 * and subject to major changes/additions as we find new needs of these classes.
 * Be aware that frequent changes to this code will be seen for a while.
 *****************************************************************************/

#ifndef __GLOBAL_HEADER__
#define __GLOBAL_HEADER__

#include "defs.h"
#include "utils.h"

//! All calls to global code are wrapped in this namespace.
namespace hoa_global {

//! The singleton pointer responsible for the management of global game data.
extern GameGlobal *GlobalManager;
//! Determines whether the code in the hoa_boot namespace should print debug statements or not.
extern bool GLOBAL_DEBUG;

//! \name Game Object Types
//@{
//! \brief Constants for different game object types
const uint8 GLOBAL_DUMMY_OBJ  = 0x00;
const uint8 GLOBAL_ITEM       = 0x01;
const uint8 GLOBAL_SKILL_BOOK = 0x02;
const uint8 GLOBAL_WEAPON     = 0x04;
const uint8 GLOBAL_HEAD_ARMOR = 0x08;
const uint8 GLOBAL_BODY_ARMOR = 0x10;
const uint8 GLOBAL_ARMS_ARMOR = 0x20;
const uint8 GLOBAL_LEGS_ARMOR = 0x40;
//@}

//! \name Game Item Usage Types
//@{
//! \brief Constants for the numerous methods of application of game items.
const uint8 GLOBAL_UNUSABLE_ITEM = 0x00;
const uint8 GLOBAL_RECOVERY_ITEM = 0x01;
const uint8 GLOBAL_BATTLE_ITEM   = 0x02;
const uint8 GLOBAL_MAP_ITEM      = 0x04;
//@}

//! \name Game Character Types
//@{
//! \brief Constants for character bit-masking for identification.
const uint32 GLOBAL_NO_CHARACTERS  = 0x00000000;
const uint32 GLOBAL_CLAUDIUS       = 0x00000001;
const uint32 GLOBAL_LAILA          = 0x00000002;
const uint32 GLOBAL_ALL_CHARACTERS = 0xFFFFFFFF;
//@}

//! \name Game Element Types
//@{
//! \brief Constants for the various elemental types.
const uint8 GLOBAL_NO_ELEMENTAL        = 0x00;
const uint8 GLOBAL_FIRE_ELEMENTAL      = 0x01;
const uint8 GLOBAL_ICE_ELEMENTAL       = 0x02;
const uint8 GLOBAL_LIGTHNING_ELEMENTAL = 0x04;
const uint8 GLOBAL_EARTH_ELEMENTAL     = 0x08;
const uint8 GLOBAL_LIGHT_ELEMENTAL     = 0x10;
const uint8 GLOBAL_DARK_ELEMENTAL      = 0x20;
//@}

//! \name Game Status Types
//@{
//! \brief Constants used to reflect various status ailments.
const uint32 GLOBAL_NO_STATUS     = 0x00000000;
const uint32 GLOBAL_POISON_STATUS = 0x00000001;
const uint32 GLOBAL_SLOW_STATUS   = 0x00000002;
const uint32 GLOBAL_SLEEP_STATUS  = 0x00000004;
//@}

//! \name Game Sounds
//@{
//! \brief Constants for referring to commonly accessed sound media.
const uint32 GLOBAL_SOUND_CONFIRM =  0;
const uint32 GLOBAL_SOUND_CANCEL  =  1;
const uint32 GLOBAL_SOUND_OBTAIN  =  2;
const uint32 GLOBAL_SOUND_BUMP    =  3;
//@}

/*!****************************************************************************
 * \brief A parent class that all the different item classes inherit from.
 *
 * The purpose of this class is for use of polymorphism with game objects, so
 * a player can keep all sorts of objects in their inventory, foes can drop
 *
 * \note 1) The copy constructor and copy assignment operator are private because
 * we don't want to accidentally allow the player to find a hack for duplicating
 * objects in their inventory.
 *
 * \note 2) There are several member access functions to safe-guard the
 * programmer against accidentally changing the important values of the class
 * members.
 *****************************************************************************/
class GlobalObject {
private:
	GlobalObject(const GlobalObject&) {}
	GlobalObject& operator=(const GlobalObject&) {}
protected:
	//! The name of the object, as will be displayed on the player's screen.
	std::string obj_name;
	//! \brief A numerical value that defines what type of object this is.
	//! See the Game Object Type constants for a list of the different object types.
	uint8 obj_type;
	//! \brief A bit-mask that determines what charactesr can use the said object.
	//! See the Game Character Type constants for a list of the different characters.
	uint32 usable_by;
	//! \brief An identification number for each item.
	//! Two of the same object will share the same ID number.
	uint32 obj_id;
	//! The number of objects contained within this instance.
	uint32 obj_count;
public:
	GlobalObject(std::string name, uint8 type, uint32 usable, uint32 id, uint32 count);
	GlobalObject();
	~GlobalObject();

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	void SetName(std::string name) { obj_name = name; }
	std::string GetName() { return obj_name; }
	void SetType(uint8 type) { obj_type = type; }
	uint8 GetType() { return obj_type; }
	void SetUsableBy(uint32 use) { usable_by = use; }
	uint32 GetUsableBy() { return usable_by; }
	void SetID(uint32 id) { obj_id = id; }
	uint32 GetID() { return obj_id; }
	void SetCount(uint32 amount) { obj_count = amount; if (obj_count < 0) obj_count = 0; }
	void IncCount(uint32 amount) { obj_count += amount; }
	void DecCount(uint32 amount) { obj_count -= amount; if (obj_count < 0) obj_count = 0; }
	//@}
};

/*!****************************************************************************
 * \brief A class for representing items found in the game.
 *
 * This class is for "general" items, like healing potions. Weapons, Armor, and
 * SkillBooks are not considered to be items in this context.
 *
 * \note 1) The copy constructor and copy assignment operator are private because
 * we don't want to accidentally allow the player to find a hack for duplicating
 * objects in their inventory.
 *
 * \note 2) There are several member access functions to safe-guard the
 * programmer against accidentally changing the important values of the class
 * members.
 *****************************************************************************/
class GlobalItem : public GlobalObject {
private:
	//! \brief A bit-mask stating where the item may be used.
	//! See the Game Item Usage Type constants for a list of locations.
	uint8 _use_case;

	GlobalItem(const GlobalItem&) {}
	GlobalItem& operator=(const GlobalItem&) {}
public:
	GlobalItem(std::string name, uint8 use, uint32 usable, uint32 id, uint32 count);
	GlobalItem();
	~GlobalItem();

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	void SetUseCase(uint8 use) { _use_case = use; }
	uint8 GetUseCase() { return _use_case; }
	//@}
};



/*!****************************************************************************
 * \brief A class for representing weapons found in the game.
 *
 * It should be fairly obvious, but not all weapons can be equipped by all
 * characters.
 *
 * \note 1) The copy constructor and copy assignment operator are private because
 * we don't want to accidentally allow the player to find a hack for duplicating
 * objects in their inventory.
 *
 * \note 2) There are several member access functions to safe-guard the
 * programmer against accidentally changing the important values of the class
 * members.
 *****************************************************************************/
class GlobalWeapon : public GlobalObject {
private:
	GlobalWeapon(const GlobalWeapon&) {}
	GlobalWeapon& operator=(const GlobalWeapon&) {}
public:
	GlobalWeapon(std::string name, uint32 usable, uint32 id, uint32 count);
	GlobalWeapon();
	~GlobalWeapon();
};

 /*!****************************************************************************
 * \brief A class for representing armor found in the game.
 *
 * It should be fairly obvious, but not all armor can be equipped by all
 * characters. Even though there's only one armor class, there are actually four
 * types of armor: head, body, arms, and legs. The GlobalObject#obj_type member is used
 * to identify what armor category an instance of this class belongs to. All armor
 * have the same members/properties, so it doesn't make any sense to make four
 * identical classes different only in name for the four armor types.
 *
 * \note 1) The copy constructor and copy assignment operator are private because
 * we don't want to accidentally allow the player to find a hack for duplicating
 * objects in their inventory.
 *
 * \note 2) There are several member access functions to safe-guard the
 * programmer against accidentally changing the important values of the class
 * members.
 *****************************************************************************/
class GlobalArmor : public GlobalObject {
private:
	GlobalArmor(const GlobalArmor&) {}
	GlobalArmor& operator=(const GlobalArmor&) {}
public:
	GlobalArmor(std::string name, uint8 type, uint32 usable, uint32 id, uint32 count);
	GlobalArmor();
	~GlobalArmor();
};

/*!****************************************************************************
 * \brief A class for representing skills used in the game.
 *
 * A skill can be used by both friendly and foe. Some skills can become innate
 * (require no skill points to use) after time.
 *
 * \note 1) The copy constructor and copy assignment operator are private because
 * we don't want to accidentally allow the player to find a hack for duplicating
 * objects in their inventory.
 *
 * \note 2) There are several member access functions to safe-guard the
 * programmer against accidentally changing the important values of the class
 * members.
 *****************************************************************************/
class GlobalSkill {
private:
	//! The name of the skill, as will be displayed to the player on the screen.
	std::string _skill_name;
	//! The amount of skill points (SP) that the skill consumes (zero is valid).
	uint32 _sp_usage;

	uint32 _skill_type; //defined by the enums

	uint32 _attack;
	uint32 _defense;
	uint32 _support;

	/*

	Okay, if there is a warmup time when we select the skill,
	we put the character straight into warmup mode, passing in a
	new SkillAction.  Otherwise, set them as having a new action.
	This way, we don't have to hack up a warmupaction and cooldownaction.

	So upon selecting a skill from the battle mode gui, check if the
	character should go into a warmup mode.

	*/
	uint32 _warmUpTime;
	uint32 _coolDownTime;

	//! The level required to use this skill
	uint32 _levelRequired;
	uint32 _numArguments;

	hoa_battle::Actor *_host;
	std::vector<hoa_battle::Actor *> _arguments;
	std::vector<hoa_battle::BattleAction *> _actions;
public:

	GlobalSkill(std::string name, uint32 sp);
	GlobalSkill();
	~GlobalSkill();

	void PerformSkill(hoa_battle::Actor *a, std::vector<hoa_battle::Actor *> args);
	void AddBattleAction(hoa_battle::BattleAction *bsa);

	uint32 GetCooldownTime();
	uint32 GetWarmupTime();
};

/*!****************************************************************************
 * \brief A class for representing "attack points" present on a character or enemy.
 *
 * An attack point is a place where a sprite may be attacked (its not a numerical
 * quantity). Sprites may have multiple attack points, each with their own resistances
 * and weaknesses. The standard number of attack points on character battle sprites
 * is four, found on the head, body, arms, and legs.
 *
 * The x_position and y_position members determine the pinpoint location of the attack
 * point on the sprite, where each value of 1.0 represents the length of one "battle tile".
 * (See battle.h for more information on the coordinate system in battle mode). The lower
 * left hand corner of the sprite is [0.0, 0.0], and increasing the x and y position members
 * move the attack point to the right and upwards, respectively.
 *
 * \note 1) The copy constructor and copy assignment operator are private because
 * we don't want to accidentally allow the player to find a hack for duplicating
 * objects in their inventory.
 *
 * \note 2) There are several member access functions to safe-guard the
 * programmer against accidentally changing the important values of the class
 * members.
 *****************************************************************************/
class GlobalAttackPoint {
private:
	//! The x position of the attack point on the sprite.
	float _x_position;
	//! The y position of the attack point on the sprite.
	float _y_position;

	//! The amount of defense ability this attack point has.
	uint32 _defense;
	//! The amount of evade ability this attack point has.
	uint32 _evade;
	//! A bit-mask of elemental weaknesses of the attack point.
	uint8 _elemental_weakness;
	//! A bit-mask of elemental resistances of the attack point.
	uint8 _elemental_resistance;
	//! A bit-mask of status weaknesses of the attack point.
	uint8 _status_weakness;
	//! A bit-mask of status resistances of the attack point.
	uint8 _status_resistance;
public:
	GlobalAttackPoint(float x, float y, uint32 def, uint32 eva, uint8 elem_weak,
	             uint8 elem_res, uint8 stat_weak, uint8 stat_res);
	GlobalAttackPoint();
	~GlobalAttackPoint();
	GlobalAttackPoint(const GlobalAttackPoint&) {}
	GlobalAttackPoint& operator=(const GlobalAttackPoint&) {}

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	void SetXPosition(float x) { _x_position = x; }
	float GetXPosition() { return _x_position; }
	void SetYPosition(float y) { _y_position = y; }
	float GetYPosition() { return _y_position; }
	void SetDefense(uint32 def) { _defense = def; }
	uint32 GetDefense() { return _defense; }
	void SetEvade(uint32 eva) { _evade = eva; }
	uint32 GetEvade() { return _evade; }
	void SetElementWeakness(uint8 elem_weak) { _elemental_weakness = elem_weak; }
	uint8 GetElementWeakness() { return _elemental_weakness; }
	void SetElementResistance(uint8 elem_res) { _elemental_resistance = elem_res; }
	uint8 GetElementResistance() { return _elemental_resistance; }
	void SetStatusWeakness(uint8 stat_weak) { _status_weakness = stat_weak; }
	uint8 GetStatusWeakness() { return _status_weakness; }
	void SetStatusResistance(uint8 stat_res) { _status_resistance = stat_res; }
	uint8 GetStatusResistance() { return _status_resistance; }
	//@}
};

/*!****************************************************************************
 * \brief A class for representing enemies in the game.
 *
 * Enemies are very similar to characters in the term of the data members and member
 * access functions. The major difference is that the player can control characters
 * but not enemies.
 *
 * The Allacrost battle system is designed to give the player a unique experience for
 * each individual battle. One of the ways we accomplish this is by randomizing the stats
 * of enemies for every battle. First, a random Guassian number is chosen with the party's
 * median experience level as the mean. A random guassian number is chosen for each enemy,
 * so encountered enemies in battle can have a variety of experience levels (but the majority
 * are within one standard deviation of the average party level).
 *
 * Secondly, the enemy is thrown through a "level-up" simulator that simulates the growth of
 * its statistics for each experience level. Each statistic is simulated seperately and has
 * class members for: the base amount, the growth amount, and the rate (chance) of growth
 * per level. Read the battle design document for more information on this.
 *
 * \note 1) One thing I haven't included here that we will need is some sort of call to an
 * AI algorithm to decide what action the enemy should take. However, this is a rather
 * complicated construct so for now I've left it out until we decide on how to manage
 * our AI routines.
 *
 * \note 2) Once we figure things out between handling this class between the map code and
 * battle code a little more, we'll need to define the copy constructor and copy assignment
 * operator. For now they are left blank.
 *****************************************************************************/
class GlobalEnemy {
private:
	//! The enemy's name, as seen by the player on the screen.
	std::string _enemy_name;
	//! The base of the character's file name, used to retrieve various data for the enemy.
	std::string _filename;
	//! An identification number for the enemy type.
	uint32 _enemy_id;
	//! The width of the enemy sprite, in number of "virtual battle tiles".
	uint32 _enemy_width;
	//! The height of the enemy sprite, in number of "virtual battle tiles".
	uint32 _enemy_height;
	//! The skill set that the enemy may choose from to take actions.
	std::vector<GlobalSkill *> _enemy_skills;
	//! The various attack points for the enemy.
	std::vector<GlobalAttackPoint> _attack_points;
	//! The frame images for the enemy sprite.
	std::vector<hoa_video::StillImage> _sprite_frames;

	//! The current number of hit points for the enemy.
	uint32 _hit_points;
	//! The maximum number of hit points that the enemy may have.
	uint32 _max_hit_points;
	//! The current number of skill points for the enemy.
	uint32 _skill_points;
	//! The maximum number of skill points that the enemy may have.
	uint32 _max_skill_points;
	//! The number of experience points the enemy gives the party when defeated.
	uint32 _experience_points;
	//! The current experience level of the enemy.
	uint32 _experience_level;
	//! The strength index of the enemy.
	uint32 _strength;
	//! The intelligence index of the enemy.
	uint32 _intelligence;
	//! The agility index of the enemy.
	uint32 _agility;

	//! \name Starting Base Statistics
	//@{
	//! \brief These are the base statistics for the enemy when on experience level 1.
	uint32 _base_hit_points;
	uint32 _base_skill_points;
	uint32 _base_experience_points;
	uint32 _base_strength;
	uint32 _base_intelligence;
	uint32 _base_agility;
	//@}

	//! \name Growth Amount of Statistics
	//@{
	//! \brief The increase in statistic values between experience levels is reflected in these members.
	uint32 _growth_hit_points;
	uint32 _growth_skill_points;
	uint32 _growth_experience_points;
	uint32 _growth_strength;
	uint32 _growth_intelligence;
	uint32 _growth_agility;
	//@}

public:
	GlobalEnemy();
	~GlobalEnemy();
// 	GlobalEnemy(const GlobalEnemy&) {}
// 	GlobalEnemy& operator=(const GlobalEnemy&) {}

	/*!
	 *  \brief Simulates the growth of the enemy from the base experience level.
	 *  \param lvl The final level to simulate to.
	 *
	 *  This function "simulates" the growth of an enemy from its base xp level (level 1) to
	 *  the specified level in the argument. It sounds more complicated than it really is. All
	 *  that the function does is the following:
	 *
	 *  -# Start with the base statistics
	 *  -# For every level gain, calculate a random number between 0.0 and 1.0
	 *  -# If the random number is less than the rate member for the specific,
	 *     increase that stat by the growth member.
	 *
	 *  In the end, its just number crunching to come up with a slightly different enemy for each
	 *  battle, so the player doesn't keep fighting the same old enemy over and over, like the
	 *  "level 5 scorpion with 60 HP".
	 */
	void LevelSimulator(uint32 lvl);

	std::string GetName() { return _enemy_name; }

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	void SetHP(uint32 hp) { _hit_points = hp; }
	uint32 GetHP() { return _hit_points; }
	void SetMaxHP(uint32 max_hp) { _max_hit_points = max_hp; }
	uint32 GetMaxHP() { return _max_hit_points; }
	void SetSP(uint32 sp) { _skill_points = sp; }
	uint32 GetSP() { return _skill_points; }
	void SetMaxSP(uint32 sp) { _max_skill_points = sp; }
	uint32 GetMaxSP() { return _max_skill_points; }
	void SetXP(uint32 xp) { _experience_points = xp; }
	uint32 GetXP() { return _experience_points; }
	void SetXPLevel(uint32 xp_lvl) { _experience_level = xp_lvl; }
	uint32 GetXPLevel() { return _experience_level; }
	void SetStrength(uint32 str) { _strength = str; }
	uint32 GetStrength() { return _strength; }
	void SetIntelligence(uint32 intel) { _intelligence = intel; }
	uint32 GetIntelligence() { return _intelligence; }
	void SetAgility(uint32 agi) { _agility = agi; }
	uint32 GetAgility() { return _agility; }

	//!Added by visage November 16
	uint32 GetBaseHitPoints() { return _base_hit_points; }
	uint32 GetBaseSkillPoints() { return _base_skill_points; }
	uint32 GetBaseExperiencePoints() { return _base_experience_points; }
	uint32 GetBaseStrength() { return _base_strength; }
	uint32 GetBaseIntelligence() { return _base_intelligence; }
	uint32 GetBaseAgility() { return _base_agility; }
	uint32 GetGrowthHitPoints() { return _growth_hit_points; }
	uint32 GetGrowthSkillPoints() { return _growth_skill_points; }
	uint32 GetGrowthExperiencePoints() { return _growth_experience_points; }
	uint32 GetGrowthStrength() { return _growth_strength; }
	uint32 GetGrowthIntelligence() { return _growth_intelligence; }
	uint32 GetGrowthAgility() { return _growth_agility; }
	std::vector<GlobalSkill *> GetSkills() { return _enemy_skills; }
	std::vector<GlobalAttackPoint> GetAttackPoints() { return _attack_points; }

	void AddSkill(GlobalSkill *sk) { _enemy_skills.push_back(sk); }
	//@}
}; // class GlobalEnemy

/*!****************************************************************************
 * \brief A class for representing playable game characters.
 *
 * This calls represents playable game characters only (those that you can control,
 * equip, and send into battle). It does not cover NPCs or anything else. All
 * active character objects in the game are wrapped inside the GameGlobal class.
 *
 * \note 1) Almost always, a character will have four, and only four, attack points.
 * However, I've left it as a vector here in case we have some special case later in
 * the game where we want to add or remove attack points from a character.
 *****************************************************************************/
class GlobalCharacter {
private:
	//! The character's name, as seen by the player on the screen.
	std::string _name;
	//! The base of the character's file name, used to retrieve various data for the character.
	std::string _filename;
	//! \brief An identifier for the character.
	//! See the Game Character Type constants.
	uint32 _char_id;

	//! The weapon that the character currently has equipped.
	GlobalWeapon *_eq_weapon;
	//! The head armor that the character currently has equipped.
	GlobalArmor *_eq_head;
	//! The body armor that the character currently has equipped.
	GlobalArmor *_eq_body;
	//! The arm armor that the character currently has equipped.
	GlobalArmor *_eq_arms;
	//! The leg armor that the character currently has equipped.
	GlobalArmor *_eq_legs;
	//! The attack skills the character can currently use.
	std::vector<GlobalSkill *> _attack_skills;
	//! The defense skills the character can currently use.
	std::vector<GlobalSkill *> _defense_skills;
	//! The support skills the character can currently use.
	std::vector<GlobalSkill *> _support_skills;
	//! The (four) attack points of the character.
	std::vector<GlobalAttackPoint> _attack_points;
	//! The current number of hit points of the character.
	uint32 _hit_points;
	//! The maximum number of hit points the character may have.
	uint32 _max_hit_points;
	//! The current number of skill points of the character.
	uint32 _skill_points;
	//! The maximum number of skill points the character may have.
	uint32 _max_skill_points;
	//! The current number of experience points of the character.
	uint32 _experience_points;
	//! The current experience level of the character.
	uint32 _experience_level;
	//! The current number of experience points needed to reach the next experience level.
	uint32 _experience_next_level;
	//! The character's strength index.
	uint32 _strength;
	//! The character's intelligence index.
	uint32 _intelligence;
	//! The character's agility index.
	uint32 _agility;

	//! The frame images for the character's map sprite.
	std::vector<hoa_video::StillImage> _map_frames;
	//! The frame images for the character's battle sprite.
	std::vector<hoa_video::StillImage> _battle_frames;
public:
	GlobalCharacter(std::string na, std::string fn, uint32 id);
	~GlobalCharacter();

	//! \name Weapon and Armor Equip Functions
	//@{
	//! \brief Swaps in and out equipment on the character.
	//! \param *new_eq The new weapon or armor to equip.
	//! \return The previously equipped weapon or armor.
	GlobalWeapon* EquipWeapon(GlobalWeapon *_new_eq);
	GlobalArmor* EquipHeadArmor(GlobalArmor *_new_eq);
	GlobalArmor* EquipBodyArmor(GlobalArmor *_new_eq);
	GlobalArmor* EquipArmsArmor(GlobalArmor *_new_eq);
	GlobalArmor* EquipLegsArmor(GlobalArmor *_new_eq);
	//@}

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	void SetName(std::string na) { _name = na; }
	std::string GetName() { return _name; }
	void SetFilename(std::string fn) { _filename = fn; }
	std::string GetFilename() { return _filename; }
	void SetID(uint32 id) { _char_id = id; }
	uint32 GetID() { return _char_id; }
	void SetHP(uint32 hp) { _hit_points = hp; }
	uint32 GetHP() { return _hit_points; }
	void SetMaxHP(uint32 max_hp) { _max_hit_points = max_hp; }
	uint32 GetMaxHP() { return _max_hit_points; }
	void SetSP(uint32 sp) { _skill_points = sp; }
	uint32 GetSP() { return _skill_points; }
	void SetMaxSP(uint32 sp) { _max_skill_points = sp; }
	uint32 GetMaxSP() { return _max_skill_points; }
	void AddXP(uint32 xp);
	void SetXP(uint32 xp) { _experience_points = xp; }
	uint32 GetXP() { return _experience_points; }
	void SetXPLevel(uint32 xp_lvl) { _experience_level = xp_lvl; }
	uint32 GetXPLevel() { return _experience_level; }
	void SetXPNextLevel(uint32 xp_next) { _experience_next_level = xp_next; }
	uint32 GetXPForNextLevel() { return _experience_next_level; }
	void SetStrength(uint32 str) { _strength = str; }
	uint32 GetStrength() { return _strength; }
	void SetIntelligence(uint32 intel) { _intelligence = intel; }
	uint32 GetIntelligence() { return _intelligence; }
	void SetAgility(uint32 agi) { _agility = agi; }
	uint32 GetAgility() { return _agility; }
	
	std::vector<GlobalSkill *> GetAttackSkills() { return _attack_skills; }
	std::vector<GlobalSkill *> GetDefenseSkills() { return _defense_skills; }
	std::vector<GlobalSkill *> GetSupportSkills() { return _support_skills; }

	std::vector<GlobalAttackPoint> GetAttackPoints() { return _attack_points; }

	void AddAttackSkill(GlobalSkill *sk) { _attack_skills.push_back(sk); }
	void AddDefenseSkill(GlobalSkill *sk) { _defense_skills.push_back(sk); }
	void AddSupportSkill(GlobalSkill *sk) { _support_skills.push_back(sk); }
	//@}
};

/*!**************************************************************************
 * \brief Represents a party of characters
 *
 * This class contains a group of characters.  There is generally only one 
 * active party, but there could be at times multiple parties, for example
 * some of the fights in FF VI.
 ***************************************************************************/
class GlobalParty
{
private:
	//! Character id's on those in the party (max of 4)
	std::vector<uint32> _characters;
public:
	GlobalParty();
	~GlobalParty();
	//! Add a character to the party
	void AddCharacter(uint32 char_id);
	//! Remove a character from the party
	void RemoveCharacter(uint32 char_id);
	//! Get all the characters in the party
	//! \returns a vector containing the character ids of the characters in the party.
	std::vector<uint32> GetCharacters() { return _characters; }
	//! Gets the party size
	uint32 GetPartySize() { return _characters.size(); }
};



/*!****************************************************************************
 * \brief A class that retains all the state information about the active game.
 *
 * In addition to retaining all information about the running game such as time
 * elapsed, this class also serves as a resource manager for frequently accessed
 * data. Basically, we hard-code what media should be kept in this class and
 * it is permanently retained here until the game exits. This includes things like
 * common menu sounds and graphics.
 *
 * \note 1) This class is a singleton.
 *
 * \note 2) This class still needs \c A \c LOT of work done. It will be modified
 * along with the code in map.h, battle.h, and menu.h, which will use this class
 * heavily. The class shall be developed progressively along with the progress
 * of these other files.
 *****************************************************************************/
class GameGlobal {
private:
	SINGLETON_DECLARE(GameGlobal);
	//! The characters currently in the party.
	std::vector<GlobalCharacter*> _characters;
	//! The inventory of the party.
	std::vector<GlobalObject> _inventory;
	//! The amount of financial resources the party currently has.
	uint32 _money;
	//! The active party (currently only support for one party, may need to be changed)
	GlobalParty _party;

// 	hoa_video::GameVideo *VideoManager;

public:
	SINGLETON_METHODS(GameGlobal);

	//! Adds a new character to the party.
	//! \param *ch A pointer to the GlobalCharacter object to add to the party.
	void AddCharacter(GlobalCharacter *ch);
	//! Returns a pointer to a character currently in the party.
	//! \param id The ID number of the character to retrieve.
	//! \return A pointer to the character, or NULL if the character was not found.
	GlobalCharacter* GetCharacter(uint32 id);
	//! Money handling functions, 
	//! Get, returns the current amount
	//! Set, sets the money to the specified amount
	//! Add, adds the specified amount to the current amount
	//! Subtract, takes away the specified amount from the current amount
	//@{
	uint32 GetMoney();
	void SetMoney(uint32 amount);
	void AddMoney(uint32 amount);
	void SubtractMoney(uint32 amount);
	// @}
	//! Gets the Characters in the active party
	//! \returns The Characters in the active party
	std::vector<GlobalCharacter *> GetParty();
}; // class GameGlobal


} // namespace hoa_global

#endif
