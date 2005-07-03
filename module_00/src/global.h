/* 
 * global.h
 *	Header file for the Hero of Allacrost global game classes
 *	(C) 2004 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */
 
#ifndef __GLOBAL_HEADER__
#define __GLOBAL_HEADER__ 

#include <vector>
#include "SDL.h" 
#include "defs.h"
#include "utils.h"
#include "video.h"

namespace hoa_global {

extern bool GLOBAL_DEBUG;

// Constants for different game object types
const unsigned char GLOBAL_DUMMY_OBJ  = 0x00;
const unsigned char GLOBAL_ITEM       = 0x01;
const unsigned char GLOBAL_SKILL_BOOK = 0x02;
const unsigned char GLOBAL_WEAPON     = 0x04;
const unsigned char GLOBAL_HEAD_ARMOR = 0x08;
const unsigned char GLOBAL_BODY_ARMOR = 0x10;
const unsigned char GLOBAL_ARMS_ARMOR = 0x20;
const unsigned char GLOBAL_LEGS_ARMOR = 0x40;

// Constants for the types of applications of game items
const unsigned char GLOBAL_UNUSABLE_ITEM = 0x00;
const unsigned char GLOBAL_RECOVERY_ITEM = 0x01;
const unsigned char GLOBAL_BATTLE_ITEM   = 0x02;
const unsigned char GLOBAL_MAP_ITEM      = 0x04;

// Constants for character bit-masking for identification
const unsigned int GLOBAL_NO_CHARACTERS  = 0x00000000;
const unsigned int GLOBAL_CLAUDIUS       = 0x00000001;
const unsigned int GLOBAL_LAILA          = 0x00000002;
const unsigned int GLOBAL_ALL_CHARACTERS = 0xFFFFFFFF;

// Constants for elemental types
const unsigned char GLOBAL_NO_ELEMENTAL        = 0x00;
const unsigned char GLOBAL_FIRE_ELEMENTAL      = 0x01;
const unsigned char GLOBAL_ICE_ELEMENTAL       = 0x02;
const unsigned char GLOBAL_LIGTHNING_ELEMENTAL = 0x04;
const unsigned char GLOBAL_EARTH_ELEMENTAL     = 0x08;
const unsigned char GLOBAL_LIGHT_ELEMENTAL     = 0x10;
const unsigned char GLOBAL_DARK_ELEMENTAL      = 0x20;

// Constants used to reflect various status
const unsigned int GLOBAL_NO_STATUS     = 0x00000000;
const unsigned int GLOBAL_POISON_STATUS = 0x00000001;
const unsigned int GLOBAL_SLOW_STATUS   = 0x00000002;
const unsigned int GLOBAL_SLEEP_STATUS  = 0x00000004;

// The maximum number of SP the characters can have
const int GLOBAL_MAX_SP = 100;

// Constants for commonly accessed sounds
const unsigned int GLOBAL_SOUND_CONFIRM = 0x00000001;
const unsigned int GLOBAL_SOUND_CANCEL  = 0x00000002;



/******************************************************************************
 GObject class - A parent class that all the different item classes inherit from
	
	>>>members<<< 
		unsigned char obj_type: value indicating what "type" the item is
		unsigned int usable_by: a bit-mask determining what characters can use the said object
		int obj_id: an identification number for each item.
		int obj_count: the number of items in this instance

	>>>functions<<< 
		

	>>>notes<<< 
		
 *****************************************************************************/
class GObject {
private:
	GObject(const GObject&) {}
	GObject& operator=(const GObject&) {}
protected:
	std::string obj_name;
	unsigned char obj_type;
	unsigned int usable_by;
	int obj_id;
	int obj_count;
public:
	GObject(std::string name, unsigned char type, unsigned int usable, int id, int count);
	GObject();
	~GObject();
	
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
};



/******************************************************************************
 GItem class - class for representing all game items 
	
	>>>members<<< 
		unsigned char use_case: a bit-mask stating where the item may be used (recovery, battle, etc.)

	>>>functions<<< 

	>>>notes<<< 
		
 *****************************************************************************/
class GItem : public GObject {
private:
	unsigned char use_case;
	
	GItem(const GItem&) {}
	GItem& operator=(const GItem&) {}
public:
	GItem(std::string name, unsigned char use, unsigned int usable, int id, int count);
	GItem();
	~GItem();
	
	void SetUseCase(unsigned char use) { use_case = use; }
	unsigned char GetUseCase() { return use_case; }
};
 
 
/******************************************************************************
 GSkillBook class - class for representing all game skill books
	
	>>>members<<< 

		vector<GSkill> book_skills: what skills the book contains
	>>>functions<<< 

	>>>notes<<< 
		
 *****************************************************************************/
class GSkillBook : public GObject {
private:
	std::vector<GSkill> skills;
	
	GSkillBook(const GSkillBook&) {}
	GSkillBook& operator=(const GSkillBook&) {}
public:
	GSkillBook(std::string name, unsigned char type, unsigned int usable, int id, int count);
	GSkillBook();
	~GSkillBook();
	
// void LoadBookSkills(); <<< fills up the skills vector. Or should this go in the GameData class?
};


/******************************************************************************
 GWeapon class - class for representing all game weapons
	
	>>>members<<< 

	>>>functions<<< 

	>>>notes<<< 
		
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


/******************************************************************************
 GArmor class - class for representing all game armors
	
	>>>members<<< 

	>>>functions<<< 

	>>>notes<<< 
		
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


/******************************************************************************
 GSkill class - class for representing all character and enemy skills
	
	>>>members<<< 
		unsigned int sp_usage: The amount of skill points that using the skill consumes

	>>>functions<<< 

	>>>notes<<< 
		
 *****************************************************************************/
class GSkill {
private:
	std::string skill_name;
	unsigned int sp_usage;
public:
	GSkill(std::string name, unsigned int sp);
	GSkill();
	~GSkill();
	GSkill(const GSkill&) {}
	GSkill& operator=(const GSkill&) {}
};



/******************************************************************************
 GAttackPoint class - class for representing "attack points" on a character or enemy
	
	>>>members<<< 
		float x_position: the x position where this AP is located on the sprite
		float y_position: the y position where this AP is located on the sprite
		
		int base_defense: the natural defense rating of the attack point
		int base_evade: the natrual evade rating of the attack point
		unsigned char elemental_weakness: bit mask of what elements the AP is vulerable to
		unsigned char elemental_resistance: bit mask of what elements the AP is resistant against
		unsigned char status_weakness: bit mask of what status afflictions the AP is vulernable to
		unsigned char status_resistance: bit mask of what status afflictions the AP is resistant against

	>>>functions<<< 

	>>>notes<<< 
	1) The x_pos and y_pos arguments determine where the attack point is located on a sprite. They are
			offsets of the bottom left corner of the sprite (that has a position of 0,0) and are always
			positive. Each value of 1.0 corresponds to one "virtual tile" in the battle code.
		
 *****************************************************************************/
class GAttackPoint {
private:
	float x_position;
	float y_position;
	
	unsigned int defense;
	unsigned int evade;
	unsigned char elemental_weakness;
	unsigned char elemental_resistance;
	unsigned char status_weakness;
	unsigned char status_resistance;
public:
	GAttackPoint(float x, float y, int def, int eva, unsigned char elem_weak, 
	             unsigned char elem_res, unsigned char stat_weak, unsigned char stat_res);
	GAttackPoint();
	~GAttackPoint();
	GAttackPoint(const GAttackPoint&) {}
	GAttackPoint& operator=(const GAttackPoint&) {}
	
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
};


/******************************************************************************
 GEnemy class - class for representing enemies
	
	>>>members<<< 
		std::string enemy_name: the name of the enemy
		int enemy_id: a unique id number is assigned to each enemy in the game
		int enemy_width: width of the enemy sprite
		int enemy_height: height of the enmy sprite
		std::vector<GSkill> enemy_skills: the skills that the enemy can use
		std::vecotr<GAttackPoint> attack_points: vector of the enemy's attack points
		std::vector<ImageDescripor> sprite_frames: the images of the sprite
		
		unsigned int hit_points: the current number of hit points
		unsigned int max_hit_points: maximum number of hit points
		unsigned int skill_points: current number of skill points
		unsigned int experience_points: number of experience points given when defeated
		unsigned int experience_level: current experience level
		unsigned int strength: enemy's strength
		unsigned int intelligence: enemy's intelligence
		unsigned int agility: enemy's agility
	>>>functions<<< 

	>>>notes<<< 
		1) One thing I haven't included here that we will need is some sort of call to an
			AI algorithm to decide what action the enemy should take. However, this is a rather
			complicated construct so for now I've left it out until we decide on how to manage
			our AI routines.
			
		2) The width and height arguments correspond to the number of "virtual tiles" the sprite
			image consumes in the battle code.
			
		3) Once we figure things out between handling this class between the map code and battle
			code a little more, we'll need to define the copy constructor and copy assignment
			operator. For now they are left blank.
 *****************************************************************************/
class GEnemy {
private:
	std::string enemy_name;
	int enemy_id;
	int enemy_width;
	int enemy_height;
	std::vector<GSkill> enemy_skills;
	std::vector<GAttackPoint> attack_points;
	std::vector<hoa_video::ImageDescriptor> sprite_frames;
	
	unsigned int hit_points;
	unsigned int max_hit_points;
	unsigned int skill_points;
	unsigned int experience_points;
	unsigned int experience_level;
	unsigned int strength;
	unsigned int intelligence;
	unsigned int agility;
	
	unsigned int base_hit_points;
	unsigned int base_experience_points;
	unsigned int base_strength;
	unsigned int base_intelligence;
	unsigned int base_agility;
	
	unsigned int growth_hit_points;
	unsigned int growth_experience_points;
	unsigned int growth_strength;
	unsigned int growth_intelligence;
	unsigned int growth_agility;
	
	float rate_hit_points;
	float rate_strength;
	float rate_intelligence;
	float rate_agility;
public:
// 	GEnemy() {}
// 	~GEnemy() {}
// 	GEnemy(const GEnemy&) {}
// 	GEnemy& operator=(const GEnemy&) {}
	
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
};


/******************************************************************************
	GCharacter - A class that represents a playable game character
		
	>>>members<<<
		std::string name: the name of the character
		GWeapon *eq_weapon: currently equipped weapon
		GArmor *eq_head: currently equipped head armor
		GArmor *eq_body: currently equipped body armor
		GArmor *eq_arms: currently equipped arm armor
		GArmor *eq_legs: currently equipped leg armor
		std::vector<GSkill> skills: the skills that the character can use
		
		unsigned int hit_points: the current number of hit points
		unsigned int max_hit_points: maximum number of hit points
		unsigned int skill_points: current number of skill points
		unsigned int experience_points: currently number of experience points
		unsigned int experience_level: current experience level
		unsigned int experience_next_level: experience points needed to gain an experienece level
		unsigned int strength: character's strength
		unsigned int intelligence: character's intelligence
		unsigned int agility: character's agility
	>>>functions<<<
		
	>>>notes<<<
		1) Almost always, a character will have four and only four attack points. However, I've
			left it as a vector here in case we have some special case later in the game where we
			want to add or remove attack points from a character.
 *****************************************************************************/
class GCharacter {
private:
	std::string name;
	GWeapon *eq_weapon;
	GArmor *eq_head;
	GArmor *eq_body;
	GArmor *eq_arms;
	GArmor *eq_legs;
	std::vector<GSkill> attack_skills;
	std::vector<GSkill> defense_skills;
	std::vector<GSkill> support_skills;
	std::vector<GAttackPoint> attack_points;
	std::vector<hoa_video::ImageDescriptor> map_frames;
	std::vector<hoa_video::ImageDescriptor> battle_frames;
	
	unsigned int hit_points;
	unsigned int max_hit_points;
	unsigned int skill_points;
	unsigned int experience_points;
	unsigned int experience_level;
	unsigned int experience_next_level;
	unsigned int strength;
	unsigned int intelligence;
	unsigned int agility;
public:
	GCharacter() {}
	~GCharacter() {}
	
	GWeapon* EquipWeapon(GWeapon *new_weapon);
	GArmor* EquipHeadArmor(GArmor *new_armor);
	GArmor* EquipBodyArmor(GArmor *new_armor);
	GArmor* EquipArmsArmor(GArmor *new_armor);
	GArmor* EquipLegsArmor(GArmor *new_armor);
	
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
};


/******************************************************************************
	GTime class - Saves information about the elapsed game time
		
	>>>members<<<
		
	>>>functions<<<
		
	>>>notes<<<
		
 *****************************************************************************/
class GTime {
private:
	friend class GameInstance;
	unsigned char seconds;
	unsigned char minutes;
	unsigned char hours;
public:
	void UpdateTime() {
		seconds++;
		if (seconds >= 60) {
			minutes++;
			if (minutes >= 60) {
				hours++;
			}
		}
	}
	void SetTime(unsigned char h, unsigned char m, unsigned char s) {
		hours = h;
		minutes = m;
		seconds = s;
	}
};

/******************************************************************************
	GameInstance class - Singleton containing information about the state of the game
		
	>>>members<<<
		
	>>>functions<<<
		
	>>>notes<<<
		
 *****************************************************************************/
class GameInstance {
private:
	SINGLETON_DECLARE(GameInstance);
	GTime game_time;
	std::vector<GCharacter> characters;
	std::vector<GObject> inventory;
	unsigned int money;
	
	
public:
	SINGLETON_METHODS(GameInstance);
	
};


} // namespace hoa_global

#endif
