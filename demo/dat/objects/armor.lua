------------------------------------------------------------------------------[[
-- Filename: armor.lua
--
-- Description: This file contains the definitions of all armors that exist in
-- Hero of Allacrost. Each armor has a unique integer identifier that is used
-- as its key in the armors table below.
--
-- Note (1): Armors ids do *not* need to be sequential. When you make a new 
-- armor, keep it grouped with similar weapon types (helments with helmets, etc.)
-- and keep some space between groups. This way, we won't get a garbled mess of
-- weapons.
--
-- Note (2): Valid ids for armors are 20001-60000. Do not go out of bounds with
-- this limit, as other value ranges correspond to other types of objects
-- (items, weapons, etc.)
------------------------------------------------------------------------------]]

-- All armor definitions are stored in this table
armor = {}


-- -----------------------------------------------------------------------------
-- IDs 20001-30000 are reserved for head armor
-- -----------------------------------------------------------------------------

armor[20001] = {
	name = "Karlate Helmet",
	description = "Standard Karlate issued equipment",
	icon = "img/icons/armor/karlate_helmet.png",
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	physical_defense = 6,
	metaphysical_defense = 2,
	standard_price = 80,
	slots = 0
}


-- -----------------------------------------------------------------------------
-- IDs 30001-40000 are reserved for torso armor
-- -----------------------------------------------------------------------------

armor[30001] = {
	name = "Karlate Breastplate",
	description = "Standard Karlate issued equipment",
	icon = "img/icons/armor/karlate_breastplate.png",
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	physical_defense = 8,
	metaphysical_defense = 0,
	standard_price = 170,
	slots = 0
}


-- -----------------------------------------------------------------------------
-- IDs 40001-50000 are reserved for arm armor
-- -----------------------------------------------------------------------------

armor[40001] = {
	name = "Karlate Shield",
	description = "Standard Karlate issued equipment",
	icon = "img/icons/armor/karlate_shield.png",
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	physical_defense = 2,
	metaphysical_defense = 0,
	standard_price = 40,
	slots = 0
}


-- -----------------------------------------------------------------------------
-- IDs 50001-60000 are reserved for leg armor
-- -----------------------------------------------------------------------------

armor[50001] = {
	name = "Karlate Greaves",
	description = "Standard Karlate issued equipment",
	icon = "img/icons/armor/karlate_greaves.png",
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	physical_defense = 3,
	metaphysical_defense = 0,
	standard_price = 25,
	slots = 0
}
