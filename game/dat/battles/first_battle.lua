-- This script executes for the very first battle that the player encounters in a new game.
-- Its purpose is to present a dialogue to the player at the start of the battle to provide
-- a brief explanation of the battle system

function Initialize(battle_instance)
	print "BATTLE SCRIPT --- Initialize";
	counter = 0;

	Battle = battle_instance;
	DialogueManager = Battle:GetDialogueSupervisor();

	main_dialogue = {};
	local text;
	
	-- Add all speakers for the dialogues to be added
	-- TODO: all of these custom speaker calls should be replaced with calls to AddCharacterSpeaker() later
	DialogueManager:AddCustomSpeaker(1000, "Claudius", "img/portraits/map/claudius.png");
	DialogueManager:AddCustomSpeaker(1001, "Mark", "");
	DialogueManager:AddCustomSpeaker(1002, "Lukar", "");

	-- The dialogue constructed below offers the player instructions on how to do battle. It is displayed only once in the first few seconds
	-- of battle, before any action can be taken. The player is presented with several options that they can read to get more information on
	-- the battle system. One of the options that the player may select from will finish the dialogue, allow the battle to resume.
	main_dialogue = hoa_battle.BattleDialogue(1);
		text = hoa_system.Translate("Hey rookie! Now don't go telling us that you forgot how to fight.");
		main_dialogue:AddLine(text, 1001);
		text = hoa_system.Translate("Shut it Mark.");
		main_dialogue:AddLine(text, 1002);
		text = hoa_system.Translate("Claudius, is there anything you need to ask?");
		main_dialogue:AddLine(text, 1002);
		text = hoa_system.Translate("No sir. I have not forgotten my training.");
		main_dialogue:AddLine(text, 1000);
		text = hoa_system.Translate("Good. Now let us quickly dispatch of this minor threat.");
		main_dialogue:AddLine(text, 1002);
	DialogueManager:AddDialogue(main_dialogue);
end



function Update()
	if (main_dialogue:HasAlreadySeen() == false) then
		if ((Battle:GetState() ~= hoa_battle.BattleMode.BATTLE_STATE_INITIAL) and (DialogueManager:IsDialogueActive() == false)) then
			DialogueManager:BeginDialogue(1);
		end
	end
end



function Draw()
	-- No draw code is needed for this script
end
