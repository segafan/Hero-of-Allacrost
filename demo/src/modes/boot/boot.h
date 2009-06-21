///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    boot.h
*** \author  Viljami Korhonen, mindflayer@allacrost.org
*** \brief   Header file for boot mode interface.
***
*** This code handles the game event processing and frame drawing when the user
*** is in boot mode (the boot screen and menus).
*** ***************************************************************************/

#ifndef __BOOT_HEADER__
#define __BOOT_HEADER__

#include <string>
#include <vector>

#include "utils.h"
#include "defs.h"

#include "video.h"
#include "mode_manager.h"

#include "boot_menu.h"
#include "boot_credits.h"
#include "boot_welcome.h"

#include "menu_views.h"

//! All calls to boot mode are wrapped in this namespace.
namespace hoa_boot {

//! Determines whether the code in the hoa_boot namespace should print debug statements or not.
extern bool BOOT_DEBUG;

//! An internal namespace to be used only within the boot code. Don't use this namespace anywhere else!
namespace private_boot {


enum BOOT_STATE {
	BOOT_INVALID = 0,
	BOOT_INTRO   = 1,
	BOOT_MAIN    = 2,
	BOOT_LOAD    = 3,
	BOOT_OPTIONS = 4,
	BOOT_CREDITS = 5,
	BOOT_TOTAL   = 6
};

enum WAIT_FOR {
	WAIT_KEY,
	WAIT_JOY_BUTTON,
	WAIT_JOY_AXIS
};

} // namespace private_boot

/** ****************************************************************************
*** \brief Handles everything that needs to be done on the game's boot screen.
***
*** This is the first mode that is pushed onto the game stack when the program starts.
*** Because the user can set various game settings from this game mode, it has a
*** heavy amount of interaction with the game engine classes.
***
*** ***************************************************************************/
class BootMode : public hoa_mode_manager::GameMode {
	friend class BootMenu;

public:
	BootMode();

	~BootMode();

	//! Resets appropriate class members. Called whenever BootMode is made the active game mode.
	void Reset();

	//! Wrapper function that calls different update functions depending on the menu state.
	void Update();

	//! Wrapper function that calls different draw functions depending on the menu state.
	void Draw();

private:
	//! If true, the logo is animating (sword flying and so on...)
	static bool _initial_entry;

	//! If true, boot mode is exiting and we have to wait for the screen to finish fading out.
	bool _fade_out;

	//! Music pieces to be used at the boot screen.
	std::vector<hoa_audio::MusicDescriptor> _boot_music;
	//! Sounds that will be used at the boot screen.
	std::vector<hoa_audio::SoundDescriptor> _boot_sounds;
	//! Images that will be used at the boot screen.
	std::vector<hoa_video::StillImage> _boot_images;

	//! Credits screen window
	private_boot::CreditsScreen _credits_screen;

	//! Welcome screen window
	WelcomeScreen _welcome_screen;

	//! \brief Pointer to the active boot menu
	BootMenu* _active_menu;

	//!
	hoa_video::MenuWindow _menu_window;

	//! Main menu
	BootMenu _main_menu;

	//! 'Options' menu
	BootMenu _options_menu;

	//! 'Video Options' menu
	BootMenu _video_options_menu;

	//! 'Audio Options' menu
	BootMenu _audio_options_menu;

	//! 'Language' menu
	BootMenu _language_options_menu;

	//! 'Key Settings' menu
	BootMenu _key_settings_menu;

	//! 'Joystick Settings' menu
	BootMenu _joy_settings_menu;

	//! 'Load Profile' menu
	BootMenu _load_profile_menu;

	//! 'Save Profile' Menu
	BootMenu _save_profile_menu;

	//! 'Profiles' Menu
	BootMenu _profiles_menu;

	//! 'User Input' Menu
	BootMenu _user_input_menu;

	//! The function to call when a new key has been pressed (if we're waiting for one.)
	void (BootMode::*_key_setting_function)(const SDLKey &);

	//! The function to call when a new joystick button has been pressed (if we're waiting for one.)
	void (BootMode::*_joy_setting_function)(uint8 button);

	//! The function to call when a new joystick axis has been moved (if we're waiting for one.)
	void (BootMode::*_joy_axis_setting_function)(int8 axis);

	//! The function to call when we want to overwrite 
	void (BootMode::*_overwrite_function) ();

	//! Displays the select a key window.
	hoa_menu::MessageWindow _message_window;

	//! Displays the please type file name window
	hoa_menu::MessageWindow _file_name_alert;

	//! Displays the actual filename being typed
	hoa_menu::MessageWindow _file_name_window;

	//! 'Resolution switcher' menu
	BootMenu _resolution_menu;

	//! Latest version according to version check
	bool _latest_version;

	//! Has the user modified game settings?
	bool _has_modified_settings;

	//! If this isn't the latest version, what is?
	std::string _latest_version_number;

	//! Make sure we store the filename currently being typed
	std::string _current_filename;



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

	//! Waits infinitely for a key press
	SDLKey _WaitKeyPress();

	//! Waits infinitely for a joystick press
	uint8 _WaitJoyPress();

	/**
	*** \brief Redefines a key to be mapped to another command. Waits for keypress using _WaitKeyPress()
	**/
	//@{
	void _RedefineUpKey();
	void _RedefineDownKey();
	void _RedefineLeftKey();
	void _RedefineRightKey();
	void _RedefineConfirmKey();
	void _RedefineCancelKey();
	void _RedefineMenuKey();
	void _RedefineSwapKey();
	void _RedefineLeftSelectKey();
	void _RedefineRightSelectKey();
	void _RedefinePauseKey();
	//@}

	/**
	*** \brief Pass through functions to the InputManager, because on windows having the function pointers point
	*** directly to the InputManager caused heap corruption.
	**/
	//@{
	void _SetUpKey(const SDLKey &key);
	void _SetDownKey(const SDLKey &key);
	void _SetLeftKey(const SDLKey &key);
	void _SetRightKey(const SDLKey &key);
	void _SetConfirmKey(const SDLKey &key);
	void _SetCancelKey(const SDLKey &key);
	void _SetMenuKey(const SDLKey &key);
	void _SetSwapKey(const SDLKey &key);
	void _SetLeftSelectKey(const SDLKey &key);
	void _SetRightSelectKey(const SDLKey &key);
	void _SetPauseKey(const SDLKey &key);
	//@}

	/**
	*** \brief Redefines a joystick button to be mapped to another command. Waits for press using _WaitJoyPress()
	**/
	//@{
	void _RedefineXAxisJoy();
	void _RedefineYAxisJoy();
	void _RedefineThresholdJoy();

	void _RedefineConfirmJoy();
	void _RedefineCancelJoy();
	void _RedefineMenuJoy();
	void _RedefineSwapJoy();
	void _RedefineLeftSelectJoy();
	void _RedefineRightSelectJoy();
	void _RedefinePauseJoy();
	void _RedefineQuitJoy();
	//@}

	/**
	*** \brief Pass through functions to the InputManager, because on windows having the function pointers point
	*** directly to the InputManager caused heap corruption.
	**/
	//@{
	void _SetXAxisJoy(int8 axis);
	void _SetYAxisJoy(int8 axis);
	void _SetConfirmJoy(uint8 button);
	void _SetCancelJoy(uint8 button);
	void _SetMenuJoy(uint8 button);
	void _SetSwapJoy(uint8 button);
	void _SetLeftSelectJoy(uint8 button);
	void _SetRightSelectJoy(uint8 button);
	void _SetPauseJoy(uint8 button);
	//@}

	/** \brief init's the message window to display while waiting for a new key or joystick button press
	 **/
	void _ShowMessageWindow(bool joystick);

	/** \brief init's the message window to display while waiting for a new key, joystick button press, or joystick axis move
	 **/
	void _ShowMessageWindow(private_boot::WAIT_FOR wait);


	/** \brief overwrites the profile if the user has confirmed 
	**/
	void _OverwriteProfile();

	/**
	*** \brief Setups the corresponding menu (initialize menu members, set callbacks)
	**/
	//@{
	void _SetupMainMenu();
	void _SetupOptionsMenu();
	void _SetupVideoOptionsMenu();
	void _SetupAudioOptionsMenu();
	void _SetupLanguageOptionsMenu();
	void _SetupKeySetttingsMenu();
	void _SetupJoySetttingsMenu();
	void _SetupResolutionMenu();
	void _SetupLoadProfileMenu();
	void _SetupSaveProfileMenu();
	void _SetupProfileMenu();
	void _SetupUserInputMenu();
	//@}

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
	//! Battle debug confirmed
	void _OnBattleDebug();
	//! Menu debug confirmed
	void _OnMenuDebug();
	//! Shop debug confirmed
	void _OnShopDebug();

	// Options' handlers
	//! 'Video' confirmed
	void _OnVideoOptions();
	//! 'Audio' confirmed
	void _OnAudioOptions();
	//! 'Language' confirmed
	void _OnLanguageOptions();
	//! 'Key settings' confirmed
	void _OnKeySettings();
	//! 'Joystick settings' confirmed
	void _OnJoySettings();

	//! 'Resolution' confirmed
	void _OnResolution();
	//! 'Video mode' confirmed
	void _OnVideoMode();

	//! Specific language selected
	void _OnLanguageSelect();

	//! Sound volume down
	void _OnSoundLeft();
	//! Sound volume up
	void _OnSoundRight();
	//! Music volume down
	void _OnMusicLeft();
	//! Music volume up
	void _OnMusicRight();


	/**
	*** \brief Resolution switching functions
	**/
	//@{
	void _SetResolution(int32 width, int32 height);
	void _OnResolution640x480();
	void _OnResolution800x600();
	void _OnResolution1024x768();
	//@}

	//! Brightness increment
	void _OnBrightnessLeft();

	//! Brightness decrement
	void _OnBrightnessRight();


	//! Restores default key settings
	void _OnRestoreDefaultKeys();
	//! Restores default joystick settings
	void _OnRestoreDefaultJoyButtons();


	//! Switches to the menu which loads the profile specified by the user
	void _OnLoadProfile();

	//! Switches to the menu which saves the settings to a file specified by the user
	void _OnSaveProfile();

	//! Saves and Loads profiles specified by the user
	void _OnProfiles();

	//! Loads the settings file specified by the user
	void _OnLoadFile();
	
	//! Asks user for filename and then saves the settings to a .lua file
	void _OnSaveFile();

	//! Adds a letter to the currently selected filename
	void _OnPickLetter();


	//! Updates the video options screen
	void _UpdateVideoOptions();

	//! Updates the audio options screen
	void _UpdateAudioOptions();

	//! Updates the key settings screen
	void _UpdateKeySettings();

	//! Updates the joystick settings screen
	void _UpdateJoySettings();

	//! Updates the save and load profile menus
	void _UpdateSaveAndLoadProfiles();

	/**
	** \brief Saves the settings to a file specified by the user
	** \param fileName the name of the file for the settings to be saved to, if a blank string is passed the default "settings.lua" will be ** used
	**/
	void _SaveSettingsFile(const std::string& fileName);


	/**
	** \brief Saves the settings to a file specified by the user
	** \param fileName the name of the file for the settings to be loaded from if a blank string is passed the default "settings.lua" will ** be used
	**/
	bool _LoadSettingsFile(const std::string& fileName);

	/** \brief returns the directory listing for the user data path
	** \return A vector listing all the files in the directory not including the default settings.lua file, this is meant for personalized ** profiles only
	**/
	std::vector<std::string> _GetDirectoryListingUserDataPath();

}; // class BootMode : public hoa_mode_manager::GameMode

} // namespace hoa_boot

#endif // __BOOT_HEADER__
