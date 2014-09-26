------------------------------------------------------------------------------[[
-- Filename: menus.lua
--
-- Description: Tests for the MenuMode interface. These tests are used to make sure
-- that all menus display correctly. Tests are also used to check that any items or
-- skills that are usable in the field are executed successfully from within MenuMode.
--
-- Note: Please try to keep these tests organized from the most simple to the most
-- complex. Generally, adding more characters, equipment, and skills contribute
-- to making MenuMode more complex as it has more data it needs to organize and process.
------------------------------------------------------------------------------]]

local ns = {}
setmetatable(ns, {__index = _G})
menus = ns;
setfenv(1, ns);

-- Test IDs 2,001 - 3,000 are reserved for menus
tests = {}

tests[2001] = {
	name = "Basic Early Game Menu";
	description = "Party consists of a single character with a minimal amount of equipment. This is the type of menu " ..
		"we would expect to see frequently in the early stages of the game.";
	ExecuteTest = function()
		GlobalManager:AddCharacter(1); -- Claudius
		GlobalManager:AddDrunes(100);
		GlobalManager:AddToInventory(1, 5);
		GlobalManager:AddToInventory(1001, 2);
		GlobalManager:AddToInventory(10001, 1);

		local menu = hoa_menu.MenuMode();
		ModeManager:Push(menu); 
	end
}

