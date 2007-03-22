///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    boot_menu.h
 * \author  Viljami Korhonen, mindflayer@allacrost.org
 * \brief   Header file for the boot-mode menu
 *
 * This code extends some parts of the OptionBox to match the
 * requirements of the boot menu. This includes features like multi-depth 
 * menus and custom visualization as well as custom event handling.
 * However, no inheritance was used as I found it to be more of a nuisance
 * than benefit in here!
 *****************************************************************************/
 
#ifndef __BOOT_MENU__
#define __BOOT_MENU__

#include "utils.h"
#include <string>
#include <vector>
#include "video.h"
#include "defs.h"


//! All calls to boot mode are wrapped in this namespace.
namespace hoa_boot {

/*!****************************************************************************
 *  \brief The BootMenu-class will help in creation of the boot-menu.
 *   All of the boot-menu functions are given via function pointers to the class.
 *****************************************************************************/
class BootMenu
{
public:
	BootMenu(BootMenu * parent = 0, bool windowed = false, BootMode * boot_mode = 0);

	~BootMenu();
	
	/** \brief Adds a new menu option with the desired function attached to it
	*** \param text A text representing the new option
	*** \param *confirm_function 'Confirm' handler from the BootMode
	*** \param *left_function 'Left' handler from the BootMode
	*** \param *right_function 'Right' handler from the BootMode
	*** \param *up_function 'Up' handler from the BootMode
	*** \param *down_function 'Down' handler from the BootMode
	**/
	void AddOption(const hoa_utils::ustring & text, void (BootMode::*confirm_function)() = 0, void (BootMode::*left_function)() = 0, void (BootMode::*right_function)() = 0, void (BootMode::*up_function)() = 0, void (BootMode::*down_function)() = 0);

	/** \brief Changes text of the option
	*** \param index index of the text to be changed
	*** \param text new text for the option
	**/
	void SetOptionText(uint32 index, const hoa_utils::ustring & text);

	/** \brief Enables or disables the option with the given index
	*** \param index index of the option to be disabled
	*** \param enable enable or disable the option
	**/
	void EnableOption(int32 index, bool enable);

	/** \brief Toggles this menu to be windowed (or not windowed)
	*** \param windowed true == enable windowing
	**/
	void SetWindowed(bool windowed);

	/** \brief Sets a new parent for this menu
	*** \param parent Pointer to the new parent
	**/
	void SetParent(BootMenu * parent);

	/** \brief Sets new text density
	*** \param density A new density value
	**/
	void SetTextDensity(float density);

	/** \brief Updates menu events. Call this every frame just like you would do on OptionBox!
	*** \return The event code from OptionBox::GetEvent()
	**/
	int32 GetEvent();

	/** \brief Returns parent of the menu
	*** \return Parent of the menu (or 0 for the root menu)
	**/
	BootMenu * GetParent() const;

	/** \brief Returns boolean if the menu is windowed or not
	*** \return True if windowed. False if not
	**/
	bool IsWindowed() const;

	/**
	*** \brief Returns true if the currently selected option is enabled and a confirm handler is available
	**/
	bool IsSelectionEnabled() const;

	//! Draws menu on the screen
	bool Draw();

	//! Handles the confirm key
	void ConfirmPressed();

	//! Handles the left key
	void LeftPressed();

	//! Handles the right key
	void RightPressed();

	//! Handles the up key
	void UpPressed();

	//! Handles the down key
	void DownPressed();

	//! Handles the cancel key
	void CancelPressed();


	/** \brief Updates the menu window
	*** \param frameTime time of the frame in ms
	**/
	static void UpdateWindow(int32 frame_time);

	/** \brief Shows or hides the menu window
	*** \param toggle menu window on or off
	**/
	static void ShowWindow(bool toggle);


private:
	//! Inits the menu window, called in the constructor of the class
	static void _InitWindow();


	//! A pointer to the boot mode
	static BootMode * _boot_mode;

	//! A pointer to the parent menu (or 0 for the root menu)
	BootMenu * _parent_menu;

	//! OptionBox displayed on the screen
	hoa_video::OptionBox _current_menu;


	//! Is this menu windowed (and thus vertically aligned) or not
	bool _is_windowed;

	//! A window needed by some menus
	static hoa_video::MenuWindow * _menu_window;

	//! confirm-key handlers for all options in the menu
	std::vector<void (BootMode::*)()> _confirm_handlers;

	//! left-key handlers for all options in the menu
	std::vector<void (BootMode::*)()> _left_handlers;

	//! right-key handlers for all options in the menu
	std::vector<void (BootMode::*)()> _right_handlers;

	//! up-key handlers for all options in the menu
	std::vector<void (BootMode::*)()> _up_handlers;

	//! down-key handlers for all options in the menu
	std::vector<void (BootMode::*)()> _down_handlers;

}; // end class BootMenu


}

#endif
