------------------------------------------------------------------------------[[
-- Filename: battle.lua
--
-- Description: This file contains all tests that create an instance of BattleMode
-- with various configurations. The tests here are primarily for testing different
-- enemy parties and character strengths.
--
-- Note: To keep the tests in this file organized, reserve the first 100 tests (3001-3100)
-- for tests intended to test BattleMode itself. Tests primarily intended for balancing
-- should follow after this set. Try to keep tests sorted in order from more basic to more
-- advanced, or from early-game to late-game battles.
------------------------------------------------------------------------------]]

local ns = {}
setmetatable(ns, {__index = _G})
battles = ns;
setfenv(1, ns);

-- Test IDs 1,001 - 2,000 are reserved for battles
tests = {}

-- Begin tests intended for BattleMode interface and features

tests[1001] = {
	name = "Single-Character Early Game Battle";
	description = "Party consists of a single character versus two early game enemies. A handful of potions are available " ..
		"in the party inventory as well.";
	ExecuteTest = function()
		GlobalManager:SetBattleSetting(hoa_global.GameGlobal.GLOBAL_BATTLE_WAIT);
		GlobalManager:AddCharacter(1); -- Claudius
		GlobalManager:AddCharacter(2); -- Claudius
		GlobalManager:AddCharacter(4); -- Claudius
		GlobalManager:AddToInventory(1, 3); -- Minor healing potions

		local battle = hoa_battle.BattleMode();
		battle:AddEnemy(2);
		battle:AddEnemy(2);
		battle:AddEnemy(3);
		battle:AddEnemy(5);
		ModeManager:Push(battle); 
	end
}

-- Begin tests intended for battle balancing
