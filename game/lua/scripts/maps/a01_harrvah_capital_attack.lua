--------------------------------------------------------------------------------
-- a01_harrvah_capital_attack.lua
--
-- A script specific to the main storyline events. The Harrvah Capital is under
-- attack by demons and the player has to navigate his party through the chaos
-- to find the king.
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
music_filenames[1] = "mus/Seeking_New_Worlds.ogg";

-- Primary Map Classes
Map = {};
ObjectManager = {};
DialogueManager = {};
EventManager = {};
TreasureManager = {};
GlobalEvents = {};

enemy_ids = { }

-- Containers used to hold pointers to various class objects.
contexts = {};
zones = {};
objects = {};
sprites = {};
dialogues = {};
events = {};

-- All custom map functions are contained within the following table.
-- String keys in this table serves as the names of these functions. 
functions = {};

-- Shorthand names for map contexts
contexts["exterior"] = hoa_map.MapMode.CONTEXT_01; -- Displays the exterior of the town
contexts["interior"] = hoa_map.MapMode.CONTEXT_03; -- Displays the interiors of various structures



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
	Map:AddTileLayerToOrder(3);

	print "CreateZones()"
	CreateZones();
	print "CreateObjects()"
	CreateObjects();
	print "CreateSprites()"
	CreateSprites();
	print "CreateEnemies()"
--	CreateEnemies();
	print "CreateDialogues()"
	CreateDialogues();
	print "CreateEvents()"
	CreateEvents();

	-- Visuals: night lightning
	VideoManager:EnableLightOverlay(hoa_video.Color(0.0, 0.0, 0.3, 0.6));

	Map:SetCurrentTrack(0);

	-- TODO: figure out if visuals should be disabled normally, or wait for control to be given to the player before they are displayed
	-- Map:DisableIntroductionVisuals();
	Map.unlimited_stamina = true; -- DEBUG: Set to false for release
	Map:ShowStaminaBar(false);
	Map:ShowDialogueIcons(true);
	print "Load Complete";

	Map:SetCamera(sprites["claudius"]);

	-- The map begins with an opening scene before control is given to the player
--	Map:PushState(hoa_map.MapMode.STATE_SCENE);
--	EventManager:StartEvent(events["opening_scene"], 2500);
end -- Load(m)



function Update()
	-- Nothing special required
end



function Draw()
	Map:DrawMapLayers();
end


function CreateZones()
	---------- Context Zones

	-- The following zone implements the context switching for all structures found in the town.
	-- The zone sections correspond to the area just outside of the doors to these buildings.
	-- They are ordered starting from the bottom left of the map, going toward the right.
	zones["town_doors"] = hoa_map.ContextZone(contexts["exterior"], contexts["interior"]);
	-- Southwest home 1
	zones["town_doors"]:AddSection(22, 26, 176, 177, true);
	zones["town_doors"]:AddSection(22, 26, 175, 176, false);
	-- Soutwest home 2
	zones["town_doors"]:AddSection(48, 52, 180, 181, true);
	zones["town_doors"]:AddSection(48, 52, 179, 180, false);
	-- Item Shop
	zones["town_doors"]:AddSection(80, 84, 180, 181, true);
	zones["town_doors"]:AddSection(80, 84, 179, 180, false);
	-- Inn
	zones["town_doors"]:AddSection(116, 120, 182, 1835, true);
	zones["town_doors"]:AddSection(116, 120, 181, 182, false);

	-- West home 1
	zones["town_doors"]:AddSection(20, 24, 152, 153, true);
	zones["town_doors"]:AddSection(20, 24, 151, 152, false);
	-- West home 2
	zones["town_doors"]:AddSection(48, 52, 150, 151, true);
	zones["town_doors"]:AddSection(48, 52, 149, 150, false);
	-- West home 3
	zones["town_doors"]:AddSection(78, 82, 152, 153, true);
	zones["town_doors"]:AddSection(78, 82, 151, 152, false);
	-- Weapon Shop
	zones["town_doors"]:AddSection(148, 152, 154, 155, true);
	zones["town_doors"]:AddSection(148, 152, 153, 154, false);
	-- East home
	zones["town_doors"]:AddSection(178, 182, 148, 149, true);
	zones["town_doors"]:AddSection(178, 182, 147, 148, false);

	-- Northwest home 1
	zones["town_doors"]:AddSection(12, 16, 122, 123, true);
	zones["town_doors"]:AddSection(12, 16, 121, 122, false);
	-- Northwest home 2
	zones["town_doors"]:AddSection(50, 54, 126, 127, true);
	zones["town_doors"]:AddSection(50, 54, 125, 126, false);
	-- Northeast home 1
	zones["town_doors"]:AddSection(116, 120, 126, 127, true);
	zones["town_doors"]:AddSection(116, 120, 125, 126, false);
	-- Northwest home 2
	zones["town_doors"]:AddSection(140, 144, 122, 123, true);
	zones["town_doors"]:AddSection(140, 144, 121, 122, false);
	-- Claudius' home
	zones["town_doors"]:AddSection(168, 172, 124, 125, true);
	zones["town_doors"]:AddSection(168, 172, 123, 124, false);

	Map:AddZone(zones["town_doors"]);
end



function CreateObjects()

end



function CreateSprites()
	local sprite;
	local animation;
	local ground_object_layer = 0;

	-- X/Y position that represents the southern middle part of the map
	local startx = 98;
	local starty = 205;

	-- Create sprites for the three playable characters
	sprites["claudius"] = ConstructSprite("Claudius", 1, startx, starty);
	sprites["claudius"]:SetDirection(hoa_map.MapMode.NORTH);
	ObjectManager:AddObject(sprites["claudius"], ground_object_layer);

	sprites["mark"] = ConstructSprite("Knight01", 2, startx - 8, starty);
	sprites["mark"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["mark"]:SetName(hoa_system.Translate("Mark"));
	ObjectManager:AddObject(sprites["mark"], ground_object_layer);

	sprites["lukar"] = ConstructSprite("Knight01", 3, startx + 8, starty);
	sprites["lukar"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["lukar"]:SetName(hoa_system.Translate("Lukar"));
	ObjectManager:AddObject(sprites["lukar"], ground_object_layer);

	-- Create the captain, his sergeant, and one senior knight leading the troop heading due East
	sprites["captain"] = ConstructSprite("Knight06", 10, startx + 20, starty);
	sprites["captain"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["captain"]:SetName(hoa_system.Translate("Captain Bravis"));
	ObjectManager:AddObject(sprites["captain"], ground_object_layer);

	sprites["sergeant"] = ConstructSprite("Knight05", 11, startx + 24, starty);
	sprites["sergeant"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["sergeant"]:SetName(hoa_system.Translate("Sergeant Methus"));
	ObjectManager:AddObject(sprites["sergeant"], ground_object_layer);

	sprites["senior_knight"] = ConstructSprite("Knight04", 12, startx + 28, starty);
	sprites["senior_knight"]:SetDirection(hoa_map.MapMode.NORTH);
	ObjectManager:AddObject(sprites["senior_knight"], ground_object_layer);

	-- TEMP: townspeople that should eventually be moved to the map script for the attack aftermath
	sprites["laila"] = ConstructSprite("Laila", 500, 180, 118);
	sprites["laila"]:SetDirection(hoa_map.MapMode.SOUTH);
	sprites["laila"]:SetContext(contexts["interior"]);
	ObjectManager:AddObject(sprites["laila"], ground_object_layer);

	sprites["marcus"] = ConstructSprite("Marcus", 501, 186, 112);
	sprites["marcus"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["marcus"]:SetContext(contexts["interior"]);
	ObjectManager:AddObject(sprites["marcus"], ground_object_layer);

	sprites["vanica"] = ConstructSprite("Vanica", 502, 166, 112);
	sprites["vanica"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["vanica"]:SetContext(contexts["interior"]);
	ObjectManager:AddObject(sprites["vanica"], ground_object_layer);

	sprites["weapon_merchant"] = ConstructSprite("Alexander", 510, 144, 140);
	sprites["weapon_merchant"]:SetDirection(hoa_map.MapMode.SOUTH);
	sprites["weapon_merchant"]:SetContext(contexts["interior"]);
	ObjectManager:AddObject(sprites["weapon_merchant"], ground_object_layer);

	sprites["item_merchant"] = ConstructSprite("Female Merchant", 520, 73, 173);
	sprites["item_merchant"]:SetDirection(hoa_map.MapMode.EAST);
	sprites["item_merchant"]:SetContext(contexts["interior"]);
	ObjectManager:AddObject(sprites["item_merchant"], ground_object_layer);

	sprites["inn_keeper"] = ConstructSprite("Octavia", 530, 118, 168);
	sprites["inn_keeper"]:SetDirection(hoa_map.MapMode.SOUTH);
	sprites["inn_keeper"]:SetContext(contexts["interior"]);
	ObjectManager:AddObject(sprites["inn_keeper"], ground_object_layer);

	sprites["inn_worker"] = ConstructSprite("Livia", 531, 113, 174);
	sprites["inn_worker"]:SetDirection(hoa_map.MapMode.EAST);
	sprites["inn_worker"]:SetContext(contexts["interior"]);
	ObjectManager:AddObject(sprites["inn_worker"], ground_object_layer);
end -- function CreateSprites()



function CreateEnemies()

end



function CreateDialogues()
	local dialogue;
	local text;

	----------------------------------------------------------------------------
	---------- Dialogues attached to characters
	----------------------------------------------------------------------------
	-- Dialogue corresponding to events upon entering the map
	dialogue = hoa_map.MapDialogue.Create(10);
		text = hoa_system.Translate("The city is under attack by demons.");
		dialogue:AddLine(text, sprites["captain"]:GetObjectID());
	sprites["captain"]:AddDialogueReference(10);

	-- TEMP: should eventually be moved to the map script for the attack aftermath
	dialogue = hoa_map.MapDialogue.Create(100);
		text = hoa_system.Translate("Welcome home, Claudius. What's the matter?");
		dialogue:AddLine(text, sprites["laila"]:GetObjectID());
		text = hoa_system.Translate("It's...nothing. I'm just glad you're okay.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
	sprites["laila"]:AddDialogueReference(100);

	dialogue = hoa_map.MapDialogue.Create(101);
		text = hoa_system.Translate("Oh dear, oh dear.");
		dialogue:AddLine(text, sprites["vanica"]:GetObjectID());
	sprites["vanica"]:AddDialogueReference(101);

	dialogue = hoa_map.MapDialogue.Create(102);
		text = hoa_system.Translate("Your mother and I are doing what we can for those who lost their homes in the attack.");
		dialogue:AddLine(text, sprites["marcus"]:GetObjectID());
		text = hoa_system.Translate("Let me help. Show me where I can make myself useful.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("No, you need to rest. We may be attacked again you know, and how are you going to defend our city if you're exhausted?");
		dialogue:AddLine(text, sprites["marcus"]:GetObjectID());
	sprites["marcus"]:AddDialogueReference(102);

	dialogue = hoa_map.MapDialogue.Create(103);
		text = hoa_system.Translate("Sorry, this inn has converted into a shelter after the attack and we're completely full.");
		dialogue:AddLine(text, sprites["inn_keeper"]:GetObjectID());
	sprites["inn_keeper"]:AddDialogueReference(103);

	dialogue = hoa_map.MapDialogue.Create(104);
		text = hoa_system.Translate("So much work to do. I'm exhausted.");
		dialogue:AddLine(text, sprites["inn_worker"]:GetObjectID());
	sprites["inn_worker"]:AddDialogueReference(104);

	dialogue = hoa_map.MapDialogue.Create(105);
		text = hoa_system.Translate("Most of my inventory was destroyed in the attack.");
		dialogue:AddLine(text, sprites["item_merchant"]:GetObjectID());
		dialogue:AddLineEventAtEnd(1000); -- event_chains["item_shop"]
	sprites["item_merchant"]:AddDialogueReference(105);

	dialogue = hoa_map.MapDialogue.Create(106);
		text = hoa_system.Translate("Demand has skyrocketed after our city was attacked. I've sold most of my armaments, but I have a few selections remaining.");
		dialogue:AddLine(text, sprites["weapon_merchant"]:GetObjectID());
		dialogue:AddLineEventAtEnd(1001); -- event_chains["weapon_armor_shop"] = 1001;
	sprites["weapon_merchant"]:AddDialogueReference(106);

	----------------------------------------------------------------------------
	---------- Dialogues triggered by events
	----------------------------------------------------------------------------

end -- function CreateDialogues()


-- Creates all events and sets up the entire event sequence chain
function CreateEvents()
	event_chains = {}; -- Holds IDs of the starting event for each event chain
	local event = {};

	-- Event Group #1: Initial map entrance and dialogue

	----------------------------------------------------------------------------
	---------- Miscellaneous Events
	----------------------------------------------------------------------------
	event_chains["item_shop"] = 1000;
	event = hoa_map.CustomEvent.Create(event_chains["item_shop"], "LoadItemShop", "");

	event_chains["weapon_armor_shop"] = 1001;
	event = hoa_map.CustomEvent.Create(event_chains["weapon_armor_shop"], "LoadWeaponArmorShop", "");

end -- function CreateEvents()

----------------------------------------------------------------------------
---------- Event Functions
----------------------------------------------------------------------------

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


-- Puts game state into shop mode with items
functions["LoadItemShop"] = function()
	LoadNewShop(
		1, 4 -- healing potions
	);
end


-- Puts game state into shop mode with items
functions["LoadWeaponArmorShop"] = function()
	LoadNewShop(
		10002, 1, -- iron sword
		20001, 2, -- karlate helm
		30001, 3, -- leather chain mail
		30002, 1  -- karlate breast plate
	);
end

