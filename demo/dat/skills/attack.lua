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
		--If the random float is bigger, then we landed the hit
		if ((hoa_utils.RandomFloat() * 100) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 32 - target:GetPhysicalDefense());
		end
		AudioManager:PlaySound("snd/sword_swipe.wav");
	end
}

-- Another attack skill with faster warm-up and increased damage
skills[2] = {
	name = "Forward Thrust",
	description = "A quick and daring thrust into an opponent's flesh.",
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	sp_required = 2,
	warmup_time = 1200,
	cooldown_time = 0,
	level_required = 3,
	usage = hoa_global.GameGlobal.GLOBAL_USE_BATTLE,
	target_alignment = hoa_global.GameGlobal.GLOBAL_ALIGNMENT_NEUTRAL,

	BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if ((hoa_utils.RandomFloat() * 100) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 45 - target:GetPhysicalDefense());
		end
		AudioManager:PlaySound("snd/sword_swipe.wav");
	end
}


-- Enemy attack skills
skills[100] = {
	name = "Snake bite",
	description = "A really painful, venomous snake bite",
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	sp_required = 0,
	warmup_time = 900,
	cooldown_time = 0,
	level_required = 1,
	usage = hoa_global.GameGlobal.GLOBAL_USE_BATTLE,
	target_alignment = hoa_global.GameGlobal.GLOBAL_ALIGNMENT_BAD,

	BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if ((hoa_utils.RandomFloat() * 100) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 14 - target:GetPhysicalDefense());
		end
		--target:TakeDamage(14);
		AudioManager:PlaySound("snd/snake_attack.wav");
	end
}

skills[101] = {
	name = "Spider sting",
	description = "Do spiders have stings? Better than having a 'bite' everywhere!",
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	sp_required = 0,
	warmup_time = 1400,
	cooldown_time = 0,
	level_required = 1,
	usage = hoa_global.GameGlobal.GLOBAL_USE_BATTLE,
	target_alignment = hoa_global.GameGlobal.GLOBAL_ALIGNMENT_BAD,

	BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if ((hoa_utils.RandomFloat() * 100) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 13 - target:GetPhysicalDefense());
		end
		--target:TakeDamage(13);
		AudioManager:PlaySound("snd/spider_attack.wav");
	end
}

skills[102] = {
	name = "Slime Attack",
	description = "Green, wobbly attack made of sheer terror",
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	sp_required = 0,
	warmup_time = 1100,
	cooldown_time = 0,
	level_required = 1,
	usage = hoa_global.GameGlobal.GLOBAL_USE_BATTLE,
	target_alignment = hoa_global.GameGlobal.GLOBAL_ALIGNMENT_BAD,

	BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if ((hoa_utils.RandomFloat() * 100) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 10 - target:GetPhysicalDefense());
		end
		--target:TakeDamage(10);
		AudioManager:PlaySound("snd/slime_attack.wav");
	end
}


skills[103] = {
	name = "Skeleton Sword Attack",
	description = "Mighty Skeleton King's Sword Thrust. Don't get in way!",
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	sp_required = 0,
	warmup_time = 1400,
	cooldown_time = 0,
	level_required = 1,
	usage = hoa_global.GameGlobal.GLOBAL_USE_BATTLE,
	target_alignment = hoa_global.GameGlobal.GLOBAL_ALIGNMENT_BAD,

	BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if ((hoa_utils.RandomFloat() * 100) > target:GetCombatEvade()) then
			target:TakeDamage(instigator:GetPhysicalAttack() + 20 - target:GetPhysicalDefense());
		end
		--target:TakeDamage(20);
		AudioManager:PlaySound("snd/skeleton_attack.wav");
	end
}


