------------------------------------------------------------------------------[[
-- Filename: maps.lua
--
-- Description: This file contains all tests that create an instance of MapMode.
-- The tests here are primarily for testing map files and their scripted code.
-- Generally it is a good idea to create a test for each map file, or possibly 
-- multiple tests for a file so that it is easy to get to a certain point in the
-- just before an important event or sequence, such as a boss encounter.
--
-- Note: It is strongly advised to state the name of the map that will be opened
-- in the test name. This makes the life of the user much easier. Recommended format:
-- "[Name Of Map] - Configuration Details".
------------------------------------------------------------------------------]]

local ns = {}
setmetatable(ns, {__index = _G})
maps = ns;
setfenv(1, ns);

-- Character IDs. Each ID can have only a single bit active as IDs are used in bitmask operations.
CLAUDIUS  = 1;
MARK      = 2;
DESTER    = 4;
LUKAR     = 8;

--------------------------------------------------------------------------------
-- Common Functions
--------------------------------------------------------------------------------

-- Sets the map location and creates and pushes the MapMode instance
function StartMap(location)
	GlobalManager:SetLocation(location);
	local map = hoa_map.MapMode(location);
	ModeManager:Push(map);
end

-- Test IDs 1 - 1,000 are reserved for maps
tests = {}

tests[1] = {
	name = "[River Access Cave] - New Game Data";
	description = "Places the user in the first dungeon with the party and status that the player receives upon beginning a new game. " ..
		"Effectively, this test is a way to start a new game and skip over the intro sequence seen on the opening map.";
	ExecuteTest = function()
		-- Note: These calls should match what is found in the NewGame() function in dat/global.lua. Update this test accordingly 
		-- if the contents of that function change

		GlobalManager:AddCharacter(LUKAR);
		GlobalManager:AddCharacter(DESTER);
		GlobalManager:AddCharacter(MARK);
		GlobalManager:AddCharacter(CLAUDIUS);
		GlobalManager:AddNewEventGroup("global_events");
		GlobalManager:SetDrunes(100);
		GlobalManager:AddToInventory(1, 4);

		StartMap("dat/maps/river_access_cave.lua");
	end
}

-- tests[2]: Reserved for a test of the River Access Cave that spawns the player just before the boss battle at the end of the dungeon


tests[3] = {
	name = "[Opening Scene] - Return to City";
	description = "Places the user in the first dungeon with the party and status that the player receives upon beginning a new game. " ..
		"Effectively, this test is a way to start a new game and skip over the intro sequence seen on the opening map.";
	ExecuteTest = function()

		GlobalManager:AddCharacter(LUKAR);
		GlobalManager:AddCharacter(DESTER);
		GlobalManager:AddCharacter(MARK);
		GlobalManager:AddCharacter(CLAUDIUS);
		GlobalManager:AddNewEventGroup("global_events");
		GlobalManager:SetDrunes(200);
		GlobalManager:AddToInventory(1, 5);

		-- This is what the map uses to determine whether to begin the first scene (opening) or the second scene (return)
		GlobalManager:AddNewEventGroup("dat_maps_river_access_cave_lua");

		StartMap("dat/maps/opening_scene.lua");
	end
}

tests[4] = {
	name = "[Harrvah City] - First Visit, City Under Attack";
	description = "Places the player at the entrance to Harrvah for the first visit to the city. This is when Claudius is " ..
		"joined by other knights returning from their mission to the cave, where they find the city under attack by the " ..
		"demonic forces.";

	ExecuteTest = function()
		GlobalManager:AddCharacter(LUKAR);
		GlobalManager:AddCharacter(DESTER);
		GlobalManager:AddCharacter(MARK);
		GlobalManager:AddCharacter(CLAUDIUS);
		GlobalManager:AddNewEventGroup("global_events");
		GlobalManager:SetDrunes(200);
		GlobalManager:AddToInventory(1, 5);

		StartMap("dat/maps/harrvah_city.lua");
	end
}
