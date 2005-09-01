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
using namespace hoa_engine::private_engine;

namespace hoa_engine {

bool ENGINE_DEBUG = false;
SINGLETON_INITIALIZE(GameModeManager);
SINGLETON_INITIALIZE(GameSettings);
SINGLETON_INITIALIZE(GameInput);

// Initialize static members of GameMode class
GameAudio* GameMode::AudioManager = NULL;
GameVideo* GameMode::VideoManager = NULL;
GameData* GameMode::DataManager = NULL;
GameInput* GameMode::InputManager = NULL;
GameModeManager* GameMode::ModeManager = NULL;
GameSettings* GameMode::SettingsManager = NULL;
GameInstance* GameMode::InstanceManager = NULL;

// ****************************************************************************
// ******************************** GameMode **********************************
// ****************************************************************************


// The constructor automatically sets up all the singleton pointers
GameMode::GameMode() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameMode constructor invoked" << endl;
	mode_type = ENGINE_DUMMY_MODE; // This should be replaced by the child class
	AudioManager =    GameAudio::GetReference();
	VideoManager =    GameVideo::GetReference();
	DataManager =     GameData::GetReference();
	InputManager =    GameInput::GetReference();
	ModeManager =     GameModeManager::GetReference();
	SettingsManager = GameSettings::GetReference();
	InstanceManager = GameInstance::GetReference();
}


// The constructor automatically sets up all the singleton pointers
GameMode::GameMode(uint8 mt) {
	if (ENGINE_DEBUG) cout << "ENGINE: GameMode constructor invoked" << endl;
	mode_type = mt;
	AudioManager =    GameAudio::GetReference();
	VideoManager =    GameVideo::GetReference();
	DataManager =     GameData::GetReference();
	InputManager =    GameInput::GetReference();
	ModeManager =     GameModeManager::GetReference();
	SettingsManager = GameSettings::GetReference();
	InstanceManager = GameInstance::GetReference();
}



GameMode::~GameMode() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameMode destructor invoked" << endl;
	// delete coordinate system pointer here
}


// Initializes static singleton pointers for later use
void GameMode::InitializeSingletonPointers() {
	AudioManager =    GameAudio::GetReference();
	VideoManager =    GameVideo::GetReference();
	DataManager =     GameData::GetReference();
	InputManager =    GameInput::GetReference();
	ModeManager =     GameModeManager::GetReference();
	SettingsManager = GameSettings::GetReference();
	InstanceManager = GameInstance::GetReference();
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
bool GameModeManager::Initialize() {
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
	
	return true;
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
		_game_stack.back()->Reset();
		
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

	// Remaining members are initialized in the Initialize() function
}




GameSettings::~GameSettings() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameSettings destructor invoked" << endl;
}


// Makes a call to the data manager for retrieving configured settings
bool GameSettings::Initialize() {
	GameData::GetReference()->LoadGameSettings(); // Initializes remaining data members
	return true;
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
	
	_joyaxis_x_first = true;
	_joyaxis_y_first = true;

	_joystick._js = NULL;
}



GameInput::~GameInput() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameInput destructor invoked" << endl;
	
	// If open, close the joystick before exiting
	if (_joystick._js != NULL) {
		SDL_JoystickClose(_joystick._js);
	}
}


// Initialize singleton pointers and key/joystick systems. Always returns true
bool GameInput::Initialize() {
	// Setup singleton pointers
	_ModeManager = GameModeManager::GetReference();
	_SettingsManager = GameSettings::GetReference();
	_DataManager = GameData::GetReference();
	_VideoManager = GameVideo::GetReference();

	// Loads saved settings to setup the key and joystick configurations
	_DataManager->LoadKeyJoyState(&_key, &_joystick);
	
	// Attempt to initialize and setup the configured joystick
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0) {
		cerr << "ENGINE: ERROR: Failed to initailize SDL joystick subsystem" << endl;
	}
	else {
		if (SDL_NumJoysticks() == 0) { // No joysticks found
			SDL_JoystickEventState(SDL_IGNORE);
			SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
		}
		else { // At least one joystick exists
			SDL_JoystickEventState(SDL_ENABLE);
			// TEMP: need to allow user to specify which joystick to open, if multiple exist
			_joystick._js = SDL_JoystickOpen(_joystick._joy_index);
		}
	}
	return true;
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
			_KeyEventHandler(event.key);
			break;
		}
		else {
			_JoystickEventHandler(event);
			break;
		}
	} // while (SDL_PollEvent(&event)
	
	// Compare the current and previous peak joystick axis values to detect movement events
	if (_joystick._js != NULL) {
		// ******************************* X-Axis Movment *************************************
		// Check for a x-axis boundary change from left to center or right
		if ((_joystick._x_previous_peak <= -JOYAXIS_THRESHOLD) && 
		    (_joystick._x_current_peak > -JOYAXIS_THRESHOLD)) { 
			_left_state = false;
			_left_release = true;
			// Check for a right x-axis boundary change
			if (_joystick._x_current_peak >= JOYAXIS_THRESHOLD) {
				_right_state = true;
				_right_press = true;
			}
		}
		// Check for a x-axis boundary change from right to center or left
		else if ((_joystick._x_previous_peak >= JOYAXIS_THRESHOLD) && 
		         (_joystick._x_current_peak < JOYAXIS_THRESHOLD)) {
			_right_state = false;
			_right_release = true;
			// Check for a left x-axis boundary change
			if (_joystick._x_current_peak <= -JOYAXIS_THRESHOLD) {
				_left_state = true;
				_left_press = true;
			}
		}
		// Check for a x-axis boundary change from center to left
		else if ((_joystick._x_current_peak <= -JOYAXIS_THRESHOLD) && 
		         (_joystick._x_previous_peak > -JOYAXIS_THRESHOLD) && 
		         (_joystick._x_previous_peak < JOYAXIS_THRESHOLD)) {
			_left_state = true;
			_left_press = true;
		}
		// Check for a x-axis boundary change from center to right
		else if ((_joystick._x_current_peak >= JOYAXIS_THRESHOLD) && 
		         (_joystick._x_previous_peak > -JOYAXIS_THRESHOLD) && 
		         (_joystick._x_previous_peak > JOYAXIS_THRESHOLD)) {
			_right_state = true;
			_right_press = true;
		}
		
		// ******************************* Y-Axis Movment *************************************
		// Check for a y-axis boundary change from up to center or down
		if ((_joystick._y_previous_peak <= -JOYAXIS_THRESHOLD) && 
		    (_joystick._y_current_peak > -JOYAXIS_THRESHOLD)) { 
			_up_state = false;
			_up_release = true;
			// Check for a down y-axis boundary change
			if (_joystick._y_current_peak >= JOYAXIS_THRESHOLD) {
				_down_state = true;
				_down_press = true;
			}
		}
		// Check for a y-axis boundary change from down to center or up
		else if ((_joystick._y_previous_peak >= JOYAXIS_THRESHOLD) && 
		         (_joystick._y_current_peak < JOYAXIS_THRESHOLD)) {
			_down_state = false;
			_down_release = true;
			// Check for an up y-axis boundary change
			if (_joystick._y_current_peak <= -JOYAXIS_THRESHOLD) {
				_up_state = true;
				_up_press = true;
			}
		}
		// Check for a y-axis boundary change from center to up
		else if ((_joystick._y_current_peak <= -JOYAXIS_THRESHOLD) && 
		         (_joystick._y_previous_peak > -JOYAXIS_THRESHOLD) && 
		         (_joystick._y_previous_peak < JOYAXIS_THRESHOLD)) {
			_up_state = true;
			_up_press = true;
		}
		// Check for a x-axis boundary change from center to down
		else if ((_joystick._y_current_peak >= JOYAXIS_THRESHOLD) && 
		         (_joystick._y_previous_peak > -JOYAXIS_THRESHOLD) && 
		         (_joystick._y_previous_peak < JOYAXIS_THRESHOLD)) {
			_down_state = true;
			_down_press = true;
		}
		
		// Save previous peak values for the next iteration of event processing
		_joystick._x_previous_peak = _joystick._x_current_peak;
		_joystick._y_previous_peak = _joystick._y_current_peak;
		
		// Reset first axis motion detectors for next event processing loop
		_joyaxis_x_first = true;
		_joyaxis_y_first = true;
	} // (_joystick._js != NULL)
} // void GameInput::EventHandler()



// Handles all keyboard events for the game
void GameInput::_KeyEventHandler(SDL_KeyboardEvent& key_event) {
	if (key_event.type == SDL_KEYDOWN) { // Key was pressed
		if (key_event.keysym.mod & KMOD_CTRL) { // CTRL key was held down
			if (key_event.keysym.sym == SDLK_a) {	
				// Toggle the display of advanced video engine information
				_VideoManager->ToggleAdvancedDisplay();
			}
			else if (key_event.keysym.sym == SDLK_f) {
				// Toggle between full-screen and windowed mode
				if (ENGINE_DEBUG) cout << "Toggle fullscreen!" << endl;
				_VideoManager->ToggleFullscreen();
				_VideoManager->ApplySettings();
				return;
			}
			else if (key_event.keysym.sym == SDLK_q) {
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
			else if (key_event.keysym.sym == SDLK_r) {
				_VideoManager->ToggleFPS();
				return;
			}
			else if (key_event.keysym.sym == SDLK_s) {
				// Take a screenshot of the current game
				_VideoManager->MakeScreenshot();
				return;
			}
			else if (key_event.keysym.sym == SDLK_t) {
				// Display and cycle through the texture sheets
				_VideoManager->DEBUG_NextTexSheet();
				return;
			}
			
			else
				return;
		}

		// Note: a switch-case statement won't work here because Key.up is not an
		// integer value the compiler will whine and cry about it ;_;
		if (key_event.keysym.sym == _key._up) {
			//if (ENGINE_DEBUG) cout << "ENGINE: up key pressed." << endl;
			_up_state = true;
			_up_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key._down) {
			//if (ENGINE_DEBUG) cout << "ENGINE: down key pressed." << endl;
			_down_state = true;
			_down_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key._left) {
			//if (ENGINE_DEBUG) cout << "ENGINE: left key pressed." << endl;
			_left_state = true;
			_left_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key._right) {
			//if (ENGINE_DEBUG) cout << "ENGINE: right key pressed." << endl;
			_right_state = true;
			_right_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key._confirm) {
			//if (ENGINE_DEBUG) cout << "ENGINE: confirm key pressed." << endl;
			_confirm_state = true;
			_confirm_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key._cancel) {
			//if (ENGINE_DEBUG) cout << "ENGINE: cancel key pressed." << endl;
			_cancel_state = true;
			_cancel_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key._menu) {
			//if (ENGINE_DEBUG) cout << "ENGINE: menu key pressed." << endl;
			_menu_state = true;
			_menu_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key._swap) {
			//if (ENGINE_DEBUG) cout << "ENGINE: swap key presed." << endl;
			_swap_state = true;
			_swap_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key._left_select) {
			//if (ENGINE_DEBUG) cout << "ENGINE: left select key pressed." << endl;
			_left_select_state = true;
			_left_select_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key._right_select) {
			//if (ENGINE_DEBUG) cout << "ENGINE: right select key pressed." << endl;
			_right_select_state = true;
			_right_select_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key._pause) {
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
		if (key_event.keysym.mod & KMOD_CTRL) // Don't recognize a key release if ctrl is down
			return;

		if (key_event.keysym.sym == _key._up) {
			//if (ENGINE_DEBUG) cout << "ENGINE: up key released." << endl;
			_up_state = false;
			_up_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key._down) {
			//if (ENGINE_DEBUG) cout << "ENGINE: down key released." << endl;
			_down_state = false;
			_down_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key._left) {
			//if (ENGINE_DEBUG) cout << "ENGINE: left key released." << endl;
			_left_state = false;
			_left_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key._right) {
			//if (ENGINE_DEBUG) cout << "ENGINE: right key released." << endl;
			_right_state = false;
			_right_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key._confirm) {
			//if (ENGINE_DEBUG) cout << "ENGINE: confirm key released." << endl;
			_confirm_state = false;
			_confirm_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key._cancel) {
			//if (ENGINE_DEBUG) cout << "ENGINE: cancel key released." << endl;
			_cancel_state = false;
			_cancel_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key._menu) {
			//if (ENGINE_DEBUG) cout << "ENGINE: menu key released." << endl;
			_menu_state = false;
			_menu_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key._swap) {
			//if (ENGINE_DEBUG) cout << "ENGINE: swap key released." << endl;
			_swap_state = false;
			_swap_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key._left_select) {
			//if (ENGINE_DEBUG) cout << "ENGINE: left select key released." << endl;
			_left_select_state = false;
			_left_select_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key._right_select) {
			//if (ENGINE_DEBUG) cout << "ENGINE: right select key released." << endl;
			_right_select_state = false;
			_right_select_release = true;
			return;
		}
	}
} // void GameInput::_KeyEventHandler(SDL_KeyboardEvent& key_event)


// Handles all joystick events for the game 
void GameInput::_JoystickEventHandler(SDL_Event& js_event) {
	if (js_event.type == SDL_JOYAXISMOTION) {
		if (js_event.jaxis.axis == 0) { // X-axis motion
			if (_joyaxis_x_first == true) {
				_joystick._x_current_peak = js_event.jaxis.value;
				_joyaxis_x_first = false;
			}
			else if (abs(js_event.jaxis.value) > abs(_joystick._x_current_peak)) {
				_joystick._x_current_peak = js_event.jaxis.value;
			} 
		}
		else { // Y-axis motion
			if (_joyaxis_y_first == true) {
				_joystick._y_current_peak = js_event.jaxis.value;
				_joyaxis_y_first = false;
			}
			if (abs(js_event.jaxis.value) > abs(_joystick._y_current_peak)) {
				_joystick._y_current_peak = js_event.jaxis.value;
			} 
		}
	} // if (js_event.type == SDL_JOYAXISMOTION)
	else if (js_event.type == SDL_JOYBUTTONDOWN) {
		if (js_event.jbutton.button == _joystick._confirm) {
			//if (ENGINE_DEBUG) cout << "ENGINE: confirm button pressed." << endl;
			_confirm_state = true;
			_confirm_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick._cancel) {
			//if (ENGINE_DEBUG) cout << "ENGINE: cancel button pressed." << endl;
			_cancel_state = true;
			_cancel_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick._menu) {
			//if (ENGINE_DEBUG) cout << "ENGINE: menu button pressed." << endl;
			_menu_state = true;
			_menu_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick._swap) {
			//if (ENGINE_DEBUG) cout << "ENGINE: swap button presed." << endl;
			_swap_state = true;
			_swap_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick._left_select) {
			//if (ENGINE_DEBUG) cout << "ENGINE: left select button pressed." << endl;
			_left_select_state = true;
			_left_select_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick._right_select) {
			//if (ENGINE_DEBUG) cout << "ENGINE: right select button pressed." << endl;
			_right_select_state = true;
			_right_select_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick._pause) {
			//if (ENGINE_DEBUG) cout << "ENGINE: pause button pressed." << endl;
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
		else if (js_event.jbutton.button == _joystick._quit) {
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
	} // else if (js_event.type == JOYBUTTONDOWN)
	else if (js_event.type == SDL_JOYBUTTONUP) {
		if (js_event.jbutton.button == _joystick._confirm) {
			//if (ENGINE_DEBUG) cout << "ENGINE: confirm button released." << endl;
			_confirm_state = false;
			_confirm_release = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick._cancel) {
			//if (ENGINE_DEBUG) cout << "ENGINE: cancel button released." << endl;
			_cancel_state = false;
			_cancel_release = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick._menu) {
			//if (ENGINE_DEBUG) cout << "ENGINE: menu button released." << endl;
			_menu_state = false;
			_menu_release = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick._swap) {
			//if (ENGINE_DEBUG) cout << "ENGINE: swap button released." << endl;
			_swap_state = false;
			_swap_release = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick._left_select) {
			//if (ENGINE_DEBUG) cout << "ENGINE: left select button released." << endl;
			_left_select_state = false;
			_left_select_release = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick._right_select) {
			//if (ENGINE_DEBUG) cout << "ENGINE: right select button released." << endl;
			_right_select_state = false;
			_right_select_release = true;
			return;
		}
	} // else if (js_event.type == JOYBUTTONUP)
	
	// SDL_JOYBALLMOTION and SDL_JOYHATMOTION are ignored for now. Should we process them?
	
} // void GameInput::_JoystickEventHandler(SDL_Event& js_event)

}// namespace hoa_engine
