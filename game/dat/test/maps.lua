------------------------------------------------------------------------------[[
-- Filename: maps.lua
--
-- Description: This file contains all tests that create an instance of MapMode
-- with various configurations. The tests here are primarily for testing map files
-- and scripting.
------------------------------------------------------------------------------]]

local ns = {}
setmetatable(ns, {__index = _G})
maps = ns;
setfenv(1, ns);

-- Test IDs 1 - 1,000 are reserved for maps
tests = {}

tests[1] = {
	name = "First Cave Dungeon Map";
	description = "Places the user in the first dungeon, effectively skipping over the introduction of the game";
	ExecuteTest = function()
		-- Make sure that any global data is cleared away
		GlobalManager:ClearAllData();

		-- Create the initial party, drunes, and inventory
		GlobalManager:AddCharacter(LUKAR);
		GlobalManager:AddCharacter(DESTER);
		GlobalManager:AddCharacter(MARK);
		GlobalManager:AddCharacter(CLAUDIUS);
		GlobalManager:AddNewEventGroup("global_events"); -- this group stores the primary list of events completed in the game
		GlobalManager:SetDrunes(100);
		GlobalManager:AddToInventory(1, 4);

		-- Set the location, load the opening map and add it to the game stack, and remove boot mode from the game stack
		local location_name = "dat/maps/river_access_cave.lua"
		GlobalManager:SetLocation(location_name);
		local opening_map = hoa_map.MapMode(location_name);

		ModeManager:Push(opening_map);
	end
}

