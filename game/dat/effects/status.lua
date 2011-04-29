------------------------------------------------------------------------------[[
-- Filename: status.lua
--
-- Description: This file contains the implementations of all status effects in
-- Hero of Allacrost. The list of different types of status effects and their
-- corresponding names may be found in src/modes/common/global/global_effects.*.
--
-- Each status effect implementation requires the following data to be defined.
-- {name} - The name of the status effect as it will be shown to the player
-- {duration} - The duration that the effect lasts, in milliseconds
-- {icon_index} - A numeric index to the row of images where the icons for this effect
-- {opposite_effect} - The status which acts as an opposite status to this one
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
	duration = 30000,
	icon_index = 6,
	-- TODO: eventually this will have an opposite effect. Change the line below later when that effect is available.
	opposite_effect = hoa_global.GameGlobal.GLOBAL_STATUS_INVALID, 

	Apply = function(effect)
		status_effects[hoa_global.GameGlobal.GLOBAL_STATUS_FORTITUDE_BOOST].ModifyAttribute(effect);
	end,

	Update = function(effect)
		if (effect:IsIntensityChanged() == true) then
			status_effects[hoa_global.GameGlobal.GLOBAL_STATUS_FORTITUDE_BOOST].ModifyAttribute(effect);
		end
	end,

	Remove = function(effect)
		effect:GetAffectedActor():ResetFortitude();
	end,

	ModifyAttribute = function(effect)
		actor = effect:GetAffectedActor();
		intensity = effect:GetIntensity();

		actor:ResetFortitude();
		base_fortitude = actor:GetFortitude();

		if (intensity == hoa_global.GameGlobal.GLOBAL_INTENSITY_NEUTRAL) then
			-- Fortitude was already reset. No further action needed
		elseif (intensity == hoa_global.GameGlobal.GLOBAL_INTENSITY_POS_LESSER) then
			actor:SetFortitude(base_fortitude * 2); -- Fortitude 2x
		elseif (intensity == hoa_global.GameGlobal.GLOBAL_INTENSITY_POS_MODERATE) then
			actor:SetFortitude(base_fortitude * 3); -- Fortitude 3x
		elseif (intensity == hoa_global.GameGlobal.GLOBAL_INTENSITY_POS_GREATER) then
			actor:SetFortitude(base_fortitude * 4); -- Fortitude 4x
		elseif (intensity == hoa_global.GameGlobal.GLOBAL_INTENSITY_POS_EXTREME) then
			actor:SetFortitude(base_fortitude * 5); -- Fortitude 5x
		else
			print("Lua warning: status effect had an invalid intensity value: " .. intensity);
		end
	end,
}


status_effects[hoa_global.GameGlobal.GLOBAL_STATUS_PARALYSIS] = {
	name = hoa_system.Translate("Paralysis"),
	duration = 10000,
	icon_index = 8,
	opposite_effect = hoa_global.GameGlobal.GLOBAL_STATUS_INVALID,
	
	Apply = function(effect)
		-- TODO: apply paralysis state to actor
	end,

	Update = function(effect)
		-- Nothing needs to be updated for this effect
	end,

	Remove = function(effect)
		-- TODO: restore actor to active state
	end,
}

