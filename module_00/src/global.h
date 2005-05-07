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


namespace hoa_global {

const bool GLOBAL_DEBUG = true;

// gitem = identifiers for different types of weapons
enum gitem { blank_i, item_i, sbook_i, weapon_i, armor_i };







/******************************************************************************
 GameItem class - A parent class that all the different item classes inherit from
	
	>>>members<<< 
		gitem itype: value indicating what "type" the item is
		int item_id: an identification number for each item.
		int icount: the number of items in this instance

	>>>functions<<< 
		

	>>>notes<<< 
		
 *****************************************************************************/
// class GameItem {
// protected:
// 	gitem itype;
// 	int item_id;
// 	int icount;
// public:
// 	GameItem();
// 	GameItem( gitem );
// 	bool operator== ( const cItem & ) const;
// 	bool operator< ( const cItem & ) const;
// 	bool operator> ( const cItem & ) const;
// 	bool operator< ( const cItem & ) const;
// 	bool operator< ( const cItem & ) const;
// 	GameItem & operator= ( const cItem & );
// 	GameItem& operator++ ();
// 	GameItem& operator-- ();
// 	GameItem& operator=+ ();
// 	GameItem& operator=- ();
// }; // class GameItem



/******************************************************************************
 GItem class - A parent class that all the different item classes inherit from
	
	>>>members<<< 
		int effect: a bit-mask stating what type of item it is (recovery, battle, map, etc)

	>>>functions<<< 
		Use(): use the item by executing it's function and decrementing it's icount

	>>>notes<<< 
		
 *****************************************************************************/
// class GItem {
// public:
// 	int effect;
// 	Use();
// };
// 
// class GSkillBook {
// 	int useable;
// };
// 
// class GWeapon {
// 
// };
// 
// class GArmor {
// 
// };
// 
// 
// class GItemPack { 
// 	 std::map< GameItem, unsigned int > contents;
// public:
// 
// 	 GItemPack( GItem & , unsigned long );
// 	 void clear( );
// 	 unsigned long add( const cItem & , const unsigned long );
// 	 unsigned long remove( const cItem & , const unsigned long );
// 	 unsigned long getAmount( const cItem & ) const;
// 	 const std::map< cItem, unsigned long > & getItems( ) const;
// 	 GItemPack & operator= ( const cItemPack & );
// 	 GItemPack & operator+= ( const cItemPack & );
// 	 GItemPack operator+ ( const cItemPack & ) const;
// 
// };
// 
// class GameParty;
// 	
// 
// class GameCharacter {
// private:
// 	 std::string name;
// 	 Weapon eq_weapon;
// 	 Armor eq_head;
// 	 Armor eq_body;
// 	 Armor eq_arms;
// 	 Armor eq_legs;
// 	 std::vector<Skill> attack_skills;
// 	 std::vector<Skill> defense_skills;
// 	 std::vector<Skill> support_skills;
// 	 // AttackPoints[4]
// 	 unsigned int hit_points;
// 	 unsigned int skill_points;
// 	 unsigned int xp_points;
// 	 unsigned int xp_level;
// 	 unsigned int xp_next_lvl;
// 	 unsigned int strength;
// 	 unsigned int intelligence;
// 	 unsigned int agility;
// 	 friend GameParty;
// public:
// 	 Weapon EquipWeapon(Weapon new_weapon);
// 	 Armor EquipHeadArmor(Armor new_armor);
// 	 Armor EquipBodyArmor(Armor new_armor);
// 	 Armor EquipArmsArmor(Armor new_armor);
// 	 Armor EquipLegsArmor(Armor new_armor);
// };


} // namespace hoa_global

#endif
