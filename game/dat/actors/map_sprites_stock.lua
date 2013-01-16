sprites = {}
enemies = {}

local NORMAL_SPEED = hoa_map.MapMode.NORMAL_SPEED;
local SLOW_SPEED = hoa_map.MapMode.SLOW_SPEED;
local VERY_SLOW_SPEED = hoa_map.MapMode.VERY_SLOW_SPEED;
local VERY_FAST_SPEED = hoa_map.MapMode.VERY_FAST_SPEED;

sprites["Claudius"] = {
	name = hoa_system.Translate("Claudius"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/characters/claudius_walk.png",
	running_animations = "img/sprites/characters/claudius_run.png",
	face_portrait = "img/portraits/face/claudius.png"
}

sprites["Laila"] = {
	name = hoa_system.Translate("Laila"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = SLOW_SPEED,

	standard_animations = "img/sprites/characters/laila_walk.png",
	running_animations = "img/sprites/characters/laila_run.png",
	face_portrait = "img/portraits/face/laila.png"
}

sprites["Kyle"] = {
	name = hoa_system.Translate("Kyle"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = SLOW_SPEED,

	standard_animations = "img/sprites/characters/kyle_walk.png",
	face_portrait = "img/portraits/face/kyle.png"
}

sprites["Captain"] = {
	name = hoa_system.Translate("Captain"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = SLOW_SPEED,

	standard_animations = "img/sprites/characters/soldier_npc02_walk.png"
}

sprites["Marcus"] = {
	name = hoa_system.Translate("Marcus"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = VERY_SLOW_SPEED,

	standard_animations = "img/sprites/characters/marcus_walk.png",
	face_portrait = "img/portraits/face/marcus.png"
}

sprites["Vanica"] = {
	name = hoa_system.Translate("Vanica"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = VERY_SLOW_SPEED,
	
	standard_animations = "img/sprites/characters/vanica_walk.png",
	face_portrait = "img/portraits/face/vanica.png"
}

sprites["Alexander"] = {
	name = hoa_system.Translate("Alexander"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = SLOW_SPEED,
	
	standard_animations = "img/sprites/characters/man_npc02_walk.png"
}

sprites["Laine"] = {
	name = hoa_system.Translate("Laine"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = SLOW_SPEED,
	
	standard_animations = "img/sprites/characters/man_npc01_walk.png"
}

sprites["Torl"] = {
	name = hoa_system.Translate("Torl"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = VERY_FAST_SPEED,
	
	standard_animations = "img/sprites/characters/boy_npc01_walk.png"
}

sprites["Female Merchant"] = {
	name = hoa_system.Translate("Female Merchant"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = SLOW_SPEED,
	
	standard_animations = "img/sprites/characters/woman_npc01_walk.png"
}

sprites["Livia"] = {
	name = hoa_system.Translate("Livia"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = SLOW_SPEED,
	
	standard_animations = "img/sprites/characters/girl_npc02_walk.png"
}

sprites["Octavia"] = {
	name = hoa_system.Translate("Octavia"),
	coll_half_width = 1.0,
	coll_height = 2.0,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,
	
	standard_animations = "img/sprites/characters/woman_npc02_walk.png"
}

sprites["Karlate"] = {
	name = hoa_system.Translate("Karlate"),
	coll_half_width = 0.95,
	coll_height = 1.9,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = SLOW_SPEED,

	standard_animations = "img/sprites/characters/soldier_npc01_walk.png"
}


sprites["Mak Hound"] = {
	name = hoa_system.Translate("Mak Hound"),
	coll_half_width = 3.0,
	coll_height = 3.0,
	img_half_width = 4.0,
	img_height = 8.0,
	movement_speed = SLOW_SPEED,

	standard_animations = "img/sprites/creatures/mak_hound.png"
}


sprites["Rubble"] = {
	name = "????",
	coll_half_width = 1.0,
	coll_height = 2.0,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,

	standard_animations = "img/sprites/objects/rocks.png"
}


enemies["slime"] = {
	coll_half_width = 1.0,
	coll_height = 2.0,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,
	standard_animations = "img/sprites/enemies/slime_walk.png"
}


enemies["snake"] = {
	coll_half_width = 1.0,
	coll_height = 2.0,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,
	standard_animations = "img/sprites/enemies/snake_walk.png"
}


enemies["scorpion"] = {
	coll_half_width = 1.0,
	coll_height = 2.0,
	img_half_width = 1.0,
	img_height = 4.0,
	movement_speed = NORMAL_SPEED,
	standard_animations = "img/sprites/enemies/scorpion_walk.png"
}


sprite = {}
enemy = {}

function ConstructSprite(name, id, x, y)
	-- Convert the X/Y cooridnates into integer + offset values
	local x_int = math.floor(x);
	local x_off = x - x_int;
	local y_int = math.floor(y);
	local y_off = y - y_int;

	dir = (2 ^ math.random(0, 3));

	if (sprites[name]) then
		sprite = hoa_map.MapSprite();
		sprite:SetName(sprites[name].name);
		sprite:SetObjectID(id);
		sprite:SetContext(1);
		sprite:SetXPosition(x_int, x_off);
		sprite:SetYPosition(y_int, y_off);
		sprite:SetCollHalfWidth(sprites[name].coll_half_width);
		sprite:SetCollHeight(sprites[name].coll_height);
		sprite:SetImgHalfWidth(sprites[name].img_half_width);
		sprite:SetImgHeight(sprites[name].img_height);
		sprite:SetMovementSpeed(sprites[name].movement_speed);
		sprite:SetDirection(dir);
		sprite:LoadStandardAnimations(sprites[name].standard_animations);
		if (sprites[name].running_animations) then
			sprite:LoadRunningAnimations(sprites[name].running_animations);
		end
		if (sprites[name].face_portrait) then
			sprite:LoadFacePortrait(sprites[name].face_portrait);
		end
		return sprite;
	else
		return nil;
	end
end



function ConstructEnemySprite(name, Map)
	if (enemies[name] == nil) then
		return nil;
	end
	
	enemy = hoa_map.EnemySprite();
	enemy:SetObjectID(Map.object_supervisor:GenerateObjectID());
	enemy:SetContext(hoa_map.MapMode.CONTEXT_01);
	enemy:SetCollHalfWidth(enemies[name].coll_half_width);
	enemy:SetCollHeight(enemies[name].coll_height);
	enemy:SetImgHalfWidth(enemies[name].img_half_width);
	enemy:SetImgHeight(enemies[name].img_height);
	enemy:SetMovementSpeed(enemies[name].movement_speed);
	enemy:LoadStandardAnimations(enemies[name].standard_animations);
	
	return enemy;
end

