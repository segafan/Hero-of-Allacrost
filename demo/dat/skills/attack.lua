------------------------------------------------------------------------------[[
-- Filename: attack.lua
--
-- Description: This file contains the definitions of all attack skills that
-- exist in Hero of Allacrost. Each attack skill has a unique integer identifier
-- that is used as its key in the skills table below.
------------------------------------------------------------------------------]]

-- All attack skills definitions are stored in this table
if (skills == nil) then
   skills = {}
end


-- -----------------------------------------------------------------------------
-- IDs 1-10000 are reserved for attack skills
-- -----------------------------------------------------------------------------

skills[1] = {
	name = "Slicing Rain",
	description = "A simple but effective vertical sword slash.",
	sp_required = 0,
	warmup_time = 2000,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if ((instigator:GetCombatAgility() * (hoa_utils.RandomFloat() * 100)) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 5 - target:GetPhysicalDefense());
		end
		AudioManager:PlaySound("snd/sword_swipe.wav");
	end
}

-- Another attack skill with faster warm-up and increased damage
skills[2] = {
	name = "Forward Thrust",
	description = "A quick and daring thrust into an opponent's flesh.",
	sp_required = 2,
	warmup_time = 1200,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if ((instigator:GetCombatAgility() * (hoa_utils.RandomFloat() * 100)) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 10 - target:GetPhysicalDefense());
		end
		AudioManager:PlaySound("snd/sword_swipe.wav");
	end
}

skills[3] = {
	name = "Power Up",
	description = "Increase strength.",
	sp_required = 3,
	warmup_time = 1200,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ACTOR,
	target_ally = true,

	BattleExecute = function(target, instigator)
		target:AddNewEffect(1);
		AudioManager:PlaySound("snd/rumble.wav");
	end
}

skills[11] = {
	name = "Quick Shot",
	description = "A quick but weak crossbow attack.",
	sp_required = 0,
	warmup_time = 1000,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(target, instigator)
		if ((instigator:GetCombatAgility() * (hoa_utils.RandomFloat() * 100)) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 8 - target:GetPhysicalDefense());
			AudioManager:PlaySound("snd/crossbow.ogg");
		else
			AudioManager:PlaySound("snd/crossbow_miss.ogg");
		end
	end
}


-- Enemy attack skills
skills[100] = {
	name = "Snake bite",
	sp_required = 0,
	warmup_time = 900,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if ((instigator:GetCombatAgility() * (hoa_utils.RandomFloat() * 100)) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 14 - target:GetPhysicalDefense());
		end
		--target:TakeDamage(14);
		AudioManager:PlaySound("snd/snake_attack.wav");
	end
}

skills[101] = {
	name = "Spider sting",
	sp_required = 0,
	warmup_time = 1400,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if ((instigator:GetCombatAgility() * (hoa_utils.RandomFloat() * 100)) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 13 - target:GetPhysicalDefense());
		end
		--target:TakeDamage(13);
		AudioManager:PlaySound("snd/spider_attack.wav");
	end
}

skills[102] = {
	name = "Slime Attack",
	sp_required = 0,
	warmup_time = 1100,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if ((instigator:GetCombatAgility() * (hoa_utils.RandomFloat() * 100)) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 10 - target:GetPhysicalDefense());
		end
		--target:TakeDamage(10);
		AudioManager:PlaySound("snd/slime_attack.wav");
	end
}


skills[103] = {
	name = "Skeleton Sword Attack",
	sp_required = 0,
	warmup_time = 1400,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if ((instigator:GetCombatAgility() * (hoa_utils.RandomFloat() * 100)) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 20 - target:GetPhysicalDefense());
		end
		--target:TakeDamage(20);
		AudioManager:PlaySound("snd/skeleton_attack.wav");
	end
}

skills[104] = {
   name = "Drake attack",
   sp_required = 0,
   warmup_time = 900,
   cooldown_time = 0,
   target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
   target_ally = false,
   
   BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if ((instigator:GetCombatAgility() * (hoa_utils.RandomFloat() * 100)) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 20 - target:GetPhysicalDefense());
		end
	end
}

skills[105] = {
   name = "Deceleon attack",
   sp_required = 0,
   warmup_time = 900,
   cooldown_time = 0,
   target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
   target_ally = false,
   
   BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if ((instigator:GetCombatAgility() * (hoa_utils.RandomFloat() * 100)) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 20 - target:GetPhysicalDefense());
		end
	end
}

skills[106] = {
   name = "Nagaruda attack",
   sp_required = 0,
   warmup_time = 1400,
   cooldown_time = 0,
   target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
   target_ally = false,
   
   BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if ((instigator:GetCombatAgility() * (hoa_utils.RandomFloat() * 100)) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 30 - target:GetPhysicalDefense());
		end
	end
}

skills[107] = {
   name = "Aurum Drakueli attack",
   sp_required = 0,
   warmup_time = 900,
   cooldown_time = 0,
   target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
   target_ally = false,
   
   BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if ((instigator:GetCombatAgility() * (hoa_utils.RandomFloat() * 100)) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 20 - target:GetPhysicalDefense());
		end
	end
}
