/* 
 * global.cpp
 *	Hero of Allacrost global game class code
 *	(C) 2004 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */

#include <iostream>
#include "global.h"
#include "quit.h"
#include "pause.h"

using namespace std;
using namespace hoa_quit;
using namespace hoa_pause;

namespace hoa_global {

SINGLETON_INITIALIZE(GameModeManager);
SINGLETON_INITIALIZE(GameSettings);


// Set the initial mtype value to something bad. The child class should replace it.
GameMode::GameMode() { 
	mtype = dummy_m; 
}


// Do nothing destructor
GameMode::~GameMode() { }


// ************************** GameModeManager *****************************

// Do-nothing constructor
GameModeManager::GameModeManager() { }



// The destructor frees all the modes still on the stack
GameModeManager::~GameModeManager() { 
	while ( game_stack.size() != 0 ) {
		delete game_stack.back();
		game_stack.pop_back();
	}
}



// Free the top mode on the stack and pop it off
void GameModeManager::Pop() { 
	if ( game_stack.size() > 0 ) {
		delete game_stack.back();
		game_stack.pop_back();
	}
}



// Push a new game mode onto the stack
void GameModeManager::Push(GameMode* gm) {
	game_stack.push_back(gm);
}



// Returns the mode type of the game mode on the top of the stack
gmode GameModeManager::GetGameType() { 
	if ( game_stack.empty() ) 
		return dummy_m;		
	else
		return game_stack.back()->mtype; 
}



// Returns a pointer to the game mode that's currently on the top of the stack
GameMode* GameModeManager::GetTop() {
	if ( game_stack.empty() )
		return NULL;
	else
		return game_stack.back();
}



// Used for debugging purposes ONLY. Prints the contents of the game mode stack.
void GameModeManager::PrintStack() {
	cout << "DEBUG: Printing Game Stack" << endl;
	if ( game_stack.size() == 0 ) {
		cout << " Stack is empty. " << endl;
		return;
	}
	
	cout << "***top of stack***" << endl;
	for (int i = game_stack.size() - 1; i >= 0; i--)
		cout << " index: " << i << " type: " << game_stack[i]->mtype << endl;
	cout << "***bottom of stack***" << endl;
}



// **************************** GameSettings *******************************



// The constructor initalize all the data fields inside the GameSettings class
GameSettings::GameSettings() {
	if (GLOBAL_DEBUG) cerr << "DEBUG: GameSettings constructor invoked" << endl;
	paused_vol_type = GLOBAL_SAME_VOLUME_ON_PAUSE;
	not_done = true;
	last_update = 0;
	fps_timer = 0;
	fps_counter = 0;
	fps_rate = 0.0;
	
	InputStatus.up_state = false;
	InputStatus.up_press = false;
	InputStatus.up_release = false;
		
	InputStatus.down_state = false;
	InputStatus.down_press = false;
	InputStatus.down_release = false;
	
	InputStatus.left_state = false;
	InputStatus.left_press = false;
	InputStatus.left_release = false;
	
	InputStatus.right_state = false;
	InputStatus.right_press = false;
	InputStatus.right_release = false;
	
	InputStatus.confirm_state = false;
	InputStatus.confirm_press = false;
	InputStatus.confirm_release = false;
	
	InputStatus.cancel_state = false;
	InputStatus.cancel_press = false;
	InputStatus.cancel_release = false;
	
	InputStatus.menu_state = false;
	InputStatus.menu_press = false;
	InputStatus.menu_release = false;
	
	InputStatus.joystick.js = NULL;

	// Remaining members are initialized by GameData->LoadGameSettings(), called in loader.cpp
}



// The destructor closes our joysticks
GameSettings::~GameSettings() {
	if (InputStatus.joystick.js != NULL) // If its open, close our joy stick before exiting
		SDL_JoystickClose(InputStatus.joystick.js);
	if (GLOBAL_DEBUG) cerr << "DEBUG: GameSettings destructor invoked" << endl;
}



// Returns the difference between the time now and last_update (in ms) and calculates frame rate
Uint32 GameSettings::UpdateTime() {
	Uint32 tmp;
	tmp = last_update;            // tmp = last update time
	last_update = SDL_GetTicks(); // Set the last update time to now
	tmp = last_update - tmp;      // tmp = difference between now and last update
	fps_timer += tmp;             // Increase our fps millisecond timer
	fps_counter++;                // Increase our frame count
	
	if (fps_timer >= 1000) {                                   // One second or more has expired...
		fps_rate = 1000 * (float)fps_counter / (float)fps_timer; // Set our new fps rate
		fps_counter = 0;                                         // Reset our frame counter
		fps_timer = 0;                                           // Reset our fps timer
		cout << "FPS: " << fps_rate << endl;
	}
	
	return (tmp);   // Return the difference between the last update time and the time now
}



// Set up our timers for the first frame draw
void GameSettings::SetTimer() {
	last_update = SDL_GetTicks();
	fps_timer = 0;
}



// Handles all of the event processing for the game.
void GameSettings::EventHandler() {
	SDL_Event event;	// Holds the game event
	
	// Reset all of our press and release flags so they don't get detected twice.
	InputStatus.up_press = false; 
	InputStatus.up_release = false;
	InputStatus.down_press = false;
	InputStatus.down_release = false;
	InputStatus.left_press = false;
	InputStatus.left_release = false; 
	InputStatus.right_press = false;
	InputStatus.right_release = false; 
	InputStatus.confirm_press = false;
	InputStatus.confirm_release = false; 
	InputStatus.cancel_press = false;
	InputStatus.cancel_release = false; 
	InputStatus.menu_press = false;
	InputStatus.menu_release = false;
	InputStatus.swap_press = false;
	InputStatus.swap_release = false;
	InputStatus.rselect_press = false;
	InputStatus.rselect_release = false;
	InputStatus.lselect_press = false;
	InputStatus.lselect_release = false;
	
	while (SDL_PollEvent(&event)) { // Loops until we are out of events to process	 
		if (event.type == SDL_QUIT) {	
			// Get a temporary pointer to the ModeManager singleton
			GameModeManager *ModeManager = GameModeManager::_GetReference(); 
			
			// We quit the game without question if we are in BootMode or QuitMode
			if (ModeManager->GetGameType() == boot_m || ModeManager->GetGameType() == quit_m) {
				not_done = false;
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
			if (GLOBAL_DEBUG) cout << "Active event" << endl;
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
void GameSettings::KeyEventHandler(SDL_KeyboardEvent *key_event) {
	if (key_event->type == SDL_KEYDOWN) { // Key was pressed
		if (key_event->keysym.mod & KMOD_CTRL) {
			if (key_event->keysym.sym == SDLK_f) {
				if (GLOBAL_DEBUG) cout << " Toggle Fullscreen!" << endl;
				return;
			}
			else if (key_event->keysym.sym == SDLK_s) {
				if (GLOBAL_DEBUG) cout << " Took Screenshot!" << endl;
				return;
			}
			else if (key_event->keysym.sym == SDLK_q) {
				// Get a temporary pointer to the ModeManager singleton
				GameModeManager *ModeManager = GameModeManager::_GetReference(); 
				
				// We quit the game without question if we are in BootMode or QuitMode
				if (ModeManager->GetGameType() == boot_m || ModeManager->GetGameType() == quit_m) {
					not_done = false;
				}
				// Otherwise, we push QuitMode onto the stack
				else {
					QuitMode *QM = new QuitMode();
					ModeManager->Push(QM);
				}
			}
			else
				return;
		}
		
		// Note: a switch-case statement won't work here because SettingsManager.InputStatus.key.up
		//	is not an integer value and if you set it as a case the compiler will whine and cry ;_;
		if (key_event->keysym.sym == InputStatus.key.up) {
			if (GLOBAL_DEBUG) cout << " up key pressed." << endl;
			InputStatus.up_state = true;
			InputStatus.up_press = true;
			return;
		}
		else if (key_event->keysym.sym == InputStatus.key.down) {
			if (GLOBAL_DEBUG) cout << " down key pressed." << endl;
			InputStatus.down_state = true;
			InputStatus.down_press = true;
			return;
		}
		else if (key_event->keysym.sym == InputStatus.key.left) {
			if (GLOBAL_DEBUG) cout << " left key pressed." << endl;
			InputStatus.left_state = true;
			InputStatus.left_press = true;
			return;
		}
		else if (key_event->keysym.sym == InputStatus.key.right) {
			if (GLOBAL_DEBUG) cout << " right key pressed." << endl;
			InputStatus.right_state = true;
			InputStatus.right_press = true;
			return;
		}
		else if (key_event->keysym.sym == InputStatus.key.confirm) {
			if (GLOBAL_DEBUG) cout << " confirm key pressed." << endl;
			InputStatus.confirm_state = true;
			InputStatus.confirm_press = true;
			return;
		}
		else if (key_event->keysym.sym == InputStatus.key.cancel) {
			if (GLOBAL_DEBUG) cout << " cancel key pressed." << endl;
			InputStatus.cancel_state = true;
			InputStatus.cancel_press = true;
			return;
		}
		else if (key_event->keysym.sym == InputStatus.key.menu) {
			if (GLOBAL_DEBUG) cout << " menu key pressed." << endl;
			InputStatus.menu_state = true;
			InputStatus.menu_press = true;
			return;
		}
		else if (key_event->keysym.sym == InputStatus.key.pause) {
			if (GLOBAL_DEBUG) cout << " pause key pressed." << endl;
			// Get a temporary pointer to the ModeManager singleton
			GameModeManager *ModeManager = GameModeManager::_GetReference(); 
				
			// Don't pause if we are in BootMode, QuitMode
			if (ModeManager->GetGameType() == boot_m || ModeManager->GetGameType() == quit_m) {
				return;
			}
			// If we are in PauseMode, unpause the game
			else if (ModeManager->GetGameType() == pause_m) {
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
		if (key_event->keysym.mod & KMOD_CTRL) // We don't want to recognize a key release if ctrl is down
			return;
		
		if (key_event->keysym.sym == InputStatus.key.up) {
			if (GLOBAL_DEBUG) cout << " up key released." << endl;
			InputStatus.up_state = false;
			InputStatus.up_release = true;
			return;
		}
		else if (key_event->keysym.sym == InputStatus.key.down) {
			if (GLOBAL_DEBUG) cout << " down key released." << endl;
			InputStatus.down_state = false;
			InputStatus.down_release = true;
			return;
		}
		else if (key_event->keysym.sym == InputStatus.key.left) {
			if (GLOBAL_DEBUG) cout << " left key released." << endl;
			InputStatus.left_state = false;
			InputStatus.left_release = true;
			return;
		}
		else if (key_event->keysym.sym == InputStatus.key.right) {
			if (GLOBAL_DEBUG) cout << " right key released." << endl;
			InputStatus.right_state = false;
			InputStatus.right_release = true;
			return;
		}
		else if (key_event->keysym.sym == InputStatus.key.confirm) {
			if (GLOBAL_DEBUG) cout << " confirm key released." << endl;
			InputStatus.confirm_state = false;
			InputStatus.confirm_release = true;
			return;
		}
		else if (key_event->keysym.sym == InputStatus.key.cancel) {
			if (GLOBAL_DEBUG) cout << " cancel key released." << endl;
			InputStatus.cancel_state = false;
			InputStatus.cancel_release = true;
			return;
		}
		else if (key_event->keysym.sym == InputStatus.key.menu) {
			if (GLOBAL_DEBUG) cout << " menu key released." << endl;
			InputStatus.menu_state = false;
			InputStatus.menu_release = true;
			return;
		}
		// Note: we don't care about pause key releases
	}
}



// Handles all joystick events for the game 
// >> on my to do list. - Tyler, Sept 22nd
void GameSettings::JoystickEventHandler(SDL_Event *js_event) {
	
	switch (js_event->type) {
		case SDL_JOYAXISMOTION:
		case SDL_JOYBALLMOTION:
		case SDL_JOYHATMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			break;
	}
}
 
}// namespace hoa_global
