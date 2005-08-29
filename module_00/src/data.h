///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    data.h
 * \author  Vladimir Mitrovic, snipe714@allacrost.org
 * \date    Last Updated: August 25th, 2005
 * \brief   Header file for data and scripting engine.
 *
 * This code serves as the bridge between the game engine (written in C++) and
 * the data and scripting files (written in Lua).
 *
 * \note You shouldn't need to modify this code if you are wishing to extend
 * the game (either for a new inherited GameMode class, or a new data/scripting
 * file). Contact the author of this code if you feel it lacks functionality
 * that you need.
 *****************************************************************************/

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

//! All calls to the data engine are wrapped in this namespace.
namespace hoa_data {

//! Determines whether the code in the hoa_data namespace should print debug statements or not.
extern bool DATA_DEBUG;

//! A list of Lua libraries to expose for use in scripts.
static const luaL_reg LUALIBS[] = {
	{"base", luaopen_base},
	{"table", luaopen_table},
	{"io", luaopen_io},
	{"string", luaopen_string},
	{"math", luaopen_math},
	{"debug", luaopen_debug},
	{"loadlib", luaopen_loadlib},
	{NULL, NULL} };

//! For quick reference to the top of the Lua stack.
const int LUA_STACK_TOP = -1;

/*!****************************************************************************
 *  \brief Manages all the interactions between the C++ game engine and the Lua data and script files.
 *
 *  This primary function of this class is to load and store game data from Lua files. Allacrost
 *  uses the `.hoa` extension for all of its data and script files. Because this class needs access
 *  to so many members of other classes, many of which it would be unsafe to leave public, several
 *  classes declare the GameData class a friend so that it may access and modify this data as it
 *  needs.
 *
 *  \note 1) This class is a singleton
 *****************************************************************************/
class GameData {
private:
	SINGLETON_DECLARE(GameData);

	//! \name Singleton Class Pointers
	//@{
	//! \brief Pointers to the singleton classes frequently used by this class.
	hoa_audio::GameAudio *_AudioManager;
	hoa_video::GameVideo *_VideoManager;
	hoa_engine::GameSettings *_SettingsManager;
	//@}

// BEGIN Lua related stuff
	//! The Lua stack, which handles all data sharing between C++ and Lua.
	lua_State *_l_stack;

	//! \name Lua Table Access Functions
	//@{
	//! \brief These functions look up a member of a Lua table and return its value.
	//! \param *key The name of the table member to access.
	bool _GetTableBool(const char *key);
	int32 _GetTableInt(const char *key);
	float _GetTableFloat(const char *key);
	std::string _GetTableString(const char * key);
	//@}

	//! \name Lua Variable Access Functions
	//@{
	//! \brief These functions look up a global variable in a Lua file and return its value.
	//! \param *key The name of the Lua variable to access.
	//! The table in question is assumed to be at the top of the stack.
	bool _GetGlobalBool(const char *key);
	int32 _GetGlobalInt(const char *key);
	float _GetGlobalFloat(const char *key);
	std::string _GetGlobalString(const char *key);
	//@}

	//! \name Lua Vector Fill Functions
	//@{
	//! \brief These functions fill a vector with members of a table in a Lua file.
	//! \param *vect A pointer to the vector of elements to fill.
	//! \param *key The name of the table to use to fill the vector.
	void _FillStringVector(std::vector<std::string> *vect, const char *key);
	void _FillIntVector(std::vector<int32> *vect, const char *key);
	//@}
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
	void LoadMap(hoa_map::MapMode *mapmode, int32 map_id);
};

} // namespace hoa_data

#endif
