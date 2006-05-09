///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    boot.cpp
 * \author  Viljami Korhonen, mindflayer@allacrost.org
 * \brief   Source file for boot mode interface.
 *****************************************************************************/


#include "utils.h"
#include <iostream>
#include <sstream>
#include "boot.h"
#include "boot_menu.h"
#include "audio.h"
#include "video.h"
#include "data.h"
#include "global.h"
#include "mode_manager.h"
#include "input.h"
#include "settings.h"
#include "map.h"
#include "battle.h" // tmp

using namespace std;
using namespace hoa_boot::private_boot;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_settings;
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
BootMode::BootMode() :
_main_menu(0, false, this)
{
	if (BOOT_DEBUG) cout << "BOOT: BootMode constructor invoked." << endl;

	mode_type = MODE_MANAGER_BOOT_MODE;

	// No, we're not fading out yet!
	_fade_out = false;

	// But yes, we want the darned animation already
	_logo_animating = true;
	
	// Sword setup
	_sword_x = 415.0f;
	_sword_y = 700.0f;
	_sword_angle = 90.0f;
	_sword_scale = 1.0f;

	ReadDataDescriptor read_data;
	if (!read_data.OpenFile("dat/config/boot.lua")) {
		cout << "BOOT ERROR: failed to load data file" << endl;
	}

	// Load all the bitmaps using this StillImage
	StillImage im;

	// The background
	im.SetFilename(read_data.ReadString("background_image"));
	im.SetDimensions(read_data.ReadFloat("background_image_width"),
	                 read_data.ReadFloat("background_image_height"));
	_boot_images.push_back(im);

	// The logo background
	im.SetFilename(read_data.ReadString("logo_background"));
	im.SetDimensions(read_data.ReadFloat("logo_background_width"),
	                 read_data.ReadFloat("logo_background_height"));
	_boot_images.push_back(im);

	// The big-ass sword of character whopping
	im.SetFilename(read_data.ReadString("logo_sword"));
	im.SetDimensions(read_data.ReadFloat("logo_sword_width"),
	                 read_data.ReadFloat("logo_sword_height"));
	_boot_images.push_back(im);

	// Logo text
	im.SetFilename(read_data.ReadString("logo_text"));
	im.SetDimensions(read_data.ReadFloat("logo_text_width"),
	                 read_data.ReadFloat("logo_text_height"));
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
	MusicDescriptor new_music;
	for (uint32 i = 0; i < new_music_files.size(); i++) {
		_boot_music.push_back(new_music);
		_boot_music[i].LoadMusic(new_music_files[i]);
	}

	SoundDescriptor new_sound;
	_boot_sounds.push_back(new_sound);
	_boot_sounds.push_back(new_sound);
	_boot_sounds.push_back(new_sound);
	_boot_sounds.push_back(new_sound);
	_boot_sounds[0].LoadSound(new_sound_files[0]);
	_boot_sounds[1].LoadSound(new_sound_files[1]);
	_boot_sounds[2].LoadSound(new_sound_files[2]);
	_boot_sounds[3].LoadSound(new_sound_files[3]);
	
	// This loop causes a seg fault for an unknown reason. Roots is looking into it (04/01/2006)
// 	for (uint32 i = 0; i < new_sound_files.size(); i++) {
// 		_boot_sounds.push_back(new_sound);
// 		_boot_sounds[i].LoadSound(new_sound_files[i]);
// 	}


	for (uint32 i = 0; i < _boot_images.size(); i++) {
		VideoManager->LoadImage(_boot_images[i]);
	}

	// Init the menu window
	BootMenu::InitWindow();

	// Setup every menu and their possible sub-menus
	_SetupMainMenu();
	_SetupOptionsMenu();
	_SetupVideoOptionsMenu();
	_SetupAudioOptionsMenu();

	// Set main menu as our current menu
	_current_menu = &_main_menu;
}


// The destructor frees all used music, sounds, and images.
BootMode::~BootMode() {
	if (BOOT_DEBUG) cout << "BOOT: BootMode destructor invoked." << endl;

	for (uint32 i = 0; i < _boot_music.size(); i++) {
		_boot_music[i].FreeMusic();
	}
	for (uint32 i = 0; i < _boot_sounds.size(); i++)
		_boot_sounds[i].FreeSound();
	for (uint32 i = 0; i < _boot_images.size(); i++)
		VideoManager->DeleteImage(_boot_images[i]);
}


// Resets appropriate class members.
void BootMode::Reset() {
	// Play the intro theme
	_boot_music[0].PlayMusic();
	// Set the coordinate system that BootMode uses
	VideoManager->SetCoordSys(0, 1024, 0, 768);
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
}


// Animates the logo when the boot mode starts up. Should not be called before LoadBootImages.
void BootMode::_AnimateLogo(uint32 time_elapsed) {

	// Sequence start times
	static const uint32 SEQUENCE_ONE = 1000;
	static const uint32 SEQUENCE_TWO = 1200;
	static const uint32 SEQUENCE_THREE = 1800;
	static const uint32 SEQUENCE_FOUR = 2000;

	// Total time in ms
	static uint32 total_time = 0;

	// sword velocity on the x-axis
	static float sword_velocity_x = -2.0f;

	// sword velocity on the y-axis
	static float sword_velocity_y = -2.0f;

	// Get the total time in ms
	total_time += time_elapsed;

	// Sequence one: unsheathe the sword
	if (total_time >= SEQUENCE_ONE && total_time < SEQUENCE_TWO)
	{
		sword_velocity_x -= 0.05f * time_elapsed;
		_sword_x += sword_velocity_x;
	}
	else if (total_time >= SEQUENCE_TWO && total_time < SEQUENCE_THREE)
	{
		_sword_angle += 0.5f * time_elapsed;
		_sword_x += sword_velocity_x;
		_sword_y += sword_velocity_y;
	}
	if (total_time >= SEQUENCE_FOUR)
	{
		_sword_scale = 1.0f;
		_sword_angle = 0.0f;
		_sword_x = 762.0f;
		_sword_y = 578.0f;
		_logo_animating = false;
	}

	
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


// Inits the main menu
void BootMode::_SetupMainMenu() {

	// Add all the needed menu options to the main menu
	_main_menu.AddOption(MakeWideString("New Game"), &BootMode::OnNewGame);
	_main_menu.AddOption(MakeWideString("Battle-Mode"), &BootMode::OnLoadGame);
	_main_menu.AddOption(MakeWideString("Options"), &BootMode::OnOptions);
	_main_menu.AddOption(MakeWideString("Credits"), &BootMode::OnCredits);
	_main_menu.AddOption(MakeWideString("Quit"), &BootMode::OnQuit);
}


// Inits the options menu
void BootMode::_SetupOptionsMenu() {
	_options_menu.AddOption(MakeWideString("Video"), &BootMode::OnVideoOptions);
	_options_menu.AddOption(MakeWideString("Audio"), &BootMode::OnAudioOptions);
	_options_menu.AddOption(MakeWideString("Language"));
	_options_menu.AddOption(MakeWideString("Key Settings"));
	_options_menu.AddOption(MakeWideString("Joystick Settings"));
	_options_menu.SetWindowed(true);
	_options_menu.SetParent(&_main_menu);
}


// Inits the video-options menu
void BootMode::_SetupVideoOptionsMenu()
{
	_video_options_menu.AddOption(MakeWideString("Resolution:"));

	if (VideoManager->IsFullscreen())
		_video_options_menu.AddOption(MakeWideString("Window mode: full-screen"), &BootMode::OnVideoMode);
	else
		_video_options_menu.AddOption(MakeWideString("Window mode: windowed"), &BootMode::OnVideoMode);
	_video_options_menu.AddOption(MakeWideString("Brightness:"));
	_video_options_menu.AddOption(MakeWideString("Image quality:"));
	_video_options_menu.SetWindowed(true);
	_video_options_menu.SetParent(&_options_menu);
}


// Inits the audio-options menu
void BootMode::_SetupAudioOptionsMenu()
{
	std::ostringstream sound_volume("");
	sound_volume << "Sound Volume: " << static_cast<int>(AudioManager->GetSoundVolume() * 100.0f) << " %";

	std::ostringstream music_volume("");
	music_volume << "Music Volume: " << static_cast<int>(AudioManager->GetMusicVolume() * 100.0f) << " %";

	_audio_options_menu.AddOption(MakeWideString(sound_volume.str()));
	_audio_options_menu.AddOption(MakeWideString(music_volume.str()));
	_audio_options_menu.SetWindowed(true);
	_audio_options_menu.SetParent(&_options_menu);
}


// Main menu handlers
// 'New Game' confirmed
void BootMode::OnNewGame()
{
	if (BOOT_DEBUG)
		cout << "BOOT: Starting new game." << endl;
	
	GlobalCharacter *claud = new GlobalCharacter("Claudius", "claudius", GLOBAL_CLAUDIUS);
	GlobalManager->AddCharacter(claud);
	_fade_out = true;
	VideoManager->FadeScreen(Color::black, 1.0f);
}


// 'Load Game' confirmed
void BootMode::OnLoadGame()
{
	if (BOOT_DEBUG)
		cout << "BOOT: Entering battle mode" << endl;

	BattleMode *BM = new BattleMode();
	ModeManager->Pop();
	ModeManager->Push(BM);
}


// 'Options' confirmed
void BootMode::OnOptions()
{
	_current_menu = &_options_menu;
}


// 'Credits' confirmed
void BootMode::OnCredits()
{
	if (BOOT_DEBUG)
		cout << "BOOT: Viewing credits now!" << endl;
}


// 'Quit' confirmed
void BootMode::OnQuit()
{
	SettingsManager->ExitGame();
}


// 'Video' confirmed	
void BootMode::OnVideoOptions()
{
	_current_menu = &_video_options_menu;
}


// 'Audio' confirmed
void BootMode::OnAudioOptions()
{
	_current_menu = &_audio_options_menu;
}


// 'Video mode' confirmed
void BootMode::OnVideoMode()
{
	// Toggle fullscreen / windowed
	VideoManager->ToggleFullscreen();
	VideoManager->ApplySettings();

	// Change the text accordingly
	if (VideoManager->IsFullscreen())
		_video_options_menu.SetOptionText(1, MakeWideString("Window mode: full-screen"));
	else
		_video_options_menu.SetOptionText(1, MakeWideString("Window mode: windowed"));
}


// This is called once every frame iteration to update the status of the game
void BootMode::Update() {
	uint32 time_elapsed = SettingsManager->GetUpdateTime();

	// Screen is in the process of fading out
	if (_fade_out)
	{
		// When the screen is finished fading to black, create a new map mode and fade back in
		if (!VideoManager->IsFading()) {
			MapMode *MM = new MapMode();
			ModeManager->Pop();
			ModeManager->Push(MM);
			VideoManager->FadeScreen(Color::clear, 1.0f);
		}
		return;
	}

	// Main logo is animating at the moment
	if (_logo_animating)
	{
		_AnimateLogo(time_elapsed);
	}

	// Update the menu window
	BootMenu::UpdateWindow(time_elapsed);

	// A key was pressed -> handle it 
	if (InputManager->ConfirmPress()) 
	{
		// Play the confirm sound (TODO: Perhaps we shouldn't play this if nothing happened on confirm?)
		_boot_sounds.at(0).PlaySoundA();

		_current_menu->ConfirmPressed();
		
		// Update window status
		if (_current_menu->IsWindowed())
		{
			BootMenu::ShowWindow(true);
		}
		else
		{
			BootMenu::ShowWindow(false);
		}
	}
	else if (InputManager->LeftPress()) 
	{
		_current_menu->LeftPressed();
	}
	else if(InputManager->RightPress()) 
	{
		_current_menu->RightPressed();
	}
	else if(InputManager->UpPress())
	{
		_current_menu->UpPressed();
	}
	else if(InputManager->DownPress())
	{
		_current_menu->DownPressed();
	}
	else if(InputManager->CancelPress())
	{
		_current_menu->CancelPressed();

		// Go up in the menu hierarchy if possible
		if (_current_menu->GetParent() != 0)
		{
			// Play the cancel sound here
			_boot_sounds.at(1).PlaySoundA();

			_current_menu = _current_menu->GetParent();

			// Update window status again
			if (_current_menu->IsWindowed())
			{
				BootMenu::ShowWindow(true);
			}
			else
			{
				BootMenu::ShowWindow(false);
			}
		}
	}

	// Update menu events
	_current_menu->GetEvent();	
}


// Draws our next frame to the video back buffer
void BootMode::Draw() {

	// Draw the background image
	VideoManager->Move(512, 384);
	VideoManager->SetDrawFlags(VIDEO_NO_BLEND, 0);
	VideoManager->DrawImage(_boot_images[0]);

	// Draw the logo background
	VideoManager->Move(512, 648);
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->DrawImage(_boot_images[1]);

	// Draw the sword with it's custom angle and scale
	VideoManager->Move(_sword_x, _sword_y);
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->Scale(_sword_scale, _sword_scale);
	VideoManager->Rotate(_sword_angle);
	VideoManager->DrawImage(_boot_images[2]);

	// Draw the logo text on top of the sword
	VideoManager->Move(512, 648);
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->DrawImage(_boot_images[3]);

	// Draw the currently visible menu (and it's window if needed)
	_current_menu->Draw();

	return;
}


} // namespace hoa_boot
