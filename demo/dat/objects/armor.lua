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
	description = "Standard Karlate issued equipment. Battle worn but still reliable head protection.",
	icon = "img/icons/armor/karlate_helmet.png",
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	physical_defense = 6,
	metaphysical_defense = 2,
	standard_price = 80,
	slots = 0
}

armor[20002] = {
	name = "Cobalt Helm",
	description = "A small helmet composed of a secret colbalt based alloy.",
	icon = "img/icons/armor/cobalt_helm.png",
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	physical_defense = 8,
	metaphysical_defense = 4,
	standard_price = 140,
	slots = 0
}


-- -----------------------------------------------------------------------------
-- IDs 30001-40000 are reserved for torso armor
-- -----------------------------------------------------------------------------

armor[30001] = {
	name = "Karlate Breastplate",
	description = "Standard Karlate issued equipment. Effectively protects the torso from most types of attack.",
	icon = "img/icons/armor/karlate_breastplate.png",
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	physical_defense = 8,
	metaphysical_defense = 2,
	standard_price = 170,
	slots = 0
}

armor[30002] = {
	name = "Leather Chain Mail",
	description = "A light chain mail woven into a tough, leather cuirass.",
	icon = "img/icons/armor/leather_chain_mail.png",
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	physical_defense = 14,
	metaphysical_defense = 3,
	standard_price = 320,
	slots = 0
}


-- -----------------------------------------------------------------------------
-- IDs 40001-50000 are reserved for arm armor
-- -----------------------------------------------------------------------------

armor[40001] = {
	name = "Karlate Shield",
	description = "Standard Karlate issued equipment. Strong wooden oak protects from all but the heaviest of assaults.",
	icon = "img/icons/armor/karlate_shield.png",
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	physical_defense = 2,
	metaphysical_defense = 0,
	standard_price = 90,
	slots = 0
}

armor[40002] = {
	name = "Phoenix Shield",
	description = "A tall steel shield with a mighty phoenix embroidered on the front.",
	icon = "img/icons/armor/phoenix_shield.png",
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	physical_defense = 6,
	metaphysical_defense = 6,
	standard_price = 150,
	slots = 0
}


-- -----------------------------------------------------------------------------
-- IDs 50001-60000 are reserved for leg armor
-- -----------------------------------------------------------------------------

armor[50001] = {
	name = "Karlate Greaves",
	description = "Standard Karlate issued equipment. Light metal alloy protects the legs while minimizing the negative impact on movement.",
	icon = "img/icons/armor/karlate_greaves.png",
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	physical_defense = 3,
	metaphysical_defense = 0,
	standard_price = 120,
	slots = 0
}
