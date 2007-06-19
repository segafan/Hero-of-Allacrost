////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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


#include "battle_actors.h"

namespace hoa_battle {

namespace private_battle {

/** \brief Enums for the various states that the ActionWindow class may be in
*** See the descriptions of the various views for the ActionWindow class to
*** understand what these constants represent
***
**/
enum ACTION_WINDOW_VIEWS {
	VIEW_INVALID = -1,
	VIEW_ACTION_CATEGORY = 0,
	VIEW_ACTION_SELECTION = 1,
	VIEW_TARGET_SELECTION = 2,
	VIEW_ACTION_INFORMATION = 3,
	VIEW_TOTAL = 4
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
	friend class BattleMode;
public:
	ActionWindow();

	~ActionWindow();

	/** \brief Initializes the window so that it is ready to prepare and select another action
	*** \param character A pointer to the character whom an action is being selected for
	*** Calling this function will un-hide the menu window
	**/
	void Initialize(BattleCharacterActor* character);

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
	ACTION_WINDOW_VIEWS GetState() const
		{ return _state; }

	uint32 GetActionCategory() const
		{ return _selected_action_category; }

	hoa_global::GLOBAL_TARGET GetActionTargetType() const
		{ return _action_target_type; }

	hoa_global::GLOBAL_ALIGNMENT GetActionAlignmentType() const
		{ return _action_alignment_type; }

	hoa_global::GlobalSkill* GetSelectedSkill() const
		{ return _skill_list->at(_selected_action); }

	hoa_global::GlobalItem* GetSelectedItem() const
		{ return _item_list[_selected_action]; }
	//@}
private:
	//! \brief The state that the action window is in, which reflects the contents of the window
	ACTION_WINDOW_VIEWS _state;

	//! \brief A pointer to the character whom is currently selected for initiating an action
	BattleCharacterActor* _character;

	/** \brief The action category that the player selected when he/she was at that menu
	*** The value of this member is often compared with the ACTION_TYPE constants at the top of battle.h
	**/
	uint32 _selected_action_category;

	//! \brief The action that the player selected when he/she was at the action selection menu
	uint32 _selected_action;

	//! \brief The type of target for the currently selected action (attack point, actor, or party)
	hoa_global::GLOBAL_TARGET _action_target_type;

	//! \brief The type of alignment for the currently selected action (friend, foe, or neutral)
	hoa_global::GLOBAL_ALIGNMENT _action_alignment_type;

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

	// TODO: implement when rendered text system is more mature
/*
	/ ** \brief A string of text which serves as a list header for the action selection screen
	*** This pointer is set to either _skill_selection_header or _item_selection_header
	** /
	hoa_video::RenderedString* _action_selection_header;

	//! \brief Rendered text of "Skill     SP" as a header for the skill selection list
	hoa_video::RenderedString* _skill_selection_header;

	//! \brief Rendered text of "Item     QTY" as a header for the item selection list
	hoa_video::RenderedString* _item_selection_header;
*/

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
}; // class ActionWindow : public hoa_video::MenuWindow

/** ****************************************************************************
*** \brief The window displayed once a battle has either been won or lost
***
*** This window is located in the center of the screen and only appears when a victor
*** has been decided in the battle. The contents of this window differ depending on
*** whether the battle was victorious or a loss.
*** ***************************************************************************/
class FinishWindow : public hoa_video::MenuWindow {
	friend class BattleMode;
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

private:
	//! \brief Text that displays "VICTORY" or "DEFEAT" depending upon the battle's outcome
// 	hoa_video::RenderedText* _finish_status;

	//! \brief The list of options that the player may choose from when they lose the battle
	hoa_video::OptionBox _lose_options;
}; // class FinishWindow : public hoa_video:MenuWindow

} // namespace private_battle

} // namespace hoa_battle

#endif // __BATTLE_WINDOWS_HEADER__
