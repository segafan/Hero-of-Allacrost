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
 * \author  Corey Hoffstein, visage@allacrost.org
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

namespace private_battle {

/*

        ACTOR

*/

Actor::Actor(BattleMode *ABattleMode, uint32 AXLocation, uint32 AYLocation) :
        _ownerBattleMode(ABattleMode),
        _X_Origin(AXLocation),
        _Y_Origin(AYLocation),
        _X_Location(AXLocation),
        _Y_Location(AYLocation),
        _maxSkillPoints(0),
        _currentSkillPoints(0),
        _isMoveCapable(true),
        _isAlive(true),
        _nextAction(NULL),
        _performingAction(false),
        _warmupTime(0),
        _cooldownTime(0),
        _defensiveModeBonus(0),
        _totalStrengthModifier(0),
        _totalAgilityModifier(0),
        _totalIntelligenceModifier(0),
        _animation("NEUTRAL")
{

}

Actor::Actor(const Actor& AOtherActor) :
        _ownerBattleMode(AOtherActor._ownerBattleMode),
        _X_Origin(AOtherActor._X_Origin),
        _Y_Origin(AOtherActor._Y_Origin),
        _X_Location(AOtherActor._X_Location),
        _Y_Location(AOtherActor._Y_Location),
        _maxSkillPoints(AOtherActor._maxSkillPoints),
        _currentSkillPoints(AOtherActor._currentSkillPoints),
        _isMoveCapable(AOtherActor._isMoveCapable),
        _isAlive(AOtherActor._isAlive),
        _nextAction(AOtherActor._nextAction),
        _performingAction(AOtherActor._performingAction),
        _warmupTime(AOtherActor._warmupTime),
        _cooldownTime(AOtherActor._cooldownTime),
        _defensiveModeBonus(AOtherActor._defensiveModeBonus),
        _totalStrengthModifier(AOtherActor._totalStrengthModifier),
        _totalAgilityModifier(AOtherActor._totalAgilityModifier),
        _totalIntelligenceModifier(AOtherActor._totalIntelligenceModifier),
        _animation(AOtherActor._animation)
{

}

/*!
        Need to implement this!
*/
Actor::~Actor() {

}

/*!
        Stuff relating to, you know, death
*/
void Actor::Die() {
        _isAlive = false;
}

bool Actor::IsAlive() const {
        return _isAlive;
}

/*!
        Get the mode we are currently fighting in
*/
const BattleMode *Actor::GetOwnerBattleMode() const {
        return _ownerBattleMode;
}

/*!
        Manage effects that the player is feeling
*/
void Actor::UpdateEffects(uint32 ATimeElapsed) {
        for(unsigned int i = 0; i < _effects.size(); i++) {
                _effects[i].Update(ATimeElapsed);
        }
}

void Actor::PushEffect(const ActorEffect AEffect) {
        _effects.push_back(AEffect);
}

/*!
        Set the next action, action related methods
*/
void Actor::SetNextAction(Action * const ANextAction) {
        _nextAction = ANextAction;
}

void Actor::PerformAction() {
        SetPerformingAction(true);
        /*   ...   */
}

bool Actor::HasNextAction() const {
        return _nextAction != NULL;
}

/*!
        Is the player frozen, asleep, et cetera?
*/
bool Actor::IsMoveCapable() const {
        return _isMoveCapable;
}

void Actor::SetMoveCapable(bool AMoveCapable) {
        _isMoveCapable = AMoveCapable;
}

/*!
        If the player is warming up, it really can't do anything
        Sort of a special case
*/
bool Actor::IsWarmingUp() const {
        return _warmupTime != 0;
}

void Actor::SetWarmupTime(uint32 AWarmupTime) {
        _warmupTime = AWarmupTime;
}

/*!
        Defensive mode boosts defense
*/
bool Actor::IsInDefensiveMode() const {
        return _defensiveModeBonus != 0;
}
		
void Actor::SetDefensiveBonus(uint32 ADefensiveBonus) {
        _defensiveModeBonus = ADefensiveBonus;
}

/*!
        If we are currently performing an action we can't do anything else
        on the update
*/
bool Actor::IsPerformingAction() const {
        return _performingAction;
}

void Actor::SetPerformingAction(bool AIsPerforming) {
        _performingAction = AIsPerforming;
}

/*!
        GlobalCharacter and GlobalEnemy will use a Map of sorts
        to map strings to image animations
        This sets our characters animation
*/
void Actor::SetAnimation(std::string ACurrentAnimation) {
        _animation = ACurrentAnimation;
}

const std::string Actor::GetAnimation() const {
        return _animation;
}


void Actor::SetTotalStrengthModifier(uint32 AStrengthModifier) {
        _totalStrengthModifier = AStrengthModifier;
}

uint32 Actor::GetTotalStrengthModifier() {
        return _totalStrengthModifier;
}

void Actor::SetTotalAgilityModifier(uint32 AAgilityModifier) {
        _totalAgilityModifier = AAgilityModifier;
}

uint32 Actor::GetTotalAgilityModifier() {
        return _totalAgilityModifier;
}

void Actor::SetTotalIntelligenceModifier(uint32 AIntelligenceModifier) {
        _totalIntelligenceModifier = AIntelligenceModifier;
}

uint32 Actor::GetTotalIntelligenceModifier() {
        return _totalIntelligenceModifier;
}

/*

        BATTLEUI

*/

BattleUI::BattleUI(BattleMode * const ABattleMode) :
        _bm(ABattleMode),
        _currentlySelectedActor(NULL),
        _necessarySelections(0),
        _currentHoverSelection(0),
        _numberMenuItems(0)
{

}
		
/*!
        Get the actor we are currently on
*/
Actor * const BattleUI::GetSelectedActor() const {
        return _currentlySelectedActor;
}

/*!
        We clicked on an actor
*/
void BattleUI::SetActorSelected(Actor * const AWhichActor) {
        _currentlySelectedActor = AWhichActor;
}

/*!
        No actor is selected...we are now selecting an actor
*/
void BattleUI::DeselectActor() {
        _currentlySelectedActor = NULL;
}      

/*!
        Get other people selected
*/
std::list<Actor *> BattleUI::GetSelectedArgumentActors() const {
        return _currentlySelectedArgumentActors;
}

/*!
        The actor we just selected is now an argument
*/
void BattleUI::SetActorAsArgument(Actor * const AActor) {
        _currentlySelectedArgumentActors.push_back(AActor);
}

/*!
        No longer do we want this actor as an argument
*/
void BattleUI::RemoveActorAsArgument(Actor * const AActor) {
        _currentlySelectedArgumentActors.remove(AActor);
}

/*!
        Sets the number of arguments we should be allowing
*/
void BattleUI::SetNumberNecessarySelections(uint32 ANumSelections) {
        _necessarySelections = ANumSelections;
}

/*! PlayerCharacter 


*/


PlayerActor::PlayerActor(GlobalCharacter * const AWrapped, BattleMode * const ABattleMode, uint32 AXLoc, uint32 AYLoc) :
        Actor(ABattleMode, AXLoc, AYLoc),
        _wrappedCharacter(AWrapped) {
        
}

PlayerActor::~PlayerActor() {

}

void PlayerActor::Update(uint32 ATimeElapsed) {

}

void PlayerActor::Draw() {

}

/*!
        Get the skills from GlobalCharacter
*/
std::vector<GlobalSkill *> PlayerActor::GetAttackSkills() const {
       return _wrappedCharacter->GetAttackSkills();
}

std::vector<GlobalSkill *> PlayerActor::GetDefenseSkills() const {
        return _wrappedCharacter->GetDefenseSkills();
}

std::vector<GlobalSkill *> PlayerActor::GetSupportSkills() const {
        return _wrappedCharacter->GetSupportSkills();
}

/*!
        More getters from GlobalCharacter
*/
const std::string PlayerActor::GetName() const {
        return _wrappedCharacter->GetName();
}

const std::vector<GlobalAttackPoint> PlayerActor::GetAttackPoints() const {
        return _wrappedCharacter->GetAttackPoints();
}

uint32 PlayerActor::GetHealth() const {
        return _wrappedCharacter->GetHP();
}

void PlayerActor::SetHealth(uint32 AHealth) {
        _wrappedCharacter->SetHP(AHealth);
}

uint32 PlayerActor::GetMaxHealth() const {
        return _wrappedCharacter->GetMaxHP();
}

uint32 PlayerActor::GetSkillPoints() const {
        return _wrappedCharacter->GetSP();
}

void PlayerActor::SetSkillPoints(uint32 ASkillPoints) {
        _wrappedCharacter->SetSP(ASkillPoints);
}

uint32 PlayerActor::GetMaxSkillPoints() const {
        return _wrappedCharacter->GetMaxSP();
}

uint32 PlayerActor::GetStrength() const {
        return _wrappedCharacter->GetStrength();
}

uint32 PlayerActor::GetIntelligence() const {
        return _wrappedCharacter->GetIntelligence();
}

uint32 PlayerActor::GetAgility() const {
        return _wrappedCharacter->GetAgility();
}

uint32 PlayerActor::GetMovementSpeed() const {
        return _wrappedCharacter->GetMovementSpeed();
}


/*! EnemyActor

*/

EnemyActor::EnemyActor(GlobalEnemy AGlobalEnemy, BattleMode * const ABattleMode, uint32 AXLoc, uint32 AYLoc) :
        Actor(ABattleMode, AXLoc, AYLoc),
        _wrappedEnemy(AGlobalEnemy) {
        
        
        
}

EnemyActor::~EnemyActor() {

}

void EnemyActor::Update(uint32 ATimeElapsed) {

}

void EnemyActor::Draw() {

}

/*!
        Has the GlobalEnemy level up to average_level
*/
void EnemyActor::LevelUp(uint32 AAverageLevel) {
        _wrappedEnemy.LevelSimulator(AAverageLevel);
}

/*!
        The AI routine
*/
void EnemyActor::DoAI() {

}

/*!
        GlobalEnemy getters
*/
const std::vector<GlobalSkill *> EnemyActor::GetSkills() const {
        return _wrappedEnemy.GetSkills();
}

std::string EnemyActor::GetName() const {
        return _wrappedEnemy.GetName();
}

const std::vector<GlobalAttackPoint> EnemyActor::GetAttackPoints() const {
        return _wrappedEnemy.GetAttackPoints();
}

uint32 EnemyActor::GetHealth() const {
        return _wrappedEnemy.GetHP();
}

void EnemyActor::SetHealth(uint32 AHealth) {
        _wrappedEnemy.SetHP(AHealth);
}

uint32 EnemyActor::GetMaxHealth() const {
        return _wrappedEnemy.GetMaxHP();
}

uint32 EnemyActor::GetSkillPoints() const {
        return _wrappedEnemy.GetSP();
}

void EnemyActor::SetSkillPoints(uint32 ASkillPoints) {
        _wrappedEnemy.SetSP(ASkillPoints);
}

uint32 EnemyActor::GetMaxSkillPoints() const {
        return _wrappedEnemy.GetMaxSP();
}

uint32 EnemyActor::GetStrength() const {
        return _wrappedEnemy.GetStrength();
}

uint32 EnemyActor::GetIntelligence() const {
        return _wrappedEnemy.GetIntelligence();
}

uint32 EnemyActor::GetAgility() const {
        return _wrappedEnemy.GetAgility();
}

uint32 EnemyActor::GetMovementSpeed() const {
        return _wrappedEnemy.GetMovementSpeed();
}

/*! Actions

*/

Action::Action(Actor * const AHostActor, std::vector<Actor *> AArguments, const std::string ASkillName) :
        	_host(AHostActor),
		_arguments(AArguments),
                _skillName(ASkillName)
{

}

Action::~Action() {

}

Actor * const Action::GetHost() const {
        return _host;
}

const std::vector<Actor *> Action::GetArguments() const {
        return _arguments;
}
                
const std::string Action::GetSkillName() const {
        return _skillName;
}

/*!
        Effect

*/

ActorEffect::ActorEffect(Actor * const AHost, std::string AEffectName, StatusSeverity AHowSevere,
                                uint32 ATTL, bool ACanMove, uint32 AHealthModifier, 
                                uint32 ASkillPointModifier, uint32 AStrengthModifier, 
                                uint32 AIntelligenceModifier, uint32 AAgilityModifier, 
                                uint32 AUpdateLength) : 
        _host(AHost),
        _EffectName(AEffectName),
        _TTL(ATTL),
        _severeness(AHowSevere),
        _canMove(ACanMove),
        _healthModifier(AHealthModifier),
        _skillPointModifier(ASkillPointModifier),
        _strengthModifier(ASkillPointModifier),
        _intelligenceModifier(AIntelligenceModifier),
        _agilityModifier(AAgilityModifier),
        _updateLength(AUpdateLength),
        _age(0),
        _timesUpdated(0)
{
        _lastUpdate = SettingsManager->GetUpdateTime();
}

ActorEffect::~ActorEffect() {

}

uint32 ActorEffect::GetTTL() const {
        return _TTL;
}

void ActorEffect::Update(uint32 ATimeElapsed) {

}

Actor * const ActorEffect::GetHost() const {
        return _host;
}

std::string ActorEffect::GetEffectName() const {
        return _EffectName;
}

uint32 ActorEffect::GetUpdateLength() const {
        return _updateLength;
}

uint32 ActorEffect::GetLastUpdate() const {
        return _lastUpdate;
}

bool ActorEffect::CanMove() const {
        return _canMove;
}

uint32 ActorEffect::GetHealthModifier() const {
        return _healthModifier;
}

uint32 ActorEffect::GetSkillPointModifier() const {
        return _skillPointModifier;
}

uint32 ActorEffect::GetStrengthModifier() const {
        return _strengthModifier;
}

uint32 ActorEffect::GetIntelligenceModifier() const {
        return _intelligenceModifier;
}

uint32 ActorEffect::GetAgilityModifier() const {
        return _agilityModifier;
}

void ActorEffect::SetLastUpdate(uint32 ALastUpdate) {
        _lastUpdate = ALastUpdate;
}

void ActorEffect::UndoEffect() const {

}

/*!
        Scripted Event class wrapper
*/
        
ScriptEvent::ScriptEvent(Actor *AHost, std::list<Actor *> AArguments, std::string AScriptName) :
        _scriptName(AScriptName),
        _host(AHost),
        _arguments(AArguments)
{

}

ScriptEvent::~ScriptEvent() {

}

void ScriptEvent::RunScript() {
        //get script from global script repository and run,
        //passing in list of arguments and host actor
}

Actor *ScriptEvent::GetHost() {
        return _host;
}

} //end namespace for private battle

/*!
        The actual battle mode
*/


BattleMode::BattleMode() : 
        _UserInterface(this),
        _performingScript(false)
{
        Reset();
        
        //Test load a background
        StillImage backgrd;
	backgrd.SetFilename("img/backdrops/battle_screen.jpg");
        backgrd.SetDimensions(SCREEN_LENGTH*TILE_SIZE, SCREEN_HEIGHT*TILE_SIZE);
        _battle_images.push_back(backgrd);
	if(!VideoManager->LoadImage(_battle_images[0]))
		cerr << "Failed to load background image." << endl; //failed to laod image
}

BattleMode::~BattleMode() {
        // Delete the player actors.  Don't want to waste memory
        std::deque<PlayerActor *>::iterator pc_itr = _playerActors.begin();
	for(; pc_itr != _playerActors.end(); pc_itr++) {
		delete *pc_itr;
	}

        //get rid of the battle images that we created (TEMP)
        for (uint32 i = 0; i < _battle_images.size(); i++) {
		VideoManager->DeleteImage(_battle_images[i]);
	}
}

//! Resets appropriate class members. Called whenever BootMode is made the active game mode.
void BattleMode::Reset() {
        //VideoManager->SetCoordSys(0.0f, (float)SCREEN_LENGTH, 0.0f, (float)SCREEN_HEIGHT);
        VideoManager->SetCoordSys(0.0f, (float)SCREEN_LENGTH*TILE_SIZE, 0.0f,  (float)SCREEN_HEIGHT*TILE_SIZE);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
}

//! Wrapper function that calls different update functions depending on the battle state.
void BattleMode::Update() {
        uint32 updateTime = SettingsManager->GetUpdateTime();
        
        //check here for end conditions.  How many people are still alive?
        
        for(unsigned int i = 0; i < _PCsInBattle.size(); i++) {
                _PCsInBattle[i]->Update(updateTime);
        }
        
        for(unsigned int i = 0; i < _enemyActors.size(); i++) {
                _enemyActors[i]->Update(updateTime);
                _enemyActors[i]->DoAI();
        }
        
        //check if any scripts need to run here
}

//! Wrapper function that calls different draw functions depending on the battle state.
void BattleMode::Draw() {
        DrawBackground();
        DrawCharacters();
}

void BattleMode::DrawBackground() {
        VideoManager->Move(0,0);
	VideoManager->SetDrawFlags(VIDEO_NO_BLEND, 0);
	VideoManager->DrawImage(_battle_images[0]);
}

void BattleMode::DrawCharacters() {
        for(unsigned int i = 0; i < _PCsInBattle.size(); i++) {
                _PCsInBattle[i]->Draw();
        }
        
        for(unsigned int i = 0; i < _enemyActors.size(); i++) {
                _enemyActors[i]->Draw();
        }
}

//! Shutdown the battle mode
void BattleMode::ShutDown() {
        ModeManager->Pop();
}

//!Are we performing an action
bool BattleMode::IsPerformingScript() {
        return _performingScript;
}

//! Sets T/F whether an action is being performed
void BattleMode::SetPerformingScript(bool AIsPerforming) {
        _performingScript = AIsPerforming;
}

//! Adds an actor waiting to perform an action to the queue.
void BattleMode::AddToActionQueue(Actor *AActorToAdd) {
        _actionQueue.push_back(AActorToAdd);
}

//! Removes an actor from the action queue (perhaps they died, et cetera)
void BattleMode::RemoveFromActionQueue(Actor *AActorToRemove) {
        _actionQueue.remove(AActorToRemove);
}

 void BattleMode::AddScriptEventToQueue(ScriptEvent AEventToAdd) {
        _scriptQueue.push_back(AEventToAdd);
 }
        
//! Remove all scripted events for an actor
void BattleMode::RemoveScriptedEventsForActor(Actor *AActorToRemove) {
        std::list<ScriptEvent>::iterator it = _scriptQueue.begin();
        
        while( it != _scriptQueue.end() ) { 
                if((*it).GetHost() == AActorToRemove) {
                        _scriptQueue.erase(it); //remove this location
                }
                else //otherwise, increment the iterator 
                        it++; 
        }
}

//! Returns all player actors
std::deque<PlayerActor *> BattleMode::ReturnCharacters() const {
        return _PCsInBattle;
}

                
} // namespace hoa_battle

