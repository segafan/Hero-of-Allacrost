///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
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
*** All binding code for the globals is contained within this file.
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

#include "battle.h"
#include "battle_actors.h"
#include "map.h"
#include "map_actions.h"
#include "map_dialogue.h"
#include "map_objects.h"
#include "map_sprites.h"
#include "map_treasure.h"
#include "map_zones.h"
#include "shop.h"

using namespace luabind;

namespace hoa_defs {

void BindModesToLua()
{
	// ----- Map Mode Bindings
	{
	using namespace hoa_map;
	using namespace hoa_map::private_map;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapMode, hoa_mode_manager::GameMode>("MapMode")
			.def(constructor<const std::string&>())
			.def_readwrite("_camera", &MapMode::_camera)
			.def_readwrite("_ignore_input", &MapMode::_ignore_input)
			.def_readwrite("_run_forever", &MapMode::_run_forever)
			.def_readwrite("_run_disabled", &MapMode::_run_disabled)
			.def_readwrite("_run_stamina", &MapMode::_run_stamina)
			.def_readonly("_map_event_group", &MapMode::_map_event_group)
			.def("_AddGroundObject", &MapMode::_AddGroundObject, adopt(_2))
			.def("_AddPassObject", &MapMode::_AddPassObject, adopt(_2))
			.def("_AddSkyObject", &MapMode::_AddSkyObject, adopt(_2))
			.def("_AddZone", &MapMode::_AddZone, adopt(_2))
			.def("_SetCameraFocus", &MapMode::_SetCameraFocus)
			.def("_SetMapState", &MapMode::_SetMapState)
			.def("_GetMapState", &MapMode::_GetMapState)
			.def("_GetGeneratedObjectID", &MapMode::_GetGeneratedObjectID)
			.def("_DrawMapLayers", &MapMode::_DrawMapLayers)

			.scope
			[
				def("_ShowDialogueIcons", &MapMode::_ShowDialogueIcons),
				def("_IsShowingDialogueIcons", &MapMode::_IsShowingDialogueIcons)
			]

			// Namespace constants
			.enum_("constants") [
				// Map states
				value("EXPLORE", EXPLORE),
				value("DIALOGUE", DIALOGUE),
				value("OBSERVATION", OBSERVATION),
				// Object types
				value("PHYSICAL_TYPE", PHYSICAL_TYPE),
				value("VIRTUAL_TYPE", VIRTUAL_TYPE),
				value("SPRITE_TYPE", SPRITE_TYPE),
				// Sprite directions
				value("NORTH", NORTH),
				value("SOUTH", SOUTH),
				value("EAST", EAST),
				value("WEST", WEST),
				value("NW_NORTH", NW_NORTH),
				value("NW_WEST", NW_WEST),
				value("NE_NORTH", NE_NORTH),
				value("NE_EAST", NE_EAST),
				value("SW_SOUTH", SW_SOUTH),
				value("SW_WEST", SW_WEST),
				value("SE_SOUTH", SE_SOUTH),
				value("SE_EAST", SE_EAST),
				// Sprite animations
				value("ANIM_STANDING_SOUTH", ANIM_STANDING_SOUTH),
				value("ANIM_STANDING_NORTH", ANIM_STANDING_NORTH),
				value("ANIM_STANDING_WEST", ANIM_STANDING_WEST),
				value("ANIM_STANDING_EAST", ANIM_STANDING_EAST),
				value("ANIM_WALKING_SOUTH", ANIM_WALKING_SOUTH),
				value("ANIM_WALKING_NORTH", ANIM_WALKING_NORTH),
				value("ANIM_WALKING_WEST", ANIM_WALKING_WEST),
				value("ANIM_WALKING_EAST", ANIM_WALKING_EAST),
				// Sprite speeds
				value("VERY_SLOW_SPEED", static_cast<uint32>(VERY_SLOW_SPEED)),
				value("SLOW_SPEED", static_cast<uint32>(SLOW_SPEED)),
				value("NORMAL_SPEED", static_cast<uint32>(NORMAL_SPEED)),
				value("FAST_SPEED", static_cast<uint32>(FAST_SPEED)),
				value("VERY_FAST_SPEED", static_cast<uint32>(VERY_FAST_SPEED)),
				// Map dialogues
				value("DIALOGUE_INFINITE", DIALOGUE_INFINITE)
			]
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapObject>("MapObject")
			.def("SetObjectID", &MapObject::SetObjectID)
			.def("SetContext", &MapObject::SetContext)
			.def("SetXPosition", &MapObject::SetXPosition)
			.def("SetYPosition", &MapObject::SetYPosition)
			.def("SetImgHalfWidth", &MapObject::SetImgHalfWidth)
			.def("SetImgHeight", &MapObject::SetImgHeight)
			.def("SetCollHalfWidth", &MapObject::SetCollHalfWidth)
			.def("SetCollHeight", &MapObject::SetCollHeight)
			.def("SetUpdatable", &MapObject::SetUpdatable)
			.def("SetVisible", &MapObject::SetVisible)
			.def("SetNoCollision", &MapObject::SetNoCollision)
			.def("SetDrawOnSecondPass", &MapObject::SetDrawOnSecondPass)
			.def("GetObjectID", &MapObject::GetObjectID)
			.def("GetContext", &MapObject::GetContext)
//			.def("GetXPosition", &MapObject::GetXPosition)
//			.def("GetYPosition", &MapObject::GetYPosition)
			.def("GetImgHalfWidth", &MapObject::GetImgHalfWidth)
			.def("GetImgHeight", &MapObject::GetImgHeight)
			.def("GetCollHalfWidth", &MapObject::GetCollHalfWidth)
			.def("GetCollHeight", &MapObject::GetCollHeight)
			.def("IsUpdatable", &MapObject::IsUpdatable)
			.def("IsVisible", &MapObject::IsVisible)
			.def("IsNoCollision", &MapObject::IsNoCollision)
			.def("IsDrawOnSecondPass", &MapObject::IsDrawOnSecondPass)
			// TEMP: because GetXPosition and GetYPostiion seem to give a runtime error in Lua
			.def_readonly("x_position", &MapObject::x_position)
			.def_readonly("y_position", &MapObject::y_position)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<PhysicalObject, MapObject>("PhysicalObject")
			.def(constructor<>())
			.def("AddAnimation", &PhysicalObject::AddAnimation)
			.def("SetCurrentAnimation", &PhysicalObject::SetCurrentAnimation)
			.def("SetAnimationProgress", &PhysicalObject::SetAnimationProgress)
			.def("GetCurrentAnimation", &PhysicalObject::GetCurrentAnimation)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapTreasure, PhysicalObject>("MapTreasure")
			.def(constructor<std::string, uint8>())
			.def(constructor<std::string, uint8, uint8, uint8>())
			.def("AddObject", &MapTreasure::AddObject)
			.def("AddDrunes", &MapTreasure::AddDrunes)
			.def("IsEmpty", &MapTreasure::IsEmpty)
			.def("Open", &MapTreasure::Open)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<VirtualSprite, MapObject>("VirtualSprite")
			.def(constructor<>())
			.def("SetDirection", &VirtualSprite::SetDirection)
			.def("SetMovementSpeed", &VirtualSprite::SetMovementSpeed)
			.def("SetFacePortrait", &VirtualSprite::SetFacePortrait)
			.def("GetDirection", &VirtualSprite::GetDirection)
			.def("GetMovementSpeed", &VirtualSprite::GetMovementSpeed)
			.def("AddAction", &VirtualSprite::AddAction, adopt(_2))
			.def("AddDialogue", &VirtualSprite::AddDialogue, adopt(_2))
			.def("ClearDialogues", &VirtualSprite::ClearDialogues)
			.def("ShowDialogueIcon", &VirtualSprite::ShowDialogueIcon)
			.def("IsShowingDialogueIcon", &VirtualSprite::IsShowingDialogueIcon)
			.def_readwrite("current_action", &VirtualSprite::current_action)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapSprite, VirtualSprite>("MapSprite")
			.def(constructor<>())
			.def("SetName", &MapSprite::SetName)
			.def("SetCurrentAnimation", &MapSprite::SetCurrentAnimation)
			.def("GetCurrentAnimation", &MapSprite::GetCurrentAnimation)
			.def("LoadStandardAnimations", &MapSprite::LoadStandardAnimations)
			.def("LoadRunningAnimations", &MapSprite::LoadRunningAnimations)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<EnemySprite, MapSprite>("EnemySprite")
			.def(constructor<>())
			.def(constructor<std::string>())
			.def("Reset", &EnemySprite::Reset)
			.def("NewEnemyParty", &EnemySprite::NewEnemyParty)
			.def("AddEnemy", &EnemySprite::AddEnemy)
			.def("GetAggroRange", &EnemySprite::GetAggroRange)
			.def("GetTimeToChange", &EnemySprite::GetTimeToChange)
			.def("GetTimeToSpawn", &EnemySprite::GetTimeToSpawn)
			.def("GetBattleMusicTheme", &EnemySprite::GetBattleMusicTheme)
			.def("IsDead", &EnemySprite::IsDead)
			.def("IsSpawning", &EnemySprite::IsSpawning)
			.def("IsHostile", &EnemySprite::IsHostile)
			.def("SetZone", &EnemySprite::SetZone)
			.def("SetAggroRange", &EnemySprite::SetAggroRange)
			.def("SetTimeToChange", &EnemySprite::SetTimeToChange)
			.def("SetTimeToSpawn", &EnemySprite::SetTimeToSpawn)
			.def("SetBattleMusicTheme", &EnemySprite::SetBattleMusicTheme)
			.def("ChangeStateDead", &EnemySprite::ChangeStateDead)
			.def("ChangeStateSpawning", &EnemySprite::ChangeStateSpawning)
			.def("ChangeStateHostile", &EnemySprite::ChangeStateHostile)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<ZoneSection>("ZoneSection")
			.def(constructor<uint16, uint16, uint16, uint16>())
			.def_readwrite("top_row", &ZoneSection::top_row)
			.def_readwrite("bottom_row", &ZoneSection::bottom_row)
			.def_readwrite("left_col", &ZoneSection::left_col)
			.def_readwrite("right_col", &ZoneSection::right_col)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapZone>("MapZone")
			.def(constructor<>())
			.def("AddSection", &MapZone::AddSection, adopt(_2))
			.def("IsInsideZone", &MapZone::IsInsideZone)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<EnemyZone, MapZone>("EnemyZone")
			.def(constructor<uint32, bool>())
			.def("AddEnemy", &EnemyZone::AddEnemy, adopt(_2))
			.def("IsRestrained", &EnemyZone::IsRestrained)
			.def("SetRestrained", &EnemyZone::SetRestrained)
			.def("SetRegenTime", &EnemyZone::SetRegenTime)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<ContextZone, MapZone>("ContextZone")
			.def(constructor<MAP_CONTEXT, MAP_CONTEXT>())
			.def("AddSection", &ContextZone::AddSection, adopt(_2))
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapDialogue>("MapDialogue")
			.def(constructor<>())
			.def("AddText", &MapDialogue::AddText)
			.def("AddOption", &MapDialogue::AddOption)
			.def("SetMaxViews", &MapDialogue::SetMaxViews)
			.def("SetNextLine", &MapDialogue::SetNextLine)
			.def("EndDialogue", &MapDialogue::EndDialogue)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<SpriteAction>("SpriteAction")
			.def("Execute", &SpriteAction::Execute)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<ActionPathMove, SpriteAction>("ActionPathMove")
			.def(constructor<VirtualSprite*>())
			.def("SetDestination", &ActionPathMove::SetDestination)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<ActionRandomMove, SpriteAction>("ActionRandomMove")
			.def(constructor<VirtualSprite*>())
			.def_readwrite("total_movement_time", &ActionRandomMove::total_movement_time)
			.def_readwrite("total_direction_time", &ActionRandomMove::total_direction_time)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<ActionAnimate, SpriteAction>("ActionAnimate")
			.def(constructor<VirtualSprite*>())
			.def("AddFrame", &ActionAnimate::AddFrame)
			.def("SetLoopCount", &ActionAnimate::SetLoopCount)
	];
	} // End using map mode namespaces

	// ----- Battle Mode bindings
	{
	using namespace hoa_battle;
	using namespace hoa_battle::private_battle;
	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<BattleMode, hoa_mode_manager::GameMode>("BattleMode")
			.def(constructor<>())
			.def("AddEnemy", (void(BattleMode::*)(uint32)) &BattleMode::AddEnemy)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<BattleActor>("BattleActor")
			.def("GetPhysicalAttack", &BattleActor::GetPhysicalAttack)
			.def("GetPhysicalDefense", &BattleActor::GetPhysicalDefense)
			.def("GetCombatEvade", &BattleActor::GetCombatEvade)
			.def("GetCombatAgility", &BattleActor::GetCombatAgility)
			.def("TakeDamage", &BattleActor::TakeDamage)
			.def("GetActor", &BattleActor::GetActor)
			.def("AddHitPoints", &BattleActor::AddHitPoints)
			.def("AddStrength", &BattleActor::AddStrength)
			.def("AddNewEffect", &BattleActor::AddNewEffect)
	];
	} // End using battle mode namespaces

	// ----- Shop Mode bindings
	{
	using namespace hoa_shop;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_shop")
	[
		class_<ShopMode, hoa_mode_manager::GameMode>("ShopMode")
			.def(constructor<>())
			.def("AddObject", &ShopMode::AddObject)
	];

	} // End using shop mode namespaces
} // BindModesToLua

} // namespace hoa_lua
