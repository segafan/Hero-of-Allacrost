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

armor[50001] = {
	name = "Karlate Greaves",
	description = "Standard Karlate issued equipment. Light metal alloy protects the legs while minimizing the negative impact on movement.",
	icon = "img/icons/armor/karlate_greaves.png",
	physical_defense = 3,
	metaphysical_defense = 0,
	standard_price = 120,
	usable_by = hoa_global.GameGlobal.GLOBAL_CHARACTER_CLAUDIUS,
	slots = 0
}
