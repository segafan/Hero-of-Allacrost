------------------------------------------------------------------------------[[
-- Filename: support.lua
--
-- Description: This file contains the definitions of all support skills that
-- exist in Hero of Allacrost. Each support skill has a unique integer identifier
-- that is used as its key in the skills table below.
------------------------------------------------------------------------------]]

-- All support skills definitions are stored in this table
if (skills == nil) then
	skills = {}
end


--------------------------------------------------------------------------------
-- IDs 20001-30000 are reserved for support skills
--------------------------------------------------------------------------------

skills[20001] = {
	name = hoa_system.Translate("Refresh"),
	description = hoa_system.Translate("Heals a party member by restoring a small amount of life force."),
	sp_required = 2,
	warmup_time = 1500,
	cooldown_time = 200,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ALLY,
   
	BattleExecute = function(user, target)
		target_actor = target:GetActor();
		target_actor:AddHitPoints(hoa_utils.RandomBoundedInteger(30,50));
		AudioManager:PlaySound("snd/heal.wav");
	end,
   
	FieldExecute = function(target, instigator)
		target:AddHitPoints(hoa_utils.RandomBoundedInteger(30,50));
		AudioManager:PlaySound("snd/heal_spell.wav");
	end
}
