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
weapons = {}


-- -----------------------------------------------------------------------------
-- IDs 10001-10500 are reserved for swords
-- -----------------------------------------------------------------------------

weapons[10001] = {
	name = "Karlate Sword",
	description = "Standard Karlate issued equipment. A light weight iron sword suitable for most skirmishes.",
	icon = "img/icons/weapons/karlate_sword.png",
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	physical_attack = 12,
	metaphysical_attack = 0,
	standard_price = 120,
	slots = 0
}
