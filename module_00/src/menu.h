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
#include "defs.h"
#include "engine.h"

namespace hoa_menu {

extern bool MENU_DEBUG;

namespace local_menu {

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

	std::vector<hoa_video::ImageDescriptor> menu_images;
	std::vector<hoa_audio::MusicDescriptor> menu_music;
	std::vector<hoa_audio::SoundDescriptor> menu_sound;
public:
	MenuMode();
	~MenuMode();

	void Update(Uint32 time_elapsed);
	void Draw();
};


} // namespace hoa_menu

#endif
