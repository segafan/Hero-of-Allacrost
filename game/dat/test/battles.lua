------------------------------------------------------------------------------[[
-- Filename: battle.lua
--
-- Description: This file contains all tests that create an instance of BattleMode
-- with various configurations. The tests here are primarily for testing different
-- enemy parties and character strengths.
------------------------------------------------------------------------------]]

local ns = {}
setmetatable(ns, {__index = _G})
battles = ns;
setfenv(1, ns);

-- Test IDs 1,001 - 2,000 are reserved for battles
tests = {}

