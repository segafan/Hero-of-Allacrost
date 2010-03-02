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
	name = hoa_system.Translate("Slicing Rain"),
	description = hoa_system.Translate("A simple but effective sword slash."),
	sp_required = 0,
	warmup_time = 2000,
	cooldown_time = 200,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(user, target)
		user:ChangeSpriteAnimation("attack");
		target_actor = target:GetActor();
		target_point = target_actor:GetAttackPoint(target:GetAttackPoint());

		--If the random float is bigger, then we landed the hit
		if (hoa_utils.RandomFloat() * 100 > target_point:GetTotalEvadeRating()) then
			target_actor:RegisterDamage(user:GetTotalPhysicalAttack() + 5 - target_point:GetTotalPhysicalDefense());
			AudioManager:PlaySound("snd/swordslice1.wav");
		else
			target_actor:RegisterMiss();
			AudioManager:PlaySound("snd/sword_swipe.wav");
		end
	end
}

-- Another attack skill with faster warm-up and increased damage
skills[2] = {
	name = hoa_system.Translate("Forward Thrust"),
	description = hoa_system.Translate("A quick and daring thrust into an opponent's flesh."),
	sp_required = 1,
	warmup_time = 1200,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(target, instigator)
		instigator:ChangeSpriteAnimation("attack");

		if (hoa_utils.RandomFloat() * 100 > target:GetTotalEvadeRating() + 2.5) then
			target:TakeDamage(instigator:GetTotalPhysicalAttack() + 10 - target:GetPhysicalDefense());
			AudioManager:PlaySound("snd/swordslice1.wav");
		else
			target:TakeDamage(0);
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
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(target, instigator)
		instigator:ChangeSpriteAnimation("attack");

		if (hoa_utils.RandomFloat() * 100 > target:GetTotalEvadeRating() + 5.5) then
			target:TakeDamage(instigator:GetTotalPhysicalAttack() - target:GetPhysicalDefense());
			AudioManager:PlaySound("snd/swordslice1.wav");
			target:AddNewEffect(3);
		else
			target:TakeDamage(0);
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
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_PARTY,
	target_ally = false,

	BattleExecute = function(target, instigator)
		instigator:ChangeSpriteAnimation("attack");
		target:TakeDamage(instigator:GetTotalPhysicalAttack());
		AudioManager:PlaySound("snd/rumble.wav");
	end
}

skills[11] = {
	name = hoa_system.Translate("Quick Shot"),
	description = hoa_system.Translate("A quick but weak crossbow attack."),
	sp_required = 0,
	warmup_time = 1000,
	cooldown_time = 0,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(user, target)
		target_actor = target:GetActor();
		target_point = target_actor:GetAttackPoint(target:GetAttackPoint());

		if (hoa_utils.RandomFloat() * 100 > target_point:GetTotalEvadeRating() + 5.5) then
			target_actor:RegisterDamage(user:GetTotalPhysicalAttack() + 8 - target_point:GetTotalPhysicalDefense());
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
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(user, target)
		target_actor = target:GetActor();
		target_point = target_actor:GetAttackPoint(target:GetAttackPoint());
		user:ChangeSpriteAnimation("attack");

		if (hoa_utils.RandomFloat() * 100 > target_point:GetTotalEvadeRating()) then
			target_actor:RegisterDamage(user:GetTotalPhysicalAttack() + 3 - target_point:GetTotalPhysicalDefense());
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
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(target, instigator)
		instigator:ChangeSpriteAnimation("attack");
		if (hoa_utils.RandomFloat() * 100 > target:GetTotalEvadeRating() + 8.5) then
			target:TakeDamage(instigator:GetTotalPhysicalAttack() + 20 - target:GetPhysicalDefense());
			AudioManager:PlaySound("snd/swordslice2.wav");
		else
			target:TakeDamage(0);
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
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(user, target)
		target_actor = target:GetActor();
		target_point = target_actor:GetAttackPoint(target:GetAttackPoint());
		user:ChangeSpriteAnimation("attack");

		--If the random float is bigger, then we landed the hit
		if (hoa_utils.RandomFloat() * 100 > target_point:GetTotalEvadeRating()) then
			target_actor:RegisterDamage(user:GetTotalPhysicalAttack() + 5 - target_point:GetTotalPhysicalDefense());
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
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if (hoa_utils.RandomFloat() * 100 > target:GetTotalEvadeRating() + 5.5) then
			target:TakeDamage(instigator:GetTotalPhysicalAttack() * 1.5 + 0 - target:GetPhysicalDefense());
			AudioManager:PlaySound("snd/sword_swipe.wav");
		else
			target:TakeDamage(0);
			AudioManager:PlaySound("snd/sword_swipe.wav");
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
		if (hoa_utils.RandomFloat() * 100 > target:GetTotalEvadeRating()) then
			target:TakeDamage(instigator:GetTotalPhysicalAttack() + 14 - target:GetPhysicalDefense());
			AudioManager:PlaySound("snd/snake_attack.wav");
		end
		instigator:ChangeSpriteAnimation("attack");
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
		if (hoa_utils.RandomFloat() * 100 > target:GetTotalEvadeRating()) then
			target:TakeDamage(instigator:GetTotalPhysicalAttack() + 13 - target:GetPhysicalDefense());
			AudioManager:PlaySound("snd/spider_attack.wav");
		else
			target:TakeDamage(0);
		end
		instigator:ChangeSpriteAnimation("attack");
	end
}

skills[102] = {
	name = "Slime Attack",
	sp_required = 0,
	warmup_time = 1100,
	cooldown_time = 500,
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(user, target)
		target_actor = target:GetActor();
		target_point = target_actor:GetAttackPoint(target:GetAttackPoint());

		-- If the random float is bigger, then we landed the hit
		if (hoa_utils.RandomFloat() * 100 > target_point:GetTotalEvadeRating()) then
			target_actor:RegisterDamage(user:GetTotalPhysicalAttack() + 10 - target_point:GetTotalPhysicalDefense());
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
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ATTACK_POINT,
	target_ally = false,

	BattleExecute = function(target, instigator)
		--If the random float is bigger, then we landed the hit
		if (hoa_utils.RandomFloat() * 100 > target:GetTotalEvadeRating()) then
			target:TakeDamage(instigator:GetTotalPhysicalAttack() + 20 - target:GetPhysicalDefense());
		end
		instigator:ChangeSpriteAnimation("attack");
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
		if (hoa_utils.RandomFloat() * 100 > target:GetTotalEvadeRating()) then
			target:TakeDamage(instigator:GetTotalPhysicalAttack() + 20 - target:GetPhysicalDefense());
			AudioManager:PlaySound("snd/skeleton_attack.wav");
		else
			target:TakeDamage(0);
		end
		instigator:ChangeSpriteAnimation("attack");
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
		if (hoa_utils.RandomFloat() * 100 > target:GetTotalEvadeRating()) then
			target:TakeDamage(instigator:GetTotalPhysicalAttack() + 20 - target:GetPhysicalDefense());
		end
		instigator:ChangeSpriteAnimation("attack");
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
		if (hoa_utils.RandomFloat() * 100 > target:GetTotalEvadeRating()) then
			target:TakeDamage(instigator:GetTotalPhysicalAttack() + 30 - target:GetPhysicalDefense());
		end
		instigator:ChangeSpriteAnimation("attack");
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
		if (hoa_utils.RandomFloat() * 100 > target:GetTotalEvadeRating()) then
			target:TakeDamage(instigator:GetTotalPhysicalAttack() + 20 - target:GetPhysicalDefense());
		end
		instigator:ChangeSpriteAnimation("attack");
	end
}
