///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    battle.cpp
 * \author  Tim Hargreaves, balthazar@allacrost.org
 * \date    Last Updated: August 12th, 2005
 * \brief   Source file for battle mode interface.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include "battle.h"
#include "audio.h"
#include "video.h"
#include "engine.h"
#include "global.h"
#include "data.h"

using namespace std;
using namespace hoa_battle::private_battle;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_engine;
using namespace hoa_global;
using namespace hoa_data;



namespace hoa_battle {

bool BATTLE_DEBUG = false;

/* 

To Tim: From the GameMode class that BattleMode inherits from, you already have working access to
all of the game's singleton classes so you don't need to worry about allocating pointers to them, etc.
You can find them all inside the GameMode class in engine.h, but here's a reference of them for your
convenience:

	hoa_audio::GameAudio *AudioManager;
	hoa_video::GameVideo *VideoManager;
	hoa_data::GameData *DataManager;
	GameInput *InputManager;
	GameModeManager *ModeManager;
	GameSettings *SettingsManager;
*/

/*
 * Actor
 *
 *
 *
 *
 *
 */

//Actor constructor
Actor::Actor(BattleMode *bm, uint32 x, uint32 y) {
	_ownerBattleMode = bm;
	_X_Location = x;
	_Y_Location = y;
	_isMoveCapable = true;
	_nextAction = NULL;
	_mode = new IdleMode(this);
	_performingAction = false;
}

//copy constructor
Actor::Actor(const Actor&  a) {
	_X_Location = a._X_Location;
	_Y_Location = a._Y_Location;
	_mode = a._mode;
	_effects = a._effects;
	_maxSkillPoints = a._maxSkillPoints;
	_currentSkillPoints = a._currentSkillPoints;
	_isMoveCapable = a._isMoveCapable;
	_isAlive = a._isAlive;
	_nextAction = a._nextAction;
	_warmingUp = a._warmingUp;
	_defensiveMode = a._defensiveMode;
	_supporters = a._supporters;
	_performingAction = a._performingAction;
	_minorBattleActions = a._minorBattleActions;
	
	_mode = new IdleMode(this);
}

Actor::~Actor() {
	//get rid of the next move, if we have one
	delete _nextAction;
	//delete our current mode
	delete _mode;
	
	std::list<ActorEffect *>::iterator it = _effects.begin();
	for(;it != _effects.end(); ) {
		delete *it;
	}
	
	_ownerBattleMode->RemoveFromActionQueue(this);
}

// Get the owning battle mode
BattleMode *Actor::GetOwnerBattleMode() {
	return _ownerBattleMode;
}

//Add a battle action 
void Actor::AddBattleAction(BattleAction *act) {
	if(act->IsConcurrent()) {
		//add it to the owning battle mode
		_ownerBattleMode->AddConcurrentBattleAction(act);
	}
	else {
		_minorBattleActions.push_back(act);
	}
}

//Set a new mode, delete the old
void Actor::SetMode(ActorMode *m) {
	delete _mode; //delete on NULL does nothing
	_mode = m;
	cout << "Actor: Changing actor mode." << endl;
}

//Return the mode
ActorMode *Actor::GetMode() {
	return _mode;
}

//update the current effects affecting the character
void Actor::UpdateEffects(uint32 dt) {
	cout << "Actor: Update effects." << endl;
	std::list<ActorEffect *>::iterator it = _effects.begin();
	for(;it != _effects.end(); ) {
		(*it)->Update(dt); 
		if((*it)->GetTTL() == 0) { //if the TTL is 0, the effect should wear off
			(*it)->UndoEffect();
			_effects.erase(it);
		}
		else {
			it++;
		}
	}
}

// add an effect to the actor's list of effects
void Actor::PushEffect(ActorEffect *e) {
	_effects.push_back(e);
}

// Remove an effect from the list
void Actor::RemoveEffect(ActorEffect *e) {
	_effects.remove(e);
	delete e;
}

// Perform the action we are waiting to perform
void Actor::PerformAction() {
	cout << "Actor: Perform Action." << endl;
	_nextAction->PerformAction();
}

// Set the actors next action
void Actor::SetNextAction(Action *a) {
	delete _nextAction;
	_nextAction = a;
}

// Do we have an action to perform?
bool Actor::HasNextAction() {
	return _nextAction != NULL;
}

// Are we performing an action?
bool Actor::IsPerformingAction() {
	return _performingAction;
}

// Set whether or not we are performing an action
void Actor::SetPerformingAction(bool performing) {
	_performingAction = performing;
}

// Is the character capable of moving?
bool Actor::IsMoveCapable() {
	return _isMoveCapable;
}

// Set whether or not the character is capable of moving
void Actor::SetMoveCapable(bool capable) {
	_isMoveCapable = capable;
}

// Add a supporter to call when we get attacked
void Actor::AddSupporter(Actor *a) {
	_supporters.push_back(a);
}

// Remove a supporter
void Actor::RemoveSupporter(Actor *a) {
	_supporters.remove(a);
}

// Put the actor into defensive move
void Actor::SetDefensiveMode(bool d) {
	_defensiveMode = d;
}

// Is the actor in defensive mode?
bool Actor::IsInDefensiveMode() {
	return _defensiveMode;
}

// Is the actor warming up?
bool Actor::IsWarmingUp() {
	return _warmingUp;
}

// Set whether or not the actor is warming up
void Actor::SetWarmingUp(bool warmup) {
	_warmingUp = warmup;
}

// Does the actor have minor actions?
bool Actor::HasMinorActions() {
	return _minorBattleActions.size() > 0;
}

// Update the action at the front of the list
void Actor::UpdateMinorActions(uint32 dt) {
	_minorBattleActions[0]->Update(dt);
	//if _minorBattleActions[0] has ties, update the ties
}

// This should be removed
void Actor::SetAnimation(std::string animation) {
	//set the animation to "animation"
	std::cout << "Setting animation to: " << animation << std::endl;
}

/**
 * BattleUI
 *
 *
 *
 *
 *
 */

BattleUI::BattleUI(BattleMode *bm) {
	_bm = bm;
}
		
Actor* BattleUI::GetSelectedActor() {
	return _currentlySelectedActor;
}

void BattleUI::SetActorSelected(Actor *a) {
	_currentlySelectedActor = a;
}

void BattleUI::DeselectActor() {
	_currentlySelectedActor = NULL;
}

std::list<Actor *> BattleUI::GetSelectedArgumentActors() {
	return _currentlySelectedArgumentActors;
}

void BattleUI::setActorAsArgument(Actor *a) {
	_currentlySelectedArgumentActors.push_back(a);
}

void BattleUI::RemoveActorAsArgument(Actor *a) {
	//we don't have to erase the actor because it will be
	//cleaned up at the end of the battle mode when all
	//actors are cleaned up
	//everything should stay alive until then
	_currentlySelectedArgumentActors.remove(a);
}

void BattleUI::SetNumberNecessarySelections(uint32 select) {
	_necessarySelections = select;
}

/**
 * PlayerActor
 *
 *
 *
 *
 *
 */

PlayerActor::PlayerActor(GCharacter *wrapped, BattleMode *bm, int x, int y) 
	: Actor(bm, x, y) {
	_wrappedCharacter = wrapped;
}

PlayerActor::~PlayerActor() {
	//don't delete _wrappedCharacter--it should exist outside
	//of battle as well
}

void PlayerActor::Update(uint32 dt) {
	std::cout << "PlayerActor: Updating: " << this << std::endl;
	
	if(HasMinorActions()) {
		UpdateMinorActions(dt);
	}
	else {
		//update the current effects on the player
		std::cout << "\tPlayerActor: Update effects." << std::endl;
		UpdateEffects(dt);
		//update the current mode on the player
		std::cout << "\tPlayerActor: Update mode." << std::endl;
		GetMode()->Update(dt);
	}
}

void PlayerActor::Draw() {
	std::cout << "PlayerActor: Draw: " << this << std::endl;
	//draw outselves here
}
		
std::vector<GSkill *> PlayerActor::GetAttackSkills() {
	return _wrappedCharacter->GetAttackSkills();
}

std::vector<GSkill *> PlayerActor::GetDefenseSkills() {
	return _wrappedCharacter->GetDefenseSkills();
}

std::vector<GSkill *> PlayerActor::GetSupportSkills() {
	return _wrappedCharacter->GetSupportSkills();
}

uint32 PlayerActor::GetAgility() {
	return _wrappedCharacter->GetAgility();
}

uint32 PlayerActor::GetStrength() {
	return _wrappedCharacter->GetStrength();
}

uint32 PlayerActor::GetMaxHealth() {
	return _wrappedCharacter->GetMaxHP();
}

uint32 PlayerActor::GetSkillPoints() {
	return _wrappedCharacter->GetSP();
} 

std::vector<GAttackPoint> PlayerActor::GetAttackPoints() {
	return _wrappedCharacter->GetAttackPoints();
} 

uint32 PlayerActor::GetIntelligence() {
	return _wrappedCharacter->GetIntelligence();
} 

uint32 PlayerActor::GetMaxSkillPoints() {
	return _wrappedCharacter->GetMaxSP();
} 

std::string PlayerActor::GetName() {
	return _wrappedCharacter->GetName();
}

uint32 PlayerActor::GetHealth() {
	return _wrappedCharacter->GetSP();
}


/**
 * EnemyActor
 *
 *
 *
 *
 *
 */

EnemyActor::EnemyActor(GEnemy *ge, BattleMode *bm, int x, int y) 
	: Actor(bm, x,  y) {
	_wrappedEnemy = ge;
}

EnemyActor::~EnemyActor() {
	delete _wrappedEnemy;
}

void EnemyActor::Update(uint32 dt) {
	std::cout << "EnemyActor: Updating: " << this << std::endl;
	
	//if we are currently performing an action 
	// if( IsPerformingAction() ) {
	//using _animationMode and _wrappedCharacter, 
	//we find the GetFrameProgress() on the current
	//frame we are processing -- and if it is 100%,
	//we call "finish action."
	//	 _nextAction->FinishAction();
	// }
	
	std::cout << "\tEnemyActor: Update effects." << std::endl;
	UpdateEffects(dt);
	
	std::cout << "\tEnemyActor: Update mode." << std::endl;
	GetMode()->Update(dt);
	
	std::cout << "\tEnemyActor: Do AI." << std::endl;
	DoAI(dt);
}

void EnemyActor::Draw() {
	std::cout << "EnemyActor: Draw: " << this << std::endl;
	//draw ourselves here
}

void EnemyActor::DoAI(uint32 dt) {
	std::cout << "EnemyActor: Doing AI..." << std::endl;
	
	if(!HasNextAction()) {
		std::cout << "EnemyActor: Does not have next action..." << std::endl;
		//get our enemy
		
		//get a skill
		//if our skill requires warmup time, perform the warmup
		//otherwise, go into actionmode
		
		std::cout << "EnemyActor: Getting PCs in battle..." << std::endl;
		std::list<PlayerActor *> targets = GetOwnerBattleMode()->ReturnCharacters();
		std::vector<GSkill *> skills = _wrappedEnemy->GetSkills();
		std::list<PlayerActor *>::iterator it = targets.begin();
		std::vector<Actor *> truetargets;
		for(unsigned int i = 0; i < targets.size(); i++) {
			truetargets.push_back(*it);
			it++;
		}
		std::cout << "Setting mode to attack..." << std::endl;
		SetMode(new ActionMode(this, new SkillAction(skills[0], this, truetargets)));
	}
	//do some artificial intelligence, or something like that.
	//as long as HasNextAction() is false...
}

void EnemyActor::LevelUp(uint32 average_level) {
	//[0.0, 1.0] RandomUnit()  
	//uint32 GaussianValue(int32 mean, int32 range, bool positive_value)
	uint32 level = hoa_utils::GaussianValue(average_level,
													RandomNumber(-3, 3), true);
	
	uint32 base_health = _wrappedEnemy->GetBaseHitPoints();
	uint32 base_exp = _wrappedEnemy->GetBaseExperiencePoints();
	uint32 base_skill = _wrappedEnemy->GetBaseSkillPoints();
	uint32 base_strength = _wrappedEnemy->GetBaseStrength();
	uint32 base_intelligence = _wrappedEnemy->GetBaseIntelligence();
	uint32 base_agility = _wrappedEnemy->GetBaseAgility();
	
	uint32 growth_health = _wrappedEnemy->GetGrowthHitPoints();
	uint32 growth_exp = _wrappedEnemy->GetGrowthExperiencePoints();
	uint32 growth_skill = _wrappedEnemy->GetGrowthSkillPoints();
	uint32 growth_strength = _wrappedEnemy->GetGrowthStrength();
	uint32 growth_intelligence = _wrappedEnemy->GetGrowthIntelligence();
	uint32 growth_agility = _wrappedEnemy->GetGrowthAgility();
	
	for(uint32 i = 0; i < level; i++) {
		base_health = base_health + hoa_utils::GaussianValue(growth_health,
													RandomNumber(-3, 3), true);
		base_exp = base_exp + hoa_utils::GaussianValue(growth_exp,
													RandomNumber(-3, 3), true);
		base_skill = base_skill+ hoa_utils::GaussianValue(growth_skill,
													RandomNumber(-3, 3), true);
		base_strength = base_strength + hoa_utils::GaussianValue(growth_strength,
													RandomNumber(-3, 3), true);
		base_intelligence = base_intelligence + hoa_utils::GaussianValue(
													growth_intelligence,
													RandomNumber(-3, 3), true);
		base_agility = base_agility + hoa_utils::GaussianValue(growth_agility,
													RandomNumber(-3, 3), true);
	}

	_wrappedEnemy->SetMaxHP(base_health);
	_wrappedEnemy->SetXP(base_exp);
	_wrappedEnemy->SetXPLevel(level);
	_wrappedEnemy->SetSP(base_skill);
	_wrappedEnemy->SetStrength(base_strength);
	_wrappedEnemy->SetIntelligence(base_intelligence);
	_wrappedEnemy->SetAgility(base_agility);
}

std::vector<GSkill *> EnemyActor::GetSkills() {
	return _wrappedEnemy->GetSkills();
	//here we need to deal with the current level and the level required to use
	//the skill
}

std::vector<GAttackPoint> EnemyActor::GetAttackPoints() {
	return _wrappedEnemy->GetAttackPoints();
}

uint32 EnemyActor::GetAgility() {
	return _wrappedEnemy->GetAgility();
}

uint32 EnemyActor::GetStrength() {
	return _wrappedEnemy->GetStrength();
}

uint32 EnemyActor::GetIntelligence() {
	return _wrappedEnemy->GetIntelligence();
}

uint32 EnemyActor::GetHealth() {
	return _wrappedEnemy->GetHP();
}

uint32 EnemyActor::GetMaxHealth() {
	return _wrappedEnemy->GetMaxHP();
}

uint32 EnemyActor::GetSkillPoints() {
	return _wrappedEnemy->GetSP();
}

uint32 EnemyActor::GetMaxSkillPoints() {
	return _wrappedEnemy->GetMaxSP();
}

std::string EnemyActor::GetName() {
	return _wrappedEnemy->GetName();
}

/**
 * ActorMode
 *
 *
 *
 *
 *
 */

ActorMode::ActorMode(Actor *a) {
	_host = a;
}

ActorMode::~ActorMode() {

}

Actor *ActorMode::GetHost() {
	return _host;
}

/**
 * SupportMode
 *
 *
 *
 *
 *
 */

SupportMode::SupportMode(uint32 TTL, Actor *a, std::vector<Actor *> supported) : 
	ActorMode(a) {
	_supported = supported;
	std::vector<Actor *>::iterator it = _supported.begin();
	
	//go through those we are supporting and set our character as a supporter
	for(; it != _supported.end(); it++) {
		(*it)->AddSupporter(a);
	}
	a->SetAnimation("SUPPORT");
}

void SupportMode::Update(uint32 dt) {
	//SubtractTTL(dt);
}

void SupportMode::UndoMode() {
	Actor *host = GetHost();
	std::vector<Actor *>::iterator it = _supported.begin();
	for(; it != _supported.end(); it++) {
		(*it)->RemoveSupporter(host);
	}
	host->SetMode(new IdleMode(host));
}

/**
 * DefensiveMode
 *
 *
 *
 *
 *
 */

DefensiveMode::DefensiveMode(Actor *a) : 
	ActorMode(a) {
	//put the host in defensive mode
	a->SetDefensiveMode(true);
	a->SetAnimation("DEFENSIVE");
}

void DefensiveMode::Update(uint32 dt) {
	//falala, nothing to do
}

void DefensiveMode::UndoMode() {
	//get the host out of defensive mode
	Actor *host = GetHost();
	host->SetDefensiveMode(false);
	host->SetMode(new IdleMode(host));
}

/**
 * IdleMode
 *
 *
 *
 *
 *
 */

IdleMode::IdleMode(Actor *a) : 
	ActorMode(a) {
	a->SetAnimation("IDLE");
}

void IdleMode::Update(uint32 dt) {
	//falala, nothing to do
}

void IdleMode::UndoMode() {
}

/**
 * WarmUpMode
 *
 *
 *
 *
 *
 */

WarmUpMode::WarmUpMode(uint32 TTL, Action *act, Actor *a) : 
	ActorMode(a) {
	_TTL = TTL;
	_action = act;
	a->SetWarmingUp(true);
	a->SetAnimation("WARMUP");
}

void WarmUpMode::Update(uint32 dt) {
	//just update the time to live
	_TTL = _TTL - dt;
	if(_TTL < 0) _TTL = 0;
	
	if(_TTL == 0) {
		UndoMode();
	}
}

void WarmUpMode::UndoMode() {
	//tell the actor we are ready to perform the action
	Actor *host = GetHost();
	//add to battle mode event queue
	host->SetWarmingUp(false);
	host->SetMode(new ActionMode(host, _action));
}

/**
 * ActionMode
 *
 *
 *
 *
 *
 */
 
ActionMode::ActionMode(Actor *a, Action *act) :
	ActorMode(a) {
	_action = act;
	//we stay in our previous stance, but set the next action
	a->SetNextAction(act);
	//add ourselves to the action queue in the battle mode
	a->SetPerformingAction(true);
	std::cout << "ActionMode: Adding action to global queue..." << std::endl;
	a->GetOwnerBattleMode()->AddToActionQueue(a);
}

ActionMode::~ActionMode() {
	//since the action is complete, 
	//we should tell the owning battle mode
	//that we are no longer performing an action
	
	//this should be called when we are put 
	//into a new mode by the action, but it
	//idle or cooldown -- the actor should 
	//delete their current mode -- hence,
	//we tell the battle mode the action is over
	
	std::cout << "ActionMode: Action finished..." << std::endl;
	BattleMode *bm = GetHost()->GetOwnerBattleMode();
	GetHost()->SetPerformingAction(false);
	bm->SetPerformingAction(false);
}

void ActionMode::Update(uint32 dt) {
	//do nothing here
}

Action *ActionMode::GetAction() {
	return _action;
}

void ActionMode::UndoMode() {
}

/**
 * CoolDownMode
 *
 *
 *
 *
 *
 */

CoolDownMode::CoolDownMode(uint32 TTL, Actor *a) : 
	ActorMode(a) {
	_TTL = TTL;
	a->SetWarmingUp(true);
	a->SetAnimation("COOLDOWN");
}

void CoolDownMode::Update(uint32 dt) {
	//update the cooldown
	_TTL = _TTL - dt;
	if(_TTL < 0) _TTL = 0;
	
	if(_TTL == 0) {
		UndoMode();
	}
}

void CoolDownMode::UndoMode() {
	//tell the actor we are ready to perform the action
	Actor *host = GetHost();
	host->SetMode(new IdleMode(host));
}

/**
 * VisualEffect
 *
 *
 *
 *	So vague, I don't even know what to do.  Aw hamburgers.
 *
 *
 */
 
VisualEffect::VisualEffect(std::string am, hoa_video::AnimatedImage i) {
	_animationMode = am;
	_image = i;
}

void VisualEffect::Draw() {

}

std::string VisualEffect::GetAnimationMode() {
	return _animationMode;
}


/**
 * ActorEffect
 *
 *
 *
 *
 *
 */

ActorEffect::ActorEffect(Actor *a, std::string name, uint32 ttl, uint32 ctc,
					uint32 ul, VisualEffect *ve, SoundDescriptor se) {
	_host = a;
	_EffectName = name;
	_TTL = ttl;
	_chanceToCure = ctc;
	_updateLength = ul;
	_visualEffect = ve;
	_soundEffect = se;
	
	//here, we actually need system time
	_age = 0;
	_lastUpdate = _age;
}

ActorEffect::~ActorEffect() {
	delete _visualEffect;
}

uint32 ActorEffect::GetTTL() {
	return _TTL;
}

void ActorEffect::SubtractTTL(uint32 dt) {
	_TTL = _TTL - dt;
	if(_TTL < 0) _TTL = 0;
}

void ActorEffect::Update(uint32 dt) {
	SubtractTTL(dt);
}

Actor *ActorEffect::GetHost() {
	return _host;
}

std::string ActorEffect::GetEffectName() {
	return _EffectName;
}

uint32 ActorEffect::GetChanceToCure() {
	return _chanceToCure;
}

uint32 ActorEffect::GetUpdateLength() {
	return _updateLength;
}

uint32 ActorEffect::GetLastUpdate() {
	return _lastUpdate;
}

void ActorEffect::SetLastUpdate(uint32 lu) {
	_lastUpdate = lu;
}

VisualEffect *ActorEffect::GetVisualEffect() {
	return _visualEffect;
}

SoundDescriptor ActorEffect::GetSoundEffect() {
	return _soundEffect;
}
		
/**
 * Action
 *
 *
 *
 *
 *
 */

Action::Action(Actor *p, std::vector<Actor *> args) {
	_host = p;
	_arguments = args;
}

Action::~Action() {
}

Actor *Action::GetHost() {
	return _host;
}

std::vector<Actor *> Action::GetArguments() {
	return _arguments;
}

/**
 * SkillAction
 *
 *
 *
 *
 *
 */

SkillAction::SkillAction(GSkill *s, Actor *p, std::vector<Actor *> args) :
Action(p, args)
{
	_skill = s;
}

SkillAction::~SkillAction() {
	FinishAction();
}

void SkillAction::PerformAction() {
	PerformSkill();
}

void SkillAction::PerformSkill() {
	//here, from GSkill, we work our magic

	//okay, here we actually perform the skill 
	std::cout << "SkillAction: Perform Skill" << std::endl;
	_skill->PerformSkill(GetHost(), GetArguments());
}

void SkillAction::PerformCooldown() {
	//we either go into cooldown mode or idle mode, based 
	//on the skill
	if(_skill->GetCooldownTime() > 0) {
		GetHost()->SetMode(new CoolDownMode(_skill->GetCooldownTime(), GetHost()));
	}
	else {
		GetHost()->SetMode(new IdleMode(GetHost()));
	}
}

void SkillAction::FinishAction() {
	PerformCooldown();
}

/**
 * SwapAction
 *
 *
 *
 *
 *
 *
 */
 
SwapAction::SwapAction(Actor *p, std::vector<Actor *> args) :
Action(p, args)
{
}

SwapAction::~SwapAction() {
	FinishAction();
}

void SwapAction::PerformAction() {
	//here, we must tell the battle mode to swap the characters
	//from _PCsInBattle and _playerActors
	//so remove our host from _PCsInBattle and push it into _playerActors
	//and remove our argument from _playerActors and put it into _PCsInBattle
	
	Actor *host = GetHost();
	std::vector<Actor *> args = GetArguments();
	std::vector<Actor *>::iterator it = args.begin();
	
	//get the host to run out
	host->SetAnimation("RUNOUT");
	//where we want our new character to run to
	//uint32 destination_x = host->GetX();
	//uint32 destination_y = host->GetY();
	
	//get the argument to run in -- to destination_x and destination_y
	(*it)->SetAnimation("RUNIN");
}

void SwapAction::FinishAction() {
	Actor *host = GetHost();
	std::vector<Actor *> args = GetArguments();
	std::vector<Actor *>::iterator it = args.begin();
	
	host->SetMode(new IdleMode(host));
	(*it)->SetMode(new IdleMode(*it));
}

/**
 * UseItemAction
 *
 *
 *
 *
 *
 */
 
UseItemAction::UseItemAction(GItem *i, Actor *p, std::vector<Actor *> args) :
Action(p, args)
{
	_item = i;
}

UseItemAction::~UseItemAction() {
	FinishAction();
}

void UseItemAction::PerformAction() {
	//here, p uses GItem on args
	
	Actor *host = GetHost();
	//put the character into use item animation mode
	host->SetAnimation("USEITEM");
}

void UseItemAction::FinishAction() {
	Actor *host = GetHost();
	host->SetMode(new IdleMode(host));
}

/**
 * BattleMode
 *
 *
 *
 *
 *
 */
 
BattleMode::BattleMode() : _UserInterface(this) {
	cerr << "BATTLE: BattleMode constructor invoked." << endl;
	
	// Load the map from the Lua data file
	//DataManager->LoadMap(this, map_id);
	
	// To Tim: This sets up a cooriate system where "0, 0" is the top left hand corner of the screen,
	// extends to SCREEN_LENGTH and SCREEN_HEIGHT, and has 1 depth level (ie, it's 2D)
	
	cerr << "Making new skill." << endl;
	GSkill *slash = new GSkill("Slash", 5);
	cerr << "Making move relative to origin." << endl;
	MoveAction *ma = new MoveRelativeToOrigin(5, 5);
	cerr << "New perform skill." << endl;
	PerformSkill *ps = new PerformSkill();
	cerr << "Adding battle skill actions." << endl;
	PlayCharacterAnimation *ca = new PlayCharacterAnimation("SWORD ATTACK");
	slash->AddBattleAction(ma);
	slash->AddBattleAction(ps);
	slash->AddBattleAction(ca);
	
	
	cerr << "Adding the skill to claudius." << endl;
	GCharacter *claud = InstanceManager->GetCharacter(GLOBAL_CLAUDIUS);
	//claud->AddAttackSkill(slash);
	
	cerr << "Creating claudius player character." << endl;
	PlayerActor *claudius = new PlayerActor(claud, this, 0, 0);
	_playerActors.push_back(claudius);
	_PCsInBattle.push_back(claudius);
	
	GEnemy *e = new GEnemy();
	cerr << "Adding the skill to the enemy." << endl;
	e->AddSkill(slash);
	EnemyActor *enemy = new EnemyActor(e, this, 1, 1);
	_enemyActors.push_back(enemy);
		
		
	_performingAction = false;
	
	Reset();
}


BattleMode::~BattleMode() {
	// Clean up any allocated music/images/sounds/whatever data here. Don't forget! ^_~
	cerr << "BATTLE: BattleMode destructor invoked." << endl;
	
	//clean up our actors
	std::list<PlayerActor *>::iterator pc_itr = _playerActors.begin();
	std::list<EnemyActor *>::iterator enemy_itr = _enemyActors.begin();
	
	for(; pc_itr != _playerActors.end(); pc_itr++) {
		delete *pc_itr;
	}
	for(; enemy_itr != _enemyActors.end(); enemy_itr++) {
		delete *enemy_itr;
	}
}


void BattleMode::Reset() {
	// Setup the coordinate system
	VideoManager->SetCoordSys(0.0f, (float)SCREEN_LENGTH, 0.0f, (float)SCREEN_HEIGHT);
}


void BattleMode::Update(uint32 time_elapsed) {
	// This function is the top level function that updates the status of the game. You'll likely write
	// several sub-functions that this function calls to keep the size of the function a sane amount.
	// For example, UpdateCharacters() and UpdateEnemies(). Make these sub-functions private, because
	// nothing else should need to know about them.
	
	// time_elapsed tells us how long it's been since the last time this function was called.
	
	//get input
	std::cout << "Updating with DT of: " << time_elapsed << std::endl;
	
	//update our characters
	std::list<PlayerActor *>::iterator pc_itr = _PCsInBattle.begin();
	std::list<EnemyActor *>::iterator enemy_itr = _enemyActors.begin();
	
	//is the battle over?
	if(pc_itr == _PCsInBattle.end() || enemy_itr == _enemyActors.end()) {
		ShutDown();
	}
	
	//update the concurrent battle actions
	for(unsigned int i = 0; i < _concurrentActions.size(); i++) {
		_concurrentActions[i]->Update(time_elapsed);
	}
	
	//update the PCs
	for(; pc_itr != _PCsInBattle.end(); pc_itr++) {
		(*pc_itr)->Update(time_elapsed);
	}
	//update the enemies
	for(; enemy_itr != _enemyActors.end(); enemy_itr++) {
		(*enemy_itr)->Update(time_elapsed);
	}
	
	if(_actionQueue.size() > 0 && !IsPerformingAction()) {
		std::list<Actor *>::iterator it = _actionQueue.begin();
		_currentlyPerforming = *it;
		_actionQueue.pop_front();
		SetPerformingAction(true);
		_currentlyPerforming->PerformAction();
	}
}

void BattleMode::ShutDown() {
	ModeManager->Pop();
}

void BattleMode::Draw() {
	// This function draws the next frame that will be displayed to the screen. Like Update(), you'll
	// probably write several sub-functions to keep the size of this function manageable. 
	DrawBackground();
	DrawCharacters();
}

void BattleMode::DrawBackground() {
	std::cout << "Draw the Background." << std::endl;
}

bool BattleMode::IsPerformingAction() {
	return _performingAction;
}

void BattleMode::SetPerformingAction(bool isPerforming) {
	_performingAction = isPerforming;
}

void BattleMode::DrawCharacters() {
	//update our characters
	std::list<PlayerActor *>::iterator pc_itr = _PCsInBattle.begin();
	std::list<EnemyActor *>::iterator enemy_itr = _enemyActors.begin();
	
	//update the PCs
	for(; pc_itr != _PCsInBattle.end(); pc_itr++) {
		(*pc_itr)->Draw();
	}
	//update the enemies
	for(; enemy_itr != _enemyActors.end(); enemy_itr++) {
		(*enemy_itr)->Draw();
	}
}

void BattleMode::AddToActionQueue(Actor *a) {
	_actionQueue.push_back(a);
}

void BattleMode::RemoveFromActionQueue(Actor *a) {
	_actionQueue.remove(a);
}

void BattleMode::AddConcurrentBattleAction(BattleAction *act) {
	_concurrentActions.push_back(act);
}

std::list<PlayerActor *> BattleMode::ReturnCharacters() {
	return _PCsInBattle;
}

} // namespace hoa_battle
