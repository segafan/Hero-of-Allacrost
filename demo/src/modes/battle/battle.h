////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle.h
*** \author  Viljami Korhonen, mindflayer@allacrost.org
*** \author  Corey Hoffstein, visage@allacrost.org
*** \author  Andy Gardner, chopperdave@allacrost.org
*** \brief   Header file for battle mode interface.
***
*** This code handles event processing, game state updates, and video frame
*** drawing when the user is fighting a battle.
*** ***************************************************************************/

#ifndef __BATTLE_HEADER__
#define __BATTLE_HEADER__

#include <string>
#include <vector>
#include <deque>
#include <map>

#include "utils.h"
#include "defs.h"
#include "video.h"
#include "audio.h"
#include "global.h"
#include "global_actors.h"
#include "mode_manager.h"
#include "system.h"

namespace hoa_battle {

extern bool BATTLE_DEBUG;

//! An internal namespace to be used only within the battle code. Don't use this namespace anywhere else!
namespace private_battle {

//! A pointer to the BattleMode object that is managing the current battle that is taking place
extern BattleMode* current_battle;

//! \name Screen dimension constants
//@{
//! Battle scenes are visualized via an invisible grid of 64x64 tiles
const uint32 TILE_SIZE     = 64;
//! The length of the screen in number of tiles (16 x 64 = 1024)
const uint32 SCREEN_LENGTH = 16;
//! The height of the screen in number of tiles (12 x 64 = 768)
const uint32 SCREEN_HEIGHT = 12; 
//@}

/** \brief Possible monster locations in monster creation order. 
//  \note This is probably only a temporary fix until we get the monster sorting to work correctly
**/
const float MONSTER_LOCATIONS[][2] =
{
	{ 515.0f, 768.0f - 360.0f}, // 768 - because of reverse Y-coordinate system 
	{ 494.0f, 768.0f - 450.0f},
	{ 510.0f, 768.0f - 550.0f},
	{ 580.0f, 768.0f - 630.0f},
	{ 675.0f, 768.0f - 390.0f},
	{ 655.0f, 768.0f - 494.0f},
	{ 793.0f, 768.0f - 505.0f},
	{ 730.0f, 768.0f - 600.0f}
};


/** \name Action Type Constants
*** \brief Identifications for the types of actions a player's characters may perform
**/
//@{
const uint32 ACTION_TYPE_ATTACK    = 0;
const uint32 ACTION_TYPE_DEFEND    = 1;
const uint32 ACTION_TYPE_SUPPORT   = 2;
const uint32 ACTION_TYPE_ITEM      = 3;
//@}

/** \brief Enumerated values for the possible states that the user's input context may be in
*** These constants are used throughout the battle code for various purposes, including determining what 
*** user input commands should do, which components of the battle scene should be updated, and what 
*** objects in the battle scene should be drawn.
**/
enum CURSOR_STATE {
	CURSOR_IDLE = 0,
	CURSOR_SELECT_ACTION_TYPE = 1,
	CURSOR_SELECT_ACTION_LIST = 2,
	CURSOR_SELECT_TARGET = 3,
	CURSOR_SELECT_ATTACK_POINT = 4
};

//! Returned as an index when looking for a character or enemy and they do not exist
const uint32 INVALID_BATTLE_ACTOR_INDEX = 999;

//! When a battle first starts, this is the wait time for the slowest actor
const uint32 MAX_INIT_WAIT_TIME = 8000;

//! Warm up time for using items (try to keep short, should be constant regardless
// of item used
const uint32 ITEM_WARM_UP_TIME = 1000;

/** \brief Finds the average experience level of all members in the party
*** \return A floating point value representing the average level|
***
*** This calculation includes both characters in the active party and those in
*** the reservers.
**/
float ComputeAveragePartyLevel();

/** ****************************************************************************
*** \brief Representation of a single, scripted action to be executed in battle
***
***
*** ***************************************************************************/
class ScriptEvent {
public:
	ScriptEvent(BattleActor* source, std::deque<BattleActor*> targets, const std::string & script_name, uint32 warm_up_time);
	ScriptEvent(BattleActor* source, BattleActor* target, hoa_global::GlobalItem* item, uint32 warm_up_time = ITEM_WARM_UP_TIME);
	ScriptEvent(BattleActor* source, BattleActor* target, hoa_global::GlobalSkill* skill);

	~ScriptEvent();

	//! Executes the script
	void RunScript();

	//! \name Class member access functions
	//@{
	BattleActor * GetSource()
		{ return _source; }

	inline hoa_system::Timer GetWarmUpTime() const
		{ return _warm_up_time; }

	inline BattleActor* GetTarget()
		{ return _target; }

	inline hoa_global::GlobalItem* GetItem()
		{ return _item; }

	inline hoa_global::GlobalSkill* GetSkill()
		{ return _skill; }

	//@}

	//! \name Class member access functions
	//@{
	//hoa_global::GlobalActor * GetSource() { return _source; }
	//@}

	//! \brief Updates the script
	void Update();

	// \brief Returns the amount of time left in warm up
	// \return warm up time left
	//inline hoa_system::Timer GetWarmUpTime() const { return _warm_up_time; }

	//! \brief Gets the BattleActor hosting this script
	//inline IBattleActor* GetActor() { return _actor_source; }

private:
	//! The name of the executing script
	std::string _script_name;

	//! The actor whom is initiating this script
	BattleActor* _source;

	//! Pointer to the skill attached to this script (for skill events only)
	hoa_global::GlobalSkill* _skill;

	//! Pointer to the item attached to this script (for item events only)
	hoa_global::GlobalItem* _item;

	//hoa_global::GlobalActor * _source;

	//! The targets of the script
	BattleActor* _target;

	//! The targets of the script FIX ME
	std::deque<BattleActor *> _targets;

	//! The amount of time to wait to execute the script
	hoa_system::Timer _warm_up_time;
	//! If the script is ready to run or not
};

} // namespace private_battle

/** ****************************************************************************
*** \brief Manages all objects, events, and scenes that occur in a battle
***
*** To create a battle, first you must create an instance of this class. Next,
*** the battle must be populated with enemies by using the AddEnemy() methods
*** prescribed below. You must then call the InitializeEnemies() method so that
*** the added enemies are ready for the battle to come. This should all be done
*** prior the Reset() method being called. If you fail to add any enemies,
*** an error will occur and the battle will self-terminate itself.
*** ***************************************************************************/
class BattleMode : public hoa_mode_manager::GameMode {
	friend class private_battle::BattleActor;
	friend class private_battle::BattleCharacterActor;
	friend class private_battle::BattleEnemyActor;
	friend class private_battle::ScriptEvent;

public:
	BattleMode();

	~BattleMode();

	/**
	*** \brief Overloaded gamestate methods for the battle mode
	**/
	//@{
	//! Resets appropriate class members. Called whenever BattleMode is made the active game mode.
	void Reset();
	
	//! This method calls different update functions depending on the battle state.
	void Update();

	//! This method calls different draw functions depending on the battle state.
	void Draw();
	//@}

	/** \brief Adds a new active enemy to the battle field
	*** \param new_enemy A copy of the GlobalEnemy object to add to the battle
	*** This method uses the GlobalEnemy copy constructor to create a copy of the enemy. The GlobalEnemy
	*** passed as an argument should be in its default loaded state (that is, it should have an experience
	*** level equal to zero).
	**/
	void AddEnemy(hoa_global::GlobalEnemy new_enemy);

	/** \brief Adds a new active enemy to the battle field
	*** \param new_enemy_id The id number of the new enemy to add to the battle
	*** This method works precisely the same was as the method which takes a GlobalEnemy argument,
	*** only this version will construct the global enemy just using its id (meaning that it has
	*** to open up the Lua file which defines the enemy). If the GlobalEnemy has already been
	*** defined somewhere else, it is better to pass it in to the alternative definition of this
	*** function.
	**/
	void AddEnemy(uint32 new_enemy_id)
		{ AddEnemy(hoa_global::GlobalEnemy(new_enemy_id)); }

	// NOTE: this is temporary because of a signature matching problem with the seperate AddEnemy definitions above
	// This method is being bound to Lua until the problem is resolved, at which point this method should be removed
	void TEMP_AddEnemy(uint32 new_enemy_id)
		{ AddEnemy(hoa_global::GlobalEnemy(new_enemy_id)); }

	/** \brief Prepares all added enemies for the battle to come
	*** Any enemies added after this call is made will <b>not</b> be present in the battle, at least
	*** until this method is called once more.
	**/
	void InitializeEnemies();

	//! Are we performing an action
	bool _IsPerformingScript() const
		{ return _performing_script; }

	//! Sets whether an action is being performed or not, and what that action is
	void SetPerformingScript(bool is_performing, private_battle::ScriptEvent* se);

	//! Added a scripted event to the queue
	void AddScriptEventToQueue(private_battle::ScriptEvent* event)
		{ _script_queue.push_back(event); }

	//! Remove all scripted events for an actor
	void RemoveScriptedEventsForActor(hoa_battle::private_battle::BattleActor * actor);
	//void RemoveScriptedEventsForActor(hoa_global::GlobalActor * actor);

	//! Returns all player actors
	std::deque<private_battle::BattleCharacterActor*> GetCharacters() const
		{ return _character_actors; }

	//! Returns all targeted actors
	//std::deque<private_battle::IBattleActor*> GetSelectedActors() const
	//	{ return _selected_actor_arguments; }

	//! Is the battle over?
	bool IsBattleOver() const
		{ return _battle_over; }

	//! Was the battle victorious?
	bool IsVictorious() const
		{ return _victorious_battle; }

	//! Handle player victory
	void PlayerVictory();
	
	//! Handle player defeat
	void PlayerDefeat();

	uint32 GetNumberOfCharacters() const
		{ return _character_actors.size(); }

	uint32 GetNumberOfEnemies() const
		{ return _enemy_actors.size(); }

	uint32 GetIndexOfFirstAliveEnemy() const;

	uint32 GetIndexOfLastAliveEnemy() const;

	uint32 GetIndexOfFirstIdleCharacter() const;

	//! Useful for item and skill targeting
	uint32 GetIndexOfNextAliveEnemy(bool move_upward) const;

	//! Returns the player actor at the deque location 'index'
	private_battle::BattleCharacterActor * GetPlayerCharacterAt(uint32 index) const
		{ return _character_actors.at(index); }

	//! Returns the enemy actor at the deque location 'index'
	private_battle::BattleEnemyActor * GetEnemyActorAt(uint32 index) const
		{ return _enemy_actors.at(index); }

	//! Returns the index of a player character
	uint32 GetIndexOfCharacter(private_battle::BattleCharacterActor * const Actor) const;

	//! \brief Swap a character from _player_actors to _player_actors_in_battle
	// This may become more complicated if it is done in a wierd graphical manner
	void SwapCharacters(private_battle::BattleCharacterActor * ActorToRemove, private_battle::BattleCharacterActor * ActorToAdd);

	// \brief Gets the active ScriptEvent
	// \param se the ScriptEvent to designate as active
	inline private_battle::ScriptEvent* GetActiveScript() { return _active_se; }

private:
	//! When set to true, all preparations have been made and the battle is ready to begin
	bool _initialized;

	//! Set to true whenever an actor (player or enemy) is performing an action
	bool _performing_script;

	//! The script currently being performed
	private_battle::ScriptEvent* _active_se;

	//! Set to true when either side of the battle is dead
	bool _battle_over;

	//! Set to true if it was player who won the battle.
	bool _victorious_battle;

	/** \brief Container for all music to be played during the battle
	*** The first element in this vector is the primary battle track. For most battles, only a primary track
	*** is required. However, some battles may require additional tracks to toggle between.
	**/
	std::vector<hoa_audio::MusicDescriptor> _battle_music;

	//! Container for various sounds that need to be played during the battle
	std::vector<hoa_audio::SoundDescriptor> _battle_sounds;

	//! \name Battle Background Data
	//@{
	//! The full-screen, static background image to be used for the battle
	hoa_video::StillImage _battle_background;

	//! Container for images (both still and animated) that are to be drawn in the background
	std::vector<hoa_video::ImageDescriptor*> _background_images;
	//@}

	//! \name Battle Actors Data
	//@{
	/** \brief A map containing pointers to all actors both on and off the battle field
	*** The actor objects are indexed by their unique ID numbers. This structure is used primarily
	*** for two things. First, it serves as a convenient index to look up and retrieve an actor
	*** object when only an ID number is known. Second, it is used to prevent memory leaks by
	*** ensuring that all BattleActor objects are deleted when the battle ends.
	**/
	std::map<uint8, private_battle::BattleActor*> _battle_actors;

	/** \brief Contains the original set of enemies and their status
	*** This data is retained in the case that the player loses the battle and chooses to re-try
	*** the battle from the beginning. This data will be used to restore BattleMode::_enemy_actors.
	**/
	std::deque<hoa_global::GlobalEnemy> _original_enemies;

	/** \brief Contains the original set of characters and their status
	*** This data is retained in the case that the player loses the battle and chooses to re-try
	*** the battle from the beginning. This data will be used to restore BattleMode::_character_actors.
	**/
	std::deque<hoa_global::GlobalCharacter> _original_characters;

	/** \brief Characters that are presently fighting in the battle
	*** No more than four characters may be fighting at any given time, thus this structure will never
	*** contain more than four CharacterActor objects. This structure does <b>not</b> include any characters
	*** that are in the party, but not actively fighting in the battle. This structure includes characters
	*** that have zero hit points.
	**/
	std::deque<private_battle::BattleCharacterActor*> _character_actors;

	/** \brief Enemies that are presently fighting in the battle
	*** There is a theoretical limit on how many enemies may fight in one battle, but that is dependent upon
	*** the sprite size of all active enemies and this limit will be detected by the BattleMode class.
	*** This structure includes enemies that have zero hit points.
	**/
	std::deque<private_battle::BattleEnemyActor*> _enemy_actors;

	/** \brief Characters that are in the party reserves
	*** This structure contains characters which are in the current party, but are not fighting in the battle.
	*** They may be swapped into the battle by the player.
	**/
	std::deque<private_battle::BattleCharacterActor*> _reserve_characters;
	//@}

	//! \name Player Selection Data
	//@{
	//! The current CharacterActor that is selected by the player
	private_battle::BattleCharacterActor * _selected_character;
	//! The current/last EnemyActor that is/was selected by the player
	//private_battle::BattleEnemyActor * _selected_enemy;
	//! The current target for the player's selection
	private_battle::BattleActor *_selected_target;

	//! The number of selections that must be made for an action
	// FIX ME: Obsolete
	//uint32 _necessary_selections;
	//! The current attack point we are pointing to
	uint32 _attack_point_selected;
	//! The number of items in this menu
	uint32 _number_menu_items;
	//! The state of the menu cursor
	private_battle::CURSOR_STATE _cursor_state;
	//! The cursor location in the _action_type_menu.  For pure hackery reasons only
	uint32 _action_type_menu_cursor_location;

	//! Character index of the currently selected actor
	//FIX ME Don't think this is needed anymore, have a handle
	//to the char via _selected_character
	int32 _actor_index;
	//! Argument selector
	uint32 _argument_actor_index;
	//! The actors we have selected as arguments OBSOLETE
	//std::deque<private_battle::BattleActor*> _selected_actor_arguments;
	//@}

	//! \name Battle GUI Objects and Images
	//@{
	//! The static image that is drawn for the bottom menu
	hoa_video::StillImage _bottom_menu_image;
	/** \brief The menu window used for action types and action lists
	*** This menu window is always located on the left side of the screen.
	**/
	hoa_video::MenuWindow* _action_menu_window;
	/** \brief The menu window used to list your currently selected action
	*** It's placed at the top of the left menu, reserving the bottom for
	*** the action list
	**/
	hoa_video::MenuWindow _action_type_window;
	/** \brief The option menu that lists the types of actions that a character may take in battle
	*** Typically this list includes "attack", "defend", "support", and "item". More types may appear
	*** under special circumstances and conditions.
	**/
	hoa_video::OptionBox _action_type_menu;
	/** \brief The option menu that lists the actions that a character may take in battle
	*** 
	**/
	hoa_video::OptionBox* _action_list_menu;
	/** \brief The options that a player may choose to take if they lose the battle
	*** The list of options include: re-try battle, quit to main menu, or quit game.
	**/
	hoa_video::OptionBox _battle_lose_menu;
	/** \brief An image that indicates that a particular actor has been selected
	*** This image best suites character sprites and enemy sprites of similar size. It does not work
	*** well with larger or smaller sprites.
	**/
	hoa_video::StillImage _actor_selection_image;
	/** \brief An image that points out the location of specific attack points on an actor
	*** This image may be used for both character and enemy actors. It is used to indicate an actively selected
	*** attack point, <b>not</b> just any attack points present.
	**/
	hoa_video::AnimatedImage _attack_point_indicator;
	/** \brief The icons used for representing each of the possible action types in battle
	*** This vector is used solely for the purpose of drawing the chosen action above the
	*** action list in battle.  For example, if they choose attack, then the attack icon along
	*** with the word 'Attack' will appear at the top of the window, followed by the action list
	**/
	std::vector<hoa_video::StillImage> _action_type_icons;
	/** \brief The universal stamina bar that all battle actors proceed along
	*** All battle actors have a portrait that moves along this meter to signify their
	*** turn in the rotation.  The meter and corresponding portraits must be drawn after the
	*** character sprites.
	**/
	hoa_video::StillImage _universal_time_meter;
	//@}

	//! \name Character Swap Card Data
	//@{
	/** \brief The number of character swaps that the player may currently perform
	*** The maximum number of swaps ever allowed is four, thus the value of this class member will always have the range [0, 4].
	*** This member is also used to determine how many swap cards to draw on the battle screen.
	**/
	uint8 _current_number_swaps;
	/** \brief A running counter to determine when a player may be given another swap card
	*** The units of this timer are milliseconds. The timer is initially set to around 5 minutes.
	*** Once the timer reaches below zero, it is reset and BattleMode::_num_swap_cards is incremented by one.
	**/
	int32 _swap_countdown_timer;

	/** \brief Image that indicates when a player may perform character swapping
	*** This image is drawn in the lower left corner of the screen. When no swaps are available to the player,
	*** the image is drawn in gray-scale.
	**/
	hoa_video::StillImage _swap_icon;
	/** \brief Used for visual display of how many swaps a character may perform
	*** This image is drawn in the lower left corner of the screen, just above the swap indicator. This image 
	*** may be drawn on the screen up to four times (in a card-stack fashion), one for each swap that is
	*** available to be used. It is not drawn when the player has no swaps available.
	**/
	hoa_video::StillImage _swap_card;
	//@}

	//! Used for scaling actor wait times
	uint32 _min_agility;

	/* \brief A 1 to 1 mapping of the names we put in our item action list.
	** \note Gives us a quick handle to directly manipulate and/or pass the item to Lua
	*/
	std::vector<hoa_global::GlobalItem*> _item_list;

	/* \brief A 1 to 1 mapping of the names we put in our skill action list.
	** \note Gives us a quick handle to directly manipulate and/or pass the skill to Lua
	*/
	std::vector<hoa_global::GlobalSkill*> _skill_list;

	//! Holds the selected index from the action list
	int32 _selected_option_index;

	//! \name Actor Action Processing
	//@{
	//! A FIFO queue of actor actions to perform
	std::list<private_battle::ScriptEvent*> _script_queue;
	//@}

	//! An Index to the (x,y) location of the next created monster (MONSTER_LOCATIONS array)
	int32 _next_monster_location_index;

	////////////////////////////// PRIVATE METHODS ///////////////////////////////

	/** \brief Loads images and other hard-coded data for the battle
	*** \note This function is temporary until there is necessary support for battle mode from the map code
	*** and from Lua scripting
	**/
	void _TEMP_LoadTestData();

	void _CreateCharacterActors();
	void _CreateEnemyActors();

	//! Initializes all data necessary for the battle to begin
	void _Initialize();

	//! Shutdown the battle mode
	void _ShutDown();

	//! \brief Returns the number of enemies that are still alive in the battle
	const uint32 _NumberEnemiesAlive() const;
	/** \brief Returns the number of characters that are still alive in the battle
	*** \note This function only counts the characters on the screen, not characters in the party reserves
	**/
	const uint32 _NumberOfCharactersAlive() const;

	/** \brief Creates the action list menu depending upon which action type the player has chosen
	***
	**/
	void _ConstructActionListMenu();

	/** \name Update helper functions
	*** \brief Functions which update the state of various battle components
	**/
	//@{
	//! Updates which character the player has chosen to select
	void _UpdateCharacterSelection();

	//! Processes user input when the player's cursor is on the action type menu
	void _UpdateActionTypeMenu();

	//! Processes user input when the player's cursor is on the action list menu
	void _UpdateActionListMenu();

	//! Processes user input when the player's cursor is selecting a target for an action
	void _UpdateTargetSelection();

	//! Processes user input when the player's cursor is selecting an attack point for an action
	void _UpdateAttackPointSelection();
	//@}

	/** \name Draw helper functions
	*** \brief Functions which draw various components of the battle screen
	**/
	//@{
	/** \brief Draws all background images and animations
	*** The images and effects drawn by this function will never be drawn over anything else in the battle
	*** (battle sprites, menus, etc.). 
	**/
	void _DrawBackgroundVisuals();

	/** \brief Draws all character and enemy sprites as well as any sprite visuals
	*** In addition to the sprites themselves, this function draws special effects and indicators for the sprites.
	*** For example, the actor selector image and any visible action effects like magic.
	**/
	void _DrawSprites();

	/** \brief Draws the universal time meter and the portraits attached to it
	*** Portraits are retrieved from the battle actors.
	**/
	void _DrawTimeMeter();

	/** \brief Draws the bottom menu visuals and information
	*** The bottom menu contains a wide array of information, including swap cards, character portraits, character names,
	*** and both character and enemy status. This menu is perpetually drawn on the battle screen.
	**/
	void _DrawBottomMenu();

	/** \brief Draws the left menu for action types and action lists
	*** The action type and action list menus are only drawn when a character is able to take an action in battle. Either
	*** the action type or the action list menu is drawn at any given time (they do not exist simulatenously).
	**/
	void _DrawActionMenu();

	/** \brief Draws the upper menu for dialogues that occur in-battle
	*** This menu is only drawn when a dialogue takes place in battle. In-battle dialogues are a rare-occurence, so this
	*** menu is not drawn very often.
	*** \TODO Dialogues are currently not supported in battles. The feature will be added sometime in the future.
	**/
	void _DrawDialogueMenu();

	/** \brief Draws the small window above the action list
	*** This window is for indicatinig what your current action is, in case you forgot what you chose
	**/
	void _DrawActionTypeWindow();
	//@}
}; // class BattleMode : public hoa_mode_manager::GameMode

} // namespace hoa_battle

#endif // __BATTLE_HEADER__
