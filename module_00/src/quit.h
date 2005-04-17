/* 
 * quit.h
 *  Header file for Hero of Allacrost quit mode
 *  (C) 2005 by Tyler Olsen
 *
 *  This code is licensed under the GNU GPL. It is free software and you may modify it 
 *   and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *   for details.
 */

#ifndef __QUIT_HEADER__
#define __QUIT_HEADER__ 
 
// Partially defined namespace to avoid recursive inclusion problems.
namespace hoa_quit {
	class QuitMode;
} // namespace hoa_quit

#include <string>
#include "audio.h"
#include "video.h"
#include "global.h"
#include "boot.h"

namespace hoa_quit {

namespace local_quit {

// Some constants for different quit options the user can select in this mode
const int QUIT_GAME        = 0;
const int QUIT_TO_BOOTMENU = 1;
const int QUIT_CANCEL      = 2;

} // namespace local_quit
 
/******************************************************************************
  QuitMode Class - A mode pushed onto the stack to confirm a user's quit command

	>>>members<<<
		int quit_selected: retains what option the user currently has selected (quit game, quit to boot, or cancel)
	
	>>>functions<<<
		QuitMode(): Constructor either pauses audio or changes audio level and prepares to draw quit prompt
		~QuitMode(): The constructor does nothing
		
		Update(Uint32 time_elapsed): Processes user input. Changes the game mode stack.
		Draw(): Draws the saved screen in the background and the quit prompt text
 
	>>>notes<<<
		1) QuitMode is automatically pushed onto the game mode stack when the user presses CTRL+Q.
			Check loader.cpp if you are curious.
		
		2) QuitMode is never pushed onto the stack if the current mode is BootMode or QuitMode. If CTRL+Q
			is pressed during either one of these modes, the game exits without warning.
			
		3) THIS IS IMPORTANT! During some scenes of the game you might need the audio to be synchornized with the
			game, and so if the user tries to quit the game you want to pause the audio so the audio doesn't go out of 
			synch with the action. In order to do this, when you beginning such a scene you need to set the 
			pause_audio_on_quit member of the GameSettings class to TRUE. When you are finished with this type of scene, 
			you must must MUST remember to set this member back to FALSE. Don't forget this!!!
 *****************************************************************************/
class QuitMode : public hoa_global::GameMode {
private:
	int quit_selected;
	hoa_global::InputState* input;	
public: 
  QuitMode();
  ~QuitMode();
  
  void Update(Uint32 time_elapsed);
  void Draw();
};

} // namespace hoa_quit

#endif
