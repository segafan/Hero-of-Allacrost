///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    mode_bindings.cpp
*** \author  Daniel Steuernol, steu@allacrost.org
*** \brief   Lua bindings for game mode code
***
*** All bindings for the game mode code is contained within this file.
*** Therefore, everything that you see bound within this file will be made
*** available in Lua.
***
*** \note To most C++ programmers, the syntax of the binding code found in this
*** file may be very unfamiliar and obtuse. Refer to the Luabind documentation
*** as necessary to gain an understanding of this code style.
*** **************************************************************************/

#include "defs.h"

// Common code headers
#include "dialogue.h"
#include "global_actors.h"
#include "global_effects.h"

#include "battle.h"
#include "battle_actors.h"
#include "battle_command.h"
#include "battle_dialogue.h"
#include "battle_effects.h"
#include "battle_utils.h"
#include "map.h"
#include "map_dialogue.h"
#include "map_events.h"
#include "map_objects.h"
#include "map_sprites.h"
#include "map_treasure.h"
#include "map_utils.h"
#include "map_zones.h"
#include "menu.h"
#include "shop.h"
#include "test.h"

using namespace luabind;

namespace hoa_defs {

void BindModeCode() {
	// ----- Map Mode Bindings
	{
	using namespace hoa_map;
	using namespace hoa_map::private_map;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapMode, hoa_mode_manager::GameMode>("MapMode")
			.def(constructor<const std::string&>())
			.def_readonly("object_supervisor", &MapMode::_object_supervisor)
			.def_readonly("event_supervisor", &MapMode::_event_supervisor)
			.def_readonly("dialogue_supervisor", &MapMode::_dialogue_supervisor)
			.def_readonly("treasure_supervisor", &MapMode::_treasure_supervisor)
			.def_readonly("map_event_group", &MapMode::_map_event_group)

			.def_readonly("camera", &MapMode::_camera)
			.def_readonly("player_sprite", &MapMode::_player_sprite)
			.def_readonly("virtual_focus", &MapMode::_virtual_focus)
			.def_readwrite("unlimited_stamina", &MapMode::_unlimited_stamina)
			.def_readwrite("running_disabled", &MapMode::_running_disabled)
			.def_readwrite("run_stamina", &MapMode::_run_stamina)

			.def("PlayMusic", &MapMode::PlayMusic)
			.def("AddZone", &MapMode::AddZone, adopt(_2))
			.def("SetCamera", (void(MapMode::*)(private_map::VirtualSprite*))&MapMode::SetCamera)
			.def("SetCamera", (void(MapMode::*)(private_map::VirtualSprite*, uint32))&MapMode::SetCamera)
			.def("SetPlayerSprite", &MapMode::SetPlayerSprite)
			.def("MoveVirtualFocus", (void(MapMode::*)(uint16, uint16))&MapMode::MoveVirtualFocus)
			.def("MoveVirtualFocus", (void(MapMode::*)(uint16, uint16, uint32))&MapMode::MoveVirtualFocus)
			.def("IsCameraOnVirtualFocus", &MapMode::IsCameraOnVirtualFocus)
			.def("ClearLayerOrder", &MapMode::ClearLayerOrder)
			.def("AddTileLayerToOrder", &MapMode::AddTileLayerToOrder)
			.def("AddObjectLayerToOrder", &MapMode::AddObjectLayerToOrder)
			.def("IsDialogueIconsVisible", &MapMode::IsDialogueIconsVisible)
			.def("ShowDialogueIcons", &MapMode::ShowDialogueIcons)
			.def("IsStaminaBarHidden", &MapMode::IsStaminaBarVisible)
			.def("ShowStaminaBar", &MapMode::ShowStaminaBar)
			.def("DisableIntroductionVisuals", &MapMode::DisableIntroductionVisuals)
			.def("SetCurrentTrack", &MapMode::SetCurrentTrack)
			.def("CurrentState", &MapMode::CurrentState)
			.def("PushState", &MapMode::PushState)
			.def("PopState", &MapMode::PopState)
			.def("GetMapEventGroup", &MapMode::GetMapEventGroup)
			.def("DrawMapLayers", &MapMode::_DrawMapLayers)

			// Namespace constants
			.enum_("constants") [
				// Map states
				value("STATE_EXPLORE", STATE_EXPLORE),
				value("STATE_SCENE", STATE_SCENE),
				value("STATE_DIALOGUE", STATE_DIALOGUE),
				value("STATE_TREASURE", STATE_TREASURE),
				// Map contexts
				value("CONTEXT_NONE", MAP_CONTEXT_NONE),
				value("CONTEXT_01", MAP_CONTEXT_01),
				value("CONTEXT_02", MAP_CONTEXT_02),
				value("CONTEXT_03", MAP_CONTEXT_03),
				value("CONTEXT_04", MAP_CONTEXT_04),
				value("CONTEXT_05", MAP_CONTEXT_05),
				value("CONTEXT_06", MAP_CONTEXT_06),
				value("CONTEXT_07", MAP_CONTEXT_07),
				value("CONTEXT_08", MAP_CONTEXT_08),
				value("CONTEXT_09", MAP_CONTEXT_09),
				value("CONTEXT_10", MAP_CONTEXT_10),
				value("CONTEXT_11", MAP_CONTEXT_11),
				value("CONTEXT_12", MAP_CONTEXT_12),
				value("CONTEXT_13", MAP_CONTEXT_13),
				value("CONTEXT_14", MAP_CONTEXT_14),
				value("CONTEXT_15", MAP_CONTEXT_15),
				value("CONTEXT_16", MAP_CONTEXT_16),
				value("CONTEXT_17", MAP_CONTEXT_17),
				value("CONTEXT_18", MAP_CONTEXT_18),
				value("CONTEXT_19", MAP_CONTEXT_19),
				value("CONTEXT_20", MAP_CONTEXT_20),
				value("CONTEXT_21", MAP_CONTEXT_21),
				value("CONTEXT_22", MAP_CONTEXT_22),
				value("CONTEXT_23", MAP_CONTEXT_23),
				value("CONTEXT_24", MAP_CONTEXT_24),
				value("CONTEXT_25", MAP_CONTEXT_25),
				value("CONTEXT_26", MAP_CONTEXT_26),
				value("CONTEXT_27", MAP_CONTEXT_27),
				value("CONTEXT_28", MAP_CONTEXT_28),
				value("CONTEXT_29", MAP_CONTEXT_29),
				value("CONTEXT_30", MAP_CONTEXT_30),
				value("CONTEXT_31", MAP_CONTEXT_31),
				value("CONTEXT_32", MAP_CONTEXT_32),
				value("CONTEXT_ALL", MAP_CONTEXT_ALL),
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
				value("ANIM_ATTACKING_EAST", ANIM_ATTACKING_EAST),
				// Sprite speeds
				value("VERY_SLOW_SPEED", static_cast<uint32>(VERY_SLOW_SPEED)),
				value("SLOW_SPEED", static_cast<uint32>(SLOW_SPEED)),
				value("NORMAL_SPEED", static_cast<uint32>(NORMAL_SPEED)),
				value("FAST_SPEED", static_cast<uint32>(FAST_SPEED)),
				value("VERY_FAST_SPEED", static_cast<uint32>(VERY_FAST_SPEED))
			]
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<ObjectSupervisor>("ObjectSupervisor")
			.def("GenerateObjectID", &ObjectSupervisor::GenerateObjectID)
			.def("GetNumberObjects", &ObjectSupervisor::GetNumberObjects)
			.def("GetObjectByIndex", &ObjectSupervisor::GetObjectByIndex)
			.def("GetObject", &ObjectSupervisor::GetObject)
			.def("AddObjectLayer", &ObjectSupervisor::AddObjectLayer)
			.def("AddObject", (void(private_map::ObjectSupervisor::*)(private_map::MapObject*))&ObjectSupervisor::AddObject, adopt(_2))
			.def("AddObject", (void(private_map::ObjectSupervisor::*)(private_map::MapObject*, uint32))&ObjectSupervisor::AddObject, adopt(_2))
			.def("MoveObjectToLayer", &ObjectSupervisor::MoveObjectToLayer)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapObject>("MapObject")
			.def("SetObjectID", &MapObject::SetObjectID)
			.def("SetContext", &MapObject::SetContext)
			.def("SetPosition", &MapObject::SetPosition)
			.def("SetXPosition", &MapObject::SetXPosition)
			.def("SetYPosition", &MapObject::SetYPosition)
			.def("SetImgHalfWidth", &MapObject::SetImgHalfWidth)
			.def("SetImgHeight", &MapObject::SetImgHeight)
			.def("SetCollHalfWidth", &MapObject::SetCollHalfWidth)
			.def("SetCollHeight", &MapObject::SetCollHeight)
			.def("SetUpdatable", &MapObject::SetUpdatable)
			.def("SetVisible", &MapObject::SetVisible)
			.def("SetNoCollision", &MapObject::SetNoCollision)
			.def("GetObjectID", &MapObject::GetObjectID)
			.def("GetObjectLayerID", &MapObject::GetObjectLayerID)
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
			// TEMP: because GetXPosition and GetYPostiion seem to give a runtime error in Lua
			.def_readonly("x_position", &MapObject::x_position)
			.def_readonly("y_position", &MapObject::y_position)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<PhysicalObject, MapObject>("PhysicalObject")
			.def(constructor<>())
			.def("AddAnimation", (void(PhysicalObject::*)(std::string))&PhysicalObject::AddAnimation)
			.def("SetCurrentAnimation", &PhysicalObject::SetCurrentAnimation)
			.def("SetAnimationProgress", &PhysicalObject::SetAnimationProgress)
			.def("GetCurrentAnimation", &PhysicalObject::GetCurrentAnimation)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<TreasureObject, PhysicalObject>("TreasureObject")
			.def(constructor<std::string, uint8, uint8, uint8>())
			.def("GetTreasure", &TreasureObject::GetTreasure)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<VirtualSprite, MapObject>("VirtualSprite")
			.def(constructor<>())
			.def("SetMoving", &VirtualSprite::SetMoving)
			.def("SetDirection", &VirtualSprite::SetDirection)
			.def("SetMovementSpeed", &VirtualSprite::SetMovementSpeed)
			.def("IsMoving", &VirtualSprite::IsMoving)
			.def("GetDirection", &VirtualSprite::GetDirection)
			.def("GetMovementSpeed", &VirtualSprite::GetMovementSpeed)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapSprite, VirtualSprite>("MapSprite")
			.def(constructor<>())
			.def("SetName", &MapSprite::SetName)
			.def("SetCurrentAnimation", &MapSprite::SetCurrentAnimation)
			.def("GetCurrentAnimation", (uint8(MapSprite::*)()const)&MapSprite::GetCurrentAnimation)
			.def("GetCurrentAnimation", (hoa_video::AnimatedImage&(MapSprite::*)())&MapSprite::GetCurrentAnimation)
			.def("GetAnimation", &MapSprite::GetAnimation)
			.def("LoadFacePortrait", &MapSprite::LoadFacePortrait)
			.def("LoadStandardAnimations", &MapSprite::LoadStandardAnimations)
			.def("LoadRunningAnimations", &MapSprite::LoadRunningAnimations)
			.def("LoadAttackAnimations", &MapSprite::LoadAttackAnimations)
			.def("AddDialogueReference", &MapSprite::AddDialogueReference)
			.def("ClearDialogueReferences", &MapSprite::ClearDialogueReferences)
			.def("RemoveDialogueReference", &MapSprite::RemoveDialogueReference)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<EnemySprite, MapSprite>("EnemySprite")
			.def(constructor<>())
			.def("Reset", &EnemySprite::Reset)
			.def("NewEnemyParty", &EnemySprite::NewEnemyParty)
			.def("AddEnemy", &EnemySprite::AddEnemy)
			.def("GetPursuitRange", &EnemySprite::GetPursuitRange)
			.def("GetDirectionChangeTime", &EnemySprite::GetDirectionChangeTime)
			.def("GetFadeTime", &EnemySprite::GetFadeTime)
			.def("IsStateSpawn", &EnemySprite::IsStateSpawn)
			.def("IsStateActive", &EnemySprite::IsStateActive)
			.def("IsStateActiveZoned", &EnemySprite::IsStateActiveZoned)
			.def("IsStateDissipate", &EnemySprite::IsStateDissipate)
			.def("IsStateInactive", &EnemySprite::IsStateInactive)
			.def("SetZone", &EnemySprite::SetZone)
			.def("SetPursuitRange", &EnemySprite::SetPursuitRange)
			.def("SetDirectionChangeTime", &EnemySprite::SetDirectionChangeTime)
			.def("SetFadeTime", &EnemySprite::SetFadeTime)
			.def("SetBattleMusicFile", &EnemySprite::SetBattleMusicFile)
			.def("SetBattleBackgroundFile", &EnemySprite::SetBattleBackgroundFile)
			.def("SetBattleScriptFile", &EnemySprite::SetBattleScriptFile)
			.def("ChangeStateSpawn", &EnemySprite::ChangeStateSpawn)
			.def("ChangeStateActive", &EnemySprite::ChangeStateActive)
			.def("ChangeStateActiveZoned", &EnemySprite::ChangeStateActiveZoned)
			.def("ChangeStateDissipate", &EnemySprite::ChangeStateDissipate)
			.def("ChangeStateInactive", &EnemySprite::ChangeStateInactive)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapZone>("MapZone")
			.def(constructor<>())
			.def(constructor<uint16, uint16, uint16, uint16>())
			.def(constructor<uint16, uint16, uint16, uint16, MAP_CONTEXT>())
			.def("AddSection", &MapZone::AddSection)
			.def("IsInsideZone", &MapZone::IsInsideZone)
			.def("GetActiveContexts", &MapZone::GetActiveContexts)
			.def("SetActiveContexts", &MapZone::SetActiveContexts)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<CameraZone, MapZone>("CameraZone")
			.def(constructor<>())
			.def(constructor<uint16, uint16, uint16, uint16>())
			.def(constructor<uint16, uint16, uint16, uint16, MAP_CONTEXT>())
			.def("IsCameraInside", &CameraZone::IsCameraInside)
			.def("IsCameraEntering", &CameraZone::IsCameraEntering)
			.def("IsCameraExiting", &CameraZone::IsCameraExiting)
			.def("IsPlayerSpriteInside", &CameraZone::IsPlayerSpriteInside)
			.def("IsPlayerSpriteEntering", &CameraZone::IsPlayerSpriteEntering)
			.def("IsPlayerSpriteExiting", &CameraZone::IsPlayerSpriteExiting)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<ResidentZone, MapZone>("ResidentZone")
			.def(constructor<>())
			.def(constructor<uint16, uint16, uint16, uint16>())
			.def(constructor<uint16, uint16, uint16, uint16, MAP_CONTEXT>())
			.def("IsResidentEntering", &ResidentZone::IsResidentEntering)
			.def("IsResidentExiting", &ResidentZone::IsResidentExiting)
			.def("IsSpriteResident", (bool(ResidentZone::*)(uint32)const)&ResidentZone::IsSpriteResident)
			.def("IsSpriteResident", (bool(ResidentZone::*)(VirtualSprite*)const)&ResidentZone::IsSpriteResident)
			.def("IsCameraResident", &ResidentZone::IsCameraResident)
			.def("IsSpriteEntering", (bool(ResidentZone::*)(uint32)const)&ResidentZone::IsSpriteEntering)
			.def("IsSpriteEntering", (bool(ResidentZone::*)(VirtualSprite*)const)&ResidentZone::IsSpriteEntering)
			.def("IsCameraEntering", &ResidentZone::IsCameraEntering)
			.def("IsSpriteExiting", (bool(ResidentZone::*)(uint32)const)&ResidentZone::IsSpriteExiting)
			.def("IsSpriteExiting", (bool(ResidentZone::*)(VirtualSprite*)const)&ResidentZone::IsSpriteExiting)
			.def("IsCameraExiting", &ResidentZone::IsCameraExiting)
			.def("GetResident", &ResidentZone::GetResident)
			.def("GetEnteringResident", &ResidentZone::GetEnteringResident)
			.def("GetExitingResident", &ResidentZone::GetExitingResident)
			.def("GetNumberResidents", &ResidentZone::GetNumberResidents)
			.def("GetNumberEnteringResidents", &ResidentZone::GetNumberEnteringResidents)
			.def("GetNumberExitingResidents", &ResidentZone::GetNumberExitingResidents)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<EnemyZone, MapZone>("EnemyZone")
			.def(constructor<>())
			.def(constructor<uint16, uint16, uint16, uint16>())
			.def("AddEnemy", &EnemyZone::AddEnemy, adopt(_2))
			.def("AddSpawnSection", &EnemyZone::AddSpawnSection)
			.def("ForceSpawnAllEnemies", &EnemyZone::ForceSpawnAllEnemies)
			.def("IsRoamingRestrained", &EnemyZone::IsRoamingRestrained)
			.def("GetSpawnTime", &EnemyZone::GetSpawnTime)
			.def("SetRoamingRestrained", &EnemyZone::SetRoamingRestrained)
			.def("SetSpawnTime", &EnemyZone::SetSpawnTime)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<ContextZone, MapZone>("ContextZone")
			.def(constructor<MAP_CONTEXT, MAP_CONTEXT>())
			.def("AddSection", (void(ContextZone::*)(uint16, uint16, uint16, uint16, bool))&ContextZone::AddSection)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<DialogueSupervisor>("DialogueSupervisor")
			.def("BeginDialogue", &DialogueSupervisor::BeginDialogue)
			.def("EndDialogue", &DialogueSupervisor::EndDialogue)
			.def("GetDialogue", &DialogueSupervisor::GetDialogue)
			.def("GetCurrentDialogue", &DialogueSupervisor::GetCurrentDialogue)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapDialogue, hoa_common::CommonDialogue>("MapDialogue")
			.scope
			[
				def("Create", &MapDialogue::Create)
			]
			.def("AddLine", (void(MapDialogue::*)(std::string, uint32))&MapDialogue::AddLine)
			.def("AddLine", (void(MapDialogue::*)(std::string, uint32, int32))&MapDialogue::AddLine)
			.def("AddLine", (void(MapDialogue::*)(std::string))&MapDialogue::AddLine)
			.def("AddLineTiming", (void(MapDialogue::*)(uint32))&MapDialogue::AddLineTiming)
			.def("AddLineTiming", (void(MapDialogue::*)(uint32, uint32))&MapDialogue::AddLineTiming)
			.def("AddLineEventAtStart", (void(MapDialogue::*)(uint32))&MapDialogue::AddLineEventAtStart)
			.def("AddLineEventAtStart", (void(MapDialogue::*)(uint32, uint32))&MapDialogue::AddLineEventAtStart)
			.def("AddLineEventAtEnd", (void(MapDialogue::*)(uint32))&MapDialogue::AddLineEventAtEnd)
			.def("AddLineEventAtEnd", (void(MapDialogue::*)(uint32, uint32))&MapDialogue::AddLineEventAtEnd)
			.def("AddOption", (void(MapDialogue::*)(std::string))&MapDialogue::AddOption)
			.def("AddOption", (void(MapDialogue::*)(std::string, int32))&MapDialogue::AddOption)
			.def("AddOptionEvent", (void(MapDialogue::*)(uint32))&MapDialogue::AddOptionEvent)
			.def("AddOptionEvent", (void(MapDialogue::*)(uint32, uint32))&MapDialogue::AddOptionEvent)
			.def("Validate", &MapDialogue::Validate)
			.def("SetInputBlocked", &MapDialogue::SetInputBlocked)
			.def("SetRestoreState", &MapDialogue::SetRestoreState)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<EventSupervisor>("EventSupervisor")
			.def("RegisterEvent", &EventSupervisor::RegisterEvent, adopt(_2))
			.def("StartEvent", (void(EventSupervisor::*)(uint32))&EventSupervisor::StartEvent)
			.def("StartEvent", (void(EventSupervisor::*)(MapEvent*))&EventSupervisor::StartEvent)
			.def("StartEvent", (void(EventSupervisor::*)(uint32, uint32))&EventSupervisor::StartEvent)
			.def("StartEvent", (void(EventSupervisor::*)(MapEvent*, uint32))&EventSupervisor::StartEvent)
			.def("TerminateEvent", &EventSupervisor::TerminateEvent)
			.def("IsEventActive", &EventSupervisor::IsEventActive)
			.def("HasActiveEvent", &EventSupervisor::HasActiveEvent)
			.def("HasLaunchEvent", &EventSupervisor::HasLaunchEvent)
			.def("GetEvent", &EventSupervisor::GetEvent)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapEvent>("MapEvent")
			.def("GetEventID", &MapEvent::GetEventID)
			.def("AddEventLinkAtStart", (void(MapEvent::*)(uint32))&MapEvent::AddEventLinkAtStart)
			.def("AddEventLinkAtStart", (void(MapEvent::*)(uint32, uint32))&MapEvent::AddEventLinkAtStart)
			.def("AddEventLinkAtEnd", (void(MapEvent::*)(uint32))&MapEvent::AddEventLinkAtEnd)
			.def("AddEventLinkAtEnd", (void(MapEvent::*)(uint32, uint32))&MapEvent::AddEventLinkAtEnd)
	];


	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<DialogueEvent, MapEvent>("DialogueEvent")
			.scope
			[
				def("Create", &DialogueEvent::Create)
			]
			.def("SetStopCameraMovement", &DialogueEvent::SetStopCameraMovement)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<SoundEvent, MapEvent>("SoundEvent")
			.scope
			[
				def("Create", &SoundEvent::Create)
			]
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapTransitionEvent, MapEvent>("MapTransitionEvent")
			.scope
			[
				def("Create", &MapTransitionEvent::Create)
			]
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<BattleEncounterEvent, MapEvent>("BattleEncounterEvent")
			.scope
			[
				def("Create", &BattleEncounterEvent::Create)
			]
			.def("SetMusic", &BattleEncounterEvent::SetMusic)
			.def("SetBackground", &BattleEncounterEvent::SetBackground)
			.def("AddEnemy", &BattleEncounterEvent::AddEnemy)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<CustomEvent, MapEvent>("CustomEvent")
			.scope
			[
				def("Create", &CustomEvent::Create)
			]
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<SpriteEvent, MapEvent>("SpriteEvent")
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<ChangeDirectionSpriteEvent, SpriteEvent>("ChangeDirectionSpriteEvent")
			.scope
			[
				def("Create", (ChangeDirectionSpriteEvent*(*)(uint32, VirtualSprite*, uint16))&ChangeDirectionSpriteEvent::Create),
				def("Create", (ChangeDirectionSpriteEvent*(*)(uint32, uint16, uint16))&ChangeDirectionSpriteEvent::Create)
			]
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<AnimateSpriteEvent, MapEvent>("AnimateSpriteEvent")
			.scope
			[
				def("Create", (AnimateSpriteEvent*(*)(uint32, VirtualSprite*))&AnimateSpriteEvent::Create),
				def("Create", (AnimateSpriteEvent*(*)(uint32, uint16))&AnimateSpriteEvent::Create)
			]
			.def("AddFrame", &AnimateSpriteEvent::AddFrame)
			.def("SetLoopCount", &AnimateSpriteEvent::SetLoopCount)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<RandomMoveSpriteEvent, SpriteEvent>("RandomMoveSpriteEvent")
			.scope
			[
				def("Create", (RandomMoveSpriteEvent*(*)(uint32, VirtualSprite*, uint32, uint32))&RandomMoveSpriteEvent::Create),
				def("Create", (RandomMoveSpriteEvent*(*)(uint32, uint16, uint32, uint32))&RandomMoveSpriteEvent::Create)
			]
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<PathMoveSpriteEvent, SpriteEvent>("PathMoveSpriteEvent")
			.scope
			[
				def("Create", (PathMoveSpriteEvent*(*)(uint32, VirtualSprite*, int16, int16))&PathMoveSpriteEvent::Create),
				def("Create", (PathMoveSpriteEvent*(*)(uint32, uint16, int16, int16))&PathMoveSpriteEvent::Create)
			]
			.def("SetRelativeDestination", &PathMoveSpriteEvent::SetRelativeDestination)
			.def("SetDestination", &PathMoveSpriteEvent::SetDestination)
			.def("SetFinalDirection", &PathMoveSpriteEvent::SetFinalDirection)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<CustomSpriteEvent, SpriteEvent>("CustomSpriteEvent")
			.scope
			[
				def("Create", (CustomSpriteEvent*(*)(uint32, VirtualSprite*, std::string, std::string))&CustomSpriteEvent::Create),
				def("Create", (CustomSpriteEvent*(*)(uint32, uint16, std::string, std::string))&CustomSpriteEvent::Create)
			]
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapTreasure>("MapTreasure")
			.def(constructor<>())
			.def("AddDrunes", &MapTreasure::AddDrunes)
			.def("AddObject", &MapTreasure::AddObject)
			.def("IsTaken", &MapTreasure::IsTaken)
			.def("SetTaken", &MapTreasure::SetTaken)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<TreasureSupervisor>("TreasureSupervisor")
			.def("Initialize", (void(TreasureSupervisor::*)(TreasureObject*))&TreasureSupervisor::Initialize)
			.def("Initialize", (void(TreasureSupervisor::*)(MapTreasure*))&TreasureSupervisor::Initialize)
	];

	} // End using map mode namespaces



	// ----- Battle Mode bindings
	{
	using namespace hoa_battle;
	using namespace hoa_battle::private_battle;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		def("CalculateStandardEvasion", (bool(*)(BattleTarget*)) &CalculateStandardEvasion),
		def("CalculateStandardEvasionAdder", (bool(*)(BattleTarget*, float)) &CalculateStandardEvasion),
		def("CalculateStandardEvasionMultiplier", (bool(*)(BattleTarget*, float)) &CalculateStandardEvasionMultiplier),
		def("CalculatePhysicalDamage", (uint32(*)(BattleActor*, BattleTarget*)) &CalculatePhysicalDamage),
		def("CalculatePhysicalDamage", (uint32(*)(BattleActor*, BattleTarget*, float)) &CalculatePhysicalDamage),
		def("CalculatePhysicalDamageAdder", (uint32(*)(BattleActor*, BattleTarget*, int32)) &CalculatePhysicalDamageAdder),
		def("CalculatePhysicalDamageAdder", (uint32(*)(BattleActor*, BattleTarget*, int32, float)) &CalculatePhysicalDamageAdder),
		def("CalculatePhysicalDamageMultiplier", (uint32(*)(BattleActor*, BattleTarget*, float)) &CalculatePhysicalDamageMultiplier),
		def("CalculatePhysicalDamageMultiplier", (uint32(*)(BattleActor*, BattleTarget*, float, float)) &CalculatePhysicalDamageMultiplier),
		def("CalculateEtherealDamage", (uint32(*)(BattleActor*, BattleTarget*)) &CalculateEtherealDamage),
		def("CalculateEtherealDamage", (uint32(*)(BattleActor*, BattleTarget*, float)) &CalculateEtherealDamage),
		def("CalculateEtherealDamageAdder", (uint32(*)(BattleActor*, BattleTarget*, int32)) &CalculateEtherealDamageAdder),
		def("CalculateEtherealDamageAdder", (uint32(*)(BattleActor*, BattleTarget*, int32, float)) &CalculateEtherealDamageAdder),
		def("CalculateEtherealDamageMultiplier", (uint32(*)(BattleActor*, BattleTarget*, float)) &CalculateEtherealDamageMultiplier),
		def("CalculateEtherealDamageMultiplier", (uint32(*)(BattleActor*, BattleTarget*, float, float)) &CalculateEtherealDamageMultiplier)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<BattleMode, hoa_mode_manager::GameMode>("BattleMode")
			.def(constructor<>())
			.def("AddEnemy", (void(BattleMode::*)(uint32)) &BattleMode::AddEnemy)
			.def("LoadBattleScript", &BattleMode::LoadBattleScript)
			.def("RestartBattle", &BattleMode::RestartBattle)
			.def("FreezeTimers", &BattleMode::FreezeTimers)
			.def("UnFreezeTimers", &BattleMode::UnFreezeTimers)
			.def("GetState", &BattleMode::GetState)
			.def("ChangeState", &BattleMode::ChangeState)
			.def("OpenCommandMenu", &BattleMode::OpenCommandMenu)
			.def("IsBattleFinished", &BattleMode::IsBattleFinished)
			.def("SetPlayFinishMusic", &BattleMode::SetPlayFinishMusic)
			.def("GetNumberOfCharacters", &BattleMode::GetNumberOfCharacters)
			.def("GetNumberOfEnemies", &BattleMode::GetNumberOfEnemies)
			.def("GetMedia", &BattleMode::GetMedia)
			.def("GetDialogueSupervisor", &BattleMode::GetDialogueSupervisor)
			.def("GetCommandSupervisor", &BattleMode::GetCommandSupervisor)

			// Namespace constants
			.enum_("constants") [
				// Battle states
				value("BATTLE_STATE_INITIAL", BATTLE_STATE_INITIAL),
				value("BATTLE_STATE_NORMAL", BATTLE_STATE_NORMAL),
				value("BATTLE_STATE_COMMAND", BATTLE_STATE_COMMAND),
				value("BATTLE_STATE_EVENT", BATTLE_STATE_EVENT),
				value("BATTLE_STATE_VICTORY", BATTLE_STATE_VICTORY),
				value("BATTLE_STATE_DEFEAT", BATTLE_STATE_DEFEAT),
				value("BATTLE_STATE_EXITING", BATTLE_STATE_EXITING)
			]
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<BattleMedia>("BattleMedia")
			.def("SetBackgroundImage", &BattleMedia::SetBackgroundImage)
			.def("SetBattleMusic", &BattleMedia::SetBattleMusic)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<BattleActor, hoa_global::GlobalActor>("BattleActor")
			.def("ChangeSpriteAnimation", &BattleActor::ChangeSpriteAnimation)
			.def("RegisterDamage", (void(BattleActor::*)(uint32)) &BattleActor::RegisterDamage)
			.def("RegisterDamage", (void(BattleActor::*)(uint32, BattleTarget*)) &BattleActor::RegisterDamage)
			.def("RegisterHealing", &BattleActor::RegisterHealing)
			.def("RegisterMiss", &BattleActor::RegisterMiss)
			.def("RegisterStatusChange", &BattleActor::RegisterStatusChange)
			.def("ResetHitPoints", &BattleActor::ResetHitPoints)
			.def("ResetMaxHitPoints", &BattleActor::ResetMaxHitPoints)
			.def("ResetSkillPoints", &BattleActor::ResetSkillPoints)
			.def("ResetMaxSkillPoints", &BattleActor::ResetMaxSkillPoints)
			.def("ResetStrength", &BattleActor::ResetStrength)
			.def("ResetVigor", &BattleActor::ResetVigor)
			.def("ResetFortitude", &BattleActor::ResetFortitude)
			.def("ResetProtection", &BattleActor::ResetProtection)
			.def("ResetAgility", &BattleActor::ResetAgility)
			.def("ResetEvade", &BattleActor::ResetEvade)
			.def("TotalPhysicalDefense", &BattleActor::TotalPhysicalDefense)
			.def("TotalEtherealDefense", &BattleActor::TotalEtherealDefense)
			.def("TotalEvadeRating", &BattleActor::TotalEvadeRating)
			.def("SetStatePaused", &BattleActor::SetStatePaused)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<BattleCharacter, BattleActor>("BattleCharacter")
			.def("ChangeSpriteAnimation", &BattleCharacter::ChangeSpriteAnimation)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<BattleEnemy, BattleActor>("BattleEnemy")
			.def("ChangeSpriteAnimation", &BattleEnemy::ChangeSpriteAnimation)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<CommandSupervisor>("CommandSupervisor")
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<BattleDialogue, hoa_common::CommonDialogue>("BattleDialogue")
			.def(constructor<uint32>())
			.def("AddLine", (void(BattleDialogue::*)(std::string, uint32))&BattleDialogue::AddLine)
			.def("AddLine", (void(BattleDialogue::*)(std::string, uint32, int32))&BattleDialogue::AddLine)
			.def("AddLineTimed", (void(BattleDialogue::*)(std::string, uint32, uint32))&BattleDialogue::AddLineTimed)
			.def("AddLineTimed", (void(BattleDialogue::*)(std::string, uint32, int32, uint32))&BattleDialogue::AddLineTimed)
			.def("AddOption", (void(BattleDialogue::*)(std::string))&BattleDialogue::AddOption)
			.def("AddOption", (void(BattleDialogue::*)(std::string, int32))&BattleDialogue::AddOption)
			.def("Validate", &BattleDialogue::Validate)
			.def("SetHaltBattleAction", &BattleDialogue::SetHaltBattleAction)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<DialogueSupervisor>("DialogueSupervisor")
			.def("AddDialogue", &DialogueSupervisor::AddDialogue, adopt(_2))
			.def("AddCharacterSpeaker", &DialogueSupervisor::AddCharacterSpeaker)
			.def("AddEnemySpeaker", &DialogueSupervisor::AddEnemySpeaker)
			.def("AddCustomSpeaker", &DialogueSupervisor::AddCustomSpeaker)
			.def("ChangeSpeakerName", &DialogueSupervisor::ChangeSpeakerName)
			.def("ChangeSpeakerPortrait", &DialogueSupervisor::ChangeSpeakerPortrait)
			.def("BeginDialogue", &DialogueSupervisor::BeginDialogue)
			.def("EndDialogue", &DialogueSupervisor::EndDialogue)
			.def("ForceNextLine", &DialogueSupervisor::ForceNextLine)
			.def("IsDialogueActive", &DialogueSupervisor::IsDialogueActive)
			.def("GetCurrentDialogue", &DialogueSupervisor::GetCurrentDialogue)
			.def("GetLineCounter", &DialogueSupervisor::GetLineCounter)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<BattleTarget>("BattleTarget")
			.def("SetPointTarget", &BattleTarget::SetPointTarget)
			.def("SetActorTarget", &BattleTarget::SetActorTarget)
			.def("SetPartyTarget", &BattleTarget::SetPartyTarget)
			.def("IsValid", &BattleTarget::IsValid)
			.def("SelectNextPoint", &BattleTarget::SelectNextPoint)
			.def("SelectNextActor", &BattleTarget::SelectNextActor)
			.def("GetType", &BattleTarget::GetType)
			.def("GetPoint", &BattleTarget::GetPoint)
			.def("GetActor", &BattleTarget::GetActor)
			.def("GetPartyActor", &BattleTarget::GetPartyActor)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<BattleEffect>("BattleEffect")
			.def("GetEffectActor", &BattleEffect::GetEffectActor)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<StatusEffect, BattleEffect>("StatusEffect")
			.def("GetDurationTimer", &StatusEffect::GetDurationTimer)
			.def("GetIntensity", &StatusEffect::GetIntensity)
			.def("IncrementIntensity", &StatusEffect::IncrementIntensity)
			.def("DecrementIntensity", &StatusEffect::DecrementIntensity)
			.def("SetIntensity", &StatusEffect::SetIntensity)
			.def("IsIntensityChanged", &StatusEffect::IsIntensityChanged)
	];

	} // End using battle mode namespaces

	// ----- Menu Mode bindings
	{
	using namespace hoa_menu;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_menu")
	[
		class_<MenuMode, hoa_mode_manager::GameMode>("MenuMode")
			.def(constructor<>())
	];

	} // End using menu mode namespaces

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

	// ----- Test Mode bindings
	{
	using namespace hoa_test;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_test")
	[
		class_<TestMode, hoa_mode_manager::GameMode>("TestMode")
			.def("SetImmediateTestID", &TestMode::SetImmediateTestID)
	];

	} // End using test mode namespaces

} // void BindModeCode()

} // namespace hoa_defs
