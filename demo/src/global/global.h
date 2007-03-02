////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    global.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for the global game manager
***
*** This file contains the GameGlobal class, which is used to manage all data
*** that is shared "globally" by the various game modes. For example, it
*** contains the current characters in the party, the party's inventory, etc.
*** The definition of characters, items, and other related global data are
*** implemented in the other global header files (e.g. global_actors.h). All
*** of these global files share the same hoa_global namespace.
*** ***************************************************************************/

#ifndef __GLOBAL_HEADER__
#define __GLOBAL_HEADER__

#include "defs.h"
#include "utils.h"
#include "script.h"

#include "global_objects.h"
#include "global_actors.h"
#include "global_skills.h"

//! \brief All calls to global code are wrapped in this namespace.
namespace hoa_global {

//! \brief The singleton pointer responsible for the management of global game data.
extern GameGlobal *GlobalManager;

//! \brief  Determines whether the code in the hoa_global namespace should print debug statements or not.
extern bool GLOBAL_DEBUG;

/** ****************************************************************************
*** \brief Retains all the state information about the player's active game
***
*** This class is a resource manager for the current state of the game that is
*** being played. It retains all of the characters in the player's party, the
*** party's inventory, etc. This class assists the various game modes by allowing
*** them to share data with each other on a "global" basis.
***
*** \note This class is a singleton, even though it is technically not an engine
*** manager class. There can only be one game instance that the player is playing
*** at any given time.
*** ***************************************************************************/
class GameGlobal {
	friend class GlobalItem;
	friend class GlobalWeapon;
	friend class GlobalArmor;

public:
	SINGLETON_METHODS(GameGlobal);

	/** \brief Makes all relevant global classes and methods available to Lua.
	*** This function only needs to be called once when the application starts.
	**/
	static void BindToLua();

	/** \brief Deletes all data stored within the GameGlobal class object
	*** This function is meant to be called when the user quits the current game instance
	*** and returns to the boot screen. It will delete all characters, inventory, and other
	*** data relevant to the current game.
	**/
	void ClearAllData();

	/** \brief Adds a new character to the party.
	*** \param id The ID number of the character to add to the party.
	**/
	void AddCharacter(uint32 id);

	/** \brief Removes a character from the party.
	*** \param id The ID number of the character to remove from the party.
	**/
	void RemoveCharacter(uint32 id);

	/** \brief Adds a new item to the inventory
	*** \param obj_id The identifier value of the object to add
	*** \param obj_count The number of instances of the object to add (default == 1)
	*** If the item already exists in the inventory, then instead the GlobalObject#_count member is used to
	*** increment the count of the stored item.
	**/
	void AddToInventory(uint32 obj_id, uint32 obj_count = 1);

	/** \brief Removes an item from the inventory
	*** \param obj_id The identifier value of the item to remove
	*** \note If the item is not in the inventory, the function will do nothing.
	***
	*** This function removes the item regardless of what the GlobalObject#_count member is set to.
	*** If you want to remove only a certain number of instances of the item, use the function
	*** GameGlobal#DecrementObjectCount.
	**/
	void RemoveFromInventory(uint32 obj_id);

	/** \brief Increments the number (count) of an object in the inventory
	*** \param item_id The integer identifier of the item that will have its count incremented
	*** \param count The amount to increase the object's count by
	***
	*** If the item does not exist in the inventory, this function will do nothing. If the count parameter
	*** is set to zero, no change will take place. Overflow conditions are not checked.
	***
	*** \note The callee can not assume that the function call succeeded, but rather has to check this themselves.
	**/
	void IncrementObjectCount(uint32 obj_id, uint32 obj_count);

	/** \brief Decrements the number (count) of an object in the inventory
	*** \param item_id The integer identifier of the item that will have its count decremented
	*** \param count The amount to decrease the object's count by
	***
	*** If the item does not exist in the inventory, this function will do nothing. If the count parameter
	*** is set to zero, no change will take place. If the count parameter is greater than or equal to the
	*** current count of the object, the object will be removed from the inventory.
	***
	*** \note The callee can not assume that the function call succeeded, but rather has to check this themselves.
	**/
	void DecrementObjectCount(uint32 obj_id, uint32 obj_count);

	//! \note The overflow condition is not checked here: we just assume it will never occur
	void AddFunds(uint32 amount)
		{ _funds += amount; }

	//! \note The amount is only subtracted if the current funds is equal to or exceeds the amount to subtract
	void SubtractFunds(uint32 amount)
		{ if (_funds >= amount) _funds -= amount; }

	//! \name Class Member Access Functions
	//@{
	void SetFunds(uint32 amount)
		{ _funds = amount; }

	/** \brief Returns a pointer to a character currently in the party.
	*** \param id The ID number of the character to retrieve.
	*** \return A pointer to the character, or NULL if the character was not found.
	***/
	GlobalCharacter* GetCharacter(uint32 id);

	const uint32 GetFunds() const
		{ return _funds; }

	std::vector<GlobalItem*>* GetInventoryItems()
		{ return &_inventory_items; }

	std::vector<GlobalWeapon*>* GetInventoryWeapons()
		{ return &_inventory_weapons; }

	std::vector<GlobalArmor*>*  GetInventoryHeadArmor()
		{ return &_inventory_head_armor; }

	std::vector<GlobalArmor*>*  GetInventoryTorsoArmor()
		{ return &_inventory_torso_armor; }

	std::vector<GlobalArmor*>*  GetInventoryArmArmor()
		{ return &_inventory_arm_armor; }

	std::vector<GlobalArmor*>*  GetInventoryLegArmor()
		{ return &_inventory_leg_armor; }

	std::vector<GlobalShard*>* GetInventoryShards()
		{ return &_inventory_shards; }

	std::vector<GlobalKeyItem*>* GetInventoryKeyItems()
		{ return &_inventory_key_items; }
	//@}

	//! \brief Returns a pointer to the active party
	GlobalParty* GetActiveParty()
		{ return &_active_party; }

private:
	SINGLETON_DECLARE(GameGlobal);
	//! \brief The amount of financial resources the party currently has.
	uint32 _funds;

	/** \brief A map containing all characters that the player has discovered
	*** This map contains all characters that the player has met with, regardless of whether or not they are in the active party.
	*** The map key is the character's unique ID number.
	**/
	std::map<uint32, GlobalCharacter*> _characters;

	/** \brief Retains a list of all of the objects currently stored in the player's inventory
	*** This map is used to quickly check if an item is in the inventory or not. The key to the map is the object's
	*** identification number. When an object is added to the inventory, if it already exists then the object counter
	*** is simply increased instead of adding an entire new class object. When the object count becomes zero, the object
	*** is removed from the inventory. Duplicates of all objects are retained in the various inventory containers below.
	**/
	std::map<uint32, GlobalObject*> _inventory;

	/** \brief Inventory containers
	*** These vectors contain the inventory of the entire party. The vectors are sorted according to the user's personal preferences.
	*** When a new object is added to the inventory, by default it will be placed at the end of the vector.
	**/
	//@{
	std::vector<GlobalItem*>    _inventory_items;
	std::vector<GlobalWeapon*>  _inventory_weapons;
	std::vector<GlobalArmor*>   _inventory_head_armor;
	std::vector<GlobalArmor*>   _inventory_torso_armor;
	std::vector<GlobalArmor*>   _inventory_arm_armor;
	std::vector<GlobalArmor*>   _inventory_leg_armor;
	std::vector<GlobalShard*>   _inventory_shards;
	std::vector<GlobalKeyItem*> _inventory_key_items;
	//@}

	/** \brief The active party of characters
	*** The active party contains the group of characters that will fight when a battle begins. This party can be up to
	*** four characters, and should always contain at least one character.
	**/
	GlobalParty _active_party;

	//! \brief Script files that hold data for various global objects
	//@{
	hoa_script::ScriptDescriptor _items_script;
	hoa_script::ScriptDescriptor _weapons_script;
	hoa_script::ScriptDescriptor _armor_script;
	//@}
}; // class GameGlobal

} // namespace hoa_global

#endif
