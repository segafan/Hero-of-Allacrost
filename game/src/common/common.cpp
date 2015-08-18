///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    common.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for common code shared across the source tree
*** ***************************************************************************/

// Allacrost utilties
#include "utils.h"

// Local common headers
#include "common.h"

using namespace std;

namespace hoa_common {

bool COMMON_DEBUG = false;

string DetermineLuaFileTablespaceName(const string& filename) {
	if (filename.empty()) {
		IF_PRINT_WARNING(COMMON_DEBUG) << "function received an empty string argument" << endl;
		return "";
	}
	
	string tablespace_name;
	int32 last_slash_index = filename.find_last_of("/");
	int32 period_index = filename.find(".");
	tablespace_name = filename.substr(last_slash_index + 1, period_index - (last_slash_index + 1));
	
	if (tablespace_name.empty()) {
		IF_PRINT_WARNING(COMMON_DEBUG) << "function received an unexpected filename string: " << filename << endl;
		return "";
	}
	
	// Lua identifiers must start with a letter. If the filename starts with a nonalphabetic character,
	// our convention is to prepend "a" to the tablespace name in that file
	char first = tablespace_name[0];
	if ((first < 'A' || first > 'Z') && (first < 'a' || first > 'z')) {
		tablespace_name = 'a' + tablespace_name;
	}
	return tablespace_name;
}

} // namespace hoa_common
