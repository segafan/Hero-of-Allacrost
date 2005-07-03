/* 
 * engine.h
 *	Header file for the Hero of Allacrost core game engine
 *	(C) 2005 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */
 
#ifndef __ENGINE_HEADER__
#define __ENGINE_HEADER__ 

#include <vector>
#include "SDL.h" 
#include "defs.h"
#include "utils.h"


namespace hoa_engine {

extern bool ENGINE_DEBUG;

// These are constants used for changing the audio during PauseMode and QuitMode
const unsigned char ENGINE_PAUSE_AUDIO = 0;
const unsigned char ENGINE_ZERO_VOLUME = 1;
const unsigned char ENGINE_HALF_VOLUME = 2;
const unsigned char ENGINE_SAME_VOLUME = 3;

// Different states of operation the game can be in.
const unsigned char ENGINE_DUMMY_MODE  = 0;
const unsigned char ENGINE_BOOT_MODE   = 1;
const unsigned char ENGINE_MAP_MODE    = 2;
const unsigned char ENGINE_BATTLE_MODE = 3;
const unsigned char ENGINE_MENU_MODE   = 4;
const unsigned char ENGINE_SHOP_MODE   = 5;
const unsigned char ENGINE_PAUSE_MODE  = 6;
const unsigned char ENGINE_QUIT_MODE   = 7;
const unsigned char ENGINE_SCENE_MODE  = 8;
const unsigned char ENGINE_WORLD_MODE  = 9;

// Used for what language the game is running in
const unsigned char ENGINE_ENGLISH = 0;
const unsigned char ENGINE_SPANISH = 1;
const unsigned char ENGINE_GERMAN  = 2;



/******************************************************************************
 GameMode class - A parent class that all other game mode classes inherit from.

	>>>members<<<
		unsigned char mode_type: value indicating what "type" of GameMode this is.
		GameXXX* XXXManager: easy references to all the game's singleton classes

	>>>functions<<< 
		virtual void Update(Uint32 time_elapsed): A purely virtual function that updates the game status
		virtual void Render(): A purely virtual function that draws the next frame of the screen

	>>>notes<<< 
		1) Both the copy constructor and copy assignment operator are private.
		
		2) >>>>>>>>>>>>>>>>>>>>>THIS IS VERY IMPORTANT<<<<<<<<<<<<<<<<<<<<<<<<< 
			Never, under any circumstances should you ever invoke the delete function on a pointer to this 
			object or its related subclasses. The reason is that all of the memory reference handling is done 
			by the GameModeManager class. If you attempt to ignore this warning you will generate a segmentation 
			fault. Godspeed gentlemen.
 *****************************************************************************/
class GameMode {
protected:
	unsigned char mode_type;
	
	hoa_audio::GameAudio *AudioManager;
	hoa_video::GameVideo *VideoManager;
	hoa_data::GameData *DataManager;
	GameInput *InputManager;
	GameModeManager *ModeManager;
	GameSettings *SettingsManager;
	hoa_global::GameInstance *InstanceManager;
	
	friend class GameModeManager;
private:
	GameMode(const GameMode& other) {}
	GameMode& operator=(const GameMode& other) {}
public:
	GameMode();
	~GameMode();
	virtual void Update(Uint32 time_elapsed) = 0;
	virtual void Draw() = 0;
};



/******************************************************************************
 GameModeManager class - Contains a stack of GameMode objects for organizing recent game modes.
	>>>This class is a singleton<<<
	
	>>>members<<< 
		vector<GameMode*> game_stack: a stack containing all of the current game modes.

	>>>functions<<< 
		void Pop(): Removes the top item from the stack and destroys it.
		void Push(GameMode* gm): Puts a new item on the top of the stack.
		unsigned char GetGameType(): Returns the type of the top stack element
		GameMode* GetTop(): Returns a copy of the top stack pointer
		void PrintStack(): For debugging purposes ONLY. Prints out the stack information.

	>>>notes<<< 
		1) The mode that is on the top of the stack (back of the vector) is the game mode that is currently
			active. Update() and Draw() will always be called on the game mode on the top of the stack.
		
		2) You might be wondering why the game_stack uses a vector container rather than a stack.
			There are two reasons, the first being that we can't do a debug printout of the game_stack
			without removing elements *if* a stack is used. The second reason is "just in case" we need
			to access a game_stack element that is not on the top of the stack sometime down the road.
 *****************************************************************************/
class GameModeManager {
private:
	SINGLETON_DECLARE(GameModeManager);
	std::vector<GameMode*> game_stack;
public: 
	SINGLETON_METHODS(GameModeManager);
	
	void Pop();
	void PopAll();
	void Push(GameMode* gm);
	unsigned char GetGameType();
	GameMode* GetTop();
	void PrintStack();
}; // class GameModeManager



/******************************************************************************
 GameSettings class - Contains various information about the user preferences and settings
	>>>This class is a singleton<<<

	>>>members<<< 
		Uint32 last_update: the last time the game was updated
		Uint32 fps_timer: retains the number of milliseconds that have expired for frame rate calculation
		int fps_counter: counts the number of frames that have been drawn
		float fps_rate: our frames drawn per second. Updated once every second or so
		unsigned int frame_tick: the amount of time (ms) that we should wait inbetween frame draws
		SDL_Rect screen_res: retains the current screen width and height
		bool full_screen: true if we are running the game in full screen mode. False if we are not.
		int music_vol: the music volume level. Valid range is [0, 128]
		int sound_vol: the sound volume level. Valid range is [0, 128]
		unsigned char paused_vol_type: used by PauseMode and QuitMode for changing the volume
		bool not_done: when this is set to false, the program will exit
		InputState InputStatus: retains the user input configuration and status
		glanguage language: the language currently selected by the user
	
	>>>functions<<< 
		Uint32 UpdateTime(): sets the last_update member to the current time and returns their difference
		void SetTimer(): sets up the timer info. Only called during program initialization
		void ToggleFullScreen(): pretty self-explanatory don't you think...
	
	>>>notes<<< 
		1) The reason this class contains things like the volume and screen resolution instead of
			the GameAudio and GameVideo classes is because all of this data are things that the user
			can configure for themselves. It's much easier to load from and store to a config file 
			with one class than several.
			
		2) There's a chance we could get errors in other parts of the code if the value returned by 
			UpdateTime() is zero. We might get around this if we always make sure it at least returns
			one, but I'm not sure there exists a computer fast enough that we have to worry about it. ^_^
			
		3) *DO NOT* call SetTimer() anywhere in your code. It should only be in the game initialization
			part of the code in loader.cpp. If you call it, you will be spelling your own d-o-o-m.
 *****************************************************************************/
class GameSettings {
private:
	SINGLETON_DECLARE(GameSettings);
	Uint32 last_update;
	Uint32 fps_timer;
	int fps_counter;
	float fps_rate;
	bool not_done;
	unsigned char language;
	SDL_Rect screen_info;
	bool full_screen;
	unsigned char pause_volume_action;
public:
	SINGLETON_METHODS(GameSettings);
	
	int music_vol;
	int sound_vol;
	
	Uint32 UpdateTime();
	void SetTimer();
	void SetFullScreen(bool fs) { full_screen = fs; }
	void ToggleFullScreen() { if (full_screen) full_screen = false; else full_screen = true; }
	bool FullScreen() { return full_screen; }
	SDL_Rect GetScreenInfo() { return screen_info; }
	void SetScreenInfo(SDL_Rect info) { screen_info = info; }
	unsigned char GetLanguage() { return language; }
	void SetLanguage(unsigned char lang) { language = lang; }
	bool NotDone() { return not_done; }
	void ExitGame() { not_done = false; }
	void SetPauseVolumeAction(unsigned char action) { pause_volume_action = action; }
	unsigned char GetPauseVolumeAction() { return pause_volume_action; }
};



/******************************************************************************
	KeyState class - a data structure holding information about the user defined key settings.
	
	>>>members<<<
	SDLKey *name*: stores the key that corresponds to the generic function of *name*
	
	>>>functions<<<
	
	>>>notes<<<
	1) The only classes that need to call this are GameInput and GameData. GameData
		Initializes the members, and GameInput uses the keys to check for input.
 *****************************************************************************/
class KeyState {
public:
	KeyState() {}
	~KeyState() {}
private:
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
	
	friend class GameInput;
	friend class hoa_data::GameData;
};



/******************************************************************************
	JoystickState class - a data structure holding information about the status of the joystick
	
	>>>members<<<
	SDL_Joystick *js: a pointer to the active joystick
	
	>>>functions<<<
	
	>>>notes<<<
	1) The only classes that need to call this are GameInput and GameData. GameData
		Initializes the members, and GameInput uses the keys to check for input.
		
	2) This class is still incomplete and needs to be implemented. It's hard for me to do when
		SDL won't recognize my joystick though, because I can't test it.
 *****************************************************************************/
class JoystickState {
public:
	JoystickState() {}
	~JoystickState() {}
private:
	SDL_Joystick *js;
	
	friend class GameInput;
	friend class hoa_data::GameData;
};



/******************************************************************************
 GameInput class - Manages and handles all the game's user input
	>>>This class is a singleton<<<
	
	>>>members<<<
	bool *name_state*: true if input is being held down
	bool *name_press*: true if input was just pressed
	bool *name_release*: true if input was just released
	
	GameModeManager *ModeManager: a pointer to the singleton class
	
	>>>functions<<< 
		void KeyEventHandler(SDL_KeyboardEvent *key_event): helps handle keyboard events
		void JoystickEventHandler(SDL_Event *js_event): helps handle joystick events
		void EventHandler(): handles all user input events and sets input status flags appropriately
		bool *NameState*(): returns true if input is being held down
		bool *NamePress*(): true if input was just pressed
		bool *NameRelease*(): true if input was just released
		
	>>>notes<<<
		1) There is also a 'pause' input, but it is handled automatically by this class and
			your code will never need to worry about it. However, you can determine what happens
			to the audio on a pause even (see the GameSettings class for info on that).
			
		2) EventHandler() is only called in the main game loop. Don't call it in your code.
		
		3) Just to make it perfectly clear, all you need to remember are the *State, *Press, and
			*Release functions, so just memorize the name of the inputs that preceed those functions.
			{{{ Up, Down, Left, Right, Confirm, Cancel, Menu, Swap, LSelect, RSelect }}}
 *****************************************************************************/
class GameInput {
private:
	SINGLETON_DECLARE(GameInput);
	KeyState Key;
	JoystickState Joystick;
	
	bool up_state;
	bool up_press;
	bool up_release;
	bool down_state;
	bool down_press;
	bool down_release;
	bool left_state;
	bool left_press;
	bool left_release;
	bool right_state;
	bool right_press;
	bool right_release;
	bool confirm_state;
	bool confirm_press;
	bool confirm_release;
	bool cancel_state;
	bool cancel_press;
	bool cancel_release;
	bool menu_state;
	bool menu_press;
	bool menu_release;
	bool swap_state;
	bool swap_press;
	bool swap_release;
	bool left_select_state;
	bool left_select_press;
	bool left_select_release;
	bool right_select_state;
	bool right_select_press;
	bool right_select_release;
	
	GameModeManager *ModeManager;
	GameSettings *SettingsManager;
	hoa_data::GameData *DataManager;
	
	void KeyEventHandler(SDL_KeyboardEvent *key_event);
	void JoystickEventHandler(SDL_Event *js_event);
public:
	SINGLETON_METHODS(GameInput);
	
	void EventHandler();
	bool UpState() { return up_state; }
	bool UpPress() { return up_press; }
	bool UpRelease() { return up_release; }
	bool DownState() { return down_state; }
	bool DownPress() { return down_press; }
	bool DownRelease() { return down_release; }
	bool LeftState() { return left_state; }
	bool LeftPress() { return left_press; }
	bool LeftRelease() { return left_release; }
	bool RightState() { return right_state; }
	bool RightPress() { return right_press; }
	bool RightRelease() { return right_release; }
	bool ConfirmState() { return confirm_state; }
	bool ConfirmPress() { return confirm_press; }
	bool ConfirmRelease() { return confirm_release; }
	bool CancelState() { return cancel_state; }
	bool CancelPress() { return cancel_press; }
	bool CancelRelease() { return cancel_release; }
	bool MenuState() { return menu_state; }
	bool MenuPress() { return menu_press; }
	bool MenuRelease() { return menu_release; }
	bool SwapState() { return swap_state; }
	bool SwapPress() { return swap_press; }
	bool SwapRelease() { return swap_release; }
	bool LeftSelectState() { return left_select_state; }
	bool LeftSelectPress() { return left_select_press; }
	bool LeftSelectRelease() { return left_select_release; }
	bool RightSelectState() { return right_select_state; }
	bool RightSelectPress() { return right_select_press; }
	bool RightSelectRelease() { return right_select_release; }
};



} // namespace hoa_engine

#endif
