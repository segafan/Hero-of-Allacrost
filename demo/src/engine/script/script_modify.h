///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    script_modify.h
*** \author  Daniel Steuernol - steu@allacrost.org,
***          Tyler Olsen - roots@allacrost.org
*** \brief   Header file for the ModifyScriptDescriptor class.
*** ***************************************************************************/

#ifndef __SCRIPT_MODIFY_HEADER__
#define __SCRIPT_MODIFY_HEADER__

#include "utils.h"
#include "defs.h"

#include "script.h"
#include "script_read.h"

namespace hoa_script {

/** ****************************************************************************
*** \brief Represents a Lua file opened with read, execute, and modify permissions.
***
*** This class features all the functionality found in the ReadScriptDescriptor
*** class, plus the additional ability to modify existing Lua data and save it
*** back to the file. An object of this class should only be created if the user
*** intends to modify the Lua data in the file at some point (i.e. don't use
*** this class over the ReadScriptDescriptor simply because it has more
*** functionality available).
***
*** In order to permanently (and irreversibly) change Lua data in the file, the
*** user must call the CommitChanges function after making one or several
*** ModifyData calls.
***
*** \note This class and is features are still highly experimental and incomplete.
***
*** \todo Add ability to modify tables and their data
*** ***************************************************************************/
class ModifyScriptDescriptor : public ReadScriptDescriptor {
	friend class GameScript;
public:
	~ModifyScriptDescriptor();

	/** \name File Access Functions
	*** \note These are derived from ScriptDescriptor, refer to the comments for these 
	*** methods in the header file for that class.
	**/
	//@{
	bool OpenFile(const std::string& file_name);
	bool OpenFile();
	void CloseFile();
	//@}

	/** \brief This function updates the global table with the specified key, value pair.
	*** \param key The key name of the variable to be change
	*** \param variable The new value to set the key
	*** \note If the key varname does not exist in the lua stack, it will be added as a new key
	*** with the specified value.
	**/
	template <class T> void ModifyData(const std::string& key, T variable);

	//! \brief Commits all modified changes to the Lua file for permanent retention
	void CommitChanges();

private:
	/** \brief Functions to print out a table during stack output.
	*** \todo Roots: I honestly have no idea what the purpose of this function is, since its not called
	*** from anywhere
	***
	**/
	void _SaveStackProcessTable(WriteScriptDescriptor& sd, const std::string &name, luabind::object table);
}; // class ModifyScriptDescriptor

//-----------------------------------------------------------------------------
// Template Function Definitions
//-----------------------------------------------------------------------------

template <class T> void ModifyScriptDescriptor::ModifyData(const std::string& key, T variable) {
	if (_open_tables.empty() == false) // Get from the table of globals
		std::cerr << "ModifyScriptDescriptor::ModifyData() does not support table elements yet!" << std::endl;

	luabind::object o(luabind::from_stack(_lstack, LUA_GLOBALSINDEX));

	for (luabind::iterator it(o), end; it != end; ++it) {
		// Check to see if global variable exists
		if (luabind::object_cast<std::string>(it.key()) == key) {
			// Change the global variable if it is found
			*it = variable;
			return;
		}
	}

	// If we arrive here, then varname does not exist in the globals so add it
	luabind::settable(o, key, variable);
} // template <class T> void ModifyScriptDescriptor::ChangeSetting(const std::string &varname, T variable)

} // namespace hoa_script

#endif // __SCRIPT_MODIFY_HEADER
