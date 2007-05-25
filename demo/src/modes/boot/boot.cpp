///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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
#include "boot_credits.h"
#include "audio.h"
#include "video.h"
#include "script.h"
#include "global.h"
#include "mode_manager.h"
#include "input.h"
#include "system.h"
#include "map.h"
#include "battle.h" // tmp
#include "menu.h" // even more tmp
#include "shop.h" // tmp
#include <fstream> // more more tmp

using namespace std;
using namespace hoa_boot::private_boot;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_script;
using namespace hoa_map;
using namespace hoa_battle; // tmp

namespace hoa_boot {

bool BOOT_DEBUG = false;


// Initialize static members here
bool BootMode::_logo_animating = true;
uint32 welcome = 0;


// ****************************************************************************
// *************************** GENERAL FUNCTIONS ******************************
// ****************************************************************************

// The constructor initializes variables and sets up the path names of the boot images
BootMode::BootMode() :
	_fade_out(false),
	_main_menu(0, false, this),
	_latest_version(true),
	_has_modified_settings(false)
{
	if (BOOT_DEBUG) cout << "BOOT: BootMode constructor invoked." << endl;
	mode_type = MODE_MANAGER_BOOT_MODE;

	ScriptDescriptor read_data;
	if (!read_data.OpenFile("dat/config/boot.lua", SCRIPT_READ)) {
		cout << "BOOT ERROR: failed to load data file" << endl;
	}

	// Load all bitmaps using this StillImage
	StillImage im;

	// background
	im.SetFilename(read_data.ReadString("background_image"));
	im.SetDimensions(read_data.ReadFloat("background_image_width"),
	                 read_data.ReadFloat("background_image_height"));
	_boot_images.push_back(im);

	// logo background
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
	read_data.ReadStringVector("music_files", new_music_files);

	vector<string> new_sound_files;
	read_data.ReadStringVector("sound_files", new_sound_files);

	if (read_data.GetError() != SCRIPT_NO_ERRORS) {
		cout << "BOOT ERROR: some error occured during reading of boot data file" << endl;
	}

	// Load all music files used in boot mode
	_boot_music.resize(new_music_files.size(), MusicDescriptor());
	for (uint32 i = 0; i < new_music_files.size(); i++) {
		if (_boot_music[i].LoadMusic(new_music_files[i]) == false) {
			cout << "BOOT: failed to load music file " << new_music_files[i] << endl;
			SystemManager->ExitGame();
			return;
		}
	}

	SoundDescriptor new_sound;
	_boot_sounds.push_back(new_sound);
	_boot_sounds.push_back(new_sound);
	_boot_sounds.push_back(new_sound);
	_boot_sounds.push_back(new_sound);
	_boot_sounds.push_back(new_sound);
	_boot_sounds[0].LoadSound(new_sound_files[0]);
	_boot_sounds[1].LoadSound(new_sound_files[1]);
	_boot_sounds[2].LoadSound(new_sound_files[2]);
	_boot_sounds[3].LoadSound(new_sound_files[3]);
	_boot_sounds[4].LoadSound(new_sound_files[4]);

	// Check the version
	_latest_version = true; //IsLatestVersion();
	if (!_latest_version)
		_latest_version_number = GetLatestVersion();
	else
		_latest_version_number = "";

	// This loop causes a seg fault for an unknown reason. Roots is looking into it (04/01/2006)
// 	for (uint32 i = 0; i < new_sound_files.size(); i++) {
// 		_boot_sounds.push_back(new_sound);
// 		_boot_sounds[i].LoadSound(new_sound_files[i]);
// 	}

	// Load all bitmaps
	for (uint32 i = 0; i < _boot_images.size(); i++) {
		VideoManager->LoadImage(_boot_images[i]);
	}

	// Construct our menu hierarchy here
	_SetupMainMenu();
	_SetupOptionsMenu();
	_SetupVideoOptionsMenu();
	_SetupAudioOptionsMenu();
	_SetupKeySetttingsMenu();
	_SetupJoySetttingsMenu();
	_SetupResolutionMenu();

	// Set the main menu as our currently selected menu
	_current_menu = &_main_menu;
}


// The destructor frees all used music, sounds, and images.
BootMode::~BootMode() {
	if (BOOT_DEBUG) cout << "BOOT: BootMode destructor invoked." << endl;

	for (uint32 i = 0; i < _boot_music.size(); i++)
		_boot_music[i].FreeMusic();

	for (uint32 i = 0; i < _boot_sounds.size(); i++)
		_boot_sounds[i].FreeSound();

	for (uint32 i = 0; i < _boot_images.size(); i++)
		VideoManager->DeleteImage(_boot_images[i]);
}


// Resets appropriate class members.
void BootMode::Reset() {
	// Set the coordinate system that BootMode uses
	VideoManager->SetCoordSys(0.0f, 1024.0f, 0.0f, 768.0f);
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->DisableFog(); // Turn off any remaining fog
	VideoManager->SetTextColor(Color::white);

	// Reset the game universe
	GlobalManager->ClearAllData();

	// Decide which music track to play
	if (_logo_animating)
		_boot_music.at(1).PlayMusic(); // Opening Effect
	else
		_boot_music.at(0).PlayMusic(); // Main theme
}


// Animates the logo when the boot mode starts up. Should not be called before LoadBootImages.
void BootMode::_AnimateLogo() {
	// Sequence starting times. Note: I've changed _every_ variable here into floats
	// to avoid unneccessary type casts that would kill performance! -Viljami
	static const float SEQUENCE_ONE = 0.0f;
	static const float SEQUENCE_TWO = SEQUENCE_ONE + 1000.0f;
	static const float SEQUENCE_THREE = SEQUENCE_TWO + 2000.0f;
	static const float SEQUENCE_FOUR = SEQUENCE_THREE + 575.0f;
	static const float SEQUENCE_FIVE = SEQUENCE_FOUR + 1900.0f;
	static const float SEQUENCE_SIX = SEQUENCE_FIVE + 1400.0f;
	static const float SEQUENCE_SEVEN = SEQUENCE_SIX + 3500.0f;

	// Sword setup
	static float sword_x = 670.0f;
	static float sword_y = 360.0f;
	static float rotation = -90.0f;

	// Total time in ms
	static float total_time = 0.0f;

	// Get the frametime and update total time
	float time_elapsed = static_cast<float>(SystemManager->GetUpdateTime());
	total_time += time_elapsed;

	// Sequence one: black
	if (total_time >= SEQUENCE_ONE && total_time < SEQUENCE_TWO)
	{
	}
	// Sequence two: fade in logo+sword
	else if (total_time >= SEQUENCE_TWO && total_time < SEQUENCE_THREE)
	{
		float alpha = (total_time - SEQUENCE_TWO) / (SEQUENCE_THREE - SEQUENCE_TWO);

		VideoManager->Move(512.0f, 385.0f); // logo bg
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->DrawImage(_boot_images[1], Color(alpha, alpha, alpha, 1.0f));
		VideoManager->Move(sword_x, sword_y); // sword
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->Rotate(-90.0f);
		VideoManager->DrawImage(_boot_images[2], Color(alpha, alpha, alpha, 1.0f));
		VideoManager->Move(512.0f, 385.0f); // text
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->DrawImage(_boot_images[3], Color(alpha, alpha, alpha, 1.0f));
	}
	// Sequence three: Sword unsheathe & slide
	else if (total_time >= SEQUENCE_THREE && total_time < SEQUENCE_FOUR)
	{
		float dt = (total_time - SEQUENCE_THREE) * 0.001f;
		sword_x = 670.0f + (dt * dt) * 660.0f; // s = s0 + 0.5 * a * t^2
		VideoManager->Move(512.0f, 385.0f); // logo bg
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->DrawImage(_boot_images[1]);
		VideoManager->Move(sword_x, sword_y); // sword
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->Rotate(-90.0f);
		VideoManager->DrawImage(_boot_images[2]);
		VideoManager->Move(512.0f, 385.0f); // text
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->DrawImage(_boot_images[3]);
	}
	// Sequence four: Spin around the sword
	else if (total_time >= SEQUENCE_FOUR && total_time < SEQUENCE_FIVE)
	{
		const float ROTATIONS = 720.0f + 90.0f;
		const float SPEED_LEFT = 35.0f;
		const float SPEED_UP = 750.0f;
		const float GRAVITY = 120.0f;

		// Delta goes from 0.0f to 1.0f
		float delta = ((total_time - SEQUENCE_FOUR) / (SEQUENCE_FIVE - SEQUENCE_FOUR));
		float dt = (total_time - SEQUENCE_FOUR) * 0.001f;
		sword_x = 885.941f - dt*dt * SPEED_LEFT; // Small accelerated movement to left
		sword_y = 360.0f   - dt*dt * GRAVITY + SPEED_UP * delta;
		rotation = -90.0f + delta * ROTATIONS;

		VideoManager->Move(512.0f, 385.0f); // logo bg
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->DrawImage(_boot_images[1]);
		VideoManager->Move(512.0f, 385.0f); // text
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->DrawImage(_boot_images[3]);
		VideoManager->Move(sword_x, sword_y); // sword
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->Rotate(rotation);
		VideoManager->DrawImage(_boot_images[2]);
	}
	// Sequence five: Sword comes back
	else if (total_time >= SEQUENCE_FIVE && total_time < SEQUENCE_SIX)
	{
		// Delta goes from 0.0f to 1.0f
		float delta_root = (total_time - SEQUENCE_FIVE) / (SEQUENCE_SIX - SEQUENCE_FIVE);
		float delta = delta_root * delta_root * delta_root * delta_root;
		float newX = (1.0f - delta) * sword_x + 762.0f * delta;
		float newY = (1.0f - delta) * sword_y + 310.0f * delta;

		VideoManager->Move(512.0f, 385.0f); // logo bg
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->DrawImage(_boot_images[1]);
		VideoManager->Move(512.0f, 385.0f); // text
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->DrawImage(_boot_images[3]);
		VideoManager->Move(newX, newY); // sword
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->DrawImage(_boot_images[2]);
	}
	// Sequence six: flash of light
	else if (total_time >= SEQUENCE_SIX && total_time < SEQUENCE_SEVEN)
	{
		// Delta goes from 1.0f to 0.0f
		float delta = (total_time - SEQUENCE_SIX) / (SEQUENCE_SEVEN - SEQUENCE_SIX);
		delta = 1.0f - delta * delta;
		VideoManager->EnableFog(Color::white, delta);
		_DrawBackgroundItems();
	}
	else if (total_time >= SEQUENCE_SEVEN)
	{
		_EndOpeningAnimation();
		_DrawBackgroundItems();
	}
}


// Draws background image, logo and sword at their default locations
void BootMode::_DrawBackgroundItems() {
	VideoManager->Move(512.0f, 384.0f);
	VideoManager->SetDrawFlags(VIDEO_NO_BLEND, 0);
	VideoManager->DrawImage(_boot_images[0]); // Draw background

	VideoManager->Move(512.0f, 648.0f);
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->DrawImage(_boot_images[1]); // Draw the logo background

	VideoManager->Move(762.0f, 578.0f);
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->DrawImage(_boot_images[2]); // Draw the sword

	VideoManager->Move(512, 648);
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->DrawImage(_boot_images[3]); // Draw the logo text
}


// Stops playback of the opening animation
void BootMode::_EndOpeningAnimation() {
	VideoManager->DisableFog(); // Turn off the fog

	// Stop playing SFX and start playing the main theme
	_boot_music.at(1).SetFadeOutTime(1000);
	_boot_music.at(1).StopMusic();
	_boot_music.at(0).SetFadeInTime(5000);
	_boot_music.at(0).PlayMusic();
	
	// Load the settings file for reading in the welcome variable
	ScriptDescriptor settings_lua;
	if (!settings_lua.OpenFile("dat/config/settings.lua", SCRIPT_READ)) {
		cout << "BOOT ERROR: failed to load the settings file!" << endl;
	}
	welcome = settings_lua.ReadInt("welcome");
	settings_lua.CloseFile();
	if (welcome) {
		_welcome_screen.Show();
	}

	_logo_animating = false;
}


// Waits infinitely for a key press
SDLKey BootMode::_WaitKeyPress() {
	SDL_Event event;
	while (SDL_WaitEvent(&event)) {
		if (event.type == SDL_KEYDOWN)
			return event.key.keysym.sym;
	}

	return event.key.keysym.sym; // This line is actually never reached but the compiler will complain if it's removed :)
}


// Waits infinitely for a joystick press
uint8 BootMode::_WaitJoyPress() {
	SDL_Event event;
	while (SDL_WaitEvent(&event)) {
		if (event.type == SDL_JOYBUTTONDOWN)
			return event.jbutton.button;
	}

	return event.jbutton.button;
}


// Redefines a key to be mapped to another command. Waits for keypress using _WaitKeyPress()
void BootMode::_RedefineUpKey() {
	InputManager->SetUpKey(_WaitKeyPress());
	_UpdateKeySettings();
}

void BootMode::_RedefineDownKey() {
	InputManager->SetDownKey(_WaitKeyPress());
	_UpdateKeySettings();
}

void BootMode::_RedefineLeftKey() {
	InputManager->SetLeftKey(_WaitKeyPress());
	_UpdateKeySettings();
}

void BootMode::_RedefineRightKey() {
	InputManager->SetRightKey(_WaitKeyPress());
	_UpdateKeySettings();
}

void BootMode::_RedefineConfirmKey() {
	InputManager->SetConfirmKey(_WaitKeyPress());
	_UpdateKeySettings();
}

void BootMode::_RedefineCancelKey() {
	InputManager->SetCancelKey(_WaitKeyPress());
	_UpdateKeySettings();
}

void BootMode::_RedefineMenuKey() {
	InputManager->SetMenuKey(_WaitKeyPress());
	_UpdateKeySettings();
}

void BootMode::_RedefineSwapKey() {
	InputManager->SetSwapKey(_WaitKeyPress());
	_UpdateKeySettings();
}

void BootMode::_RedefineLeftSelectKey() {
	InputManager->SetLeftSelectKey(_WaitKeyPress());
	_UpdateKeySettings();
}

void BootMode::_RedefineRightSelectKey() {
	InputManager->SetRightSelectKey(_WaitKeyPress());
	_UpdateKeySettings();
}

void BootMode::_RedefinePauseKey() {
	InputManager->SetPauseKey(_WaitKeyPress());
	_UpdateKeySettings();
}


// Redefines a joystick button to be mapped to another command. Waits for press using _WaitJoyPress()
void BootMode::_RedefineConfirmJoy() {
	InputManager->SetConfirmJoy(_WaitJoyPress());
	_UpdateJoySettings();
}
void BootMode::_RedefineCancelJoy() {
	InputManager->SetCancelJoy(_WaitJoyPress());
	_UpdateJoySettings();
}
void BootMode::_RedefineMenuJoy() {
	InputManager->SetMenuJoy(_WaitJoyPress());
	_UpdateJoySettings();
}
void BootMode::_RedefineSwapJoy() {
	InputManager->SetSwapJoy(_WaitJoyPress());
	_UpdateJoySettings();
}
void BootMode::_RedefineLeftSelectJoy() {
	InputManager->SetLeftSelectJoy(_WaitJoyPress());
	_UpdateJoySettings();
}
void BootMode::_RedefineRightSelectJoy() {
	InputManager->SetRightSelectJoy(_WaitJoyPress());
	_UpdateJoySettings();
}
void BootMode::_RedefinePauseJoy() {
	InputManager->SetPauseJoy(_WaitJoyPress());
	_UpdateJoySettings();
}


// Inits the main menu
void BootMode::_SetupMainMenu() {

	// Add all the needed menu options to the main menu
	_main_menu.AddOption(MakeUnicodeString("New Game"), &BootMode::_OnNewGame);
	_main_menu.AddOption(MakeUnicodeString("Load Game"));
	_main_menu.AddOption(MakeUnicodeString("Options"), &BootMode::_OnOptions);
	_main_menu.AddOption(MakeUnicodeString("Credits"), &BootMode::_OnCredits);
	_main_menu.AddOption(MakeUnicodeString("Quit"), &BootMode::_OnQuit);
	_main_menu.AddOption(MakeUnicodeString("Battle"), &BootMode::_OnBattleDebug);
	_main_menu.AddOption(MakeUnicodeString("Menu"), &BootMode::_OnMenuDebug);
	_main_menu.AddOption(MakeUnicodeString("Shop"), &BootMode::_OnShopDebug);

	_main_menu.EnableOption(1, false); // gray out "load game" for now.
}


// Inits the options menu
void BootMode::_SetupOptionsMenu() {
	_options_menu.AddOption(MakeUnicodeString("Video"), &BootMode::_OnVideoOptions);
	_options_menu.AddOption(MakeUnicodeString("Audio"), &BootMode::_OnAudioOptions);
	_options_menu.AddOption(MakeUnicodeString("Language"));
	_options_menu.AddOption(MakeUnicodeString("Key Settings"), &BootMode::_OnKeySettings);
	_options_menu.AddOption(MakeUnicodeString("Joystick Settings"), &BootMode::_OnJoySettings);

	// Disable some of the options
	_options_menu.EnableOption(2, false); // Language

	_options_menu.SetWindowed(true);
	_options_menu.SetParent(&_main_menu);
}


// Inits the video-options menu
void BootMode::_SetupVideoOptionsMenu()
{
	_video_options_menu.AddOption(MakeUnicodeString("Resolution:"), &BootMode::_OnResolution);
	_video_options_menu.AddOption(MakeUnicodeString("Window mode:"), &BootMode::_OnVideoMode, &BootMode::_OnVideoMode, &BootMode::_OnVideoMode); // Left & right will change window mode as well as plain 'confirm' !
	_video_options_menu.AddOption(MakeUnicodeString("Brightness:"), 0, &BootMode::_OnBrightnessLeft, &BootMode::_OnBrightnessRight);
	_video_options_menu.AddOption(MakeUnicodeString("Image quality:"));

	_video_options_menu.EnableOption(3, false); // disable image quality
	_video_options_menu.SetWindowed(true);
	_video_options_menu.SetParent(&_options_menu);
}


// Inits the audio-options menu
void BootMode::_SetupAudioOptionsMenu()
{
	_audio_options_menu.AddOption(MakeUnicodeString("Sound Volume: "), 0, &BootMode::_OnSoundLeft, &BootMode::_OnSoundRight);
	_audio_options_menu.AddOption(MakeUnicodeString("Music Volume: "), 0, &BootMode::_OnMusicLeft, &BootMode::_OnMusicRight);
	_audio_options_menu.SetWindowed(true);
	_audio_options_menu.SetParent(&_options_menu);
}


// Inits the key-settings menu
void BootMode::_SetupKeySetttingsMenu() {
	_key_settings_menu.AddOption(MakeUnicodeString("Up: "), &BootMode::_RedefineUpKey);
	_key_settings_menu.AddOption(MakeUnicodeString("Down: "), &BootMode::_RedefineDownKey);
	_key_settings_menu.AddOption(MakeUnicodeString("Left: "), &BootMode::_RedefineLeftKey);
	_key_settings_menu.AddOption(MakeUnicodeString("Right: "), &BootMode::_RedefineRightKey);
	_key_settings_menu.AddOption(MakeUnicodeString("Confirm: "), &BootMode::_RedefineConfirmKey);
	_key_settings_menu.AddOption(MakeUnicodeString("Cancel: "), &BootMode::_RedefineCancelKey);
	_key_settings_menu.AddOption(MakeUnicodeString("Menu: "), &BootMode::_RedefineMenuKey);
	_key_settings_menu.AddOption(MakeUnicodeString("Swap: "), &BootMode::_RedefineSwapKey);
	_key_settings_menu.AddOption(MakeUnicodeString("Left Select: "), &BootMode::_RedefineLeftSelectKey);
	_key_settings_menu.AddOption(MakeUnicodeString("Right Select: "), &BootMode::_RedefineRightSelectKey);
	_key_settings_menu.AddOption(MakeUnicodeString("Pause: "), &BootMode::_RedefinePauseKey);

	_key_settings_menu.AddOption(MakeUnicodeString("Restore defaults"), &BootMode::_OnRestoreDefaultKeys);
	_key_settings_menu.SetWindowed(true);
	_key_settings_menu.SetParent(&_options_menu);
	_key_settings_menu.SetTextDensity(30.0f); // Shorten the distance between text lines
}


void BootMode::_SetupJoySetttingsMenu() {
	_joy_settings_menu.AddOption(MakeUnicodeString("Confirm: "), &BootMode::_RedefineConfirmJoy);
	_joy_settings_menu.AddOption(MakeUnicodeString("Cancel: "), &BootMode::_RedefineCancelJoy);
	_joy_settings_menu.AddOption(MakeUnicodeString("Menu: "), &BootMode::_RedefineMenuJoy);
	_joy_settings_menu.AddOption(MakeUnicodeString("Swap: "), &BootMode::_RedefineSwapJoy);
	_joy_settings_menu.AddOption(MakeUnicodeString("Left Select: "), &BootMode::_RedefineLeftSelectJoy);
	_joy_settings_menu.AddOption(MakeUnicodeString("Right Select: "), &BootMode::_RedefineRightSelectJoy);
	_joy_settings_menu.AddOption(MakeUnicodeString("Pause: "), &BootMode::_RedefinePauseJoy);

	_joy_settings_menu.AddOption(MakeUnicodeString("Restore defaults"), &BootMode::_OnRestoreDefaultJoyButtons);
	_joy_settings_menu.SetWindowed(true);
	_joy_settings_menu.SetParent(&_options_menu);
	_joy_settings_menu.SetTextDensity(40.0f); // Shorten the distance between text lines
}


void BootMode::_SetupResolutionMenu() {
	_resolution_menu.AddOption(MakeUnicodeString("640 x 480"), &BootMode::_OnResolution640x480);
	_resolution_menu.AddOption(MakeUnicodeString("800 x 600"), &BootMode::_OnResolution800x600);
	_resolution_menu.AddOption(MakeUnicodeString("1024 x 768"), &BootMode::_OnResolution1024x768);
	_resolution_menu.SetParent(&_video_options_menu);
	_resolution_menu.SetWindowed(true);
}


// Main menu handlers
// 'New Game' confirmed
void BootMode::_OnNewGame() {
	if (BOOT_DEBUG)	cout << "BOOT: Starting new game." << endl;

	_SaveSettingsFile();

	GlobalManager->AddCharacter(GLOBAL_CHARACTER_CLAUDIUS);
	GlobalManager->SetFunds(500);

	_fade_out = true;
	VideoManager->FadeScreen(Color::black, 1.0f);
	_boot_music.at(0).SetFadeOutTime(500); // Fade out the music
	_boot_music.at(0).StopMusic();
}


// 'Load Game' confirmed. Not done yet, sorry mate.
void BootMode::_OnLoadGame() {
}


// 'Options' confirmed
void BootMode::_OnOptions() {
	_current_menu = &_options_menu;
	_has_modified_settings = true; // Lazy way to check if we've changed anything. The correct way is to move this line in every setting-modifying function...
}


// 'Credits' confirmed
void BootMode::_OnCredits() {
	_credits_screen.Show();
}


// 'Quit' confirmed
void BootMode::_OnQuit() {
	// Save settings before quitting
	_SaveSettingsFile();
	SystemManager->ExitGame();
}

// Battle debug confirmed
void BootMode::_OnBattleDebug() {
	ModeManager->Pop();
	GlobalManager->AddCharacter(GLOBAL_CHARACTER_CLAUDIUS);
	BattleMode *BM = new BattleMode();
	BM->AddEnemy(101);
	BM->AddEnemy(102);
	BM->AddEnemy(103);
	BM->AddEnemy(104);
	BM->AddEnemy(105);
	BM->AddEnemy(106);
	ModeManager->Push(BM);
}

// Menu debug confirmed
void BootMode::_OnMenuDebug() {
	ModeManager->Pop();
	GlobalManager->AddCharacter(GLOBAL_CHARACTER_CLAUDIUS);
	hoa_menu::MenuMode *MM = new hoa_menu::MenuMode("The Boot Screen", "img/menus/locations/desert_cave.png");
	ModeManager->Push(MM);
}

// Shop debug confirmed
void BootMode::_OnShopDebug() {
	hoa_shop::ShopMode *SM = new hoa_shop::ShopMode();
	ModeManager->Push(SM);
}


// 'Resolution' confirmed
void BootMode::_OnResolution() {
	_current_menu = &_resolution_menu;
}


// 'Video' confirmed
void BootMode::_OnVideoOptions()
{
	_current_menu = &_video_options_menu;
	_UpdateVideoOptions();
}


// 'Audio' confirmed
void BootMode::_OnAudioOptions()
{
	// Switch the current menu
	_current_menu = &_audio_options_menu;
	_UpdateAudioOptions();
}


// 'Key settings' confirmed
void BootMode::_OnKeySettings() {
	_current_menu = &_key_settings_menu;
	_UpdateKeySettings();
}


// 'Joystick settings' confirmed
void BootMode::_OnJoySettings() {
	_current_menu = &_joy_settings_menu;
	_UpdateJoySettings();
}


// 'Video mode' confirmed
void BootMode::_OnVideoMode() {
	// Toggle fullscreen / windowed
	VideoManager->ToggleFullscreen();
	VideoManager->ApplySettings();

	_UpdateVideoOptions();
}


// Sound volume down
void BootMode::_OnSoundLeft() {
	AudioManager->SetSoundVolume(AudioManager->GetSoundVolume() - 0.1f);
	_UpdateAudioOptions();
	_boot_sounds.at(4).PlaySound(); // Play a sound for user to hear new volume level.
}


// Sound volume up
void BootMode::_OnSoundRight() {
	AudioManager->SetSoundVolume(AudioManager->GetSoundVolume() + 0.1f);
	_UpdateAudioOptions();
	_boot_sounds.at(4).PlaySound(); // Play a sound for user to hear new volume level
}


// Music volume down
void BootMode::_OnMusicLeft() {
	AudioManager->SetMusicVolume(AudioManager->GetMusicVolume() - 0.1f);
	_UpdateAudioOptions();
}


// Music volume up
void BootMode::_OnMusicRight() {
	AudioManager->SetMusicVolume(AudioManager->GetMusicVolume() + 0.1f);
	_UpdateAudioOptions();
}

// Resolution setters
void BootMode::_SetResolution(int32 width, int32 height) {
	VideoManager->SetResolution(width, height);
	VideoManager->ApplySettings();
	_current_menu = &_video_options_menu; // return back to video options
	_UpdateVideoOptions();
}

void BootMode::_OnResolution640x480() {
	if (VideoManager->GetWidth() != 640 && VideoManager->GetHeight() != 480)
		_SetResolution(640, 480);
}

void BootMode::_OnResolution800x600() {
	if (VideoManager->GetWidth() != 800 && VideoManager->GetHeight() != 600)
		_SetResolution(800, 600);
}

void BootMode::_OnResolution1024x768() {
	if (VideoManager->GetWidth() != 1024 && VideoManager->GetHeight() != 768)
		_SetResolution(1024, 768);
}

// Brightness increment. Actually the correct term is "gamma correction" but I think it's easier for the user to think of it just as brightness!
void BootMode::_OnBrightnessLeft() {
	VideoManager->SetGamma(VideoManager->GetGamma() - 0.1f);
	_UpdateVideoOptions();
}

// Brightness decrement
void BootMode::_OnBrightnessRight() {
	VideoManager->SetGamma(VideoManager->GetGamma() + 0.1f);
	_UpdateVideoOptions();
}


// Restores default key settings
void BootMode::_OnRestoreDefaultKeys() {
	InputManager->RestoreDefaultKeys();
	_UpdateKeySettings();
}


// Restores default joystick settings
void BootMode::_OnRestoreDefaultJoyButtons() {
	InputManager->RestoreDefaultJoyButtons();
	_UpdateJoySettings();
}


// Updates the video options screen
void BootMode::_UpdateVideoOptions() {
	// Update resolution text
	std::ostringstream resolution("");
	resolution << "Resolution: " << VideoManager->GetWidth() << " x " << VideoManager->GetHeight();
	_video_options_menu.SetOptionText(0, MakeUnicodeString(resolution.str()));

	// Update text on current video mode
	if (VideoManager->IsFullscreen())
		_video_options_menu.SetOptionText(1, MakeUnicodeString("Window mode: fullscreen"));
	else
		_video_options_menu.SetOptionText(1, MakeUnicodeString("Window mode: windowed"));

	// Update brightness
	_video_options_menu.SetOptionText(2, MakeUnicodeString("Brightness: " + NumberToString(VideoManager->GetGamma() * 50.0f + 0.5f) + " %"));
}


// Updates the audio options screen
void BootMode::_UpdateAudioOptions() {
	std::ostringstream sound_volume("");
	sound_volume << "Sound Volume: " << static_cast<int32>(AudioManager->GetSoundVolume() * 100.0f + 0.5f) << " %";

	std::ostringstream music_volume("");
	music_volume << "Music Volume: " << static_cast<int32>(AudioManager->GetMusicVolume() * 100.0f + 0.5f) << " %";

	_audio_options_menu.SetOptionText(0, MakeUnicodeString(sound_volume.str()));
	_audio_options_menu.SetOptionText(1, MakeUnicodeString(music_volume.str()));
}


// Updates the key settings screen
void BootMode::_UpdateKeySettings() {
	// Update key names
	_key_settings_menu.SetOptionText(0, MakeUnicodeString("Move Up: " + InputManager->GetUpKeyName()));
	_key_settings_menu.SetOptionText(1, MakeUnicodeString("Move Down: " + InputManager->GetDownKeyName()));
	_key_settings_menu.SetOptionText(2, MakeUnicodeString("Move Left: " + InputManager->GetLeftKeyName()));
	_key_settings_menu.SetOptionText(3, MakeUnicodeString("Move Right: " + InputManager->GetRightKeyName()));
	_key_settings_menu.SetOptionText(4, MakeUnicodeString("Confirm: " + InputManager->GetConfirmKeyName()));
	_key_settings_menu.SetOptionText(5, MakeUnicodeString("Cancel: " + InputManager->GetCancelKeyName()));
	_key_settings_menu.SetOptionText(6, MakeUnicodeString("Menu: " + InputManager->GetMenuKeyName()));
	_key_settings_menu.SetOptionText(7, MakeUnicodeString("Swap: " + InputManager->GetSwapKeyName()));
	_key_settings_menu.SetOptionText(8, MakeUnicodeString("Left Select: " + InputManager->GetLeftSelectKeyName()));
	_key_settings_menu.SetOptionText(9, MakeUnicodeString("Right Select: " + InputManager->GetRightSelectKeyName()));
	_key_settings_menu.SetOptionText(10, MakeUnicodeString("Pause: " + InputManager->GetPauseKeyName()));
}


void BootMode::_UpdateJoySettings() {
	_joy_settings_menu.SetOptionText(0, MakeUnicodeString("Confirm: Button " + NumberToString(InputManager->GetConfirmJoy())));
	_joy_settings_menu.SetOptionText(1, MakeUnicodeString("Cancel: Button " + NumberToString(InputManager->GetCancelJoy())));
	_joy_settings_menu.SetOptionText(2, MakeUnicodeString("Menu: Button " + NumberToString(InputManager->GetMenuJoy())));
	_joy_settings_menu.SetOptionText(3, MakeUnicodeString("Swap: Button " + NumberToString(InputManager->GetSwapJoy())));
	_joy_settings_menu.SetOptionText(4, MakeUnicodeString("Left Select : Button " + NumberToString(InputManager->GetLeftSelectJoy())));
	_joy_settings_menu.SetOptionText(5, MakeUnicodeString("Right Select: Button " + NumberToString(InputManager->GetRightSelectJoy())));
	_joy_settings_menu.SetOptionText(6, MakeUnicodeString("Pause: Button " + NumberToString(InputManager->GetPauseJoy())));
}


// Saves all the game settings into a .lua file
void BootMode::_SaveSettingsFile() {

	// No need to save the settings if we haven't edited anything!
	// TODO: Uncomment the next line when the lua tables are saved correctly!
	//if (!_has_modified_settings)
		return;

	// Load the settings file for reading in the original data
	ScriptDescriptor settings_lua;
	if (!settings_lua.OpenFile("dat/config/settings.lua", SCRIPT_READ)) {
		cout << "BOOT ERROR: failed to load the settings file!" << endl;
	}

	// Write the current settings into the .lua file
	// video
	settings_lua.ChangeSetting<int32>("video_settings.screen_resx", VideoManager->GetWidth());
	settings_lua.ChangeSetting<int32>("video_settings.screen_resy", VideoManager->GetHeight());
	settings_lua.ChangeSetting<std::string>("video_settings.full_screen", VideoManager->IsFullscreen() ? "true" : "false");
	settings_lua.ChangeSetting<float>("video_settings.brightness", VideoManager->GetGamma());

	// audio
	settings_lua.ChangeSetting<float>("audio_settings.music_vol", AudioManager->GetMusicVolume());
	settings_lua.ChangeSetting<float>("audio_settings.sound_vol", AudioManager->GetSoundVolume());

	// input
	settings_lua.ChangeSetting<int32>("key_settings.up", InputManager->GetUpKey());
	settings_lua.ChangeSetting<int32>("key_settings.down", InputManager->GetDownKey());
	settings_lua.ChangeSetting<int32>("key_settings.left", InputManager->GetLeftKey());
	settings_lua.ChangeSetting<int32>("key_settings.right", InputManager->GetRightKey());
	settings_lua.ChangeSetting<int32>("key_settings.confirm", InputManager->GetConfirmKey());
	settings_lua.ChangeSetting<int32>("key_settings.cancel", InputManager->GetCancelKey());
	settings_lua.ChangeSetting<int32>("key_settings.menu", InputManager->GetMenuKey());
	settings_lua.ChangeSetting<int32>("key_settings.swap", InputManager->GetSwapKey());
	settings_lua.ChangeSetting<int32>("key_settings.left_select", InputManager->GetLeftSelectKey());
	settings_lua.ChangeSetting<int32>("key_settings.right_select", InputManager->GetRightSelectKey());
	settings_lua.ChangeSetting<int32>("key_settings.pause", InputManager->GetPauseKey());
	settings_lua.ChangeSetting<int32>("joystick_settings.confirm", InputManager->GetConfirmJoy());
	settings_lua.ChangeSetting<int32>("joystick_settings.cancel", InputManager->GetCancelJoy());
	settings_lua.ChangeSetting<int32>("joystick_settings.menu", InputManager->GetMenuJoy());
	settings_lua.ChangeSetting<int32>("joystick_settings.swap", InputManager->GetSwapJoy());
	settings_lua.ChangeSetting<int32>("joystick_settings.left_select", InputManager->GetLeftSelectJoy());
	settings_lua.ChangeSetting<int32>("joystick_settings.right_select", InputManager->GetRightSelectJoy());
	settings_lua.ChangeSetting<int32>("joystick_settings.pause", InputManager->GetPauseJoy());

	// and save it!
	settings_lua.SaveStack("dat/config/new_settings.lua");
}


// This is called once every frame iteration to update the status of the game
void BootMode::Update() {
	uint32 time_elapsed = SystemManager->GetUpdateTime();

	// Screen is in the process of fading out
	if (_fade_out)
	{
		// When the screen is finished fading to black, create a new map mode and fade back in
		if (!VideoManager->IsFading()) {
			ModeManager->Pop();
			MapMode *MM = new MapMode("dat/maps/demo_town.lua");
			ModeManager->Push(MM);
			VideoManager->FadeScreen(Color::clear, 1.0f);
		}
		return;
	}
	else if (_logo_animating) // We're animating the opening logo
	{
		if (InputManager->AnyKeyPress()) // Check if we want to skip the demo
		{
			_EndOpeningAnimation();
			return;
		}
		else
		{
			return; // Otherwise skip rest of the event handling for now
		}
	}

	// Update the menu window
	BootMenu::UpdateWindow(time_elapsed);

	// Update the credits window (because it may be hiding/showing!)
	_credits_screen.UpdateWindow(time_elapsed);

	//CD: Handle key press here, just like any other time
	if (_welcome_screen.IsVisible())
	{
		if (InputManager->AnyKeyPress())
		{
			_boot_sounds.at(0).PlaySound();
			welcome = 0;
			_welcome_screen.Hide();

			// Read in the settings file for writing back the welcome variable
			string line = "";
			fstream settings_file("dat/config/settings.lua");
			if (settings_file.is_open()) {
				while (!settings_file.eof() && line != "welcome = 1;")
					getline(settings_file, line);
				settings_file.seekp(-3, ios::cur);
				settings_file.put('0');
				settings_file.close();
			}
			else
				cout << "BOOT ERROR: failed to load the settings file!" << endl;
		}

		return;
	}

	// A confirm-key was pressed -> handle it (but ONLY if the credits screen isn't visible)
	if (InputManager->ConfirmPress() && !_credits_screen.IsVisible())
	{
		// Play 'confirm sound' if the selection isn't grayed out and it has a confirm handler
		if (_current_menu->IsSelectionEnabled())
			_boot_sounds.at(0).PlaySound();
		else
			_boot_sounds.at(3).PlaySound(); // Otherwise play a silly 'b�p'

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
	else if (InputManager->LeftPress() && !_credits_screen.IsVisible())
	{
		_current_menu->LeftPressed();
	}
	else if(InputManager->RightPress() && !_credits_screen.IsVisible())
	{
		_current_menu->RightPressed();
	}
	else if(InputManager->UpPress() && !_credits_screen.IsVisible())
	{
		_current_menu->UpPressed();
	}
	else if(InputManager->DownPress() && !_credits_screen.IsVisible())
	{
		_current_menu->DownPressed();
	}
	else if(InputManager->CancelPress())
	{
		// Close the credits-screen if it was visible
		if (_credits_screen.IsVisible())
		{
			_credits_screen.Hide();
			_boot_sounds.at(1).PlaySound(); // Play cancel sound here as well
		}

		// Otherwise the cancel was given for the main menu
		_current_menu->CancelPressed();

		// Go up in the menu hierarchy if possible
		if (_current_menu->GetParent() != 0)
		{
			// Play cancel sound
			_boot_sounds.at(1).PlaySound();

			// Go up a level in the menu hierarchy
			_current_menu = _current_menu->GetParent();

			// Update window status again
			if (_current_menu->IsWindowed())
				BootMenu::ShowWindow(true);
			else
				BootMenu::ShowWindow(false);
		}
	}

	// Update menu events
	_current_menu->GetEvent();
}


// Draws our next frame to the video back buffer
void BootMode::Draw() {

	// If we're animating logo at the moment, handle all drawing in there and simply return
	if (_logo_animating)
	{
		_AnimateLogo();
		return;
	}

	_DrawBackgroundItems();

	// Decide whether to draw the credits window, welcome window or the main menu
	if (_credits_screen.IsVisible())
		_credits_screen.Draw();
	else if (_welcome_screen.IsVisible())
		_welcome_screen.Draw();
	else
		_current_menu->Draw();

	if (!_latest_version)
	{
		VideoManager->SetTextColor(Color::green);
		VideoManager->Move(482.0f, 553.0f);
		VideoManager->DrawText("New version available from allacrost.org: " + _latest_version_number);
	}

	VideoManager->Move(65.0f, 10.0f);
	VideoManager->SetFont("default");
	VideoManager->SetTextColor(Color::gray);
	VideoManager->DrawText("Tech Demo");
	VideoManager->MoveRelative(730.0f, 0.0f);
	VideoManager->DrawText("Copyright (C) 2004 - 2007 The Allacrost Project");
}


} // namespace hoa_boot
