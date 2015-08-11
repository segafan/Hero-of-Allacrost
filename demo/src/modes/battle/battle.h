////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
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

#include "defs.h"
#include "utils.h"

#include "audio.h"
#include "mode_manager.h"
#include "system.h"
#include "video.h"

#include "global.h"

#include "battle_utils.h"

namespace hoa_battle {

extern bool BATTLE_DEBUG;

//! \brief An internal namespace to be used only within the battle code. Don't use this namespace anywhere else!
namespace private_battle {

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
	friend class private_battle::FinishWindow;

public:
	BattleMode();

	~BattleMode();

	//! \brief Returns a pointer to the currently active instance of battle mode
	static BattleMode* CurrentInstance()
		{ return _current_instance; }

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

	/** \brief Sets the background image for the battle
	*** \param filename The filename of the new background image to load
	**/
	void SetBackground(const std::string& filename);

	/** \brief Adds a piece of music to the battle soundtrack
	*** \param filename The full filename of the music to play
	*** Note that the first piece of music added is the one that will be played upon entering battle. All subsequent pieces
	*** of music added must be explicitly triggered to play by certain scripted conditions in battle. If no music is added
	*** for a battle, a default battle theme will be played.
	**/
	void AddMusic(const std::string& filename);

	//! \brief Pauses all timers used in any battle mode classes
	void FreezeTimers();

	//! \brief Unpauses all timers used in any battle mode classes
	void UnFreezeTimers();

	/** \brief Changes the state of the battle and performs any initializations and updates needed
	*** \param new_state The new state to change the battle to
	**/
	void ChangeState(private_battle::BATTLE_STATE new_state);

	//! \brief Returns true if the battle has finished and entered either the victory or defeat state
	bool IsBattleFinished() const
		 { return ((_state == private_battle::BATTLE_STATE_VICTORY) || (_state == private_battle::BATTLE_STATE_DEFEAT)); }

	//! \brief Exits the battle performing any final changes as needed
	void Exit();

	/** \brief Plays the specified piece of music that has already been added to the battle soundtrack
	*** \param filename The filename of the music to play
	***
	*** If the requested music was not found no change to music playback will take place.
	**/
	void PlayMusic(const std::string& filename);

	//! \brief Returns the number of character actors in the battle, both living and dead
	uint32 GetNumberOfCharacters() const
		{ return _character_actors.size(); }

	//! \brief Returns the number of enemy actors in the battle, both living and dead
	uint32 GetNumberOfEnemies() const
		{ return _enemy_actors.size(); }

	/** \brief Retrieves a specific status icon with the proper type and intensity
	*** \param type The type of status effect the user is trying to retrieve the icon for
	*** \param intensity The intensity level of the icon to retrieve
	*** \return The icon representation of the element type and intensity, or NULL if no appropriate image was found
	**/
	hoa_video::StillImage* GetStatusIcon(hoa_global::GLOBAL_STATUS type, hoa_global::GLOBAL_INTENSITY intensity);

	/** \name Battle notification methods
	*** These methods are called by other battle classes to indicate events such as when an actor
	*** changes its state. Often BattleMode will respond by updating the state of one or more of its
	*** members and calling other battle classes to notify them of the event.
	**/
	//@{
	/** \brief Performs any necessary changes in response to a character entering the command state
	*** \param character A pointer to the character who is now in the ACTOR_STATE_COMMAND state
	**/
	void NotifyCharacterCommand(private_battle::BattleCharacter* character);

	/** \brief Called whenever the player has finished selecting a command for a character
	*** \param character A pointer to the character that just had its command completed.
	**/
	void NotifyCharacterCommandComplete(private_battle::BattleCharacter* character);

	/** \brief Called to notify BattleMode when an actor is ready to execute an action
	*** \param actor A pointer to the actor who has entered the state ACTOR_STATE_READY
	**/
	void NotifyActorReady(private_battle::BattleActor* actor);

	/** \brief Performs any necessary changes in response to an actor's death
	*** \param actor A pointer to the actor who is now deceased
	**/
	void NotifyActorDeath(private_battle::BattleActor* actor);
	//@}

	//! \name Class member accessor methods
	//@{
	std::deque<private_battle::BattleCharacter*>& GetCharacterActors()
		{ return _character_actors; }

	std::deque<private_battle::BattleEnemy*>& GetEnemyActors()
		{ return _enemy_actors; }

	std::deque<private_battle::BattleActor*>& GetCharacterParty()
		{ return _character_party; }

	std::deque<private_battle::BattleActor*>& GetEnemyParty()
		{ return _enemy_party; }
	//@}

	/** \brief Adds a new event to the battle
	*** \param event A pointer to the event to add
	**/
// 	void AddEvent(BattleEvent* event)
// 		{ _events.push_back(event); }

	/** \brief Adds a line of dialogue to display during the battle
	*** \param speaker_name The name of the speaker of the line of dialogue
	*** \param text The dialogue text
	**/
// 	void AddDialogue(std::string speaker_name, std::string text);

	//! \brief Shows the next line of dialogue
// 	void ShowDialogue();

	//! \name Class Member Accessor Methods
	//@{
	hoa_video::StillImage& GetCharacterBarCovers()
		{ return _character_bar_covers; }
	//@}

private:
	//! \brief A static pointer to the currently active instance of battle mode
	static BattleMode* _current_instance;

	//! \brief Retains the current state of the battle
	private_battle::BATTLE_STATE _state;

	//! \name Battle Actor Containers
	//@{
	/** \brief Characters that are presently fighting in the battle
	*** No more than four characters may be fighting at any given time, thus this structure will never
	*** contain more than four BattleCharacter objects. This structure does not include any characters
	*** that are in the party, but not actively fighting in the battle. This structure includes
	*** characters that have zero hit points.
	**/
	std::deque<private_battle::BattleCharacter*> _character_actors;

	/** \brief Identical to the _character_actors container except that the elements are BattleActor pointers
	*** \note This container is necessary for the GlobalTarget class, which needs a common data type so that
	*** it may point to either the character or enemy party.
	**/
	std::deque<private_battle::BattleActor*> _character_party;

	/** \brief Enemies that are presently fighting in the battle
	*** There is a theoretical limit on how many enemies may fight in one battle, but that is dependent upon
	*** the sprite size of all active enemies and this limit will be detected by the BattleMode class.
	*** This structure includes enemies that have zero hit points.
	**/
	std::deque<private_battle::BattleEnemy*> _enemy_actors;

	/** \brief Identical to the _enemy_actors container except that the elements are BattleActor pointers
	*** \note This container is necessary for the GlobalTarget class, which needs a common data type so that
	*** it may point to either the character or enemy party.
	**/
	std::deque<private_battle::BattleActor*> _enemy_party;

	/** \brief A FIFO queue for characters who are awaiting their turn to have a command selected by the player
	*** When a character completes the wait time for their idle state, they enter the command state and are
	*** automatically placed in this queue. This structure is necessary as while the player is selecting
	*** commands for one character, a different character's idle time may expire. If the battle is played in
	*** "wait" mode where the battle becomes effectively paused when the player is selecting a command, this
	*** structure is unnecessary, but it is still used regardless of whether the play mode is "wait" or "active".
	*** Note that only characters enter this queue and not enemies, as enemies have their commands automatically
	*** and immediately selected by the game's AI.
	**/
	std::list<private_battle::BattleCharacter*> _command_queue;

	/** \brief A FIFO queue of all actors that are ready to perform an action
	*** When an actor has completed the wait time for their warm-up state, they enter the ready state and are
	*** placed in this queue. The actor at the front of the queue is in the acting state, meaning that they are
	*** executing their action. All other actors in the queue are waiting for the acting actor to finish and
	*** be removed from the queue before they can take their turn.
	**/
	std::list<private_battle::BattleActor*> _ready_queue;
	//@}

	//! \name Battle supervisor classes
	//@{
	//! \brief Manages state and visuals when the player is selecting a command for a character
	private_battle::CommandSupervisor* _command_supervisor;

	/** \brief Window which presents information and options after a battle is concluded
	*** Located at the center of the screen, this window only appears after one party in the battle has defeated
	*** the other.
	**/
	private_battle::FinishWindow* _finish_window;
	//@}

	//! \name Character Swap Data
	//@{
	/** \brief The number of character swaps that the player may currently perform
	*** The maximum number of swaps ever allowed is four, thus the value of this class member will always have the range [0, 4].
	*** This member is also used to determine how many swap cards to draw on the battle screen.
	**/
	uint8 _current_number_swaps;
	//@}

	//! \brief The default battle music to play during battles
	std::string _default_music;

	//! \brief The currently playing music
	std::string _current_music;

	//! \brief the winning music
	std::string _winning_music;

	//! \brief the losing music
	std::string _losing_music;

	//! \brief Contains BattleEvents applicable to current battle
	std::vector<BattleEvent*> _events;

	//! \name Battle Media Data
	//@{
	//! \brief The full-screen, static background image to be used for the battle
	hoa_video::StillImage _battle_background;

	//! \brief Container for images (both still and animated) that are to be drawn in the background
	std::vector<hoa_video::ImageDescriptor*> _background_images;

	//! \brief Contains the entire set of status effect icons
	std::vector<hoa_video::StillImage> _status_icons;

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
	hoa_video::StillImage _character_bar_covers;

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

	/** \brief Container for all music to be played during the battle
	*** The first element in this vector is the primary battle track. For most battles, only a primary track
	*** is required. However, some battles may require additional tracks to toggle between.
	**/
	std::map<std::string, hoa_audio::MusicDescriptor> _battle_music;
	//@}

	////////////////////////////// PRIVATE METHODS ///////////////////////////////

	//! \brief Initializes all data necessary for the battle to begin
	void _Initialize();

	/** \brief Sets the origin location of all character and enemy actors
	*** The location of the actors in both parties is dependent upon the number and physical size of the actor
	*** (the size of its sprite image). This function implements the algorithm that determines those locations.
	**/
	void _DetermineActorLocations();

	//! \brief Returns the number of enemies that are still alive in the battle
	uint32 _NumberEnemiesAlive() const;

	/** \brief Returns the number of characters that are still alive in the battle
	*** \note This function only counts the characters on the screen, not characters in the party reserves
	**/
	uint32 _NumberCharactersAlive() const;

	//! \name Draw assistant functions
	//@{
	/** \brief Draws all background images and animations
	*** The images and effects drawn by this function will never be drawn over anything else in the battle
	*** (battle sprites, menus, etc.).
	**/
	void _DrawBackgroundGraphics();

	/** \brief Draws all character and enemy sprites as well as any sprite visuals
	*** In addition to the sprites themselves, this function draws special effects and indicators for the sprites.
	*** For example, the actor selector image and any visible action effects like magic.
	**/
	void _DrawSprites();

	//! \brief Draws all GUI graphics on the screen
	void _DrawGUI();

	/** \brief Draws the bottom menu visuals and information
	*** The bottom menu contains a wide array of information, including swap cards, character portraits, character names,
	*** and both character and enemy status. This menu is perpetually drawn on the battle screen.
	**/
	void _DrawBottomMenu();

	//! \brief Draws the stamina bar and the icons of the actors of both parties
	void _DrawStaminaBar();

	//! \brief Draws indicator text and graphics for each actor on the field
	void _DrawIndicators();
	//@}
}; // class BattleMode : public hoa_mode_manager::GameMode

} // namespace hoa_battle

#endif // __BATTLE_HEADER__
