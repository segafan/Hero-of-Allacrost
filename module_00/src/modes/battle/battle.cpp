////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle.cpp
*** \author  Corey Hoffstein, visage@allacrost.org
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
#include "data.h"

#include "battle.h"
#include "battle_actors.h"
#include "boot.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_data;

using namespace hoa_battle::private_battle;
using namespace hoa_boot;

namespace hoa_battle {

bool BATTLE_DEBUG = false;

namespace private_battle {

BattleMode* current_battle = NULL;

////////////////////////////////////////////////////////////////////////////////
// SCRIPTEVENT CLASS
////////////////////////////////////////////////////////////////////////////////

ScriptEvent::ScriptEvent(BattleActor* source, deque<BattleActor*> targets, string script_name) :
	_script_name(script_name),
	_source(source),
	_targets(targets)
{}



ScriptEvent::~ScriptEvent()
{}



void ScriptEvent::RunScript() {
	// TEMP: do basic damage to the actors
	for (uint8 i = 0; i < _targets.size(); i++) {
		_targets[i]->TEMP_Deal_Damage(GaussianRandomValue(12, 2));
	}
	// TODO: get script from global script repository and run, passing in list of arguments and host actor
}

} // namespace private battle

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- Initialization and Destruction Code
////////////////////////////////////////////////////////////////////////////////

int32 BattleMode::MAX_PLAYER_CHARACTERS_IN_BATTLE = 4;
int32 BattleMode::MAX_ENEMY_CHARACTERS_IN_BATTLE  = 8;

BattleMode::BattleMode() :
	_performing_script(false),
	_battle_over(false),
	_victorious_battle(false),
	_selected_character(NULL),
	_selected_enemy(NULL),
	_necessary_selections(0),
	_attack_point_selected(0),
	_number_menu_items(0),
	_cursor_state(CURSOR_IDLE),
	_action_menu_window(NULL),
	_action_list_menu(NULL),
	_current_number_swaps(0),
	_swap_countdown_timer(300000) // 5 minutes
{
	std::vector <hoa_video::StillImage> attack_point_indicator;
	StillImage frame;
	frame.SetDimensions(16, 16);
	frame.SetFilename("img/icons/battle/ap_indicator_fr0.png");
	attack_point_indicator.push_back(frame);
	frame.SetFilename("img/icons/battle/ap_indicator_fr1.png");
	attack_point_indicator.push_back(frame);
	frame.SetFilename("img/icons/battle/ap_indicator_fr2.png");
	attack_point_indicator.push_back(frame);
	frame.SetFilename("img/icons/battle/ap_indicator_fr3.png");
	attack_point_indicator.push_back(frame);

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < attack_point_indicator.size(); i++) {
		if (!VideoManager->LoadImage(attack_point_indicator[i]))
			cerr << "BATTLE ERROR: Failed to load attack point indicator." << endl;
	}
	VideoManager->EndImageLoadBatch();
	
	for (uint32 i = 0; i < attack_point_indicator.size(); i++) {
		_attack_point_indicator.AddFrame(attack_point_indicator[i], 10);
	}
	
	_actor_selection_image.SetDimensions(109, 78);
	_actor_selection_image.SetFilename("img/icons/battle/character_selector.png");
	if (!VideoManager->LoadImage(_actor_selection_image)) {
		cerr << "BATTLE ERROR: Unable to load player selector image." << endl;
	}

	vector<ustring> action_type_options;
	action_type_options.push_back(MakeUnicodeString("<img/icons/battle/attack.png><55>Attack"));
	action_type_options.push_back(MakeUnicodeString("<img/icons/battle/defend.png><55>Defend"));
	action_type_options.push_back(MakeUnicodeString("<img/icons/battle/support.png><55>Support"));
	action_type_options.push_back(MakeUnicodeString("<img/icons/battle/item.png><55>Item"));

	_action_type_menu.SetOptions(action_type_options);
	_action_type_menu.EnableOption(1, false); // Disable defend and support for now
	_action_type_menu.EnableOption(2, false);

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);

	_action_menu_window = new MenuWindow();
	_action_menu_window->Create(210.0f, 384.0f);
	_action_menu_window->SetPosition(0.0f, 544.0f);
	_action_menu_window->SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_action_menu_window->Hide();

	_action_type_menu_cursor_location = 0;

// 	_action_type_menu.SetOwner(_action_menu_window);
	_action_type_menu.SetCursorOffset(-20.0f, 25.0f);
	_action_type_menu.SetCellSize(100.0f, 80.0f);
	_action_type_menu.SetSize(1, 4);
	_action_type_menu.SetPosition(30.0f, 512.0f);
	_action_type_menu.SetFont("battle");
	_action_type_menu.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_action_type_menu.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_action_type_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	_action_type_menu.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
 	_action_type_menu.SetSelection(0); // This line may be causing a seg-fault!

	_battle_lose_menu.SetCellSize(128.0f, 50.0f);
	_battle_lose_menu.SetPosition(530.0f, 380.0f);
	_battle_lose_menu.SetSize(1, 1);
	_battle_lose_menu.SetFont("battle");
	_battle_lose_menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_battle_lose_menu.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_battle_lose_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	_battle_lose_menu.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_battle_lose_menu.SetCursorOffset(-60.0f, 25.0f);
	vector<ustring> loseText;
	loseText.push_back(MakeUnicodeString("Return to the main menu"));
	_battle_lose_menu.SetOptions(loseText);
 	_battle_lose_menu.SetSelection(0); // This line may be causing a seg-fault!

	_TEMP_LoadTestData();

	_actor_index = GetIndexOfFirstIdleCharacter();
	// TODO: From the average level of the party, level up all enemies passed in
} // BattleMode::BattleMode()



BattleMode::~BattleMode() {
	// Don't let current_battle keep pointing to this object instance any longer
	if (current_battle == this) {
		current_battle = NULL;
	}

	for (uint32 i = 0; i < _battle_music.size(); i++)
		_battle_music[i].FreeMusic();

	for (uint32 i = 0; i < _battle_sound.size(); i++)
		_battle_sound[i].FreeSound();

	// Delete all character and enemy actors
	for (deque<CharacterActor*>::iterator i = _character_actors.begin(); i != _character_actors.end(); ++i) {
		delete *i;
	}
	_character_actors.clear();
	for (deque<EnemyActor*>::iterator i = _enemy_actors.begin(); i != _enemy_actors.end(); ++i) {
		delete *i;
	}
	_enemy_actors.clear();

	// Remove all of the battle images that were loaded
	VideoManager->DeleteImage(_battle_background);
	VideoManager->DeleteImage(_bottom_menu_image);
	VideoManager->DeleteImage(_actor_selection_image);
	VideoManager->DeleteImage(_attack_point_indicator);
	VideoManager->DeleteImage(_swap_icon);
	VideoManager->DeleteImage(_swap_card);

	// Delete all GUI objects that are allocated
	if (_action_list_menu) {
		delete _action_list_menu;
		_action_list_menu = 0;
	}
	if (_action_menu_window) {
		_action_menu_window->Destroy();
		delete _action_menu_window;
		_action_menu_window = 0;
	}
} // BattleMode::~BattleMode()



void BattleMode::Reset() {
	current_battle = this;
	VideoManager->SetCoordSys(0.0f, static_cast<float>(SCREEN_LENGTH * TILE_SIZE),
		0.0f, static_cast<float>(SCREEN_HEIGHT * TILE_SIZE));
	VideoManager->SetFont("battle");
	if (_battle_music[0].IsPlaying() == false) {
		_battle_music[0].PlayMusic();
	}
}


void BattleMode::_TEMP_LoadTestData() {
	// Load all background images
	_battle_background.SetFilename("img/backdrops/battle/desert_cave.png");
	_battle_background.SetDimensions(SCREEN_LENGTH * TILE_SIZE, SCREEN_HEIGHT * TILE_SIZE);
	if (!VideoManager->LoadImage(_battle_background)) {
		cerr << "BATTLE ERROR: Failed to load background image: " << endl;
		_ShutDown();
	}

	_bottom_menu_image.SetFilename("img/menus/battle_bottom_menu.png");
	_bottom_menu_image.SetDimensions(1024, 128);
	if (!VideoManager->LoadImage(_bottom_menu_image)) {
		cerr << "BATTLE ERROR: Failed to load bottom menu image: " << endl;
		_ShutDown();
	}

	_swap_icon.SetFilename("img/icons/battle/swap_icon.png");
	_swap_icon.SetDimensions(35, 30);
	if (!VideoManager->LoadImage(_swap_icon)) {
		cerr << "BATTLE ERROR: Failed to load swap icon: " << endl;
		_ShutDown();
	}
	
	_swap_card.SetFilename("img/icons/battle/swap_card.png");
	_swap_card.SetDimensions(25, 37);
	if (!VideoManager->LoadImage(_swap_card)) {
		cerr << "BATTLE ERROR: Failed to load swap card: " << endl;
		_ShutDown();
	}

	// Load the battle music track
	MusicDescriptor MD;
	MD.LoadMusic("mus/Confrontation.ogg");
	_battle_music.push_back(MD);

	// Construct all battle actors
	_CreateCharacterActors();
	_CreateEnemyActors();
}



void BattleMode::_CreateCharacterActors() {
	_character_actors.clear();

	if (GlobalManager->GetCharacter(GLOBAL_CLAUDIUS) == NULL) {
		cerr << "BATTLE ERROR: could not retrieve Claudius character" << endl;
		_ShutDown();
	}
	else {
		CharacterActor *claudius = new CharacterActor(GlobalManager->GetCharacter(GLOBAL_CLAUDIUS), 256, 320);
		_character_actors.push_back(claudius);
		_selected_character = claudius;
		_actor_index = IndexLocationOfPlayerCharacter(claudius);
	}
} // void BattleMode::_CreateCharacterActors()



void BattleMode::_CreateEnemyActors() {
	StillImage frame; // used for populating the sprite_frames vector
	vector<StillImage> sprite_frames; // A vector to fill it with all four damage frames for each sprite
	EnemyActor* enemy; // A pointer to the new enemy actor to add to the battle

	while (_enemy_actors.size() <= 0)
	{
		// Create the Green Slime EnemyActor
		if (Probability(50))
		{
			sprite_frames.clear();
			enemy = NULL;

			frame.SetDimensions(64, 64);
			frame.SetFilename("img/sprites/battle/enemies/green_slime.png");
			sprite_frames.push_back(frame);
			frame.SetFilename("img/sprites/battle/enemies/green_slime_hp66.png");
			sprite_frames.push_back(frame);
			frame.SetFilename("img/sprites/battle/enemies/green_slime_hp33.png");
			sprite_frames.push_back(frame);
			frame.SetFilename("img/sprites/battle/enemies/green_slime_hp00.png");
			sprite_frames.push_back(frame);

			VideoManager->BeginImageLoadBatch();
			for (uint32 i = 0; i < sprite_frames.size(); i++) {
				if (!VideoManager->LoadImage(sprite_frames[i])) {
					cerr << "BATTLE ERROR: Failed to load sprite frame: " << endl;
					_ShutDown();
				}
			}
			VideoManager->EndImageLoadBatch();

			GlobalEnemy green_slime("slime");
			green_slime.SetName(MakeUnicodeString("Green Slime"));
			green_slime.AddAnimation("IDLE", sprite_frames);
			green_slime.LevelSimulator(2);
			enemy = new EnemyActor(green_slime, 768, 256);
			_enemy_actors.push_back(enemy);
		}

		// Create the Spider EnemyActor
		if (Probability(50))
		{
			sprite_frames.clear();
			enemy = NULL;
			frame.SetDimensions(64, 64);
			frame.SetFilename("img/sprites/battle/enemies/spider.png");
			sprite_frames.push_back(frame);
			frame.SetFilename("img/sprites/battle/enemies/spider_hp66.png");
			sprite_frames.push_back(frame);
			frame.SetFilename("img/sprites/battle/enemies/spider_hp33.png");
			sprite_frames.push_back(frame);
			frame.SetFilename("img/sprites/battle/enemies/spider_hp00.png");
			sprite_frames.push_back(frame);

			VideoManager->BeginImageLoadBatch();
			for (uint32 i = 0; i < sprite_frames.size(); i++) {
				if (!VideoManager->LoadImage(sprite_frames[i])) {
					cerr << "BATTLE ERROR: Failed to load sprite image: " << endl;
					_ShutDown();
				}
			}
			VideoManager->EndImageLoadBatch();

			GlobalEnemy spider("spider");
			spider.SetName(MakeUnicodeString("Spider"));
			spider.AddAnimation("IDLE", sprite_frames);
			spider.LevelSimulator(2);
			enemy = new EnemyActor(spider, 512, 320);
			_enemy_actors.push_back(enemy);
		}

		// Create the Snake EnemyActor
		if (Probability(50))
		{	
			sprite_frames.clear();
			enemy = NULL;
			frame.SetDimensions(128, 64);
			frame.SetFilename("img/sprites/battle/enemies/snake.png");
			sprite_frames.push_back(frame);
			frame.SetFilename("img/sprites/battle/enemies/snake_hp66.png");
			sprite_frames.push_back(frame);
			frame.SetFilename("img/sprites/battle/enemies/snake_hp33.png");
			sprite_frames.push_back(frame);
			frame.SetFilename("img/sprites/battle/enemies/snake_hp00.png");
			sprite_frames.push_back(frame);

			VideoManager->BeginImageLoadBatch();
			for (uint32 i = 0; i < sprite_frames.size(); i++) {
				if (!VideoManager->LoadImage(sprite_frames[i])) {
					cerr << "BATTTLE ERROR: Failed to load snake sprite frame: " << endl;
					_ShutDown();
				}
			}
			VideoManager->EndImageLoadBatch();

			GlobalEnemy snake("snake");
			snake.SetName(MakeUnicodeString("Snake"));
			snake.AddAnimation("IDLE", sprite_frames);
			snake.LevelSimulator(2);
			enemy = new EnemyActor(snake, 576, 192);
			_enemy_actors.push_back(enemy);
		}

		// Create the Skeleton EnemyActor
		if (Probability(50))
		{
			sprite_frames.clear();
			enemy = NULL;
			frame.SetDimensions(64, 128);
			frame.SetFilename("img/sprites/battle/enemies/skeleton.png");
			sprite_frames.push_back(frame);
			frame.SetFilename("img/sprites/battle/enemies/skeleton_hp66.png");
			sprite_frames.push_back(frame);
			frame.SetFilename("img/sprites/battle/enemies/skeleton_hp33.png");
			sprite_frames.push_back(frame);
			frame.SetFilename("img/sprites/battle/enemies/skeleton_hp00.png");
			sprite_frames.push_back(frame);

			VideoManager->BeginImageLoadBatch();
			for (uint32 i = 0; i < sprite_frames.size(); i++) {
				if (!VideoManager->LoadImage(sprite_frames[i])) {
					cerr << "BATTLE ERROR: failed to load skeleton sprite frame: " << endl;
					_ShutDown();
				}
			}
			VideoManager->EndImageLoadBatch();

			GlobalEnemy skeleton("skeleton");
			skeleton.SetName(MakeUnicodeString("Skeleton"));
			skeleton.AddAnimation("IDLE", sprite_frames);
			skeleton.LevelSimulator(2);
			enemy = new EnemyActor(skeleton, 704, 384);
			_enemy_actors.push_back(enemy);
		}
	}
} // void BattleMode::_CreateEnemyActors()



void BattleMode::_ShutDown() {
	if (BATTLE_DEBUG) cout << "BATTLE: ShutDown() called!" << endl;

	_battle_music[0].StopMusic();

	// This call will clear the input state
	InputManager->EventHandler();
	
	// Remove this BattleMode instance from the game stack
	ModeManager->Pop(); 
} // void BattleMode::_ShutDown()

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- Update Code
////////////////////////////////////////////////////////////////////////////////

void BattleMode::Update() {
	_battle_over = (_NumberEnemiesAlive() == 0) || (_NumberCharactersAlive() == 0);
	
	if (_battle_over) {
		_victorious_battle = (_NumberEnemiesAlive() == 0);
		if (_victorious_battle) {
			if (InputManager->ConfirmPress()) {
				PlayerVictory();
			}
		}
		else { 
			_battle_lose_menu.Update(SystemManager->GetUpdateTime()); // Update lose menu
			if (InputManager->ConfirmRelease()) {
				// _battle_lose_menu.HandleConfirmKey(); // This needs to be handled when there's more than 1 option				
				PlayerDefeat();
			}
		}
		// Do not update other battle components when the battle has already ended
		return;
	}

	// Update all battle actors
	for (uint8 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->Update();
	}
	for (uint8 i = 0; i < _enemy_actors.size(); i++) {
		_enemy_actors[i]->Update();
	}

	// Run any scripts that are sitting in the queue
	if (!_IsPerformingScript() && _script_queue.size() > 0) {
		_script_queue.front().RunScript();
		SetPerformingScript(true);
	}

	// Update various menus and other GUI graphics as appropriate
	if (_cursor_state == CURSOR_SELECT_ACTION_TYPE) {
		_action_type_menu.Update(SystemManager->GetUpdateTime());
	}
	if (_action_list_menu && _cursor_state == CURSOR_SELECT_ACTION_LIST) {
		_action_list_menu->Update(SystemManager->GetUpdateTime());
	}
	if (_cursor_state == CURSOR_SELECT_ATTACK_POINT) {
		_attack_point_indicator.Update();
	}

	// Process user input depending upon which state the menu cursor is in
	switch (_cursor_state) {
		case CURSOR_IDLE:
			_UpdateCharacterSelection();
			break;
		case CURSOR_SELECT_ACTION_TYPE:
			_UpdateActionTypeMenu();
			break;
		case CURSOR_SELECT_ACTION_LIST:
			_UpdateActionListMenu();
			break;
		case CURSOR_SELECT_TARGET:
			_UpdateTargetSelection();
			break;
		case CURSOR_SELECT_ATTACK_POINT:
			_UpdateAttackPointSelection();
			break;
	} // switch (_cursor_state)
} // void BattleMode::Update()



void BattleMode::_UpdateCharacterSelection() {
	// NOTE: Comment needed here, when would this situation occur and why do we need to return?
	if (_actor_index == -1) {
		_actor_index = GetIndexOfFirstIdleCharacter();
		return;
	}
	// Return if the player does not have more than one character so select
	if (_NumberCharactersAlive() == 1) {
		_cursor_state = CURSOR_SELECT_ACTION_TYPE;
		_action_menu_window->Show();
		return;
	}

	// Handle user input commands: up, down, left, right, confirm
	if (InputManager->UpPress() || InputManager->RightPress()) {
		// Select the next character above the currently selected one
		// If no such character exists, the selected character will remain selected
		uint32 working_index = _actor_index;
		while (working_index < GetNumberOfCharacterActors()) {
			if (GetPlayerCharacterAt((working_index + 1))->IsAlive()) {
				_actor_index = working_index + 1;
				break;
			}
			else {
				++working_index;
			}
		}
	}
	else if (InputManager->DownPress() || InputManager->LeftPress()) {
		// Select the next character below the currently selected one.
		// If no such character exists, the selected character will remain selected
		uint32 working_index = _actor_index;
		while (working_index > 0) {
			if (GetPlayerCharacterAt((working_index - 1))->IsAlive()) {
				_actor_index = working_index - 1;
				break;
			}
			else {
				--working_index;
			}
		}
	}
	else if (InputManager->ConfirmPress()) {
		_selected_character = GetPlayerCharacterAt(_actor_index);
		_cursor_state = CURSOR_SELECT_ACTION_TYPE;
		_action_menu_window->Show();
	}
} // void BattleMode::_UpdateCharacterSelection()



void BattleMode::_UpdateActionTypeMenu() {
	// Handle user input commands: up, down, confirm, cancel
	if (InputManager->UpPress()) {
		if (_action_type_menu_cursor_location > 0) {
				_action_type_menu.HandleUpKey();
				_action_type_menu_cursor_location--;
		}
	}
	else if (InputManager->DownPress()) {
		if (_action_type_menu_cursor_location < 3) {
			_action_type_menu.HandleDownKey();
			_action_type_menu_cursor_location++;
		}
	}
	else if (InputManager->ConfirmPress()) {
		// Construct the action list menu for the action selected
		_cursor_state = CURSOR_SELECT_ACTION_LIST;
		_ConstructActionListMenu();
	}
	else if (InputManager->CancelPress()) {
		// Only return to selecting characters if there is more than one character
		if (_NumberCharactersAlive() > 1) {
			_actor_index = GetIndexOfFirstIdleCharacter();
			_cursor_state = CURSOR_IDLE;
			_action_menu_window->Hide();
		}
	}
} // void BattleMode::_UpdateActionTypeMenu()




void BattleMode::_UpdateActionListMenu() {
	if (InputManager->DownPress()) {
		_action_list_menu->HandleDownKey();
	}
	else if (InputManager->UpPress()) {
		_action_list_menu->HandleUpKey();
	}
	else if (InputManager->ConfirmPress()) {
		// TEMP: only allows to select one target
		_necessary_selections = 1;
		_argument_actor_index = GetIndexOfFirstAliveEnemy();
		_selected_enemy = GetEnemyActorAt(_argument_actor_index);

		// TODO: retrieve the selected skill, place the cursor on either characters or enemies,
		// depending on whom that skill should target by default

		_cursor_state = CURSOR_SELECT_TARGET;
	}
	else if (InputManager->CancelPress()) {
		_cursor_state = CURSOR_SELECT_ACTION_TYPE;
	}
} // void BattleMode::_UpdateActionListMenu()



void BattleMode::_UpdateTargetSelection() {
	if (InputManager->DownPress() || InputManager->LeftPress()) {
		// Select the character "to the top"
		int32 working_index = _argument_actor_index;
		while (working_index > 0) {
			if (GetEnemyActorAt((working_index - 1))->IsAlive()) {
				_argument_actor_index = working_index - 1;
				_selected_enemy = GetEnemyActorAt(_argument_actor_index);
				break;
			}
			else {
				--working_index;
			}
		}
	}
	else if (InputManager->UpPress() || InputManager->RightPress()) {
		// Select the character "to the bottom"
		uint32 working_index = _argument_actor_index;
		while (working_index < GetNumberOfEnemyActors() - 1) {
			if (GetEnemyActorAt((working_index + 1))->IsAlive()) {
				_argument_actor_index = working_index + 1;
				_selected_enemy = GetEnemyActorAt(_argument_actor_index);
				break;
			}
			else {
				++working_index;
			}
		}
	}
	else if (InputManager->ConfirmPress()) {
		_cursor_state = CURSOR_SELECT_ATTACK_POINT;
		// TODO: Implement cursor memory for attack points here
		_attack_point_selected = 0;
	}
	else if (InputManager->CancelPress()) {
		_cursor_state = CURSOR_SELECT_ACTION_LIST;
	}
} // void BattleMode::_UpdateTargetSelection()



void BattleMode::_UpdateAttackPointSelection() {
	EnemyActor *e = GetEnemyActorAt(_argument_actor_index);
	vector<GlobalAttackPoint*>global_attack_points = e->GetAttackPoints();

	if (InputManager->ConfirmPress()) {
		_selected_actor_arguments.push_back(dynamic_cast<BattleActor*>(GetEnemyActorAt(_argument_actor_index)));
		if (_selected_actor_arguments.size() == _necessary_selections) {
			AddScriptEventToQueue(ScriptEvent(_selected_character, _selected_actor_arguments, "sword_swipe"));
			_selected_character->SetQueuedToPerform(true);
			_selected_actor_arguments.clear();
			_selected_enemy = NULL;

			_actor_index = GetIndexOfFirstIdleCharacter();
			_cursor_state = CURSOR_IDLE;
			_action_menu_window->Hide();
		}
		else {
			_cursor_state = CURSOR_SELECT_TARGET;
		}
	}
	else if (InputManager->UpPress() || InputManager->RightPress()) {
		if (_attack_point_selected < global_attack_points.size() - 1) {
			_attack_point_selected++;
		}
		else if (_attack_point_selected == global_attack_points.size() - 1) {
			_attack_point_selected = 0;
		}
	}
	else if (InputManager->DownPress() || InputManager->LeftPress()) {
		if (_attack_point_selected > 0) {
			_attack_point_selected--;
		}
		else if (_attack_point_selected == 0) {
			_attack_point_selected = global_attack_points.size() - 1;
		}
	}
	else if (InputManager->CancelPress()) {
		_cursor_state = CURSOR_SELECT_TARGET;
	}
} // void BattleMode::_UpdateAttackPointSelection()

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- Draw Code
////////////////////////////////////////////////////////////////////////////////

void BattleMode::Draw() {
	// Apply scene lighting if the battle has finished
	if (_battle_over) {
		if (_victorious_battle) {
			VideoManager->EnableSceneLighting(Color(0.914f, 0.753f, 0.106f, 0.5f)); // Golden color for victory
		}
		else {
			VideoManager->EnableSceneLighting(Color(1.0f, 0.0f, 0.0f, 0.5f)); // Red color for defeat
		}
	}

	_DrawBackgroundVisuals();
	_DrawSprites();
	_DrawBottomMenu();
	_DrawActionMenu();
	_DrawDialogueMenu();
	
	if (_battle_over) {
		VideoManager->DisableSceneLighting();
		// Draw a victory screen along with the loot. TODO: Maybe do this in a separate function
		if (_victorious_battle) {
			VideoManager->Move(520.0f, 384.0f);
			VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
			VideoManager->SetTextColor(Color::white);
			VideoManager->DrawText("Your party is victorious!\n\nExp: +50\n\nLoot : 1 HP Potion");
		}
		// Show the lose screen
		else {
				_battle_lose_menu.Draw();
				VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
				VideoManager->Move(520.0f, 430.0f);
				VideoManager->DrawText("Your party has been defeated!");
		}
	} // endif (_battle_over)
} // void BattleMode::Draw()



void BattleMode::_DrawBackgroundVisuals() {
	// Draw the full-screen, static background image
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_NO_BLEND, 0);
	VideoManager->Move(0, 0);
	VideoManager->DrawImage(_battle_background);

	// TODO: Draw other background objects and animations
} // void BattleMode::_DrawBackgroundVisuals()



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
} // void BattleMode::_DrawSprites()



void BattleMode::_DrawBottomMenu() {
	// Draw the static image for the lower menu
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(0, 0);
	VideoManager->DrawImage(_bottom_menu_image);

	// Draw the swap icon and any swap cards
	VideoManager->Move(6, 16);
	VideoManager->DrawImage(_swap_icon, Color::gray);
	VideoManager->Move(6, 68);
	for (uint8 i = 0; i < _current_number_swaps; i++) {
		VideoManager->DrawImage(_swap_card);
		VideoManager->MoveRelative(4, -4);
	}
	
	// Draw the selected character's portrait, blended according to the character's current HP level
	_selected_character->DrawPortrait();

	// Draw the status information of all character actors
	for (uint8 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->DrawStatus();
	}

	// Draw the status information of the selected enemy
	if (_selected_enemy != NULL) {
		_selected_enemy->DrawStatus();
	}
} // void BattleMode::_DrawBottomMenu()



void BattleMode::_DrawActionMenu() {
	// If the battle is over, none of these menus need to be drawn
	if (_battle_over) {
		return;
	}

	// Draw the action menu window
	if (_cursor_state != CURSOR_IDLE && _action_menu_window) {
		_action_menu_window->Draw();
	}

	// Draw the action type menu
	if (_cursor_state == CURSOR_SELECT_ACTION_TYPE) {
		_action_type_menu.Draw();
	}

	// Draw the action list menu
	if (_cursor_state >= CURSOR_SELECT_ACTION_LIST) {
		_action_list_menu->Draw();
	}
} // void BattleMode::_DrawActionMenu()


// TODO: This feature is not yet supported
void BattleMode::_DrawDialogueMenu() {
	return;
}

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- Miscellaneous Code
////////////////////////////////////////////////////////////////////////////////

const uint8 BattleMode::_NumberEnemiesAlive() const {
	uint8 enemy_count = 0;
	for (uint8 i = 0; i < _enemy_actors.size(); i++) {
		if (_enemy_actors[i]->IsAlive()) {
			enemy_count++;
		}
	}
	return enemy_count;
}



const uint8 BattleMode::_NumberCharactersAlive() const {
	uint8 character_count = 0;
	for (uint8 i = 0; i < _character_actors.size(); i++) {
		if (_character_actors[i]->IsAlive()) {
			character_count++;
		}
	}
	return character_count;
}



void BattleMode::_ConstructActionListMenu() {
	CharacterActor *p = GetPlayerCharacterAt(_actor_index);
	// If an old submenu exists, delete it
	if (_action_list_menu) {
		delete _action_list_menu;
	}
// 	if (_action_menu_window) {
// 		_action_menu_window->Destroy();
// 		delete _action_menu_window;
// 	}

	_action_list_menu = new OptionBox();
	_action_list_menu->SetPosition(10.0f, 512.0f);
	_action_list_menu->SetFont("battle");
	_action_list_menu->SetAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_action_list_menu->SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_action_list_menu->SetSelectMode(VIDEO_SELECT_SINGLE);
	_action_list_menu->SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_action_list_menu->SetCellSize(200.0f, 60.0f);
	_action_list_menu->SetCursorOffset(-20.0f, 25.0f);

	if (_action_type_menu_cursor_location == ACTION_TYPE_ATTACK) {
		vector<GlobalSkill*> attack_skills = p->GetAttackSkills();
		if (attack_skills.size() <= 0) {
			_cursor_state = CURSOR_SELECT_ACTION_TYPE;
		}
		else {
			vector<ustring> attack_skill_names;
			for (uint32 i = 0; i < attack_skills.size(); ++i) {
				string skill_string = attack_skills[i]->GetName() + string("  ") + hoa_utils::NumberToString(attack_skills[i]->GetSPUsage());
				attack_skill_names.push_back(MakeUnicodeString(skill_string));
			}
			_action_list_menu->SetSize(1, attack_skill_names.size());
			_action_list_menu->SetOptions(attack_skill_names);
			_action_list_menu->SetSelection(0);

// 			_action_menu_window = new MenuWindow();
// 			_action_menu_window->Create(200.0f, 20.0f + 50.0f * attack_skill_names.size());
// 			_action_menu_window->SetPosition(0.0f, 600.0f);
// 			_action_menu_window->Show();
		}
	} // if (_action_type_menu_cursor_location == ACTION_TYPE_ATTACK)
	
	else if (_action_type_menu_cursor_location == ACTION_TYPE_DEFEND) {
		vector <GlobalSkill*> defense_skills = p->GetDefenseSkills();
		if (defense_skills.size() <= 0) {
			_cursor_state = CURSOR_SELECT_ACTION_TYPE;
		}
		else {
			vector<ustring> defense_skill_names;
			for (uint32 i = 0; i < defense_skills.size(); ++i) {
				ostringstream sp_usage;
				sp_usage << defense_skills[i]->GetSPUsage();
				string skill_string = defense_skills[i]->GetName() + string("     ") + sp_usage.str();
				defense_skill_names.push_back(MakeUnicodeString(skill_string));
			}
			_action_list_menu->SetOptions(defense_skill_names);
			_action_list_menu->SetSize(1, defense_skill_names.size());


// 			_action_menu_window = new MenuWindow();
// 			_action_menu_window->Create(200.0f, 20.0f + 50.0f * defense_skill_names.size());
// 			_action_menu_window->SetPosition(0.0f, 600.0f);
// 			_action_menu_window->Show();
		}
	} // else if (_action_type_menu_cursor_location == ACTION_TYPE_DEFEND)
	
	else if (_action_type_menu_cursor_location == ACTION_TYPE_SUPPORT) {
		vector<GlobalSkill*> support_skills = p->GetSupportSkills();
		if (support_skills.size() <= 0) {
			_cursor_state = CURSOR_SELECT_ACTION_TYPE;
		}
		else {
			vector<ustring> support_skill_names;
			for (uint32 i = 0; i < support_skills.size(); ++i) {
				ostringstream sp_usage;
				sp_usage << support_skills[i]->GetSPUsage();
				string skill_string = support_skills[i]->GetName() + string("     ") + sp_usage.str();
				support_skill_names.push_back(MakeUnicodeString(skill_string));
			}
	
			_action_list_menu->SetOptions(support_skill_names);
			_action_list_menu->SetSize(1, support_skill_names.size());

// 			_action_menu_window = new MenuWindow();
// 			_action_menu_window->Create(200.0f, 20.0f + 50.0f * support_skill_names.size());
// 			_action_menu_window->SetPosition(0.0f, 600.0f);
// 			_action_menu_window->Show();
		}
	} // else if (_action_type_menu_cursor_location == ACTION_TYPE_SUPPORT)
	
	else if (_action_type_menu_cursor_location == ACTION_TYPE_ITEM) {
		vector<GlobalObject*> inv = GlobalManager->GetInventory();

		// Calculate the number of rows, this is dividing by 6, and if there is a remainder > 0, add one more row for the remainder
		_action_list_menu->SetSize(6, inv.size() / 6 + ((inv.size() % 6) > 0 ? 1 : 0));
		vector<ustring> inv_names;

		for (uint32 i = 0; i < inv.size(); ++i) {
			// Create the item text
			ostringstream os_obj_count;
			os_obj_count << inv[i]->GetCount();
			string inv_item_str = string("<") + inv[i]->GetIconPath() + string("><32>") + inv[i]->GetName()
				+ string("<R>") + string("    ") + os_obj_count.str();
			inv_names.push_back(MakeUnicodeString(inv_item_str));
		}

		_action_list_menu->SetOptions(inv_names);
		_action_list_menu->SetSize(1, inv_names.size());

// 		_action_menu_window = new MenuWindow();
// 		_action_menu_window->Create(200.0f, 20.0f + 50.0f * inv_names.size());
// 		_action_menu_window->SetPosition(0.0f, 600.0f);
// 		_action_menu_window->Show();
	} // else if (_action_type_menu_cursor_location == ACTION_TYPE_ITEM)

	else {
		cerr << "BATTLE ERROR: Unknown action type number: " << _action_type_menu_cursor_location << endl;
		cerr << "> Exiting game" << endl;
		exit(1);
	}
} // void BattleMode::_ConstructActionListMenu()

//! Sets T/F whether an action is being performed
void BattleMode::SetPerformingScript(bool AIsPerforming) {
	
	// Check if a script has just ended. Set the script to stop performing and pop the script from the front of the queue
	if (AIsPerforming == false && _performing_script == true) {
		_script_queue.front().GetSource()->SetQueuedToPerform(false);
		_script_queue.pop_front();
	}

	_performing_script = AIsPerforming;
}



void BattleMode::RemoveScriptedEventsForActor(BattleActor *AActorToRemove) {
	std::list<private_battle::ScriptEvent>::iterator it = _script_queue.begin();

	while (it != _script_queue.end()) {
		if ((*it).GetSource() == AActorToRemove) {
			it = _script_queue.erase(it);	//remove this location
		}
		else {
			//otherwise, increment the iterator
			it++;
		}
	}
}



void BattleMode::PlayerVictory() {
	if (BATTLE_DEBUG) cout << "Player has won a battle!" << endl;

	// Give player some loot
	GlobalManager->AddItemToInventory(HP_POTION);

	// Give some experience as well
	GlobalCharacter *claudius = GlobalManager->GetCharacter(GLOBAL_CLAUDIUS);
	if (claudius != 0) {
		claudius->AddXP(50);
	}

	VideoManager->DisableFog();
	_ShutDown();
}



void BattleMode::PlayerDefeat() {
	if (BATTLE_DEBUG) cout << "Player was defeated in a battle!" << endl;
	_ShutDown();
	ModeManager->PopAll();
	BootMode *BM = new BootMode();
	ModeManager->Push(BM);
}



void BattleMode::SwapCharacters(private_battle::CharacterActor *AActorToRemove, private_battle::CharacterActor *AActorToAdd) {
	//put AActorToAdd at AActorToRemove's location
	for (std::deque < private_battle::CharacterActor * >::iterator it = _character_actors.begin(); it != _character_actors.end(); it++) {
		if (*it == AActorToRemove) {
			_character_actors.erase(it);
			break;
		}
	}

	//set location and origin to removing characters location and origin
	AActorToAdd->SetXOrigin(AActorToRemove->GetXOrigin());
	AActorToAdd->SetYOrigin(AActorToRemove->GetYOrigin());
	AActorToAdd->SetXLocation(static_cast<float>(AActorToRemove->GetXOrigin()));
	AActorToAdd->SetYLocation(static_cast<float>(AActorToRemove->GetYOrigin()));

	_character_actors.push_back(AActorToAdd);	//add the other character to battle
}



int32 BattleMode::GetIndexOfFirstAliveEnemy() {

	std::deque<private_battle::EnemyActor*>::iterator it = _enemy_actors.begin();
	for (uint32 i = 0; it != _enemy_actors.end(); i++, it++) {
		if ((*it)->IsAlive()) {
			return i;
		}
	}

	return -1;
}



int32 BattleMode::GetIndexOfFirstIdleCharacter() {
	int32 index = -1;

	deque<CharacterActor*>::iterator it = _character_actors.begin();
	for (uint32 i = 0; it != _character_actors.end(); i++, it++) {
		if (!(*it)->IsQueuedToPerform() && (*it)->IsAlive()) {
			index = i;
			break;
		}
	}

		return index;
}



int32 BattleMode::IndexLocationOfPlayerCharacter(private_battle::CharacterActor* const AActor) {
	int32 index = 0;
	deque<CharacterActor*>::iterator it = _character_actors.begin();
	for (; it != _character_actors.end(); it++) {
		if (*it == AActor) {
			return index;
		} else {
			index++;
		}
	}
	return -1;
}

} // namespace hoa_battle
