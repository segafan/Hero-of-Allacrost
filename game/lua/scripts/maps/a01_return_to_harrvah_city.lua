--------------------------------------------------------------------------------
-- 01_return_to_harrvah_city.lua
--
-- Similar to a01_opening_scene.lua and using the same map. The entire map is a
-- scene of soldiers marching through the desert back to their home.
--------------------------------------------------------------------------------

local ns = {}
setmetatable(ns, {__index = _G})
a01_return_to_harrvah_city = ns;
setfenv(1, ns);

data_file = "lua/data/maps/harrvah_desert_cave_path.lua";
location_filename = "img/portraits/locations/blank.png";
map_name = "";

Map = {};

function Load(m)
	Map = m;

	ObjectManager = Map.object_supervisor;
	DialogueManager = Map.dialogue_supervisor;
	EventManager = Map.event_supervisor;
	TreasureManager = Map.treasure_supervisor;
	GlobalEvents = Map.map_event_group;
	
	-- Global starting coordinates for the center of the group of knights. All sprites
	-- use these coordinates in determining their initial positions.
	group_start_x = 300;
	group_start_y = 24;

	VideoManager:EnableLightOverlay(hoa_video.Color(0.0, 0.0, 0.3, 0.6));
	
	CreateCharacters();
	CreateNPCs();
	CreateDialogue();
	CreateEvents();

	Map:SetCamera(claudius);
	EventManager:StartEvent(10);

	-- This entire map is played out in scene state. As soon as the map is loaded, we start the chain of events.
	Map:PushState(hoa_map.MapMode.STATE_SCENE);
	Map:DisableIntroductionVisuals();
	Map:ShowStaminaBar(false);
	Map:ShowDialogueIcons(false);
end -- Load(m)



function Update()
	-- TODO: add a point light on left side of screen representing the city ablaze with gradually increasing intensity
end



function Draw()
	Map:DrawMapLayers();
--	VideoManager:ApplyLightingOverlay();
end

--------------------------------------------------------------------------------
-- Setup functions
--------------------------------------------------------------------------------

-- Creates the sprites for all characters in the party
function CreateCharacters()
	claudius = {};
	mark = {};
	dester = {};
	lukar = {};

	claudius = ConstructSprite("Claudius", 1000, group_start_x + 0.5, group_start_y + 0.5);
	claudius:SetDirection(hoa_map.MapMode.WEST);
	claudius:SetMovementSpeed(hoa_map.MapMode.SLOW_SPEED);
	claudius:SetNoCollision(true);
	Map:AddGroundObject(claudius);

	mark = ConstructSprite("Karlate", 1001, group_start_x + 1.5, group_start_y + 3);
	mark:SetDirection(hoa_map.MapMode.WEST);
	mark:SetName(hoa_system.Translate("Mark"));
	mark:SetNoCollision(true);
	Map:AddGroundObject(mark);

	dester = ConstructSprite("Karlate", 1002, group_start_x + 2.5, group_start_y - 2);
	dester:SetDirection(hoa_map.MapMode.WEST);
	dester:SetName(hoa_system.Translate("Dester"));
	dester:SetNoCollision(true);
	Map:AddGroundObject(dester);

	lukar = ConstructSprite("Karlate", 1003, group_start_x + 4, group_start_y);
	lukar:SetDirection(hoa_map.MapMode.WEST);
	lukar:SetName(hoa_system.Translate("Lukar"));
	lukar:SetNoCollision(true);
	Map:AddGroundObject(lukar);	
end


-- Creates all non-playable character spirtes
function CreateNPCs()
	local sprite;

	sprite = ConstructSprite("Captain", 2000, group_start_x + 8, group_start_y);
	sprite:SetDirection(hoa_map.MapMode.WEST);
	sprite:SetName(hoa_system.Translate("Captain Bravis"));
	sprite:SetNoCollision(true);
	Map:AddGroundObject(sprite);

	sprite = ConstructSprite("Karlate", 2001, group_start_x + 6, group_start_y - 3.5);
	sprite:SetDirection(hoa_map.MapMode.WEST);
	sprite:SetNoCollision(true);
	Map:AddGroundObject(sprite);

	sprite = ConstructSprite("Karlate", 2001, group_start_x + 6, group_start_y - 3.5);
	sprite:SetDirection(hoa_map.MapMode.WEST);
	sprite:SetNoCollision(true);
	Map:AddGroundObject(sprite);

	sprite = ConstructSprite("Karlate", 2002, group_start_x + 6.5, group_start_y + 2);
	sprite:SetDirection(hoa_map.MapMode.WEST);
	sprite:SetNoCollision(true);
	Map:AddGroundObject(sprite);

	sprite = ConstructSprite("Karlate", 2003, group_start_x - 3, group_start_y - 3);
	sprite:SetDirection(hoa_map.MapMode.WEST);
	sprite:SetNoCollision(true);
	Map:AddGroundObject(sprite);

	sprite = ConstructSprite("Karlate", 2004, group_start_x - 4, group_start_y + 5);
	sprite:SetDirection(hoa_map.MapMode.WEST);
	sprite:SetNoCollision(true);
	Map:AddGroundObject(sprite);

	sprite = ConstructSprite("Karlate", 2005, group_start_x - 5, group_start_y - 1);
	sprite:SetDirection(hoa_map.MapMode.WEST);
	sprite:SetNoCollision(true);
	Map:AddGroundObject(sprite);

	sprite = ConstructSprite("Karlate", 2006, group_start_x - 5, group_start_y - 5);
	sprite:SetDirection(hoa_map.MapMode.WEST);
	sprite:SetNoCollision(true);
	Map:AddGroundObject(sprite);

	sprite = ConstructSprite("Karlate", 2007, group_start_x - 6, group_start_y + 2);
	sprite:SetDirection(hoa_map.MapMode.WEST);
	sprite:SetNoCollision(true);
	Map:AddGroundObject(sprite);

	sprite = ConstructSprite("Karlate", 2008, group_start_x - 8, group_start_y - 4);
	sprite:SetDirection(hoa_map.MapMode.WEST);
	sprite:SetNoCollision(true);
	Map:AddGroundObject(sprite);

	sprite = ConstructSprite("Karlate", 2009, group_start_x - 9, group_start_y + 5);
	sprite:SetDirection(hoa_map.MapMode.WEST);
	sprite:SetNoCollision(true);
	Map:AddGroundObject(sprite);
	
	-- This sprite is the scout that runs in from the left side of the screen
	sprite = ConstructSprite("Karlate", 2010, 20, group_start_y);
	sprite:SetDirection(hoa_map.MapMode.WEST);
	sprite:SetMovementSpeed(hoa_map.MapMode.VERY_FAST_SPEED);
	sprite:SetNoCollision(true);
	Map:AddGroundObject(sprite);
end


-- Creates all dialogue that takes place through characters and events
function CreateDialogue()
	local dialogue;
	local text;

	dialogue = hoa_map.MapDialogue.Create(10);
		dialogue:SetInputBlocked(true);
		text = hoa_system.Translate("I can't wait to get back home. Maybe now that the water supply is restored they'll finally let us take a shower. I've been covered in sand for days.");
		dialogue:AddLine(text, 1001);
		dialogue:AddLineTiming(8000);
		
	dialogue = hoa_map.MapDialogue.Create(20);
		dialogue:SetInputBlocked(true);
		text = hoa_system.Translate("That's odd, the sky is brighter in the direction of the city.");
		dialogue:AddLine(text, 2005);
		dialogue:AddLineTiming(4000);
		text = hoa_system.Translate("I bet the citizens are out celebrating now that the water's returned. They're probably preparing to welcome us back as heroes!");
		dialogue:AddLine(text, 1001);
		dialogue:AddLineTiming(7000);
		text = hoa_system.Translate("Maybe...but who would still be awake at this hour?");
		dialogue:AddLine(text, 1002);
		dialogue:AddLineTiming(3000);
		text = hoa_system.Translate("Our scout should be returning soon, we'll find out then.");
		dialogue:AddLine(text, 1003);
		dialogue:AddLineTiming(4000);

	dialogue = hoa_map.MapDialogue.Create(30);
		text = hoa_system.Translate("Sir!");
		dialogue:AddLine(text, 2009);
		dialogue:AddLineTiming(3000);
		text = hoa_system.Translate("Catch your breath soldier. What's wrong?");
		dialogue:AddLine(text, 2000);
		dialogue:AddLineTiming(4000);
		text = hoa_system.Translate("The city! *huff* The city...it's ablaze!");
		dialogue:AddLine(text, 2009);
		dialogue:AddLineTiming(3000);
		dialogue:AddLineEventAtEnd(310);
end


-- Creates all events and sets up the entire event sequence chain
function CreateEvents()
	local event = {};

	-- Move all sprites away from the cave entrance
	local total_march_distance = 280;
	local march_distance = -220;
	event = hoa_map.PathMoveSpriteEvent.Create(10, 1000, march_distance, 0);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtStart(11);
	event:AddEventLinkAtStart(12);
	event:AddEventLinkAtStart(13);
	event:AddEventLinkAtStart(14);
	event:AddEventLinkAtStart(15);
	event:AddEventLinkAtStart(16);
	event:AddEventLinkAtStart(17);
	event:AddEventLinkAtStart(18);
	event:AddEventLinkAtStart(19);
	event:AddEventLinkAtStart(20);
	event:AddEventLinkAtStart(21);
	event:AddEventLinkAtStart(22);
	event:AddEventLinkAtStart(23);
	event:AddEventLinkAtStart(50, 4000);
	event:AddEventLinkAtStart(51, 20000);
	event:AddEventLinkAtEnd(24); -- The scout moves towards the main party as the dialogue ends
	event:AddEventLinkAtEnd(52, 2000); -- The scout makes his report
	event = hoa_map.PathMoveSpriteEvent.Create(11, 1001, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(12, 1002, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(13, 1003, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(14, 2000, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(15, 2001, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(16, 2002, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(17, 2003, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(18, 2004, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(19, 2005, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(20, 2006, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(21, 2007, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(22, 2008, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(23, 2009, march_distance, 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(24, 2010, total_march_distance + march_distance, 0);
	event:SetRelativeDestination(true);

	-- Create the dialogue
	event = hoa_map.DialogueEvent.Create(50, 10);
	event = hoa_map.DialogueEvent.Create(51, 20);
	event = hoa_map.DialogueEvent.Create(52, 30);

	-- Change movement speed of all sprites
	event = hoa_map.CustomSpriteEvent.Create(250, 1000, "ChangeSpriteMovementSpeed", "");
	event:AddEventLinkAtStart(251);
	event:AddEventLinkAtStart(252);
	event:AddEventLinkAtStart(253);
	event:AddEventLinkAtStart(254);
	event:AddEventLinkAtStart(255);
	event:AddEventLinkAtStart(256);
	event:AddEventLinkAtStart(257);
	event:AddEventLinkAtStart(258);
	event:AddEventLinkAtStart(259);
	event:AddEventLinkAtStart(260);
	event:AddEventLinkAtStart(261);
	event:AddEventLinkAtStart(262);
	event:AddEventLinkAtStart(263);
	event:AddEventLinkAtStart(264);
	event = hoa_map.CustomSpriteEvent.Create(251, 1001, "ChangeSpriteMovementSpeed", "");
	event = hoa_map.CustomSpriteEvent.Create(252, 1002, "ChangeSpriteMovementSpeed", "");
	event = hoa_map.CustomSpriteEvent.Create(253, 1003, "ChangeSpriteMovementSpeed", "");
	event = hoa_map.CustomSpriteEvent.Create(254, 2000, "ChangeSpriteMovementSpeed", "");
	event = hoa_map.CustomSpriteEvent.Create(255, 2001, "ChangeSpriteMovementSpeed", "");
	event = hoa_map.CustomSpriteEvent.Create(256, 2002, "ChangeSpriteMovementSpeed", "");
	event = hoa_map.CustomSpriteEvent.Create(257, 2003, "ChangeSpriteMovementSpeed", "");
	event = hoa_map.CustomSpriteEvent.Create(258, 2004, "ChangeSpriteMovementSpeed", "");
	event = hoa_map.CustomSpriteEvent.Create(259, 2005, "ChangeSpriteMovementSpeed", "");
	event = hoa_map.CustomSpriteEvent.Create(260, 2006, "ChangeSpriteMovementSpeed", "");
	event = hoa_map.CustomSpriteEvent.Create(261, 2007, "ChangeSpriteMovementSpeed", "");
	event = hoa_map.CustomSpriteEvent.Create(262, 2008, "ChangeSpriteMovementSpeed", "");
	event = hoa_map.CustomSpriteEvent.Create(263, 2009, "ChangeSpriteMovementSpeed", "");
	event = hoa_map.CustomSpriteEvent.Create(264, 2010, "ChangeSpriteMovementSpeed", "");

	-- Rush off to the city
	event = hoa_map.PathMoveSpriteEvent.Create(310, 1000, -(total_march_distance + march_distance), 0);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtStart(250);
	event:AddEventLinkAtStart(311);
	event:AddEventLinkAtStart(312);
	event:AddEventLinkAtStart(313);
	event:AddEventLinkAtStart(314);
	event:AddEventLinkAtStart(315);
	event:AddEventLinkAtStart(316);
	event:AddEventLinkAtStart(317);
	event:AddEventLinkAtStart(318);
	event:AddEventLinkAtStart(319);
	event:AddEventLinkAtStart(320);
	event:AddEventLinkAtStart(321);
	event:AddEventLinkAtStart(322);
	event:AddEventLinkAtStart(323);
	event:AddEventLinkAtStart(324);
	event:AddEventLinkAtEnd(500);
	event = hoa_map.PathMoveSpriteEvent.Create(311, 1001, -(total_march_distance + march_distance), 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(312, 1002, -(total_march_distance + march_distance), 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(313, 1003, -(total_march_distance + march_distance), 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(314, 2000, -(total_march_distance + march_distance), 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(315, 2001, -(total_march_distance + march_distance), 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(316, 2002, -(total_march_distance + march_distance), 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(317, 2003, -(total_march_distance + march_distance), 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(318, 2004, -(total_march_distance + march_distance), 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(319, 2005, -(total_march_distance + march_distance), 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(320, 2006, -(total_march_distance + march_distance), 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(321, 2007, -(total_march_distance + march_distance), 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(322, 2008, -(total_march_distance + march_distance), 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(323, 2009, -(total_march_distance + march_distance), 0);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(324, 2010, -(total_march_distance + march_distance), 0);
	event:SetRelativeDestination(true);
	event = hoa_map.MapTransitionEvent.Create(500, "lua/scripts/maps/a01_harrvah_city_attack.lua");
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

