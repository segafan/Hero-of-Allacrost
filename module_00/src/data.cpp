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

#include "utils.h"
#include <iostream>
#include "data.h"
#include "audio.h"
#include "video.h"
#include "engine.h"
#include "boot.h"
#include "map.h"


using namespace std;
using namespace hoa_engine;
using namespace hoa_video;
using namespace hoa_audio;
using namespace hoa_utils;
using namespace hoa_map;

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

void GameData::OpenLuaFile(const char* file_name) {
	if (luaL_loadfile(_l_stack, file_name) || lua_pcall(_l_stack, 0, 0, 0))
		cerr << "DATA ERROR: Could not load " << file_name << " :: " <<
		        lua_tostring(_l_stack, -1) << endl;
}

void GameData::OpenTable(const char* tbl_name) {
	lua_getglobal(_l_stack, tbl_name);
	if (!lua_istable(_l_stack, LUA_STACK_TOP))
		cerr << "DATA ERROR: could not retrieve table \"" << tbl_name << "\"" << endl;
}

void GameData::CloseTable() {
	lua_pop(_l_stack, 1);
}

// Get an individual boolean field from a table, assumed to be on top of the stack
void GameData::GetTableBool (const char *key, bool &ref) {
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isboolean(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	ref = (0 != lua_toboolean(_l_stack, -1));
	lua_pop(_l_stack, 1);
}

// Get an individual integer field from a table, assumed to be on top of the stack
void GameData::GetTableInt (const char *key, int32 &ref) {
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isnumber(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	ref = (int32)lua_tonumber(_l_stack, -1);
	lua_pop(_l_stack, 1);
}

// Get an individual floating point field from a table, assumed to be on top of the stack
void GameData::GetTableFloat (const char *key, float &ref) {
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isnumber(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	ref = (float)lua_tonumber(_l_stack, -1);
	lua_pop(_l_stack, 1);
}

// Get an individual string field from a table, assumed to be on top of the stack
void GameData::GetTableString (const char *key, string &ref) {
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isstring(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	ref = (string)lua_tostring(_l_stack, -1);
	lua_pop(_l_stack, 1);
}



void GameData::GetGlobalBool(const char *key, bool &ref) {
	lua_getglobal(_l_stack, key);
	ref = (0 != lua_toboolean(_l_stack, LUA_STACK_TOP));
	lua_pop(_l_stack, 1);
}

void GameData::GetGlobalInt(const char *key, int32 &ref) {
	lua_getglobal(_l_stack, key);
	ref = (int32)lua_tonumber(_l_stack, LUA_STACK_TOP);
	lua_pop(_l_stack, 1);
}

void GameData::GetGlobalFloat(const char *key, float &ref) {
	lua_getglobal(_l_stack, key);
	ref = (float)lua_tonumber(_l_stack, LUA_STACK_TOP);
	lua_pop(_l_stack, 1);
}

void GameData::GetGlobalString(const char * key, string &ref) {
	lua_getglobal(_l_stack, key);
	ref = (string)lua_tostring(_l_stack, LUA_STACK_TOP);
	lua_pop(_l_stack, 1);
}

// TODO L8R: Maybe turn this function into FillVector(vector*, const char*, int lua_type),
//           which will fill a custom vector?
void GameData::FillStringVector(const char *key, vector<string> &vect) {
	lua_getglobal(_l_stack, key);
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: table " << key << " does not exist, or " << key << "isn't a table\n";
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
		cerr << "DATA ERROR: table " << key << " does not exist, or " << key << "isn't a table\n";
		return;
	}
	int32 t = lua_gettop(_l_stack);
	lua_pushnil(_l_stack);
	while (lua_next(_l_stack, t)) {
		vect.push_back((int32)lua_tonumber(_l_stack, LUA_STACK_TOP));
		lua_pop(_l_stack, 1);
	}
}


// This function is for DEBUGGING PURPOSES ONLY! It prints the contents of the Lua stack from top to bottom.
void GameData::PrintLuaStack() {
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
