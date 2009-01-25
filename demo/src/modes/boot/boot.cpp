///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    boot.cpp
*** \author  Viljami Korhonen, mindflayer@allacrost.org
*** \brief   Source file for boot mode interface.
*** ***************************************************************************/

#include <iostream>
#include <sstream>

#include "audio.h"
#include "script.h"
#include "input.h"
#include "system.h"

#include "global.h"

#include "boot.h"
#include "map.h"
#include "save_mode.h"
#include "battle.h" // tmp
#include "menu.h" // even more tmp
#include "shop.h" // tmp

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
bool BootMode::_initial_entry = true;

// ****************************************************************************
// *************************** GENERAL FUNCTIONS ******************************
// ****************************************************************************

// The constructor initializes variables and sets up the path names of the boot images
BootMode::BootMode() :
	_fade_out(false),
	_key_setting_function(NULL),
	_joy_setting_function(NULL),
	_joy_axis_setting_function(NULL),
	_message_window(string(""), 210.0f, 35.0f),
	_latest_version(true),
	_has_modified_settings(false)
{
	IF_PRINT_DEBUG(BOOT_DEBUG) << "BOOT: BootMode constructor invoked." << endl;
	mode_type = MODE_MANAGER_BOOT_MODE;

	ReadScriptDescriptor read_data;
	if (!read_data.OpenFile("dat/config/boot.lua")) {
		PRINT_ERROR << "BOOT ERROR: failed to load data file" << endl;
	}

	// Load all bitmaps using this StillImage
	StillImage im;
	bool success = true;

	success &= im.Load(read_data.ReadString("background_image"), read_data.ReadFloat("background_image_width"), read_data.ReadFloat("background_image_height"));
	_boot_images.push_back(im);

	success &= im.Load(read_data.ReadString("logo_background"), read_data.ReadFloat("logo_background_width"), read_data.ReadFloat("logo_background_height"));
	_boot_images.push_back(im);

	success &= im.Load(read_data.ReadString("logo_sword"), read_data.ReadFloat("logo_sword_width"), read_data.ReadFloat("logo_sword_height"));
	_boot_images.push_back(im);

	success &= im.Load(read_data.ReadString("logo_text"), read_data.ReadFloat("logo_text_width"), read_data.ReadFloat("logo_text_height"));
	_boot_images.push_back(im);

	if (success == false) {
		PRINT_ERROR << "BOOT ERROR: failed to load a boot mode image" << endl;
	}

	// Load the audio stuff
	// Make a call to the config code that loads in two vectors of strings
	vector<string> new_music_files;
	read_data.ReadStringVector("music_files", new_music_files);

	vector<string> new_sound_files;
	read_data.ReadStringVector("sound_files", new_sound_files);

	if (read_data.IsErrorDetected()) {
		PRINT_ERROR << "some error occured during reading of boot data file" << endl;
		PRINT_ERROR << read_data.GetErrorMessages() << endl;
	}

	read_data.CloseFile();

	// Load all music and sound files used in boot mode
	_boot_music.resize(new_music_files.size(), MusicDescriptor());
	for (uint32 i = 0; i < new_music_files.size(); i++) {
		if (_boot_music[i].LoadAudio(new_music_files[i]) == false) {
			PRINT_ERROR << "failed to load music file: " << new_music_files[i] << endl;
			SystemManager->ExitGame();
			return;
		}
	}

	_boot_sounds.resize(new_sound_files.size(), SoundDescriptor());
	for (uint32 i = 0; i < new_sound_files.size(); i++) {
		if (_boot_sounds[i].LoadAudio(new_sound_files[i]) == false) {
			PRINT_ERROR << "failed to load sound file: " << new_sound_files[i] << endl;
			SystemManager->ExitGame();
			return;
		}
	}

	// Check the version
	_latest_version = true; //IsLatestVersion();
	if (!_latest_version)
		_latest_version_number = GetLatestVersion();
	else
		_latest_version_number = "";

	_menu_window.Create(250.0f, 400.0f);
	_menu_window.SetPosition(387.0f, 560.0f);
	_menu_window.SetDisplayMode(VIDEO_MENU_INSTANT);
	_menu_window.Hide();

/*
	if (!_is_windowed) // without a window
	{
		_active_menu.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
// 		_active_menu.SetCellSize(150.0f, 70.0f);
		_active_menu.SetPosition(552.0f, 50.0f);
		_active_menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
		_active_menu.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
		_active_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
		_active_menu.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
		_active_menu.SetCursorOffset(-50.0f, 28.0f);
// 		_active_menu.SetSize(_active_menu.GetNumberOptions(), 1);
	}
	else // windowed
	{
		_active_menu.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
// 		_active_menu.SetCellSize(210.0f, 50.0f);
		_active_menu.SetPosition(150.0f, 200.0f);
		_active_menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
		_active_menu.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
		_active_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
		_active_menu.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
		_active_menu.SetCursorOffset(-50.0f, 28.0f);
// 		_active_menu.SetSize(1, _active_menu.GetNumberOptions());
		_active_menu.SetOwner(_menu_window);
	}
*/

	// Construct our menu hierarchy here
	_SetupMainMenu();
	_SetupOptionsMenu();
	_SetupVideoOptionsMenu();
	_SetupAudioOptionsMenu();
	_SetupLanguageOptionsMenu();
	_SetupKeySetttingsMenu();
	_SetupJoySetttingsMenu();
	_SetupResolutionMenu();

	// make sure message window is not visible
	_message_window.Hide();
}


// The destructor frees all used music, sounds, and images.
BootMode::~BootMode() {
	_menu_window.Destroy();
	_SaveSettingsFile();

	if (BOOT_DEBUG) cout << "BOOT: BootMode destructor invoked." << endl;

	for (uint32 i = 0; i < _boot_music.size(); i++)
		_boot_music[i].FreeAudio();

	for (uint32 i = 0; i < _boot_sounds.size(); i++)
		_boot_sounds[i].FreeAudio();

	_key_setting_function = 0;
	_joy_setting_function = 0;
}


// Resets appropriate class members.
void BootMode::Reset() {
	// Set the coordinate system that BootMode uses
	VideoManager->SetCoordSys(0.0f, 1023.0f, 0.0f, 767.0f);
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);

	GlobalManager->ClearAllData(); 	// Resets the game universe to a NULL state
	BootMenu::active_boot_mode = this;

	// Decide which music track to play
	if (_initial_entry)
		_boot_music.at(1).Play(); // Opening Effect
	else
		_boot_music.at(0).Play(); // Main theme
}





// This is called once every frame iteration to update the status of the game
void BootMode::Update() {
	_menu_window.Update(SystemManager->GetUpdateTime());

	if (InputManager->QuitPress() == true) {
		SystemManager->ExitGame();
		return;
	}

	// Screen is in the process of fading out
	if (_fade_out)
	{
		// When the screen is finished fading to black, create a new map mode and fade back in
		if (!VideoManager->IsFading()) {
			ModeManager->Pop();
			try {
				MapMode *MM = new MapMode(MakeStandardString(GlobalManager->GetLocationName()));
				ModeManager->Push(MM);
			} catch (luabind::error e) {
				cerr << "Map::_Load -- Error loading map dat/maps/demo_town.lua, returning to BootMode." << endl;
				cerr << "Exception message:" << endl;
				ScriptManager->HandleLuaError(e);
			}
			VideoManager->FadeScreen(Color::clear, 1000);
		}
		return;
	}
	else if (_initial_entry) // We're animating the opening logo
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

	// Update the credits window (because it may be hiding/showing!)
	_credits_screen.Update(SystemManager->GetUpdateTime());

	//CD: Handle key press here, just like any other time
	if (_welcome_screen.IsVisible())
	{
		if (InputManager->AnyKeyPress())
		{
			_boot_sounds.at(0).Play();
			_welcome_screen.Hide();

			// save the settings (automatically changes the welcome variable to 0
			_has_modified_settings = true;
			_SaveSettingsFile();
		}

		return;
	}

	// Check for waiting keypresses or joystick button presses
	SDL_Event ev = InputManager->GetMostRecentEvent();
	if (_joy_setting_function != NULL)
	{
		if (InputManager->AnyKeyPress() && ev.type == SDL_JOYBUTTONDOWN)
		{
			(this->*_joy_setting_function)(InputManager->GetMostRecentEvent().jbutton.button);
			_joy_setting_function = NULL;
			_has_modified_settings = true;
			_UpdateJoySettings();
			_message_window.Hide();
		}
		if (InputManager->CancelPress())
		{
			_joy_setting_function = NULL;
			_message_window.Hide();
		}
		return;
	}

	if (_joy_axis_setting_function != NULL)
	{
		int8 x = InputManager->GetLastAxisMoved();
		if (x != -1)
		{
			(this->*_joy_axis_setting_function)(x);
			_joy_axis_setting_function = NULL;
			_has_modified_settings = true;
			_UpdateJoySettings();
			_message_window.Hide();
		}
		if (InputManager->CancelPress())
		{
			_joy_axis_setting_function = NULL;
			_message_window.Hide();
		}
		return;
	}

	if (_key_setting_function != NULL)
	{
		if (InputManager->AnyKeyPress() && ev.type == SDL_KEYDOWN)
		{
			(this->*_key_setting_function)(InputManager->GetMostRecentEvent().key.keysym.sym);
			_key_setting_function = NULL;
			_has_modified_settings = true;
			_UpdateKeySettings();
			_message_window.Hide();
		}
		if (InputManager->CancelPress())
		{
			_key_setting_function = NULL;
			_message_window.Hide();
		}
		return;
	}

	_active_menu->Update();

	// A confirm-key was pressed -> handle it (but ONLY if the credits screen isn't visible)
	if (InputManager->ConfirmPress() && !_credits_screen.IsVisible())
	{
		// Play 'confirm sound' if the selection isn't grayed out and it has a confirm handler
		if (_active_menu->IsEnabled(_active_menu->GetSelection()))
			_boot_sounds.at(0).Play();
		else
			_boot_sounds.at(3).Play(); // Otherwise play a different sound

		_active_menu->InputConfirm();

	}
	else if (InputManager->LeftPress() && !_credits_screen.IsVisible())
	{
		_active_menu->InputLeft();
	}
	else if(InputManager->RightPress() && !_credits_screen.IsVisible())
	{
		_active_menu->InputRight();
	}
	else if(InputManager->UpPress() && !_credits_screen.IsVisible())
	{
		_active_menu->InputUp();
	}
	else if(InputManager->DownPress() && !_credits_screen.IsVisible())
	{
		_active_menu->InputDown();
	}
	else if(InputManager->CancelPress())
	{
		// Close the credits-screen if it was visible
		if (_credits_screen.IsVisible())
		{
			_credits_screen.Hide();
			_boot_sounds.at(1).Play(); // Play cancel sound here as well
		}
		else if (_active_menu == &_main_menu) {
			
		}
		else if (_active_menu == &_options_menu) {
			_active_menu = &_main_menu;
		}
		else if (_active_menu == &_video_options_menu) {
			_active_menu = &_options_menu;
		}
		else if (_active_menu == &_audio_options_menu) {
			_active_menu = &_options_menu;
		}
		else if (_active_menu == &_language_options_menu) {
			_active_menu = &_options_menu;
		}
		else if (_active_menu == &_key_settings_menu) {
			_active_menu = &_options_menu;
		}
		else if (_active_menu == &_joy_settings_menu) {
			_active_menu = &_options_menu;
		}
		else if (_active_menu == &_resolution_menu) {
			_active_menu = &_video_options_menu;
		}

		// check to see if settings need to be saved (if we're exiting from the key or joystick
		// settings menu
		_SaveSettingsFile();

		// Play cancel sound
		_boot_sounds.at(1).Play();
	}

	// Update menu events
// 	_active_menu->GetEvent();
}


// Draws our next frame to the video back buffer
void BootMode::Draw() {
	// If we're animating logo at the moment, handle all drawing in there and simply return
	if (_initial_entry)
	{
		_AnimateLogo();
		return;
	}

	_DrawBackgroundItems();

	_menu_window.Draw();


	// Decide whether to draw the credits window, welcome window or the main menu
	if (_credits_screen.IsVisible())
		_credits_screen.Draw();
	else if (_welcome_screen.IsVisible())
		_welcome_screen.Draw();
	else if (_active_menu != NULL)
		_active_menu->Draw();

	if (!_latest_version)
	{
		VideoManager->Text()->SetDefaultTextColor(Color::green);
		VideoManager->Move(482.0f, 553.0f);
		VideoManager->Text()->Draw("New version available from allacrost.org: " + _latest_version_number);
	}

	VideoManager->Move(65.0f, 10.0f);
	VideoManager->Text()->SetDefaultFont("default");
	VideoManager->Text()->SetDefaultTextColor(Color::gray);
	VideoManager->Text()->Draw("Demo 1.0.0");
	VideoManager->MoveRelative(730.0f, 0.0f);
	VideoManager->Text()->Draw("Copyright (C) 2004 - 2009 The Allacrost Project");

// 	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
	VideoManager->Move(0, 0);
	_message_window.Draw();
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
	else if (total_time >= SEQUENCE_TWO && total_time < SEQUENCE_THREE) {
		float alpha = (total_time - SEQUENCE_TWO) / (SEQUENCE_THREE - SEQUENCE_TWO);

		VideoManager->Move(512.0f, 385.0f); // logo bg
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		_boot_images[1].Draw(Color(alpha, alpha, alpha, 1.0f));
		VideoManager->Move(sword_x, sword_y); // sword
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->Rotate(-90.0f);
		_boot_images[2].Draw(Color(alpha, alpha, alpha, 1.0f));
		VideoManager->Move(512.0f, 385.0f); // text
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		_boot_images[3].Draw(Color(alpha, alpha, alpha, 1.0f));
	}
	// Sequence three: Sword unsheathe & slide
	else if (total_time >= SEQUENCE_THREE && total_time < SEQUENCE_FOUR) {
		float dt = (total_time - SEQUENCE_THREE) * 0.001f;
		sword_x = 670.0f + (dt * dt) * 660.0f; // s = s0 + 0.5 * a * t^2
		VideoManager->Move(512.0f, 385.0f); // logo bg
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		_boot_images[1].Draw();
		VideoManager->Move(sword_x, sword_y); // sword
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->Rotate(-90.0f);
		_boot_images[2].Draw();
		VideoManager->Move(512.0f, 385.0f); // text
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		_boot_images[3].Draw();
	}
	// Sequence four: Spin around the sword
	else if (total_time >= SEQUENCE_FOUR && total_time < SEQUENCE_FIVE) {
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
		_boot_images[1].Draw();
		VideoManager->Move(512.0f, 385.0f); // text
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		_boot_images[3].Draw();
		VideoManager->Move(sword_x, sword_y); // sword
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		VideoManager->Rotate(rotation);
		_boot_images[2].Draw();
	}
	// Sequence five: Sword comes back
	else if (total_time >= SEQUENCE_FIVE && total_time < SEQUENCE_SIX) {
		// Delta goes from 0.0f to 1.0f
		float delta_root = (total_time - SEQUENCE_FIVE) / (SEQUENCE_SIX - SEQUENCE_FIVE);
		float delta = delta_root * delta_root * delta_root * delta_root;
		float newX = (1.0f - delta) * sword_x + 762.0f * delta;
		float newY = (1.0f - delta) * sword_y + 310.0f * delta;

		VideoManager->Move(512.0f, 385.0f); // logo bg
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		_boot_images[1].Draw();
		VideoManager->Move(512.0f, 385.0f); // text
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		_boot_images[3].Draw();
		VideoManager->Move(newX, newY); // sword
		VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
		_boot_images[2].Draw();
	}
	// Sequence six: flash of light
	else if (total_time >= SEQUENCE_SIX && total_time < SEQUENCE_SEVEN) {
		// Delta goes from 1.0f to 0.0f
		float delta = (total_time - SEQUENCE_SIX) / (SEQUENCE_SEVEN - SEQUENCE_SIX);
		delta = 1.0f - delta * delta;
		VideoManager->EnableFog(Color::white, delta);
		_DrawBackgroundItems();
	}
	else if (total_time >= SEQUENCE_SEVEN) {
		_EndOpeningAnimation();
		_DrawBackgroundItems();
	}
}


// Draws background image, logo and sword at their stationary locations
void BootMode::_DrawBackgroundItems() {
	VideoManager->Move(512.0f, 384.0f);
	VideoManager->SetDrawFlags(VIDEO_NO_BLEND, 0);
	_boot_images[0].Draw(); // Draw background

	VideoManager->Move(512.0f, 648.0f);
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	_boot_images[1].Draw(); // Draw the logo background

	VideoManager->Move(762.0f, 578.0f);
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	_boot_images[2].Draw(); // Draw the sword

	VideoManager->Move(512.0f, 648.0f);
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	_boot_images[3].Draw(); // Draw the logo text
}



void BootMode::_EndOpeningAnimation() {
	VideoManager->DisableFog(); // Turn off the fog

	// Stop playing SFX and start playing the main theme
//	_boot_music.at(1).SetFadeOutTime(1000);
	_boot_music.at(1).Stop();
//	_boot_music.at(0).SetFadeInTime(5000);
	_boot_music.at(0).Play();

//	Effects::FadeOut(_boot_music.at(1), 10.0f);
//	Effects::FadeIn(_boot_music.at(0), 50.0f);

	// Load the settings file for reading in the welcome variable
	ReadScriptDescriptor settings_lua;
	string file = GetSettingsFilename();
	if (!settings_lua.OpenFile(file)) {
		PRINT_WARNING << "failed to load the boot settings file" << endl;
	}

	settings_lua.OpenTable("settings");
	if (settings_lua.ReadInt("welcome") == 1) {
		_welcome_screen.Show();
	}
	settings_lua.CloseTable();
	settings_lua.CloseFile();
	_initial_entry = false;

	_active_menu = &_main_menu;
}


// Waits indefinitely for a key press
SDLKey BootMode::_WaitKeyPress() {
	SDL_Event event;
	while (SDL_WaitEvent(&event)) {
		if (event.type == SDL_KEYDOWN)
			break;
	}

	return event.key.keysym.sym;
}


// Waits indefinitely for a joystick press
uint8 BootMode::_WaitJoyPress() {
	SDL_Event event;
	while (SDL_WaitEvent(&event)) {
		if (event.type == SDL_JOYBUTTONDOWN)
			break;
	}

	return event.jbutton.button;
}


// Redefines a key to be mapped to another command. Waits for keypress using _WaitKeyPress()
void BootMode::_RedefineUpKey() {
	_key_setting_function = &BootMode::_SetUpKey;
	_ShowMessageWindow(false);
}



void BootMode::_RedefineDownKey() {
	_key_setting_function = &BootMode::_SetDownKey;
	_ShowMessageWindow(false);
}



void BootMode::_RedefineLeftKey() {
	_key_setting_function = &BootMode::_SetLeftKey;
	_ShowMessageWindow(false);
}



void BootMode::_RedefineRightKey() {
	_key_setting_function = &BootMode::_SetRightKey;
	_ShowMessageWindow(false);
}



void BootMode::_RedefineConfirmKey() {
	_key_setting_function = &BootMode::_SetConfirmKey;
	_ShowMessageWindow(false);
}



void BootMode::_RedefineCancelKey() {
	_key_setting_function = &BootMode::_SetCancelKey;
	_ShowMessageWindow(false);
}



void BootMode::_RedefineMenuKey() {
	_key_setting_function = &BootMode::_SetMenuKey;
	_ShowMessageWindow(false);
}



void BootMode::_RedefineSwapKey() {
	_key_setting_function = &BootMode::_SetSwapKey;
	_ShowMessageWindow(false);
}



void BootMode::_RedefineLeftSelectKey() {
	_key_setting_function = &BootMode::_SetLeftSelectKey;
	_ShowMessageWindow(false);
}



void BootMode::_RedefineRightSelectKey() {
	_key_setting_function = &BootMode::_SetRightSelectKey;
	_ShowMessageWindow(false);
}



void BootMode::_RedefinePauseKey() {
	_key_setting_function = &BootMode::_SetPauseKey;
	_ShowMessageWindow(false);
}



void BootMode::_SetUpKey(const SDLKey &key) {
	InputManager->SetUpKey(key);
}



void BootMode::_SetDownKey(const SDLKey &key) {
	InputManager->SetDownKey(key);
}


void BootMode::_SetLeftKey(const SDLKey &key) {
	InputManager->SetLeftKey(key);
}



void BootMode::_SetRightKey(const SDLKey &key) {
	InputManager->SetRightKey(key);
}



void BootMode::_SetConfirmKey(const SDLKey &key) {
	InputManager->SetConfirmKey(key);
}



void BootMode::_SetCancelKey(const SDLKey &key) {
	InputManager->SetCancelKey(key);
}



void BootMode::_SetMenuKey(const SDLKey &key) {
	InputManager->SetMenuKey(key);
}



void BootMode::_SetSwapKey(const SDLKey &key) {
	InputManager->SetSwapKey(key);
}



void BootMode::_SetLeftSelectKey(const SDLKey &key) {
	InputManager->SetLeftSelectKey(key);
}



void BootMode::_SetRightSelectKey(const SDLKey &key) {
	InputManager->SetRightSelectKey(key);
}



void BootMode::_SetPauseKey(const SDLKey &key) {
	InputManager->SetPauseKey(key);
}


// Redefine joystick axes settings
void BootMode::_RedefineXAxisJoy()
{
	_joy_axis_setting_function = &BootMode::_SetXAxisJoy;
	_ShowMessageWindow(WAIT_JOY_AXIS);
	InputManager->ResetLastAxisMoved();
}



void BootMode::_RedefineYAxisJoy()
{
	_joy_axis_setting_function = &BootMode::_SetYAxisJoy;
	_ShowMessageWindow(WAIT_JOY_AXIS);
	InputManager->ResetLastAxisMoved();
}


// Redefines a joystick button to be mapped to another command. Waits for press using _WaitJoyPress()
void BootMode::_RedefineConfirmJoy()
{
	_joy_setting_function = &BootMode::_SetConfirmJoy;
	_ShowMessageWindow(true);
}



void BootMode::_RedefineCancelJoy()
{
	_joy_setting_function = &BootMode::_SetCancelJoy;
	_ShowMessageWindow(true);
}



void BootMode::_RedefineMenuJoy()
{
	_joy_setting_function = &BootMode::_SetMenuJoy;
	_ShowMessageWindow(true);
}



void BootMode::_RedefineSwapJoy()
{
	_joy_setting_function = &BootMode::_SetSwapJoy;
	_ShowMessageWindow(true);
}



void BootMode::_RedefineLeftSelectJoy()
{
	_joy_setting_function = &BootMode::_SetLeftSelectJoy;
	_ShowMessageWindow(true);
}



void BootMode::_RedefineRightSelectJoy()
{
	_joy_setting_function = &BootMode::_SetRightSelectJoy;
	_ShowMessageWindow(true);
}



void BootMode::_RedefinePauseJoy()
{
	_joy_setting_function = &BootMode::_SetPauseJoy;
	_ShowMessageWindow(true);
}

void BootMode::_SetXAxisJoy(int8 axis)
{ InputManager->SetXAxisJoy(axis); }
void BootMode::_SetYAxisJoy(int8 axis)
{ InputManager->SetYAxisJoy(axis); }
void BootMode::_SetConfirmJoy(uint8 button)
{ InputManager->SetConfirmJoy(button); }
void BootMode::_SetCancelJoy(uint8 button)
{ InputManager->SetCancelJoy(button); }
void BootMode::_SetMenuJoy(uint8 button)
{ InputManager->SetMenuJoy(button); }
void BootMode::_SetSwapJoy(uint8 button)
{ InputManager->SetSwapJoy(button); }
void BootMode::_SetLeftSelectJoy(uint8 button)
{ InputManager->SetLeftSelectJoy(button); }
void BootMode::_SetRightSelectJoy(uint8 button)
{ InputManager->SetRightSelectJoy(button); }
void BootMode::_SetPauseJoy(uint8 button)
{ InputManager->SetPauseJoy(button); }

void BootMode::_ShowMessageWindow(WAIT_FOR wait)
{
	string message = "";
	if (wait == WAIT_JOY_BUTTON)
		message = "Please press a new joystick button.";
	else if (wait == WAIT_KEY)
		message = "Please press a new key.";
	else if (wait == WAIT_JOY_AXIS)
		message = "Please move an axis.";
	else {
		PRINT_WARNING << "Undefined wait value." << std::endl;
		return;
	}
	_message_window.SetText(message);
	_message_window.Show();
}

void BootMode::_ShowMessageWindow(bool joystick)
{
	if (joystick)
		_ShowMessageWindow(WAIT_JOY_BUTTON);
	else
		_ShowMessageWindow(WAIT_KEY);
}


// Inits the main menu
void BootMode::_SetupMainMenu() {
	_main_menu.SetPosition(512.0f, 50.0f);
	_main_menu.SetDimensions(600.0f, 50.0f, 8, 1, 8, 1);
	_main_menu.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	_main_menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_main_menu.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_main_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	_main_menu.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_main_menu.SetCursorOffset(-50.0f, 28.0f);

	// Add all the needed menu options to the main menu
	_main_menu.AddOption(MakeUnicodeString("New Game"), &BootMode::_OnNewGame);
	_main_menu.AddOption(MakeUnicodeString("Load Game"), &BootMode::_OnLoadGame);
	_main_menu.AddOption(MakeUnicodeString("Options"), &BootMode::_OnOptions);
	_main_menu.AddOption(MakeUnicodeString("Credits"), &BootMode::_OnCredits);
	
	// TEMP: these options are for debugging purposes only and should be removed for releases
	_main_menu.AddOption(MakeUnicodeString("Battle"), &BootMode::_OnBattleDebug);
	_main_menu.AddOption(MakeUnicodeString("Menu"), &BootMode::_OnMenuDebug);
	_main_menu.AddOption(MakeUnicodeString("Shop"), &BootMode::_OnShopDebug);

	_main_menu.AddOption(MakeUnicodeString("Quit"), &BootMode::_OnQuit);

	string path = GetUserDataPath(true) + "saved_game_1.lua";
	if (DoesFileExist(path) == false) {
		_main_menu.EnableOption(1, false);
		_main_menu.SetSelection(0);
	}
	else {
		_main_menu.SetSelection(1);
	}
}


// Inits the options menu
void BootMode::_SetupOptionsMenu() {
	_options_menu.SetPosition(512.0f, 384.0f);
	_options_menu.SetDimensions(300.0f, 500.0f, 1, 5, 1, 5);
	_options_menu.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	_options_menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_options_menu.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_options_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	_options_menu.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_options_menu.SetCursorOffset(-50.0f, 28.0f);

	_options_menu.AddOption(MakeUnicodeString("Video"), &BootMode::_OnVideoOptions);
	_options_menu.AddOption(MakeUnicodeString("Audio"), &BootMode::_OnAudioOptions);
	_options_menu.AddOption(MakeUnicodeString("Language"), &BootMode::_OnLanguageOptions);
	_options_menu.AddOption(MakeUnicodeString("Key Settings"), &BootMode::_OnKeySettings);
	_options_menu.AddOption(MakeUnicodeString("Joystick Settings"), &BootMode::_OnJoySettings);

	_options_menu.SetSelection(0);
}


// Inits the video-options menu
void BootMode::_SetupVideoOptionsMenu()
{
	_video_options_menu.SetPosition(512.0f, 384.0f);
	_video_options_menu.SetDimensions(300.0f, 400.0f, 1, 4, 1, 4);
	_video_options_menu.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	_video_options_menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_video_options_menu.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_video_options_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	_video_options_menu.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_video_options_menu.SetCursorOffset(-50.0f, 28.0f);

	_video_options_menu.AddOption(MakeUnicodeString("Resolution:"), &BootMode::_OnResolution);
	_video_options_menu.AddOption(MakeUnicodeString("Window mode:"), &BootMode::_OnVideoMode, &BootMode::_OnVideoMode, &BootMode::_OnVideoMode); // Left & right will change window mode as well as plain 'confirm' !
	_video_options_menu.AddOption(MakeUnicodeString("Brightness:"), NULL, &BootMode::_OnBrightnessLeft, &BootMode::_OnBrightnessRight);
	_video_options_menu.AddOption(MakeUnicodeString("Image quality:"));

	_video_options_menu.EnableOption(3, false); // disable image quality

	_video_options_menu.SetSelection(0);
}


// Inits the audio-options menu
void BootMode::_SetupAudioOptionsMenu()
{
	_audio_options_menu.SetPosition(512.0f, 384.0f);
	_audio_options_menu.SetDimensions(300.0f, 200.0f, 1, 2, 1, 2);
	_audio_options_menu.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	_audio_options_menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_audio_options_menu.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_audio_options_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	_audio_options_menu.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_audio_options_menu.SetCursorOffset(-50.0f, 28.0f);

	_audio_options_menu.AddOption(MakeUnicodeString("Sound Volume: "), 0, &BootMode::_OnSoundLeft, &BootMode::_OnSoundRight);
	_audio_options_menu.AddOption(MakeUnicodeString("Music Volume: "), 0, &BootMode::_OnMusicLeft, &BootMode::_OnMusicRight);

	_audio_options_menu.SetSelection(0);
}


// Inits the language-select menu
void BootMode::_SetupLanguageOptionsMenu()
{
	_language_options_menu.SetPosition(512.0f, 384.0f);
	_language_options_menu.SetDimensions(300.0f, 200.0f, 1, 1, 1, 1);
	_language_options_menu.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	_language_options_menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_language_options_menu.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_language_options_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	_language_options_menu.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_language_options_menu.SetCursorOffset(-50.0f, 28.0f);

	_language_options_menu.AddOption(MakeUnicodeString("French"), &BootMode::_OnLanguageSelect);

	_language_options_menu.SetSelection(0);
}


// Inits the key-settings menu
void BootMode::_SetupKeySetttingsMenu() {
	_key_settings_menu.SetPosition(512.0f, 384.0f);
	_key_settings_menu.SetDimensions(300.0f, 500.0f, 1, 11, 1, 11);
	_key_settings_menu.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	_key_settings_menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_key_settings_menu.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_key_settings_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	_key_settings_menu.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_key_settings_menu.SetCursorOffset(-50.0f, 28.0f);

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
}


void BootMode::_SetupJoySetttingsMenu() {
	_joy_settings_menu.SetPosition(512.0f, 384.0f);
	_joy_settings_menu.SetDimensions(300.0f, 500.0f, 1, 10, 1, 10);
	_joy_settings_menu.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	_joy_settings_menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_joy_settings_menu.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_joy_settings_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	_joy_settings_menu.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_joy_settings_menu.SetCursorOffset(-50.0f, 28.0f);

	ustring dummy;
	_joy_settings_menu.AddOption(dummy, &BootMode::_RedefineXAxisJoy);
	_joy_settings_menu.AddOption(dummy, &BootMode::_RedefineYAxisJoy);
//	_joy_settings_menu.AddOption(dummy, &BootMode::_RedefineThresholdJoy);

	_joy_settings_menu.AddOption(dummy, &BootMode::_RedefineConfirmJoy);
	_joy_settings_menu.AddOption(dummy, &BootMode::_RedefineCancelJoy);
	_joy_settings_menu.AddOption(dummy, &BootMode::_RedefineMenuJoy);
	_joy_settings_menu.AddOption(dummy, &BootMode::_RedefineSwapJoy);
	_joy_settings_menu.AddOption(dummy, &BootMode::_RedefineLeftSelectJoy);
	_joy_settings_menu.AddOption(dummy, &BootMode::_RedefineRightSelectJoy);
	_joy_settings_menu.AddOption(dummy, &BootMode::_RedefinePauseJoy);
//	_joy_settings_menu.AddOption(dummy, &BootMode::_RedefineQuitJoy);

	_joy_settings_menu.AddOption(MakeUnicodeString("Restore defaults"), &BootMode::_OnRestoreDefaultJoyButtons);
}


void BootMode::_SetupResolutionMenu() {
	_resolution_menu.SetPosition(512.0f, 384.0f);
	_resolution_menu.SetDimensions(300.0f, 200.0f, 1, 3, 1, 3);
	_resolution_menu.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	_resolution_menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_resolution_menu.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_resolution_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	_resolution_menu.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_resolution_menu.SetCursorOffset(-50.0f, 28.0f);

	_resolution_menu.AddOption(MakeUnicodeString("640 x 480"), &BootMode::_OnResolution640x480);
	_resolution_menu.AddOption(MakeUnicodeString("800 x 600"), &BootMode::_OnResolution800x600);
	_resolution_menu.AddOption(MakeUnicodeString("1024 x 768"), &BootMode::_OnResolution1024x768);
	
	if (VideoManager->GetScreenWidth() == 640)
		_resolution_menu.SetSelection(0);
	else if (VideoManager->GetScreenWidth() == 800)
		_resolution_menu.SetSelection(1);
	else if (VideoManager->GetScreenWidth() == 1024)
		_resolution_menu.SetSelection(2);
}


// Main menu handlers
// 'New Game' confirmed
void BootMode::_OnNewGame() {
	GlobalManager->NewGame();

	_fade_out = true;
	VideoManager->FadeScreen(Color::black, 1000); // Fade to black over the course of one second
//	_boot_music.at(0).SetFadeOutTime(500); // Fade out the music
	_boot_music.at(0).Stop();
}


void BootMode::_OnLoadGame() {
	_boot_music.at(0).Stop();
	// TODO: SaveMode music should take over when this is used for loading games...
	hoa_save::SaveMode *SVM = new hoa_save::SaveMode(false);
	ModeManager->Push(SVM);
}


// 'Options' confirmed
void BootMode::_OnOptions() {
	_active_menu = &_options_menu;
}


// 'Credits' confirmed
void BootMode::_OnCredits() {
	_credits_screen.Show();
}

// 'Quit' confirmed
void BootMode::_OnQuit()
{ SystemManager->ExitGame(); }

// Battle debug confirmed
void BootMode::_OnBattleDebug() {
//	ModeManager->Pop();
	GlobalManager->AddCharacter(1);
	BattleMode *BM = new BattleMode();
	BM->AddEnemy(1);
	//BM->AddEnemy(1);
	ModeManager->Push(BM);
}

// Menu debug confirmed
void BootMode::_OnMenuDebug() {
//	ModeManager->Pop();
	GlobalManager->AddCharacter(1);
	hoa_menu::MenuMode *MM = new hoa_menu::MenuMode(MakeUnicodeString("The Boot Screen"), "img/menus/locations/desert_cave.png");
	ModeManager->Push(MM);
}

// Shop debug confirmed
void BootMode::_OnShopDebug() {
	GlobalManager->AddCharacter(1);
	GlobalManager->AddDrunes(500);
	hoa_shop::ShopMode *SM = new hoa_shop::ShopMode();
	ModeManager->Push(SM);
}

// 'Resolution' confirmed
void BootMode::_OnResolution() {
	_active_menu = &_resolution_menu;
}


// 'Video' confirmed
void BootMode::_OnVideoOptions()
{
	_active_menu = &_video_options_menu;
	_UpdateVideoOptions();
}


// 'Audio' confirmed
void BootMode::_OnAudioOptions()
{
	// Switch the current menu
	_active_menu = &_audio_options_menu;
	_UpdateAudioOptions();
}


// 'Language' confirmed
void BootMode::_OnLanguageOptions()
{
	// Switch the current menu
	_active_menu = &_language_options_menu;
	//_UpdateLanguageOptions();
}


// 'Key settings' confirmed
void BootMode::_OnKeySettings() {
	_active_menu = &_key_settings_menu;
	_UpdateKeySettings();
}


// 'Joystick settings' confirmed
void BootMode::_OnJoySettings() {
	_active_menu = &_joy_settings_menu;
	_UpdateJoySettings();
}


// 'Video mode' confirmed
void BootMode::_OnVideoMode() {
	// Toggle fullscreen / windowed
	VideoManager->ToggleFullscreen();
	VideoManager->ApplySettings();
	_UpdateVideoOptions();
	_has_modified_settings = true;
}


// Specific language selected
void BootMode::_OnLanguageSelect() {
	SystemManager->SetLanguage("fr");
	// TODO: when the new language is set by the above call, we need to reload/refresh all text,
	// otherwise the new language will not take effect.
}


// Sound volume down
void BootMode::_OnSoundLeft() {
	AudioManager->SetSoundVolume(AudioManager->GetSoundVolume() - 0.1f);
	_UpdateAudioOptions();
	_boot_sounds.at(4).Play(); // Play a sound for user to hear new volume level.
	_has_modified_settings = true;
}


// Sound volume up
void BootMode::_OnSoundRight() {
	AudioManager->SetSoundVolume(AudioManager->GetSoundVolume() + 0.1f);
	_UpdateAudioOptions();
	_boot_sounds.at(4).Play(); // Play a sound for user to hear new volume level
	_has_modified_settings = true;
}


// Music volume down
void BootMode::_OnMusicLeft() {
	AudioManager->SetMusicVolume(AudioManager->GetMusicVolume() - 0.1f);
	_UpdateAudioOptions();
	_has_modified_settings = true;
}


// Music volume up
void BootMode::_OnMusicRight() {
	AudioManager->SetMusicVolume(AudioManager->GetMusicVolume() + 0.1f);
	_UpdateAudioOptions();
	_has_modified_settings = true;
}

// Resolution setters
void BootMode::_SetResolution(int32 width, int32 height) {
	VideoManager->SetResolution(width, height);
	VideoManager->ApplySettings();
// 	_active_menu = &_video_options_menu; // return back to video options
	_UpdateVideoOptions();
	_has_modified_settings = true;
}

void BootMode::_OnResolution640x480() {
	if (VideoManager->GetScreenWidth() != 640 && VideoManager->GetScreenHeight() != 480)
		_SetResolution(640, 480);
}

void BootMode::_OnResolution800x600() {
	if (VideoManager->GetScreenWidth() != 800 && VideoManager->GetScreenHeight() != 600)
		_SetResolution(800, 600);
}

void BootMode::_OnResolution1024x768() {
	if (VideoManager->GetScreenWidth() != 1024 && VideoManager->GetScreenHeight() != 768)
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
	_has_modified_settings = true;
}


// Restores default joystick settings
void BootMode::_OnRestoreDefaultJoyButtons() {
	InputManager->RestoreDefaultJoyButtons();
	_UpdateJoySettings();
	_has_modified_settings = true;
}


// Updates the video options screen
void BootMode::_UpdateVideoOptions() {
	// Update resolution text
	std::ostringstream resolution("");
	resolution << "Resolution: " << VideoManager->GetScreenWidth() << " x " << VideoManager->GetScreenHeight();
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
	_key_settings_menu.SetOptionText(0, MakeUnicodeString("Move Up<r>" + InputManager->GetUpKeyName()));
	_key_settings_menu.SetOptionText(1, MakeUnicodeString("Move Down<r>" + InputManager->GetDownKeyName()));
	_key_settings_menu.SetOptionText(2, MakeUnicodeString("Move Left<r>" + InputManager->GetLeftKeyName()));
	_key_settings_menu.SetOptionText(3, MakeUnicodeString("Move Right<r>" + InputManager->GetRightKeyName()));
	_key_settings_menu.SetOptionText(4, MakeUnicodeString("Confirm<r>" + InputManager->GetConfirmKeyName()));
	_key_settings_menu.SetOptionText(5, MakeUnicodeString("Cancel<r>" + InputManager->GetCancelKeyName()));
	_key_settings_menu.SetOptionText(6, MakeUnicodeString("Menu<r>" + InputManager->GetMenuKeyName()));
	_key_settings_menu.SetOptionText(7, MakeUnicodeString("Swap<r>" + InputManager->GetSwapKeyName()));
	_key_settings_menu.SetOptionText(8, MakeUnicodeString("Left Select<r>" + InputManager->GetLeftSelectKeyName()));
	_key_settings_menu.SetOptionText(9, MakeUnicodeString("Right Select<r>" + InputManager->GetRightSelectKeyName()));
	_key_settings_menu.SetOptionText(10, MakeUnicodeString("Pause<r>" + InputManager->GetPauseKeyName()));
}


void BootMode::_UpdateJoySettings() {
	int32 i = 0;
	_joy_settings_menu.SetOptionText(i++, MakeUnicodeString("X Axis<r>" + NumberToString(InputManager->GetXAxisJoy())));
	_joy_settings_menu.SetOptionText(i++, MakeUnicodeString("Y Axis<r>" + NumberToString(InputManager->GetYAxisJoy())));
	_joy_settings_menu.SetOptionText(i++, MakeUnicodeString("Confirm: Button<r>" + NumberToString(InputManager->GetConfirmJoy())));
	_joy_settings_menu.SetOptionText(i++, MakeUnicodeString("Cancel: Button<r>" + NumberToString(InputManager->GetCancelJoy())));
	_joy_settings_menu.SetOptionText(i++, MakeUnicodeString("Menu: Button<r>" + NumberToString(InputManager->GetMenuJoy())));
	_joy_settings_menu.SetOptionText(i++, MakeUnicodeString("Swap: Button<r>" + NumberToString(InputManager->GetSwapJoy())));
	_joy_settings_menu.SetOptionText(i++, MakeUnicodeString("Left Select : Button<r>" + NumberToString(InputManager->GetLeftSelectJoy())));
	_joy_settings_menu.SetOptionText(i++, MakeUnicodeString("Right Select: Button<r>" + NumberToString(InputManager->GetRightSelectJoy())));
	_joy_settings_menu.SetOptionText(i++, MakeUnicodeString("Pause: Button<r>" + NumberToString(InputManager->GetPauseJoy())));
}


// Saves all the game settings into a .lua file
void BootMode::_SaveSettingsFile() {

	// No need to save the settings if we haven't edited anything!
	if (!_has_modified_settings)
		return;

	// Load the settings file for reading in the original data
	string file = GetUserDataPath(false) + "settings.lua";
	if (!DoesFileExist(file))
		CopyFile(string("dat/config/settings.lua"), file);

	ModifyScriptDescriptor settings_lua;
	if (!settings_lua.OpenFile(file)) {
		cout << "BOOT ERROR: failed to load the settings file!" << endl;
	}

	// Write the current settings into the .lua file
	settings_lua.ModifyInt("settings.welcome", 0);
	// video
	settings_lua.OpenTable("settings");
	settings_lua.ModifyInt("video_settings.screen_resx", VideoManager->GetScreenWidth());
	settings_lua.ModifyInt("video_settings.screen_resy", VideoManager->GetScreenHeight());
	settings_lua.ModifyBool("video_settings.full_screen", VideoManager->IsFullscreen());
	//settings_lua.ModifyFloat("video_settings.brightness", VideoManager->GetGamma());

	// audio
	settings_lua.ModifyFloat("audio_settings.music_vol", AudioManager->GetMusicVolume());
	settings_lua.ModifyFloat("audio_settings.sound_vol", AudioManager->GetSoundVolume());

	// input
	settings_lua.ModifyInt("key_settings.up", InputManager->GetUpKey());
	settings_lua.ModifyInt("key_settings.down", InputManager->GetDownKey());
	settings_lua.ModifyInt("key_settings.left", InputManager->GetLeftKey());
	settings_lua.ModifyInt("key_settings.right", InputManager->GetRightKey());
	settings_lua.ModifyInt("key_settings.confirm", InputManager->GetConfirmKey());
	settings_lua.ModifyInt("key_settings.cancel", InputManager->GetCancelKey());
	settings_lua.ModifyInt("key_settings.menu", InputManager->GetMenuKey());
	settings_lua.ModifyInt("key_settings.swap", InputManager->GetSwapKey());
	settings_lua.ModifyInt("key_settings.left_select", InputManager->GetLeftSelectKey());
	settings_lua.ModifyInt("key_settings.right_select", InputManager->GetRightSelectKey());
	settings_lua.ModifyInt("key_settings.pause", InputManager->GetPauseKey());
	settings_lua.ModifyInt("joystick_settings.x_axis", InputManager->GetXAxisJoy());
	settings_lua.ModifyInt("joystick_settings.y_axis", InputManager->GetYAxisJoy());
	settings_lua.ModifyInt("joystick_settings.confirm", InputManager->GetConfirmJoy());
	settings_lua.ModifyInt("joystick_settings.cancel", InputManager->GetCancelJoy());
	settings_lua.ModifyInt("joystick_settings.menu", InputManager->GetMenuJoy());
	settings_lua.ModifyInt("joystick_settings.swap", InputManager->GetSwapJoy());
	settings_lua.ModifyInt("joystick_settings.left_select", InputManager->GetLeftSelectJoy());
	settings_lua.ModifyInt("joystick_settings.right_select", InputManager->GetRightSelectJoy());
	settings_lua.ModifyInt("joystick_settings.pause", InputManager->GetPauseJoy());

	// and save it!
	settings_lua.CommitChanges();
	settings_lua.CloseFile();

	_has_modified_settings = false;
}

} // namespace hoa_boot
