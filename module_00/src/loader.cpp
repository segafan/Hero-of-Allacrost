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
#include "global.h"
#include "audio.h"
#include "video.h"
#include "boot.h"
#include "data.h"
#include "utils.h"
#include "quit.h"

#include "map.h"

using namespace std;
using namespace hoa_global;
using namespace hoa_utils;
using namespace hoa_data;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_boot;
using namespace hoa_quit;

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



// Handles all of the event processing for the game.
void EventHandler() {
	Singleton<GameSettings> SettingsManager;
	Singleton<GameModeManager> ModeManager;
	SDL_Event event;	// Holds the game event
	
	// Reset all of our press and release flags so they don't get detected twice.
	SettingsManager->InputStatus.up_press = false; 
	SettingsManager->InputStatus.up_release = false;
	SettingsManager->InputStatus.down_press = false;
	SettingsManager->InputStatus.down_release = false;
	SettingsManager->InputStatus.left_press = false;
	SettingsManager->InputStatus.left_release = false; 
	SettingsManager->InputStatus.right_press = false;
	SettingsManager->InputStatus.right_release = false; 
	SettingsManager->InputStatus.confirm_press = false;
	SettingsManager->InputStatus.confirm_release = false; 
	SettingsManager->InputStatus.cancel_press = false;
	SettingsManager->InputStatus.cancel_release = false; 
	SettingsManager->InputStatus.menu_press = false;
	SettingsManager->InputStatus.menu_release = false;
	SettingsManager->InputStatus.swap_press = false;
	SettingsManager->InputStatus.swap_release = false;
	SettingsManager->InputStatus.rselect_press = false;
	SettingsManager->InputStatus.rselect_release = false;
	SettingsManager->InputStatus.lselect_press = false;
	SettingsManager->InputStatus.lselect_release = false;
	SettingsManager->InputStatus.pause_press = false;
	SettingsManager->InputStatus.pause_release = false;
	
	while (SDL_PollEvent(&event)) { // Loops until we are out of events to process	 
		switch (event.type) {	
			case SDL_QUIT:
				// We quit the game without question if we are in BootMode or QuitMode
				if (ModeManager->GetGameType() == boot_m || ModeManager->GetGameType() == quit_m) {
					SettingsManager->not_done = false;
				}
				// Otherwise, we push QuitMode onto the stack
				else {
					QuitMode *QM = new QuitMode();
					ModeManager->Push(QM);
				}
				return;
			case SDL_ACTIVEEVENT: // Should we care about Active events?
				if (LOADER_DEBUG) cout << "Active event" << endl;
				break;
			
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				KeyEventHandler(&event.key);
				break;
				
			case SDL_JOYAXISMOTION:
			case SDL_JOYBALLMOTION:
			case SDL_JOYHATMOTION:
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				JoystickEventHandler(&event);
				break;
		} // switch (event.type)
	}
}



// Handles all keyboard events for the game
void KeyEventHandler(SDL_KeyboardEvent *key_event) {
	Singleton<GameSettings> SettingsManager;
	Singleton<GameModeManager> ModeManager;
	
	if (key_event->type == SDL_KEYDOWN) { // Key was pressed
		if (key_event->keysym.mod & KMOD_CTRL) {
			switch (key_event->keysym.sym) {
				case SDLK_f:
					if (LOADER_DEBUG) cout << " Toggle Fullscreen!" << endl;
					return;
				case SDLK_s:
					if (LOADER_DEBUG) cout << " Took Screenshot!" << endl;
					return;
				case SDLK_q:
					// We quit the game without question if we are in BootMode or QuitMode
					if (ModeManager->GetGameType() == boot_m || ModeManager->GetGameType() == quit_m) {
						SettingsManager->not_done = false;
					}
					// Otherwise, we push QuitMode onto the stack
					else {
						QuitMode *QM = new QuitMode();
						ModeManager->Push(QM);
					}
					return;
				default:
					return;
			}
		}
		
		// Note: a switch-case statement won't work here because SettingsManager.InputStatus.key.up
		//	is not an integer value and if you set it as a case the compiler will whine and cry ;_;
		if (key_event->keysym.sym == SettingsManager->InputStatus.key.up) {
			if (LOADER_DEBUG) cout << " up key pressed." << endl;
			SettingsManager->InputStatus.up_state = true;
			SettingsManager->InputStatus.up_press = true;
			return;
		}
		else if (key_event->keysym.sym == SettingsManager->InputStatus.key.down) {
			if (LOADER_DEBUG) cout << " down key pressed." << endl;
			SettingsManager->InputStatus.down_state = true;
			SettingsManager->InputStatus.down_press = true;
			return;
		}
		else if (key_event->keysym.sym == SettingsManager->InputStatus.key.left) {
			if (LOADER_DEBUG) cout << " left key pressed." << endl;
			SettingsManager->InputStatus.left_state = true;
			SettingsManager->InputStatus.left_press = true;
			return;
		}
		else if (key_event->keysym.sym == SettingsManager->InputStatus.key.right) {
			if (LOADER_DEBUG) cout << " right key pressed." << endl;
			SettingsManager->InputStatus.right_state = true;
			SettingsManager->InputStatus.right_press = true;
			return;
		}
		else if (key_event->keysym.sym == SettingsManager->InputStatus.key.confirm) {
			if (LOADER_DEBUG) cout << " confirm key pressed." << endl;
			SettingsManager->InputStatus.confirm_state = true;
			SettingsManager->InputStatus.confirm_press = true;
			return;
		}
		else if (key_event->keysym.sym == SettingsManager->InputStatus.key.cancel) {
			if (LOADER_DEBUG) cout << " cancel key pressed." << endl;
			SettingsManager->InputStatus.cancel_state = true;
			SettingsManager->InputStatus.cancel_press = true;
			return;
		}
		else if (key_event->keysym.sym == SettingsManager->InputStatus.key.menu) {
			if (LOADER_DEBUG) cout << " menu key pressed." << endl;
			SettingsManager->InputStatus.menu_state = true;
			SettingsManager->InputStatus.menu_press = true;
			return;
		}
		else if (key_event->keysym.sym == SettingsManager->InputStatus.key.pause) {
			if (LOADER_DEBUG) cout << " pause key pressed." << endl;
			SettingsManager->InputStatus.pause_state = true;
			SettingsManager->InputStatus.pause_press = true;
			return;
		}
	}
	
	else { // Key was released
		if (key_event->keysym.mod & KMOD_CTRL) // We don't want to recognize a key release if ctrl is down
			return;
		
		if (key_event->keysym.sym == SettingsManager->InputStatus.key.up) {
			if (LOADER_DEBUG) cout << " up key released." << endl;
			SettingsManager->InputStatus.up_state = false;
			SettingsManager->InputStatus.up_release = true;
			return;
		}
		else if (key_event->keysym.sym == SettingsManager->InputStatus.key.down) {
			if (LOADER_DEBUG) cout << " down key released." << endl;
			SettingsManager->InputStatus.down_state = false;
			SettingsManager->InputStatus.down_release = true;
			return;
		}
		else if (key_event->keysym.sym == Singleton<GameSettings>()->InputStatus.key.left) {
			if (LOADER_DEBUG) cout << " left key released." << endl;
			SettingsManager->InputStatus.left_state = false;
			SettingsManager->InputStatus.left_release = true;
			return;
		}
		else if (key_event->keysym.sym == SettingsManager->InputStatus.key.right) {
			if (LOADER_DEBUG) cout << " right key released." << endl;
			SettingsManager->InputStatus.right_state = false;
			SettingsManager->InputStatus.right_release = true;
			return;
		}
		else if (key_event->keysym.sym == Singleton<GameSettings>()->InputStatus.key.confirm) {
			if (LOADER_DEBUG) cout << " confirm key released." << endl;
			SettingsManager->InputStatus.confirm_state = false;
			SettingsManager->InputStatus.confirm_release = true;
			return;
		}
		else if (key_event->keysym.sym == SettingsManager->InputStatus.key.cancel) {
			if (LOADER_DEBUG) cout << " cancel key released." << endl;
			SettingsManager->InputStatus.cancel_state = false;
			SettingsManager->InputStatus.cancel_release = true;
			return;
		}
		else if (key_event->keysym.sym == SettingsManager->InputStatus.key.menu) {
			if (LOADER_DEBUG) cout << " menu key released." << endl;
			SettingsManager->InputStatus.menu_state = false;
			SettingsManager->InputStatus.menu_release = true;
			return;
		}
		else if (key_event->keysym.sym == SettingsManager->InputStatus.key.pause) {
			if (LOADER_DEBUG) cout << " pause key released." << endl;
			SettingsManager->InputStatus.pause_state = false;
			SettingsManager->InputStatus.pause_release = true;
			return;
		}
	}
}



// Handles all joystick events for the game 
// >> on my to do list. - Tyler, Sept 22nd
void JoystickEventHandler(SDL_Event *js_event) {
	
	switch (js_event->type) {
		case SDL_JOYAXISMOTION:
		case SDL_JOYBALLMOTION:
		case SDL_JOYHATMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			break;
	}
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
	else												 // At least one joystick exists
		SDL_JoystickEventState(SDL_ENABLE);
	
	
	Singleton<GameAudio> AudioManager;
	Singleton<GameVideo> VideoManager;	
  
	Singleton<GameModeManager> ModeManager;
	
	Singleton<GameData> DataManager;
	Singleton<GameSettings> SettingsManager;
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
		EventHandler();
		
		// 4) Updates the game status
		ModeManager->GetTop()->Update(SettingsManager->UpdateTime());
		      
		// 5) Reset our key and joystick press and release flags
		//SettingsManager->ResetInputFlags();
	} // while (SettingsManager.not_done) 

	// This line is reached when SettingsManager->not_done is set to false
	return 0;
}
