local ns = {}
setmetatable(ns, {__index = _G})
a01_unblock_underground_river = ns;
setfenv(1, ns);

data_file = "lua/data/maps/harrvah_underground_river_cave.lua";
location_filename = "img/portraits/locations/desert_cave.png"
map_name = "River Access Cave"

enemy_ids = { 1, 2, 3, 4, 5, 6 }

-- Containers used to hold pointers to various class objects.
-- A unique string key is used for each object entered into these tables.
objects = {};
sprites = {};
dialogues = {};
events = {};
zones = {};
-- All custom map functions are contained within the following table..
-- String keys in this table serves as the names of these functions. 
functions = {};

function Load(m)
	-- Setup global pointers for the MapMode instance as well as the various supervisors for convenience
	Map = m;
	ObjectManager = Map.object_supervisor;
	DialogueManager = Map.dialogue_supervisor;
	EventManager = Map.event_supervisor;
	TreasureManager = Map.treasure_supervisor;
	GlobalEvents = Map.map_event_group;

	print "CREATE OBJECTS"
	CreateObjects();
	print "CREATE CHARACTERS"
	CreateCharacters();
	print "CREATE ENEMIES"
	CreateEnemies();
	print "CREATE DIALOGUE"
	CreateDialogue();
	print "CREATE EVENTS"
	CreateEvents();
	
	print "SET CAMERA"
	Map:SetCamera(sprites["claudius"]);
	print "MOVE FOCUS"
	Map:MoveVirtualFocus(80, 130);

	-- Visual effects: dark lighting, various light halos
	VideoManager:EnableLightOverlay(hoa_video.Color(0.0, 0.0, 0.0, 0.6));
	
	-- DEBUG: uncomment the lines below to set the camera to locations close to testing areas
	
	-- Location: just before enemy boss
	-- Map.camera:SetXPosition(205, 0); Map.camera:SetYPosition(5, 0);

	Map:SetCurrentTrack(0);

	-- The map begins with an opening scene before control is given to the player
	Map:PushState(hoa_map.MapMode.STATE_SCENE);
	EventManager:StartEvent(events["opening_dialogue"], 2500);

	-- TEMP: enabled for development. Change this to false prior to release
	Map.unlimited_stamina = true;
	print "DONE WITH LOAD"
end -- function Load(m)


-- Mandatory function for map updates
function Update()
--[[
	if (corpse_dicovery_zone:IsCameraEntering() == true) then
		if (GlobalEvents:DoesEventExist("corpse_seen") == false) then
			GlobalEvents:AddNewEvent("corpse_seen", 1);
			EventManager:StartEvent(10);
		end
	end
	
	if (corpse_zone:IsCameraEntering() == true and not Map:IsCameraOnVirtualFocus()) then
		if (GlobalEvents:DoesEventExist("corpse_found") == false) then
			GlobalEvents:AddNewEvent("corpse_found", 1);
			EventManager:StartEvent(11);
		end
	end

	if (long_route_zone:IsCameraEntering() == true) then
		if (GlobalEvents:DoesEventExist("passage_collapsed") == false) then
			EventManager:StartEvent(20);
		end
	end
	
	if (short_route_zone:IsCameraEntering() == true) then
		if (GlobalEvents:DoesEventExist("knight_moved") == false) then
			GlobalEvents:AddNewEvent("knight_moved", 1);
			EventManager:StartEvent(30);
		end
	end
	
	if (collapse_zone:IsCameraEntering() == true) then
		if (GlobalEvents:DoesEventExist("passage_collapsed") == false) then
			GlobalEvents:AddNewEvent("passage_collapsed", 1);
			EventManager:StartEvent(40);
		end
	end

	if ((forward_passage_zone:IsCameraEntering() == true) and (Map.camera:IsVisible() == true)) then
		EventManager:StartEvent(50);
	end
	
	if ((backward_passage_zone:IsCameraEntering() == true) and (Map.camera:IsVisible() == true)) then
		EventManager:StartEvent(60);
	end
	
	if ((spring_zone:IsCameraEntering() == true)) then
		if (GlobalEvents:DoesEventExist("spring_discovered") == false) then
			GlobalEvents:AddNewEvent("spring_discovered", 1);
			Map.camera:SetMoving(false);
			EventManager:StartEvent(70);
		end
	end
	
	if ((riverbed_zone:IsCameraEntering() == true)) then
		if (GlobalEvents:DoesEventExist("riverbed_entered") == false) then
			GlobalEvents:AddNewEvent("riverbed_entered", 1);
			EventManager:StartEvent(80);
		end
	end
--]]
end -- function Update()


-- Mandatory function for custom drawing
function Draw()
	Map:DrawMapLayers();
end


-- Creates non-sprite map objects
function CreateObjects()
	-- Add a skeleton corpse to the bottom right area of the map
	objects["corpse"] = hoa_map.PhysicalObject();
	objects["corpse"]:SetObjectID(3000);
	objects["corpse"]:SetContext(1);
	objects["corpse"]:SetImgHalfWidth(2.0);
	objects["corpse"]:SetImgHeight(2.0);
	objects["corpse"]:SetCollHalfWidth(2.0);
	objects["corpse"]:SetCollHeight(1.0);
	objects["corpse"]:SetXPosition(206, 0.0);
	objects["corpse"]:SetYPosition(147, 0.0);
	objects["corpse"]:SetUpdatable(false);
	objects["corpse"]:SetVisible(true);
	objects["corpse"]:SetNoCollision(false);
	objects["corpse"]:AddAnimation("img/misc/skeleton_corpse.png");
	Map:AddGroundObject(objects["corpse"]);
end -- function CreateObjects()


-- Creates all friendly sprites. Roughly ordered by when they are encountered by the player
function CreateCharacters()
	knight_path_sprite = {};
	knight_talk_sprite = {};
	local sprite;

	-- Squad #1: Playable character sprites
	sprites["claudius"] = ConstructSprite("Claudius", 1, 11, 227);
	sprites["claudius"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["claudius"]:SetNoCollision(true);
	Map:AddGroundObject(sprites["claudius"]);

	sprites["mark"] = ConstructSprite("Karlate", 2, 17, 227);
	sprites["mark"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["mark"]:SetMovementSpeed(hoa_map.MapMode.NORMAL_SPEED);
	sprites["mark"]:SetNoCollision(true);
	sprites["mark"]:SetName(hoa_system.Translate("Mark"));
	Map:AddGroundObject(sprites["mark"]);
	
	sprites["lukar"] = ConstructSprite("Karlate", 3, 14, 225);
	sprites["lukar"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["lukar"]:SetMovementSpeed(hoa_map.MapMode.NORMAL_SPEED);
	sprites["lukar"]:SetNoCollision(true);
	sprites["lukar"]:SetName(hoa_system.Translate("Lukar"));
	Map:AddGroundObject(sprites["lukar"]);

	-- Squad #2: Mak Hound squad found close to the cave entrance
	-- Knight at cave entrance
	sprites["entrance_knight1"] = ConstructSprite("Karlate", 10, 14, 150);
	sprites["entrance_knight1"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["entrance_knight1"]:AddDialogueReference(10);
	Map:AddGroundObject(sprites["entrance_knight1"]);

	-- Knight trying to pull Mak hound
	sprites["entrance_knight2"] = ConstructSprite("Karlate", 11, 17, 148);
	sprites["entrance_knight2"]:SetDirection(hoa_map.MapMode.WEST);
	sprites["entrance_knight2"]:AddDialogueReference(11);
	Map:AddGroundObject(sprites["entrance_knight2"]);

	-- Frightened Mack Hound
	sprites["entrance_mak"] = ConstructSprite("Mak Hound", 12, 13, 148);
	sprites["entrance_mak"]:SetDirection(hoa_map.MapMode.EAST);
	Map:AddGroundObject(sprites["entrance_mak"]);
	
	-- Squad #3: Knights near the passage that collapses
	-- Knight guiding others through the short passage
	sprites["passage_knight1"] = ConstructSprite("Karlate", 20, 149, 62);
	sprites["passage_knight1"]:SetDirection(hoa_map.MapMode.SOUTH);
	sprites["passage_knight1"]:AddDialogueReference(20);
	Map:AddGroundObject(sprites["passage_knight1"]);
	--knight_talk_sprite = sprite;
	
	-- Knight seen walking ahead through the short passage
	sprites["passage_knight2"] = ConstructSprite("Karlate", 21, 162, 62);
	sprites["passage_knight2"]:SetDirection(hoa_map.MapMode.EAST);
	sprites["passage_knight2"]:SetMovementSpeed(hoa_map.MapMode.NORMAL_SPEED);
	Map:AddGroundObject(sprites["passage_knight2"]);
	--knight_path_sprite = sprite;
	
	-- Squad #4: Knights near the end of the passage treating an injury
	-- Knight with injured ankle
	sprites["injury_knight1"] = ConstructSprite("Karlate", 30, 142, 30);
	sprites["injury_knight1"]:SetDirection(hoa_map.MapMode.SOUTH);
	sprites["injury_knight1"]:AddDialogueReference(30);
	Map:AddGroundObject(sprites["injury_knight1"]);
	
	-- Knight assisting injured knight
	sprites["injury_knight2"] = ConstructSprite("Karlate", 31, 142, 33);
	sprites["injury_knight2"]:SetDirection(hoa_map.MapMode.NORTH);
	sprites["injury_knight2"]:AddDialogueReference(31);
	Map:AddGroundObject(sprites["injury_knight2"]);

--[[
	-- All of the following NPCs are encountered at the end of the cave in the riverbed
	sprite = ConstructSprite("Captain", 2500, 248, 16);
	sprite:SetDirection(hoa_map.MapMode.WEST);
	Map:AddGroundObject(sprite);
	
	sprite = ConstructSprite("Karlate", 2501, 249, 19);
	sprite:SetDirection(hoa_map.MapMode.WEST);
	Map:AddGroundObject(sprite);
	
	sprite = ConstructSprite("Karlate", 2502, 245, 11);
	sprite:SetDirection(hoa_map.MapMode.SOUTH);
	Map:AddGroundObject(sprite);
	
	sprite = ConstructSprite("Karlate", 2503, 242, 8);
	sprite:SetDirection(hoa_map.MapMode.SOUTH);
	Map:AddGroundObject(sprite);
	
	sprite = ConstructSprite("Karlate", 2504, 239, 9);
	sprite:SetDirection(hoa_map.MapMode.SOUTH);
	Map:AddGroundObject(sprite);

	sprite = ConstructSprite("Karlate", 2505, 240, 22);
	sprite:SetDirection(hoa_map.MapMode.NORTH);
	Map:AddGroundObject(sprite);

	sprite = ConstructSprite("Karlate", 2506, 243, 23);
	sprite:SetDirection(hoa_map.MapMode.NORTH);
	Map:AddGroundObject(sprite);
	
	sprite = ConstructSprite("Karlate", 2507, 245, 21);
	sprite:SetDirection(hoa_map.MapMode.NORTH);
	Map:AddGroundObject(sprite);
	
	sprite = ConstructSprite("Karlate", 2508, 234, 20);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	Map:AddGroundObject(sprite);
	
	sprite = ConstructSprite("Karlate", 2509, 233, 17);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	Map:AddGroundObject(sprite);
	
	sprite = ConstructSprite("Karlate", 2510, 235, 14);
	sprite:SetDirection(hoa_map.MapMode.EAST);
	Map:AddGroundObject(sprite);
--]]
end -- function CreateCharacters()


-- Creates all enemy sprites and the zones that they spawn/roam in
function CreateEnemies()
	local enemy = {};
	local spawn_zone = {};
	local roam_zone = {};
--[[
	-- Zone #01: Near entrance
	roam_zone = hoa_map.EnemyZone(26, 79, 130, 141);

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
	roam_zone:AddEnemy(enemy, Map, 1);

	Map:AddZone(roam_zone);
	
	-- Zone #02: Along narrow southern passage between pit and wall
	roam_zone = hoa_map.EnemyZone(108, 140, 145, 148);

	enemy = ConstructEnemySprite("snake", Map);
	SetBattleEnvironment(enemy);
	enemy:NewEnemyParty();
	enemy:AddEnemy(3);
	enemy:AddEnemy(2);
	enemy:AddEnemy(3);
	enemy:NewEnemyParty();
	enemy:AddEnemy(6);
	enemy:AddEnemy(3);
	enemy:AddEnemy(6);
	roam_zone:AddEnemy(enemy, Map, 1);
	
	Map:AddZone(roam_zone);
	
	-- Zone #03: Near ceiling overpass entrance and around corpse
	roam_zone = hoa_map.EnemyZone(158, 167, 130, 145);
	roam_zone:AddSection(168, 215, 134, 142);
	
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
	roam_zone:AddEnemy(enemy, Map, 1);

	Map:AddZone(roam_zone);
	
	-- Zone #04: In ceiling overpass
	roam_zone = hoa_map.EnemyZone(146, 157, 90, 113);
	
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
	roam_zone:AddEnemy(enemy, Map, 1);
	
	Map:AddZone(roam_zone);
	
	-- Zone #05: Wide open area at beginning of long route
	roam_zone = hoa_map.EnemyZone(26, 115, 68, 89);
	roam_zone:AddSection(4, 72, 25, 83);

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
	roam_zone:AddEnemy(enemy, Map, 2);

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
	roam_zone:AddEnemy(enemy, Map, 1);

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
	roam_zone:AddEnemy(enemy, Map, 1);
	
	Map:AddZone(roam_zone);
	
	-- Zone #06: Above pits and before wall passage
	roam_zone = hoa_map.EnemyZone(24, 89, 12, 47);
	roam_zone:AddSection(24, 73, 6, 12);
	
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
	roam_zone:AddEnemy(enemy, Map, 3);
	
	Map:AddZone(roam_zone);
	
	-- Zone #07: Through wall passage and before spring
	roam_zone = hoa_map.EnemyZone(116, 143, 6, 29);
	roam_zone:AddSection(144, 167, 6, 13);
	
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
	roam_zone:AddEnemy(enemy, Map, 1);

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
	roam_zone:AddEnemy(enemy, Map, 1);

	Map:AddZone(roam_zone);
--]]
end -- function CreateEnemies()


-- Creates all dialogue that takes place through characters and events
function CreateDialogue()
	local dialogue;
	local text;

	----------------------------------------------------------------------------
	---------- Dialogues that are attached to characters
	----------------------------------------------------------------------------
	dialogue = hoa_map.MapDialogue.Create(10);
		text = hoa_system.Translate("Watch your step and keep moving. It's not far to the river bed.");
		dialogue:AddLine(text, 10);

	dialogue = hoa_map.MapDialogue.Create(11);
		text = hoa_system.Translate("She's terrified and won't budge.");
		dialogue:AddLine(text, 11);
		text = hoa_system.Translate("For such large animals, Mak hounds sure can be cowardly.");
		dialogue:AddLine(text, 2);
		text = hoa_system.Translate("Go on ahead. We'll catch up when we get her moving again.");
		dialogue:AddLine(text, 11);

	dialogue = hoa_map.MapDialogue.Create(20);
		text = hoa_system.Translate("The river bed is just through this passage. Be careful, the walls are a little unstable.");
		dialogue:AddLine(text, 2004);

	-- After the passage collapse event, this dialogue is added to the knight guide
	dialogue = hoa_map.MapDialogue.Create(21);
		text = hoa_system.Translate("I'll direct everyone remaining to take the longer route.");
		dialogue:AddLine(text, 2004);

	dialogue = hoa_map.MapDialogue.Create(30);
		text = hoa_system.Translate("Dammit, ow ow ow...");
		dialogue:AddLine(text, 2008);

	dialogue = hoa_map.MapDialogue.Create(31);
		text = hoa_system.Translate("He sprained his ankle on a loose rock so we're treating his injury. Move on ahead, you're almost there.");
		dialogue:AddLine(text, 2009);

	----------------------------------------------------------------------------
	---------- Dialogues that are triggered by events
	----------------------------------------------------------------------------
	-- Event: Entering the cave
	dialogue = hoa_map.MapDialogue.Create(100);
		text = hoa_system.Translate("Claudius. I want you to lead us down to the riverbed.");
		dialogue:AddLine(text, 3);

	dialogue = hoa_map.MapDialogue.Create(101);
		text = hoa_system.Translate("Woah, wait a damn minute Lukar! Why are you putting a rookie like him in charge?");
		dialogue:AddLine(text, 2);

	dialogue = hoa_map.MapDialogue.Create(102);
		text = hoa_system.Translate("Relax, Mark. This is the best way for him to get experience.");
		dialogue:AddLine(text, 3);

	dialogue = hoa_map.MapDialogue.Create(103);
		text = hoa_system.Translate("Claudius, I realize that this is your first real mission as a knight. If you're not up to this task, that's okay.");
		dialogue:AddLine(text, 3);
		text = hoa_system.Translate("No, I can do it. I won't lead us astray.");
		dialogue:AddLine(text, 1);
		text = hoa_system.Translate("Good. The path we're taking should be pretty straight forward, so don't worry about getting lost.");
		dialogue:AddLine(text, 3);
		text = hoa_system.Translate("Tcsh. Just try not to get us all killed, okay rookie?");
		dialogue:AddLine(text, 2);

	dialogue = hoa_map.MapDialogue.Create(104);
		text = hoa_system.Translate("Use the [ARROW KEYS] to walk around. You can hold down two orthogonal keys to move diagonally.");
		dialogue:AddLine(text, 3);
		text = hoa_system.Translate("Hold down the [CANCEL] key while moving to run. Running drains stamina, so you can only run for a short period of time. The stamina meter in the lower right corner of the screen shows you how much stamina you have remaining. Once you stop running, stamina will gradually accumulate until the meter is full again.");
		dialogue:AddLine(text, 3);

	-- Event: First battle encounter
	dialogue = hoa_map.MapDialogue.Create(110);
		text = hoa_system.Translate("Wait, look up ahead.");
		dialogue:AddLine(text, 2);

	dialogue = hoa_map.MapDialogue.Create(111);
		text = hoa_system.Translate("There's an enemy in our way.");
		dialogue:AddLine(text, 2);
		text = hoa_system.Translate("A battle occurs whenever you and an enemy collide. Sometimes you can avoid a battle by sneaking past or running by an enemy before it has a chance to engage you. When you enter a battle you can no longer run away. At that point you must defeat your opponent before they defeat you.");
		dialogue:AddLine(text, 3);
		text = hoa_system.Translate("That enemy doesn't appear to be much of a threat. Instead of running let it engage us. I want to make sure you remember how a knight does battle.");
		dialogue:AddLine(text, 3);
		text = hoa_system.Translate("Let's see how useful you are in a real fight, rookie.");
		dialogue:AddLine(text, 2);

	-- Event: After first battle victory
	dialogue = hoa_map.MapDialogue.Create(120);
		text = hoa_system.Translate("Nicely done. After a battle ends you'll have a short moment of invulnerability to get away from any other enemies that may be roaming nearby. If you are surrounded by multiple foes when a battle begins, be ready to make a break for it as soon as the battle ends if you don't want to keep fighting.");
		dialogue:AddLine(text, 3);
		text = hoa_system.Translate("Or you can not get yourself surrounded in the first place like a dumbass.");
		dialogue:AddLine(text, 2);
		text = hoa_system.Translate("Well, Mark is correct even though he could have phrased that better.");
		dialogue:AddLine(text, 3);
		text = hoa_system.Translate("You must also be careful as sometimes an enemy will re-spawn after a short period of time, so don't sit around thinking that you're out of danger. Other times an enemy that is defeated will not appear again. It appears that the enemy we just defeated will not respawn.");
		dialogue:AddLine(text, 3);

	dialogue = hoa_map.MapDialogue.Create(121);
		text = hoa_system.Translate("Ah, one more thing. You can access the party menu by pressing the [MENU] key while on a map. You won't be able to access this menu when a dialogue is taking place nor during a scene that is occuring on the map. In the party menu you can heal your characters, change out your equipment, and check out other information.");
		dialogue:AddLine(text, 3);

	-- Event: Encountering first NPCs in the cave
	dialogue = hoa_map.MapDialogue.Create(130);
		text = hoa_system.Translate("Hold. There are some friendly units up ahead. Characters that have information to share will have a small icon appear above them. As you get closer to the character, the icon will gradually appear. Stand facing a character and hit the [CONFIRM] key to hear what they have to say.");
		dialogue:AddLine(text, 3);
		text = hoa_system.Translate("Note that a character may have more than one piece of information to share, or may have something new to say after a particular event has occurred. The icon will look differently if the character has dialogue that you have not seen before.");
		dialogue:AddLine(text, 3);
		text = hoa_system.Translate("So in other words, I should keep initiating a conversation with these characters until the new dialogue icon no longer appears above their head.");
		dialogue:AddLine(text, 1);
		text = hoa_system.Translate("Look out, boys. We got a regular genius over here. Maybe you should have become a scholar instead of a knight.");
		dialogue:AddLine(text, 2);

	-- Event: Discovery of corpse in south east part of cave
	dialogue = hoa_map.MapDialogue.Create(140);
		text = hoa_system.Translate("A corpse. That's always a reassuring find in a place like this.");
		dialogue:AddLine(text, 2);
		text = hoa_system.Translate("Hey, I think I see something under its hand.");
		dialogue:AddLine(text, 1);
		text = hoa_system.Translate("Good eye, Claudius. Go near it and press the [CONFIRM] key to acquire the item.");
		dialogue:AddLine(text, 3);

	dialogue = hoa_map.MapDialogue.Create(141);
		text = hoa_system.Translate("Hidden objects will glimmer periodically like we just saw.");
		dialogue:AddLine(text, 3);
		text = hoa_system.Translate("Be vigilant when you are exploring a new area and look around for such hidden treasures.");
		dialogue:AddLine(text, 3);

	-- Event: Player tries to go long route before short route
	dialogue = hoa_map.MapDialogue.Create(150);
		text = hoa_system.Translate("Hey! Over here!");
		dialogue:AddLine(text, 20);
	
	-- Event: As passage is collapsing
	dialogue = hoa_map.MapDialogue.Create(160);
		text = hoa_system.Translate("Look out!");
		dialogue:AddLine(text, 3);
		dialogue:AddLineTiming(1000);

	-- Event: After passage collapse occurs
	dialogue = hoa_map.MapDialogue.Create(161);
		text = hoa_system.Translate("Woah! Are you guys alright?");
		dialogue:AddLine(text, 20);
		text = hoa_system.Translate("We're all fine. But the passage has caved in.");
		dialogue:AddLine(text, 3);
		text = hoa_system.Translate("There's another way around, although it's a longer route. Follow me.");
		dialogue:AddLine(text, 20);

	-- Event: Stopping player from trying to proceed to lower levels of the cave
	dialogue = hoa_map.MapDialogue.Create(170);
		text = hoa_system.Translate("Wait. This path looks like it leads down into the earth. The area we're trying to reach likely isn't down there. Let's head another direction..");
		dialogue:AddLine(text, 3);
		text = hoa_system.Translate("Good. Who knows what could be lurking in the darkness down there..");
		dialogue:AddLine(text, 2);

	-- Event: While crossing the narrow bridge between the two pits after hearing an evil hiss
	dialogue = hoa_map.MapDialogue.Create(180);
		text = hoa_system.Translate("Did you hear that? What the hell was that sound?");
		dialogue:AddLine(text, 1);
		text = hoa_system.Translate("I don't know, but I've got a bad feeling about this mission.");
		dialogue:AddLine(text, 2);
		text = hoa_system.Translate("Well the sooner we achieve our objective, the sooner we get out of here and go home. So move your ass instead of your mouth.");
		dialogue:AddLine(text, 3);

	-- Event: Encountering the pool of running water near the end of the cave
	dialogue = hoa_map.MapDialogue.Create(190);
		text = hoa_system.Translate("Hey check it out. The water is still running here.");
		dialogue:AddLine(text, 2);
		text = hoa_system.Translate("That's a good sign. The river obstruction must be close.");
		dialogue:AddLine(text, 3);

	-- Event: Player reaches dry river bed
	dialogue = hoa_map.MapDialogue.Create(200);
		text = hoa_system.Translate("Finally made it.");
		dialogue:AddLine(text, 2);
	
	dialogue = hoa_map.MapDialogue.Create(201);
		text = hoa_system.Translate("Listen up! There's a large boulder obstructing the underground river that flows through here. When we move it aside, we get to head out of this place.");
		dialogue:AddLine(text, 2500);
		text = hoa_system.Translate("Mikal! Torren! Take your units and secure the ropes around that overgrown rock. Jasper's unit will prepare the Maks to help us move it. The rest of you stay alert and watch our backs. Who knows what the hell may be in this cave with us.");
		dialogue:AddLine(text, 2500);

	dialogue = hoa_map.MapDialogue.Create(202);
		text = hoa_system.Translate("Hey, I heard that noise earlier. It sounds like its closer now.");
		dialogue:AddLine(text, 2505);
		text = hoa_system.Translate("Keep your eyes peeled and your swords ready men.");
		dialogue:AddLine(text, 1003);
		text = hoa_system.Translate("I don't know how you expect to see shit in here. I can barely see my own hand.");
		dialogue:AddLine(text, 1002);
		text = hoa_system.Translate("Over there! Watch out!");
		dialogue:AddLine(text, 1000);

	-- Event: After boss battle
	dialogue = hoa_map.MapDialogue.Create(210);
		text = hoa_system.Translate("Damnit, the captain's been wounded along with half our troops.");
		dialogue:AddLine(text, 1);
		text = hoa_system.Translate("*cough cough*\nI'll be alright. Great job taking down that monster men, I'm proud.");
		dialogue:AddLine(text, 2500);
		text = hoa_system.Translate("We've achieved our objective here. Tend to the wounded and then let's make our way back home.");
		dialogue:AddLine(text, 2500);
end -- function CreateDialogue()



function CreateEvents()
	local event = {};

	----- Event Chain 01: Initial scene and 4-part dialogue when the player first enters the cave
	-- Part #1: Lukar turns around and asks Cladius to lead
	event = hoa_map.PathMoveSpriteEvent.Create(10, 1, 0, -16);
	events["opening_dialogue"] = event;
	event:SetRelativeDestination(true);
	event:AddEventLinkAtStart(11);
	event:AddEventLinkAtStart(12);
	event:AddEventLinkAtEnd(13, 1000);
	event:AddEventLinkAtEnd(14, 1500);
	event = hoa_map.PathMoveSpriteEvent.Create(11, 2, 0, -16);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(12, 3, 0, -16);
	event:SetRelativeDestination(true);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(13, 3, hoa_map.MapMode.SOUTH);
	event = hoa_map.DialogueEvent.Create(14, 100);
	event:AddEventLinkAtEnd(15, 250);
	event:AddEventLinkAtEnd(16, 500);
	event:AddEventLinkAtEnd(17, 500);

	-- Part #2: Mark protests Lukar's decision
	event = hoa_map.ChangeDirectionSpriteEvent.Create(15, 2, hoa_map.MapMode.WEST);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(16, 1, hoa_map.MapMode.EAST);
	event = hoa_map.DialogueEvent.Create(17, 101);
	event:AddEventLinkAtEnd(18, 200);

	-- Part #3: Lukar reassures Mark
	event = hoa_map.ChangeDirectionSpriteEvent.Create(18, 3, hoa_map.MapMode.EAST);
	event:AddEventLinkAtEnd(19, 100);
	event = hoa_map.DialogueEvent.Create(19, 102);
	event:AddEventLinkAtEnd(20, 300);

	-- Part #4: Lukar asks Claudius again, who accepts. Mark and Lukar's sprites disappear into Claudius and player is given control
	event = hoa_map.PathMoveSpriteEvent.Create(20, 3, -5, 0);
	event:SetRelativeDestination(true);
	event:AddEventLinkAtEnd(21);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(21, 3, hoa_map.MapMode.SOUTH);
	event:AddEventLinkAtEnd(22, 100);
	event = hoa_map.ChangeDirectionSpriteEvent.Create(22, 1, hoa_map.MapMode.NORTH);
	event:AddEventLinkAtEnd(23, 100);
	event = hoa_map.DialogueEvent.Create(23, 103);
	event:AddEventLinkAtEnd(24, 100);
	event:AddEventLinkAtEnd(25, 100);
	event = hoa_map.PathMoveSpriteEvent.Create(24, 2, -8, 0);
	event:AddEventLinkAtEnd(26);
	event:SetRelativeDestination(true);
	event = hoa_map.PathMoveSpriteEvent.Create(25, 3, 0, 4);
	event:SetRelativeDestination(true);
	event = hoa_map.CustomEvent.Create(26, "EndOpeningScene", "");

	----- Event Chain 02: First enemy encounter
	event = hoa_map.DialogueEvent.Create(30, 120);
	event:AddEventLinkAtStart(31);
	event = hoa_map.CustomEvent.Create(31, "SpawnFirstEnemy", "");
	event:AddEventLinkAtEnd(32, 500);
	event = hoa_map.DialogueEvent.Create(32, 121);
	event:AddEventLinkAtEnd(33);
	event = hoa_map.CustomEvent.Create(33, "EngageFirstEnemy", "");
	event:AddEventLinkAtEnd(34);
	-- Follow-up dialogue begins immediately after the battle ends
	event = hoa_map.DialogueEvent.Create(34, 130);
	event:AddEventLinkAtEnd(35, 750);
	event = hoa_map.DialogueEvent.Create(35, 131);

	-- Event Chain 03: 

--[[
	-- Event Chain 01: Discovery of corpse in cave
	corpse_dicovery_zone = hoa_map.CameraZone(185, 200, 135, 150, hoa_map.MapMode.CONTEXT_01 + hoa_map.MapMode.CONTEXT_02);
	Map:AddZone(corpse_dicovery_zone);
	-- Dialog when seeing the corpse
	event = hoa_map.DialogueEvent.Create(10, 500);
	event:SetStopCameraMovement(true);
	event:AddEventLinkAtEnd(13);
	-- Move camera to corpse
	event = hoa_map.CustomEvent.Create(13, 16, 0);
	event:AddEventLinkAtEnd(14, 3000);
	-- Move camera back to Cladius
	event = hoa_map.CustomEvent.Create(14, 17, 0);	
	
	corpse_zone = hoa_map.CameraZone(202, 210, 145, 149, hoa_map.MapMode.CONTEXT_01 + hoa_map.MapMode.CONTEXT_02);
	Map:AddZone(corpse_zone);
	-- Start dialogue about corpse
	event = hoa_map.DialogueEvent.Create(11, 501);
	event:SetStopCameraMovement(true);
	event:AddEventLinkAtEnd(12);
	-- Add treasure
	event = hoa_map.CustomEvent.Create(12, 6, 0);

	print "EVENT CHAIN #02"
	-- Event Chain 02: Prevent player from going long route before cave collapse
	long_route_zone = hoa_map.CameraZone(132, 136, 70, 80, hoa_map.MapMode.CONTEXT_01);
	Map:AddZone(long_route_zone);

	-- Enter scene state
	event = hoa_map.CustomEvent.Create(20, 1, 0);
	event:AddEventLinkAtEnd(21);
		
	-- Move camera to karlate sprite
	event = hoa_map.CustomEvent.Create(21, 18, 0);
	event:AddEventLinkAtEnd(22, 1000);
	
	-- Throw up dialogue calling out player's party
	event = hoa_map.DialogueEvent.Create(22, 510);
	event:SetStopCameraMovement(true);
	event:AddEventLinkAtEnd(23);
		
	-- Move camera back to Cladius
	event = hoa_map.CustomEvent.Create(23, 17, 0);
	event:AddEventLinkAtEnd(24, 500);
	
	-- Move player sprite to NPC that called out
	event = hoa_map.PathMoveSpriteEvent.Create(24, 1000, 149, 68);
	event:AddEventLinkAtEnd(25);
		
	-- Exit scene state
	event = hoa_map.CustomEvent.Create(25, 2, 0);
		
	print "EVENT CHAIN #03"
	-- Event Chain 03: Knight moves safely through short route while player watches
	short_route_zone = hoa_map.CameraZone(156, 160, 60, 63, hoa_map.MapMode.CONTEXT_01);
	Map:AddZone(short_route_zone);	
	-- Enter scene state
	event = hoa_map.CustomEvent.Create(30, 1, 0);
	event:AddEventLinkAtEnd(31);		
	-- Move camera to karlate sprite
	event = hoa_map.CustomEvent.Create(31, 19, 0);
	event:AddEventLinkAtEnd(32, 300);
	-- Move knight sprite down passage
	event = hoa_map.PathMoveSpriteEvent.Create(32, knight_path_sprite, 210, 61);
	--event:AddEventLinkAtStart(32);
	event:AddEventLinkAtStart(33, 2000);
	event:AddEventLinkAtStart(34, 3000);
	event:AddEventLinkAtEnd(35);
	-- Move camera back
	event = hoa_map.CustomEvent.Create(33, 17, 0);
	-- Exit scene state
	event = hoa_map.CustomEvent.Create(34, 2, 0);
	-- Move knight sprite to river bed area
	event = hoa_map.CustomEvent.Create(35, 12, 0);

	print "EVENT CHAIN #04"
	-- Event Chain 04: Short route passage collapses
	collapse_zone = hoa_map.CameraZone(186, 189, 60, 63, 1);
	Map:AddZone(collapse_zone);	
	-- Enter scene state
	event = hoa_map.CustomEvent.Create(40, 1, 0);
	event:AddEventLinkAtStart(41);
	-- Play collapse sound
	event = hoa_map.SoundEvent.Create(41, "snd/cave-in.ogg");	
	event:AddEventLinkAtStart(42, 250); 
	-- Warning message
	event = hoa_map.DialogueEvent.Create(42, 520);
	event:AddEventLinkAtEnd(43);
	-- Shake the screen
	event = hoa_map.CustomEvent.Create(43, 3, 0);
	event:AddEventLinkAtEnd(44);
	-- Fade screen to black
	event = hoa_map.CustomEvent.Create(44, 7, 9);
	event:AddEventLinkAtEnd(45);
	-- Change all objects to context "passage collapsed"
	event = hoa_map.CustomEvent.Create(45, 10, 0);
	event:AddEventLinkAtEnd(46);
	-- Fade screen back in
	event = hoa_map.CustomEvent.Create(46, 8, 9);
	event:AddEventLinkAtEnd(47);
	-- Dialogue after passage has collapsed		
	event = hoa_map.DialogueEvent.Create(47, 521);
	event:AddEventLinkAtEnd(48);
		
		-- Change dialogue of sprite guide
		event = hoa_map.CustomEvent.Create(48, 15, 0);
		event:AddEventLinkAtEnd(49);
		
		-- Exit scene state
		event = hoa_map.CustomEvent.Create(49, 2, 0);
		
		print "EVENT CHAIN #05"
	-- Event Chain 05: Moving forward through wall passage
	forward_passage_zone = hoa_map.CameraZone(78, 79, 6, 7, hoa_map.MapMode.CONTEXT_01 + hoa_map.MapMode.CONTEXT_02);
	Map:AddZone(forward_passage_zone);
	
		-- Make player sprite invisible with no collision detection
		event = hoa_map.CustomEvent.Create(50, 13, 0);
		event:AddEventLinkAtEnd(51);
		
		-- Move camera inside of wall
		event = hoa_map.PathMoveSpriteEvent.Create(51, sprites["claudius"], 85, 6);
		event:AddEventLinkAtEnd(52);
		
		-- Move camera down and to the right near wall passage exit
		event = hoa_map.PathMoveSpriteEvent.Create(52, sprites["claudius"], 104, 22);
		event:AddEventLinkAtEnd(53);
		
		-- Move sprite back outside of wall
		event = hoa_map.PathMoveSpriteEvent.Create(53, sprites["claudius"], 112, 21);
		event:AddEventLinkAtEnd(54);
		
		-- Make player sprite visible and restore collision detection
		event = hoa_map.CustomEvent.Create(54, 14, 0);
		
		print "EVENT CHAIN #06"
	-- Event Chain 06: Moving backward through wall passage
	backward_passage_zone = hoa_map.CameraZone(110, 111, 20, 21, hoa_map.MapMode.CONTEXT_01 + hoa_map.MapMode.CONTEXT_02);
	Map:AddZone(backward_passage_zone);

		-- Make player sprite invisible with no collision detection
		event = hoa_map.CustomEvent.Create(60, 13, 0);
		event:AddEventLinkAtEnd(61);
		
		-- Move camera inside of wall
		event = hoa_map.PathMoveSpriteEvent.Create(61, sprites["claudius"], 112, 21);
		event:AddEventLinkAtEnd(62);
		
		-- Move camera up and to the left near wall passage exit
		event = hoa_map.PathMoveSpriteEvent.Create(62, sprites["claudius"], 85, 6);
		event:AddEventLinkAtEnd(63);
		
		-- Move sprite back outside of wall
		event = hoa_map.PathMoveSpriteEvent.Create(63, sprites["claudius"], 76, 6);
		event:AddEventLinkAtEnd(64);
		
		-- Make player sprite visible and restore collision detection
		event = hoa_map.CustomEvent.Create(64, 14, 0);
		
		print "EVENT CHAIN #07"
	-- Event Chain 07: Arriving at spring just before riverbed
	spring_zone = hoa_map.CameraZone(171, 186, 5, 10, hoa_map.MapMode.CONTEXT_01 + hoa_map.MapMode.CONTEXT_02);
	Map:AddZone(spring_zone);
	
		-- Begin dialogue between characters
		event = hoa_map.DialogueEvent.Create(70, 530);
		
		print "EVENT CHAIN #08"
	-- Event Chain 08: Arriving at riverbed
	riverbed_zone = hoa_map.CameraZone(220, 221, 2, 11, hoa_map.MapMode.CONTEXT_01 + hoa_map.MapMode.CONTEXT_02);
	Map:AddZone(riverbed_zone);
	
		-- Put map in scene state
		event = hoa_map.CustomEvent.Create(80, 1, 0);
		event:AddEventLinkAtEnd(81);
		
		-- Move player sprite in to the gathering of knights in the river bed
		event = hoa_map.PathMoveSpriteEvent.Create(81, sprites["claudius"], 238, 12);
		event:AddEventLinkAtEnd(82);
		
		-- Make sure player sprite is facing the captain
		event = hoa_map.ChangeDirectionSpriteEvent.Create(82, sprites["claudius"], hoa_map.MapMode.EAST);
		event:AddEventLinkAtEnd(83);
		
		-- Remove map scene state
		event = hoa_map.CustomEvent.Create(83, 5, 0);
		event:AddEventLinkAtEnd(84);
		
		-- Begin dialogue among party characters
		event = hoa_map.DialogueEvent.Create(84, 540);
		event:AddEventLinkAtEnd(85);
		
		-- Begin dialogue given from captain
		event = hoa_map.DialogueEvent.Create(85, 541);
		event:AddEventLinkAtEnd(1000);
		event:AddEventLinkAtEnd(86, 1000);
		
		-- Begin dialogue preceeding boss battle encounter
		event = hoa_map.DialogueEvent.Create(86, 542);
		event:SetStopCameraMovement(true);
		event:AddEventLinkAtEnd(87);
		
		-- Boss battle
		event = hoa_map.BattleEncounterEvent.Create(87, 91);
		event:SetMusic("mus/The_Creature_Awakens.ogg");
		event:SetBackground("img/backdrops/battle/desert_cave.png");
		event:AddEventLinkAtEnd(90);
		
			print "EVENT CHAIN #09"
	-- Event Chain 09: After boss battle
		
		-- Change all objects to context "water unblocked"
		event = hoa_map.CustomEvent.Create(90, 11, 0);
		event:AddEventLinkAtEnd(91, 500);
		
		-- Post-battle dialogue
		event = hoa_map.DialogueEvent.Create(91, 550);
		
		event:AddEventLinkAtEnd(92);
		-- Transition to the return to city scene
		event = hoa_map.MapTransitionEvent.Create(92, "lua/scripts/maps/a01_return_to_harrvah_city.lua");
		

	----------------------------------------------------------------------------
	---------- Miscellaneous Events
	----------------------------------------------------------------------------

	-- Sound played during conversation with knight and just before boss battle
	event = hoa_map.SoundEvent.Create(1000, "snd/evil_hiss.ogg");
	
--]]
end -- function CreateEvents()

-- Called at the end of the first event chain to hide all character sprites but Claudius
-- and give control over to the player.
functions["EndOpeningScene"] = function()
	sprites["mark"]:SetVisible(false);
	sprites["lukar"]:SetVisible(false);
	sprites["claudius"]:SetNoCollision(false);
	Map:PopState();
end


functions["SpawnFirstEnemy"] = function()

end


functions["EngageFirstEnemy"] = function()

end



-- Stop camera sprite and enter scene state
--functions[1] = function()
--	Map.camera:SetMoving(false);
--	Map:PushState(hoa_map.MapMode.STATE_SCENE);
--end


-- Restore previous map state (typically from "scene" to "explore")
functions[2] = function()
	Map:PopState();
end


-- Short screen shake during the passage collapse event chain
functions[3] = function()
	VideoManager:ShakeScreen(2.0, 2000.0, hoa_video.GameVideo.VIDEO_FALLOFF_NONE);
end


-- Change map to scene state
functions[4] = function()
	Map:PushState(hoa_map.MapMode.STATE_SCENE);
end


-- Pop current map state
functions[5] = function()
	Map:PopState();
end


-- Gives a potion to the player via the treasure menu
functions[6] = function()
	AudioManager:PlaySound("snd/obtain.wav");
	corpse_treasure = hoa_map.MapTreasure();
	corpse_treasure:AddObject(1, 1);
	TreasureManager:Initialize(corpse_treasure);
end


-- Quickly Fades the screen to black
functions[7] = function()
	VideoManager:FadeScreen(hoa_video.Color(0.0, 0.0, 0.0, 1.0), 1000);
end


-- Quickly fades screen from back into full view
functions[8] = function()
	VideoManager:FadeScreen(hoa_video.Color(0.0, 0.0, 0.0, 0.0), 1000);
end


-- Returns true when screen is no longer in the process of fading
functions[9] = function()
	if (VideoManager:IsFading() == true) then
		return false;
	else
		return true;
	end
end


-- Switches the map context of all map objects to the "passage collapsed" context
functions[10] = function()
	swap_context_all_objects(hoa_map.MapMode.CONTEXT_02);
end


-- Switches the map context of all map objects to the "water unblocked" context
functions[11] = function()
	swap_context_all_objects(hoa_map.MapMode.CONTEXT_03);
end


-- Makes the knight that moved along the short path disappear
functions[12] = function()
	--knight_path_sprite:SetNoCollision(true);
	--knight_path_sprite:SetVisible(false);
end


-- Change to scene state and make camera sprite invisible with no collision
functions[13] = function()
	Map.camera:SetMoving(false);
	Map:PushState(hoa_map.MapMode.STATE_SCENE);
	Map.camera:SetVisible(false);
	Map.camera:SetNoCollision(true);
end


-- Exit scene state and restore camera sprite visibility and collision status
functions[14] = function()
	Map:PopState();
	Map.camera:SetVisible(true);
	Map.camera:SetNoCollision(false);
end


-- Replace dialogue of the knight that guides the player to the right path after the passage collapse
functions[15] = function()
	--knight_talk_sprite:RemoveDialogueReference(30);
	--knight_talk_sprite:AddDialogueReference(31);
end

-- Move camera to corpse
functions[16] = function()
	Map:MoveVirtualFocus(206, 147);
	Map:SetCamera(ObjectManager.virtual_focus, 2000);
end

-- Move camera back to player
functions[17] = function()
	Map:SetCamera(claudius, 500);
end

-- Move camera to talking karlate sprite
functions[18] = function()
	Map:MoveVirtualFocus(149, 62);
	Map:SetCamera(ObjectManager.virtual_focus, 1000);
end

-- Move camera to talking karlate sprite
functions[19] = function()
	Map:SetCamera(knight_path_sprite, 500);
end



-- Helper function that swaps the context for all objects on the map to the context provided in the argument
swap_context_all_objects = function(new_context)
	local max_index = ObjectManager:GetNumberObjects() - 1;
	
	for i = 0, max_index do
		ObjectManager:GetObjectByIndex(i):SetContext(new_context);
	end
end


-- Sets common battle environment settings for enemy sprites
SetBattleEnvironment = function(enemy)
	enemy:SetBattleMusicTheme("mus/Battle_Jazz.ogg");
	enemy:SetBattleBackground("img/backdrops/battle/desert_cave.png");
	enemy:SetBattleScript("lua/scripts/battles/first_battle.lua");
end
