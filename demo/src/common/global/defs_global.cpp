///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    defs_global.cpp
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

#include "global.h"
#include "global_actors.h"
#include "global_effects.h"
#include "global_objects.h"
#include "global_skills.h"
#include "global_utils.h"

using namespace luabind;

namespace hoa_defs {

void BindGlobalsToLua()
{
	// ---------- Bind Global Components
	{
	using namespace hoa_global;

	def("GetTargetText", &GetTargetText),
	def("IsTargetPoint", &IsTargetPoint),
	def("IsTargetActor", &IsTargetActor),
	def("IsTargetParty", &IsTargetParty),
	def("IsTargetSelf", &IsTargetSelf),
	def("IsTargetAlly", &IsTargetAlly),
	def("IsTargetFoe", &IsTargetFoe),
	// TODO: Luabind doesn't like these functions. I think its because they take reference arguments.
// 	def("IncrementIntensity", &IncrementIntensity),
// 	def("DecrementIntensity", &DecrementIntensity),

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GameGlobal>("GameGlobal")
			.def("AddCharacter", (void(GameGlobal::*)(uint32)) &GameGlobal::AddCharacter)
			.def("RemoveCharacter", (void(GameGlobal::*)(uint32)) &GameGlobal::RemoveCharacter)
			.def("GetCharacter", &GameGlobal::GetCharacter)
			.def("GetDrunes", &GameGlobal::GetDrunes)
			.def("SetDrunes", &GameGlobal::SetDrunes)
			.def("AddDrunes", &GameGlobal::AddDrunes)
			.def("SubtractDrunes", &GameGlobal::SubtractDrunes)
			.def("AddToInventory", (void (GameGlobal::*)(uint32, uint32)) &GameGlobal::AddToInventory)
			.def("RemoveFromInventory", (void (GameGlobal::*)(uint32)) &GameGlobal::RemoveFromInventory)
			.def("IncrementObjectCount", &GameGlobal::IncrementObjectCount)
			.def("DecrementObjectCount", &GameGlobal::DecrementObjectCount)
			.def("DoesEventGroupExist", &GameGlobal::DoesEventGroupExist)
			.def("DoesEventExist", &GameGlobal::DoesEventExist)
			.def("AddNewEventGroup", &GameGlobal::AddNewEventGroup)
			.def("GetEventGroup", &GameGlobal::GetEventGroup)
			.def("GetEventValue", &GameGlobal::GetEventValue)
			.def("GetNumberEventGroups", &GameGlobal::GetNumberEventGroups)
			.def("GetNumberEvents", &GameGlobal::GetNumberEvents)
			.def("SetLocation", (void(GameGlobal::*)(const std::string&)) &GameGlobal::SetLocation)

			// Namespace constants
			.enum_("constants") [
				// Character type constants
				value("GLOBAL_CHARACTER_INVALID", GLOBAL_CHARACTER_INVALID),
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
				value("GLOBAL_USE_FIELD", GLOBAL_USE_FIELD),
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
				// Status type constants
				value("GLOBAL_STATUS_HP_BOOST", GLOBAL_STATUS_HP_BOOST),
				value("GLOBAL_STATUS_HP_DRAIN", GLOBAL_STATUS_HP_DRAIN),
				value("GLOBAL_STATUS_SP_BOOST", GLOBAL_STATUS_SP_BOOST),
				value("GLOBAL_STATUS_SP_DRAIN", GLOBAL_STATUS_SP_DRAIN),
				value("GLOBAL_STATUS_STRENGTH_BOOST", GLOBAL_STATUS_STRENGTH_BOOST),
				value("GLOBAL_STATUS_STRENGTH_DRAIN", GLOBAL_STATUS_STRENGTH_DRAIN),
				value("GLOBAL_STATUS_VIGOR_BOOST", GLOBAL_STATUS_VIGOR_BOOST),
				value("GLOBAL_STATUS_VIGOR_DRAIN", GLOBAL_STATUS_VIGOR_DRAIN),
				value("GLOBAL_STATUS_FORTITUDE_BOOST", GLOBAL_STATUS_FORTITUDE_BOOST),
				value("GLOBAL_STATUS_FORTITUDE_DRAIN", GLOBAL_STATUS_FORTITUDE_DRAIN),
				value("GLOBAL_STATUS_PROTECTION_BOOST", GLOBAL_STATUS_PROTECTION_BOOST),
				value("GLOBAL_STATUS_PROTECTION_DRAIN", GLOBAL_STATUS_PROTECTION_DRAIN),
				value("GLOBAL_STATUS_AGILITY_BOOST", GLOBAL_STATUS_AGILITY_BOOST),
				value("GLOBAL_STATUS_AGILITY_DRAIN", GLOBAL_STATUS_AGILITY_DRAIN),
				value("GLOBAL_STATUS_EVADE_BOOST", GLOBAL_STATUS_EVADE_BOOST),
				value("GLOBAL_STATUS_EVADE_DRAIN", GLOBAL_STATUS_EVADE_DRAIN),
				value("GLOBAL_STATUS_PARALYSIS", GLOBAL_STATUS_PARALYSIS),
				// Intensity type constants
				value("GLOBAL_INTENSITY_NEG_EXTREME", GLOBAL_INTENSITY_NEG_EXTREME),
				value("GLOBAL_INTENSITY_NEG_GREATER", GLOBAL_INTENSITY_NEG_GREATER),
				value("GLOBAL_INTENSITY_NEG_MODERATE", GLOBAL_INTENSITY_NEG_MODERATE),
				value("GLOBAL_INTENSITY_NEG_LESSER", GLOBAL_INTENSITY_NEG_LESSER),
				value("GLOBAL_INTENSITY_NEUTRAL", GLOBAL_INTENSITY_NEUTRAL),
				value("GLOBAL_INTENSITY_POS_LESSER", GLOBAL_INTENSITY_POS_LESSER),
				value("GLOBAL_INTENSITY_POS_MODERATE", GLOBAL_INTENSITY_POS_MODERATE),
				value("GLOBAL_INTENSITY_POS_GREATER", GLOBAL_INTENSITY_POS_GREATER),
				value("GLOBAL_INTENSITY_POS_EXTREME", GLOBAL_INTENSITY_POS_EXTREME),
				// Target constants
				value("GLOBAL_TARGET_INVALID", GLOBAL_TARGET_INVALID),
				value("GLOBAL_TARGET_SELF_POINT", GLOBAL_TARGET_SELF_POINT),
				value("GLOBAL_TARGET_ALLY_POINT", GLOBAL_TARGET_ALLY_POINT),
				value("GLOBAL_TARGET_FOE_POINT", GLOBAL_TARGET_FOE_POINT),
				value("GLOBAL_TARGET_SELF", GLOBAL_TARGET_SELF),
				value("GLOBAL_TARGET_ALLY", GLOBAL_TARGET_ALLY),
				value("GLOBAL_TARGET_FOE", GLOBAL_TARGET_FOE),
				value("GLOBAL_TARGET_ALL_ALLIES", GLOBAL_TARGET_ALL_ALLIES),
				value("GLOBAL_TARGET_ALL_FOES", GLOBAL_TARGET_ALL_FOES)
			]
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalEventGroup>("GlobalEventGroup")
			.def("DoesEventExist", &GlobalEventGroup::DoesEventExist)
			.def("AddNewEvent", &GlobalEventGroup::AddNewEvent)
			.def("GetEvent", &GlobalEventGroup::GetEvent)
			.def("SetEvent", &GlobalEventGroup::SetEvent)
			.def("GetNumberEvents", &GlobalEventGroup::GetNumberEvents)
			.def("GetGroupName", &GlobalEventGroup::GetGroupName)
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
			.def("GetAttackPoint", &GlobalActor::GetAttackPoint)
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

			.def("AddHitPoints", &GlobalActor::AddHitPoints)
			.def("SubtractHitPoints", &GlobalActor::SubtractHitPoints)
			.def("AddMaxHitPoints", &GlobalActor::AddMaxHitPoints)
			.def("SubtractMaxHitPoints", &GlobalActor::SubtractMaxHitPoints)
			.def("AddSkillPoints", &GlobalActor::AddSkillPoints)
			.def("SubtractSkillPoints", &GlobalActor::SubtractSkillPoints)
			.def("AddMaxSkillPoints", &GlobalActor::AddMaxSkillPoints)
			.def("SubtractMaxSkillPoints", &GlobalActor::SubtractMaxSkillPoints)
			.def("AddStrength", &GlobalActor::AddStrength)
			.def("SubtractStrength", &GlobalActor::SubtractStrength)
			.def("AddVigor", &GlobalActor::AddVigor)
			.def("SubtractVigor", &GlobalActor::SubtractVigor)
			.def("AddFortitude", &GlobalActor::AddFortitude)
			.def("SubtractFortitude", &GlobalActor::SubtractFortitude)
			.def("AddProtection", &GlobalActor::AddProtection)
			.def("SubtractProtection", &GlobalActor::SubtractProtection)
			.def("AddAgility", &GlobalActor::AddAgility)
			.def("SubtractAgility", &GlobalActor::SubtractAgility)
			.def("AddEvade", &GlobalActor::AddEvade)
			.def("SubtractEvade", &GlobalActor::SubtractEvade)

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
			.def("AddSkill", &GlobalCharacter::AddSkill)
	];

	module(hoa_script::ScriptManager->GetGlobalState(), "hoa_global")
	[
		class_<GlobalParty>("GlobalParty")
			.def("AddHitPoints", &GlobalParty::AddHitPoints)
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
			.def("GetType", &GlobalStatusEffect::GetType)
			.def("GetIntensity", &GlobalStatusEffect::GetIntensity)
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

	// Bind GlobalManager to Lua
	luabind::object global_table = luabind::globals(hoa_script::ScriptManager->GetGlobalState());
	global_table["GlobalManager"] = hoa_global::GlobalManager;
} // BindGlobalsToLua

} // namespace hoa_defs
