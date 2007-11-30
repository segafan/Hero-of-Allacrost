------------------------------------------------------------------------------[[
-- Filename: weapons.lua
--
-- Description: This file contains the definitions of all weapons that exist in
-- Hero of Allacrost. Each weapon has a unique integer identifier that is used
-- as its key in the weapons table below.
--
-- Note (1): Weapons ids do *not* need to be sequential. When you make a new 
-- weapon, keep it grouped with similar weapon types (swords with swords, etc.)
-- and keep some space between groups. This way, we won't get a garbled mess of
-- weapons.
--
-- Note (2): Valid ids for items are 10001-20000. Do not break this limit, because
-- other value ranges correspond to other types of objects (items, armor, etc.)
------------------------------------------------------------------------------]]

-- All weapon definitions are stored in this table
if (weapons == nil) then
   weapons = {}
end


-- -----------------------------------------------------------------------------
-- IDs 10001-10500 are reserved for swords
-- -----------------------------------------------------------------------------

weapons[10001] = {
	name = "Karlate Sword",
	description = "Standard Karlate issued equipment. A light weight iron sword suitable for most skirmishes.",
	icon = "img/icons/weapons/karlate_sword.png",
	physical_attack = 12,
	metaphysical_attack = 0,
	standard_price = 120,
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	slots = 0
}

weapons[10002] = {
	name = "Iron Sword",
	description = "A sturdy but somewhat dull sword forged from a single block of solid iron.",
	icon = "img/icons/weapons/iron_sword.png",
	physical_attack = 16,
	metaphysical_attack = 0,
	standard_price = 250,
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	slots = 0
}

-- -----------------------------------------------------------------------------
-- IDs 10501-11000 are reserved for crossbows
-- -----------------------------------------------------------------------------

weapons[10501] = {
	name = "Quick Crossbow",
	description = "A lightweight but effective crossbow.",
	icon = "img/icons/weapons/quick-crossbow.png",
	physical_attack = 9,
	metaphysical_attack = 0,
	standard_price = 100,
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_LAILA,
	slots = 0
}

weapons[10502] = {
	name = "Standard Crossbow",
	description = "Standard Karlate issued equipment. A crossbow designed for accuraccy and ease of use.",
	icon = "img/icons/weapons/standard-crossbow.png",
	physical_attack = 13,
	metaphysical_attack = 0,
	standard_price = 140,
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_LAILA,
	slots = 0
}

weapons[10503] = {
	name = "Reinforced Crossbow",
	description = "An improved version of the standard Karlate crossbow.",
	icon = "img/icons/weapons/reinforced-crossbow.png",
	physical_attack = 17,
	metaphysical_attack = 0,
	standard_price = 290,
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_LAILA,
	slots = 0
}

weapons[10504] = {
	name = "Arbalest",
	description = "Heavy crossbow designed for battle with large monsters.",
	icon = "img/icons/weapons/arbalest.png",
	physical_attack = 21,
	metaphysical_attack = 0,
	standard_price = 440,
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_LAILA,
	slots = 0
}
