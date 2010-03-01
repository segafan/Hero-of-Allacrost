////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle.cpp
*** \author  Viljami Korhonen, mindflayer@allacrost.org
*** \author  Corey Hoffstein, visage@allacrost.org
*** \author  Andy Gardner, chopperdave@allacrost.org
*** \brief   Source file for battle mode interface.
*** ***************************************************************************/

#include "audio.h"
#include "input.h"
#include "mode_manager.h"
#include "script.h"
#include "video.h"

#include "pause.h"

#include "battle.h"
#include "battle_actors.h"
#include "battle_actions.h"
#include "battle_command.h"
#include "battle_events.h"
#include "battle_finish.h"
#include "battle_utils.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_script;
using namespace hoa_pause;

using namespace hoa_battle::private_battle;

namespace hoa_battle {

bool BATTLE_DEBUG = false;
bool wait = true;
// Initialize static class variable
BattleMode* BattleMode::_current_instance = NULL;

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- primary methods
////////////////////////////////////////////////////////////////////////////////

BattleMode::BattleMode() :
	_state(BATTLE_STATE_INVALID),
	_selected_character_index(0),
	_selected_target_index(0),
	_selected_character(NULL),
	_selected_target(NULL),
	_selected_attack_point(0),
	_command_supervisor(new CommandSupervisor()),
	_finish_window(new FinishWindow()),
	_current_number_swaps(0),
	_default_music("mus/Confrontation.ogg"),
	_winning_music("mus/Allacrost_Fanfare.ogg"),
	_losing_music("mus/Allacrost_Intermission.ogg")
// 	_dialogue_window(true),
// 	_after_scripts_finished(false)
{
	IF_PRINT_DEBUG(BATTLE_DEBUG) << "constructor invoked" << endl;

	mode_type = MODE_MANAGER_BATTLE_MODE;

	if (_battle_background.Load("img/backdrops/battle/desert_cave.png") == false)
		PRINT_ERROR << "failed to load default background image" << endl;

	if (_stamina_icon_selected.Load("img/menus/stamina_icon_selected.png") == false)
		PRINT_ERROR << "failed to load stamina icon selected image" << endl;

	_attack_point_indicator.SetDimensions(16.0f, 16.0f);
	if (_attack_point_indicator.LoadFromFrameGrid("img/icons/battle/attack_point_target.png", vector<uint32>(4, 10), 1, 4) == false)
		PRINT_ERROR << "failed to load attack point indicator." << endl;

	if (_stamina_meter.Load("img/menus/stamina_bar.png") == false)
		PRINT_ERROR << "failed to load time meter." << endl;

	if (_actor_selection_image.Load("img/icons/battle/character_selector.png") == false)
		PRINT_ERROR << "unable to load player selector image." << endl;

	if (_character_selection.Load("img/menus/battle_character_selection.png") == false)
		PRINT_ERROR << "failed to load character selection image" << endl;

	if (_character_bar_covers.Load("img/menus/battle_character_bars.png") == false)
		PRINT_ERROR << "failed to load character bars image" << endl;

	if (_bottom_menu_image.Load("img/menus/battle_bottom_menu.png") == false)
		PRINT_ERROR << "failed to load bottom menu image: " << endl;

	if (_swap_icon.Load("img/icons/battle/swap_icon.png") == false)
		PRINT_ERROR << "failed to load swap icon: " << endl;

	if (_swap_card.Load("img/icons/battle/swap_card.png") == false)
		PRINT_ERROR << "failed to load swap card: " << endl;
} // BattleMode::BattleMode()



BattleMode::~BattleMode() {
	for (map<string, MusicDescriptor>::iterator i = _battle_music.begin(); i != _battle_music.end(); i++)
		i->second.FreeAudio();

	// Delete all character and enemy actors
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		delete _character_actors[i];
	}
	_character_actors.clear();

	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		delete _enemy_actors[i];
	}
	_enemy_actors.clear();

	_command_queue.clear();
	_ready_queue.clear();

	delete _command_supervisor;
	delete _finish_window;

	if (_current_instance == this) {
		_current_instance = NULL;
	}
} // BattleMode::~BattleMode()



void BattleMode::Reset() {
	_current_instance = this;

	VideoManager->SetCoordSys(0.0f, 1023.0f, 0.0f, 767.0f);

	// Load the default battle music track if no other music has been added
	if (_battle_music.empty()) {
		_battle_music[_default_music] = MusicDescriptor();
		if (_battle_music[_default_music].LoadAudio(_default_music) == false) {
			IF_PRINT_WARNING(BATTLE_DEBUG) << "failed to load default battle music" << endl;
		}
		_current_music = _default_music;
		_battle_music[_default_music].Play();
	}
	else {
		map<string, MusicDescriptor>::iterator i = _battle_music.begin();
		_current_music = i->first;
		i->second.Play();
	}

	if (_state == BATTLE_STATE_INVALID) {
		_Initialize();
	}

	UnFreezeTimers();
}



void BattleMode::Update() {
	_attack_point_indicator.Update(); // Required update to animated image

	if (InputManager->QuitPress()) {
		FreezeTimers();
		ModeManager->Push(new PauseMode(true));
		return;
	}
	if (InputManager->PausePress()) {
		FreezeTimers();
		ModeManager->Push(new PauseMode(false));
		return;
	}

	// Determine if the battle should proceed to the victory or defeat state
	if (IsBattleFinished() == false) {
		bool characters_defeated = true;
		for (uint32 i = 0; i < _character_actors.size(); i++) {
			if (_character_actors[i]->IsAlive() == true) {
				characters_defeated = false;
				break;
			}
		}

		bool enemies_defeated = true;
		for (uint32 i = 0; i < _enemy_actors.size(); i++) {
			if (_enemy_actors[i]->IsAlive() == true) {
				enemies_defeated = false;
				break;
			}
		}

		if ((characters_defeated == true) && (enemies_defeated == true)) {
			IF_PRINT_WARNING(BATTLE_DEBUG) << "both parties were defeated, changing to defeat state" << endl;
			ChangeState(BATTLE_STATE_DEFEAT);
		}
		else if (characters_defeated == true) {
			ChangeState(BATTLE_STATE_DEFEAT);
		}
		else if (enemies_defeated == true) {
			ChangeState(BATTLE_STATE_VICTORY);
		}
	}

	// Process the actor ready queue
	if (_ready_queue.empty() == false) {
		// Only the acting actor is examined in the ready queue. If this actor is in the READY state,
		// that means it has been waiting for BattleMode to allow it to begin its action and thus
		// we set it to the ACTING state. We do nothing while it is in the ACTING state, allowing the
		// actor to completely finish its action. When the actor enters any other state, it is presumed
		// to be finished with the action or otherwise incapacitated and is removed from the queue.
		BattleActor* acting_actor = _ready_queue.front();
		switch (acting_actor->GetState()) {
			case ACTOR_STATE_READY:
				acting_actor->ChangeState(ACTOR_STATE_ACTING);
				break;
			case ACTOR_STATE_ACTING:
				break;
			default:
				_ready_queue.pop_front();
				break;
		}
	}

	// First, call update functions of any BattleEvents
// 	ScriptObject* during_func;
// 	for (uint32 i = 0; i < _events.size(); i++) {
// 		during_func = _events[i]->GetDuringFunction();
// 		ScriptCallFunction<void>(*during_func, this);
// 	}

	// Update all actors
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->Update();
	}
	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		_enemy_actors[i]->Update();
	}

	// ----- (3): Execute any scripts that are sitting in the queue
// 	if (_action_queue.size())
// 	{
// 		_UpdateScripts();
// 		_CleanupActionQueue();
// 	} // if (_action_queue.size())

	// ----- (4): Try to select an idle character if no character is currently selected
// 	if (_selected_character == NULL)
// 	{
// 		_ActivateNextCharacter();
// 	}



	if (_state == BATTLE_STATE_INITIAL) {
		// TODO
	}
	else if (_state == BATTLE_STATE_NORMAL) {
		// TODO: check if any characters are in the ACTOR_STATE_COMMAND state, if so move to BATTLE_STATE_COMMAND
// 		for (uint32 i = 0; i <
	}
	else if (_state == BATTLE_STATE_COMMAND) {
		_command_supervisor->Update();
	}
	else if (_state == BATTLE_STATE_EVENT) {
// 		_dialogue_window._display_textbox.Update();
// 		_dialogue_window._display_options.Update();
//
// 		if (InputManager->ConfirmPress()) {
// 			_dialogue_window.Reset();
// 			if (_dialogue_text.empty()) {
// 				ChangeState(BATTLE_STATE_NORMAL);
// 			}
// 			else {
// 				ShowDialogue();
// 				return;
// 			}
// 			UnFreezeTimers();
// 		}
// 		else {
// 			FreezeTimers();
// 			return;
// 		}
	}
	else if (_state == BATTLE_STATE_VICTORY) {

	}
	else if (_state == BATTLE_STATE_DEFEAT) {

	}
	else {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid states" << endl;
		Exit();
	}




	// ----- (1): If the battle is over, only execute this small block of update code
	if (_state == BATTLE_STATE_VICTORY || _state == BATTLE_STATE_DEFEAT) {
// 		if (_after_scripts_finished == false) {
// 			ScriptObject* after_func;
// 			for (uint32 i = 0; i < _events.size(); i++) {
// 				after_func = _events[i]->GetAfterFunction();
// 				ScriptCallFunction<void>(*after_func, this);
// 			}
// 			_after_scripts_finished = true;
// 			return;
// 		}

		if (_finish_window->GetState() == FINISH_INVALID) { // Indicates that the battle has just now finished
			//_finish_window = new FinishWindow();
			// make sure the battle has our music
			if (_state == BATTLE_STATE_VICTORY) {
				AddMusic(_winning_music);
				PlayMusic(_winning_music);
			}
			else {
				AddMusic(_losing_music);
				PlayMusic(_losing_music);
			}

			_finish_window->Initialize(_state == BATTLE_STATE_VICTORY);
		}

		// The FinishWindow::Update() function handles all update code when a battle is over.
		// The call to shut down battle mode is also made from within this call.
		_finish_window->Update();

		// Do not update other battle code if the battle has already ended
		return;
	}
} // void BattleMode::Update()



void BattleMode::Draw() {
	// Apply scene lighting if the battle has finished
	if ((_state == BATTLE_STATE_VICTORY || _state == BATTLE_STATE_DEFEAT)) {// && _after_scripts_finished) {
		if (_state == BATTLE_STATE_VICTORY) {
			VideoManager->EnableSceneLighting(Color(0.914f, 0.753f, 0.106f, 1.0f)); // Golden color for victory
		}
		else {
			VideoManager->EnableSceneLighting(Color(1.0f, 0.0f, 0.0f, 1.0f)); // Red color for defeat
		}
	}

	_DrawBackgroundGraphics();
	_DrawSprites();
	_DrawGUI();
}

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- secondary methods
////////////////////////////////////////////////////////////////////////////////

void BattleMode::AddEnemy(GlobalEnemy* new_enemy) {
	// Don't add the enemy if it has an invalid ID or an experience level that is not zero
	if (new_enemy->GetID() == 0) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "attempted to add a new enemy with an invalid id: " << new_enemy->GetID() << endl;
		return;
	}
	if (new_enemy->GetExperienceLevel() != 0) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "attempted to add a new enemy that had already been initialized: " << new_enemy->GetID() << endl;
		return;
	}

	new_enemy->Initialize(GlobalManager->AverageActivePartyExperienceLevel());
	_enemy_actors.push_back(new BattleEnemy(new_enemy));
}



void BattleMode::SetBackground(const string& filename) {
	if (_battle_background.Load(filename) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "failed to load background image: " << filename << endl;
		if (_battle_background.Load("img/backdrops/battle/desert_cave.png") == false)
			IF_PRINT_WARNING(BATTLE_DEBUG) << "failed to load default background image" << endl;
	}
}



void BattleMode::AddMusic(const string& filename) {
	if (_battle_music.find(filename) != _battle_music.end()) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "attempted to add music that had been added previously: " << filename << endl;
		return;
	}

	_battle_music[filename] = MusicDescriptor();
	if (_battle_music[filename].LoadAudio(filename) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "failed to load music file: " << filename << endl;
	}
}



void BattleMode::FreezeTimers() {
	// Pause scripts
// 	list<BattleAction*>::iterator it = _action_queue.begin();

	// Pause character and enemy state timers
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->GetStateTimer().Pause();
	}
	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		_enemy_actors[i]->GetStateTimer().Pause();
	}
}



void BattleMode::UnFreezeTimers() {
	// FIX ME: Do not unpause timers for paralyzed actors

	// Unpause scripts
// 	list<BattleAction*>::iterator it = _action_queue.begin();
// 	while (it != _action_queue.end()) {
// 		(*it)->GetWarmUpTime()->Run();
// 		it++;
// 	}

	// Unpause character and enemy state timers
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->GetStateTimer().Run();
	}
	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		_enemy_actors[i]->GetStateTimer().Run();
	}
}



void BattleMode::ChangeState(BATTLE_STATE new_state) {
	if (_state == new_state) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "battle was already in the state to change to: " << _state << endl;
		return;
	}

	_state = new_state;
	switch (_state) {
		case BATTLE_STATE_INITIAL:
			// TODO: recursive call below is temporary. This state should cause sprites to run in from off screen.
			ChangeState(BATTLE_STATE_NORMAL);
			break;
		case BATTLE_STATE_NORMAL:
			break;
		case BATTLE_STATE_COMMAND:
			_command_supervisor->Initialize(_selected_character);
			break;
		case BATTLE_STATE_EVENT:
// 			_speaker_name = _dialogue_text.front();
// 			_dialogue_text.pop_front();
// 			hoa_utils::ustring text = _dialogue_text.front();
// 			_dialogue_text.pop_front();
//
// 			_dialogue_window.Reset();
// 			_dialogue_window._display_textbox.SetDisplayText(text);
// 			_dialogue_window.Initialize();
			break;
		case BATTLE_STATE_VICTORY:
			break;
		case BATTLE_STATE_DEFEAT:
			break;
		default:
			IF_PRINT_WARNING(BATTLE_DEBUG) << "changed to invalid battle state: " << _state << endl;
			break;
	}
}



void BattleMode::Exit() {
	_battle_music[_current_music].Stop();

	// TEMP: Restore all dead characters back to life by giving them a single health point
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		if (_character_actors[i]->IsAlive() == false) {
// 			_character_actors[i]->SetHitPoints(1);
// 			_character_actors[i]->RetrieveBattleAnimation("idle")->GetCurrentFrame()->DisableGrayScale();
		}
	}

	ModeManager->Pop();
}



void BattleMode::PlayMusic(const string& filename) {
	if (_battle_music.find(filename) == _battle_music.end()) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "requested music file was not loaded: " << filename << endl;
		return;
	}

	// TODO: this isn't playing the requested music in the argument, its playing the first track
	map<string, MusicDescriptor>::iterator i = _battle_music.begin();
	i->second.Play();
	_current_music = i->first;
}



void BattleMode::NotifyCharacterCommand(BattleCharacter* character) {
	for (list<BattleCharacter*>::iterator i = _command_queue.begin(); i != _command_queue.end(); i++) {
		if (character == (*i)) {
			IF_PRINT_WARNING(BATTLE_DEBUG) << "character was already present in the command queue" << endl;
			return;
		}
	}

	_command_queue.push_back(character);
	if (_state != BATTLE_STATE_COMMAND) {
		ChangeState(BATTLE_STATE_COMMAND);
	}
}



void BattleMode::NotifyActorReady(BattleActor* actor) {
	for (list<BattleActor*>::iterator i = _ready_queue.begin(); i != _ready_queue.end(); i++) {
		if (actor == (*i)) {
			IF_PRINT_WARNING(BATTLE_DEBUG) << "actor was already present in the ready queue" << endl;
			return;
		}
	}

	_ready_queue.push_back(actor);
}



void BattleMode::NotifyActorDeath(BattleActor* actor) {
	// Remove actor from command and ready queues if it is found in either one
	if (actor->IsEnemy() == false)
		_command_queue.remove(dynamic_cast<BattleCharacter*>(actor));
	_ready_queue.remove(actor);

	// If the actor who died is the character the player is selecting a command for, reset the action window
	// TODO: should be moved to battle command code notification
	if (actor->IsEnemy() == false && (((BattleActor *)(_selected_character)) == actor)) {
		_selected_character = NULL;
		// TODO: notify command supervisor
	}
}

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- private methods
////////////////////////////////////////////////////////////////////////////////

void BattleMode::_Initialize() {
	// (1): Construct all character battle actors from the active party
	GlobalParty* active_party = GlobalManager->GetActiveParty();
	if (active_party->GetPartySize() == 0) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "no characters in the active party, exiting battle" << endl;
		ModeManager->Pop();
		return;
	}

	for (uint32 i = 0; i < active_party->GetPartySize(); i++) {
		BattleCharacter* new_actor = new BattleCharacter(dynamic_cast<GlobalCharacter*>(active_party->GetActorAtIndex(i)));
		_character_actors.push_back(new_actor);
	}

// 	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
// 		_enemy_actors[i]->GetGlobalEnemy()->Initialize(GlobalManager->AverageActivePartyExperienceLevel());
// 	}

	// (2): Determine the origin position for all characters and enemies
	_DetermineActorLocations();

	// (2): Find the actor with the lowest agility rating
	uint32 min_agility = 0xFFFFFFFF;
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		if (_character_actors[i]->GetAgility() < min_agility)
			min_agility = _character_actors[i]->GetAgility();
	}
	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		if (_enemy_actors[i]->GetAgility() < min_agility)
			min_agility = _enemy_actors[i]->GetAgility();
	}

	// Andy: Once every game loop, the SystemManager's timers are updated
	// However, in between calls, battle mode is constructed. As part
	// of battle mode's construction, each actor is given a wait timer
	// that is triggered on initialization. But the moving of the stamina
	// portrait uses the update time from SystemManager.  Therefore, the
	// amount of time since SystemManager last updated is greater than
	// the amount of time that has expired on the actors' wait timers
	// during the first orund of battle mode.  This gives the portrait an
	// extra boost, so once the wait time expires for an actor, his portrait
	// is past the designated stopping point

	// <--      time       -->
	// A----------X-----------B
	// If the SystemManager has its timers updated at A and B, and battle mode is
	// constructed and initialized at X, you can see the amount of time between
	// X and B (how much time passed on the wait timers in round 1) is significantly
	// smaller than the time between A and B.  Hence the extra boost to the stamina
	// portrait's location

	// FIX ME This will not work in the future (i.e. paralysis)...realized this
	// after writing all the above crap
	// CD: Had to move this to before timers are initalized, otherwise this call will give
	// our timers a little extra nudge with regards to time elapsed, thus making the portraits
	// stop before they reach they yellow/orange line
	// TODO: This should be fixed once battles have a little smoother start (characters run in from
	// off screen to their positions, and stamina icons do not move until they are ready in their
	// battle positions). Once that feature is available, remove this call.
	SystemManager->UpdateTimers();

	// (3): Adjust each actor's idle state time based on their agility proportion to the slowest actor
	// If an actor's agility is twice that of the actor with the lowest agility, then they will have an
	// idle state time that is half of the slowest actor. We also use timer_multiplier to adjust start times.
	// The lower the value of timer_multiplier, the faster the battle goes
	float proportion;
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		proportion = timer_multiplier * (static_cast<float>(min_agility) / static_cast<float>(_character_actors[i]->GetAgility()));
		_character_actors[i]->SetIdleStateTime(static_cast<uint32>(MAX_INIT_WAIT_TIME * proportion));
		_character_actors[i]->ChangeState(ACTOR_STATE_IDLE);
	}
	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		proportion = timer_multiplier * (static_cast<float>(min_agility) / static_cast<float>(_enemy_actors[i]->GetAgility()));
		_enemy_actors[i]->SetIdleStateTime(static_cast<uint32>(MAX_INIT_WAIT_TIME * proportion));
		_enemy_actors[i]->ChangeState(ACTOR_STATE_IDLE);
	}

	_command_supervisor->ConstructCharacterSettings();

	// (4): Invoke any events that should occur when the battle begins
// 	ScriptObject* before_func;
// 	for (uint32 i = 0; i < _events.size(); i++) {
// 		before_func = _events[i]->GetBeforeFunction();
// 		ScriptCallFunction<void>(*before_func, this);
// 	}

	ChangeState(BATTLE_STATE_INITIAL);
} // void BattleMode::_Initialize()



void BattleMode::_DetermineActorLocations() {
	// Temporary static positions for enemies
	const float TEMP_ENEMY_LOCATIONS[][2] = {
		{ 515.0f, 768.0f - 360.0f }, // 768.0f - because of reverse Y-coordinate system
		{ 494.0f, 768.0f - 450.0f },
		{ 510.0f, 768.0f - 550.0f },
		{ 580.0f, 768.0f - 630.0f },
		{ 675.0f, 768.0f - 390.0f },
		{ 655.0f, 768.0f - 494.0f },
		{ 793.0f, 768.0f - 505.0f },
		{ 730.0f, 768.0f - 600.0f }
	};

	float position_x, position_y;

	// Determine the position of the top-right most character in the party
	switch (_character_actors.size()) {
		case 1:
			position_x = 80.0f;
			position_y = 288.0f;
			break;
		case 2:
			position_x = 118.0f;
			position_y = 343.0f;
			break;
		case 3:
			position_x = 122.0f;
			position_y = 393.0f;
			break;
		case 4:
			position_x = 160.0f;
			position_y = 448.0f;
			break;
		default:
			position_x = 160.0f;
			position_y = 448.0f;
			break;
	}

	// Go through all characters, changing each successive position to be down and to the left
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->SetXOrigin(position_x);
		_character_actors[i]->SetYOrigin(position_y);
		_character_actors[i]->SetXLocation(position_x);
		_character_actors[i]->SetYLocation(position_y);
		position_x -= 42.0f;
		position_y -= 105.0f;
	}

	// TEMP: assign static locations to enemies
	uint32 temp_pos = 0;
	for (uint32 i = 0; i < _enemy_actors.size(); i++, temp_pos++) {
		position_x = TEMP_ENEMY_LOCATIONS[temp_pos][0];
		position_y = TEMP_ENEMY_LOCATIONS[temp_pos][1];
		_enemy_actors[i]->SetXOrigin(position_x);
		_enemy_actors[i]->SetYOrigin(position_y);
		_enemy_actors[i]->SetXLocation(position_x);
		_enemy_actors[i]->SetYLocation(position_y);
	}
} // void BattleMode::_DetermineActorLocations()



uint32 BattleMode::_NumberEnemiesAlive() const {
	uint32 enemy_count = 0;
	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		if (_enemy_actors[i]->IsAlive() == true) {
			enemy_count++;
		}
	}
	return enemy_count;
}



uint32 BattleMode::_NumberCharactersAlive() const {
	uint32 character_count = 0;
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		if (_character_actors[i]->IsAlive() == true) {
			character_count++;
		}
	}
	return character_count;
}



// void BattleMode::_SetInitialTarget() {
// 	if (_action_window->_action_target_ally == true) {
// 		_selected_target_index = -1;
// 		_selected_target_index = GetIndexOfNextAliveCharacter(true);
// 		_selected_target = GetPlayerCharacterAt(_selected_target_index);
// 		_selected_attack_point = 0;
// 	}
// 	else {
// 		_selected_target = GetEnemyActorAt(GetIndexOfFirstAliveEnemy());
// 		_selected_target_index = GetIndexOfFirstAliveEnemy();
// 		_selected_attack_point = 0;
// 	}
// }
//
//
//
// void BattleMode::_SelectNextTarget(bool forward_direction) {
// 	if (_selected_target == NULL) {
// 		_SetInitialTarget();
// 		return;
// 	}
//
// 	uint32 previous_target = _selected_target_index;
// 	if (forward_direction) {
// 		if (_action_window->_action_target_ally == true) {
// 			_selected_target_index = (_selected_target_index + 1) % _character_actors.size();
// 			_selected_target = _character_actors[_selected_target_index];
// 		}
// 		else {
// 			_selected_target_index = GetIndexOfNextAliveEnemy(forward_direction);
// 			_selected_target = (static_cast<uint32>(_selected_target_index) == INVALID_BATTLE_ACTOR_INDEX) ?
// 				NULL : _enemy_actors[_selected_target_index];
// 		}
// 	}
//
// 	else {
// 		if (_action_window->_action_target_ally == true) {
// 			_selected_target_index--;
//
// 			if (_selected_target_index < 0) {
// 				_selected_target_index = _character_actors.size() - 1;
// 			}
// 			_selected_target = _character_actors[_selected_target_index];
// 		}
// 		else {
// 			_selected_target_index = GetIndexOfNextAliveEnemy(forward_direction);
// 			_selected_target = (static_cast<uint32>(_selected_target_index) == INVALID_BATTLE_ACTOR_INDEX) ?
// 				NULL : _enemy_actors[_selected_target_index];
// 		}
// 	}
//
// 	if (previous_target != static_cast<uint32>(_selected_target_index))
// 		_selected_attack_point = 0;
// }
//
//
//
// void BattleMode::_SelectNextAttackPoint(bool forward_direction) {
// 	if (_selected_target == NULL) {
// 		_SetInitialTarget();
// 		return;
// 	}
//
// 	if (forward_direction) {
// 		_selected_attack_point++;
// 		if (_selected_attack_point >= _selected_target->GetActor()->GetAttackPoints().size())
// 			_selected_attack_point = 0;
// 	}
//
// 	else {
// 		if (_selected_attack_point == 0)
// 			_selected_attack_point = _selected_target->GetActor()->GetAttackPoints().size() - 1;
// 		else
// 			_selected_attack_point--;
// 	}
// }



// void BattleMode::_UpdateScripts() {
// 	BattleAction* action;
//
// 	for (list<BattleAction*>::iterator i = _action_queue.begin(); i != _action_queue.end(); i++) {
// 		action = (*i);
//
// // 		if (action->ShouldBeRemoved())
// // 			continue;
//
// 		if (action->GetSource()->GetState() == ACTOR_STATE_READY) {
// 			action->Execute();
// 		}
// 	}
// }
//
//
//
// void BattleMode::_CleanupActionQueue() {
// 	list<BattleAction*>::iterator it;
//
// 	for (it = _action_queue.begin(); it != _action_queue.end();) {
// 		if ((*it)->ShouldBeRemoved()) {
// 			it = _action_queue.erase(it);
// 		}
// 		else {
// 			++it;
// 		}
// 	}
// }



void BattleMode::_DrawBackgroundGraphics() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_NO_BLEND, 0);
	VideoManager->Move(0.0f, 0.0f);
	_battle_background.Draw();

	// TODO: Draw other background objects and animations
}



void BattleMode::_DrawSprites() {
	// TODO: Draw sprites in order based on their x and y coordinates on the screen (bottom to top, then left to right)

	// Draw all character sprites
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->DrawSprite();
	}

	// Draw all enemy sprites
	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		_enemy_actors[i]->DrawSprite();
	}
}



void BattleMode::_DrawGUI() {
	_DrawBottomMenu();
	_DrawStaminaBar();
	_DrawIndicators();

	// TODO: draw dialogue window if it is active
// 	if (_state == BATTLE_STATE_EVENT) {
// 		_dialogue_window.Draw(&_speaker_name, NULL);
// 	}
	if (_command_supervisor->GetState() != COMMAND_STATE_INVALID) {
		_command_supervisor->Draw();
	}
	if ((_state == BATTLE_STATE_VICTORY || _state == BATTLE_STATE_DEFEAT)) {// && _after_scripts_finished) {
		_finish_window->Draw();
	}
}



void BattleMode::_DrawBottomMenu() {
	// Draw the static image for the lower menu
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(0.0f, 0.0f);
	_bottom_menu_image.Draw();

	// Draw the swap icon and any swap cards
	VideoManager->Move(6.0f, 16.0f);
	_swap_icon.Draw(Color::gray);
	VideoManager->Move(6.0f, 68.0f);
	for (uint8 i = 0; i < _current_number_swaps; i++) {
		_swap_card.Draw();
		VideoManager->MoveRelative(4.0f, -4.0f);
	}

	// Draw the selected character's portrait, blended according to the character's current HP level
	if (_selected_character)
		_selected_character->DrawPortrait();

	// Draw the status information of all character actors
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->DrawStatus(i);
	}
}



void BattleMode::_DrawStaminaBar() {
	// ----- (1): Draw the stamina bar
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->Move(1010.0f, 128.0f);
	_stamina_meter.Draw();

	// ----- (2): Determine the draw order of all stamina icons
// 	GLOBAL_TARGET target_type = _command_supervisor->GetSelectedTarget().GetType();
// 	bool target_character = _action_window->IsActionTargetAlly();

	// A container to hold all actors that should have their stamina icons drawn
	vector<BattleActor*> live_actors;

	for (uint32 i = 0; i < _character_actors.size(); i++) {
		if (_character_actors[i]->IsAlive())
			live_actors.push_back(_character_actors[i]);
	}
	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		if (_enemy_actors[i]->IsAlive())
			live_actors.push_back(_enemy_actors[i]);
	}

	//std::vector<bool> selected(live_actors.size(), false);
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);

	vector<float> draw_positions(live_actors.size(), 0.0f);
	for (uint32 i = 0; i < live_actors.size(); i++) {
		switch (live_actors[i]->GetState()) {
			case ACTOR_STATE_IDLE:
				draw_positions[i] = STAMINA_LOCATION_BOTTOM + (STAMINA_LOCATION_COMMAND - STAMINA_LOCATION_BOTTOM) *
					live_actors[i]->GetStateTimer().PercentComplete();
				break;
			case ACTOR_STATE_COMMAND:
				draw_positions[i] = STAMINA_LOCATION_COMMAND;
				break;
			case ACTOR_STATE_WARM_UP:
				draw_positions[i] = STAMINA_LOCATION_COMMAND + (STAMINA_LOCATION_TOP - STAMINA_LOCATION_COMMAND) *
					live_actors[i]->GetStateTimer().PercentComplete();
				break;
			case ACTOR_STATE_READY:
				draw_positions[i] = STAMINA_LOCATION_TOP;
				break;
			case ACTOR_STATE_ACTING:
				draw_positions[i] = STAMINA_LOCATION_TOP + 25.0f;
				break;
			case ACTOR_STATE_COOL_DOWN:
				draw_positions[i] = STAMINA_LOCATION_BOTTOM;
				break;
			default:
				// This case is invalid. Instead of printing a debug message that will get echoed every
				// loop, draw the icon at a clearly invalid position well away from the stamina bar
				draw_positions[i] = STAMINA_LOCATION_BOTTOM - 50.0f;
				break;
		}
	}

	// TODO: sort the draw positions container and correspond that to live_actors
// 	sort(draw_positions.begin(), draw_positions.end());

	// ----- (3): Draw all stamina icons in order
	for (uint32 i = 0; i < live_actors.size(); i++) {
		VideoManager->Move(1000.0f, draw_positions[i]);
		live_actors[i]->GetStaminaIcon().Draw();
	}

	// TODO: Draw stamina icons in proper draw order
// 	if (target_type  == GLOBAL_TARGET_PARTY) {
// 		if (target_character == true) { // All characters are selected
// 			for (uint32 i = 0; i < live_actors.size(); i++) {
// 				live_actors[i]->DrawStaminaIcon(!live_actors[i]->IsEnemy());
// 			}
// 		}
// 		else { // All enemies are selected
// 			for (uint32 i = 0; i < live_actors.size(); i++) {
// 				live_actors[i]->DrawStaminaIcon(live_actors[i]->IsEnemy());
// 			}
// 		}
// 	}
// 	else { // Find the actor who is the selected target
// 		for (uint32 i = 0; i < live_actors.size(); i++) {
// 			live_actors[i]->DrawStaminaIcon(live_actors[i] == _selected_target);
// 		}
// 	}

	// ----- (3): Draw all stamina icons for live actors
	/*VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	for (uint32 i = 0; i < live_actors.size(); i++) {
		live_actors[i]->DrawStaminaIcon(selected[i]);
	}*/
} // void BattleMode::_DrawStaminaBar()



void BattleMode::_DrawIndicators() {
	// TODO: Draw sprites indicators in an ordered manner?

	// Draw all character sprites
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->DrawIndicators();
	}

	// Draw all enemy sprites
	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		_enemy_actors[i]->DrawIndicators();
	}
}



// void BattleMode::_ActivateNextCharacter() {
// 	_selected_character_index = GetIndexOfNextIdleCharacter(_selected_character);
// 	if (_selected_character_index != static_cast<int32>(INVALID_BATTLE_ACTOR_INDEX)) {
// 		_selected_character = GetPlayerCharacterAt(_selected_character_index);
// 		_selected_character->ChangeState(ACTOR_STATE_READY);
// 		//_selected_character->GetWaitTime()->Pause();
// 		_action_window->Initialize(_selected_character);
// 		if (wait)
// 			FreezeTimers(); // TEMP: for non-active battle mode
// 	}
// }


// void BattleMode::RemoveActionsForActor(BattleActor * actor) {
// 	list<BattleAction*>::iterator it = _action_queue.begin();
//
// 	while (it != _action_queue.end()) {
// 		if ((*it)->GetSource() == actor) {
// 			if ((*it)->IsItemAction()) {
// 				ItemAction* action = dynamic_cast<ItemAction*>(*it);
// 				action->GetItem()->IncrementCount(1);
// 			}
// 			//it = _action_queue.erase(it);	//remove this location
// 			(*it)->MarkForRemoval();
// 		}
// 		it++;
// 	}
// }




// void BattleMode::AddToTurnQueue(BattleCharacter* character) {
// 	_characters_awaiting_turn.push_back(character);
// }
//
//
//
// void BattleMode::RemoveFromTurnQueue(BattleCharacter *character) {
// 	for (list<private_battle::BattleCharacter*>::iterator i = _characters_awaiting_turn.begin();
// 		i != _characters_awaiting_turn.end(); i++)
// 	{
// 		if ((*i) == character) {
// 			_characters_awaiting_turn.erase(i);
// 			return;
// 		}
// 	}
//
// 	IF_PRINT_WARNING(BATTLE_WARNING) << "character not found in turn queue" << endl;
// }



// uint32 BattleMode::GetIndexOfFirstAliveEnemy() const {
// 	deque<BattleEnemy*>::const_iterator it = _enemy_actors.begin();
// 	for (uint32 i = 0; it != _enemy_actors.end(); i++, it++) {
// 		if ((*it)->IsAlive()) {
// 			return i;
// 		}
// 	}
//
// 	// This should never be reached
// 	return INVALID_BATTLE_ACTOR_INDEX;
// }
//
//
//
// uint32 BattleMode::GetIndexOfLastAliveEnemy() const {
// 	deque<BattleEnemy*>::const_iterator it = _enemy_actors.end()-1;
// 	for (int32 i = _enemy_actors.size()-1; i >= 0; i--, it--) {
// 		if ((*it)->IsAlive()) {
// 			return i;
// 		}
// 	}
//
// 	// This should never be reached
// 	return INVALID_BATTLE_ACTOR_INDEX;
// }



// uint32 BattleMode::GetIndexOfNextIdleCharacter(BattleCharacter *ignore) const {
// 	BattleCharacter* character;
//
// 	if (!_characters_awaiting_turn.size()) {
// 		return INVALID_BATTLE_ACTOR_INDEX;
// 	}
// 	else if (_characters_awaiting_turn.size() == 1 || !ignore) {
// 		character = _characters_awaiting_turn.front();
// 	}
// 	else {
// 		list<BattleCharacter*>::const_iterator it2;
// 		//Find our guy in the waiting queue
// 		for (it2 = _characters_awaiting_turn.begin(); it2 != _characters_awaiting_turn.end(); it2++) {
// 			if ((*it2) == ignore) {
// 				break;
// 			}
// 		}
//
// 		//Grab the guy after him
// 		if ((++it2) != _characters_awaiting_turn.end()) {
// 			character = (*it2);
// 		}
// 		//Loop around if he's at the end of the list
// 		else {
// 			character = _characters_awaiting_turn.front();
// 		}
// 	}
//
// 	deque<BattleCharacter*>::const_iterator it = _character_actors.begin();
// 	for (uint32 i = 0; it != _character_actors.end(); i++, it++) {
// 		if ((*it) == character) {
// 			return i;
// 		}
// 	}
//
// 	return INVALID_BATTLE_ACTOR_INDEX;
// }



// uint32 BattleMode::GetIndexOfNextAliveCharacter(bool move_upward) const {
// 	if (move_upward) {
// 		for (uint32 i = _selected_target_index + 1; i < _character_actors.size(); i++) {
// 			if (_character_actors[i]->IsAlive()) {
// 				return i;
// 			}
// 		}
// 		for (int32 i = 0; i <= _selected_target_index; ++i) {
// 			if (_character_actors[i]->IsAlive()) {
// 				return i;
// 			}
// 		}
//
// 		// This should never be reached
// 		return INVALID_BATTLE_ACTOR_INDEX;
// 	}
// 	else {
// 		for (int32 i = static_cast<int32>(_selected_target_index) - 1; i >= 0; i--) {
// 			if (_character_actors[i]->IsAlive()) {
// 				return i;
// 			}
// 		}
// 		for (int32 i = static_cast<int32>(_character_actors.size()) - 1; i >= static_cast<int32>(_selected_target_index); i--) {
// 			if (_character_actors[i]->IsAlive())
// 			{
// 				return i;
// 			}
// 		}
//
// 		// This should never be reached
// 		return INVALID_BATTLE_ACTOR_INDEX;
// 	}
// }
//
//
//
// uint32 BattleMode::GetIndexOfNextAliveEnemy(bool move_upward) const {
// 	if (move_upward) {
// 		for (uint32 i = _selected_target_index + 1; i < _enemy_actors.size(); i++) {
// 			if (_enemy_actors[i]->IsAlive()) {
// 				return i;
// 			}
// 		}
// 		for (int32 i = 0; i <= _selected_target_index; ++i) {
// 			if (_enemy_actors[i]->IsAlive()) {
// 				return i;
// 			}
// 		}
//
// 		// This should never be reached
// 		return INVALID_BATTLE_ACTOR_INDEX;
// 	}
// 	else {
// 		for (int32 i = static_cast<int32>(_selected_target_index) - 1; i >= 0; i--) {
// 			if (_enemy_actors[i]->IsAlive()) {
// 				return i;
// 			}
// 		}
// 		for (int32 i = static_cast<int32>(_enemy_actors.size()) - 1; i >= static_cast<int32>(_selected_target_index); i--) {
// 			if (_enemy_actors[i]->IsAlive()) {
// 				return i;
// 			}
// 		}
//
// 		// This should never be reached
// 		return INVALID_BATTLE_ACTOR_INDEX;
// 	}
// }


/*
void BattleMode::AddDialogue(string speaker_name, string text) {
	_dialogue_text.push_back(MakeUnicodeString(speaker_name));
	_dialogue_text.push_back(MakeUnicodeString(text));
}



void BattleMode::ShowDialogue() {
	_speaker_name = _dialogue_text.front();
	_dialogue_text.pop_front();
	hoa_utils::ustring text = _dialogue_text.front();
	_dialogue_text.pop_front();

	_dialogue_window.Reset();
	_dialogue_window._display_textbox.SetDisplayText(text);
	_dialogue_window.Initialize();
	ChangeState(BATTLE_STATE_DIALOGUE);
}*/

} // namespace hoa_battle
