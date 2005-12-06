///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    data.cpp
 * \author  Vladimir Mitrovic, snipe714@allacrost.org
 * \date    Last Updated: August 25th, 2005
 * \brief   Source file for data and scripting engine.
 *****************************************************************************/

#include <iostream>
#include <stdarg.h>
#include "data.h"

using namespace std;
using namespace hoa_data::private_data;

namespace hoa_data {

GameData *DataManager = NULL;
bool DATA_DEBUG = false;
SINGLETON_INITIALIZE(GameData);


// The constructor opens lua and its associated libraries.
GameData::GameData() {
	if (DATA_DEBUG) cout << "DATA: GameData constructor invoked." << endl;
	_l_stack = lua_open();

	// Load the Lua libraries
	const luaL_reg *lib = LUALIBS;
	for (; lib->func; lib++) {
		lib->func(_l_stack);      // open library
		lua_settop(_l_stack, 0);  // Clear the stack
	}
}

// Close Lua upon destruction
GameData::~GameData() {
	if (DATA_DEBUG) cout << "DATA: GameData destructor invoked." << endl;
	lua_close(_l_stack);
}

// Required method for all singletons
bool GameData::Initialize() {
	return true;
}

bool GameData::_IsWriteFileOpen() {
	if (!_outfile) {
		cerr << "DATA WARNING: Attempted to write to an output file when no file was opened." << endl;
		return false;
	}
	return true;
}

// Return false if no write tables are open
bool GameData::_IsWriteTableOpen() {
	return (!_open_tables.empty());
}

// Writes the "path" for all the open tables. Ex) tbl1[tbl2][tbl3]
void GameData::_WriteTablePath() {
	// No error checking is done since _IsWriteTableOpen is always called before it to make sure that
	// at least one table is open, and the functions that call this function always check that _outfile
	// is valid.
	_outfile << _open_tables[0];
	for (uint32 i = 1; i < _open_tables.size(); i++) {
		_outfile << '[' << _open_tables[i] << ']';
	}
}

// ****************************************************************************
// ************************ Lua file access functions *************************
// ****************************************************************************

void GameData::OpenLuaFile(const char* file_name) {
	if (luaL_loadfile(_l_stack, file_name) || lua_pcall(_l_stack, 0, 0, 0))
		cerr << "DATA ERROR: Could not load " << file_name << " :: " <<
		        lua_tostring(_l_stack, -1) << endl;
}

// void GameData::ReadCloseFile();

// ****************************************************************************
// ********************** Lua variable access functions ***********************
// ****************************************************************************

// Get a global boolean field
bool GameData::GetGlobalBool(const char *key) {
	lua_getglobal(_l_stack, key);
	bool ret = (0 != lua_toboolean(_l_stack, LUA_STACK_TOP));
	lua_pop(_l_stack, 1);
	
	return ret;
}

void GameData::GetGlobalBoolRef(const char *key, bool &ref) {
	ref = GetGlobalBool(key);
}

// Get a global integer field
int32 GameData::GetGlobalInt(const char *key) {
	lua_getglobal(_l_stack, key);
	int32 ret = (int32)lua_tonumber(_l_stack, LUA_STACK_TOP);
	lua_pop(_l_stack, 1);
	
	return ret;
}

void GameData::GetGlobalIntRef(const char *key, int32 &ref) {
	ref = GetGlobalInt(key);
}

// Get a global floating point field
float GameData::GetGlobalFloat(const char *key) {
	lua_getglobal(_l_stack, key);
	float ret = (float)lua_tonumber(_l_stack, LUA_STACK_TOP);
	lua_pop(_l_stack, 1);
	
	return ret;
}

void GameData::GetGlobalFloatRef(const char *key, float &ref) {
	ref = GetGlobalFloat(key);
}

// Get a global string field
string GameData::GetGlobalString(const char *key) {
	lua_getglobal(_l_stack, key);
	string ret = (string)lua_tostring(_l_stack, LUA_STACK_TOP);
	lua_pop(_l_stack, 1);
	
	return ret;
}

void GameData::GetGlobalStringRef(const char * key, string &ref) {
	ref = GetGlobalString(key);
}

// ****************************************************************************
// ************************ Lua table access functions ************************
// ****************************************************************************	

void GameData::OpenTable(const char* tbl_name) {
	lua_getglobal(_l_stack, tbl_name);
	if (!lua_istable(_l_stack, LUA_STACK_TOP))
		cerr << "DATA ERROR: could not retrieve table \"" << tbl_name << "\"" << endl;
}

void GameData::CloseTable() {
	lua_pop(_l_stack, 1);
}

bool GameData::OpenSubTable(const int32 key) {
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: could not complete call OpenSubTable because top stack element was not a table" << endl;
		return false;
	}
	lua_pushnumber(_l_stack, key);
	lua_gettable(_l_stack, LUA_STACK_TOP - 1);
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: could not retreive sub-table using integer key " << key << endl;
		return false;
	}
	return true;
}

bool GameData::OpenSubTable(const char* key) {
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: could not complete call OpenSubTable because top stack element was not a table" << endl;
		return false;
	}
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, LUA_STACK_TOP - 1);
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: could not retreive sub-table using string key " << key << endl;
		return false;
	}
	return true;
}

void GameData::CloseSubTable() {
	lua_pop(_l_stack, 1);
}

// NOTE: This function doesn't work yet. I'm still trying to figure out why.
int32 GameData::GetTableSize(const char *tbl_name) {
	int32 result;
	lua_getglobal(_l_stack, tbl_name);
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: could not retrieve table \"" << tbl_name << "\"" << endl;
		return -1;
	}
	result = luaL_getn(_l_stack, -1);
	lua_pop(_l_stack, 1);
	return result;
}

// NOTE: I'll write this function once I fix the above version
int32 GameData::GetTableSize() {
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: GetTableSize() called when top stack element was not a table" << endl;
		return -1;
	}
	return luaL_getn(_l_stack, -1);
}

// Get an individual boolean field from a table, assumed to be on top of the stack
bool GameData::GetTableBool(const char *key) {
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isboolean(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	bool ret = (0 != lua_toboolean(_l_stack, -1));
	lua_pop(_l_stack, 1);
	
	return ret;
}

void GameData::GetTableBoolRef(const char *key, bool &ref) {
	ref = GetTableBool(key);
}

bool GameData::GetTableBool(const int32 key) {
	lua_pushnumber(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isboolean(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	bool ret = (0 != lua_toboolean(_l_stack, -1));
	lua_pop(_l_stack, 1);
	
	return ret;
}

void GameData::GetTableBoolRef(const int32 key, bool &ref) {
	ref = GetTableBool(key);
}

// Get an individual integer field from a table, assumed to be on top of the stack
int32 GameData::GetTableInt(const char *key) {
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isnumber(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	int32 ret = (int32)lua_tonumber(_l_stack, -1);
	lua_pop(_l_stack, 1);
	
	return ret;
}

void GameData::GetTableIntRef(const char *key, int32 &ref) {
	ref = GetTableInt(key);
}

int32 GameData::GetTableInt(const int32 key) {
	lua_pushnumber(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isnumber(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	int32 ret = (int32)lua_tonumber(_l_stack, -1);
	lua_pop(_l_stack, 1);
	
	return ret;
}

void GameData::GetTableIntRef(const int32 key, int32 &ref) {
	ref = GetTableInt(key);
}

// Get an individual floating point field from a table, assumed to be on top of the stack
float GameData::GetTableFloat(const char *key) {
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isnumber(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	float ret = (float)lua_tonumber(_l_stack, -1);
	lua_pop(_l_stack, 1);
	
	return ret;
}

void GameData::GetTableFloatRef(const char *key, float &ref) {
	ref = GetTableFloat(key);
}

float GameData::GetTableFloat(const int32 key) {
	lua_pushnumber(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isnumber(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	float ret = (float)lua_tonumber(_l_stack, -1);
	lua_pop(_l_stack, 1);
	
	return ret;
}

void GameData::GetTableFloatRef(const int32 key, float &ref) {
	ref = GetTableFloat(key);
}

// Get an individual string field from a table, assumed to be on top of the stack
string GameData::GetTableString(const char *key) {
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isstring(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	string ret = (string)lua_tostring(_l_stack, -1);
	lua_pop(_l_stack, 1);
	
	return ret;
}

void GameData::GetTableStringRef(const char *key, string &ref) {
	ref = GetTableString(key);
}

string GameData::GetTableString(const int32 key) {
	lua_pushnumber(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isstring(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	string ret = (string)lua_tostring(_l_stack, -1);
	lua_pop(_l_stack, 1);
	
	return ret;
}

void GameData::GetTableStringRef(const int32 key, string &ref) {
	ref = GetTableString(key);
}

// ****************************************************************************
// ************************ Lua vector fill functions *************************
// ****************************************************************************

// TODO L8R: Maybe turn this function into FillVector(vector*, const char*, int lua_type),
//           which will fill a custom vector?
void GameData::FillStringVector(const char *key, vector<string> &vect) {
	lua_getglobal(_l_stack, key);
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: table " << key << " does not exist, or " << key << "isn't a table" << endl;
		return;
	}
	int32 t = lua_gettop(_l_stack);
	lua_pushnil(_l_stack);
	while (lua_next(_l_stack, t)) {
		vect.push_back((string)lua_tostring(_l_stack, LUA_STACK_TOP));
		lua_pop(_l_stack, 1);
	}
}

void GameData::FillIntVector(const char *key, std::vector<int32> &vect) {
	lua_getglobal(_l_stack, key);
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: table " << key << " does not exist, or " << key << "isn't a table" << endl;
		return;
	}
	int32 t = lua_gettop(_l_stack);
	lua_pushnil(_l_stack);
	while (lua_next(_l_stack, t)) {
		vect.push_back((int32)lua_tonumber(_l_stack, LUA_STACK_TOP));
		lua_pop(_l_stack, 1);
	}
}

void GameData::FillFloatVector(const char *key, std::vector<float> &vect) {
	lua_getglobal(_l_stack, key);
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: table " << key << " does not exist, or " << key << "isn't a table" << endl;
		return;
	}
	int32 t = lua_gettop(_l_stack);
	lua_pushnil(_l_stack);
	while (lua_next(_l_stack, t)) {
		vect.push_back(lua_tonumber(_l_stack, LUA_STACK_TOP));
		lua_pop(_l_stack, 1);
	}
}

void GameData::FillStringVector(vector<string> &vect) {
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: FillStringVector called when top stack element was not a table" << endl;
		return;
	}
	int32 t = lua_gettop(_l_stack);
	lua_pushnil(_l_stack);
	while (lua_next(_l_stack, t)) {
		vect.push_back((string)lua_tostring(_l_stack, LUA_STACK_TOP));
		lua_pop(_l_stack, 1);
	}
}

void GameData::FillIntVector(std::vector<int32> &vect) {
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: FillIntVector called when top stack element was not a table" << endl;
		return;
	}
	int32 t = lua_gettop(_l_stack);
	lua_pushnil(_l_stack);
	while (lua_next(_l_stack, t)) {
		if (!lua_isnumber(_l_stack, LUA_STACK_TOP)) {
			cout << "DATA WTF: Not a number!" << endl;
		}
		vect.push_back((int32)lua_tonumber(_l_stack, LUA_STACK_TOP));
		lua_pop(_l_stack, 1);
	}
}

void GameData::FillFloatVector(std::vector<float> &vect) {
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: FillFloatVector called when top stack element was not a table" << endl;
		return;
	}
	int32 t = lua_gettop(_l_stack);
	lua_pushnil(_l_stack);
	while (lua_next(_l_stack, t)) {
		vect.push_back(lua_tonumber(_l_stack, LUA_STACK_TOP));
		lua_pop(_l_stack, 1);
	}
}

// ****************************************************************************
// ********************** Lua function calling functions **********************
// ****************************************************************************

void GameData::CallGlobalFunction(const char *func, const char *sig, ...) {
	va_list vl;
	int narg, nres;  /* number of arguments and results */

	va_start(vl, sig);
	lua_getglobal(_l_stack, func);  /* get function */

	/* push arguments */
	narg = 0;
	while (*sig) {  /* push arguments */
		switch (*sig++) {

		case 'd':  /* double argument */
			lua_pushnumber(_l_stack, va_arg(vl, double));
			break;

		case 'i':  /* int argument */
			lua_pushnumber(_l_stack, va_arg(vl, int));
			break;

		case 's':  /* string argument */
			lua_pushstring(_l_stack, va_arg(vl, char *));
			break;

		case '>':
			goto endwhile;

		default:
			std::cout << "invalid option (" << *(sig - 1) << ")\n";
		}
		narg++;
		luaL_checkstack(_l_stack, 1, "too many arguments");
	}

endwhile:
	/* do the call */
	nres = strlen(sig);  /* number of expected results */
	if (lua_pcall(_l_stack, narg, nres, 0) != 0)  /* do the call */
		std::cout << "error running function " << func << " : " << lua_tostring(_l_stack, -1);

	/* retrieve results */
	nres = -nres;  /* stack index of first result */
	while (*sig) {  /* get results */
		switch (*sig++) {

		case 'd':  /* double result */
			if (!lua_isnumber(_l_stack, nres))
				std::cout << "wrong result type\n";
			*va_arg(vl, double *) = lua_tonumber(_l_stack, nres);
			break;

		case 'i':  /* int result */
			if (!lua_isnumber(_l_stack, nres))
				std::cout << "wrong result type\n";
			*va_arg(vl, int *) = (int)lua_tonumber(_l_stack, nres);
			break;

		case 's':  /* string result */
			if (!lua_isstring(_l_stack, nres))
				std::cout << "wrong result type\n";
			*va_arg(vl, const char **) = lua_tostring(_l_stack, nres);
			break;

		default:
			std::cout << "invalid option (" << *(sig - 1) << ")\n";
		}
		nres++;
	}
	va_end(vl);
}

// Assumes that a table is on top of the stack
void GameData::CallTableFunction(const char *func, const char *sig, ...) {
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: CallTableFunction invoked for function " << func 
		     << " when table was not on the top of the stack" << endl;
		return;
	}
	
	va_list vl;
	int narg, nres;  /* number of arguments and results */

	va_start(vl, sig);
	lua_getglobal(_l_stack, func);  /* get function */

	/* push arguments */
	narg = 0;
	while (*sig) {  /* push arguments */
		switch (*sig++) {

		case 'd':  /* double argument */
			lua_pushnumber(_l_stack, va_arg(vl, double));
			break;

		case 'i':  /* int argument */
			lua_pushnumber(_l_stack, va_arg(vl, int));
			break;

		case 's':  /* string argument */
			lua_pushstring(_l_stack, va_arg(vl, char *));
			break;

		case '>':
			goto endwhile;

		default:
			std::cout << "invalid option (" << *(sig - 1) << ")\n";
		}
		narg++;
		luaL_checkstack(_l_stack, 1, "too many arguments");
	}

endwhile:
	/* do the call */
	nres = strlen(sig);  /* number of expected results */
	if (lua_pcall(_l_stack, narg, nres, 0) != 0)  /* do the call */
		std::cout << "error running function " << func << " : " << lua_tostring(_l_stack, -1);

	/* retrieve results */
	nres = -nres;  /* stack index of first result */
	while (*sig) {  /* get results */
		switch (*sig++) {

		case 'd':  /* double result */
			if (!lua_isnumber(_l_stack, nres))
				std::cout << "wrong result type\n";
			*va_arg(vl, double *) = lua_tonumber(_l_stack, nres);
			break;

		case 'i':  /* int result */
			if (!lua_isnumber(_l_stack, nres))
				std::cout << "wrong result type\n";
			*va_arg(vl, int *) = (int)lua_tonumber(_l_stack, nres);
			break;

		case 's':  /* string result */
			if (!lua_isstring(_l_stack, nres))
				std::cout << "wrong result type\n";
			*va_arg(vl, const char **) = lua_tostring(_l_stack, nres);
			break;

		default:
			std::cout << "invalid option (" << *(sig - 1) << ")\n";
		}
		nres++;
	}
	va_end(vl);
}

// ****************************************************************************
// *********************** Lua<->C++ binding functions ************************
// ****************************************************************************

// start the registration of member functions and objects
void GameData::RegisterClassStart() {
	lua_newtable(_l_stack);
	lua_pushstring(_l_stack, "__index");
	lua_pushvalue(_l_stack, -2);
	lua_settable(_l_stack, -3);
}

// end the registration of member functions and objects
void GameData::RegisterClassEnd() {
	lua_pop(_l_stack, 1);
}

// register a class' (not objects!) member function to be used from Lua
template <typename Class, typename Function>
bool GameData::RegisterMemberFunction(const char *funcname, Class *obj, Function func) {	
	lua_pushstring(_l_stack, funcname);
	lua_pushobjectdirectclosure((lua_State*)_l_stack, (Class *)obj, func, (unsigned int)0);
	lua_settable(_l_stack, -3);
	
	return true;	// add error checking laters
}

// register the object of some class (which was previously registered) to be used from Lua
template <typename Class>
bool GameData::RegisterObject(const char *objname, Class *obj) {
	lua_pushstring(_l_stack, objname);
	lua_newtable(_l_stack);
	lua_pushstring(_l_stack, "__object");
	lua_pushlightuserdata(_l_stack, (void*)obj);
	lua_settable(_l_stack, -3);
	lua_pushvalue(_l_stack, -3);
	lua_setmetatable(_l_stack, -2);
	lua_settable(_l_stack, LUA_GLOBALSINDEX);
	
	return true;	// add error checking laters
}

// register a (non-member or static) function to be used from Lua
template <typename Function>
bool GameData::RegisterFunction(const char* funcname, Function func) {
	lua_pushstring(_l_stack, funcname);
	lua_pushdirectclosure(_l_stack, func, 0);
	lua_settable(_l_stack, LUA_GLOBALSINDEX);
	
	return true;	// add error checking laters
}

// ****************************************************************************
// ************************ Lua file write functions **************************
// ****************************************************************************

// Opens a file for writing permissions.
bool GameData::WriteOpenFile(const char* file_name) {
	if (_outfile.is_open()) {
		cerr << "DATA WARNING: Tried to open a write file when another was already open for writing" << endl;
		return false;
	}
	
	_outfile.open(file_name);
	if (!_outfile) {
		cerr << "DATA ERROR: Failed to open file " << file_name << " for writing." << endl;
		return false;
	}
	return true;
}

// Closes the writing output file.
void GameData::WriteCloseFile() {
	if (!_outfile) {
		cerr << "DATA WARNING: Tried to close an output file when nothing was open" << endl;
	}
	else {
		_outfile.close();
	}
}

// ****************************************************************************
// *********************** Lua comment write functions ************************
// ****************************************************************************

void GameData::WriteInsertNewLine() {
	if (_IsWriteFileOpen()) {
		_outfile << endl;
	}
}

void GameData::WriteComment(const char* comment) {
	if (_IsWriteFileOpen()) {
		_outfile << "-- " << comment << endl;
	}
}

void GameData::WriteComment(std::string& comment) {
	if (_IsWriteFileOpen()) {
		_outfile << "-- " << comment << endl;
	}
}

void GameData::WriteBeginCommentBlock() {
	if (_IsWriteFileOpen()) {
		_outfile << "--[[" << endl;
	}
}

void GameData::WriteEndCommentBlock() {
	if (_IsWriteFileOpen()) {
		_outfile << "--]]" << endl;
	}
}

void GameData::WriteLine(const char* comment) {
	if (_IsWriteFileOpen()) {
		_outfile << comment << endl;
	}
}

void GameData::WriteLine(std::string& comment) {
	if (_IsWriteFileOpen()) {
		_outfile << comment << endl;
	}
}

// ****************************************************************************
// *********************** Lua variable write functions ***********************
// ****************************************************************************

// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
void GameData::WriteBool(const char *key, bool value) {
	if (_IsWriteFileOpen()) {
		if (_IsWriteTableOpen()) {
			_WriteTablePath();
			_outfile << '[' << key << ']' << " = ";
			if (value)
				_outfile << "true" << endl;
			else
				_outfile << "false" << endl;
		}
		else {
			_outfile << key << " = ";
			if (value)
				_outfile << "true" << endl;
			else
				_outfile << "false" << endl;
		}
	}
}

// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
void GameData::WriteInt(const char *key, int32 value) {
	if (_IsWriteFileOpen()) {
		if (_IsWriteTableOpen()) {
			_WriteTablePath();
			_outfile << '[' << key << ']' << " = " << value << endl;
		}
		else {
			_outfile << key << " = " << value << endl;
		}
	}
}

// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
void GameData::WriteFloat(const char *key, float value) {
	if (_IsWriteFileOpen()) {
		if (_IsWriteTableOpen()) {
			_WriteTablePath();
			_outfile << '[' << key << ']' << " = " << value << endl;
		}
		else {
			_outfile << key << " = " << value << endl;
		}
	}
}

// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
// TODO: Check for bad strings (ie, if it contains puncutation charcters like , or ])
void GameData::WriteString(const char *key, const char* value) {
	if (_IsWriteFileOpen()) {
		if (_IsWriteTableOpen()) {
			_WriteTablePath();
			_outfile << '[' << key << ']' << " = \"" << value << "\"" << endl;
		}
		else {
			_outfile << key << " = \"" << value << "\"" << endl;
		}
	}
}

// This will become a key of the most recently opened table. If no tables are opened, it becomes a global.
// TODO: Check for bad strings (ie, if it contains puncutation charcters like , or ])
void GameData::WriteString(const char *key, std::string& value) {
	if (_IsWriteFileOpen()) {
		if (_IsWriteTableOpen()) {
			_WriteTablePath();
			_outfile << '[' << key << ']' << " = \"" << value << "\"" << endl;
		}
		else {
			_outfile << key << " = \"" << value << "\"" << endl;
		}
	}
}

// ****************************************************************************
// ************************ Lua table write functions *************************
// ****************************************************************************

// Writes the new table name to the file and manages the state of the context
void GameData::WriteBeginTable(const char *table_name) {
	if (_IsWriteFileOpen()) {
		if (_IsWriteTableOpen()) {
			_WriteTablePath();
			_outfile << '[' << table_name << "] = {}" << endl;
		}
		else {
			_outfile << table_name << " = {}" << endl;
		}
		
		_open_tables.push_back(table_name);
	}
}

// This doesn't actually do any file write operations, but we still need to call it.
// It does internal scope handling of the lua file so things are written in the write global/table space.
void GameData::WriteEndTable() { 
	if (_IsWriteTableOpen()) {
		_open_tables.pop_back();
	}
	else {
		if (DATA_DEBUG) cerr << "DATA WARNING: Tried to close a table during writing when no table was open" << endl;
	}
}

// ****************************************************************************
// ************************ Lua vector write functions ************************
// ****************************************************************************

void GameData::WriteBoolVector(const char *key, std::vector<bool> &vect) {
	if (_IsWriteFileOpen()) {
		if (_IsWriteTableOpen()) {
			_WriteTablePath();
			_outfile << '[' << key << "] = { ";
		}
		else {
			_outfile << key << " = { ";
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
}

void GameData::WriteIntVector(const char *key, std::vector<int32> &vect) {
	if (vect.size() == 0) {
		if (DATA_DEBUG) cerr << "DATA WARNING: passed a vector of size zero to WriteIntVector()" << endl;
		return;
	}
	
	if (_IsWriteFileOpen()) {
		if (_IsWriteTableOpen()) {
			_WriteTablePath();
			_outfile << '[' << key << "] = { ";
		}
		else {
			_outfile << key << " = { ";
		}
		
		_outfile << vect[0];
		for (uint32 i = 1; i < vect.size(); i++) {
			_outfile << ", " << vect[i];
		}
		_outfile << " }" << endl;
	}
}

void GameData::WriteFloatVector(const char *key, std::vector<float> &vect) {
	if (vect.size() == 0) {
		if (DATA_DEBUG) cerr << "DATA WARNING: passed a vector of size zero to WriteFloatVector()" << endl;
		return;
	}
	
	if (_IsWriteFileOpen()) {
		if (_IsWriteTableOpen()) {
			_WriteTablePath();
			_outfile << '[' << key << "] = { ";
		}
		else {
			_outfile << key << " = { ";
		}
		
		_outfile << vect[0];
		for (uint32 i = 1; i < vect.size() - 1; i++) {
			_outfile << ", " << vect[i];
		}
		_outfile << " }" << endl;
	}
}

// TODO: Check for bad strings (ie, if it contains puncutation charcters like , or ])
void GameData::WriteStringVector(const char *key, std::vector<std::string> &vect) {
	if (vect.size() == 0) {
		if (DATA_DEBUG) cerr << "DATA WARNING: passed a vector of size zero to WriteStringVector()" << endl;
		return;
	}
	
	if (_IsWriteFileOpen()) {
		if (_IsWriteTableOpen()) {
			_WriteTablePath();
			_outfile << '[' << key << "] = { ";
		}
		else {
			_outfile << key << " = { ";
		}
		
		_outfile << "\"" << vect[0] << "\"";
		for (uint32 i = 1; i < vect.size(); i++) {
			_outfile << ", \"" << vect[i] << "\"";
		}
		_outfile << " }" << endl;
	}
}

// ****************************************************************************
// ************************** Lua debugging functions *************************
// ****************************************************************************

// This function is for DEBUGGING PURPOSES ONLY! It prints the contents of the Lua stack from top to bottom.
void GameData::DEBUG_PrintLuaStack() {
	int32 type;

	cout << "DEBUG: Printing lua stack" << endl;
	for (int32 i = lua_gettop(_l_stack); i > 0; i--) {  // Print each element starting from the top
		type = lua_type(_l_stack, i);
		switch (type) {
		case LUA_TNIL:
			cout << "WARNING: NIL" << endl;
			break;
		case LUA_TBOOLEAN:
			cout << "BOOLEAN: " << lua_toboolean(_l_stack, i) << endl;
			break;
		case LUA_TNUMBER:
			cout << "NUMBER:  " << lua_tonumber(_l_stack, i) << endl;
			break;
		case LUA_TSTRING:
			cout << "STRING:  " << lua_tostring(_l_stack, i) << endl;
			break;
		case LUA_TTABLE:
			cout << "TABLE    " << endl;
			break;
		case LUA_TFUNCTION:
			cout << "FUNCTION " << endl;
			break;
		default:
			cout << "OTHER:   " << lua_typename(_l_stack, type) << endl;
			break;
		}
	}
}

} // namespace hoa_data
