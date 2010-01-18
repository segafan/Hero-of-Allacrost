////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
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

#include "video.h"
#include "script.h"

#include "global_actors.h"
#include "global_effects.h"

namespace hoa_global {

namespace private_global {

/** \name Object ID Range Constants
*** These constants set the maximum valid ID ranges for each object category.
*** The full valid range for each object category ID is:
*** - Items:            1-10000
*** - Weapons:      10001-20000
*** - Head Armor:   20001-30000
*** - Torso Armor:  30001-40000
*** - Arm Armor:    40001-50000
*** - Leg Armor:    50001-60000
*** - Shards:       60001-70000
*** - Key Items:    70001-80000
**/
//@{
const uint32 OBJECT_ID_INVALID   = 0;
const uint32 MAX_ITEM_ID         = 10000;
const uint32 MAX_WEAPON_ID       = 20000;
const uint32 MAX_HEAD_ARMOR_ID   = 30000;
const uint32 MAX_TORSO_ARMOR_ID  = 40000;
const uint32 MAX_ARM_ARMOR_ID    = 50000;
const uint32 MAX_LEG_ARMOR_ID    = 60000;
const uint32 MAX_SHARD_ID        = 70000;
const uint32 MAX_KEY_ITEM_ID     = 80000;
const uint32 OBJECT_ID_EXCEEDS   = 80001;
//@}

} // namespace private_global

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
	GLOBAL_OBJECT_SHARD       =  6,
	GLOBAL_OBJECT_KEY_ITEM    =  7,
	GLOBAL_OBJECT_TOTAL       =  8
};

/** \brief Creates a new type of GlobalObject
*** \param id The id value of the object to create
*** \param count The count of the new object to create (default value == 1)
*** \return A pointer to the newly created GlobalObject, or NULL if the object could not be created
***
*** This function actually does not create a GlobalObject (it can't, since its an abstract class),
*** but rather creates one of the derived object class types depending on the value of the id argument.
**/
GlobalObject* GlobalCreateNewObject(uint32 id, uint32 count = 1);


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
	GlobalObject() :
		_id(0), _count(0) {}

	GlobalObject(uint32 id, uint32 count) :
		_id(id), _count(count) {}

	virtual ~GlobalObject()
		{}

	/** \brief Purely virtual function used to distinguish between object types
	*** \return A value that represents the type of object
	**/
	virtual GLOBAL_OBJECT GetObjectType() const = 0;

	/** \brief Increments the number of objects represented by the specified amount
	*** \param count The number of objects to add (default value == 1)
	**/
	void IncrementCount(uint32 count = 1)
		{ _count += count; }

	/** \brief Decrements the number of objects represented by the specified amount
	*** \param count The number of objects to remove (default value == 1)
	*** \note When the count reaches zero, this class object will <b>not</b> self-destruct. It is the user's
	*** responsiblity to check if the count becomes zero, and to destroy the object if it is appropriate to do so.
	**/
	void DecrementCount(uint32 count = 1)
		{ if (count > _count) _count = 0; else _count -= count; }

	//! \name Class Member Access Functions
	//@{
	uint32 GetID() const
		{ return _id; }

	hoa_utils::ustring GetName() const
		{ return _name; }

	hoa_utils::ustring GetDescription() const
		{ return _description; }

	uint32 GetCount() const
		{ return _count; }

	void SetCount(uint32 count)
		{ _count = count; }

	uint32 GetPrice() const
		{ return _price; }

	const hoa_video::StillImage& GetIconImage() const
		{ return _icon_image; }
	//@}

protected:
	/** \brief An identification number for each unique item
	*** \note An ID number of zero indicates an invalid object
	**/
	uint32 _id;

	//! \brief The name of the object as it would be displayed on a screen
	hoa_utils::ustring _name;

	//! \brief A short description of the item to display on the screen
	hoa_utils::ustring _description;

	//! \brief How many occurences of the object are represented by this class object instance
	uint32 _count;

	//! \brief The listed price of the object in the game's markets
	uint32 _price;

	//! \brief The image icon of the object
	hoa_video::StillImage _icon_image;
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
***
*** \todo Item script functions should take abstract target type class pointers,
*** not character or actor pointers.
*** ***************************************************************************/
class GlobalItem : public GlobalObject {
public:
	GlobalItem(uint32 id, uint32 count = 1);

	~GlobalItem();

	GlobalItem(const GlobalItem& copy);

	GlobalItem& operator=(const GlobalItem& copy);

	GLOBAL_OBJECT GetObjectType() const
		{ return GLOBAL_OBJECT_ITEM; }

	//! \brief Returns true if the item can be used in battle mode
	bool IsUsableInBattle()
		{ return (_battle_use_function != NULL); }

	//! \brief Returns true if the item can be used in menu mode
	bool IsUsableInMenu()
		{ return (_menu_use_function != NULL); }

	//! \name Class Member Access Functions
	//@{
	GLOBAL_TARGET GetTargetType() const
		{ return _target_type; }

	bool IsTargetAlly() const
		{ return _target_ally; }

	/** \brief Returns a pointer to the ScriptObject of the battle use function
	*** \note This function will return NULL if the skill is not usable in battle
	**/
	const ScriptObject* GetBattleUseFunction() const
		{ return _battle_use_function; }

	/** \brief Returns a pointer to the ScriptObject of the menu use function
	*** \note This function will return NULL if the skill is not usable in menus
	**/
	const ScriptObject* GetMenuUseFunction() const
		{ return _menu_use_function; }
	//@}

private:
	/** \brief The type of target for the item.
	*** Target types include attack points, actors, and parties. This enum type is defined in global_actors.h
	**/
	GLOBAL_TARGET _target_type;

	//! \brief If true the item should target allies, otherwise it should target enemies
	bool _target_ally;

	//! \brief A pointer to the script function that performs the item's effect while in battle.
	ScriptObject* _battle_use_function;

	//! \brief A pointer to the script function that performs the item's effect while in a menu
	ScriptObject* _menu_use_function;
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
	GlobalWeapon(uint32 id, uint32 count = 1);

	~GlobalWeapon()
		{}

	GLOBAL_OBJECT GetObjectType() const
		{ return GLOBAL_OBJECT_WEAPON; }

	uint32 GetPhysicalAttack() const
		{ return _physical_attack; }

	uint32 GetMetaphysicalAttack() const
		{ return _metaphysical_attack; }

	uint32 GetUsableBy() const
		{ return _usable_by; }

	const std::vector<GlobalShard*>& GetSockets() const
		{ return _sockets; }

	const std::map<GLOBAL_ELEMENTAL, GLOBAL_INTENSITY>& GetElementalEffects() const
		{ return _elemental_effects; }

private:
	//! \brief The amount of physical damage that the weapon causes
	uint32 _physical_attack;

	//! \brief The amount of metaphysical damage that the weapon causes
	uint32 _metaphysical_attack;

	/** \brief A bit-mask that determines which characters can use or equip the object
	*** See the Game Character ID constants in global_actors.h for more information
	**/
	uint32 _usable_by;

	/** \brief Sockets which may be used to place shards on the weapon
	*** Many weapons may have no sockets, so it is not uncommon for the size of this vector to be
	*** zero. When a socket is available but empty (has no attached shard), the pointer at that index
	*** will be NULL.
	**/
	std::vector<GlobalShard*> _sockets;

	/** \brief Container that holds the intensity of each type of elemental effect of the weapon
	*** No elemental effect is indicated by an intensity of GLOBAL_INTENSITY_NEUTRAL
	**/
	std::map<GLOBAL_ELEMENTAL, GLOBAL_INTENSITY> _elemental_effects;

	// TODO: Add status effects to weapons
	// std::map<GLOBAL_STATUS, GLOBAL_INTENSITY> _status_effects;

}; // class GlobalWeapon : public GlobalObject


/** ****************************************************************************
*** \brief Represents all four types of armor found in the game
***
*** Not all pieces of armor can be equipped by all characters. Even though there's
*** only one armor class, there are actually four types of armor: head, torso, arm,
*** and leg. The GetObjectType method is used to identify what armor category
*** an instance of this class belongs to. All four types of armor have the same
*** members/properties.
*** ***************************************************************************/
class GlobalArmor : public GlobalObject {
public:
	GlobalArmor(uint32 id, uint32 count = 1);

	~GlobalArmor()
		{}

	GLOBAL_OBJECT GetObjectType() const;

	uint32 GetPhysicalDefense() const
		{ return _physical_defense; }

	uint32 GetMetaphysicalDefense() const
		{ return _metaphysical_defense; }

	uint32 GetUsableBy() const
		{ return _usable_by; }

	const std::vector<GlobalShard*>& GetSockets() const
		{ return _sockets; }

	const std::map<GLOBAL_ELEMENTAL, GLOBAL_INTENSITY>& GetElementalEffects() const
		{ return _elemental_effects; }

private:
	//! \brief The amount of physical defense that the armor provides
	uint32 _physical_defense;

	//! \brief The amount of metaphysical defense that the armor provides
	uint32 _metaphysical_defense;

	/** \brief A bit-mask that determines which characters can use or equip the object
	*** See the Game Character ID constants in global_actors.h for more information
	**/
	uint32 _usable_by;

	/** \brief Sockets which may be used to place shards on the armor
	*** Many armor may have no sockets, so it is not uncommon for the size of this vector to be
	*** zero. When a socket is available but empty (has no attached shard), the pointer at that index
	*** will be NULL.
	**/
	std::vector<GlobalShard*> _sockets;

	/** \brief Container that holds the intensity of each type of elemental effect of the armor
	*** No elemental effect is indicated by an intensity of GLOBAL_INTENSITY_NEUTRAL
	**/
	std::map<GLOBAL_ELEMENTAL, GLOBAL_INTENSITY> _elemental_effects;

	// TODO: Add status effects to weapons
	// std::map<GLOBAL_STATUS, GLOBAL_INTENSITY> _status_effects;
}; // class GlobalArmor : public GlobalObject


/** ****************************************************************************
*** \brief Represents a shard item
***
*** Shards are small items that can be combined with weapons and armor to
*** enhance their properties.
***
*** \todo This class is not yet implemented
*** ***************************************************************************/
class GlobalShard : public GlobalObject {
public:
	GlobalShard(uint32 id, uint32 count = 1) :
		GlobalObject(id, count) {}

	GLOBAL_OBJECT GetObjectType() const
		{ return GLOBAL_OBJECT_SHARD; }
}; // class GlobalShard : public GlobalObject


/** ****************************************************************************
*** \brief Represents key items found in the game
***
*** Key items are items which can not be used by the player. They simply sit
*** idly in the inventory.
***
*** \todo This class is not yet implemented
*** ***************************************************************************/
class GlobalKeyItem : public GlobalObject {
public:
	GlobalKeyItem(uint32 id, uint32 count = 1) :
		GlobalObject(id, count) {}

	GLOBAL_OBJECT GetObjectType() const
		{ return GLOBAL_OBJECT_KEY_ITEM; }
}; // class GlobalKeyItem : public GlobalObject

} // namespace hoa_global

#endif // __GLOBAL_OBJECTS_HEADER__
