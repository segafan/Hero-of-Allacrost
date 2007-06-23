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
*** \author  Daniel Steuernol - steu@allacrost.org,
***          Tyler Olsen - roots@allacrost.org
***
*** \brief   Source file for the scripting engine.
*** ***************************************************************************/

#include <iostream>
#include <stdarg.h>

#include "script.h"

using namespace std;
using namespace luabind;

using namespace hoa_utils;
using namespace hoa_script::private_script;

template<> hoa_script::GameScript* Singleton<hoa_script::GameScript>::_singleton_reference = NULL;

namespace hoa_script {

GameScript* ScriptManager = NULL;
bool SCRIPT_DEBUG = false;

//-----------------------------------------------------------------------------
// GameScript Class Functions
//-----------------------------------------------------------------------------

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
