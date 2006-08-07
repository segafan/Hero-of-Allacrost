////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle.h
*** \author  Corey Hoffstein, visage@allacrost.org
*** \brief   Header file for battle mode interface.
***
*** This code handles the game event processing and frame drawing when the user
*** is fighting a battle.
*** ***************************************************************************/

#ifndef __BATTLE_HEADER__
#define __BATTLE_HEADER__

#include <string>
#include <vector>
#include <deque>

#include "utils.h"
#include "defs.h"
#include "mode_manager.h"
#include "global.h"
#include "video.h"
#include "audio.h"

namespace hoa_battle {

extern bool BATTLE_DEBUG;

class BattleStatTypes {
public:
	int32 volt;  // strong against water, weak against earth
	int32 earth; // strong against volt, weak against fire
	int32 water; // strong against fire, weak against volt
	int32 fire;  // strong against earth, weak against water

	int32 piercing;
	int32 slashing;
	int32 bludgeoning;
};

enum StatusSeverity { LESSER = 0, NORMAL, GREATER, ULTIMATE };


namespace private_battle {

const uint32 TILE_SIZE = 64;     // The virtual "tile map" that we discussed in the forums has square 64 pixel tiles
const uint32 SCREEN_LENGTH = 16; // Number of tiles long the screen is
const uint32 SCREEN_HEIGHT = 12; // The number of tiles high the screen is

const uint32 DRAW_OFFSET_LEFT = 224;  // from the left of the screen = 0
const uint32 DRAW_OFFSET_TOP = 640;   // from bottom of the screen = 0 (128 from the top of the screen)
const uint32 DRAW_OFFSET_BOTTOM = 64; // from the bottom of the screen = 0

const uint32 NUM_WIDTH_TILES = 12;
const uint32 NUM_HEIGHT_TILES = 9;

/** \brief Manages the user interface for battles
***
**/
class BattleUI {


public:
	BattleUI(BattleMode* const ABattleMode);
 ~BattleUI();

	//! \brief Update player info
	void Update(uint32 AUpdateTime);
	//! \brief Draw the GUI images
	void Draw();
	void DrawTopElements();

	//! \brief Get the actor we are currently on
	PlayerActor *const GetSelectedActor() const
		{ return _currently_selected_player_actor; }

	//! \brief We clicked on an actor
	void SetPlayerActorSelected(PlayerActor* const AWhichActor);

	//! \brief No actor is selected...we are now selecting an actor
	void DeselectPlayerActor()
		{ _currently_selected_player_actor = NULL; }

	//! \brief Get other people selected
	std::deque<BattleActor*> GetSelectedArgumentActors() const
		{ return _currently_selected_argument_actors; }

	//! \brief The actor we just selected is now an argument
	void SetActorAsArgument(BattleActor* const AActor)
		{ _currently_selected_argument_actors.push_back(AActor); }

	//! \brief Sets the number of arguments we should be allowing
	void SetNumberNecessarySelections(uint32 ANumSelections)
		{ _necessary_selections = ANumSelections; }

	//! \brief The player lost.  Show them the menu
	void ShowPlayerDefeatMenu();

	//! \brief The player won. Show them their loot
	void ShowPlayerVictoryMenu();

private:
	enum CURSOR_STATE {
		CURSOR_ON_PLAYER_CHARACTERS = 0,
		CURSOR_ON_ENEMY_CHARACTERS = 1,
		CURSOR_ON_MENU = 2,
		CURSOR_ON_SUB_MENU = 3,
		CURSOR_SELECT_TARGET = 4,
		CURSOR_ON_SELECT_MAP = 5
	};

	//! The battlemode we belong to
	BattleMode *_battle_mode;
	//! The current actor we have clicked on
	PlayerActor *_currently_selected_player_actor;
	//! Character index of the currently selected actor
	int32 _actor_index;
	//! Argument selector
	int32 _argument_actor_index;
	//! The actors we have selected as arguments
	std::deque<BattleActor*>_currently_selected_argument_actors;
	//! A stack of menu selections we have gone through
	std::deque<uint32> _currently_selected_menu_item;
	//! The number of selections that must be made for an action
	uint32 _necessary_selections;
	//! The menu item we are hovering over
	uint32 _current_hover_selection;
	//! The current MAP we are pointing to
	uint32 _current_map_selection;
	//! The number of items in this menu
	uint32 _number_menu_items;
	//! The state of our cursor
	CURSOR_STATE _cursor_state;
	//! The general option box
	hoa_video::OptionBox _general_menu;
	//! The cursor location in the _general_menu.  For pure hackery reasons only
	uint32 _general_menu_cursor_location;
	//! The sub menu. Recreated every time it is chosen
	hoa_video::OptionBox* _sub_menu;
	hoa_video::MenuWindow* _sub_menu_window;
	//! The "loser" - menu
	hoa_video::OptionBox _battle_lose_menu;
	//! The selected cursor
	hoa_video::StillImage _player_selector_image;
	//! The MAPS cursor
	hoa_video::AnimatedImage _MAPS_indicator;
}; // class BattleUI

/**
***
**/
class ScriptEvent {
public:
	ScriptEvent(BattleActor* AHost, std::deque<BattleActor*> AArguments, std::string AScriptName);
	~ScriptEvent();
	void RunScript();

	BattleActor *GetHost()
		{ return _host; }

private:
	std::string _script_name;
	BattleActor *_host;
	std::deque < BattleActor * >_arguments;
};
} // namespace private_battle

/**
***
**/
class BattleMode : public hoa_mode_manager::GameMode {
	friend class hoa_data::GameData;
// 	friend class private_battle::CharacterActor;
// 	friend class private_battle::EnemyActor;
public:
	static int32 MAX_PLAYER_CHARACTERS_IN_BATTLE;
	static int32 MAX_ENEMY_CHARACTERS_IN_BATTLE;

	BattleMode();
	~BattleMode();

	//! \brief Resets appropriate class members. Called whenever BootMode is made the active game mode.
	void Reset();
	//! \brief Wrapper function that calls different update functions depending on the battle state.
	void Update();
	//! \brief Wrapper function that calls different draw functions depending on the battle state.
	void Draw();

	//! \brief Sets T/F whether an action is being performed
	void SetPerformingScript(bool AIsPerforming);

	//! \brief Added a scripted event to the queue
	void AddScriptEventToQueue(private_battle::ScriptEvent AEventToAdd)
		{ _script_queue.push_back(AEventToAdd); }
	//! \brief Remove all scripted events for an actor
	void RemoveScriptedEventsForActor(private_battle::BattleActor *AActorToRemove);

	//! \brief Returns all player actors
	std::deque<private_battle::PlayerActor*> ReturnCharacters() const;

	//! \brief The number of players alive
	uint32 NumberOfPlayerCharactersAlive();
	//! Is the battle over?
	const bool IsBattleOver() const
		{ return _battle_over; }
	//! Was the battle victorious?
	const bool IsVictorious() const
		{ return _victorious_battle; }

	//! \brief Victory stuff
	void PlayerVictory();
	//! \brief Defeat stuff
	void PlayerDefeat();

	uint32 GetNumberOfPlayerCharacters()
		{ return _players_characters_in_battle.size(); }
	uint32 GetNumberOfEnemyActors()
		{ return _enemy_actors.size(); }
	int32 GetIndexOfFirstAliveEnemy();
	int32 GetIndexOfFirstIdleCharacter();

	//! \brief Return the player character at the deque location 'index'
	private_battle::PlayerActor *GetPlayerCharacterAt(uint32 AIndex) const
		{ return _players_characters_in_battle[AIndex]; }
	private_battle::EnemyActor *BattleMode::GetEnemyActorAt(uint32 AIndex) const
		{ return _enemy_actors[AIndex]; }

	//! \brief Returns the index of a player character
	int32 IndexLocationOfPlayerCharacter(private_battle::PlayerActor *const AActor);

	//! \brief Swap a character from _player_actors to _player_actors_in_battle
	// This may become more complicated if it is done in a wierd graphical manner
	void SwapCharacters(private_battle::PlayerActor *AActorToRemove, private_battle::PlayerActor *AActorToAdd);

private:
	std::vector<hoa_video::StillImage> _battle_images;
	std::vector<hoa_audio::MusicDescriptor> _battle_music;
	std::vector<hoa_audio::SoundDescriptor> _battle_sound;

	//! Current list of actors
	std::deque<private_battle::PlayerActor*> _player_actors;

	//! The global enemies used in this battle
	//   Used for restoring the battle mode
	std::deque<hoa_global::GlobalEnemy> _global_enemies;

	//actors actually in battle
	std::deque<private_battle::EnemyActor*> _enemy_actors;
	std::deque<private_battle::PlayerActor*> _players_characters_in_battle;

	//! a queue of scripted events to perform
	std::list<private_battle::ScriptEvent> _script_queue;

	//! the user interface belonging to this battle mode
	private_battle::BattleUI _user_interface;

	//! Is an action being performed?
	bool _performing_script;
	//! Is the battle over
	bool _battle_over;
	//! if _battle_over == true the battle was either won or lost
	bool _victorious_battle;

	//! Swapping information
	uint32 _num_swap_cards;
	uint32 _max_swap_cards;
	uint32 _last_time_swap_awarded;

	//! Drawing methods
	void _DrawBackground();
	void _DrawCharacters();

	//! Shutdown the battle mode
	void _ShutDown();


	//! Are we performing an action
	const bool _IsPerformingScript() const
		{ return _performing_script; }

	void _TEMP_LoadTestData();

	void _BuildPlayerCharacters();
	void _BuildEnemyActors();
}; // class BattleMode : public hoa_mode_manager::GameMode

} // namespace hoa_battle

#endif // __BATTLE_HEADER__
