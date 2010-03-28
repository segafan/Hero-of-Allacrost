////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_finish.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for battle finish menu
*** ***************************************************************************/

#include "mode_manager.h"
#include "input.h"
#include "system.h"
#include "video.h"

#include "battle.h"
#include "battle_actions.h"
#include "battle_actors.h"
#include "battle_finish.h"
#include "battle_utils.h"

#include "boot.h"

using namespace std;

using namespace hoa_utils;

using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_gui;
using namespace hoa_input;
using namespace hoa_mode_manager;
using namespace hoa_system;
using namespace hoa_global;

namespace hoa_battle {

namespace private_battle {

////////////////////////////////////////////////////////////////////////////////
// FinishDefeat class
////////////////////////////////////////////////////////////////////////////////

FinishDefeat::FinishDefeat() :
	_number_retry_times(0)
{
	_options_window.Create(512.0f, 64.0f);
	_options_window.SetPosition(512.0f, 60.0f);
	_options_window.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_TOP);

	_tooltip_window.Create(512.0f, 112.0f);
	_tooltip_window.SetPosition(512.0f, 124.0f);
	_tooltip_window.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_TOP);

	_outcome_message.SetPosition(512.0f, 384.0f);
	_outcome_message.SetDimensions(400.0f, 100.0f);
	_outcome_message.SetDisplaySpeed(30);
	_outcome_message.SetTextStyle(TextStyle("text24", Color::white));
	_outcome_message.SetDisplayMode(VIDEO_TEXT_INSTANT);
	_outcome_message.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_outcome_message.SetTextAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_outcome_message.SetDisplayText(Translate("But the heroes were defeated..."));

	_options.AddOption(UTranslate("Retry"));
	_options.AddOption(UTranslate("Restart"));
	_options.AddOption(UTranslate("Return"));
	_options.AddOption(UTranslate("Retire"));
	_options.SetPosition(270.0f, 130.0f);
	_options.SetDimensions(128.0f, 200.0f, 1, 4, 1, 4);
	_options.SetTextStyle(TextStyle("title22", Color::white, VIDEO_TEXT_SHADOW_DARK));
	_options.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_options.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_options.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_options.SetCursorOffset(-60.0f, 25.0f);
	_options.SetSelection(0);
	_options.SetOwner(&_options_window);
	// TEMP: these two options are disabled because their features are not yet implemented
	_options.EnableOption(0, false);
	_options.EnableOption(1, false);
}



void FinishDefeat::Update() {
	if (InputManager->ConfirmPress()) {
		switch (_options.GetSelection()) {
			case DEFEAT_OPTION_RETRY:
				// TODO: feature not yet implemented. Reset the battle from the beginning.
				_number_retry_times++;
				break;
			case DEFEAT_OPTION_RESTART:
				// TODO: feature not yet implemented. Either auto-load last saved game or enter save
				// mode and allow user to select file to load (but do not allow user to save a file)
				break;
			case DEFEAT_OPTION_RETURN:
				ModeManager->PopAll();
				ModeManager->Push(new hoa_boot::BootMode());
				break;
			case DEFEAT_OPTION_RETIRE:
				SystemManager->ExitGame();
				break;
			default:
				IF_PRINT_WARNING(BATTLE_DEBUG) << "invalid option selection: " << _options.GetSelection() << endl;
				break;
		}
	}

	else if (InputManager->LeftPress()) {
		_options.InputLeft();
	}
	else if (InputManager->RightPress()) {
		_options.InputRight();
	}
}



void FinishDefeat::Draw() {
	_options_window.Draw();
	_tooltip_window.Draw();
	_outcome_message.Draw();
	_options.Draw();
	_tooltip.Draw();
}

////////////////////////////////////////////////////////////////////////////////
// FinishWindow class
////////////////////////////////////////////////////////////////////////////////

FinishWindow::FinishWindow()
{
	//CD: We should really move all this to Initialize() instead

	// TODO: declare the MenuSkin to be used
	//Just like the ones in Menu Mode
	float start_x = (1024 - 800) / 2 + 144;
	float start_y = 768 - ((768 - 600) / 2 + 15);

	if (!MenuWindow::Create(480.0f, 560.0f))
		IF_PRINT_WARNING(BATTLE_DEBUG) << "the call to MenuWindow::Create() failed" << endl;

	MenuWindow::SetPosition(start_x, start_y);

	for (int32 i = 0; i < 4; ++i)
	{
		for (int32 j = 0; j < 8; ++j)
			_growth_gained[i][j] = 0;
	}

	_state = FINISH_INVALID;

	//Create character windows
	_InitCharacterWindows(start_x, start_y);
	//Create items and xp & money window
	_InitSpoilsWindows(start_x, start_y);
	//Initalize victory text (but don't set the string yet)
	_InitVictoryText();
	//Retry, quit, etc.
	_InitLoseOptions();
}



FinishWindow::~FinishWindow() {
	_character_window[0].Destroy();
	_character_window[1].Destroy();
	_character_window[2].Destroy();
	_character_window[3].Destroy();

	_xp_and_money_window.Destroy();
	_items_window.Destroy();

	MenuWindow::Destroy();
}



void FinishWindow::Initialize(bool victory) {
	MenuWindow::Show();

	_victory_money = 0;
	_victory_xp = 0;
	_victory_items.clear();

	for (uint32 i = 0; i < BattleMode::CurrentInstance()->_character_actors.size(); i++) {
		_characters.push_back(BattleMode::CurrentInstance()->_character_actors[i]->GetGlobalCharacter());
		_character_growths.push_back(_characters[i]->GetGrowth());
		_char_portraits[i].Load("img/portraits/map/" + BattleMode::CurrentInstance()->_character_actors[i]->GetFilename() + ".png", 100.0f, 100.0f);
	}

	if (victory) {
		_state = FINISH_WIN_ANNOUNCE;
		_finish_outcome.SetDisplayText("The heroes are victorious!");
		_TallyXPMoneyAndItems();
	}
	else {
		_state = FINISH_LOSE_ANNOUNCE;
		_finish_outcome.SetDisplayText("The heroes have been defeated...");
	}
}

void FinishWindow::_InitCharacterWindows(float start_x, float start_y)
{
	_character_window[0].Create(480.0f, 140.0f, ~VIDEO_MENU_EDGE_BOTTOM, VIDEO_MENU_EDGE_BOTTOM);
	_character_window[0].SetPosition(start_x, start_y - 12.0f);
	_character_window[0].Show();

	_character_window[1].Create(480.0f, 140.0f, ~VIDEO_MENU_EDGE_BOTTOM, VIDEO_MENU_EDGE_BOTTOM);
	_character_window[1].SetPosition(start_x, start_y - 12.0f - 140.0f);
	_character_window[1].Show();

	_character_window[2].Create(480.0f, 140.0f, ~VIDEO_MENU_EDGE_BOTTOM, VIDEO_MENU_EDGE_BOTTOM);
	_character_window[2].SetPosition(start_x, start_y - 11.0f - 140.0f * 2.0f);
	_character_window[2].Show();

	_character_window[3].Create(480.0f, 140.0f, VIDEO_MENU_EDGE_ALL, ~VIDEO_MENU_EDGE_ALL);//~VIDEO_MENU_EDGE_BOTTOM, VIDEO_MENU_EDGE_BOTTOM);
	_character_window[3].SetPosition(start_x, start_y - 10.0f - 140.0f * 3.0f);
	_character_window[3].Show();
}


void FinishWindow::_InitSpoilsWindows(float start_x, float start_y)
{
	_xp_and_money_window.Create(480.0f, 72.0f, VIDEO_MENU_EDGE_ALL, ~VIDEO_MENU_EDGE_ALL);
	_xp_and_money_window.SetPosition(start_x, start_y + 50.0f);
	_xp_and_money_window.Show();

	_items_window.Create(480.0f, 560.0f, ~VIDEO_MENU_EDGE_TOP, VIDEO_MENU_EDGE_TOP);
	_items_window.SetPosition(start_x, start_y - 13.0f);
	_items_window.Show();
}


void FinishWindow::_InitLoseOptions()
{
	vector<ustring> lose_text;
	lose_text.push_back(UTranslate("Retry the battle"));
	lose_text.push_back(UTranslate("Load from last save point"));
	lose_text.push_back(UTranslate("Return to main menu"));
	lose_text.push_back(UTranslate("Exit the game"));
	_lose_options.SetOptions(lose_text);
	_lose_options.SetPosition(270.0f, 130.0f);
	_lose_options.SetDimensions(128.0f, 200.0f, 1, 4, 1, 4);
	_lose_options.SetTextStyle(TextStyle("text22", Color::white, VIDEO_TEXT_SHADOW_DARK));
	_lose_options.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_lose_options.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_lose_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_lose_options.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_lose_options.SetCursorOffset(-60.0f, 25.0f);
	_lose_options.SetSelection(0);
	_lose_options.SetOwner(this);
	_lose_options.EnableOption(0, false);
	_lose_options.EnableOption(1, false);
}


void FinishWindow::_InitVictoryText()
{
	_finish_outcome.SetPosition(512.0f, 384.0f);
	_finish_outcome.SetDimensions(400.0f, 100.0f);
	_finish_outcome.SetDisplaySpeed(30);
	_finish_outcome.SetTextStyle(TextStyle("text24", Color::white));
	_finish_outcome.SetDisplayMode(VIDEO_TEXT_INSTANT);
	_finish_outcome.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_finish_outcome.SetTextAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
}

// ----- Tallies all the stuff we've won (xp, money, items)

void FinishWindow::_TallyXPMoneyAndItems()
{
	GlobalEnemy* ge;
	vector<GlobalObject*> objects;
	map<GlobalObject*, int32>::iterator iter;

	deque<BattleEnemy*>& all_enemies = BattleMode::CurrentInstance()->GetEnemyActors();
	for (uint32 i = 0; i < all_enemies.size(); ++i)
	{
		ge = all_enemies[i]->GetGlobalEnemy();
		_victory_money += ge->GetDrunesDropped();
		_victory_xp += ge->GetExperiencePoints();
		ge->DetermineDroppedObjects(objects);

		for (uint32 j = 0; j < objects.size(); ++j)
		{
			iter = _victory_items.find(objects[j]);
			if (iter != _victory_items.end())
			{
				iter->second++;
			}
			else
			{
				_victory_items.insert(make_pair(objects[j], 1));
			}
		}
		// also add 1 SP per enemy to each character in the party
		GlobalParty *party = GlobalManager->GetActiveParty();
		for (uint32 i = 0; i < party->GetPartySize(); ++i)
			party->GetActorAtIndex(i)->AddSkillPoints(1);
	}

	deque<BattleCharacter*>& all_characters = BattleMode::CurrentInstance()->GetCharacterActors();
	uint32 num_alive_characters = 0;
	for (uint32 i = 0; i < all_characters.size(); ++i)
	{
		if (all_characters[i]->IsAlive())
		{
			++num_alive_characters;
		}
	}
	_victory_xp /= num_alive_characters;
}

void FinishWindow::_ClearLearnedSkills()
{
	for (uint32 i = 0; i < _characters.size(); ++i)
	{
		_character_growths[i]->GetSkillsLearned()->clear();
	}
}

// ----- UPDATE METHODS

void FinishWindow::Update() {
	MenuWindow::Update(SystemManager->GetUpdateTime());

	switch (_state) {
		case FINISH_WIN_ANNOUNCE:
			_UpdateAnnounceWin();
			break;
		case FINISH_WIN_SHOW_GROWTH:
		case FINISH_WIN_RESOLVE_GROWTH:
		case FINISH_WIN_SHOW_SKILLS:
		case FINISH_WIN_SHOW_SPOILS:
		case FINISH_WIN_RESOLVE_SPOILS:
			_UpdateWinWaitForOK();
			break;
		case FINISH_WIN_COUNTDOWN_GROWTH:
			_UpdateWinGrowth();
			break;
		case FINISH_WIN_COUNTDOWN_SPOILS:
			_UpdateWinSpoils();
			break;
		case FINISH_WIN_COMPLETE:
			BattleMode::CurrentInstance()->Exit();
			break;
		case FINISH_LOSE_ANNOUNCE:
			_UpdateAnnounceLose();
			break;
		case FINISH_LOSE_CONFIRM:
			_UpdateLoseConfirm();
			break;
		case FINISH_INVALID:
		case FINISH_TOTAL:
		default:
			if (BATTLE_DEBUG)
				cerr << "BATTLE ERROR: In FinishWindow::Update(), the window state was invalid: " << _state << endl;
			return;
	}
} // void FinishWindow::Update()



void FinishWindow::_UpdateAnnounceWin() {
//	This block is for gradual text. Currently, battle mode uses full text.
// This block causes the game to freeze at the end of battle.
/*	if (_finish_outcome.IsFinished() == false) {
		_finish_outcome.Update(SystemManager->GetUpdateTime());

		if (InputManager->ConfirmPress())
			_finish_outcome.ForceFinish();
		return;
	} */

	if (InputManager->ConfirmPress())
		_state = FINISH_WIN_SHOW_GROWTH;
}

// If OK was pressed, just move to the next state
void FinishWindow::_UpdateWinWaitForOK()
{
	if (InputManager->ConfirmPress())
	{
		switch (_state)
		{
			case FINISH_WIN_SHOW_GROWTH:
				_state = FINISH_WIN_COUNTDOWN_GROWTH;
				break;
			case FINISH_WIN_RESOLVE_GROWTH:
				_state = FINISH_WIN_SHOW_SKILLS;
				break;
			case FINISH_WIN_SHOW_SKILLS:
				_state = FINISH_WIN_SHOW_SPOILS;
				_ClearLearnedSkills(); //so we don't render them every battle
				break;
			case FINISH_WIN_SHOW_SPOILS:
				_state = FINISH_WIN_COUNTDOWN_SPOILS;
				break;
			case FINISH_WIN_RESOLVE_SPOILS:
				_state = FINISH_WIN_COMPLETE;
				break;
			default:
				if (BATTLE_DEBUG)
					cerr << "BATTLE ERROR: In FinishWindow::_UpdateWinWaitForOK(), the window state was invalid: " << _state << endl;
				return;
		}
	}
}

void FinishWindow::_UpdateWinGrowth() {
	static uint32 time_of_next_update = SDL_GetTicks();
	uint32 xp_to_add = 1;

	if (InputManager->ConfirmPress())
	{
		xp_to_add = _victory_xp;
		_victory_xp = 0;
	}
	else if (SDL_GetTicks() < time_of_next_update)
	{
		return;
	}
	else
	{
		--_victory_xp;
	}

	for (uint32 i = 0; i < _characters.size(); ++i)
	{
		if (_characters[i]->IsAlive())
		{
			if (_characters[i]->AddExperiencePoints(xp_to_add))
			{
				do {
					//Record growth stats for each character for rendering
					//HP
					_growth_gained[i][0] += _character_growths[i]->GetHitPointsGrowth();
					//SP
					_growth_gained[i][1] += _character_growths[i]->GetSkillPointsGrowth();
					//STR
					_growth_gained[i][2] += _character_growths[i]->GetStrengthGrowth();
					//VIG
					_growth_gained[i][3] += _character_growths[i]->GetVigorGrowth();
					//FOR
					_growth_gained[i][4] += _character_growths[i]->GetFortitudeGrowth();
					//PRO
					_growth_gained[i][5] += _character_growths[i]->GetProtectionGrowth();
					//AGI
					_growth_gained[i][6] += _character_growths[i]->GetAgilityGrowth();
					//EVD
					_growth_gained[i][7] += static_cast<int>(_character_growths[i]->GetEvadeGrowth());

					if (_character_growths[i]->IsExperienceLevelGained())
					{
						//Play Sound
					}
					_character_growths[i]->AcknowledgeGrowth();
				} while(_character_growths[i]->IsGrowthDetected());
			}
		}
	}

	//We've allocated all the XP
	if (!_victory_xp)
		_state = FINISH_WIN_RESOLVE_GROWTH;

	//Every 50 milliseconds we update
	time_of_next_update += 50;
}

void FinishWindow::_UpdateWinSpoils() {
	static uint32 time_of_next_update = SDL_GetTicks();
	uint32 money_to_add = 1;

	if (InputManager->ConfirmPress())
	{
		money_to_add = _victory_money;
		_victory_money = 0;
	}
	else if (SDL_GetTicks() < time_of_next_update)
	{
		return;
	}
	else
	{
		--_victory_money;
	}

	GlobalManager->AddDrunes(money_to_add);

	if (!_victory_money)
	{
		std::map<GlobalObject*, int32>::iterator iter;

		for (iter = _victory_items.begin(); iter != _victory_items.end(); ++iter)
		{
			GlobalManager->AddToInventory(iter->first->GetID(), iter->second);
		}

		_state = FINISH_WIN_RESOLVE_SPOILS;
	}

	//Every 50 milliseconds we update
	time_of_next_update += 50;
}



void FinishWindow::_UpdateAnnounceLose() {
	_lose_options.Update();

//	This block is for gradual text. Currently, battle mode uses full text.
// This block causes the game to freeze at the end of battle.
/*	if (_finish_outcome.IsFinished() == false) {
		_finish_outcome.Update(SystemManager->GetUpdateTime());

		if (InputManager->ConfirmPress())
			_finish_outcome.ForceFinish();
		return;
	} */

	if (InputManager->UpPress()) {
		_lose_options.InputUp();
	}
	else if (InputManager->DownPress()) {
		_lose_options.InputDown();
	}
	else if (InputManager->ConfirmPress()) {
		switch (_lose_options.GetSelection()) {
			case 0: // Retry the battle
				// TODO
				break;
			case 1: // Load from last save point
				// TODO
				break;
			case 2: // Return to main menu
			case 3: // Exit game
				_state = FINISH_LOSE_CONFIRM;
				break;
		}
	}

}


void FinishWindow::_UpdateLoseConfirm() {
	if (_lose_options.GetSelection() == 2) {
		// Remove all game modes on the stack and return to boot mode
		ModeManager->PopAll();
		ModeManager->Push(new hoa_boot::BootMode());
	}
	else {
		SystemManager->ExitGame();
	}
}

// ----- DRAW METHODS

void FinishWindow::Draw() {
	VideoManager->DisableSceneLighting();
	//VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);
	//TEMP!
	//Two different window arrangements for win and lose would be best
	//Win has all the elaborate windows, lose just has the game over options
	//MenuWindow::Draw();
	//_items_window.Draw();

	//TEMP!!!
	//Just so everyone has an idea of the potential setup for the finishwindow

	switch (_state) {
		case FINISH_WIN_ANNOUNCE:
			_DrawAnnounceWin();
			break;
		case FINISH_WIN_SHOW_GROWTH:
		case FINISH_WIN_COUNTDOWN_GROWTH:
		case FINISH_WIN_RESOLVE_GROWTH:
			_character_window[0].Draw();
			_character_window[1].Draw();
			_character_window[2].Draw();
			_character_window[3].Draw();

			_xp_and_money_window.Draw();
			_DrawWinGrowth();
			break;
		case FINISH_WIN_SHOW_SKILLS:
			_character_window[0].Draw();
			_character_window[1].Draw();
			_character_window[2].Draw();
			_character_window[3].Draw();

			_xp_and_money_window.Draw();
			_DrawWinSkills();
			break;
		case FINISH_WIN_SHOW_SPOILS:
		case FINISH_WIN_COUNTDOWN_SPOILS:
		case FINISH_WIN_RESOLVE_SPOILS:
			_items_window.Draw();
			_xp_and_money_window.Draw();
			_DrawWinSpoils();
			break;
		case FINISH_LOSE_ANNOUNCE:
			_DrawAnnounceLose();
			break;
		case FINISH_LOSE_CONFIRM:
			_DrawLoseConfirm();
			break;
		case FINISH_WIN_COMPLETE:
			break;
		case FINISH_INVALID:
		case FINISH_TOTAL:
		default:
			if (BATTLE_DEBUG)
				cerr << "BATTLE ERROR: In FinishWindow::Draw(), the window state was invalid: " << _state << endl;
			return;
	}
}


void FinishWindow::_DrawAnnounceWin() {
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->Move(512.0f, 384.0f);
// 	_finish_outcome.Draw();
	VideoManager->Text()->Draw(UTranslate("Victory!!!"), TextStyle("title24"));
}



void FinishWindow::_DrawWinGrowth() {
	//Draw XP Earned
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->Move(496, 683);
	VideoManager->Text()->Draw(UTranslate("XP Gained: ") + MakeUnicodeString(NumberToString(_victory_xp)));

	//Now draw char info
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_CENTER, 0);
	//VideoManager->Move(265, 580);
	VideoManager->Move(270, 595);

	ustring display_text;
	for (uint32 i = 0; i < _characters.size(); ++i)
	{
		//Portraits
		_char_portraits[i].Draw();

		VideoManager->MoveRelative(5,-55);
		VideoManager->Text()->Draw(UTranslate("Lv. ") +
			MakeUnicodeString(NumberToString(_characters[i]->GetExperienceLevel())));
		VideoManager->MoveRelative(0, -15);
		VideoManager->Text()->Draw(UTranslate("XP To Next: ") +
			MakeUnicodeString(NumberToString(_characters[i]->GetExperienceForNextLevel() - _characters[i]->GetExperiencePoints())));

		//First column
		//VideoManager->MoveRelative(150, 40);
		//VideoManager->MoveRelative(150, 25);
		VideoManager->MoveRelative(140, 105);

		//HP
		display_text = UTranslate("HP: ") +
			MakeUnicodeString(NumberToString(_characters[i]->GetMaxHitPoints()));
		if (_growth_gained[i][0])
		{
			display_text += MakeUnicodeString(" (") +
			MakeUnicodeString(NumberToString(_growth_gained[i][0])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		//SP
		VideoManager->MoveRelative(0, -26);
		display_text = UTranslate("SP: ") +
			MakeUnicodeString(NumberToString(_characters[i]->GetMaxSkillPoints()));
		if (_growth_gained[i][1])
		{
			display_text += MakeUnicodeString(" (") +
			MakeUnicodeString(NumberToString(_growth_gained[i][1])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		//STR
		VideoManager->MoveRelative(0, -26);
		display_text = UTranslate("STR: ") +
			MakeUnicodeString(NumberToString(_characters[i]->GetStrength()));
		if (_growth_gained[i][2])
		{
			display_text += MakeUnicodeString(" (") +
			MakeUnicodeString(NumberToString(_growth_gained[i][2])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		//VIG
		VideoManager->MoveRelative(0, -26);
		display_text = UTranslate("VIG: ") +
			MakeUnicodeString(NumberToString(_characters[i]->GetVigor()));
		if (_growth_gained[i][3])
		{
			display_text += MakeUnicodeString(" (") +
			MakeUnicodeString(NumberToString(_growth_gained[i][3])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		//Second Column
		//FOR
		VideoManager->MoveRelative(155, 78);
		display_text = UTranslate("FOR: ") +
			MakeUnicodeString(NumberToString(_characters[i]->GetStrength()));
		if (_growth_gained[i][4])
		{
			display_text += MakeUnicodeString(" (") +
			MakeUnicodeString(NumberToString(_growth_gained[i][4])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		//PRO
		VideoManager->MoveRelative(0, -26);
		display_text = UTranslate("PRO: ") +
			MakeUnicodeString(NumberToString(_characters[i]->GetProtection()));
		if (_growth_gained[i][5])
		{
			display_text += MakeUnicodeString(" (") +
			MakeUnicodeString(NumberToString(_growth_gained[i][5])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		//AGI
		VideoManager->MoveRelative(0, -26);
		display_text = UTranslate("AGI: ") +
			MakeUnicodeString(NumberToString(_characters[i]->GetAgility()));
		if (_growth_gained[i][6])
		{
			display_text += MakeUnicodeString(" (") +
			MakeUnicodeString(NumberToString(_growth_gained[i][6])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		//EVD
		VideoManager->MoveRelative(0, -26);
		display_text = UTranslate("EVD: ") +
			MakeUnicodeString(NumberToString(_characters[i]->GetEvade()));
		if (_growth_gained[i][7])
		{
			display_text += MakeUnicodeString(" (") +
			MakeUnicodeString(NumberToString(_growth_gained[i][7])) + MakeUnicodeString(")");
		}
		VideoManager->Text()->Draw(display_text);

		VideoManager->MoveRelative(-300,-140 + 43);
	}
}

void FinishWindow::_DrawWinSkills()
{
	//Draw XP Earned
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->Move(496, 683);
	VideoManager->Text()->Draw(UTranslate("XP Gained: ") + MakeUnicodeString(NumberToString(_victory_xp)));

	//Now draw char info
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_CENTER, 0);
	//VideoManager->Move(265, 580);
	VideoManager->Move(270, 595);

	std::vector<GlobalSkill*>* skills_learned = NULL;
	ustring display_text;
	for (uint32 i = 0; i < _characters.size(); ++i)
	{
		//Portrait
		_char_portraits[i].Draw();
		//TEMP
		VideoManager->MoveRelative(140, 35);
		VideoManager->Text()->Draw(UTranslate("Skills Learned"));
		VideoManager->MoveRelative(50, -30);

		skills_learned = _character_growths[i]->GetSkillsLearned();

		for (uint32 j = 0; j < skills_learned->size(); ++j)
		{
			VideoManager->Text()->Draw(skills_learned->at(j)->GetName());
			VideoManager->MoveRelative(0, -20);
		}

		VideoManager->MoveRelative(-190, -5 + (20 * (float)(skills_learned->size())) - 140);
	}
}

void FinishWindow::_DrawWinSpoils()
{
	//VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_CENTER, 0);
	//VideoManager->Move(496, 683);
	//VideoManager->Move(96, 683);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_CENTER, 0);
	VideoManager->Move(280, 683);
	VideoManager->Text()->Draw(UTranslate("Drunes: ") + MakeUnicodeString(NumberToString(_victory_money)));

	VideoManager->SetDrawFlags(VIDEO_X_RIGHT, VIDEO_Y_CENTER, 0);
	VideoManager->Move(712, 683);
	VideoManager->Text()->Draw(MakeUnicodeString("$ ") + MakeUnicodeString(NumberToString(GlobalManager->GetDrunes())));

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
	//VideoManager->Move(700, 640);
	VideoManager->Move(475, 640);
	VideoManager->Text()->Draw(UTranslate("Items"));
	//VideoManager->MoveRelative(-140, -25);
	VideoManager->MoveRelative(-200, -35);

	std::map<GlobalObject*, int32>::iterator iter;

	for (iter = _victory_items.begin(); iter != _victory_items.end(); ++iter)
	{
		VideoManager->Text()->Draw(iter->first->GetName());
		VideoManager->SetDrawFlags(VIDEO_X_RIGHT, VIDEO_Y_TOP, 0);
		VideoManager->MoveRelative(425, 0);
		VideoManager->Text()->Draw(MakeUnicodeString(NumberToString(iter->second)));
		VideoManager->MoveRelative(-425, -25);
		VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
	}
}



void FinishWindow::_DrawAnnounceLose() {

	_lose_options.Draw();

}



void FinishWindow::_DrawLoseConfirm() {

}

} // namespace private_battle

} // namespace hoa_battle
