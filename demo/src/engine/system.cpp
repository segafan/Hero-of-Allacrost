////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   system.cpp
*** \author Tyler Olsen, roots@allacrost.org
*** \brief  Source file for system code management
*** ***************************************************************************/

#include "system.h"
#include "data.h"
#include "audio.h"

using namespace std;
using namespace hoa_script;
using namespace hoa_audio;
using namespace hoa_system::private_system;

namespace hoa_system {

GameSystem *SystemManager = NULL;
bool SYSTEM_DEBUG = false;
SINGLETON_INITIALIZE(GameSystem);


// The constructor initalize all the data fields inside the GameSystem class
GameSystem::GameSystem() {
	if (SYSTEM_DEBUG) cout << "SETTINGS: GameSystem constructor invoked" << endl;
	
	_not_done = true;
	_language = "en"; // Default language is English
}



GameSystem::~GameSystem() {
	if (SYSTEM_DEBUG) cout << "SETTINGS: GameSystem destructor invoked" << endl;
}


// Makes a call to the data manager for retrieving configured settings
bool GameSystem::SingletonInitialize() {
	ScriptDescriptor settings_data;

	if (!settings_data.OpenFile("dat/config/settings.lua", READ)) {
		cout << "SYSTEM ERROR: failed to load settings from data file" << endl;
		return false;
	}

	settings_data.OpenTable("video_settings");
// 	SetFullScreen(settings_data.ReadBool("full_screen"));
	settings_data.CloseTable();

	settings_data.OpenTable("audio_settings");
	AudioManager->SetMusicVolume(settings_data.ReadFloat("music_vol"));
	AudioManager->SetSoundVolume(settings_data.ReadFloat("sound_vol"));
	settings_data.CloseTable();

	if (settings_data.GetError() != DATA_NO_ERRORS) {
		cout << "SETTINGS WARNING: some error occured during read operations from data file" << endl;
	}
	settings_data.CloseFile();
	return true;
}


// Set up the timers before the main game loop begins
void GameSystem::InitializeTimers() {
	_last_update = SDL_GetTicks();
	_update_time = 1; // Must be non-zero, otherwise bad things will happen...
	_hours_played = 0;
	_minutes_played = 0;
	_seconds_played = 0;
	_milliseconds_played = 0;
}


// Returns the difference between the time now and last_update (in ms) and calculates frame rate
void GameSystem::UpdateTimers() {
	uint32 tmp;

	tmp = _last_update;
	_last_update = SDL_GetTicks();
	_update_time = _last_update - tmp;

	_milliseconds_played += _update_time;
	if (_milliseconds_played > 1000) {
		_milliseconds_played -= 1000;
		_seconds_played += 1;
		if (_seconds_played > 60) {
			_seconds_played -= 60;
			_minutes_played += 1;
			if (_minutes_played > 60) {
				_minutes_played -= 60;
				_hours_played += 1;
			}
		}
	}
}



void GameSystem::SetLanguage(std::string lang) {
	// A 2 character string is the only allowable argument
	if (lang.size() != 2) {
		return;
	}
// 	for (uint32 i = 0; i < SUPPORTED_LANGUAGES.size(); i++) {
// 		if (lang == SUPPORTED_LANGUAGES[i]) {
// 			_language = lang;
// 			return;
// 		}
// 	}
// 	
// 	cerr << "SETTINGS ERROR: attempt to set unsupported language \"" << lang << "\" failed" << endl;
	_language = lang;
}

} // namespace hoa_settings
