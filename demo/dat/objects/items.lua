------------------------------------------------------------------------------[[
-- Filename: items.lua
--
-- Description: This file contains the definitions of all items that exist in
-- Hero of Allacrost. Each item has a unique integer identifier that is used
-- as its key in the items table below.
--
-- Note (1): Item ids do *not* need to be sequential. When you make a new item,
-- consider giving it some space from the other items. This way, we won't
-- get a jumbled mess of different ids.
--
-- Note (2): Valid ids for items are 1-10000. Do not exceed this limit, because
-- higher id values correspond to other types of objects (weapons, armor, etc.)
--
-- Note (3): Items can have three different types of attack targets: an attack
-- point, an actor, or a party. The script defined for each item assumes that
-- the argument which it is passed (GlobalAttackPoint, GlobalActor, GlobalParty)
-- is what it expected it to be.
------------------------------------------------------------------------------]]

-- All item definitions are stored in this table
if (items == nil) then
	items = {}
end

--------------------------------------------------------------------------------
-- IDs 1-1000 are reserved for healing potions
--------------------------------------------------------------------------------

items[1] = {
	name = hoa_system.Translate("Light Healing Potion"),
	description = hoa_system.Translate("Restores a small amount of hit points to a target."),
	icon = "img/icons/items/health_potion_small.png",
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ALLY,
	standard_price = 60,

	BattleUse = function(user, target)
		target_actor = target:GetActor();
		target_actor:RegisterHealing(45);
		AudioManager:PlaySound("snd/potion_drink.wav");
	end,

	FieldUse = function(target)
		target:AddHitPoints(45);
		AudioManager:PlaySound("snd/potion_drink.wav");
	end
}

items[2] = {
	name = hoa_system.Translate("Medium Healing Potion"),
	description = hoa_system.Translate("Restores a moderate amount of hit points to a target."),
	icon = "img/icons/items/health_potion_medium.png",
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ALLY,
	standard_price = 200,

	BattleUse = function(user, target)
		target_actor = target:GetActor();
		target_actor:RegisterHealing(45);
		AudioManager:PlaySound("snd/potion_drink.wav");
	end,

	FieldUse = function(target)
		target:AddHitPoints(150);
		AudioManager:PlaySound("snd/potion_drink.wav");
	end
}

items[3] = {
	name = hoa_system.Translate("Blessed Healing Potion"),
	description = hoa_system.Translate("Restores a large amount of hit points to a target."),
	icon = "img/icons/items/health_potion_large.png",
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ALLY,
	standard_price = 500,

	BattleUse = function(user, target)
		target_actor = target:GetActor();
		target_actor:RegisterHealing(45);
		AudioManager:PlaySound("snd/potion_drink.wav");
	end,

	FieldUse = function(target)
		target:AddHitPoints(250);
		AudioManager:PlaySound("snd/potion_drink.wav");
	end
}

items[4] = {
	name = hoa_system.Translate("Super Healing Potion"),
	description = hoa_system.Translate("Restores a small amount of hit points to all party members."),
	icon = "img/icons/items/health_potion_huge.png",
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ALL_ALLIES,
	standard_price = 1000,

	BattleUse = function(user, target)
		-- TODO: no support yet for party targets
		target_party = target:GetParty();
		-- TODO: iterate through each living actor in the party and add healing
		target_actor:RegisterHealing(100);
		AudioManager:PlaySound("snd/potion_drink.wav");
	end,

	FieldUse = function(target)
		target:AddHitPoints(100);
		AudioManager:PlaySound("snd/potion_drink.wav");
	end
}

--------------------------------------------------------------------------------
-- IDs 1001-2000 are reserved for status potions
--------------------------------------------------------------------------------

-- items[1001] = {}

--------------------------------------------------------------------------------
-- IDs 2001-3000 are reserved for elemental potions
--------------------------------------------------------------------------------

-- items[2001] = {}

--------------------------------------------------------------------------------
-- IDs 3001-4000 are reserved for damaging items
--------------------------------------------------------------------------------

items[3001] = {
	name = hoa_system.Translate("Bomb"),
	description = hoa_system.Translate("Damages one enemy."),
	icon = "img/icons/items/scroll_red.png",
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_FOE,
	standard_price = 200,

	BattleUse = function(user, target)
		target_actor = target:GetActor();
		target_actor:RegisterDamage(256);
		AudioManager:PlaySound("snd/rumble.wav");
	end,

	FieldUse = function(target)
		AudioManager:PlaySound("snd/cancel.wav");
	end
}

items[3002] = {
	name = hoa_system.Translate("Super Bomb"),
	description = hoa_system.Translate("Major damage to all enemies."),
	icon = "img/icons/items/scroll_red.png",
	target_type = hoa_global.GameGlobal.GLOBAL_TARGET_ALL_FOES,
	standard_price = 1000,

	BattleUse = function(user, target)
		-- TODO: no support yet for party targets
		target_party = target:GetParty();
		-- TODO: iterate through each living actor in the party and add damage
		target_actor:RegisterDamage(500);
		AudioManager:PlaySound("snd/rumble.wav");
	end,

	FieldUse = function(target)
		AudioManager:PlaySound("snd/cancel.wav");
	end
}
