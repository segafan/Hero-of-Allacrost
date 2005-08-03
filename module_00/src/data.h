/*
 * data.h
 *  Hero of Allacrost header file for data management
 *  (C) 2004, 2005 by Vladimir Mitrovic
 *
 *  This code is licensed under the GNU GPL. It is free software and you may modify it 
 *   and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *   for details.
 */

#ifndef __DATA_HEADER__
#define __DATA_HEADER__

#include "utils.h"

extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}
#include <string>
#include <vector>
#include "defs.h"



namespace hoa_data {

extern bool DATA_DEBUG;

// A list of libraries to expose for use in scripts
static const luaL_reg LUALIBS[] = {
	{"base", luaopen_base},
	{"table", luaopen_table},
	{"io", luaopen_io},
	{"string", luaopen_string},
	{"math", luaopen_math},
	{"debug", luaopen_debug},
	{"loadlib", luaopen_loadlib},
	{NULL, NULL} };

// A quick reference to the top of the Lua stack
const int LUA_STACK_TOP = -1;



/******************************************************************************
 * GameData class - Manages all the (Lua) data files and handling. 
 *  >>>This class is a singleton<<<
 *
 * members: l_stack: the Lua stack. Handles all data sharing between C++ and Lua
 *
 * functions: 
 *	XXX GetTableXXX(const char *key):
 *		Searches a table (assumed to be on top of the stack) for key and returns its value of
 *		type XXX (bool, int, float or string)
 *	XXX GetGlobalXXX(const char *key):
 *		Gets the value of the global variable and returns it (neet, eh?). XXX can be bool, int,
 *		float or string.
 *	void FillStringVector(vector<string> *vect, const char *key):
 *		Fetches all members of the Lua string array accessed by key, and loads vect with them.
 *	void PrintLuaStack():
 *		Prints the contents of the Lua stack to standard output. Used for debugging purposes *only*!
 *
 *	void LoadGameSettings():
 *		Allocates the members of the GameSettings singleton class with the values retained in a
 *		data file.
 *	void LoadBootdata(vector<string> *boot_images,
 *                      vector<string> *boot_sound,
 *                      vector<string> *boot_music):
 *		The vectors passed to this function are the BootMode's member variables with similar names.
 *		The function fills them and passes them to the VideoManager and AudioManager singletons. It
 *		also sets the coordinate system used for the boot mode. See also the file data/config/boot.hoa
 *		
 *****************************************************************************/
class GameData {
private:
	SINGLETON_DECLARE(GameData);
	
	hoa_audio::GameAudio *AudioManager;
	hoa_video::GameVideo *VideoManager;
	
// BEGIN Lua related stuff
	lua_State *l_stack;
	
	bool GetTableBool(const char *key);
	int GetTableInt(const char *key);
	float GetTableFloat(const char *key);
	std::string GetTableString(const char * key);
	
	bool GetGlobalBool(const char *key);
	int GetGlobalInt(const char *key);
	float GetGlobalFloat(const char *key);
	std::string GetGlobalString(const char * key);
	
	void FillStringVector(std::vector<std::string> *vect, const char *key);
	void FillIntVector(std::vector<int> *vect, const char *key);
// END Lua related stuff

public:
	SINGLETON_METHODS(GameData);
	void PrintLuaStack();

	void LoadGameSettings();
	void ResetGameSettings();
	void LoadKeyJoyState(hoa_engine::KeyState *keystate, hoa_engine::JoystickState *joystate);
	void LoadBootData(std::vector<hoa_video::ImageDescriptor> *boot_images,
	                  std::vector<hoa_audio::SoundDescriptor> *boot_sound,
	                  std::vector<hoa_audio::MusicDescriptor> *boot_music);
	void LoadMap(hoa_map::MapMode *mapmode, int map_id);
};

} // namespace hoa_data

#endif