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
	name = hoa_system.Translate("Karlate Sword"),
	description = hoa_system.Translate("Standard Karlate issued equipment. A light weight iron sword suitable for most skirmishes."),
	icon = "img/icons/weapons/karlate_sword.png",
	physical_attack = 12,
	metaphysical_attack = 0,
	standard_price = 120,
	usable_by = CLAUDIUS + KYLE,
	slots = 0
}

weapons[10002] = {
	name = hoa_system.Translate("Iron Sword"),
	description = hoa_system.Translate("A sturdy but somewhat dull sword forged from a single block of solid iron."),
	icon = "img/icons/weapons/iron_sword.png",
	physical_attack = 16,
	metaphysical_attack = 0,
	standard_price = 250,
	usable_by = CLAUDIUS + KYLE,
	slots = 0
}

-- -----------------------------------------------------------------------------
-- IDs 10501-11000 are reserved for crossbows
-- -----------------------------------------------------------------------------

weapons[10501] = {
	name = hoa_system.Translate("Quick Crossbow"),
	description = hoa_system.Translate("A lightweight but effective crossbow."),
	icon = "img/icons/weapons/quick_crossbow.png",
	physical_attack = 9,
	metaphysical_attack = 0,
	standard_price = 100,
	usable_by = LAILA,
	slots = 0
}

weapons[10502] = {
	name = hoa_system.Translate("Standard Crossbow"),
	description = hoa_system.Translate("Standard issued equipment for archers. A crossbow designed for accuracy and ease of use."),
	icon = "img/icons/weapons/standard_crossbow.png",
	physical_attack = 13,
	metaphysical_attack = 0,
	standard_price = 140,
	usable_by = LAILA,
	slots = 0
}

weapons[10503] = {
	name = hoa_system.Translate("Reinforced Crossbow"),
	description = hoa_system.Translate("An improved version of the standard crossbow that mitigates recoil."),
	icon = "img/icons/weapons/reinforced_crossbow.png",
	physical_attack = 17,
	metaphysical_attack = 0,
	standard_price = 290,
	usable_by = LAILA,
	slots = 0
}

weapons[10504] = {
	name = hoa_system.Translate("Arbalest"),
	description = hoa_system.Translate("Heavy crossbow designed for piercing through thick armor."),
	icon = "img/icons/weapons/arbalest.png",
	physical_attack = 21,
	metaphysical_attack = 0,
	standard_price = 440,
	usable_by = LAILA,
	slots = 0
}
