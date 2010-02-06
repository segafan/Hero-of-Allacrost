///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    defs.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for Lua binding code
***
*** All binding code is contained within this file and this file alone.
*** Therefore, everything that you see bound within this file will be made
*** available in Lua. All binding code is contained within this single file
*** because the binding code greatly increases the compilation time, but this
*** effect is mitigated if it is contained within a single file.
***
*** \note To most C++ programmers, the syntax of the binding code found in this
*** file may be very unfamiliar and obtuse. Refer to the Luabind documentation
*** as necessary to gain an understanding of this code style.
*** **************************************************************************/

#include "utils.h"
#include "defs.h"

#include "script.h"

using namespace luabind;

namespace hoa_defs {

void BindUtilsToLua() {
	// ---------- Bind Util Functions
	{
	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_utils")
	[
		def("RandomFloat", (float(*)(void)) &hoa_utils::RandomFloat)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_utils")
	[
		def("RandomBoundedInteger", &hoa_utils::RandomBoundedInteger)
	];
	}
} // void BindEngineToLua()

} // namespace hoa_defs
