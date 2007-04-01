///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    scipt.h
*** \author  Daniel Steuernol - steu@allacrost.org,
***          Tyler Olsen - roots@allacrost.org,
***          Vladimir Mitrovi - snipe714@allacrost.org
*** \brief   Header file for scripting engine.
***
*** This code serves as the bridge between the game engine (written in C++) and
*** the data and scripting files (written in Lua).
***
*** \note You shouldn't need to modify this code if you are wishing to extend
*** the game (either for a new inherited GameMode class, or a new data/scripting
*** file). Contact the author of this code if you feel it lacks functionality
*** that you need.
*** ***************************************************************************/

#ifndef __SCRIPT_HEADER__
#define __SCRIPT_HEADER__

#include <fstream>
extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

// This needs a comment: what is check and why is it undefined for darwin?
#ifdef __MACH__
	#undef check
#endif

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
#include <luabind/adopt_policy.hpp>

#include "utils.h"
#include "defs.h"

/** \brief A macro for a reference to a Lua object
*** The object may be any type of Lua data, including booleans, integers, floats, strings, tables,
*** functions, etc. This member is typically used outside of this engine as a reference to make
*** Lua function calls.
**/
#define ScriptObject luabind::object

//! \brief A macro for making Lua function calls
#define ScriptCallFunction luabind::call_function

//! \brief All calls to the scripting engine are wrapped in this namespace.
namespace hoa_script {

//! \brief The singleton pointer responsible for the interaction between the C++ engine and Lua scripts.
extern GameScript *ScriptManager;
//! \brief Determines whether the code in the hoa_script namespace should print debug statements or not.
extern bool SCRIPT_DEBUG;

/** \name Script Error Codes
*** \brief Constants used for detecting various error conditions in the ScriptDescriptor class.
**/
//@{
const uint32 SCRIPT_NO_ERRORS           = 0x00000000;
//! Occurs when a file is not open or has invalid permissions and user tries to operate on the file data.
const uint32 SCRIPT_BAD_FILE_ACCESS     = 0x00000001;
//! Occurs when a numerical key is used for a variable read/write in the global space.
const uint32 SCRIPT_BAD_GLOBAL          = 0x00000002;
//! Occurs when a table failed to open.
const uint32 SCRIPT_OPEN_TABLE_FAILURE  = 0x00000004;
//! Occurs when a table failed to close. Usually means too many close operations were invoked.
const uint32 SCRIPT_CLOSE_TABLE_FAILURE = 0x00000008;
//! Occurs when a requested table key (an element in the table) is not valid
const uint32 SCRIPT_INVALID_TABLE_KEY   = 0x00000010;
//! Occurs when user attempts to write a vector of size 0, or read a vector that is not initially empty.
const uint32 SCRIPT_BAD_VECTOR_SIZE     = 0x00000020;
//! Occurs when a value retrieved from the Lua stack is an incorrect type
const uint32 SCRIPT_BAD_TYPE            = 0x00000040;
//@}

/** \name Script File Access Modes
*** \brief Used to indicate with what priveledges a file is to be opened with.
**/
enum SCRIPT_ACCESS_MODE {
	SCRIPT_CLOSED = 0,
	SCRIPT_READ,
	SCRIPT_WRITE,
	// TODO: A mode which allows "read and modify"
	// SCRIPT_MODIFY,
};



//! An internal namespace to be used only by the scripting engine itself. Don't use this namespace anywhere else!
namespace private_script {

//! Used for reference to the top of the Lua stack.
const int32 STACK_TOP = -1;

} // namespace private_script



/** ****************************************************************************
*** \brief A class for representing open script files
***
*** This class serves to read and write script files. These files are Lua scripts
*** which are processed by the game's scripting engine. Files with a .lua extension
*** are human-readable, uncompiled files and files with a .hoa extension are
*** compiled script files.
***
*** \note Compiled Lua files exhibit faster performance than uncompile files.
*** ***************************************************************************/
class ScriptDescriptor {
	friend class GameScript;
private:
	//! Functions to print out a table during stack output.
	void _SaveStackProcessTable(ScriptDescriptor &sd, std::string &name, luabind::object table);
public:
	ScriptDescriptor ()
		{ _filename = ""; _access_mode = SCRIPT_CLOSED; _error_code = 0; _lstack = NULL; }

	~ScriptDescriptor ()
		{}

	//! \brief Returns a pointer to the local lua state
	lua_State* GetLuaState()
		{ return _lstack; }

	//! \name Class Member Access Functions
	//@{
	bool IsFileOpen()
		{ return (_access_mode != SCRIPT_CLOSED); }

	std::string GetFilename()
		{ return _filename; }

	SCRIPT_ACCESS_MODE GetAccessMode()
		{ return _access_mode; }

	uint32 GetErrorCode()
		{ return _error_code; }

	std::vector<std::string>& GetOpenTables()
		{ return _open_tables; }
	//@}

	/** \name File access functions
	*** \brief Functions for opening and closing of readable Lua files.
	**/
	//@{
	/** \param file_name The name of the Lua file to be opened.
	*** \param mode The mode with which to access the file with (read, write)
	*** \return False on failure or true on success.
	*** \note This is the only function that uses explicit error checking. In other words,
	*** an error in this function call will not change the return value of the GetError() function.
	**/
	bool OpenFile(std::string file_name, SCRIPT_ACCESS_MODE mode);

	/** \brief Opens the file identified by the _filename class member
	*** \param mode The mode with which to access the file with (read, write)
	*** \return False on failure, true on success.
	**/
	bool OpenFile(SCRIPT_ACCESS_MODE mode);

	//! \brief Closes the script file and sets the _access_mode member to SCRIPT_CLOSED.
	void CloseFile();
	//@}

	/** \brief Used to check if any error occured in previous operations.
	*** \return A bit-mask value of all error conditions that have been detected.
	*** \note Everytime this function is called, the internal _error_code flag is cleared.
	***
	*** It is good practice to call this function after chunks of function calls to the
	*** children of this class (ReadScriptDescriptor, WriteScriptDescriptor) to detect if
	*** anything went wrong. The bit-mask values returned by this function are all listed
	*** in the constants called "Data Error Codes" at the top of this file. It is perfectly
	*** acceptable (although maybe pedantic) to call this function after every member function
	*** of this class is invoked.
	***
	*** It is up to the user of this API to figure out how to recover when they detect an error
	*** condition. The only thing that the code in the script management classes do is to
	*** prevent errors from causing segmentation faults.
	***
	**/
	uint32 GetError()
		{ uint32 code = _error_code; _error_code = SCRIPT_NO_ERRORS; return code; }

	/** \brief Prints out the contents of the Lua stack mechanism to standard output
	*** The elements are printed from stack top to stack bottom. As the function name
	*** indicates, this function exists for debugging purposes.
	**/
	void DEBUG_PrintLuaStack();

	//! \brief Debug function that prints out all global variable names to standard output.
	void DEBUG_ShowGlobals();

	// -------------------- Read Access Functions

	/** \name Variable Read Access Functions
	*** \brief These functions grab a basic data type from the Lua file and return its value.
	*** \param key The name or numeric id of the Lua variable to access.
	*** \return The value of the variable requested.
	*** \note The integer keys are only valid for variables stored in a table, not for global variables.
	***
	*** These functions call the template _Read() functions with a default return value.
	**/
	//@{
	bool ReadBool(std::string key)
		{ return _Read<bool>(key.c_str(), false); }

	bool ReadBool(const int32 key)
		{ return _Read<bool>(key, false); }

	int32 ReadInt(std::string key)
		{ return _Read<int32>(key.c_str(), 0); }

	int32 ReadInt(const int32 key)
		{ return _Read<uint32>(key, 0); }

	uint32 ReadUInt(std::string key)
		{ return _Read<int32>(key.c_str(), 0); }

	uint32 ReadUInt(const int32 key)
		{ return _Read<uint32>(key, 0); }

	float ReadFloat(std::string key)
		{ return _Read<float>(key.c_str(), 0.0f); }

	float ReadFloat(const int32 key)
		{ return _Read<float>(key, 0.0f); }

	std::string ReadString(std::string key)
		{ return _Read<std::string>(key.c_str(), ""); }

	std::string ReadString(const int32 key)
		{ return _Read<std::string>(key, ""); }

	hoa_utils::ustring ReadUString(std::string key)
		{ return _Read<hoa_utils::ustring>(key.c_str(), hoa_utils::MakeUnicodeString("")); }

	hoa_utils::ustring ReadUString(const int32 key)
		{ return _Read<hoa_utils::ustring>(key, hoa_utils::MakeUnicodeString("")); }
	//@}

	/** \name Vector Read Access Functions
	*** \brief These functions fill a vector with members of a table read from the Lua file.
	*** \param key The name of the table to use to fill the vector.
	*** \param vect A reference to the vector of elements to fill.
	***
	*** The table that these functions attempt to access is assumed to be <b>closed</b>. If the
	*** table is open prior to calling these functions, they will not operate properly. All of
	*** these functions call the template _ReadVector() functions to perform their operations.
	***
	*** \note The integer keys are only valid for tables that are elements of a parent table.
	*** They can not be used to access tables in the global space.
	**/
	//@{
	void ReadBoolVector(std::string key, std::vector<bool> &vect)
		{ _ReadVector<std::string, bool>(key, vect); }

	void ReadBoolVector(const int32 key, std::vector<bool> &vect)
		{ _ReadVector<bool>(key, vect); }

	void ReadIntVector(std::string key, std::vector<int32> &vect)
		{ _ReadVector<std::string, int32>(key, vect); }

	void ReadIntVector(const int32 key, std::vector<int32> &vect)
		{ _ReadVector<int32>(key, vect); }

	void ReadUIntVector(std::string key, std::vector<uint32> &vect)
		{ _ReadVector<std::string, uint32>(key, vect); }

	void ReadUIntVector(const int32 key, std::vector<uint32> &vect)
		{ _ReadVector<uint32>(key, vect); }

	void ReadFloatVector(std::string key, std::vector<float> &vect)
		{ _ReadVector<std::string, float>(key, vect); }

	void ReadFloatVector(const int32 key, std::vector<float> &vect)
		{ _ReadVector<float>(key, vect); }

	void ReadStringVector(std::string key, std::vector<std::string> &vect)
		{ _ReadVector<std::string, std::string>(key, vect); }

	void ReadStringVector(const int32 key, std::vector<std::string> &vect)
		{ _ReadVector<std::string>(key, vect); }

	void ReadUStringVector(std::string key, std::vector<hoa_utils::ustring> &vect)
		{ _ReadVector<std::string, hoa_utils::ustring>(key, vect); }

	void ReadUStringVector(const int32 key, std::vector<hoa_utils::ustring> &vect)
		{ _ReadVector<hoa_utils::ustring>(key, vect); }
	//@}

	/** \name Read Table Access Functions
	*** \brief These functions perform various Lua table operations.
	*** \param key The name of the table to operate on.
	*** \return Zero if there are no table elements or there was an error.
	***
	*** After a table is opened, it becomes the active "space" that all of the data read
	*** operations operate on. You must <b>always</b> remember to close a table once you are
	*** finished reading data from it.
	***
	*** \note The version of ReadOpenTable that uses an integer key will only work when there is
	*** at least one table already open. In other words, we can't use a number to address a
	*** table that is defined in the file's global space.
	**/
	//@{
	void ReadOpenTable(std::string key);

	void ReadOpenTable(const int32 key);

	void ReadCloseTable();

	uint32 ReadGetTableSize(std::string key);

	uint32 ReadGetTableSize(const int32 key);

	//! \note This will attempt to get the size of the most recently opened table.
	uint32 ReadGetTableSize();
	//@}

	/** \name Read Function Access Functions
	*** \param key The name of the function if it is contained in the global space, or the key
	*** if the function is embedded in a table.
	*** \return A luabind::object class object, which can be used to call the function. It effectively
	*** serves as a function pointer.
	**/
	//@{
	luabind::object ReadFunctionPointer(std::string key);

	//! \note The calling function may <b>not</b> be contained within the global space for an integer key.
	luabind::object ReadFunctionPointer(int32 key);
	//@}

	// -------------------- Write Access Functions

	/*! \name Lua CommentWrite Functions
	 *  \brief Writes comments into a Lua file
	 *  \param comment The comment to write to the file.
	 *
	 *  \note All functions automatically insert and append a new line character before returning.
	 *
	 */
	//@{
	//! Inserts a blank line into the text.
	void WriteInsertNewLine();

	//! Writes a string of text and prepends it with a comment. Equivalent to `// comment` in C++.
	void WriteComment(const char* comment);
	void WriteComment(std::string& comment);

	//! After this function is invoked, every single function call will be a comment. Equivalent to `/*` in C++.
	void WriteBeginCommentBlock();

	//! Ends a comment block. Equivalent to `*/` in C++
	void WriteEndCommentBlock();

	/*! \brief Writes the plain string of text to the file with no modification, except for a newline.
	 *  \note Typically, unless you \c really know what you are doing, you should only call this between
	 *  the beginning and end of a comment block.
	 */
	void WriteLine(const char* comment);
	void WriteLine(std::string& comment);
	//@}

	/** \name Lua Variable Write Functions
	*** \brief These functions will write a single variable and its value to a Lua file.
	*** \param *key The name of the Lua variable to write.
	*** \param value The value of the new global variable to write.
	*** \note If no write tables are open when these calls are made, the variables become global
	*** in the Lua file. Otherwise, they become keys of the most recently opened table.
	**/
	//@{
	void WriteBool(const char *key, bool value);
	void WriteBool(const int32 key, bool value);
	void WriteBool(std::string &key, bool value);
	void WriteInt(const char *key, int32 value);
	void WriteInt(const int32 key, int32 value);
	void WriteInt(std::string &key, int32 value);
	void WriteFloat(const char *key, float value);
	void WriteFloat(const int32 key, float value);
	void WriteFloat(std::string &key, float value);
	void WriteString(const char *key, const char* value);
	void WriteString(const char *key, std::string& value);
	void WriteString(const int32 key, const char* value);
	void WriteString(const int32 key, std::string& value);
	void WriteString(std::string &key, const char *value);
	void WriteString(std::string &key, std::string &value);
	//@}

	/** \name Lua Vector Write Functions
	*** \brief These functions write a vector of data to a Lua file.
	*** \param key The name of the table to use in the Lua file to represent the data.
	*** \param &vect A reference to the vector of elements to write.
	**/
	//@{
	void WriteBoolVector(const char *key, std::vector<bool> &vect);
	void WriteIntVector(const char *key, std::vector<int32> &vect);
	void WriteFloatVector(const char *key, std::vector<float> &vect);
	void WriteStringVector(const char *key, std::vector<std::string> &vect);
	//@}

	/** \name Lua Table Write Functions
	*** \brief These functions write Lua tables and their members
	*** \param key The name of the table to write.
	*** \note If you begin a new table and then begin another when you haven't ended the first one, the
	*** new table will become a key to the first. A table will only become global when there are no other
	*** write tables open.
	**/
	//@{
	void WriteBeginTable(const char *key);
	void WriteBeginTable(int key);
	void WriteEndTable();
	//@}

	// -------------------- Modify Access Functions

	/** \brief This function updates the global table with the specified key, value pair.
	*** \param varname The key name of the variable to be change
	*** \param variable The new value to set the key
	*** \note If the key varname does not exist in the lua stack, it will be added as a new key
	*** with the specified value.
	**/
	template<class T> void ChangeSetting(const std::string &varname, T variable);

	//! \brief Writes out the stack to disk
	void SaveStack(const std::string &filename);

private:
	//! \brief The name of the file that the class object represents.
	std::string _filename;

	//! \brief The access mode for the file, including if the file is not opened.
	SCRIPT_ACCESS_MODE _access_mode;

	//! \brief A bit-mask that is used to set and detect various error conditions.
	uint32 _error_code;

	//! \brief The names of the Lua tables that are currently open.
	std::vector<std::string> _open_tables;

	//! \brief The Lua stack, which handles all data sharing between C++ and Lua.
	lua_State *_lstack;

	//! \brief The output file stream to write to for when the file is opened in write mode.
	std::ofstream _outfile;

	/** \brief Checks if a file has the proper access priveledges and sets the error code if it does not
	*** \param mode The file access mode to check for
	*** \return True if the file access mode is the same as the function parameter, otherwise false.
	*** \note Only call this function if you want to set the error flag if the access mode is incorrect.
	*** If you simply want to determine what the access mode is, do a simple boolean comparison with the
	*** ScriptDescriptor#_access_mode member.
	**/
	bool _CheckFileAccess(SCRIPT_ACCESS_MODE mode);

	//! \brief Writes the pathname of all open tables (i.e., table1[table2][table3])
	void _WriteTablePath();

	/** \name Variable Read Access Templates
	*** \brief These template functions are called by the public ReadTYPE functions of this class.
	*** \param key The name or numeric id of the Lua variable to access.
	*** \param default_value The value for the function to return if an error occurs.
	*** \return The value of the variable requested.
	*** \note The integer keys are only valid for variables stored in a table, not for global variables.
	**/
	//@{
	template <class T> T _Read(int32 key, T default_value);
	template <class T> T _Read(const char *key, T default_value);
	//@}

	/** \name Vector Read Access Templates
	*** \brief These template functions are called by the public ReadTYPEVector functions of this class.
	*** \param key The name or numeric id of the Lua variable to access.
	*** \param default_value The value for the function to return if an error occurs.
	*** \return The value of the variable requested.
	*** \note The integer keys are only valid for variables stored in a table, not for global variables.
	**/
	//@{
	template <class T> void _ReadVector(int32 key, std::vector<T> &vect);
	template <class T, class U> void _ReadVector(T key, std::vector<U> &vect);
	//@}

}; // class ScriptDescriptor


/** ****************************************************************************
*** \brief Singleton class that manages all open script files.
***
*** This class monitors all open script files and their descriptor objects. It
*** maintains a global Lua state that all open Lua files use to communicate
*** with each other and with the C++ engine.
***
*** \note This class is a singleton
*** ***************************************************************************/
class GameScript {
	friend class ScriptDescriptor;

public:
	SINGLETON_METHODS(GameScript);

	//! \brief Returns a pointer to the global lua state
	lua_State *GetGlobalState()
		{ return _global_state; }

	/** \brief Checks if a file is already in use by a ScriptDescriptor object.
	*** \param filename The name of the file to check.
	*** \return True if the filename is registered to a ScriptDescriptor object who has the file opened.
	**/
	bool IsFileOpen(std::string& filename);

private:
	SINGLETON_DECLARE(GameScript);

	//! \brief Maintains a list of all data files that are currently open.
	std::map<std::string, ScriptDescriptor*> _open_files;

	//! \brief The lua state shared globally by all files
	lua_State *_global_state;

	//! \brief Adds an open file to the list of open files
	void _AddOpenFile(ScriptDescriptor* sd);

	//! \brief Removes an open file from the list of open files
	void _RemoveOpenFile(ScriptDescriptor* sd);
}; // class GameScript

// *****************************************************************************
// ************** ScriptDescriptor Template Function Definitions ***************
// *****************************************************************************

template <class T> T ScriptDescriptor::_Read(int32 key, T default_value) {
	if (_CheckFileAccess(SCRIPT_READ) == false)
		return false;

	if (_open_tables.size() == 0) {
		// There needs to be a table
		if (SCRIPT_DEBUG)
			std::cerr << "SCRIPT ERROR: _Read() No open tables to read from." << std::endl;
		_error_code |= SCRIPT_OPEN_TABLE_FAILURE;
		return default_value;
	}

	luabind::object o(luabind::from_stack(_lstack, private_script::STACK_TOP));
	if (luabind::type(o) != LUA_TTABLE) {
		// table not on top of stack
		if (SCRIPT_DEBUG) std::cerr << "SCRIPT ERROR: _Read() Top of stack is not a table." << std::endl;
		_error_code |= SCRIPT_BAD_GLOBAL;
		return default_value;
	}

	try {
		return luabind::object_cast<T>(o[key]);
	}
	catch (...) {
		if (SCRIPT_DEBUG) std::cerr << "SCRIPT ERROR: _Read() Unable to cast value to correct type." << std::endl;
		_error_code |= SCRIPT_INVALID_TABLE_KEY;
	}

	return default_value;
} // template <class T> T ScriptDescriptor::_Read(int32 key, T default_value)



template <class T> T ScriptDescriptor::_Read(const char *key, T default_value) {
	if (_CheckFileAccess(SCRIPT_READ) == false)
		return false;

	// Global value
	if (_open_tables.size() == 0) {
		lua_getglobal(_lstack, key);
		luabind::object o(luabind::from_stack(_lstack, private_script::STACK_TOP));

		if (!o) {
			if (SCRIPT_DEBUG) std::cerr << "SCRIPT ERROR: _Read() Unable to access global " << key << std::endl;
			_error_code |= SCRIPT_BAD_GLOBAL;
			return default_value;
		}

		try {
			T ret_val = luabind::object_cast<T>(o);
			lua_pop(_lstack, 1);
			return ret_val;
		}
		catch (...) {
			if (SCRIPT_DEBUG) std::cerr << "SCRIPT ERROR: _Read() Unable to cast value to correct type." << std::endl;
			_error_code |= SCRIPT_BAD_GLOBAL;
		}
	}
	// there is an open table, get the key from the table
	else {
		luabind::object o(luabind::from_stack(_lstack, private_script::STACK_TOP));
		if (luabind::type(o) != LUA_TTABLE) {
			// table not on top of stack
			if (SCRIPT_DEBUG) std::cerr << "SCRIPT ERROR: _Read() Top of stack is not a table." << std::endl;
			_error_code |= SCRIPT_BAD_GLOBAL;
			return default_value;
		}

		try {
			return luabind::object_cast<T>(o[key]);
		}
		catch (...) {
			if (SCRIPT_DEBUG) std::cerr << "SCRIPT ERROR: _Read() Unable to cast value to correct type." << std::endl;
			_error_code |= SCRIPT_INVALID_TABLE_KEY;
		}
	}
	return default_value;
} // template <class T> T ScriptDescriptor::_Read(const char *key, T default_value)



template <class T> void ScriptDescriptor::_ReadVector(int32 key, std::vector<T> &vect) {
	// Check that there is at least one open table, required for integer keys
	if (_open_tables.size() == 0) {
		if (SCRIPT_DEBUG) std::cerr << "SCRIPT ERROR: _ReadVector() Need at least one table open to use a numerical key." << std::endl;
		_error_code |= SCRIPT_BAD_GLOBAL;
		return;
	}

	// Now that the open table condition has been checked, call the other template function
	_ReadVector<int32, T>(key, vect);
} // template <class T> void ScriptDescriptor::_ReadVector(int32 key, std::vector<T> &vect)



template <class T, class U> void ScriptDescriptor::_ReadVector(T key, std::vector<U> &vect) {
	if (_CheckFileAccess(SCRIPT_READ) == false)
		return;

	ReadOpenTable(key);
	// Grab the table off the stack
	luabind::object o(luabind::from_stack(_lstack, private_script::STACK_TOP));
	if (luabind::type(o) != LUA_TTABLE) {
		if (SCRIPT_DEBUG)
			std::cerr << "SCRIPT ERROR: _ReadVector() No table on top of stack, unable to continue." << std::endl;
		_error_code = SCRIPT_INVALID_TABLE_KEY;
		return;
	}
	// We have a table loop through all items
	for (luabind::iterator it(o), end; it != end; it++) {
		try {
			vect.push_back(luabind::object_cast<U>((*it)));
		}
		catch (...) {
			if (SCRIPT_DEBUG)
				std::cerr << "SCRIPT ERROR: _ReadVector() Unable to cast value to correct type." << std::endl;
			_error_code = SCRIPT_INVALID_TABLE_KEY;
		}
	}

	ReadCloseTable();
} // template <class T, class U> void ScriptDescriptor::_ReadVector(T key, std::vector<U> &vect)



template <class T> void ScriptDescriptor::ChangeSetting(const std::string &varname, T variable) {
	// Get the table of globals
	luabind::object o(luabind::from_stack(_lstack, LUA_GLOBALSINDEX));
	for (luabind::iterator it(o), end; it != end; ++it) {
		// Check to see if global variable exists
		if (luabind::object_cast<std::string>(it.key()) == varname) {
			// Change the global variable if it is found
			*it = variable;
			return;
		}
	}

	// If we arrive here, then varname does not exist in the globals so add it
	luabind::settable(o, varname, variable);
}

} // namespace hoa_script

#endif
