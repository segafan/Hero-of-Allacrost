///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    main_options.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Implementations for functions that handle command-line arguments.
*** **************************************************************************/

#include "utils.h"
#include "defs.h"
#include "audio.h"
#include "video.h"
#include "data.h"
#include "mode_manager.h"
#include "input.h"
#include "settings.h"
#include "global.h"
#include "main_options.h"

using namespace std;
using namespace hoa_utils;

namespace hoa_main {

bool ParseProgramOptions(int32_t &return_code, int32_t argc, char **argv) {
	// Convert the argument list to a vector of strings for convenience
	std::vector<std::string> options(argv, argv + argc);
	return_code = 0;
	
	for (uint32 i = 1; i < options.size(); i++) {
		if (options[i] == "-c" || options[i] == "--check") {
			if (CheckFiles() == true) {
				return_code = 0;
			}
			else {
				return_code = 1;
			}
			return false;
		}
		else if (options[i] == "-d" || options[i] == "--debug") {
			if ((i + 1) >= options.size()) {
				cerr << "Option " << options[i] << "requires an argument." << endl;
				PrintUsage();
				return_code = 1;
				return false;
			}
			if (EnableDebugging(options[i + 1]) == false) {
				return_code = 1;
				return false;
			}
			i++;
		}
		else if (options[i] == "-h" || options[i] == "--help") {
			PrintUsage();
			return_code = 0;
			return false;
		}
		else if (options[i] == "-i" || options[i] == "--info") {
			if (PrintSystemInformation() == true) {
				return_code = 0;
			}
			else {
				return_code = 1;
			}
			return false;
		}
		else if (options[i] == "-r" || options[i] == "--reset") {
			if (ResetSettings() == true) {
				return_code = 0;
			}
			else {
				return_code = 1;
			}
			return_code = 0;
			return false;
		}
		else {
			cerr << "Unrecognized option: " << options[i] << endl;
			PrintUsage();
			return_code = 1;
			return false;
		}
	}
	
	return true;
} // bool ParseProgramOptions(int32_t &return_code, int32_t argc, char **argv)

// Prints out the usage options (arguments) for running the program (work in progress)
void PrintUsage() {
	cout << "usage: allacrost [options]" << endl;
	cout << "  --check/-c        :: checks all files for integrity" << endl;
	cout << "  --debug/-d <args> :: enables debug statements in specifed sections of the program" << endl;
	cout << "  --help/-h         :: prints this help menu" << endl;
	cout << "  --info/-i         :: prints information about the user's system" << endl;
	cout << "  --reset/-r        :: resets game configuration to use default settings" << endl;
}


// EnableDebugging enables various debugging print statements in different parts of the game
bool EnableDebugging(string vars) {
	if (vars.empty()) {
		cerr << "ERROR: debug specifier string is empty" << endl;
		return false;
	}
	
	// A vector of all the debug arguments
	vector<string> args;
	
	// Find the first non-whitespace character
	uint32 sbegin = 0;
	while (vars[sbegin] == ' ' || vars[sbegin] == '\t') {
		sbegin++;
		if (sbegin >= vars.size()) {
			cerr << "ERROR: no white-space characters in debug specifier string" << endl;
			return false;
		}
	}
	
	// Parse the vars string on white-space characters and fill the args vector
	// TODO: this loop needs to be made more robust to errors
	for (uint32 i = sbegin; i < vars.size(); i++) {
		if (vars[i] == ' ' || vars[i] == '\t') {
			args.push_back(vars.substr(sbegin, i - sbegin));
			sbegin = i + 1;
		}
	}
	args.push_back(vars.substr(sbegin, vars.size() - sbegin));
	
	// Enable all specified debug variables
	for (uint32 i = 0; i < args.size(); i++) {
		if (args[i] == "all") {
			hoa_audio::AUDIO_DEBUG                  = true;
			hoa_battle::BATTLE_DEBUG                = true;
			hoa_boot::BOOT_DEBUG                    = true;
			hoa_data::DATA_DEBUG                    = true;
			hoa_mode_manager::MODE_MANAGER_DEBUG    = true;
			hoa_input::INPUT_DEBUG                  = true;
			hoa_settings::SETTINGS_DEBUG            = true;
			hoa_global::GLOBAL_DEBUG                = true;
			hoa_map::MAP_DEBUG                      = true;
			hoa_menu::MENU_DEBUG                    = true;
			hoa_pause::PAUSE_DEBUG                  = true;
			hoa_quit::QUIT_DEBUG                    = true;
			hoa_scene::SCENE_DEBUG                  = true;
			hoa_utils::UTILS_DEBUG                  = true;
			hoa_video::VIDEO_DEBUG                  = true;
		}
		else if (args[i] == "audio") {
			hoa_audio::AUDIO_DEBUG = true;
		}
		else if (args[i] == "battle") {
			hoa_battle::BATTLE_DEBUG = true;
		}
		else if (args[i] == "boot") {
			hoa_boot::BOOT_DEBUG = true;
		}
		else if (args[i] == "data") {
			hoa_data::DATA_DEBUG = true;
		}
		else if (args[i] == "mode_manager") {
			hoa_mode_manager::MODE_MANAGER_DEBUG = true;
		}
		else if (args[i] == "input") {
			hoa_input::INPUT_DEBUG = true;
		}
		else if (args[i] == "settings") {
			hoa_settings::SETTINGS_DEBUG = true;
		}
		else if (args[i] == "global") {
			hoa_global::GLOBAL_DEBUG = true;
		}
		else if (args[i] == "map") {
			hoa_map::MAP_DEBUG = true;
		}
		else if (args[i] == "menu") {
			hoa_menu::MENU_DEBUG = true;
		}
		else if (args[i] == "pause") {
			hoa_pause::PAUSE_DEBUG = true;
		}
		else if (args[i] == "quit") {
			hoa_quit::QUIT_DEBUG = true;
		}
		else if (args[i] == "scene") {
			hoa_scene::SCENE_DEBUG = true;
		}
		else if (args[i] == "utils") {
			hoa_utils::UTILS_DEBUG = true;
		}
		else if (args[i] == "video") {
			hoa_video::VIDEO_DEBUG = true;
		}
		else {
			cerr << "ERROR: invalid debug argument: " << args[i] << endl;
			return false;
		}
	} // for (uint32 i = 0; i < args.size(); i++)
	
	return true;
} // bool EnableDebugging(string vars)


// Prints version numbers for SDL libraries, video rendering information, and other info
//  about the user's system (work in progress)
bool PrintSystemInformation() {
	cout << "_____Printing system information_____" << endl;

	// Initialize SDL and its subsystems and make sure it shutdowns properly on exit
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
		cerr << "ERROR: Unable to initialize SDL: " << SDL_GetError() << endl;
		return false;
	}
	else {
		cout << "SDL Initialized succesfully." << endl;
	}
	atexit(SDL_Quit);

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

	cout << " *** AUDIO INFORMATION *** " << endl;
	hoa_audio::AudioManager = hoa_audio::GameAudio::Create();
	if (!hoa_audio::AudioManager->Initialize()) {
		cerr << "ERROR: unable to initialize AudioManager" << endl;
		return false;
	}
	else {
		hoa_audio::AudioManager->DEBUG_PrintInfo();
	}
	hoa_audio::GameAudio::Destroy();

	cout << " *** JOYSTICK INFORMATION *** " << endl;

	SDL_Joystick *js_test;
	int32 js_num = SDL_NumJoysticks(); // Get the number of joysticks available
	cout << "SDL has recognized " << js_num << " joysticks on this system." << endl;
	for (int32 i = 0; i < js_num; i++) { // Print out information about each joystick
		js_test = SDL_JoystickOpen(i);
		if (js_test == NULL)
			cout << "ERROR: SDL was unable to open joystick #" << i << endl;
		else {
			cout << "Joystick #" << i << endl;
			cout << "> Name:          " << SDL_JoystickName(i) << endl;
			cout << "> Axes:          " << SDL_JoystickNumAxes(js_test) << endl;
			cout << "> Buttons:       " << SDL_JoystickNumButtons(js_test) << endl;
			cout << "> Trackballs:    " << SDL_JoystickNumBalls(js_test) << endl;
			cout << "> Hat Switches:  " << SDL_JoystickNumHats(js_test) << endl;
			SDL_JoystickClose(js_test);
		}
	}

	cout << "User Event range: [" << SDL_USEREVENT << "," << SDL_NUMEVENTS-1 << "]: "
			 << SDL_NUMEVENTS - SDL_USEREVENT << " distinct user events." << endl;
	return true;
} // bool PrintSystemInformation()


// Resets the game settings to default.
bool ResetSettings() {
	cerr << "This option is not yet implemented." << endl;

	// And then if they say yes, we overwrite the user prefs file with the default prefs file
	return false;
} // bool ResetSettings()



// NOTE: the following function contains operating system dependant code
// Prints any bad file checks (work in progress)
bool CheckFiles() {
	cout << "This option is not yet implemented." << endl;
	return false;
} // bool CheckFiles()

} // namespace hoa_main
