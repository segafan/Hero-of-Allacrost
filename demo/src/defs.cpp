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
			.def("AddCharacter", &GameGlobal::AddCharacter)
			.def("GetCharacter", &GameGlobal::GetCharacter)
			.def("GetFunds", &GameGlobal::GetFunds)
			.def("SetFunds", &GameGlobal::SetFunds)
			.def("AddFunds", &GameGlobal::AddFunds)
			.def("SubtractFunds", &GameGlobal::SubtractFunds)
			.def("AddToInventory", &GameGlobal::AddToInventory)
			.def("RemoveFromInventory", &GameGlobal::RemoveFromInventory)
			.def("IncrementObjectCount", &GameGlobal::IncrementObjectCount)
			.def("DecrementObjectCount", &GameGlobal::DecrementObjectCount)

			// Namespace constants
			.enum_("constants") [
				// Character type constants
				value("GLOBAL_CHARACTER_CLAUDIUS", GLOBAL_CHARACTER_CLAUDIUS),
				// Object type constants
				value("GLOBAL_OBJECT_INVALID", GLOBAL_OBJECT_INVALID),
				value("GLOBAL_OBJECT_ITEM", GLOBAL_OBJECT_ITEM),
				value("GLOBAL_OBJECT_WEAPON", GLOBAL_OBJECT_WEAPON),
				value("GLOBAL_OBJECT_HEAD_ARMOR", GLOBAL_OBJECT_HEAD_ARMOR),
				value("GLOBAL_OBJECT_TORSO_ARMOR", GLOBAL_OBJECT_TORSO_ARMOR),
				value("GLOBAL_OBJECT_ARM_ARMOR", GLOBAL_OBJECT_ARM_ARMOR),
				value("GLOBAL_OBJECT_LEG_ARMOR", GLOBAL_OBJECT_LEG_ARMOR),
				value("GLOBAL_OBJECT_JEWEL", GLOBAL_OBJECT_JEWEL),
				value("GLOBAL_OBJECT_KEY_ITEM", GLOBAL_OBJECT_KEY_ITEM),
				// Item usage constants
				value("GLOBAL_USE_INVALID", GLOBAL_USE_INVALID),
				value("GLOBAL_USE_MENU", GLOBAL_USE_MENU),
				value("GLOBAL_USE_BATTLE", GLOBAL_USE_BATTLE),
				value("GLOBAL_USE_ALL", GLOBAL_USE_ALL),
				// Item and skill alignment constants
				value("GLOBAL_ALIGNMENT_INVALID", GLOBAL_ALIGNMENT_INVALID),
				value("GLOBAL_ALIGNMENT_GOOD", GLOBAL_ALIGNMENT_GOOD),
				value("GLOBAL_ALIGNMENT_BAD", GLOBAL_ALIGNMENT_BAD),
				value("GLOBAL_ALIGNMENT_NEUTRAL", GLOBAL_ALIGNMENT_NEUTRAL),
				value("GLOBAL_ALIGNMENT_TOTAL", GLOBAL_ALIGNMENT_TOTAL),
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
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalActor>("GlobalActor")
			.def("GetName", &GlobalActor::GetName)
			.def("GetHitPoints", &GlobalActor::GetHitPoints)
			.def("GetMaxHitPoints", &GlobalActor::GetMaxHitPoints)
			.def("GetSkillPoints", &GlobalActor::GetSkillPoints)
			.def("GetMaxSkillPoints", &GlobalActor::GetMaxSkillPoints)
			.def("GetExperienceLevel", &GlobalActor::GetExperienceLevel)
			.def("GetStrength", &GlobalActor::GetStrength)
			.def("GetVigor", &GlobalActor::GetVigor)
			.def("GetFortitude", &GlobalActor::GetFortitude)
			.def("GetResistance", &GlobalActor::GetResistance)
			.def("GetAgility", &GlobalActor::GetAgility)
			.def("GetEvade", &GlobalActor::GetEvade)
			.def("GetPhysicalAttackRating", &GlobalActor::GetPhysicalAttackRating)
			.def("GetMetaphysicalAttackRating", &GlobalActor::GetMetaphysicalAttackRating)
			.def("GetWeaponEquipped", &GlobalActor::GetWeaponEquipped)
			.def("GetArmorEquipped", &GlobalActor::GetArmorEquipped)
			.def("GetAttackPoints", &GlobalActor::GetAttackPoints)
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
		class_<GlobalEnemy, GlobalActor>("GlobalEnemy")
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalCharacter, GlobalActor>("GlobalCharacter")
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalParty>("GlobalParty")
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalObject>("GlobalObject")
			.def("GetID", &GlobalObject::GetID)
			.def("GetName", &GlobalObject::GetName)
			.def("GetType", &GlobalObject::GetType)
			.def("GetUsableBy", &GlobalObject::GetUsableBy)
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
// 			.def(constructor<>(uint32, uint32))
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalArmor, GlobalObject>("GlobalArmor")
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



	// ---------- (3) Bind Game Mode Components



	// ----- Map Mode Bindings
	{
	using namespace hoa_map;
	using namespace hoa_map::private_map;

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<MapMode, hoa_mode_manager::GameMode>("MapMode")
			.def(constructor<const std::string&>())
			.def("_AddGroundObject", &MapMode::_AddGroundObject, adopt(_2))
			.def("_AddPassObject", &MapMode::_AddPassObject, adopt(_2))
			.def("_AddSkyObject", &MapMode::_AddSkyObject, adopt(_2))
			.def("_AddZone", &MapMode::_AddZone, adopt(_2))
			.def("_SetCameraFocus", &MapMode::_SetCameraFocus)
			.def("_SetMapState", &MapMode::_SetMapState)
			.def("_GetMapState", &MapMode::_GetMapState)
			.def("_GetGeneratedObjectID", &MapMode::_GetGeneratedObjectID)

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
				value("EAST", WEST),
				value("WEST", EAST),
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
		class_<VirtualSprite, MapObject>("VirtualSprite")
			.def(constructor<>())
			.def("SetDirection", &VirtualSprite::SetDirection)
			.def("SetMovementSpeed", &VirtualSprite::SetMovementSpeed)
			.def("SetFacePortrait", &VirtualSprite::SetFacePortrait)
			.def("GetDirection", &VirtualSprite::GetDirection)
			.def("GetMovementSpeed", &VirtualSprite::GetMovementSpeed)
			.def("AddAction", &VirtualSprite::AddAction, adopt(_2))
			.def("AddDialogue", &VirtualSprite::AddDialogue, adopt(_2))
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
			.def("SetZone", &EnemySprite::SetZone)
			.def("Reset", &EnemySprite::Reset)
			.def("SetAggroRange", &EnemySprite::SetAggroRange)
			.def("GetAggroRange", &EnemySprite::GetAggroRange)
			.def("SetTimeToChange", &EnemySprite::SetTimeToChange)
			.def("GetTimeToChange", &EnemySprite::GetTimeToChange)
			.def("SetTimeToSpawn", &EnemySprite::SetTimeToSpawn)
			.def("GetTimeToSpawn", &EnemySprite::GetTimeToSpawn)
			.def("ChangeStateDead", &EnemySprite::ChangeStateDead)
			.def("ChangeStateSpawning", &EnemySprite::ChangeStateSpawning)
			.def("ChangeStateHostile", &EnemySprite::ChangeStateHostile)
			.def("IsDead", &EnemySprite::IsDead)
			.def("IsSpawning", &EnemySprite::IsSpawning)
			.def("IsHostile", &EnemySprite::IsHostile)
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
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
	[
		class_<SpriteAction>("SpriteAction")
			.def("Load", &SpriteAction::Load)
			.def("Execute", &SpriteAction::Execute)
	];

// 	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
// 	[
// 		class_<ActionPathMove, SpriteAction>("ActionPathMove")
// 			.def(constructor<>())
// 	];

// 	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_map")
// 	[
// 		class_<ActionAnimate, SpriteAction>("ActionAnimate")
// 			.def(constructor<>())
// 	];
	} // End using map mode namespaces



	// ----- Battle Mode bindings
	{
	using namespace hoa_battle;
	using namespace hoa_battle::private_battle;
	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_battle")
	[
		class_<BattleActor>("BattleActor")
			.def(constructor<>())
			.def("SetHitPoints", &BattleActor::SetHitPoints)
			.def("SetMaxHitPoints", &BattleActor::SetMaxHitPoints)
			.def("SetSkillPoints", &BattleActor::SetSkillPoints)
			.def("SetMaxSkillPoints", &BattleActor::SetMaxSkillPoints)
			.def("SetStrength", &BattleActor::SetStrength)
			.def("SetVigor", &BattleActor::SetVigor)
			.def("SetFortitude", &BattleActor::SetFortitude)
			.def("SetResistance", &BattleActor::SetResistance)
			.def("SetAgility", &BattleActor::SetAgility)
			.def("SetEvade", &BattleActor::SetEvade)
			.def("GetHitPoints", &BattleActor::GetHitPoints)
			.def("GetMaxHitPoints", &BattleActor::GetMaxHitPoints)
			.def("GetSkillPoints", &BattleActor::GetSkillPoints)
			.def("GetMaxSkillPoints", &BattleActor::GetMaxSkillPoints)
			.def("GetStrength", &BattleActor::GetStrength)
			.def("GetVigor", &BattleActor::GetVigor)
			.def("GetFortitude", &BattleActor::GetFortitude)
			.def("GetResistance", &BattleActor::GetResistance)
			.def("GetAgility", &BattleActor::GetAgility)
			.def("GetEvade", &BattleActor::GetEvade)
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
	global_table["GameModeManager"]  = hoa_mode_manager::ModeManager;
	global_table["ScriptManager"]    = hoa_script::ScriptManager;
	global_table["SystemManager"]    = hoa_system::SystemManager;
	global_table["VideoManager"]     = hoa_video::VideoManager;

} // void BindEngineToLua()

} // namespace hoa_defs
