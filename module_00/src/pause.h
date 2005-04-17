/* 
 * pause.h
 *  Header file for Hero of Allacrost paused mode
 *  (C) 2005 by Tyler Olsen
 *
 *  This code is licensed under the GNU GPL. It is free software and you may modify it 
 *   and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *   for details.
 */

#ifndef __PAUSED_HEADER__
#define __PAUSED_HEADER__ 

// Partially defined namespace to avoid recursive inclusion problems.
namespace hoa_pause {
	class PauseMode;
} // namespace hoa_pause
 
#include <string>
#include "audio.h"
#include "video.h"
#include "global.h"

namespace hoa_pause {
 
/******************************************************************************
  PauseMode Class - A mode pushed onto the stack when the user pauses the game

	>>>members<<<
		Just some references to some singleton classes.
 
	>>>functions<<<
		PausedMode(): Constructor either pauses audio or changes audio level and displays "Paused" on screen
		~PausedMode(): Constructor does nothing
		
		void Update(Uint32 time_elapsed): Checks to see if the user has requested to unpause the game
		void Draw(): This function does nothing, but is necessary since all game modes must have this function
 
	>>>notes<<<
		1) THIS IS IMPORTANT! During some scenes of the game you might need the audio to be synchornized with the
			game, and so if the user pauses the game you want to pause the audio so the audio doesn't go out of synch
			with the action. In order to do this, when you beginning such a scene you need to set the
			pause_vol_type member of the GameSettings class to GLOBAL_PAUSE_VOLUME_ON_PAUSE. When you are finished with
			this type of scene, you must must MUST remember to restore this member back to it's default value. Don't
			forget this!!!
 *****************************************************************************/
class PauseMode : public hoa_global::GameMode {
private:
	hoa_global::InputState* input;	
public: 
  PauseMode();
  ~PauseMode();
  
  void Update(Uint32 time_elapsed);
  void Draw();
};

} // namespace hoa_pause

#endif
