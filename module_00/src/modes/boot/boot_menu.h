///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
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
 *****************************************************************************/
 
#ifndef __BOOT_MENU__
#define __BOOT_MENU__

#include "utils.h"
#include <string>
#include <vector>
#include "video.h"
#include "defs.h"
#include "mode_manager.h"
#include "gui.h"
#include "global.h"
#include "menu_views.h"

//! All calls to boot mode are wrapped in this namespace.
namespace hoa_boot {

/*!****************************************************************************
 *  \brief The BootMenu-class will help in creation of the boot-menu.
 *   All of the boot-menu functions are given via function pointers to the class.
 *****************************************************************************/
class BootMenu
{
public:
	BootMenu(BootMenu * parent = 0);
	~BootMenu();

	//! Adds a new option with the desired function attached to it
	void AddOption(const hoa_utils::ustring & text, void (*function)());

private:
	//! Possible parent-menu (always 0 for the root-menu!)
	BootMenu * _parent;

	//! Current OptionBox displaying
	hoa_video::OptionBox _current_menu;

	//! Child menus for the menu options (or 0 if no child is available from the option!)
	std::vector<BootMenu *> _childs;

	//! confirm-key handlers for all options. These are given by the boot-mode.
	std::vector<void (*)()> _confirm_handlers;

}; // end class BootMenu


}

#endif
