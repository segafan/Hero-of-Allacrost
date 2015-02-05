--------------------------------------------------------------------------------
-- a01_harrvah_capital_attack.lua
--
-- This script controls the events that happen on the map after a player starts
-- a new game. It is heavy on dialogue and the user has no control over the characters.
--------------------------------------------------------------------------------
local ns = {}
setmetatable(ns, {__index = _G})
a01_harrvah_capital_attack = ns;
setfenv(1, ns);

data_file = "lua/data/maps/harrvah_capital.lua";
location_filename = "img/portraits/locations/blank.png";
map_name = "Harrvah Capital";

sound_filenames = {};

music_filenames = {};

Map = {};

function Load(m)
	Map = m;

	ObjectManager = Map.object_supervisor;
	DialogueManager = Map.dialogue_supervisor;
	EventManager = Map.event_supervisor;
	TreasureManager = Map.treasure_supervisor;
	GlobalEvents = Map.map_event_group;

	-- Setup the order in which we wish to draw the tile and object layers
	Map:ClearLayerOrder();
	Map:AddTileLayerToOrder(0);
	Map:AddTileLayerToOrder(1);
	Map:AddObjectLayerToOrder(0);
	Map:AddTileLayerToOrder(2);
	
	-- Visuals: night lightning
	VideoManager:EnableLightOverlay(hoa_video.Color(0.0, 0.0, 0.3, 0.6));
	
	CreateSprites();
	Map:SetCamera(claudius);
	CreateDialogue();
	CreateEvents();
	--EventManager:StartEvent(10);

	--local event = hoa_map.CustomSpriteEvent.Create(1000, 10, "FocusCameraOnSprite", "");

	-- Map:PushState(hoa_map.MapMode.STATE_SCENE);
	-- TODO: figure out if visuals should be disabled normally, or wait for control to be given to the player before they are displayed
	-- Map:DisableIntroductionVisuals();
	Map:ShowStaminaBar(true);
	Map:ShowDialogueIcons(true);
end -- Load(m)



function Update()
	-- Nothing special required
end



function Draw()
	Map:DrawMapLayers();
end


-- Creates all sprites for the characters, knights, and hounds
function CreateSprites()
	local sprite;
	local animation;

	-- X/Y position that represents the southern middle part of the map
	local startx = 98;
	local starty = 252;

	-- Create sprites for the three playable characters
	claudius = {};
	mark = {};
	lukar = {};

	claudius = ConstructSprite("Claudius", 1, startx, starty);
	claudius:SetDirection(hoa_map.MapMode.NORTH);
	claudius:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	claudius:SetNoCollision(true);
	animation = claudius:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(claudius, 0);

	mark = ConstructSprite("Knight01", 2, startx - 8, starty);
	mark:SetDirection(hoa_map.MapMode.NORTH);
	mark:SetName(hoa_system.Translate("Mark"));
	mark:SetNoCollision(true);
	animation = mark:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(mark, 0);

	lukar = ConstructSprite("Knight01", 3, startx + 8, starty);
	lukar:SetDirection(hoa_map.MapMode.NORTH);
	lukar:SetName(hoa_system.Translate("Lukar"));
	lukar:SetNoCollision(true);
	animation = lukar:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(lukar, 0);

	-- Create the captain, his sergeant, and one senior knight leading the troop heading due East
	sprite = ConstructSprite("Knight06", 10, startx + 20, starty);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetName(hoa_system.Translate("Captain Bravis"));
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite, 0);

	sprite = ConstructSprite("Knight05", 11, startx + 24, starty);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetName(hoa_system.Translate("Sergeant Methus"));
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite, 0);

	sprite = ConstructSprite("Knight04", 12, startx + 28, starty);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	sprite:SetNoCollision(true);
	animation = sprite:GetAnimation(hoa_map.MapMode.ANIM_WALKING_EAST);
	animation:RandomizeCurrentLoopProgress();
	ObjectManager:AddObject(sprite, 0);
end -- function CreateSprites()


-- Creates all dialogue that takes place through characters and events
function CreateDialogue()
	local dialogue;
	local text;

	-- Dialogue corresponding to events upon entering the map
	dialogue = hoa_map.MapDialogue.Create(10);
		text = hoa_system.Translate("The city is under attack by demons.");
		dialogue:AddLine(text, 1);

end -- function CreateDialogue()


-- Creates all events and sets up the entire event sequence chain
function CreateEvents()
	local event = {};
	
	-- TODO: Add event chains

	-- Event Group #1: Initial map entrance and dialogue


end -- function CreateEvents()


-- Container for all map class event functions
functions = {};

-- Sprite function: Focus map camera on sprite
functions["FocusCameraOnSprite"] = function(sprite)
	Map:SetCamera(sprite, 1000);
end


-- Sprite function: Disable collision and visibility on sprite
functions["DisableCollisionAndVisibility"] = function(sprite)
	sprite:SetVisible(false);
	sprite:SetNoCollision(true);
end


-- Sprite function: Change movement speed of a sprite
functions["ChangeSpriteMovementSpeed"] = function(sprite)
	sprite:SetMovementSpeed(hoa_map.MapMode.VERY_FAST_SPEED);
end

