///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    data.cpp
 * \author  Vladimir Mitrovic, snipe714@allacrost.org
 * \brief   Source file for data and scripting engine.
 *****************************************************************************/

#include <iostream>
#include <stdarg.h>
#include "data.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_data::private_data;

namespace hoa_data {

GameData *DataManager = NULL;
bool DATA_DEBUG = false;
SINGLETON_INITIALIZE(GameData);


// ****************************************************************************
// ************************** ReadDataDescriptor ******************************
// ****************************************************************************

bool ReadDataDescriptor::_IsFileOpen() {
	if (_file_open == false) {
		_error_code |= DATA_BAD_FILE_ACCESS;
		if (DATA_DEBUG)
			cerr << "DATA ERROR: Attempt to operate on un-opened file " << _filename << endl;
	}
	return _file_open;
}

bool ReadDataDescriptor::OpenFile(const char* file_name) {
	_filename = file_name;
	return OpenFile();
}

bool ReadDataDescriptor::OpenFile() {
	// Open Lua first if it is not open already
	if (_file_open == false) {
		_lstack = lua_open();
		// Load the Lua libraries
		const luaL_reg *lib = LUALIBS;
		for (; lib->func; lib++) {
			lib->func(_lstack);      // open library
			lua_settop(_lstack, 0);  // Clear the stack
		}
	}
	
	// Attempt to load the Lua file.
	if (luaL_loadfile(_lstack, _filename.c_str()) || lua_pcall(_lstack, 0, 0, 0)) {
		cerr << "DATA ERROR: Could not load " << _filename << " :: " << lua_tostring(_lstack, STACK_TOP) << endl;
		_file_open = false;
		_filename = "";
		return false;
	}
	else {
		_file_open = true;
	}
	return _file_open;
}

void ReadDataDescriptor::CloseFile() {
	_open_tables.clear();
	lua_close(_lstack);
	_lstack = NULL;
	_file_open = false;
}

// ************************ Variable Access Functions *************************

// TODO: All of the functions in this section need more complete error checking.
// TODO: UString functions need to be implemented at some point

bool ReadDataDescriptor::ReadBool(const char *key) {
	if (!_IsFileOpen()) {
		return false;
	}
	
	bool value = false;
	if (_open_tables.size() == 0) { // Then this is a global variable we are fetching
		lua_getglobal(_lstack, key);
		value = (0 != lua_toboolean(_lstack, STACK_TOP));
		lua_pop(_lstack, 1);
	}
	
	else { // This is a table element we are fetching
		lua_pushstring(_lstack, key);
		lua_gettable(_lstack, STACK_TOP - 1);
		if (!lua_isboolean(_lstack, STACK_TOP)) {
			cerr << "DATA ERROR: Invalid table field" << endl;
			_error_code |= DATA_INVALID_TABLE_KEY;
		}
		else {
			value = (0 != lua_toboolean(_lstack, STACK_TOP));
		}
		lua_pop(_lstack, 1);
	}
	return value;
}

bool ReadDataDescriptor::ReadBool(const int32 key) {
	if (!_IsFileOpen()) {
		return false;
	}
	
	bool value = false;
	// At least one table must be open to use a numerical key
	if (_open_tables.size() == 0) {
		_error_code |= DATA_BAD_GLOBAL;
		return false;
	}
	
	lua_pushnumber(_lstack, key);
	lua_gettable(_lstack, STACK_TOP - 1);
	if (!lua_isboolean(_lstack, STACK_TOP)) {
		cerr << "DATA ERROR: Invalid table field" << endl;
		_error_code |= DATA_INVALID_TABLE_KEY;
	}
	else {
		value = (0 != lua_toboolean(_lstack, STACK_TOP));
	}
	lua_pop(_lstack, 1);
	return value;
}

int32 ReadDataDescriptor::ReadInt(const char *key) {
	if (!_IsFileOpen()) {
		return 0;
	}
	
	int32 value = 0;
	if (_open_tables.size() == 0) { // Then this is a global variable we are fetching
		lua_getglobal(_lstack, key);

		value = static_cast<int32>(lua_tonumber(_lstack, STACK_TOP));
		lua_pop(_lstack, 1);
	}
	else { // This is a table element we are fetching
		lua_pushstring(_lstack, key);
		lua_gettable(_lstack, STACK_TOP - 1);
		if (!lua_isnumber(_lstack, STACK_TOP)) {
			cerr << "DATA ERROR: Invalid table field" << endl;
			_error_code |= DATA_INVALID_TABLE_KEY;
		}
		else {
			value = static_cast<int32>(lua_tonumber(_lstack, STACK_TOP));
		}
		lua_pop(_lstack, 1);
	}
	return value;
}

int32 ReadDataDescriptor::ReadInt(const int32 key) {
	if (!_IsFileOpen()) {
		return 0;
	}
	
	int32 value = 0;
	// At least one table must be open to use a numerical key
	if (_open_tables.size() == 0) {
		_error_code |= DATA_BAD_GLOBAL;
		return 0;
	}
	
	lua_pushnumber(_lstack, key);
	lua_gettable(_lstack, STACK_TOP - 1);
	if (!lua_isnumber(_lstack, STACK_TOP)) {
		cerr << "DATA ERROR: Invalid table field" << endl;
		_error_code |= DATA_INVALID_TABLE_KEY;
	}
	else {
		value = static_cast<int32>(lua_tonumber(_lstack, STACK_TOP));
	}
	lua_pop(_lstack, 1);
	
	return value;
}

float ReadDataDescriptor::ReadFloat(const char *key) {
	if (!_IsFileOpen()) {
		return 0.0f;
	}
	
	float value = 0.0f;
	if (_open_tables.size() == 0) { // Then this is a global variable we are fetching
		lua_getglobal(_lstack, key);

		value = static_cast<int32>(lua_tonumber(_lstack, STACK_TOP));
		lua_pop(_lstack, 1);
	}
	else { // This is a table element we are fetching
		lua_pushstring(_lstack, key);
		lua_gettable(_lstack, STACK_TOP - 1);
		if (!lua_isnumber(_lstack, STACK_TOP)) {
			cerr << "DATA ERROR: Invalid table field" << endl;
			_error_code |= DATA_INVALID_TABLE_KEY;
		}
		else {
			value = static_cast<float>(lua_tonumber(_lstack, STACK_TOP));
		}
		lua_pop(_lstack, 1);
	}
	return value;
}

float ReadDataDescriptor::ReadFloat(const int32 key) {
	if (!_IsFileOpen()) {
		return 0.0f;
	}
	
	float value = 0.0f;
	// At least one table must be open to use a numerical key
	if (_open_tables.size() == 0) {
		_error_code |= DATA_BAD_GLOBAL;
		return false;
	}
	
	lua_pushnumber(_lstack, key);
	lua_gettable(_lstack, STACK_TOP - 1);
	if (!lua_isnumber(_lstack, STACK_TOP)) {
		cerr << "DATA ERROR: Invalid table field" << endl;
		_error_code |= DATA_INVALID_TABLE_KEY;
	}
	else {
		value = static_cast<float>(lua_tonumber(_lstack, STACK_TOP));
	}
	lua_pop(_lstack, 1);
	
	return value;
}

string ReadDataDescriptor::ReadString(const char *key) {
	if (!_IsFileOpen()) {
		return "";
	}
	
	string value = "";
	if (_open_tables.size() == 0) { // Then this is a global variable we are fetching
		lua_getglobal(_lstack, key);

		value = lua_tostring(_lstack, STACK_TOP);
		lua_pop(_lstack, 1);
	}
	else { // This is a table element we are fetching
		lua_pushstring(_lstack, key);
		lua_gettable(_lstack, STACK_TOP - 1);
		if (!lua_isstring(_lstack, STACK_TOP)) {
			cerr << "DATA ERROR: Invalid table field" << endl;
			_error_code |= DATA_INVALID_TABLE_KEY;
		}
		else {
			value = lua_tostring(_lstack, STACK_TOP);
		}
		lua_pop(_lstack, 1);
	}
	return value;
}

string ReadDataDescriptor::ReadString(const int32 key) {
	if (!_IsFileOpen()) {
		return "";
	}
	
	string value = "";
	// At least one table must be open to use a numerical key
	if (_open_tables.size() == 0) {
		_error_code |= DATA_BAD_GLOBAL;
		return false;
	}
	
	lua_pushnumber(_lstack, key);
	lua_gettable(_lstack, STACK_TOP - 1);
	if (!lua_isstring(_lstack, STACK_TOP)) {
		cerr << "DATA ERROR: Invalid table field" << endl;
		_error_code |= DATA_INVALID_TABLE_KEY;
	}
	else {
		value = lua_tostring(_lstack, STACK_TOP);
	}
	lua_pop(_lstack, 1);
	
	return value;
}

ustring ReadDataDescriptor::ReadUString(const char *key, const char *lang) {
	if (!_IsFileOpen()) {
		return MakeUnicodeString("");
	}
	
	return MakeUnicodeString("");
}

ustring ReadDataDescriptor::ReadUString(const int32 key, const char *lang) {
	if (!_IsFileOpen()) {
		return MakeUnicodeString("");
	}
	
	return MakeUnicodeString("");
}

// ************************* Table Access Functions ***************************

void ReadDataDescriptor::OpenTable(const char *key) {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (_open_tables.size() == 0) { // Then fetch the table from the global space
		lua_getglobal(_lstack, key);
		if (!lua_istable(_lstack, STACK_TOP)) {
			cerr << "DATA ERROR: could not retrieve table \"" << key << "\"" << endl;
			_error_code |= DATA_OPEN_TABLE_FAILURE;
			return;
		}
		_open_tables.push_back(key);
	}
	
	// Then the table to fetch is an element of another table
	else {
		lua_pushstring(_lstack, key);
		lua_gettable(_lstack, STACK_TOP - 1);
		if (!lua_istable(_lstack, STACK_TOP)) {
			cerr << "DATA ERROR: could not retreive sub-table using string key " << key << endl;
			_error_code |= DATA_OPEN_TABLE_FAILURE;
			return;
		}
		_open_tables.push_back(key);
	}
}

void ReadDataDescriptor::OpenTable(const int32 key) {
	if (!_IsFileOpen()) {
		return;
	}
	
	// At least one table must be open to use a numerical key
	if (_open_tables.size() == 0) {
		_error_code |= DATA_BAD_GLOBAL;
		return;
	}
	
	lua_pushnumber(_lstack, key);
	lua_gettable(_lstack, STACK_TOP - 1);
	if (!lua_istable(_lstack, STACK_TOP)) {
		cerr << "DATA ERROR: could not retreive sub-table using integer key " << key << endl;
		_error_code |= DATA_OPEN_TABLE_FAILURE;
		return;
	}
	// TODO: Convert the key to a string and add it to the open tables list
	_open_tables.push_back("numeric");
}

void ReadDataDescriptor::CloseTable() {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (_open_tables.size() == 0) {
		_error_code |= DATA_CLOSE_TABLE_FAILURE;
		return;
	}
	_open_tables.pop_back();
	lua_pop(_lstack, 1);
}

uint32 ReadDataDescriptor::GetTableSize(const char *key) {
	if (!_IsFileOpen()) {
		return 0;
	}
	
	uint32 size = 0;
	uint32 error_save = _error_code; // Temporarily save the error code
	_error_code = DATA_NO_ERRORS;
	OpenTable(key);
	// Only grab the size if the operation was successful.
	if (_error_code == DATA_NO_ERRORS) {
		size = static_cast<uint32>(luaL_getn(_lstack, STACK_TOP));
		CloseTable();
	}
	// Restore the error code along with the new errors from this call (if any) and return zero.
	_error_code = _error_code | error_save;
	return size;
}

uint32 ReadDataDescriptor::GetTableSize(const int32 key) {
	if (!_IsFileOpen()) {
		return 0;
	}
	
	uint32 error_save = _error_code; // Temporarily save the error code
	uint32 size = 0;
	
	_error_code = DATA_NO_ERRORS;
	OpenTable(key);
	// Only grab the size if the operation was successful.
	if (_error_code == DATA_NO_ERRORS) {
		size = static_cast<uint32>(luaL_getn(_lstack, STACK_TOP));
		CloseTable();
	}
	// Restore the error code along with the new errors from this call (if any) and return zero.
	_error_code = _error_code | error_save;
	return size;
}

uint32 ReadDataDescriptor::GetTableSize() {
	if (!_IsFileOpen()) {
		return 0;
	}
	
	// Attempt to get the size of the most recently opened table
	if (_open_tables.size() == 0) {
		return 0;
	}
	return static_cast<uint32>(luaL_getn(_lstack, STACK_TOP));
}

// ************************* Vector Fill Functions ****************************

void ReadDataDescriptor::FillIntVector(const char *key, std::vector<int32> &vect) {
	if (!_IsFileOpen()) {
		return;
	}
	
	// Reset the error code and try to open the table
	uint32 error_save = _error_code;
	_error_code = DATA_NO_ERRORS;
	OpenTable(key);
	if (_error_code != DATA_NO_ERRORS) {
		_error_code |= error_save;
		if (DATA_DEBUG)
			cerr << "DATA ERROR: table " << key << " does not exist, or " << key << "isn't a table" << endl;
		return;
	}
	_error_code = error_save;

	// Check to see if there is an element for key == 0. If so, push an empty element first
	bool zero_first = false;
	lua_pushnumber(_lstack, 0);
	lua_gettable(_lstack, STACK_TOP - 1);
	if (lua_isnumber(_lstack, STACK_TOP)) {
		zero_first = true;
		vect.push_back(0);
	}
	lua_pop(_lstack, 1);
	
	// Fetch all the values of the table
	int32 t = lua_gettop(_lstack);
	lua_pushnil(_lstack);
	while (lua_next(_lstack, t)) {
		vect.push_back(static_cast<int32>(lua_tonumber(_lstack, STACK_TOP)));
		lua_pop(_lstack, 1);
	}
	
	CloseTable();
	
	// If we had an element at key == 0, remove the last element and make it the first
	if (zero_first) {
		vect[0] = vect.back();
		vect.pop_back();
	}
}

void ReadDataDescriptor::FillFloatVector(const char *key, std::vector<float> &vect) {
	if (!_IsFileOpen()) {
		return;
	}
	
	// Reset the error code and try to open the table
	uint32 error_save = _error_code;
	_error_code = DATA_NO_ERRORS;
	OpenTable(key);
	if (_error_code != DATA_NO_ERRORS) {
		_error_code |= error_save;
		if (DATA_DEBUG)
			cerr << "DATA ERROR: table " << key << " does not exist, or " << key << "isn't a table" << endl;
		return;
	}
	_error_code = error_save;

	// Check to see if there is an element for key == 0. If so, push an empty element first
	bool zero_first = false;
	lua_pushnumber(_lstack, 0);
	lua_gettable(_lstack, STACK_TOP - 1);
	if (lua_isnumber(_lstack, STACK_TOP)) {
		zero_first = true;
		vect.push_back(0);
	}
	lua_pop(_lstack, 1);
	
	// Fetch all the values of the table
	int32 t = lua_gettop(_lstack);
	lua_pushnil(_lstack);
	while (lua_next(_lstack, t)) {
		vect.push_back(static_cast<float>(lua_tonumber(_lstack, STACK_TOP)));
		lua_pop(_lstack, 1);
	}
	
	CloseTable();
	
	// If we had an element at key == 0, remove the last element and make it the first
	if (zero_first) {
		vect[0] = vect.back();
		vect.pop_back();
	}
}

void ReadDataDescriptor::FillStringVector(const char *key, std::vector<std::string> &vect) {
	if (!_IsFileOpen()) {
		return;
	}
	
	// Reset the error code and try to open the table
	uint32 error_save = _error_code;
	_error_code = DATA_NO_ERRORS;
	OpenTable(key);
	if (_error_code != DATA_NO_ERRORS) {
		_error_code |= error_save;
		if (DATA_DEBUG)
			cerr << "DATA ERROR: table " << key << " does not exist, or " << key << "isn't a table" << endl;
		return;
	}
	_error_code = error_save;
	
	// Check to see if there is an element for key == 0. If so, push an empty element first
	bool zero_first = false;
	lua_pushnumber(_lstack, 0);
	lua_gettable(_lstack, STACK_TOP - 1);
	if (lua_isstring(_lstack, STACK_TOP)) {
		zero_first = true;
		vect.push_back("");
	}
	lua_pop(_lstack, 1);
	
	// Fetch all the values of the table
	int32 t = lua_gettop(_lstack);
	lua_pushnil(_lstack);
	while (lua_next(_lstack, t)) {
		vect.push_back(lua_tostring(_lstack, STACK_TOP));
		lua_pop(_lstack, 1);
	}
	
	CloseTable();
	
	// If we had an element at key == 0, remove the last element and make it the first
	if (zero_first) {
		vect[0] = vect.back();
		vect.pop_back();
	}
}

void ReadDataDescriptor::FillIntVector(const int32 key, std::vector<int32> &vect) {
	if (!_IsFileOpen()) {
		return;
	}
	
	// Must have at least one open table to use a numerical key
	if (_open_tables.size() == 0) {
		cout << "FillIntVector(int32, vect) thinks that the table size is zero..." << endl;
		_error_code |= DATA_BAD_GLOBAL;
		return;
	}
	
	// Reset the error code and try to open the table
	uint32 error_save = _error_code;
	_error_code = DATA_NO_ERRORS;
	OpenTable(key);
	if (_error_code != DATA_NO_ERRORS) {
		_error_code |= error_save;
		if (DATA_DEBUG)
			cerr << "DATA ERROR: table " << key << " does not exist, or " << key << "isn't a table" << endl;
		return;
	}
	_error_code = error_save;

	// Check to see if there is an element for key == 0. If so, push an empty element first
	bool zero_first = false;
	lua_pushnumber(_lstack, 0);
	lua_gettable(_lstack, STACK_TOP - 1);
	if (lua_isnumber(_lstack, STACK_TOP)) {
		zero_first = true;
		vect.push_back(0);
	}
	lua_pop(_lstack, 1);
	
	// Fetch all the values of the table
	int32 t = lua_gettop(_lstack);
	lua_pushnil(_lstack);
	while (lua_next(_lstack, t)) {
		vect.push_back(static_cast<int32>(lua_tonumber(_lstack, STACK_TOP)));
		lua_pop(_lstack, 1);
	}
	
	CloseTable();
	
	// If we had an element at key == 0, remove the last element and make it the first
	if (zero_first) {
		vect[0] = vect.back();
		vect.pop_back();
	}
}

void ReadDataDescriptor::FillFloatVector(const int32 key, std::vector<float> &vect) {
	if (!_IsFileOpen()) {
		return;
	}
	
	// Must have at least one open table to use a numerical key
	if (_open_tables.size() == 0) {
		_error_code |= DATA_BAD_GLOBAL;
		return;
	}
	
	// Reset the error code and try to open the table
	uint32 error_save = _error_code;
	_error_code = DATA_NO_ERRORS;
	OpenTable(key);
	if (_error_code != DATA_NO_ERRORS) {
		_error_code |= error_save;
		if (DATA_DEBUG)
			cerr << "DATA ERROR: table " << key << " does not exist, or " << key << "isn't a table" << endl;
		return;
	}
	_error_code = error_save;

	// Check to see if there is an element for key == 0. If so, push an empty element first
	bool zero_first = false;
	lua_pushnumber(_lstack, 0);
	lua_gettable(_lstack, STACK_TOP - 1);
	if (lua_isnumber(_lstack, STACK_TOP)) {
		zero_first = true;
		vect.push_back(0);
	}
	lua_pop(_lstack, 1);
	
	// Fetch all the values of the table
	int32 t = lua_gettop(_lstack);
	lua_pushnil(_lstack);
	while (lua_next(_lstack, t)) {
		vect.push_back(static_cast<float>(lua_tonumber(_lstack, STACK_TOP)));
		lua_pop(_lstack, 1);
	}
	
	CloseTable();
	
	// If we had an element at key == 0, remove the last element and make it the first
	if (zero_first) {
		vect[0] = vect.back();
		vect.pop_back();
	}
}

void ReadDataDescriptor::FillStringVector(const int32 key, std::vector<std::string> &vect) {
	if (!_IsFileOpen()) {
		return;
	}
	
	// Must have at least one open table to use a numerical key
	if (_open_tables.size() == 0) {
		_error_code |= DATA_BAD_GLOBAL;
		return;
	}
	
	// Reset the error code and try to open the table
	uint32 error_save = _error_code;
	_error_code = DATA_NO_ERRORS;
	OpenTable(key);
	if (_error_code != DATA_NO_ERRORS) {
		_error_code |= error_save;
		if (DATA_DEBUG)
			cerr << "DATA ERROR: table " << key << " does not exist, or " << key << "isn't a table" << endl;
		return;
	}
	_error_code = error_save;

	// Check to see if there is an element for key == 0. If so, push an empty element first
	bool zero_first = false;
	lua_pushnumber(_lstack, 0);
	lua_gettable(_lstack, STACK_TOP - 1);
	if (lua_isstring(_lstack, STACK_TOP)) {
		zero_first = true;
		vect.push_back("");
	}
	lua_pop(_lstack, 1);
	
	// Fetch all the values of the table
	int32 t = lua_gettop(_lstack);
	lua_pushnil(_lstack);
	while (lua_next(_lstack, t)) {
		vect.push_back(lua_tostring(_lstack, STACK_TOP));
		lua_pop(_lstack, 1);
	}
	
	CloseTable();
	
	// If we had an element at key == 0, remove the last element and make it the first
	if (zero_first) {
		vect[0] = vect.back();
		vect.pop_back();
	}
}

// *************************** Lua Function Calls *****************************

void ReadDataDescriptor::CallFunction(const char *func, const char *sig, ...) {
	if (!_IsFileOpen()) {
		return;
	}
	
	va_list vl;
	int narg, nres;  /* number of arguments and results */

	va_start(vl, sig);
	
	if (_open_tables.size() == 0) { // This is a global function call
		lua_getglobal(_lstack, func);  /* get function */
	}
	else {
		// NOTE: Not sure if this is correct...
		lua_pushstring(_lstack, func);
		lua_gettable(_lstack, STACK_TOP - 1);
	}
		
	/* push arguments */
	narg = 0;
	while (*sig) {  /* push arguments */
		switch (*sig++) {

		case 'd':  /* double argument */
			lua_pushnumber(_lstack, va_arg(vl, double));
			break;

		case 'i':  /* int argument */
			lua_pushnumber(_lstack, va_arg(vl, int));
			break;

		case 's':  /* string argument */
			lua_pushstring(_lstack, va_arg(vl, char *));
			break;

		case '>':
			goto endwhile;

		default:
			std::cout << "invalid option (" << *(sig - 1) << ")\n";
		}
		narg++;
		luaL_checkstack(_lstack, 1, "too many arguments");
	}

endwhile:
	/* do the call */
	nres = strlen(sig);  /* number of expected results */
	if (lua_pcall(_lstack, narg, nres, 0) != 0)  /* do the call */
		std::cout << "error running function " << func << " : " << lua_tostring(_lstack, -1);

	/* retrieve results */
	nres = -nres;  /* stack index of first result */
	while (*sig) {  /* get results */
		switch (*sig++) {

		case 'd':  /* double result */
			if (!lua_isnumber(_lstack, nres))
				std::cout << "wrong result type\n";
			*va_arg(vl, double *) = lua_tonumber(_lstack, nres);
			break;

		case 'i':  /* int result */
			if (!lua_isnumber(_lstack, nres))
				std::cout << "wrong result type\n";
			*va_arg(vl, int *) = (int)lua_tonumber(_lstack, nres);
			break;

		case 's':  /* string result */
			if (!lua_isstring(_lstack, nres))
				std::cout << "wrong result type\n";
			*va_arg(vl, const char **) = lua_tostring(_lstack, nres);
			break;

		default:
			std::cout << "invalid option (" << *(sig - 1) << ")\n";
		}
		nres++;
	}
	va_end(vl);
}

// *********************** Lua<->C++ binding functions ************************

// Start the registration of member functions and objects
void ReadDataDescriptor::RegisterClassStart() {
	if (!_IsFileOpen()) {
		return;
	}
	
	lua_newtable(_lstack);
	lua_pushstring(_lstack, "__index");
	lua_pushvalue(_lstack, -2);
	lua_settable(_lstack, -3);
}

// End the registration of member functions and objects
void ReadDataDescriptor::RegisterClassEnd() {
	if (!_IsFileOpen()) {
		return;
	}
	
	lua_pop(_lstack, 1);
}

// Register a class' (not objects!) member function to be used from Lua
template <typename Class, typename Function>
void ReadDataDescriptor::RegisterMemberFunction(const char *funcname, Class *obj, Function func) {
	if (!_IsFileOpen()) {
		return;
	}
	
	lua_pushstring(_lstack, funcname);
	lua_pushobjectdirectclosure((lua_State*)_lstack, (Class *)obj, func, (unsigned int)0);
	lua_settable(_lstack, -3);
	
	// add error checking later
}

// Register the object of some class (which was previously registered) to be used from Lua
template <typename Class>
void ReadDataDescriptor::RegisterObject(const char *objname, Class *obj) {
	if (!_IsFileOpen()) {
		return;
	}
	
	lua_pushstring(_lstack, objname);
	lua_newtable(_lstack);
	lua_pushstring(_lstack, "__object");
	lua_pushlightuserdata(_lstack, (void*)obj);
	lua_settable(_lstack, -3);
	lua_pushvalue(_lstack, -3);
	lua_setmetatable(_lstack, -2);
	lua_settable(_lstack, LUA_GLOBALSINDEX);
	
	// add error checking later
}

// Register a (non-member or static) function to be used from Lua
template <typename Function>
void ReadDataDescriptor::RegisterFunction(const char* funcname, Function func) {
	if (!_IsFileOpen()) {
		return;
	}
	
	lua_pushstring(_lstack, funcname);
	lua_pushdirectclosure(_lstack, func, 0);
	lua_settable(_lstack, LUA_GLOBALSINDEX);
	
	// add error checking later
}

// **************************** Debugging functions ***************************

// This function is for DEBUGGING PURPOSES ONLY! It prints the contents of the Lua stack from top to bottom.
void ReadDataDescriptor::DEBUG_PrintLuaStack() {
	if (!_IsFileOpen()) {
		return;
	}
	
	int32 type;
	cout << "DATA DEBUG: Printing lua stack" << endl;
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
}

// ****************************************************************************
// ************************* WriteDataDescriptor *****************************
// ****************************************************************************

bool WriteDataDescriptor::_IsFileOpen() {
	if (_file_open == false) {
		_error_code |= DATA_BAD_FILE_ACCESS;
		if (DATA_DEBUG)
			cerr << "DATA ERROR: Attempt to operate on unopened file: " << _filename << endl;
	}
	return _file_open;
}

// Writes the "path" for all the open tables. Ex) tbl1[tbl2][tbl3]
void WriteDataDescriptor::_WriteTablePath() {
	// No error checking is done since _IsWriteTableOpen is always called before it to make sure that
	// at least one table is open, and the functions that call this function always check that _outfile
	// is valid.
	_outfile << _open_tables[0];
	for (uint32 i = 1; i < _open_tables.size(); i++) {
		_outfile << '[' << _open_tables[i] << ']';
	}
}

// ************************** File Access Functions ***************************

// Opens a file for writing permissions.
bool WriteDataDescriptor::OpenFile(const char* file_name) {
	_filename = file_name;
	return OpenFile();
}

bool WriteDataDescriptor::OpenFile() {
	if (_file_open) {
		cerr << "DATA WARNING: Tried to open a write file when another was already open for writing" << endl;
		return false;
	}
	
	_outfile.open(_filename.c_str());
	if (!_outfile) {
		cerr << "DATA ERROR: Failed to open file " << _filename << " for writing." << endl;
		_file_open = false;
		_filename = "";
	}
	else {
		_file_open = true;
	}
	
	return _file_open;
}

// Closes the writing output file.
void WriteDataDescriptor::CloseFile() {
	_open_tables.clear();
	if (!_file_open) {
		cerr << "DATA WARNING: Tried to close an output file when nothing was open" << endl;
	}
	else {
		_outfile.close();
		_file_open = false;
	}
}

// ************************* Comment Write Functions **************************

void WriteDataDescriptor::InsertNewLine() {
	if (!_IsFileOpen()) {
		return;
	}
	
	_outfile << endl;
}

void WriteDataDescriptor::WriteComment(const char* comment) {
	if (!_IsFileOpen()) {
		return;
	}
	
	_outfile << "-- " << comment << endl;
}

void WriteDataDescriptor::WriteComment(std::string& comment) {
	if (!_IsFileOpen()) {
		return;
	}
	
	_outfile << "-- " << comment << endl;
}

void WriteDataDescriptor::BeginCommentBlock() {
	if (!_IsFileOpen()) {
		return;
	}
	
	_outfile << "--[[" << endl;
}

void WriteDataDescriptor::EndCommentBlock() {
	if (!_IsFileOpen()) {
		return;
	}
	
	_outfile << "--]]" << endl;
}

void WriteDataDescriptor::WriteLine(const char* comment) {
	if (!_IsFileOpen()) {
		return;
	}
	
	_outfile << comment << endl;
}

void WriteDataDescriptor::WriteLine(std::string& comment) {
	if (!_IsFileOpen()) {
		return;
	}
	
	_outfile << comment << endl;
}

// ************************ Variable Write Functions **************************

// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
void WriteDataDescriptor::WriteBool(const char *key, bool value) {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (_open_tables.size() == 0) {
		_outfile << key << " = ";
		if (value)
			_outfile << "true" << endl;
		else
			_outfile << "false" << endl;
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
void WriteDataDescriptor::WriteInt(const char *key, int32 value) {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (_open_tables.size() == 0) {
		_outfile << key << " = " << value << endl;
	}
	else {
		_WriteTablePath();
		_outfile << '[' << key << ']' << " = " << value << endl;
	}
}

// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
void WriteDataDescriptor::WriteFloat(const char *key, float value) {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (_open_tables.size() == 0) {
		_outfile << key << " = " << value << endl;
	}
	
	else {
		_WriteTablePath();
		_outfile << '[' << key << ']' << " = " << value << endl;
	}
}

// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
// TODO: Check for bad strings (ie, if it contains puncutation charcters like , or ])
void WriteDataDescriptor::WriteString(const char *key, const char* value) {
	if (!_IsFileOpen()) {
		return;
	}
	
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
void WriteDataDescriptor::WriteString(const char *key, std::string& value) {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (_open_tables.size() == 0) {
		_outfile << key << " = \"" << value << "\"" << endl;
	}
	else {
		_WriteTablePath();
		_outfile << '[' << key << ']' << " = \"" << value << "\"" << endl;
	}
}

// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
void WriteDataDescriptor::WriteBool(const int32 key, bool value) {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (_open_tables.empty()) {
		_error_code |= DATA_BAD_GLOBAL;
		if (DATA_DEBUG)
			cerr << "DATA ERROR: Attempt to write a numerical value as a global key" << endl;
		return;
	}
	
	_WriteTablePath();
	_outfile << '[' << key << ']' << " = ";
	if (value)
		_outfile << "true" << endl;
	else
		_outfile << "false" << endl;
}

// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
void WriteDataDescriptor::WriteInt(const int32 key, int32 value) {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (_open_tables.empty()) {
		_error_code |= DATA_BAD_GLOBAL;
		if (DATA_DEBUG)
			cerr << "DATA ERROR: Attempt to write a numerical value as a global key" << endl;
		return;
	}
	
	_WriteTablePath();
	_outfile << '[' << key << ']' << " = " << value << endl;
}

// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
void WriteDataDescriptor::WriteFloat(const int32 key, float value) {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (_open_tables.empty()) {
		_error_code |= DATA_BAD_GLOBAL;
		if (DATA_DEBUG)
			cerr << "DATA ERROR: Attempt to write a numerical value as a global key" << endl;
		return;
	}
	
	_WriteTablePath();
	_outfile << '[' << key << ']' << " = " << value << endl;
}

// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
// TODO: Check for bad strings (ie, if it contains puncutation charcters like , or ])
void WriteDataDescriptor::WriteString(const int32 key, const char* value) {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (_open_tables.empty()) {
		_error_code |= DATA_BAD_GLOBAL;
		if (DATA_DEBUG)
			cerr << "DATA ERROR: Attempt to write a numerical value as a global key" << endl;
		return;
	}
	
	_WriteTablePath();
	_outfile << '[' << key << ']' << " = \"" << value << "\"" << endl;
}

// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
// TODO: Check for bad strings (ie, if it contains puncutation charcters like , or ])
void WriteDataDescriptor::WriteString(const int32 key, std::string& value) {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (_open_tables.empty()) {
		_error_code |= DATA_BAD_GLOBAL;
		if (DATA_DEBUG)
			cerr << "DATA ERROR: Attempt to write a numerical value as a global key" << endl;
		return;
	}
	
	_WriteTablePath();
	_outfile << '[' << key << ']' << " = \"" << value << "\"" << endl;
}

// ****************************************************************************
// ************************ Lua table write functions *************************
// ****************************************************************************

// Writes the new table name to the file and manages the state of the context
void WriteDataDescriptor::BeginTable(const char *key) {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (_open_tables.size() == 0) {
		_outfile << key << " = {}" << endl;
	}
	else {
		_WriteTablePath();
		_outfile << '[' << key << "] = {}" << endl;
	}
	
	_open_tables.push_back(key);
}

// Does internal scope handling of the lua file so things are written in the write global/table space.
// This doesn't actually do any file write operations, but we still need to call it.
void WriteDataDescriptor::EndTable() {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (_open_tables.empty()) {
		_error_code |= DATA_CLOSE_TABLE_FAILURE;
		if (DATA_DEBUG) 
			cerr << "DATA WARNING: Tried to close a table during writing when no table was open" << endl;
	}
	else {
		_open_tables.pop_back();
	}
}

// ************************** Vector Write Functions **************************

void WriteDataDescriptor::WriteBoolVector(const char *key, std::vector<bool> &vect) {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (vect.empty()) {
		_error_code |= DATA_BAD_VECTOR_SIZE;
		if (DATA_DEBUG) 
			cerr << "DATA WARNING: passed a vector of size zero for writing." << endl;
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

void WriteDataDescriptor::WriteIntVector(const char *key, std::vector<int32> &vect) {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (vect.empty()) {
		_error_code |= DATA_BAD_VECTOR_SIZE;
		if (DATA_DEBUG) 
			cerr << "DATA WARNING: passed a vector of size zero for writing." << endl;
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

void WriteDataDescriptor::WriteFloatVector(const char *key, std::vector<float> &vect) {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (vect.empty()) {
		_error_code |= DATA_BAD_VECTOR_SIZE;
		if (DATA_DEBUG) 
			cerr << "DATA WARNING: passed a vector of size zero for writing." << endl;
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

// TODO: Check for bad strings (ie, if it contains puncutation charcters like , or ])
void WriteDataDescriptor::WriteStringVector(const char *key, std::vector<std::string> &vect) {
	if (!_IsFileOpen()) {
		return;
	}
	
	if (vect.empty()) {
		_error_code |= DATA_BAD_VECTOR_SIZE;
		if (DATA_DEBUG) 
			cerr << "DATA WARNING: passed a vector of size zero for writing." << endl;
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

// ****************************************************************************
// *********************** GameData Class Functions ***************************
// ****************************************************************************

GameData::GameData() {
	if (DATA_DEBUG) cout << "DATA: GameData constructor invoked." << endl;
}

GameData::~GameData() {
	if (DATA_DEBUG) cout << "DATA: GameData destructor invoked." << endl;
}

// Required method for all singletons
bool GameData::SingletonInitialize() {
	return true;
}

} // namespace hoa_data
