///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file   mode_manager.cpp
*** \author Tyler Olsen, roots@allacrost.org
*** \brief  Source file for managing user settings
*** **************************************************************************/

#include "mode_manager.h"
#include "boot.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_boot;


template<> hoa_mode_manager::GameModeManager* Singleton<hoa_mode_manager::GameModeManager>::_singleton_reference = NULL;

namespace hoa_mode_manager {

GameModeManager* ModeManager = NULL;
bool MODE_MANAGER_DEBUG = false;

// ****************************************************************************
// ***** GameMode class
// ****************************************************************************

GameMode::GameMode() {
	if (MODE_MANAGER_DEBUG) cout << "MODE MANAGER: GameMode constructor invoked" << endl;
	
	// The value of this member should later be replaced by the child class
	mode_type = MODE_MANAGER_DUMMY_MODE; 
}

GameMode::GameMode(uint8 mt) {
	if (MODE_MANAGER_DEBUG) cout << "MODE MANAGER: GameMode constructor invoked" << endl;
	mode_type = mt;
}

GameMode::~GameMode() {
	if (MODE_MANAGER_DEBUG) cout << "MODE MANAGER: GameMode destructor invoked" << endl;
}

// ****************************************************************************
// ***** GameModeManager class
// ****************************************************************************

// This constructor must be defined for the singleton macro
GameModeManager::GameModeManager() {
	if (MODE_MANAGER_DEBUG) cout << "MODE MANAGER: GameModeManager constructor invoked" << endl;
	_pop_count = 0;
	_state_change = false;
}


// The destructor frees all the modes still on the stack
GameModeManager::~GameModeManager() {
	if (MODE_MANAGER_DEBUG) cout << "MODE MANAGER: GameModeManager destructor invoked" << endl;
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
bool GameModeManager::SingletonInitialize() {
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
void GameModeManager::Pop() {
	_pop_count++;
	_state_change = true;
}



// Pop off all game modes
void GameModeManager::PopAll() {
	_pop_count = static_cast<uint32>(_game_stack.size());
}


// Push a new game mode onto the stack
void GameModeManager::Push(GameMode* gm) {
	_push_stack.push_back(gm);
	_state_change = true;
}


// Returns the mode type of the game mode on the top of the stack
uint8 GameModeManager::GetGameType() {
	if (_game_stack.empty())
		return MODE_MANAGER_DUMMY_MODE;
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
void GameModeManager::Update() {
	// If a Push() or Pop() function was called, we need to adjust the state of the game stack.
	if (_state_change == true) {
		// Pop however many game modes we need to from the stop of the stack
		while (_pop_count != 0) {
			if (_game_stack.empty()) {
				if (MODE_MANAGER_DEBUG) {
					cerr << "MODE MANAGER WARNING: Tried to pop off more game modes than were on the stack!" << endl;
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
			cerr << "MODE MANAGER ERROR: Game stack is empty! Now re-initializing boot mode." << endl;
			SingletonInitialize();
		}

		// Call the newly active game mode's Reset() function to re-initialize the game mode
		_game_stack.back()->Reset();

		// Reset the state change variable
		_state_change = false;
	} // if (_state_change == true)

	// Call the Update function on the top stack mode (the active game mode)
	_game_stack.back()->Update();
}


// Checks if any game modes need to be pushed or popped off the stack, then updates the top stack mode.
void GameModeManager::Draw() {
	if (_game_stack.size() == 0) {
		return;
	}

	_game_stack.back()->Draw();
}


// Used for debugging purposes ONLY. Prints the contents of the game mode stack.
void GameModeManager::DEBUG_PrintStack() {
	cout << "MODE MANAGER DEBUG: Printing Game Stack" << endl;
	if (_game_stack.size() == 0) {
		cout << "***Game stack is empty!" << endl;
		return;
	}

	cout << "***top of stack***" << endl;
	for (int32 i = static_cast<int32>(_game_stack.size()) - 1; i >= 0; i--)
		cout << " index: " << i << " type: " << _game_stack[i]->mode_type << endl;
	cout << "***bottom of stack***" << endl;
}

} // namespace hoa_mode_manager
