------------------------------------------------------------------------------[[
-- Filename: attack.lua
--
-- Description: This file contains the definitions of all attack skills that
-- exist in Hero of Allacrost. Each attack skill has a unique integer identifier
-- that is used as its key in the skills table below.
------------------------------------------------------------------------------]]

-- All attack skills definitions are stored in this table
skills = {}


-- -----------------------------------------------------------------------------
-- IDs 1-10000 are reserved for attack skills
-- -----------------------------------------------------------------------------

skills[1] = {
	name = "Slicing Rain",
	description = "A simple but effective vertical sword slash.",
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	sp_required = 0,
	warmup_time = 2000,
	cooldown_time = 0,
	level_required = 1,
	usage = hoa_global.GameGlobal.GLOBAL_USE_BATTLE,
	target_alignment = hoa_global.GameGlobal.GLOBAL_ALIGNMENT_NEUTRAL,

	BattleExecute = function(target, instigator)
		target:TakeDamage(32);
		AudioManager:PlaySound("snd/sword_swipe.wav");
	end
}

-- Another attack skill with a slightly faster warmup but less damage
skills[2] = {
	name = "Super Nyrkkii!",
	description = "Even Mike Tyson would be afraid of a punch this powerful!",
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	sp_required = 0,
	warmup_time = 1200,
	cooldown_time = 0,
	level_required = 1,
	usage = hoa_global.GameGlobal.GLOBAL_USE_BATTLE,
	target_alignment = hoa_global.GameGlobal.GLOBAL_ALIGNMENT_BAD,

	BattleExecute = function(target, instigator)
		target:TakeDamage(15);
		AudioManager:PlaySound("snd/sword_swipe.wav");
	end
}
