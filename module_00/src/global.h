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
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 18th, 2005
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

#include "utils.h"
#include <vector>
#include "SDL.h"
#include "defs.h"

//! All calls to global code are wrapped in this namespace.
namespace hoa_global {

//! Determines whether the code in the hoa_boot namespace should print debug statements or not.
extern bool GLOBAL_DEBUG;

//! \name Game Object Types
//@{
//! \brief Constants for different game object types
const unsigned char GLOBAL_DUMMY_OBJ  = 0x00;
const unsigned char GLOBAL_ITEM       = 0x01;
const unsigned char GLOBAL_SKILL_BOOK = 0x02;
const unsigned char GLOBAL_WEAPON     = 0x04;
const unsigned char GLOBAL_HEAD_ARMOR = 0x08;
const unsigned char GLOBAL_BODY_ARMOR = 0x10;
const unsigned char GLOBAL_ARMS_ARMOR = 0x20;
const unsigned char GLOBAL_LEGS_ARMOR = 0x40;
//@}

//! \name Game Item Usage Types
//@{
//! \brief Constants for the numerous methods of application of game items.
const unsigned char GLOBAL_UNUSABLE_ITEM = 0x00;
const unsigned char GLOBAL_RECOVERY_ITEM = 0x01;
const unsigned char GLOBAL_BATTLE_ITEM   = 0x02;
const unsigned char GLOBAL_MAP_ITEM      = 0x04;
//@}

//! \name Game Character Types
//@{
//! \brief Constants for character bit-masking for identification.
const uint GLOBAL_NO_CHARACTERS  = 0x00000000;
const uint GLOBAL_CLAUDIUS       = 0x00000001;
const uint GLOBAL_LAILA          = 0x00000002;
const uint GLOBAL_ALL_CHARACTERS = 0xFFFFFFFF;
//@}

//! \name Game Element Types
//@{
//! \brief Constants for the various elemental types.
const unsigned char GLOBAL_NO_ELEMENTAL        = 0x00;
const unsigned char GLOBAL_FIRE_ELEMENTAL      = 0x01;
const unsigned char GLOBAL_ICE_ELEMENTAL       = 0x02;
const unsigned char GLOBAL_LIGTHNING_ELEMENTAL = 0x04;
const unsigned char GLOBAL_EARTH_ELEMENTAL     = 0x08;
const unsigned char GLOBAL_LIGHT_ELEMENTAL     = 0x10;
const unsigned char GLOBAL_DARK_ELEMENTAL      = 0x20;
//@}

//! \name Game Status Types
//@{
//! \brief Constants used to reflect various status ailments.
const unsigned int GLOBAL_NO_STATUS     = 0x00000000;
const unsigned int GLOBAL_POISON_STATUS = 0x00000001;
const unsigned int GLOBAL_SLOW_STATUS   = 0x00000002;
const unsigned int GLOBAL_SLEEP_STATUS  = 0x00000004;
//@}

//! \name Game Sounds
//@{
//! \brief Constants for referring to commonly accessed sound media.
const uint GLOBAL_SOUND_CONFIRM =  0;
const uint GLOBAL_SOUND_CANCEL  =  1;
const uint GLOBAL_SOUND_OBTAIN  =  2;
const uint GLOBAL_SOUND_BUMP    =  3;
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
class GObject {
private:
	GObject(const GObject&) {}
	GObject& operator=(const GObject&) {}
protected:
	//! The name of the object, as will be displayed on the player's screen.
	std::string obj_name;
	//! \brief A numerical value that defines what type of object this is.
	//! See the Game Object Type constants for a list of the different object types.
	unsigned char obj_type;
	//! \brief A bit-mask that determines what charactesr can use the said object.
	//! See the Game Character Type constants for a list of the different characters.
	unsigned int usable_by;
	//! \brief An identification number for each item.
	//! Two of the same object will share the same ID number.
	int obj_id;
	//! The number of objects contained within this instance.
	int obj_count;
public:
	GObject(std::string name, unsigned char type, unsigned int usable, int id, int count);
	GObject();
	~GObject();

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	void SetName(std::string name) { obj_name = name; }
	std::string GetName() { return obj_name; }
	void SetType(unsigned char type) { obj_type = type; }
	unsigned char GetType() { return obj_type; }
	void SetUsableBy(unsigned int use) { usable_by = use; }
	unsigned int GetUsableBy() { return usable_by; }
	void SetID(int id) { obj_id = id; }
	int GetID() { return obj_id; }
	void SetCount(int amount) { obj_count = amount; if (obj_count < 0) obj_count = 0; }
	void IncCount(int amount) { obj_count += amount; }
	void DecCount(int amount) { obj_count -= amount; if (obj_count < 0) obj_count = 0; }
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
class GItem : public GObject {
private:
	//! \brief A bit-mask stating where the item may be used.
	//! See the Game Item Usage Type constants for a list of locations.
	unsigned char use_case;

	GItem(const GItem&) {}
	GItem& operator=(const GItem&) {}
public:
	GItem(std::string name, unsigned char use, unsigned int usable, int id, int count);
	GItem();
	~GItem();

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	void SetUseCase(unsigned char use) { use_case = use; }
	unsigned char GetUseCase() { return use_case; }
	//@}
};

/*!****************************************************************************
 * \brief A class for representing skill books found in the game.
 *
 * Skill Books are rare objects found in the game that contain different skills.
 * Once a skill book is found, the skills are not immediately available to the 
 * characters, unless they are at a relatively high experience level. Skills found
 * in skill books are learned by retaining the book in the inventory, and then
 * gaining a certain number of experience levels to learn the skill.
 *
 * \note 1) The copy constructor and copy assignment operator are private because
 * we don't want to accidentally allow the player to find a hack for duplicating
 * objects in their inventory.
 *
 * \note 2) There are several member access functions to safe-guard the
 * programmer against accidentally changing the important values of the class
 * members.
 *****************************************************************************/
class GSkillBook : public GObject {
private:
	//! The list of skills that this skill book contains.
	std::vector<GSkill> skills;

	GSkillBook(const GSkillBook&) {}
	GSkillBook& operator=(const GSkillBook&) {}
public:
	GSkillBook(std::string name, unsigned char type, unsigned int usable, int id, int count);
	GSkillBook();
	~GSkillBook();

// void LoadBookSkills(); <<< fills up the skills vector. Or should this go in the GameData class?
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
class GWeapon : public GObject {
private:
	GWeapon(const GWeapon&) {}
	GWeapon& operator=(const GWeapon&) {}
public:
	GWeapon(std::string name, unsigned int usable, int id, int count);
	GWeapon();
	~GWeapon();
};

 /*!****************************************************************************
 * \brief A class for representing armor found in the game.
 *
 * It should be fairly obvious, but not all armor can be equipped by all 
 * characters. Even though there's only one armor class, there are actually four
 * types of armor: head, body, arms, and legs. The GObject#obj_type member is used
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
class GArmor : public GObject {
private:
	GArmor(const GArmor&) {}
	GArmor& operator=(const GArmor&) {}
public:
	GArmor(std::string name, unsigned char type, unsigned int usable, int id, int count);
	GArmor();
	~GArmor();
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
class GSkill {
private:
	//! The name of the skill, as will be displayed to the player on the screen.
	std::string skill_name;
	//! The amount of skill points (SP) that the skill consumes (zero is valid).
	unsigned int sp_usage;
	
	GSkill(const GSkill&) {}
	GSkill& operator=(const GSkill&) {}
public:
	GSkill(std::string name, unsigned int sp);
	GSkill();
	~GSkill();
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
class GAttackPoint {
private:
	//! The x position of the attack point on the sprite.
	float x_position;
	//! The y position of the attack point on the sprite.
	float y_position;

	//! The amount of defense ability this attack point has.
	unsigned int defense;
	//! The amount of evade ability this attack point has.
	unsigned int evade;
	//! A bit-mask of elemental weaknesses of the attack point.
	unsigned char elemental_weakness;
	//! A bit-mask of elemental resistances of the attack point.
	unsigned char elemental_resistance;
	//! A bit-mask of status weaknesses of the attack point.
	unsigned char status_weakness;
	//! A bit-mask of status resistances of the attack point.
	unsigned char status_resistance;
public:
	GAttackPoint(float x, float y, int def, int eva, unsigned char elem_weak,
	             unsigned char elem_res, unsigned char stat_weak, unsigned char stat_res);
	GAttackPoint();
	~GAttackPoint();
	GAttackPoint(const GAttackPoint&) {}
	GAttackPoint& operator=(const GAttackPoint&) {}

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	void SetXPosition(float x) { x_position = x; }
	float GetXPosition() { return x_position; }
	void SetYPosition(float y) { y_position = y; }
	float GetYPosition() { return y_position; }
	void SetDefense(unsigned int def) { defense = def; }
	unsigned int GetDefense() { return defense; }
	void SetEvade(unsigned int eva) { evade = eva; }
	unsigned int GetEvade() { return evade; }
	void SetElementWeakness(unsigned char elem_weak) { elemental_weakness = elem_weak; }
	unsigned char GetElementWeakness() { return elemental_weakness; }
	void SetElementResistance(unsigned char elem_res) { elemental_resistance = elem_res; }
	unsigned char GetElementResistance() { return elemental_resistance; }
	void SetStatusWeakness(unsigned char stat_weak) { status_weakness = stat_weak; }
	unsigned char GetStatusWeakness() { return status_weakness; }
	void SetStatusResistance(unsigned char stat_res) { status_resistance = stat_res; }
	unsigned char GetStatusResistance() { return status_resistance; }
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
class GEnemy {
private:
	//! The enemy's name, as seen by the player on the screen.
	std::string enemy_name;
	//! The base of the character's file name, used to retrieve various data for the enemy.
	std::string filename;
	//! An identification number for the enemy type.
	int enemy_id;
	//! The width of the enemy sprite, in number of "virtual battle tiles".
	int enemy_width;
	//! The height of the enemy sprite, in number of "virtual battle tiles".
	int enemy_height;
	//! The skill set that the enemy may choose from to take actions.
	std::vector<GSkill> enemy_skills;
	//! The various attack points for the enemy.
	std::vector<GAttackPoint> attack_points;
	//! The frame images for the enemy sprite.
	std::vector<hoa_video::ImageDescriptor> sprite_frames;

	//! The current number of hit points for the enemy.
	uint hit_points;
	//! The maximum number of hit points that the enemy may have.
	uint max_hit_points;
	//! The current number of skill points for the enemy.
	uint skill_points;
	//! The maximum number of skill points that the enemy may have.
	uint max_skill_points;
	//! The number of experience points the enemy gives the party when defeated.
	uint experience_points;
	//! The current experience level of the enemy.
	uint experience_level;
	//! The strength index of the enemy.
	uint strength;
	//! The intelligence index of the enemy.
	uint intelligence;
	//! The agility index of the enemy.
	uint agility;

	//! \name Starting Base Statistics
	//@{
	//! \brief These are the base statistics for the enemy when on experience level 1. 
	uint base_hit_points;
	uint base_experience_points;
	uint base_strength;
	uint base_intelligence;
	uint base_agility;
	//@}

	//! \name Growth Amount of Statistics
	//@{
	//! \brief The increase in statistic values between experience levels is reflected in these members.
	uint growth_hit_points;
	uint growth_experience_points;
	uint growth_strength;
	uint growth_intelligence;
	uint growth_agility;
	//@}

	//! \name Growth Rate of Statistics
	//@{
	//! \brief Values between 0.0 and 1.0 that reflect how likely a stastic is to grow between experience levels.
	float rate_hit_points;
	float rate_strength;
	float rate_intelligence;
	float rate_agility;
	//@}
public:
	GEnemy();
	~GEnemy();
// 	GEnemy(const GEnemy&) {}
// 	GEnemy& operator=(const GEnemy&) {}

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
	void LevelSimulator(uint lvl);

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	void SetHP(unsigned int hp) { hit_points = hp; }
	unsigned int GetHP() { return hit_points; }
	void SetMaxHP(unsigned int max_hp) { max_hit_points = max_hp; }
	unsigned int GetMaxHP() { return max_hit_points; }
	void SetSP(unsigned int sp) { skill_points = sp; }
	unsigned int GetSP() { return skill_points; }
	void SetXP(unsigned int xp) { experience_points = xp; }
	unsigned int GetXP() { return experience_points; }
	void SetXPLevel(unsigned int xp_lvl) { experience_level = xp_lvl; }
	unsigned int GetXPLevel() { return experience_level; }
	void SetStrength(unsigned int str) { strength = str; }
	unsigned int GetStrength() { return strength; }
	void SetIntelligence(unsigned int intel) { intelligence = intel; }
	unsigned int GetIntelligence() { return intelligence; }
	void SetAgility(unsigned int agi) { agility = agi; }
	unsigned int GetAgility() { return agility; }
	//@}
};

/*!****************************************************************************
 * \brief A class for representing playable game characters.
 *
 * This calls represents playable game characters only (those that you can control,
 * equip, and send into battle). It does not cover NPCs or anything else. All 
 * active character objects in the game are wrapped inside the GameInstance class.
 *
 * \note 1) Almost always, a character will have four, and only four, attack points. 
 * However, I've left it as a vector here in case we have some special case later in 
 * the game where we want to add or remove attack points from a character.
 *****************************************************************************/
class GCharacter {
private:
	//! The character's name, as seen by the player on the screen.
	std::string name;
	//! The base of the character's file name, used to retrieve various data for the character.
	std::string filename;
	//! \brief An identifier for the character.
	//! See the Game Character Type constants.
	uint char_id;

	//! The weapon that the character currently has equipped.
	GWeapon *eq_weapon;
	//! The head armor that the character currently has equipped.
	GArmor *eq_head;
	//! The body armor that the character currently has equipped.
	GArmor *eq_body;
	//! The arm armor that the character currently has equipped.
	GArmor *eq_arms;
	//! The leg armor that the character currently has equipped.
	GArmor *eq_legs;
	//! The attack skills the character can currently use.
	std::vector<GSkill> attack_skills;
	//! The defense skills the character can currently use.
	std::vector<GSkill> defense_skills;
	//! The support skills the character can currently use.
	std::vector<GSkill> support_skills;
	//! The (four) attack points of the character.
	std::vector<GAttackPoint> attack_points;
	//! The frame images for the character's map sprite.
	std::vector<hoa_video::ImageDescriptor> map_frames;
	//! The frame images for the character's battle sprite.
	std::vector<hoa_video::ImageDescriptor> battle_frames;

	//! The current number of hit points of the character.
	unsigned int hit_points;
	//! The maximum number of hit points the character may have.
	unsigned int max_hit_points;
	//! The current number of skill points of the character.
	unsigned int skill_points;
	//! The maximum number of skill points the character may have.
	unsigned int max_skill_points;
	//! The current number of experience points of the character.
	unsigned int experience_points;
	//! The current experience level of the character.
	unsigned int experience_level;
	//! The current number of experience points needed to reach the next experience level.
	unsigned int experience_next_level;
	//! The character's strength index.
	unsigned int strength;
	//! The character's intelligence index.
	unsigned int intelligence;
	//! The character's agility index.
	unsigned int agility;
public:
	GCharacter(std::string na, std::string fn, uint id);
	~GCharacter();

	//! \name Weapon and Armor Equip Functions
	//@{
	//! \brief Swaps in and out equipment on the character.
	//! \param *new_eq The new weapon or armor to equip.
	//! \return The previously equipped weapon or armor.
	GWeapon* EquipWeapon(GWeapon *new_eq);
	GArmor* EquipHeadArmor(GArmor *new_eq);
	GArmor* EquipBodyArmor(GArmor *new_eq);
	GArmor* EquipArmsArmor(GArmor *new_eq);
	GArmor* EquipLegsArmor(GArmor *new_eq);
	//@}

	//! Loads the character's various map and battle frame images from memory.
	void LoadFrames();

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	void SetName(std::string na) { name = na; }
	std::string GetName() { return name; }
	void SetFilename(std::string fn) { filename = fn; }
	std::string GetFilename() { return filename; }
	void SetID(uint id) { char_id = id; }
	uint GetID() { return char_id; }
	std::vector<hoa_video::ImageDescriptor>* GetMapFrames() { return &map_frames; }
	void SetHP(unsigned int hp) { hit_points = hp; }
	unsigned int GetHP() { return hit_points; }
	void SetMaxHP(unsigned int max_hp) { max_hit_points = max_hp; }
	unsigned int GetMaxHP() { return max_hit_points; }
	void SetSP(unsigned int sp) { skill_points = sp; }
	unsigned int GetSP() { return skill_points; }
	void SetXP(unsigned int xp) { experience_points = xp; }
	unsigned int GetXP() { return experience_points; }
	void SetXPLevel(unsigned int xp_lvl) { experience_level = xp_lvl; }
	unsigned int GetXPLevel() { return experience_level; }
	void SetXPNextLevel(unsigned int xp_next) { experience_next_level = xp_next; }
	unsigned int GetXPNextLevel() { return experience_next_level; }
	void SetStrength(unsigned int str) { strength = str; }
	unsigned int GetStrength() { return strength; }
	void SetIntelligence(unsigned int intel) { intelligence = intel; }
	unsigned int GetIntelligence() { return intelligence; }
	void SetAgility(unsigned int agi) { agility = agi; }
	unsigned int GetAgility() { return agility; }
	//@}
};

/*!****************************************************************************
 * \brief Manages elapsed game time.
 *
 * This simple class retains and updates the elapsed game time. When in pause mode,
 * quit mode, and boot mode, time is frozen and does not update.
 * 
 * \note 1) The only instance of this class that you should need already exists
 * privately in the GameInstance class.
 *****************************************************************************/
class GTime {
private:
	friend class GameInstance;
	//! The number of seconds expired.
	unsigned char seconds;
	//! The number of minutes expired.
	unsigned char minutes;
	//! The number of hours expired.
	unsigned char hours;
public:
	//! Updates the elapsed time by one second.
	void UpdateTime() {
		seconds++;
		if (seconds >= 60) {
			minutes++;
			if (minutes >= 60) {
				hours++;
			}
		}
	}
	//! Resets the class members to the specified time.
	//! \param h The amount of hours to set.
	//! \param m The amount of minutes to set.
	//! \param s The amount of seconds to set.
	void SetTime(unsigned char h, unsigned char m, unsigned char s) {
		hours = h;
		minutes = m;
		seconds = s;
	}
}; // class GTime

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
class GameInstance {
private:
	SINGLETON_DECLARE(GameInstance);
	//! An object for retaining the elapsed game time.
	GTime game_time;
	//! The characters currently in the party.
	std::vector<GCharacter*> characters;
	//! The inventory of the party.
	std::vector<GObject> inventory;
	//! The amount of financial resources the party currently has.
	unsigned int money;

// 	hoa_video::GameVideo *VideoManager;

public:
	SINGLETON_METHODS(GameInstance);

	//! Adds a new character to the party.
	//! \param *ch A pointer to the GCharacter object to add to the party.
	void AddCharacter(GCharacter *ch);
	//! Returns a pointer to a character currently in the party.
	//! \param id The ID number of the character to retrieve.
	//! \return A pointer to the character, or NULL if the character was not found.
	GCharacter* GetCharacter(uint id);
}; // class GameInstance


} // namespace hoa_global

#endif
