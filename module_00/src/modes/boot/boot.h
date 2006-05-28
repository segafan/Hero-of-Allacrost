///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    boot.h
 * \author  Viljami Korhonen, mindflayer@allacrost.org
 * \brief   Header file for boot mode interface.
 *
 * This code handles the game event processing and frame drawing when the user
 * is in boot mode (the boot screen and menus). It inherits from the GameMode class
 * and there will only ever be one BattleMode inside the game stack at any one time.
 *****************************************************************************/

#ifndef __BOOT_HEADER__
#define __BOOT_HEADER__

#include "utils.h"
#include <string>
#include <vector>
#include "defs.h"
#include "mode_manager.h"
#include "video.h"
#include "boot_menu.h"
#include "boot_credits.h"

//! All calls to boot mode are wrapped in this namespace.
namespace hoa_boot {

//! Determines whether the code in the hoa_boot namespace should print debug statements or not.
extern bool BOOT_DEBUG;

//! An internal namespace to be used only within the boot code. Don't use this namespace anywhere else!
namespace private_boot {


//! \name Default key settings
//! \brief The default SDL key mapping used with Allacrost.
//@{
const uint32 UP_KEY_DEFAULT           = SDLK_UP;
const uint32 DOWN_KEY_DEFAULT         = SDLK_DOWN;
const uint32 LEFT_KEY_DEFAULT         = SDLK_LEFT;
const uint32 RIGHT_KEY_DEFAULT        = SDLK_RIGHT;
const uint32 CONFIRM_KEY_DEFAULT      = SDLK_f;
const uint32 CANCEL_KEY_DEFAULT       = SDLK_d;
const uint32 MENU_KEY_DEFAULT         = SDLK_s;
const uint32 SWAP_KEY_DEFAULT         = SDLK_a;
const uint32 PAUSE_KEY_DEFAULT        = SDLK_SPACE;
const uint32 LEFT_SELECT_KEY_DEFAULT  = SDLK_w;
const uint32 RIGHT_SELECT_KEY_DEFAULT = SDLK_e;
//@}

} // namespace private_boot

/*!****************************************************************************
 * \brief Handles everything that needs to be done on the game's boot screen.
 *
 * This is the first mode that is pushed onto the game stack when the program starts.
 * Because the user can set various game settings from this game mode, it has a
 * heavy amount of interaction with the GameSettings class.
 *
 * \note Although the basic features are functional in this code, much remains
 * to be done. The reason it hasn't been done before was primarily our lack of
 * a GUI + text rendering, but now that that requirement has been met this code
 * should see a lot of development in the near future.
 *****************************************************************************/
class BootMode : public hoa_mode_manager::GameMode {
private:
	//! If true, boot mode is exiting and we have to wait for the screen to finish fading out.
	bool _fade_out;

	//! If true, the logo is animating (sword flying and so on...)
	static bool _logo_animating;

	//! Music pieces to be used at the boot screen.
	std::vector<hoa_audio::MusicDescriptor> _boot_music;
	//! Sounds that will be used at the boot screen.
	std::vector<hoa_audio::SoundDescriptor> _boot_sounds;
	//! Images that will be used at the boot screen.
	std::vector<hoa_video::StillImage> _boot_images;

	//! Credits screen window
	CreditsScreen _credits_screen;
	
	//! A pointer to the current menu visible
	BootMenu * _current_menu;

	//! Main menu
	BootMenu _main_menu;

	//! 'Options' menu
	BootMenu _options_menu;

	//! 'Video Options' menu
	BootMenu _video_options_menu;

	//! 'Audio Options' menu
	BootMenu _audio_options_menu;

	
	/*!
	 *  \brief Animates the game logo when this class is first initialized.
	 *
	 *  The logo animation sequences:
	 *  1) When game first starts up, screen is totally black
     *
	 *  2) Logo gradually fades in from the background to appear on 
	 *     the center of the screen, with the sword placed horizontally
	 *	   as if it is "sheathed" inside the word Allacros
     *
     *  3) After logo fade in is complete, the sword slides out (unsheathes)
	 *     and moves to the right
	 *
     *  4) After sword is completely removed, it moves upwards and makes 
	 *     full 360 degree swings a few times (as if an invisible person was 
	 *	   swinging/twirling it) with powerful "woosh" sounds cutting the air.
     *
     *  5) Sword slows down, then comes crashing into the logo (in its
	 *     default vertical position) with a loud ka-ching
	 *  
	 *  6) A brilliant flash of white light eminates from the sword along 
	 *     with the ka-ching sound, quickly whiting out the entire screen
     *
	 *  7) The light fades away, revealing the desert background image 
	 *	   (instead of a blank screen) and the logo is now at the top 
	 *	   center of the screen.
     *
     *  8) Finally, the copyright text at the very bottom of the 
	 *     screen appears, along with the boot menu (New Game, etc.) 
	 */
	void _AnimateLogo();

	//! Draws background image, logo and sword at their default locations
	void _DrawBackgroundItems();

	//! Stops playback of the opening animation
	void _EndOpeningAnimation();


	/*!
	 *  \brief Redefines a key to be mapped to another command.
	 *  \param &change_key The key to be re-mapped.
	 */
	void _RedefineKey(SDLKey& change_key);

	//! Inits the main menu
	void _SetupMainMenu();
	//! Inits the options menu
	void _SetupOptionsMenu();
	//! Inits the video-options menu
	void _SetupVideoOptionsMenu();
	//! Inits the audio-options menu
	void _SetupAudioOptionsMenu();

	// Main Menu handlers
	//! 'New Game' confirmed
	void _OnNewGame();
	//! 'Load Game' confirmed
	void _OnLoadGame();
	//! 'Options' confirmed
	void _OnOptions();
	//! 'Credits' confirmed
	void _OnCredits();
	//! 'Quit' confirmed
	void _OnQuit();

	// Options' handlers
	//! 'Video' confirmed
	void _OnVideoOptions();
	//! 'Audio' confirmed
	void _OnAudioOptions();

	//! 'Video mode' confirmed
	void _OnVideoMode();

	//! Sound volume down
	void _OnSoundLeft();
	//! Sound volume up
	void _OnSoundRight();
	//! Music volume down
	void _OnMusicLeft();
	//! Music volume up
	void _OnMusicRight();


	//! Updates the video options screen
	void _UpdateVideoOptions();

	//! Updates the audio options screen
	void _UpdateAudioOptions();

public:
	//! Initializes class members and loads media data.
	BootMode();
	//! Frees all media data (images and audio).
	~BootMode();

	//! Resets appropriate class members. Called whenever BootMode is made the active game mode.
	void Reset();
	//! Wrapper function that calls different update functions depending on the menu state.
	void Update();
	//! Wrapper function that calls different draw functions depending on the menu state.
	void Draw();
	
};

} // namespace hoa_boot

#endif
