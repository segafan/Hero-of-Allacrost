------------------------------------------------------------------------------[[
-- Filename: status.lua
--
-- Description: This file contains the implementations of all status effects in
-- Hero of Allacrost. The list of different types of status effects and their
-- corresponding names may be found in src/modes/common/global/global_effects.*.
--
-- Each status effect implementation requires the following data to be defined.
-- {name} - The name of the status effect as it will be shown to the player
-- {icon_index} - A numeric index to the row of images where the icons for this effect
-- {opposite_status} - The status which acts as an opposite status to this one
-- {Apply} - A function executed when the status effect is applied to the target
-- {Update} - A function executed periodically while the status is still in effect
-- {Remove} - A function executed when the status effect is no longer active on the target
--
-- To verify what a status effect's icon_index should be, examine the image file 
-- img/icons/effects/status.png and find the appropriate row of icons.
-- 
-- The opposite_status property is only applicable to some pairs of status effects.
-- For example, a pair of effects that increase strength and decrease strength. Each status
-- effect can have only one opposite effect, not several. If the status effect has no opposite
-- effect, this member should be set to hoa_global.GameGlobal.GLOBAL_STATUS_INVALID.
--
-- The Apply, Update, and Remove functions are all called with an argument, {effect},
-- which is a pointer to the BattleStatusEffect object that was constructed to represent
-- this status. Use this object to access data relevant to the status effect. Most 
-- implementations of these functions will want to grab pointers to the following pieces of
-- data.
--
-- effect:GetTimer() - returns the BattleTimer object for the effect
-- effect:GetAffectedActor() - returns the BattleActor object that the effect is active on
-- effect:GetIntensity() - returns the current intensity of the active effect
-- effect:IsIntensityChanged() - returns true if the intensity level has undergone a recent change
--
-- NOTE: Unlike elemental effects, status effects only ever have intensity levels that are
-- neutral or in the positive range of values, never the negative range. You should not concern
-- yourself with negative intensity values in this file.
------------------------------------------------------------------------------]]

-- All item definitions are stored in this table
if (status_effects == nil) then
   status_effects = {}
end


status_effects[hoa_global.GameGlobal.GLOBAL_STATUS_FORTITUDE_BOOST] = {
	name = hoa_system.Translate("Raise Fortitude"),
	icon_index = 6,
	opposite_effect = hoa_global.GameGlobal.GLOBAL_STATUS_INVALID,

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
	end,

	Update = function(effect)
		if (effect:IsIntensityChanged() == true) then
			
		end
	end,

	Remove = function(effect)
		actor = effect:GetAffectedActor();
		actor:ResetFortitude();
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

