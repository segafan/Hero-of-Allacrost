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
	    GetTable*Ref() functions return the value by reference, and GetTable*() return by value.
	*/ 
	void OpenTable(const char *tbl_name);
	void CloseTable();
	
	bool GetTableBool(const char *key);
	void GetTableBoolRef(const char *key, bool &ref);
	
	int32 GetTableInt(const char *key);
	void GetTableIntRef(const char *key, int32 &ref);
	
	float GetTableFloat(const char *key);
	void GetTableFloatRef(const char *key, float &ref);
	
	std::string GetTableString(const char *key);
	void GetTableStringRef(const char * key, std::string &ref);
	//@}

	//! \name Lua Variable Access Functions
	//@{
	//! \brief These functions look up a global variable in a Lua file and return its value.
	//! \param *key The name of the Lua variable to access.
	//! \param &ref A reference to the variable to be filled with the requested value.
	bool GetGlobalBool(const char *key);
	void GetGlobalBoolRef(const char *key, bool &ref);
	
	int32 GetGlobalInt(const char *key);
	void GetGlobalIntRef(const char *key, int32 &ref);
	
	float GetGlobalFloat(const char *key);
	void GetGlobalFloatRef(const char *key, float &ref);
	
	std::string GetGlobalString(const char *key);
	void GetGlobalStringRef(const char *key, std::string &ref);
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
	
	//! \name Lua<->C++ binding functions
	//@{
	//! \brief Functions used to bind C++ class member functions and objects to be used directly from Lua.
	//! \param *obj the object to be registered (see notes below)
	//! \param *objname the name by which the object will be referred from Lua
	//! \param func a pointer to the function to be registered
	//! \param *funcname the name by which the function will be referred to from Lua
	/*!
	    Calling protocol:
	    1. RegisterClassStart()
	    2. RegisterMemberFunction(...), and/or RegisterObject(...), in any order.
	    3. RegisterClassEnd()
	    
	    The RegisterFunction() function can be called at any time, independently of the other functions
	    from this group.
	    
	    Notes on usage:
	    
	    - when calling RegisterMemberFunction, the second parameter (Class *obj) should be specified like this:
	        (MyClass*)0
	      For example:
	      	RegisterMemberFunction("GetSomething", (MyClass*)0, &MyClass::GetSomething);
	      Why add that argument when it's not used, you ask? Well, LuaPlus needs that cast because of it's
	      template magic, and that's the easy way to let it know what class it's working with.
	      But, when calling RegisterObject(), you specify a valid object pointer (don't forget the cast).
	    
	    - one RegisterClassStart/RegisterClassEnd block creates one Lua metatable and uses it to assign C++ stuff.
	      What this means is that if you register all the members of a class and, say, one object of that class
	      inside one RCS/RCE block, and then make a new block which just registers another object of the
	      same class, that won't work, because that object will be assigned a NEW (and empty) metatable.
	      The solution: either register all the objects you need inside one RCS/RCE block, or re-register all the
	      class methods the object needs inside the second block too. This can be useful, actually.
	      See the HTML documentation for a more detailed explanation. 
	*/ 
	void RegisterClassStart();
	void RegisterClassEnd();
	template <typename Class, typename Function>
	bool RegisterMemberFunction(const char *funcname, Class *obj, Function func);
	template <typename Class>
	bool RegisterObject(const char *objname, Class *obj);
	template <typename Function>
	bool RegisterFunction(const char* funcname, Function func);
	//@}
	
	//! \name Lua function calling wrapper
	//@{
	//! \brief This function allows you to call arbitrary Lua functions from C++
	//! \param *func a string containing the name of the Lua function to be called
	//! \param *sig a string describing arguments and results. See below for explanation.
	/*!
	    This function is an almost identical copy of the call_va() function from the book
	    "Programming In Lua", chapter 25.3. It lets you call an arbitrary Lua function with
	    a desired number of arguments and results.
	    
	    Here's how the *sig string is used: for example, if you want to call a function that
	    receives two integeres, and returns a double and a string, then *sig would equal "ii>ds".
	    The ">" is used to delimit results from arguments. Here is a list of descriptors you can use:
	    "i" - an integer value
	    "d" - double precision floating point value
	    "s" - a string. When supplying CallGlobalFunction with a variable to hold the return value of
	          string type, you should supply the address of a char* . Example:
		  	char *retval = 0;
			CallGlobalFunction("function_returning_a_string", ">s", &retval);
		  The function allocates the space for the string.
	*/
	void CallGlobalFunction(const char *func, const char *sig, ...);
	//@}
	
// END Lua related stuff

	SINGLETON_METHODS(GameData);
};

} // namespace hoa_data

#endif
