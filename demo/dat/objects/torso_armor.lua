------------------------------------------------------------------------------[[
-- Filename: torso_armor.lua
--
-- Description: This file contains the definitions of all torso armors that exist
-- in Hero of Allacrost. Each armor has a unique integer identifier that is used
-- as its key in the armor table below.
--
-- Note (1): Armors ids do *not* need to be sequential. When you make a new 
-- armor, keep it grouped with similar torso armor types (tunics, plated mails, 
-- etc.) and keep some space between groups. This way, we won't get a garbled
-- mess of torso armor definitions.
--
-- Note (2): Valid ids for head armors are 30001-40000. Do not go out of bounds
-- with this limit, as other value ranges correspond to other types of objects
-- (items, weapons, etc.)
------------------------------------------------------------------------------]]

-- All armor definitions are stored in this table
armor = {}

armor[30001] = {
	name = "Karlate Breastplate",
	description = "Standard Karlate issued equipment. Effectively protects the torso from most types of attack.",
	icon = "img/icons/armor/karlate_breastplate.png",
	physical_defense = 8,
	metaphysical_defense = 2,
	standard_price = 170,
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	slots = 0
}

armor[30002] = {
	name = "Leather Chain Mail",
	description = "A light chain mail woven into a tough, leather cuirass.",
	icon = "img/icons/armor/leather_chain_mail.png",
	physical_defense = 14,
	metaphysical_defense = 3,
	standard_price = 320,
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	slots = 0
}
