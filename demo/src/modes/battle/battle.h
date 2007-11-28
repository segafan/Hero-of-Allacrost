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

#include "battle_actions.h"
#include "battle_actors.h"
#include "battle_windows.h"

namespace hoa_battle {

extern bool BATTLE_DEBUG;

//! \brief An internal namespace to be used only within the battle code. Don't use this namespace anywhere else!
namespace private_battle {

//! \brief A pointer to the BattleMode object that is managing the current battle that is taking place
extern BattleMode* current_battle;

//! \name Screen dimension constants
//@{
//! \brief Battle scenes are visualized via an invisible grid of 64x64 tiles
const uint32 TILE_SIZE     = 64;
//! \brief The length of the screen in number of tiles (16 x 64 = 1024)
const uint32 SCREEN_LENGTH = 16;
//! \brief The height of the screen in number of tiles (12 x 64 = 768)
const uint32 SCREEN_HEIGHT = 12; 
//@}

/** \brief Possible monster locations in monster creation order. 
*** \note This is a temporary hack until an enemy placement
*** algorithm is properly working.
**/
const float MONSTER_LOCATIONS[][2] = {
	{ 515.0f, 768.0f - 360.0f }, // 768.0f - because of reverse Y-coordinate system 
	{ 494.0f, 768.0f - 450.0f },
	{ 510.0f, 768.0f - 550.0f },
	{ 580.0f, 768.0f - 630.0f },
	{ 675.0f, 768.0f - 390.0f },
	{ 655.0f, 768.0f - 494.0f },
	{ 793.0f, 768.0f - 505.0f },
	{ 730.0f, 768.0f - 600.0f }
};

const float PLAYER_LOCATIONS[][2] = {
	{ 256.0f, 320.0f },
	{ 150.0f, 270.0f }
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

//! Returned as an index when looking for a character or enemy and they do not exist
const uint32 INVALID_BATTLE_ACTOR_INDEX = 999;

//! When a battle first starts, this is the wait time for the slowest actor
const uint32 MAX_INIT_WAIT_TIME = 8000;

//! Warm up time for using items (try to keep short, should be constant regardless
// of item used
const uint32 ITEM_WARM_UP_TIME = 1000;

//! True if we are using active battle mode (i.e. timers do not pause when player is making choices
//FIX ME should be handled via the options menu
const bool ACTIVE_BATTLE_MODE = false;

/** \brief Finds the average experience level of all members in the party
*** \return A floating point value representing the average level|
***
*** This calculation includes both characters in the active party and those in
*** the reservers.
**/
float ComputeAveragePartyLevel();

class BattleException : public hoa_utils::Exception {
public: 
	//! \brief The constructor
	BattleException(const std::string & message, const std::string & file="", const int line=-1, const std::string & function="") throw();

    //! \brief The destructor
	~BattleException() throw();
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
***
*** \todo Add a RestartBattle() function that re-initializes all battle data and
*** begins the battle over from the start.
***
*** \bug If timers are paused when then the game enters pause mod or quit mode, when
*** it returns to battle mode the paused timers will incorrectly be resumed. Need
*** to save/restore additional state information about timers on a pause event.
*** ***************************************************************************/
class BattleMode : public hoa_mode_manager::GameMode {
	friend class private_battle::BattleActor;
	friend class private_battle::BattleCharacter;
	friend class private_battle::BattleEnemy;
	friend class private_battle::BattleAction;
	friend class private_battle::ActionWindow;
	friend class private_battle::FinishWindow;

public:
	BattleMode();

	~BattleMode();

	//! \name Inherited methods for the GameMode class
	//@{
	//! \brief Resets appropriate class members. Called whenever BattleMode is made the active game mode.
	void Reset();
	
	//! \brief This method calls different update functions depending on the battle state.
	void Update();

	//! \brief This method calls different draw functions depending on the battle state.
	void Draw();
	//@}

	/** \brief Adds a new active enemy to the battle field
	*** \param new_enemy A copy of the GlobalEnemy object to add to the battle
	*** This method uses the GlobalEnemy copy constructor to create a copy of the enemy. The GlobalEnemy
	*** passed as an argument should be in its default loaded state (that is, it should have an experience
	*** level equal to zero).
	**/
	void AddEnemy(hoa_global::GlobalEnemy* new_enemy);

	/** \brief Adds a new active enemy to the battle field
	*** \param new_enemy_id The id number of the new enemy to add to the battle
	*** This method works precisely the same was as the method which takes a GlobalEnemy argument,
	*** only this version will construct the global enemy just using its id (meaning that it has
	*** to open up the Lua file which defines the enemy). If the GlobalEnemy has already been
	*** defined somewhere else, it is better to pass it in to the alternative definition of this
	*** function.
	**/
	void AddEnemy(uint32 new_enemy_id)
		{ AddEnemy(new hoa_global::GlobalEnemy(new_enemy_id)); }

	/** \brief Adds a piece of music to the battle soundtrack
	*** \param music_filename The full filename of the music to play
	*** Note that the first piece of music added is the one that will be played upon entering battle. All subsequent pieces
	*** of music added must be explicitly triggered to play by certain scripted conditions in battle. If no music is added
	*** for a battle, a default battle theme will be played.
	**/
	void AddMusic(const std::string& music_filename);

	//! \brief Plays the specified piece of music
	void PlayMusic(const std::string &music_filename);

	// TODO: Some of the public methods below should probably not be public...


	//! \brief Sets whether an action is being performed or not, and what that action is
	void SetPerformingAction(bool is_performing, private_battle::BattleAction* se);

	//! \brief Added a scripted event to the queue
	void AddBattleActionToQueue(private_battle::BattleAction* event)
		{ _action_queue.push_back(event); }

	//! \brief Remove all scripted events for an actor
	void RemoveActionsForActor(hoa_battle::private_battle::BattleActor* actor);

	//! \brief Handles any changes to the mode's state in response to an actor's death
	// NOTE: Could expand into a generic notify function that can handle multiple types of
	// messages
	void NotifyOfActorDeath(hoa_battle::private_battle::BattleActor* actor);

	//! \brief Returns all player actors
	std::deque<private_battle::BattleCharacter*> GetCharacters() const
		{ return _character_actors; }

	//! \brief Freezes all timers in battle mode. Used when game is paused or using wait battle mode.
	void FreezeTimers();

	//! \brief Unfreezes all timers in battle mode.  Used when game is unpaused or using wait battle mode.
	void UnFreezeTimers();

	//! \brief Is the battle over?
	bool IsBattleOver() const
		{ return _battle_over; }

	//! \brief Was the battle victorious?
	bool IsVictorious() const
		{ return _victorious_battle; }

	uint32 GetNumberOfCharacters() const
		{ return _character_actors.size(); }

	uint32 GetNumberOfEnemies() const
		{ return _enemy_actors.size(); }

	uint32 GetIndexOfFirstAliveEnemy() const;

	uint32 GetIndexOfLastAliveEnemy() const;

	uint32 GetIndexOfFirstIdleCharacter() const;

	//! \brief Useful for item and skill targeting
	uint32 GetIndexOfNextAliveEnemy(bool move_upward) const;

	//! \brief Returns the player actor at the deque location 'index'
	private_battle::BattleCharacter * GetPlayerCharacterAt(uint32 index) const
		{ return _character_actors.at(index); }

	//! \brief Returns the enemy actor at the deque location 'index'
	private_battle::BattleEnemy * GetEnemyActorAt(uint32 index) const
		{ return _enemy_actors.at(index); }

	//! \brief Returns the index of a certain character
	uint32 GetIndexOfCharacter(private_battle::BattleCharacter* const actor) const;

	//! \brief Swap a character from _player_actors to _player_actors_in_battle
	// This may become more complicated if it is done in a wierd graphical manner
	void SwapCharacters(private_battle::BattleCharacter * ActorToRemove, private_battle::BattleCharacter * ActorToAdd);

	private_battle::BattleAction* GetActiveAction()
		{ return _active_action; }

private:
	//! \brief When set to true, all preparations have been made and the battle is ready to begin
	bool _initialized;

	//! \brief Set to true when either the character or enemy party has been defeated
	bool _battle_over;

	//! \brief Set to true if it was player who won the battle.
	bool _victorious_battle;

	/** \brief Container for all music to be played during the battle
	*** The first element in this vector is the primary battle track. For most battles, only a primary track
	*** is required. However, some battles may require additional tracks to toggle between.
	**/
	std::map<std::string, hoa_audio::MusicDescriptor> _battle_music;

	//! \name Battle Background Data
	//@{
	//! \brief The full-screen, static background image to be used for the battle
	hoa_video::StillImage _battle_background;

	//! \brief Container for images (both still and animated) that are to be drawn in the background
	std::vector<hoa_video::ImageDescriptor*> _background_images;
	//@}

	//! \name Battle Actor Containers
	//@{
	/** \brief Characters that are presently fighting in the battle
	*** No more than four characters may be fighting at any given time, thus this structure will never
	*** contain more than four CharacterActor objects. This structure does <b>not</b> include any characters
	*** that are in the party, but not actively fighting in the battle. This structure includes characters
	*** that have zero hit points.
	**/
	std::deque<private_battle::BattleCharacter*> _character_actors;

	/** \brief Enemies that are presently fighting in the battle
	*** There is a theoretical limit on how many enemies may fight in one battle, but that is dependent upon
	*** the sprite size of all active enemies and this limit will be detected by the BattleMode class.
	*** This structure includes enemies that have zero hit points.
	**/
	std::deque<private_battle::BattleEnemy*> _enemy_actors;

	/** \brief Characters that are in the party reserves
	*** This structure contains characters which are in the current party, but are not fighting in the battle.
	*** They may be swapped into the battle by the player.
	**/
	std::deque<private_battle::BattleCharacter*> _reserve_characters;
	//@}

	//! \name Selection Data
	//@{
	/** \brief Character index of the currently selected actor
	*** \note This needs to be made defunct. Occurences of it in battle.cpp should
	*** be replaced with the index of the _selected_character member
	**/
	int32 _selected_character_index;

	//! \brief Argument selector
	int32 _selected_target_index;

	//! \brief The current character that is selected by the player
	private_battle::BattleCharacter* _selected_character;

	/** \brief The current target for the player's selection
	*** This may point to either a character or enemy actor.
	**/
	private_battle::BattleActor* _selected_target;

	/** \brief The index of the attack point on the selected target that is selected
	*** If the target type of the skill or item is not an attack point target, then
	*** the value of this member is meaningless.
	**/
	uint32 _selected_attack_point;
	//@}

	//! \name Battle GUI Windows
	//@{
	/** \brief Window which displays various information and options related to selecting actions for characters
	*** Located at the bottom right hand corner of the screen, this window is only visible when the player is
	*** actively selecting an action for a character.
	**/
	private_battle::ActionWindow* _action_window;

	/** \brief Window which presents information and options after a battle is concluded
	*** Located at the center of the screen, this window only appears after one party in the battle has defeated
	*** the other.
	**/
	private_battle::FinishWindow* _finish_window;
	//@}

	//! \name Battle GUI Images
	//@{
	//! \brief The static image that is drawn for the bottom menus
	hoa_video::StillImage _bottom_menu_image;

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

	//! \brief Used to provide a background highlight a selected character's stats
	hoa_video::StillImage _character_selection;

	//! \brief An image which contains the covers for the HP and SP bars
	hoa_video::StillImage _character_bars;

	/** \brief The universal stamina bar that is used to represent the state of battle actors
	*** All battle actors have a portrait that moves along this meter to signify their
	*** turn in the rotation.  The meter and corresponding portraits must be drawn after the
	*** character sprites.
	**/
	hoa_video::StillImage _stamina_meter;

	//! \brief The image used to highlight stamina icons for selected actors
	hoa_video::StillImage _stamina_icon_selected;

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
	//@}

	//! \brief Used for scaling actor wait times
	uint32 _min_agility;

	//! \name Action Processing
	//@{
	//! \brief The actor action currently being performed. Set to NULL if no action is being performed.
	private_battle::BattleAction* _active_action;

	//! \brief A FIFO queue of actor actions to perform
	std::list<private_battle::BattleAction*> _action_queue;

	//! \brief Contains actions which are processed over a period of time
	std::list<private_battle::BattleAction*> _action_residual;
	//@}

	//! \brief An Index to the (x,y) location of the next created monster (MONSTER_LOCATIONS array)
	int32 _next_monster_location_index;

	//! \brief The default battle music to play during battles
	std::string _default_music;

	//! \brief The currently playing music
	std::string _current_music;

	////////////////////////////// PRIVATE METHODS ///////////////////////////////

	/** \brief Loads images and other hard-coded data for the battle
	*** \note This function is temporary until there is necessary support for battle mode from the map code
	*** and from Lua scripting
	**/
	void _TEMP_LoadTestData();

	//! \brief Initializes all data necessary for the battle to begin
	void _Initialize();

	//! \brief Shutdown the battle mode
	void _ShutDown();

	//! \brief Returns true if an actor is performing an action
	bool _IsExecutingAction() const
		{ return (_active_action != NULL); }

	//! \brief Returns the number of enemies that are still alive in the battle
	uint32 _NumberEnemiesAlive() const;

	/** \brief Returns the number of characters that are still alive in the battle
	*** \note This function only counts the characters on the screen, not characters in the party reserves
	**/
	uint32 _NumberCharactersAlive() const;

	/** \brief Selects the initial target for an action to take effect on
	*** This is only used for characters to select an initial target, but not for enemies.
	**/
	void _SetInitialTarget();

	/** \brief Sets the _target_selected member to the next available target
	*** \param forward_direction Determines whether the next target should be located ahead or behind of the current one
	*** This member will also change the _attack_point selected member
	**/
	void _SelectNextTarget(bool forward_direction);

	/** \brief Sets the _attack_point_selected member to the next available attack point
	*** \param forward_direction Determines whether the next target should be located ahead or behind of the current one
	**/
	void _SelectNextAttackPoint(bool forward_direction);

	/** \name Draw helper functions
	*** \brief Functions which draw various components of the battle screen
	**/
	//@{
	/** \brief Draws all background images and animations
	*** The images and effects drawn by this function will never be drawn over anything else in the battle
	*** (battle sprites, menus, etc.). 
	**/
	void _DrawBackgroundVisuals();

	/** \brief Draws the bottom menu visuals and information
	*** The bottom menu contains a wide array of information, including swap cards, character portraits, character names,
	*** and both character and enemy status. This menu is perpetually drawn on the battle screen.
	**/
	void _DrawBottomMenu();

	/** \brief Draws all character and enemy sprites as well as any sprite visuals
	*** In addition to the sprites themselves, this function draws special effects and indicators for the sprites.
	*** For example, the actor selector image and any visible action effects like magic.
	**/
	void _DrawSprites();

	/** \brief Draws the universal stamina bar and the icons of the present actors
	**/
	void _DrawStaminaBar();
	//@}
}; // class BattleMode : public hoa_mode_manager::GameMode

} // namespace hoa_battle

#endif // __BATTLE_HEADER__
