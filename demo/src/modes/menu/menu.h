///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    menu.h
*** \author  Daniel Steuernol steu@allacrost.org
*** \brief   Header file for menu mode interface.
***
*** This code handles the game event processing and frame drawing when the user
*** is in menu mode (the main in-game menu). This mode's primary objectives are
*** to allow the user to view stastics about their party and manage inventory
*** and equipment.
*** ***************************************************************************/

#ifndef __MENU_HEADER__
#define __MENU_HEADER__

#include <string>
#include <vector>

#include "utils.h"
#include "defs.h"

#include "video.h"
#include "gui.h"
#include "mode_manager.h"
#include "global.h"

#include "menu_views.h"

//! All calls to menu mode are wrapped in this namespace.
namespace hoa_menu {

//! Determines whether the code in the hoa_menu namespace should print debug statements or not.
extern bool MENU_DEBUG;

//! An internal namespace to be used only within the menu code. Don't use this namespace anywhere else!
namespace private_menu {

//! \name Main Options Constants
//@{
const uint32 MAIN_INVENTORY      = 0;
const uint32 MAIN_SKILLS         = 1;
const uint32 MAIN_STATUS_EQUIP   = 2;
const uint32 MAIN_OPTIONS        = 3;
const uint32 MAIN_SAVE           = 4;
const uint32 MAIN_EXIT           = 5;
const uint32 MAIN_SIZE           = 6;
//@}

//! \name Inventory Menu Options Constants
//@{
const uint32 INV_USE    = 0;
const uint32 INV_SORT   = 1;
const uint32 INV_CANCEL = 2;
const uint32 INV_SIZE   = 3;
//@}

//! \name Skills Menu Options Constants
//@{
const uint32 SKILLS_CANCEL  = 0;
const uint32 SKILLS_SIZE    = 1;
//@}

//! \name Equipment Menu Options Constants
//@{
const uint32 STATUS_EQUIP_EQUIP   = 0;
const uint32 STATUS_EQUIP_REMOVE  = 1;
const uint32 STATUS_EQUIP_NEXT    = 2;
const uint32 STATUS_EQUIP_PREV    = 3;
const uint32 STATUS_EQUIP_CANCEL  = 4;
const uint32 STATUS_EQUIP_SIZE    = 5;
//@}

//! \name Options Menu Options Constants
//@{
const uint32 OPTIONS_EDIT    = 0;
const uint32 OPTIONS_SAVE    = 1;
const uint32 OPTIONS_CANCEL  = 2;
const uint32 OPTIONS_SIZE    = 3;
//@}

//! \name Save Menu Options Constants
//@{
const uint32 SAVE_SAVE    = 0;
const uint32 SAVE_CANCEL  = 1;
const uint32 SAVE_SIZE    = 2;
//@}

//! \name MenuMode OptionBox Show Flags
//! \brief Constants used to determine which option box is currently showing.
//@{
const uint32 SHOW_MAIN          = 0;
const uint32 SHOW_INVENTORY     = 1;
const uint32 SHOW_SKILLS        = 2;
const uint32 SHOW_STATUS_EQUIP  = 3;
const uint32 SHOW_OPTIONS       = 4;
const uint32 SHOW_SAVE          = 5;
//@}

} // namespace private_menu

/** ****************************************************************************
*** \brief Handles game executing while in the main in-game menu.
***
*** This mode of game operation allows the player to examine and manage their
*** party, inventory, options, and save their game.
***
*** \note MenuMode is always entered from an instance of MapMode. However, there
*** may be certain conditions where MenuMode is entered from other game modes.
***
*** \note MenuMode does not play its own music, but rather it continues playing
*** music from the previous GameMode that created it.
*** ***************************************************************************/
class MenuMode : public hoa_mode_manager::GameMode {
public:
	MenuMode();
	~MenuMode();

	void Reset();
	void Update();
	void Draw();

private:
	/** \brief Retains a snap-shot of the screen just prior to when menu mode was entered
	*** This image is perpetually drawn as the background while in menu mode
	**/
	hoa_video::StillImage _saved_screen;
	//! A graphic for the location (map) that the player is currently on
	hoa_video::StillImage _location_picture;

	std::vector<hoa_video::StillImage> _menu_images;
	//! \brief the sounds for MenuMode
	std::map<std::string, hoa_audio::SoundDescriptor> _menu_sounds;

	/** \name Main Display Windows
	*** \brief The windows that are displayed in the menu mode.
	**/
	//@{
	hoa_video::MenuWindow _bottom_window;
	private_menu::CharacterWindow _character_window0;
	private_menu::CharacterWindow _character_window1;
	private_menu::CharacterWindow _character_window2;
	private_menu::CharacterWindow _character_window3;
	private_menu::InventoryWindow _inventory_window;
	private_menu::StatusWindow _status_window;
	//@}
		
	//! \brief The current option box to display
	uint32 _current_menu_showing;
	
	//! A pointer to the current options menu
	hoa_video::OptionBox *_current_menu;
	
	//! The top level options in boot mode
	hoa_video::OptionBox _main_options;

	//! \name Sub-menu option boxes
	//@{
	hoa_video::OptionBox _menu_inventory;
	hoa_video::OptionBox _menu_skills;
	hoa_video::OptionBox _menu_status_equip;
	hoa_video::OptionBox _menu_options;
	hoa_video::OptionBox _menu_save;
	//@}
	
	//! \brief Functions to set up the option boxes
	//@{
	void _SetupOptionBoxCommonSettings(hoa_video::OptionBox *ob);
	void _SetupMainOptionBox();
	void _SetupInventoryOptionBox();
	void _SetupSkillsOptionBox();
	void _SetupStatusEquipOptionBox();
	void _SetupOptionsOptionBox();
	void _SetupSaveOptionBox();
	//@}

	/** \name Menu Handle Functions
	*** \brief Handler functions to deal with events for all the different menus
	**/
	//@{
	void _HandleMainMenu();
	void _HandleInventoryMenu();
	void _HandleSkillsMenu();
	void _HandleStatusEquipMenu();
	void _HandleOptionsMenu();
	void _HandleSaveMenu();
	//@}

	//! \brief Draws the bottom part of the menu mode.
	void _DrawBottomMenu();
};

} // namespace hoa_menu

#endif
