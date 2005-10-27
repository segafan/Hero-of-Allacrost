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
#include "gui.h"

//! All calls to menu mode are wrapped in this namespace.
namespace hoa_menu {

//! Determines whether the code in the hoa_menu namespace should print debug statements or not.
extern bool MENU_DEBUG;

//! An internal namespace to be used only within the menu code. Don't use this namespace anywhere else!
namespace private_menu {

}

/*!****************************************************************************
 * \brief Responsible for managing the game when executing in the main in-game menu.
 *
 * This code in this class and its respective partner classes is arguably one of the
 * most complex pieces of the game to date. Basic functionality in this class has been
 * working for a while, but we still have much work to do here (namely, integrating
 * map scripts). I intend to more fully document the primary operational features of
 * this class at a later time, but I would like to wait until it is in a more finalized
 * state before I do so.
 *
 * \note 1) If you change the state of random_encounters from false to true, make 
 * sure to set a valid value (< 0) for steps_till_encounter. *I might change this later*
 * 
 * \note 2) Be careful with calling the MapMode constructor, for it changes the coordinate 
 * system of the video engine without warning. Only create a new instance of this class if
 * you plan to immediately push it on top of the game stack.
 *****************************************************************************/
class MenuMode : public hoa_engine::GameMode {
private:
	friend class hoa_data::GameData;

	hoa_video::StaticImage _saved_screen;
	std::vector<hoa_video::StaticImage> _menu_images;
	std::vector<hoa_audio::MusicDescriptor> _menu_music;
	std::vector<hoa_audio::SoundDescriptor> _menu_sound;
	
	//! \name Main Display Windows
	//@{
	//! \brief The windows that are displayed in the top level of menu mode.
	hoa_video::MenuWindow _character_window0;
	hoa_video::MenuWindow _character_window1;
	hoa_video::MenuWindow _character_window2;
	hoa_video::MenuWindow _character_window3;
	hoa_video::MenuWindow _bottom_window;
	//@}
	
	//! \name Main Display Text Boxes
	//1{
	//! \brief The text boxes that are displayed in the top level of menu mode.
	
	//! The top level options in boot mode
	hoa_video::OptionBox _main_options;
public:
	MenuMode();
	~MenuMode();

	void Reset();
	void Update(uint32 time_elapsed);
	void Draw();
};


} // namespace hoa_menu

#endif
