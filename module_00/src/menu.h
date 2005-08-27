///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    menu.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 12th, 2005
 * \brief   Header file for menu mode interface.
 *
 * This code handles the game event processing and frame drawing when the user
 * is in menu mode, (the main in-game menu). This mode's primary objectives
 * are to allow the user to view stastics about their party and manage inventory
 * and equipment.
 *
 * \note As you can see, this code is largely underdeveloped and this file is
 * more of a placeholder than anything useful at this point.
 *****************************************************************************/

#ifndef __MENU_HEADER__
#define __MENU_HEADER__

#include "utils.h"
#include <string>
#include <vector>
#include "video.h"
#include "defs.h"
#include "engine.h"

//! All calls to menu mode are wrapped in this namespace.
namespace hoa_menu {

//! Determines whether the code in the hoa_menu namespace should print debug statements or not.
extern bool MENU_DEBUG;

//! An internal namespace to be used only within the menu code. Don't use this namespace anywhere else!
namespace private_menu {

}

 /******************************************************************************
	MenuMode Class

	>>>members<<<

	>>>functions<<<

	>>>notes<<<
 *****************************************************************************/
class MenuMode : public hoa_engine::GameMode {
private:
	friend class hoa_data::GameData;

	hoa_video::ImageDescriptor _saved_screen;
	std::vector<hoa_video::ImageDescriptor> _menu_images;
	std::vector<hoa_audio::MusicDescriptor> _menu_music;
	std::vector<hoa_audio::SoundDescriptor> _menu_sound;
public:
	MenuMode();
	~MenuMode();

	void Reset();
	void Update(uint32 time_elapsed);
	void Draw();
};


} // namespace hoa_menu

#endif
