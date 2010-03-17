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

#include "system.h"

#include "global_objects.h"
#include "global_utils.h"

namespace hoa_battle {

namespace private_battle {

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


//! \brief Position constants representing the significant locations along the stamina meter
//@{
//! \brief The bottom most position of the stamina bar
const float STAMINA_LOCATION_BOTTOM = 128.0f;

//! \brief The location where each actor is allowed to select a command
const float STAMINA_LOCATION_COMMAND = STAMINA_LOCATION_BOTTOM + 354.0f;

//! \brief The top most position of the stamina bar where actors are ready to execute their actions
const float STAMINA_LOCATION_TOP = STAMINA_LOCATION_BOTTOM + 508.0f;
//@}


//! \brief Returned as an index when looking for a character or enemy and they do not exist
const uint32 INVALID_BATTLE_ACTOR_INDEX = 999;

//! \brief When a battle first starts, this is the wait time for the slowest actor
const uint32 MAX_INIT_WAIT_TIME = 8000;

//! \brief Warm up time for using items (try to keep short, should be constant regardless of item used)
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
	ACTOR_STATE_IDLE          =  0, //!< Actor is recovering stamina so they can execute another action
	ACTOR_STATE_COMMAND       =  1, //!< Actor is finished with the idle state but has not yet selected an action to execute
	ACTOR_STATE_WARM_UP       =  2, //!< Actor has selected an action and is preparing to execute it
	ACTOR_STATE_READY         =  3, //!< Actor is prepared to execute action and is waiting their turn to act
	ACTOR_STATE_ACTING        =  4, //!< Actor is in the process of executing their selected action
	ACTOR_STATE_COOL_DOWN     =  5, //!< Actor is finished with previous action execution and recovering
	ACTOR_STATE_DEAD          =  6, //!< Actor has perished and is inactive in battle
	ACTOR_STATE_PARALYZED     =  7, //!< Actor is in some state of paralysis and can not act nor recover stamina
	ACTOR_STATE_TOTAL         =  8
};



//! \brief Enums for the various states that the CommandSupervisor class may be in
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


//! \brief Enums for the various states that the FinishWindow class may be in
enum FINISH_STATE {
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




/** \name Command battle calculation functions
*** These functions perform many of the common calculations that are needed in battle such as determining
*** evasion and the amount of damage dealt. Lua functions that implement the effect of skills and items
*** make the most use of these functions. Thus, these functions are specifically designed for that use
*** and do not utilize C++ features that Lua can not take advantage of, such as references or default
*** values for function arguments.
***
*** There are also many functions that share the same name but have a different function signature. These
*** functions perform the same task but some take extra arguments to make the calculation more flexible. For
*** example, many functions have a version that allows adjustment of the variation by accepting a standard deviation
*** argument.
***
*** \note These calculations only work for valid non-party type targets. If it is desired to use these methods
*** on a party target, a set of targets for each actor in the target party must be extracted and those actor
*** targets used individually for these various methods.
**/
//@{
/** \brief Determines if a target has evaded an attack or other action
*** \param target A pointer to the target to calculate evasion for
*** \return True if the target evasion was successful
**/
bool CalculateStandardEvasion(BattleTarget* target);

/** \brief Determines if a target has evaded an attack or other action
*** \param target A pointer to the target to calculate evasion for
*** \param add_evasion A modifier value to be added to the standard evasion rating
*** \return True if the target evasion was successful
***
*** The additional_evasion may be positive or negative. If the total evasion value falls below 0.0f
*** the function will return false and if that value exceeds 100.0f it will return true. Otherwise the total
*** evade value will serve as a standard probability distribution to determine whether the evasion was
*** successful or not.
**/
bool CalculateStandardEvasion(BattleTarget* target, float add_evasion);

/** \brief Determines if a target has evaded an attack or other action
*** \param target A pointer to the target to calculate evasion for
*** \param mul_evasion A modifier value to be multiplied to the standard evasion rating
*** \return True if the target evasion was successful
***
*** This function operates the same as the CalculateStandardEvasion(...) functions with the exception that
*** its float argument is used as a multiple in the evasion calculation instead of an addition. So for instance
*** if the user wants the evasion chance to increase by 20%, 1.2f would be passed in for the multiple_evasion
*** argument. Negative values are also accepted, and -1.2f would decrease the evasion chance by 20%.
**/
bool CalculateStandardEvasionMultiplier(BattleTarget* target, float mul_evasion);

/** \brief Determines the amount of damage caused with a standard attack
*** \param attacker A pointer to the attacker who is causing the damage
*** \param target A pointer to the target that will be receiving the damage
*** \return The amount of damage dealt, which will always be a non-zero value unless there was an error
***
*** This function uses both the physical and metaphysical attack/defense ratings to calculate the total
*** damage caused. This function uses a gaussian random distribution with a standard deviation of ten percent
*** to perform variation in the damage caused. Therefore this function may return different values each time
*** it is called with the same arguments. If the amount of damage calculates out to zero, a small random
*** non-zero value will be returned instead.
**/
uint32 CalculateStandardDamage(BattleActor* attacker, BattleTarget* target);

/** \brief Determines the amount of damage caused with a standard attack
*** \param attacker A pointer to the attacker who is causing the damage
*** \param target A pointer to the target that will be receiving the damage
*** \param add_phys An additional amount to add to the physical damage dealt
*** \param add_meta An additional amount to add to the metaphyiscal damage dealt
*** \return The amount of damage dealt, which will always be a non-zero value unless there was an error
***
*** The physical and metaphysical add modifiers may be positive or negative. Large negative values can often
*** skew the calculations and cause the calculated damage dealt to drop to zero so be cautious about the
*** add modifier values given to this function
**/
uint32 CalculateStandardDamage(BattleActor* attacker, BattleTarget* target, int32 add_phys, int32 add_meta);

/** \brief Determines the amount of damage caused with a standard attack
*** \param attacker A pointer to the attacker who is causing the damage
*** \param target A pointer to the target that will be receiving the damage
*** \param std_dev The standard deviation to use in the gaussian distribution, where "7.5f" would represent 7.5% standard deviation
*** \return The amount of damage dealt, which will always be a non-zero value unless there was an error
***
*** The std_dev value is always relative to the amount of absolute damage calculated prior to the gaussian randomization.
*** This means you can not use this function to declare an absolute standard deviation, where "20.0f" would correspond to
*** a stadnard deviation of 20HP from the absolute damage.
**/
uint32 CalculateStandardDamage(BattleActor* attacker, BattleTarget* target, float std_dev);

/** \brief Determines the amount of damage caused with a standard attack
*** \param attacker A pointer to the attacker who is causing the damage
*** \param target A pointer to the target that will be receiving the damage
*** \param add_phys An additional amount to add to the physical damage dealt
*** \param add_meta An additional amount to add to the metaphyiscal damage dealt
*** \param std_dev The standard deviation to use in the gaussian distribution, where "7.5f" would represent 7.5% standard deviation
*** \return The amount of damage dealt, which will always be a non-zero value unless there was an error
**/
uint32 CalculateStandardDamage(BattleActor* attacker, BattleTarget* target, int32 add_phys, int32 add_meta, float std_dev);

/** \brief Determines the amount of damage caused with a standard attack
*** \param attacker A pointer to the attacker who is causing the damage
*** \param target A pointer to the target that will be receiving the damage
*** \param mul_phys An additional amount to be multiplied to the physical damage dealt
*** \param mul_meta An additional amount to be multiplied to the metaphyiscal damage dealt
*** \return The amount of damage dealt, which will always be a non-zero value unless there was an error
***
*** This function operates the same as the CalculateStandardDamage(...) functions with the exception that
*** its float arguments are used as multiplers in the damage calculation instead of an addition multipler.
*** So for instance if the user wants the physical damage to increase by 20% and the metaphysical damage to
*** decrease by 15%, the values of mul_phys and mul_meta would be 1.2f and -1.15f respectively. A zero value
*** for the multiplier will have no effect on the damage calculation.
**/
uint32 CalculateStandardDamageMultiplier(BattleActor* attacker, BattleTarget* target, float mul_phys, float mul_meta);

/** \brief Determines the amount of damage caused with a standard attack
*** \param attacker A pointer to the attacker who is causing the damage
*** \param target A pointer to the target that will be receiving the damage
*** \param mul_phys An additional amount to be multiplied to the physical damage dealt
*** \param mul_meta An additional amount to be multiplied to the metaphyiscal damage dealt
*** \param std_dev The standard deviation to use in the gaussian distribution, where "7.5f" would represent 7.5% standard deviation
*** \return The amount of damage dealt, which will always be a non-zero value unless there was an error
***
*** This function signature allows the additional option of setting the standard deviation in the gaussian random value calculation.
**/
uint32 CalculateStandardDamageMultiplier(BattleActor* attacker, BattleTarget* target, float mul_phys, float mul_meta, float std_dev);
//@}


/** ****************************************************************************
*** \brief Builds upon the SystemTimer to provide more flexibility and features
***
*** Battle mode timers are a bit more advanced over the standard system engine
*** timer to meet the needs of some timers in battle mode. The additional features
*** available to battle timers over system timers include the following.
***
*** - The ability to set the expiration time of the current loop to any value
*** - Apply a floating-point multiplier to speed up or slow down the timer
***
*** \note Not all timers in battle mode will require the features of this class.
*** Evaluate the needs of a timer in the battle code and determine whether or not
*** it should use this class or if the standard SystemTimer will meet its needs.
*** ***************************************************************************/
class BattleTimer : hoa_system::SystemTimer {
	friend class SystemEngine; // For allowing SystemEngine to call the _AutoUpdate() method

public:
	BattleTimer();

	/** \brief Creates and places the timer in the SYSTEM_TIMER_INITIAL state
	*** \param duration The duration (in milliseconds) that the timer should count for
	*** \param loops The number of times that the timer should loop for. Default value is set to no looping.
	**/
	BattleTimer(uint32 duration, int32 loops = 0);

	~BattleTimer()
		{}

	//! \brief Overrides the SystemTimer::Update() method
	void Update();

	/** \brief Overrides the SystemTimer::Update(uint32) method
	*** \param time The amount of time to increment the timer by
	**/
	void Update(uint32 time);

	/** \brief Sets the time expired member and updates the timer object appropriately
	*** \param time The value to set for the expiration time
	***
	*** This method will do nothing if the state of the object is SYSTEM_TIMER_INVALID or SYSTEM_TIMER_FINISHED.
	*** The new expiration time applies only to the current loop. So for example, if the timer is on loop #2
	*** of 5 and the loop duration is 1000ms, setting the time to 1500ms will result in the timer
	*** state changing to loop #3 and set the expiration time back to zero. Reaching the last loop
	*** will set the timer to the FINISHED state. Using a zero value for the time while on the first loop will
	*** change the timer to the INITIAL state.
	***
	*** If you're only looking to increment the expiration time and wish for that increment to take
	*** effect for more than just the current loop, use the Update(uint32) method. Although be aware that
	*** Update() will take into account any timer multipliers that are active while this method ignores
	*** the timer multiplier.
	**/
	void SetTimeExpired(uint32 time);

	/** \brief Activates or deactivates the timer multiplier
	*** \param activate True will activate and set the multiplier while false will deactivate.
	*** \param multiplier The multiplier value to apply towards the timer, which should be positive.
	***
	*** If the activate argument is false, the multiplier value will be ignored. The multiplier
	*** value is multiplied directly to the raw update time to obtain the actual update time.
	*** So for example if the raw update time is 25 and the multiplier value is 0.8f, the actual
	*** update time for the class will be 20.
	**/
	void ActivateTimeMultiplier(bool activate, float multiplier);

	//! \name Class member accessor methods
	//@{
	bool IsTimeMultiplierActive() const
		{ return _time_multiplier_active; }

	float GetTimeMultiplier() const
		{ return _time_multiplier; }
	//@}

protected:
	//! \brief When true the timer multiplier is applied to all timer updates
	bool _time_multiplier_active;

	//! \brief A zero or positive value that is multiplied to the update time
	float _time_multiplier;

	//! \brief Overrides the SystemTimer::_AutoUpdate() method
	virtual void _AutoUpdate();

private:
	/** \brief Computes and returns the update time after the multiplier has been applied
	*** \param time The raw update time to use in the calculation
	*** \return The modified update time
	***
	*** This method does not do any error checking such as whether the multiplier is a valid number
	*** (non-negative) or whether or not the multiplier is active. The code that calls this function
	*** is responsible for that condition checking.
	**/
	uint32 _ApplyMultiplier(uint32 time);
}; // class BattleTimer : hoa_system::SystemTimer


/** ****************************************************************************
*** \brief Container class for representing the target of a battle action
***
*** Valid target types include attack points, actors, and parties. This class is
*** somewhat of a wrapper and allows a single instance of BattleTarget to represent
*** any of these types. It also contains a handful of methods useful in determining
*** the validity of a selected target and selecting another target of the same type.
***
*** Many of these functions are dependent on receiving a pointer to a BattleActor
*** object that is using or intends to use the BattleTarget object. This is necessary
*** because the different types of the GLOBAL_TARGET enum are relative and the class
*** selects different targets relative to the user. For example, selecting the next
*** actor when the target type is GLOBAL_TARGET_ALLY requires knowing whether the user
*** is a character or an enemy.
*** ***************************************************************************/
class BattleTarget {
public:
	BattleTarget();

	~BattleTarget()
		{}

	//! \brief Resets all class members, invalidating the target
	void InvalidateTarget();

	/** \brief Used to set the initial target
	*** \param user A pointer to the actor which will use the target
	*** \param type The type of target to set
	***
	*** If the function fails to find an initial target, the target type will be set to
	*** GLOBAL_TARGET_INVALID. The initial attack point is always the first available point on the
	*** actor (index 0). The initial actor will always be the first valid actor in their respective
	*** party (index 0).
	**/
	void SetInitialTarget(BattleActor* user, hoa_global::GLOBAL_TARGET type);

	/** \brief Sets the target to a specific attack point on an actor
	*** \param type The type of target to set, must be one of the point type targets
	*** \param attack_point An integer index into the actor's attack points
	*** \param actor The actor to set for the target (default value == NULL)
	*** A NULL actor simply means that the class should continue pointing to the current actor.
	*** This is useful for cycling through the available attack points on an actor. Note that if the
	*** actor argument is NULL, the _actor member should not be NULL when the function is called.
	*** If both are NULL, calling this method will perform no changes.
	**/
	void SetPointTarget(hoa_global::GLOBAL_TARGET type, uint32 attack_point, BattleActor* actor = NULL);

	/** \brief Sets the target to an actor
	*** \param type The type of target to set, must be one of the actor type targets
	*** \param actor A pointer to the actor to set for the target
	**/
	void SetActorTarget(hoa_global::GLOBAL_TARGET type, BattleActor* actor);

	/** \brief Sets the target to a party
	*** \param type The type of target to set, must be one of the party type targets
	*** \param actor A pointer to the party to set for the target
	**/
	void SetPartyTarget(hoa_global::GLOBAL_TARGET type, std::deque<BattleActor*>* party);

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
	*** \param user A pointer to the actor which is using this target
	*** \param direction Tells the method to look either forward or backward (true/false) for the next target
	*** \param valid_criteria When true the method will only select targets determined to be valid by IsValid()
	*** \return True if the attack point or actor target was changed, false if no change took place
	***
	*** This method should only be invoked when the _type member is equal to one of the "POINT" types.
	*** Under normal circumstances this method will simply reference the next attack point available on
	*** the targeted actor. However, if the actor is deceased and the valid_criteria member is set to true,
	*** this will cause the method to look for the next available actor and call the SelectNextActor() method.
	***
	*** If the action/skill/item has special criteria for determining what type of a target is valid, the valid_criteria
	*** member should be set to false. This will ignore whether or not the target actor is deceased and allow external
	*** code to determine the validity of the new attack point target itself.
	**/
	bool SelectNextPoint(BattleActor* user, bool direction = true, bool valid_criteria = true);

	/** \brief Changes the target actor to reference the next available actor
	*** \param user A pointer tot he actor which is using this target
	*** \param direction Tells the method to look either forward or backward (true/false) for the next target
	*** \param valid_criteria When true the method will only select actors determined to be valid by IsValid()
	*** \return True if the _actor member was changed, false if it was not
	***
	*** This method should only be called when the target type is not one of the party types.
	**/
	bool SelectNextActor(BattleActor* user, bool direction = true, bool valid_criteria = true);

	//! \name Class member accessor methods
	//@{
	hoa_global::GLOBAL_TARGET GetType() const
		{ return _type; }

	uint32 GetPoint() const
		{ return _point; }

	BattleActor* GetActor() const
		{ return _actor; }

	std::deque<BattleActor*>* GetParty() const
		{ return _party; }
	//@}

private:
	//! \brief The type of target this object represents (attack point, actor, or party)
	hoa_global::GLOBAL_TARGET _type;

	//! \brief The attack point to target, as an index to the proper point on the _actor
	uint32 _point;

	//! \brief The actor to target
	BattleActor* _actor;

	//! \brief The party to target
	std::deque<BattleActor*>* _party;
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

	/** \brief Increments the count of an item by one
	*** \note This method should not be called under normal battle circumstances
	**/
	void IncrementCount();

	/** \brief Decrements the count of the item by one
	*** Will also decrement the available count if the two counts are equal
	**/
	void DecrementCount();

	/** \brief A wrapper function that retrieves the actual count of the item
	*** \note Calling this function is equivalent to calling GetItem().GetCount()
	**/
	uint32 GetCount() const
		{ return _item.GetCount(); }

	/** \brief A wrapper function that retrieves the target type of the item
	*** \note Calling this function is equivalent to calling GetItem().GetTargetType()
	**/
	hoa_global::GLOBAL_TARGET GetTargetType() const
		{ return _item.GetTargetType(); }

private:
	//! \brief The item that this class represents
	hoa_global::GlobalItem _item;

	//! \brief The number of instances of this item that are available to be selected to be used
	uint32 _available_count;
}; // class BattleItem

} // namespace private_battle

} // namespace hoa_battle

#endif // __BATTLE_UTILS_HEADER__
