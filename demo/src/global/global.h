////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    global.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \brief   Header file for the global game manager
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

#include "global_objects.h"
#include "global_actors.h"

//! All calls to global code are wrapped in this namespace.
namespace hoa_global {

//! The singleton pointer responsible for the management of global game data.
extern GameGlobal *GlobalManager;
//! Determines whether the code in the hoa_boot namespace should print debug statements or not.
extern bool GLOBAL_DEBUG;


enum TESTI {HP_POTION = 991};
/** ****************************************************************************
*** \brief Represents a party of characters
***
*** This class contains a group or "party" of characters. The purpose of this class
*** is tied mostly to convience for the GameGlobal class, as characters may need
*** to be organized into groups. For example, the characters that are in the active
*** party versus the "reserved" party, or in the case of when the characters split
*** into multiple parties according with the story or to achieve a particular goal.
***
*** \note The characters that are in the party are not deleted by this class. Remember
*** this because it may lead to memory leaks if one is not careful.
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
// 	std::vector<GlobalCharacter*>& GetCharacters() const
// 		{ return _characters; }
	uint32 GetPartySize() const
		{ return _characters.size(); }
	bool IsPartyEmpty() const
		{ return (_characters.size() == 0); }
	//@}

	/** \brief Adds a character to the party
	*** \param character A pointer to the character to add
	*** Note that if the character is found to already be in the party, the character will not
	*** be added a second time.
	**/
	void AddCharacter(GlobalCharacter* character);
	/** \brief Removes a character from the party
	*** \param character A pointer to the character to remove
	*** \return A pointer to the character that was removed, or NULL if the character was not found in the party
	**/
	GlobalCharacter* RemoveCharacter(GlobalCharacter* character);

	// Raging_Hog: Is this good?
	std::vector<GlobalCharacter*> GetCharacters(){ return _characters; }

private:
	//! \brief The characters that are in this party
	std::vector<GlobalCharacter*> _characters;
}; // class GlobalCharacterParty



/** ****************************************************************************
*** \brief Retains all the state information about the player's active game
***
*** This class is best viewed as a resource manager for the current game that is
*** being played. It retains all of the characters in the player's party, the
*** elapsed time, the party's inventory, and much more. This class assists the
*** various game modes by allowing them to share data with each other on a "global"
*** basis.
***
*** \note This class is a singleton, even though it is technically not an engine
*** manager class. There can only be one game instance that the player is playing
*** at any given time
*** ***************************************************************************/
class GameGlobal {
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
	const uint32 GetMoney()
		{ return _money; }
	void SetMoney(uint32 amount)
		{ _money = amount; }
	void AddMoney(uint32 amount)
		{ _money += amount; }
	void SubtractMoney(uint32 amount)
		{ if (_money > amount) _money -= amount; }
	// @}

	//! Inventory Functions
	//! GetInventory returns the entire inventory.
	//!		This function returns a reference so the inventory can be edited directly
	//! AddItemToInventory(GlobalObject &) adds the given object to the inventory
	//@{
	std::vector<GlobalObject *> &GetInventory()
		{ return _inventory; }
	void AddItemToInventory(GlobalObject *obj);
	void RemoveFromInventory(GlobalObject *obj);
	//@}

	//! Item functions
	//! GetItemName returns the string name of the item
	//! GetItemIconPath return the icon path for the given item id
	//! SetItemName allows you to set an item's name
	//! SetItemIconPath allows you to set an item's icon path
	//@{
	std::string GetItemName(uint32 id)
		{ return _game_item_names[id]; }
	std::string GetItemIconPath(uint32 id)
		{ return _game_item_icon_paths[id]; }
	void SetItemName(uint32 key, std::string value)
		{ _game_item_names[key] = value; }
	void SetItemIconPath(uint32 key, std::string value)
		{ _game_item_icon_paths[key] = value; }
	//@}
	
	//! Gets the Characters in the active party
	//! \returns The Characters in the active party
	std::vector<GlobalCharacter *> GetParty();

private:
	SINGLETON_DECLARE(GameGlobal);
	/** \brief A map containing all characters that the player has discovered
	*** This map contains all characters that the player has met with, regardless of whether or not they are in the party.
	*** The map key is the character's unique ID number.
	**/
	std::vector<GlobalCharacter*> _characters;
	//! The inventory of the party.
	std::vector<GlobalObject*> _inventory;
	//! \brief The amount of financial resources the party currently has.
	uint32 _money;
	//! The active party (currently only support for one party, may need to be changed)
	GlobalCharacterParty _party;
	//! \brief the string names of the items
	std::map<uint32, std::string> _game_item_names;
	//! \brief the icon path of the item
	std::map<uint32, std::string> _game_item_icon_paths;
}; // class GameGlobal

} // namespace hoa_global

#endif
