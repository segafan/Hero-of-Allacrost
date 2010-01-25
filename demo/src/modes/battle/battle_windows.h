////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_windows.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for battle menu windows
***
*** This code contains classes that represent windows on the battle screen.
*** These classes manage both update and draw code to those windows.
***
*** \todo Add a dialogue window class when battle dialogue support is
*** implemented.
*** ***************************************************************************/

#ifndef __BATTLE_WINDOWS_HEADER__
#define __BATTLE_WINDOWS_HEADER__

#include "utils.h"
#include "defs.h"

#include "video.h"
#include "global.h"

namespace hoa_battle {

namespace private_battle {

/** \brief Enums for the various states that the ActionWindow class may be in
*** See the descriptions of the various views for the ActionWindow class to
*** understand what these constants represent
***
**/
enum ACTION_WINDOW_STATE {
	VIEW_INVALID = -1,
	//! Player is selecting the type of action to execute
	VIEW_ACTION_CATEGORY = 0,
	//! Player is selecting from a list of actions to execute
	VIEW_ACTION_SELECTION = 1,
	//! Player is selecting the target to execute the action on
	VIEW_TARGET_SELECTION = 2,
	//! Player is viewing information about the selected action
	VIEW_ACTION_INFORMATION = 3,
	VIEW_TOTAL = 4
};

/** \brief Enums for the various states that the FinishWindow class may be in
*** See the descriptions of the various views for the ActionWindow class to
*** understand what these constants represent
***
**/
enum FINISH_WINDOW_STATE {
	FINISH_INVALID = -1,
	//! Announces that the player is victorious and notes any characters who have gained an experience level
	FINISH_WIN_ANNOUNCE = 0,
	//! Initial display of character stats
	FINISH_WIN_SHOW_GROWTH = 1,
	//! Performs countdown of XP (adding it to chars) and triggers level ups
	FINISH_WIN_COUNTDOWN_GROWTH = 2,
	//! All XP has been added (or should be added instantly), shows final stats
	FINISH_WIN_RESOLVE_GROWTH = 3,
	//! Display of any skills learned
	FINISH_WIN_SHOW_SKILLS = 4,
	//! Reports all drunes earned and dropped items obtained
	FINISH_WIN_SHOW_SPOILS = 5,
	//! Adds $ earned to party's pot
	FINISH_WIN_COUNTDOWN_SPOILS = 6,
	//! All money and items have been added
	FINISH_WIN_RESOLVE_SPOILS = 7,
	//! We've gone through all the states of the FinishWindow in Win form
	FINISH_WIN_COMPLETE = 8,
	//! Announces that the player has lost and queries the player for an action
	FINISH_LOSE_ANNOUNCE = 9,
	//! Used to double-confirm when the player selects to quit the game or return to the main menu
	FINISH_LOSE_CONFIRM = 10,
	FINISH_TOTAL = 11
};

/** ****************************************************************************
*** \brief Represents the battle window where the player selects actions to execute.
***
*** This window is located at the bottom right hand corner of the screen. This window
*** has a constant position and size, but its inner contents can change greatly depending
*** on the context of the battle. These content views are described below.
*** 
*** - View #1: Action Category
*** In the first view, the player is presented with a list of possible action categories
*** (attack, defend, etc.). Which categories are available depends upon the current
*** character for whom the player is selection an action, and also whether or not any usable
*** items are in the party's inventory. After selecting a category, we proceed to the second
*** view.
*** 
*** - View #2: Action Selection
*** In this view, the previously selected action category is displayed along with a list of
*** the possible actions of that category and the amount of SP that they require to be used.
*** Once the player selects an action to take, we proceed to view #3. The player can also
*** hold down the MENU key, which will invoke view #4 to be displayed.
*** 
*** - View #3: Target Selection
*** This view displays information about the target that the player currently has selected
*** (an enemy or character). This information includes the target's name, attack point,
*** elemental and status effect properties, etc. Note that if the action selected targets 
*** either the entire party of characters or enemies, then this view is skipped.
***
*** - View #4: Action Information
*** This view displays detailed information about the action that is currently selected.
*** This includes the action's written description and any properties of the action that
*** would take effect if the action is to be executed.
***
*** \todo Implement cursor memory for selection of action categories, actions, targets,
*** etc.
***
*** \todo Add a function that allows you to add new action categories in the case
*** of categories available after special events
*** ***************************************************************************/
class ActionWindow : public hoa_video::MenuWindow {
	friend class hoa_battle::BattleMode;

public:
	ActionWindow();

	~ActionWindow();

	/** \brief Initializes the window so that it is ready to prepare and select another action
	*** \param character A pointer to the character whom an action is being selected for
	*** Calling this function will un-hide the menu window
	**/
	void Initialize(BattleCharacter* character);

	/** \brief Sets class members to initial state
	*** This call also hides the window from the screen. You'll need to call the Initialize
	*** function once more to show it again.
	**/
	void Reset();

	//! \brief Updates the state of the menu window and calls appropriate helper method
	void Update();

	//! \brief Draws the menu window and calls appropriate helper method
	void Draw();

	//! \name Member Access Functions
	//@{
	ACTION_WINDOW_STATE GetState() const
		{ return _state; }

	uint32 GetActionCategory() const
		{ return _selected_action_category; }

	hoa_global::GLOBAL_TARGET GetActionTargetType() const
		{ return _action_target_type; }

	bool IsActionTargetAlly() const
		{ return _action_target_ally; }

	hoa_global::GlobalSkill* GetSelectedSkill() const
		{ return _skill_list->at(_selected_action); }

	hoa_global::GlobalItem* GetSelectedItem() const
		{ return _item_list.at(_selected_action); }
	//@}
private:
	//! \brief The state that the action window is in, which reflects the contents of the window
	ACTION_WINDOW_STATE _state;

	//! \brief A pointer to the character whom is currently selected for initiating an action
	BattleCharacter* _character;

	/** \brief The action category that the player selected when he/she was at that menu
	*** The value of this member is often compared with the ACTION_TYPE constants at the top of battle.h
	**/
	uint32 _selected_action_category;

	//! \brief The action that the player selected when he/she was at the action selection menu
	uint32 _selected_action;

	//! \brief The type of target for the currently selected action (attack point, actor, or party)
	hoa_global::GLOBAL_TARGET _action_target_type;

	//! \brief If true the action should target an ally, otherwise it should target an enemy
	bool _action_target_ally;

	/** \brief Contains the list of items that are available for use in battle
	*** Each entry in here will correspond to each entry in the _action_selection_list when the
	*** player selects an item to be used. This vector is not necessarily a copy of the GlobalItem
	*** vector stored in the global inventory, since only items avaialable for use in battle are
	*** stored in here.
	**/
	std::vector<hoa_global::GlobalItem*> _item_list;

	/** \brief Contains the list of skills that are available for use in battle
	*** This is essentially a pointer to the attack, defend, or support skill set for the selected
	*** character. Skills are contained within this vector even if they can not be used because of
	*** insuffient SP available, or for any other reason.
	**/
	std::vector<hoa_global::GlobalSkill*>* _skill_list;

	/** \brief The option box that lists the types of actions that a character may take in battle
	*** Typically this list includes "attack", "defend", "support", and "item". More types may appear
	*** under special circumstances and conditions.
	**/
	hoa_video::OptionBox _action_category_list;

	//! \brief The option box that lists the actions that can be taken after a category is selected
	hoa_video::OptionBox _action_selection_list;

	//! \brief A vector containing the icons used for representing each action category
	std::vector<hoa_video::StillImage> _action_category_icons;

	//! \name TextImage objects
	//@{
	//! \brief Rendered text of "Skill     SP" as a header for the skill selection list
	hoa_video::TextImage _skill_selection_header;

	//! \brief Rendered text of "Item     QTY" as a header for the item selection list
	hoa_video::TextImage _item_selection_header;

	//! \brief Rendered text that contains information about the currently selected action
	hoa_video::TextImage _action_information;

	//! \brief Rendered text that contains information about the currently selected target
	hoa_video::TextImage _target_information;
	//@}

	// ----- Private methods

	//! \brief Sets up the action category option box and images
	void _InitActionCategoryList();

	//! \brief Sets up the selection list option box
	void _InitActionSelectionList();
	
	//! \brief Initializes Skill/SP and Item/Qty headers for the action selection lists
	void _InitSelectionHeaders();

	//! \brief Initializes action and target information text objects
	void _InitInformationText();

	//! \brief Handles update processing when the _state member is VIEW_ACTION_CATEGORY
	void _UpdateActionCategory();

	//! \brief Handles update processing when the _state member is VIEW_ACTION_SELECTION
	void _UpdateActionSelection();

	//! \brief Handles update processing when the _state member is VIEW_TARGET_SELECTION
	void _UpdateTargetSelection();

	//! \brief Handles update processing when the _state member is VIEW_ACTION_INFORMATION
	void _UpdateActionInformation();

	//! \brief Handles update processing when the _state member is VIEW_ACTION_CATEGORY
	void _DrawActionCategory();

	//! \brief Handles update processing when the _state member is VIEW_ACTION_SELECTION
	void _DrawActionSelection();

	//! \brief Handles update processing when the _state member is VIEW_TARGET_SELECTION
	void _DrawTargetSelection();

	//! \brief Handles update processing when the _state member is VIEW_ACTION_INFORMATION
	void _DrawActionInformation();

	//! \brief Reconstructs the _action_selection_list based on the current character and selected action category
	void _ConstructActionSelectionList();

	//! \brief Reconstructs the rendered text for displaying information about the selected target
	void _ConstructTargetInformation();

	//! \brief Reconstructs the rendered text for displaying information about the selected action
	void _ConstructActionInformation();
}; // class ActionWindow : public hoa_video::MenuWindow

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
class FinishWindow : public hoa_video::MenuWindow {
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

	FINISH_WINDOW_STATE GetState() const
		{ return _state; }

private:
	//! \brief The state that the window is in, which determines its contents
	FINISH_WINDOW_STATE _state;

	//! \brief Pointers to all characters who took part in the battle
	std::vector<hoa_global::GlobalCharacter*> _characters;

	//! \brief The growth members for all object pointers in the _characters table
	std::vector<hoa_global::GlobalCharacterGrowth*> _character_growths;

	//! \brief Tallies the amount of growth each character has received for each stat
	int _growth_gained[4][8];

	//! \brief Text that displays the battle's outcome (victory or defeat)
	hoa_video::TextBox _finish_outcome;

	//! \brief The list of options that the player may choose from when they lose the battle
	hoa_video::OptionBox _lose_options;

	//! \brief The window containing the XP and money won
	hoa_video::MenuWindow _xp_and_money_window;

	//! \brief The windows that show character portraits and stats
	hoa_video::MenuWindow _character_window[4];

	//! \brief Lists the items won
	hoa_video::MenuWindow _items_window;

	//! \brief Character portraits
	hoa_video::StillImage _char_portraits[4];

	//! \brief The amount of money won
	int32 _victory_money;

	//! \brief The amount of xp earned (per character)
	int32 _victory_xp;

	//! \brief Items won from battle (<ID, quantity>)
	std::map<hoa_global::GlobalObject*, int32> _victory_items;

	// ----- Private methods
	/*!
	 * \brief Creates 4 character windows
	 * \param start_x The x coordinate for the upper left corner of the window
	 * \param start_y The y coordinate for the upper left corner of the window
	 */
	void _InitCharacterWindows(float start_x, float start_y);

	/*!
	 * \brief Creates _xp_and_money_window and _items_window
	 * \param start_x The x coordinate for the upper left corner of the window
	 * \param start_y The y coordinate for the upper left corner of the window
	 */
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

#endif // __BATTLE_WINDOWS_HEADER__
