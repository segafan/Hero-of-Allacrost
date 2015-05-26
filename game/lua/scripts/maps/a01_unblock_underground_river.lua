--------------------------------------------------------------------------------
-- a01_unblock_underground_river.lua
--
-- Main storyline event. The first dungeon encountered in the game, the player
-- makes his way through a cave to a riverbed, where a boss enemy is encountered.

--------------------------------------------------------------------------------
local ns = {}
setmetatable(ns, {__index = _G})
a01_unblock_underground_river = ns;
setfenv(1, ns);

data_file = "lua/data/maps/harrvah_underground_river_cave.lua";
location_filename = "img/portraits/locations/desert_cave.png"
map_name = "River Access Cave"

sound_filenames = {};

music_filenames = {};
music_filenames[1] = "mus/Cave2.ogg";

-- Primary Map Classes
Map = {};
ObjectManager = {};
DialogueManager = {};
EventManager = {};
TreasureManager = {};
GlobalEvents = {};

enemy_ids = { 1, 2, 3, 4, 5, 6 }

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
contexts["base"] = hoa_map.MapMode.CONTEXT_01; -- Upon first entering the cave
contexts["collapsed"] = hoa_map.MapMode.CONTEXT_02; -- Active context after the passage collapse event



function Load(m)
	-- Setup global pointers for the MapMode instance as well as the various supervisors for convenience
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
	
	Map:SetCamera(sprites["claudius"]);
	sprites["claudius"]:SetMovementSpeed(hoa_map.MapMode.VERY_FAST_SPEED); -- DEBUG
	Map:MoveVirtualFocus(80, 130);

	-- Visual effects: dark lighting, various light halos
	VideoManager:EnableLightOverlay(hoa_video.Color(0.0, 0.0, 0.0, 0.7));
	
	-- DEBUG: uncomment the lines below to set the camera to locations close to testing areas
	
	-- Location: just before enemy boss
	-- Map.camera:SetXPosition(205, 0); Map.camera:SetYPosition(5, 0);

	Map:SetCurrentTrack(0);

	-- The map begins with an opening scene before control is given to the player
	Map:PushState(hoa_map.MapMode.STATE_SCENE);
	EventManager:StartEvent(events["opening_dialogue"], 2500);

	-- TEMP: enabled for development. Change this to false prior to release
	Map.unlimited_stamina = true;
	print "LOAD COMPLETE"
end -- function Load(m)



function Update()
	-- TODO: setup a scripted battle for the first enemy encounter
	--[[
	if ((zones["first_enemy_encounter"]:IsCameraEntering() == true)) then
		if (GlobalEvents:DoesEventExist("first_enemy_encounter") == false) then
			GlobalEvents:AddNewEvent("first_enemy_encounter", 1);
			Map.camera:SetMoving(false);
			EventManager:StartEvent(event_chains["first_enemy"]);
		end
	end
	--]]

	if (zones["corpse_discovery"]:IsCameraEntering() == true) then
		if (GlobalEvents:DoesEventExist("corpse_seen") == false) then
			GlobalEvents:AddNewEvent("corpse_seen", 1);
			EventManager:StartEvent(event_chains["find_corpse"]);
		end
	end

	if (zones["long_route"]:IsCameraEntering() == true and Map:CurrentState() == hoa_map.MapMode.STATE_EXPLORE) then
		if (GlobalEvents:DoesEventExist("passage_collapsed") == false) then
			EventManager:StartEvent(event_chains["go_short_route"]);
		end
	end
	
	if (zones["short_route"]:IsCameraEntering() == true) then
		if (GlobalEvents:DoesEventExist("knight_moved") == false) then
			GlobalEvents:AddNewEvent("knight_moved", 1);
			EventManager:StartEvent(event_chains["observe_passing"]);
		end
	end
	
	if (zones["collapse"]:IsCameraEntering() == true) then
		if (GlobalEvents:DoesEventExist("passage_collapsed") == false) then
			GlobalEvents:AddNewEvent("passage_collapsed", 1);
			EventManager:StartEvent(event_chains["passage_collapse"]);
		end
	end

	if ((zones["forward_passage"]:IsCameraEntering() == true) and (Map.camera:IsVisible() == true)) then
		EventManager:StartEvent(event_chains["pass_wall_forward"]);
	end
	
	if ((zones["backward_passage"]:IsCameraEntering() == true) and (Map.camera:IsVisible() == true)) then
		EventManager:StartEvent(event_chains["pass_wall_backward"]);
	end
	
	if ((zones["spring_discovery"]:IsCameraEntering() == true)) then
		if (GlobalEvents:DoesEventExist("spring_discovered") == false) then
			GlobalEvents:AddNewEvent("spring_discovered", 1);
			Map.camera:SetMoving(false);
			EventManager:StartEvent(event_chains["spring_arrival"]);
		end
	end
	
	if ((zones["riverbed_arrival"]:IsCameraEntering() == true)) then
		if (GlobalEvents:DoesEventExist("riverbed_arrival") == false) then
			GlobalEvents:AddNewEvent("riverbed_arrival", 1);
			EventManager:StartEvent(event_chains["riverbed_arrival"]);
		end
	end
end -- function Update()



function Draw()
	Map:DrawMapLayers();
end



function CreateZones()
	---------- Event Trigger Zones
	zones["first_enemy_encounter"] = hoa_map.CameraZone(6, 20, 184, 186, contexts["base"]);
	Map:AddZone(zones["first_enemy_encounter"]);

	zones["corpse_discovery"] = hoa_map.CameraZone(180, 182, 134, 140, contexts["base"] + contexts["collapsed"]);
	Map:AddZone(zones["corpse_discovery"]);

	zones["long_route"] = hoa_map.CameraZone(132, 136, 70, 80, contexts["base"]);
	Map:AddZone(zones["long_route"]);

	zones["short_route"] = hoa_map.CameraZone(156, 160, 60, 63, contexts["base"]);
	Map:AddZone(zones["short_route"]);	

	zones["collapse"] = hoa_map.CameraZone(186, 189, 60, 63, contexts["base"]);
	Map:AddZone(zones["collapse"]);	

	zones["forward_passage"] = hoa_map.CameraZone(78, 79, 6, 7, contexts["base"] + contexts["collapsed"]);
	Map:AddZone(zones["forward_passage"]);

	zones["backward_passage"] = hoa_map.CameraZone(110, 111, 20, 21, contexts["base"] + contexts["collapsed"]);
	Map:AddZone(zones["backward_passage"]);

	zones["spring_discovery"] = hoa_map.CameraZone(171, 186, 5, 10, contexts["base"] + contexts["collapsed"]);
	Map:AddZone(zones["spring_discovery"]);

	zones["riverbed_arrival"] = hoa_map.CameraZone(220, 221, 2, 11, contexts["base"] + contexts["collapsed"]);
	Map:AddZone(zones["riverbed_arrival"]);

	---------- Enemy Zones
	-- Zone #01: Near entrance
	zones["enemy01"] = hoa_map.EnemyZone(26, 79, 130, 141);
	Map:AddZone(zones["enemy01"]);

	-- Zone #02: Along narrow southern passage between pit and wall
	zones["enemy02"] = hoa_map.EnemyZone(108, 140, 145, 148);
	Map:AddZone(zones["enemy02"]);
	
	-- Zone #03: Near ceiling overpass entrance and around corpse
	zones["enemy03"] = hoa_map.EnemyZone(158, 167, 130, 145);
	zones["enemy03"]:AddSection(168, 215, 134, 142);
	Map:AddZone(zones["enemy03"]);
	
	-- Zone #04: In ceiling overpass
	zones["enemy04"] = hoa_map.EnemyZone(146, 157, 90, 113);
	Map:AddZone(zones["enemy04"]);
	
	-- Zone #05: Wide open area at beginning of long route
	zones["enemy05"] = hoa_map.EnemyZone(26, 115, 68, 89);
	zones["enemy05"]:AddSection(4, 72, 25, 83);
	Map:AddZone(zones["enemy05"]);
	
	-- Zone #06: Above pits and before wall passage
	zones["enemy06"] = hoa_map.EnemyZone(24, 89, 12, 47);
	zones["enemy06"]:AddSection(24, 73, 6, 12);
	Map:AddZone(zones["enemy06"]);
	
	-- Zone #07: Through wall passage and before spring
	zones["enemy07"] = hoa_map.EnemyZone(116, 143, 6, 29);
	zones["enemy07"]:AddSection(144, 167, 6, 13);
	Map:AddZone(zones["enemy07"]);
end -- function CreateZones()



function CreateObjects()
	local object = {};
	local treasure = {};

	-- TODO: Move this treasure deeper into the cave instead of near the entrance
	object = hoa_map.TreasureObject("img/misc/chest1.png", 4, 1, 1);
	object:SetObjectID(1001);
	object:SetXPosition(35, 0);
	object:SetYPosition(6, 0);
	treasure = object:GetTreasure();
	treasure:AddDrunes(75);
	treasure:AddObject(1, 2);
	ObjectManager:AddObject(object, 0);
end



function CreateSprites()
	----------------------------------------------------------------------------
	---------- Create character party sprites
	----------------------------------------------------------------------------
	-- Group #1: Playable character sprites
	sprites["claudius"] = ConstructSprite("Claudius", 1, 11, 227);
	sprites["claudius"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["claudius"]:SetNoCollision(true);
	ObjectManager:AddObject(sprites["claudius"], 0);

	sprites["mark"] = ConstructSprite("Knight01", 2, 17, 227);
	sprites["mark"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["mark"]:SetNoCollision(true);
	sprites["mark"]:SetName(hoa_system.Translate("Mark"));
	ObjectManager:AddObject(sprites["mark"], 0);
	
	sprites["lukar"] = ConstructSprite("Knight01", 3, 14, 225);
	sprites["lukar"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["lukar"]:SetNoCollision(true);
	sprites["lukar"]:SetName(hoa_system.Translate("Lukar"));
	ObjectManager:AddObject(sprites["lukar"], 0);

	----------------------------------------------------------------------------
	---------- Create NPCs in roughly the order encountered by the player
	----------------------------------------------------------------------------
	-- Knight near cave entrance serving as a guide
	sprites["entrance_knight"] = ConstructSprite("Knight01", 10, 14, 148);
	sprites["entrance_knight"]:SetDirection(hoa_map.MapMode.SOUTH);
	ObjectManager:AddObject(sprites["entrance_knight"], 0);
	
	-- Knight guiding others through the short passage
	sprites["passage_knight1"] = ConstructSprite("Knight03", 20, 149, 62);
	sprites["passage_knight1"]:SetDirection(hoa_map.MapMode.SOUTH);
	ObjectManager:AddObject(sprites["passage_knight1"], 0);
	
	-- Knight seen walking ahead through the short passage
	sprites["passage_knight2"] = ConstructSprite("Knight02", 21, 162, 62);
	sprites["passage_knight2"]:SetDirection(hoa_map.MapMode.EAST);
	ObjectManager:AddObject(sprites["passage_knight2"], 0);
	
	-- Knight with injured ankle near the end of the long passage
	sprites["injury_knight1"] = ConstructSprite("Knight03", 30, 142, 30);
	sprites["injury_knight1"]:SetDirection(hoa_map.MapMode.SOUTH);
	ObjectManager:AddObject(sprites["injury_knight1"], 0);
	
	-- Knight assisting the injured knight
	sprites["injury_knight2"] = ConstructSprite("Knight01", 31, 142, 33);
	sprites["injury_knight2"]:SetDirection(hoa_map.MapMode.NORTH);
	ObjectManager:AddObject(sprites["injury_knight2"], 0);

	-- All of the following NPCs are encountered at the end of the cave in the riverbed
	sprites["captain"] = ConstructSprite("Knight06", 2500, 248, 16);
	sprites["captain"]:SetDirection(hoa_map.MapMode.WEST);
	ObjectManager:AddObject(sprites["captain"], 0);

	sprites["sergeant"] = ConstructSprite("Knight05", 2501, 249, 19);
	sprites["sergeant"]:SetDirection(hoa_map.MapMode.WEST);
	ObjectManager:AddObject(sprites["sergeant"], 0);
	
	sprites["river_knight1"] = ConstructSprite("Knight02", 2502, 245, 11);
	sprites["river_knight1"]:SetDirection(hoa_map.MapMode.SOUTH);
	ObjectManager:AddObject(sprites["river_knight1"], 0);
	
	sprites["river_knight2"] = ConstructSprite("Knight03", 2503, 242, 8);
	sprites["river_knight2"]:SetDirection(hoa_map.MapMode.SOUTH);
	ObjectManager:AddObject(sprites["river_knight2"], 0);
	
	sprites["river_knight3"] = ConstructSprite("Knight02", 2504, 239, 9);
	sprites["river_knight3"]:SetDirection(hoa_map.MapMode.SOUTH);
	ObjectManager:AddObject(sprites["river_knight3"], 0);

	sprites["river_knight4"] = ConstructSprite("Knight01", 2505, 240, 22);
	sprites["river_knight4"]:SetDirection(hoa_map.MapMode.NORTH);
	ObjectManager:AddObject(sprites["river_knight4"], 0);

	sprites["river_knight5"] = ConstructSprite("Knight02", 2506, 243, 23);
	sprites["river_knight5"]:SetDirection(hoa_map.MapMode.NORTH);
	ObjectManager:AddObject(sprites["river_knight5"], 0);
	
	sprites["river_knight6"] = ConstructSprite("Knight03", 2507, 245, 21);
	sprites["river_knight6"]:SetDirection(hoa_map.MapMode.NORTH);
	ObjectManager:AddObject(sprites["river_knight6"], 0);
	
	sprites["river_knight7"] = ConstructSprite("Knight02", 2508, 234, 20);
	sprites["river_knight7"]:SetDirection(hoa_map.MapMode.EAST);
	ObjectManager:AddObject(sprites["river_knight7"], 0);
	
	sprites["river_knight8"] = ConstructSprite("Knight01", 2509, 233, 17);
	sprites["river_knight8"]:SetDirection(hoa_map.MapMode.EAST);
	ObjectManager:AddObject(sprites["river_knight8"], 0);
	
	sprites["river_knight9"] = ConstructSprite("Knight02", 2510, 235, 14);
	sprites["river_knight9"]:SetDirection(hoa_map.MapMode.EAST);
	ObjectManager:AddObject(sprites["river_knight9"], 0);
end -- function CreateSprites()



function CreateEnemies()
	-- Sets common battle environment settings for enemy sprites
	local SetBattleEnvironment = function(enemy)
		enemy:SetBattleMusicTheme("mus/Battle_Jazz.ogg");
		enemy:SetBattleBackground("img/backdrops/battle/desert_cave.png");
		enemy:SetBattleScript("lua/scripts/battles/first_battle.lua");
	end

	local enemy = {};

	---------- Create enemy sprites and adds them to the zones that they spawn/roam in
	-- Group #01
	enemy = ConstructEnemySprite("slime", Map);
	SetBattleEnvironment(enemy);
	enemy:NewEnemyParty();
	enemy:AddEnemy(1);
	enemy:AddEnemy(1);
	enemy:AddEnemy(1);
	enemy:NewEnemyParty();
	enemy:AddEnemy(1);
	enemy:AddEnemy(2);
	enemy:NewEnemyParty();
	enemy:AddEnemy(1);
	enemy:AddEnemy(6);
	zones["enemy01"]:AddEnemy(enemy, Map, 1);

	-- Group #02
	SetBattleEnvironment(enemy);
	enemy:NewEnemyParty();
	enemy:AddEnemy(3);
	enemy:AddEnemy(2);
	enemy:AddEnemy(3);
	enemy:NewEnemyParty();
	enemy:AddEnemy(6);
	enemy:AddEnemy(3);
	enemy:AddEnemy(6);
	zones["enemy02"]:AddEnemy(enemy, Map, 1);
	
	-- Group #03
	enemy = ConstructEnemySprite("scorpion", Map);
	SetBattleEnvironment(enemy);
	enemy:NewEnemyParty();
	enemy:AddEnemy(5);
	enemy:AddEnemy(5);
	enemy:NewEnemyParty();
	enemy:AddEnemy(5);
	enemy:AddEnemy(2);
	enemy:AddEnemy(5);
	enemy:NewEnemyParty();
	enemy:AddEnemy(5);
	enemy:AddEnemy(1);
	enemy:AddEnemy(1);
	enemy:AddEnemy(1);
	zones["enemy03"]:AddEnemy(enemy, Map, 1);

	-- Group #04
	enemy = ConstructEnemySprite("snake", Map);
	SetBattleEnvironment(enemy);
	enemy:NewEnemyParty();
	enemy:AddEnemy(6);
	enemy:AddEnemy(3);
	enemy:AddEnemy(2);
	enemy:AddEnemy(3);
	enemy:AddEnemy(6);
	enemy:NewEnemyParty();
	enemy:AddEnemy(6);
	enemy:AddEnemy(6);
	enemy:AddEnemy(6);
	enemy:AddEnemy(6);
	zones["enemy04"]:AddEnemy(enemy, Map, 1);
	
	-- Group #05
	enemy = ConstructEnemySprite("slime", Map);
	SetBattleEnvironment(enemy);
	enemy:NewEnemyParty();
	enemy:AddEnemy(1);
	enemy:AddEnemy(1);
	enemy:AddEnemy(1);
	enemy:NewEnemyParty();
	enemy:AddEnemy(1);
	enemy:AddEnemy(2);
	enemy:NewEnemyParty();
	enemy:AddEnemy(1);
	enemy:AddEnemy(6);
	zones["enemy05"]:AddEnemy(enemy, Map, 1);

	enemy = ConstructEnemySprite("snake", Map);
	SetBattleEnvironment(enemy);
	enemy:NewEnemyParty();
	enemy:AddEnemy(6);
	enemy:AddEnemy(3);
	enemy:AddEnemy(2);
	enemy:AddEnemy(3);
	enemy:AddEnemy(6);
	enemy:NewEnemyParty();
	enemy:AddEnemy(6);
	enemy:AddEnemy(6);
	enemy:AddEnemy(6);
	enemy:AddEnemy(6);
	zones["enemy05"]:AddEnemy(enemy, Map, 1);

	enemy = ConstructEnemySprite("scorpion", Map);
	SetBattleEnvironment(enemy);
	enemy:NewEnemyParty();
	enemy:AddEnemy(5);
	enemy:AddEnemy(5);
	enemy:NewEnemyParty();
	enemy:AddEnemy(5);
	enemy:AddEnemy(2);
	enemy:AddEnemy(5);
	enemy:NewEnemyParty();
	enemy:AddEnemy(5);
	enemy:AddEnemy(1);
	enemy:AddEnemy(1);
	enemy:AddEnemy(1);
	zones["enemy05"]:AddEnemy(enemy, Map, 1);
	
	-- Group #06
	enemy = ConstructEnemySprite("scorpion", Map);
	SetBattleEnvironment(enemy);
	enemy:NewEnemyParty();
	enemy:AddEnemy(5);
	enemy:AddEnemy(5);
	enemy:NewEnemyParty();
	enemy:AddEnemy(5);
	enemy:AddEnemy(2);
	enemy:AddEnemy(5);
	enemy:NewEnemyParty();
	enemy:AddEnemy(5);
	enemy:AddEnemy(5);
	enemy:AddEnemy(6);
	enemy:AddEnemy(6);
	zones["enemy06"]:AddEnemy(enemy, Map, 1);

	-- Group #07
	enemy = ConstructEnemySprite("snake", Map);
	SetBattleEnvironment(enemy);
	enemy:NewEnemyParty();
	enemy:AddEnemy(6);
	enemy:AddEnemy(3);
	enemy:AddEnemy(2);
	enemy:AddEnemy(3);
	enemy:AddEnemy(6);
	enemy:NewEnemyParty();
	enemy:AddEnemy(6);
	enemy:AddEnemy(6);
	enemy:AddEnemy(6);
	enemy:AddEnemy(6);
	zones["enemy07"]:AddEnemy(enemy, Map, 1);

	enemy = ConstructEnemySprite("scorpion", Map);
	SetBattleEnvironment(enemy);
	enemy:NewEnemyParty();
	enemy:AddEnemy(5);
	enemy:AddEnemy(5);
	enemy:NewEnemyParty();
	enemy:AddEnemy(5);
	enemy:AddEnemy(2);
	enemy:AddEnemy(5);
	enemy:NewEnemyParty();
	enemy:AddEnemy(5);
	enemy:AddEnemy(1);
	enemy:AddEnemy(1);
	enemy:AddEnemy(1);
	zones["enemy07"]:AddEnemy(enemy, Map, 1);
end -- function CreateEnemies()



function CreateDialogues()
	event_dialogues = {}; -- Holds IDs of the dialogues used during events

	local dialogue;
	local text;

	----------------------------------------------------------------------------
	---------- Dialogues attached to characters
	----------------------------------------------------------------------------
	--[[ TODO: add scripting support to place down a chest by knight's feet. This dialogue should only be read once.
	dialogue = hoa_map.MapDialogue.Create(10);
		text = hoa_system.Translate("Here, take the contents of this chest. Use these items to heal yourself if you become injured.");
		dialogue:AddLine(text, sprites["entrance_knight"]:GetObjectID());
		dialogue:AddLineEventAtStart(1000, 500);
	sprites["entrance_knight"]:AddDialogueReference(10);
	--]]

	dialogue = hoa_map.MapDialogue.Create(11);
		text = hoa_system.Translate("Watch your step and keep moving. It's not far to the river bed.");
		dialogue:AddLine(text, sprites["entrance_knight"]:GetObjectID());
	sprites["entrance_knight"]:AddDialogueReference(11);

	dialogue = hoa_map.MapDialogue.Create(20);
		text = hoa_system.Translate("The river bed is just through this passage. Be careful, the walls are a little unstable.");
		dialogue:AddLine(text, sprites["passage_knight1"]:GetObjectID());
	sprites["passage_knight1"]:AddDialogueReference(20);

	-- After the passage collapse event, this dialogue will be added to the passage_knight1 sprite
	dialogue = hoa_map.MapDialogue.Create(21);
		text = hoa_system.Translate("I'll direct everyone remaining to take the longer route.");
		dialogue:AddLine(text, sprites["passage_knight1"]:GetObjectID());

	dialogue = hoa_map.MapDialogue.Create(30);
		text = hoa_system.Translate("Dammit, ow ow ow...");
		dialogue:AddLine(text, sprites["injury_knight1"]:GetObjectID());
	sprites["injury_knight1"]:AddDialogueReference(30);

	dialogue = hoa_map.MapDialogue.Create(31);
		text = hoa_system.Translate("He sprained his ankle on a loose rock, so we're treating his injury. Move on ahead, you're almost there.");
		dialogue:AddLine(text, sprites["injury_knight2"]:GetObjectID());
	sprites["injury_knight2"]:AddDialogueReference(31);

	----------------------------------------------------------------------------
	---------- Dialogues triggered by events
	----------------------------------------------------------------------------
	-- Event: Entering the cave
	event_dialogues["entrance1"] = 100;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["entrance1"]);
		text = hoa_system.Translate("Claudius, I want you to lead us down to the riverbed.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());

	event_dialogues["entrance2"] = 101;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["entrance2"]);
		text = hoa_system.Translate("Wait a damn minute Lukar! Why are you putting a rookie like him in charge?");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());

	event_dialogues["entrance3"] = 102;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["entrance3"]);
		text = hoa_system.Translate("Relax, Mark. This is the best way for him to get experience.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());

	event_dialogues["entrance4"] = 103;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["entrance4"]);
		text = hoa_system.Translate("Claudius, I realize that this is your first real mission as a knight. If you're not up to this task, that's okay.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("No, I can do it. I won't lead us astray.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("Good. The path we're taking should be pretty straight forward, so don't worry about getting lost.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("Tcsh. Just try not to get us all killed, okay rookie?");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());

	-- Event: First battle encounter
	event_dialogues["first_enemy_encounter1"] = 110;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["first_enemy_encounter1"]);
		text = hoa_system.Translate("Wait, look up ahead. There's an enemy in our way.");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());

	event_dialogues["first_enemy_encounter2"] = 111;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["first_enemy_encounter2"]);
		text = hoa_system.Translate("A battle occurs whenever you collide with an enemy. Sometimes you can avoid a fight by sneaking past or running by an enemy before it has a chance to engage you. When you enter a battle you can no longer run away and must defeat your opponent.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("That enemy doesn't appear to be much of a threat, so let it engage us. I want to make sure you remember how a knight does battle.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("Let's see how useful you are in a real fight, rookie.");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());

	-- Event: After first battle victory
	event_dialogues["first_enemy_encounter3"] = 120;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["first_enemy_encounter3"]);
		text = hoa_system.Translate("Nicely done. After a battle ends, you'll have a short moment of invulnerability to get away from any other enemies that may be roaming nearby. If you are surrounded by multiple foes when a battle begins, be ready to make a break for it as soon as the battle ends if you don't want to keep fighting.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("Or you can not get yourself surrounded in the first place like a dumbass.");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());
		text = hoa_system.Translate("Well, Mark is correct. Even though he could have phrased that better.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("You must also be careful as enemies will re-spawn after a short period of time, so don't sit around thinking that you're out of danger. Other times an enemy that is defeated will not appear again.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());

	event_dialogues["first_enemy_encounter4"] = 121;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["first_enemy_encounter4"]);
		text = hoa_system.Translate("Ah, one more thing. You can access the party menu by pressing the [MENU] key while on a map. In the party menu you can heal your characters, change out your equipment, and manage your inventory.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());

	-- Event: Encountering first NPCs in the cave
	event_dialogues["first_npc_encounter"] = 130;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["first_npc_encounter"]);
		text = hoa_system.Translate("Hold. There's a friendly ahead. Characters that have information to share will have a small icon appear above them that gradually appears as you get closer. Stand facing the character and hit the [CONFIRM] key to hear what they have to say.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("Keep in mind that a character may have more than one piece of information to share, or may have something new to say after a particular event has occurred. The icon will look differently if the character has dialogue that you have not seen before.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("So in other words, I should keep initiating a conversation with these characters until the new dialogue icon no longer appears above their head.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("Look out, boys. We got a regular genius over here. Maybe you should have become a scholar instead of a knight.");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());

	-- Event: Discovery of corpse in south east part of cave
	event_dialogues["corpse_discovery1"] = 140;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["corpse_discovery1"]);
		text = hoa_system.Translate("Wait, look over there.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());

	event_dialogues["corpse_discovery2"] = 141;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["corpse_discovery2"]);
		text = hoa_system.Translate("A corpse. That's always a reassuring find in a place like this.");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());
		text = hoa_system.Translate("Hey, I think I see something under its hand.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());

	-- Event: Player tries to go long route before short route
	event_dialogues["prevent_long_route"] = 150;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["prevent_long_route"]);
		text = hoa_system.Translate("Hey! Over here!");
		dialogue:AddLine(text, sprites["passage_knight1"]:GetObjectID());
	
	-- Event: As passage is collapsing
	event_dialogues["passage_collapse1"] = 160;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["passage_collapse1"]);
		text = hoa_system.Translate("Look out!");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		dialogue:AddLineTiming(1000);

	-- Event: After passage collapse occurs
	event_dialogues["passage_collapse2"] = 161;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["passage_collapse2"]);
		text = hoa_system.Translate("Woah! Are you guys alright?");
		dialogue:AddLine(text, sprites["passage_knight1"]:GetObjectID());
		text = hoa_system.Translate("We're all fine. But the passage has caved in.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("There's another way around, although it's a longer route. Follow me.");
		dialogue:AddLine(text, sprites["passage_knight1"]:GetObjectID());

	-- Event: Stopping player from trying to proceed to lower levels of the cave
	event_dialogues["prevent_level_descent"] = 170;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["prevent_level_descent"]);
		text = hoa_system.Translate("Wait. This path looks like it leads deeper into the cavern. The area we're trying to reach isn't down there. Let's head another direction.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());
		text = hoa_system.Translate("Good. Who knows what could be lurking in the darkness down there.");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());

	-- Event: While crossing the narrow bridge between the two pits after hearing an evil hiss
	event_dialogues["hiss_sound"] = 180;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["hiss_sound"]);
		text = hoa_system.Translate("Did you hear that? What the hell was that sound?");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("I don't know, but I've got a bad feeling about this mission.");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());
		text = hoa_system.Translate("Well the sooner we achieve our objective, the sooner we get out of here and go home. So move your ass instead of your mouth.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());

	-- Event: Encountering the pool of running water near the end of the cave
	event_dialogues["spring_arrival"] = 190;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["spring_arrival"]);
		text = hoa_system.Translate("Hey check it out. The water is still running here.");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());
		text = hoa_system.Translate("That's a good sign. The river obstruction must be close.");
		dialogue:AddLine(text, sprites["lukar"]:GetObjectID());

	-- Event: Player reaches dry river bed
	event_dialogues["riverbed_arrival1"] = 200;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["riverbed_arrival1"]);
		text = hoa_system.Translate("Finally made it.");
		dialogue:AddLine(text, sprites["mark"]:GetObjectID());

	event_dialogues["riverbed_arrival2"] = 201;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["riverbed_arrival2"]);
		text = hoa_system.Translate("Listen up! There's a large boulder obstructing the underground river that flows through here. When we move it aside, we get to head out of here.");
		dialogue:AddLine(text, sprites["captain"]:GetObjectID());
		text = hoa_system.Translate("Mikal! Torren! Take your units and secure the ropes around that overgrown rock. Jasper's unit will prepare the Maks to help us move it. The rest of you stay alert and watch our backs. Who knows what the hell may be in this cave with us.");
		dialogue:AddLine(text, sprites["sergeant"]:GetObjectID());

	-- Event: Before boss battle
	event_dialogues["before_boss"] = 202;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["before_boss"]);
		text = hoa_system.Translate("Hey, I heard that noise earlier. It sounds like its closer now.");
		dialogue:AddLine(text, sprites["river_knight1"]:GetObjectID());
		text = hoa_system.Translate("Keep your eyes peeled and your swords ready men.");
		dialogue:AddLine(text, sprites["river_knight2"]:GetObjectID());
		text = hoa_system.Translate("I don't know how you expect to see shit in here. I can barely see my own hand.");
		dialogue:AddLine(text, sprites["river_knight3"]:GetObjectID());
		text = hoa_system.Translate("Over there! Watch out!");
		dialogue:AddLine(text, sprites["river_knight4"]:GetObjectID());

	-- Event: After boss battle
	event_dialogues["after_boss"] = 210;
	dialogue = hoa_map.MapDialogue.Create(event_dialogues["after_boss"]);
		text = hoa_system.Translate("Damnit, the captain's been wounded along with half our troops.");
		dialogue:AddLine(text, sprites["claudius"]:GetObjectID());
		text = hoa_system.Translate("*cough cough*\nI'll be alright. Great job taking down that monster men, I'm proud.");
		dialogue:AddLine(text, sprites["captain"]:GetObjectID());
		text = hoa_system.Translate("We've achieved our objective here. Tend to the wounded and then let's make our way back home.");
		dialogue:AddLine(text, sprites["captain"]:GetObjectID());
end -- function CreateDialogues()



function CreateEvents()
	event_chains = {}; -- Holds IDs of the starting event for each event chain
	local event = {};

	---------- Event Chain 01: Initial scene and 4-part dialogue when the player first enters the cave
	print "Event Chain #01";
	event_chains["entrance"] = 10;

	-- Part #1: Lukar turns around and asks Cladius to lead
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["entrance"] + 0, 1, 0, -16);
	events["opening_dialogue"] = event;
	event:SetRelativeDestination(true);
	event:AddEventLinkAtStart(event_chains["entrance"] + 1);
	event:AddEventLinkAtStart(event_chains["entrance"] + 2);
	event:AddEventLinkAtEnd(event_chains["entrance"] + 3, 1000);
	event:AddEventLinkAtEnd(event_chains["entrance"] + 4, 1500);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["entrance"] + 1, 2, 0, -16);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["entrance"] + 2, 3, 0, -16);
	event:SetRelativeDestination(true);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(event_chains["entrance"] + 3, 3, hoa_map.MapMode.SOUTH);
	event = hoa_map.DialogueEvent.Create(event_chains["entrance"] + 4, event_dialogues["entrance1"]);
	event:AddEventLinkAtEnd(event_chains["entrance"] + 5, 250);
	event:AddEventLinkAtEnd(event_chains["entrance"] + 6, 500);
	event:AddEventLinkAtEnd(event_chains["entrance"] + 7, 500);
	-- Part #2: Mark protests Lukar's decision
	event = hoa_map.ChangeDirectionSpriteEvent.Create(event_chains["entrance"] + 5, 2, hoa_map.MapMode.WEST);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(event_chains["entrance"] + 6, 1, hoa_map.MapMode.EAST);
	event = hoa_map.DialogueEvent.Create(event_chains["entrance"] + 7, event_dialogues["entrance2"]);
	event:AddEventLinkAtEnd(18, 200);
	-- Part #3: Lukar reassures Mark
	event = hoa_map.ChangeDirectionSpriteEvent.Create(event_chains["entrance"] + 8, 3, hoa_map.MapMode.EAST);
	event:AddEventLinkAtEnd(event_chains["entrance"] + 9, 100);
	event = hoa_map.DialogueEvent.Create(event_chains["entrance"] + 9, event_dialogues["entrance3"]);
	event:AddEventLinkAtEnd(event_chains["entrance"] + 10, 300);
	-- Part #4: Lukar asks Claudius again, who accepts. Mark and Lukar's sprites disappear into Claudius
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["entrance"] + 10, 3, -5, 0);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(event_chains["entrance"] + 11);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(event_chains["entrance"] + 11, 3, hoa_map.MapMode.SOUTH);
	event:AddEventLinkAtEnd(event_chains["entrance"] + 12, 100);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(event_chains["entrance"] + 12, 1, hoa_map.MapMode.NORTH);
	event:AddEventLinkAtEnd(event_chains["entrance"] + 13, 100);
	event = hoa_map.DialogueEvent.Create(event_chains["entrance"] + 13, event_dialogues["entrance4"]);
	event:AddEventLinkAtEnd(event_chains["entrance"] + 14, 100);
	event:AddEventLinkAtEnd(event_chains["entrance"] + 15, 100);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["entrance"] + 14, 2, -8, 0);
	event:AddEventLinkAtEnd(event_chains["entrance"] + 16);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["entrance"] + 15, 3, 0, 4);
	event:SetRelativeDestination(true);
	event = hoa_map.CustomEvent.Create(event_chains["entrance"] + 16, "EndOpeningScene", "");
--	event:AddEventLinkAtEnd(event_chains["entrance"] + 17);
	-- Part #5: TODO: add a dialogue that explains the controls to the player
--	event = hoa_map.DialogueEvent.Create(event_chains["entrance"] + 17, );

	---------- Event Chain 02: First enemy encounter
	print "Event Chain #02";
	event_chains["first_enemy"] = 30;

	event = hoa_map.DialogueEvent.Create(event_chains["first_enemy"] + 0, event_dialogues["first_enemy_encounter1"]);
	event:AddEventLinkAtStart(event_chains["first_enemy"] + 1);
	event = hoa_map.CustomEvent.Create(event_chains["first_enemy"] + 1, "SpawnFirstEnemy", "");
	event:AddEventLinkAtEnd(event_chains["first_enemy"] + 2, 500);
	event = hoa_map.DialogueEvent.Create(event_chains["first_enemy"] + 2, event_dialogues["first_enemy_encounter2"]);
	event:AddEventLinkAtEnd(event_chains["first_enemy"] + 3);
	event = hoa_map.CustomEvent.Create(event_chains["first_enemy"] + 3, "EngageFirstEnemy", "");
	event:AddEventLinkAtEnd(event_chains["first_enemy"] + 4);
	-- Follow-up dialogue begins immediately after the battle ends
	event = hoa_map.DialogueEvent.Create(event_chains["first_enemy"] + 4, event_dialogues["first_enemy_encounter3"]);
	event:AddEventLinkAtEnd(event_chains["first_enemy"] + 5, 2000);
	event = hoa_map.DialogueEvent.Create(event_chains["first_enemy"] + 5, event_dialogues["first_enemy_encounter4"]);

	---------- Event Chain 03: Discovery of corpse in cave
	print "Event Chain #03";
	event_chains["find_corpse"] = 40;

	-- Dialog when seeing the corpse
	event = hoa_map.DialogueEvent.Create(event_chains["find_corpse"] + 0, event_dialogues["corpse_discovery1"]);
	event:SetStopCameraMovement(true);
	event:AddEventLinkAtEnd(event_chains["find_corpse"] + 1);
	-- Move camera to corpse
	event = hoa_map.CustomEvent.Create(event_chains["find_corpse"] + 1, "CameraPanToCorpse", "");
	event:AddEventLinkAtEnd(event_chains["find_corpse"] + 2, 3000);
	-- Walk Claudius over to corpse
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["find_corpse"] + 2, sprites["claudius"], 206, 149);
	event:AddEventLinkAtEnd(event_chains["find_corpse"] + 3);
	event:AddEventLinkAtEnd(event_chains["find_corpse"] + 4);
	event:AddEventLinkAtEnd(event_chains["find_corpse"] + 5);
	-- Face Claudius east toward the corpse
	event = hoa_map.ChangeDirectionSpriteEvent.Create(event_chains["find_corpse"] + 3, sprites["claudius"], hoa_map.MapMode.EAST);
	-- Move camera back to Cladius
	event = hoa_map.CustomEvent.Create(event_chains["find_corpse"] + 4, "SetCameraToPlayer", "");
--	event:AddEventLinkAtEnd(event_chains["find_corpse"] + 5);
	-- Start dialogue about corpse
	event = hoa_map.DialogueEvent.Create(event_chains["find_corpse"] + 5, event_dialogues["corpse_discovery2"]);
	event:SetStopCameraMovement(true);
	event:AddEventLinkAtEnd(event_chains["find_corpse"] + 6);
	-- Add treasure
	event = hoa_map.CustomEvent.Create(event_chains["find_corpse"] + 6, "RewardPotion", "");

	---------- Event Chain 04: Prevent player from going long route before cave collapse
	print "Event Chain #04";
	event_chains["go_short_route"] = 50;

	-- Enter scene state
	event = hoa_map.CustomEvent.Create(event_chains["go_short_route"] + 0, "StopMovementAndEnterScene", "");
	event:AddEventLinkAtEnd(event_chains["go_short_route"] + 1);
	-- Move camera to knight sprite
	event = hoa_map.CustomEvent.Create(event_chains["go_short_route"] + 1, "CameraToGuideSprite", "");
	event:AddEventLinkAtEnd(event_chains["go_short_route"] + 2, 1000);
	-- Throw up dialogue calling out player's party
	event = hoa_map.DialogueEvent.Create(event_chains["go_short_route"] + 2, event_dialogues["prevent_long_route"]);
	event:SetStopCameraMovement(true);
	event:AddEventLinkAtEnd(event_chains["go_short_route"] + 3);
	-- Move camera back to Cladius
	event = hoa_map.CustomEvent.Create(event_chains["go_short_route"] + 3, "SetCameraToPlayer", "");
	event:AddEventLinkAtEnd(event_chains["go_short_route"] + 4, 500);
	-- Move player sprite to NPC that called out
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["go_short_route"] + 4, sprites["claudius"]:GetObjectID(), 150, 64);
	event:AddEventLinkAtEnd(event_chains["go_short_route"] + 5);
	-- Exit scene state
	event = hoa_map.CustomEvent.Create(event_chains["go_short_route"] + 5, "PopMapState", "");
		
	---------- Event Chain 05: Knight moves safely through short route while player watches
	print "Event Chain #05";
	event_chains["observe_passing"] = 60;

	-- Enter scene state
	event = hoa_map.CustomEvent.Create(event_chains["observe_passing"] + 0, "StopMovementAndEnterScene", "");
	event:AddEventLinkAtEnd(event_chains["observe_passing"] + 1);		
	-- Move camera to knight sprite
	event = hoa_map.CustomEvent.Create(event_chains["observe_passing"] + 1, "CameraFollowPathSprite", "");
	event:AddEventLinkAtEnd(event_chains["observe_passing"] + 2, 300);
	-- Move knight sprite down passage
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["observe_passing"] + 2, sprites["passage_knight2"], 210, 61);
	--event:AddEventLinkAtStart(event_chains["observe_passing"] + 2);
	event:AddEventLinkAtStart(event_chains["observe_passing"] + 3, 2000);
	event:AddEventLinkAtStart(event_chains["observe_passing"] + 4, 3000);
	event:AddEventLinkAtEnd(event_chains["observe_passing"] + 5);
	-- Move camera back
	event = hoa_map.CustomEvent.Create(event_chains["observe_passing"] + 3, "SetCameraToPlayer", "");
	-- Exit scene state
	event = hoa_map.CustomEvent.Create(event_chains["observe_passing"] + 4, "PopMapState", "");
	-- Move knight sprite to river bed area
	event = hoa_map.CustomEvent.Create(event_chains["observe_passing"] + 5, "VanishPathSprite", "");

	---------- Event Chain 06: Short route passage collapses
	print "Event Chain #06";
	event_chains["passage_collapse"] = 70;

	-- Enter scene state
	event = hoa_map.CustomEvent.Create(event_chains["passage_collapse"] + 0, "StopMovementAndEnterScene", "");
	event:AddEventLinkAtStart(event_chains["passage_collapse"] + 1);
	-- Play collapse sound
	event = hoa_map.SoundEvent.Create(event_chains["passage_collapse"] + 1, "snd/cave-in.ogg");	
	event:AddEventLinkAtStart(event_chains["passage_collapse"] + 2, 250); 
	-- Warning message
	event = hoa_map.DialogueEvent.Create(event_chains["passage_collapse"] + 2, event_dialogues["passage_collapse1"]);
	event:AddEventLinkAtEnd(event_chains["passage_collapse"] + 3);
	-- Shake the screen
	event = hoa_map.CustomEvent.Create(event_chains["passage_collapse"] + 3, "ShakeScreen", "");
	event:AddEventLinkAtEnd(event_chains["passage_collapse"] + 4);
	-- Fade screen to black
	event = hoa_map.CustomEvent.Create(event_chains["passage_collapse"] + 4, "FadeOutScreen", "IsScreenFading");
	event:AddEventLinkAtEnd(event_chains["passage_collapse"] + 5);
	-- Change all objects to context "passage collapsed"
	event = hoa_map.CustomEvent.Create(event_chains["passage_collapse"] + 5, "SwitchContextForAllSprites", "");
	event:AddEventLinkAtEnd(event_chains["passage_collapse"] + 6);
	-- Fade screen back in
	event = hoa_map.CustomEvent.Create(event_chains["passage_collapse"] + 6, "FadeInScreen", "IsScreenFading");
	event:AddEventLinkAtEnd(event_chains["passage_collapse"] + 7);
	-- Dialogue after passage has collapsed		
	event = hoa_map.DialogueEvent.Create(event_chains["passage_collapse"] + 7, event_dialogues["passage_collapse2"]);
	event:AddEventLinkAtEnd(event_chains["passage_collapse"] + 8);
	-- Change dialogue of sprite guide
	event = hoa_map.CustomEvent.Create(event_chains["passage_collapse"] + 8, "ReplaceGuideDialogue", "");
	event:AddEventLinkAtEnd(event_chains["passage_collapse"] + 9);
	-- Exit scene state
	event = hoa_map.CustomEvent.Create(event_chains["passage_collapse"] + 9, "PopMapState", "");
		
	----------  Event Chain 07: Moving forward through wall passage
	print "Event Chain #07";
	event_chains["pass_wall_forward"] = 80;
	
	-- Make player sprite invisible with no collision detection
	event = hoa_map.CustomEvent.Create(event_chains["pass_wall_forward"] + 0, "HideCameraSprite", "");
	event:AddEventLinkAtEnd(event_chains["pass_wall_forward"] + 1);
	-- Move camera inside of wall
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["pass_wall_forward"] + 1, sprites["claudius"], 85, 6);
	event:AddEventLinkAtEnd(event_chains["pass_wall_forward"] + 2);
	-- Move camera down and to the right near wall passage exit
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["pass_wall_forward"] + 2, sprites["claudius"], 104, 22);
	event:AddEventLinkAtEnd(event_chains["pass_wall_forward"] + 3);
	-- Move sprite back outside of wall
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["pass_wall_forward"] + 3, sprites["claudius"], 112, 21);
	event:AddEventLinkAtEnd(event_chains["pass_wall_forward"] + 4);
	-- Make player sprite visible and restore collision detection
	event = hoa_map.CustomEvent.Create(event_chains["pass_wall_forward"] + 4, "ShowCameraSprite", "");
		
	---------- Event Chain 08: Moving backward through wall passage
	print "Event Chain #08";
	event_chains["pass_wall_backward"] = 90;

	-- Make player sprite invisible with no collision detection
	event = hoa_map.CustomEvent.Create(event_chains["pass_wall_backward"] + 0, "HideCameraSprite", "");
	event:AddEventLinkAtEnd(event_chains["pass_wall_backward"] + 1);
	-- Move camera inside of wall
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["pass_wall_backward"] + 1, sprites["claudius"], 112, 21);
	event:AddEventLinkAtEnd(event_chains["pass_wall_backward"] + 2);
	-- Move camera up and to the left near wall passage exit
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["pass_wall_backward"] + 2, sprites["claudius"], 85, 6);
	event:AddEventLinkAtEnd(event_chains["pass_wall_backward"] + 3);
	-- Move sprite back outside of wall
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["pass_wall_backward"] + 3, sprites["claudius"], 76, 6);
	event:AddEventLinkAtEnd(event_chains["pass_wall_backward"] + 4);
	-- Make player sprite visible and restore collision detection
	event = hoa_map.CustomEvent.Create(event_chains["pass_wall_backward"] + 4, "ShowCameraSprite", "");
		
	---------- Event Chain 09: Arriving at spring just before riverbed
	print "Event Chain #09";
	event_chains["spring_arrival"] = 100;
	
	-- Begin dialogue between characters
	event = hoa_map.DialogueEvent.Create(event_chains["spring_arrival"] + 70, event_dialogues["spring_arrival"]);
		
	---------- Event Chain 10: Arriving at riverbed
	print "Event Chain #10";
	event_chains["riverbed_arrival"] = 110;
	
	-- Put map in scene state
	event = hoa_map.CustomEvent.Create(event_chains["riverbed_arrival"] + 0, "StopMovementAndEnterScene", "");
	event:AddEventLinkAtEnd(event_chains["riverbed_arrival"] + 1);
	-- Move player sprite in to the gathering of knights in the river bed
	event = hoa_map.PathMoveSpriteEvent.Create(event_chains["riverbed_arrival"] + 1, sprites["claudius"], 238, 12);
	event:AddEventLinkAtEnd(event_chains["riverbed_arrival"] + 2);
	-- Make sure player sprite is facing the captain
	event = hoa_map.ChangeDirectionSpriteEvent.Create(event_chains["riverbed_arrival"] + 2, sprites["claudius"], hoa_map.MapMode.EAST);
	event:AddEventLinkAtEnd(event_chains["riverbed_arrival"] + 3);
	-- Remove map scene state
	event = hoa_map.CustomEvent.Create(event_chains["riverbed_arrival"] + 3, "PopState", "");
	event:AddEventLinkAtEnd(event_chains["riverbed_arrival"] + 4);
	-- Begin dialogue among party characters
	event = hoa_map.DialogueEvent.Create(event_chains["riverbed_arrival"] + 4, 540);
	event:AddEventLinkAtEnd(event_chains["riverbed_arrival"] + 5);
	-- Begin dialogue given from captain
	event = hoa_map.DialogueEvent.Create(event_chains["riverbed_arrival"] + 5, event_dialogues["riverbed_arrival1"]);
	event:AddEventLinkAtEnd(1010);
	event:AddEventLinkAtEnd(event_chains["riverbed_arrival"] + 6, 1000);
	-- Begin dialogue preceeding boss battle encounter
	event = hoa_map.DialogueEvent.Create(event_chains["riverbed_arrival"] + 6, event_dialogues["riverbed_arrival2"]);
	event:SetStopCameraMovement(true);
	event:AddEventLinkAtEnd(event_chains["riverbed_arrival"] + 7);
	-- Boss battle
	event = hoa_map.BattleEncounterEvent.Create(event_chains["riverbed_arrival"] + 7);
	event:SetMusic("mus/The_Creature_Awakens.ogg");
	event:SetBackground("img/backdrops/battle/desert_cave.png");
	event:AddEventLinkAtEnd(120);

	---------- Event Chain 11: After boss battle
	print "Event Chain #11";
	event_chains["after_boss"] = 120;

	-- TODO: show water running with another map layer or context
	-- Post-battle dialogue
	event = hoa_map.DialogueEvent.Create(event_chains["after_boss"] + 0, event_dialogues["after_boss"]);	
	event:AddEventLinkAtEnd(event_chains["after_boss"] + 1);
	-- Transition to the return to city scene
	event = hoa_map.MapTransitionEvent.Create(event_chains["after_boss"] + 1, "lua/scripts/maps/a01_return_to_harrvah_city.lua");

	----------------------------------------------------------------------------
	---------- Miscellaneous Events
	----------------------------------------------------------------------------

	-- Places a treasure chest down by the entrance knight sprite
	event = hoa_map.CustomEvent.Create(1000, "EntrancePlaceChest", "");

	-- Sound played during conversation with knight and just before boss battle
	event = hoa_map.SoundEvent.Create(1010, "snd/evil_hiss.ogg");
end -- function CreateEvents()

-- Called at the end of the first event chain to hide all character sprites but Claudius
-- and give control over to the player.
functions["EndOpeningScene"] = function()
	sprites["mark"]:SetVisible(false);
	sprites["lukar"]:SetVisible(false);
	sprites["claudius"]:SetNoCollision(false);
	Map:PopState();
end

-- Creates the first enemy encountered on the map. The enemy spawns more quickly than normal,
-- is not contained within a zone, and is spawned a short distance to the north of the player.
functions["SpawnFirstEnemy"] = function()
	print "First Enemy Spawned"
	
	--[[ TODO
	local enemy = ConstructEnemySprite("slime", Map);
	SetBattleEnvironment(enemy);
	enemy:NewEnemyParty();
	enemy:AddEnemy(1);
	enemy:AddEnemy(1);
	enemy:AddEnemy(1);
	enemy:SetXLocation(Map.camera.x_position, 0);
	enemy:SetYLocation(Map.camera.y_position - 4, 0);
	enemy:SetTimeToSpawn(1000);
	enemy:ChangeStateSpawning();
	--]]
end


functions["EngageFirstEnemy"] = function()
	print "First Enemy Engaged"

	-- TODO
end

-- TODO: A chest placed down by the knight NPC near the entrance
functions["EntrancePlaceChest"] = function()
	print "Treasure placed down";

	-- sprites["entrance_knight"] -- Turn to face chest
end;

-- Stop camera sprite and enter scene state
functions["StopMovementAndEnterScene"] = function()
	Map.camera:SetMoving(false);
	Map:PushState(hoa_map.MapMode.STATE_SCENE);
end

-- Restore previous map state (typically from "scene" to "explore")
functions["PopMapState"] = function()
	Map:PopState();
end

-- Short screen shake during the passage collapse event chain
functions["ShakeScreen"] = function()
	VideoManager:ShakeScreen(2.0, 2000.0, hoa_video.GameVideo.VIDEO_FALLOFF_NONE);
end

-- Change map to scene state
functions["PushSceneState"] = function()
	Map:PushState(hoa_map.MapMode.STATE_SCENE);
end

-- Pop current map state
functions["PopState"] = function()
	Map:PopState();
end

-- Gives a potion to the player via the treasure menu
functions["RewardPotion"] = function()
	AudioManager:PlaySound("snd/obtain.wav");
	corpse_treasure = hoa_map.MapTreasure();
	corpse_treasure:AddObject(1, 1);
	TreasureManager:Initialize(corpse_treasure);
end

-- Quickly Fades the screen to black
functions["FadeOutScreen"] = function()
	VideoManager:FadeScreen(hoa_video.Color(0.0, 0.0, 0.0, 1.0), 1000);
end

-- Quickly fades screen from back into full view
functions["FadeInScreen"] = function()
	VideoManager:FadeScreen(hoa_video.Color(0.0, 0.0, 0.0, 0.0), 1000);
end

-- Returns true when screen is no longer in the process of fading
functions["IsScreenFading"] = function()
	if (VideoManager:IsFading() == true) then
		return false;
	else
		return true;
	end
end

-- Switches the map context of all map objects to the "passage collapsed" context
functions["SwitchContextForAllSprites"] = function()
	SwapContextForAllObjects(contexts["collapsed"]);
end

-- Makes the knight that moved along the short path disappear
functions["VanishPathSprite"] = function()
	sprites["passage_knight2"]:SetNoCollision(true);
	sprites["passage_knight2"]:SetVisible(false);
end


-- Change to scene state and make camera sprite invisible with no collision
functions["HideCameraSprite"] = function()
	Map.camera:SetMoving(false);
	Map:PushState(hoa_map.MapMode.STATE_SCENE);
	Map.camera:SetVisible(false);
	Map.camera:SetNoCollision(true);
end

-- Exit scene state and restore camera sprite visibility and collision status
functions["ShowCameraSprite"] = function()
	Map:PopState();
	Map.camera:SetVisible(true);
	Map.camera:SetNoCollision(false);
end

-- Replace dialogue of the knight that guides the player to the right path after the passage collapse
functions["ReplaceGuideDialogue"] = function()
	sprites["passage_knight1"]:RemoveDialogueReference(20);
	sprites["passage_knight1"]:AddDialogueReference(21);
end

-- Move camera to corpse
functions["CameraPanToCorpse"] = function()
	Map:MoveVirtualFocus(206, 147);
	Map:SetCamera(ObjectManager.virtual_focus, 2000);
end

-- Move camera back to player
functions["SetCameraToPlayer"] = function()
	Map:SetCamera(sprites["claudius"], 500);
end

-- Move camera to talking knight sprite
functions["CameraToGuideSprite"] = function()
	Map:MoveVirtualFocus(149, 62);
	Map:SetCamera(ObjectManager.virtual_focus, 1000);
	Map:SetCamera(sprites["passage_knight1"], 1000);
end

-- Move camera to talking knight sprite
functions["CameraFollowPathSprite"] = function()
	Map:SetCamera(sprites["passage_knight2"], 500);
end

-- Helper function that swaps the context for all objects on the map to the context provided in the argument
SwapContextForAllObjects = function(new_context)
	local max_index = ObjectManager:GetNumberObjects() - 1;
	
	for i = 0, max_index do
		ObjectManager:GetObjectByIndex(i):SetContext(new_context);
	end
end

