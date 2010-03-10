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


status_effects[hoa_global.GameGlobal.GLOBAL_STATUS_FORTITUDE_BOOST] = {
	name = hoa_system.Translate("Defense Up"),

	Apply = function(effect)
		timer = effect:GetTimer();
		actor = effect:GetAffectedActor();
		intensity = effect:GetIntensity();

		actor:ResetFortitude();
		base_fortitude = actor:GetFortitude();
		if (intensity == hoa_global.GameGlobal.GLOBAL_INTENSITY_NEUTRAL) then
			-- Fortitude was already reset, no further action needed
		elseif (intensity == hoa_global.GameGlobal.GLOBAL_INTENSITY_POS_LESSER) then
			actor:SetFortitude(base_fortitude * 2); -- Fortitude 2x
		elseif (intensity == hoa_global.GameGlobal.GLOBAL_INTENSITY_POS_MODERATE) then
			actor:SetFortitude(base_fortitude * 3); -- Fortitude 3x
		elseif (intensity == hoa_global.GameGlobal.GLOBAL_INTENSITY_POS_GREATER) then
			actor:SetFortitude(base_fortitude * 4); -- Fortitude 4x
		elseif (intensity == hoa_global.GameGlobal.GLOBAL_INTENSITY_POS_EXTREME) then
			actor:SetFortitude(base_fortitude * 5); -- Fortitude 5x
		else
			-- TODO: print some sort of error message?
		end
		timer:SetDuration(30000); -- 30 seconds
	end
}





status_effects[100] = {
	name = hoa_system.Translate("Dummy effect"),
	
	Init = function(thisEffect, target)
	end,

	Update = function(thisEffect, target)
	end,

	Remove = function(thisEffect, target)
	end
}

status_effects[200] = {
	name = hoa_system.Translate("Defense Up"),

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

status_effects[300] = {
	name = hoa_system.Translate("Stun"),
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

status_effects[400] = {
	name = hoa_system.Translate("Dodge Enemies"),

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
