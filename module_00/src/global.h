/* 
 * global.h
 *	Header file for the Hero of Allacrost global game classes
 *	(C) 2004 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */
 
#ifndef __GLOBAL_HEADER__
#define __GLOBAL_HEADER__ 

#include <vector>
#include "SDL.h" 
#include "utils.h"
#include "data.h"

namespace hoa_global {

const bool GLOBAL_DEBUG = true;

// These are constants used for changing volume level during PausedMode and QuitMode
const int GLOBAL_ZERO_VOLUME = 0;
const int GLOBAL_HALF_VOLUME = 1;
const int GLOBAL_SAME_VOLUME = 0;

// gmode = Game mode. Different states of operation the game can be in.
enum gmode { dummy_m, boot_m, menu_m, map_m, battle_m, shop_m, world_m, paused_m, quit_m, scene_m };

// glanguage = Game language. Allows us to set what language the game is running in.
enum glanguage { english_l, spanish_l, german_l };

// gitem = identifiers for different types of weapons
enum gitem { blank_i, item_i, sbook_i, weapon_i, armor_i };

// Partially defined here so GameMode can declare it a friend
class GameModeManager;

/******************************************************************************
 GameMode class - A parent class that all other game mode classes inherit from.

	>>>members<<<
		gmode mtype: value indicating what "type" of GameMode this is. 

	>>>functions<<< 
		virtual void Update(Uint32 time_elapsed): A purely virtual function that updates the game status
		virtual void Render(): A purely virtual function that draws the next frame of the screen
		gmode GetGameType(): Simply returns the value of mtype.

	>>>notes<<< 
		1) Both the copy constructor and copy assignment operator are private.

		2) Notice that the destructor is virtual. The subclasses that inherit from this class
			handle destruction.

		3) >>>>>>>>>>>>>>>>>>>>>THIS IS VERY IMPORTANT<<<<<<<<<<<<<<<<<<<<<<<<< 
			Never, under any circumstances should you ever invoke the delete function on a pointer to this 
			object or its related subclasses. The reason is that all of the memory reference handling is done 
			by the GameModeManager class. If you attempt to ignore this warning you will generate segmentation 
			faults. Godspeed gentlemen.
 *****************************************************************************/
class GameMode {
protected:
	gmode mtype;
	
	friend class GameModeManager;
private:
	GameMode( const GameMode& other ) {}
	GameMode& operator=( const GameMode& other ) {}
public:
	GameMode() { mtype = dummy_m; }
	virtual ~GameMode() {}
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
	std::vector<GameMode*> game_stack;
	
	GameModeManager() {}
	GameModeManager( const GameModeManager& gs ) {}
	GameModeManager& operator=( const GameModeManager& gs ) {}
	SINGLETON2(GameModeManager);
public: 
	~GameModeManager();
	
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
	KeyState struct - a data structure holding information about the user defined key settings.

	SDLKey *name*: stores the key that corresponds to the generic function of *name*
 *****************************************************************************/
typedef struct KeyState {
	SDLKey up;	
	SDLKey down; 
	SDLKey left;
	SDLKey right;
	
	SDLKey confirm;
	SDLKey cancel;
	SDLKey menu;
	SDLKey swap; // Anyone know a better name?
	
	SDLKey rselect; // Right shoulder button
	SDLKey lselect; // Left shoulder button

	SDLKey pause;
} KeyState;



/******************************************************************************
	JoystickState struct - a data structure holding information about the status of the joystick
	
	>>> this is on my to do list. - Tyler, Sept 22nd
 *****************************************************************************/
typedef struct JoystickState {
	SDL_Joystick *js;
} JoystickState;



/******************************************************************************
 InputState struct - a container for holding both keyboard and joystick configuration data.
	 Includes a series of boolean values that represent the combination of both keyboard and joystick status.

	bool *name_state*:   true if a key is being held down
	bool *name_press*:   true if key was just pressed
	bool *name_release*: true if key was just released 
 *****************************************************************************/
typedef struct InputState {
	KeyState key;
	JoystickState joystick;
	
	
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
	
	bool rselect_state;
	bool rselect_press;
	bool rselect_release;
	
	bool lselect_state;
	bool lselect_press;
	bool lselect_release;
	
	bool pause_state;
	bool pause_press;
	bool pause_release;	
} InputState;



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
		int paused_vol_type: used by PauseMode and QuitMode for changing the volume
		bool paused_audio_on_quit: if set to true, during PauseMode or QuitMode the audio will pause
		bool not_done: when this is set to false, the program will exit
		InputState InputStatus: retains the user input configuration and status
		glanguage language: the language currently selected by the user
	
	>>>functions<<< 
		~GameSettings(): closes any open joysticks
		
		void ResetInputFlags(): resets all press and release input flags to false 
		Uint32 UpdateTime(): sets the last_update member to the current time and returns their difference
	
	>>>notes<<< 
		1) The reason this class contains things like the volume and screen resolution instead of
			the GameAudio and GameVideo classes is because all of this data are things that the user
			can configure for themselves. It's much easier to load from and store to a config file 
			with one class than several.
			
		2) There's a chance we could get seg faults in other parts of the code if the value returned by 
			UpdateTime() is zero. I might think of a way around this (like always making sure it at least
			returns one), but until then be careful...
 *****************************************************************************/
class GameSettings {
private:
	Uint32 last_update;
	Uint32 fps_timer;
	int fps_counter;
	
	GameSettings();
	GameSettings( const GameSettings& gs ) {}
	GameSettings& operator=( const GameSettings& gs ) {}
	SINGLETON2(GameSettings);
public:
	float fps_rate;
	SDL_Rect screen_res;
	bool full_screen;
	int music_vol;
	int sound_vol;
	int paused_vol_type;
	bool pause_audio_on_quit;
	bool not_done;
	InputState InputStatus;
	glanguage language;
	
	~GameSettings();
	
	void ResetInputFlags();	
	Uint32 UpdateTime();
	void SetTimer();
};



} // namespace hoa_global

#endif
