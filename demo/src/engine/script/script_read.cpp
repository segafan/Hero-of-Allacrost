///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    script_read.cpp
*** \author  Daniel Steuernol - steu@allacrost.org,
***          Tyler Olsen - roots@allacrost.org
*** \brief   Source file for the ReadScriptDescriptor class.
*** ***************************************************************************/

#include "utils.h"

#include "script.h"
#include "script_read.h"

using namespace std;
using namespace luabind;

using namespace hoa_utils;
using namespace hoa_script::private_script;

namespace hoa_script {

ReadScriptDescriptor::~ReadScriptDescriptor() {
	if (IsFileOpen()) {
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT WARNING: ReadScriptDescriptor destructor was called when file was still open: "
				<< _filename << endl;
		CloseFile();
	}
	
	_filename = "";
	_access_mode = SCRIPT_CLOSED;
	_error_messages.clear();
	_open_tables.clear();
}

//-----------------------------------------------------------------------------
// File Access Functions
//-----------------------------------------------------------------------------

bool ReadScriptDescriptor::OpenFile(const string& file_name) {
	if (ScriptManager->IsFileOpen(file_name) == true) {
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT WARNING: ReadScriptDescriptor::OpenFile() attempted to open file that is already opened: "
				<< file_name << endl;
		return false;
	}

	
	// Check if this file was opened previously.
	if ((_lstack = ScriptManager->_CheckForPreviousLuaState(file_name)) == NULL)
	{	
		// Increases the global stack size by 1 element. That is needed because the new thread will be pushed in the
		// stack and we have to be sure there is enough space there.
		lua_checkstack(ScriptManager->GetGlobalState(),1);
		_lstack = lua_newthread(ScriptManager->GetGlobalState());

		// Attempt to load and execute the Lua file.
		if (luaL_loadfile(_lstack, file_name.c_str()) != 0 || lua_pcall(_lstack, 0, 0, 0)) {
			cerr << "SCRIPT ERROR: ReadScriptDescriptor::OpenFile() could not open the file " << file_name << endl;
			cerr << lua_tostring(_lstack, private_script::STACK_TOP) << endl;
			_access_mode = SCRIPT_CLOSED;
			return false;
		}
	}

	_filename = file_name;
	_access_mode = SCRIPT_READ;
	ScriptManager->_AddOpenFile(this);
	return true;
} // bool ReadScriptDescriptor::OpenFile(string file_name)



bool ReadScriptDescriptor::OpenFile() {
	if (_filename == "") {
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT ERROR: ReadScriptDescriptor::OpenFile(), could not open file "
				<< "because of an invalid file name (empty string)." << endl;
		return false;
	}

	return OpenFile(_filename);
}



void ReadScriptDescriptor::CloseFile() {
	if (IsFileOpen() == false) {
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT ERROR: ReadScriptDescriptor::CloseFile() could not close the "
				<< "file because it was not open." << endl;
		return;
	}

	if (SCRIPT_DEBUG && IsErrorDetected()) {
		cerr << "SCRIPT WARNING: In ReadScriptDescriptor::CloseFile(), the file " << _filename
			<< " had error messages remaining. They are as follows:" << endl;
		cerr << _error_messages.str() << endl;
	}

	_lstack = NULL;
	_error_messages.clear();
	_open_tables.clear();
	_access_mode = SCRIPT_CLOSED;
	ScriptManager->_RemoveOpenFile(this);
}

//-----------------------------------------------------------------------------
// Existence Checking Functions
//-----------------------------------------------------------------------------

bool ReadScriptDescriptor::_DoesDataExist(const string& key, int32 type) {
	// Check whether the user is trying to read a global variable or one stored in a table
	if (_open_tables.size() == 0) { // Variable is a global
		lua_getglobal(_lstack, key.c_str());
		luabind::object o(luabind::from_stack(_lstack, private_script::STACK_TOP));
		return _CheckDataType(type, o);
	}

	else { // Variable is a member of a table
		luabind::object o(luabind::from_stack(_lstack, private_script::STACK_TOP));
		if (luabind::type(o) != LUA_TTABLE) {
			_error_messages << "* _DoesDataExist() failed because the top of the stack was not "
				<< "a table when trying to check for the table member: " << key << endl;
			return false;
		}

		luabind::object obj(o[key]);
		return _CheckDataType(type, obj);
	}
}



bool ReadScriptDescriptor::_DoesDataExist(int32 key, int32 type) {
	if (_open_tables.size() == 0) {
		_error_messages << "* _DoesDataExist() failed because no tables were open when trying to "
			<< "examine the table member: " << key << endl;
		return false;
	}

	luabind::object o(luabind::from_stack(_lstack, private_script::STACK_TOP));
	if (luabind::type(o) != LUA_TTABLE) {
		_error_messages << "* _DoesDataExist() failed because the top of the stack was not "
			<< "a table when trying to check for the table member: " << key << endl;
		return false;
	}

	luabind::object obj(o[key]);
	return _CheckDataType(type, obj);
}



bool ReadScriptDescriptor::_CheckDataType(int32 type, luabind::object& obj_check) {
	int32 object_type = luabind::type(obj_check);

	if (obj_check.is_valid() == false)
		return false;

	// When this type is passed to the function, we don't care what type the object is as long
	// as it was seen to be something
	if (type == LUA_TNIL) {
		return true;
	}

	// Simple type comparison is all that is needed for all non-numeric types
	else if (type == object_type) {
		return true;
	}

	// Because Lua only has a "number" type, we have to do perform a special cast 
	// to examine integer versus floating point types
	else if (object_type == LUA_TNUMBER) {
		if (type == INTEGER_TYPE) {
			try {
				luabind::object_cast<int32>(obj_check);
				lua_pop(_lstack, 1);
				return true;
			}
			catch (...) {
				return false;
			}
		}
		else if (type == UINTEGER_TYPE) {
			try {
				luabind::object_cast<uint32>(obj_check);
				lua_pop(_lstack, 1);
				return true;
			}
			catch (...) {
				return false;
			}
		}
		else if (type == FLOAT_TYPE) {
			try {
				luabind::object_cast<float>(obj_check);
				lua_pop(_lstack, 1);
				return true;
			}
			catch (...) {
				return false;
			}
		}
		else {
			return false;
		}
	}

	else {
		return false;
	}
} // bool ReadScriptDescriptor::_CheckDataType(int32 type, luabind::object& obj_check)

//-----------------------------------------------------------------------------
// Function Pointer Read Functions
//-----------------------------------------------------------------------------

object ReadScriptDescriptor::ReadFunctionPointer(const string& key) {
	if (_open_tables.size() == 0) { // The function should be in the global space
		lua_getglobal(_lstack, key.c_str());

		luabind::object o(from_stack(_lstack, STACK_TOP));

		if (!o) {
			_error_messages << "* ReadFunctionPointer() failed because it was unable to access the function "
				<< "for the global key: " << key << endl;
			return luabind::object();
		}

		if (type(o) != LUA_TFUNCTION) {
			_error_messages << "* ReadFunctionPointer() failed because the data retrieved was not a function "
				<< "for the global key: " << key << endl;
			return luabind::object();
		}

		return o;
	}

	else { // The function should be an element of the most recently opened table
		luabind::object o(from_stack(_lstack, STACK_TOP));
		if (type(o) != LUA_TTABLE) {
			_error_messages << "* ReadFunctionPointer() failed because the top of the stack was not a table "
				<< "for the table element key: " << key << endl;
			return luabind::object();
		}
	
		if (type(o[key]) != LUA_TFUNCTION) {
			_error_messages << "* ReadFunctionPointer() failed because the data retrieved was not a function "
				<< "for the table element key: " << key << endl;
			return luabind::object();
		}
		
		return o[key];
	}
} // object ReadScriptDescriptor::ReadFunctionPointer(string key)



object ReadScriptDescriptor::ReadFunctionPointer(int32 key) {
	// Fucntion is always a table element for integer keys
	luabind::object o(from_stack(_lstack, STACK_TOP));
	if (type(o) != LUA_TTABLE) {
		_error_messages << "* _ReadFunctionPointer() failed because the top of the stack was not a table "
			<< "for the table element key: " << key << endl;
		return o;
	}

	if (type(o[key]) != LUA_TFUNCTION) {
		_error_messages << "* _ReadFunctionPointer() failed because the data retrieved was not a function "
			<< "for the table element key: " << key << endl;
		return o;
	}

	return o[key];
} // object ReadScriptDescriptor::ReadFunctionPointer(int32 key)

//-----------------------------------------------------------------------------
// Table Operation Functions
//-----------------------------------------------------------------------------

void ReadScriptDescriptor::OpenTable(const string& table_name) {
	if (_open_tables.size() == 0) { // Fetch the table from the global space
		lua_getglobal(_lstack, table_name.c_str());
		if (!lua_istable(_lstack, STACK_TOP)) {
			_error_messages << "* OpenTable() failed because the data retrieved was not a table "
				<< "or did not exist for the global key " << table_name << endl;
			return;
		}
		_open_tables.push_back(table_name);
	}

	else { // The table to fetch is an element of another table
		lua_pushstring(_lstack, table_name.c_str());
		lua_gettable(_lstack, STACK_TOP - 1);
		if (!lua_istable(_lstack, STACK_TOP)) {
			_error_messages << "* OpenTable() failed because the data retrieved was not a table "
				<< "or did not exist for the table element key " << table_name << endl;
			return;
		}
		_open_tables.push_back(table_name);
	}
} // void ReadScriptDescriptor::OpenTable(string key)



void ReadScriptDescriptor::OpenTable(int32 table_name) {
	// At least one table must be open to use a numerical key
	if (_open_tables.size() == 0) {
		_error_messages << "* OpenTable() failed because there were no tables open when trying "
				<< "to open the with the element key " << table_name << endl;
		return;
	}
	
	lua_pushnumber(_lstack, table_name);
	lua_gettable(_lstack, STACK_TOP - 1);
	if (!lua_istable(_lstack, STACK_TOP)) {
		_error_messages << "* OpenTable() failed because the data retrieved was not a table "
				<< "or did not exist for the table element key " << table_name << endl;
		return;
	}

	_open_tables.push_back(NumberToString(table_name));
} // void ReadScriptDescriptor::OpenTable(int32 key)



void ReadScriptDescriptor::CloseTable() {
	if (_open_tables.size() == 0) {
		_error_messages << "* CloseTable() failed because there were no open tables to close" << endl;
		return;
	}

	_open_tables.pop_back();
	lua_pop(_lstack, 1);
}



void ReadScriptDescriptor::CloseAllTables() {
	while (_open_tables.size() != 0) {
		CloseTable();
	}
}



uint32 ReadScriptDescriptor::GetTableSize(const string& table_name) {
	uint32 size = 0;

	OpenTable(table_name);
	size = GetTableSize();
	CloseTable();
	return size;
}



uint32 ReadScriptDescriptor::GetTableSize(int32 table_name) {
	uint32 size = 0;

	OpenTable(table_name);
	size = GetTableSize();
	CloseTable();

	return size;
}


// Attempts to get the size of the most recently opened table
uint32 ReadScriptDescriptor::GetTableSize() {
	if (_open_tables.size() == 0) {
		_error_messages << "* GetTableSize() failed because there were no open tables to get the size of" << endl;
		return 0;
	}

	return static_cast<uint32>(luaL_getn(_lstack, STACK_TOP));
}

//-----------------------------------------------------------------------------
// Miscellaneous Functions
//-----------------------------------------------------------------------------

void ReadScriptDescriptor::DEBUG_PrintLuaStack() {
	int32 type; // a variable to temporarily hold the type of Lua data read

	cout << "SCRIPT DEBUG: Printing script's lua stack:" << endl;
	for (int32 i = lua_gettop(_lstack); i > 0; i--) {  // Print each element starting from the top of the stack
		type = lua_type(_lstack, i);
		switch (type) {
			case LUA_TNIL:
				cout << "* " << i << "= NIL" << endl;
				break;
			case LUA_TBOOLEAN:
				cout << "* " << i << "= BOOLEAN: " << lua_toboolean(_lstack, i) << endl;
				break;
			case LUA_TNUMBER:
				cout << "* " << i << "= NUMBER:  " << lua_tonumber(_lstack, i) << endl;
				break;
			case LUA_TSTRING:
				cout << "* " << i << "= STRING:  " << lua_tostring(_lstack, i) << endl;
				break;
			case LUA_TTABLE:
				cout << "* " << i << "= TABLE" << endl;
				break;
			case LUA_TFUNCTION:
				cout << "* " << i << "= FUNCTION" << endl;
				break;
			case LUA_TUSERDATA:
				cout << "* " << i << "= USERDATA " << endl;
				break;
			case LUA_TLIGHTUSERDATA:
				cout << "* " << i << "= LIGHTUSERDATA " << endl;
				break;
			case LUA_TTHREAD:
				cout << "* " << i << "= THREAD " << endl;
				break;
			default:
				cout << "* " << i << "= OTHER: " << lua_typename(_lstack, type) << endl;
				break;
		}
	}
	cout << endl;
} // void ReadScriptDescriptor::DEBUG_PrintLuaStack()



void ReadScriptDescriptor::DEBUG_PrintGlobals() {
	cout << "SCRIPT DEBUG: Printing script's global variables:" << endl;

	object o(from_stack(_lstack, LUA_GLOBALSINDEX));
	for (luabind::iterator it(o), end; it != end; ++it) {
		cout << it.key() << " = " << (*it) << " ::: data type = " << type(*it) << endl;
		if (luabind::type(*it) == LUA_TTABLE) {
			if (object_cast<string>(it.key()) != "_G")
				DEBUG_PrintTable(object(*it), 1);
		}
	}
	cout << endl;
}


void ReadScriptDescriptor::DEBUG_PrintTable(object table, int tab)
{
	for (luabind::iterator it(table), end; it != end; ++it)
	{
		for (int i = 0; i < tab; ++i)
			cout << '\t';
		cout << it.key() << " = " << (*it) << " (Type: " << type(*it) << ")" << endl;
		if (type(*it) == LUA_TTABLE)
			DEBUG_PrintTable(object(*it), tab + 1);
	}
}

} // namespace hoa_script
