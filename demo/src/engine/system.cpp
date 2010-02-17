////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   system.cpp
*** \author Tyler Olsen, roots@allacrost.org
*** \author Andy Gardner, chopperdave@allacrost.org
*** \brief  Source file for system code management
*** ***************************************************************************/

#ifdef _WIN32
	#include <direct.h>
	#include <stdlib.h>          // defines _MAX_PATH constant
	#define PATH_MAX _MAX_PATH   // redefine _MAX_PATH to be compatible with Darwin's PATH_MAX
#elif defined __MACH__
	#include <unistd.h>
	#include <cstdlib>
#elif defined __linux__
	#include <limits.h>
#endif

// #include "gettext.h"
#include <libintl.h>

#include "system.h"
#include "audio.h"
#include "script.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_script;
using namespace hoa_mode_manager;

template<> hoa_system::SystemEngine* Singleton<hoa_system::SystemEngine>::_singleton_reference = NULL;

namespace hoa_system {

SystemEngine* SystemManager = NULL;
bool SYSTEM_DEBUG = false;



string Translate(const string& text) {
	// gettext is a C library so the gettext() function takes/returns a C-style char* string
	return string(gettext(text.c_str()));
}



ustring UTranslate(const string& text) {
	return MakeUnicodeString(Translate(text));
}

// -----------------------------------------------------------------------------
// SystemTimer Class
// -----------------------------------------------------------------------------

SystemTimer::SystemTimer() :
	_state(SYSTEM_TIMER_INVALID),
	_duration(0),
	_number_loops(0),
	_mode_owner(NULL),
	_time_expired(0),
	_times_completed(0)
{}



SystemTimer::~SystemTimer() {
	// If the timer is still in the invalid state, the SystemManager never received a reference to it.
	if (_state == SYSTEM_TIMER_INVALID)
		return;

	// Remove the reference to this timer object from the SystemManager
	SystemManager->_system_timers.erase(this);
}



void SystemTimer::Initialize(uint32 duration, int32 number_loops, hoa_mode_manager::GameMode* mode_owner) {
	// If the state is invalid, this is the first time that this timer has been initialized and we need to pass it
	// along to the SystemManager
	// CD: Rather than checking state, it would make more sense to check and see if it has an owner
	// If it does, then we don't add it here b/c the owner will update it.  Otherwise, system will have to update it
	// Also, the owner should technically be any object, not just a game mode
	if (_state == SYSTEM_TIMER_INVALID)
	{
		SystemManager->_system_timers.insert(this);
	}

	_duration = duration;
	_number_loops = number_loops;
	_mode_owner = mode_owner;

	_state = SYSTEM_TIMER_INITIAL;
	_time_expired = 0;
	_times_completed = 0;
}



void SystemTimer::SetDuration(uint32 duration) {
	if (IsInitial()) {
		_duration = duration;
	}
	else {
		if (SYSTEM_DEBUG)
			cerr << "SYSTEM WARNING: SystemTimer::SetDuration() was invoked when the timer was not in the "
				<< "initial state. No operation was performed." << endl;
		return;
	}
}



void SystemTimer::SetNumberLoops(int32 number_loops) {
	if (IsInitial()) {
		_number_loops = number_loops;
	}
	else {
		if (SYSTEM_DEBUG)
			cerr << "SYSTEM WARNING: SystemTimer::SetNumberLoops() was invoked when the timer was not in the "
				<< "initial state. No operation was performed." << endl;
		return;
	}
}



void SystemTimer::SetModeOwner(hoa_mode_manager::GameMode* mode_owner) {
	if (IsInitial()) {
		_mode_owner = mode_owner;
	}
	else {
		if (SYSTEM_DEBUG)
			cerr << "SYSTEM WARNING: SystemTimer::SetModeOwner() was invoked when the timer was not in the "
				<< "initial state. No operation was performed." << endl;
		return;
	}
}



void SystemTimer::_UpdateTimer() {
	if (IsRunning() == false)
		return;

	_time_expired += SystemManager->GetUpdateTime();

	if (_time_expired >= _duration) {
		_times_completed++;

		// Checks if infinite looping is enabled
		if (_number_loops < 0) {
			_time_expired -= _duration;
		}
		// Checks if the number of loops have expired
		else if (_times_completed >= static_cast<uint32>(_number_loops)) {
			_time_expired = 0;
			_state = SYSTEM_TIMER_FINISHED;
		}
		// Otherwise, there are still additional loops to complete
		else {
			_time_expired -= _duration;
		}
	}
}

// -----------------------------------------------------------------------------
// SystemEngine Class
// -----------------------------------------------------------------------------

SystemEngine::SystemEngine() {
	if (SYSTEM_DEBUG)
		cout << "SETTINGS: SystemEngine constructor invoked" << endl;

	_not_done = true;
	SetLanguage("en"); //Default language is English
}



SystemEngine::~SystemEngine() {
	if (SYSTEM_DEBUG)
		cout << "SETTINGS: SystemEngine destructor invoked" << endl;
}



bool SystemEngine::SingletonInitialize() {
	// Initialize the gettext library
	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");

	#if defined(_WIN32) || defined(__MACH__)
		char buffer[PATH_MAX];
		// Get the current working directory.
		string cwd(getcwd(buffer, PATH_MAX));
		cwd.append("/translations/");
		bindtextdomain("allacrost", cwd.c_str());
		bind_textdomain_codeset("allacrost", "UTF-8");
		textdomain("allacrost");
	#elif (defined(__linux__) || defined(__FreeBSD__)) && !defined(RELEASE_BUILD)
		// Look for translation files in LOCALEDIR only if they are not available in the
		// current directory.
		if (ifstream("dat/config/settings.lua") == NULL) {
			bindtextdomain(PACKAGE, LOCALEDIR);
			bind_textdomain_codeset(PACKAGE, "UTF-8");
			textdomain(PACKAGE);
		} else {
			char buffer[PATH_MAX];
			// Get the current working directory.
			string cwd(getcwd(buffer, PATH_MAX));
			cwd.append("/txt/");
			bindtextdomain(PACKAGE, cwd.c_str());
			bind_textdomain_codeset(PACKAGE, "UTF-8");
			textdomain(PACKAGE);
		}
	#else
		bindtextdomain(PACKAGE, LOCALEDIR);
		bind_textdomain_codeset(PACKAGE, "UTF-8");
		textdomain(PACKAGE);
	#endif

	// Called here to set the default English language to use nice quote characters.
	SetLanguage("en@quot");

	return true;
}


// Set up the timers before the main game loop begins
void SystemEngine::InitializeTimers() {
	_last_update = SDL_GetTicks();
	_update_time = 1; // Set to non-zero, otherwise bad things may happen...
	_hours_played = 0;
	_minutes_played = 0;
	_seconds_played = 0;
	_milliseconds_played = 0;
	_system_timers.clear();
}



void SystemEngine::UpdateTimers() {
	// ----- (1): Update the update game timer
	uint32 tmp = _last_update;
	_last_update = SDL_GetTicks();
	_update_time = _last_update - tmp;

	// ----- (2): Update the game play timer
	_milliseconds_played += _update_time;
	if (_milliseconds_played >= 1000) {
		_seconds_played += _milliseconds_played / 1000;
		_milliseconds_played = _milliseconds_played % 1000;
		if (_seconds_played >= 60) {
			_minutes_played += _seconds_played / 60;
			_seconds_played = _seconds_played % 60;
			if (_minutes_played >= 60) {
				_hours_played += _minutes_played / 60;
				_minutes_played = _minutes_played % 60;
			}
		}
	}

	// ----- (3): Update all SystemTimer objects
	for (set<SystemTimer*>::iterator i = _system_timers.begin(); i != _system_timers.end(); i++)
		(*i)->_UpdateTimer();
}



void SystemEngine::ExamineSystemTimers() {
	GameMode* active_mode = ModeManager->GetTop();
	GameMode* timer_mode = NULL;

	for (set<SystemTimer*>::iterator i = _system_timers.begin(); i != _system_timers.end(); i++) {
		timer_mode = (*i)->GetModeOwner();
		if (timer_mode == NULL)
			continue;

		if (timer_mode == active_mode)
			(*i)->Run();
		else
			(*i)->Pause();
	}
}



void SystemEngine::SetLanguage(std::string lang) {
	_language = lang;

	/// @TODO, implement a cross-platform wrapper for setenv
	#ifdef _WIN32
		SetEnvironmentVariable("LANGUAGE", _language.c_str());
	#else
		setenv ("LANGUAGE", _language.c_str(), 1);
	#endif
}



void SystemEngine::WaitForThread(Thread * thread) {
#if (THREAD_TYPE == SDL_THREADS)
	SDL_WaitThread(thread, NULL);
#endif
}

Semaphore * SystemEngine::CreateSemaphore(int max) {
#if (THREAD_TYPE == SDL_THREADS)
	return SDL_CreateSemaphore(max);
#endif
}

void SystemEngine::DestroySemaphore(Semaphore * s) {
#if (THREAD_TYPE == SDL_THREADS)
	SDL_DestroySemaphore(s);
#endif
}

void SystemEngine::LockThread(Semaphore * s) {
#if (THREAD_TYPE == SDL_THREADS)
	SDL_SemWait(s);
#endif
}

void SystemEngine::UnlockThread(Semaphore * s) {
#if (THREAD_TYPE == SDL_THREADS)
	SDL_SemPost(s);
#endif
}


} // namespace hoa_system
