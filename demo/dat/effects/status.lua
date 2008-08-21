------------------------------------------------------------------------------[[
-- Filename: status.lua
--
-- Description: This file contains the definitions of all status effects in
-- Hero of Allacrost. Each item has a unique integer identifier that is used
-- as its key in the items table below.
------------------------------------------------------------------------------]]

-- All item definitions are stored in this table
if (status_effects == nil) then
   status_effects = {}
end


status_effects[1] = {
	name = "Strength Up",
	
	Init = function(target)
		target:AddStrength(400);
	end,

	Update = function(target)
	end,

	Remove = function(target)
		target:AddStrength(-400);
	end
}

status_effects[2] = {
	name = "Defense Up",
	
	Init = function(target)
		target:AddFortitude(400);
	end,

	Update = function(target)
	end,

	Remove = function(target)
		target:AddFortitude(-400);
	end
}