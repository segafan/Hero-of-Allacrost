///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    boot.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 23rd, 2005
 * \brief   Source file for boot mode interface.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include "boot.h"
#include "audio.h"
#include "video.h"
#include "data.h"
#include "global.h"
#include "engine.h"
#include "map.h"
#include "battle.h" // tmp

using namespace std;
using namespace hoa_boot::private_boot;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_engine;
using namespace hoa_global;
using namespace hoa_data;
using namespace hoa_map;
using namespace hoa_battle; // tmp

namespace hoa_boot {

bool BOOT_DEBUG = false;

// ****************************************************************************
// *************************** GENERAL FUNCTIONS ******************************
// ****************************************************************************

// The constructor initializes variables and sets up the path names of the boot images
BootMode::BootMode() {
	if (BOOT_DEBUG) cout << "BOOT: BootMode constructor invoked." << endl;

	mode_type = ENGINE_BOOT_MODE;

	_vmenu_index.push_back(LOAD_MENU);
	_menu_hidden = false;

	DataManager->LoadBootData(&_boot_images, &_boot_sound, &_boot_music);

	VideoManager->SetCoordSys(0, 1024, 0, 768); // TEMPORARY

 	for (uint32 i = 0; i < _boot_images.size(); i++) {
 		VideoManager->LoadImage(_boot_images[i]);
 	}
	for (uint32 i = 0; i < _boot_music.size(); i++) {
		AudioManager->LoadMusic(_boot_music[i]);
	}
	for (uint32 i = 0; i < _boot_sound.size(); i++) {
		AudioManager->LoadSound(_boot_sound[i]);
	}

	AudioManager->PlayMusic(_boot_music[0], AUDIO_NO_FADE, AUDIO_LOOP_FOREVER);
}



// The destructor frees all used music, sounds, and images.
BootMode::~BootMode() {
	if (BOOT_DEBUG) cout << "BOOT: BootMode destructor invoked." << endl;

	for (uint32 i = 0; i < _boot_music.size(); i++)
		AudioManager->FreeMusic(_boot_music[i]);
	for (uint32 i = 0; i < _boot_sound.size(); i++)
		AudioManager->FreeSound(_boot_sound[i]);
	for (uint32 i = 0; i < _boot_images.size(); i++)
		VideoManager->DeleteImage(_boot_images[i]);
}





// Animates the logo when the boot mode starts up. Should not be called before LoadBootImages.
void BootMode::_AnimateLogo() {
	cout << "TEMP: Do nothing" << endl;
	// Write a series of image moves/rotations to animate the logo here.
}




// Redefines the change_key reference. Waits indefinitely for user to press any key.
void BootMode::_RedefineKey(SDLKey& change_key) {
//	SDL_Event event;

// 	KeyState *key_set = &(SettingsManager->InputStatus.key); // A shortcut

// 	while (SDL_WaitEvent(&event)) { // Waits indefinitely for events to occur.
// 		switch (event.type) {
// 			case SDL_KEYDOWN:
// 				// The following series of if statements ensure mutual exclusion of the key map
// 				if (key_set->up == event.key.keysym.sym) {
// 					key_set->up = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				if (key_set->down == event.key.keysym.sym) {
// 					key_set->down = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				if (key_set->left == event.key.keysym.sym) {
// 					key_set->left = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				if (key_set->right == event.key.keysym.sym) {
// 					key_set->right = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				if (key_set->confirm == event.key.keysym.sym) {
// 					key_set->confirm = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				if (key_set->cancel == event.key.keysym.sym) {
// 					key_set->cancel = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				if (key_set->menu == event.key.keysym.sym) {
// 					key_set->menu = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				if (key_set->pause == event.key.keysym.sym) {
// 					key_set->pause = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				change_key = event.key.keysym.sym;
// 				return;
// 		}
// 	}
}


// ****************************************************************************
// **************************** UPDATE FUNCTIONS ******************************
// ****************************************************************************


// This is called once every frame iteration to update the status of the game
void BootMode::Update(uint32 time_elapsed) {
	// If our menu is hidden, we don't do anything until a user event occurs.
	if (_menu_hidden) {
		if (InputManager->ConfirmPress() || InputManager->CancelPress() || InputManager->MenuPress() ||
				InputManager->SwapPress() || InputManager->LeftSelectPress() || InputManager->RightSelectPress() ||
				InputManager->UpPress() || InputManager->DownPress() || InputManager->LeftPress() ||
				InputManager->RightPress())
			_menu_hidden = false;
		return;
	}

	switch (_vmenu_index[0]) {
		case NEW_MENU:
			_UpdateNewMenu();
			break;
		case LOAD_MENU:
			_UpdateLoadMenu();
			break;
		case OPTIONS_MENU:
			_UpdateOptionsMenu();
			break;
		case CREDITS_MENU:
			_UpdateCreditsMenu();
			break;
		case HIDE_MENU:
			_UpdateHideMenu();
			break;
		case QUIT_MENU:
			_UpdateQuitMenu();
			break;
	}
}



// Handles events when NEW_MENU is selected.
void BootMode::_UpdateNewMenu() {
	// Left and right flags are mutually exclusive. We ignore right if both are pressed.
	if (InputManager->LeftPress()) {
		_vmenu_index[0] = QUIT_MENU;
		cout << "QUIT MENU" << endl;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (InputManager->RightPress()) {
		_vmenu_index[0] = LOAD_MENU;
		cout << "LOAD MENU" << endl;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}

	if (InputManager->ConfirmPress()) {
		AudioManager->PlaySound(_boot_sound[2], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // game loading sound
		if (BOOT_DEBUG) cout << "BOOT: Starting new game." << endl;
		// Remove the boot mode from the top of the stack

		GCharacter *claud = new GCharacter("Claudius", "claudius", GLOBAL_CLAUDIUS);
		InstanceManager->AddCharacter(claud);

		MapMode *MM = new MapMode(0);
		ModeManager->Pop();
		ModeManager->Push(MM);
	}
}



// Handles events when LOAD_MENU is selected. Two levels of sub-menus here.
void BootMode::_UpdateLoadMenu() {
	// Handle main menu events when all submenus are closed
	if (_vmenu_index.size() == 1) {
		// Left and right flags are mutually exclusive. We ignore right if both are pressed.
		if (InputManager->LeftPress()) {
			_vmenu_index[0] = NEW_MENU;
			cout << "NEW MENU" << endl;
			AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		else if (InputManager->RightPress()) {
			_vmenu_index[0] = OPTIONS_MENU;
			cout << "OPTIONS MENU" << endl;
			AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}

		if (InputManager->ConfirmPress()) {
			_vmenu_index.push_back(0);
			cout << "*Entering Saved Game Selection Screen" << endl;
			// Load the list of saved games
			AudioManager->PlaySound(_boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
			// Load a vector of the saved games here
			BattleMode *BM = new BattleMode();
			ModeManager->Pop();
			ModeManager->Push(BM);
		}

		return;
	} // if (_vmenu_index.size() == 1)


	// Handle submenu level 1 events. This is the "saved game selection screen" submenu
	if (_vmenu_index.size() == 2) {
		if (InputManager->CancelPress()) {
			AudioManager->PlaySound(_boot_sound[1], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // cancel sound
			_vmenu_index.pop_back();
			cout << "*Exiting Saved game selection screen" << endl;
			return;
		}
		if (InputManager->ConfirmPress()) {
			_vmenu_index.push_back(0);
			// Load the specific saved game data screen
			cout << "*Entering saved game confirmation screen" << endl;
			AudioManager->PlaySound(_boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
			// Call a config routine to load details about the selected saved game
			return;
		}

		// Change selected game. Up and down key presses mutality exclusive. Up has higher priority.
		if (InputManager->UpPress()) {
			if (_vmenu_index[1] != 0)
				_vmenu_index[1] = _vmenu_index[1] - 1;
			else
				_vmenu_index[1] = 5; // 5 = number of saved games (TEMP)
			cout << "*Saved game " << _vmenu_index[1] << " selected." << endl;
			AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		if (InputManager->DownPress()) {
			if (_vmenu_index[1] != 5) // 5 = num_saved_games (TEMP)
				_vmenu_index[1] = _vmenu_index[1] + 1;
			else
				_vmenu_index[1] = 0;
			cout << "*Saved game " << _vmenu_index[1] << " selected." << endl;
			AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		}

		return;
	} // if (_vmenu_index.size() == 1)


	// Handle submenu level 2 events. This is the "saved game confirmation screen" submenu
	if (_vmenu_index.size() == 3) {
		if (InputManager->CancelPress()) {
			AudioManager->PlaySound(_boot_sound[1], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // cancel sound
			_vmenu_index.pop_back();
			cout << "*Exiting saved game confirmation screen" << endl;
		}
		else if (InputManager->ConfirmPress()) {
			cout << "*Loading saved game!" << endl;
			AudioManager->PlaySound(_boot_sound[2], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // game loading sound
			// Load the game
		}
	} // if (vmenu_index.size() == 2)

}


// Updates when in options menu. Calls other update handling function. Two levels of submenus
void BootMode::_UpdateOptionsMenu() {
	// Handle events when we are in level 0 menu
	if (_vmenu_index.size() == 1) {
		if (InputManager->LeftPress()) {
			_vmenu_index[0] = LOAD_MENU;
			cout << "LOAD MENU" << endl;
			AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		else if (InputManager->RightPress()) {
			_vmenu_index[0] = CREDITS_MENU;
			cout << "CREDITS_MENU" << endl;
			AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}

		if (InputManager->ConfirmPress()) {
			_vmenu_index.push_back(0); // Left-Right main options menu
			_vmenu_index.push_back(0); // Up-Down specific options menu
			cout << "*Entering options menu!" << endl;
			AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		}
		return;
	}

	// Level 1 submenu
	if (_vmenu_index.size() == 3) {
		if (InputManager->CancelPress()) {
			_vmenu_index.pop_back();
			_vmenu_index.pop_back();
			cout << "*Exiting options menu" << endl;
			AudioManager->PlaySound(_boot_sound[1], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // cancel sound
			return;
		}

		switch (_vmenu_index[1]) {
			case VIDEO_OP:
				_UpdateVideoOptions();
				break;
			case AUDIO_OP:
				_UpdateAudioOptions();
				break;
			case LANGUAGE_OP:
				_UpdateLanguageOptions();
				break;
			case KEYS_OP:
				_UpdateKeyOptions();
				break;
			case JOYSTICK_OP:
				_UpdateJoystickOptions();
				break;
		}
	}
}



// Handles updates when the credits menu is selected. There is one sub-menu level.
void BootMode::_UpdateCreditsMenu() {
	// Handle events when we are in level 0 menu
	if (_vmenu_index.size() == 1) {
		// Left and right flags are mutually exclusive. We ignore right if both are pressed.
		if (InputManager->LeftPress()) {
			_vmenu_index[0] = OPTIONS_MENU;
			cout << "OPTIONS MENU" << endl;
			AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		else if (InputManager->RightPress()) {
			cout << "HIDE_MENU" << endl;
			_vmenu_index[0] = HIDE_MENU;
			AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}

		if (InputManager->ConfirmPress()) {
			_vmenu_index.push_back(0);
			cout << "*Viewing credits now!" << endl;
			// Load in the credits text.
			AudioManager->PlaySound(_boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
		}
		return;
	}

	// Level 1 submenu
	if (InputManager->ConfirmPress() || InputManager->CancelPress()) {
		_vmenu_index.pop_back();
		cout << "*Exiting credits" << endl;
	}
	// else() { We update the credits animation position? }
}



// Handles updates when hide menu is selected. No sub-menu levels here.
void BootMode::_UpdateHideMenu() {
	if (InputManager->LeftPress()) {
		_vmenu_index[0] = CREDITS_MENU;
		cout << "CREDITS_MENU" << endl;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (InputManager->RightPress()) {
		_vmenu_index[0] = QUIT_MENU;
		cout << "QUIT_MENU" << endl;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (InputManager->ConfirmPress()) {
		_menu_hidden = true;
	}
}



// Handles updates when quit menu is selected. No sub-menu levels here
void BootMode::_UpdateQuitMenu() {
	if (InputManager->LeftPress()) {
		_vmenu_index[0] = HIDE_MENU;
		cout << "HIDE_MENU" << endl;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (InputManager->RightPress()) {
		_vmenu_index[0] = NEW_MENU;
		cout << "NEW_MENU" << endl;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (InputManager->ConfirmPress()) {
		SettingsManager->ExitGame();
	}
}



// Handle updates when video options menu is selected. One sub-menu level.
void BootMode::_UpdateVideoOptions() {
	if (InputManager->LeftPress()) {
		_vmenu_index[1] = JOYSTICK_OP;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: JOYSTICK MENU" << endl;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	else if (InputManager->RightPress()) {
		_vmenu_index[1] = AUDIO_OP;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: AUDIO MENU" << endl;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}

	if (InputManager->UpPress()) {
		if (_vmenu_index[2] != 0)
			_vmenu_index[2] = _vmenu_index[2] - 1;
		else
			_vmenu_index[2] = 4;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (InputManager->DownPress()) {
		if (_vmenu_index[2] != 4)
			_vmenu_index[2] = _vmenu_index[2] + 1;
		else
			_vmenu_index[2] = 0;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}

	if (InputManager->ConfirmPress()) {
		switch (_vmenu_index[2]) {
			case 0:
				// Toggle fullscreen
				break;
			case 1:
				// Change video resolution
				break;
			case 2:
				// Change other video setting
				break;
			case 3:
				// Change other video setting
				break;
			case 4:
				// Change other video setting
				break;
		}
	}
}



// Update code for audio options menu. Uses two sub-menu levels.
void BootMode::_UpdateAudioOptions() {
	if (_vmenu_index.size() == 3) {
		if (InputManager->LeftPress()) {
			_vmenu_index[1] = VIDEO_OP;
			_vmenu_index[2] = 0;
			cout << "OPTIONS: VIDEO MENU" << endl;
			AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		else if (InputManager->RightPress()) {
			_vmenu_index[1] = LANGUAGE_OP;
			_vmenu_index[2] = 0;
			cout << "OPTIONS: LANGUAGE MENU" << endl;
			AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}

		if (InputManager->UpPress()) {
			if (_vmenu_index[2] != 0)
				_vmenu_index[2] = _vmenu_index[2] - 1;
			else
				_vmenu_index[2] = 1;
			AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		}
		else if (InputManager->DownPress()) {
			if (_vmenu_index[2] != 1)
				_vmenu_index[2] = _vmenu_index[2] + 1;
			else
				_vmenu_index[2] = 0;
			AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		}

		if (InputManager->ConfirmPress()) {
			_vmenu_index.push_back(0);
		}
		return;
	}

	// Changing volume levels
	if (InputManager->CancelPress()) {
		_vmenu_index.pop_back();
		AudioManager->PlaySound(_boot_sound[1], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // cancel sound
		return;
	}

	if (_vmenu_index[2] == 0) { // Change music volume
		if (InputManager->LeftState()) { // Decrease music volume

		}
		if (InputManager->RightState()) { // Increase music volume

		}
	}
	else if (_vmenu_index[2] == 1) { // Change sound volume
		if (InputManager->LeftState()) { // Decrease music volume

		}
		if (InputManager->RightState()) { // Increase music volume

		}
	}
}



// Handles language options menu updates. One sub-menu level.
void BootMode::_UpdateLanguageOptions() {
	if (InputManager->LeftPress()) {
		_vmenu_index[1] = AUDIO_OP;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: AUDIO MENU" << endl;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	else if (InputManager->RightPress()) {
		_vmenu_index[1] = KEYS_OP;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: KEY SETTINGS MENU" << endl;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}

	if (InputManager->UpPress()) {
		if (_vmenu_index[2] != 0)
			_vmenu_index[2] = _vmenu_index[2] - 1;
		else
			_vmenu_index[2] = 2;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (InputManager->DownPress()) {
		if (_vmenu_index[2] != 2)
			_vmenu_index[2] = _vmenu_index[2] + 1;
		else
			_vmenu_index[2] = 0;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}

	if (InputManager->ConfirmPress()) {
		switch (_vmenu_index[2]) {
			case 0:
				// Change language to English
				break;
			case 1:
				// Change language to German
				break;
			case 2:
				// Change language to Spanish
				break;
		}
	}
}



// Handle updates when key settings options menu is selected. One sub-menu.
void BootMode::_UpdateKeyOptions() {
	if (InputManager->LeftPress()) {
		_vmenu_index[1] = LANGUAGE_OP;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: LANGUAGE MENU" << endl;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	else if (InputManager->RightPress()) {
		_vmenu_index[1] = JOYSTICK_OP;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: JOYSTICK MENU" << endl;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}

	if (InputManager->UpPress()) {
		if (_vmenu_index[2] != 0)
			_vmenu_index[2] = _vmenu_index[2] - 1;
		else
			_vmenu_index[2] = 7;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (InputManager->DownPress()) {
		if (_vmenu_index[2] != 7)
			_vmenu_index[2] = _vmenu_index[2] + 1;
		else
			_vmenu_index[2] = 0;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}

	if (InputManager->ConfirmPress()) {
		switch (_vmenu_index[2]) {
			case 0: // Change up key
				//RedefineKey(InputManager->Key.up&);
				AudioManager->PlaySound(_boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 1: // Change down key
				//RedefineKey(&(InputManager->Key.down));
				AudioManager->PlaySound(_boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 2: // Change left key
				//RedefineKey(&(InputManager->Key.left));
				AudioManager->PlaySound(_boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 3: // Change right key
				//RedefineKey(&(InputManager->Key.right));
				AudioManager->PlaySound(_boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 4: // Change confirm key
				//RedefineKey(&(InputManager->Key.confirm));
				AudioManager->PlaySound(_boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 5: // Change cancel key
				//RedefineKey(&(InputManager->Key.cancel));
				AudioManager->PlaySound(_boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 6: // Change menu key
				//RedefineKey(&(InputManager->Key.menu));
				AudioManager->PlaySound(_boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 7: // Change pause key
				//RedefineKey(&(InputManager->Key.pause));
				AudioManager->PlaySound(_boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
		}
	}
}



// Handle updates when joystick options menu is selected. One sub-menu.
void BootMode::_UpdateJoystickOptions() {
	if (InputManager->LeftPress()) {
		_vmenu_index[1] = KEYS_OP;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: KEY SETTINGS MENU" << endl;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	else if (InputManager->RightPress()) {
		_vmenu_index[1] = VIDEO_OP;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: VIDEO MENU" << endl;
		AudioManager->PlaySound(_boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}

// This function is incomplete for now. It will be very much like the UpdateKeyOptions when finished.
}


// ****************************************************************************
// ***************************** DRAW FUNCTIONS *******************************
// ****************************************************************************


// Draws our next frame to the video back buffer
void BootMode::Draw() {
	// Draw the backdrop image

	//VideoManager->Move(0, 0);
	//VideoManager->Move(-1024/2, 0);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_NO_BLEND, 0);
	VideoManager->DrawImage(_boot_images[0]);

	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_TOP, VIDEO_BLEND, 0);
	VideoManager->Move(1024/2, 50);
	VideoManager->DrawImage(_boot_images[2]);

	VideoManager->Move(1024/2, 575);
	VideoManager->DrawImage(_boot_images[1]);

	if (_menu_hidden) { // Then we are already done!
		return;
	}

	// Draw logo at top center of screen

	// Draw plain main menu at bottom center.

	// Draw text in menus depending on language
	return;
}


// Draws a window displaying summary info of all the saved games on the system
void BootMode::_DrawLoadMenu() {
	cout << "TEMP: DrawLoadMenu() invoked" << endl;
}


// Draws the menu screen for the selected game and displays a confirmation dialogue
void BootMode::_DrawLoadGame() {
	cout << "TEMP: DrawLoadGame() invoked." << endl;
}

// Draws the video options menu.
void BootMode::_DrawVideoOptions() {
	cout << "TEMP: DrawVideoOptions() invoked." << endl;
	// Draw a full screen checkbox and the various video modes available
}

// Draws the audio options menu.
void BootMode::_DrawAudioOptions() {
	cout << "TEMP: DrawAudioOptions() invoked." << endl;
	// Draw a music and sound volume bar thing
}

// Draws the language options menu.
void BootMode::_DrawLanguageOptions() {
	cout << "TEMP: DrawLanguageOptions() invoked." << endl;
	// Draw a list of all the languages available.
}

// Draws the key options menu.
void BootMode::_DrawKeyOptions() {
	cout << "TEMP: DrawKeyOptions() invoked." << endl;
	// Draw a list of all the function->key mappings. I'll need strings for all the keysyms...
}

// Draws the joystick options menu.
void BootMode::_DrawJoystickOptions() {
	cout << "TEMP: DrawJoystickOptions() invoked." << endl;
	// This will be implemented later.
}

// Draws the credits menu.
void BootMode::_DrawCredits() {
	cout << "TEMP: DrawCredits() invoked." << endl;
	// Draw a text box with our names and junk. Later will be made into a specify animation.
}


}
