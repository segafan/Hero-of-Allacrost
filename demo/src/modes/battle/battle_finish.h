////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_finish.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for battle finish menu
***
*** This code takes effect after either the character or enemy party has emerged
*** victorious in the battle.
*** ***************************************************************************/

#ifndef __BATTLE_FINISH_HEADER__
#define __BATTLE_FINISH_HEADER__

#include "utils.h"
#include "defs.h"

#include "video.h"

#include "gui.h"
#include "global.h"

#include "battle_utils.h"

namespace hoa_battle {

namespace private_battle {

//! \brief The set of defeat options that the player can select
//@{
const uint32 DEFEAT_OPTION_RETRY     = 0;
const uint32 DEFEAT_OPTION_RESTART   = 1;
const uint32 DEFEAT_OPTION_RETURN    = 2;
const uint32 DEFEAT_OPTION_RETIRE    = 3;
//@}

//! \brief The maximum number of times that the player can retry the battle if they lose
const uint32 MAX_NUMBER_RETRIES   = 2;

/** ****************************************************************************
*** \brief A collection of GUI objects drawn when the player loses the battle
***
*** This class assists the FinishSupervisor class. It is only utilized when the
*** player's characters are defeated in battle and presents the player with a
*** number of options
***
*** - Retry: resets the state of the battle to the beginning and allows the
*** - Restart: loads the game state from the last save point
*** - Return: brings the player back to boot mode
*** - Retire: exits the game
*** ***************************************************************************/
class FinishDefeat {
public:
	FinishDefeat();

	~FinishDefeat()
		{}

	uint32 GetNumberRetryTimes() const
		{ return _number_retry_times; }

	//! \brief Processes user input and updates the GUI controls
	void Update();

	//! \brief Draws the finish window and GUI contents to the screen
	void Draw();

private:
	//! \brief The number of times that the player has lost and chosen to retry the battle
	uint32 _number_retry_times;

	//! \brief The window that the defeat message and options are displayed upon
	hoa_gui::MenuWindow _options_window;

	//! \brief The window that the defeat message and options are displayed upon
	hoa_gui::MenuWindow _tooltip_window;

	//! \brief Text that displays the battle's outcome
	hoa_gui::TextBox _outcome_message;

	//! \brief The list of options that the player may choose from when they lose the battle
	hoa_gui::OptionBox _options;

	//! \brief Tooltip text explaining the currently selected option
	hoa_gui::TextBox _tooltip;
}; // class FinishDefeat


/** ****************************************************************************
*** \brief The window displayed once a battle has either been won or lost
***
*** This window is located in the center right portion of the screen and only appears
*** when an outcome has been decided in the battle. The contents of this window differ
*** depending on whether the battle was victorious or a loss. If the player won
*** the battle, they will have their victory spoils written to the screen along
*** with any character growth information (e.g. experience level up). If the player
*** lost the battle, they will be presented with a number of options. The player
*** may choose to:
***
*** - Retry the battle from the beginning
*** - Load the game from the last save point
*** - Return to the game's main menu
*** - Exit the game
***
*** \todo Add feature where spoils (XP, drunes, etc) are quickly counted down as
*** they go into the party's posession.
*** ***************************************************************************/
class FinishWindow : public hoa_gui::MenuWindow {
	friend class hoa_battle::BattleMode;

public:
	FinishWindow();

	~FinishWindow();

	/** \brief Un-hides the window display and creates the window contents
	*** \param victory Set to true if the player's party was victorious in battle; false if he/she was defeated
	**/
	void Initialize(bool victory);

	//! \brief Updates the state of the window
	void Update();

	//! \brief Draws the window and its contents
	void Draw();

	FINISH_STATE GetState() const
		{ return _state; }

private:
	//! \brief The state that the window is in, which determines its contents
	FINISH_STATE _state;

	//! \brief The amount of money won
	int32 _victory_money;

	//! \brief The amount of xp earned (per character)
	int32 _victory_xp;

	//! \brief Tallies the amount of growth each character has received for each stat
	int _growth_gained[4][8];

	//! \brief Pointers to all characters who took part in the battle
	std::vector<hoa_global::GlobalCharacter*> _characters;

	//! \brief The growth members for all object pointers in the _characters table
	std::vector<hoa_global::GlobalCharacterGrowth*> _character_growths;

	//! \brief The music to play if the character party is victorious
	hoa_audio::MusicDescriptor _victory_music;

	//! \brief The music to play if the character party is defeated
	hoa_audio::MusicDescriptor _defeat_music;

	//! \brief Text that displays the battle's outcome (victory or defeat)
	hoa_gui::TextBox _finish_outcome;

	//! \brief The list of options that the player may choose from when they lose the battle
	hoa_gui::OptionBox _lose_options;

	//! \brief The window containing the XP and money won
	hoa_gui::MenuWindow _xp_and_money_window;

	//! \brief The windows that show character portraits and stats
	hoa_gui::MenuWindow _character_window[4];

	//! \brief Lists the items won
	hoa_gui::MenuWindow _items_window;

	//! \brief Character portraits
	hoa_video::StillImage _char_portraits[4];

	//! \brief Items won from battle (<ID, quantity>)
	std::map<hoa_global::GlobalObject*, int32> _victory_items;

	// ----- Private methods

	/** \brief Creates 4 character windows
	*** \param start_x The x coordinate for the upper left corner of the window
	*** \param start_y The y coordinate for the upper left corner of the window
	**/
	void _InitCharacterWindows(float start_x, float start_y);

	/** \brief Creates _xp_and_money_window and _items_window
	*** \param start_x The x coordinate for the upper left corner of the window
	*** \param start_y The y coordinate for the upper left corner of the window
	**/
	void _InitSpoilsWindows(float start_x, float start_y);

	//! \brief Sets up the OptionBox for things like retrying the battle
	void _InitLoseOptions();

	//! \brief Either victory or death
	void _InitVictoryText();

	//! \brief Tallies up the xp, money, and items earned from killing the enemies
	void _TallyXPMoneyAndItems();

	//! \brief Use this to clear learned skills after they've been shown so that they don't render every battle
	void _ClearLearnedSkills();

	//! \brief Handles update processing when the _state member is FINISH_ANNOUNCE_WIN
	void _UpdateAnnounceWin();

	//! \brief Handles update processing when the _state member is FINISH_WIN_GROWTH
	void _UpdateWinGrowth();

	//! \brief Just waits for the player to press OK, then moves on
	void _UpdateWinWaitForOK();

	//! \brief Handles update processing when the _state member is FINISH_WIN_SPOILS
	void _UpdateWinSpoils();

	//! \brief Handles update processing when the _state member is FINISH_ANNOUNCE_LOSE
	void _UpdateAnnounceLose();

	//! \brief Handles update processing when the _state member is FINISH_LOSE_CONFIRM
	void _UpdateLoseConfirm();

	//! \brief Handles update processing when the _state member is FINISH_ANNOUNCE_WIN
	void _DrawAnnounceWin();

	//! \brief Handles update processing when the _state member is FINISH_WIN_GROWTH
	void _DrawWinGrowth();

	//! \brief Handles update processing when the _state member is FINISH_WIN_GROWTH
	void _DrawWinSkills();

	//! \brief Handles update processing when the _state member is FINISH_WIN_SPOILS
	void _DrawWinSpoils();

	//! \brief Handles update processing when the _state member is FINISH_ANNOUNCE_LOSE
	void _DrawAnnounceLose();

	//! \brief Handles update processing when the _state member is FINISH_LOSE_CONFIRM
	void _DrawLoseConfirm();
}; // class FinishWindow : public hoa_video:MenuWindow

} // namespace private_battle

} // namespace hoa_battle

#endif // __BATTLE_FINISH_HEADER__
