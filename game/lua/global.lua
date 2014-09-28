
-- Character IDs. Each ID can have only a single bit active as IDs are used in bitmask operations.
CLAUDIUS  = 1;
MARK      = 2;
DESTER    = 4;
LUKAR     = 8;

function NewGame()
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
	local location_name = "lua/scripts/maps/a01_opening_scene.lua"
	GlobalManager:SetLocation(location_name);
	local opening_map = hoa_map.MapMode(location_name);

	ModeManager:Pop();	
	ModeManager:Push(opening_map);
end


-- Helper functions
function LoadNewMap(map_name)
	ModeManager:Pop();
	local new_map = hoa_map.MapMode("lua/scripts/maps/" .. map_name .. ".lua");
	ModeManager:Push(new_map);
end



function LoadNewShop(...)
	local i, v, item;
	local shop = hoa_shop.ShopMode();
	for i,v in ipairs(arg) do
		if (i % 2 == 1) then
			item = v
		else
			shop:AddObject(item, v)
		end
	end
	ModeManager:Push(shop);
end


-- Dummy functions

enemy_ids = {}
map_functions = {}

map_functions[0] = function()
	return true;
end
