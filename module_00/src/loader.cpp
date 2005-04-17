/* 
 * loader.cpp
 *	Hero of Allacrost game intialization code
 *	(C) 2004 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */

#include <iostream>
#include <string>
#include "utils.h"
#include "audio.h"
#include "video.h"
#include "data.h"
#include "global.h"
#include "boot.h"

using namespace std;
using namespace hoa_global;
using namespace hoa_utils;
using namespace hoa_data;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_boot;

using namespace hoa_map;

const bool LOADER_DEBUG = false;

// PrintUsage prints out the usage options (arguments) for running the program
void PrintUsage();

// EnableDebugging enables various debugging print statements in different parts of the system
void EnableDebugging();

// SystemInfo prints the user's system information, as SDL sees it
void SystemInfo();

// FileCheck compares the filenames and (where appropriate) sizes to check the integrity of the game files. 
void FileCheck();

// Resets the game settings to default.
void ResetSettings();



// Handles all event processing in the game
void EventHandler();

// Handles all game keyboard events
void KeyEventHandler(SDL_KeyboardEvent *key_event);

// Handles all game joystick events
void JoystickEventHandler(SDL_Event *js_event);


// Prints out the usage options (arguments) for running the program (work in progress)
void PrintUsage() {
	cout << "Found a -usage argument!" << endl;
	cout << "User Event range: [" << SDL_USEREVENT << "," << SDL_NUMEVENTS-1 << "]: "
			 << SDL_NUMEVENTS - SDL_USEREVENT << " distinct user events." << endl;
}



// Prints version numbers for SDL libraries, video rendering information, and other info
//	about the user's system (work in progress)
void PrintSysInfo() {
	cout << "_____Printing system information_____" << endl;
	
	// Initialize SDL and its subsystems and make sure it shutdowns properly on exit
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) != 0) {
		cout << "ERROR: Unable to initialize SDL: " << SDL_GetError() << endl;
		return;
	}
	else
		cout << "SDL Initialized succesfully." << endl;
	atexit(SDL_Quit);

	//VideoManager.InitVideo(); // Initialize the video manager
	//AudioManager.InitAudio(); // Initialize the audio manager
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
	cout << ">Creates hardware surfaces: ";
	if (user_video->hw_available == 1) cout << "yes\n";
	else cout << "no\n";
	cout << ">Has window manager available: ";
	if (user_video->wm_available == 1) cout << "yes\n";
	else cout << "no\n";
	cout << ">Hardware to hardware blits accelerated: ";
	if (user_video->blit_hw == 1) cout << "yes\n";
	else cout << "no\n";
	cout << ">Hardware to hardware colorkey blits accelerated: ";
	if (user_video->blit_hw_CC == 1) cout << "yes\n";
	else cout << "no\n";
	cout << ">Hardware to hardware alpha blits accelerated: ";
	if (user_video->blit_hw_A == 1) cout << "yes\n";
	else cout << "no\n";
	cout << ">Software to hardware blits acceleerated: ";
	if (user_video->blit_sw == 1) cout << "yes\n";
	else cout << "no\n";
	cout << ">Software to hardware colorkey blits accelerated: ";
	if (user_video->blit_sw_CC == 1) cout << "yes\n";
	else cout << "no\n";
	cout << ">Software to hardware alpha blits accelerated: ";
	if (user_video->blit_sw_A == 1) cout << "yes\n";
	else cout << "no\n";
	cout << ">Color fills accelerated: ";
	if (user_video->blit_fill == 1) cout << "yes\n";
	else cout << "no\n";
	cout << ">Total video memory: " << user_video->video_mem << " kilobytes" << "\n" << endl;
	
	//cout << "The best pixel format: " << user_video->vfmt << endl;
	
	// Audio Information
	
	// Joystick Information
	cout << " *** JOYSTICK INFORMATION *** " << endl;
	
	SDL_Joystick *js_test;
	int js_num = SDL_NumJoysticks(); // Get the number of joysticks available
	cout << "SDL has recognized " << js_num << " on this system." << endl;
	for (int i = 0; i < js_num; i++) { // Print out information about each joystick
		js_test = SDL_JoystickOpen(i);
		if (js_test == NULL)
			cout << "ERROR: SDL was unable to open joystick #" << i << endl;
		else {
			cout << "Joystick #" << i << endl;
			cout << ">Name: " << SDL_JoystickName(i) << endl;
			cout << ">Axes: " << SDL_JoystickNumAxes(js_test) << endl;
			cout << ">Buttons: " << SDL_JoystickNumButtons(js_test) << endl;
			cout << ">Trackballs: " << SDL_JoystickNumBalls(js_test) << endl;
			cout << ">Hat Switches: " << SDL_JoystickNumHats(js_test) << endl;
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
void PrintFileCheck() {
	cout << "Found a -check argument!" << endl;
}



// If you don't know what main is, you shouldn't be looking at this code.
int main(int argc, char *argv[]) {
	//ProcessCommandArguments(argc, argv);

	// Initialize SDL. The video, audio, and joystick subsystems are initialized elsewhere.
	if (SDL_Init(SDL_INIT_TIMER) != 0) {
		cout << "ERROR: Unable to initialize SDL: " << SDL_GetError() << endl;
		return 1;
	}
	atexit(SDL_Quit);
	
	SDL_EnableUNICODE(1); // Enable unicode for multilingual support
	
	// Ignore the events that we don't care about so they never appear in the event queue
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_VIDEOEXPOSE, SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);
	// NOTE: SDL_ActiveEvent reports mouse focus, input focus, iconified status. Should we disable it???
	
	// Does the subsystem need to be initialized before checking the num of joysticks?
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	if (SDL_NumJoysticks() == 0) { // No joysticks found
		SDL_JoystickEventState(SDL_IGNORE);
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	}
	else // At least one joystick exists
		SDL_JoystickEventState(SDL_ENABLE);
	
	GameAudio *AudioManager = GameAudio::_Create();
	GameVideo *VideoManager = GameVideo::_Create();
	GameData *DataManager = GameData::_Create();
	GameModeManager *ModeManager = GameModeManager::_Create();
	GameSettings *SettingsManager = GameSettings::_Create();
	
	DataManager->LoadGameSettings(); // Initializes remaining data members of Settings Manager
	
	AudioManager->SetMusicVolume(SettingsManager->music_vol);
	AudioManager->SetSoundVolume(SettingsManager->sound_vol);

	SDL_Rect test_mode = {0,0,800,600}; // TEMPORARY
	VideoManager->ChangeMode(test_mode);

	BootMode* BM = new BootMode(); // Create our first game mode...
	ModeManager->Push(BM);         // ...and push it on the game mode stack!
		
	SettingsManager->SetTimer();	// Initialize our game and FPS timers
	
	// This is the main loop for the game. The loop iterates once every frame.
	while (SettingsManager->not_done) {
		// 1) Draws the screen to the back buffer
		ModeManager->GetTop()->Draw();
		
		// 2) Displays the new frame on the screen
		VideoManager->Render();

		// 3) Process all new events
		SettingsManager->EventHandler();
		
		// 4) Updates the game status
		ModeManager->GetTop()->Update(SettingsManager->UpdateTime());
		      
		// 5) Reset our key and joystick press and release flags
		//SettingsManager->ResetInputFlags();
	} // while (SettingsManager.not_done) 

	// Begin exit sequence and destroy our singleton classes
	GameAudio::_Destroy();
	GameVideo::_Destroy();
	GameData::_Destroy();
	GameModeManager::_Destroy();
	GameSettings::_Destroy();
	
	return 0;
}
