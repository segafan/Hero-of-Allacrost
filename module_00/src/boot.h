/* 
 * hoa_boot.h
 *	Header file for Hero of Allacrost boot menu
 *	(C) 2004 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */
 
#ifndef __BOOT_HEADER__
#define __BOOT_HEADER__ 
 
#include "utils.h"
#include <string>
#include <vector>
#include "SDL.h"
#include "defs.h"
#include "engine.h"

namespace hoa_boot {

extern bool BOOT_DEBUG;

// Used to cycle thru the boot menus
const int NEW_MENU     = 0;
const int LOAD_MENU    = 1;
const int OPTIONS_MENU = 2;
const int CREDITS_MENU = 3;
const int HIDE_MENU    = 4;
const int QUIT_MENU    = 5;

const int VIDEO_OP     = 0;
const int AUDIO_OP     = 1;
const int LANGUAGE_OP  = 2;
const int KEYS_OP      = 3;
const int JOYSTICK_OP  = 4;



// Work on this and where it will go exactly
typedef struct SavedGameDescriptor {
	std::string map_name;
	unsigned int money;
	// a class/struct for keeping the time
} SavedGameDescriptor;



/******************************************************************************
 * BootMode Class
 *
 *	members: menu_hidden: if true code only draws the background and no menus/logo
 *					 vmenu_index: a vector storing menu pointers in a stack-like structure
 *
 * functions: void LoadBootImages():
 *							Loads all images used in the boot screen
 *						void LoadBootAudio(): 
 *							Loads all music and sounds used in the boot screen and plays the opening
 *						SDLKey RedefineKey():
 *							Toggles current_menu hidden value and reloads screen
 *
 *						void EventHandler(SDL_Event event):
 *							Handles user input events. This function will only be called in BOOT_MODE
 *
 * notes:
 *****************************************************************************/
class BootMode : public hoa_engine::GameMode {
private:
	bool menu_hidden;
	std::vector<int> vmenu_index;
	std::vector<hoa_audio::MusicDescriptor> boot_music;
	std::vector<hoa_audio::SoundDescriptor> boot_sound;
	std::vector<hoa_video::ImageDescriptor> boot_images;
	
	void AnimateLogo();
	
	void RedefineKey(SDLKey& change_key);
	
	void UpdateNewMenu();
	void UpdateLoadMenu(); 
	void UpdateOptionsMenu();
	void UpdateCreditsMenu();
	void UpdateHideMenu();
	void UpdateQuitMenu();	
	
	void UpdateVideoOptions();
	void UpdateAudioOptions();
	void UpdateLanguageOptions();
	void UpdateKeyOptions();
	void UpdateJoystickOptions();
		
	void DrawLoadMenu();
	void DrawLoadGame();
	void DrawVideoOptions();
	void DrawAudioOptions();
	void DrawLanguageOptions();
	void DrawKeyOptions();
	void DrawJoystickOptions(); 
	void DrawCredits();
public: 
	BootMode();
	~BootMode();
	
	void Update(Uint32 time_elapsed);
	void Draw();
};

} // namespace hoa_boot

#endif
