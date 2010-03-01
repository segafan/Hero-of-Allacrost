///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_utils.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for battle mode utility code
***
*** This file contains utility code that is shared among the various battle mode
*** classes.
*** ***************************************************************************/

#ifndef __BATTLE_UTILS_HEADER__
#define __BATTLE_UTILS_HEADER__

#include "defs.h"
#include "utils.h"

#include "global.h"

namespace hoa_battle {

namespace private_battle {

// This is used in timers to adjust the speed of the entire battle. I think its basically meant to be a
// TEMP variable
extern float timer_multiplier;

extern bool wait;


//! \name Screen dimension constants
//@{
//! \brief Battle scenes are visualized via an invisible grid of 64x64 tiles
const uint32 TILE_SIZE     = 64;
//! \brief The length of the screen in number of tiles (16 x 64 = 1024)
const uint32 SCREEN_LENGTH = 16;
//! \brief The height of the screen in number of tiles (12 x 64 = 768)
const uint32 SCREEN_HEIGHT = 12;
//@}


/** \name Action Type Constants
*** \brief Identifications for the types of actions a player's characters may perform
**/
//@{
const uint32 CATEGORY_ATTACK    = 0;
const uint32 CATEGORY_DEFEND    = 1;
const uint32 CATEGORY_SUPPORT   = 2;
const uint32 CATEGORY_ITEM      = 3;
//@}

//! Returned as an index when looking for a character or enemy and they do not exist
const uint32 INVALID_BATTLE_ACTOR_INDEX = 999;

//! When a battle first starts, this is the wait time for the slowest actor
const uint32 MAX_INIT_WAIT_TIME = 8000;

//! Warm up time for using items (try to keep short, should be constant regardless of item used)
const uint32 ITEM_WARM_UP_TIME = 1000;


//! \brief Used to indicate what state the overall battle is currently operating in
enum BATTLE_STATE {
	BATTLE_STATE_INVALID   = -1,
	BATTLE_STATE_INITIAL   =  0, //!< Character sprites are running in from off-screen to their battle positions
	BATTLE_STATE_NORMAL    =  1, //!< Normal state where player is watching actions play out and waiting for a turn
	BATTLE_STATE_COMMAND   =  2, //!< Player is choosing a command for a character
	BATTLE_STATE_EVENT     =  3, //!< A scripted event is taking place, suspending all standard action
	BATTLE_STATE_VICTORY   =  4, //!< Battle has ended with the characters victorious
	BATTLE_STATE_DEFEAT    =  5, //!< Battle has ended with the characters defeated
	BATTLE_STATE_TOTAL     =  6
};


//! \brief Represents the possible states that a BattleActor may be in
enum ACTOR_STATE {
	ACTOR_STATE_INVALID       = -1,
	//! Actor is recovering stamina so they can execute another action
	ACTOR_STATE_IDLE          =  0,
	//! Actor is finished with the idle state but has not yet selected an action to execute
	ACTOR_STATE_COMMAND       =  1,
	//! Actor has selected an action and is preparing to execute it
	ACTOR_STATE_WARM_UP       =  2,
	//! Actor is prepared to execute action and is waiting their turn to act
	ACTOR_STATE_READY         =  3,
	//! Actor is in the process of executing their selected action
	ACTOR_STATE_ACTING        =  4,
	//! Actor is finished with previous action execution and recovering
	ACTOR_STATE_COOL_DOWN     =  5,
	//! Actor has perished and is inactive in battle
	ACTOR_STATE_DEAD          =  6,
	//! Actor is in some state of paralysis and can not act nor recover stamina
	ACTOR_STATE_PARALYZED     =  7,
	ACTOR_STATE_TOTAL         =  8
};



/** \brief Enums for the various states that the CommandSupervisor class may be in
*** See the descriptions of the various views for the ActionWindow class to
*** understand what these constants represent
***
**/
enum COMMAND_STATE {
	COMMAND_STATE_INVALID = -1,
	//! Player is selecting the type of action to execute
	COMMAND_STATE_CATEGORY = 0,
	//! Player is selecting from a list of actions to execute
	COMMAND_STATE_ACTION = 1,
	//! Player is selecting the target to execute the action on
	COMMAND_STATE_TARGET = 2,
	//! Player is viewing information about the selected action
	COMMAND_STATE_INFORMATION = 3,
	COMMAND_STATE_TOTAL = 4
};


/** \brief Enums for the various states that the FinishWindow class may be in
*** See the descriptions of the various views for the ActionWindow class to
*** understand what these constants represent
***
**/
enum FINISH_WINDOW_STATE {
	FINISH_INVALID = -1,
	//! Announces that the player is victorious and notes any characters who have gained an experience level
	FINISH_WIN_ANNOUNCE = 0,
	//! Initial display of character stats
	FINISH_WIN_SHOW_GROWTH = 1,
	//! Performs countdown of XP (adding it to chars) and triggers level ups
	FINISH_WIN_COUNTDOWN_GROWTH = 2,
	//! All XP has been added (or should be added instantly), shows final stats
	FINISH_WIN_RESOLVE_GROWTH = 3,
	//! Display of any skills learned
	FINISH_WIN_SHOW_SKILLS = 4,
	//! Reports all drunes earned and dropped items obtained
	FINISH_WIN_SHOW_SPOILS = 5,
	//! Adds $ earned to party's pot
	FINISH_WIN_COUNTDOWN_SPOILS = 6,
	//! All money and items have been added
	FINISH_WIN_RESOLVE_SPOILS = 7,
	//! We've gone through all the states of the FinishWindow in Win form
	FINISH_WIN_COMPLETE = 8,
	//! Announces that the player has lost and queries the player for an action
	FINISH_LOSE_ANNOUNCE = 9,
	//! Used to double-confirm when the player selects to quit the game or return to the main menu
	FINISH_LOSE_CONFIRM = 10,
	FINISH_TOTAL = 11
};


//! \brief Position constants representing the significant locations along the stamina meter
//@{
//! \brief The bottom most position of the stamina bar
const float STAMINA_LOCATION_BOTTOM = 128.0f;

//! \brief The location where each actor is allowed to select a command
const float STAMINA_LOCATION_COMMAND = STAMINA_LOCATION_BOTTOM + 354.0f;

//! \brief The top most position of the stamina bar where actors are ready to execute their actions
const float STAMINA_LOCATION_TOP = STAMINA_LOCATION_BOTTOM + 508.0f;
//@}





/** ****************************************************************************
*** \brief Container class for representing the target of a battle action
***
*** Valid target types include attack points, actors, and parties. This class is
*** somewhat of a wrapper and allows a single instance of BattleTarget to represent
*** any of these types. It also contains a handful of methods useful in determining
*** the validity of a selected target and moving to another target of the same type.
*** ***************************************************************************/
class BattleTarget {
public:
	BattleTarget();

	~BattleTarget()
		{}

	/** \brief Sets the target to a specific attack point on an actor
	*** \param attack_point An integer index into the actor's attack points
	*** \param actor The actor to set for the target (default value == NULL)
	*** A NULL actor simply means that the class should continue pointing to the current actor.
	*** This is useful for cycling through attack points on an actor. Note that if the actor argument
	*** is NULL, the _actor member should not be NULL. If both are NULL, calling this method will
	*** result in no change.
	**/
	void SetAttackPointTarget(uint32 attack_point, BattleActor* actor = NULL);

	/** \brief Sets the target to an actor
	*** \param actor A pointer to the actor to set for the target
	**/
	void SetActorTarget(BattleActor* actor);

	/** \brief Sets the target to a party
	*** \param actor A pointer to the party to set for the target
	**/
	void SetPartyTarget(std::set<BattleActor*>* party);

	/** \brief Returns true if the target is valid
	*** This method assumes that a valid target is one that is alive (non-zero HP). If the target type
	*** is an actor or attack point, the function returns true so long as the target actor is alive.
	*** If the target type is a party, this method will always return true as parties always have at
	*** least one living actor unless the battle has ended.
	***
	*** Not all actions/skills/items should rely on this method for determining whether or not the
	*** target is valid for their particular circumstances. For example, a revive item is only valid
	*** to use on a dead actor. Other actions/items may have their own criteria for determining what
	*** is a valid target.
	**/
	bool IsValid();

	/** \brief Changes the target attack point to reference the next available attack point target
	*** \param direction Tells the method to look either forward or backward (true/false) for the next target
	*** \param valid_criteria When true the method will only select targets determined to be valid by IsTargetValid()
	*** \return True if the attack point or actor target was changed, false if no change took place
	***
	*** This method should only be invoked when the _type member is equal to GLOBAL_TARGET_ATTACK_POINT.
	*** Under normal circumstances this method will simply reference the next attack point available on
	*** the targeted actor. However, if the actor is deceased and the valid_criteria member is set to true,
	*** this will cause the method to look for the next available actor and call the SelectNextActor() method.
	***
	*** If the action/skill/item has special criteria for determining what type of a target is valid, the valid_criteria
	*** member should be set to false. This will ignore whether or not the target actor is deceased and allow external
	*** code to determine the validity of the new attack point target itself.
	**/
	bool SelectNextAttackPoint(bool direction = true, bool valid_criteria = true);

	/** \brief Changes the target actor to reference the next available actor
	*** \param direction Tells the method to look either forward or backward (true/false) for the next target
	*** \param valid_criteria When true the method will only select actors determined to be valid by IsTargetValid()
	*** \return True if the _actor member was changed, false if it was not
	***
	*** This method should only be called when the _type member is equal to GLOBAL_TARGET_ATTACK_POINT or
	*** GLOBAL_TARGET_ACTOR.
	**/
	bool SelectNextActor(bool direction = true, bool valid_criteria = true);

	//! \name Class member accessor methods
	//@{
	hoa_global::GLOBAL_TARGET GetType() const
		{ return _type; }

	uint32 GetAttackPoint() const
		{ return _attack_point; }

	BattleActor* GetActor() const
		{ return _actor; }

	std::set<BattleActor*>* GetParty() const
		{ return _party; }
	//@}

private:
	//! \brief The type of target this object represents (attack point, actor, or party)
	hoa_global::GLOBAL_TARGET _type;

	//! \brief An attack point to target, as an index to the proper point on the target_actor
	uint32 _attack_point;

	//! \brief The actor to target
	BattleActor* _actor;

	//! \brief The party to target
	std::set<BattleActor*>* _party;
}; // class BattleTarget


/** ****************************************************************************
*** \brief A simple container class for items that may be used in battle
***
*** This class adds an additional member to be associated with GlobalItem objects
*** which keeps track of how many of that item are available to use. This is necessary
*** because when an actor selects an item to use, they do not immediately use that
*** item and may ultimately not use the item due to the user becoming incapacitated
*** or having no valid target for the item. At all times, the available count of an item
*** will be less than or equal to the actual count of the item.
***
*** The proper way to use this class is to call the following methods for the following
*** situations.
***
*** - DecrementAvailableCount(): call when an actor has selected to use an item
*** - IncrementAvaiableAcount(): call when an actor does not use an item that it selected
*** - DecrementCount(): call when the item is actually used
***
*** \note Do not call the IncrementCount(), DecrementCount(), or SetCount() methods on the GlobalItem
*** pointer. This will circumvent the ability of this class to keep an accurate and correct available
*** count. Instead, use the IncrementCount() and DecrementCount() methods of this BattleItem class
*** directly.
*** ***************************************************************************/
class BattleItem {
public:
	//! \param item A pointer to the item to represent. Should be a non-NULL value.
	BattleItem(hoa_global::GlobalItem item);

	~BattleItem();

	//! \brief Class member accessor methods
	//@{
	hoa_global::GlobalItem& GetItem()
		{ return _item; }

	uint32 GetAvailableCount() const
		{ return _available_count; }
	//@}

	/** \brief Increases the available count of the item by one
	*** The available count will not be allowed to exceed the GlobalItem _count member
	**/
	void IncrementAvailableCount();

	/** \brief Decreases the available count of the item by one
	*** The available count will not be allowed to decrement below zero
	**/
	void DecrementAvailableCount();

	/** \brief A wrapper function that retrieves the actual count of the item
	*** \note Calling this function is equivalent to calling GetItem().GetCount()
	**/
	uint32 GetCount() const
		{ return _item.GetCount(); }

	/** \brief Increments the count of an item by one
	*** \note This method should not be called under normal battle circumstances
	**/
	void IncrementCount();

	/** \brief Decrements the count of the item by one
	*** Will also decrement the available count if the two counts are equal
	**/
	void DecrementCount();

private:
	//! \brief The item that this class represents
	hoa_global::GlobalItem _item;

	//! \brief The number of instances of this item that are available to be selected to be used
	uint32 _available_count;
}; // class BattleItem

} // namespace private_battle

} // namespace hoa_battle

#endif // __BATTLE_UTILS_HEADER__
