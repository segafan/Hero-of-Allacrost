/* 
 * engine.h
 *	Header file for the Hero of Allacrost core game engine
 *	(C) 2004 by Tyler Olsen
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

const bool ENGINE_DEBUG = true;

// These are constants used for changing the audio during PausedMode and QuitMode
const unsigned char ENGINE_PAUSE_AUDIO_ON_PAUSE = 0;
const unsigned char ENGINE_ZERO_VOLUME_ON_PAUSE = 1;
const unsigned char ENGINE_HALF_VOLUME_ON_PAUSE = 2;
const unsigned char ENGINE_SAME_VOLUME_ON_PAUSE = 3;

// gmode = Game mode. Different states of operation the game can be in.
enum gmode { dummy_m, boot_m, menu_m, map_m, battle_m, shop_m, world_m, pause_m, quit_m, scene_m };

// glanguage = Game language. Allows us to set what language the game is running in.
enum glanguage { english_l, spanish_l, german_l };



/******************************************************************************
 GameMode class - A parent class that all other game mode classes inherit from.

	>>>members<<<
		gmode mtype: value indicating what "type" of GameMode this is.
		Game* *Manager: easy references to all the game's singleton classes

	>>>functions<<< 
		virtual void Update(Uint32 time_elapsed): A purely virtual function that updates the game status
		virtual void Render(): A purely virtual function that draws the next frame of the screen
		gmode GetGameType(): Simply returns the value of mtype.

	>>>notes<<< 
		1) Both the copy constructor and copy assignment operator are private.
		
		2) >>>>>>>>>>>>>>>>>>>>>THIS IS VERY IMPORTANT<<<<<<<<<<<<<<<<<<<<<<<<< 
			Never, under any circumstances should you ever invoke the delete function on a pointer to this 
			object or its related subclasses. The reason is that all of the memory reference handling is done 
			by the GameModeManager class. If you attempt to ignore this warning you will generate segmentation 
			faults. Godspeed gentlemen.
 *****************************************************************************/
class GameMode {
protected:
	gmode mtype;
	
	hoa_audio::GameAudio *AudioManager;
	hoa_video::GameVideo *VideoManager;
	hoa_data::GameData *DataManager;
	GameInput *InputManager;
	GameModeManager *ModeManager;
	GameSettings *SettingsManager;
	
	friend class GameModeManager;
private:
	GameMode( const GameMode& other ) {}
	GameMode& operator=( const GameMode& other ) {}
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
		gmode GetGameType(): Returns the type of the top stack element. If the stack is empty, returns bad.
		GameMode* GetTop(): Returns a copy of the top stack pointer
		void PrintStack(): For debugging purposes ONLY. Prints out the stack and all the GameMode type values stored.

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
	void Push(GameMode* gm);
	gmode GetGameType();
	GameMode* GetTop();
	void PrintStack();
}; // class GameModeManager



/******************************************************************************
 GameItem class - A parent class that all the different item classes inherit from
	
	>>>members<<< 
		gitem itype: value indicating what "type" the item is
		int item_id: an identification number for each item.
		int icount: the number of items in this instance

	>>>functions<<< 
		

	>>>notes<<< 
		
 *****************************************************************************/
// class GameItem {
// protected:
// 	gitem itype;
// 	int item_id;
// 	int icount;
// public:
// 	GameItem();
// 	GameItem( gitem );
// 	bool operator== ( const cItem & ) const;
// 	bool operator< ( const cItem & ) const;
// 	bool operator> ( const cItem & ) const;
// 	bool operator< ( const cItem & ) const;
// 	bool operator< ( const cItem & ) const;
// 	GameItem & operator= ( const cItem & );
// 	GameItem& operator++ ();
// 	GameItem& operator-- ();
// 	GameItem& operator=+ ();
// 	GameItem& operator=- ();
// }; // class GameItem



/******************************************************************************
 GItem class - A parent class that all the different item classes inherit from
	
	>>>members<<< 
		int effect: a bit-mask stating what type of item it is (recovery, battle, map, etc)

	>>>functions<<< 
		Use(): use the item by executing it's function and decrementing it's icount

	>>>notes<<< 
		
 *****************************************************************************/
// class GItem {
// public:
// 	int effect;
// 	Use();
// };
// 
// class GSkillBook {
// 	int useable;
// };
// 
// class GWeapon {
// 
// };
// 
// class GArmor {
// 
// };
// 
// 
// class GItemPack { 
// 	 std::map< GameItem, unsigned int > contents;
// public:
// 
// 	 GItemPack( GItem & , unsigned long );
// 	 void clear( );
// 	 unsigned long add( const cItem & , const unsigned long );
// 	 unsigned long remove( const cItem & , const unsigned long );
// 	 unsigned long getAmount( const cItem & ) const;
// 	 const std::map< cItem, unsigned long > & getItems( ) const;
// 	 GItemPack & operator= ( const cItemPack & );
// 	 GItemPack & operator+= ( const cItemPack & );
// 	 GItemPack operator+ ( const cItemPack & ) const;
// 
// };
// 
// class GameParty;
// 	
// 
// class GameCharacter {
// private:
// 	 std::string name;
// 	 Weapon eq_weapon;
// 	 Armor eq_head;
// 	 Armor eq_body;
// 	 Armor eq_arms;
// 	 Armor eq_legs;
// 	 std::vector<Skill> attack_skills;
// 	 std::vector<Skill> defense_skills;
// 	 std::vector<Skill> support_skills;
// 	 // AttackPoints[4]
// 	 unsigned int hit_points;
// 	 unsigned int skill_points;
// 	 unsigned int xp_points;
// 	 unsigned int xp_level;
// 	 unsigned int xp_next_lvl;
// 	 unsigned int strength;
// 	 unsigned int intelligence;
// 	 unsigned int agility;
// 	 friend GameParty;
// public:
// 	 Weapon EquipWeapon(Weapon new_weapon);
// 	 Armor EquipHeadArmor(Armor new_armor);
// 	 Armor EquipBodyArmor(Armor new_armor);
// 	 Armor EquipArmsArmor(Armor new_armor);
// 	 Armor EquipLegsArmor(Armor new_armor);
// };






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
	bool not_done;
public:
	SINGLETON_METHODS(GameSettings);
	
	float fps_rate;
	SDL_Rect screen_res;
	bool full_screen;
	int music_vol;
	int sound_vol;
	unsigned char paused_vol_type;
	glanguage language;
	
	Uint32 UpdateTime();
	void SetTimer();
	bool NotDone() { return not_done; }
	void ExitGame() { not_done = false; }
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
	SDLKey rselect;
	SDLKey lselect;
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
	bool lselect_state;
	bool lselect_press;
	bool lselect_release;
	bool rselect_state;
	bool rselect_press;
	bool rselect_release;
	
	GameModeManager *ModeManager;
	
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
	bool LSelectState() { return lselect_state; }
	bool LSelectPress() { return lselect_press; }
	bool LSelectRelease() { return lselect_release; }
	bool RSelectState() { return rselect_state; }
	bool RSelectPress() { return rselect_press; }
	bool RSelectRelease() { return rselect_release; }
};



} // namespace hoa_engine

#endif
