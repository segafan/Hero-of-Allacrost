///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    scipt.h
 * \author  Vladimir Mitrovic, snipe714@allacrost.org, Daniel Steuernol steu@allacrost.org
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

#ifndef __SCRIPT_HEADER__
#define __SCRIPT_HEADER__

#include <fstream>
extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
#include "utils.h"
#include "defs.h"

//! All calls to the data engine are wrapped in this namespace.
namespace hoa_script {

//! The singleton pointer responsible for the interaction between the C++ engine and Lua scripts.
extern GameScript *ScriptManager;
//! Determines whether the code in the hoa_script namespace should print debug statements or not.
extern bool SCRIPT_DEBUG;

//! An internal namespace to be used only by the data engine itself. Don't use this namespace anywhere else!
namespace private_script {

//! For quick reference to the top of the Lua stack.
const int32 STACK_TOP = -1;

} // namespace private_script

//! \name Data Error Codes
//@{
const uint32 DATA_NO_ERRORS           = 0x00000000;
//! Occurs when a numerical key is used for a variable read/write in the global space.
const uint32 DATA_BAD_GLOBAL          = 0x00000001;
//! Occurs when a table failed to open.
const uint32 DATA_OPEN_TABLE_FAILURE  = 0x00000002;
//! Occurs when a table failed to close. Usually means too many close operations were invoked.
const uint32 DATA_CLOSE_TABLE_FAILURE = 0x00000004;
//! Occurs when a table field that is read contains no data.
const uint32 DATA_INVALID_TABLE_KEY   = 0x00000008;
//! Occurs when a file is not open and user tries to operate on the file data.
const uint32 DATA_BAD_FILE_ACCESS     = 0x00000010;
//! Occurs when user attempts to write a vector of size 0, or fill a vector that is not initially empty.
const uint32 DATA_BAD_VECTOR_SIZE     = 0x00000020;
//@}

//! \brief Data file access modes
enum DATA_ACCESS_MODE
{
	READ,
	WRITE,
};


/*!****************************************************************************
 *  \brief An abstract class for representing data files
 *
 *  This class serves to read and write data files.
 *  These data files are actually Lua scripts which are processed by the game
 *  engine. Files with a .lua extension are human-readable, uncompiled files
 *  and files with a .hoa extension are compiled files. 
 *
 *  \note 1) Compiled Lua files exhibit faster performance than uncompile files.
 *****************************************************************************/
class ScriptDescriptor {
	friend class GameScript;
protected:
	//! The name of the file that is being operating on.
	std::string _filename;
	//! Boolean value indicating whether a file is open or not.
	bool _file_open;
	//! A bit-mask that is used to set and detect various error conditions.
	uint32 _error_code;
	//! The names of the Lua tables that are currently opened for operations.
	std::vector<std::string> _open_tables;
	//! The file access mode for the current open table
	DATA_ACCESS_MODE _access_mode;
	//! The Lua stack, which handles all data sharing between C++ and Lua.
	lua_State *_lstack;
	//! Returns true if the file is open. Otherwise sets the error code.
	bool _IsFileOpen();
	//! The output file stream to write to.
	std::ofstream _outfile;
	//! Writes the pathname of all open tables (ie, table1[table2][table3])
	void _WriteTablePath();
public:
	ScriptDescriptor () { _filename = ""; _file_open = false; _error_code = 0; _lstack = NULL; }
	~ScriptDescriptor () {}
	
	/*! \brief Used to check if any error occured in previous operations.
	 *  \return A bit-mask value of all error conditions that have been detected.
	 *
	 *  It is good practice to call this function after chunks of function calls to the
	 *  children of this class (ReadScriptDescriptor, WriteScriptDescriptor) to detect if
	 *  anything went wrong. The bit-mask values returned by this function are all listed
	 *  in the constants called "Data Error Codes" at the top of this file. It is perfectly
	 *  acceptable (although maybe pedantic) to call this function after every function call 
	 *  to the children of this class.
	 *  
	 *  It is up to the API user to figure out how to recover when they detect an error condition.
	 *  The only thing that the code in the data management classes do are to prevent errors
	 *  from causing segmentation faults.
	 *
	 *  \note Everytime this function is called, the internal _error_code flag is cleared.
	 */
	uint32 GetError()
	{ uint32 code = _error_code; _error_code = DATA_NO_ERRORS; return code; }
	
	//! \name Class Member Access Functions
	//@{
	bool IsFileOpen() { return _file_open; }
	std::string GetFilename() { return _filename; }
	uint32 GetErrorCode() { return _error_code; }
	std::vector<std::string> &GetOpenTables() { return _open_tables; }
	//@}

	//! \name File access functions
	//! \brief Functions for opening and closing of readable Lua files.
	//@{
	//! \param file_name The name of the Lua file to be opened.
	//! \return False on failure.
	//! \note This is the only function that uses explicit error checking. In other words, an error in
	//! this function call will not change the return value of the GetError() function
	bool OpenFile(std::string file_name, DATA_ACCESS_MODE acess_mode);
	//! OpenFile() with no argument assumes the correct filename is already loaded in the class object.
	//! \return False on failure.
	//! \note This is the only function that uses explicit error checking. In other words, an error in
	//! this function call will not change the return value of the GetError() function
	bool OpenFile(DATA_ACCESS_MODE access_mode);
	void CloseFile();
	//@}

	/*! \name Variable Access Functions
	 *  \brief These functions grab a basic data type from the Lua file and return its value.
	 *  \param key The name of the Lua variable to access.
	 *  \param lang The two-letter language identifier for unicode string retrieval.
	 *  \note The integer keys are only valid for variables stored in a table, not for global variables.
	 */
	//@{
	bool ReadBool(std::string key)
	{ return Read<bool>(key.c_str(), false); }
	bool ReadBool(const int32 key)
	{ return Read<bool>(key, false); }
	int32 ReadInt(std::string key)
	{ return Read<int32>(key.c_str(), 0); }
	int32 ReadInt(const int32 key)
	{ return Read<int32>(key, 0); }
	float ReadFloat(std::string key)
	{ return Read<float>(key.c_str(), 0.0f); }
	float ReadFloat(const int32 key)
	{ return Read<float>(key, 0.0f); }
	std::string ReadString(std::string key)
	{ return Read<std::string>(key.c_str(), ""); }
	std::string ReadString(const int32 key)
	{ return Read<std::string>(key, ""); }
	hoa_utils::ustring ReadUString(std::string key)
	{ return Read<hoa_utils::ustring>(key.c_str(), hoa_utils::MakeUnicodeString("")); }
	hoa_utils::ustring ReadUString(const int32 key)
	{ return Read<hoa_utils::ustring>(key, hoa_utils::MakeUnicodeString("")); }

	template <class T> T Read(int32 key, T default_value)
	{
		if (_access_mode != READ)
			return false;

		if (!_IsFileOpen())
			return false;

		if (_open_tables.size() == 0)
		{
			// there needs to be a table
			if (SCRIPT_DEBUG)
				std::cerr << "SCRIPTDESCRIPTOR: No open tables to read from." << std::endl;
			_error_code = DATA_OPEN_TABLE_FAILURE;
			return default_value;
		}

		luabind::object o(luabind::from_stack(_lstack, private_script::STACK_TOP));
		if (luabind::type(o) != LUA_TTABLE)
		{
			// table not on top of stack
			if (SCRIPT_DEBUG)
				std::cerr << "SCRIPTDESCRIPTOR: Top of stack is not a table." << std::endl;
			_error_code = DATA_BAD_GLOBAL;
			return default_value;
		}

		try
		{
			return luabind::object_cast<T>(o[key]);
		}
		catch (...)
		{
			if (SCRIPT_DEBUG)
				std::cerr << "SCRIPTDESCRIPTOR: Unable to cast value to correct type." << std::endl;
			_error_code = DATA_INVALID_TABLE_KEY;
		}

		return default_value;
	}

	template <class T> T Read(const char *key, T default_value)
	{
		if (_access_mode != READ)
			return false;

		if (!_IsFileOpen())
			return false;

		// Global value
		if (_open_tables.size() == 0)
		{
			lua_getglobal(_lstack, key);
			luabind::object o(luabind::from_stack(_lstack, private_script::STACK_TOP));

			if (!o)
			{
				if (SCRIPT_DEBUG)
					std::cerr << "SCRIPTDESCRIPTOR: Unable to access global " << key << std::endl;
				_error_code = DATA_BAD_GLOBAL;
				return default_value;
			}

			try
			{
				T ret_val = luabind::object_cast<T>(o);
				lua_pop(_lstack, 1);
				return ret_val;
			}
			catch (...)
			{
				if (SCRIPT_DEBUG)
					std::cerr << "SCRIPTDESCRIPTOR: Unable to cast value to correct type." << std::endl;
				_error_code = DATA_BAD_GLOBAL;
			}
		}
		// there is an open table, get the key from the table
		else
		{
			luabind::object o(luabind::from_stack(_lstack, private_script::STACK_TOP));
			if (luabind::type(o) != LUA_TTABLE)
			{
				// table not on top of stack
				if (SCRIPT_DEBUG)
					std::cerr << "SCRIPTDESCRIPTOR: Top of stack is not a table." << std::endl;
				_error_code = DATA_BAD_GLOBAL;
				return default_value;
			}

			try
			{
				return luabind::object_cast<T>(o[key]);
			}
			catch (...)
			{
				if (SCRIPT_DEBUG)
					std::cerr << "SCRIPTDESCRIPTOR: Unable to cast value to correct type." << std::endl;
				_error_code = DATA_INVALID_TABLE_KEY;
			}
		}
		return default_value;
	}
	//@}
	
	/*! \name Lua Table Access Functions
	 *  \brief These functions open and close tables in the Lua file.
	 *  \param key The name of the table to open.
	 *  \return Zero if there are no table elements or there was an error.
	 *  
	 *  After a table is opened, it becomes the active "space" that all of the data read
	 *  operations operate on. You must \c always remember to close a table once you are
	 *  finished reading data from it.
	 *  
	 *  \note The version of OpenTable that uses an integer key will only work when there is
	 *  at least one table already open. In other words, we can't use a number to address a 
	 *  table that is in the global space.
	 */
	//@{
	void OpenTable(std::string key);
	void OpenTable(const int32 key);
	void CloseTable();
	uint32 GetTableSize(std::string key);
	uint32 GetTableSize(const int32 key);
	//! The function call with no arguments attempts to get the size of the most recently opened table.
	uint32 GetTableSize();
	//@}
	
	//! Read Data functions these can only be called when the access mode is READ
	/*! \name Vector Fill Functions
	 *  \brief These functions fill a vector with members of a table read from the Lua file.
	 *  \param key The name of the table to use to fill the vector.	 
	 *  \param vect A reference to the vector of elements to fill.
	 *  
	 *  The table that these functions attempt to access is assumed to be \c not currently
	 *  opened. If it is, these functions will not execute properly.
	 *  
	 *  \note The integer keys are only valid for tables that are elements of a parent table. 
	 *  They can not be used to access tables in the global space.
	 */
	//@{
	void FillIntVector(std::string key, std::vector<int32> &vect)
	{ FillVector<std::string, int32>(key, vect); }
	void FillFloatVector(std::string key, std::vector<float> &vect)
	{ FillVector<std::string, float>(key, vect); }
	void FillStringVector(std::string key, std::vector<std::string> &vect)
	{ FillVector<std::string, std::string>(key, vect); }
	void FillIntVector(const int32 key, std::vector<int32> &vect)
	{ FillVector<int32>(key, vect); }
	void FillFloatVector(const int32 key, std::vector<float> &vect)
	{ FillVector<float>(key, vect); }
	void FillStringVector(const int32 key, std::vector<std::string> &vect)
	{ FillVector<std::string>(key, vect); }

	template <class T> void FillVector(int32 key, std::vector<T> &vect)
	{
		// need at least one open table
		if (_open_tables.size() == 0)
		{
			if (SCRIPT_DEBUG)
				std::cerr << "SCRIPTDESCRIPTOR: Need at least one table open to use a numerical key." << std::endl;
			_error_code |= DATA_BAD_GLOBAL;
			return;
		}

		FillVector<int32, T>(key, vect);
	}

	template <class T, class U> void FillVector(T key, std::vector<U> &vect)
	{
		if (_access_mode != READ)
			return;

		if (!_IsFileOpen())
			return;

		OpenTable(key);
		// Grab the table off the stack
		luabind::object o(luabind::from_stack(_lstack, private_script::STACK_TOP));
		if (luabind::type(o) != LUA_TTABLE)
		{
			if (SCRIPT_DEBUG)
				std::cerr << "SCRIPTDESCRIPTOR: No table on top of stack, unable to continue." << std::endl;
			_error_code = DATA_INVALID_TABLE_KEY;
			return;
		}
		// We have a table loop through all items
		for (luabind::iterator it(o), end; it != end; it++)
		{
			try
			{
				vect.push_back(luabind::object_cast<U>((*it)));
			}
			catch (...)
			{
				if (SCRIPT_DEBUG)
					std::cerr << "SCRIPTDESCRIPTOR: Unable to cast value to correct type." << std::endl;
				_error_code = DATA_INVALID_TABLE_KEY;
			}
		}

		CloseTable();
	}
	//@}

	void DEBUG_PrintLuaStack();

	/*! \name Lua CommentWrite Functions
	 *  \brief Writes comments into a Lua file
	 *  \param comment The comment to write to the file.
	 *
	 *  \note All functions automatically insert and append a new line character before returning.
	 *  
	 *  \note There is no support for writing unicode string comments (yet). May be implemented in the
	 *  future.
	 */
	//@{
	//! Inserts a blank line into the text.
	void InsertNewLine();
	//! Writes a string of text and prepends it with a comment. Equivalent to `// comment` in C++.
	void WriteComment(const char* comment);
	void WriteComment(std::string& comment);
	//! After this function is invoked, every single function call will be a comment. Equivalent to `/*` in C++.
	void BeginCommentBlock();
	//! Ends a comment block. Equivalent to `*/` in C++
	void EndCommentBlock();
	/*! \brief Writes the plain string of text to the file with no modification, except for a newline.
	 *  \note Typically, unless you \c really know what you are doing, you should only call this between
	 *  the beginning and end of a comment block.
	 */
	void WriteLine(const char* comment);
	void WriteLine(std::string& comment);
	//@}
	
	//! Write Data functions
	/*! \name Lua Variable Write Functions
	 *  \brief These functions will write a single variable and its value to a Lua file.
	 *  \param *key The name of the Lua variable to write.
	 *  \param value The value of the new global variable to write.
	 *  \note If no write tables are open when these calls are made, the variables become global
	 *  in the Lua file. Otherwise, they become keys of the most recently opened table.
	 */
	//@{
	void WriteBool(const char *key, bool value);
	void WriteInt(const char *key, int32 value);
	void WriteFloat(const char *key, float value);
	void WriteString(const char *key, const char* value);
	void WriteString(const char *key, std::string& value);
	void WriteBool(const int32 key, bool value);
	void WriteInt(const int32 key, int32 value);
	void WriteFloat(const int32 key, float value);
	void WriteString(const int32 key, const char* value);
	void WriteString(const int32 key, std::string& value);
	//@}
	
	/*! \name Lua Table Write Functions
	 *  \brief These functions write Lua tables and their members
	 *  \param *key The name of the table to write.
	 *  \note If you begin a new table and then begin another when you haven't ended the first one, the
	 *  new table will become a key to the first. A table will only become global when there are no other
	 *  write tables open.
	 */
	//@{
	void BeginTable(const char *key);
	void EndTable();
	//@}
	
	/*! \name Lua Vector Write Functions
	 *  \brief These functions write a vector of data to a Lua file.
	 *  \param *key The name of the table to use in the Lua file to represent the data.
	 *  \param &vect A reference to the vector of elements to write.
	 */
	//@{
	void WriteBoolVector(const char *key, std::vector<bool> &vect);
	void WriteIntVector(const char *key, std::vector<int32> &vect);
	void WriteFloatVector(const char *key, std::vector<float> &vect);
	void WriteStringVector(const char *key, std::vector<std::string> &vect);
	//@}

}; // class ScriptDescriptor

/*!****************************************************************************
 *  \brief Singleton class that manages all open data files.
 *
 *  This class monitors all open data files and their descriptor objects.
 *
 *  \note 1) This class is a singleton
 *  
 *  \note 2) In the future, this class will manage all the open data files and
 *  make sure that no file is opened more than once at the same time.
 *****************************************************************************/
class GameScript {
	friend class ScriptDescriptor;
private:
	SINGLETON_DECLARE(GameScript);
	
	//! Maintains a list of all data files currently open.
	std::map<std::string, ScriptDescriptor*> _open_files;
	//! Global lua state
	lua_State *_global_state;
public:
	SINGLETON_METHODS(GameScript);
	
	//! Gets the global lua state
	lua_State *GetGlobalState()
	{ return _global_state; }

	//! Adds an open file to the list of open files
	void AddOpenFile(const ScriptDescriptor &sd);
	//! Removes an open file from the list of open files
	void RemoveOpenFile(const ScriptDescriptor &sd);
	//! Checks if a file is already in use by a ScriptDescriptor object.
	//! \param filename The name of the file to check.
	//! \return True if the filename is registered to a ScriptDescriptor object who has the file opeend.
	bool CheckOpenFile(std::string filename) { return false; }
}; // class GameScript

} // namespace hoa_script

#endif
