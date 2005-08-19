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
#include "quit.h"
#include "pause.h"

using namespace std;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_data;
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
	AudioManager = GameAudio::_GetReference();
	VideoManager = GameVideo::_GetReference();
	DataManager = GameData::_GetReference();
	InputManager = GameInput::_GetReference();
	ModeManager = GameModeManager::_GetReference();
	SettingsManager = GameSettings::_GetReference();
	InstanceManager = GameInstance::_GetReference();
}


// The constructor automatically sets up all the singleton pointers
GameMode::GameMode(unsigned char mt) {
	if (ENGINE_DEBUG) cout << "ENGINE: GameMode constructor invoked" << endl;
	mode_type = mt;
	AudioManager = GameAudio::_GetReference();
	VideoManager = GameVideo::_GetReference();
	DataManager = GameData::_GetReference();
	InputManager = GameInput::_GetReference();
	ModeManager = GameModeManager::_GetReference();
	SettingsManager = GameSettings::_GetReference();
	InstanceManager = GameInstance::_GetReference();
}



GameMode::~GameMode() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameMode destructor invoked" << endl;
}


// ****************************************************************************
// ***************************** GameModeManager ******************************
// ****************************************************************************


// This constructor must be defined for the singleton macro
GameModeManager::GameModeManager() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameModeManager constructor invoked" << endl;
}



// The destructor frees all the modes still on the stack
GameModeManager::~GameModeManager() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameModeManager destructor invoked" << endl;
	while (game_stack.size() != 0) {
		Pop();
	}
}



// Free the top mode on the stack and pop it off
inline void GameModeManager::Pop() {
	if (game_stack.size() > 0) {
		delete game_stack.back();
		game_stack.pop_back();
	}
}



// Pop off all game modes
void GameModeManager::PopAll() {
	while (game_stack.size() > 0) {
		delete game_stack.back();
		game_stack.pop_back();
	}
}


// Push a new game mode onto the stack
inline void GameModeManager::Push(GameMode* gm) {
	game_stack.push_back(gm);
}



// Returns the mode type of the game mode on the top of the stack
inline unsigned char GameModeManager::GetGameType() {
	if (game_stack.empty())
		return ENGINE_DUMMY_MODE;
	else
		return game_stack.back()->mode_type;
}



// Returns a pointer to the game mode that's currently on the top of the stack
GameMode* GameModeManager::GetTop() {
	if (game_stack.empty())
		return NULL;
	else
		return game_stack.back();
}



// Used for debugging purposes ONLY. Prints the contents of the game mode stack.
void GameModeManager::PrintStack() {
	cout << "ENGINE: Printing Game Stack" << endl;
	if ( game_stack.size() == 0 ) {
		cout << "***ERROR: Game stack is empty!" << endl;
		return;
	}

	cout << "***top of stack***" << endl;
	for (int i = (int) game_stack.size() - 1; i >= 0; i--)
		cout << " index: " << i << " type: " << game_stack[i]->mode_type << endl;
	cout << "***bottom of stack***" << endl;
}


// ****************************************************************************
// **************************** GameSettings **********************************
// ****************************************************************************


// The constructor initalize all the data fields inside the GameSettings class
GameSettings::GameSettings() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameSettings constructor invoked" << endl;
	pause_volume_action = ENGINE_SAME_VOLUME;
	not_done = true;
	last_update = 0;
	fps_timer = 0;
	fps_counter = 0;
	fps_rate = 0.0;

	// Remaining members are initialized by GameData->LoadGameSettings(), called in main.cpp
}




GameSettings::~GameSettings() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameSettings destructor invoked" << endl;
}



// Returns the difference between the time now and last_update (in ms) and calculates frame rate
Uint32 GameSettings::UpdateTime() {
	Uint32 tmp;
	tmp = last_update;
	last_update = SDL_GetTicks();
	tmp = last_update - tmp;      // tmp = time now minus time this function was last called
	fps_timer += tmp;
	fps_counter++;

	if (fps_timer >= 1000) { // One second or more has expired, so update the FPS rate
		fps_rate = 1000 * static_cast<float>(fps_counter) / static_cast<float>(fps_timer);
		fps_counter = 0;
		fps_timer = 0;
		cout << "FPS: " << fps_rate << endl;
	}

	return (tmp); // Return the difference between the last update time and the time now
}



// Set up the timers for the first frame draw
void GameSettings::SetTimer() {
	last_update = SDL_GetTicks();
	fps_timer = 0;
}


// ****************************************************************************
// ******************************** GameInput *********************************
// ****************************************************************************


// Initialize class members, call GameData routines for key and joystick initialization
GameInput::GameInput() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameInput constructor invoked" << endl;
	up_state = false;
	up_press = false;
	up_release = false;

	down_state = false;
	down_press = false;
	down_release = false;

	left_state = false;
	left_press = false;
	left_release = false;

	right_state = false;
	right_press = false;
	right_release = false;

	confirm_state = false;
	confirm_press = false;
	confirm_release = false;

	cancel_state = false;
	cancel_press = false;
	cancel_release = false;

	menu_state = false;
	menu_press = false;
	menu_release = false;

	swap_state = false;
	swap_press = false;
	swap_release = false;

	right_select_state = false;
	right_select_press = false;
	right_select_release = false;

	left_select_state = false;
	left_select_press = false;
	left_select_release = false;

	Joystick.js = NULL;

	// Because of these calls, these classes must be created before GameInput
	ModeManager = GameModeManager::_GetReference();
	SettingsManager = GameSettings::_GetReference();
	GameData *DataManager = GameData::_GetReference();

	// CALL HERE: Call DataManager->SomeFn(Key&, Joystick&); to setup KeyState and JoystickState
	DataManager->LoadKeyJoyState(&Key, &Joystick);
}



GameInput::~GameInput() {
	if (ENGINE_DEBUG) cout << "ENGINE: GameInput destructor invoked" << endl;
	if (Joystick.js != NULL) // If its open, close the joystick before exiting
		SDL_JoystickClose(Joystick.js);
}


// Handles all of the event processing for the game.
void GameInput::EventHandler() {
	SDL_Event event;	// Holds the game event

	// Reset all of the press and release flags so they don't get detected twice.
	up_press = false;
	up_release = false;
	down_press = false;
	down_release = false;
	left_press = false;
	left_release = false;
	right_press = false;
	right_release = false;
	confirm_press = false;
	confirm_release = false;
	cancel_press = false;
	cancel_release = false;
	menu_press = false;
	menu_release = false;
	swap_press = false;
	swap_release = false;
	right_select_press = false;
	right_select_release = false;
	left_select_press = false;
	left_select_release = false;

	while (SDL_PollEvent(&event)) { // Loops until we are out of events to process
		if (event.type == SDL_QUIT) {

			// We quit the game without question if we are in BootMode or QuitMode
			if (ModeManager->GetGameType() == ENGINE_BOOT_MODE || ModeManager->GetGameType() == ENGINE_QUIT_MODE) {
				SettingsManager->ExitGame();
			}
			// Otherwise, we push QuitMode onto the stack
			else {
				QuitMode *QM = new QuitMode();
				ModeManager->Push(QM);
			}
			return;
		}
		else if (event.type == SDL_ACTIVEEVENT) {
			// Should we care about Active events?
			if (ENGINE_DEBUG) cout << "Active event" << endl;
			break;
		}
		else if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
			KeyEventHandler(&event.key);
			break;
		}
		else {
			JoystickEventHandler(&event);
			break;
		} // switch (event.type)
	}
}



// Handles all keyboard events for the game
void GameInput::KeyEventHandler(SDL_KeyboardEvent *key_event) {
	if (key_event->type == SDL_KEYDOWN) { // Key was pressed
		if (key_event->keysym.mod & KMOD_CTRL) { // CTRL key was held down
			if (key_event->keysym.sym == SDLK_f) {
				//if (ENGINE_DEBUG) cout << " Toggle Fullscreen!" << endl;
				return;
			}
			else if (key_event->keysym.sym == SDLK_s) {
				//if (ENGINE_DEBUG) cout << " Took Screenshot!" << endl;

				GameVideo *VideoManager = GameVideo::_GetReference();
				VideoManager->MakeScreenshot();
				return;
			}
			else if (key_event->keysym.sym == SDLK_t) {
				// press T to display and cycle through the texture sheets
				GameVideo *VideoManager = GameVideo::_GetReference();
				VideoManager->DEBUG_NextTexSheet();
			}
			else if (key_event->keysym.sym == SDLK_q) {
				// Quit the game without question if we are in BootMode or QuitMode
				if (ModeManager->GetGameType() == ENGINE_BOOT_MODE || ModeManager->GetGameType() == ENGINE_QUIT_MODE) {
					SettingsManager->ExitGame();
				}
				// Otherwise, push QuitMode onto the stack
				else {
					QuitMode *QM = new QuitMode();
					ModeManager->Push(QM);
				}
			}
			else
				return;
		}

		// Note: a switch-case statement won't work here because Key.up is not an
		// integer value the compiler will whine and cry about it ;_;
		if (key_event->keysym.sym == Key.up) {
			//if (ENGINE_DEBUG) cout << "ENGINE: up key pressed." << endl;
			up_state = true;
			up_press = true;
			return;
		}
		else if (key_event->keysym.sym == Key.down) {
			//if (ENGINE_DEBUG) cout << "ENGINE: down key pressed." << endl;
			down_state = true;
			down_press = true;
			return;
		}
		else if (key_event->keysym.sym == Key.left) {
			//if (ENGINE_DEBUG) cout << "ENGINE: left key pressed." << endl;
			left_state = true;
			left_press = true;
			return;
		}
		else if (key_event->keysym.sym == Key.right) {
			//if (ENGINE_DEBUG) cout << "ENGINE: right key pressed." << endl;
			right_state = true;
			right_press = true;
			return;
		}
		else if (key_event->keysym.sym == Key.confirm) {
			//if (ENGINE_DEBUG) cout << "ENGINE: confirm key pressed." << endl;
			confirm_state = true;
			confirm_press = true;
			return;
		}
		else if (key_event->keysym.sym == Key.cancel) {
			//if (ENGINE_DEBUG) cout << "ENGINE: cancel key pressed." << endl;
			cancel_state = true;
			cancel_press = true;
			return;
		}
		else if (key_event->keysym.sym == Key.menu) {
			//if (ENGINE_DEBUG) cout << "ENGINE: menu key pressed." << endl;
			menu_state = true;
			menu_press = true;
			return;
		}
		else if (key_event->keysym.sym == Key.pause) {
			//if (ENGINE_DEBUG) cout << "ENGINE: pause key pressed." << endl;
			// Don't pause if we are in BootMode, QuitMode
			if (ModeManager->GetGameType() == ENGINE_BOOT_MODE || ModeManager->GetGameType() == ENGINE_QUIT_MODE) {
				return;
			}
			// If we are in PauseMode, unpause the game
			else if (ModeManager->GetGameType() == ENGINE_PAUSE_MODE) {
				ModeManager->Pop(); // We're no longer in pause mode, so pop it from the stack
			}
			// Otherwise, we push PauseMode onto the stack
			else {
				PauseMode *PM = new PauseMode();
				ModeManager->Push(PM);
			}
			return;
		}
	}

	else { // Key was released
		if (key_event->keysym.mod & KMOD_CTRL) // Don't recognize a key release if ctrl is down
			return;

		if (key_event->keysym.sym == Key.up) {
			//if (ENGINE_DEBUG) cout << "ENGINE: up key released." << endl;
			up_state = false;
			up_release = true;
			return;
		}
		else if (key_event->keysym.sym == Key.down) {
			//if (ENGINE_DEBUG) cout << "ENGINE: down key released." << endl;
			down_state = false;
			down_release = true;
			return;
		}
		else if (key_event->keysym.sym == Key.left) {
			//if (ENGINE_DEBUG) cout << "ENGINE: left key released." << endl;
			left_state = false;
			left_release = true;
			return;
		}
		else if (key_event->keysym.sym == Key.right) {
			//if (ENGINE_DEBUG) cout << "ENGINE: right key released." << endl;
			right_state = false;
			right_release = true;
			return;
		}
		else if (key_event->keysym.sym == Key.confirm) {
			//if (ENGINE_DEBUG) cout << "ENGINE: confirm key released." << endl;
			confirm_state = false;
			confirm_release = true;
			return;
		}
		else if (key_event->keysym.sym == Key.cancel) {
			//if (ENGINE_DEBUG) cout << "ENGINE: cancel key released." << endl;
			cancel_state = false;
			cancel_release = true;
			return;
		}
		else if (key_event->keysym.sym == Key.menu) {
			//if (ENGINE_DEBUG) cout << "ENGINE: menu key released." << endl;
			menu_state = false;
			menu_release = true;
			return;
		}
	}
}



// Handles all joystick events for the game (not implemented yet)
void GameInput::JoystickEventHandler(SDL_Event *js_event) {
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
