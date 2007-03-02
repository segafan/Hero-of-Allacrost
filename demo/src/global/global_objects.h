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
#include "script.h"

#include "global_actors.h"
#include "global_skills.h"

namespace hoa_global {

/** \name GlobalObject Types
*** \brief Used for identification of different game object types
**/
enum GLOBAL_OBJECT {
	GLOBAL_OBJECT_INVALID     = -1,
	GLOBAL_OBJECT_ITEM        =  0,
	GLOBAL_OBJECT_WEAPON      =  1,
	GLOBAL_OBJECT_HEAD_ARMOR  =  2,
	GLOBAL_OBJECT_TORSO_ARMOR =  3,
	GLOBAL_OBJECT_ARM_ARMOR   =  4,
	GLOBAL_OBJECT_LEG_ARMOR   =  5,
	GLOBAL_OBJECT_JEWEL       =  6,
	GLOBAL_OBJECT_KEY_ITEM    =  7,
	GLOBAL_OBJECT_TOTAL       =  8
};


/** \name GlobalItem Usage Cases
*** \brief Enum values used for identification of different game object types
**/
enum GLOBAL_ITEM_USE {
	GLOBAL_ITEM_USE_INVALID = -1,
	GLOBAL_ITEM_USE_MENU    =  0,
	GLOBAL_ITEM_USE_BATTLE  =  1,
	GLOBAL_ITEM_USE_ALL     =  2,
	GLOBAL_ITEM_USE_TOTAL   =  3
};



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
		{ _id = 0; }

	virtual ~GlobalObject()
		{}

	/** \brief Increments the number of objects represented by the specified amount
	*** \param count The number of objects to add (a positive integer)
	**/
	void IncrementCount(uint32 count)
		{ _count += count; }

	/** \brief Decrements the number of objects represented by the specified amount
	*** \param count The number of objects to remove (a positive integer)
	*** \note When the count reaches zero, this class object will <b>not</b> self-destruct. It is the user's
	*** responsiblity to check if the count becomes zero, and to destroy the object if it is appropriate to do so.
	**/
	void DecrementCount(uint32 count)
		{ if (count > _count) _count = 0; else _count -= count; }

	//! \name Class Member Access Functions
	//@{
	uint32 GetID() const
		{ return _id; }

	hoa_utils::ustring GetName() const
		{ return _name; }

	hoa_utils::ustring GetDescription() const
		{ return _description; }

	GLOBAL_OBJECT GetType() const
		{ return _type; }

	uint32 GetUsableBy() const
		{ return _usable_by; }

	uint32 GetCount() const
		{ return _count; }
	//@}

protected:
	/** \brief An identification number for each unique item
	*** \note The ID number zero does not correspond to a valid item
	**/
	uint32 _id;

	/** \brief A numerical value that defines what type of object this is.
	*** See the GameObject Types constants for a list of the different object types.
	**/
	GLOBAL_OBJECT _type;

	//! \brief The name of the object as it would be displayed on a screen
	hoa_utils::ustring _name;

	//! \brief A short description of the item to display to the player
	hoa_utils::ustring _description;

	//! \brief How many items are represented within this class object instance
	uint32 _count;

	/** \brief A bit-mask that determines which characters can use the said object.
	*** See the "Game Character Types" constants in global_characters.h for more information
	**/
	uint32 _usable_by;

	//! \brief An image icon of the object
	hoa_video::StillImage _icon_image;

private:
	GlobalObject(const GlobalObject&);
	GlobalObject& operator=(const GlobalObject&);

	/** \brief Loads the item's data from a file and sets the members of the class
	***
	*** This function is essentially an assitant to the class constructor. It is
	*** required because GlobalObject requires at least one purely virtual function
	*** to be an abstract class.
	**/
	virtual void _Load() = 0;
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
	GlobalItem(uint32 id, uint32 count = 1)
		{ _id = id; _type = GLOBAL_OBJECT_ITEM; _count = count; _Load(); }
	
	~GlobalItem()
		{}

	/** \brief Calls the script function which performs the item's use
	*** \param target A void pointer to the target, which should be either a pointer to a
	*** GlobalAttackPoint, GlobalActor, or GlobalParty class object
	*** \note This will reduce the count member by zero. If the count member is already zero,
	*** this function will return without doing anything.
	**/
	void Use(GlobalTarget* target);

	//! \name Class Member Access Functions
	//@{
	GLOBAL_ITEM_USE GetUsage() const
		{ return _usage; }

	GLOBAL_TARGET GetTargetType() const
		{ return _target_type; }
	//@}

private:
	/** \brief Values to indicate where the item may be used
	*** Items may only be used in either menu mode or battle mode. If an item is to be used in another game mode,
	*** then it must rely on either the menu or battle use values.
	**/
	GLOBAL_ITEM_USE _usage;

	/** \brief The type of target for the item.
	*** Target types include attack points, actors, and parties. This enum  type is defined in global_skills.h
	**/
	GLOBAL_TARGET _target_type;

	//! \brief A reference to the script function that performs the items action.
	ScriptObject _function;

	GlobalItem(const GlobalItem&);
	GlobalItem& operator=(const GlobalItem&);

	//! \brief Loads the item's data from a file and sets the members of the class
	void _Load();
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
	GlobalWeapon(uint32 id, uint32 count = 1)
		{  _id = id; _type = GLOBAL_OBJECT_WEAPON; _count = count; _Load(); }

	~GlobalWeapon()
		{}

private:
	//! The amount of physical damage that the weapon causes
	uint32 _physical_attack;

	//! The amount of metaphysical damage that the weapon causes
	uint32 _metaphysical_attack;

	std::map<GLOBAL_ELEMENTAL, uint32> _elemental_bonuses;

	std::map<GLOBAL_STATUS, uint32> _status_bonuses;

	// TODO std::vector<GlobalGem*> _sockets;

	GlobalWeapon(const GlobalWeapon&);
	GlobalWeapon& operator=(const GlobalWeapon&);

	//! \brief Loads the weapons's data from a file and sets the members of the class
	void _Load();
}; // class GlobalWeapon : public GlobalObject



/** ****************************************************************************
*** \brief Represents the four types of armor found in the game
***
*** Not all pieces of armor can be equipped by all characters. Even though there's
*** only one armor class, there are actually four types of armor: head, torso, arm,
*** and leg. The GlobalObject#_type member is used to identify what armor category
*** an instance of this class belongs to. All armor have the same members/properties,
*** so it doesn't make any sense to make four identical classes different only in
*** name for the four armor types.
*** ***************************************************************************/
class GlobalArmor : public GlobalObject {
public:
	GlobalArmor(uint32 id, uint32 count = 1)
		{  _id = id; _count = count; _Load(); }

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

	//! \brief Loads the armor's data from a file and sets the members of the class
	void _Load();
}; // class GlobalArmor : public GlobalObject


/** ****************************************************************************
*** \brief Represents a shard item
***
*** Shards are small items that can be combined with weapons and armor to 
*** enhance their properties.
*** ***************************************************************************/
class GlobalShard : public GlobalObject {
	// TODO
};


/** ****************************************************************************
*** \brief Represents key items found in the game
***
*** Key items are items which can not be used by the player. They simply sit
*** idly in the inventory.
*** ***************************************************************************/
class GlobalKeyItem : public GlobalObject {
	// TODO
};


} // namespace hoa_global

#endif // __GLOBAL_OBJECTS_HEADER__
