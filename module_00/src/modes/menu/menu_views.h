///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    menu_views.h
 * \author  Daniel Steuernol steu@allacrost.org
 * \brief   Header file for the different menu views.
 *
 * This code handles the different views that the user will see while the 
 * is in menu mode, (the main in-game menu). This mode's primary objectives
 * are to allow the user to view stastics about their party and manage inventory
 * and equipment.
 *
 *****************************************************************************/
 
#ifndef __MENU_VIEWS__
#define __MENU_VIEWS__

#include "utils.h"
#include <string>
#include <vector>
#include "video.h"
#include "defs.h"
#include "gui.h"
#include "global.h"
#include "menu_views.h"

//! All menu mode calls are in this namespace
namespace hoa_menu {

//! This namespace is for private menu stuff only
namespace private_menu {
}

/*!***************************************************************
 * \brief Represents a window used to select a character to perform
 *		  an action on.
 *
 * There should only ever be one of these active, usually used after
 * selecting an item in the inventory, that needs a character to work
 * on.
 *****************************************************************/
class MiniCharacterSelectWindow : public hoa_video::MenuWindow
{
private:
	//! \brief specifies if the char select window is active
	bool _char_window_active;
	//! \brief pointer to the current character that the cursor is pointing to.
	uint32 _current_char_selected;
	//! \brief the item that was selected in the inventory.
	uint32 _selected_item_index;
	//! \brief Hide copy constructor.
	MiniCharacterSelectWindow(MiniCharacterSelectWindow &other)
	{}
public:
	//! \brief CharacterWindow main constructor
	MiniCharacterSelectWindow();
	//! \brief CharacterWindow Destructor
	~MiniCharacterSelectWindow();
	//! \brief render this window to the screen.
	bool Draw();
	//! \brief change the active status of the window
	void Activate(bool new_status);
	bool IsActive() { return _char_window_active; }

	//! \brief Selected Index accessor
	//@{
	void SetSelectedIndex(uint32 selected) { _selected_item_index = selected; }
	uint32 GetSelectedIndex() { return _selected_item_index; }
	//@}


	void Update();
};

/*!***************************************************************
 * \brief Represents the Status window, displaying all the information
 *		  about the character.
 *
 * This window display all the attributes, and the equipment of a
 * character.  You can scroll through them all as well, to view all
 * the different characters.
 *****************************************************************/
class StatusWindow : public hoa_video::MenuWindow
{
private:
	//! \brief char portraits
	hoa_video::StillImage _head_portrait;
	hoa_video::StillImage _full_portrait;
	//! \brief the current character for this screen.
	hoa_global::GlobalCharacter *_current_char;
	//! \brief if the window is active or not
	bool _active;
	//! \brief current cursor position
	float _cursor_x;
	float _cursor_y;
public:
	//! \brief constructor
	StatusWindow();
	//! \brief destructor
	~StatusWindow();
	//! \brief render this window to the screen.
	bool Draw();

	//! \brief update function handles input to the window
	void Update();

	//! \brief Check if status window is active
	bool IsActive() { return _active; }
	//! \brief Active this window
	void Activate(bool new_value);
};

/*!***********************************************************************
 * \brief Represents the inventory window to browse the party's inventory
 *
 * There probably should only be one of these windows.  It will contain
 * all the necessary stuff to handle the party's inventory.
 *************************************************************************/
class InventoryWindow : public hoa_video::MenuWindow
{
private:
	//! OptionBox to display all the items
	hoa_video::OptionBox _inventory_items;
	//! Flag to specify if the inventory is active
	bool _inventory_active;
	//! The MiniCharacterWindow to be shown when needed to select a character
	MiniCharacterSelectWindow _char_window;

	//! brief Method to update the item text in the inventory items
	void UpdateItemText();
public:
	//! Class Constructor
	InventoryWindow();
	//! Class Destructor
	~InventoryWindow();

	//! Change the inventory active flag
	void Activate(bool new_status);
	bool IsActive() { return _inventory_active; }

	//! If the inventory window is ready to cancel out, or cancel out a sub-window
	bool CanCancel();
	
	void Update();

	//! \brief Draw the inventory window
	//! Takes care of drawing the inventory window to the screen.
	bool Draw();
};

/*!***********************************************************************
 * \brief Represents an individual character window for the in-game menu.
 * 
 * There should be one of these windows for each character in the game.
 * It will contain all the information to be drawn for that character, and
 * also handles the placement of this.
 *************************************************************************/
class CharacterWindow : public hoa_video::MenuWindow
{
private:
	//! The name of the character that this window corresponds(sp?) to
	uint32 _char_id;
	//! The image of the character
	hoa_video::StillImage _portrait;
public:
	/*!
	 * \brief CharacterWindow Default constructor
	 */
	CharacterWindow();
	/*!
     * \brief CharacterWindow Destructor
	 */
	~CharacterWindow();

	//! Set the character for this window
	void SetCharacter(hoa_global::GlobalCharacter *character);

	/*!
	 * \brief render this window to the screen.
	 */
	bool Draw();
};

}

#endif
