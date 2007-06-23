///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    script_modify.cpp
*** \author  Daniel Steuernol - steu@allacrost.org,
***          Tyler Olsen - roots@allacrost.org
*** \brief   Source file for the ModifyScriptDescriptor class.
*** ***************************************************************************/

#include "script.h"
#include "script_modify.h"
#include "script_read.h"
#include "script_write.h"

using namespace std;
using namespace luabind;

using namespace hoa_utils;

namespace hoa_script {

ModifyScriptDescriptor::~ModifyScriptDescriptor() {
	if (IsFileOpen()) {
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT WARNING: ModifyScriptDescriptor destructor was called when file was still open: "
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

bool ModifyScriptDescriptor::OpenFile(const std::string& file_name) {
	if (ScriptManager->IsFileOpen(file_name) == true) {
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT WARNING: ModifyScriptDescriptor::OpenFile() attempted to open file that is already opened: "
				<< file_name << endl;
		return false;
	}

	// Increases the global stack size by 1 element. That is needed because the new thread will be pushed in the
	// stack and we have to be sure there is enough space there.
	lua_checkstack(ScriptManager->GetGlobalState(),1);
	_lstack = lua_newthread(ScriptManager->GetGlobalState());

	// Attempt to load the Lua file.
	if (lua_dofile(_lstack, file_name.c_str()) != 0) {
		cerr << "SCRIPT ERROR: ModifyScriptDescriptor::OpenFile() could not open the file " << file_name << endl;
		_access_mode = SCRIPT_CLOSED;
		return false;
	}

	_filename = file_name;
	_access_mode = SCRIPT_READ;
	ScriptManager->_AddOpenFile(this);
	return true;
} // bool ModifyScriptDescriptor::OpenFile(std::string file_name)



bool ModifyScriptDescriptor::OpenFile() {
	if (_filename == "") {
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT ERROR: ModifyScriptDescriptor::OpenFile(), could not open file "
				<< "because of an invalid file name (empty string)." << endl;
		return false;
	}

	return OpenFile(_filename);
}



void ModifyScriptDescriptor::CloseFile() {
	if (IsFileOpen() == false) {
		if (SCRIPT_DEBUG)
			cerr << "SCRIPT ERROR: ModifyScriptDescriptor::CloseFile() could not close the "
				<< "file because it was not open." << endl;
		return;
	}

	if (SCRIPT_DEBUG && IsErrorDetected()) {
		cerr << "SCRIPT WARNING: In ModifyScriptDescriptor::CloseFile(), the file " << _filename
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
// Modify Function Definitions
//-----------------------------------------------------------------------------

void ModifyScriptDescriptor::CommitChanges() {
	WriteScriptDescriptor write_file;
	if (write_file.OpenFile(_filename) == false)
		return;
	object o(luabind::from_stack(_lstack, LUA_GLOBALSINDEX));
	DEBUG_PrintGlobals();
	for (luabind::iterator it(o), end; it != end; ++it) {
		switch(luabind::type(*it)) {
		case LUA_TBOOLEAN:
// 			write_file.WriteBool(object_cast<string>(it.key()), object_cast<bool>(*it));
			break;
		case LUA_TNUMBER:
// 			write_file.WriteFloat(object_cast<string>(it.key()), object_cast<float>(*it));
			break;
		case LUA_TSTRING:
// 			write_file.WriteString(object_cast<string>(it.key()), object_cast<string>(*it));
			break;
		case LUA_TTABLE:
//			this->_SaveStackProcessTable(write_file, object_cast<string>(it.key()), object(*it));
			break;
		}
	}
	write_file.CloseFile();
}



void ModifyScriptDescriptor::_SaveStackProcessTable(WriteScriptDescriptor& write_file, const string &name, luabind::object table) {
	write_file.BeginTable(name.c_str());
	for (luabind::iterator it(table), end; it != end; ++it) {
		switch(luabind::type(*it)) {
		case LUA_TBOOLEAN:
// 			write_file.WriteBool(object_cast<string>(it.key()), object_cast<bool>(*it));
			break;
		case LUA_TNUMBER:
// 			write_file.WriteFloat(object_cast<string>(it.key()), object_cast<float>(*it));
			break;
		case LUA_TSTRING:
// 			write_file.WriteString(object_cast<string>(it.key()), object_cast<string>(*it));
			break;
		case LUA_TTABLE:
// 			this->_SaveStackProcessTable(write_file, object_cast<string>(it.key()), object(*it));
			break;
		}
	}
	write_file.EndTable();
}

} // namespace hoa_script
