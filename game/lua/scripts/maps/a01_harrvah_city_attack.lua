local ns = {}
setmetatable(ns, {__index = _G})
a01_harrvah_city_attack = ns;
setfenv(1, ns);

-- The map name and location graphic
data_file = "lua/data/maps/harrvah_city.lua";
location_filename = "img/portraits/locations/blank.png"
map_name = "Harrvah City"

claudius_sprite = nil;

function Load(m)
	-- First, record the current map in the map variable that is global to this script
	Map = m;
	Map.unlimited_stamina = true;
	ObjectManager = Map.object_supervisor;
	DialogueManager = Map.dialogue_supervisor;
	EventManager = Map.event_supervisor;
	TreasureManager = Map.treasure_supervisor;
	GlobalEvents = Map.map_event_group;

	-- Play music once the map becomes active for the first time
	Map:SetCurrentTrack(0);

	-- Create the sprite that the player controls
	claudius_sprite = ConstructSprite("Claudius", 1000, 14, 85);
	claudius_sprite:SetDirection(hoa_map.MapMode.NORTH);
	ObjectManager:AddObject(claudius_sprite, 0);
	Map:SetCamera(claudius_sprite);

	CreateDoors();
	CreateCharacters();
	CreateEnemies();
	-- TODO: throws a run-time error when creating dialogue. Fix this issue and then uncomment function call below
	-- CreateDialogue();
	CreateEvents();
end


-- Mandatory function for map updates
function Update()

end


-- Mandatory function for custom drawing
function Draw()
	Map:DrawMapLayers();
end


-- Create zones for switching the map context (usually doors or stairs)
function CreateDoors()
	context_zone = hoa_map.ContextZone(1, 2);
	context_zone:AddSection(154, 157, 156, 157, false);
	context_zone:AddSection(154, 157, 158, 159, true);
	Map:AddZone(context_zone);

	context_zone = hoa_map.ContextZone(1, 2);
	context_zone:AddSection(154, 157, 120, 121, false);
	context_zone:AddSection(154, 157, 122, 123, true);
	Map:AddZone(context_zone);

	context_zone = hoa_map.ContextZone(1, 2);
	context_zone:AddSection(378, 381, 142, 143, false);
	context_zone:AddSection(378, 381, 144, 145, true);
	Map:AddZone(context_zone);

	context_zone = hoa_map.ContextZone(1, 2);
	context_zone:AddSection(314, 317, 116, 117, false);
	context_zone:AddSection(314, 317, 118, 119, true);
	Map:AddZone(context_zone);
	
	context_zone = hoa_map.ContextZone(1, 2);
	context_zone:AddSection(346, 349,  80,  81, false);
	context_zone:AddSection(346, 349,  82,  83, true);
	Map:AddZone(context_zone);
end -- function CreateDoors()


-- Creates all non-playable character spirtes
function CreateCharacters()
	local sprite = {};

	sprite = ConstructSprite("Laila", 2000, 14, 70);
	sprite:SetDirection(hoa_map.MapMode.SOUTH);
	sprite:AddDialogueReference(10);
	sprite:AddDialogueReference(11);
	ObjectManager:AddObject(sprite, 0);
end -- function CreateCharacters()


-- Creates all enemy sprites
function CreateEnemies()

end -- function CreateEnemies()


-- Creates all dialogue that takes place through characters and events
function CreateDialogue()
	local dialogue;
	local text;

	dialogue = hoa_map.MapDialogue(10);
		text = hoa_system.Translate("Laila! I heard the city was burning?!");
		dialogue:AddLine(text, 1000);
		text = hoa_system.Translate("Well unfortunately this is as far as this release goes. There's only a partly constructed city here right now.");
		dialogue:AddLine(text, 2000);
		text = hoa_system.Translate("You can explore this area if you like. Some of the buildings here may be entered by walking up to the doors, but not all of them.");
		dialogue:AddLine(text, 2000);
		text = hoa_system.Translate("Also I would recommend that you not save your game while you are on this map, because you are stuck here with no way to go back.");
		dialogue:AddLine(text, 2000);
	DialogueManager:AddDialogue(dialogue);
	
	dialogue = hoa_map.MapDialogue(11);
		text = hoa_system.Translate("The next release should see this city in a more completed state. Once its finished, when you visit here it will be under attack by an army of demons that you will have to do battle with to save the people that live here.");
		dialogue:AddLine(text, 2000);
		text = hoa_system.Translate("Thanks for playing and see you next time!");
		dialogue:AddLine(text, 2000);
	DialogueManager:AddDialogue(dialogue);
end -- function CreateDialogue()


-- Creates all events
function CreateEvents()

end -- function CreateEvents()
