///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file   input.cpp
*** \author Tyler Olsen, roots@allacrost.org
*** \brief  Source file for processing user input
*** **************************************************************************/

#include "input.h"
#include "video.h"
#include "data.h"
#include "mode_manager.h"
#include "settings.h"
#include "quit.h"
#include "pause.h"

using namespace std;
using namespace hoa_video;
using namespace hoa_data;
using namespace hoa_mode_manager;
using namespace hoa_settings;
using namespace hoa_quit;
using namespace hoa_pause;
using namespace hoa_input::private_input;

namespace hoa_input {

GameInput *InputManager = NULL;
bool INPUT_DEBUG = false;
SINGLETON_INITIALIZE(GameInput);


// Initializes class members
GameInput::GameInput() {
	if (INPUT_DEBUG) cout << "INPUT: GameInput constructor invoked" << endl;
	_up_state             = false;
	_up_press             = false;
	_up_release           = false;
	_down_state           = false;
	_down_press           = false;
	_down_release         = false;
	_left_state           = false;
	_left_press           = false;
	_left_release         = false;
	_right_state          = false;
	_right_press          = false;
	_right_release        = false;
	_confirm_state        = false;
	_confirm_press        = false;
	_confirm_release      = false;
	_cancel_state         = false;
	_cancel_press         = false;
	_cancel_release       = false;
	_menu_state           = false;
	_menu_press           = false;
	_menu_release         = false;
	_swap_state           = false;
	_swap_press           = false;
	_swap_release         = false;
	_right_select_state   = false;
	_right_select_press   = false;
	_right_select_release = false;
	_left_select_state    = false;
	_left_select_press    = false;
	_left_select_release  = false;

	_joyaxis_x_first      = true;
	_joyaxis_y_first      = true;
	_joystick.js          = NULL;
}


GameInput::~GameInput() {
	if (INPUT_DEBUG) cout << "INPUT: GameInput destructor invoked" << endl;

	// If a joystick is open, close it before exiting
	if (_joystick.js != NULL) {
		SDL_JoystickClose(_joystick.js);
	}
}

// Initialize singleton pointers and key/joystick systems.
bool GameInput::Initialize() {
	// Initialize the SDL joystick subsystem
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0) {
		cerr << "INPUT ERROR: failed to initailize the SDL joystick subsystem" << endl;
		return false;
	}

	// Loads saved settings to setup the key and joystick configurations
	string in_filename = "dat/config/settings.lua";
	ReadDataDescriptor input_map_data;
	if (input_map_data.OpenFile(in_filename.c_str()) == false) {
		cerr << "INPUT ERROR: failed to open data file for reading: "
		     << in_filename << endl;
		return false;
	}

	input_map_data.OpenTable("key_settings");
	_key.up           = static_cast<SDLKey>(input_map_data.ReadInt("up"));
	_key.down         = static_cast<SDLKey>(input_map_data.ReadInt("down"));
	_key.left         = static_cast<SDLKey>(input_map_data.ReadInt("left"));
	_key.right        = static_cast<SDLKey>(input_map_data.ReadInt("right"));
	_key.confirm      = static_cast<SDLKey>(input_map_data.ReadInt("confirm"));
	_key.cancel       = static_cast<SDLKey>(input_map_data.ReadInt("cancel"));
	_key.menu         = static_cast<SDLKey>(input_map_data.ReadInt("menu"));
	_key.swap         = static_cast<SDLKey>(input_map_data.ReadInt("swap"));
	_key.left_select  = static_cast<SDLKey>(input_map_data.ReadInt("left_select"));
	_key.right_select = static_cast<SDLKey>(input_map_data.ReadInt("right_select"));
	_key.pause        = static_cast<SDLKey>(input_map_data.ReadInt("pause"));
	input_map_data.CloseTable();

	if (input_map_data.GetError() != DATA_NO_ERRORS) {
		cerr << "INPUT ERROR: failure while trying to retrieve key map "
		     << "information from file: " << in_filename << endl;
		return false;
	}

	input_map_data.OpenTable("joystick_settings");
	_joystick.joy_index    = static_cast<int32>(input_map_data.ReadInt("index"));
	_joystick.confirm      = static_cast<uint8>(input_map_data.ReadInt("confirm"));
	_joystick.cancel       = static_cast<uint8>(input_map_data.ReadInt("cancel"));
	_joystick.menu         = static_cast<uint8>(input_map_data.ReadInt("menu"));
	_joystick.swap         = static_cast<uint8>(input_map_data.ReadInt("swap"));
	_joystick.left_select  = static_cast<uint8>(input_map_data.ReadInt("left_select"));
	_joystick.right_select = static_cast<uint8>(input_map_data.ReadInt("right_select"));
	_joystick.pause        = static_cast<uint8>(input_map_data.ReadInt("pause"));
	_joystick.quit         = static_cast<uint8>(input_map_data.ReadInt("quit"));
	input_map_data.CloseTable();

	if (input_map_data.GetError() != DATA_NO_ERRORS) {
		cerr << "INPUT: an error occured while trying to retrieve joystick mapping information "
		     << "from file: " << in_filename << endl;
		return false;
	}
	input_map_data.CloseFile();

	// Attempt to initialize and setup the joystick system
	if (SDL_NumJoysticks() == 0) { // No joysticks found
		SDL_JoystickEventState(SDL_IGNORE);
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	}
	else { // At least one joystick exists
		SDL_JoystickEventState(SDL_ENABLE);
		// TODO: need to allow user to specify which joystick to open, if multiple exist
		_joystick.js = SDL_JoystickOpen(_joystick.joy_index);
	}

	return true;
}


// Loads the default key settings from the lua file and sets them back
bool GameInput::RestoreDefaultKeys() {
	// Load the settings file
	string in_filename = "dat/config/settings.lua";
	ReadDataDescriptor settings_file;
	if (!settings_file.OpenFile(in_filename.c_str())) {
		cerr << "INPUT ERROR: failed to open data file for reading: " << in_filename << endl;
		return false;
	}

	// Load all default keys from the table
	settings_file.OpenTable("key_defaults");
	_key.up           = static_cast<SDLKey>(settings_file.ReadInt("up"));
	_key.down         = static_cast<SDLKey>(settings_file.ReadInt("down"));
	_key.left         = static_cast<SDLKey>(settings_file.ReadInt("left"));
	_key.right        = static_cast<SDLKey>(settings_file.ReadInt("right"));
	_key.confirm      = static_cast<SDLKey>(settings_file.ReadInt("confirm"));
	_key.cancel       = static_cast<SDLKey>(settings_file.ReadInt("cancel"));
	_key.menu         = static_cast<SDLKey>(settings_file.ReadInt("menu"));
	_key.swap         = static_cast<SDLKey>(settings_file.ReadInt("swap"));
	_key.left_select  = static_cast<SDLKey>(settings_file.ReadInt("left_select"));
	_key.right_select = static_cast<SDLKey>(settings_file.ReadInt("right_select"));
	_key.pause        = static_cast<SDLKey>(settings_file.ReadInt("pause"));
	settings_file.CloseTable();

	settings_file.CloseFile();

	return true;
}


// Loads the default joystick settings from the lua file and sets them back
bool GameInput::RestoreDefaultJoyButtons()
{
	// Load the settings file
	string in_filename = "dat/config/settings.lua";
	ReadDataDescriptor settings_file;
	if (!settings_file.OpenFile(in_filename.c_str())) {
		cerr << "INPUT ERROR: failed to open data file for reading: " << in_filename << endl;
		return false;
	}

	// Load all default buttons from the table
	settings_file.OpenTable("joystick_defaults");
	_joystick.confirm      = static_cast<uint8>(settings_file.ReadInt("confirm"));
	_joystick.cancel       = static_cast<uint8>(settings_file.ReadInt("cancel"));
	_joystick.menu         = static_cast<uint8>(settings_file.ReadInt("menu"));
	_joystick.swap         = static_cast<uint8>(settings_file.ReadInt("swap"));
	_joystick.left_select  = static_cast<uint8>(settings_file.ReadInt("left_select"));
	_joystick.right_select = static_cast<uint8>(settings_file.ReadInt("right_select"));
	_joystick.pause        = static_cast<uint8>(settings_file.ReadInt("pause"));
	settings_file.CloseTable();

	settings_file.CloseFile();

	return true;
}


// Handles all of the event processing for the game.
void GameInput::EventHandler() {
	SDL_Event event;	// Holds the game event

	// Reset all of the press and release flags so that they don't get detected twice.
	_up_press             = false;
	_up_release           = false;
	_down_press           = false;
	_down_release         = false;
	_left_press           = false;
	_left_release         = false;
	_right_press          = false;
	_right_release        = false;
	_confirm_press        = false;
	_confirm_release      = false;
	_cancel_press         = false;
	_cancel_release       = false;
	_menu_press           = false;
	_menu_release         = false;
	_swap_press           = false;
	_swap_release         = false;
	_right_select_press   = false;
	_right_select_release = false;
	_left_select_press    = false;
	_left_select_release  = false;

	// Loops until there are no remaining events to process
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {

			// Quit the game without question if the active game mode is BootMode or QuitMode
			if (ModeManager->GetGameType() == MODE_MANAGER_BOOT_MODE
				|| ModeManager->GetGameType() == MODE_MANAGER_QUIT_MODE) {
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
			// if (INPUT_DEBUG) cout << "Active event" << endl;
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
	if (_joystick.js != NULL) {
		// ******************************* X-Axis Movment *************************************
		// Check for a x-axis boundary change from left to center or right
		if ((_joystick.x_previous_peak <= -JOYAXIS_THRESHOLD) &&
		    (_joystick.x_current_peak > -JOYAXIS_THRESHOLD)) {
			_left_state = false;
			_left_release = true;
			// Check for a right x-axis boundary change
			if (_joystick.x_current_peak >= JOYAXIS_THRESHOLD) {
				_right_state = true;
				_right_press = true;
			}
		}
		// Check for a x-axis boundary change from right to center or left
		else if ((_joystick.x_previous_peak >= JOYAXIS_THRESHOLD) &&
		         (_joystick.x_current_peak < JOYAXIS_THRESHOLD)) {
			_right_state = false;
			_right_release = true;
			// Check for a left x-axis boundary change
			if (_joystick.x_current_peak <= -JOYAXIS_THRESHOLD) {
				_left_state = true;
				_left_press = true;
			}
		}
		// Check for a x-axis boundary change from center to left
		else if ((_joystick.x_current_peak <= -JOYAXIS_THRESHOLD) &&
		         (_joystick.x_previous_peak > -JOYAXIS_THRESHOLD) &&
		         (_joystick.x_previous_peak < JOYAXIS_THRESHOLD)) {
			_left_state = true;
			_left_press = true;
		}
		// Check for a x-axis boundary change from center to right
		else if ((_joystick.x_current_peak >= JOYAXIS_THRESHOLD) &&
		         (_joystick.x_previous_peak > -JOYAXIS_THRESHOLD) &&
		         (_joystick.x_previous_peak > JOYAXIS_THRESHOLD)) {
			_right_state = true;
			_right_press = true;
		}

		// ******************************* Y-Axis Movment *************************************
		// Check for a y-axis boundary change from up to center or down
		if ((_joystick.y_previous_peak <= -JOYAXIS_THRESHOLD) &&
		    (_joystick.y_current_peak > -JOYAXIS_THRESHOLD)) {
			_up_state = false;
			_up_release = true;
			// Check for a down y-axis boundary change
			if (_joystick.y_current_peak >= JOYAXIS_THRESHOLD) {
				_down_state = true;
				_down_press = true;
			}
		}
		// Check for a y-axis boundary change from down to center or up
		else if ((_joystick.y_previous_peak >= JOYAXIS_THRESHOLD) &&
		         (_joystick.y_current_peak < JOYAXIS_THRESHOLD)) {
			_down_state = false;
			_down_release = true;
			// Check for an up y-axis boundary change
			if (_joystick.y_current_peak <= -JOYAXIS_THRESHOLD) {
				_up_state = true;
				_up_press = true;
			}
		}
		// Check for a y-axis boundary change from center to up
		else if ((_joystick.y_current_peak <= -JOYAXIS_THRESHOLD) &&
		         (_joystick.y_previous_peak > -JOYAXIS_THRESHOLD) &&
		         (_joystick.y_previous_peak < JOYAXIS_THRESHOLD)) {
			_up_state = true;
			_up_press = true;
		}
		// Check for a x-axis boundary change from center to down
		else if ((_joystick.y_current_peak >= JOYAXIS_THRESHOLD) &&
		         (_joystick.y_previous_peak > -JOYAXIS_THRESHOLD) &&
		         (_joystick.y_previous_peak < JOYAXIS_THRESHOLD)) {
			_down_state = true;
			_down_press = true;
		}

		// Save previous peak values for the next iteration of event processing
		_joystick.x_previous_peak = _joystick.x_current_peak;
		_joystick.y_previous_peak = _joystick.y_current_peak;

		// Reset first axis motion detectors for next event processing loop
		_joyaxis_x_first = true;
		_joyaxis_y_first = true;
	} // (_joystick.js != NULL)
} // void GameInput::EventHandler()



// Handles all keyboard events for the game
void GameInput::_KeyEventHandler(SDL_KeyboardEvent& key_event) {
	if (key_event.type == SDL_KEYDOWN) { // Key was pressed
		if (key_event.keysym.mod & KMOD_CTRL) { // CTRL key was held down
			if (key_event.keysym.sym == SDLK_a) {
				// Toggle the display of advanced video engine information
				VideoManager->ToggleAdvancedDisplay();
			}
			else if (key_event.keysym.sym == SDLK_f) {
				// Toggle between full-screen and windowed mode
				if (INPUT_DEBUG) cout << "Toggle fullscreen!" << endl;
				VideoManager->ToggleFullscreen();
				VideoManager->ApplySettings();
				return;
			}
			else if (key_event.keysym.sym == SDLK_q) {
				// Quit the game without question if the current game mode is BootMode or QuitMode
				if (ModeManager->GetGameType() == MODE_MANAGER_BOOT_MODE
					|| ModeManager->GetGameType() == MODE_MANAGER_QUIT_MODE) {
					SettingsManager->ExitGame();
				}
				// Otherwise, enter QuitMode
				else {
					QuitMode *QM = new QuitMode();
					ModeManager->Push(QM);
				}
			}
			else if (key_event.keysym.sym == SDLK_r) {
				VideoManager->ToggleFPS();
				return;
			}
			else if (key_event.keysym.sym == SDLK_s) {
				// Take a screenshot of the current game
				VideoManager->MakeScreenshot();
				return;
			}
			else if (key_event.keysym.sym == SDLK_t) {
				// Display and cycle through the texture sheets
				VideoManager->DEBUG_NextTexSheet();
				return;
			}

			else
				return;
		}

		// Note: a switch-case statement won't work here because Key.up is not an
		// integer value the compiler will whine and cry about it ;_;
		if (key_event.keysym.sym == _key.up) {
			_up_state = true;
			_up_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key.down) {
			_down_state = true;
			_down_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key.left) {
			_left_state = true;
			_left_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key.right) {
			_right_state = true;
			_right_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key.confirm) {
			_confirm_state = true;
			_confirm_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key.cancel) {
			_cancel_state = true;
			_cancel_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key.menu) {
			_menu_state = true;
			_menu_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key.swap) {
			_swap_state = true;
			_swap_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key.left_select) {
			_left_select_state = true;
			_left_select_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key.right_select) {
			_right_select_state = true;
			_right_select_press = true;
			return;
		}
		else if (key_event.keysym.sym == _key.pause) {
			// Don't pause if the current game mode is BootMode or QuitMode
			if (ModeManager->GetGameType() == MODE_MANAGER_BOOT_MODE
				|| ModeManager->GetGameType() == MODE_MANAGER_QUIT_MODE) {
				return;
			}
			// If the current game mode is PauseMode, unpause the game
			else if (ModeManager->GetGameType() == MODE_MANAGER_PAUSE_MODE) {
				ModeManager->Pop();
			}
			// Otherwise, make PauseMode the active game mode
			else {
				PauseMode *PM = new PauseMode();
				ModeManager->Push(PM);
			}
			return;
		}
	}

	else { // Key was released
		if (key_event.keysym.mod & KMOD_CTRL) // Don't recognize a key release if ctrl is down
			return;

		if (key_event.keysym.sym == _key.up) {
			_up_state = false;
			_up_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.down) {
			_down_state = false;
			_down_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.left) {
			_left_state = false;
			_left_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.right) {
			_right_state = false;
			_right_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.confirm) {
			_confirm_state = false;
			_confirm_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.cancel) {
			_cancel_state = false;
			_cancel_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.menu) {
			_menu_state = false;
			_menu_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.swap) {
			_swap_state = false;
			_swap_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.left_select) {
			_left_select_state = false;
			_left_select_release = true;
			return;
		}
		else if (key_event.keysym.sym == _key.right_select) {
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
				_joystick.x_current_peak = js_event.jaxis.value;
				_joyaxis_x_first = false;
			}
			else if (abs(js_event.jaxis.value) > abs(_joystick.x_current_peak)) {
				_joystick.x_current_peak = js_event.jaxis.value;
			}
		}
		else { // Y-axis motion
			if (_joyaxis_y_first == true) {
				_joystick.y_current_peak = js_event.jaxis.value;
				_joyaxis_y_first = false;
			}
			if (abs(js_event.jaxis.value) > abs(_joystick.y_current_peak)) {
				_joystick.y_current_peak = js_event.jaxis.value;
			}
		}
	} // if (js_event.type == SDL_JOYAXISMOTION)
	else if (js_event.type == SDL_JOYBUTTONDOWN) {
		if (js_event.jbutton.button == _joystick.confirm) {
			_confirm_state = true;
			_confirm_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.cancel) {
			_cancel_state = true;
			_cancel_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.menu) {
			_menu_state = true;
			_menu_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.swap) {
			_swap_state = true;
			_swap_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.left_select) {
			_left_select_state = true;
			_left_select_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.right_select) {
			_right_select_state = true;
			_right_select_press = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.pause) {
			// Don't pause if the current game mode is BootMode or QuitMode
			if (ModeManager->GetGameType() == MODE_MANAGER_BOOT_MODE
				|| ModeManager->GetGameType() == MODE_MANAGER_QUIT_MODE) {
				return;
			}
			// If the current game mode is PauseMode, unpause the game
			else if (ModeManager->GetGameType() == MODE_MANAGER_PAUSE_MODE) {
				ModeManager->Pop();
			}
			// Otherwise, make PauseMode the active game mode
			else {
				PauseMode *PM = new PauseMode();
				ModeManager->Push(PM);
			}
			return;
		}
		else if (js_event.jbutton.button == _joystick.quit) {
			// Quit the game without question if the current game mode is BootMode or QuitMode
			if (ModeManager->GetGameType() == MODE_MANAGER_BOOT_MODE
				|| ModeManager->GetGameType() == MODE_MANAGER_QUIT_MODE) {
				SettingsManager->ExitGame();
			}
			// Otherwise, enter QuitMode
			else {
				QuitMode *QM = new QuitMode();
				ModeManager->Push(QM);
			}
			return;
		}
	} // else if (js_event.type == JOYBUTTONDOWN)
	else if (js_event.type == SDL_JOYBUTTONUP) {
		if (js_event.jbutton.button == _joystick.confirm) {
			_confirm_state = false;
			_confirm_release = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.cancel) {
			_cancel_state = false;
			_cancel_release = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.menu) {
			_menu_state = false;
			_menu_release = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.swap) {
			_swap_state = false;
			_swap_release = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.left_select) {
			_left_select_state = false;
			_left_select_release = true;
			return;
		}
		else if (js_event.jbutton.button == _joystick.right_select) {
			_right_select_state = false;
			_right_select_release = true;
			return;
		}
	} // else if (js_event.type == JOYBUTTONUP)

	// SDL_JOYBALLMOTION and SDL_JOYHATMOTION are ignored for now. Should we process them?

} // void GameInput::_JoystickEventHandler(SDL_Event& js_event)


// Sets a new key over an older one. If the same key is used elsewhere, the older one is removed
void GameInput::_SetNewKey(SDLKey & old_key, SDLKey new_key)
{
	if (_key.up == new_key)  // up key used already
	{
		_key.up = old_key;
		old_key = new_key;
		return;
	}
	if (_key.down == new_key)  // down key used already
	{
		_key.down = old_key;
		old_key = new_key;
		return;
	}
	if (_key.left == new_key)  // left key used already
	{
		_key.left = old_key;
		old_key = new_key;
		return;
	}
	if (_key.right == new_key)  // right key used already
	{
		_key.right = old_key;
		old_key = new_key;
		return;
	}
	if (_key.confirm == new_key)  // confirm key used already
	{
		_key.confirm = old_key;
		old_key = new_key;
		return;
	}
	if (_key.cancel == new_key)  // cancel key used already
	{
		_key.cancel = old_key;
		old_key = new_key;
		return;
	}
	if (_key.menu == new_key)  // menu key used already
	{
		_key.menu = old_key;
		old_key = new_key;
		return;
	}
	if (_key.swap == new_key)  // swap key used already
	{
		_key.swap = old_key;
		old_key = new_key;
		return;
	}
	if (_key.left_select == new_key)  // left_select key used already
	{
		_key.left_select = old_key;
		old_key = new_key;
		return;
	}
	if (_key.right_select == new_key)  // right_select key used already
	{
		_key.right_select = old_key;
		old_key = new_key;
		return;
	}
	if (_key.pause == new_key)  // pause key used already
	{
		_key.pause = old_key;
		old_key = new_key;
		return;
	}

	old_key = new_key; // Otherwise simply overwrite the old value
} // end GameInput::_SetNewKey(SDLKey & old_key, SDLKey new_key)


// Sets a new joystick button over an older one. If the same button is used elsewhere, the older one is removed
void GameInput::_SetNewJoyButton(uint8 & old_button, uint8 new_button)
{
	if (_joystick.confirm == new_button)  // confirm button used already
	{
		_joystick.confirm = old_button;
		old_button = new_button;
		return;
	}
	if (_joystick.cancel == new_button)  // cancel button used already
	{
		_joystick.cancel = old_button;
		old_button = new_button;
		return;
	}
	if (_joystick.menu == new_button)  // menu button used already
	{
		_joystick.menu = old_button;
		old_button = new_button;
		return;
	}
	if (_joystick.swap == new_button)  // swap button used already
	{
		_joystick.swap = old_button;
		old_button = new_button;
		return;
	}
	if (_joystick.left_select == new_button)  // left_select button used already
	{
		_joystick.left_select = old_button;
		old_button = new_button;
		return;
	}
	if (_joystick.right_select == new_button)  // right_select button used already
	{
		_joystick.right_select = old_button;
		old_button = new_button;
		return;
	}
	if (_joystick.pause == new_button)  // pause button used already
	{
		_joystick.pause = old_button;
		old_button = new_button;
		return;
	}

	old_button = new_button; // Otherwise simply overwrite the old value
} // end GameInput::_SetNewJoyButton(uint8 & old_button, uint8 new_button)


} // namespace hoa_input
