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
 * \author  Corey Hoffstein, visage@allacrost.org
 * \date    Last Updated: December 15, 2005
 * \brief   Header file for battle mode interface.
 *
 * This code handles the game event processing and frame drawing when the user
 * is fighting a battle. 
 *****************************************************************************/

#ifndef __BATTLE_HEADER__
#define __BATTLE_HEADER__

#include "battle_actions.h"

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

namespace private_battle {

const int32 TILE_SIZE = 64; // The virtual "tile map" that we discussed in the forums has square 64 pixel tiles
const int32 SCREEN_LENGTH = 16; // Number of tiles long the screen is
const int32 SCREEN_HEIGHT = 12; // The number of tiles high the screen is

}

/**
	Actor is the general entity partaking in battle.  It will be inherited by player actors and enemy actors.
*/
class Actor {
	private:
		//! The mode we belong to
		BattleMode *_ownerBattleMode;
		//! The X location of the actor on the battle grid
		uint32 _X_Location;
		//! The Y location of the actor on the battle grid
		uint32 _Y_Location;
		//! A stack of the current modes effecting the character
		ActorMode *_mode;
		//! A list of effects and ailments on the character
		std::list<ActorEffect *> _effects;
		//! The maximum stamina
		uint32 _maxSkillPoints;
		//! The remaining level of stamina
		uint32 _currentSkillPoints;
		//! Tells whether the character can move (frozen, burned, et cetera)
		bool _isMoveCapable;
		//! Tells if the character is alive or dead
		bool _isAlive;
		//! The next action to perform
		Action *_nextAction;
		//! Are we performing the action right now?
		bool _performingAction;
		//! Are we warming up for the action?
		bool _warmingUp;
		//! Are we in defensive mode
		bool _defensiveMode;
		//! Are we being supported?  By whom?
		std::list<Actor *> _supporters;
		/*! The list of battle skill actions currently waiting to be performed
			the first in the list is the current one being performed
		*/
		std::vector<BattleAction *> _minorBattleActions;
		
	public:
		Actor(BattleMode *bm, uint32 x, uint32 y);
		Actor(const Actor&  a);
		
		virtual ~Actor();
		virtual void Update(uint32 dt) = 0;
		virtual void Draw() = 0;
		
		BattleMode *GetOwnerBattleMode();
		
		void SetMode(ActorMode *m);
		ActorMode *GetMode();
		
		/*!
			Manage effects that the player is feeling
		*/
		void UpdateEffects(uint32 dt);
		void PushEffect(ActorEffect *e);
		void RemoveEffect(ActorEffect *e);
		
		/*!
			Set the next action, action related methods
		*/
		void SetNextAction(Action *a);
		void PerformAction();
		bool HasNextAction();
		
		/*!
			Add a minor action (animation, GSkill related things, et cetera)
		*/
		void AddBattleAction(BattleAction *act);
		
		/*!
			Is the player frozen, asleep, et cetera?
		*/
		bool IsMoveCapable();
		void SetMoveCapable(bool capable);
		
		/*!
			If the player is warming up, it really can't do anything
			Sort of a special case
		*/
		bool IsWarmingUp();
		void SetWarmingUp(bool warmup);
		
		/*!
			Supporters are people who...well, wait, we haven't decided yet...
		*/
		void AddSupporter(Actor *a);
		void RemoveSupporter(Actor *a);
		
		/*!
			Defensive mode boosts defense
		*/
		void SetDefensiveMode(bool d);
		bool IsInDefensiveMode();
		
		/*!
			If we are currently performing an action we can't do anything else
			on the update
		*/
		bool IsPerformingAction();
		void SetPerformingAction(bool performing);
		
		/*!
			Do we have minor actions to perform?
		*/
		bool HasMinorActions();
		void UpdateMinorActions(uint32 dt);
		
		/*!
			GCharacter and GEnemy will use a Map of sorts
			to map strings to image animations
			This sets our characters animation
		*/
		void SetAnimation(std::string animation);
		
		/*!
			Specific getters for classes that inherit
		*/
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

/*!
	The user interface for the battle mode
*/
class BattleUI {
	private:
		//! The battlemode we belong to
		BattleMode *_bm;
		//! The current actor we have clicked on
		Actor *_currentlySelectedActor;
		//! The actors we have selected as arguments
		std::list<Actor *> _currentlySelectedArgumentActors;
		//! A stack of menu selections we have gone through
		std::list<int> _currentlySelectedMenuItem;
		//! The number of selections that must be made for an action
		uint32 _necessarySelections;
		//! The menu item we are hovering over
		uint32 _currentHoverSelection;
		//! The number of items in this menu
		uint32 _numberMenuItems;
		
		//! Swapping
		uint32 _numSwapCards;
		uint32 _maxSwapCards;
		uint32 _lastTimeSwapAwarded;
		
	public:
		BattleUI(BattleMode *bm);
		
		/*!
			Get the actor we are currently on
		*/
		Actor* GetSelectedActor();
		
		/*!
			We clicked on an actor
		*/
		void SetActorSelected(Actor *a);
		
		/*!
			No actor is selected...we are now selecting an actor
		*/
		void DeselectActor();
		
		/*!
			Get other people selected
		*/
		std::list<Actor *> GetSelectedArgumentActors();
		
		/*!
			The actor we just selected is now an argument
		*/
		void setActorAsArgument(Actor *a);
		
		/*!
			No longer do we want this actor as an argument
		*/
		void RemoveActorAsArgument(Actor *a);
		
		/*!
			Sets the number of arguments we should be allowing
		*/
		void SetNumberNecessarySelections(uint32 select);
};

class PlayerActor : public Actor {
	private:
		//! The global character we have wrapped around
		GCharacter *_wrappedCharacter;
	
	public:
		PlayerActor(GCharacter *_wrapped, BattleMode *bm, int x, int y);
		~PlayerActor();
		void Update(uint32 dt);
		void Draw();
		
		/*!
			Get the skills from GCharacter
		*/
		std::vector<GSkill *> GetAttackSkills();
		std::vector<GSkill *> GetDefenseSkills();
		std::vector<GSkill *> GetSupportSkills();
		
		/*!
			More getters from GCharacter
		*/
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
		//! The enemy we have wrapped around
		GEnemy *_wrappedEnemy;
		
	public:
		EnemyActor(GEnemy *ge, BattleMode *bm, int x, int y);
		~EnemyActor();
		void Update(uint32 dt);
		void Draw();

		/*!
			Has the GEnemy level up to average_level
		*/
		void LevelUp(uint32 average_level);
		
		/*!
			The AI routine
		*/
		void DoAI(uint32 dt);

		/*!
			GEnemy getters
		*/
		std::vector<GSkill *> GetSkills();
		
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

/*!
	ActorMode puts actor derived classes into a Mode, which sounds sort of vague.
	But please, let me continue.
	Modes are often used to tell the character how to animate, what to do next, et cetera.
	In defensive mode, it puts the character into defense mode, in warmup mode, it waits
	until the warmup time is finished before performing an action, et cetera.
	Each mode is fairly specific.
*/
class ActorMode {
	private:
		//! Who we are effecting
		Actor *_host;
	
	public:
		ActorMode(Actor *a);
		virtual ~ActorMode();
		Actor *GetHost();
		
		virtual void Update(uint32 dt) = 0;
		virtual void UndoMode() = 0;
};

/*!
	Support mode puts an actor in a mode where they will wait to perform some 
	sort of "support" on another character.
*/
class SupportMode : public ActorMode {
	private: 
		std::vector<Actor *> _supported;
		
	public:
		SupportMode(uint32 TTL, Actor *a, std::vector<Actor *> supported);
		void Update(uint32 dt);
		void UndoMode();
};

/*!
	Defensive Mode puts an actor into a defensive stance.
*/
class DefensiveMode : public ActorMode {
	public:
		DefensiveMode(Actor *a);
		void Update(uint32 dt);
		void UndoMode();
};

/*!
	The default mode for an actor, where they are simply standing
*/
class IdleMode : public ActorMode {
	public:
		IdleMode(Actor *a);
		void Update(uint32 dt);
		void UndoMode();
};

/*!
	The actor is either waiting to, or currently performing an action
*/
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

/*!
	The actor has performed an action which requires a cooldown period,
	so the actor goes into cooldown mode.
*/
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

/*!
	The actor is attempting to perform an action which requires warming up.
	Thus, we go into warmup mode, which will eventually turn into ActionMode.
*/
class WarmUpMode : public ActorMode {
	private:
		//! How long the mode should last
		uint32 _TTL;
		//! The action to perform after warming up
		Action *_action;
		void SubtractTTL(uint32 dt);
	public:
		WarmUpMode(uint32 TTL, Action *_act, Actor *a);
		void Update(uint32 dt);
		void UndoMode();
};

/*!
	For now, this class is vague at best.  Don't even worry about it.
*/
class VisualEffect {
	private:
		//! The animation that should go with the effect
		hoa_video::AnimatedImage _image;
		//! The animation mode the character should switch into
		std::string _animationMode;
	public:
		VisualEffect(std::string am, hoa_video::AnimatedImage i);
		VisualEffect(const VisualEffect& ve);
		VisualEffect& operator= (const VisualEffect& ve);
		
		void Draw();
		std::string GetAnimationMode();
};

/*!
	Action is fairly self explainatory.  They are verbs an actor can take:
	use item, use a skill, swap with another actor on their team, et cetera.
*/
class Action {
	private:
		//! The Host
		Actor *_host;
		//! A list of argument actors for the action
		std::vector<Actor *> _arguments;

	public:
		Action(Actor *p, std::vector<Actor *> args);
		virtual ~Action();
		
		Actor *GetHost();
		std::vector<Actor *> GetArguments();
				
		virtual void PerformAction() = 0;
		virtual void FinishAction() = 0;
};

/*!
	This action performs a skill
*/
class SkillAction : public Action {
	private:
		//! The skill that is going to be performed.
		GSkill *_skill;
		
		void PerformSkill();
		void PerformCooldown();
		
	public:
		SkillAction(GSkill *s, Actor *p, std::vector<Actor *> args);
		~SkillAction();
		void PerformAction();
		void FinishAction();
		void Update(uint32 dt);
};

/*!
	This action defines a swap with another actor
*/
class SwapAction : public Action {
	private:
		
	public:
		SwapAction(Actor *p, std::vector<Actor *> args);
		~SwapAction();
		void PerformAction();
		void FinishAction();
		void Update(uint32 dt);
};

/*!
	This action uses an item in inventory
*/
class UseItemAction : public Action {
	private:
		//! The item we are going to use
		GItem *_item;
		
	public:
		UseItemAction(GItem *i, Actor *p, std::vector<Actor *> args);
		~UseItemAction();
		void PerformAction();
		void FinishAction();
		void Update(uint32 dt);
};

/*!
	Actor Effects affect the stats of an Actor, be it burn, sleep, frozen, 
	poison, et cetera.
*/
class ActorEffect {
	private:
		//! Who we are effecting
		Actor *_host;
		//! The name of the effect
		std::string _EffectName;
		//! The length the effect will last
		uint32 _TTL;
		//! The chance the character has at healing themselves
		uint32 _chanceToCure;
		/*! How often the effect does something
		     -1 for update once
		*/
		uint32 _updateLength;
		//! How old the effect is
		uint32 _age;
		//! When the last update was
		uint32 _lastUpdate;
		//! The visual effect associated with this effect
		VisualEffect *_visualEffect;
		//! The sound effect associated with this effect
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

/*!
	This effect changes health, mana, or skill points.
*/
class AilmentEffect : public ActorEffect {
	private:
		//! Tells if the player can still move
		bool _canMove;
		//! How much health should be modified
		uint32 _healthModifier;
		//! How much mana should be modified
		uint32 _manaModifier;
		//! How much stamina (skill points) points should be modified
		uint32 _skillPointsModifier;
	
	public:
		AilmentEffect(Actor *a, std::string name, uint32 ttl, uint32 ctc,
					uint32 ul, VisualEffect *ve, SoundDescriptor se,
					bool cm, uint32 hm, uint32 mm, uint32 sm);
		void DoEffect();
		void UndoEffect();
};

/*!
	Status Effects change the stats of a character temporarily
*/
class StatusEffect : public ActorEffect {
	private:
		//! How much strength should be modified
		uint32 _strengthModifier;
		//! How much intelligence should be modified
		uint32 _intelligenceModifier;
		//! How much agility should be modified
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

	The big kahuna
 *****************************************************************************/
class BattleMode : public hoa_engine::GameMode {
private:
	friend class hoa_data::GameData;

	//! minor battle actions that should take place at the same time as other actions
	std::vector<BattleAction *> _concurrentActions;
	
	std::vector<hoa_video::StillImage> _battle_images;
	std::vector<hoa_audio::MusicDescriptor> _battle_music;
	//std::vector<hoa_audio::SoundDescriptor> _battle_sound;

	//! Current list of actors 
	std::list<PlayerActor *> _playerActors;
	
	//actors actually in battle
	std::list<EnemyActor *> _enemyActors;
	std::list<PlayerActor *> _PCsInBattle;
	
	//! a queue of actors trying to perform actions
	std::list<Actor *> _actionQueue;
	//! the actor currently performing an action
	Actor *_currentlyPerforming;
	
	//! the user interface belonging to this battle mode
	BattleUI _UserInterface;

	//! the number of enemies..._enemyActors.size()
	int32 _num_enemies;
	
	//! Is an action being performed?
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
	
	//! Sets T/F whether an action is being performed
	void SetPerformingAction(bool isPerforming);
	
	//! Adds a concurrent battle action
	void AddConcurrentBattleAction(BattleAction *act);
	
	//! Adds an actor waiting to perform an action to the queue.
	void AddToActionQueue(Actor *a);
	
	//! Removes an actor from the action queue (perhaps they died, et cetera)
	void RemoveFromActionQueue(Actor *a);

	//! Returns all player actors
	std::list<PlayerActor *> ReturnCharacters();
};


} // namespace hoa_battle

#endif

