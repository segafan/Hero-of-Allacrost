///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    script.cpp
*** \author  Daniel Steuernol, steu@allacrost.org
***          Tyler Olsen, roots@allacrost.org
***          Vladimir Mitrovic, snipe714@allacrost.org
***
*** \brief   Source file for scripting engine.
*** ***************************************************************************/

#include <iostream>
#include <stdarg.h>
#include <boost/lexical_cast.hpp>

#include "script.h"

using namespace std;
using namespace luabind;

using namespace hoa_utils;
using namespace hoa_script::private_script;



template<> hoa_script::GameScript* Singleton<hoa_script::GameScript>::_singleton_reference = NULL;

namespace hoa_script {

GameScript* ScriptManager = NULL;
bool SCRIPT_DEBUG = false;

// *****************************************************************************
// ***************************** ScriptDescriptor ******************************
// *****************************************************************************

bool ScriptDescriptor::_CheckFileAccess(SCRIPT_ACCESS_MODE mode) {
	if (_access_mode == mode) {
		return true;
	}
	else {
		_error_code |= SCRIPT_BAD_FILE_ACCESS;
		if (SCRIPT_DEBUG) {
			if (_access_mode == SCRIPT_CLOSED)
				cerr << "SCRIPT ERROR: Attempted to operate on un-opened file " << _filename << endl;
			else
				cerr << "SCRIPT ERROR: Invalid permissions on file " << _filename << endl;
		}
		return false;
	}
}



bool ScriptDescriptor::OpenFile(const std::string& file_name, SCRIPT_ACCESS_MODE mode) {
	if (ScriptManager->IsFileOpen(file_name) == true) {
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT WARNING: Attempted to open file that is already opened: " << file_name << endl;
		return false;
	}

	// Case for opening with read permissions
	if (mode == SCRIPT_READ) {
		// Needs a comment: what does this do exactly?
		_lstack = lua_newthread(ScriptManager->GetGlobalState());

		// Attempt to load the Lua file.
		if (lua_dofile(_lstack, file_name.c_str())) {
			cerr << "SCRIPT ERROR: Could not load file " << file_name << " for reading. " << endl;
			_filename = "";
			_access_mode = SCRIPT_CLOSED;
			return false;
		}
		else {
			_filename = file_name;
			_access_mode = SCRIPT_READ;
			ScriptManager->_AddOpenFile(this);
			return true;
		}
	}

	// Case for opening with write permissions
	else if (mode == SCRIPT_WRITE) {
		_outfile.open(file_name.c_str());

		if (!_outfile) {
			cerr << "SCRIPT ERROR: Failed to open file " << _filename << " for writing." << endl;
			_filename = "";
			_access_mode = SCRIPT_CLOSED;
			return false;
		}
		else {
			_filename = file_name;
			_access_mode = SCRIPT_WRITE;
			ScriptManager->_AddOpenFile(this);
			return true;
		}
	}

	return false;
} // bool ScriptDescriptor::OpenFile(std::string file_name, SCRIPT_ACCESS_MODE mode)



bool ScriptDescriptor::OpenFile(SCRIPT_ACCESS_MODE mode) {
	if (_filename == "") {
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT ERROR: Could not open file because of invalid file name (null string)" << endl;
		return false;
	}
	return OpenFile(_filename, mode);
}



void ScriptDescriptor::CloseFile() {
	_open_tables.clear();

	if (_access_mode == SCRIPT_CLOSED) {
		cerr << "SCRIPT WARNING: Tried to close an output file when nothing was open" << endl;
		return;
	}
	else if (_access_mode == SCRIPT_READ) {
		_lstack = NULL;
		_access_mode = SCRIPT_CLOSED;
	}
	else if (_access_mode == SCRIPT_WRITE) {
		_outfile.close();
		_access_mode = SCRIPT_CLOSED;
	}
	ScriptManager->_RemoveOpenFile(this);
}


// This function is for DEBUGGING PURPOSES ONLY! It prints the contents of the Lua stack from top to bottom.
void ScriptDescriptor::DEBUG_PrintLuaStack() {
	if (_CheckFileAccess(SCRIPT_READ) == false)
		return;

	int32 type;
	cout << "SCRIPT DEBUG: Printing lua stack" << endl;
	for (int32 i = lua_gettop(_lstack); i > 0; i--) {  // Print each element starting from the top
		type = lua_type(_lstack, i);
		switch (type) {
		case LUA_TNIL:
			cout << i << ": NIL" << endl;
			break;
		case LUA_TBOOLEAN:
			cout << i << ": BOOLEAN: " << lua_toboolean(_lstack, i) << endl;
			break;
		case LUA_TNUMBER:
			cout << i << ": NUMBER:  " << lua_tonumber(_lstack, i) << endl;
			break;
		case LUA_TSTRING:
			cout << i << ": STRING:  " << lua_tostring(_lstack, i) << endl;
			break;
		case LUA_TTABLE:
			cout << i << ": TABLE    " << endl;
			break;
		case LUA_TFUNCTION:
			cout << i << ": FUNCTION " << endl;
			break;
		case LUA_TUSERDATA:
			cout << i << ": USERDATA " << endl;
			break;
		case LUA_TLIGHTUSERDATA:
			cout << i << ": LIGHTUSERDATA " << endl;
			break;
		case LUA_TTHREAD:
			cout << i << ": THREAD " << endl;
			break;
		default:
			cout << "OTHER:   " << lua_typename(_lstack, type) << endl;
			break;
		}
	}
} // void ScriptDescriptor::DEBUG_PrintLuaStack()


void ScriptDescriptor::DEBUG_ShowGlobals() {
	cout << "SCRIPT DEBUG: Writing out globals." << endl;
	object o(from_stack(_lstack, LUA_GLOBALSINDEX));
	for (luabind::iterator it(o), end; it != end; ++it)
	{
		cout << it.key() << " = " << (*it) << "   TYPE: " << type(*it) << endl;
		if (luabind::type(*it) == LUA_TTABLE) {
			// TODO: what needs to go here? Printing out the table contents?
		}
	}
}

// ****************************** Read Functions *******************************

void ScriptDescriptor::ReadOpenTable(const std::string& key) {
	if (_CheckFileAccess(SCRIPT_READ) == false)
		return;

	if (_open_tables.size() == 0) { // Fetch the table from the global space
		lua_getglobal(_lstack, key.c_str());
		if (!lua_istable(_lstack, STACK_TOP)) {
			cerr << "SCRIPT ERROR: could not retrieve table \"" << key << "\"" << endl;
			_error_code |= SCRIPT_OPEN_TABLE_FAILURE;
			return;
		}
		_open_tables.push_back(key);
	}

	else { // The table to fetch is an element of another table
		lua_pushstring(_lstack, key.c_str());
		lua_gettable(_lstack, STACK_TOP - 1);
		if (!lua_istable(_lstack, STACK_TOP)) {
			cerr << "SCRIPT ERROR: could not retreive sub-table using string key " << key << endl;
			_error_code |= SCRIPT_OPEN_TABLE_FAILURE;
			return;
		}
		_open_tables.push_back(key);
	}
} // void ScriptDescriptor::ReadOpenTable(std::string key)



void ScriptDescriptor::ReadOpenTable(const int32 key) {
	if (_CheckFileAccess(SCRIPT_READ) == false)
		return;

	// At least one table must be open to use a numerical key
	if (_open_tables.size() == 0) {
		_error_code |= SCRIPT_BAD_GLOBAL;
		return;
	}

	lua_pushnumber(_lstack, key);
	lua_gettable(_lstack, STACK_TOP - 1);
	if (!lua_istable(_lstack, STACK_TOP)) {
		cerr << "SCRIPT ERROR: could not retreive sub-table using integer key " << key << endl;
		_error_code |= SCRIPT_OPEN_TABLE_FAILURE;
		return;
	}

	_open_tables.push_back(NumberToString(key));
} // void ScriptDescriptor::OpenTable(const int32 key)



void ScriptDescriptor::ReadCloseTable() {
	if (_CheckFileAccess(SCRIPT_READ) == false)
		return;

	if (_open_tables.size() == 0) {
		_error_code |= SCRIPT_CLOSE_TABLE_FAILURE;
		return;
	}

	_open_tables.pop_back();
	lua_pop(_lstack, 1);
}



uint32 ScriptDescriptor::ReadGetTableSize(const std::string& key) {
	if (_CheckFileAccess(SCRIPT_READ) == false)
		return 0;

	uint32 size = 0;
	uint32 error_save = _error_code; // Temporarily save the error code
	_error_code = SCRIPT_NO_ERRORS;
	ReadOpenTable(key);
	// Only grab the size if the operation was successful.
	if (_error_code == SCRIPT_NO_ERRORS) {
		size = static_cast<uint32>(luaL_getn(_lstack, STACK_TOP));
		ReadCloseTable();
	}
	// Restore the error code along with the new errors from this call (if any) and return zero.
	_error_code = _error_code | error_save;
	return size;
}



uint32 ScriptDescriptor::ReadGetTableSize(const int32 key) {
	if (_CheckFileAccess(SCRIPT_READ) != true)
		return 0;

	uint32 error_save = _error_code; // Temporarily save the error code
	uint32 size = 0;

	_error_code = SCRIPT_NO_ERRORS;
	ReadOpenTable(key);
	// Only grab the size if the operation was successful.
	if (_error_code == SCRIPT_NO_ERRORS) {
		size = static_cast<uint32>(luaL_getn(_lstack, STACK_TOP));
		ReadCloseTable();
	}
	// Restore the error code along with the new errors from this call (if any) and return zero.
	_error_code = _error_code | error_save;
	return size;
}



uint32 ScriptDescriptor::ReadGetTableSize() {
	if (_CheckFileAccess(SCRIPT_READ) != true)
		return 0;

	// Attempt to get the size of the most recently opened table
	if (_open_tables.size() == 0) {
		return 0;
	}
	return static_cast<uint32>(luaL_getn(_lstack, STACK_TOP));
}



object ScriptDescriptor::ReadFunctionPointer(const std::string& key) {
	if (_CheckFileAccess(SCRIPT_READ) == false)
		return luabind::object();

	// Key is a global value if true
	if (_open_tables.size() == 0) {
		lua_getglobal(_lstack, key.c_str());

		luabind::object o(from_stack(_lstack, STACK_TOP));

		if (!o) {
			if (SCRIPT_DEBUG)
				cerr << "SCRIPT DESCRIPTOR: Unable to access global " << key << endl;
			_error_code |= SCRIPT_BAD_GLOBAL;
			return luabind::object();
		}

		if (type(o) != LUA_TFUNCTION) {
			if (SCRIPT_DEBUG)
				cerr << "SCRIPT DESCRIPTOR: Unexpected type for retrieved value " << key << endl;
			_error_code |= SCRIPT_BAD_TYPE;
			return luabind::object();
		}

		return o;
	}

	// There is an open table, get the key from the table
	else {
		luabind::object o(from_stack(_lstack, STACK_TOP));
		if (type(o) != LUA_TTABLE) {
			// Table not on top of stack
			if (SCRIPT_DEBUG)
				cerr << "SCRIPT DESCRIPTOR: Top of stack is not a table." << endl;
			_error_code |= SCRIPT_BAD_GLOBAL;
			return luabind::object();
		}
	
		if (type(o[key]) != LUA_TFUNCTION) {
			if (SCRIPT_DEBUG)
				cerr << "SCRIPT DESCRIPTOR: Unexpected type for retrieved value " << key << endl;
			_error_code |= SCRIPT_BAD_TYPE;
			return luabind::object();
		}
		
		return o[key];
	}
} // object ScriptDescriptor::ReadFunctionPointer(string key)



object ScriptDescriptor::ReadFunctionPointer(int32 key) {
	if (_CheckFileAccess(SCRIPT_READ) == false)
		return luabind::object();

	// Key is always a table element
	luabind::object o(from_stack(_lstack, STACK_TOP));
	if (type(o) != LUA_TTABLE) {
		// Table not on top of stack
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT DESCRIPTOR: Top of stack is not a table." << endl;
		_error_code |= SCRIPT_BAD_GLOBAL;
		return o;
	}

	if (type(o[key]) != LUA_TFUNCTION) {
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT DESCRIPTOR: Unexpected type for retrieved value " << key << endl;
		_error_code |= SCRIPT_BAD_TYPE;
		return o;
	}

	return o[key];
} // object ScriptDescriptor::ReadFunctionPointer(int32 key)

// ***************************** Write Functions *******************************

// Writes the "path" for all the open tables. Ex) table01[table02][table03]
void ScriptDescriptor::_WriteTablePath() {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	// No error checking is done since _IsWriteTableOpen is always called before it to make sure that
	// at least one table is open, and the functions that call this function always check that _outfile
	// is valid.
	_outfile << _open_tables[0];
	for (uint32 i = 1; i < _open_tables.size(); i++) {
		_outfile << '[' << _open_tables[i] << ']';
	}
}



void ScriptDescriptor::WriteInsertNewLine() {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	_outfile << endl;
}




void ScriptDescriptor::WriteComment(const std::string& comment) {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	_outfile << "-- " << comment << endl;
}



void ScriptDescriptor::WriteBeginCommentBlock() {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	_outfile << "--[[" << endl;
}



void ScriptDescriptor::WriteEndCommentBlock() {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	_outfile << "--]]" << endl;
}




void ScriptDescriptor::WriteLine(const std::string& comment) {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	_outfile << comment << endl;
}


// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
void ScriptDescriptor::WriteBool(const std::string &key, bool value) {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	if (_open_tables.size() == 0) {
		_outfile << key << " = ";
		if (value)
			_outfile << "true" << endl;
		else
			_outfile << "false" << endl;
		//lua_getglobal(_lstack, key);
		//if (lua_type(_lstack, STACK_TOP) == LUA_TNIL)
		//{
		//	// global does not exist, add to stack
		//	lua_pushboolean(_lstack, value);
		//	lua_setglobal(_lstack, key);
		//}
	}
	else {
		_WriteTablePath();
		_outfile << '[' << key << ']' << " = ";
		if (value)
			_outfile << "true" << endl;
		else
			_outfile << "false" << endl;
	}
}


// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
void ScriptDescriptor::WriteBool(const int32 key, bool value) {
	if (_open_tables.empty()) {
		_error_code |= SCRIPT_BAD_GLOBAL;
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT ERROR: Attempt to write a numerical value as a global key" << endl;
		return;
	}

	WriteBool(boost::lexical_cast<std::string>(key), value);
}



// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
void ScriptDescriptor::WriteInt(const std::string &key, int32 value) {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	if (_open_tables.size() == 0) {
		_outfile << key << " = " << value << endl;
	}
	else {
		_WriteTablePath();
		_outfile << '[' << key << ']' << " = " << value << endl;
	}
}


// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
void ScriptDescriptor::WriteInt(const int32 key, int32 value) {
	if (_open_tables.empty()) {
		_error_code |= SCRIPT_BAD_GLOBAL;
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT ERROR: Attempt to write a numerical value as a global key" << endl;
		return;
	}

	WriteInt(boost::lexical_cast<std::string>(key), value);
}



// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
void ScriptDescriptor::WriteFloat(const std::string &key, float value) {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	if (_open_tables.size() == 0) {
		_outfile << key << " = " << value << endl;
	}

	else {
		_WriteTablePath();
		_outfile << '[' << key << ']' << " = " << value << endl;
	}
}


// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
void ScriptDescriptor::WriteFloat(const int32 key, float value) {
	if (_open_tables.empty()) {
		_error_code |= SCRIPT_BAD_GLOBAL;
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT ERROR: Attempt to write a numerical value as a global key" << endl;
		return;
	}

	WriteFloat(boost::lexical_cast<std::string>(key),value);
}


// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
// TODO: Check for bad strings (ie, if it contains puncutation charcters like , or ])
void ScriptDescriptor::WriteString(const std::string &key, const std::string &value) {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	if (_open_tables.size() == 0) {
		_outfile << key << " = \"" << value << "\"" << endl;

	}
	else {
		_WriteTablePath();
		_outfile << '[' << key << ']' << " = \"" << value << "\"" << endl;
	}
}

// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
// TODO: Check for bad strings (ie, if it contains puncutation charcters like , or ])
void ScriptDescriptor::WriteString(const int32 key, const std::string& value) {
	if (_open_tables.empty()) {
		_error_code |= SCRIPT_BAD_GLOBAL;
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT ERROR: Attempt to write a numerical value as a global key" << endl;
		return;
	}

	WriteString(boost::lexical_cast<std::string>(key), value);
}



void ScriptDescriptor::WriteBoolVector(const std::string &key, std::vector<bool> &vect) {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	if (vect.empty()) {
		_error_code |= SCRIPT_BAD_VECTOR_SIZE;
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT WARNING: passed a vector of size zero for writing." << endl;
		return;
	}

	if (_open_tables.size() == 0) {
		_outfile << key << " = { ";
	}
	else {
		_WriteTablePath();
		_outfile << '[' << key << "] = { ";
	}

	if (vect[0])
		_outfile << "true";
	else
		_outfile << "false";
	for (uint32 i = 1; i < vect.size(); i++) {
		if (vect[i])
			_outfile << ", true";
		else
			_outfile << ", false";
	}
	_outfile << " }" << endl;
}

void ScriptDescriptor::WriteBoolVector(const int32 key, std::vector<bool> &vect) {
	WriteBoolVector(boost::lexical_cast<std::string>(key),vect);
}



void ScriptDescriptor::WriteIntVector(const std::string &key, std::vector<int32> &vect) {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	if (vect.empty()) {
		_error_code |= SCRIPT_BAD_VECTOR_SIZE;
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT WARNING: passed a vector of size zero for writing." << endl;
		return;
	}

	if (_open_tables.size() == 0) {
			_outfile << key << " = { ";
		}
	else {
		_WriteTablePath();
		_outfile << '[' << key << "] = { ";
	}

	_outfile << vect[0];
	for (uint32 i = 1; i < vect.size(); i++) {
		_outfile << ", " << vect[i];
	}
	_outfile << " }" << endl;
}

void ScriptDescriptor::WriteIntVector(const int32 key, std::vector<int> &vect) {
	WriteIntVector(boost::lexical_cast<std::string>(key),vect);
}



void ScriptDescriptor::WriteFloatVector(const std::string &key, std::vector<float> &vect) {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	if (vect.empty()) {
		_error_code |= SCRIPT_BAD_VECTOR_SIZE;
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT WARNING: passed a vector of size zero for writing." << endl;
		return;
	}

	if (_open_tables.size() == 0) {
		_outfile << key << " = { ";

	}
	else {
		_WriteTablePath();
		_outfile << '[' << key << "] = { ";
	}

	_outfile << vect[0];
	for (uint32 i = 1; i < vect.size() - 1; i++) {
		_outfile << ", " << vect[i];
	}
	_outfile << " }" << endl;
}

void ScriptDescriptor::WriteFloatVector(const int32 key, std::vector<float> &vect) {
	WriteFloatVector(boost::lexical_cast<std::string>(key),vect);
}


// TODO: Check for bad strings (ie, if it contains puncutation charcters like , or ])
void ScriptDescriptor::WriteStringVector(const std::string &key, std::vector<std::string> &vect) {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	if (vect.empty()) {
		_error_code |= SCRIPT_BAD_VECTOR_SIZE;
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT WARNING: passed a vector of size zero for writing." << endl;
		return;
	}

	if (_open_tables.size() == 0) {
		_outfile << key << " = { ";
	}
	else {
		_WriteTablePath();
		_outfile << '[' << key << "] = { ";
	}

	_outfile << "\"" << vect[0] << "\"";
	for (uint32 i = 1; i < vect.size(); i++) {
		_outfile << ", \"" << vect[i] << "\"";
	}
	_outfile << " }" << endl;
}

void ScriptDescriptor::WriteStringVector(const int32 key, std::vector<std::string> &vect) {
	WriteStringVector(boost::lexical_cast<std::string>(key),vect);
}


// Writes the new table name to the file and manages the state of the context
void ScriptDescriptor::WriteBeginTable(const std::string &key) {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	if (_open_tables.size() == 0) {
		_outfile << key << " = {}" << endl;
	}
	else {
		_WriteTablePath();
		_outfile << '[' << key << "] = {}" << endl;
	}

	_open_tables.push_back(key);
}


// Writes the new tables using an integer as the key
void ScriptDescriptor::WriteBeginTable(int key)
{
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	if (_open_tables.size() == 0)
		_outfile << key << " = {}" << endl;
	else
	{
		_WriteTablePath();
		_outfile << '[' << key << "] = {}" << endl;
	}

	_open_tables.push_back(NumberToString<int>(key));
}

// Does internal scope handling of the lua file so things are written in the write global/table space.
// This doesn't actually do any file write operations, but we still need to call it.
void ScriptDescriptor::WriteEndTable() {
	if (_CheckFileAccess(SCRIPT_WRITE) != true)
		return;

	if (_open_tables.empty()) {
		_error_code |= SCRIPT_CLOSE_TABLE_FAILURE;
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT WARNING: Tried to close a table during writing when no table was open" << endl;
	}
	else {
		_open_tables.pop_back();
	}
}

// ***************************** Modify Functions ******************************

void ScriptDescriptor::SaveStack(const std::string &filename) {
	ScriptDescriptor sd;
	sd.OpenFile(filename, SCRIPT_WRITE);
	object o(luabind::from_stack(_lstack, LUA_GLOBALSINDEX));
	this->DEBUG_ShowGlobals();
	for (luabind::iterator it(o), end; it != end; ++it) {
		switch(luabind::type(*it)) {
		case LUA_TBOOLEAN:
// 			sd.WriteBool(object_cast<string>(it.key()), object_cast<bool>(*it));
			break;
		case LUA_TNUMBER:
// 			sd.WriteFloat(object_cast<string>(it.key()), object_cast<float>(*it));
			break;
		case LUA_TSTRING:
// 			sd.WriteString(object_cast<string>(it.key()), object_cast<string>(*it));
			break;
		case LUA_TTABLE:
			//this->_SaveStackProcessTable(sd, object_cast<string>(it.key()), object(*it));
			break;
		}
	}
	sd.CloseFile();
}



void ScriptDescriptor::_SaveStackProcessTable(ScriptDescriptor &sd, const string &name, luabind::object table) {
	sd.WriteBeginTable(name.c_str());
	for (luabind::iterator it(table), end; it != end; ++it) {
		switch(luabind::type(*it)) {
		case LUA_TBOOLEAN:
// 			sd.WriteBool(object_cast<string>(it.key()), object_cast<bool>(*it));
			break;
		case LUA_TNUMBER:
// 			sd.WriteFloat(object_cast<string>(it.key()), object_cast<float>(*it));
			break;
		case LUA_TSTRING:
// 			sd.WriteString(object_cast<string>(it.key()), object_cast<string>(*it));
			break;
		case LUA_TTABLE:
// 			this->_SaveStackProcessTable(sd, object_cast<string>(it.key()), object(*it));
			break;
		}
	}
	sd.WriteEndTable();
}

// *****************************************************************************
// *********************** GameScript Class Functions **************************
// *****************************************************************************

GameScript::GameScript() {
	if (SCRIPT_DEBUG) cout << "SCRIPT: GameScript constructor invoked." << endl;

	// Initialize Lua and LuaBind
	_global_state = lua_open();
	luabind::open(_global_state);
	lua_baselibopen(_global_state);
	lua_iolibopen(_global_state);
	lua_strlibopen(_global_state);
	lua_mathlibopen(_global_state);
}



GameScript::~GameScript() {
	if (SCRIPT_DEBUG) cout << "SCRIPT: GameScript destructor invoked." << endl;

	_open_files.clear();
	lua_close(_global_state);
	_global_state = NULL;
}



bool GameScript::SingletonInitialize() {
	// TODO: Open the user setting's file and apply those settings
	return true;
}



void GameScript::HandleLuaError(luabind::error& err) {
	lua_State *state = err.state();
	cerr << "SCRIPT ERROR: a run-time Lua error has occured with the following error message:\n  " << endl;
	cerr << lua_tostring(state, lua_gettop(state)) << endl;
	lua_pop(state, 1);
}



void GameScript::HandleCastError(luabind::cast_failed& err) {
	cerr << "SCRIPT ERROR: the return value of a Lua function call could not be successfully converted "
		<< "to the specified C++ type: " << err.what() << endl;
}



void GameScript::_AddOpenFile(ScriptDescriptor* sd) {
	// NOTE: Function assumes that the file is not already open
	_open_files.insert(make_pair(sd->_filename, sd));
}



void GameScript::_RemoveOpenFile(ScriptDescriptor* sd) {
	// NOTE: Function assumes that the ScriptDescriptor file is already open
	_open_files.erase(sd->_filename);
}



bool GameScript::IsFileOpen(const std::string& filename) {
	return false; // TEMP: working on resolving the issue with files being opened multiple times

	if (_open_files.find(filename) != _open_files.end()) {
		return true;
	}
	return false;
}

} // namespace hoa_script
