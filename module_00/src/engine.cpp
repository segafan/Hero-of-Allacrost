/* 
 * engine.cpp
 *	Hero of Allacrost core game engine source code
 *	(C) 2005 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */

#include <iostream>
#include "engine.h"
#include "audio.h"
#include "video.h"
#include "data.h"
#include "quit.h"
#include "pause.h"

using namespace std;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_data;
using namespace hoa_quit;
using namespace hoa_pause;

namespace hoa_engine {

SINGLETON_INITIALIZE(GameModeManager);
SINGLETON_INITIALIZE(GameSettings);
SINGLETON_INITIALIZE(GameInput);


// ****************************************************************************
// ******************************** GameMode **********************************
// ****************************************************************************


// Set the initial mtype value to something bad. The child class should replace it.
GameMode::GameMode() { 
	mtype = dummy_m; 
	AudioManager = GameAudio::_GetReference();
	VideoManager = GameVideo::_GetReference();
	DataManager = GameData::_GetReference();
	InputManager = GameInput::_GetReference();
	ModeManager = GameModeManager::_GetReference();
	SettingsManager = GameSettings::_GetReference();
	
}


// Do nothing destructor
GameMode::~GameMode() { }


// ****************************************************************************
// ***************************** GameModeManager ******************************
// ****************************************************************************


// Do-nothing constructor (necessary for the singleton macros)
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


// ****************************************************************************
// **************************** GameSettings **********************************
// ****************************************************************************


// The constructor initalize all the data fields inside the GameSettings class
GameSettings::GameSettings() {
	if (ENGINE_DEBUG) cerr << "DEBUG: GameSettings constructor invoked" << endl;
	paused_vol_type = ENGINE_SAME_VOLUME_ON_PAUSE;
	not_done = true;
	last_update = 0;
	fps_timer = 0;
	fps_counter = 0;
	fps_rate = 0.0;

	// Remaining members are initialized by GameData->LoadGameSettings(), called in loader.cpp
}



// The destructor closes our joysticks
GameSettings::~GameSettings() {
	if (ENGINE_DEBUG) cerr << "DEBUG: GameSettings destructor invoked" << endl;
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


// ****************************************************************************
// ******************************** GameInput *********************************
// ****************************************************************************


// Initialize class members, call GameData routines for key and joystick initialization
GameInput::GameInput() {
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
	
	rselect_state = false;
	rselect_press = false;
	rselect_release = false;
	
	lselect_state = false;
	lselect_press = false;
	lselect_release = false;
	
	Joystick.js = NULL;
	// CALL HERE: Call DataManager->SomeFn(Key&, Joystick&); to setup KeyState and JoystickState
	GameData *DataManager = GameData::_GetReference();
	DataManager->LoadKeyJoyState(&Key, &Joystick);
	
	// Because of this call, the GameModeManager class must be initialized before GameInput
	ModeManager = GameModeManager::_GetReference();
}



GameInput::~GameInput() {
	if (Joystick.js != NULL) // If its open, close the joystick before exiting
		SDL_JoystickClose(Joystick.js);
}


// Handles all of the event processing for the game.
void GameInput::EventHandler() {
	SDL_Event event;	// Holds the game event
	
	// Reset all of our press and release flags so they don't get detected twice.
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
	rselect_press = false;
	rselect_release = false;
	lselect_press = false;
	lselect_release = false;
	
	while (SDL_PollEvent(&event)) { // Loops until we are out of events to process	 
		if (event.type == SDL_QUIT) {
			
			// We quit the game without question if we are in BootMode or QuitMode
			if (ModeManager->GetGameType() == boot_m || ModeManager->GetGameType() == quit_m) {
				(GameSettings::_GetReference())->ExitGame();
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
				if (ENGINE_DEBUG) cout << " Toggle Fullscreen!" << endl;
				return;
			}
			else if (key_event->keysym.sym == SDLK_s) {
				if (ENGINE_DEBUG) cout << " Took Screenshot!" << endl;
				return;
			}
			else if (key_event->keysym.sym == SDLK_q) {
				// Quit the game without question if we are in BootMode or QuitMode
				if (ModeManager->GetGameType() == boot_m || ModeManager->GetGameType() == quit_m) {
					(GameSettings::_GetReference())->ExitGame();
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
			//if (ENGINE_DEBUG) cout << " up key pressed." << endl;
			up_state = true;
			up_press = true;
			return;
		}
		else if (key_event->keysym.sym == Key.down) {
			//if (ENGINE_DEBUG) cout << " down key pressed." << endl;
			down_state = true;
			down_press = true;
			return;
		}
		else if (key_event->keysym.sym == Key.left) {
			//if (ENGINE_DEBUG) cout << " left key pressed." << endl;
			left_state = true;
			left_press = true;
			return;
		}
		else if (key_event->keysym.sym == Key.right) {
			//if (ENGINE_DEBUG) cout << " right key pressed." << endl;
			right_state = true;
			right_press = true;
			return;
		}
		else if (key_event->keysym.sym == Key.confirm) {
			//if (ENGINE_DEBUG) cout << " confirm key pressed." << endl;
			confirm_state = true;
			confirm_press = true;
			return;
		}
		else if (key_event->keysym.sym == Key.cancel) {
			//if (ENGINE_DEBUG) cout << " cancel key pressed." << endl;
			cancel_state = true;
			cancel_press = true;
			return;
		}
		else if (key_event->keysym.sym == Key.menu) {
			//if (ENGINE_DEBUG) cout << " menu key pressed." << endl;
			menu_state = true;
			menu_press = true;
			return;
		}
		else if (key_event->keysym.sym == Key.pause) {
			//if (ENGINE_DEBUG) cout << " pause key pressed." << endl;
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
		if (key_event->keysym.mod & KMOD_CTRL) // Don't recognize a key release if ctrl is down
			return;
		
		if (key_event->keysym.sym == Key.up) {
			//if (ENGINE_DEBUG) cout << " up key released." << endl;
			up_state = false;
			up_release = true;
			return;
		}
		else if (key_event->keysym.sym == Key.down) {
			//if (ENGINE_DEBUG) cout << " down key released." << endl;
			down_state = false;
			down_release = true;
			return;
		}
		else if (key_event->keysym.sym == Key.left) {
			//if (ENGINE_DEBUG) cout << " left key released." << endl;
			left_state = false;
			left_release = true;
			return;
		}
		else if (key_event->keysym.sym == Key.right) {
			//if (ENGINE_DEBUG) cout << " right key released." << endl;
			right_state = false;
			right_release = true;
			return;
		}
		else if (key_event->keysym.sym == Key.confirm) {
			//if (ENGINE_DEBUG) cout << " confirm key released." << endl;
			confirm_state = false;
			confirm_release = true;
			return;
		}
		else if (key_event->keysym.sym == Key.cancel) {
			//if (ENGINE_DEBUG) cout << " cancel key released." << endl;
			cancel_state = false;
			cancel_release = true;
			return;
		}
		else if (key_event->keysym.sym == Key.menu) {
			//if (ENGINE_DEBUG) cout << " menu key released." << endl;
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
