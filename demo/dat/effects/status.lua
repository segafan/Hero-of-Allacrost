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
	name = "Dummy effect",
	
	Init = function(thisEffect, target)
	end,

	Update = function(thisEffect, target)
	end,

	Remove = function(thisEffect, target)
	end
}

status_effects[2] = {
	name = "Defense Up",

	Init = function(thisEffect, target)
		thisEffect:SetForModifier(0.25); -- Increase Physical Defense by 25%
		thisEffect:SetDuration(30000); -- This is about three "turns"
		thisEffect:StartTimer();
	end,

	Update = function(thisEffect, target)
	end,

	Remove = function(thisEffect, target)
	end
}

status_effects[3] = {
	name = "Stun",
	-- Stop an actor's timer for a brief period
	
	Init = function(thisEffect, target)
		thisEffect:SetStunEffect(true);
		thisEffect:SetDuration(10000); -- This is about one "turn"
		thisEffect:StartTimer();
	end,

	Update = function(thisEffect, target)
	end,

	Remove = function(thisEffect, target)
	end
}

status_effects[4] = {
	name = "Dodge Enemies",

	Init = function(thisEffect, target)
		thisEffect:SetEvaModifier(9.0); -- Increase evade by 900%
		thisEffect:SetDuration(20000); -- This is about two "turns"
		thisEffect:StartTimer();
	end,

	Update = function(thisEffect, target)
	end,

	Remove = function(thisEffect, target)
	end
}
