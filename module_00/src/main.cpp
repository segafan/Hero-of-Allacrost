///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    main.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 12th, 2005
 * \brief   Allacrost initialization code and main game loop.
 *
 * The code in this file is the first to run when the game is started and the
 * last to run before the game exits. It also processes command-line arguments.
 * The core engine of Allacrost uses time-based updating, which basically means
 * that we update the game status by different amounts based on how much time
 * expired since the last update. The main game loop consists of the following
 * steps.
 *
 * -# Render the newly drawn frame to the screen.
 * -# Collect information on new user input events.
 * -# Update the main loop timer.
 * -# Update the game status based on how much time expired from the last update.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include <string>
#include "defs.h"
#include "audio.h"
#include "video.h"
#include "data.h"
#include "engine.h"
#include "global.h"
#include "boot.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_engine;
using namespace hoa_global;
using namespace hoa_data;
using namespace hoa_boot;
using namespace hoa_map;


/*!
 *  \brief Prints out the usage options (command-line arguments) for running the program.
 */
void PrintUsage();

/*!
 *  \brief Enables debugging print statements in various parts of the game engine.
 *  \param *arg The name of the debugger variable to enable.
 */
void EnableDebugging(const char* arg);

/*!
 *  \brief Prints information about the user's system, as SDL sees it.
 */
void SystemInfo();

/*!
 *  \brief Checks the integrity of the game's file structure to make sure no files are missing or corrupt.
 */
void FileCheck();

/*!
 *  \brief Resets the game settings (audio levels, key mappings, etc.) to their default values.
 */
void ResetSettings();



// Prints out the usage options (arguments) for running the program (work in progress)
void PrintUsage() {
	cout << "Usage: allacrost -d arg -[chir]" << endl;
	cout << "User Event range: [" << SDL_USEREVENT << "," << SDL_NUMEVENTS-1 << "]: "
			 << SDL_NUMEVENTS - SDL_USEREVENT << " distinct user events." << endl;
}


// EnableDebugging enables various debugging print statements in different parts of the game
void EnableDebugging(const char* arg) {
	if (strcmp(arg, "all") == 0) {
		hoa_audio::AUDIO_DEBUG = true;
		hoa_battle::BATTLE_DEBUG = true;
		hoa_boot::BOOT_DEBUG = true;
		hoa_data::DATA_DEBUG = true;
		hoa_engine::ENGINE_DEBUG = true;
		hoa_global::GLOBAL_DEBUG = true;
		hoa_map::MAP_DEBUG = true;
		hoa_menu::MENU_DEBUG = true;
		hoa_pause::PAUSE_DEBUG = true;
		hoa_quit::QUIT_DEBUG = true;
		hoa_scene::SCENE_DEBUG = true;
		hoa_utils::UTILS_DEBUG = true;
		hoa_video::VIDEO_DEBUG = true;
		return;
	}
	if (strcmp(arg, "audio") == 0) {
		hoa_audio::AUDIO_DEBUG = true;
		return;
	}
	if (strcmp(arg, "battle") == 0) {
		hoa_battle::BATTLE_DEBUG = true;
		return;
	}
	if (strcmp(arg, "boot") == 0) {
		hoa_boot::BOOT_DEBUG = true;
		return;
	}
	if (strcmp(arg, "data") == 0) {
		hoa_data::DATA_DEBUG = true;
		return;
	}
	if (strcmp(arg, "engine") == 0) {
		hoa_engine::ENGINE_DEBUG = true;
		return;
	}
	if (strcmp(arg, "global") == 0) {
		hoa_global::GLOBAL_DEBUG = true;
		return;
	}
	if (strcmp(arg, "map") == 0) {
		hoa_map::MAP_DEBUG = true;
		return;
	}
	if (strcmp(arg, "menu") == 0) {
		hoa_menu::MENU_DEBUG = true;
		return;
	}
	if (strcmp(arg, "pause") == 0) {
		hoa_pause::PAUSE_DEBUG = true;
		return;
	}
	if (strcmp(arg, "quit") == 0) {
		hoa_quit::QUIT_DEBUG = true;
		return;
	}
	if (strcmp(arg, "scene") == 0) {
		hoa_scene::SCENE_DEBUG = true;
		return;
	}
	if (strcmp(arg, "utils") == 0) {
		hoa_utils::UTILS_DEBUG = true;
		return;
	}
	if (strcmp(arg, "video") == 0) {
		hoa_video::VIDEO_DEBUG = true;
		return;
	}
}


// Prints version numbers for SDL libraries, video rendering information, and other info
//  about the user's system (work in progress)
void SystemInfo() {
	cout << "_____Printing system information_____" << endl;

	// Initialize SDL and its subsystems and make sure it shutdowns properly on exit
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) != 0) {
		cerr << "ERROR: Unable to initialize SDL: " << SDL_GetError() << endl;
		return;
	}
	else {
		cout << "SDL Initialized succesfully." << endl;
	}
	atexit(SDL_Quit);

	// General Information
	cout << " *** GENERAL INFORMATION *** " << endl;

	// Video Information
	cout << " *** VIDEO INFORMATION *** " << endl;
	char video_driver[20];
	SDL_VideoDriverName(video_driver, 20);
	cout << "Video driver name: " << video_driver << "\n" << endl;

	const SDL_VideoInfo *user_video;
	user_video = SDL_GetVideoInfo(); // Get information about the user's video system
	cout << "Best available video mode" << endl;
	cout << "> Creates hardware surfaces: ";
	if (user_video->hw_available == 1) cout << "yes\n";
	else cout << "no\n";
	cout << "> Has window manager available: ";
	if (user_video->wm_available == 1) cout << "yes\n";
	else cout << "no\n";
	cout << "> Hardware to hardware blits accelerated: ";
	if (user_video->blit_hw == 1) cout << "yes\n";
	else cout << "no\n";
	cout << "> Hardware to hardware colorkey blits accelerated: ";
	if (user_video->blit_hw_CC == 1) cout << "yes\n";
	else cout << "no\n";
	cout << "> Hardware to hardware alpha blits accelerated: ";
	if (user_video->blit_hw_A == 1) cout << "yes\n";
	else cout << "no\n";
	cout << "> Software to hardware blits acceleerated: ";
	if (user_video->blit_sw == 1) cout << "yes\n";
	else cout << "no\n";
	cout << "> Software to hardware colorkey blits accelerated: ";
	if (user_video->blit_sw_CC == 1) cout << "yes\n";
	else cout << "no\n";
	cout << "> Software to hardware alpha blits accelerated: ";
	if (user_video->blit_sw_A == 1) cout << "yes\n";
	else cout << "no\n";
	cout << "> Color fills accelerated: ";
	if (user_video->blit_fill == 1) cout << "yes\n";
	else cout << "no\n";
	cout << "> Total video memory: " << user_video->video_mem << " kilobytes" << "\n" << endl;

	//cout << "The best pixel format: " << user_video->vfmt << endl;

	// Audio Information

	// Joystick Information
	cout << " *** JOYSTICK INFORMATION *** " << endl;

	SDL_Joystick *js_test;
	int32 js_num = SDL_NumJoysticks(); // Get the number of joysticks available
	cout << "SDL has recognized " << js_num << " on this system." << endl;
	for (uint32 i = 0; i < js_num; i++) { // Print out information about each joystick
		js_test = SDL_JoystickOpen(i);
		if (js_test == NULL)
			cout << "ERROR: SDL was unable to open joystick #" << i << endl;
		else {
			cout << "Joystick #" << i << endl;
			cout << "> Name: " << SDL_JoystickName(i) << endl;
			cout << "> Axes: " << SDL_JoystickNumAxes(js_test) << endl;
			cout << "> Buttons: " << SDL_JoystickNumButtons(js_test) << endl;
			cout << "> Trackballs: " << SDL_JoystickNumBalls(js_test) << endl;
			cout << "> Hat Switches: " << SDL_JoystickNumHats(js_test) << endl;
			SDL_JoystickClose(js_test);
		}
	}
}


// Resets the game settings to default.
void ResetSettings() {
	cout << "Are you sure you want to reset your settings? Your current configuration will be lost." << endl;

	// And then if they say yes, we overwrite the user prefs file with the default prefs file
}



// NOTE: the following function contains operating system dependant code
// Prints any bad file checks (work in progress)
void FileCheck() {
	cout << "Found a -c argument!" << endl;
}



// If you don't know what main is, you shouldn't be looking at this code.
int32 main(int32 argc, char *argv[]) {
	// Process command arguments with getopt
	int32 arg;

	while ((arg = getopt(argc, argv, "+cd:hir")) != -1) {
		switch (arg) {
			case 'c':
				FileCheck();
				return 0;
			case 'd':
				EnableDebugging(optarg);
				break;
			case 'h':
				PrintUsage();
				return 0;
			case 'i':
				SystemInfo();
				return 0;
			case 'r':
				ResetSettings();
				return 0;
			case '?':
			default:
				// getopt automatically tells user about the bad argument
				PrintUsage();
				return 1;
		}
	}

	// Initialize SDL. The video, audio, and joystick subsystems are initialized elsewhere.
	if (SDL_Init(SDL_INIT_TIMER) != 0) {
		cerr << "ERROR: Unable to initialize SDL: " << SDL_GetError() << endl;
		return 1;
	}
	atexit(SDL_Quit);
	
	// Create all the game singletons
	GameAudio *AudioManager = GameAudio::Create();
	GameVideo *VideoManager = GameVideo::Create();
	GameData *DataManager = GameData::Create();
	GameModeManager *ModeManager = GameModeManager::Create();
	GameSettings *SettingsManager = GameSettings::Create();
	GameInput *InputManager = GameInput::Create();
	GameInstance *InstanceManager = GameInstance::Create();

	if (!VideoManager->Initialize()) {
		cerr << "ERROR: unable to initialize VideoManager" << endl;
		return 1;
	}
	if (!AudioManager->Initialize()) {
		cerr << "ERROR: unable to initialize AudioManager" << endl;
		return 1;
	}
	if (!DataManager->Initialize()) {
		cerr << "ERROR: unable to initialize DataManager" << endl;
		return 1;
	}
	if (!ModeManager->Initialize()) {
		cerr << "ERROR: unable to initialize ModeManager" << endl;
		return 1;
	}
	if (!SettingsManager->Initialize()) {
		cerr << "ERROR: unable to initialize SettingsManager" << endl;
		return 1;
	}
	if (!InputManager->Initialize()) {
		cerr << "ERROR: unable to initialize InputManager" << endl;
		return 1;
	}
	if (!InstanceManager->Initialize()) {
		cerr << "ERROR: unable to initialize InstanceManager" << endl;
		return 1;
	}
	
	// Disable (hide) the mouse cursor
	SDL_ShowCursor(SDL_DISABLE);
	
	// Set the window title + icon
	SDL_WM_SetCaption("Hero of Allacrost", NULL);
	
	// Enable unicode for multilingual keyboard support
	SDL_EnableUNICODE(1); 
	
	// Ignore the events that we don't care about so they never appear in the event queue
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_VIDEOEXPOSE, SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);
	// NOTE: SDL_ActiveEvent reports mouse focus, input focus, iconified status. Should we disable it???
	
	AudioManager->SetMusicVolume(SettingsManager->music_vol);
	AudioManager->SetSoundVolume(SettingsManager->sound_vol);
	
	
	VideoManager->SetMenuSkin("img/menus/concrete",
	                          Color(0.0f, 0.0f, 1.0f, 0.5f),
	                          Color(0.0f, 0.0f, 1.0f, 0.5f),
	                          Color(0.0f, 0.0f, 1.0f, 0.5f),
	                          Color(0.0f, 0.0f, 1.0f, 0.5f));
	if(!VideoManager->LoadFont("img/fonts/tarnhalo.ttf", "default", 16))
		cerr << "MAP: ERROR > Couldn't load map font!" << endl;



	// Retains the amount of time (in milliseconds) between main loop iterations
	uint32 frame_time = 0;
	SettingsManager->SetTimer();	// Initialize the game and frames-per-second timers

	// This is the main loop for the game. The loop iterates once every frame drawn to the screen.
	while (SettingsManager->NotDone()) {
		// 1) Render the scene
		VideoManager->Clear();
		ModeManager->GetTop()->Draw();
		VideoManager->Display(frame_time);
		
		// 2) Process all new events
		InputManager->EventHandler();
		
		// 3) Update the timer
		frame_time = SettingsManager->UpdateTime();
		
		// 4) Update the game status
		ModeManager->Update(frame_time);
	} // while (SettingsManager->NotDone())

	// Begin exit sequence and destroy the singleton classes
	GameData::Destroy();
	GameInput::Destroy();
	GameModeManager::Destroy();
	GameSettings::Destroy();
	GameInstance::Destroy();
	GameAudio::Destroy();
	GameVideo::Destroy();

	return 0;
}
