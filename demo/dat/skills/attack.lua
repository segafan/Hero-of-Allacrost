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


--------------------------------------------------------------------------------
-- IDs 1-10000 are reserved for attack skills
--------------------------------------------------------------------------------

skills[1] = {
	name = hoa_system.Translate("Slicing Rain"),
	description = hoa_system.Translate("A simple but effective sword slash."),
	sp_required = 0,
	warmup_time = 2000,
	cooldown_time = 200,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		user:ChangeSpriteAnimation("attack");
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 5, 0));
			AudioManager:PlaySound("snd/swordslice1.wav");
		else
			target_actor:RegisterMiss();
			AudioManager:PlaySound("snd/sword_swipe.wav");
		end
	end
}

skills[2] = {
	name = hoa_system.Translate("Forward Thrust"),
	description = hoa_system.Translate("A quick and daring thrust into an opponent's flesh."),
	sp_required = 1,
	warmup_time = 500,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		user:ChangeSpriteAnimation("attack");
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target, 5.0) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 0, 0, 0.5));
			AudioManager:PlaySound("snd/swordslice1.wav");
		else
			target_actor:RegisterMiss();
			AudioManager:PlaySound("snd/sword_swipe.wav");
		end

	end
}

skills[3] = {
	name = hoa_system.Translate("Stun Slash"),
	description = hoa_system.Translate("A quick and daring thrust into an opponent's flesh."),
	sp_required = 3,
	warmup_time = 1200,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		user:ChangeSpriteAnimation("attack");
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target, 5.5) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 0, 0));
			target_actor:AddNewEffect(3);
			AudioManager:PlaySound("snd/swordslice1.wav");
		else
			target_actor:RegisterMiss();
			AudioManager:PlaySound("snd/sword_swipe.wav");
		end
	end
}

skills[4] = {
	-- FIXME: This does not work yet.
	name = hoa_system.Translate("Whirlwind"),
	description = hoa_system.Translate("Damage all enemies"),
	sp_required = 7,
	warmup_time = 1200,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ALL_FOES,

	BattleExecute = function(user, target)
		user:ChangeSpriteAnimation("attack");
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 0, 0));
			AudioManager:PlaySound("snd/rumble.wav");
		else
			target_actor:RegisterMiss();
			AudioManager:PlaySound("snd/sword_swipe.wav");
		end
	end
}

skills[11] = {
	name = hoa_system.Translate("Quick Shot"),
	description = hoa_system.Translate("A quick but weak crossbow attack."),
	sp_required = 0,
	warmup_time = 1000,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		user:ChangeSpriteAnimation("attack");
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target, 5.5) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 8, 0));
			AudioManager:PlaySound("snd/crossbow.ogg");
		else
			target_actor:RegisterMiss();
			AudioManager:PlaySound("snd/crossbow_miss.ogg");
		end
	end
}

skills[21] = {
	name = hoa_system.Translate("Side Slash"),
	description = hoa_system.Translate("A very basic sword attack."),
	sp_required = 0,
	warmup_time = 1500,
	cooldown_time = 500,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		user:ChangeSpriteAnimation("attack");
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 3, 0));
			AudioManager:PlaySound("snd/swordslice2.wav");
		else
			target_actor:RegisterMiss();
			AudioManager:PlaySound("snd/sword_swipe.wav");
		end
	end
}

skills[22] = {
	name = hoa_system.Translate("Blade Rush"),
	description = hoa_system.Translate("Attempt for a powerful blow."),
	sp_required = 4,
	warmup_time = 2000,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		user:ChangeSpriteAnimation("attack");
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target, 8.5) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 20, 0));
			AudioManager:PlaySound("snd/swordslice2.wav");
		else
			target_actor:RegisterMiss();
			AudioManager:PlaySound("snd/sword_swipe.wav");
		end
	end
}

skills[31] = {
	name = hoa_system.Translate("Stab"),
	description = hoa_system.Translate("Stab enemy with dagger."),
	sp_required = 0,
	warmup_time = 2000,
	cooldown_time = 750,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		user:ChangeSpriteAnimation("attack");
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 5, 0));
			AudioManager:PlaySound("snd/sword_swipe.wav");
		else
			target_actor:RegisterMiss();
			AudioManager:PlaySound("snd/sword_swipe.wav");
		end
	end
}

skills[32] = {
	name = hoa_system.Translate("Dagger Throw"),
	description = hoa_system.Translate("Throw dagger at an enemy."),
	sp_required = 3,
	warmup_time = 2000,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		user:ChangeSpriteAnimation("attack");
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target, 5.5) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamageMultiplier(user, target, 0.5, 0));
			AudioManager:PlaySound("snd/sword_swipe.wav");
		else
			target_actor:RegisterMiss();
			AudioManager:PlaySound("snd/sword_swipe.wav");
		end
	end
}

--------------------------------------------------------------------------------
-- Enemy attack skills
--------------------------------------------------------------------------------

skills[100] = {
	name = "Snake bite",
	sp_required = 0,
	warmup_time = 900,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 14, 0));
			AudioManager:PlaySound("snd/snake_attack.wav");
		else
			target_actor:RegisterMiss();
		end
	end
}

skills[101] = {
	name = "Spider sting",
	sp_required = 0,
	warmup_time = 1400,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 13, 0));
			AudioManager:PlaySound("snd/spider_attack.wav");
		else
			target_actor:RegisterMiss();
		end
	end
}

skills[102] = {
	name = "Slime Attack",
	sp_required = 0,
	warmup_time = 1100,
	cooldown_time = 500,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 10, 0));
			AudioManager:PlaySound("snd/slime_attack.wav");
		else
			target_actor:RegisterMiss();
		end
	end
}


skills[103] = {
	name = "Skeleton Sword Attack",
	sp_required = 0,
	warmup_time = 1400,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 20, 0));
			AudioManager:PlaySound("snd/skeleton_attack.wav");
		else
			target_actor:RegisterMiss();
		end
	end
}

skills[104] = {
	name = "Drake attack",
	sp_required = 0,
	warmup_time = 900,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 20, 0));
			AudioManager:PlaySound("snd/skeleton_attack.wav");
		else
			target_actor:RegisterMiss();
		end
	end
}

skills[105] = {
	name = "Deceleon attack",
	sp_required = 0,
	warmup_time = 900,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 20, 0));
			--AudioManager:PlaySound("???"); TODO: find an appropriate sound
		else
			target_actor:RegisterMiss();
		end
	end
}

skills[106] = {
	name = "Nagaruda attack",
	sp_required = 0,
	warmup_time = 1400,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 30, 0));
			--AudioManager:PlaySound("???"); TODO: find an appropriate sound
		else
			target_actor:RegisterMiss();
		end
	end
}

skills[107] = {
	name = "Aurum Drakueli attack",
	sp_required = 0,
	warmup_time = 900,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE_POINT,

	BattleExecute = function(user, target)
		target_actor = target:GetActor();

		if (hoa_battle.CalculateStandardEvasion(target) == false) then
			target_actor:RegisterDamage(hoa_battle.CalculateStandardDamage(user, target, 20, 0));
			--AudioManager:PlaySound("???"); TODO: find an appropriate sound
		else
			target_actor:RegisterMiss();
		end
	end
}
