////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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

#include <iostream>
#include <sstream>

#include "utils.h"
#include "audio.h"
#include "video.h"
#include "input.h"
#include "mode_manager.h"
#include "system.h"
#include "global.h"
#include "script.h"
#include "battle.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_script;

using namespace hoa_battle::private_battle;

namespace hoa_battle {

bool BATTLE_DEBUG = false;

namespace private_battle {

BattleMode* current_battle = NULL;


////////////////////////////////////////////////////////////////////////////////
// BattleException class
////////////////////////////////////////////////////////////////////////////////
BattleException::BattleException(const std::string & message, const std::string & file, const int line, const std::string & function) throw() :
	Exception(message, file, line, function) {
}

BattleException::~BattleException() throw() {
}

} // namespace private battle


////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- Initialization and Destruction Code
////////////////////////////////////////////////////////////////////////////////

BattleMode::BattleMode() :
	_initialized(false),
	_battle_over(false),
	_victorious_battle(false),
	_selected_character_index(0),
	_selected_target_index(0),
	_selected_character(NULL),
	_selected_target(NULL),
	_selected_attack_point(0),
	//_finish_window(NULL),
	_current_number_swaps(0),
	_swap_countdown_timer(300000), // 5 minutes
	_min_agility(9999),
	//_active_action(NULL),
	_next_monster_location_index(0),
	_default_music("mus/Confrontation.ogg"),
	_winning_music("mus/Allacrost_Fanfare.ogg"),
	_losing_music("mus/Allacrost_Intermission.ogg")
{
	if (BATTLE_DEBUG)
		cout << "BATTLE: BattleMode constructor invoked" << endl;

	mode_type = MODE_MANAGER_BATTLE_MODE;

	if (_stamina_icon_selected.Load("img/menus/stamina_icon_selected.png", 45, 45) == false)
		cerr << "BATTLE ERROR: Failed to load stamina icon selected image" << endl;

	_attack_point_indicator.SetDimensions(16, 16);
	if (_attack_point_indicator.LoadFromFrameGrid("img/icons/battle/attack_point_target.png", vector<uint32>(4, 10), 1, 4) == false)
		cerr << "BATTLE ERROR: Failed to load attack point indicator." << endl;

	//Load the universal time meter image
	if (_stamina_meter.Load("img/menus/stamina_bar.png", 10, 512) == false)
		cerr << "BATTLE ERROR: Failed to load time meter." << endl;

	_actor_selection_image.SetDimensions(109, 78);
	if (_actor_selection_image.Load("img/icons/battle/character_selector.png") == false)
		cerr << "BATTLE ERROR: Unable to load player selector image." << endl;

	if (_character_selection.Load("img/menus/battle_character_selection.png") == false)
		cerr << "BATTLE ERROR: Failed to load character selection image" << endl;

	if (_character_bars.Load("img/menus/battle_character_bars.png") == false)
		cerr << "BATTLE ERROR: Failed to load character bars image" << endl;

	if (_bottom_menu_image.Load("img/menus/battle_bottom_menu.png", 1024, 128) == false)
		cerr << "BATTLE ERROR: Failed to load bottom menu image: " << endl;

	if (_swap_icon.Load("img/icons/battle/swap_icon.png", 35, 30) == false)
		cerr << "BATTLE ERROR: Failed to load swap icon: " << endl;

	if (_swap_card.Load("img/icons/battle/swap_card.png", 25, 37) == false)
		cerr << "BATTLE ERROR: Failed to load swap card: " << endl;

	_action_window;// = new ActionWindow();
	_TEMP_LoadTestData();
} // BattleMode::BattleMode()



BattleMode::~BattleMode() {
	// Don't let current_battle keep pointing to this object instance any longer
	if (current_battle == this) {
		current_battle = NULL;
	}

	for (map<string, MusicDescriptor>::iterator i = _battle_music.begin(); i != _battle_music.end(); i++)
		i->second.FreeAudio();

	// Delete all character and enemy actors
	for (deque<BattleCharacter*>::iterator i = _character_actors.begin(); i != _character_actors.end(); i++) {
		delete *i;
	}
	_character_actors.clear();
	_characters_awaiting_turn.clear();

	for (deque<BattleEnemy*>::iterator i = _enemy_actors.begin(); i != _enemy_actors.end(); i++) {
		delete *i;
	}
	_enemy_actors.clear();

	// FIX ME: If item scripts are still there, add the item back to the inventory
	for (std::list<BattleAction*>::iterator i = _action_queue.begin(); i != _action_queue.end(); i++) {
		if ((*i)->IsItemAction()) {
			ItemAction* action = dynamic_cast<ItemAction*>(*i);
			action->GetItem()->IncrementCount(1);
		}
		delete *i;
	}
	_action_queue.clear();

	for (std::list<DamageText*>::iterator it = _damage_text_list.begin(); it != _damage_text_list.end(); ++it)
	{
		delete *it;
	}
	_damage_text_list.clear();

} // BattleMode::~BattleMode()



void BattleMode::Reset() {
	current_battle = this;

	VideoManager->SetCoordSys(0.0f, 1024.0f, 0.0f, 768.0f);
	VideoManager->Text()->SetDefaultFont("battle");

	// Load the default battle music track if no other music has been added
	if (_battle_music.empty()) {
		_battle_music[_default_music] = MusicDescriptor();
		if (_battle_music[_default_music].LoadAudio(_default_music) == false) {
			PRINT_ERROR << "failed to load default battle theme music" << endl;
		}
		_battle_music[_default_music].Play();
		_current_music = _default_music;
	}
	else
	{
		map<string, MusicDescriptor>::iterator i = _battle_music.begin();
		i->second.Play();
		_current_music = i->first;
	}

	if (_initialized == false) {
		_Initialize();
	}
} // void BattleMode::Reset()



void BattleMode::AddEnemy(GlobalEnemy* new_enemy) {
	// (1): Don't add the enemy if it has an invalid ID or an experience level that is not zero
	if (new_enemy->GetID() == 0) {
		if (BATTLE_DEBUG) {
			cerr << "BATTLE WARNING: attempted to add a new enemy with an invalid id (0). "
				<< "The enemy was not added to the battle." << endl;
		}
		return;
	}
	if (new_enemy->GetExperienceLevel() != 0) {
		if (BATTLE_DEBUG) {
			cerr << "BATTLE WARNING: attempted to add a new enemy that had already been initialized to experience level "
				<< new_enemy->GetExperienceLevel() << ". The enemy was not added to the battle." << endl;
		}
	}

	// (2): Level the enemy up to be within a reasonable range of the party's strength
	new_enemy->Initialize(GlobalManager->AverageActivePartyExperienceLevel());

	// (4): Construct the enemy battle actor to be placed on the battle field
	float x = MONSTER_LOCATIONS[_next_monster_location_index][0];
	float y = MONSTER_LOCATIONS[_next_monster_location_index][1];
	_next_monster_location_index++;

	// TEMP
	// The next line modulus have been changed. Howeever, a better solution is to provide a std::vector
	// instead of an array for MONSTER_LOCATIONS, init in the constructor, and make modulus with the vector length
	// _next_monster_location_index = _next_monster_location_index % (sizeof(MONSTER_LOCATIONS)/2); <-- Before
	_next_monster_location_index = _next_monster_location_index % 8;  // <-- Now

	BattleEnemy* enemy_actor= new BattleEnemy(new_enemy, x, y);
	_enemy_actors.push_back(enemy_actor);
}



void BattleMode::AddMusic(const string& music_filename) {
	if (music_filename == "") {
		if (BATTLE_DEBUG)
			cerr << "BATTLE WARNING: BattleMode::AddMusic was given an empty string argument" << endl;
		return;
	}

	if (_battle_music.find(music_filename) != _battle_music.end())
		// music is already loaded, just return
		return;


	_battle_music[music_filename] = MusicDescriptor();
	if (_battle_music[music_filename].LoadAudio(music_filename) == false) {
		IF_PRINT_WARNING(BATTLE_DEBUG) << "failed to load music file: " << music_filename << endl;
	}
}

void BattleMode::PlayMusic(const string &music_filename)
{
	if (_battle_music.find(music_filename) != _battle_music.end())
	{
		// music is already loaded, play first piece
		map<string, MusicDescriptor>::iterator i = _battle_music.begin();
		i->second.Play();
		_current_music = i->first;
	}
}



void BattleMode::_TEMP_LoadTestData() {
	// Load all background images
	if (_battle_background.Load("img/backdrops/battle/desert_cave.png", SCREEN_LENGTH * TILE_SIZE, SCREEN_HEIGHT * TILE_SIZE) == false) {
		cerr << "BATTLE ERROR: Failed to load background image: " << endl;
		_ShutDown();
	}	
}



void BattleMode::_Initialize() {
	// TODO: If the battle is already initialized, restore the initial state
	// if (_initialized)
	// else

	// (1): Construct all character battle actors from the active party
	GlobalParty* active_party = GlobalManager->GetActiveParty();
	if (active_party->GetPartySize() == 0) {
		if (BATTLE_DEBUG)
			cerr << "BATTLE ERROR: In BattleMode::_Initialize(), the size of the active party was zero" << endl;
		ModeManager->Pop(); // Self-destruct the battle mode
		return;
	}

	for (uint32 i = 0; i < active_party->GetPartySize(); i++) {
		GlobalCharacter* new_character = dynamic_cast<GlobalCharacter*>(active_party->GetActorAtIndex(i));
		BattleCharacter* new_actor = new BattleCharacter(new_character, PLAYER_LOCATIONS[i][0], PLAYER_LOCATIONS[i][1]);
		_character_actors.push_back(new_actor);
	}

	//_selected_character_index = GetIndexOfFirstIdleCharacter();
	//_selected_character = GetPlayerCharacterAt(_selected_character_index);

	// (2) Loop through and find the actor with the lowest agility
	for (uint8 i = 0; i < _enemy_actors.size(); i++) {
		if (_enemy_actors[i]->GetActor()->GetAgility() < _min_agility)
			_min_agility = _enemy_actors[i]->GetActor()->GetAgility();
	}

	for (uint8 i = 0; i < _character_actors.size(); i++) {
		if (_character_actors[i]->GetActor()->GetAgility() < _min_agility)
			_min_agility = _character_actors[i]->GetActor()->GetAgility();
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

	//FIX ME This will not work in the future (i.e. paralysis)...realized this
	//after writing all the above crap
	//CD: Had to move this to before timers are initalized, otherwise this call will give
	//our timers a little extra nudge with regards to time elapsed, thus making the portraits
	//stop before they reach they yellow/orange line
	SystemManager->UpdateTimers();


	//Now adjust starting wait times based on agility proportions
	//If current actor's agility is twice the lowest agility, then
	//they will have a wait time that is half of the slowest actor
	float proportion;

	for (uint8 i = 0; i < _enemy_actors.size(); i++) {
		proportion = static_cast<float>(_min_agility) / static_cast<float>(_enemy_actors[i]->GetActor()->GetAgility());
		_enemy_actors[i]->GetWaitTime()->Initialize(static_cast<uint32>(MAX_INIT_WAIT_TIME * proportion));

		//Start the timer.  We can do this here because the calculations will be done so quickly
		//that the other chars wont fall far behind.
		_enemy_actors[i]->ResetWaitTime();
	}

	for (uint8 i = 0; i < _character_actors.size(); i++) {
		proportion = static_cast<float>(_min_agility) / static_cast<float>(_character_actors[i]->GetActor()->GetAgility());
		_character_actors[i]->GetWaitTime()->Initialize(static_cast<uint32>(MAX_INIT_WAIT_TIME * proportion));

		//Start the timer.  We can do this here because the calculations will be done so quickly
		//that the other chars wont fall far behind.
		_character_actors[i]->ResetWaitTime();
	}

	_initialized = true;
} // void BattleMode::_Initialize()



void BattleMode::_ShutDown() {
	_battle_music[_current_music].Stop();

	_ReviveCharacters();

	// This call will clear the input state
	InputManager->EventHandler();

	// Remove this BattleMode instance from the game stack
	ModeManager->Pop();
}

//Bring all dead people back with 1 HP
void BattleMode::_ReviveCharacters()
{
	for (uint32 i = 0; i < _character_actors.size(); ++i)
	{
		if (!_character_actors[i]->IsAlive())
		{
			_character_actors[i]->GetActor()->RetrieveBattleAnimation("idle")->GetCurrentFrame()->DisableGrayScale();
			_character_actors[i]->GetActor()->SetHitPoints(1);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- update methods
////////////////////////////////////////////////////////////////////////////////

void BattleMode::Update() {
	_battle_over = (_NumberEnemiesAlive() == 0) || (_NumberCharactersAlive() == 0);
	_victorious_battle = (_NumberEnemiesAlive() == 0);

	// ----- (1): If the battle is over, only execute this small block of update code
	if (_battle_over) {
		if (/*!_finish_window*/_finish_window.GetState() == FINISH_INVALID) { // Indicates that the battle has just now finished
			//_finish_window = new FinishWindow();
			// make sure the battle has our music
			if (_victorious_battle)
			{
				AddMusic(_winning_music);
				PlayMusic(_winning_music);
			}
			else
			{
				AddMusic(_losing_music);
				PlayMusic(_losing_music);
			}
			
			_action_window.Reset();
			_finish_window.Initialize(_victorious_battle);

		}

		// The FinishWindow::Update() function handles all update code when a battle is over.
		// The call to shut down battle mode is also made from within this call.
		_finish_window.Update();

		// Do not update other battle code if the battle has already ended
		return;
	} // if (_battle_over)

	// ----- (2): Update the state of all battle actors and graphics
	for (uint8 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->Update();
	}
	for (uint8 i = 0; i < _enemy_actors.size(); i++) {
		_enemy_actors[i]->Update();
	}
	_attack_point_indicator.Update();

	// ----- (3): Execute any scripts that are sitting in the queue
	if (_action_queue.size())
	{
		_UpdateScripts();
		_CleanupActionQueue();
	} // if (_action_queue.size())

	// ----- (4): Try to select an idle character if no character is currently selected
	if (_selected_character == NULL)
	{
		_ActivateNextCharacter();
	}

	// ----- (5): Update the action window if the player is making an action or target selection
	if (_action_window.GetState() != VIEW_INVALID)
		_action_window.Update();

	// ----- (6): Cleanup any lingering damage text
	if (_damage_text_list.size())
	{
		_RemoveExpiredDamageText();
	}
} // void BattleMode::Update()

void BattleMode::_UpdateScripts()
{
	BattleAction* se;
	std::list<private_battle::BattleAction*>::iterator it;

	for (it = _action_queue.begin(); it != _action_queue.end(); it++)
	{
		se = (*it);

		if (se->ShouldBeRemoved())
			continue;

		se->Update();

		if (se->GetWarmUpTime()->IsFinished())// && !_IsExecutingAction())
		{
			//SetPerformingAction(true, se);
			se->RunScript();
			//ran_script = true;
			//Later have battle mode call UpdateActiveBattleAction instead
			//_action_queue.pop_front();
		}
	}

	//Do this out here so iterator doesnt get screwed up mid-loop
	/*if (ran_script)
	{
		SetPerformingAction(false, NULL);
	}*/
}

void BattleMode::_CleanupActionQueue()
{
	std::list<private_battle::BattleAction*>::iterator it;

	for (it = _action_queue.begin(); it != _action_queue.end();)
	{
		if ((*it)->ShouldBeRemoved())
		{
			it = _action_queue.erase(it);
		}
		else
		{
			++it;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- draw methods
////////////////////////////////////////////////////////////////////////////////

void BattleMode::Draw() {
	// Apply scene lighting if the battle has finished
	if (_battle_over) {
		if (_victorious_battle) {
			VideoManager->EnableSceneLighting(Color(0.914f, 0.753f, 0.106f, 1.0f)); // Golden color for victory
		}
		else {
			VideoManager->EnableSceneLighting(Color(1.0f, 0.0f, 0.0f, 1.0f)); // Red color for defeat
		}
	}

	_DrawBackgroundVisuals();
	_DrawBottomMenu();
	_DrawSprites();
	_DrawDamageTextList();
	_DrawStaminaBar();

	if (_action_window.GetState() != VIEW_INVALID) {
		_action_window.Draw();
	}

	if (_battle_over) {
		_finish_window.Draw();
	}
} // void BattleMode::Draw()



void BattleMode::_DrawBackgroundVisuals() {
	// Draw the full-screen, static background image
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_NO_BLEND, 0);
	VideoManager->Move(0, 0);
	_battle_background.Draw();

	// TODO: Draw other background objects and animations
} // void BattleMode::_DrawBackgroundVisuals()



void BattleMode::_DrawBottomMenu() {
	// Draw the static image for the lower menu
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(0, 0);
	_bottom_menu_image.Draw();

	// Draw the swap icon and any swap cards
	VideoManager->Move(6, 16);
	_swap_icon.Draw(Color::gray);
	VideoManager->Move(6, 68);
	for (uint8 i = 0; i < _current_number_swaps; i++) {
		_swap_card.Draw();
		VideoManager->MoveRelative(4, -4);
	}

	// Draw the selected character's portrait, blended according to the character's current HP level
	if (_selected_character)
		_selected_character->DrawPortrait();

	// Draw the status information of all character actors
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->DrawStatus();
	}
} // void BattleMode::_DrawBottomMenu()



void BattleMode::_DrawSprites() {
	// TODO: Draw sprites in order based on their x and y coordinates on the screen (bottom to top, then left to right)

	// Draw all character sprites
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->DrawSprite();
	}

	// Sort and draw the enemies
	//std::deque<private_battle::BattleEnemy*> sorted_enemy_actors = _enemy_actors;
 	//std::sort(sorted_enemy_actors.begin(), sorted_enemy_actors.end(), AscendingYSort());

	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		_enemy_actors[i]->DrawSprite();
	}
} // void BattleMode::_DrawSprites()


void BattleMode::_DrawDamageTextList()
{
	if (!_damage_text_list.size())
		return;

	std::list<DamageText*>::const_iterator it = _damage_text_list.begin();

	for (; it != _damage_text_list.end(); ++it)
	{
		(*it)->Draw();
	}
}

void BattleMode::_DrawStaminaBar() {
	// ----- (1): Draw the stamina bar
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->Move(1010, 128);
	_stamina_meter.Draw();

	// ----- (2): Determine the draw order of all stamina icons and whether or not they are selected
	GLOBAL_TARGET target_type = _action_window.GetActionTargetType();
	bool target_character = _action_window.IsActionTargetAlly();
	std::vector<BattleActor*> live_actors;

	//FIX ME Below is the logic that should be used...requires change to UpdateTargetSelection code
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

	//CD: Condensed the below code so it takes fewer iterations and if checks
	//to draw the portraits (on average)
	if (target_type  == GLOBAL_TARGET_PARTY)
	{
		if (target_character == true) { // All characters are selected
			for (uint32 i = 0; i < live_actors.size(); i++) {
				/*if (live_actors[i]->IsEnemy() == false) {
					selected[i] = true;
				}*/
				live_actors[i]->DrawStaminaIcon(!live_actors[i]->IsEnemy());
			}
		}
		else { // All enemies are selected
			for (uint32 i = 0; i < live_actors.size(); i++) {
				/*if (live_actors[i]->IsEnemy() == true) {
					selected[i] = true;
				}*/
				live_actors[i]->DrawStaminaIcon(live_actors[i]->IsEnemy());
			}
		}
	}
	else { // Find the actor who is the selected target
		for (uint32 i = 0; i < live_actors.size(); i++) {
			/*if (live_actors[i] == _selected_target) {
				selected[i] = true;
				break;
			}*/
			live_actors[i]->DrawStaminaIcon(live_actors[i] == _selected_target);
		}
	}

	// ----- (3): Draw all stamina icons for live actors
	/*VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	for (uint32 i = 0; i < live_actors.size(); i++) {
		live_actors[i]->DrawStaminaIcon(selected[i]);
	}*/
} // void BattleMode::_DrawStaminaBar()

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- miscellaneous Code
////////////////////////////////////////////////////////////////////////////////

void BattleMode::_ActivateNextCharacter()
{
	_selected_character_index = GetIndexOfNextIdleCharacter(_selected_character);
	if (_selected_character_index != static_cast<int32>(INVALID_BATTLE_ACTOR_INDEX))
	{
		_selected_character = GetPlayerCharacterAt(_selected_character_index);
		_selected_character->SetState(ACTOR_AWAITING_TURN);
		//_selected_character->GetWaitTime()->Pause();
		_action_window.Initialize(_selected_character);
	}
}

void BattleMode::_SetInitialTarget() {
	if (_action_window._action_target_ally == true) {
		_selected_target = GetPlayerCharacterAt(0);
		_selected_target_index = 0;
		_selected_attack_point = 0;
	}
	else {
		_selected_target = GetEnemyActorAt(GetIndexOfFirstAliveEnemy());
		_selected_target_index = GetIndexOfFirstAliveEnemy();
		_selected_attack_point = 0;
	}
}



void BattleMode::_SelectNextTarget(bool forward_direction) {
	if (_selected_target == NULL) {
		_SetInitialTarget();
		return;
	}

	uint32 previous_target = _selected_target_index;
	if (forward_direction)
	{
		if (_action_window._action_target_ally == true)
		{
			_selected_target_index = (_selected_target_index + 1) % _character_actors.size();
			_selected_target = _character_actors[_selected_target_index];
		}
		else
		{
			_selected_target_index = GetIndexOfNextAliveEnemy(forward_direction);
			_selected_target = (_selected_target_index == INVALID_BATTLE_ACTOR_INDEX) ? NULL : _enemy_actors[_selected_target_index];
		}

	}

	else {

		if (_action_window._action_target_ally == true)
		{
			_selected_target_index--;

			if (_selected_target_index < 0)
			{
				_selected_target_index = _character_actors.size() - 1;
			}
			_selected_target = _character_actors[_selected_target_index];
		}
		else
		{
			_selected_target_index = GetIndexOfNextAliveEnemy(forward_direction);
			_selected_target = (_selected_target_index == INVALID_BATTLE_ACTOR_INDEX) ? NULL : _enemy_actors[_selected_target_index];
		}
	}

	if (previous_target != _selected_target_index)
		_selected_attack_point = 0;
} // void BattleMode::_SelectNextTarget(bool forward_direction)



void BattleMode::_SelectNextAttackPoint(bool forward_direction) {
	if (_selected_target == NULL) {
		_SetInitialTarget();
		return;
	}

	if (forward_direction) {
		_selected_attack_point++;
		if (_selected_attack_point >= _selected_target->GetActor()->GetAttackPoints()->size())
			_selected_attack_point = 0;
	}

	else {
		if (_selected_attack_point == 0)
			_selected_attack_point = _selected_target->GetActor()->GetAttackPoints()->size() - 1;
		else
			_selected_attack_point--;
	}
} // void BattleMode::_SelectNextAttackPoint(bool forward_direction)



bool _TEMPIsA1Smaller(BattleEnemy* a1, BattleEnemy* a2) {
	if (a1->GetYLocation() - a1->GetActor()->GetSpriteHeight() < a2->GetYLocation() - a2->GetActor()->GetSpriteHeight())
		return true;

	return false;
}


// Ascending Y sorting functor. We want to compare the actual objects, NOT pointers!
struct AscendingYSort {
	bool operator()(BattleEnemy* a1, BattleEnemy* a2)
	{
		//return ((*a1) < (*a2));
		return _TEMPIsA1Smaller(a1, a2);
	}
};



uint32 BattleMode::_NumberEnemiesAlive() const {
	uint32 enemy_count = 0;
	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		if (_enemy_actors[i]->IsAlive()) {
			enemy_count++;
		}
	}
	return enemy_count;
}



uint32 BattleMode::_NumberCharactersAlive() const {
	uint32 character_count = 0;
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		if (_character_actors[i]->IsAlive()) {
			character_count++;
		}
	}
	return character_count;
}



/*void BattleMode::SetPerformingAction(bool is_performing, BattleAction* se) {
	// Check if a script has just ended. Set the script to stop performing and pop the script from the front of the queue
	// CD: Only one script will be running at a time, so only need to check the incoming bool

	if (is_performing == false)
	{
		// Remove the first scripted event from the queue
		// _action_queue.front().GetSource() is always either BattleEnemy or BattleCharacter
		//IBattleActor * source = dynamic_cast<IBattleActor*>(_action_queue.front().GetSource());
		//IBattleActor* source = _action_queue.front().GetSource();
		BattleActor* source = (*_active_action).GetSource();
		if (source) {
			source->SetState(ACTOR_IDLE);
			//FIXME Use char and enemy stats
			source->ResetWaitTime();
		}

		//	std::list<private_battle::BattleAction*>::iterator it = _action_queue.begin();
		//	while (it != _action_queue.end()) {
		//		if ((*it) == _active_action) {
		//			_action_queue.erase(it);
		//			break;
		//		}
		//		it++;
		//	}
		}
		else {
			cerr << "Invalid BattleActor pointer in SetPerformingScript()" << endl;
			SystemManager->ExitGame();
		}
	}
	else
	{
		if (se == NULL) {
			cerr << "Invalid BattleActor pointer in SetPerformingScript()" << endl;
			SystemManager->ExitGame();
		}
	}

	_active_action = se;
}*/



void BattleMode::RemoveActionsForActor(BattleActor * actor) {
	std::list<private_battle::BattleAction*>::iterator it = _action_queue.begin();

	while (it != _action_queue.end()) {
		if ((*it)->GetSource() == actor) {
			if ((*it)->IsItemAction()) {
				ItemAction* action = dynamic_cast<ItemAction*>(*it);
				action->GetItem()->IncrementCount(1);
			}
			//it = _action_queue.erase(it);	//remove this location
			(*it)->MarkForRemoval();
		}
		it++;
	}
}

void BattleMode::NotifyOfActorDeath(BattleActor *actor)
{
	RemoveActionsForActor(actor);
	if (!actor->IsEnemy() && (((BattleActor *)(_selected_character)) == actor))
	{
		_selected_character = NULL;
		_action_window.Reset();
	}

	BattleCharacter *character = dynamic_cast<BattleCharacter*>(actor);
	if (character)
	{
		RemoveFromTurnQueue(character);
	}
}

void BattleMode::FreezeTimers() {
	// Pause scripts
	std::list<private_battle::BattleAction*>::iterator it = _action_queue.begin();

	while (it != _action_queue.end()) {
		(*it)->GetWarmUpTime()->Pause();
		it++;
	}

	// Pause characters
	for (uint32 i = 0; i < _character_actors.size(); ++i)
		_character_actors.at(i)->GetWaitTime()->Pause();

	//Pause enemies
	for (uint32 i = 0; i < _enemy_actors.size(); ++i)
		_enemy_actors.at(i)->GetWaitTime()->Pause();
}


void BattleMode::UnFreezeTimers() {
	// FIX ME: Do not unpause timers for paralyzed actors

	// Unpause scripts
	std::list<private_battle::BattleAction*>::iterator it = _action_queue.begin();
	while (it != _action_queue.end()) {
		(*it)->GetWarmUpTime()->Run();
		it++;
	}

	// Unpause characters
	for (uint32 i = 0; i < _character_actors.size(); ++i)
		_character_actors.at(i)->GetWaitTime()->Run();

	// Unpause enemies
	for (uint32 i = 0; i < _enemy_actors.size(); ++i)
		_enemy_actors.at(i)->GetWaitTime()->Run();
}



void BattleMode::SwapCharacters(BattleCharacter* ActorToRemove, BattleCharacter * ActorToAdd) {
	// Remove 'ActorToRemove'
	for (std::deque < BattleCharacter * >::iterator it = _character_actors.begin(); it != _character_actors.end(); it++) {
		if (*it == ActorToRemove) {
			_character_actors.erase(it);
			break;
		}
	}

	// set location and origin to removing characters location and origin
	ActorToAdd->SetXOrigin(ActorToRemove->GetXOrigin());
	ActorToAdd->SetYOrigin(ActorToRemove->GetYOrigin());
	ActorToAdd->SetXLocation(static_cast<float>(ActorToRemove->GetXOrigin()));
	ActorToAdd->SetYLocation(static_cast<float>(ActorToRemove->GetYOrigin()));

	_character_actors.push_back(ActorToAdd);	//add the other character to battle
}


void BattleMode::AddToTurnQueue(BattleCharacter* character)
{
	_characters_awaiting_turn.push_back(character);
}

void BattleMode::RemoveFromTurnQueue(BattleCharacter *character)
{
	std::list<private_battle::BattleCharacter*>::iterator it;

	for (it = _characters_awaiting_turn.begin(); it != _characters_awaiting_turn.end(); it++)
	{
		if ((*it) == character)
		{
			_characters_awaiting_turn.erase(it);
			break;
		}
	}
}


uint32 BattleMode::GetIndexOfFirstAliveEnemy() const {
	std::deque<private_battle::BattleEnemy*>::const_iterator it = _enemy_actors.begin();
	for (uint32 i = 0; it != _enemy_actors.end(); i++, it++) {
		if ((*it)->IsAlive()) {
			return i;
		}
	}

	// This should never be reached
	return INVALID_BATTLE_ACTOR_INDEX;
}



uint32 BattleMode::GetIndexOfLastAliveEnemy() const {
	std::deque<private_battle::BattleEnemy*>::const_iterator it = _enemy_actors.end()-1;
	for (int32 i = _enemy_actors.size()-1; i >= 0; i--, it--) {
		if ((*it)->IsAlive()) {
			return i;
		}
	}

	// This should never be reached
	return INVALID_BATTLE_ACTOR_INDEX;
}



uint32 BattleMode::GetIndexOfNextIdleCharacter(BattleCharacter *ignore) const
{
	BattleCharacter* character;

	if (!_characters_awaiting_turn.size())
	{
		return INVALID_BATTLE_ACTOR_INDEX;
	}	
	else if (_characters_awaiting_turn.size() == 1 || !ignore)
	{
		character = _characters_awaiting_turn.front();
	}
	else
	{
		std::list<BattleCharacter*>::const_iterator it2;
		//Find our guy in the waiting queue
		for (it2 = _characters_awaiting_turn.begin(); it2 != _characters_awaiting_turn.end(); it2++)
		{
			if ((*it2) == ignore)
			{
				break;
			}
		}

		//Grab the guy after him
		if ((++it2) != _characters_awaiting_turn.end())
		{
			character = (*it2);
		}
		//Loop around if he's at the end of the list
		else
		{
			character = _characters_awaiting_turn.front();
		}
	}

	std::deque<BattleCharacter*>::const_iterator it = _character_actors.begin();
	for (uint32 i = 0; it != _character_actors.end(); i++, it++)
	{
		if ((*it) == character)
		{
			return i;
		}
	}
	
	return INVALID_BATTLE_ACTOR_INDEX;
}



uint32 BattleMode::GetIndexOfCharacter(BattleCharacter* const actor) const {
	deque<BattleCharacter*>::const_iterator it = _character_actors.begin();
	for (int32 i = 0; it != _character_actors.end(); i++, it++) {
		if (*it == actor)
			return i;
	}

	// This should never be reached
	return INVALID_BATTLE_ACTOR_INDEX;
}

uint32 BattleMode::GetIndexOfNextAliveCharacter(bool move_upward) const
{
	if (move_upward) {
		for (uint32 i = _selected_target_index + 1; i < _character_actors.size(); i++) {
			if (_character_actors[i]->IsAlive()) {
				return i;
			}
		}
		for (int32 i = 0; i <= _selected_target_index; ++i) {
			if (_character_actors[i]->IsAlive()) {
				return i;
			}
		}

		// This should never be reached
		return INVALID_BATTLE_ACTOR_INDEX;
	}
	else {
		for (int32 i = static_cast<int32>(_selected_target_index) - 1; i >= 0; i--) {
			if (_character_actors[i]->IsAlive()) {
				return i;
			}
		}
		for (int32 i = static_cast<int32>(_character_actors.size()) - 1; i >= static_cast<int32>(_selected_target_index); i--) {
			if (_character_actors[i]->IsAlive())
			{
				return i;
			}
		}

		// This should never be reached
		return INVALID_BATTLE_ACTOR_INDEX;
	}
}

uint32 BattleMode::GetIndexOfNextAliveEnemy(bool move_upward) const {
	if (move_upward) {
		for (uint32 i = _selected_target_index + 1; i < _enemy_actors.size(); i++) {
			if (_enemy_actors[i]->IsAlive()) {
				return i;
			}
		}
		for (int32 i = 0; i <= _selected_target_index; ++i) {
			if (_enemy_actors[i]->IsAlive()) {
				return i;
			}
		}

		// This should never be reached
		return INVALID_BATTLE_ACTOR_INDEX;
	}
	else {
		for (int32 i = static_cast<int32>(_selected_target_index) - 1; i >= 0; i--) {
			if (_enemy_actors[i]->IsAlive()) {
				return i;
			}
		}
		for (int32 i = static_cast<int32>(_enemy_actors.size()) - 1; i >= static_cast<int32>(_selected_target_index); i--) {
			if (_enemy_actors[i]->IsAlive())
			{
				return i;
			}
		}

		// This should never be reached
		return INVALID_BATTLE_ACTOR_INDEX;
	}
} // uint32 BattleMode::GetIndexOfNextAliveEnemy(bool move_upward) const


void BattleMode::AddDamageText(hoa_utils::ustring text, uint32 duration, float x, float y)
{
	_damage_text_list.push_front(new DamageText(text, duration, x, y));
}

void BattleMode::_RemoveExpiredDamageText()
{
	std::list<DamageText*>::iterator it;

	for (it = _damage_text_list.begin(); it != _damage_text_list.end();)
	{
		if ((*it)->GetTimer()->IsFinished())
		{
			it = _damage_text_list.erase(it);
		}
		else
		{
			++it;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// DamageText class
////////////////////////////////////////////////////////////////////////////////


void DamageText::Draw()
{
	VideoManager->Text()->SetDefaultFont("battle_dmg");
	VideoManager->Text()->SetDefaultTextColor(Color::red);
	VideoManager->Move(_x_pos, _y_pos + ( _timer.GetTimeExpired() / 35.0f ));
	VideoManager->Text()->Draw(_text);
	VideoManager->Text()->SetDefaultFont("battle");
}

} // namespace hoa_battle
