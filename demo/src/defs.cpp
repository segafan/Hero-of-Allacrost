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

#include "audio.h"
#include "input.h"
#include "mode_manager.h"
#include "script.h"
#include "system.h"
#include "video.h"

#include "global.h"
#include "global_objects.h"
#include "global_actors.h"
#include "global_skills.h"

#include "battle.h"
#include "battle_actors.h"
#include "map.h"
#include "map_actions.h"
#include "map_dialogue.h"
#include "map_objects.h"
#include "map_sprites.h"
#include "map_zones.h"
#include "shop.h"

using namespace luabind;

namespace hoa_defs {

void BindEngineToLua() {
	// ---------- (1) Bind Engine Components



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




	// ---------- (2) Bind Global Components
	{
	using namespace hoa_global;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GameGlobal>("GameGlobal")
			.def("AddCharacter", (void(GameGlobal::*)(uint32)) &GameGlobal::AddCharacter)
			.def("GetCharacter", &GameGlobal::GetCharacter)
			.def("GetDrunes", &GameGlobal::GetDrunes)
			.def("SetDrunes", &GameGlobal::SetDrunes)
			.def("AddDrunes", &GameGlobal::AddDrunes)
			.def("SubtractDrunes", &GameGlobal::SubtractDrunes)
			.def("AddToInventory", (void (GameGlobal::*)(uint32, uint32)) &GameGlobal::AddToInventory)
			.def("RemoveFromInventory", (void (GameGlobal::*)(uint32)) &GameGlobal::RemoveFromInventory)
			.def("IncrementObjectCount", &GameGlobal::IncrementObjectCount)
			.def("DecrementObjectCount", &GameGlobal::DecrementObjectCount)

			// Namespace constants
			.enum_("constants") [
				// Character type constants
				value("GLOBAL_CHARACTER_INVALID", GLOBAL_CHARACTER_INVALID),
				value("GLOBAL_CHARACTER_CLAUDIUS", GLOBAL_CHARACTER_CLAUDIUS),
				value("GLOBAL_CHARACTER_LAILA", GLOBAL_CHARACTER_LAILA),
				value("GLOBAL_CHARACTER_ALL", GLOBAL_CHARACTER_ALL),
				// Object type constants
				value("GLOBAL_OBJECT_INVALID", GLOBAL_OBJECT_INVALID),
				value("GLOBAL_OBJECT_ITEM", GLOBAL_OBJECT_ITEM),
				value("GLOBAL_OBJECT_WEAPON", GLOBAL_OBJECT_WEAPON),
				value("GLOBAL_OBJECT_HEAD_ARMOR", GLOBAL_OBJECT_HEAD_ARMOR),
				value("GLOBAL_OBJECT_TORSO_ARMOR", GLOBAL_OBJECT_TORSO_ARMOR),
				value("GLOBAL_OBJECT_ARM_ARMOR", GLOBAL_OBJECT_ARM_ARMOR),
				value("GLOBAL_OBJECT_LEG_ARMOR", GLOBAL_OBJECT_LEG_ARMOR),
				value("GLOBAL_OBJECT_SHARD", GLOBAL_OBJECT_SHARD),
				value("GLOBAL_OBJECT_KEY_ITEM", GLOBAL_OBJECT_KEY_ITEM),
				// Item usage constants
				value("GLOBAL_USE_INVALID", GLOBAL_USE_INVALID),
				value("GLOBAL_USE_MENU", GLOBAL_USE_MENU),
				value("GLOBAL_USE_BATTLE", GLOBAL_USE_BATTLE),
				value("GLOBAL_USE_ALL", GLOBAL_USE_ALL),
				// Item and skill alignment constants
				value("GLOBAL_POSITION_HEAD", GLOBAL_POSITION_HEAD),
				value("GLOBAL_POSITION_TORSO", GLOBAL_POSITION_TORSO),
				value("GLOBAL_POSITION_ARMS", GLOBAL_POSITION_ARMS),
				value("GLOBAL_POSITION_LEGS", GLOBAL_POSITION_LEGS),
				// Global skill types
				value("GLOBAL_SKILL_INVALID", GLOBAL_SKILL_INVALID),
				value("GLOBAL_SKILL_ATTACK", GLOBAL_SKILL_ATTACK),
				value("GLOBAL_SKILL_DEFEND", GLOBAL_SKILL_DEFEND),
				value("GLOBAL_SKILL_SUPPORT", GLOBAL_SKILL_SUPPORT),
				// Elemental type constants
				value("GLOBAL_ELEMENTAL_FIRE", GLOBAL_ELEMENTAL_FIRE),
				value("GLOBAL_ELEMENTAL_WATER", GLOBAL_ELEMENTAL_WATER),
				value("GLOBAL_ELEMENTAL_VOLT", GLOBAL_ELEMENTAL_VOLT),
				value("GLOBAL_ELEMENTAL_EARTH", GLOBAL_ELEMENTAL_EARTH),
				value("GLOBAL_ELEMENTAL_SLICING", GLOBAL_ELEMENTAL_SLICING),
				value("GLOBAL_ELEMENTAL_SMASHING", GLOBAL_ELEMENTAL_SMASHING),
				value("GLOBAL_ELEMENTAL_MAULING", GLOBAL_ELEMENTAL_MAULING),
				value("GLOBAL_ELEMENTAL_PIERCING", GLOBAL_ELEMENTAL_PIERCING),
				// Target constants
				value("GLOBAL_TARGET_INVALID", GLOBAL_TARGET_INVALID),
				value("GLOBAL_TARGET_ATTACK_POINT", GLOBAL_TARGET_ATTACK_POINT),
				value("GLOBAL_TARGET_ACTOR", GLOBAL_TARGET_ACTOR),
				value("GLOBAL_TARGET_PARTY", GLOBAL_TARGET_PARTY)
			]
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalAttackPoint>("GlobalAttackPoint")
			.def("GetName", &GlobalAttackPoint::GetName)
			.def("GetXPosition", &GlobalAttackPoint::GetXPosition)
			.def("GetYPosition", &GlobalAttackPoint::GetYPosition)
			.def("GetFortitudeModifier", &GlobalAttackPoint::GetFortitudeModifier)
			.def("GetProtectionModifier", &GlobalAttackPoint::GetProtectionModifier)
			.def("GetEvadeModifier", &GlobalAttackPoint::GetEvadeModifier)
			.def("GetActorOwner", &GlobalAttackPoint::GetActorOwner)
			.def("GetTotalPhysicalDefense", &GlobalAttackPoint::GetTotalPhysicalDefense)
			.def("GetTotalMetaphysicalDefense", &GlobalAttackPoint::GetTotalMetaphysicalDefense)
			.def("GetTotalEvadeRating", &GlobalAttackPoint::GetTotalEvadeRating)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalActor>("GlobalActor")
			.def("GetID", &GlobalActor::GetID)
			.def("GetName", &GlobalActor::GetName)
			.def("GetFilename", &GlobalActor::GetFilename)
			.def("GetHitPoints", &GlobalActor::GetHitPoints)
			.def("GetMaxHitPoints", &GlobalActor::GetMaxHitPoints)
			.def("GetSkillPoints", &GlobalActor::GetSkillPoints)
			.def("GetMaxSkillPoints", &GlobalActor::GetMaxSkillPoints)
			.def("GetExperienceLevel", &GlobalActor::GetExperienceLevel)
			.def("GetStrength", &GlobalActor::GetStrength)
			.def("GetVigor", &GlobalActor::GetVigor)
			.def("GetFortitude", &GlobalActor::GetFortitude)
			.def("GetProtection", &GlobalActor::GetProtection)
			.def("GetAgility", &GlobalActor::GetAgility)
			.def("GetEvade", &GlobalActor::GetEvade)
			.def("GetTotalPhysicalAttack", &GlobalActor::GetTotalPhysicalAttack)
			.def("GetTotalMetaphysicalAttack", &GlobalActor::GetTotalMetaphysicalAttack)
// 			.def("GetWeaponEquipped", &GlobalActor::GetWeaponEquipped)
// 			.def("GetArmorEquipped", (GlobalArmor* (GlobalActor::*)(uint32)) &GlobalActor::GetArmorEquipped)
// 			.def("GetAttackPoints", &GlobalActor::GetAttackPoints)
// 			.def("GetElementalAttackBonuses", &GlobalActor::GetElementalAttackBonuses)
// 			.def("GetStatusAttackBonuses", &GlobalActor::GetStatusAttackBonuses)
// 			.def("GetElementalDefenseBonuses", &GlobalActor::GetElementalDefenseBonuses)
// 			.def("GetStatusDefenseBonuses", &GlobalActor::GetStatusDefenseBonuses)

			.def("SetHitPoints", &GlobalActor::SetHitPoints)
			.def("SetSkillPoints", &GlobalActor::SetSkillPoints)
			.def("SetMaxHitPoints", &GlobalActor::SetMaxHitPoints)
			.def("SetMaxSkillPoints", &GlobalActor::SetMaxSkillPoints)
			.def("SetExperienceLevel", &GlobalActor::SetExperienceLevel)
			.def("SetStrength", &GlobalActor::SetStrength)
			.def("SetVigor", &GlobalActor::SetVigor)
			.def("SetFortitude", &GlobalActor::SetFortitude)
			.def("SetProtection", &GlobalActor::SetProtection)
			.def("SetAgility", &GlobalActor::SetAgility)
			.def("SetEvade", &GlobalActor::SetEvade)

			.def("IsAlive", &GlobalActor::IsAlive)
// 			.def("EquipWeapon", &GlobalActor::EquipWeapon)
// 			.def("EquipArmor", &GlobalActor::EquipArmor)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalCharacterGrowth>("GlobalCharacterGrowth")
			.def_readwrite("_hit_points_growth", &GlobalCharacterGrowth::_hit_points_growth)
			.def_readwrite("_skill_points_growth", &GlobalCharacterGrowth::_skill_points_growth)
			.def_readwrite("_strength_growth", &GlobalCharacterGrowth::_strength_growth)
			.def_readwrite("_vigor_growth", &GlobalCharacterGrowth::_vigor_growth)
			.def_readwrite("_fortitude_growth", &GlobalCharacterGrowth::_fortitude_growth)
			.def_readwrite("_protection_growth", &GlobalCharacterGrowth::_protection_growth)
			.def_readwrite("_agility_growth", &GlobalCharacterGrowth::_agility_growth)
			.def_readwrite("_evade_growth", &GlobalCharacterGrowth::_evade_growth)
			.def("_AddSkill", &GlobalCharacterGrowth::_AddSkill)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalCharacter, GlobalActor>("GlobalCharacter")
			.def("GetGrowth", &GlobalCharacter::GetGrowth)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalParty>("GlobalParty")
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalEnemy, GlobalActor>("GlobalEnemy")
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalObject>("GlobalObject")
			.def("GetID", &GlobalObject::GetID)
			.def("GetName", &GlobalObject::GetName)
			.def("GetType", &GlobalObject::GetObjectType)
			.def("GetCount", &GlobalObject::GetCount)
			.def("IncrementCount", &GlobalObject::IncrementCount)
			.def("DecrementCount", &GlobalObject::DecrementCount)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalItem, GlobalObject>("GlobalItem")
// 			.def(constructor<>(uint32, uint32))
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalWeapon, GlobalObject>("GlobalWeapon")
			.def("GetUsableBy", &GlobalWeapon::GetUsableBy)
// 			.def(constructor<>(uint32, uint32))
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalArmor, GlobalObject>("GlobalArmor")
			.def("GetUsableBy", &GlobalArmor::GetUsableBy)
// 			.def(constructor<>(uint32, uint32))
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalStatusEffect>("GlobalStatusEffect")
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalElementalEffect>("GlobalElementalEffect")
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalSkill>("GlobalSkill")
	];

	} // End using global namespaces

	// ---------- (3) Bind Util Functions
	{
	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_utils")
	[
		def("RandomFloat", (float(*)(void)) &hoa_utils::RandomFloat)
	];
	}


	// ---------- (4) Bind Game Mode Components


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
			.def_readwrite("_running", &MapMode::_running)
			.def_readwrite("_run_forever", &MapMode::_run_forever)
			.def_readwrite("_run_disabled", &MapMode::_run_disabled)
			.def_readwrite("_run_stamina", &MapMode::_run_stamina)
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
			.def("GetXPosition", &MapObject::GetXPosition)
			.def("GetYPosition", &MapObject::GetYPosition)
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
		class_<ChestObject, PhysicalObject>("ChestObject")
			.def(constructor<std::string>())
			.def(constructor<std::string, uint8>())
			.def(constructor<std::string, uint8, uint8>())
			.def(constructor<std::string,uint8, uint8, uint32>())
			.def("AddObject", &ChestObject::AddObject)
			.def("AddDrunes", &ChestObject::AddDrunes)
			.def("UpdateHideForce", &ChestObject::UpdateHideForce)
			.def("SetHidingForce", &ChestObject::SetHidingForce)
			.def("GetHidingForce", &ChestObject::GetHidingForce)
			.def("IsHidden", &ChestObject::IsHidden)
			.def("IsUsed", &ChestObject::IsUsed)
			.def("Use", &ChestObject::Use)
			.def("Clear", &ChestObject::Clear)
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
			.def("ShowDialogueIcon", &VirtualSprite::ShowDialogueIcon)
			.def("IsShowingDialogueIcon", &VirtualSprite::IsShowingDialogueIcon)
			.def_readwrite("current_action", &VirtualSprite::current_action)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapSprite, VirtualSprite>("MapSprite")
			.def(constructor<>())
			.def("SetName", &MapSprite::SetName)
			.def("SetWalkSound", &MapSprite::SetWalkSound)
			.def("SetCurrentAnimation", &MapSprite::SetCurrentAnimation)
			.def("GetWalkSound", &MapSprite::GetWalkSound)
			.def("GetCurrentAnimation", &MapSprite::GetCurrentAnimation)
			.def("LoadStandardAnimations", &MapSprite::LoadStandardAnimations)
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
			.def_readwrite("start_row", &ZoneSection::start_row)
			.def_readwrite("end_row", &ZoneSection::end_row)
			.def_readwrite("start_col", &ZoneSection::start_col)
			.def_readwrite("end_col", &ZoneSection::end_col)
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
			.def("IsRestraining", &EnemyZone::IsRestraining)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapDialogue>("MapDialogue")
			.def(constructor<>())
			.def("AddText", &MapDialogue::AddText)
			.def("AddOption", &MapDialogue::AddOption)
			.def("SetMaxViews", &MapDialogue::SetMaxViews)
			.def("GoToLine", &MapDialogue::GoToLine)
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
			.def("SetLoops", &ActionAnimate::SetLoops)
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
			.def("TakeDamage", &BattleActor::TakeDamage)
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



	// ---------- (4) Bind engine class objects
	luabind::object global_table = luabind::globals(hoa_script::ScriptManager->GetGlobalState());
	global_table["AudioManager"]     = hoa_audio::AudioManager;
	global_table["InputManager"]     = hoa_input::InputManager;
	global_table["ModeManager"]      = hoa_mode_manager::ModeManager;
	global_table["ScriptManager"]    = hoa_script::ScriptManager;
	global_table["SystemManager"]    = hoa_system::SystemManager;
	global_table["VideoManager"]     = hoa_video::VideoManager;
	global_table["GlobalManager"]	 = hoa_global::GlobalManager;

} // void BindEngineToLua()

} // namespace hoa_defs
