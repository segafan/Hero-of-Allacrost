CLAUDIUS = 1
LAILA = 2
KYLE = 3
RAFAELA = 4

function NewGame()
	GlobalManager:AddCharacter(CLAUDIUS);
	GlobalManager:SetDrunes(250);
	GlobalManager:AddToInventory(1, 2);
	GlobalManager:SetLocation("dat/maps/opening_scene.lua");
end


-- Helper functions

function LoadNewMap(map_name)
	ModeManager:Pop();
	local cave_map = hoa_map.MapMode("dat/maps/" .. map_name .. ".lua");
	ModeManager:Push(cave_map);
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
