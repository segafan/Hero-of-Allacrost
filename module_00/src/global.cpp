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

using namespace std;

namespace hoa_global {

SINGLETON1(GameModeManager);
SINGLETON1(GameSettings);



// ************************** GameModemanager *****************************

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
	pause_audio_on_quit = false;
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
	
	InputStatus.pause_state = false;
	InputStatus.pause_press = false;
	InputStatus.pause_release = false;
	
	InputStatus.joystick.js = NULL;

	// Remaining members are initialized by GameData->LoadGameSettings(), called in loader.cpp
}



// The destructor closes our joysticks
GameSettings::~GameSettings() {
	if (InputStatus.joystick.js != NULL) // If its open, close our joy stick before exiting
		SDL_JoystickClose(InputStatus.joystick.js);
	if (GLOBAL_DEBUG) cerr << "DEBUG: GameSettings destructor invoked" << endl;
}



// Reset press/release flags
void GameSettings::ResetInputFlags() {
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
	InputStatus.pause_press = false;
	InputStatus.pause_release = false;
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
 
}// namespace hoa_global
