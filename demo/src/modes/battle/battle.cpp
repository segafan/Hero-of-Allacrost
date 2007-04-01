////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle.cpp
*** \author  Viljami Korhonen, mindflayer@allacrost.org
*** \author  Corey Hoffstein, visage@allacrost.org
*** \author  Andy Gardner, chopperdave@allacrost.org
*** \brief   Source file for battle mode interface.
*** ***************************************************************************/

#include <iostream>
#include <sstream>

#include "utils.h"
#include "audio.h"
#include "video.h"
#include "input.h"
#include "mode_manager.h"
#include "system.h"
#include "global.h"
#include "script.h"

#include "battle.h"
#include "battle_actors.h"
#include "boot.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_script;

using namespace hoa_battle::private_battle;
using namespace hoa_boot;

namespace hoa_battle {

bool BATTLE_DEBUG = false;

namespace private_battle {

BattleMode * current_battle = NULL;

////////////////////////////////////////////////////////////////////////////////
// SCRIPTEVENT CLASS
////////////////////////////////////////////////////////////////////////////////

//ScriptEvent::ScriptEvent(hoa_global::GlobalActor * source, std::deque<IBattleActor*> targets, const std::string & script_name) :
ScriptEvent::ScriptEvent(BattleActor* source, std::deque<BattleActor*> targets, const std::string & script_name, uint32 warm_up_time) :
	_script_name(script_name),
	_source(source),
	_targets(targets),
	_item(NULL),
	_skill(NULL)
{
	_warm_up_time.SetDuration(warm_up_time);
	_warm_up_time.Reset();
	_warm_up_time.Play();
}

//Constructor for a script event that uses an item
ScriptEvent::ScriptEvent(BattleActor* source, BattleActor* target, GlobalSkill* skill) :
	_item(NULL),
	_skill(skill),
	_target(target),
	_source(source)
{
	_warm_up_time.SetDuration(skill->GetWarmupTime());
	_warm_up_time.Reset();
	_warm_up_time.Play();
}

//Constructor for a script event that uses an item
ScriptEvent::ScriptEvent(BattleActor* source, BattleActor* target, GlobalItem* item, uint32 warm_up_time) :
	_item(item),
	_skill(NULL),
	_target(target),
	_source(source)
{
	_warm_up_time.SetDuration(warm_up_time);
	_warm_up_time.Reset();
	_warm_up_time.Play();
}


ScriptEvent::~ScriptEvent()
{
}


void ScriptEvent::Update()
{
	//_warm_up_time -= SystemManager->GetUpdateTime();
	float offset = SystemManager->GetUpdateTime() * (107.f / _warm_up_time.GetDuration());
	
	_source->SetTimePortraitLocation(_source->GetTimePortraitLocation() + offset);
	//TODO Any warm up animations
}

void ScriptEvent::RunScript() {
	//If _item is non-NULL, then we're using an item
	if (_item)
	{
		if (_item->GetTargetType() == GLOBAL_TARGET_PARTY)
		{
			if (_target->IsEnemy())
			{
				//Loop through enemies and apply the item
				for (uint32 i = 0; i < current_battle->GetNumberOfEnemies(); ++i)
				{
					_item->Use(current_battle->GetEnemyActorAt(i), _source);
				}
				
				//_item->DecrementCount(1);

				/*if (_item->GetCount() == 0)
				{
					GlobalManager->RemoveFromInventory(_item->GetID());
				}*/
			}
			else
			{
				//Loop through all party members and apply
				for (uint32 i = 0; i < current_battle->GetNumberOfCharacters(); ++i)
				{
					_item->Use(current_battle->GetPlayerCharacterAt(i), _source);
				}

				//_item->DecrementCount(1);

				/*if (_item->GetCount() == 0)
				{
					GlobalManager->RemoveFromInventory(_item->GetID());
				}*/
			}
		}
		else
		{
			_item->Use(_target, _source);
			//_item->DecrementCount(1);

			/*if (_item->GetCount() == 0)
			{
				GlobalManager->RemoveFromInventory(_item->GetID());
			}*/
			//FIX ME Reconstruct action list somehow (do via ran_script)
		}

		if (_item->GetCount() == 0)
		{
			GlobalManager->RemoveFromInventory(_item->GetID());
		}
	}
	// TEMP: do basic damage to the actors
	else if (_skill)
	{
		if (_skill->GetTargetType() == GLOBAL_TARGET_PARTY)
		{
			if (_target->IsEnemy())
			{
				//Loop through enemies and apply the item
				for (uint32 i = 0; i < current_battle->GetNumberOfEnemies(); ++i)
				{
					_skill->Use(current_battle->GetEnemyActorAt(i), _source);
				}
			}
			else
			{
				//Loop through all party members and apply
				for (uint32 i = 0; i < current_battle->GetNumberOfCharacters(); ++i)
				{
					_skill->Use(current_battle->GetPlayerCharacterAt(i), _source);
				}
			}
		}
		else
		{
			_skill->Use(_target, _source);
		}

		_source->SetSkillPoints(_source->GetSkillPoints() - _skill->GetSPRequired());
		
		//TEMP
		current_battle->_battle_sounds[4].PlaySound();
	}
	else
	{
		//Skill code here
		for (uint8 i = 0; i < _targets.size(); i++) {

			BattleActor * actor = _targets[i];
			actor->TakeDamage(GaussianRandomValue(12, 2.0f));

			// TODO: Do this better way!
			/*if (MakeStandardString(this->GetSource()->GetActor()->GetName()) == "Spider")
				current_battle->_battle_sounds[0].PlaySound();
			else if (MakeStandardString(this->GetSource()->GetActor()->GetName()) == "Green Slime")
				current_battle->_battle_sounds[1].PlaySound();
			else if (MakeStandardString(this->GetSource()->GetActor()->GetName()) == "Skeleton")
				current_battle->_battle_sounds[2].PlaySound();
			else if (MakeStandardString(this->GetSource()->GetActor()->GetName()) == "Claudius")
				current_battle->_battle_sounds[3].PlaySound();
			else if (MakeStandardString(this->GetSource()->GetActor()->GetName()) == "Snake")*/
			current_battle->_battle_sounds[4].PlaySound();
		}
		// TODO: get script from global script repository and run, passing in list of arguments and host actor
	}
}

} // namespace private battle


////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- Initialization and Destruction Code
////////////////////////////////////////////////////////////////////////////////

BattleMode::BattleMode() :
	_initialized(false),
	_performing_script(false),
	_active_se(NULL),
	_battle_over(false),
	_victorious_battle(false),
	_selected_character(NULL),
	_selected_target(NULL),
	_attack_point_selected(0),
	_number_menu_items(0),
	_cursor_state(CURSOR_IDLE),
	_action_menu_window(NULL),
	_action_list_menu(NULL),
	_current_number_swaps(0),
	_swap_countdown_timer(300000), // 5 minutes
	_min_agility(9999),
	_selected_option_index(0),
	_item_list(NULL)
{
	if (BATTLE_DEBUG)
		cout << "BATTLE: BattleMode constructor invoked" << endl;

	std::vector <hoa_video::StillImage> attack_point_indicator;
	StillImage frame;
	frame.SetDimensions(16, 16);
	frame.SetFilename("img/icons/battle/ap_indicator_fr0.png");
	attack_point_indicator.push_back(frame);
	frame.SetFilename("img/icons/battle/ap_indicator_fr1.png");
	attack_point_indicator.push_back(frame);
	frame.SetFilename("img/icons/battle/ap_indicator_fr2.png");
	attack_point_indicator.push_back(frame);
	frame.SetFilename("img/icons/battle/ap_indicator_fr3.png");
	attack_point_indicator.push_back(frame);

	for (uint32 i = 0; i < attack_point_indicator.size(); i++) {
		if (!VideoManager->LoadImage(attack_point_indicator[i]))
			cerr << "BATTLE ERROR: Failed to load attack point indicator." << endl;
	}

	for (uint32 i = 0; i < attack_point_indicator.size(); i++) {
		_attack_point_indicator.AddFrame(attack_point_indicator[i], 10);
	}

	//Load the universal time meter image
	_universal_time_meter.SetDimensions(10, 512);
	_universal_time_meter.SetFilename("img/menus/stamina_bar.png");
	if (!VideoManager->LoadImage(_universal_time_meter))
		cerr << "BATTLE ERROR: Failed to load time meter." << endl;

	//Load in action type icons, FIXME add more later
	frame.SetDimensions(45, 45);
	frame.SetFilename("img/icons/battle/attack.png");
	_action_type_icons.push_back(frame);
	frame.SetFilename("img/icons/battle/defend.png");
	_action_type_icons.push_back(frame);
	frame.SetFilename("img/icons/battle/support.png");
	_action_type_icons.push_back(frame);
	frame.SetFilename("img/icons/battle/item.png");
	_action_type_icons.push_back(frame);

	for (uint16 i = 0; i < _action_type_icons.size(); i++) {
		if (!VideoManager->LoadImage(_action_type_icons[i]))
			cerr << "BATTLE ERROR: Failed to load action type icons." << endl;
	}

	_actor_selection_image.SetDimensions(109, 78);
	_actor_selection_image.SetFilename("img/icons/battle/character_selector.png");
	if (!VideoManager->LoadImage(_actor_selection_image)) {
		cerr << "BATTLE ERROR: Unable to load player selector image." << endl;
	}

	vector<ustring> action_type_options;
	action_type_options.push_back(MakeUnicodeString("<img/icons/battle/attack.png><55>Attack"));
	action_type_options.push_back(MakeUnicodeString("<img/icons/battle/defend.png><55>Defend"));
	action_type_options.push_back(MakeUnicodeString("<img/icons/battle/support.png><55>Support"));
	action_type_options.push_back(MakeUnicodeString("<img/icons/battle/item.png><55>Item"));

	_action_type_menu.SetOptions(action_type_options);
	_action_type_menu.EnableOption(1, false); // Disable defend & support
	_action_type_menu.EnableOption(2, false);
	// TEMP: inventory not ready yet from GlobalManager
	if (GlobalManager->GetInventoryItems()->empty())
		_action_type_menu.EnableOption(3, false);

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);

	_action_menu_window = new MenuWindow();//384
	_action_menu_window->Create(210.0f, 430.0f);
	_action_menu_window->SetPosition(0.0f, 544.0f);
	_action_menu_window->SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_action_menu_window->Hide();

	_action_type_window.Create(210.0f, 75.0f);
	_action_type_window.SetPosition(0.0f, 544.0f);
	_action_type_window.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_action_type_window.Show();

	_action_type_menu_cursor_location = 0;

// 	_action_type_menu.SetOwner(_action_menu_window);
	_action_type_menu.SetCursorOffset(-20.0f, 25.0f);
	_action_type_menu.SetCellSize(100.0f, 80.0f);
	_action_type_menu.SetSize(1, 4);
	_action_type_menu.SetPosition(30.0f, 542.0f);
	_action_type_menu.SetFont("battle");
	_action_type_menu.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_action_type_menu.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_action_type_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	_action_type_menu.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
 	_action_type_menu.SetSelection(0); // This line may be causing a seg-fault!

	_battle_lose_menu.SetCellSize(128.0f, 50.0f);
	_battle_lose_menu.SetPosition(530.0f, 380.0f);
	_battle_lose_menu.SetSize(1, 1);
	_battle_lose_menu.SetFont("battle");
	_battle_lose_menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_battle_lose_menu.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_battle_lose_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
	_battle_lose_menu.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_battle_lose_menu.SetCursorOffset(-60.0f, 25.0f);
	vector<ustring> loseText;
	loseText.push_back(MakeUnicodeString("Return to the main menu"));
	_battle_lose_menu.SetOptions(loseText);
 	_battle_lose_menu.SetSelection(0); // This line may be causing a seg-fault!

	_TEMP_LoadTestData();
} // BattleMode::BattleMode()



BattleMode::~BattleMode() {
	// Don't let current_battle keep pointing to this object instance any longer
	if (current_battle == this) {
		current_battle = NULL;
	}

	for (uint32 i = 0; i < _battle_music.size(); i++)
		_battle_music.at(i).FreeMusic();

	for (uint32 i = 0; i < _battle_sounds.size(); i++)
		_battle_sounds.at(i).FreeSound();

	// Delete all character and enemy actors
	for (deque<BattleCharacterActor*>::iterator i = _character_actors.begin(); i != _character_actors.end(); ++i) {
		delete *i;
	}
	_character_actors.clear();

	for (deque<BattleEnemyActor*>::iterator i = _enemy_actors.begin(); i != _enemy_actors.end(); ++i) {
		delete *i;
	}
	_enemy_actors.clear();

	//FIX ME If item scripts are still there, add the item back to the inventory
	for (std::list<ScriptEvent*>::iterator i = _script_queue.begin(); i != _script_queue.end(); ++i)
	{
		if ((*i)->GetItem())
		{
			(*i)->GetItem()->IncrementCount(1);
		}
		delete *i;
	}
	_script_queue.clear();

	_item_list.clear();
	_skill_list.clear();

	// Remove all of the battle images that were loaded
	VideoManager->DeleteImage(_battle_background);
	VideoManager->DeleteImage(_bottom_menu_image);
	VideoManager->DeleteImage(_actor_selection_image);
	VideoManager->DeleteImage(_attack_point_indicator);
	VideoManager->DeleteImage(_swap_icon);
	VideoManager->DeleteImage(_swap_card);
	VideoManager->DeleteImage(_universal_time_meter);

	//Remove action type icons
	for (uint16 i = 0; i < _action_type_icons.size(); i++) {
		VideoManager->DeleteImage(_action_type_icons[i]);
	}

	// Delete all GUI objects that are allocated
	if (_action_list_menu) {
		delete _action_list_menu;
		_action_list_menu = 0;
	}
	if (_action_menu_window) {
		_action_menu_window->Destroy();
		delete _action_menu_window;
		_action_menu_window = 0;
	}

	_action_type_window.Destroy();
} // BattleMode::~BattleMode()



void BattleMode::Reset() {
	current_battle = this;

	VideoManager->SetCoordSys(0.0f, static_cast<float>(SCREEN_LENGTH * TILE_SIZE),
		0.0f, static_cast<float>(SCREEN_HEIGHT * TILE_SIZE));
	VideoManager->SetFont("battle");

	if (_battle_music[0].IsPlaying() == false) {
		_battle_music[0].PlayMusic();
	}

	if (_initialized == false) {
		_Initialize();
	}
}


void BattleMode::AddEnemy(GlobalEnemy new_enemy) {
	// (1): Don't add the enemy if it has an invalid ID or an experience level that is not zero
	if (new_enemy.GetID() < 1) {
		if (BATTLE_DEBUG) {
			cerr << "BATTLE WARNING: attempted to add a new enemy with an invalid id (0)."
				<< "The enemy was not added to the battle." << endl;
		}
		return;
	}
	if (new_enemy.GetExperienceLevel() != 0) {
		if (BATTLE_DEBUG) {
			cerr << "BATTLE WARNING: attempted to add a new enemy that had already been initialized to experience level "
				<< new_enemy.GetExperienceLevel() << ". The enemy was not added to the battle." << endl;
		}
	}

	// (2): Level the enemy up to be within a reasonable range of the party's strength
	new_enemy.LevelSimulator(2); // TODO: use guassian random variable and average party xp level to set

	// (3): Hold a copy of this enemy in case the battle needs to be restarted
	_original_enemies.push_back(new_enemy);

	// (4): Construct the enemy battle actor to be placed on the battle field
	BattleEnemyActor* enemy_actor= new BattleEnemyActor(new_enemy, static_cast<float>(RandomBoundedInteger(400, 600)), static_cast<float>(RandomBoundedInteger(200, 400)));
	enemy_actor->InitBattleActorStats(&new_enemy);
	_enemy_actors.push_back(enemy_actor);
}



void BattleMode::_TEMP_LoadTestData() {
	// Load all background images
	_battle_background.SetFilename("img/backdrops/battle/desert_cave.png");
	_battle_background.SetDimensions(SCREEN_LENGTH * TILE_SIZE, SCREEN_HEIGHT * TILE_SIZE);
	if (!VideoManager->LoadImage(_battle_background)) {
		cerr << "BATTLE ERROR: Failed to load background image: " << endl;
		_ShutDown();
	}

	_bottom_menu_image.SetFilename("img/menus/battle_bottom_menu.png");
	_bottom_menu_image.SetDimensions(1024, 128);
	if (!VideoManager->LoadImage(_bottom_menu_image)) {
		cerr << "BATTLE ERROR: Failed to load bottom menu image: " << endl;
		_ShutDown();
	}

	_swap_icon.SetFilename("img/icons/battle/swap_icon.png");
	_swap_icon.SetDimensions(35, 30);
	if (!VideoManager->LoadImage(_swap_icon)) {
		cerr << "BATTLE ERROR: Failed to load swap icon: " << endl;
		_ShutDown();
	}

	_swap_card.SetFilename("img/icons/battle/swap_card.png");
	_swap_card.SetDimensions(25, 37);
	if (!VideoManager->LoadImage(_swap_card)) {
		cerr << "BATTLE ERROR: Failed to load swap card: " << endl;
		_ShutDown();
	}

	// Load the battle music track
	MusicDescriptor MD;
	MD.LoadMusic("mus/Confrontation.ogg");
	_battle_music.push_back(MD);

	// Load the battle sfx
	SoundDescriptor SD;
	_battle_sounds.push_back(SD);
	_battle_sounds.push_back(SD);
	_battle_sounds.push_back(SD);
	_battle_sounds.push_back(SD);
	_battle_sounds.push_back(SD);
	_battle_sounds[0].LoadSound("snd/spider_attack.wav");
	_battle_sounds[1].LoadSound("snd/slime_attack.wav");
	_battle_sounds[2].LoadSound("snd/skeleton_attack.wav");
	_battle_sounds[3].LoadSound("snd/sword_swipe.wav");
	_battle_sounds[4].LoadSound("snd/snake_attack.wav");
}

void BattleMode::_Initialize()
{
	// (1): Construct all character battle actors from the active party
	GlobalParty* active_party = GlobalManager->GetActiveParty();
	if (active_party->GetPartySize() == 0) {
		if (BATTLE_DEBUG) 
			cerr << "BATTLE ERROR: In BattleMode::_Initialize(), the size of the active party was zero" << endl;
		ModeManager->Pop(); // Self-destruct the battle mode
		return;
	}
	// TODO: implement this later
	for (uint32 i = 0; i < active_party->GetPartySize(); i++) {
		GlobalCharacter* new_character = dynamic_cast<GlobalCharacter*>(active_party->GetActor(i));
		BattleCharacterActor* new_actor = new BattleCharacterActor(new_character, 256, 320);
		new_actor->InitBattleActorStats(new_character);
		_character_actors.push_back(new_actor);
		_selected_character = new_actor;
	}
	// TEMP: just add Claudius to the battle
// 	BattleCharacterActor * claudius = new BattleCharacterActor(GlobalManager->GetCharacter(GLOBAL_CHARACTER_CLAUDIUS), 256, 320);
// 	_character_actors.push_back(claudius);
// 	_selected_character = claudius;

	_actor_index = GetIndexOfFirstIdleCharacter();

	// (2) Loop through and find the actor with the lowest agility
	for (uint8 i = 0; i < _enemy_actors.size(); ++i)
	{
		if ( _enemy_actors[i]->GetAgility() < _min_agility )
			_min_agility = _enemy_actors[i]->GetAgility();
	}

	for (uint8 i = 0; i < _character_actors.size(); ++i)
	{
		if ( _character_actors[i]->GetAgility() < _min_agility )
			_min_agility = _character_actors[i]->GetAgility();
	}

	//Now adjust starting wait times based on agility proportions
	//If current actor's agility is twice the lowest agility, then
	//they will have a wait time that is half of the slowest actor
	float proportion;

	for (uint8 i = 0; i < _enemy_actors.size(); ++i)
	{
		proportion = static_cast<float>(_min_agility) / static_cast<float>(_enemy_actors[i]->GetAgility());
		_enemy_actors[i]->GetWaitTime()->SetDuration(static_cast<uint32>(MAX_INIT_WAIT_TIME * proportion));

		//Start the timer.  We can do this here because the calculations will be done so quickly
		//that the other chars wont fall far behind.
		_enemy_actors[i]->ResetWaitTime();
	}

	for (uint8 i = 0; i < _character_actors.size(); ++i)
	{
		proportion = static_cast<float>(_min_agility) / static_cast<float>(_character_actors[i]->GetAgility());
		_character_actors[i]->GetWaitTime()->SetDuration(static_cast<uint32>(MAX_INIT_WAIT_TIME * proportion));

		//Start the timer.  We can do this here because the calculations will be done so quickly
		//that the other chars wont fall far behind.
		_character_actors[i]->ResetWaitTime();
	}

	// Andy: Once every game loop, the SystemManager's timers are updated
	// However, in between calls, battle mode is constructed. As part
	// of battle mode's construction, each actor is given a wait timer
	// that is triggered on initialization. But the moving of the stamina
	// portrait uses the update time from SystemManager.  Therefore, the
	// amount of time since SystemManager last updated is greater than
	// the amount of time that has expired on the actors' wait timers
	// during the first orund of battle mode.  This gives the portrait an
	// extra boost, so once the wait time expires for an actor, his portrait
	// is past the designated stopping point

	// <--      time       -->
	// A----------X-----------B
	// If the SystemManager has its timers updated at A and B, and battle mode is
	// constructed and initialized at X, you can see the amount of time between
	// X and B (how much time passed on the wait timers in round 1) is significantly
	// smaller than the time between A and B.  Hence the extra boost to the stamina
	// portrait's location

	//FIX ME This will not work in the future (i.e. paralysis)...realized this
	//after writing all the above crap
	SystemManager->UpdateTimers();

	//TEMP: For testing battle item usage only!  Pull once usage is proven!
	// Adds a health potion to your inventory
	GlobalManager->AddToInventory(1);
	_action_type_menu.EnableOption(3,true);

	_initialized = true;
} // void BattleMode::_Initialize()



void BattleMode::_ShutDown() {
	if (BATTLE_DEBUG) cout << "BATTLE: ShutDown() called!" << endl;

	_battle_music[0].StopMusic();

	// This call will clear the input state
	InputManager->EventHandler();

	// Remove this BattleMode instance from the game stack
	ModeManager->Pop();
}

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- Update Code
////////////////////////////////////////////////////////////////////////////////

void BattleMode::Update() {
	_battle_over = (_NumberEnemiesAlive() == 0) || (_NumberOfCharactersAlive() == 0);

	if (_battle_over) {
		_victorious_battle = (_NumberEnemiesAlive() == 0);
		if (_victorious_battle) {
			if (InputManager->ConfirmPress()) {
				PlayerVictory();
			}
		}
		else {
			_battle_lose_menu.Update(SystemManager->GetUpdateTime()); // Update lose menu
			if (InputManager->ConfirmRelease()) {
				// _battle_lose_menu.HandleConfirmKey(); // This needs to be handled when there's more than 1 option
				PlayerDefeat();
			}
		}
		// Do not update other battle components when the battle has already ended
		return;
	}

	// Update all battle actors
	for (uint8 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->Update();
	}
	for (uint8 i = 0; i < _enemy_actors.size(); i++) {
		_enemy_actors[i]->Update();
	}

	// Run any scripts that are sitting in the queue
	if (_script_queue.size()) {
	//if (!_IsPerformingScript() && _script_queue.size() > 0) {
		std::list<private_battle::ScriptEvent*>::iterator it;
		bool ran_script = false;
		ScriptEvent* se;
		//for (uint8 i = 0; i < _script_queue.size(); i++)
		for (it = _script_queue.begin(); it != _script_queue.end(); it++)
		{
			se = (*it);//_script_queue.front();
			se->Update();
			//(*it).Update();
			//se._warm_up_time -= SystemManager->GetUpdateTime();
			if (se->GetWarmUpTime().HasExpired() && !_IsPerformingScript())
			{
				SetPerformingScript(true,se);
				se->RunScript();
				ran_script = true;
				//Later have battle mode call UpdateActiveScriptEvent instead
				//_script_queue.pop_front();
			}
		}

		//Do this out here so iterator doesnt get screwed up mid-loop
		if (ran_script)
		{
			//If we used an item, immediately reconstruct the action list
			//This way if an item is used
			//if (se->GetItem())
			//	_ConstructActionListMenu();

			SetPerformingScript(false,NULL);
		}
	}

	// Update various menus and other GUI graphics as appropriate
	if (_cursor_state == CURSOR_SELECT_ACTION_TYPE) {
		_action_type_menu.Update(SystemManager->GetUpdateTime());
	}
	if (_action_list_menu && _cursor_state == CURSOR_SELECT_ACTION_LIST) {
		_action_list_menu->Update(SystemManager->GetUpdateTime());
	}
	if (_cursor_state == CURSOR_SELECT_ATTACK_POINT) {
		_attack_point_indicator.Update();
	}

	// Process user input depending upon which state the menu cursor is in
	switch (_cursor_state) {
		case CURSOR_IDLE:
			_UpdateCharacterSelection();
			break;
		case CURSOR_SELECT_ACTION_TYPE:
			_UpdateActionTypeMenu();
			break;
		case CURSOR_SELECT_ACTION_LIST:
			_UpdateActionListMenu();
			break;
		case CURSOR_SELECT_TARGET:
			_UpdateTargetSelection();
			break;
		case CURSOR_SELECT_ATTACK_POINT:
			_UpdateAttackPointSelection();
			break;
	} // switch (_cursor_state)
} // void BattleMode::Update()



void BattleMode::_UpdateCharacterSelection() {
	// NOTE: Comment needed here, when would this situation occur and why do we need to return?
	// ANDY: This is the first time that character selection comes into focus, so we don't want
	// to process user input on the same loop.  This is because the input is from the previous
	// loop and isn't valid for the menu.

	if (_actor_index == INVALID_BATTLE_ACTOR_INDEX) {
		_actor_index = GetIndexOfFirstIdleCharacter();
		return;
	}
	// Return if the player does not have more than one character so select
	if (_NumberOfCharactersAlive() == 1) {
		_cursor_state = CURSOR_SELECT_ACTION_TYPE;
		_action_menu_window->Show();
		return;
	}

	// Handle user input commands: up, down, left, right, confirm
	if (InputManager->UpPress() || InputManager->RightPress()) {
		// Select the next character above the currently selected one
		// If no such character exists, the selected character will remain selected
		uint32 working_index = _actor_index;
		BattleCharacterActor *bca;

		while (working_index < GetNumberOfCharacters()) {
			bca = GetPlayerCharacterAt(working_index + 1);
			if (bca->IsAlive() && bca->GetWaitTime()->HasExpired() && !bca->IsQueuedToPerform())
			{
				_actor_index = working_index + 1;
				break;
			}
			/*if (GetPlayerCharacterAt((working_index + 1))->GetActor()->IsAlive()) {
				_actor_index = working_index + 1;
				break;
			}*/
			//else {
			++working_index;
			//}
		}
	}
	else if (InputManager->DownPress() || InputManager->LeftPress()) {
		// Select the next character below the currently selected one.
		// If no such character exists, the selected character will remain selected
		uint32 working_index = _actor_index;
		BattleCharacterActor *bca;

		while (working_index > 0) {
			bca = GetPlayerCharacterAt(working_index + 1);
			if (bca->IsAlive() && bca->GetWaitTime()->HasExpired() && !bca->IsQueuedToPerform())
			{
				_actor_index = working_index - 1;
				break;
			}
			/*if (GetPlayerCharacterAt((working_index - 1))->GetActor()->IsAlive()) {
				_actor_index = working_index - 1;
				break;
			}
			else {*/
			--working_index;
			//}
		}
	}
	else if (InputManager->ConfirmPress()) {
		_selected_character = GetPlayerCharacterAt(_actor_index);
		_cursor_state = CURSOR_SELECT_ACTION_TYPE;
		_action_menu_window->Show();
	}
} // void BattleMode::_UpdateCharacterSelection()



void BattleMode::_UpdateActionTypeMenu() {
	// Handle user input commands: up, down, confirm, cancel
	if (InputManager->UpPress()) {
		if (_action_type_menu_cursor_location > 0) {
				_action_type_menu.HandleUpKey();
				_action_type_menu_cursor_location--;
		}
	}
	else if (InputManager->DownPress()) {
		if (_action_type_menu_cursor_location < 3) {
			_action_type_menu.HandleDownKey();
			_action_type_menu_cursor_location++;
		}
	}
	else if (InputManager->ConfirmPress() ) {
		// Construct the action list menu for the action selected
		if (_action_type_menu_cursor_location == ACTION_TYPE_ATTACK) {
			_cursor_state = CURSOR_SELECT_ACTION_LIST;
			_ConstructActionListMenu();
		}
		else if (_action_type_menu_cursor_location == ACTION_TYPE_ITEM) {
			_cursor_state = CURSOR_SELECT_ACTION_LIST;
			_ConstructActionListMenu();
		}

	}
	else if (InputManager->CancelPress()) {
		// Only return to selecting characters if there is more than one character
		if (_NumberOfCharactersAlive() > 1) {
			_actor_index = GetIndexOfFirstIdleCharacter();
			_cursor_state = CURSOR_IDLE;
			_action_menu_window->Hide();
		}
	}
} // void BattleMode::_UpdateActionTypeMenu()



void BattleMode::_UpdateActionListMenu() {
	if (InputManager->DownPress()) {
		_action_list_menu->HandleDownKey();
	}
	else if (InputManager->UpPress()) {
		_action_list_menu->HandleUpKey();
	}
	else if (InputManager->ConfirmPress()) {
		// If attacking, select target
		// TEMP: only allows to select one target
		if (_action_type_menu_cursor_location == ACTION_TYPE_ATTACK) {
			//FIX ME: _necessary_selections is a useless field.  This will always
			//be 1.  If an attack targets all bad guys, then all will be selected at
			//once, not in sequence.
			//_necessary_selections = 1;
			_argument_actor_index = GetIndexOfFirstAliveEnemy();
			_selected_target = GetEnemyActorAt(_argument_actor_index);
			_selected_option_index = _action_list_menu->GetSelection();
			_cursor_state = CURSOR_SELECT_TARGET;
		}
		else if (_action_type_menu_cursor_location == ACTION_TYPE_ITEM) {
			//Set cursor to first index
			//
			//_necessary_selections = 1;
			//_argument_actor_index = GetIndexOfFirstAliveEnemy();
			//_selected_enemy = GetEnemyActorAt(_argument_actor_index);
			_cursor_state = CURSOR_SELECT_TARGET;
			_selected_option_index = _action_list_menu->GetSelection();
			_argument_actor_index = 0;

			GlobalItem* selected_item = _item_list[_selected_option_index];

			//FIX ME: Need some way of creating multiple cursors in the event of
			//a party item, also need cursor memory
			switch (selected_item->GetTargetAlignment())
			{
				case GLOBAL_ALIGNMENT_GOOD:
				case GLOBAL_ALIGNMENT_NEUTRAL:
					//_selected_character = GetPlayerCharacterAt(0);
					//_selected_enemy = NULL;
					_selected_target = GetPlayerCharacterAt(0);
					break;
				case GLOBAL_ALIGNMENT_BAD:
					//_selected_character = NULL;
					_selected_target = GetEnemyActorAt(GetIndexOfFirstAliveEnemy());
					break;
				default:
					break;
			}
		}
		// TODO: retrieve the selected skill, place the cursor on either characters or enemies,
		// depending on whom that skill should target by default
	}
	else if (InputManager->CancelPress()) {
		_cursor_state = CURSOR_SELECT_ACTION_TYPE;
	}
} // void BattleMode::_UpdateActionListMenu()



void BattleMode::_UpdateTargetSelection() {
	switch (_action_type_menu_cursor_location)
	{
	case ACTION_TYPE_ATTACK:
		if (InputManager->DownPress() || InputManager->LeftPress()) {
			GlobalSkill* skill = _skill_list.at(_selected_option_index);

			if (skill->GetTargetType() != GLOBAL_TARGET_PARTY)
			{
				switch (skill->GetTargetAlignment())
				{
					case GLOBAL_ALIGNMENT_BAD:
						_argument_actor_index = GetIndexOfNextAliveEnemy(false);
						_selected_target = GetEnemyActorAt(_argument_actor_index);
						break;

					case GLOBAL_ALIGNMENT_NEUTRAL:
						if (InputManager->DownPress())
						{
							if (!_selected_target->IsEnemy())
							{
								if (_argument_actor_index)
								{
									--_argument_actor_index;
								}
								else
								{
									_argument_actor_index = GlobalManager->GetActiveParty()->GetPartySize() - 1;
								}

								_selected_target = GetPlayerCharacterAt(_argument_actor_index);
							}
							else
							{
								_argument_actor_index = GetIndexOfNextAliveEnemy(false);
								_selected_target = GetEnemyActorAt(_argument_actor_index);
							}
						}
						break;

					default:
						break;
				}
			}
			//Switch to target the party if targeting bad guys
			//Has to be at least one character alive for this to work,
			//otherwise how the hell did we get to this point
			if (InputManager->LeftPress() &&
				skill->GetTargetAlignment() == GLOBAL_ALIGNMENT_NEUTRAL &&
				_selected_target->IsEnemy())
			{
				for(uint8 i = 0; i < _character_actors.size(); ++i)
				{
					if (_character_actors[i]->IsAlive())
					{
						_argument_actor_index = i;
						_selected_target = GetPlayerCharacterAt(i);
						break;
					}
				}				
			}
			// Select the character "to the top"
			/*uint32 working_index = _argument_actor_index;
			while (true) {
				if (working_index > 0)
					--working_index;
				else
					working_index = GetIndexOfLastAliveEnemy();

				if (GetEnemyActorAt((working_index))->IsAlive()) {
					_argument_actor_index = working_index;
					_selected_enemy = GetEnemyActorAt(_argument_actor_index);
					break;
				}
			}*/
		}
		else if (InputManager->UpPress() || InputManager->RightPress()) {
			GlobalSkill* skill = _skill_list.at(_selected_option_index);

			if (skill->GetTargetType() != GLOBAL_TARGET_PARTY)
			{
				switch (skill->GetTargetAlignment())
				{
					case GLOBAL_ALIGNMENT_BAD:
						_argument_actor_index = GetIndexOfNextAliveEnemy(true);
						_selected_target = GetEnemyActorAt(_argument_actor_index);
						break;

					case GLOBAL_ALIGNMENT_NEUTRAL:
						if (InputManager->UpPress())
						{
							if (!_selected_target->IsEnemy())
							{
								if (_argument_actor_index < GlobalManager->GetActiveParty()->GetPartySize() - 1)
								{
									++_argument_actor_index;
								}
								else
								{
									_argument_actor_index = 0;
								}

								_selected_target = GetPlayerCharacterAt(_argument_actor_index);
							}
							else
							{
								_argument_actor_index = GetIndexOfNextAliveEnemy(true);
								_selected_target = GetEnemyActorAt(_argument_actor_index);
							}
						}
						break;

					default:
						break;
				}

			}
			//Switch to target party if targeting bad guys
			if (InputManager->RightPress() &&
				skill->GetTargetAlignment() == GLOBAL_ALIGNMENT_NEUTRAL &&
				!_selected_target->IsEnemy())
			{
				_argument_actor_index = GetIndexOfFirstAliveEnemy();
				_selected_target = GetEnemyActorAt(_argument_actor_index);
			}
			// Select the character "to the bottom"
			/*uint32 working_index = _argument_actor_index;
			while (true) {
				if (working_index < GetNumberOfEnemies() - 1)
					working_index++;
				else
					working_index = GetIndexOfFirstAliveEnemy();

				if (GetEnemyActorAt((working_index))->IsAlive()) {
					_argument_actor_index = working_index;
					_selected_enemy = GetEnemyActorAt(_argument_actor_index);
					break;
				}
			}*/
		}
		else if (InputManager->ConfirmPress()) {
			GlobalSkill* skill = _skill_list.at(_selected_option_index);

			if (skill->GetTargetType() == GLOBAL_TARGET_ATTACK_POINT)
			{
				_cursor_state = CURSOR_SELECT_ATTACK_POINT;
				// TODO: Implement cursor memory for attack points here
				_attack_point_selected = 0;
			}
			else
			{
				//Make script
				ScriptEvent *se = new ScriptEvent(_selected_character, _selected_target, skill);
				AddScriptEventToQueue(se);
				_selected_character->SetQueuedToPerform(true);
				_selected_target = NULL;

				_actor_index = GetIndexOfFirstIdleCharacter();
				_cursor_state = CURSOR_IDLE;
				_action_menu_window->Hide();
			}
		}
		else if (InputManager->CancelPress()) {
			_cursor_state = CURSOR_SELECT_ACTION_LIST;
		}
		break;

	case ACTION_TYPE_ITEM:
		
		//FIX ME: Check for enemy or friendly item type
		if (InputManager->DownPress() || InputManager->LeftPress()) {
			GlobalItem* item = _item_list.at(_selected_option_index);

			if (item->GetTargetType() != GLOBAL_TARGET_PARTY)
			{
				switch (item->GetTargetAlignment())
				{
					case GLOBAL_ALIGNMENT_GOOD:
						if (_argument_actor_index)
						{
							--_argument_actor_index;
						}
						else
						{
							_argument_actor_index = GlobalManager->GetActiveParty()->GetPartySize() - 1;
						}

						_selected_target = GetPlayerCharacterAt(_argument_actor_index);
						break;

					case GLOBAL_ALIGNMENT_BAD:
						_argument_actor_index = GetIndexOfNextAliveEnemy(false);
						_selected_target = GetEnemyActorAt(_argument_actor_index);
						break;

					case GLOBAL_ALIGNMENT_NEUTRAL:
						if (InputManager->DownPress())
						{
							if (!_selected_target->IsEnemy())
							{
								if (_argument_actor_index)
								{
									--_argument_actor_index;
								}
								else
								{
									_argument_actor_index = GlobalManager->GetActiveParty()->GetPartySize() - 1;
								}

								_selected_target = GetPlayerCharacterAt(_argument_actor_index);
							}
							else
							{
								_argument_actor_index = GetIndexOfNextAliveEnemy(false);
								_selected_target = GetEnemyActorAt(_argument_actor_index);
							}
						}
						break;

					default:
						break;
				}

			}
			//Switch to target party if targeting bad guys
			if (InputManager->LeftPress() &&
				item->GetTargetAlignment() == GLOBAL_ALIGNMENT_NEUTRAL &&
				_selected_target->IsEnemy())
			{
				_argument_actor_index = 0;
				_selected_target = GetPlayerCharacterAt(0);
			}
		}
		else if (InputManager->UpPress() || InputManager->RightPress()) {
			GlobalItem* item = _item_list.at(_selected_option_index);

			if (item->GetTargetType() != GLOBAL_TARGET_PARTY)
			{
				switch (item->GetTargetAlignment())
				{
					case GLOBAL_ALIGNMENT_GOOD:
						if (_argument_actor_index < (GlobalManager->GetActiveParty()->GetPartySize() - 1))
						{
							++_argument_actor_index;
						}
						else
						{
							_argument_actor_index = 0;
						}

						_selected_target = GetPlayerCharacterAt(_argument_actor_index);
						break;

					case GLOBAL_ALIGNMENT_BAD:
						_argument_actor_index = GetIndexOfNextAliveEnemy(true);
						_selected_target = GetEnemyActorAt(_argument_actor_index);
						break;

					case GLOBAL_ALIGNMENT_NEUTRAL:
						if (InputManager->UpPress())
						{
							if (!_selected_target->IsEnemy())
							{
								if (_argument_actor_index < GlobalManager->GetActiveParty()->GetPartySize() - 1)
								{
									++_argument_actor_index;
								}
								else
								{
									_argument_actor_index = 0;
								}

								_selected_target = GetPlayerCharacterAt(_argument_actor_index);
							}
							else
							{
								_argument_actor_index = GetIndexOfNextAliveEnemy(true);
								_selected_target = GetEnemyActorAt(_argument_actor_index);
							}
						}
						break;

					default:
						break;
				}

			}
			//Switch to target party if targeting bad guys
			if (InputManager->RightPress() &&
				item->GetTargetAlignment() == GLOBAL_ALIGNMENT_NEUTRAL &&
				!_selected_target->IsEnemy())
			{
				_argument_actor_index = GetIndexOfFirstAliveEnemy();
				_selected_target = GetEnemyActorAt(_argument_actor_index);
			}
			/*if (item->GetTargetType() != GLOBAL_TARGET_PARTY)
			{
				if (_argument_actor_index < (GlobalManager->GetActiveParty()->GetPartySize() - 1))
				{
					++_argument_actor_index;
				}
				else
				{
					_argument_actor_index = 0;
				}

				_selected_character = GetPlayerCharacterAt(_argument_actor_index);
			}
			//Set _select_character to NULL
			//It's an easy-to-check indicator that this item targets everyone
			else
			{
				_argument_actor_index = 0;
				_select_character = NULL;
			}*/
		}
		else if (InputManager->ConfirmPress()) {
			GlobalItem* item = _item_list.at(_selected_option_index);
			//FIX ME Don't know if this is the best approach
			//We decrement the count now so that if the next char wants to use
			//items, they know how many are left
			//If curr char uses item, then dec stays, and if count == 0 then it's
			//removed from inventory...if item is not used (i.e. battle over), it's
			//inc'd back
			item->DecrementCount(1);

			ScriptEvent *se = new ScriptEvent(_selected_character, _selected_target, item);
			AddScriptEventToQueue(se);
			_selected_character->SetQueuedToPerform(true);
			_selected_target = NULL;

			_actor_index = GetIndexOfFirstIdleCharacter();
			_cursor_state = CURSOR_IDLE;
			_action_menu_window->Hide();

			//If we've got anotRefresh item list
			//if (_actor_index != INVALID_BATTLE_ACTOR_INDEX)
			//	_ConstructActionListMenu();
			//_cursor_state = CURSOR_SELECT_ATTACK_POINT;
			// TODO: Implement cursor memory for attack points here
			//_attack_point_selected = 0;
			//USE ITEM
			//DEC COUNT
			//REFRESH ITEM LIST
		}
		else if (InputManager->CancelPress()) {
			_cursor_state = CURSOR_SELECT_ACTION_LIST;
			_selected_target = NULL;
		}
		break;

	default:
		break;
	}
} // void BattleMode::_UpdateTargetSelection()



void BattleMode::_UpdateAttackPointSelection() {
	BattleEnemyActor * e = GetEnemyActorAt(_argument_actor_index);
	vector<GlobalAttackPoint*>global_attack_points = e->GetActor()->GetAttackPoints();

	if (InputManager->ConfirmPress()) {
		//_selected_actor_arguments.push_back(GetEnemyActorAt(_argument_actor_index));
		//if (_selected_actor_arguments.size() == _necessary_selections) {

		//AddScriptEventToQueue(ScriptEvent(_selected_character, _selected_actor_arguments, "sword_swipe", 2000));
		ScriptEvent *se = new ScriptEvent(_selected_character, _selected_target, _skill_list.at(_selected_option_index));
		AddScriptEventToQueue(se);//(GetPlayerCharacterAt(_actor_index), _selected_actor_arguments, "sword_swipe", 1000));
		_selected_character->SetQueuedToPerform(true);
		//_selected_actor_arguments.clear();
		_selected_target = NULL;

		_actor_index = GetIndexOfFirstIdleCharacter();
		_cursor_state = CURSOR_IDLE;
		_action_menu_window->Hide();
		/*}
		else {
			_cursor_state = CURSOR_SELECT_TARGET;
		}*/
	}
	else if (InputManager->UpPress() || InputManager->RightPress()) {
		if (_attack_point_selected < global_attack_points.size() - 1) {
			_attack_point_selected++;
		}
		else if (_attack_point_selected == global_attack_points.size() - 1) {
			_attack_point_selected = 0;
		}
	}
	else if (InputManager->DownPress() || InputManager->LeftPress()) {
		if (_attack_point_selected > 0) {
			_attack_point_selected--;
		}
		else if (_attack_point_selected == 0) {
			_attack_point_selected = global_attack_points.size() - 1;
		}
	}
	else if (InputManager->CancelPress()) {
		_cursor_state = CURSOR_SELECT_TARGET;
	}
} // void BattleMode::_UpdateAttackPointSelection()

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- Draw Code
////////////////////////////////////////////////////////////////////////////////

void BattleMode::Draw() {
	// Apply scene lighting if the battle has finished
	if (_battle_over) {
		if (_victorious_battle) {
			VideoManager->EnableSceneLighting(Color(0.914f, 0.753f, 0.106f, 1.0f)); // Golden color for victory
		}
		else {
			VideoManager->EnableSceneLighting(Color(1.0f, 0.0f, 0.0f, 1.0f)); // Red color for defeat
		}
	}

	_DrawBackgroundVisuals();
	_DrawSprites();
	_DrawTimeMeter();
	_DrawBottomMenu();
	_DrawActionMenu();
	_DrawDialogueMenu();

	if (_battle_over) {
		VideoManager->DisableSceneLighting();
		// Draw a victory screen along with the loot. TODO: Maybe do this in a separate function
		if (_victorious_battle) {
			VideoManager->Move(520.0f, 384.0f);
			VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
			VideoManager->SetTextColor(Color::white);
			VideoManager->DrawText("Your party is victorious!\n\nExp: +50\n\nLoot : 1 HP Potion");
		}
		// Show the lose screen
		else {
				_battle_lose_menu.Draw();
				VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
				VideoManager->Move(520.0f, 430.0f);
				VideoManager->DrawText("Your party has been defeated!");
		}
	} // endif (_battle_over)
} // void BattleMode::Draw()



void BattleMode::_DrawBackgroundVisuals() {
	// Draw the full-screen, static background image
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_NO_BLEND, 0);
	VideoManager->Move(0, 0);
	VideoManager->DrawImage(_battle_background);

	// TODO: Draw other background objects and animations
} // void BattleMode::_DrawBackgroundVisuals()

bool _TEMPIsA1Smaller(BattleEnemyActor* a1, BattleEnemyActor* a2)
{
	if ((a1->GetYLocation() - (a1->GetActor()->GetHeight())) > (a2->GetYLocation() - (a2->GetActor()->GetHeight())))
		return true;

	return false;
}

void BattleMode::_DrawSprites() {
	// TODO: Draw sprites in order based on their x and y coordinates on the screen (bottom to top, then left to right)

	// Draw all character sprites
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->DrawSprite();
	}

	// Ascending Y sorting functor. We want to compare the actual objects, NOT pointers!
	struct AscendingYSort
	{
		bool operator()(BattleEnemyActor* a1, BattleEnemyActor* a2)
		{
			//return ((*a1) < (*a2));
			return _TEMPIsA1Smaller(a1, a2);
		}
	};

	// Sort and draw the enemies
	std::deque<private_battle::BattleEnemyActor*> sorted_enemy_actors = _enemy_actors;
// 	std::sort(sorted_enemy_actors.begin(), sorted_enemy_actors.end(), AscendingYSort());

	for (uint32 i = 0; i < sorted_enemy_actors.size(); i++) {
		sorted_enemy_actors[i]->DrawSprite();
	}
} // void BattleMode::_DrawSprites()


void BattleMode::_DrawTimeMeter() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->Move(1010, 128);
	VideoManager->DrawImage(_universal_time_meter);
	// Draw all character portraits
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	//BattleEnemyActor * e = GetEnemyActorAt(_argument_actor_index);

	GLOBAL_TARGET target_type = GLOBAL_TARGET_INVALID;
	bool selected;// = false;

	if (_cursor_state == CURSOR_SELECT_TARGET || _cursor_state == CURSOR_SELECT_ATTACK_POINT)
	{
		if (_action_type_menu_cursor_location == ACTION_TYPE_ITEM)
		{
			target_type = _item_list.at(_selected_option_index)->GetTargetType();
		}
		//FIX ME Work in option for flee
		else
		{
			//FIX ME
			target_type = _skill_list.at(_selected_option_index)->GetTargetType();//GLOBAL_TARGET_ATTACK_POINT;
		}
	}

	//FIX ME Below is the logic that should be used...requires change to UpdateTargetSelection code
	for (uint32 i = 0; i < _character_actors.size(); i++)
	{
		selected = false;

		if (_cursor_state == CURSOR_SELECT_TARGET || _cursor_state == CURSOR_SELECT_ATTACK_POINT)
		{
			//FIX ME Temp code.  Instead, check chosen item or skill's
			//Target Type.  If PARTY and _selected_target is a BattleCharacterActor,
			//loop through and highlight all characters
			if (target_type == GLOBAL_TARGET_PARTY)
			{
				if (!_selected_target->IsEnemy())
				{
					selected = true;
				}
			}
			else
			{
				if (!_selected_target->IsEnemy() && _selected_target == _character_actors[i])
				{
					selected = true;
				}
			}
		}

		_character_actors[i]->DrawTimePortrait(selected);
	}

	// Draw all enemy sprites
	// FIX ME use some logic for targeting highlight, loop on _selected_actor_arguments
	for (uint32 i = 0; i < _enemy_actors.size(); i++)
	{
		selected = false;

		if (_cursor_state == CURSOR_SELECT_TARGET || _cursor_state == CURSOR_SELECT_ATTACK_POINT)
		{
			//FIX ME Temp code.  Instead, check chosen item or skill's
			//Target Type.  If PARTY and _selected_target is a BattleCharacterActor,
			//loop through and highlight all characters
			if (target_type == GLOBAL_TARGET_PARTY)
			{
				if (_selected_target->IsEnemy())
				{
					selected = true;
				}
			}
			else
			{
				if (_selected_target->IsEnemy() && _selected_target == _enemy_actors[i])
				{
					selected = true;
				}
			}
		}
		_enemy_actors[i]->DrawTimePortrait(selected);
	}
} // void BattleMode::_DrawSprites()


void BattleMode::_DrawBottomMenu() {
	// Draw the static image for the lower menu
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	VideoManager->Move(0, 0);
	VideoManager->DrawImage(_bottom_menu_image);

	// Draw the swap icon and any swap cards
	VideoManager->Move(6, 16);
	VideoManager->DrawImage(_swap_icon, Color::gray);
	VideoManager->Move(6, 68);
	for (uint8 i = 0; i < _current_number_swaps; i++) {
		VideoManager->DrawImage(_swap_card);
		VideoManager->MoveRelative(4, -4);
	}

	// Draw the selected character's portrait, blended according to the character's current HP level
	_selected_character->DrawPortrait();

	// Draw the status information of all character actors
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		_character_actors[i]->DrawStatus();
	}

	// Draw the status information of the selected enemy
	if (_selected_target && _selected_target->IsEnemy()) {
		_selected_target->DrawStatus();
	}
} // void BattleMode::_DrawBottomMenu()


void BattleMode::_DrawActionMenu() {
	// If the battle is over, none of these menus need to be drawn
	if (_battle_over) {
		return;
	}

	// Draw the action menu window
	if (_cursor_state != CURSOR_IDLE && _action_menu_window != 0) {
		_action_menu_window->Draw();
	}

	// Draw the action type menu
	if (_cursor_state == CURSOR_SELECT_ACTION_TYPE) {
		if (_selected_character->GetActor()->GetAttackSkills().size() == 0)
			_action_type_menu.EnableOption(0, false);
		if (_selected_character->GetActor()->GetDefenseSkills().size() == 0)
			_action_type_menu.EnableOption(1, false);
		if (_selected_character->GetActor()->GetSupportSkills().size() == 0)
			_action_type_menu.EnableOption(2, false);
		//FIX ME Remember to reenable elsewhere in case an item is added back
		//mid-battle (i.e. you steal one or kill an enemy who stole one from you)
		if (GlobalManager->GetInventoryItems()->size() == 0)
			_action_type_menu.EnableOption(3, false);

		_action_type_menu.Draw();
	}

	// Draw the action list menu
	if (_cursor_state >= CURSOR_SELECT_ACTION_LIST) {
		_DrawActionTypeWindow();
		_action_list_menu->Draw();
	}
} // void BattleMode::_DrawActionMenu()

void BattleMode::_DrawActionTypeWindow() {
	_action_type_window.Draw();

	VideoManager->Move(30.0f, 525.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
	VideoManager->DrawImage(_action_type_icons[_action_type_menu_cursor_location]);
	VideoManager->MoveRelative(55.0f, -20.0f);
	VideoManager->SetDrawFlags(VIDEO_Y_CENTER, 0);

	switch (_action_type_menu_cursor_location)
	{
	case ACTION_TYPE_ATTACK:
		VideoManager->DrawText("Attack");
		break;
	case ACTION_TYPE_DEFEND:
		VideoManager->DrawText("Defend");
		break;
	case ACTION_TYPE_SUPPORT:
		VideoManager->DrawText("Support");
		break;
	case ACTION_TYPE_ITEM:
		VideoManager->DrawText("Item");
		break;
	/*case ACTION_TYPE_FLEE:
		break;*/
	default: cerr << "BATTLE ERROR: Unknown action type number: " << _action_type_menu_cursor_location << endl;
		break;
	}

	if (_action_type_menu_cursor_location <= ACTION_TYPE_SUPPORT)
	{
		VideoManager->MoveRelative(-55.0f, -30.0f);
		VideoManager->DrawText("Action");
		VideoManager->MoveRelative(154.0f, 0.0f);
		VideoManager->DrawText("SP");
	}
	else if (_action_type_menu_cursor_location == ACTION_TYPE_ITEM)
	{
		VideoManager->MoveRelative(-58.0f, -30.0f);
		VideoManager->DrawText("Item");
		VideoManager->MoveRelative(150.0f, 0.0f);
		VideoManager->DrawText("Qty");
	}
}

// TODO: This feature is not yet supported
void BattleMode::_DrawDialogueMenu() {
	return;
}

////////////////////////////////////////////////////////////////////////////////
// BattleMode class -- Miscellaneous Code
////////////////////////////////////////////////////////////////////////////////

const uint32 BattleMode::_NumberEnemiesAlive() const {
	uint32 enemy_count = 0;
	for (uint32 i = 0; i < _enemy_actors.size(); i++) {
		if (_enemy_actors[i]->IsAlive()) {
			enemy_count++;
		}
	}
	return enemy_count;
}


const uint32 BattleMode::_NumberOfCharactersAlive() const {
	uint32 character_count = 0;
	for (uint32 i = 0; i < _character_actors.size(); i++) {
		if (_character_actors[i]->IsAlive()) {
			character_count++;
		}
	}
	return character_count;
}


void BattleMode::_ConstructActionListMenu() {
	BattleCharacterActor * p = GetPlayerCharacterAt(_actor_index);
	// If an old submenu exists, delete it
	if (_action_list_menu) {
		delete _action_list_menu;
		_action_list_menu = 0;
	}

	_action_list_menu = new OptionBox();
	_action_list_menu->SetPosition(5.0f, 445.0f);
	_action_list_menu->SetFont("battle");
	_action_list_menu->SetAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_action_list_menu->SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_action_list_menu->SetSelectMode(VIDEO_SELECT_SINGLE);
	_action_list_menu->SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	_action_list_menu->SetCellSize(200.0f, 60.0f);
	_action_list_menu->SetCursorOffset(-20.0f, 25.0f);

	if (_action_type_menu_cursor_location == ACTION_TYPE_ATTACK) {
		vector<GlobalSkill*> attack_skills = p->GetActor()->GetAttackSkills();
		if (attack_skills.empty()) {
			_cursor_state = CURSOR_SELECT_ACTION_TYPE;
			return;
		}
		else {
			vector<ustring> attack_skill_names;
			for (uint32 i = 0; i < attack_skills.size(); ++i) {
				ustring skill_string = MakeUnicodeString("<L> ") + attack_skills[i]->GetName() + MakeUnicodeString("<R>") +
					MakeUnicodeString(NumberToString(attack_skills[i]->GetSPRequired())) + MakeUnicodeString(" ");

				_action_list_menu->AddOption(skill_string);
				_skill_list.push_back(attack_skills[i]);

				if (attack_skills[i]->GetSPRequired() > _selected_character->GetSkillPoints())
					_action_list_menu->EnableOption(_action_list_menu->GetNumOptions() - 1, false);
				//attack_skill_names.push_back(skill_string);
			}
			_action_list_menu->SetSize(1, _action_list_menu->GetNumOptions());
			//_action_list_menu->SetOptions(attack_skill_names);
			_action_list_menu->SetSelection(0);
		}
	}

	else if (_action_type_menu_cursor_location == ACTION_TYPE_DEFEND) {
		vector <GlobalSkill*> defense_skills = p->GetActor()->GetDefenseSkills();
		if (defense_skills.empty()) {
			_cursor_state = CURSOR_SELECT_ACTION_TYPE;
		}
		else {
			vector<ustring> defense_skill_names;
			for (uint32 i = 0; i < defense_skills.size(); ++i) {
				string skill_string = MakeStandardString(defense_skills[i]->GetName()) + string("     ") + NumberToString(defense_skills[i]->GetSPRequired());
				defense_skill_names.push_back(MakeUnicodeString(skill_string));
			}
			_action_list_menu->SetOptions(defense_skill_names);
			_action_list_menu->SetSize(1, defense_skill_names.size());
		}
	}

	else if (_action_type_menu_cursor_location == ACTION_TYPE_SUPPORT) {
		vector<GlobalSkill*> support_skills = p->GetActor()->GetSupportSkills();
		if (support_skills.empty()) {
			_cursor_state = CURSOR_SELECT_ACTION_TYPE;
		}
		else {
			vector<ustring> support_skill_names;
			for (uint32 i = 0; i < support_skills.size(); ++i) {
				string skill_string = MakeStandardString(support_skills[i]->GetName()) + string("     ") + NumberToString(support_skills[i]->GetSPRequired());
				support_skill_names.push_back(MakeUnicodeString(skill_string));
			}

			_action_list_menu->SetOptions(support_skill_names);
			_action_list_menu->SetSize(1, support_skill_names.size());
		}
	}

	else if (_action_type_menu_cursor_location == ACTION_TYPE_ITEM) {
		vector<GlobalItem*>* inv = GlobalManager->GetInventoryItems();
		_item_list.clear();
		// TEMP: GlobalManager inventory is not ready yet
// 		Inventory inv = GlobalManager->GetInventory();
 		if (inv->empty()) {
 			_cursor_state = CURSOR_SELECT_ACTION_TYPE;
			return;
 		}

		// Calculate the number of rows, this is dividing by 6, and if there is a remainder > 0, add one more row for the remainder
 		_action_list_menu->SetSize(1, inv->size() / 6 + ((inv->size() % 6) > 0 ? 1 : 0));
		vector<ustring> inv_names;

		// Get the name of each item in the inventory
		// TEMP: commented this out until inventory management in GameGlobal is finished
		GlobalItem *item = NULL;
 		for (uint32 i = 0; i < inv->size(); ++i) {
 			item = inv->at(i);
// 			// NOTE: item->GetIconPath is defunct. Option box will be able to take image arguments in the future
			//We check for GetCount() because if a character is warming up with a use-item script,
			//we temporarily dec the count.  If he uses the item, we leave it dec'd.  If he doesn't,
			//then we inc the count back in.  So at this point in the code, you could potentially have
			//items still part of the inventory, but temporarily unavailable for selection until we
			//see how the previous char's use-item script pans out.
			if (item->GetUsage() >= GLOBAL_USE_BATTLE && item->GetCount())
			{
 				ustring inv_item_str = MakeUnicodeString("<L> ") + item->GetName() +
					MakeUnicodeString("<R>") +
					MakeUnicodeString(NumberToString(item->GetCount())) +
					MakeUnicodeString(" ");
 				inv_names.push_back(inv_item_str);
				_item_list.push_back(item);
			}
 		}

		_action_list_menu->SetOptions(inv_names);
		//_action_list_menu->SetSize(1, inv_names.size());
		_action_list_menu->SetSelection(0);
	}

	else {
		cerr << "BATTLE ERROR: Unknown action type number: " << _action_type_menu_cursor_location << endl;
		cerr << "Exiting game" << endl;
		SystemManager->ExitGame();
	}
} // void BattleMode::_ConstructActionListMenu()


// Sets whether an action is being performed or not
void BattleMode::SetPerformingScript(bool is_performing, ScriptEvent* se)
{

	// Check if a script has just ended. Set the script to stop performing and pop the script from the front of the queue
	// ANDY: Only one script will be running at a time, so only need to check the incoming bool

	if (!is_performing)// == false && _performing_script == true)
	{

		// Remove the first scripted event from the queue
		// _script_queue.front().GetSource() is always either BattleEnemyActor or BattleCharacterActor
		//IBattleActor * source = dynamic_cast<IBattleActor*>(_script_queue.front().GetSource());
		//IBattleActor* source = _script_queue.front().GetSource();
		BattleActor* source = (*_active_se).GetSource();
		if (source) {
			source->SetQueuedToPerform(false);
			//ScriptEvent t = *_active_se;

			std::list<private_battle::ScriptEvent*>::iterator it = _script_queue.begin();
			while (it != _script_queue.end())
			{
				if ((*it) == _active_se)
				{
					_script_queue.erase(it);
					break;
				}
				it++;
			}
			//_script_queue.erase(_active_se);
			//_script_queue.pop_front();
			//_script_queue.remove(t);
			//FIX ME Use char and enemy stats
			source->ResetWaitTime();
			_active_se = NULL;
		}
		else {
			cerr << "Invalid IBattleActor pointer in SetPerformingScript()" << endl;
			SystemManager->ExitGame();
		}
	}
	else// if (is_performing && !_performing_script)
	{
		/*if (se == NULL)
			_active_se = se;
		else
		{*/
		if (se == NULL)
		{
			cerr << "Invalid IBattleActor pointer in SetPerformingScript()" << endl;
			SystemManager->ExitGame();
		}
	}

	_performing_script = is_performing;
	_active_se = se;
}



void BattleMode::RemoveScriptedEventsForActor(BattleActor * actor) {
//void BattleMode::RemoveScriptedEventsForActor(IBattleActor * actor) {
	std::list<private_battle::ScriptEvent*>::iterator it = _script_queue.begin();

	while (it != _script_queue.end()) {
		if ((*it)->GetSource() == actor) {
			if ((*it)->GetItem())
				(*it)->GetItem()->IncrementCount(1);
			it = _script_queue.erase(it);	//remove this location
		}
		else {
			it++;
		}
	}
}


// Handle player victory
void BattleMode::PlayerVictory() {
	if (BATTLE_DEBUG) cout << "BATTLE: Player has won a battle!" << endl;

	// Give player some loot
	// TODO: Fix this with proper ID's!
	GlobalManager->AddToInventory(1); // adds one healing potion

	// Give some experience for each character in the party
	for (uint32 i = 0; i < _character_actors.size(); ++i) {
		_character_actors.at(i)->GetActor()->AddExperienceLevel(1); // TODO: Add only experience, NOT exp LEVEL!
	}

	VideoManager->DisableFog();
	_ShutDown();
}


// Handle player defeat
void BattleMode::PlayerDefeat() {
	if (BATTLE_DEBUG) cout << "Player was defeated in a battle!" << endl;
	_ShutDown();
	ModeManager->PopAll();
	BootMode *BM = new BootMode();
	ModeManager->Push(BM);
}


void BattleMode::SwapCharacters(BattleCharacterActor * ActorToRemove, BattleCharacterActor * ActorToAdd) {
	// Remove 'ActorToRemove'
	for (std::deque < BattleCharacterActor * >::iterator it = _character_actors.begin(); it != _character_actors.end(); it++) {
		if (*it == ActorToRemove) {
			_character_actors.erase(it);
			break;
		}
	}

	// set location and origin to removing characters location and origin
	ActorToAdd->SetXOrigin(ActorToRemove->GetXOrigin());
	ActorToAdd->SetYOrigin(ActorToRemove->GetYOrigin());
	ActorToAdd->SetXLocation(static_cast<float>(ActorToRemove->GetXOrigin()));
	ActorToAdd->SetYLocation(static_cast<float>(ActorToRemove->GetYOrigin()));

	_character_actors.push_back(ActorToAdd);	//add the other character to battle
}



uint32 BattleMode::GetIndexOfFirstAliveEnemy() const {

	std::deque<private_battle::BattleEnemyActor*>::const_iterator it = _enemy_actors.begin();
	for (uint32 i = 0; it != _enemy_actors.end(); i++, it++) {
		if ((*it)->IsAlive()) {
			return i;
		}
	}

	//Should not be reached
	return INVALID_BATTLE_ACTOR_INDEX;
}


uint32 BattleMode::GetIndexOfLastAliveEnemy() const {

	std::deque<private_battle::BattleEnemyActor*>::const_iterator it = _enemy_actors.end()-1;
	for (int32 i = _enemy_actors.size()-1; i >= 0; i--, it--) {
		if ((*it)->IsAlive()) {
			return i;
		}
	}

	//Should not be reached
	return INVALID_BATTLE_ACTOR_INDEX;
}



uint32 BattleMode::GetIndexOfFirstIdleCharacter() const {

	BattleCharacterActor *bca;
	deque<BattleCharacterActor*>::const_iterator it = _character_actors.begin();

	for (uint32 i = 0; it != _character_actors.end(); i++, it++) {
		bca = (*it);
		if (!bca->IsQueuedToPerform() && bca->GetActor()->IsAlive() && bca->GetWaitTime()->HasExpired())
		{
			return i;
		}
	}

	return INVALID_BATTLE_ACTOR_INDEX;
}


uint32 BattleMode::GetIndexOfCharacter(BattleCharacterActor * const Actor) const {

	deque<BattleCharacterActor*>::const_iterator it = _character_actors.begin();
	for (int32 i = 0; it != _character_actors.end(); i++, it++) {
		if (*it == Actor)
			return i;
	}

	//Should not be reached
	return INVALID_BATTLE_ACTOR_INDEX;
}

uint32 BattleMode::GetIndexOfNextAliveEnemy(bool move_upward) const
{
	//BattleCharacterActor *bca;

	if (move_upward)
	{
		for (uint32 i = _argument_actor_index; i < _enemy_actors.size(); ++i)
		{
			if (_enemy_actors[i]->GetActor()->IsAlive())
			{
				return i;
			}
		}
		for (uint32 i = 0; i <= _argument_actor_index; ++i)
		{
			if (_enemy_actors[i]->GetActor()->IsAlive())
			{
				return i;
			}
		}
		
		//Should never be reached
		return INVALID_BATTLE_ACTOR_INDEX;
	}
	else
	{
		for (uint32 i = _argument_actor_index; i >= 0; --i)
		{
			if (_enemy_actors[i]->GetActor()->IsAlive())
			{
				return i;
			}
		}
		for (uint32 i = _enemy_actors.size() - 1; i >= _argument_actor_index; --i)
		{
			if (_enemy_actors[i]->GetActor()->IsAlive())
			{
				return i;
			}
		}
		
		//Should never be reached
		return INVALID_BATTLE_ACTOR_INDEX;
	}
}

} // namespace hoa_battle
