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
 * \date    Last Updated: February 16th, 2006
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
// 	for (uint32 i = 0; i < new_music_files.size(); i++) {
// 		_boot_music.push_back(new_music);
// 		_boot_music[i].LoadMusic(new_music_files[i]);
// 	}
 
 	SoundDescriptor new_sound;
 	for (uint32 i = 0; i < new_sound_files.size(); i++) {
 		_boot_sounds.push_back(new_sound);
 		_boot_sounds[i].LoadSound(new_sound_files[i]);
 	}

	for (uint32 i = 0; i < _boot_images.size(); i++) {
		VideoManager->LoadImage(_boot_images[i]);
	}

	// Init the settings window
	_settings_window.Create(1024.0f, 400.0f);
	_settings_window.SetPosition(0.0f, 576.0f);
	_settings_window.SetDisplayMode(VIDEO_MENU_EXPAND_FROM_CENTER);
	_settings_window.Hide();

	// Setup every menu and their possible sub-menus
	_SetupMainMenu();
	_SetupOptionsMenu();
	_SetupVideoOptionsMenu();
	_SetupAudioOptionsMenu();

	// Set main menu as our currently selected menu...
	_current_menu = &_main_menu;
	_current_menu_visible = MAIN_MENU_VISIBLE;

	if (BOOT_DEBUG) cout << "BOOT MODE CONSTRUCTOR END" << endl;
}


// The destructor frees all used music, sounds, and images.
BootMode::~BootMode() {
	if (BOOT_DEBUG) cout << "BOOT: BootMode destructor invoked." << endl;

	_settings_window.Destroy();

	for (uint32 i = 0; i < _boot_music.size(); i++)
		_boot_music[i].FreeMusic();
	for (uint32 i = 0; i < _boot_sounds.size(); i++)
		_boot_sounds[i].FreeSound();
	for (uint32 i = 0; i < _boot_images.size(); i++)
		VideoManager->DeleteImage(_boot_images[i]);
}


// Resets appropriate class members.
void BootMode::Reset() {
	// Play the intro theme
	// _boot_music[0].PlayMusic();
	// Set the coordinate system that BootMode uses
	VideoManager->SetCoordSys(0, 1024, 0, 768);
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


// Inits 'default' looks for a given horizontal menu
void BootMode::_InitMenuDefaults(hoa_video::OptionBox & menu) {
	menu.SetFont("default");
	menu.SetCellSize(128.0f, 50.0f);
	//menu.SetSize(5, 1);
	menu.SetPosition(512.0f, 50.0f);
	menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	menu.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	menu.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	menu.SetCursorOffset(-35.0f, -4.0f);
}


// Inits 'default' looks for a given vertical menu inside a window
void BootMode::_InitWindowMenuDefaults(hoa_video::OptionBox & menu, hoa_video::MenuWindow & window)
{
	menu.SetFont("default");
	menu.SetCellSize(128.0f, 50.0f);
	//menu.SetSize(1, 5);
	menu.SetPosition(410.0f, 200.0f);
	menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	menu.SetOptionAlignment(VIDEO_X_RIGHT, VIDEO_Y_CENTER);
	menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	menu.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	menu.SetCursorOffset(-35.0f, -4.0f);
	menu.SetOwner(&window); // For some reason, I can't get this to work properly! Help appreciated
}


// Inits the main menu
void BootMode::_SetupMainMenu() {

	// Set defaults
	_InitMenuDefaults(_main_menu);
	_main_menu.SetSize(MAIN_MENU_SIZE, 1);

	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeWideString("New Game"));
	options.push_back(MakeWideString("Load Game"));
	options.push_back(MakeWideString("Options"));
	options.push_back(MakeWideString("Credits"));
	options.push_back(MakeWideString("Quit"));
	
	// Add strings and set the default selection.
	_main_menu.SetOptions(options);
	_main_menu.SetSelection(NEW_GAME);
}


// Inits the options menu
void BootMode::_SetupOptionsMenu() {

	// Set defaults
	_InitMenuDefaults(_options_menu);
	_options_menu.SetSize(OPTIONS_MENU_SIZE, 1);

	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeWideString("Video"));
	options.push_back(MakeWideString("Audio"));
	options.push_back(MakeWideString("Language"));
	options.push_back(MakeWideString("Key Settings"));
	options.push_back(MakeWideString("Joystick Settings"));
	options.push_back(MakeWideString("Back"));
	
	// Add strings and set the default selection.
	_options_menu.SetOptions(options);
	_options_menu.SetSelection(VIDEO_OPTIONS);
}


// Inits the video-options menu
void BootMode::_SetupVideoOptionsMenu()
{
	// Set defaults
	_InitWindowMenuDefaults(_video_options_menu, _settings_window);
	_video_options_menu.SetSize(1, VIDEO_OPTIONS_MENU_SIZE);

	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeWideString("Resolution:"));
	options.push_back(MakeWideString("Window mode:"));
	options.push_back(MakeWideString("Brightness:"));
	options.push_back(MakeWideString("Image quality:"));
	options.push_back(MakeWideString("Back"));

	// Add strings and set the default selection.
	_video_options_menu.SetOptions(options);
	_video_options_menu.SetSelection(RESOLUTION_VIDEO_OPTIONS);
}


// Inits the audio-options menu
void BootMode::_SetupAudioOptionsMenu()
{
	// Set defaults
	_InitWindowMenuDefaults(_audio_options_menu, _settings_window);
	_audio_options_menu.SetSize(1, AUDIO_OPTIONS_MENU_SIZE);

	// Generate the strings
	vector<ustring> options;
	options.push_back(MakeWideString("Sound Volume:"));
	options.push_back(MakeWideString("Music Volume:"));
	options.push_back(MakeWideString("Back"));

	// Add strings and set the default selection.
	_audio_options_menu.SetOptions(options);
	_audio_options_menu.SetSelection(SOUND_VOLUME_AUDIO_OPTIONS);
}


// Handles main menu events
void BootMode::_HandleMainMenu(private_boot::MAIN_MENU selection) {
	switch (selection)
	{
	case NEW_GAME:
		{
			if (BOOT_DEBUG) cout << "BOOT: Starting new game." << endl;
			GlobalCharacter *claud = new GlobalCharacter("Claudius", "claudius", GLOBAL_CLAUDIUS);
			GlobalManager->AddCharacter(claud);
			_fade_out = true;
			VideoManager->FadeScreen(Color::black, 1.0f);
			break;
		}
	case LOAD_GAME:
		{
			cout << "BOOT: TEMP: Entering battle mode" << endl;
			BattleMode *BM = new BattleMode();
			ModeManager->Pop();
			ModeManager->Push(BM);
			break;
		}
	case OPTIONS:
		{
			_current_menu = &_options_menu;
			_current_menu_visible = OPTIONS_MENU_VISIBLE;
			break;
		}
	case CREDITS:
		{
			cout << "BOOT: TEMP: Viewing credits now!" << endl;
			break;
		}
	case QUIT:
		{
			SettingsManager->ExitGame();
			break;
		}
	default:
		{
			cerr << "BOOT: ERROR: Invalid selection #" << static_cast<uint32>(selection) << " in main-menu" << endl;
			break;
		}
	}

}


// Handles options menu events
void BootMode::_HandleOptionsMenu(private_boot::OPTIONS_MENU selection) {
	switch (selection)
	{
	case VIDEO_OPTIONS:
		{
			_current_menu = &_video_options_menu;
			_current_menu_visible = VIDEO_OPTIONS_MENU_VISIBLE;
			_settings_window.Show();
			break;
		}
	case AUDIO_OPTIONS:
		{
			_current_menu = &_audio_options_menu;
			_current_menu_visible = AUDIO_OPTIONS_MENU_VISIBLE;
			_settings_window.Show();
			break;
		}
	case BACK_OPTIONS:
		{
			// Return to the main menu
			_current_menu = &_main_menu;
			_current_menu_visible = MAIN_MENU_VISIBLE;
			break;
		}
	default:
		{
			cerr << "BOOT: ERROR: Invalid selection #" << static_cast<uint32>(selection) << " in options-menu" << endl;
			break;
		}
	}
}


// Handles video-options menu events
void BootMode::_HandleVideoOptionsMenu(private_boot::VIDEO_OPTIONS_MENU selection)
{
	switch (selection)
	{
	case FULLWINDOWED_VIDEO_OPTIONS:
		{
			SettingsManager->ToggleFullScreen();
			break;
		}
	case BACK_VIDEO_OPTIONS:
		{
			// Return to the options menu
			_current_menu = &_options_menu;
			_current_menu_visible = OPTIONS_MENU_VISIBLE;
			_settings_window.Hide();
			break;
		}
	default:
		{
			cerr << "BOOT: ERROR: Invalid selection #" << static_cast<uint32>(selection) << " in video-options menu" << endl;
			break;
		}
	}
}


// Handles audio-options menu events
void BootMode::_HandleAudioOptionsMenu(private_boot::AUDIO_OPTIONS_MENU selection)
{
	switch (selection)
	{
	case BACK_AUDIO_OPTIONS:
		{
			// Return to the options menu
			_current_menu = &_options_menu;
			_current_menu_visible = OPTIONS_MENU_VISIBLE;
			_settings_window.Hide();
			break;
		}
	default:
		{
			cerr << "BOOT: ERROR: Invalid selection #" << static_cast<uint32>(selection) << " in audio-options menu" << endl;
			break;
		}
	}
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

	// Update settings window
	_settings_window.Update(time_elapsed);

	// A key was pressed -> handle it 
	// (sounds maybe be added inside these later on (like this: _boot_sounds[2].PlaySound(); )
	if (InputManager->ConfirmPress()) 
	{
		_current_menu->HandleConfirmKey();
	}
	else if (InputManager->LeftPress()) 
	{
		_current_menu->HandleLeftKey();
	}
	else if(InputManager->RightPress()) 
	{
		_current_menu->HandleRightKey();
	}
	else if(InputManager->UpPress())
	{
		_current_menu->HandleUpKey();
	}
	else if(InputManager->DownPress())
	{
		_current_menu->HandleDownKey();
	}
	else if(InputManager->CancelPress())
	{
		_current_menu->HandleCancelKey();
	}

	// Get the latest event from the current menu
	int32 event = _current_menu->GetEvent();
	
	// Confirm was pressed -> Find out which menu is visible and handle the confirm
	if (event == VIDEO_OPTION_CONFIRM)
	{
		switch (_current_menu_visible)
		{
			// Handle main menu confirm
			case MAIN_MENU_VISIBLE:
			{
				this->_HandleMainMenu(static_cast<MAIN_MENU>(_current_menu->GetSelection()));
				break;
			}
			// Handle options menu confirm
			case OPTIONS_MENU_VISIBLE:
			{
				this->_HandleOptionsMenu(static_cast<OPTIONS_MENU>(_current_menu->GetSelection()));
				break;
			}
			// Handle video-options menu confirm
			case VIDEO_OPTIONS_MENU_VISIBLE:
			{
				this->_HandleVideoOptionsMenu(static_cast<VIDEO_OPTIONS_MENU>(_current_menu->GetSelection()));
				break;
			}
			// Handle audio-options menu confirm
			case AUDIO_OPTIONS_MENU_VISIBLE:
			{
				_HandleAudioOptionsMenu(static_cast<AUDIO_OPTIONS_MENU>(_current_menu->GetSelection()));
				break;
			}

			default:
			{
				cerr << "BOOT: ERROR: Invalid menu #" << static_cast<uint32>(_current_menu_visible) << " visible!" << endl;
				break;
			}
		} // end switch
	} // end VIDEO_OPTION_CONFIRM
}


// ****************************************************************************
// ***************************** DRAW FUNCTIONS *******************************
// ****************************************************************************


// Draws our next frame to the video back buffer
void BootMode::Draw() {

	// Draw the background image
	VideoManager->Move(512, 384);
	VideoManager->SetDrawFlags(VIDEO_NO_BLEND, 0);
	VideoManager->DrawImage(_boot_images[0]);

	// Draw logo near the top of the screen
	VideoManager->Move(512, 668);
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->DrawImage(_boot_images[1]);

	// Draw the extra-settings window if it's set visible
	_settings_window.Draw();

	// Draw the currently visible OptionBox menu
	_current_menu->Draw();

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
	//_settings_window.Draw();
}

// Draws the audio options menu.
void BootMode::_DrawAudioOptions() {
	//_settings_window.Draw();
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
