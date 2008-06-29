CLAUDIUS = 1
LAILA = 2
RAGNAR = 3

function NewGame()
	GlobalManager:AddCharacter(RAGNAR);
	GlobalManager:SetDrunes(250);
	GlobalManager:AddToInventory(1, 2);
	GlobalManager:SetLocation("dat/maps/demo_town.lua");
	GlobalManager:SetLocation("dat/maps/tutorial_story.lua");
end
