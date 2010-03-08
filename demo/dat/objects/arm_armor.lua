------------------------------------------------------------------------------[[
-- Filename: arm_armor.lua
--
-- Description: This file contains the definitions of all head armors that exist
-- in Hero of Allacrost. Each armor has a unique integer identifier that is used
-- as its key in the armor table below.
--
-- Note (1): Armors ids do *not* need to be sequential. When you make a new 
-- armor, keep it grouped with similar arm armor types (shields, gauntlets,
-- etc.) and keep some space between groups. This way, we won't get a garbled
-- mess of arm armor definitions.
--
-- Note (2): Valid ids for head armors are 40001-50000. Do not go out of bounds
-- with this limit, as other value ranges correspond to other types of objects
-- (items, weapons, etc.)
------------------------------------------------------------------------------]]

-- All armor definitions are stored in this table
if (armor == nil) then
   armor = {}
end

-- -----------------------------------------------------------------------------
-- IDs 40001-40500 are reserved for shields
-- -----------------------------------------------------------------------------

armor[40001] = {
	name = hoa_system.Translate("Karlate Shield"),
	description = hoa_system.Translate("Standard Karlate issued equipment. Strong wooden oak protects from all but the heaviest of assaults."),
	icon = "img/icons/armor/karlate_shield.png",
	physical_defense = 2,
	metaphysical_defense = 0,
	standard_price = 90,
	usable_by = CLAUDIUS,
	slots = 0
}

armor[40002] = {
	name = hoa_system.Translate("Phoenix Shield"),
	description = hoa_system.Translate("A tall steel shield with a mighty phoenix emblazoned on the front."),
	icon = "img/icons/armor/phoenix_shield.png",
	physical_defense = 6,
	metaphysical_defense = 6,
	standard_price = 150,
	usable_by = CLAUDIUS,
	slots = 0
}

-- -----------------------------------------------------------------------------
-- IDs 40501-41000 are reserved for bracelets
-- -----------------------------------------------------------------------------

armor[40501] = {
	name = hoa_system.Translate("Stone Bracelet"),
	description = hoa_system.Translate("A bracelet crafted out of soft limestone."),
	icon = "img/icons/armor/stone_bracelet.png",
	physical_defense = 4,
	metaphysical_defense = 1,
	standard_price = 130,
	usable_by = LAILA,
	slots = 0
}

