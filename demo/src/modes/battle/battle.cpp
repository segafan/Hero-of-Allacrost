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

	if (ImageDescriptor::LoadMultiImageFromElementGrid(_status_icons, "img/icons/effects/status.png", 24, 5) == false)
		PRINT_ERROR << "failed to load status icon images" << endl;

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
	_character_party.clear();

	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		delete _enemy_actors[i];
	}
	_enemy_actors.clear();
	_enemy_party.clear();

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
		ModeManager->Push(new PauseMode(true));
		return;
	}
	if (InputManager->PausePress()) {
		ModeManager->Push(new PauseMode(false));
		return;
	}

	if (IsBattleFinished() == true) {
		_finish_window->Update();
		return;
	}

	if (_state == BATTLE_STATE_COMMAND) {
		_command_supervisor->Update();
		return; // This return effectively pauses the state of the actors while the player selects another command
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

	// Update all actors
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->Update();
	}
	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		_enemy_actors[i]->Update();
	}
} // void BattleMode::Update()



void BattleMode::Draw() {
	// Apply scene lighting if the battle has finished
	if ((_state == BATTLE_STATE_VICTORY || _state == BATTLE_STATE_DEFEAT)) {// && _after_scripts_finished) {
		if (_state == BATTLE_STATE_VICTORY) {
//			VideoManager->EnableSceneLighting(Color(0.914f, 0.753f, 0.106f, 1.0f)); // Golden color for victory
		}
		else {
//			VideoManager->EnableSceneLighting(Color(1.0f, 0.0f, 0.0f, 1.0f)); // Red color for defeat
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
	BattleEnemy* new_enemy_combatant = new BattleEnemy(new_enemy);
	_enemy_actors.push_back(new_enemy_combatant);
	_enemy_party.push_back(new_enemy_combatant);
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
			if (_command_queue.empty() == true) {
				IF_PRINT_WARNING(BATTLE_DEBUG) << "command queue was empty when trying to change to command state" << endl;
				_state = BATTLE_STATE_NORMAL;
			}
			else {
				_command_supervisor->Initialize(_command_queue.front());
			}
			break;
		case BATTLE_STATE_EVENT:
			// TODO
			break;
		case BATTLE_STATE_VICTORY:
			AddMusic(_winning_music);
			PlayMusic(_winning_music);
			_finish_window->Initialize(true);
			break;
		case BATTLE_STATE_DEFEAT:
			AddMusic(_losing_music);
			PlayMusic(_losing_music);
			_finish_window->Initialize(false);
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



StillImage* BattleMode::GetStatusIcon(GLOBAL_STATUS type, GLOBAL_INTENSITY intensity) {
	if ((type <= GLOBAL_STATUS_INVALID) || (type >= GLOBAL_STATUS_TOTAL)) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "type argument was invalid: " << type << endl;
		return NULL;
	}
	if ((intensity < GLOBAL_INTENSITY_NEUTRAL) || (intensity >= GLOBAL_INTENSITY_TOTAL)) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "type argument was invalid: " << intensity << endl;
		return NULL;
	}

	uint32 status_index = static_cast<uint32>(type);
	uint32 intensity_index = static_cast<uint32>(intensity);

	return &(_status_icons[(status_index * 5) + intensity_index]);
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



void BattleMode::NotifyCharacterCommandComplete(BattleCharacter* character) {
	if (character == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received NULL argument" << endl;
		return;
	}
	if (_command_queue.empty() == true) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "no characters were in the command queue when function was called" << endl;
		return;
	}
	if (character != _command_queue.front()) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function argument was not the same character as the front of the command queue" << endl;
	}

	character->ChangeState(ACTOR_STATE_WARM_UP);

	_command_queue.pop_front();
	if (_command_queue.empty() == true)
		ChangeState(BATTLE_STATE_NORMAL);
	else
		_command_supervisor->Initialize(_command_queue.front());
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
	if (actor == NULL) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "function received NULL argument" << endl;
		return;
	}

	// Remove actor from command and ready queues if it is found in either one
	if (actor->IsEnemy() == false)
		_command_queue.remove(dynamic_cast<BattleCharacter*>(actor));
	_ready_queue.remove(actor);

	// Notify the command supervisor about the death event if it is active
	if (_state == BATTLE_STATE_COMMAND) {
		_command_supervisor->NotifyActorDeath(actor);

		// If the actor who died was the character that the player was selecting a command for, this will cause the
		// command supervisor will return to the invalid state.
		if (_command_supervisor->GetState() == COMMAND_STATE_INVALID) {
			// Either re-initialize the command supervisor with another character, or return to the normal state
			if (_command_queue.empty() == false)
				_command_supervisor->Initialize(_command_queue.front());
			else
				ChangeState(BATTLE_STATE_NORMAL);
		}
	}

	// Determine if the battle should proceed to the victory or defeat state
	if (IsBattleFinished() == true) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "actor death occurred after battle was finished" << endl;
	}

	uint32 num_alive_characters = _NumberCharactersAlive();
	uint32 num_alive_enemies = _NumberEnemiesAlive();
	if ((num_alive_characters == 0) && (num_alive_enemies == 0)) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "both parties were defeated; changing to defeat state" << endl;
		ChangeState(BATTLE_STATE_DEFEAT);
	}
	else if (num_alive_characters == 0) {
		ChangeState(BATTLE_STATE_DEFEAT);
	}
	else if (num_alive_enemies == 0) {
		ChangeState(BATTLE_STATE_VICTORY);
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
		_character_party.push_back(new_actor);
	}
	_command_supervisor->ConstructCharacterSettings();

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
		proportion = static_cast<float>(min_agility) / static_cast<float>(_character_actors[i]->GetAgility());
		_character_actors[i]->SetIdleStateTime(static_cast<uint32>(MAX_INIT_WAIT_TIME * proportion));
		_character_actors[i]->ChangeState(ACTOR_STATE_IDLE);
	}
	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		proportion = static_cast<float>(min_agility) / static_cast<float>(_enemy_actors[i]->GetAgility());
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

	// Determine the position of the first character in the party, who will be drawn at the top
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
		default:
			position_x = 160.0f;
			position_y = 448.0f;
			break;
	}

	// Set all characters in their proper positions
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->SetXOrigin(position_x);
		_character_actors[i]->SetYOrigin(position_y);
		_character_actors[i]->SetXLocation(position_x);
		_character_actors[i]->SetYLocation(position_y);
		position_x -= 32.0f;
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



void BattleMode::_DrawBackgroundGraphics() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_NO_BLEND, 0);
	VideoManager->Move(0.0f, 0.0f);
	_battle_background.Draw();

	// TODO: Draw other background objects and animations
}



void BattleMode::_DrawSprites() {
	// Boolenas used to determine whether or not the actor selector and attack point selector graphics should be drawn
	bool draw_actor_selection = false;
	bool draw_point_selection = false;

	BattleTarget target = _command_supervisor->GetSelectedTarget(); // The target that the player has selected
	BattleActor* actor_target = target.GetActor(); // A pointer to an actor being targetted (may initially be NULL if target is party)

	// Determine if selector graphics should be drawn
	if ((_state == BATTLE_STATE_COMMAND) && (_command_supervisor->GetState() == COMMAND_STATE_TARGET)) {
		draw_actor_selection = true;
		if (IsTargetPoint(target.GetType()) == true)
			draw_point_selection = true;
	}

	// Draw the actor selector graphic
	if (draw_actor_selection == true) {
		VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, VIDEO_BLEND, 0);
		if (actor_target != NULL) {
			actor_target = target.GetActor();
			VideoManager->Move(actor_target->GetXLocation(), actor_target->GetYLocation());
			if (actor_target->IsEnemy() == false)
				VideoManager->MoveRelative(-32.0f, 0.0f); // TEMP: this should move half the distance of the actor's sprite, not 32.0f
			else
				VideoManager->MoveRelative(32.0f, 0.0f);
			_actor_selection_image.Draw();
		}
		else if (IsTargetParty(target.GetType()) == true) {
			// TODO: add support for drawing graphic under multiple actors if the target is a party
		}
		// Else this target is invalid so don't draw anything
	}

	// TODO: Draw sprites in order based on their x and y coordinates on the screen (bottom to top, then left to right)

	// Draw all character sprites
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->DrawSprite();
	}

	// Draw all enemy sprites
	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		_enemy_actors[i]->DrawSprite();
	}

	// Draw the attack point selector graphic
	if (draw_point_selection == true) {
		uint32 point = target.GetPoint();

		VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, VIDEO_BLEND, 0);
		VideoManager->Move(actor_target->GetXLocation(), actor_target->GetYLocation());
		VideoManager->MoveRelative(actor_target->GetAttackPoint(point)->GetXPosition(), actor_target->GetAttackPoint(point)->GetYPosition());
		_attack_point_indicator.Draw();
	}
} // void BattleMode::_DrawSprites()



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

	// Draw the selection highlight and portrait for the active character having a command selected by the player
	if (_state == BATTLE_STATE_COMMAND) {
		BattleCharacter* character = _command_supervisor->GetCommandCharacter();
		uint32 character_position = 0xFFFFFFFF; // Initial value used to check for warning condition

		for (uint32 i = 0; i < _character_actors.size(); i++) {
			if (_character_actors[i] == character) {
				character_position = i;
				break;
			}
		}
		if (character_position == 0xFFFFFFFF) {
			IF_PRINT_WARNING(BATTLE_DEBUG) << "the command character was not found in the character actor containers" << endl;
			character_position = 0;
		}

		VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
		VideoManager->Move(148.0f, 85.0f - (character_position * 25.0f));
		_character_selection.Draw();

		character->DrawPortrait();
	}

	// Draw the status information of all character actors
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->DrawStatus(i);
	}
}



void BattleMode::_DrawStaminaBar() {
	bool draw_icon_selection = false; // Used to determine whether or not an icon selector graphic needs to be drawn
	bool is_party_selected = false; // If true, an entire party of actors is selected
	bool is_party_enemy = false; // If true, the selected party is the enemy party

	BattleActor* selected_actor = NULL; // A pointer to the selected actor

	// ----- (1): Determine if selector graphics should be drawn
	if ((_state == BATTLE_STATE_COMMAND) && (_command_supervisor->GetState() == COMMAND_STATE_TARGET)) {
		BattleTarget target = _command_supervisor->GetSelectedTarget();

		draw_icon_selection = true;
		selected_actor = target.GetActor(); // Will remain NULL if the target type is a party

		if (target.GetType() == GLOBAL_TARGET_ALL_ALLIES) {
			is_party_selected = true;
			is_party_enemy = false;
		}
		else if (target.GetType() == GLOBAL_TARGET_ALL_FOES) {
			is_party_selected = true;
			is_party_enemy = true;
		}
	}

	// ----- (2): Determine the draw order of stamina icons for all living actors
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

	// ----- (3): Draw the stamina bar
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->Move(1010.0f, 128.0f);
	_stamina_meter.Draw();

	// ----- 4): Draw all stamina icons in order along with the selector graphic
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	for (uint32 i = 0; i < live_actors.size(); i++) {
		VideoManager->Move(1000.0f, draw_positions[i]);
		live_actors[i]->GetStaminaIcon().Draw();

		if (draw_icon_selection == true) {
			if ((is_party_selected == false) && (live_actors[i] == selected_actor))
				_stamina_icon_selected.Draw();
			else if ((is_party_selected == true) && (live_actors[i]->IsEnemy() == is_party_enemy))
				_stamina_icon_selected.Draw();
		}
	}
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



// void BattleMode::AddDialogue(string speaker_name, string text) {
// 	_dialogue_text.push_back(MakeUnicodeString(speaker_name));
// 	_dialogue_text.push_back(MakeUnicodeString(text));
// }



// void BattleMode::ShowDialogue() {
// 	_speaker_name = _dialogue_text.front();
// 	_dialogue_text.pop_front();
// 	hoa_utils::ustring text = _dialogue_text.front();
// 	_dialogue_text.pop_front();
//
// 	_dialogue_window.Reset();
// 	_dialogue_window._display_textbox.SetDisplayText(text);
// 	_dialogue_window.Initialize();
// 	ChangeState(BATTLE_STATE_DIALOGUE);
// }

} // namespace hoa_battle
