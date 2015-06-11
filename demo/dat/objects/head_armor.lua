------------------------------------------------------------------------------[[
-- Filename: head_armor.lua
--
-- Description: This file contains the definitions of all head armors that exist
-- in Hero of Allacrost. Each armor has a unique integer identifier that is used
-- as its key in the armor table below.
--
-- Note (1): Armors ids do *not* need to be sequential. When you make a new 
-- armor, keep it grouped with similar head armor types (helmets, caps, etc.)
-- and keep some space between groups. This way, we won't get a garbled mess of
-- head armor definitions.
--
-- Note (2): Valid ids for head armors are 20001-30000. Do not go out of bounds
-- with this limit, as other value ranges correspond to other types of objects
-- (items, weapons, etc.)
------------------------------------------------------------------------------]]

-- All armor definitions are stored in this table
if (armor == nil) then
   armor = {}
end

-- -----------------------------------------------------------------------------
-- IDs 20001-20500 are reserved for helmets
-- -----------------------------------------------------------------------------

armor[20001] = {
	name = hoa_system.Translate("Karlate Helmet"),
	description = hoa_system.Translate("Standard Karlate issued equipment. Battle worn but still reliable head protection."),
	icon = "img/icons/armor/karlate_helmet.png",
	physical_defense = 6,
	metaphysical_defense = 2,
	standard_price = 80,
	usable_by = CLAUDIUS,
	slots = 0
}

armor[20002] = {
	name = hoa_system.Translate("Cobalt Helm"),
	description = hoa_system.Translate("A small helmet composed of a secret cobalt based alloy."),
	icon = "img/icons/armor/cobalt_helm.png",
	physical_defense = 8,
	metaphysical_defense = 4,
	standard_price = 140,
	usable_by = CLAUDIUS,
	slots = 0
}

-- -----------------------------------------------------------------------------
-- IDs 20501-21000 are reserved for head dresses
-- -----------------------------------------------------------------------------

armor[20501] = {
	name = hoa_system.Translate("Butterfly Pins"),
	description = hoa_system.Translate("A feminine head ornament that offers little effective protection."),
	icon = "img/icons/armor/butterfly_pins.png",
	physical_defense = 2,
	metaphysical_defense = 4,
	standard_price = 60,
	usable_by = LAILA,
	slots = 0
}

armor[20502] = {
	name = hoa_system.Translate("Winged Circlet"),
	description = hoa_system.Translate("A circlet imbued with a small magical presence, which wards off evil from its bearer."),
	icon = "img/icons/armor/winged_circlet.png",
	physical_defense = 5,
	metaphysical_defense = 10,
	standard_price = 140,
	usable_by = LAILA,
	slots = 0
}
