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
	-- Multiplies Strength by 3
	
	Init = function(thisEffect, target)
		thisEffect:SetStrModifier(3.0);
	end,

	Update = function(thisEffect, target)
	end,

	Remove = function(thisEffect, target)
	end
}

status_effects[2] = {
	name = "Defense Up",

	-- Multiplies Fortitude by 3
	Init = function(thisEffect, target)
		thisEffect:SetForModifier(3.0);
	end,

	Update = function(thisEffect, target)
	end,

	Remove = function(thisEffect, target)
	end
}

status_effects[3] = {
	name = "Stun",
	
	Init = function(thisEffect, target)
		thisEffect:SetStunEffect(true);
		thisEffect:SetDuration(10000);
		thisEffect:StartTimer();
	end,

	Update = function(thisEffect, target)
	end,

	Remove = function(thisEffect, target)
	end
}

status_effects[4] = {
	name = "Hiding",
	
	Init = function(thisEffect, target)
		thisEffect:SetEvaModifier(5.0);
	end,

	Update = function(thisEffect, target)
	end,

	Remove = function(thisEffect, target)
	end
}
