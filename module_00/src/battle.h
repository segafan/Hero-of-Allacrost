///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    battle.h
 * \author  Tim Hargreaves, balthazar@allacrost.org
 * \date    Last Updated: August 17th, 2005
 * \brief   Header file for battle mode interface.
 *
 * This code handles the game event processing and frame drawing when the user
 * is fighting a battle. 
 *****************************************************************************/

#ifndef __BATTLE_HEADER__
#define __BATTLE_HEADER__

#include "utils.h"
#include <string>
#include <vector>
#include "defs.h"
#include "engine.h"

#include "global.h"
#include "video.h"
#include "audio.h"

using namespace hoa_global;
using namespace hoa_video;
using namespace hoa_audio;
using namespace hoa_utils;

namespace hoa_battle {

extern bool BATTLE_DEBUG;

/* The reason for this embedded namespace is so you can have constants with short and "friendly" names. Any
constants you make available inside hoa_battle namespace, you must prefix with BATTLE_ according to our code
standard (have you read it???) The only piece of code that uses the private_battle namespace should be battle.h
and battle.cpp, nothing more. So name those constants whatever you want! =D
*/
namespace private_battle {

const int32 TILE_SIZE = 64; // The virtual "tile map" that we discussed in the forums has square 64 pixel tiles
const int32 SCREEN_LENGTH = 16; // Number of tiles long the screen is
const int32 SCREEN_HEIGHT = 12; // The number of tiles high the screen is

}

class Actor {
	private:
		//!The mode we belong to
		BattleMode *_ownerBattleMode;
		//!The X location of the actor on the battle grid
		uint32 _X_Location;
		//!The Y location of the actor on the battle grid
		uint32 _Y_Location;
		//!A stack of the current modes effecting the character
		ActorMode *_mode;
		//!A list of effects and ailments on the character
		std::list<ActorEffect *> _effects;
		//!The maximum stamina
		uint32 _maxSkillPoints;
		//!The remaining level of stamina
		uint32 _currentSkillPoints;
		//!Tells whether the character can move (frozen, burned, et cetera)
		bool _isMoveCapable;
		//!Tells if the character is alive or dead
		bool _isAlive;
		//!The next action to perform
		Action *_nextAction;
		//!Are we performing the action right now?
		bool _performingAction;
		//!Are we warming up for the action?
		bool _warmingUp;
		//!Are we in defensive mode
		bool _defensiveMode;
		//!Are we being supported?  By whom?
		std::list<Actor *> _supporters;
		
	public:
		Actor(BattleMode *bm, uint32 x, uint32 y);
		Actor(const Actor&  a);
		
		virtual ~Actor();
		virtual void Update(uint32 dt) = 0;
		virtual void Draw() = 0;
		
		BattleMode *GetOwnerBattleMode();
		
		void SetMode(ActorMode *m);
		ActorMode *GetMode();
		
		void UpdateEffects(uint32 dt);
		void PushEffect(ActorEffect *e);
		void RemoveEffect(ActorEffect *e);
		
		void SetNextAction(Action *a);
		void PerformAction();
		bool HasNextAction();
		
		bool IsPerformingAction();
		void SetPerformingAction(bool performing);
		
		bool IsMoveCapable();
		void SetMoveCapable(bool capable);
		
		bool IsWarmingUp();
		void SetWarmingUp(bool warmup);
		
		void AddSupporter(Actor *a);
		void RemoveSupporter(Actor *a);
		
		void SetDefensiveMode(bool d);
		bool IsInDefensiveMode();
		
		void SetAnimation(std::string animationMode);
		
		virtual std::string GetName() = 0;
		virtual std::vector<GAttackPoint> GetAttackPoints() = 0;
		virtual uint32 GetHealth() = 0;
		virtual uint32 GetMaxHealth() = 0;
		virtual uint32 GetSkillPoints() = 0;
		virtual uint32 GetMaxSkillPoints() = 0;
		virtual uint32 GetStrength() = 0;
		virtual uint32 GetIntelligence() = 0;
		virtual uint32 GetAgility() = 0;
};

class BattleUI {
	private:
		//!The battlemode we belong to
		BattleMode *_bm;
		//!The current actor we have clicked on
		Actor *_currentlySelectedActor;
		//!The actors we have selected as arguments
		std::list<Actor *> _currentlySelectedArgumentActors;
		//!A stack of menu selections we have gone through
		std::vector<int> _currentlySelectedMenuItem;
		//!The number of selections that must be made for an action
		uint32 _necessarySelections;
		//!The menu item we are hovering over
		uint32 _currentHoverSelection;
		//!The number of items in this menu
		uint32 _numberMenuItems;
		
		//! Swapping
		uint32 _numSwapCards;
		uint32 _maxSwapCards;
		uint32 _lastTimeSwapAwarded;
		
	public:
		BattleUI(BattleMode *bm);
		
		Actor* GetSelectedActor();
		void SetActorSelected(Actor *a);
		void DeselectActor();
		std::list<Actor *> GetSelectedArgumentActors();
		void setActorAsArgument(Actor *a);
		void RemoveActorAsArgument(Actor *a);
		void SetNumberNecessarySelections(uint32 select);
};

class PlayerActor : public Actor {
	private:
		//!The global character we have wrapped around
		GCharacter *_wrappedCharacter;
	
	public:
		PlayerActor(GCharacter *_wrapped, BattleMode *bm, int x, int y);
		~PlayerActor();
		void Update(uint32 dt);
		void Draw();
		
		std::vector<GSkill> GetAttackSkills();
		std::vector<GSkill> GetDefenseSkills();
		std::vector<GSkill> GetSupportSkills();
		
		std::string GetName();
		std::vector<GAttackPoint> GetAttackPoints();
		uint32 GetHealth();
		uint32 GetMaxHealth();
		uint32 GetSkillPoints();
		uint32 GetMaxSkillPoints();
		uint32 GetStrength();
		uint32 GetIntelligence();
		uint32 GetAgility();
};

class EnemyActor : public Actor {
	private:
		//!The enemy we have wrapped around
		GEnemy *_wrappedEnemy;
		
	public:
		EnemyActor(GEnemy *ge, BattleMode *bm, int x, int y);
		~EnemyActor();
		void Update(uint32 dt);
		void Draw();

		void LevelUp(uint32 average_level);
		void DoAI(uint32 dt); //this should be scripted, dudes
		
		std::vector<GSkill> GetSkills();
		
		std::string GetName();
		std::vector<GAttackPoint> GetAttackPoints();
		uint32 GetHealth();
		uint32 GetMaxHealth();
		uint32 GetSkillPoints();
		uint32 GetMaxSkillPoints();
		uint32 GetStrength();
		uint32 GetIntelligence();
		uint32 GetAgility();
};

class ActorMode {
	private:
		//!Who we are effecting
		Actor *_host;
	
	public:
		ActorMode(Actor *a);
		virtual ~ActorMode();
		Actor *GetHost();
		
		virtual void Update(uint32 dt) = 0;
		virtual void UndoMode() = 0;
};

class SupportMode : public ActorMode {
	private: 
		std::list<Actor *> _supported;
		
	public:
		SupportMode(uint32 TTL, Actor *a, std::list<Actor *> supported);
		void Update(uint32 dt);
		void UndoMode();
};

class DefensiveMode : public ActorMode {
	public:
		DefensiveMode(Actor *a);
		void Update(uint32 dt);
		void UndoMode();
};

class IdleMode : public ActorMode {
	public:
		IdleMode(Actor *a);
		void Update(uint32 dt);
		void UndoMode();
};

class ActionMode : public ActorMode {
	private:
		Action *_action;
	public:
		ActionMode(Actor *a, Action *act);
		~ActionMode();
		Action *GetAction();
		void Update(uint32 dt);
		void UndoMode();
};

class CoolDownMode : public ActorMode {
	private: 
		//!How long the mode should last
		uint32 _TTL;
		void SubtractTTL(uint32 dt);
	public:
		CoolDownMode(uint32 TTL, Actor *a);
		void Update(uint32 dt);
		void UndoMode();
};

class WarmUpMode : public ActorMode {
	private:
		//!How long the mode should last
		uint32 _TTL;
		//!The action to perform after warming up
		Action *_action;
		void SubtractTTL(uint32 dt);
	public:
		WarmUpMode(uint32 TTL, Action *_act, Actor *a);
		void Update(uint32 dt);
		void UndoMode();
};


class VisualEffect {
	private:
		//!The animation that should go with the effect
		hoa_video::AnimatedImage _image;
		//!The animation mode the character should switch into
		std::string _animationMode;
	public:
		VisualEffect(std::string am, hoa_video::AnimatedImage i);
		VisualEffect(const VisualEffect& ve);
		VisualEffect& operator= (const VisualEffect& ve);
		
		void Draw();
		std::string GetAnimationMode();
};

class ActorEffect {
	private:
		//!Who we are effecting
		Actor *_host;
		//!The name of the effect
		std::string _EffectName;
		//!The length the effect will last
		uint32 _TTL;
		//!The chance the character has at healing themselves
		uint32 _chanceToCure;
		//!How often the effect does something
		//!    -1 for update once
		uint32 _updateLength;
		//!How old the effect is
		uint32 _age;
		//!When the last update was
		uint32 _lastUpdate;
		//!The visual effect associated with this effect
		VisualEffect *_visualEffect;
		//!The sound effect associated with this effect
		SoundDescriptor _soundEffect;
		
		void SubtractTTL(uint32 dt);
		
	public:
		ActorEffect(Actor *a, std::string name, uint32 ttl, uint32 ctc,
					uint32 ul, VisualEffect *ve, SoundDescriptor se);
		virtual ~ActorEffect();
		
		uint32 GetTTL();
		void Update(uint32 dt);
		
		Actor *GetHost();
		std::string GetEffectName();
		uint32 GetChanceToCure();
		uint32 GetUpdateLength();
		uint32 GetLastUpdate();
		void SetLastUpdate(uint32 lu);
		VisualEffect *GetVisualEffect();
		SoundDescriptor GetSoundEffect();
		
		virtual void DoEffect() = 0;
		virtual void UndoEffect() = 0;
};

class Action {
	private:
		//!The Host
		Actor *_host;
		//!The visual effect associated with the action
		VisualEffect *_visualEffect;
		//!The sound effect associated with the action
		SoundDescriptor _soundEffect;
		//!A list of argument actors for the action
		std::list<Actor *> _arguments;

	public:
		Action(VisualEffect *ve, SoundDescriptor se, Actor *p, std::list<Actor *> args);
		virtual ~Action();
		
		Actor *GetHost();
		VisualEffect *GetVisualEffect();
		SoundDescriptor GetSoundEffect();
		std::list<Actor *> GetArguments();
				
		virtual void PerformAction() = 0;
		virtual void FinishAction() = 0;
};

class SkillAction : public Action {
	private:
		//!The skill that is going to be performed.
		GSkill *_skill;
		
		void PerformSkill();
		
	public:
		SkillAction(GSkill *s, VisualEffect *ve, SoundDescriptor se, Actor *p, 
				std::list<Actor *> args);
		void PerformAction();
		void FinishAction();
};

class SwapAction : public Action {
	private:
		
	public:
		SwapAction(VisualEffect *ve, SoundDescriptor se, Actor *p, 
					std::list<Actor *> args);
		
		void PerformAction();
		void FinishAction();
};

class UseItemAction : public Action {
	private:
		//!The item we are going to use
		GItem *_item;
		
	public:
		UseItemAction(GItem *i, VisualEffect *ve, SoundDescriptor se, Actor *p, 
					std::list<Actor *> args);
					
		void PerformAction();
		void FinishAction();
};

class AilmentEffect : public ActorEffect {
	private:
		//!Tells if the player can still move
		bool _canMove;
		//!How much health should be modified
		uint32 _healthModifier;
		//!How much mana should be modified
		uint32 _manaModifier;
		//!How much stamina (skill points) points should be modified
		uint32 _skillPointsModifier;
	
	public:
		AilmentEffect(Actor *a, std::string name, uint32 ttl, uint32 ctc,
					uint32 ul, VisualEffect *ve, SoundDescriptor se,
					bool cm, uint32 hm, uint32 mm, uint32 sm);
		void DoEffect();
		void UndoEffect();
};

class StatusEffect : public ActorEffect {
	private:
		//!How much strength should be modified
		uint32 _strengthModifier;
		//!How much intelligence should be modified
		uint32 _intelligenceModifier;
		//!How much agility should be modified
		uint32 _agilityModifier;
	
	public:
		StatusEffect(Actor *a, std::string name, uint32 ttl, uint32 ctc,
					uint32 ul, VisualEffect ve, SoundDescriptor se,
					uint32 sm, uint32 im, uint32 am);
		void DoEffect();
		void UndoEffect();
};



 /******************************************************************************
	BattleMode Class

	>>>members<<<

	>>>functions<<<

	>>>notes<<<
 *****************************************************************************/
class BattleMode : public hoa_engine::GameMode {
private:
	friend class hoa_data::GameData;

	std::vector<hoa_video::StillImage> _battle_images;
	std::vector<hoa_audio::MusicDescriptor> _battle_music;
	//std::vector<hoa_audio::SoundDescriptor> _battle_sound;

	std::list<PlayerActor *> _playerActors;
	std::list<EnemyActor *> _enemyActors;
	std::list<PlayerActor *> _PCsInBattle;
	
	std::list<Actor *> _actionQueue;
	Actor *_currentlyPerforming;
	
	BattleUI _UserInterface;

	int32 _num_enemies;
	
	bool _performingAction;
	
	//! Drawing methods
	void DrawBackground();
	void DrawCharacters();
	
	//! Shutdown the battle mode
	void ShutDown();
	
	//!Are we performing an action
	bool IsPerformingAction();

public:
	BattleMode();
	~BattleMode();

	//! Resets appropriate class members. Called whenever BootMode is made the active game mode.
	void Reset();
	//! Wrapper function that calls different update functions depending on the battle state.
	void Update(uint32 time_elapsed);
	//! Wrapper function that calls different draw functions depending on the battle state.
	void Draw();
	
	void SetPerformingAction(bool isPerforming);
	
	void AddToActionQueue(Actor *a);
	void RemoveFromActionQueue(Actor *a);
};


} // namespace hoa_battle

#endif
