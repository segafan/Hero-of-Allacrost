CLAUDIUS = 1
LAILA = 2

function NewGame()
	GlobalManager:AddCharacter(CLAUDIUS);
--	GlobalManager:GetCharacter(CLAUDIUS):AddSkill(10001);
	GlobalManager:SetDrunes(250);
	GlobalManager:AddToInventory(1, 2);
	GlobalManager:SetLocation("dat/maps/demo_town.lua");
end
