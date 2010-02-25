------------------------------------------------------------------------------[[
-- Filename: leg_armor.lua
--
-- Description: This file contains the definitions of all leg armors that exist
-- in Hero of Allacrost. Each armor has a unique integer identifier that is used
-- as its key in the armor table below.
--
-- Note (1): Armors ids do *not* need to be sequential. When you make a new 
-- armor, keep it grouped with similar leg armor types (greaves, leggings,
-- etc.) and keep some space between groups. This way, we won't get a garbled
-- mess of leg armor definitions.
--
-- Note (2): Valid ids for head armors are 50001-60000. Do not go out of bounds
-- with this limit, as other value ranges correspond to other types of objects
-- (items, weapons, etc.)
------------------------------------------------------------------------------]]

-- All armor definitions are stored in this table
if (armor == nil) then
   armor = {}
end

-- -----------------------------------------------------------------------------
-- IDs 50001-50500 are reserved for greaves
-- -----------------------------------------------------------------------------

armor[50001] = {
	name = hoa_system.Translate("Karlate Greaves"),
	description = hoa_system.Translate("Standard Karlate issued equipment. Light metal alloy protects the legs while minimizing the negative impact on movement."),
	icon = "img/icons/armor/karlate_greaves.png",
	physical_defense = 3,
	metaphysical_defense = 0,
	standard_price = 120,
	usable_by = CLAUDIUS,
	slots = 0
}

-- -----------------------------------------------------------------------------
-- IDs 50501-51000 are reserved for light footwear
-- -----------------------------------------------------------------------------

armor[50501] = {
	name = hoa_system.Translate("Leather Sandals"),
	description = hoa_system.Translate("Light footwear that while fashionable, was not designed for battle and affords very poor protection for the user."),
	icon = "img/icons/armor/leather_sandals.png",
	physical_defense = 1,
	metaphysical_defense = 1,
	standard_price = 80,
	usable_by = LAILA,
	slots = 0
}

armor[50502] = {
	name = hoa_system.Translate("Leather Boots"),
	description = hoa_system.Translate("Comfortable leather that protects the entire foot and lower region of the shin."),
	icon = "img/icons/armor/leather_boots.png",
	physical_defense = 3,
	metaphysical_defense = 1,
	standard_price = 120,
	usable_by = LAILA,
	slots = 0
}

