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

	/** \name Data Modifier Functions
	*** \brief These functions allow the modification of primitive data types
	*** \param key The string or integer key of the variable to change
	*** \param value The value to change the variable to
	*** \note If the key could not be found in the active scope, no change will
	*** occur and an error message will be logged.
	**/
	//@{
	void ModifyBool(const std::string& key, bool value)
		{ _ModifyData(key, value); }

	void ModifyBool(int32 key, bool value)
		{ _ModifyData(key, value); }
	
	void ModifyInt(const std::string& key, int32 value)
		{ _ModifyData(key, value); }
	
	void ModifyInt(int32 key, int32 value)
		{ _ModifyData(key, value); }
	
	void ModifyUInt(const std::string& key, uint32 value)
		{ _ModifyData(key, value); }

	void ModifyUInt(int32 key, uint32 value)
		{ _ModifyData(key, value); }

	void ModifyFloat(const std::string& key, float value)
		{ _ModifyData(key, value); }
	
	void ModifyFloat(int32 key, float value)
		{ _ModifyData(key, value); }
	
	void ModifyString(const std::string& key, const std::string& value)
		{ _ModifyData(key, value); }
	
	void ModifyString(int32 key, const std::string& value)
		{ _ModifyData(key, value); }
	//@}

	/** \name Data Addition Functions
	*** \brief These functions allow the addition of new key/value pairs
	*** \param key The string or integer key of the variable to add
	*** \param value The value to set the new variable to
	*** \note If the key already exists in the active scope when these
	*** methods are called, the value of that key will be overwritten
	*** with the new value. There will be no indication that this type of
	*** overwrite operation took place, so be cautious.
	**/
	//@{
	void AddNewBool(const std::string& key, bool value)
		{ _AddNewData(key, value); }

	void AddNewBool(int32 key, bool value)
		{ _AddNewData(key, value); }
	
	void AddNewInt(const std::string& key, int32 value)
		{ _AddNewData(key, value); }
	
	void AddNewInt(int32 key, int32 value)
		{ _AddNewData(key, value); }
	
	void AddNewUInt(const std::string& key, uint32 value)
		{ _AddNewData(key, value); }

	void AddNewUInt(int32 key, uint32 value)
		{ _AddNewData(key, value); }

	void AddNewFloat(const std::string& key, float value)
		{ _AddNewData(key, value); }
	
	void AddNewFloat(int32 key, float value)
		{ _AddNewData(key, value); }
	
	void AddNewString(const std::string& key, const std::string& value)
		{ _AddNewData(key, value); }
	
	void AddNewString(int32 key, const std::string& value)
		{ _AddNewData(key, value); }
	//@}

	// TODO: Add methods to allow the creation of new tables
// 	void CreateNewTable(const std::string& key);
// 	void CreateNewTable(uint32 key);
// 	void EndNewTable();

	//! \brief Commits all modified changes to the Lua file for permanent retention
	void CommitChanges();

private:
	/** \brief Functions to print out a table during stack output.
	*** \todo Roots: I honestly have no idea what the purpose of this function is, since its not called
	*** from anywhere
	***
	**/
	void _SaveStackProcessTable(WriteScriptDescriptor& sd, const std::string &name, luabind::object table);

	/** \brief Template functions that update the key, value pair for the most recently opened table, or in the global scope
	*** \param key The key name of the variable to be change
	*** \param value The new value to set the key
	**/
	//@{
	template <class T> void _ModifyData(const std::string& key, T value);
	template <class T> void _ModifyData(int32 key, T value);
	//@}

	/** \brief Template functions which add a new key, value pair to the most recently opened scope
	*** \param key The key name of the variable to add
	*** \param value The value to set for the new variable
	**/
	//@{
	template <class T> void _AddNewData(const std::string& key, T value);
	template <class T> void _AddNewData(int32 key, T value);
	//@}
}; // class ModifyScriptDescriptor

//-----------------------------------------------------------------------------
// Template Function Definitions
//-----------------------------------------------------------------------------

template <class T> void ModifyScriptDescriptor::_ModifyData(const std::string& key, T value) {
	// A pointer to the table where the object is contained
	luabind::object* table = NULL;

	if (_open_tables.empty() == false) // Retrieve the globals table
		table = new luabind::object(luabind::from_stack(_lstack, LUA_GLOBALSINDEX));
	else // Retrieve the most recently opened table from the top of the stack
		table = new luabind::object(luabind::from_stack(_lstack, private_script::STACK_TOP));

	if (luabind::type(*table) != LUA_TTABLE) {
		_error_messages << "* _ModifyData() failed because it could not construct the table "
			<< "where the data resided: " << key << std::endl;
		delete(table);
		return;
	}

	for (luabind::iterator i(*table); i != private_script::TABLE_END; ++i) {
		// Check to see if global value exists
		try {
			if (luabind::object_cast<std::string>(i.key()) == key) {
				*i = value;
				delete(table);
				return;
			}
		}
		catch (...)  {
			// A cast failure does not indicate an error, simply continue on with the next key
		}
	}

	// If the code reaches this far, then the variable could not be found
	_error_messages << "* _ModifyData() failed because in the active scope, it did not find the "
		<< "table key: " << key << std::endl;
	delete(table);
} // template <class T> void ModifyScriptDescriptor::_ModifyData(const std::string& key, T value) {



template <class T> void ModifyScriptDescriptor::_ModifyData(int32 key, T value) {
	if (_open_tables.empty() == false) {
		_error_messages << "* _ModifyData() failed because there were no open tables when the "
			<< "function was invoked for key: " << key << std::endl;
		return;
	}

	luabind::object table(luabind::from_stack(_lstack, private_script::STACK_TOP));

	if (luabind::type(table) != LUA_TTABLE) {
		_error_messages << "* _ModifyData() failed because the top of the stack was not a table "
			<< "when trying to modify the data named: " << key << std::endl;
		return;
	}

	for (luabind::iterator i(table); i != private_script::TABLE_END; ++i) {
		// Check to see if global value exists
		try {
			if (luabind::object_cast<int32>(i.key()) == key) {
				*i = value;
				delete(table);
				return;
			}
		}
		catch (...)  {
			// A cast failure does not indicate an error, simply continue on with the next key
		}
	}

	// If the code reaches this far, then the variable could not be found
	_error_messages << "* _ModifyData() failed because in the active scope, it did not find the "
		<< "table key: " << key << std::endl;
	delete(table);
} // template <class T> void ModifyScriptDescriptor::_ModifyData(int32 key, T value) {



template <class T> void ModifyScriptDescriptor::_AddNewData(const std::string& key, T value) {
	// A pointer to the table where the object is contained
	luabind::object* table = NULL;

	if (_open_tables.empty() == false) // Retrieve the globals table
		table = new luabind::object(luabind::from_stack(_lstack, LUA_GLOBALSINDEX));
	else // Retrieve the most recently opened table from the top of the stack
		table = new luabind::object(luabind::from_stack(_lstack, private_script::STACK_TOP));

	if (luabind::type(*table) != LUA_TTABLE) {
		_error_messages << "* _AddNewData() failed because the top of the stack was not a table "
			<< "when trying to add the new data: " << key << std::endl;
		return;
	}

	// NOTE: If the key already exists in the table, its value will be overwritten here
	luabind::settable(*table, key, value);
} // template <class T> void ModifyScriptDescriptor::_AddNewData(const std::string& key, T value)



template <class T> void ModifyScriptDescriptor::_AddNewData(int32 key, T value) {
	if (_open_tables.empty() == false) {
		_error_messages << "* _AddNewData() failed because there were no open tables when the "
			<< "function was invoked for key: " << key << std::endl;
		return;
	}

	// A pointer to the table where the object is contained
	luabind::object table(luabind::from_stack(_lstack, private_script::STACK_TOP));

	if (luabind::type(table) != LUA_TTABLE) {
		_error_messages << "* _AddNewData() failed because the top of the stack was not a table "
			<< "when trying to add the new data: " << key << std::endl;
		return;
	}

	// NOTE: If the key already exists in the table, its value will be overwritten here
	luabind::settable(table, key, value);
} // template <class T> void ModifyScriptDescriptor::_AddNewData(int32 key, T value)

} // namespace hoa_script

#endif // __SCRIPT_MODIFY_HEADER
