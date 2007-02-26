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
-- IDs 1-9999 are reserved for attack skills
-- -----------------------------------------------------------------------------

skills[1] = {
	name = "Slicing Rain",
	description = "A simple but effective vertical sword slash",
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	sp_usage = 0,
	warmup_time = 0,
	cooldown_time = 0,
	level_required = 1
	usage = hoa_global.GameGlobal.GLOBAL_ITEM_USE_ALL,


	ExecuteFunction = function()
		print("Executed Slicing Rain!")
	end
}
