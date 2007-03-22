////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    pause.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for pause mode interface.
***
*** This code handles the game event processing and frame drawing when the user
*** is in pause mode (when the game is paused). In a nut-shell, all this code
*** does is save a copy of the frame being displayed when the user requests a
*** pause event, grays that image out, and then renders the text "PAUSED" while
*** waiting for the user to un-pause the game.
***
*** \note When the user enters this mode, the game will sleep for small periods
*** of time so that the game isn't using up 100% of the CPU.
*** ***************************************************************************/

#ifndef __PAUSE_HEADER__
#define __PAUSE_HEADER__

#include "utils.h"
#include "defs.h"
#include "mode_manager.h"
#include "video.h"

//! All calls to pause mode are wrapped in this namespace.
namespace hoa_pause {

//! Determines whether the code in the hoa_pause namespace should print debug statements or not.
extern bool PAUSE_DEBUG;

/*!****************************************************************************
 * \brief A mode pushed onto the game mode stack when the user pauses the game.
 *
 * This class basically saves the last frame displayed to the screen, grays it
 * out a little, and then renders the text "Paused" in the center of the screen.
 * The game remains paused until the user either presses the pause button again
 * or tries to quit the game.
 *
 * \note 1) During some scenes of the game you might need the audio to be synchronized
 * with the flow of action. If the user tries to pause the game you will want to pause 
 * the audio so the audio doesn't go out of synch with the action. In order to do this, 
 * when you begin such a scene you need to call the SetPauseVolumeAction() function of 
 * the  GameSettings class with the argument SETTINGS_PAUSE_AUDIO. When you are finished 
 * with this type of scene, you must must MUST remember to set this member back to its 
 * original value.
 *****************************************************************************/ 
class PauseMode : public hoa_mode_manager::GameMode {
private:
	//! An image of the last frame shown on the screen before PauseMode was called.
	hoa_video::StillImage _saved_screen;

	/*! \brief Retains the original volume levels when this mode is active.
	 *  Volume is restored when the mode is destroyed.
	 */
	float _saved_music_volume;
	float _saved_sound_volume;
public:
	PauseMode();
	~PauseMode();
	
	//! Resets appropriate class members. Called whenever PauseMode is made the active game mode.
	void Reset();
	//! Updates the game state by the amount of time that has elapsed
	void Update();
	//! Draws the next frame to be displayed on the screen
	void Draw();
};

} // namespace hoa_pause

#endif
