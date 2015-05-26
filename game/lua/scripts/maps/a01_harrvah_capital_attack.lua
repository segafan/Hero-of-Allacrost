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
	
	Map:SetCamera(sprites["claudius"]);

	-- TODO: figure out if visuals should be disabled normally, or wait for control to be given to the player before they are displayed
	-- Map:DisableIntroductionVisuals();

	Map:ShowStaminaBar(true);
	Map:ShowDialogueIcons(true);
	print "Load Complete";
end -- Load(m)



function Update()
	-- Nothing special required
end



function Draw()
	Map:DrawMapLayers();
end


function CreateZones()
	---------- Context Zones
	zones["inn_entrance"] = hoa_map.ContextZone(contexts["exterior"], contexts["interior"]);
	zones["inn_entrance"]:AddSection(116, 120, 181, 182, true);
	zones["inn_entrance"]:AddSection(116, 120, 180, 181, false);
	Map:AddZone(zones["inn_entrance"]);
end



function CreateObjects()

end



function CreateSprites()
	local sprite;
	local animation;

	-- X/Y position that represents the southern middle part of the map
	local startx = 98;
	local starty = 205;

	-- Create sprites for the three playable characters
	sprites["claudius"] = ConstructSprite("Claudius", 1, startx, starty);
	sprites["claudius"]:SetDirection(hoa_map.MapMode.NORTH);
	ObjectManager:AddObject(sprites["claudius"], 0);

	sprites["mark"] = ConstructSprite("Knight01", 2, startx - 8, starty);
	sprites["mark"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["mark"]:SetName(hoa_system.Translate("Mark"));
	ObjectManager:AddObject(sprites["mark"], 0);

	sprites["lukar"] = ConstructSprite("Knight01", 3, startx + 8, starty);
	sprites["lukar"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["lukar"]:SetName(hoa_system.Translate("Lukar"));
	ObjectManager:AddObject(sprites["lukar"], 0);

	-- Create the captain, his sergeant, and one senior knight leading the troop heading due East
	sprites["captain"] = ConstructSprite("Knight06", 10, startx + 20, starty);
	sprites["captain"]:SetDirection(hoa_map.MapMode.EAST);
	sprites["captain"]:SetName(hoa_system.Translate("Captain Bravis"));
	ObjectManager:AddObject(sprites["captain"], 0);

	sprites["sergeant"] = ConstructSprite("Knight05", 11, startx + 24, starty);
	sprites["sergeant"]:SetDirection(hoa_map.MapMode.EAST);
	sprites["sergeant"]:SetName(hoa_system.Translate("Sergeant Methus"));
	ObjectManager:AddObject(sprites["sergeant"], 0);

	sprites["senior_knight"] = ConstructSprite("Knight04", 12, startx + 28, starty);
	sprites["senior_knight"]:SetDirection(hoa_map.MapMode.EAST);
	ObjectManager:AddObject(sprites["senior_knight"], 0);
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

	----------------------------------------------------------------------------
	---------- Dialogues triggered by events
	----------------------------------------------------------------------------

end -- function CreateDialogues()


-- Creates all events and sets up the entire event sequence chain
function CreateEvents()
	local event = {};
	
	-- TODO: Add event chains

	-- Event Group #1: Initial map entrance and dialogue


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

