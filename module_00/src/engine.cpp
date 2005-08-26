///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    engine.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 12th, 2005
 * \brief   Source file for the core game engine.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include "engine.h"
#include "audio.h"
#include "video.h"
#include "data.h"
#include "global.h"
#include "boot.h"
#include "quit.h"
#include "pause.h"

using namespace std;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_data;
using namespace hoa_boot;
using namespace hoa_quit;
using namespace hoa_pause;
using namespace hoa_global;

namespace hoa_engine {

bool ENGINE_DEBUG = false;
SINGLETON_INITIALIZE(GameModeManager);
SINGLETON_INITIALIZE(GameSettings);
SINGLETON_INITIALIZE(GameInput);


// ****************************************************************************
// ******************************** GameMode **********************************
// ****************************************************************************


// The constructor automatically sets up all the singleton pointers
GameMode::GameMode() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameMode constructor invoked" << endl;
	mode_type = ENGINE_DUMMY_MODE; // This should be replaced by the child class
	AudioManager =    GameAudio::_GetReference();
	VideoManager =    GameVideo::_GetReference();
	DataManager =     GameData::_GetReference();
	InputManager =    GameInput::_GetReference();
	ModeManager =     GameModeManager::_GetReference();
	SettingsManager = GameSettings::_GetReference();
	InstanceManager = GameInstance::_GetReference();
}


// The constructor automatically sets up all the singleton pointers
GameMode::GameMode(uint8 mt) {
	if (ENGINE_DEBUG) cout << "ENGINE: GameMode constructor invoked" << endl;
	mode_type = mt;
	AudioManager =    GameAudio::_GetReference();
	VideoManager =    GameVideo::_GetReference();
	DataManager =     GameData::_GetReference();
	InputManager =    GameInput::_GetReference();
	ModeManager =     GameModeManager::_GetReference();
	SettingsManager = GameSettings::_GetReference();
	InstanceManager = GameInstance::_GetReference();
}



GameMode::~GameMode() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameMode destructor invoked" << endl;
	// delete coordinate system pointer here
}


// ****************************************************************************
// ***************************** GameModeManager ******************************
// ****************************************************************************


// This constructor must be defined for the singleton macro
GameModeManager::GameModeManager() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameModeManager constructor invoked" << endl;
	_pop_count = 0;
	_state_change = false;
}



// The destructor frees all the modes still on the stack
GameModeManager::~GameModeManager() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameModeManager destructor invoked" << endl;
	// Delete any game modes on the stack
	while (_game_stack.size() != 0) {
		delete _game_stack.back();
		_game_stack.pop_back();
	}
	
	// Delete any game modes on the push stack
	while (_push_stack.size() != 0) {
		delete _push_stack.back();
		_push_stack.pop_back();
	}
}


// Empties out the game stack and then initializes it by placing BootMode on the stack top.
void GameModeManager::Initialize() {
	// Delete any game modes on the stack
	while (_game_stack.size() != 0) {
		delete _game_stack.back();
		_game_stack.pop_back();
	}
	
	// Delete any game modes on the push stack
	while (_push_stack.size() != 0) {
		delete _push_stack.back();
		_push_stack.pop_back();
	}
	
	// Reset the pop counter
	_pop_count = 0;
	
	// Create a new BootMode and push it onto the true game stack immediately
	BootMode* BM = new BootMode();
	_game_stack.push_back(BM);
	_state_change = true;
}



// Free the top mode on the stack and pop it off
inline void GameModeManager::Pop() {
	_pop_count++;
	_state_change = true;
}



// Pop off all game modes
void GameModeManager::PopAll() {
	_pop_count = _game_stack.size();
}


// Push a new game mode onto the stack
inline void GameModeManager::Push(GameMode* gm) {
	_push_stack.push_back(gm);
	_state_change = true;
}



// Returns the mode type of the game mode on the top of the stack
inline uint8 GameModeManager::GetGameType() {
	if (_game_stack.empty())
		return ENGINE_DUMMY_MODE;
	else
		return _game_stack.back()->mode_type;
}



// Returns a pointer to the game mode that's currently on the top of the stack
GameMode* GameModeManager::GetTop() {
	if (_game_stack.empty())
		return NULL;
	else
		return _game_stack.back();
}


// Checks if any game modes need to be pushed or popped off the stack, then updates the top stack mode.
void GameModeManager::Update(uint32 time_elapsed) {
	// If a Push() or Pop() function was called, we need to adjust the state of the game stack.
	if (_state_change == true) {
		// Pop however many game modes we need to from the stop of thes tack
		while (_pop_count != 0) {
			if (_game_stack.empty()) { 
				if (ENGINE_DEBUG) {
					cerr << "ENGINE: WARNING: Tried to pop off more game modes than were on the stack!" << endl;
				}
				break; // Exit the loop
			}
			delete _game_stack.back();
			_game_stack.pop_back();
			_pop_count--;
		}
		
		// Push any new game modes onto the true game stack.
		while (_push_stack.size() != 0) {
			_game_stack.push_back(_push_stack.back());
			_push_stack.pop_back();
		}
		
		// Make sure there is a game mode on the stack, otherwise we'll get a segementation fault.
		if (_game_stack.empty()) {
			cerr << "ENGINE: ERROR: Game stack is empty! Now re-initializing boot mode." << endl;
			Initialize();
		}
		
		// Call the newly active game mode's "AtTop()" function to re-initialize class members
		// game_stack.back()->AtTop();
		
		// Reset the state change variable
		_state_change = false;
	}
	
	// Call the Update function on the top stack mode (the active game mode)
	_game_stack.back()->Update(time_elapsed);
}


// Used for debugging purposes ONLY. Prints the contents of the game mode stack.
void GameModeManager::PrintStack() {
	cout << "ENGINE: Printing Game Stack" << endl;
	if (_game_stack.size() == 0) {
		cout << "***ERROR: Game stack is empty!" << endl;
		return;
	}

	cout << "***top of stack***" << endl;
	for (int32 i = static_cast<int32>(_game_stack.size()) - 1; i >= 0; i--)
		cout << " index: " << i << " type: " << _game_stack[i]->mode_type << endl;
	cout << "***bottom of stack***" << endl;
}


// ****************************************************************************
// **************************** GameSettings **********************************
// ****************************************************************************


// The constructor initalize all the data fields inside the GameSettings class
GameSettings::GameSettings() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameSettings constructor invoked" << endl;
	_pause_volume_action = ENGINE_SAME_VOLUME;
	_not_done = true;
	_last_update = 0;
	_fps_timer = 0;
	_fps_counter = 0;
	_fps_rate = 0.0;

	// Remaining members are initialized by GameData->LoadGameSettings(), called in main.cpp
}




GameSettings::~GameSettings() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameSettings destructor invoked" << endl;
}



// Returns the difference between the time now and last_update (in ms) and calculates frame rate
uint32 GameSettings::UpdateTime() {
	uint32 tmp;
	tmp = _last_update;
	_last_update = SDL_GetTicks();
	tmp = _last_update - tmp;      // tmp = time now minus time this function was last called
	_fps_timer += tmp;
	_fps_counter++;

	if (_fps_timer >= 1000) { // One second or more has expired, so update the FPS rate
		_fps_rate = 1000 * static_cast<float>(_fps_counter) / static_cast<float>(_fps_timer);
		_fps_counter = 0;
		_fps_timer = 0;
		// DEFUNCT: cout << "FPS: " << fps_rate << endl; 
		// Need to make a call the the DrawFPS function in GameVideo here.
	}

	return (tmp); // Return the difference between the last update time and the time now
}



// Set up the timers for the first frame draw
void GameSettings::SetTimer() {
	_last_update = SDL_GetTicks();
	_fps_timer = 0;
}


// ****************************************************************************
// ******************************** GameInput *********************************
// ****************************************************************************


// Initialize class members, call GameData routines for key and joystick initialization
GameInput::GameInput() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameInput constructor invoked" << endl;
	_up_state = false;
	_up_press = false;
	_up_release = false;

	_down_state = false;
	_down_press = false;
	_down_release = false;

	_left_state = false;
	_left_press = false;
	_left_release = false;

	_right_state = false;
	_right_press = false;
	_right_release = false;

	_confirm_state = false;
	_confirm_press = false;
	_confirm_release = false;

	_cancel_state = false;
	_cancel_press = false;
	_cancel_release = false;

	_menu_state = false;
	_menu_press = false;
	_menu_release = false;

	_swap_state = false;
	_swap_press = false;
	_swap_release = false;

	_right_select_state = false;
	_right_select_press = false;
	_right_select_release = false;

	_left_select_state = false;
	_left_select_press = false;
	_left_select_release = false;

	_Joystick._js = NULL;

	// Because of these calls, these classes must be created before GameInput
	_ModeManager = GameModeManager::_GetReference();
	_SettingsManager = GameSettings::_GetReference();
	GameData *DataManager = GameData::_GetReference();

	// CALL HERE: Call DataManager->SomeFn(Key&, Joystick&); to setup KeyState and JoystickState
	DataManager->LoadKeyJoyState(&_Key, &_Joystick);
}



GameInput::~GameInput() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameInput destructor invoked" << endl;
	if (_Joystick._js != NULL) // If its open, close the joystick before exiting
		SDL_JoystickClose(_Joystick._js);
}


// Handles all of the event processing for the game.
void GameInput::EventHandler() {
	SDL_Event event;	// Holds the game event

	// Reset all of the press and release flags so they don't get detected twice.
	_up_press = false;
	_up_release = false;
	_down_press = false;
	_down_release = false;
	_left_press = false;
	_left_release = false;
	_right_press = false;
	_right_release = false;
	_confirm_press = false;
	_confirm_release = false;
	_cancel_press = false;
	_cancel_release = false;
	_menu_press = false;
	_menu_release = false;
	_swap_press = false;
	_swap_release = false;
	_right_select_press = false;
	_right_select_release = false;
	_left_select_press = false;
	_left_select_release = false;

	while (SDL_PollEvent(&event)) { // Loops until we are out of events to process
		if (event.type == SDL_QUIT) {

			// We quit the game without question if we are in BootMode or QuitMode
			if (_ModeManager->GetGameType() == ENGINE_BOOT_MODE || _ModeManager->GetGameType() == ENGINE_QUIT_MODE) {
				_SettingsManager->ExitGame();
			}
			// Otherwise, we push QuitMode onto the stack
			else {
				QuitMode *QM = new QuitMode();
				_ModeManager->Push(QM);
			}
			return;
		}
		else if (event.type == SDL_ACTIVEEVENT) {
			// Should we care about Active events?
			if (ENGINE_DEBUG) cout << "Active event" << endl;
			break;
		}
		else if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
			_KeyEventHandler(&event.key);
			break;
		}
		else {
			_JoystickEventHandler(&event);
			break;
		} // switch (event.type)
	}
}



// Handles all keyboard events for the game
void GameInput::_KeyEventHandler(SDL_KeyboardEvent *key_event) {
	if (key_event->type == SDL_KEYDOWN) { // Key was pressed
		if (key_event->keysym.mod & KMOD_CTRL) { // CTRL key was held down
			if (key_event->keysym.sym == SDLK_a) {	
				// Toggle the display of advanced video engine information
				GameVideo *VideoManager = GameVideo::_GetReference();
				VideoManager->ToggleAdvancedDisplay();
			}
			else if (key_event->keysym.sym == SDLK_f) {
				// Toggle between full-screen and windowed mode
				if (ENGINE_DEBUG) cout << "Toggle fullscreen!" << endl;
				GameVideo *VideoManager = GameVideo::_GetReference();
				VideoManager->ToggleFullscreen();
				VideoManager->ApplySettings();
				return;
			}
			else if (key_event->keysym.sym == SDLK_q) {
				// Quit the game without question if we are in BootMode or QuitMode
				if (_ModeManager->GetGameType() == ENGINE_BOOT_MODE || _ModeManager->GetGameType() == ENGINE_QUIT_MODE) {
					_SettingsManager->ExitGame();
				}
				// Otherwise, push QuitMode onto the stack
				else {
					QuitMode *QM = new QuitMode();
					_ModeManager->Push(QM);
				}
			}
			else if (key_event->keysym.sym == SDLK_r) {
				GameVideo *VideoManager = GameVideo::_GetReference();
				VideoManager->ToggleFPS();
			}
			else if (key_event->keysym.sym == SDLK_s) {
				// Take a screenshot of the current game
				GameVideo *VideoManager = GameVideo::_GetReference();
				VideoManager->MakeScreenshot();
				return;
			}
			else if (key_event->keysym.sym == SDLK_t) {
				// Display and cycle through the texture sheets
				GameVideo *VideoManager = GameVideo::_GetReference();
				VideoManager->DEBUG_NextTexSheet();
			}
			
			else
				return;
		}

		// Note: a switch-case statement won't work here because Key.up is not an
		// integer value the compiler will whine and cry about it ;_;
		if (key_event->keysym.sym == _Key._up) {
			//if (ENGINE_DEBUG) cout << "ENGINE: up key pressed." << endl;
			_up_state = true;
			_up_press = true;
			return;
		}
		else if (key_event->keysym.sym == _Key._down) {
			//if (ENGINE_DEBUG) cout << "ENGINE: down key pressed." << endl;
			_down_state = true;
			_down_press = true;
			return;
		}
		else if (key_event->keysym.sym == _Key._left) {
			//if (ENGINE_DEBUG) cout << "ENGINE: left key pressed." << endl;
			_left_state = true;
			_left_press = true;
			return;
		}
		else if (key_event->keysym.sym == _Key._right) {
			//if (ENGINE_DEBUG) cout << "ENGINE: right key pressed." << endl;
			_right_state = true;
			_right_press = true;
			return;
		}
		else if (key_event->keysym.sym == _Key._confirm) {
			//if (ENGINE_DEBUG) cout << "ENGINE: confirm key pressed." << endl;
			_confirm_state = true;
			_confirm_press = true;
			return;
		}
		else if (key_event->keysym.sym == _Key._cancel) {
			//if (ENGINE_DEBUG) cout << "ENGINE: cancel key pressed." << endl;
			_cancel_state = true;
			_cancel_press = true;
			return;
		}
		else if (key_event->keysym.sym == _Key._menu) {
			//if (ENGINE_DEBUG) cout << "ENGINE: menu key pressed." << endl;
			_menu_state = true;
			_menu_press = true;
			return;
		}
		else if (key_event->keysym.sym == _Key._pause) {
			//if (ENGINE_DEBUG) cout << "ENGINE: pause key pressed." << endl;
			// Don't pause if we are in BootMode, QuitMode
			if (_ModeManager->GetGameType() == ENGINE_BOOT_MODE || _ModeManager->GetGameType() == ENGINE_QUIT_MODE) {
				return;
			}
			// If we are in PauseMode, unpause the game
			else if (_ModeManager->GetGameType() == ENGINE_PAUSE_MODE) {
				_ModeManager->Pop(); // We're no longer in pause mode, so pop it from the stack
			}
			// Otherwise, we push PauseMode onto the stack
			else {
				PauseMode *PM = new PauseMode();
				_ModeManager->Push(PM);
			}
			return;
		}
	}

	else { // Key was released
		if (key_event->keysym.mod & KMOD_CTRL) // Don't recognize a key release if ctrl is down
			return;

		if (key_event->keysym.sym == _Key._up) {
			//if (ENGINE_DEBUG) cout << "ENGINE: up key released." << endl;
			_up_state = false;
			_up_release = true;
			return;
		}
		else if (key_event->keysym.sym == _Key._down) {
			//if (ENGINE_DEBUG) cout << "ENGINE: down key released." << endl;
			_down_state = false;
			_down_release = true;
			return;
		}
		else if (key_event->keysym.sym == _Key._left) {
			//if (ENGINE_DEBUG) cout << "ENGINE: left key released." << endl;
			_left_state = false;
			_left_release = true;
			return;
		}
		else if (key_event->keysym.sym == _Key._right) {
			//if (ENGINE_DEBUG) cout << "ENGINE: right key released." << endl;
			_right_state = false;
			_right_release = true;
			return;
		}
		else if (key_event->keysym.sym == _Key._confirm) {
			//if (ENGINE_DEBUG) cout << "ENGINE: confirm key released." << endl;
			_confirm_state = false;
			_confirm_release = true;
			return;
		}
		else if (key_event->keysym.sym == _Key._cancel) {
			//if (ENGINE_DEBUG) cout << "ENGINE: cancel key released." << endl;
			_cancel_state = false;
			_cancel_release = true;
			return;
		}
		else if (key_event->keysym.sym == _Key._menu) {
			//if (ENGINE_DEBUG) cout << "ENGINE: menu key released." << endl;
			_menu_state = false;
			_menu_release = true;
			return;
		}
	}
}



// Handles all joystick events for the game (not implemented yet)
void GameInput::_JoystickEventHandler(SDL_Event *js_event) {
	switch (js_event->type) {
		case SDL_JOYAXISMOTION:
		case SDL_JOYBALLMOTION:
		case SDL_JOYHATMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			break;
	}
}

}// namespace hoa_engine
