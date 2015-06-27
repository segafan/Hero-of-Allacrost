///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    common.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for common code shared across the source tree
*** ***************************************************************************/

#ifndef __COMMON_HEADER__
#define __COMMON_HEADER__

#include <string.h>

namespace hoa_common {

//! \brief Determines whether the code in the hoa_common namespace should print debug statements or not.
extern bool COMMON_DEBUG;

/** \brief Returns the tablespace name of a given Lua file
*** \param filename The fully qualified name of the Lua file
***
*** Many of our Lua files need to have their contents encapsulated in a table that we call the file's tablespace
*** (because the table serves the same function as a namespace in C++). The tablespace's purpose is to prevent
*** any name collision of variables, functions, and so on between multiple Lua files. The tablespace is near the top
*** of the Lua file and looks like this:
*** 
*** local ns = {}
*** setmetatable(ns, {__index = _G})
*** my_tablespace_name = ns;
*** setfenv(1, ns);
***
*** Our convention is to name the tablespace after the name of the file it is contained in, so "lua/data/my_file.lua"
*** would have a tablespace name of "my_file". This function takes the filename and returns the expected name of its
*** tablespace.
*** 
*** \note Not all Lua files have a tablespace. This function does not tell you whether or not a file has a tablespace.
*** 
*** \note Lua identifiers must start with a character, so in the case of files that start with a non-alphabetic character such
*** as a number, the tablespace name is prepended with an 'a'
**/
std::string DetermineLuaFileTablespaceName(const std::string& filename);

} // namespace hoa_common

#endif // __COMMON_HEADER__
