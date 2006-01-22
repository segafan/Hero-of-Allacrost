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

	_fade_out = false;

	_vmenu_index.push_back(LOAD_GAME);

	ReadDataDescriptor read_data;
	if (!read_data.OpenFile("dat/config/boot.lua")) {
		cout << "BOOT ERROR: failed to load data file" << endl;
	}

	// Load the video stuff
	StillImage im;

	// The background
	im.SetFilename(read_data.ReadString("background_image"));
	im.SetDimensions(read_data.ReadFloat("background_image_width"),
	                 read_data.ReadFloat("background_image_height"));
	_boot_images.push_back(im);

	// The logo
	im.SetFilename(read_data.ReadString("logo_image"));
	im.SetDimensions(read_data.ReadFloat("logo_image_width"),
	                 read_data.ReadFloat("logo_image_height"));
	_boot_images.push_back(im);

	// The menu
	im.SetFilename(read_data.ReadString("menu_image"));
	im.SetDimensions(read_data.ReadFloat("menu_image_width"),
	                 read_data.ReadFloat("menu_image_height"));
	_boot_images.push_back(im);

	// Set up a coordinate system - now you can use the boot.lua to set it to whatever you like
	VideoManager->SetCoordSys(read_data.ReadFloat("coord_sys_x_left"),
	                          read_data.ReadFloat("coord_sys_x_right"),
	                          read_data.ReadFloat("coord_sys_y_bottom"),
	                          read_data.ReadFloat("coord_sys_y_top"));


	// Load the audio stuff
	// Make a call to the config code that loads in two vectors of strings
	vector<string> new_music_files;
	read_data.FillStringVector("music_files", new_music_files);

	vector<string> new_sound_files;
	read_data.FillStringVector("sound_files", new_sound_files);

	if (read_data.GetError() != DATA_NO_ERRORS) {
		cout << "BOOT ERROR: some error occured during reading of boot data file" << endl;
	}

	// Push all our new music onto the boot_music vector
// 	MusicDescriptor new_music;
// 	for (uint i = 0; i < new_music_files.size(); i++) {
// 		_boot_music.push_back(new_music);
// 		_boot_music[i].LoadMusic(new_music_files[i]);
// 	}
// 
// 	SoundDescriptor new_sound;
// 	for (uint i = 0; i < new_sound_files.size(); i++) {
// 		_boot_sound.push_back(new_sound);
// 		_boot_sound[i].LoadSound(new_sound_files[i]);
// 	}

	for (uint32 i = 0; i < _boot_images.size(); i++) {
		VideoManager->LoadImage(_boot_images[i]);
	}


	// Initialize all the properties of the main menu
	_main_options.SetFont("default");
	_main_options.SetCellSize(128.0f, 50.0f);
	_main_options.SetSize(5, 1);
	_main_options.SetPosition(512.0f, 50.0f);
	_main_options.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_main_options.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_main_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_main_options.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_main_options.SetCursorOffset(-35.0f, -4.0f);

	vector<ustring> options;
	options.push_back(MakeWideString("New Game"));
	options.push_back(MakeWideString("Load Game"));
	options.push_back(MakeWideString("Options"));
	options.push_back(MakeWideString("Credits"));
	options.push_back(MakeWideString("Quit"));

	_main_options.SetOptions(options);
	_main_options.SetSelection(LOAD_GAME);

	// Initialize all the properties of the settings menu
	_setting_options.SetFont("default");
	_setting_options.SetCellSize(128.0f, 50.0f);
	_setting_options.SetSize(5, 1);
	_setting_options.SetPosition(512.0f, 500.0f);
	_setting_options.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_setting_options.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_setting_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_setting_options.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_setting_options.SetCursorOffset(-35.0f, -4.0f);

	options.clear();
	options.push_back(MakeWideString("Video"));
	options.push_back(MakeWideString("Audio"));
	options.push_back(MakeWideString("Language"));
	options.push_back(MakeWideString("Key Settings"));
	options.push_back(MakeWideString("Joystick Settings"));

	_setting_options.SetOptions(options);
	_setting_options.SetSelection(VIDEO_OPTIONS);
	cout << "BOOT MODE CONSTRUCTOR END" << endl;
}


// The destructor frees all used music, sounds, and images.
BootMode::~BootMode() {
	if (BOOT_DEBUG) cout << "BOOT: BootMode destructor invoked." << endl;

	for (uint32 i = 0; i < _boot_music.size(); i++)
		_boot_music[i].FreeMusic();
	for (uint32 i = 0; i < _boot_sound.size(); i++)
		_boot_sound[i].FreeSound();
	for (uint32 i = 0; i < _boot_images.size(); i++)
		VideoManager->DeleteImage(_boot_images[i]);
}


// Resets appropriate class members.
void BootMode::Reset() {
	// Play the intro theme
// 	_boot_music[0].PlayMusic();
	// Set the coordinate system that BootMode uses
	VideoManager->SetCoordSys(0, 1024, 0, 768);
	//
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
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
void BootMode::Update() {
	uint32 time_elapsed = SettingsManager->GetUpdateTime();

	// Screen is in the process of fading out
	if (_fade_out) {
		// When the screen is finished fading to black, create a new map mode and fade back in
		if (!VideoManager->IsFading()) {
			MapMode *MM = new MapMode();
			ModeManager->Pop();
			ModeManager->Push(MM);
			VideoManager->FadeScreen(Color::clear, 1.0f);
		}
		return;
	}


	if(InputManager->ConfirmPress()) {
		// Play Sound
		_main_options.HandleConfirmKey();
	}
	else if (InputManager->LeftPress()) {
		// Play Sound
		_main_options.HandleLeftKey();
	}
	else if(InputManager->RightPress()) {
		// Play Sound
		_main_options.HandleRightKey();
	}

	int32 event = _main_options.GetEvent();

	if (event == VIDEO_OPTION_CONFIRM) {
		switch (_main_options.GetSelection()) {
			case NEW_GAME:
			{
// 				_boot_sound[2].PlaySound(); // acquire sound
				if (BOOT_DEBUG) cout << "BOOT: Starting new game." << endl;
				GlobalCharacter *claud = new GlobalCharacter("Claudius", "claudius", GLOBAL_CLAUDIUS);
				GlobalManager->AddCharacter(claud);
				_fade_out = true;
				VideoManager->FadeScreen(Color::black, 1.0f);
				break;
			}
			case LOAD_GAME:
			{
// 				_boot_sound[0].PlaySound(); // confirm sound
				cout << "BOOT: TEMP: Entering battle mode" << endl;
				BattleMode *BM = new BattleMode();
				ModeManager->Pop();
				ModeManager->Push(BM);
				break;
			}
			case OPTIONS:
				cout << "BOOT: TEMP: Switching context to options" << endl;
				break;
			case CREDITS:
// 				_boot_sound[0].PlaySound(); // confirm sound
				cout << "BOOT: TEMP: Viewing credits now!" << endl;
				break;
			case QUIT:
				SettingsManager->ExitGame();
				break;
			default:
				cerr << "BOOT: ERROR: Invalid selection in main options list" << endl;
				break;
		}
	}
}




// Handle updates when video options menu is selected. One sub-menu level.
void BootMode::_UpdateVideoOptions() {
	if (InputManager->LeftPress()) {
		_vmenu_index[1] = JOYSTICK_OPTIONS;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: JOYSTICK MENU" << endl;
// 		_boot_sound[3].PlaySound(); // move sound
		return;
	}
	else if (InputManager->RightPress()) {
		_vmenu_index[1] = AUDIO_OPTIONS;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: AUDIO MENU" << endl;
// 		_boot_sound[3].PlaySound(); // move sound
		return;
	}

	if (InputManager->UpPress()) {
		if (_vmenu_index[2] != 0)
			_vmenu_index[2] = _vmenu_index[2] - 1;
		else
			_vmenu_index[2] = 4;
// 		_boot_sound[3].PlaySound(); // move sound
	}
	else if (InputManager->DownPress()) {
		if (_vmenu_index[2] != 4)
			_vmenu_index[2] = _vmenu_index[2] + 1;
		else
			_vmenu_index[2] = 0;
// 		_boot_sound[3].PlaySound(); // move sound
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
			_vmenu_index[1] = VIDEO_OPTIONS;
			_vmenu_index[2] = 0;
			cout << "OPTIONS: VIDEO MENU" << endl;
// 			_boot_sound[3].PlaySound(); // move sound
			return;
		}
		else if (InputManager->RightPress()) {
			_vmenu_index[1] = LANGUAGE_OPTIONS;
			_vmenu_index[2] = 0;
			cout << "OPTIONS: LANGUAGE MENU" << endl;
// 			_boot_sound[3].PlaySound(); // move sound
			return;
		}

		if (InputManager->UpPress()) {
			if (_vmenu_index[2] != 0)
				_vmenu_index[2] = _vmenu_index[2] - 1;
			else
				_vmenu_index[2] = 1;
// 			_boot_sound[3].PlaySound(); // move sound
		}
		else if (InputManager->DownPress()) {
			if (_vmenu_index[2] != 1)
				_vmenu_index[2] = _vmenu_index[2] + 1;
			else
				_vmenu_index[2] = 0;
// 			_boot_sound[3].PlaySound(); // move sound
		}

		if (InputManager->ConfirmPress()) {
			_vmenu_index.push_back(0);
		}
		return;
	}

	// Changing volume levels
	if (InputManager->CancelPress()) {
		_vmenu_index.pop_back();
// 		_boot_sound[1].PlaySound(); // cancel sound
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
		_vmenu_index[1] = AUDIO_OPTIONS;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: AUDIO MENU" << endl;
// 		_boot_sound[3].PlaySound(); // move sound
		return;
	}
	else if (InputManager->RightPress()) {
		_vmenu_index[1] = KEYS_OPTIONS;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: KEY SETTINGS MENU" << endl;
// 		_boot_sound[3].PlaySound(); // move sound
		return;
	}

	if (InputManager->UpPress()) {
		if (_vmenu_index[2] != 0)
			_vmenu_index[2] = _vmenu_index[2] - 1;
		else
			_vmenu_index[2] = 2;
// 		_boot_sound[3].PlaySound(); // move sound
	}
	else if (InputManager->DownPress()) {
		if (_vmenu_index[2] != 2)
			_vmenu_index[2] = _vmenu_index[2] + 1;
		else
			_vmenu_index[2] = 0;
// 		_boot_sound[3].PlaySound(); // move sound
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
		_vmenu_index[1] = LANGUAGE_OPTIONS;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: LANGUAGE MENU" << endl;
// 		_boot_sound[3].PlaySound(); // move sound
		return;
	}
	else if (InputManager->RightPress()) {
		_vmenu_index[1] = JOYSTICK_OPTIONS;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: JOYSTICK MENU" << endl;
// 		_boot_sound[3].PlaySound(); // move sound
		return;
	}

	if (InputManager->UpPress()) {
		if (_vmenu_index[2] != 0)
			_vmenu_index[2] = _vmenu_index[2] - 1;
		else
			_vmenu_index[2] = 7;
// 		_boot_sound[3].PlaySound(); // move sound
	}
	else if (InputManager->DownPress()) {
		if (_vmenu_index[2] != 7)
			_vmenu_index[2] = _vmenu_index[2] + 1;
		else
			_vmenu_index[2] = 0;
// 		_boot_sound[3].PlaySound(); // move sound
	}

	if (InputManager->ConfirmPress()) {
		switch (_vmenu_index[2]) {
			case 0: // Change up key
				//RedefineKey(InputManager->Key.up&);
// 				_boot_sound[0].PlaySound(); // confirm sound
				break;
			case 1: // Change down key
				//RedefineKey(&(InputManager->Key.down));
// 				_boot_sound[0].PlaySound(); // confirm sound
				break;
			case 2: // Change left key
				//RedefineKey(&(InputManager->Key.left));
// 				_boot_sound[0].PlaySound(); // confirm sound
				break;
			case 3: // Change right key
				//RedefineKey(&(InputManager->Key.right));
// 				_boot_sound[0].PlaySound(); // confirm sound
				break;
			case 4: // Change confirm key
				//RedefineKey(&(InputManager->Key.confirm));
// 				_boot_sound[0].PlaySound(); // confirm sound
				break;
			case 5: // Change cancel key
				//RedefineKey(&(InputManager->Key.cancel));
// 				_boot_sound[0].PlaySound(); // confirm sound
				break;
			case 6: // Change menu key
				//RedefineKey(&(InputManager->Key.menu));
// 				_boot_sound[0].PlaySound(); // confirm sound
				break;
			case 7: // Change pause key
				//RedefineKey(&(InputManager->Key.pause));
// 				_boot_sound[0].PlaySound(); // confirm sound
				break;
		}
	}
}



// Handle updates when joystick options menu is selected. One sub-menu.
void BootMode::_UpdateJoystickOptions() {
	if (InputManager->LeftPress()) {
		_vmenu_index[1] = KEYS_OPTIONS;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: KEY SETTINGS MENU" << endl;
// 		_boot_sound[3].PlaySound(); // move sound
		return;
	}
	else if (InputManager->RightPress()) {
		_vmenu_index[1] = VIDEO_OPTIONS;
		_vmenu_index[2] = 0;
		cout << "OPTIONS: VIDEO MENU" << endl;
// 		_boot_sound[3].PlaySound(); // move sound
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
	VideoManager->Move(512, 384);
	VideoManager->SetDrawFlags(VIDEO_NO_BLEND, 0);
	VideoManager->DrawImage(_boot_images[0]);

	// Draw logo near the top of the screen
	VideoManager->Move(512, 668);
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->DrawImage(_boot_images[1]);

	// Draw main option box
	_main_options.Draw();
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
