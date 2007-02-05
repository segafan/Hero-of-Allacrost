////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    global_objects.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for global game objects.
***
*** This file contains several representations of "objects" that need to be
*** used by many of the game mode classes. In this context, an object can be an
*** item, a weapon, a piece of armor, etc.
*** ***************************************************************************/

#ifndef __GLOBAL_OBJECTS_HEADER__
#define __GLOBAL_OBJECTS_HEADER__

#include "defs.h"
#include "utils.h"
#include "image.h"

namespace hoa_global {

/** \name GameObject Types
*** \brief Constants used for identification of different game object types
*** \note All armor types are identified using the upper nibble
**/
//@{
const uint8 GLOBAL_BAD_OBJECT  = 0x00;
const uint8 GLOBAL_ITEM        = 0x01;
const uint8 GLOBAL_WEAPON      = 0x02;
const uint8 GLOBAL_HEAD_ARMOR  = 0x04;
const uint8 GLOBAL_TORSO_ARMOR = 0x08;
const uint8 GLOBAL_ARMS_ARMOR  = 0x10;
const uint8 GLOBAL_LEGS_ARMOR  = 0x20;
//@}

/** \name GameItem Usage Types
*** \brief Constants for the numerous methods of application of game items
**/
//@{
const uint8 GLOBAL_BAD_USAGE        = 0x00;
const uint8 GLOBAL_MENU_USAGE       = 0x0F;
const uint8 GLOBAL_BATTLE_USAGE     = 0xF0;
//@}

/** ****************************************************************************
*** \brief An abstract parent class for representing a game object
***
*** This class serves as a way for the various game objects to share the same
*** code, although it may also be used to keep a collection of pointers that
*** point to different game objects that inherit from this class.
***
*** \note Each class object (or inherited class object) is designed so that
*** multiple numbers of the same object can be represented by only a single class
*** object. In other words, 50 healing potions are represented by only a single
*** class instance
*** ***************************************************************************/
class GlobalObject {
public:
	GlobalObject()
		{}

	GlobalObject(uint32 id, uint32 count = 0);

	~GlobalObject()
		{}

	//! \name Class Member Access Functions
	//@{
	uint32 GetID() const
		{ return _id; }

	hoa_utils::ustring GetName() const
		{ return _name; }

	uint8 GetType() const
		{ return _type; }

	uint32 GetUsableBy() const
		{ return _usable_by; }

	uint32 GetCount() const
		{ return _count; }

	std::string GetIconPath() const
		{ return _icon_path; }

	// NOTE: I think that some of these functions are potentially very dangerous. Changing
	// an object's ID changes the very object itself, and it should also change the name
	// and icon path of the object when the ID changes. We need to discuss these functions
	// further.
	void SetID(const uint32 id)
		{ _id = id; }

	void SetName(const hoa_utils::ustring name)
		{ _name = name; }

	void SetType(const uint8 type)
		{ _type = type; }

	void SetCount(const uint32 count)
		{ _count = count; }

	void SetUsableBy(const uint32 use)
		{ _usable_by = use; }

	void SetIconPath(const std::string icon_path)
		{ _icon_path = icon_path; }
	//@}

	/** \brief Increments the number of objects represented by the specified amount
	*** \param count The number of objects to add (a positive integer)
	**/
	void IncrementCount(uint32 count)
		{ _count += count; }
	/** \brief Decrements the number of objects represented by the specified amount
	*** \param count The number of objects to remove (a positive integer)
	*** \note When the count reaches zero, this class will not self-destruct. It is the user's
	*** responsiblity to destroy the class when it is appropriate to do so.
	**/
	void DecrementCount(uint32 count)
		{ if (count > _count) _count = 0; else _count -= count; }

protected:
	/** \brief An identification number for each unique item
	*** \note The ID number zero does not correspond to a valid item
	**/
	uint32 _id;

	//! \brief The name of the object as it would be displayed on a screen
	hoa_utils::ustring _name;

	/** \brief A numerical value that defines what type of object this is.
	*** See the GameObject Types constants for a list of the different object types.
	**/
	uint8 _type;

	//! \brief How many items are represented within this class object instance
	uint32 _count;

	/** \brief A bit-mask that determines which characters can use the said object.
	*** See the "Game Character Types" constants in global_characters.h for more information
	**/
	uint32 _usable_by;

	/** \brief The file path of the icon image for this object
	*** \note This string should never be empty
	*** \todo This member will eventually become defunct
	**/
	std::string _icon_path;

	//! \brief An image icon of the object
	hoa_video::StillImage _icon_image;

private:
	GlobalObject(const GlobalObject&);

	GlobalObject& operator=(const GlobalObject&);
}; // class GlobalObject

/** ****************************************************************************
*** \brief Represents items found and used throughout the game
***
*** This class is for "general" items such as healing potions. Each item has a
*** different effect when it is used, which is implemented by a small Lua
*** function written specifically for the item which calls it. Items may be
*** used in only certain game modes (battles, menus, etc.). Most items
*** can be used by any character, although some may only be used by certain
*** characters.
*** ***************************************************************************/
class GlobalItem : public GlobalObject {
public:
	GlobalItem()
		{}

	GlobalItem(uint32 id, uint32 count = 1);
	
	~GlobalItem()
		{}

	//! \name Class Member Access Functions
	//@{
	uint8 GetUsage() const
		{ return _usage; }
	void SetUsage(uint8 use)
		{ _usage = use; }
	//@}

private:
	//! \brief Indicates where the item may be used.
	//! See the GameItem Usage Types constants for a list of locations.
	uint8 _usage;

	// TODO: A reference to the script function to execute for this item
	// ScriptFunction *_script;


	GlobalItem(const GlobalItem&);
	GlobalItem& operator=(const GlobalItem&);
}; // class GlobalItem : public GlobalObject



/** ****************************************************************************
*** \brief Representing weapon that may be equipped by characters or enemies
***
*** It should be fairly obvious, but not all weapons can be equipped by all
*** characters.
***
*** ***************************************************************************/
class GlobalWeapon : public GlobalObject {
public:
	GlobalWeapon()
		{}
	GlobalWeapon(uint32 id, uint32 count = 0);
	~GlobalWeapon()
		{}

private:
	//! The amount of physical damage that the weapon causes
	uint32 _physical_attack;
	//! The amount of metaphysical damage that the weapon causes
	uint32 _metaphysical_attack;

	// TODO: Add elemental bonuses
	// std::vector<GlobalElementalEffect*> _elemental_attacks;
	// TODO: Add affliction bonuses
	// std::vector<GlobalStatusEffect*> _status_attacks;

	GlobalWeapon(const GlobalWeapon&);
	GlobalWeapon& operator=(const GlobalWeapon&);
}; // class GlobalWeapon : public GlobalObject



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
public:
	GlobalArmor()
		{}

	GlobalArmor(uint32 id, uint8 type, uint32 count);

	~GlobalArmor()
		{}

private:
	//! The amount of physical defense that the armor allows
	uint32 _physical_defense;
	//! The amount of metaphysical defense that the armor allows
	uint32 _metaphysical_defense;

	// TODO: Add elemental bonuses
	// std::vector<GlobalElementalEffect*> _elemental_defenses;
	// TODO: Add status effect bonuses
	// std::vector<GlobalStatusEffect*> _status_defenses;

	GlobalArmor(const GlobalArmor&);
	GlobalArmor& operator=(const GlobalArmor&);
}; // class GlobalArmor : public GlobalObject

} // namespace hoa_global

#endif // __GLOBAL_OBJECTS_HEADER__
