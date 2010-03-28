///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    defs_modes.cpp
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
#include "shop.h"

#include "global_actors.h"
#include "global_effects.h"

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
			.def_readonly("object_supervisor", &MapMode::_object_supervisor)
			.def_readonly("event_supervisor", &MapMode::_event_supervisor)
			.def_readonly("dialogue_supervisor", &MapMode::_dialogue_supervisor)
			.def_readonly("map_event_group", &MapMode::_map_event_group)

			.def_readwrite("camera", &MapMode::_camera)
			.def_readwrite("ignore_input", &MapMode::_ignore_input)
			.def_readwrite("run_forever", &MapMode::_run_forever)
			.def_readwrite("run_disabled", &MapMode::_run_disabled)
			.def_readwrite("run_stamina", &MapMode::_run_stamina)

			.def("PlayMusic", &MapMode::PlayMusic)
			.def("AddGroundObject", &MapMode::AddGroundObject, adopt(_2))
			.def("AddPassObject", &MapMode::AddPassObject, adopt(_2))
			.def("AddSkyObject", &MapMode::AddSkyObject, adopt(_2))
			.def("AddZone", &MapMode::AddZone, adopt(_2))
			.def("SetCamera", &MapMode::SetCamera)
			.def("SetShowDialogueIcons", &MapMode::SetShowDialogueIcons)
			.def("IsShowDialogueIcons", &MapMode::IsShowDialogueIcons)
			.def("DrawMapLayers", &MapMode::_DrawMapLayers)

			// Namespace constants
			.enum_("constants") [
				// Map states
				value("STATE_EXPLORE", STATE_EXPLORE),
				value("STATE_SCENE", STATE_EXPLORE),
				value("STATE_DIALOGUE", STATE_EXPLORE),
				value("STATE_TREASURE", STATE_EXPLORE),
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
			.def("GetDirection", &VirtualSprite::GetDirection)
			.def("GetMovementSpeed", &VirtualSprite::GetMovementSpeed)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapSprite, VirtualSprite>("MapSprite")
			.def(constructor<>())
			.def("SetName", &MapSprite::SetName)
			.def("SetCurrentAnimation", &MapSprite::SetCurrentAnimation)
			.def("GetCurrentAnimation", &MapSprite::GetCurrentAnimation)
			.def("LoadFacePortrait", &MapSprite::LoadFacePortrait)
			.def("LoadStandardAnimations", &MapSprite::LoadStandardAnimations)
			.def("LoadRunningAnimations", &MapSprite::LoadRunningAnimations)
			.def("LoadAttackAnimations", &MapSprite::LoadAttackAnimations)
			.def("AddDialogueReference", &MapSprite::AddDialogueReference)
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
			.def("SetBattleBackground", &EnemySprite::SetBattleBackground)
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
		class_<DialogueSupervisor>("DialogueSupervisor")
			.def("AddDialogue", &DialogueSupervisor::AddDialogue, adopt(_2))
			.def("BeginDialogue", (void(DialogueSupervisor::*)(uint32))&DialogueSupervisor::BeginDialogue)
			.def("EndDialogue", &DialogueSupervisor::EndDialogue)
			.def("GetDialogue", &DialogueSupervisor::GetDialogue)
			.def("GetCurrentDialogue", &DialogueSupervisor::GetCurrentDialogue)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapDialogue>("MapDialogue")
			.def(constructor<uint32>())
			.def("AddText", &MapDialogue::AddText)
			.def("AddOption", &MapDialogue::AddOption)
			.def("SetMaxViews", &MapDialogue::SetMaxViews)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<EventSupervisor>("EventSupervisor")
			.def("RegisterEvent", &EventSupervisor::RegisterEvent, adopt(_2))
			.def("StartEvent", (void(EventSupervisor::*)(uint32))&EventSupervisor::StartEvent)
			.def("StartEvent", (void(EventSupervisor::*)(MapEvent*))&EventSupervisor::StartEvent)
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
			.def("AddEventLink", &MapEvent::AddEventLink)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<SoundEvent, MapEvent>("SoundEvent")
			.def(constructor<uint32, std::string>())
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapTransitionEvent, MapEvent>("MapTransitionEvent")
			.def(constructor<uint32, std::string>())
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<ScriptedEvent, MapEvent>("ScriptedEvent")
			.def(constructor<uint32, uint32, uint32>())
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<SpriteEvent, MapEvent>("SpriteEvent")
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<PathMoveSpriteEvent, SpriteEvent>("PathMoveSpriteEvent")
			.def(constructor<uint32, VirtualSprite*, uint32, uint32>())
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<RandomMoveSpriteEvent, SpriteEvent>("RandomMoveSpriteEvent")
			.def(constructor<uint32, VirtualSprite*, uint32, uint32>())
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<AnimateSpriteEvent, MapEvent>("AnimateSpriteEvent")
			.def(constructor<uint32, VirtualSprite*>())
			.def("AddFrame", &AnimateSpriteEvent::AddFrame)
			.def("SetLoopCount", &AnimateSpriteEvent::SetLoopCount)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<DialogueEvent, MapEvent>("DialogueEvent")
			.def(constructor<uint32, uint32>())
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<BattleEncounterEvent, MapEvent>("BattleEncounterEvent")
			.def(constructor<uint32, uint32>())
			.def("SetMusic", &BattleEncounterEvent::SetMusic)
			.def("SetBackground", &BattleEncounterEvent::SetBackground)
			.def("AddBattleEvent", &BattleEncounterEvent::AddBattleEvent)
			.def("AddEnemy", &BattleEncounterEvent::AddEnemy)
	];

	} // End using map mode namespaces

	// ----- Battle Mode bindings
	{
	using namespace hoa_battle;
	using namespace hoa_battle::private_battle;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		def("CalculateStandardEvasion", (bool(*)(BattleTarget*)) &CalculateStandardEvasion),
		def("CalculateStandardEvasion", (bool(*)(BattleTarget*, float)) &CalculateStandardEvasion),
		def("CalculateStandardEvasionMultiplier", (bool(*)(BattleTarget*, float)) &CalculateStandardEvasionMultiplier),
		def("CalculateStandardDamage", (uint32(*)(BattleActor* attacker, BattleTarget* target)) &CalculateStandardDamage),
		def("CalculateStandardDamage", (uint32(*)(BattleActor* attacker, BattleTarget* target, int32 add_phys, int32 add_meta)) &CalculateStandardDamage),
		def("CalculateStandardDamage", (uint32(*)(BattleActor* attacker, BattleTarget* target, float std_dev)) &CalculateStandardDamage),
		def("CalculateStandardDamage", (uint32(*)(BattleActor* attacker, BattleTarget* target, int32 add_phys, int32 add_meta, float std_dev)) &CalculateStandardDamage),
		def("CalculateStandardDamageMultiplier", (uint32(*)(BattleActor* attacker, BattleTarget* target, float mul_phys, float mul_meta)) &CalculateStandardDamageMultiplier),
		def("CalculateStandardDamageMultiplier", (uint32(*)(BattleActor* attacker, BattleTarget* target, float mul_phys, float mul_meta, float std_dev)) &CalculateStandardDamageMultiplier)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<BattleMode, hoa_mode_manager::GameMode>("BattleMode")
			.def(constructor<>())
			.def("AddEnemy", (void(BattleMode::*)(uint32)) &BattleMode::AddEnemy)
// 			.def("AddDialogue", &BattleMode::AddDialogue)
// 			.def("ShowDialogue", &BattleMode::ShowDialogue)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<BattleActor, hoa_global::GlobalActor>("BattleActor")
			.def("RegisterDamage", &BattleActor::RegisterDamage)
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
			.def("TotalMetaphysicalDefense", &BattleActor::TotalMetaphysicalDefense)
			.def("TotalEvadeRating", &BattleActor::TotalEvadeRating)
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
			.def("GetParty", &BattleTarget::GetParty)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<BattleStatusEffect, hoa_global::GlobalStatusEffect>("BattleStatusEffect")
			.def("GetAffectedActor", &BattleStatusEffect::GetAffectedActor)
			.def("GetTimer", &BattleStatusEffect::GetTimer)
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
