////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
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

#include "utils.h"
#include "defs.h"
#include "audio.h"
#include "video.h"
#include "data.h"
#include "mode_manager.h"
#include "input.h"
#include "system.h"
#include "global.h"
#include "boot.h"
#include "main_options.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_data;
using namespace hoa_boot;
using namespace hoa_map;


/** \brief Frees all data allocated by Allacrost by destroying the singleton classes
***
*** \note <b>Do not attempt to call or otherwise reference this function.</b>
*** It is for use in the application's main() function only.
***
*** Deleteing the singleton class objects is equivalent to deletion of all data structures.
*** This is because all other classes and data structures in Allacrost should be managed
*** by these singletons either directly, or in directly. For example, BattleMode is a
*** class object that is managed by the GameModeManager class, and thus the GameModeManager
*** destructor will also invoke the BattleMode destructor (as well as the destructors of any
*** other game modes that exist).
**/
void QuitAllacrost() {
	// NOTE: Even if the singleton objects do not exist when this function is called, invoking
	// the static Destroy() function will do no harm (it checks that the object exists before deleting it).

	// Delete the mode manager first so that all game modes free their data
	GameModeManager::SingletonDestroy();
	// Delete the global manager second to remove all object references corresponding to other engine subsystems
	GameGlobal::SingletonDestroy();
	// Delete all of the reamining independent engine components
	GameAudio::SingletonDestroy();
	GameInput::SingletonDestroy();
	GameSystem::SingletonDestroy();
	GameData::SingletonDestroy();
	GameVideo::SingletonDestroy();
}



// Every great game begins with a single function :)
int32 main(int32 argc, char *argv[]) {
	// When the program exits, first the QuitAllacrost() function, followed by SDL_Quit()
	atexit(SDL_Quit);
	atexit(QuitAllacrost);

	// Initialize the random number generator
	srand(static_cast<unsigned int>(time(NULL)));

	// This variable is set by the ParseProgramOptions function
	int32 return_code;

	// Parse command lines and exit out of the game if needed
	if (hoa_main::ParseProgramOptions(return_code, argc, argv) == false) {
		return return_code;
	}

	// Initialize SDL. The video, audio, and joystick subsystems are initialized elsewhere.
	if (SDL_Init(SDL_INIT_TIMER) != 0) {
		cerr << "MAIN ERROR: Unable to initialize SDL: " << SDL_GetError() << endl;
		return 1;
	}

	// Create singleton class managers
	AudioManager = GameAudio::SingletonCreate();
	InputManager = GameInput::SingletonCreate();
	VideoManager = GameVideo::SingletonCreate();
	DataManager = GameData::SingletonCreate();
	SystemManager = GameSystem::SingletonCreate();

	// NOTE: The GlobalManager will not be created until the user actually starts a game instance
	ModeManager = GameModeManager::SingletonCreate();

	if (VideoManager->SingletonInitialize() == false) {
		cerr << "ERROR: unable to initialize VideoManager" << endl;
		return 1;
	}
	
	VideoManager->SetMenuSkin("img/menus/black_sleet", "img/menus/black_sleet_texture.png", Color(0.0f, 0.0f, 0.0f, 0.0f));
	if (!VideoManager->LoadFont("img/fonts/vtc_switchblade_romance.ttf", "default", 18)) {
		return 1;
	}

	VideoManager->SetFontShadowXOffset("default", 1);
	VideoManager->SetFontShadowYOffset("default", -2);
	VideoManager->SetFontShadowStyle("default", VIDEO_TEXT_SHADOW_BLACK);

	VideoManager->SetFontShadowXOffset("default", 1);
	VideoManager->SetFontShadowYOffset("default", -2);
	VideoManager->SetFontShadowStyle("default", VIDEO_TEXT_SHADOW_BLACK);

	if (!VideoManager->LoadFont("img/fonts/vtc_switchblade_romance.ttf", "map", 24)) {
		return 1;
	}

	VideoManager->SetFontShadowXOffset("map", 0);
	VideoManager->SetFontShadowYOffset("map", 0);
	VideoManager->SetFontShadowStyle("map", VIDEO_TEXT_SHADOW_BLACK);

	if (!VideoManager->LoadFont("img/fonts/vtc_switchblade_romance.ttf", "battle", 20)) {
		return 1;
	}

	VideoManager->SetFontShadowXOffset("battle", 1);
	VideoManager->SetFontShadowYOffset("battle", -2);
	VideoManager->SetFontShadowStyle("battle", VIDEO_TEXT_SHADOW_BLACK);

	if (AudioManager->SingletonInitialize() == false) {
		cerr << "ERROR: unable to initialize AudioManager" << endl;
		return 1;
	}
	if (DataManager->SingletonInitialize() == false) {
		cerr << "ERROR: unable to initialize DataManager" << endl;
		return 1;
	}
	if (ModeManager->SingletonInitialize() == false) {
		cerr << "ERROR: unable to initialize ModeManager" << endl;
		return 1;
	}
	if (SystemManager->SingletonInitialize() == false) {
		cerr << "ERROR: unable to initialize SystemManager" << endl;
		return 1;
	}
	if (InputManager->SingletonInitialize() == false) {
		cerr << "ERROR: unable to initialize InputManager" << endl;
		return 1;
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

	SystemManager->InitializeTimers();

	// This is the main loop for the game. The loop iterates once every frame drawn to the screen.
	while (SystemManager->NotDone()) {
		// 1) Render the scene
		VideoManager->Clear();
		ModeManager->Draw();
		VideoManager->Display(SystemManager->GetUpdateTime());

		// 2) Process all new events
		InputManager->EventHandler();

		// 3) Update any streaming audio sources
		// AudioManager->Update();

		// 4) Update timers for correct time-based movement operation
		SystemManager->UpdateTimers();

		// 5) Update the game status
		ModeManager->Update();
	} // while (SystemManager->NotDone())

	return EXIT_SUCCESS;
} // int32 main(int32 argc, char *argv[])
