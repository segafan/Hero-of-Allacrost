////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    save_mode.h
*** \author  Jacob Rudolph, rujasu@allacrost.org
*** \brief   Header file for save interface.
***
*** This code is for saving and loading.
*** ***************************************************************************/

#ifndef __SAVE_HEADER__
#define __SAVE_HEADER__

#include "utils.h"
#include "defs.h"

#include "video.h"
#include "mode_manager.h"

//! \brief All calls to save mode are wrapped in this namespace.
namespace hoa_save {

//! \brief Determines whether the code in the hoa_save namespace should print debug statements or not.
extern bool SAVE_DEBUG;

/** ****************************************************************************
*** \brief Handles saving and loading
***
*** ***************************************************************************/
class SaveMode : public hoa_mode_manager::GameMode {
public:
	SaveMode(bool enable_saving);

	~SaveMode();

	//! \brief Resets appropriate class members. Called whenever SaveMode is made the active game mode.
	void Reset();

	//! \brief Updates the game state by the amount of time that has elapsed
	void Update();

	//! \brief Draws the next frame to be displayed on the screen
	void Draw();

private:
	//! \brief Attempts to load a game. returns true on success, false on fail
	bool _LoadGame(int);

	//! \brief The MenuWindow for the backdrop
	hoa_gui::MenuWindow _window;

	//! \brief The music file to be played
	hoa_audio::MusicDescriptor _save_music;

	//! \brief Current state of SaveMode
	uint8 _current_state;

	//! \brief A screen capture of the last frame rendered on the screen before SaveMode was invoked
	hoa_video::StillImage _screen_capture;

	//! \brief The color used to dim the background screen capture image
	hoa_video::Color _dim_color;

	//! \brief The list of save/load/cancel
	hoa_gui::OptionBox _save_options;

	//! \brief The list of files to save/load from
	hoa_gui::OptionBox _file_list;

	//! \brief Tracks whether games can be saved, or only loaded.
	bool _saving_enabled;
}; // class SaveMode : public hoa_mode_manager::GameMode

} // namespace hoa_save

#endif // __SAVE_HEADER__
