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

#include "global_objects.h"
#include "global_actors.h"

//! All calls to global code are wrapped in this namespace.
namespace hoa_global {

//! The singleton pointer responsible for the management of global game data.
extern GameGlobal *GlobalManager;
//! Determines whether the code in the hoa_global namespace should print debug statements or not.
extern bool GLOBAL_DEBUG;

// enum TESTI {HP_POTION = 991};

/** ****************************************************************************
*** \brief Retains all the state information about the player's active game
***
*** This class is a resource manager for the current state of the game that is
*** being played. It retains all of the characters in the player's party, the
*** elapsed time, the party's inventory, and much more. This class assists the
*** various game modes by allowing them to share data with each other on a "global"
*** basis.
***
*** \note This class is a singleton, even though it is technically not an engine
*** manager class. There can only be one game instance that the player is playing
*** at any given time. The singleton object for this class is both created and
*** destroyed from the BootMode class, which is where the player selects to
*** either begin a new game or load an existing one.
*** ***************************************************************************/
class GameGlobal {
public:
	SINGLETON_METHODS(GameGlobal);

	/** \brief Adds a new character to the party.
	*** \param *ch A pointer to the GlobalCharacter object to add to the party.
	**/
	void AddCharacter(GlobalCharacter *ch);
	
	/** \brief Returns a pointer to a character currently in the party.
	*** \param id The ID number of the character to retrieve.
	*** \return A pointer to the character, or NULL if the character was not found.
	***/
	GlobalCharacter* GetCharacter(uint32 id);

	//! \name Funds Manipulation Functions
	//@{
	const uint32 GetFunds() const
		{ return _funds; }
	void SetFunds(uint32 amount)
		{ _funds = amount; }
	//! \note The overflow condition is not checked here: we just assume it will never occur
	void AddFunds(uint32 amount)
		{ _funds += amount; }
	//! \note The amount is only subtracted if the current funds is equal to or exceeds the amount to subtract
	void SubtractFunds(uint32 amount)
		{ if (_funds >= amount) _funds -= amount; }
	// @}

	//! \name Inventory Manipulation Functions
	//@{
	std::map<uint32, GlobalObject*> GetInventory() const
		{ return _inventory; }
	void AddItemToInventory(GlobalObject *obj);
	void RemoveFromInventory(GlobalObject *obj);
	//@}
	
	//! \brief Gets all of the characters in the active party
	GlobalCharacterParty GetActiveParty() const
		{ return _active_party; }

private:
	SINGLETON_DECLARE(GameGlobal);
	//! \brief The amount of financial resources the party currently has.
	uint32 _funds;

	/** \brief A map containing all characters that the player has discovered
	*** This map contains all characters that the player has met with, regardless of whether or not they are in the party.
	*** The map key is the character's unique ID number.
	**/
	std::map<uint32, GlobalCharacter*> _characters;
	/** \brief The entire inventory of the party
	*** This inventory stores all items, weapons, armor, etc. for the entire party in a map. The map key is the unique ID
	*** number for the object, which is duplicated in the GlobalObject class itself. The objects in this inventory have a
	*** count member associated with them that retains how many of that particular object is contained in the inventory.
	*** Once the count member reaches zero, the object is removed from this container.
	**/
	std::map<uint32, GlobalObject*> _inventory;

	/** \brief The active party of characters
	*** The active party contains the group of characters that will fight when a battle begins. This party can be up to
	*** four characters, and should always contain at least one character.
	**/
	GlobalCharacterParty _active_party;
}; // class GameGlobal

} // namespace hoa_global

#endif
