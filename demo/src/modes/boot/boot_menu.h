///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    boot_menu.h
*** \author  Viljami Korhonen, mindflayer@allacrost.org
*** \brief   Header file for the boot-mode menu
***
*** This code extends some parts of the OptionBox class to match the
*** requirements of boot menus. This includes features like multi-depth
*** menus and custom visualization as well as custom event handling.
*** However, no inheritance was used as I found it to be more of a nuisance
*** than benefit in here!
*** ***************************************************************************/

#ifndef __BOOT_MENU__
#define __BOOT_MENU__

#include "utils.h"
#include "defs.h"

#include "video.h"

namespace hoa_boot {

/** ****************************************************************************
*** \brief The BootMenu-class will help in creation of the boot-menu.
*** All of the boot-menu functions are given via function pointers to the class.
*** ***************************************************************************/
class BootMenu : public hoa_video::OptionBox {
public:
	BootMenu()
		{}

	~BootMenu()
		{}

	/** \brief Pointer to the active boot mode
	*** This is guaranteed to be a valid pointer and is set in the BootMode::Reset() function
	**/
	static BootMode* active_boot_mode;

	/** \brief Adds a new menu option with the desired function attached to it
	*** \param text A text representing the new option
	*** \param *confirm_function 'Confirm' handler from the BootMode
	*** \param *left_function 'Left' handler from the BootMode
	*** \param *right_function 'Right' handler from the BootMode
	*** \param *up_function 'Up' handler from the BootMode
	*** \param *down_function 'Down' handler from the BootMode
	**/
	void AddOption(const hoa_utils::ustring & text, void (BootMode::*confirm_function)() = 0, void (BootMode::*left_function)() = 0,
		void (BootMode::*right_function)() = 0, void (BootMode::*up_function)() = 0, void (BootMode::*down_function)() = 0);

	void InputConfirm();

	void InputUp();

	void InputDown();

	void InputLeft();

	void InputRight();

	void InputCancel();

private:
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
}; // class BootMenu : public hoa_video::OptionBox

}

#endif // __BOOT_MENU__
