///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    pause.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 12th, 2005
 * \brief   Header file for pause mode interface.
 *
 * This code handles the game event processing and frame drawing when the user
 * is in pause mode (when the game is paused). In a nut-shell, all this code
 * does is save a copy of the frame being displayed when the user requests a
 * pause event, grays that image out, and then renders the text "PAUSED" while
 * waiting for the user to un-pause the game.
 *
 * \note I plan to make it so that when the user enters this mode, the game
 * will sleep every 100ms or so so that the game isn't using up 100% of the CPU
 * resources.
 *****************************************************************************/

#ifndef __PAUSED_HEADER__
#define __PAUSED_HEADER__

#include "utils.h"
#include <string>
#include "defs.h"
#include "engine.h"
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
 * the  GameSettings class with the argument ENGINE_PAUSE_AUDIO. When you are finished 
 * with this type of scene, you must must MUST remember to set this member back to its 
 * original value.
 *****************************************************************************/ 
class PauseMode : public hoa_engine::GameMode {
private:
	//! An image of the last frame shown on the screen before PauseMode was called.
	hoa_video::ImageDescriptor saved_screen;
public:
	PauseMode();
	~PauseMode();
	
	//! Updates the game state by the amount of time that has elapsed
	void Update(uint32 time_elapsed);
	//! Draws the next frame to be displayed on the screen
	void Draw();
};

} // namespace hoa_pause

#endif
