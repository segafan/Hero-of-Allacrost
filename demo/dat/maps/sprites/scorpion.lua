function Load(enemy)
	enemy:SetName("Scorpion");
	enemy:SetObjectID(map:_GetGeneratedObjectID());
	enemy:SetContext(1);
	enemy:SetCollHalfWidth(1.0);
	enemy:SetCollHeight(2.0);
	enemy:SetImgHalfWidth(1.0);
	enemy:SetImgHeight(4.0);
	enemy:SetMovementSpeed(hoa_map.MapMode.VERY_SLOW_SPEED*1.5);
	enemy:LoadStandardAnimations("img/sprites/map/scorpion_walk.png");
end 
