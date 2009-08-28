------------------------------------------------------------------------------[[
-- Filename: battle_events.lua
--
-- Description: This file contains the definitions of all scripted events in
-- Hero of Allacrost battles, generally to be used for boss battles.
------------------------------------------------------------------------------]]

-- All item definitions are stored in this table
if (battle_events == nil) then
	battle_events = {}
end 

battle_events[1] = {
	name = "Duel with Kyle",
	--TODO: Add dialogue!

	Before = function(bat_mode)
	end,

	During = function(bat_mode)
	end,

	After = function(bat_mode)
	end
}
