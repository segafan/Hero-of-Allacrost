///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    defs.h
*** \author  Daniel Steuernol, steu@allacrost.org
*** \brief   Source file for Lua binding code
***
*** All binding code for the game engine is contained within this file.
*** Therefore, everything that you see bound within this file will be made
*** available in Lua. All binding code is contained within this single file
*** because the binding code greatly increases the compilation time, but this
*** effect is mitigated if it is contained within a single file (Note: Binding
*** is now split out according to dependency level (engine, global, modes).
***
*** \note To most C++ programmers, the syntax of the binding code found in this
*** file may be very unfamiliar and obtuse. Refer to the Luabind documentation
*** as necessary to gain an understanding of this code style.
*** **************************************************************************/

#include "defs.h"

#include "audio.h"
#include "input.h"
#include "mode_manager.h"
#include "script.h"
#include "system.h"
#include "video.h"

#include "global.h"

using namespace luabind;

namespace hoa_defs {

void BindEngineToLua()
{
	// ----- Audio Engine Bindings
	{
	using namespace hoa_audio;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_audio")
	[
		class_<GameAudio>("GameAudio")
			.def("PlaySound", &GameAudio::PlaySound)
	];

	} // End using audio namespaces



	// ----- Input Engine Bindings
	{
	using namespace hoa_input;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_input")
	[
		class_<GameInput>("GameInput")
	];

	} // End using input namespaces



	// ----- Mode Manager Engine Bindings
	{
	using namespace hoa_mode_manager;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_mode_manager")
	[
		class_<GameMode>("GameMode")
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_mode_manager")
	[
		class_<GameModeManager>("GameModeManager")
			.def("Push", &GameModeManager::Push, adopt(_2))
			.def("Pop", &GameModeManager::Pop)
			.def("PopAll", &GameModeManager::PopAll)
			.def("GetTop", &GameModeManager::GetTop)
			.def("Get", &GameModeManager::Get)
			.def("GetGameType", (uint8 (GameModeManager::*)(uint32))&GameModeManager::GetGameType)
			.def("GetGameType", (uint8 (GameModeManager::*)())&GameModeManager::GetGameType)
	];

	} // End using mode manager namespaces



	// ----- Script Engine Bindings
	{
	using namespace hoa_script;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_script")
	[
		class_<GameScript>("GameScript")
	];

	} // End using script namespaces



	// ----- System Engine Bindings
	{
	using namespace hoa_system;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_system")
	[
		class_<GameSystem>("GameSystem")
			.def("GetUpdateTime", &GameSystem::GetUpdateTime)
			.def("SetPlayTime", &GameSystem::SetPlayTime)
			.def("GetPlayHours", &GameSystem::GetPlayHours)
			.def("GetPlayMinutes", &GameSystem::GetPlayMinutes)
			.def("GetPlaySeconds", &GameSystem::GetPlaySeconds)
			.def("GetLanguage", &GameSystem::GetLanguage)
			.def("SetLanguage", &GameSystem::SetLanguage)
			.def("NotDone", &GameSystem::NotDone)
			.def("ExitGame", &GameSystem::ExitGame)
	];

	} // End using system namespaces



	// ----- Video Engine Bindings
	{
	using namespace hoa_video;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_video")
	[
		class_<GameVideo>("GameVideo")
	];

	} // End using video namespaces

	// ---------- Bind engine class objects
	luabind::object global_table = luabind::globals(hoa_script::ScriptManager->GetGlobalState());
	global_table["AudioManager"]     = hoa_audio::AudioManager;
	global_table["InputManager"]     = hoa_input::InputManager;
	global_table["ModeManager"]      = hoa_mode_manager::ModeManager;
	global_table["ScriptManager"]    = hoa_script::ScriptManager;
	global_table["SystemManager"]    = hoa_system::SystemManager;
	global_table["VideoManager"]     = hoa_video::VideoManager;
} // BindEngineToLua

} // namespace hoa_defs
