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
bool InitializeEngine() {
	// Initialize SDL. The video, audio, and joystick subsystems are initialized elsewhere.
	if (SDL_Init(SDL_INIT_TIMER) != 0) {
		cerr << "MAIN ERROR: Unable to initialize SDL: " << SDL_GetError() << endl;
		return false;
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
		cerr << "ERROR: unable to initialize VideoManager" << endl;
		return false;
	}

	if (VideoManager->LoadMenuSkin("black_sleet", "img/menus/black_sleet_skin.png", "img/menus/black_sleet_texture.png") == false) {
		return false;
	}

	if (VideoManager->LoadFont("img/fonts/vtc_switchblade_romance.ttf", "default", 18) == false) {
		return false;
	}

	VideoManager->SetFontShadowXOffset("default", 1);
	VideoManager->SetFontShadowYOffset("default", -2);
	VideoManager->SetFontShadowStyle("default", VIDEO_TEXT_SHADOW_BLACK);

	VideoManager->SetFontShadowXOffset("default", 1);
	VideoManager->SetFontShadowYOffset("default", -2);
	VideoManager->SetFontShadowStyle("default", VIDEO_TEXT_SHADOW_BLACK);

	if (!VideoManager->LoadFont("img/fonts/vtc_switchblade_romance.ttf", "map", 24)) {
		return false;
	}

	VideoManager->SetFontShadowXOffset("map", 0);
	VideoManager->SetFontShadowYOffset("map", 0);
	VideoManager->SetFontShadowStyle("map", VIDEO_TEXT_SHADOW_BLACK);

	if (!VideoManager->LoadFont("img/fonts/vtc_switchblade_romance.ttf", "battle", 20)) {
		return false;
	}

	VideoManager->SetFontShadowXOffset("battle", 1);
	VideoManager->SetFontShadowYOffset("battle", -2);
	VideoManager->SetFontShadowStyle("battle", VIDEO_TEXT_SHADOW_BLACK);

	// Font used to show damage received / given in battle mode
	if (!VideoManager->LoadFont("img/fonts/vtc_switchblade_romance.ttf", "battle_dmg", 24)) {
		return false;
	}

	VideoManager->SetFontShadowXOffset("battle_dmg", 1);
	VideoManager->SetFontShadowYOffset("battle_dmg", -2);
	VideoManager->SetFontShadowStyle("battle_dmg", VIDEO_TEXT_SHADOW_BLACK);

/*	if (AudioManager->SingletonInitialize() == false) {
		cerr << "ERROR: unable to initialize AudioManager" << endl;
		return false;
	}*/
	if (ScriptManager->SingletonInitialize() == false) {
		cerr << "ERROR: unable to initialize ScriptManager" << endl;
		return false;
	}
	hoa_defs::BindEngineToLua();
	if (ModeManager->SingletonInitialize() == false) {
		cerr << "ERROR: unable to initialize ModeManager" << endl;
		return false;
	}
	if (SystemManager->SingletonInitialize() == false) {
		cerr << "ERROR: unable to initialize SystemManager" << endl;
		return false;
	}
	if (InputManager->SingletonInitialize() == false) {
		cerr << "ERROR: unable to initialize InputManager" << endl;
		return false;
	}
	if (GlobalManager->SingletonInitialize() == false) {
		cerr << "ERROR: unable to initialize GlobalManager" << endl;
		return false;
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

	return true;
} // bool InitializeEngine()




int test_time (10);
clock_t t0, t1;
std::string music1, music2;

void Wait (const float time)
{
	t0 = clock ();
	do
	{
		t1 = clock ();
	} while ((t1-t0)/CLOCKS_PER_SEC < time);
}


void TestStatic (SoundDescriptor &sd)
{
	std::cout << std::endl << "- Test 1 (" << test_time << "sec.)\tStatic sound on a single buffer" << std::endl;
	std::cout << "Sound just plays" << std::endl << std::endl;

	sd.LoadSound (music1.c_str(), hoa_audio::AUDIO_LOAD_STATIC);
	sd.SetLooping (true);
	sd.Play();
	t0 = clock ();
	do
	{
		t1 = clock ();
		AudioManager->Update ();
	} while ((t1-t0)/CLOCKS_PER_SEC < test_time);
	sd.FreeSound ();
	Wait (1);
}


void TestStreamMemory (SoundDescriptor &sd)
{
	std::cout << std::endl << "- Test 2 (" << test_time << "sec.)\tStreaming sound from memory" << std::endl;
	std::cout << "Sound just plays" << std::endl << std::endl;

	sd.LoadSound (music1.c_str(), hoa_audio::AUDIO_LOAD_STREAM_MEMORY);
	sd.SetLooping (true);
	sd.Play ();
	t0 = clock();
	do
	{
		t1 = clock ();
		AudioManager->Update ();
	} while ((t1-t0)/CLOCKS_PER_SEC < test_time);
	sd.FreeSound();
	Wait (1);
}


void TestStreamFile (SoundDescriptor &sd)
{
	std::cout << std::endl << "- Test 3 (" << test_time << "sec.)\tStreaming sound from file" << std::endl;
	std::cout << "Sound just plays" << std::endl << std::endl;

	sd.LoadSound (music1.c_str(), hoa_audio::AUDIO_LOAD_STREAM_FILE);
	sd.SetLooping (true);
	sd.Play ();
	t0 = clock();
	do
	{
		t1 = clock ();
		AudioManager->Update ();
	} while ((t1-t0)/CLOCKS_PER_SEC < test_time);
	sd.FreeSound();
	Wait (1);
}


void TestCustomizedLoop (SoundDescriptor &sd)
{
	std::cout << std::endl << "- Test 4 (" << test_time << "sec.)\tCustomized looping" << std::endl;
	std::cout << "Looping Start sample: 88200" << std::endl;
	std::cout << "Looping End sample: 176400" << std::endl;
	std::cout << "Sound is looping, so it plays from 0 to End sample, then loops ";
	std::cout << "around Start and End. After 6 seconds, looping is disabled, so the sound ";
	std::cout  << "continues playing to the end" << std::endl << std::endl;

	sd.LoadSound (music1.c_str(), hoa_audio::AUDIO_LOAD_STREAM_FILE);
	sd.SetLooping (true);
	sd.SetLoopStart (2*44100);
	sd.SetLoopEnd (4*44100);
	sd.Play ();
	t0 = clock ();
	bool change = true;
	do
	{
		t1 = clock ();
		if (change && (t1-t0)/CLOCKS_PER_SEC>6)
		{
			sd.SetLooping (false);
			change = false;
		}
		AudioManager->Update ();
	} while ((t1-t0)/CLOCKS_PER_SEC < test_time);
	sd.FreeSound();
	Wait (1);
}


void Test3D (SoundDescriptor &sd)
{
	std::cout << std::endl << "- Test 5 (" << test_time << "sec.)\tTesting 3D position" << std::endl;
	std::cout  << "Sound moves around the listener in an elipsis path" << std::endl << std::endl;

	sd.LoadSound (music1, hoa_audio::AUDIO_LOAD_STREAM_FILE);
	float inc = 0.000001f;
	float radian (0.0f);
	float position[3] = { 1.0f, 0, 0 };
	sd.SetLooping (true);
	sd.Play ();
	t0 = clock ();
	do
	{
		radian += inc;
		position[0] = 0.5f*cos(radian);
		position[1] = 2.0f*sin(radian);
		sd.SetPosition (position);
		t1 = clock ();
		AudioManager->Update ();
	} while ((t1-t0)/CLOCKS_PER_SEC < test_time);
	sd.FreeSound();
}


void TestFadeIn (SoundDescriptor &sd)
{
	std::cout << std::endl << "- Test 6 (" << test_time << "sec.)\tFade In" << std::endl;
	std::cout  << "Sounds fade in." << std::endl << std::endl;

	sd.LoadSound (music1, hoa_audio::AUDIO_LOAD_STREAM_FILE);
	sd.SetLooping (true);
	Effects::FadeIn(sd,50.0f);
	t0 = clock ();
	do
	{
		t1 = clock ();
		AudioManager->Update ();
	} while ((t1-t0)/CLOCKS_PER_SEC < test_time);
	sd.FreeSound ();
}


void TestFadeOut (SoundDescriptor &sd)
{
	std::cout << std::endl << "- Test 7 (" << test_time << "sec.)\tFade Out" << std::endl;
	std::cout  << "Sounds fade out." << std::endl << std::endl;

	sd.LoadSound (music1, hoa_audio::AUDIO_LOAD_STREAM_FILE);
	sd.SetLooping (true);
	sd.Play();
	Effects::FadeOut (sd,50.0f);
	t0 = clock ();
	do
	{
		t1 = clock ();
		AudioManager->Update ();
	} while ((t1-t0)/CLOCKS_PER_SEC < test_time);
	sd.FreeSound ();
}


void TestCrossFade (SoundDescriptor &sd1, SoundDescriptor &sd2)
{
	std::cout << std::endl << "- Test 8 (" << test_time << "sec.)\tCross Fade" << std::endl;
	std::cout  << "Sounds cross fade." << std::endl << std::endl;

	sd1.LoadSound (music1, hoa_audio::AUDIO_LOAD_STREAM_FILE);
	sd1.SetLooping (true);

	sd2.LoadSound (music2, hoa_audio::AUDIO_LOAD_STREAM_FILE);
	sd2.SetLooping (true);
	sd2.Play();

	Effects::CrossFade (sd1,sd2,50.0f);
	t0 = clock ();
	do
	{
		t1 = clock ();
		AudioManager->Update ();
	} while ((t1-t0)/CLOCKS_PER_SEC < test_time);
	sd1.FreeSound ();
	sd2.FreeSound ();
}

void TestPersistantSounds()
{
	std::cout << std::endl << "-Test 9\t Playing Persistant Sounds" << std::endl;
	std::cout << "Same file is played five times, loaded once, and remains in memory." << std::endl << std::endl;

	for (int i = 0; i < 5; i++)
	{
		GameAudio::PlayPersistantSound("confirm.wav");
		t0 = clock ();
		do
		{
			t1 = clock ();
			AudioManager->Update ();
		} while ((t1-t0)/CLOCKS_PER_SEC < 2);
	}
}

void TestMusicWithSound(SoundDescriptor &sd)
{
	std::cout << std::endl << "-Test 10\t Playing Sound and Music Together" << std::endl;
	std::cout << "Several sounds are played while music is also being played." << std::endl << std::endl;

	sd.LoadSound (music1, hoa_audio::AUDIO_LOAD_STREAM_FILE);
	sd.SetLooping (true);
	sd.Play();

	for (int i = 0; i < 10; i++)
	{
		if (i == 0 || i == 5)
		{
			GameAudio::PlayPersistantSound("confirm.wav");
			t0 = clock ();
			do
			{
				t1 = clock ();
				AudioManager->Update ();
			} while ((t1-t0)/CLOCKS_PER_SEC < 1);
		}
		else if (i == 1 || i == 6)
		{
			GameAudio::PlayPersistantSound("cancel.wav");
			t0 = clock ();
			do
			{
				t1 = clock ();
				AudioManager->Update ();
			} while ((t1-t0)/CLOCKS_PER_SEC < 1);
		}
		else if (i == 2 || i == 7)
		{
			GameAudio::PlayPersistantSound("obtain.wav");
			t0 = clock ();
			do
			{
				t1 = clock ();
				AudioManager->Update ();
			} while ((t1-t0)/CLOCKS_PER_SEC < 1);
		}
		else if (i == 3 || i == 8)
		{
			GameAudio::PlayPersistantSound("potion_drink.wav");
			t0 = clock ();
			do
			{
				t1 = clock ();
				AudioManager->Update ();
			} while ((t1-t0)/CLOCKS_PER_SEC < 1);
		}
		else if (i == 4 || i == 9)
		{
			GameAudio::PlayPersistantSound("skeleton_attack.wav");
			t0 = clock ();
			do
			{
				t1 = clock ();
				AudioManager->Update ();
			} while ((t1-t0)/CLOCKS_PER_SEC < 1);
		}
	}

	sd.FreeSound();
}

void RunTests()
{
	music1 = "seek.ogg";
	music2 = "intro.ogg";

	hoa_audio::SoundDescriptor sd1, sd2;

	std::cout << "Starting... " << std::endl;

	TestStatic (sd1);
	TestStreamMemory (sd1);
	TestStreamFile (sd1);
	TestCustomizedLoop (sd1);
	Test3D (sd1);
	TestFadeIn (sd1);
	TestFadeOut (sd1);
	TestCrossFade (sd1, sd2);
	TestPersistantSounds();
	TestMusicWithSound(sd1);

	std::cout << "Ending..." << std::endl;
}

// Every great game begins with a single function :)
int32 main(int32 argc, char *argv[]) {
	// When the program exits, first the QuitAllacrost() function is called, followed by SDL_Quit()
	atexit(SDL_Quit);
	atexit(QuitAllacrost);

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
	int32 return_code;

	// Parse command lines and exit out of the game if needed
	if (hoa_main::ParseProgramOptions(return_code, argc, argv) == false) {
		return return_code;
	}

	if (InitializeEngine() == false) {
		cerr << "ERROR: failed to initialize game engine, exiting..." << endl;
		return 1;
	}


//	RunTests();


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

	return EXIT_SUCCESS;
} // int32 main(int32 argc, char *argv[])
