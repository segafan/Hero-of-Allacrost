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
#include "settings.h"
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
using namespace hoa_settings;
using namespace hoa_global;
using namespace hoa_data;

using namespace hoa_battle::private_battle;
using namespace hoa_boot;

namespace hoa_battle {

bool BATTLE_DEBUG = false;

namespace private_battle {

// *****************************************************************************
// BattleUI class
// *****************************************************************************

BattleUI::BattleUI(BattleMode* const ABattleMode) :
    _battle_mode(ABattleMode),
    _currently_selected_player_actor(NULL),
    _necessary_selections(0),
    _current_hover_selection(0),
    _current_map_selection(0),
    _number_menu_items(0),
    _cursor_state(CURSOR_ON_PLAYER_CHARACTERS),
    _sub_menu(NULL),
    _sub_menu_window(NULL)
{
	_actor_index = _battle_mode->GetIndexOfFirstIdleCharacter();

	std::vector < hoa_video::StillImage > attack_point_indicator;
	StillImage frame;
	frame.SetDimensions(16, 16);
	frame.SetFilename("img/icons/battle/indicator_1.png");
	attack_point_indicator.push_back(frame);
	frame.SetFilename("img/icons/battle/indicator_2.png");
	attack_point_indicator.push_back(frame);
	frame.SetFilename("img/icons/battle/indicator_3.png");
	attack_point_indicator.push_back(frame);
	frame.SetFilename("img/icons/battle/indicator_4.png");
	attack_point_indicator.push_back(frame);

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < attack_point_indicator.size(); i++) {
		if (!VideoManager->LoadImage(attack_point_indicator[i]))
			cerr << "BATTLE ERROR: Failed to load MAPS indicator." << endl;
	}
	VideoManager->EndImageLoadBatch();

	for (uint32 i = 0; i < attack_point_indicator.size(); i++) {
		_MAPS_indicator.AddFrame(attack_point_indicator[i], 10);
	}

	_general_menu.SetCellSize(50.0f, 79.0f);
	_general_menu.SetSize(5, 1);
	_general_menu.SetPosition(0.0f, 620.0f);
	_general_menu.SetFont("battle");
	_general_menu.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_general_menu.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_general_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	_general_menu.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);

	vector<ustring> formatText;
	formatText.push_back(MakeWideString("<img/icons/battle/icon_attack.png>"));
	formatText.push_back(MakeWideString("<img/icons/battle/icon_defend.png>"));
	formatText.push_back(MakeWideString("<img/icons/battle/icon_support.png>"));
	formatText.push_back(MakeWideString("<img/icons/battle/icon_item.png>"));
	formatText.push_back(MakeWideString("<img/icons/battle/icon_extra.png>"));

	_general_menu.SetOptions(formatText);
	_general_menu.SetSelection(0);
	_general_menu_cursor_location = 0;
	_general_menu.EnableOption(4, false);
	_general_menu.SetCursorOffset(-15, 0);

	_battle_lose_menu.SetCellSize(128.0f, 50.0f);
	_battle_lose_menu.SetPosition(530.0f, 380.0f);
	_battle_lose_menu.SetSize(1, 1);
	_battle_lose_menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_battle_lose_menu.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_battle_lose_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	_battle_lose_menu.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_battle_lose_menu.SetCursorOffset(-35.0f, -4.0f);
	vector<ustring> loseText;
	loseText.push_back(MakeWideString("Return to the main menu"));
	_battle_lose_menu.SetOptions(loseText);
	_battle_lose_menu.SetSelection(0);

	_player_selector_image.SetDimensions(109, 78);
	_player_selector_image.SetFilename("img/icons/battle/character_selection.png");
	if (!VideoManager->LoadImage(_player_selector_image)) {
		cerr << "Unable to load player selector image." << endl;
	}
} // // BattleUI::BattleUI()

BattleUI::~BattleUI() {
	//_battle_lose_menu.Destroy();
	//_general_menu.Destroy();

	if (_sub_menu) {
		//_sub_menu->Destroy();
		delete _sub_menu;
	}
	if (_sub_menu_window) {
		_sub_menu_window->Destroy();
		delete _sub_menu_window;
	}
} // BattleUI::~BattleUI()

void BattleUI::SetPlayerActorSelected(PlayerActor * const AWhichActor) {
	_currently_selected_player_actor = AWhichActor;
	_actor_index = _battle_mode->IndexLocationOfPlayerCharacter(AWhichActor);
}

void BattleUI::Draw() {
	if (_battle_mode->IsBattleOver()) {
		// Draw a victory screen along with the loot. TODO: Maybe do this in a separate function
		if (_battle_mode->IsVictorious()) {
			VideoManager->Move(520.0f, 384.0f);
			VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
			VideoManager->DisableFog();
			VideoManager->SetTextColor(Color::white);
			VideoManager->DrawText("You have won the battle!\n\nExp: +50\n\nLoot : 1 HP Potion");
			VideoManager->EnableFog(Color::orange, 0.3f); // golden fog
			VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
		}
		// Show the lose screen
		else {
				VideoManager->DisableFog();
				VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
				VideoManager->Move(520.0f, 430.0f);
				VideoManager->DrawText("You have lost the battle!");
				_battle_lose_menu.Draw();
				VideoManager->EnableFog(Color(0.6f, 0.0f, 0.0f, 1.0f), 0.6f); // blood-red fog
				VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
		}
	  return;
	} // if (_battle_mode->IsBattleOver())

	if (_cursor_state >= CURSOR_ON_SUB_MENU && _sub_menu_window) {
		_sub_menu_window->Draw();
	}
	if (_cursor_state >= CURSOR_ON_SUB_MENU && _sub_menu) {
		_sub_menu->Draw();
	}
	if (_cursor_state > CURSOR_ON_PLAYER_CHARACTERS) {
		_general_menu.Draw();
	}

	// Draw damage information

	// Draw the selector image over the currently selected character
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->Move(_currently_selected_player_actor->GetXLocation() - 20, _currently_selected_player_actor->GetYLocation() - 20);
	_player_selector_image.Draw();

	// Draw the selector image over the currently selected enemy, if any
	if (_cursor_state == CURSOR_SELECT_TARGET) {
		EnemyActor *e = _battle_mode->GetEnemyActorAt(_argument_actor_index);
		VideoManager->Move(e->GetXLocation() - 20, e->GetYLocation() - 20);
		_player_selector_image.Draw();
	}
} // void BattleUI::Draw()

void BattleUI::DrawTopElements() {
	if (_cursor_state == CURSOR_ON_SELECT_MAP) {
		EnemyActor *e = _battle_mode->GetEnemyActorAt(_argument_actor_index);
		std::vector<GlobalAttackPoint*> global_attack_points = e->GetAttackPoints();

		VideoManager->Move(e->GetXLocation() + global_attack_points[_current_map_selection]->GetXPosition(),
			e->GetYLocation() + global_attack_points[_current_map_selection]->GetYPosition());
	  VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	  VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	  VideoManager->DrawImage(_MAPS_indicator);
	  VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	  VideoManager->SetTextColor(Color(0.0f, 0.0f, 1.0f, 1.0f));
		VideoManager->Move(850, 100);
		VideoManager->DrawText(global_attack_points[_current_map_selection]->GetName());
	}
} // void BattleUI::DrawTopElements()

void BattleUI::Update(uint32 AUpdateTime) {
	if (_battle_mode->IsBattleOver()) {
		if (_battle_mode->IsVictorious()) {
			if (InputManager->ConfirmPress()) {
				_battle_mode->PlayerVictory();	// Handle victory
			}
		}
		else { // Battle was lost
			_battle_lose_menu.Update(AUpdateTime); // Update lose menu
			if (InputManager->ConfirmRelease()) {
				//_battle_lose_menu.HandleConfirmKey(); // This needs to be handled when there's more than 1 option
				InputManager->EventHandler(); // Clear input in here because we don't want confirm press in boot mode!
				_battle_mode->PlayerDefeat(); // Handle defeat
			}
		}
	// No need to handle any other menu events
	return;
	} // if (_battle_mode->IsBattleOver())

	if (_cursor_state > CURSOR_ON_PLAYER_CHARACTERS) {
		_general_menu.Update(AUpdateTime);
	}

	if (_sub_menu && _cursor_state == CURSOR_ON_SUB_MENU) {
		_sub_menu->Update(AUpdateTime);
	}

	if (_cursor_state == CURSOR_ON_SELECT_MAP) {
		_MAPS_indicator.Update();
	}

	if (_cursor_state == CURSOR_ON_PLAYER_CHARACTERS) {
		if (_actor_index == -1) {
			_actor_index = _battle_mode->GetIndexOfFirstIdleCharacter();
		}
		else if (_battle_mode->NumberOfPlayerCharactersAlive() == 1) {
			_cursor_state = CURSOR_ON_MENU;
		}
		else if (InputManager->UpPress() || InputManager->RightPress()) {
			// select the character "to the top"
			uint32 working_index = _actor_index;
			while (working_index < _battle_mode->GetNumberOfPlayerCharacters()) {
				if (_battle_mode->GetPlayerCharacterAt((working_index + 1))->IsAlive()) {
					_actor_index = working_index + 1;
					break;
				}
				else {
					++working_index;
				}
			}
		}
		else if (InputManager->DownPress() || InputManager->LeftPress()) {
			// select the character "to the bottom"
			int32 working_index = _actor_index;
			while (working_index > 0) {
				if (_battle_mode-> GetPlayerCharacterAt((working_index - 1))->IsAlive()) {
					_actor_index = working_index - 1;
					break;
				}
				else {
					--working_index;
				}
			}
		}
		else if (InputManager->ConfirmPress()) {
			// Select the current actor
			_currently_selected_player_actor = _battle_mode->GetPlayerCharacterAt(_actor_index);
			_cursor_state = CURSOR_ON_MENU;
		}
	} // if (_cursor_state == CURSOR_ON_PLAYER_CHARACTERS)
	else if (_cursor_state == CURSOR_ON_MENU) {
		if (InputManager->LeftPress()) {
			if (_general_menu_cursor_location > 0) {
					_general_menu.HandleLeftKey();
					_general_menu_cursor_location--;
			}
		}
		else if (InputManager->RightPress()) {
			if (_general_menu_cursor_location < 3) {
				_general_menu.HandleRightKey();
				_general_menu_cursor_location++;
			}
		}
		else if (InputManager->ConfirmPress()) {
			_cursor_state = CURSOR_ON_SUB_MENU;
			PlayerActor *p = _battle_mode->GetPlayerCharacterAt(_actor_index);

			// If an old submenu exists, delete it
			if (_sub_menu) {
				//_sub_menu->Destroy();
				delete _sub_menu;
			}
			if (_sub_menu_window) {
				_sub_menu_window->Destroy();
				delete _sub_menu_window;
			}

			_sub_menu = new OptionBox();
			_sub_menu->SetPosition(50.0f, 550.0f);
			_sub_menu->SetFont("battle");
			_sub_menu->SetAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
			_sub_menu->SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
			_sub_menu->SetSelectMode(VIDEO_SELECT_SINGLE);
			_sub_menu->SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
			_sub_menu->SetCellSize(100.0f, 50.0f);
			_sub_menu->SetCursorOffset(-30, -5);

			switch (_general_menu_cursor_location) {
				case 0: { // Attack
					vector<GlobalSkill*> attack_skills = p->GetAttackSkills();
					if (attack_skills.size() <= 0) {
						_cursor_state = CURSOR_ON_MENU;
						break;
					}
					vector<ustring> attack_skill_names;
					for (uint32 i = 0; i < attack_skills.size(); ++i) {
						ostringstream sp_usage;
						sp_usage << attack_skills[i]->GetSPUsage();
						string skill_string = attack_skills[i]->GetName() + string("     ") + sp_usage.str();
						attack_skill_names.push_back(MakeWideString(skill_string));
					}

					_sub_menu->SetSize(1, attack_skill_names.size());
					_sub_menu->SetOptions(attack_skill_names);
					_sub_menu->SetSelection(0);

					_sub_menu_window = new MenuWindow();
					_sub_menu_window->Create(200.0f, 20.0f + 50.0f * attack_skill_names.size());
					_sub_menu_window->SetPosition(0.0f, 600.0f);
					_sub_menu_window->Show();
					break;
				}
				case 1: { // Defend
					vector <GlobalSkill*> defense_skills = p->GetDefenseSkills();
					if (defense_skills.size() <= 0) {
						_cursor_state = CURSOR_ON_MENU;
						break;
					}

					vector<ustring> defense_skill_names;
					for (uint32 i = 0; i < defense_skills.size(); ++i) {
						ostringstream sp_usage;
						sp_usage << defense_skills[i]->GetSPUsage();
						string skill_string =
						defense_skills[i]->GetName() + string("     ") + sp_usage.str();
						defense_skill_names.push_back(MakeWideString(skill_string));
					}

					_sub_menu->SetOptions(defense_skill_names);
					_sub_menu->SetSize(1, defense_skill_names.size());
					_sub_menu->SetSelection(0);

					_sub_menu_window = new MenuWindow();
					_sub_menu_window->Create(200.0f, 20.0f + 50.0f * defense_skill_names.size());
					_sub_menu_window->SetPosition(0.0f, 600.0f);
					_sub_menu_window->Show();
					break;
				}
				case 2: { // Support
				vector<GlobalSkill*> support_skills = p->GetSupportSkills();
				if (support_skills.size() <= 0) {
					_cursor_state = CURSOR_ON_MENU;
					break;
				}

				vector<ustring> support_skill_names;
				for (uint32 i = 0; i < support_skills.size(); ++i) {
					ostringstream sp_usage;
					sp_usage << support_skills[i]->GetSPUsage();
					string skill_string = support_skills[i]->GetName() + string("     ") + sp_usage.str();
					support_skill_names.push_back(MakeWideString(skill_string));
				}

				_sub_menu->SetOptions(support_skill_names);
				_sub_menu->SetSize(1, support_skill_names.size());
				_sub_menu->SetSelection(0);

				_sub_menu_window = new MenuWindow();
				_sub_menu_window->Create(200.0f, 20.0f + 50.0f * support_skill_names.size());
				_sub_menu_window->SetPosition(0.0f, 600.0f);
				_sub_menu_window->Show();
				break;
				}
				case 3: { // Item
					vector<GlobalObject*> inv = GlobalManager->GetInventory();
					// Set the size of the option box
					// Calculate the number of rows, this is dividing by 6, and if there is a remainder > 0
					// add one more row for the remainder.
					_sub_menu->SetSize(6, inv.size() / 6 + ((inv.size() % 6) > 0 ? 1 : 0));
					vector<ustring> inv_names;

					for (uint32 i = 0; i < inv.size(); ++i) {
						// Create the item text
						ostringstream os_obj_count;
						os_obj_count << inv[i]->GetCount();
						string inv_item_str = string("<") + inv[i]->GetIconPath() + string("><32>") + inv[i]->GetName() + string("<R>") + string("    ") + os_obj_count.str();
						inv_names.push_back(MakeWideString(inv_item_str));
					}

					_sub_menu->SetOptions(inv_names);
					_sub_menu->SetSize(1, inv_names.size());
					_sub_menu->SetSelection(0);

					_sub_menu_window = new MenuWindow();
					_sub_menu_window->Create(200.0f, 20.0f + 50.0f * inv_names.size());
					_sub_menu_window->SetPosition(0.0f, 600.0f);
					_sub_menu_window->Show();
					break;
				}
			} // switch (_general_menu_cursor_location)
		} // else if (InputManager->ConfirmPress())

		else if (InputManager->CancelPress()) {
			if (_battle_mode->NumberOfPlayerCharactersAlive() > 1) {
				_actor_index = _battle_mode->GetIndexOfFirstIdleCharacter();
				_cursor_state = CURSOR_ON_PLAYER_CHARACTERS;
			}
		}
	} // else if (_cursor_state == CURSOR_ON_MENU)

	else if (_cursor_state == CURSOR_ON_SUB_MENU) {
		if (InputManager->DownPress()) {
			_sub_menu->HandleDownKey();
		}
		else if (InputManager->UpPress()) {
			_sub_menu->HandleUpKey();
		}
		else if (InputManager->ConfirmPress()) {
			// TEMP: only allows to select one target
			SetNumberNecessarySelections(1);
			_argument_actor_index = _battle_mode->GetIndexOfFirstAliveEnemy();

			// TODO: retrieve the skill
			// Place the cursor on either characters or enemies, depending on whom the skill should target
			// Place the skill in the battle script queue
			// Exit out of the menu

			_cursor_state = CURSOR_SELECT_TARGET;
		}
		else if (InputManager->CancelPress()) {
			_cursor_state = CURSOR_ON_MENU;
		}
	} // else if (_cursor_state == CURSOR_ON_SUB_MENU)
	else if (_cursor_state == CURSOR_SELECT_TARGET) {
		if (InputManager->DownPress() || InputManager->LeftPress()) {
			// Select the character "to the top"
			int32 working_index = _argument_actor_index;
			while (working_index > 0) {
				if (_battle_mode->GetEnemyActorAt((working_index - 1))->IsAlive()) {
					_argument_actor_index = working_index - 1;
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
			while (working_index < _battle_mode->GetNumberOfEnemyActors() - 1) {
				if (_battle_mode->GetEnemyActorAt((working_index + 1))->IsAlive()) {
					_argument_actor_index = working_index + 1;
					break;
				}
				else {
					++working_index;
				}
			}
		}
		else if (InputManager->ConfirmPress()) {
			_cursor_state = CURSOR_ON_SELECT_MAP;
			_current_map_selection = 0;
		}
		else if (InputManager->CancelPress()) {
			_cursor_state = CURSOR_ON_SUB_MENU;
		}
	} // else if (_cursor_state == CURSOR_SELECT_TARGET)

	else if (_cursor_state == CURSOR_ON_SELECT_MAP) {
		EnemyActor *e = _battle_mode->GetEnemyActorAt(_argument_actor_index);
		vector<GlobalAttackPoint*>global_attack_points = e->GetAttackPoints();

		if (InputManager->ConfirmPress()) {
			SetActorAsArgument(dynamic_cast<BattleActor*>(_battle_mode->GetEnemyActorAt(_argument_actor_index)));
			if (GetSelectedArgumentActors().size() == _necessary_selections) {
				_battle_mode->AddScriptEventToQueue(ScriptEvent(_currently_selected_player_actor, GetSelectedArgumentActors(), "sword_swipe"));
				_currently_selected_player_actor->SetQueuedToPerform(true);
				_currently_selected_argument_actors.clear();

				_actor_index = _battle_mode->GetIndexOfFirstIdleCharacter();
				_cursor_state = CURSOR_ON_PLAYER_CHARACTERS;
			}
			else {
				_cursor_state = CURSOR_SELECT_TARGET;
			}
		}
		else if (InputManager->UpPress() || InputManager->RightPress()) {
			if (_current_map_selection < global_attack_points.size() - 1) {
				_current_map_selection++;
			}
			else if (_current_map_selection == global_attack_points.size() - 1) {
				_current_map_selection = 0;
			}
		}
		else if (InputManager->DownPress() || InputManager->LeftPress()) {
			if (_current_map_selection > 0) {
				_current_map_selection--;
			}
			else if (_current_map_selection == 0) {
				_current_map_selection = global_attack_points.size() - 1;
			}
		}
		else if (InputManager->CancelPress()) {
			_cursor_state = CURSOR_SELECT_TARGET;
		}
	} // else if (_cursor_state == CURSOR_ON_SELECT_MAP)
} // void BattleUI::Update(uint32 AUpdateTime)

// *****************************************************************************
// ScriptEvent class
// *****************************************************************************

ScriptEvent::ScriptEvent(BattleActor* AHost, deque<BattleActor*> AArguments, string AScriptName) :
	_script_name(AScriptName),
	_host(AHost),
	_arguments(AArguments)
{}

ScriptEvent::~ScriptEvent()
{}

void ScriptEvent::RunScript() {
	// TEMP: do basic damage to the actors
	for (uint8 i = 0; i < _arguments.size(); i++) {
		_arguments[i]->TEMP_Deal_Damage(rand() % 20);
	}
	// TODO: get script from global script repository and run, passing in list of arguments and host actor
}

} // namespace private battle

// *****************************************************************************
// BattleMode class
// *****************************************************************************

int32 BattleMode::MAX_PLAYER_CHARACTERS_IN_BATTLE = 4;
int32 BattleMode::MAX_ENEMY_CHARACTERS_IN_BATTLE  = 8;

BattleMode::BattleMode() :
	_user_interface(this),
	_performing_script(false),
	_battle_over(false)
{
	MusicDescriptor MD;
	MD.LoadMusic("Confrontation");
	_battle_music.push_back(MD);

	_TEMP_LoadTestData();

	// TODO: From the average level of the party, level up all enemies passed in
}

BattleMode::~BattleMode() {
	// Delete all player actors
	deque<PlayerActor*>::iterator pc_itr = _player_actors.begin();
	for (; pc_itr != _player_actors.end(); pc_itr++) {
		delete *pc_itr;
	}
	_player_actors.clear();
	_players_characters_in_battle.clear();

	// Delete all enemy actors
	std::deque<private_battle::EnemyActor *>::iterator it = _enemy_actors.begin();
	for (; it != _enemy_actors.end(); it++) {
		delete *it;
	}
	_enemy_actors.clear();

	// Remove all of the battle images that were created (TEMP)
	for (uint32 i = 0; i < _battle_images.size(); i++) {
		VideoManager->DeleteImage(_battle_images[i]);
	}
}

//! Resets appropriate class members. Called whenever BootMode is made the active game mode.
void BattleMode::Reset() {
	//VideoManager->SetCoordSys(0.0f, (float)SCREEN_LENGTH, 0.0f, (float)SCREEN_HEIGHT);
	VideoManager->SetCoordSys(0.0f, static_cast<float>(SCREEN_LENGTH * TILE_SIZE), 0.0f, static_cast<float>(SCREEN_HEIGHT * TILE_SIZE));
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->SetFont("battle");
	_battle_music[0].PlayMusic();
}

//! Wrapper function that calls different update functions depending on the battle state.
void BattleMode::Update() {
	uint32 updateTime = SettingsManager->GetUpdateTime();

	bool defeat = true;

	if (_players_characters_in_battle.size() == 0) {
		defeat = false;
	}

	for (uint8 i = 0; i < _players_characters_in_battle.size(); i++) {
		if (_players_characters_in_battle[i]->IsAlive()) {
			defeat = false;
			break;
		}
	}

	bool victory = true;

	if (_enemy_actors.size() == 0) {
		victory = false;
	}

	for (uint8 i = 0; i < _enemy_actors.size(); i++) {
		if (_enemy_actors[i]->IsAlive()) {
			victory = false;
			break;
		}
	}

	// The battle is over if either victory or defeat is true
	if (victory || defeat) {
		_battle_over = true;
		if (victory) {
			 _victorious_battle = true;
		} else {
			_victorious_battle = false;
		}
	}

	// If the battle is not over, update the actors
	if (!_battle_over) {
		for (uint8 i = 0; i < _players_characters_in_battle.size(); i++) {
			_players_characters_in_battle[i]->Update(updateTime);
		}

		for (uint8 i = 0; i < _enemy_actors.size(); i++) {
			_enemy_actors[i]->Update(updateTime);
			_enemy_actors[i]->DoAI();
		}

		// Run any scripts that are sitting in the queue
		if (!_IsPerformingScript() && _script_queue.size() > 0) {
			_script_queue.front().RunScript();
			SetPerformingScript(true);
		}
	} // if (!_battle_over)

	_user_interface.Update(updateTime);
} // void BattleMode::Update()

//! Wrapper function that calls different draw functions depending on the battle state.
void BattleMode::Draw() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	if (IsBattleOver())
		if (IsVictorious())
			VideoManager->EnableFog(Color::orange, 0.3f); // golden fog
		else
			VideoManager->EnableFog(Color(0.6f, 0.0f, 0.0f, 1.0f), 0.6f); // blood-red fog

	_DrawBackground();
	_user_interface.Draw();
	_DrawCharacters();
	_user_interface.DrawTopElements();
	VideoManager->DisableFog();
}

void BattleMode::_DrawBackground() {
	VideoManager->Move(0, 0);
	VideoManager->SetDrawFlags(VIDEO_NO_BLEND, 0);
	VideoManager->DrawImage(_battle_images[0]);

	//_TEMP
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->DrawImage(_battle_images[1]);
}

void BattleMode::_DrawCharacters() {
	for (uint32 i = 0; i < _players_characters_in_battle.size(); i++) {
		_players_characters_in_battle[i]->Draw();
	}

	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		_enemy_actors[i]->Draw();
	}
}

//! Shutdown the battle mode
void BattleMode::_ShutDown() {
	if (BATTLE_DEBUG) cout << "BATTLE: ShutDown() called!" << endl;
	VideoManager->DisableFog();
	InputManager->EventHandler(); // Clear input
	ModeManager->Pop(); // Pop out the BattleMode state
}

//! Sets T/F whether an action is being performed
void BattleMode::SetPerformingScript(bool AIsPerforming) {
	_performing_script = AIsPerforming;

	//a script has just ended.  Set them as false to perform pop the script from the front
	if (_performing_script == false) {
		_script_queue.front().GetHost()->SetQueuedToPerform(false);
		_script_queue.pop_front();	//get that script out of here!
	}
}

//! Remove all scripted events for an actor
void BattleMode::RemoveScriptedEventsForActor(BattleActor *AActorToRemove) {
	std::list<private_battle::ScriptEvent>::iterator it = _script_queue.begin();

	while (it != _script_queue.end()) {
		if ((*it).GetHost() == AActorToRemove) {
			it = _script_queue.erase(it);	//remove this location
		}
		else {
			//otherwise, increment the iterator
			it++;
		}
	}
}

//! Returns all player actors
std::deque<PlayerActor*> BattleMode::ReturnCharacters() const {
	return _players_characters_in_battle;
}

void BattleMode::PlayerVictory() {
	//stubbed for now ... go back to map mode?  Tell the GUI, show the player, pop the state
	if (BATTLE_DEBUG) cout << "Player has won the battle!" << endl;

	// Give player some loot
	GlobalItem *new_item = new GlobalItem(GLOBAL_HP_RECOVERY_ITEM, GLOBAL_ALL_CHARACTERS, HP_POTION, 1);
	new_item->SetRecoveryAmount(180);
	GlobalManager->AddItemToInventory(new_item);

	// Give some experience as well
	GlobalCharacter *claudius = GlobalManager->GetCharacter(GLOBAL_CLAUDIUS);
	if (claudius != 0) {
		claudius->AddXP(50);
	}

	VideoManager->DisableFog();
	_ShutDown();
}

void BattleMode::PlayerDefeat() {
	if (BATTLE_DEBUG) cout << "Player was defeated in battle!" << endl;
	VideoManager->DisableFog();
	ModeManager->PopAll(); // Pop out battle mode
	BootMode *BM = new BootMode();
	ModeManager->Push(BM);
// 	ModeManager->Pop(); // Pop out map mode
}

void BattleMode::_BuildPlayerCharacters() {
	// from global party, get characters put them into PlayerCharacters make sure the list is clean first
	for (std::deque<PlayerActor*>::iterator pc_itr = _player_actors.begin(); pc_itr != _player_actors.end(); pc_itr++) {
		delete *pc_itr;
	}
	_player_actors.clear();
	_players_characters_in_battle.clear();
}

void BattleMode::_BuildEnemyActors() {
	//the x and y location of the enemies, based on how many enemies there are in the list
// 	int x = 10;			//dummy variables
// 	int y = 10;

	//make sure the list is clean first
	/*
		 std::deque<private_battle::EnemyActor *>::iterator it = _enemy_actors.begin();
		 for(; it != _enemy_actors.end(); it++)
		 delete *it;
		 _enemy_actors.clear();

		 std::deque<hoa_global::GlobalEnemy>::iterator it = _global_enemies.begin();
		 for(; it != _global_enemies.end(); it++)
		 {
		 EnemyActor *enemy = new EnemyActor(*it, this, x, y);
		 _enemy_actors.push_back(enemy);
		 }
	 */
}

void BattleMode::SwapCharacters(private_battle::PlayerActor *AActorToRemove, private_battle::PlayerActor *AActorToAdd) {
	//put AActorToAdd at AActorToRemove's location
	for (std::deque < private_battle::PlayerActor * >::iterator it = _players_characters_in_battle.begin(); it != _players_characters_in_battle.end(); it++) {
		if (*it == AActorToRemove) {
			_players_characters_in_battle.erase(it);
			break;
		}
	}

	//set location and origin to removing characters location and origin
	AActorToAdd->SetXOrigin(AActorToRemove->GetXOrigin());
	AActorToAdd->SetYOrigin(AActorToRemove->GetYOrigin());
	AActorToAdd->SetXLocation(static_cast<float>(AActorToRemove->GetXOrigin()));
	AActorToAdd->SetYLocation(static_cast<float>(AActorToRemove->GetYOrigin()));

	_players_characters_in_battle.push_back(AActorToAdd);	//add the other character to battle
}

uint32 BattleMode::NumberOfPlayerCharactersAlive() {
	int32 numAlive = 0;

	std::deque < private_battle::PlayerActor * >::iterator it = _players_characters_in_battle.begin();
	for (; it != _players_characters_in_battle.end(); it++) {
		if ((*it)->IsAlive()) {
	    numAlive++;
	  }
	}

	return numAlive;
}

int32 BattleMode::GetIndexOfFirstAliveEnemy() {
	int32 index = -1;

	std::deque<private_battle::EnemyActor*>::iterator it = _enemy_actors.begin();
	for (uint32 i = 0; it != _enemy_actors.end(); i++, it++) {
		if ((*it)->IsAlive()) {
			return i;
		}
	}

	return index;
}

int32 BattleMode::GetIndexOfFirstIdleCharacter() {
	int32 index = -1;

	deque<PlayerActor*>::iterator it = _players_characters_in_battle.begin();
	for (uint32 i = 0; it != _players_characters_in_battle.end(); i++, it++) {
		if (!(*it)->IsQueuedToPerform() && (*it)->IsAlive()) {
			index = i;
			break;
		}
	}

		return index;
}

int32 BattleMode::IndexLocationOfPlayerCharacter(private_battle::PlayerActor* const AActor) {
	int32 index = 0;
	deque<PlayerActor*>::iterator it = _players_characters_in_battle.begin();
	for (; it != _players_characters_in_battle.end(); it++) {
		if (*it == AActor) {
			return index;
		} else {
			index++;
		}
	}
	return -1;
}

void BattleMode::_TEMP_LoadTestData() {
	StillImage backgrd;
	StillImage overback;

	backgrd.SetFilename("img/backdrops/battle/battle_cave.png");
	backgrd.SetDimensions(SCREEN_LENGTH * TILE_SIZE, SCREEN_HEIGHT * TILE_SIZE);
	_battle_images.push_back(backgrd);
	if (!VideoManager->LoadImage(_battle_images[0])) {
		cerr << "Failed to load background image." << endl;
		_ShutDown();
	}

	overback.SetFilename("img/menus/battle_bottom_menu.png");
	overback.SetDimensions(1024, 128);
	_battle_images.push_back(overback);
	if (!VideoManager->LoadImage(_battle_images[1])) {
		cerr << "Failed to load background over image." << endl;
		_ShutDown();
	}


	std::vector<hoa_video::StillImage> enemyAnimation;
	StillImage anim;
	anim.SetDimensions(64, 64);
	anim.SetFilename("img/sprites/battle/enemies/spider_d0.png");
	enemyAnimation.push_back(anim);
	anim.SetFilename("img/sprites/battle/enemies/spider_d1.png");
	enemyAnimation.push_back(anim);
	anim.SetFilename("img/sprites/battle/enemies/spider_d2.png");
	enemyAnimation.push_back(anim);
	anim.SetFilename("img/sprites/battle/enemies/spider_d3.png");
	enemyAnimation.push_back(anim);

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < enemyAnimation.size(); i++) {
		if (!VideoManager->LoadImage(enemyAnimation[i])) {
			cerr << "Failed to load spider image." << endl;
			_ShutDown();
		}
	}
	VideoManager->EndImageLoadBatch();

	std::vector<hoa_video::StillImage> enemyAnimation2;
	StillImage anim2;
	anim2.SetDimensions(64, 128);
	anim2.SetFilename("img/sprites/battle/enemies/skeleton_d0.png");
	enemyAnimation2.push_back(anim2);
	anim2.SetFilename("img/sprites/battle/enemies/skeleton_d1.png");
	enemyAnimation2.push_back(anim2);
	anim2.SetFilename("img/sprites/battle/enemies/skeleton_d2.png");
	enemyAnimation2.push_back(anim2);
	anim2.SetFilename("img/sprites/battle/enemies/skeleton_d3.png");
	enemyAnimation2.push_back(anim2);

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < enemyAnimation2.size(); i++) {
		if (!VideoManager->LoadImage(enemyAnimation2[i])) {
			cerr << "Failed to load skeleton image." << endl;	//failed to laod image
			_ShutDown();
		}
	}
	VideoManager->EndImageLoadBatch();

	std::vector<hoa_video::StillImage> enemyAnimation3;
	StillImage anim3;
	anim3.SetDimensions(64, 64);
	anim3.SetFilename("img/sprites/battle/enemies/greenslime_d0.png");
	enemyAnimation3.push_back(anim3);
	anim3.SetFilename("img/sprites/battle/enemies/greenslime_d1.png");
	enemyAnimation3.push_back(anim3);
	anim3.SetFilename("img/sprites/battle/enemies/greenslime_d2.png");
	enemyAnimation3.push_back(anim3);
	anim3.SetFilename("img/sprites/battle/enemies/greenslime_d3.png");
	enemyAnimation3.push_back(anim3);

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < enemyAnimation3.size(); i++) {
		if (!VideoManager->LoadImage(enemyAnimation3[i])) {
			cerr << "Failed to load green slime image." << endl;	//failed to laod image
			_ShutDown();
		}
	}
	VideoManager->EndImageLoadBatch();

	std::vector<hoa_video::StillImage> enemyAnimation4;
	StillImage anim4;
	anim4.SetDimensions(128, 64);
	anim4.SetFilename("img/sprites/battle/enemies/snake_d0.png");
	enemyAnimation4.push_back(anim4);
	anim4.SetFilename("img/sprites/battle/enemies/snake_d1.png");
	enemyAnimation4.push_back(anim4);
	anim4.SetFilename("img/sprites/battle/enemies/snake_d2.png");
	enemyAnimation4.push_back(anim4);
	anim4.SetFilename("img/sprites/battle/enemies/snake_d3.png");
	enemyAnimation4.push_back(anim4);

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < enemyAnimation4.size(); i++) {
		if (!VideoManager->LoadImage(enemyAnimation4[i])) {
			cerr << "Failed to load snake image." << endl;	//failed to laod image
			_ShutDown();
		}
	}
	VideoManager->EndImageLoadBatch();

	GlobalCharacter *claud = GlobalManager->GetCharacter(hoa_global::GLOBAL_CLAUDIUS);
	if (claud == NULL) {
		std::cerr << "No claudius character?  What?." << std::endl;
		_ShutDown();
	} else {
		//cerr << "Creating claudius player character." << endl;
		PlayerActor *
		claudius = new PlayerActor(claud, this, 250, 200);
		_player_actors.push_back(claudius);
		_players_characters_in_battle.push_back(claudius);
		_user_interface.SetPlayerActorSelected(claudius);
	}

	GlobalEnemy e("spider");
	e.AddAnimation("IDLE", enemyAnimation);
	//e.AddAttackSkill(new GlobalSkill("sword_swipe"));
	EnemyActor *enemy = new EnemyActor(e, this, 600, 130);
	enemy->LevelUp(2);

	GlobalEnemy e2("skeleton");
	e2.AddAnimation("IDLE", enemyAnimation2);
	//e2.AddAttackSkill(new GlobalSkill("sword_swipe"));
	EnemyActor *enemy2 = new EnemyActor(e2, this, 805, 330);
	enemy2->LevelUp(2);

	GlobalEnemy e3("slime");
	e3.AddAnimation("IDLE", enemyAnimation3);
	//e3.AddAttackSkill(new GlobalSkill("sword_swipe"));
	EnemyActor *enemy3 = new EnemyActor(e3, this, 805, 170);
	enemy3->LevelUp(2);

	GlobalEnemy e4("snake");
	e4.AddAnimation("IDLE", enemyAnimation4);
	//e3.AddAttackSkill(new GlobalSkill("sword_swipe"));
	EnemyActor *enemy4 = new EnemyActor(e4, this, 600, 280);
	enemy4->LevelUp(2);

	_enemy_actors.push_back(enemy);
	_enemy_actors.push_back(enemy3);
	_enemy_actors.push_back(enemy4);
	_enemy_actors.push_back(enemy2);
}


} // namespace hoa_battle
