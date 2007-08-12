////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    main.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Allacrost initialization code and main game loop.
***
*** The code in this file is the first to execute when the game is started and
*** the last to execute before the game exits. The core engine of Allacrost
*** uses time-based updating, which means that the state of the game is
*** updated based on how much time has expired since the last update.
***
*** The main game loop consists of the following steps.
***
*** -# Render the newly drawn frame to the screen.
*** -# Collect information on new user input events.
*** -# Update the main loop timer.
*** -# Update the game status based on how much time expired from the last update.
*** ***************************************************************************/

#include <ctime>
#ifdef __MACH__
	#include <unistd.h>
	#include <string>
#endif

#include "utils.h"
#include "defs.h"

#include "main_options.h"

#include "audio_stream.h"
#include "audio.h"
#include "audio_descriptor.h"
#include <iostream>
#include <ctime>
#include <cmath>
#include <string>
#include "video.h"
#include "input.h"
#include "script.h"
#include "system.h"
#include "global.h"
#include "mode_manager.h"

#include "boot.h"
#include "map.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_script;
using namespace hoa_boot;
using namespace hoa_map;


/** \brief Frees all data allocated by Allacrost by destroying the singleton classes
***
*** \note <b>Do not attempt to call or otherwise reference this function.</b>
*** It is for use in the application's main() function only.
***
*** Deleteing the singleton class objects will free all of the memory that the game uses.
*** This is because all other classes and data structures in Allacrost are managed
*** by these singletons either directly or in directly. For example, BattleMode is a
*** class object that is managed by the GameModeManager class, and thus the GameModeManager
*** destructor will also invoke the BattleMode destructor (as well as the destructors of any
*** other game modes that exist).
**/
void QuitAllacrost() {
	// NOTE: Even if the singleton objects do not exist when this function is called, invoking the
	// static Destroy() singleton function will do no harm (it checks that the object exists before deleting it).

	// Delete the mode manager first so that all game modes free their resources
	GameModeManager::SingletonDestroy();

	// Delete the global manager second to remove all object references corresponding to other engine subsystems
	GameGlobal::SingletonDestroy();

	// Delete all of the reamining independent engine components
	GameAudio::SingletonDestroy();
	GameInput::SingletonDestroy();
	GameScript::SingletonDestroy();
	GameSystem::SingletonDestroy();
	GameVideo::SingletonDestroy();
} // void QuitAllacrost()


/** \brief Initializes all engine components and makes other preparations for the game to start
*** \return True if the game engine was initialized successfully, false if an unrecoverable error occured
***
**/
void InitializeEngine() throw (Exception) {
	// Initialize SDL. The video, audio, and joystick subsystems are initialized elsewhere.
 if (SDL_Init(SDL_INIT_TIMER) != 0) {
		throw Exception("MAIN ERROR: Unable to initialize SDL: ", __FILE__, __LINE__, __FUNCTION__);
	}

	// Create and initialize singleton class managers
	AudioManager = GameAudio::SingletonCreate();
	InputManager = GameInput::SingletonCreate();
	VideoManager = GameVideo::SingletonCreate();
	ScriptManager = GameScript::SingletonCreate();
	SystemManager = GameSystem::SingletonCreate();
	ModeManager = GameModeManager::SingletonCreate();
	GlobalManager = GameGlobal::SingletonCreate();

	if (VideoManager->SingletonInitialize() == false) {
		throw Exception("ERROR: unable to initialize VideoManager", __FILE__, __LINE__, __FUNCTION__);
	}

	if (VideoManager->GUI()->LoadMenuSkin("black_sleet", "img/menus/black_sleet_skin.png", "img/menus/black_sleet_texture.png") == false) {
		throw Exception("Failed to load the 'Black Sleet' MenuSkin images.", __FILE__, __LINE__, __FUNCTION__);
	}

	if (VideoManager->LoadFont("img/fonts/vtc_switchblade_romance.ttf", "default", 18) == false) {
		throw Exception("Failed to load the 'Switchblade Romance' font as 'default, size 18'", __FILE__, __LINE__, __FUNCTION__);
	}

	VideoManager->SetFontShadowOffsets("default", 1, -2);
	VideoManager->SetFontShadowStyle("default", VIDEO_TEXT_SHADOW_BLACK);

	if (VideoManager->LoadFont("img/fonts/vtc_switchblade_romance.ttf", "map", 24) == false) {
		throw Exception("Failed to load the 'Switchblade Romance' font as 'map, size 24'", __FILE__, __LINE__, __FUNCTION__);
	}

	VideoManager->SetFontShadowOffsets("map", 0, 0);
	VideoManager->SetFontShadowStyle("map", VIDEO_TEXT_SHADOW_BLACK);

	if (VideoManager->LoadFont("img/fonts/vtc_switchblade_romance.ttf", "battle", 20) == false) {
		throw Exception("Failed to load the 'Switchblade Romance' font as 'battle, size 20'", __FILE__, __LINE__, __FUNCTION__);
	}

	VideoManager->SetFontShadowOffsets("battle", 1, -2);
	VideoManager->SetFontShadowStyle("battle", VIDEO_TEXT_SHADOW_BLACK);

	// Font used to show damage received / given in battle mode
	if (VideoManager->LoadFont("img/fonts/vtc_switchblade_romance.ttf", "battle_dmg", 24) == false) {
		throw Exception("Failed to load the 'Switchblade Romance' font as 'battle_dmg, size 24'", __FILE__, __LINE__, __FUNCTION__);
	}

	VideoManager->SetFontShadowOffsets("battle_dmg", 1, -2);
	VideoManager->SetFontShadowStyle("battle_dmg", VIDEO_TEXT_SHADOW_BLACK);

	if (AudioManager->SingletonInitialize() == false) {
		throw Exception("ERROR: unable to initialize AudioManager", __FILE__, __LINE__, __FUNCTION__);
	}

	if (ScriptManager->SingletonInitialize() == false) {
		throw Exception("ERROR: unable to initialize ScriptManager", __FILE__, __LINE__, __FUNCTION__);
	}
	hoa_defs::BindEngineToLua();

	if (SystemManager->SingletonInitialize() == false) {
		throw Exception("ERROR: unable to initialize SystemManager", __FILE__, __LINE__, __FUNCTION__);
	}
	if (InputManager->SingletonInitialize() == false) {
		throw Exception("ERROR: unable to initialize InputManager", __FILE__, __LINE__, __FUNCTION__);
	}
	if (GlobalManager->SingletonInitialize() == false) {
		throw Exception("ERROR: unable to initialize GlobalManager", __FILE__, __LINE__, __FUNCTION__);
	}
	if (ModeManager->SingletonInitialize() == false) {
		throw Exception("ERROR: unable to initialize ModeManager", __FILE__, __LINE__, __FUNCTION__);
	}

	// Set the window title and icon name
	SDL_WM_SetCaption("Hero of Allacrost", "Hero of Allacrost");

	// Set the window icon
	#ifdef _WIN32
		SDL_WM_SetIcon(SDL_LoadBMP("img/logos/program_icon.bmp"), NULL);
	#else
		// Later, add an icon here for non-Windows systems (which support more than 32x32 .bmp files)
		SDL_WM_SetIcon(SDL_LoadBMP("img/logos/program_icon.bmp"), NULL);
	#endif

	// Hide the mouse cursor since we don't use or acknowledge mouse input from the user
	SDL_ShowCursor(SDL_DISABLE);

	// Enabled for multilingual keyboard support
	SDL_EnableUNICODE(1);

	// Ignore the events that we don't care about so they never appear in the event queue
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_VIDEOEXPOSE, SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);

	SystemManager->InitializeTimers();

} // void InitializeEngine()



// Every great game begins with a single function :)
int32 main(int32 argc, char *argv[]) {
	// When the program exits, first the QuitAllacrost() function is called, followed by SDL_Quit()
	atexit(SDL_Quit);
	atexit(QuitAllacrost);

	try {
		// Change to the directory where the Allacrost data is stored
		#ifdef __MACH__
			string path;
			path = argv[0];
			// Remove the binary name
			path.erase(path.find_last_of('/'));
			// Remove the MacOS directory
			path.erase(path.find_last_of('/'));
			// Now the program should be in app/Contents
			path.append ("/Resources/");
			chdir(path.c_str());
		#elif defined(__linux__) || defined(__FreeBSD__)
			// Look for data files in DATADIR only if they are not available in the
			// current directory.
			if (ifstream("dat/config/settings.lua") == NULL)
				chdir(DATADIR);
		#endif

		// Initialize the random number generator (note: 'unsigned int' is a required usage in this case)
		srand(static_cast<unsigned int>(time(NULL)));

		// This variable will be set by the ParseProgramOptions function
		int32 return_code = EXIT_FAILURE;

		// Parse command lines and exit out of the game if needed
		if (hoa_main::ParseProgramOptions(return_code, argc, argv) == false) {
			return return_code;
		}

		// Function call below throws exceptions if any errors occur
		InitializeEngine();

	} catch (Exception& e) {
		#ifdef WIN32
		MessageBox(NULL, e.ToString().c_str(), "Unhandled exception", MB_OK | MB_ICONERROR);
		#else
		cerr << e.ToString() << std::endl;
		#endif
		return EXIT_FAILURE;
	}


	try {
		// This is the main loop for the game. The loop iterates once for every frame drawn to the screen.
		while (SystemManager->NotDone()) {
			// 1) Render the scene
			VideoManager->Clear();
			ModeManager->Draw();
			VideoManager->Display(SystemManager->GetUpdateTime());

			// 2) Process all new events
			InputManager->EventHandler();

			// 3) Update any streaming audio sources
			AudioManager->Update();

			// 4) Update timers for correct time-based movement operation
			SystemManager->UpdateTimers();

			// 5) Update the game status
			ModeManager->Update();
		} // while (SystemManager->NotDone())
	} catch (Exception& e) {
		#ifdef WIN32
		MessageBox(NULL, e.ToString().c_str(), "Unhandled exception", MB_OK | MB_ICONERROR);
		#else
		cerr << e.ToString() << std::endl;
		#endif
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
} // int32 main(int32 argc, char *argv[])
