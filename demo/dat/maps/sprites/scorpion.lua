function Load(monster)
	monster:SetName("Scorpion");
	monster:SetObjectID( map:_GetGeneratedObjectID() );
	monster:SetContext(1);
	monster:SetCollHalfWidth(1.0);
	monster:SetCollHeight(2.0);
	monster:SetImgHalfWidth(1.0);
	monster:SetImgHeight(4.0);
	monster:SetMovementSpeed(hoa_map.MapMode.VERY_SLOW_SPEED*1.5);
end 

sprite_sheet = "img/sprites/map/scorpion_walk.png";
sprite_sheet_rows = 4;
sprite_sheet_cols = 6;

frame_speed = 45;
	
standing_south_frames = { 0 };
standing_north_frames = { 6 };
standing_west_frames = { 12 };
standing_east_frames = { 18 };

walking_south_frames = { 1,2,3,4,5 };
walking_north_frames = { 7,8,9,10,11 };
walking_west_frames = { 13,14,15,16,17 };
walking_east_frames = { 19,20,21,22,23 };