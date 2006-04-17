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
        _owner_battle_mode(ABattleMode),
        _x_origin(AXLocation),
        _y_origin(AYLocation),
        _x_location(AXLocation),
        _y_location(AYLocation),
        _max_skill_points(0),
        _current_skill_points(0),
        _is_move_capable(true),
        _is_alive(true),
        _next_action(NULL),
        _performing_action(false),
        _warmup_time(0),
        _cooldown_time(0),
        _defensive_mode_bonus(0),
        _total_strength_modifier(0),
        _total_agility_modifier(0),
        _total_intelligence_modifier(0),
        _animation("NEUTRAL")
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
        _is_alive = false;
}

bool Actor::IsAlive() const {
        return _is_alive;
}

/*!
        Get the mode we are currently fighting in
*/
const BattleMode *Actor::GetOwnerBattleMode() const {
        return _owner_battle_mode;
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
        _next_action = ANextAction;
}

void Actor::PerformAction() {
        SetPerformingAction(true);
        /*   ...   */
}

bool Actor::HasNextAction() const {
        return _next_action != NULL;
}

/*!
        Is the player frozen, asleep, et cetera?
*/
bool Actor::IsMoveCapable() const {
        return _is_move_capable;
}

void Actor::SetMoveCapable(bool AMoveCapable) {
        _is_move_capable = AMoveCapable;
}

/*!
        If the player is warming up, it really can't do anything
        Sort of a special case
*/
bool Actor::IsWarmingUp() const {
        return _warmup_time != 0;
}

void Actor::SetWarmupTime(uint32 AWarmupTime) {
        _warmup_time = AWarmupTime;
}

/*!
        Defensive mode boosts defense
*/
bool Actor::IsInDefensiveMode() const {
        return _defensive_mode_bonus != 0;
}
		
void Actor::SetDefensiveBonus(uint32 ADefensiveBonus) {
        _defensive_mode_bonus = ADefensiveBonus;
}

/*!
        If we are currently performing an action we can't do anything else
        on the update
*/
bool Actor::IsPerformingAction() const {
        return _performing_action;
}

void Actor::SetPerformingAction(bool AIsPerforming) {
        _performing_action = AIsPerforming;
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
        _total_strength_modifier = AStrengthModifier;
}

uint32 Actor::GetTotalStrengthModifier() {
        return _total_strength_modifier;
}

void Actor::SetTotalAgilityModifier(uint32 AAgilityModifier) {
        _total_agility_modifier = AAgilityModifier;
}

uint32 Actor::GetTotalAgilityModifier() {
        return _total_agility_modifier;
}

void Actor::SetTotalIntelligenceModifier(uint32 AIntelligenceModifier) {
        _total_intelligence_modifier = AIntelligenceModifier;
}

uint32 Actor::GetTotalIntelligenceModifier() {
        return _total_intelligence_modifier;
}

/*

        BATTLEUI

*/

BattleUI::BattleUI(BattleMode * const ABattleMode) :
        _bm(ABattleMode),
        _currently_selected_actor(NULL),
        _necessary_selections(0),
        _current_hover_selection(0),
        _number_menu_items(0)
{

}
		
/*!
        Get the actor we are currently on
*/
Actor * const BattleUI::GetSelectedActor() const {
        return _currently_selected_actor;
}

/*!
        We clicked on an actor
*/
void BattleUI::SetActorSelected(Actor * const AWhichActor) {
        _currently_selected_actor = AWhichActor;
}

/*!
        No actor is selected...we are now selecting an actor
*/
void BattleUI::DeselectActor() {
        _currently_selected_actor = NULL;
}      

/*!
        Get other people selected
*/
std::list<Actor *> BattleUI::GetSelectedArgumentActors() const {
        return _currently_selected_argument_actors;
}

/*!
        The actor we just selected is now an argument
*/
void BattleUI::SetActorAsArgument(Actor * const AActor) {
        _currently_selected_argument_actors.push_back(AActor);
}

/*!
        No longer do we want this actor as an argument
*/
void BattleUI::RemoveActorAsArgument(Actor * const AActor) {
        _currently_selected_argument_actors.remove(AActor);
}

/*!
        Sets the number of arguments we should be allowing
*/
void BattleUI::SetNumberNecessarySelections(uint32 ANumSelections) {
        _necessary_selections = ANumSelections;
}

/*! PlayerCharacter 


*/


PlayerActor::PlayerActor(GlobalCharacter * const AWrapped, BattleMode * const ABattleMode, uint32 AXLoc, uint32 AYLoc) :
        Actor(ABattleMode, AXLoc, AYLoc),
        _wrapped_character(AWrapped) {
        
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
       return _wrapped_character->GetAttackSkills();
}

std::vector<GlobalSkill *> PlayerActor::GetDefenseSkills() const {
        return _wrapped_character->GetDefenseSkills();
}

std::vector<GlobalSkill *> PlayerActor::GetSupportSkills() const {
        return _wrapped_character->GetSupportSkills();
}

/*!
        More getters from GlobalCharacter
*/
const std::string PlayerActor::GetName() const {
        return _wrapped_character->GetName();
}

const std::vector<GlobalAttackPoint> PlayerActor::GetAttackPoints() const {
        return _wrapped_character->GetAttackPoints();
}

uint32 PlayerActor::GetHealth() const {
        return _wrapped_character->GetHP();
}

void PlayerActor::SetHealth(uint32 AHealth) {
        _wrapped_character->SetHP(AHealth);
}

uint32 PlayerActor::GetMaxHealth() const {
        return _wrapped_character->GetMaxHP();
}

uint32 PlayerActor::GetSkillPoints() const {
        return _wrapped_character->GetSP();
}

void PlayerActor::SetSkillPoints(uint32 ASkillPoints) {
        _wrapped_character->SetSP(ASkillPoints);
}

uint32 PlayerActor::GetMaxSkillPoints() const {
        return _wrapped_character->GetMaxSP();
}

uint32 PlayerActor::GetStrength() const {
        return _wrapped_character->GetStrength();
}

uint32 PlayerActor::GetIntelligence() const {
        return _wrapped_character->GetIntelligence();
}

uint32 PlayerActor::GetAgility() const {
        return _wrapped_character->GetAgility();
}

uint32 PlayerActor::GetMovementSpeed() const {
        return _wrapped_character->GetMovementSpeed();
}


/*! EnemyActor

*/

EnemyActor::EnemyActor(GlobalEnemy AGlobalEnemy, BattleMode * const ABattleMode, uint32 AXLoc, uint32 AYLoc) :
        Actor(ABattleMode, AXLoc, AYLoc),
        _wrapped_enemy(AGlobalEnemy) {
        
        
        
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
        _wrapped_enemy.LevelSimulator(AAverageLevel);
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
        return _wrapped_enemy.GetSkills();
}

std::string EnemyActor::GetName() const {
        return _wrapped_enemy.GetName();
}

const std::vector<GlobalAttackPoint> EnemyActor::GetAttackPoints() const {
        return _wrapped_enemy.GetAttackPoints();
}

uint32 EnemyActor::GetHealth() const {
        return _wrapped_enemy.GetHP();
}

void EnemyActor::SetHealth(uint32 AHealth) {
        _wrapped_enemy.SetHP(AHealth);
}

uint32 EnemyActor::GetMaxHealth() const {
        return _wrapped_enemy.GetMaxHP();
}

uint32 EnemyActor::GetSkillPoints() const {
        return _wrapped_enemy.GetSP();
}

void EnemyActor::SetSkillPoints(uint32 ASkillPoints) {
        _wrapped_enemy.SetSP(ASkillPoints);
}

uint32 EnemyActor::GetMaxSkillPoints() const {
        return _wrapped_enemy.GetMaxSP();
}

uint32 EnemyActor::GetStrength() const {
        return _wrapped_enemy.GetStrength();
}

uint32 EnemyActor::GetIntelligence() const {
        return _wrapped_enemy.GetIntelligence();
}

uint32 EnemyActor::GetAgility() const {
        return _wrapped_enemy.GetAgility();
}

uint32 EnemyActor::GetMovementSpeed() const {
        return _wrapped_enemy.GetMovementSpeed();
}

/*! Actions

*/

Action::Action(Actor * const AHostActor, std::vector<Actor *> AArguments, const std::string ASkillName) :
        	_host(AHostActor),
		_arguments(AArguments),
                _skill_name(ASkillName)
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
        return _skill_name;
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
        _effect_name(AEffectName),
        _TTL(ATTL),
        _severeness(AHowSevere),
        _can_move(ACanMove),
        _health_modifier(AHealthModifier),
        _skill_point_modifier(ASkillPointModifier),
        _strength_modifier(ASkillPointModifier),
        _intelligence_modifier(AIntelligenceModifier),
        _agility_modifier(AAgilityModifier),
        _update_length(AUpdateLength),
        _age(0),
        _times_updated(0)
{
        _last_update = SettingsManager->GetUpdateTime();
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
        return _effect_name;
}

uint32 ActorEffect::GetUpdateLength() const {
        return _update_length;
}

uint32 ActorEffect::GetLastUpdate() const {
        return _last_update;
}

bool ActorEffect::CanMove() const {
        return _can_move;
}

uint32 ActorEffect::GetHealthModifier() const {
        return _health_modifier;
}

uint32 ActorEffect::GetSkillPointModifier() const {
        return _skill_point_modifier;
}

uint32 ActorEffect::GetStrengthModifier() const {
        return _strength_modifier;
}

uint32 ActorEffect::GetIntelligenceModifier() const {
        return _intelligence_modifier;
}

uint32 ActorEffect::GetAgilityModifier() const {
        return _agility_modifier;
}

void ActorEffect::SetLastUpdate(uint32 ALastUpdate) {
        _last_update = ALastUpdate;
}

void ActorEffect::UndoEffect() const {

}

/*!
        Scripted Event class wrapper
*/
        
ScriptEvent::ScriptEvent(Actor *AHost, std::list<Actor *> AArguments, std::string AScriptName) :
        _script_name(AScriptName),
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
        _user_interface(this),
        _performing_script(false)
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
        std::deque<PlayerActor *>::iterator pc_itr = _player_actors.begin();
	for(; pc_itr != _player_actors.end(); pc_itr++) {
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
        
        for(unsigned int i = 0; i < _players_characters_in_battle.size(); i++) {
                _players_characters_in_battle[i]->Update(updateTime);
        }
        
        for(unsigned int i = 0; i < _enemy_actors.size(); i++) {
                _enemy_actors[i]->Update(updateTime);
                _enemy_actors[i]->DoAI();
        }
        
        //check if any scripts need to run here
}

//! Wrapper function that calls different draw functions depending on the battle state.
void BattleMode::Draw() {
        _DrawBackground();
        _DrawCharacters();
}

void BattleMode::_DrawBackground() {
        VideoManager->Move(0,0);
	VideoManager->SetDrawFlags(VIDEO_NO_BLEND, 0);
	VideoManager->DrawImage(_battle_images[0]);
}

void BattleMode::_DrawCharacters() {
        for(unsigned int i = 0; i < _players_characters_in_battle.size(); i++) {
                _players_characters_in_battle[i]->Draw();
        }
        
        for(unsigned int i = 0; i < _enemy_actors.size(); i++) {
                _enemy_actors[i]->Draw();
        }
}

//! Shutdown the battle mode
void BattleMode::_ShutDown() {
        ModeManager->Pop();
}

//!Are we performing an action
bool BattleMode::_IsPerformingScript() {
        return _performing_script;
}

//! Sets T/F whether an action is being performed
void BattleMode::SetPerformingScript(bool AIsPerforming) {
        _performing_script = AIsPerforming;
}

//! Adds an actor waiting to perform an action to the queue.
void BattleMode::AddToActionQueue(Actor *AActorToAdd) {
        _action_queue.push_back(AActorToAdd);
}

//! Removes an actor from the action queue (perhaps they died, et cetera)
void BattleMode::RemoveFromActionQueue(Actor *AActorToRemove) {
        _action_queue.remove(AActorToRemove);
}

 void BattleMode::AddScriptEventToQueue(ScriptEvent AEventToAdd) {
        _script_queue.push_back(AEventToAdd);
 }
        
//! Remove all scripted events for an actor
void BattleMode::RemoveScriptedEventsForActor(Actor *AActorToRemove) {
        std::list<ScriptEvent>::iterator it = _script_queue.begin();
        
        while( it != _script_queue.end() ) { 
                if((*it).GetHost() == AActorToRemove) {
                        _script_queue.erase(it); //remove this location
                }
                else //otherwise, increment the iterator 
                        it++; 
        }
}

//! Returns all player actors
std::deque<PlayerActor *> BattleMode::ReturnCharacters() const {
        return _players_characters_in_battle;
}

                
} // namespace hoa_battle

