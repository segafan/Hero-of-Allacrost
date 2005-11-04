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

//! The singleton pointer responsible for the interaction between the C++ engine and Lua scripts.
extern GameData *DataManager;
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

	//! The Lua stack, which handles all data sharing between C++ and Lua.
	lua_State *_l_stack;
	
public:
// BEGIN Lua related stuff
	//! \name Lua file access functions
	//@{
	//! \brief Lua file access functions
	//! \param file_name The name of the Lua file to be opened.
	void OpenLuaFile(const char* file_name);
	// Do we need a CloseLuaFile() function?
	//@}

	//! \name Lua Table Access Functions
	//@{
	//! \brief These functions look up a member of a Lua table and return its value.
	//! \param *tbl_name The name of the table to open.
	//! \param *key The name of the table member to access.
	//! \param &ref A reference to the variable to be filled with the requested value.
	//! The table in question is assumed to be at the top of the stack.
	/*!
	    General usage: first call OpenTable(tname), and then any of the GetTable*()
	    functions, to access that table's members. When you're done with that particular
	    table, call CloseTable(). Obey the protocol! :)
	*/ 
	void OpenTable(const char *tbl_name);
	void CloseTable();
	void GetTableBool(const char *key, bool &ref);
	void GetTableInt(const char *key, int32 &ref);
	void GetTableFloat(const char *key, float &ref);
	void GetTableString(const char * key, std::string &ref);
	//@}

	//! \name Lua Variable Access Functions
	//@{
	//! \brief These functions look up a global variable in a Lua file and return its value.
	//! \param *key The name of the Lua variable to access.
	//! \param &ref A reference to the variable to be filled with the requested value.
	void GetGlobalBool(const char *key, bool &ref);
	void GetGlobalInt(const char *key, int32 &ref);
	void GetGlobalFloat(const char *key, float &ref);
	void GetGlobalString(const char *key, std::string &ref);
	//@}

	//! \name Lua Vector Fill Functions
	//@{
	//! \brief These functions fill a vector with members of a table in a Lua file.
	//! \param &vect A reference to the vector of elements to fill.
	//! \param *key The name of the table to use to fill the vector.
	void FillStringVector(const char *key, std::vector<std::string> &vect);
	void FillIntVector(const char *key, std::vector<int32> &vect);
	//@}
	
	void PrintLuaStack();
// END Lua related stuff

	SINGLETON_METHODS(GameData);
};

} // namespace hoa_data

#endif
