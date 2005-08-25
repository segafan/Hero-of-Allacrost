///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    boot.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 12th, 2005
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
#include "SDL.h"
#include "defs.h"
#include "engine.h"

//! All calls to boot mode are wrapped in this namespace.
namespace hoa_boot {

//! Determines whether the code in the hoa_boot namespace should print debug statements or not.
extern bool BOOT_DEBUG;

//! An internal namespace to be used only within the boot code. Don't use this namespace anywhere else!
namespace private_boot {

//! \name Main menu selections
//@{
//! \brief Constants used to cycle through the primary boot menu
const uint32 NEW_MENU     = 0;
const uint32 LOAD_MENU    = 1;
const uint32 OPTIONS_MENU = 2;
const uint32 CREDITS_MENU = 3;
const uint32 HIDE_MENU    = 4;
const uint32 QUIT_MENU    = 5;
//@}

//! \name Options menu selections
//@{
//! \brief Constants used to cycle through the options boot menu
const uint32 VIDEO_OP     = 0;
const uint32 AUDIO_OP     = 1;
const uint32 LANGUAGE_OP  = 2;
const uint32 KEYS_OP      = 3;
const uint32 JOYSTICK_OP  = 4;
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
class BootMode : public hoa_engine::GameMode {
private:
	//! If true, no menus will be drawn (so the player can get a nice good look at the background image).
	bool _menu_hidden;
	//! A vector storing various menu pointers in a stack-like structure.
	std::vector<uint32> _vmenu_index;
	//! Music to be used at the boot screen.
	std::vector<hoa_audio::MusicDescriptor> _boot_music;
	//! Sounds that will be used at the boot screen.
	std::vector<hoa_audio::SoundDescriptor> _boot_sound;
	//! Images that will be used at the boot screen.
	std::vector<hoa_video::ImageDescriptor> _boot_images;

	/*!
	 *  \brief Animates the game logo when this class is first initialized.
	 *
	 *  The logo animation consists of:
	 *  - The logo flying in from somewhere, with the 't' sword in Allacrost cut into the word
	 *  - The sword unsheating itself from the word (with sound)
	 *  - The sword spinning up and around a few times (with sound)
	 *  - The sword slicing down into its final position as a t (with sound)
	 */
	void _AnimateLogo();

	/*!
	 *  \brief Redefines a key to be mapped to another command.
	 *  \param &change_key The key to be re-mapped.
	 */
	void _RedefineKey(SDLKey& change_key);

	//! Updates the game state when "New Game" is selected in the main menu.
	void _UpdateNewMenu();
	//! Updates the game state when "Load Game" is selected in the main menu.
	void _UpdateLoadMenu();
	//! Updates the game state when "Options" is selected in the main menu.
	void _UpdateOptionsMenu();
	//! Updates the game state when "Credits" is selected in the main menu.
	void _UpdateCreditsMenu();
	//! Updates the game state when "Hide Menu" is selected in the main menu.
	void _UpdateHideMenu();
	//! Updates the game state when "Quit Menu" is selected in the main menu.
	void _UpdateQuitMenu();

	//! Updates the game state when "Video" is selected in the options menu.
	void _UpdateVideoOptions();
	//! Updates the game state when "Audio" is selected in the options menu.
	void _UpdateAudioOptions();
	//! Updates the game state when "Language" is selected in the options menu.
	void _UpdateLanguageOptions();
	//! Updates the game state when "Keyboard" is selected in the options menu.
	void _UpdateKeyOptions();
	//! Updates the game state when "Joystick" is selected in the options menu.
	void _UpdateJoystickOptions();

	//! Draws the menu when "Load Game" has been selected in the main menu.
	void _DrawLoadMenu();
	//! Draws the menu when a saved game has been selected from the "Load Game" menu.
	void _DrawLoadGame();
	//! Draws the menu when "Video" has been selected in the options menu.
	void _DrawVideoOptions();
	//! Draws the menu when "Audio" has been selected in the options menu.
	void _DrawAudioOptions();
	//! Draws the menu when "Language" has been selected in the options menu.
	void _DrawLanguageOptions();
	//! Draws the menu when "Keyboard" has been selected in the options menu.
	void _DrawKeyOptions();
	//! Draws the menu when "Joystick" has been selected in the options menu.
	void _DrawJoystickOptions();

	//! Draws the menu when "Credits" has been selected in the main menu.
	void _DrawCredits();
public:
	//! Initializes class members and loads media data.
	BootMode();
	//! Frees all media data (images and audio).
	~BootMode();

	//! Wrapper function that calls different update functions depending on the menu state.
	void Update(uint32 time_elapsed);
	//! Wrapper function that calls different draw functions depending on the menu state.
	void Draw();
};

} // namespace hoa_boot

#endif
