///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    engine.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 23rd, 2005
 * \brief   Header file for the core game engine.
 *
 * This code contains three core components of the game engine, namely the game
 * mode stack manager, input manager, and settings manager. If you really want
 * to understand how the heart of the Allacrost engine beats, you should study
 * this file and engine.cpp thoroughly.
 *****************************************************************************/

#ifndef __ENGINE_HEADER__
#define __ENGINE_HEADER__

#include "utils.h"
#include <vector>
#include "SDL.h"
#include "defs.h"

//! All calls to the main engine are wrapped in this namespace.
namespace hoa_engine {

//! The singleton pointer responsible for maintaining and updating the game mode state.
extern GameModeManager *ModeManager;
//! The singleton pointer responsible for the user's settings during game operation.
extern GameSettings *SettingsManager;
//! The singleton pointer responsible for handling and updating user input.
extern GameInput *InputManager;
//! Determines whether the code in the hoa_engine namespace should print debug statements or not.
extern bool ENGINE_DEBUG;

//! \name Pause/Quit Audio Constants
//@{
//! \brief These are constants used for changing the audio during PauseMode and QuitMode
const uint8 ENGINE_PAUSE_AUDIO = 0;
const uint8 ENGINE_ZERO_VOLUME = 1;
const uint8 ENGINE_HALF_VOLUME = 2;
const uint8 ENGINE_SAME_VOLUME = 3;
//@}

//! \name Game States/Modes
//@{
//! \brief Different modes of operation that the game can be in.
const uint8 ENGINE_DUMMY_MODE  = 0;
const uint8 ENGINE_BOOT_MODE   = 1;
const uint8 ENGINE_MAP_MODE    = 2;
const uint8 ENGINE_BATTLE_MODE = 3;
const uint8 ENGINE_MENU_MODE   = 4;
const uint8 ENGINE_SHOP_MODE   = 5;
const uint8 ENGINE_PAUSE_MODE  = 6;
const uint8 ENGINE_QUIT_MODE   = 7;
const uint8 ENGINE_SCENE_MODE  = 8;
const uint8 ENGINE_WORLD_MODE  = 9;
//@}

//! \name Game Languages
//@{
//! \brief Languages that the game can be run in.
const uint8 ENGINE_ENGLISH = 0;
const uint8 ENGINE_SPANISH = 1;
const uint8 ENGINE_GERMAN  = 2;
//@}

//! An internal namespace to be used only within the engine code. 
namespace private_engine {

//! The threshold value we use to partition the range of joystick values into on and off
const int16 JOYAXIS_THRESHOLD = 8192;

} // namespace private_engine

/*!****************************************************************************
 *  \brief A (mostly) abstract parent class that all other game mode classes inherit from.
 *
 *  The GameMode class is the starting base for developing a new mode of operation
 *  for the game. The GameModeManager class handles the various GameMode classes
 *  currently in play and one would be wise to understand the interaction between
 *  these two classes.
 *
 *  \note 1) Both the copy constructor and copy assignment operator are private.
 *
 *  \note 2) Keep in mind that just because you set the coordinate_system member,
 *  the actual coordinate system won't change until you call 
 *  VideoManager->SetCoordSys(coordinate_system). Remember to always do this in
 *  the dervied class destructor.
 *
 *  \note 3) <b>THIS IS VERY IMPORTANT.</b> Never, under any circumstances should
 *  you ever invoke the delete function on a pointer to this object or its related
 *  subclasses. The reason is that all of the memory reference handling is done
 *  by the GameModeManager class. If you attempt to ignore this warning you \b will
 *  generate a segmentation fault. Godspeed gentlemen.
 *****************************************************************************/
class GameMode {
protected:
	//! Indicates what 'mode' this object is in (what type of inherited class).
	uint8 mode_type;
	// The coordinate system that the game mode uses.
	// CoordSys coordinate_system;

	friend class GameModeManager;
private:
	//! Copy constructor is private, because having a copy of a game mode object is a \b bad idea.
	GameMode(const GameMode& other) {}
	//! Copy assignment operator is private, because having a copy of a game mode object is a \b bad idea.
	GameMode& operator=(const GameMode& other) {}
	// Note: Should I make the delete and delete[] operators private too?
public:
	//! The constructor initializes the singleton pointers.
	GameMode();
	/*!
	 *  \brief In addition to initializing the singleton pointers, this sets the proper \c mode_type.
	 *  \param mt The \c mode_type to set this object to.
	 */
	GameMode(uint8 mt);
	//! Virtual destructor, since the inherited class holds all the important data.
	virtual ~GameMode();
	/*!
	 *  \brief Purely virtual function for updating the status in this game mode
	 *  \param time_elapsed The amount of milliseconds that have elapsed since the last time this function was called.
	 */
	virtual void Update(uint32 time_elapsed) = 0;
	//! Purely virtual function for drawing the next screen frame.
	virtual void Draw() = 0;
	/*!
	 *  \brief Purely virtual function for resetting the state of the class.
	 *  
	 *  This function is called whenever the game mode is made active (ie, it is made the new active game mode
	 *  on the top of the game stack). This includes when the game mode is first created and pushed onto the
	 *  game stack, so in that manner it can also be viewed as a helper function to the constructor.
	 */
	virtual void Reset() = 0;
}; // class GameMode

/*!****************************************************************************
 *  \brief Manages and maintains all the living game modes in a stack.
 *
 *  The GameModeManager class keeps a stack of GameMode objects, where the object
 *  on the top of the stack is the active GameMode (there can only be one active
 *  game mode at any time). The virtual Update() and Draw() functions are invoked
 *  on the game mode that is on the top of the stack.
 *  
 *  When a condition is encountered in which a game mode wishes to destroy itself 
 *  and/or push a new mode onto the stack, this does not occur until the next
 *  call to the GameModeManager#Update() function. The GameModeManager#push_stack
 *  retains all the game modes we wish to push onto the stack on the next call to
 *  GameModeManager#Update(), and the GameModeManager#pop_count member retains
 *  how many modes to delete and pop off the GameModeManager#game_stack. Pop 
 *  operations are \c always performed before push operations.
 *
 *  \note 1) This class is a singleton.
 *
 *  \note 2) You might be wondering why the game_stack uses a vector container
 *  rather than a stack container. There are two reasons, the first being that
 *  we can't do a debug printout of the game_stack without removing elements *if*
 *  a stack is used. The second reason is "just in case" we need to access a stack
 *  element that is not on the top of the stack.
 *****************************************************************************/
class GameModeManager {
private:
	SINGLETON_DECLARE(GameModeManager);
	/*!
	 *  \brief A stack containing all the live game modes.
	 *  \note The back of the vector is the top of the stack.
	 */
	std::vector<GameMode*> _game_stack;

	//! A vector of game modes to push to the stack on the next call to GameModeManager#Update().
	std::vector<GameMode*> _push_stack;
	//! True if a state change occured and we need to change the active game mode.
	bool _state_change;
	//! The number of game modes to pop from the back of the stack on the next call to GameModeManager#Update().
	uint32 _pop_count;
public:
	SINGLETON_METHODS(GameModeManager);
	//! Increases the GameModeManager#pop_count member to pop the top stack item on the next 
	//! call to GameModeManager#Update().
	void Pop();
	/*!
	 *  \brief Will remove all game modes from the stack on the next call to GameModeManager#Update().
	 *  
	 *  This function sets the GameModeManager#pop_count member to the size of GameModeManager#game_stack.
	 *  If there is no game mode in GameModeManager#push_stack before the next call to GameModeManager#Update(),
	 *  The game will encounter a segmentation fault and die. Therefore, be careful with this function.
	 *  
	 *  \note Typically only used when the game exits, or when a programmer is smoking crack.
	 */
	void PopAll();
	/*!
	 *  \brief Pushes a new GameMode object on top of the stack.
	 *  \param *gm The new GameMode object that will go on the top of the stack.
	 *  \note It should be obvious, but once you push a new object on the stack
	 *  top, it will automatically become the new active game state.
	 */
	void Push(GameMode* gm);
	/*!
	 *  \brief Gets the type of the currently active game mode.
	 *  \return The value of \c mode_type of the GameMode object on the top of the stack.
	 */
	uint8 GetGameType();
	/*!
	 *  \brief Gets a pointer to the top game stack object.
	 *  \return A pointer to the GameMode object on the top of the stack.
	 */
	GameMode* GetTop();
	/*!
	 *  \brief Checks if the game stack needs modes pushed or popped, then calls Update on the top game mode stack.
	 *  \param time_elapsed The amount of time that has passed since the last call to this function.
	 */
	void Update(uint32 time_elapsed);
	/*!
	 *  \brief Prints the contents of the \c game_stack to standard output.
	 *  \note This function is for debugging purposes \b only! You normally should never call it.
	 */
	void PrintStack();
}; // class GameModeManager

/*!****************************************************************************
 *  \brief Retains and manages information about the user's preferences and settings.
 *
 *  The GameModeManager class keeps a stack of GameMode objects, where the object
 *  on the top of the stack is the active GameMode (there can only be one active
 *  game mode at any time). The virtual Update() and Draw() functions are invoked
 *  on the game mode that is on the top of the stack.
 *
 *  \note 1) This class is a singleton.
 *
 *  \note 2) The reason this class contains things like the volume and screen resolution instead of
 *  the GameAudio and GameVideo classes is because all of this data are things that the user
 *  can configure for themselves. It's much easier to load from and store to a config file
 *  using one class rather than several.
 *
 *  \note 3) This class needs a lot of clean-up work. Some functions may be redundant or simply
 *  unnecessary.
 *****************************************************************************/
class GameSettings {
private:
	SINGLETON_DECLARE(GameSettings);
	//! The last time the game was updated, in milliseconds.
	uint32 _last_update;
	//! Retains the number of milliseconds that have expired for frame rate calculation.
	uint32 _fps_timer;
	//! Keeps count of the number of frames that have been drawn.
	uint32 _fps_counter;
	//! The number of frames drawn per second. Updated approximately every one second.
	float _fps_rate;
	//! When this is set to false, the program will exit (maturally).
	bool _not_done;
	//! The language in which to render text.
	uint8 _language;
	//! Retains the current screen width and height.
	SDL_Rect _screen_info;
	//! True if the game is running in full screen mode.
	bool _full_screen;
	//! Used by PauseMode and QuitMode for temporarily changing the volume on pause/quit events.
	uint8 _pause_volume_action;
	
	//! \name Singleton Class Pointers
	//@{
	//! \brief References to the various game singletons.
	hoa_data::GameData *_DataManager;
	//@}

public:
	SINGLETON_METHODS(GameSettings);

	// NOTE: I think I'll remove these two members since you can call GetMusic/SoundVolume from the GameAudio class
	//! \brief The music volume level
	//! \note  Valid range is [0, 128]
	int32 music_vol;
	//! \brief The sound volume level
	//! \note  Valid range is [0, 128]
	int32 sound_vol;

	/*!
	 *  \brief  Sets the \c last_update member to the current time.
	 *  \return The difference between the last value of \c last_update and the current time.
	 *  \note   There's a chance we could get errors in other parts of the program code if the
	 *  value returned by UpdateTime() is zero. We can prevent this if we always make sure the
	 *  function returns at least one, but I'm not sure there exists a computer fast enough
	 *  that we have to worry about it.
	 */
	uint32 UpdateTime();
	/*!
	 *  \brief Initializes the \c last_update member to the current time (in milliseconds).
	 *  \note <b>DO NOT</b> call SetTimer() anywhere in your code. It should only be called once
	 *  in main.cpp, just before entering the main game loop. If you call it, you will be spelling
	 *  your own d-o-o-m.
	 */
	void SetTimer();
	/*!
	 *  \brief Changes game to run in full screen mode or windowed mode.
	 *  \param fs If true, the game will run in full-screen mode, otherwise in windowed mode.
	 *  \note  This function will cause no harm if the desired screen mode is already active.
	 *  \note  Function is a candidate for becoming extinct.
	 */
	void SetFullScreen(bool fs) { _full_screen = fs; }
	/*!
	 *  \brief  Toggles between full-screen and windowed mode.
	 */
	void ToggleFullScreen() { if (_full_screen) _full_screen = false; else _full_screen = true; }
	/*!
	 *  \brief  Determines if the game is running in full screen mode.
	 *  \return True if the game is running in full-screen mode.
	 *  \note   This function will likely get a name change later.
	 */
	bool FullScreen() { return _full_screen; }
	/*!
	 *  \brief  Gets information about the current screen size.
	 *  \return Information about the screen width and height.
	 */
	SDL_Rect GetScreenInfo() { return _screen_info; }
	/*!
	 *  \brief  Sets the screen size.
	 *  \param info The screen width and height.
	 */
	void SetScreenInfo(SDL_Rect info) { _screen_info = info; }
	/*!
	 *  \brief  Used to determine what language the game is running in.
	 *  \return The language that the game is running in.
	 */
	uint8 GetLanguage() { return _language; }
	/*!
	 *  \brief  Sets the language that the game should use.
	 *  \return The numerical value representing the language the game is running in.
	 */
	void SetLanguage(uint8 lang) { _language = lang; }
	/*!
	 *  \brief  Determines whether the user is done with the game.
	 *  \return False if the user is done and would like to exit the game.
	 */
	bool NotDone() { return _not_done; }
	/*!
	 *  \brief  The function to call to initialize the exit process of the game.
	 *  \note  The game won't actually quit until it tries to re-iterate through the main game loop again.
	 */
	void ExitGame() { _not_done = false; }
	/*!
	 *  \brief  Sets the action to take on the audio volume levels when the game is paused.
	 *  \param action The value to set pause_volume_action to.
	 *
	 *  This action takes place whenever the active game mode class is PauseMode or QuitMode.
	 */
	void SetPauseVolumeAction(uint8 action) { _pause_volume_action = action; }
	/*!
	 *  \brief  Used to find out what the game is set to do on a pause event.
	 *  \return The value of the action that the game takes on a pause event.
	 */
	uint8 GetPauseVolumeAction() { return _pause_volume_action; }
}; // class GameSettings

/*!****************************************************************************
 *  \brief Retains information about the user-defined key settings.
 *
 *  This class is simply a container for various SDLKey structures that represent
 *  the game's input keys.
 *
 *  \note 1) The only classes that need to interact with this class are GameInput
 *  and GameData (hence, all members are private and both classes are declared
 *  as friends). GameData initailizes the class members and GameInput uses the
 *  SDLKey members to check for keyboard input events.
 *****************************************************************************/
class KeyState {
private:
	//! \name Generic key names
	//@{
	//! \brief Each member holds the actual keyboard key that corresponds to the named key event.
	SDLKey _up;
	SDLKey _down;
	SDLKey _left;
	SDLKey _right;
	SDLKey _confirm;
	SDLKey _cancel;
	SDLKey _menu;
	SDLKey _swap;
	SDLKey _left_select;
	SDLKey _right_select;
	SDLKey _pause;
	//@}
	
	friend class GameInput;
	friend class hoa_data::GameData;
}; // class KeyState

/*!****************************************************************************
 *  \brief Retains information about the user-defined joystick settings.
 *
 *  This class is simply a container for various SDL structures that represent
 *  the joystick input. Because joystick axis movement is not a simple "on/off"
 *  state as opposed to keys, we need a little extra logic so that it can be
 *  represented as such. In the range of possible joystick values (-32768 to 32767),
 *  we section off the region into thirds and label any crossing of these 'boundaries'
 *  as state changes.
 *
 *  \note 1) The only classes that need to interact with this class are GameInput
 *  and GameData (hence, all members are private and both classes are declared
 *  as friends). GameData initailizes the class members and GameInput uses the
 *  members to check for joystick input events.
 *
 *  \note 2) This class is still incomplete and needs to be implemented. I 
 *  haven't gotten around to it due to problems getting SDL to recognize my
 *  own joystick.
 *****************************************************************************/
class JoystickState {
private:
	//! A pointer to the active joystick.
	SDL_Joystick *_js;
	
	//! An index to the SDL joystick which should be made active.
	int32 _joy_index;
	
	//! \name Generic button names.
	//@{
	//! \brief Each member retains the index that refers to the joystick button registered to the event.
	uint8 _confirm;
	uint8 _cancel;
	uint8 _menu;
	uint8 _swap;
	uint8 _left_select;
	uint8 _right_select;
	uint8 _pause;
	uint8 _quit;
	//@}
	
	//! \name Previous Peak Joystick Axis Values
	//@{
	//! \brief These variables retain the previous peak value of each joystick axis. 
	int16 _x_previous_peak;
	int16 _y_previous_peak;
	//@}
	
	//! \name Current Peak Joystick Axis Values
	//@{
	//! \brief These variables retain the current peak value of each joystick axis. 
	int16 _x_current_peak;
	int16 _y_current_peak;
	//@}
	
	friend class GameInput;
	friend class hoa_data::GameData;
}; // class JoystickState



/*!****************************************************************************
 *  \brief Retains and manages all user input events.
 *
 *  The way this class operates is by first retaining the user-defined keyboard
 *  and joystick settings. The EventHandler() function is called once every 
 *  iteration of the main game loop to process all events that have accumulated
 *  in the SDL input queue. Three boolean varaiables for each type of input event
 *  are maintained to represent the state of each input:
 *  
 *  - state   :: for when a key/button is being held down
 *  - press   :: for when a key/button was previously untouched, but has since been pressed
 *  - release :: for when a key/button was previously held down, but has since been released
 *
 *  The names of the common game events and their purposes are listed below:
 *
 *  - up           :: Moves a cursor/sprite upwards
 *  - down         :: Moves a cursor/sprite downwards
 *  - left         :: Moves a cursor/sprite left
 *  - right        :: Moves a cursor/sprite right
 *  - confirm      :: Confirms a menu selection or command
 *  - cancel       :: Cancels a menu selection or command
 *  - menu         :: Opens up a menu
 *  - swap         :: Used for swapping selected items or characters
 *  - left_select  :: Selecting multiple items or friendlys
 *  - right_select :: Selecting multiple items or foes
 *
 *  There are also other events and meta-key combination events that are handled within
 *  this class itself:
 *
 *  - pause      :: a user-defined key/button for pausing the game
 *  - Ctrl+F     :: toggles the game between running in windowed and full screen mode
 *  - Ctrl+Q     :: brings up the quit menu/quits the game
 *  - Ctrl+S     :: saves a screenshot of the current screen
 *  - Quit Event :: same as Ctrl+Q, this happens when the user tries to close the game window
 *  
 *  Keep in mind that these events are \c not mutually exclusive (you can have an up press and
 *  a down press during the same event processing). This class does not attempt to give one
 *  event precedence over the other, except in the case of pause and quit events. Therefore, in
 *  your code you should deal with the problem of not having mutual exclusive events directly.
 *
 *  \note 1) This class is a singleton.
 *
 *  \note 2) Pause and quit events are handled automatically in this class, so
 *  do not attempt to handle them in your inherited game mode classes. However,
 *  you can determine what happens to the audio on a pause or quit event (see the
 *  GameSettings class for information on that).
 *
 *  \note 3) Because this class will be used quite often to check the status of
 *  the various booleans, encapsulation has been used so that one can't
 *  accidentally change the value of one of the members and introduce hard-to-find
 *  bugs in the code. (eg. `if (up_state = true)` instead of `if (up_state == true)`.
 *  
 *  \note 4) In the end, all you really need to know about this class are the
 *  member access functions in the public section of this class (its not that hard).
 *  
 *  \note 5) Currently joystick hat and ball events are not handled by this input
 *  event manager. I may add support for them later if necessary. 
 *****************************************************************************/
class GameInput {
private:
	SINGLETON_DECLARE(GameInput);
	//! Retains the active-user defined key settings
	KeyState _key;
	//! Retains the active-user defined joystick settings
	JoystickState _joystick;
	
	//! \name Input State Members
	//@{
	//! \brief True if the named input event key/button is currently being held down
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
	
	//! \name Input Press Members
	//@{
	//! \brief True if the named input event key/button has just been pressed
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
	
	//! \name Input Release
	//@{
	//! \brief True if the named input event key/button has just been released
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
	
		
	//! \name First Joystick Axis Motion 
	//@{
	//! \brief Retains whether a joystick axis event has already occured or not
	bool _joyaxis_x_first;
	bool _joyaxis_y_first;
	//@}
	
	//! Processes all keyboard input events
	void _KeyEventHandler(SDL_KeyboardEvent& key_event);
	//! Processes all joystick input events
	void _JoystickEventHandler(SDL_Event& js_event);
public:
	SINGLETON_METHODS(GameInput);

	/*!
	 *  \brief Examines the SDL queue for all user input events and calls appropriate sub-functions.
	 *
	 *  This function handles all the meta keyboard events (events when a modifier key like Ctrl or 
	 *  Alt is held down) and all miscellaneous user input events (like clicking on the window button
	 *  to quit the game). Any keyboard or joystick events that occur are passed to the KeyEventHandler()
	 *  and JoystickEventHandler() functions.
	 *
	 *  \note EventHandler() is only called in the main game loop. Don't call it in your code.
	 */ 
	void EventHandler();

	//! \name Input state member access functions
	//@{
	//! \brief True if the named input event key/button is currently being held down
	bool UpState() { return _up_state; }
	bool DownState() { return _down_state; }
	bool LeftState() { return _left_state; }
	bool RightState() { return _right_state; }
	bool ConfirmState() { return _confirm_state; }
	bool CancelState() { return _cancel_state; }
	bool MenuState() { return _menu_state; }
	bool SwapState() { return _swap_state; }
	bool LeftSelectState() { return _left_select_state; }
	bool RightSelectState() { return _right_select_state; }
	//@}
	
	//! \name Input press members
	//@{
	//! \brief True if the named input event key/button has just been pressed
	bool UpPress() { return _up_press; }
	bool DownPress() { return _down_press; }
	bool LeftPress() { return _left_press; }
	bool RightPress() { return _right_press; }
	bool ConfirmPress() { return _confirm_press; }
	bool CancelPress() { return _cancel_press; }
	bool MenuPress() { return _menu_press; }
	bool SwapPress() { return _swap_press; }
	bool LeftSelectPress() { return _left_select_press; }
	bool RightSelectPress() { return _right_select_press; }
	//@}
	
	//! \name Input release
	//@{
	//! \brief True if the named input event key/button has just been released
	bool UpRelease() { return _up_release; }
	bool DownRelease() { return _down_release; }
	bool LeftRelease() { return _left_release; }
	bool RightRelease() { return _right_release; }
	bool ConfirmRelease() { return _confirm_release; }
	bool CancelRelease() { return _cancel_release; }
	bool MenuRelease() { return _menu_release; }
	bool SwapRelease() { return _swap_release; }
	bool LeftSelectRelease() { return _left_select_release; }
	bool RightSelectRelease() { return _right_select_release; }
	//@}
}; // class GameInput

} // namespace hoa_engine

#endif
