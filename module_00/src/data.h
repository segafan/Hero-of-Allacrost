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

#include <fstream>
extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}
#include "utils.h"
#include "defs.h"

//! All calls to the data engine are wrapped in this namespace.
namespace hoa_data {

//! The singleton pointer responsible for the interaction between the C++ engine and Lua scripts.
extern GameData *DataManager;
//! Determines whether the code in the hoa_data namespace should print debug statements or not.
extern bool DATA_DEBUG;

//! An internal namespace to be used only by the data engine itself. Don't use this namespace anywhere else!
namespace private_data {

//! For quick reference to the top of the Lua stack.
const int32 STACK_TOP = -1;

//! A list of Lua libraries to expose for use in scripts.
static const luaL_reg LUALIBS[] = {
	{"base", luaopen_base},
	{"table", luaopen_table},
	{"io", luaopen_io},
	{"string", luaopen_string},
	{"math", luaopen_math},
	{"debug", luaopen_debug},
	{"loadlib", luaopen_loadlib},
	{NULL, NULL} 
};

} // namespace private_data

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

/*!****************************************************************************
 *  \brief An abstract class for representing data files
 *
 *  This class serves as a parent class for readable and writeable data files.
 *  These data files are actually Lua scripts which are processed by the game
 *  engine. Files with a .lua extension are human-readable, uncompiled files
 *  and files with a .hoa extension are compiled files. 
 *
 *  \note 1) Compiled Lua files exhibit faster performance than uncompile files.
 *  
 *  \note 2) The members of this class can not be set within this class. The user
 *  may only modify the members of this class via the derived children classes.
 *****************************************************************************/
class DataDescriptor {
	friend class GameData;
protected:
	//! The name of the file that is being operating on.
	std::string _filename;
	//! Boolean value indicating whether a file is open or not.
	bool _file_open;
	//! A bit-mask that is used to set and detect various error conditions.
	uint32 _error_code;
	//! The names of the Lua tables that are currently opened for operations.
	std::vector<std::string> _open_tables;
public:
	DataDescriptor () { _filename = ""; _file_open = false; _error_code = 0; }
	~DataDescriptor () {}
	
	/*! \brief Used to check if any error occured in previous operations.
	 *  \return A bit-mask value of all error conditions that have been detected.
	 *
	 *  It is good practice to call this function after chunks of function calls to the
	 *  children of this class (ReadDataDescriptor, WriteDataDescriptor) to detect if
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
}; // class DataDescriptor

/*!****************************************************************************
 *  \brief Manager of readable Lua data scripts.
 *
 *  This class manages Lua files that have read privledges. Each class object
 *  maintains and manages its own Lua state.
 *
 *  \note 1) Compiled Lua files exhibit faster performance than uncompile files.
 *****************************************************************************/
class ReadDataDescriptor : public DataDescriptor {
	friend class GameData;
private:
	//! The Lua stack, which handles all data sharing between C++ and Lua.
	lua_State *_lstack;
	
	//! Returns true if the file is open. Otherwise sets the error code.
	bool _IsFileOpen();
public:
	ReadDataDescriptor () { _lstack = NULL; }
	~ReadDataDescriptor () {}
	
	//! \name File access functions
	//! \brief Functions for opening and closing of readable Lua files.
	//@{
	//! \param file_name The name of the Lua file to be opened.
	//! \return False on failure.
	//! \note This is the only function that uses explicit error checking. In other words, an error in
	//! this function call will not change the return value of the GetError() function
	bool OpenFile(const char* file_name);
	//! OpenFile() with no argument assumes the correct filename is already loaded in the class object.
	//! \return False on failure.
	//! \note This is the only function that uses explicit error checking. In other words, an error in
	//! this function call will not change the return value of the GetError() function
	bool OpenFile();
	void CloseFile();
	//@}
	
	/*! \name Variable Access Functions
	 *  \brief These functions grab a basic data type from the Lua file and return its value.
	 *  \param key The name of the Lua variable to access.
	 *  \param lang The two-letter language identifier for unicode string retrieval.
	 *  \note The integer keys are only valid for variables stored in a table, not for global variables.
	 */
	//@{
	bool ReadBool(const char *key);
	bool ReadBool(const int32 key);
	int32 ReadInt(const char *key);
	int32 ReadInt(const int32 key);
	float ReadFloat(const char *key);
	float ReadFloat(const int32 key);
	std::string ReadString(const char *key);
	std::string ReadString(const int32 key);
	hoa_utils::ustring ReadUString(const char *key, const char *lang);
	hoa_utils::ustring ReadUString(const int32 key, const char *lang);
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
	void OpenTable(const char *key);
	void OpenTable(const int32 key);
	void CloseTable();
	uint32 GetTableSize(const char *key);
	uint32 GetTableSize(const int32 key);
	//! The function call with no arguments attempts to get the size of the most recently opened table.
	uint32 GetTableSize();
	//@}
	
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
	void FillIntVector(const char *key, std::vector<int32> &vect);
	void FillFloatVector(const char *key, std::vector<float> &vect);	
	void FillStringVector(const char *key, std::vector<std::string> &vect);
	void FillIntVector(const int32 key, std::vector<int32> &vect);
	void FillFloatVector(const int32 key, std::vector<float> &vect);
	void FillStringVector(const int32 key, std::vector<std::string> &vect);
	//@}
	
	/*! \name Lua Function Calling Wrapper
	 *  \brief This function allows you to call arbitrary Lua functions from C++
	 *  \param *func a string containing the name of the Lua function to be called
	 *  \param *sig a string describing arguments and results. See below for explanation.
	  
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
	void CallFunction(const char *func, const char *sig, ...);
	
	/*! \name Lua<->C++ binding functions
	 *  \brief Functions used to bind C++ class member functions and objects to be used directly from Lua.
	 *  \param *obj the object to be registered (see notes below)
	 *  \param *objname the name by which the object will be referred from Lua
	 *  \param func a pointer to the function to be registered
	 *  \param *funcname the name by which the function will be referred to from Lua
	  
	    Calling protocol:
	    1. RegisterClassStart()
	    2. RegisterMemberFunction(...), and/or RegisterObject(...), in any order.
	    3. RegisterClassEnd()
	    
	    The RegisterFunction() function can be called at any time, independently of the other functions
	    from this group.
	    
	    Notes on usage:
	    
	    - When calling RegisterMemberFunction, the second parameter (Class *obj) should be specified like this:
	        (MyClass*)0
	      For example:
	      	RegisterMemberFunction("GetSomething", (MyClass*)0, &MyClass::GetSomething);
	      Why add that argument when it's not used, you ask? Well, LuaPlus needs that cast because of it's
	      template magic, and that's the easy way to let it know what class it's working with.
	      But, when calling RegisterObject(), you specify a valid object pointer (don't forget the cast).
	    
	    - One RegisterClassStart/RegisterClassEnd block creates one Lua metatable and uses it to assign C++ stuff.
	      What this means is that if you register all the members of a class and, say, one object of that class
	      inside one RCS/RCE block, and then make a new block which just registers another object of the
	      same class, that won't work, because that object will be assigned a NEW (and empty) metatable.
	      The solution: either register all the objects you need inside one RCS/RCE block, or re-register all the
	      class methods the object needs inside the second block too. This can be useful, actually.
	      See the HTML documentation for a more detailed explanation. 
	*/ 
	//@{
	void RegisterClassStart();
	void RegisterClassEnd();
	template <typename Class, typename Function>
	void RegisterMemberFunction(const char *funcname, Class *obj, Function func);
	template <typename Class>
	void RegisterObject(const char *objname, Class *obj);
	template <typename Function>
	void RegisterFunction(const char* funcname, Function func);
	//@}
	
	//! Prints the current contents of the Lua stack
	void DEBUG_PrintLuaStack();
}; // class ReadDataDescriptor


/*!****************************************************************************
 *  \brief Manages writing to Lua files.
 *
 *  This class manages the creation and construction of new Lua files. Currently
 *  it is only capable of writing comments, global values, and tables, but may
 *  have more functionality included in the future, such as writing functions.
 *
 *  \note 1) There is currently no support for modifying or appending to existing
 *  files. This may be added in the future (or it may become a new class).
 *  
 *  \note 2) We may also provide a function to compile the written Lua file as well.
 *  
 *  \note 3) You are allowed to assign the same key a different value twice (ie first
 *  declaring that a = 0, then later declaring that a = "pizza". There is no error
 *  checking for this condition, so just be sure that you know what you are doing.
 *****************************************************************************/
class WriteDataDescriptor : public DataDescriptor {
	friend class GameData;
private:
	//! The output file stream to write to.
	std::ofstream _outfile;
	
	//! \brief Checks if the output file for writing is open. 
	//! Used by all the standard write functions to avoid segmentation faults.
	//! \return True if the output file is open.
	bool _IsFileOpen();
	//! Writes the pathname of all open tables (ie, table1[table2][table3])
	void _WriteTablePath();
public:
	WriteDataDescriptor () {}
	~WriteDataDescriptor () {}
	
	/*! \name Lua File Write Functions
	 *  \brief Opens files for write priveledges.
	 *  \param file_name The name of the Lua file to be opened.
	 *  \note These functions are only for overwriting existing files or writing new files. You can
	 *   not append to or modify existing Lua files with these functions.
	 */
	//@{
	bool OpenFile(const char* file_name);
	bool OpenFile();
	void CloseFile();
	//@}
	
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
}; // class WriteDataDescriptor


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
class GameData {
	friend class ReadDataDescriptor;
	friend class WriteDataDescriptor;
private:
	SINGLETON_DECLARE(GameData);
	
	//! Maintains a list of all data files currently open.
	std::map<std::string, DataDescriptor*> _open_files;
public:
	SINGLETON_METHODS(GameData);
	
	//! Checks if a file is already in use by a DataDescriptor object.
	//! \param filename The name of the file to check.
	//! \return True if the filename is registered to a DataDescriptor object who has the file opeend.
	bool CheckOpenFile(std::string filename) { return false; }
}; // class GameData

} // namespace hoa_data

#endif
