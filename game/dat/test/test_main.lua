------------------------------------------------------------------------------[[
-- Filename: test_main.lua
--
-- Description: This file serves as a directory for all available functional tests
-- that are used by the TestMode class. Tests are organized into categories, where
-- each category is given a string handle (it's table name), name (that will be
-- seen by the user in the test menu), and description of what the test category
-- contains.
--
-- Within each category are numerous tests which have a unique integer ID, that
-- serve as the test's table key. Additional information that tests are required to
-- have are a name that will be used to briefly describe the test, a description
-- that describes the test in further detail, and the instructions for executing the
-- test (TODO: define what data is needed to execute tests).
--
-- Test IDs are unsigned integers. Do not use the value of zero, as that is reserved.
-- You should declare a range of IDs that are reserved for each category of tests
------------------------------------------------------------------------------]]

local ns = {}
setmetatable(ns, {__index = _G})
test_main = ns;
setfenv(1, ns);

-- A table which holds the names of each test category table in this file
categories = {}

--------------------------------------------------------------------------------
-- Mode code tests: Reserve test IDs 1 - 10,000
--------------------------------------------------------------------------------

----- maps: Reserve test IDs 1 - 1,000
table.insert(categories, "maps");
maps = {
    name = "Test Maps";
    description = "These tests load a declared map file and set any custom criteria prior to placing the user on the map. This may include declaring the character party, inventory, what global events have been triggered, setting a custom spawn position, and so on.";
    min_id = 1;
    max_id = 1000;
    file = "dat/test/maps.lua";
}

----- battles: Reserve test IDs 1,001 - 2,000
table.insert(categories, "battles");
battles = {
    name = "Test Battles";
    description = "These tests declare the character and enemy parties, inventory, skills, and any other relevant battle information such as locations or battle scripts to use. They are very useful for game balancing and testing possible battle settings and configurations";
    min_id = 1001;
    max_id = 2000;
    file = "dat/test/battles.lua";
}

--------------------------------------------------------------------------------
-- Common code tests: Reserve test IDs 10,001 - 20,000
--------------------------------------------------------------------------------



--------------------------------------------------------------------------------
-- Engine code tests: Reserve test IDs 20,001 - 30,000
--------------------------------------------------------------------------------



--------------------------------------------------------------------------------
-- Utility code tests: Reserve test IDs 30,001 - 40,000
--------------------------------------------------------------------------------



