///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file   input.h
*** \author Tyler Olsen, roots@allacrost.org
*** \brief  Header file for processing user input
***
*** \todo Currently joystick hat and ball events are not handled by this input
*** event manager. I may add support for them later if it is found necessary.
***
*** \todo Joystick processing needs more testing. It has only bee tested with
*** one gamepad (Logitech Wingman Extreme). Particularly, I'm not sure if I 
*** chose an adequate value for JOYSTICK_THRESHOLD that will be suitable for 
*** all gamepads/joysticks.
***
*** \todo This engine is missing the following functionality:
***   - Access calls for re-defining what keys/joystick buttons are mapped to
***     which input events
***   - The ability to save the current key/joystick maps to a file, or
***     overwrite the mapping in a current file.
***   - The ability to handle systems with multiple joysticks and to allow the
***     player to choose which joystick to use
***   - The ability to allow the player to disable the joystick subsystem and
***     normal keyboard commands. At least one input (keyboard or joystick)
***     should be enabled at any given time. Keyboard meta-commands (Ctrl+key)
***     are exempt from this rule and will never be disabled
*** **************************************************************************/

#ifndef __INPUT_HEADER__
#define __INPUT_HEADER__

#include "utils.h"
#include "defs.h"

//! All calls to the input engine are wrapped in this namespace.
namespace hoa_input {

//! The singleton pointer responsible for handling and updating user input.
extern GameInput *InputManager;

//! Determines whether the code in the hoa_input namespace should print debug statements or not.
extern bool INPUT_DEBUG;


//! An internal namespace to be used only within the input code.
namespace private_input {

//! The threshold value we use to partition the range of joystick values into on and off
const int16 JOYAXIS_THRESHOLD = 8192;

/** ***************************************************************************
*** \brief Retains information about the user-defined key settings.
***
*** This class is simply a container for various SDLKey structures that represent
*** the game's input keys.
*** **************************************************************************/
class KeyState {
public:
	/** \name Generic key names
	*** \brief Each member holds the actual keyboard key that corresponds to the named key event.
	**/
	//@{
	SDLKey up;
	SDLKey down;
	SDLKey left;
	SDLKey right;
	SDLKey confirm;
	SDLKey cancel;
	SDLKey menu;
	SDLKey swap;
	SDLKey left_select;
	SDLKey right_select;
	SDLKey pause;
	//@}
}; // class KeyState

/** ***************************************************************************
*** \brief Retains information about the user-defined joystick settings.
***
*** This class is simply a container for various SDL structures that represent
*** the joystick input. Because joystick axis movement is not a simple "on/off"
*** state as opposed to keys, we need a little extra logic so that it can be
*** represented as such. In the range of possible joystick values (-32768 to 32767),
*** we section off the region into thirds and label any crossing of these 'boundaries'
*** as state changes.
*** **************************************************************************/
class JoystickState {
public:
	//! A pointer to the active joystick.
	SDL_Joystick *js;

	//! An index to the SDL joystick which should be made active.
	int32 joy_index;

	//! \name Generic button names.
	/**
	***
	**/
	//@{
	//! \brief Each member retains the index that refers to the joystick button registered to the event.
	uint8 confirm;
	uint8 cancel;
	uint8 menu;
	uint8 swap;
	uint8 left_select;
	uint8 right_select;
	uint8 pause;
	uint8 quit;
	//@}

	//! \name Previous Peak Joystick Axis Values
	/**
	***
	**/
	//@{
	//! \brief These variables retain the previous peak value of each joystick axis.
	int16 x_previous_peak;
	int16 y_previous_peak;
	//@}

	//! \name Current Peak Joystick Axis Values
	/**
	***
	**/
	//@{
	//! \brief These variables retain the current peak value of each joystick axis.
	int16 x_current_peak;
	int16 y_current_peak;
	//@}

	friend class GameInput;
}; // class JoystickState

} // namespace private_input

/** ***************************************************************************
*** \brief Processes and manages all user input events.
***
*** The way this class operates is by first retaining the user-defined keyboard
*** and joystick settings. The EventHandler() function is called once every
*** iteration of the main game loop to process all events that have accumulated
*** in the SDL input queue. Three boolean varaiables for each type of input event
*** are maintained to represent the state of each input:
***
*** - state   :: for when a key/button is being held down
*** - press   :: for when a key/button was previously untouched, but has since been pressed
*** - release :: for when a key/button was previously held down, but has since been released
***
*** The names of the primary game input events and their purposes are listed below:
***
*** - up           :: Moves a cursor/sprite upwards
*** - down         :: Moves a cursor/sprite downwards
*** - left         :: Moves a cursor/sprite left
*** - right        :: Moves a cursor/sprite right
*** - confirm      :: Confirms a menu selection or command
*** - cancel       :: Cancels a menu selection or command
*** - menu         :: Opens up a menu
*** - swap         :: Used for swapping selected items or characters
*** - left_select  :: Selecting multiple items or friendlys
*** - right_select :: Selecting multiple items or foes
*** - pause        :: Pauses the game
***
*** There are also other events and meta-key combination events that are handled within
*** this class itself:
***
*** - Ctrl+F     :: toggles the game between running in windowed and full screen mode
*** - Ctrl+Q     :: brings up the quit menu/quits the game
*** - Ctrl+S     :: saves a screenshot of the current screen
*** - Quit Event :: same as Ctrl+Q, this happens when the user tries to close the game window
***
*** \note This class is a singleton.
*** 
*** \note Pause and quit events are handled automatically in this class, so there are no 
*** means available for you to detect nor handle the. However, you can determine what happens
*** to the audio on a pause or quit event (see the file settings.h for information on that).
***
*** \note Keep in mind that these events are \b not mutually exclusive (an up press and a down 
*** press may be registered at the same time). This class does not attempt to give one
*** event precedence over the other, except in the case of pause and quit events. Therefore,
*** your code you should deal with the problem of not having mutual exclusive events directly.
***
*** \note Because this class will be used quite often to check the status of the various
*** booleans, encapsulation has been used so that one can't accidentally change the value 
*** of one of the members and introduce hard-to-find bugs in the code. 
*** (eg. `if (up_state = true)` instead of `if (up_state == true)`.
***
*** \note In the end, all you really need to know about this class are the
*** member access functions in the public section of this class (its not that hard).
*** **************************************************************************/
class GameInput {
private:
	SINGLETON_DECLARE(GameInput);
	
	//! Holds the current user-defined key settings
	private_input::KeyState _key;
	//! Holds the current user-defined joystick settings
	private_input::JoystickState _joystick;


	//! Any key (or joystick button) pressed
	bool _any_key_press;

	//! Any key released
	bool _any_key_release;

	/** \name  Input State Members
	*** \brief Retain whether an input key/button is currently being held down
	**/	
	//@{
	bool _up_state;
	bool _down_state;
	bool _left_state;
	bool _right_state;
	bool _confirm_state;
	bool _cancel_state;
	bool _menu_state;
	bool _swap_state;
	bool _left_select_state;
	bool _right_select_state;
	//@}

	/** \name  Input Press Members
	*** \brief Retain whether an input key/button was just pressed 
	**/
	//@{
	bool _up_press;
	bool _down_press;
	bool _left_press;
	bool _right_press;
	bool _confirm_press;
	bool _cancel_press;
	bool _menu_press;
	bool _swap_press;
	bool _left_select_press;
	bool _right_select_press;
	//@}

	/** \name  Input Release Members
	*** \brief Retain whether an input key/button was just released
	**/
	//@{
	bool _up_release;
	bool _down_release;
	bool _left_release;
	bool _right_release;
	bool _confirm_release;
	bool _cancel_release;
	bool _menu_release;
	bool _swap_release;
	bool _left_select_release;
	bool _right_select_release;
	//@}

	/** \name  First Joystick Axis Motion
	*** \brief Retains whether a joystick axis event has already occured or not
	**/
	//@{
	bool _joyaxis_x_first;
	bool _joyaxis_y_first;
	//@}

	/** \brief Processes all keyboard input event
	*** \param key_event The event to process
	**/
	void _KeyEventHandler(SDL_KeyboardEvent& key_event);
	/** \brief Processes all joystick input event
	*** \param js_event The event to process
	**/
	void _JoystickEventHandler(SDL_Event& js_event);

	/** \brief Sets a new key over an older one. If the same key is used elsewhere, the older one is removed
	*** \param old_key key to be replaced (_key.up for example)
	*** \param new_key key to replace the old value
	**/
	void _SetNewKey(SDLKey & old_key, SDLKey new_key);

	/** \brief Sets a new joystick button over an older one. If the same button is used elsewhere, the older one is removed
	*** \param old_button to be replaced (_joystick.confirm for example)
	*** \param new_button button to replace the old value
	**/
	void _SetNewJoyButton(uint8 & old_button, uint8 new_button);
public:
	SINGLETON_METHODS(GameInput);

	/** \brief Loads the default key settings from the lua file and sets them back
	*** \return Returns false if the settings file couldn't be read
	**/
	bool RestoreDefaultKeys();

	/** \brief Loads the default joystick settings from the lua file and sets them back
	*** \return Returns false if the settings file couldn't be read
	**/
	bool RestoreDefaultJoyButtons();

	/** \brief Checks if any keyboard key or joystick button is pressed
	*** \return True if any key/button is pressed
	**/
	bool AnyKeyPress();

	/** \brief Checks if any keyboard key or joystick button is released
	*** \return True if any key/button is released
	**/
	bool AnyKeyRelease();

	void TogglePause();
	/** \brief Examines the SDL queue for all user input events and calls appropriate sub-functions.
	***
	*** This function handles all the meta keyboard events (events when a modifier key like Ctrl or
	*** Alt is held down) and all miscellaneous user input events (like clicking on the window button
	*** to quit the game). Any keyboard or joystick events that occur are passed to the KeyEventHandler()
	*** and JoystickEventHandler() functions.
	***
	*** \note EventHandler() should only be called in the main game loop. Do \b not call it anywhere else.
	**/
	void EventHandler();

	/** \name   Input state member access functions
	*** \return True if the input event key/button is being held down
	**/
	//@{
	bool UpState() 
		{ return _up_state; }
	bool DownState() 
		{ return _down_state; }
	bool LeftState() 
		{ return _left_state; }
	bool RightState() 
		{ return _right_state; }
	bool ConfirmState() 
		{ return _confirm_state; }
	bool CancelState() 
		{ return _cancel_state; }
	bool MenuState() 
		{ return _menu_state; }
	bool SwapState() 
		{ return _swap_state; }
	bool LeftSelectState() 
		{ return _left_select_state; }
	bool RightSelectState() 
		{ return _right_select_state; }
	//@}

	/** \name   Input press member access functions
	*** \return True if the input event key/button has just been pressed
	**/
	//@{
	bool UpPress() 
		{ return _up_press; }
	bool DownPress() 
		{ return _down_press; }
	bool LeftPress() 
		{ return _left_press; }
	bool RightPress() 
		{ return _right_press; }
	bool ConfirmPress() 
		{ return _confirm_press; }
	bool CancelPress() 
		{ return _cancel_press; }
	bool MenuPress() 
		{ return _menu_press; }
	bool SwapPress() 
		{ return _swap_press; }
	bool LeftSelectPress() 
		{ return _left_select_press; }
	bool RightSelectPress() 
		{ return _right_select_press; }
	//@}

	/** \name   Input release member access functions
	*** \return True if the input event key/button has just been released
	**/
	//@{
	bool UpRelease() 
		{ return _up_release; }
	bool DownRelease() 
		{ return _down_release; }
	bool LeftRelease() 
		{ return _left_release; }
	bool RightRelease() 
		{ return _right_release; }
	bool ConfirmRelease() 
		{ return _confirm_release; }
	bool CancelRelease() 
		{ return _cancel_release; }
	bool MenuRelease() 
		{ return _menu_release; }
	bool SwapRelease() 
		{ return _swap_release; }
	bool LeftSelectRelease() 
		{ return _left_select_release; }
	bool RightSelectRelease() 
		{ return _right_select_release; }
	//@}

	/** \name   Currently set key names' access functions
	*** \return SDLKey number according to the currently set key
	**/
	//@{
	std::string GetUpKeyName() const { return SDL_GetKeyName(_key.up); }
	std::string GetDownKeyName() const { return SDL_GetKeyName(_key.down); }
	std::string GetLeftKeyName() const { return SDL_GetKeyName(_key.left); }
	std::string GetRightKeyName() const { return SDL_GetKeyName(_key.right); }
	std::string GetConfirmKeyName() const { return SDL_GetKeyName(_key.confirm); }
	std::string GetCancelKeyName() const { return SDL_GetKeyName(_key.cancel); }
	std::string GetMenuKeyName() const { return SDL_GetKeyName(_key.menu); }
	std::string GetSwapKeyName() const { return SDL_GetKeyName(_key.swap); }
	std::string GetLeftSelectKeyName() const { return SDL_GetKeyName(_key.left_select); }
	std::string GetRightSelectKeyName() const { return SDL_GetKeyName(_key.right_select); }
	std::string GetPauseKeyName() const { return SDL_GetKeyName(_key.pause); }
	//@}

	/** \name   Currently set joystick buttons' access functions
	*** \return Joystick button number for the action
	**/
	//@{
	std::string GetConfirmJoy() const { return std::string("Button ") + hoa_utils::NumberToString(_joystick.confirm); }
	std::string GetCancelJoy() const { return std::string("Button ") + hoa_utils::NumberToString(_joystick.cancel); }
	std::string GetMenuJoy() const { return std::string("Button ") + hoa_utils::NumberToString(_joystick.menu); }
	std::string GetSwapJoy() const { return std::string("Button ") + hoa_utils::NumberToString(_joystick.swap); }
	std::string GetLeftSelectJoy() const { return std::string("Button ") + hoa_utils::NumberToString(_joystick.left_select); }
	std::string GetRightSelectJoy() const { return std::string("Button ") + hoa_utils::NumberToString(_joystick.right_select); }
	std::string GetPauseJoy() const { return std::string("Button ") + hoa_utils::NumberToString(_joystick.pause); }
	//@}	

	/** \name   Sets new keymappings
	*** \param	key New key for the action
	**/
	//@{
	void SetUpKey(const SDLKey & key) { _SetNewKey(_key.up, key); }
	void SetDownKey(const SDLKey & key) { _SetNewKey(_key.down, key); }
	void SetLeftKey(const SDLKey & key) { _SetNewKey(_key.left, key); }
	void SetRightKey(const SDLKey & key) { _SetNewKey(_key.right, key); }
	void SetConfirmKey(const SDLKey & key) { _SetNewKey(_key.confirm, key); }
	void SetCancelKey(const SDLKey & key) { _SetNewKey(_key.cancel, key); }
	void SetMenuKey(const SDLKey & key) { _SetNewKey(_key.menu, key); }
	void SetSwapKey(const SDLKey & key) { _SetNewKey(_key.swap, key); }
	void SetLeftSelectKey(const SDLKey & key) { _SetNewKey(_key.left_select, key); }
	void SetRightSelectKey(const SDLKey & key) { _SetNewKey(_key.right_select, key); }
	void SetPauseKey(const SDLKey & key) { _SetNewKey(_key.pause, key); }
	//@}

	/** \name   Sets new joystick buttons
	*** \param	key New button for the action
	**/
	//@{
	void SetConfirmJoy(uint8 button) { _SetNewJoyButton(_joystick.confirm, button); }
	void SetCancelJoy(uint8 button) { _SetNewJoyButton(_joystick.cancel, button); }
	void SetMenuJoy(uint8 button) { _SetNewJoyButton(_joystick.menu, button); }
	void SetSwapJoy(uint8 button) { _SetNewJoyButton(_joystick.swap, button); }
	void SetLeftSelectJoy(uint8 button) { _SetNewJoyButton(_joystick.left_select, button); }
	void SetRightSelectJoy(uint8 button) { _SetNewJoyButton(_joystick.right_select, button); }
	void SetPauseJoy(uint8 button) { _SetNewJoyButton(_joystick.pause, button); }
	//@}

}; // class GameInput

} // namespace hoa_input

#endif
