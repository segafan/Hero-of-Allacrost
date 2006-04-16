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
 * \date    Last Updated: February 20, 2006
 * \brief   Header file for battle mode interface.
 *
 * This code handles the game event processing and frame drawing when the user
 * is fighting a battle. 
 *****************************************************************************/

#ifndef __BATTLE_HEADER__
#define __BATTLE_HEADER__

#include <string>
#include <vector>
#include <deque>

#include "utils.h"
#include "defs.h"
#include "engine.h"

#include "global.h"
#include "video.h"
#include "audio.h"

using namespace hoa_global;
using namespace hoa_video;
using namespace hoa_audio;
using namespace hoa_utils;

using namespace hoa_battle::private_battle;

namespace hoa_battle {

extern bool BATTLE_DEBUG;

struct BattleStatTypes {
        int32 VOLT; //strong against water, weak against earth
        int32 EARTH; //strong against volt, weak against fire
        int32 WATER; //strong against fire, weak against volt
        int32 FIRE; //strong against earth, weak against water
        
        int32 PIERCING;
        int32 SLASHING;
        int32 BLUDGEONING;
};

enum StatusSeverity {
        LESSER = 0,
        NORMAL,
        GREATER,
        ULTIMATE
};


namespace private_battle {

const uint32 TILE_SIZE = 64; // The virtual "tile map" that we discussed in the forums has square 64 pixel tiles
const uint32 SCREEN_LENGTH = 16; // Number of tiles long the screen is
const uint32 SCREEN_HEIGHT = 12; // The number of tiles high the screen is

const uint32 DRAW_OFFSET_LEFT = 224; //from the left of the screen = 0
const uint32 DRAW_OFFSET_TOP = 640; //from bottom of the screen = 0 (128 from the top of the screen)
const uint32 DRAW_OFFSET_BOTTOM = 64; //from the bottom of the screen = 0

const uint32 NUM_WIDTH_TILES = 12;
const uint32 NUM_HEIGHT_TILES = 9;

/*!
	Actor Effects affect the stats of an Actor, be it burn, sleep, frozen, 
	poison, et cetera.
        Goes before the private namespace because Actor needs to know about it
*/
class ActorEffect {
	private:
		//! Who we are effecting
		Actor *_host;
		//! The name of the effect
		std::string _EffectName;
		//! The length the effect will last
		uint32 _TTL;
                
                StatusSeverity _severeness;
                
		/*! How often the effect does something
		     -1 for update once
		*/
                bool _canMove;
                uint32 _healthModifier;
                uint32 _skillPointModifier;
                
                uint32 _strengthModifier;
                uint32 _intelligenceModifier;
                uint32 _agilityModifier;
                
                //how often to update the effect
		uint32 _updateLength;
		//! How old the effect is
		uint32 _age;
		//! When the last update was
		uint32 _lastUpdate;
                //! How many times this effect updated on the player
                uint32 _timesUpdated;
		
		void SubtractTTL(uint32 dt);
		
	public:
		ActorEffect(Actor * const AHost, std::string AEffectName, StatusSeverity AHowSevere,
                                uint32 ATTL, bool ACanMove, uint32 AHealthModifier, 
                                uint32 ASkillPointModifier, uint32 AStrengthModifier, 
                                uint32 AIntelligenceModifier, uint32 AAgilityModifier, 
                                uint32 AUpdateLength);
		virtual ~ActorEffect();
		
		uint32 GetTTL() const;
		void Update(uint32 ATimeElapsed);
		
		Actor * const GetHost() const;
		std::string GetEffectName() const;
		uint32 GetUpdateLength() const;
		uint32 GetLastUpdate() const;
                
                bool CanMove() const;
                uint32 GetHealthModifier() const;
                uint32 GetSkillPointModifier() const;
                uint32 GetStrengthModifier() const;
                uint32 GetIntelligenceModifier() const;
                uint32 GetAgilityModifier() const;
                
		void SetLastUpdate(uint32 ALastUpdate);
		
		void UndoEffect() const;
};

/**
	Actor is the general entity partaking in battle.  It will be inherited by player actors and enemy actors.
*/
class Actor {
	private:
		//! The mode we belong to
		BattleMode *_ownerBattleMode;
		//! The original X location of the actor
		uint32 _X_Origin;
		//! The original Y location of the actor
		uint32 _Y_Origin;
		//! The X location of the actor on the battle grid
		uint32 _X_Location;
		//! The Y location of the actor on the battle grid
		uint32 _Y_Location;
		//! A list of effects and ailments on the character
		std::deque<ActorEffect> _effects;
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
                uint32 _warmupTime;
                //! Are we cooling down from an action?
                uint32 _cooldownTime;
                //! Do we have a defensive mode bonus?  how much?
                uint32 _defensiveModeBonus;
                
                //! The sum of all modifiers from effects
                uint32 _totalStrengthModifier;
                uint32 _totalAgilityModifier;
                uint32 _totalIntelligenceModifier;
		
                //! Our current animation to update and draw
		std::string _animation;
		
	public:
		Actor(BattleMode *ABattleMode, uint32 AXLocation, uint32 AYLocation);
		Actor(const Actor& AOtherActor);
		
		virtual ~Actor();
		virtual void Update(uint32 ATimeElapsed) = 0;
		virtual void Draw() = 0;
		
		/*!
			Stuff relating to, you know, death
		*/
		void Die();
		bool IsAlive() const;
		
                /*!
                        Get the mode we are currently fighting in
                */
		const BattleMode *GetOwnerBattleMode() const;
		
		/*!
			Manage effects that the player is feeling
		*/
		void UpdateEffects(uint32 ATimeElapsed);
		void PushEffect(const ActorEffect AEffect);
		
		/*!
			Set the next action, action related methods
		*/
		void SetNextAction(Action * const ANextAction);
		void PerformAction();
		bool HasNextAction() const;
		
		/*!
			Is the player frozen, asleep, et cetera?
		*/
		bool IsMoveCapable() const;
		void SetMoveCapable(bool AMoveCapable);
		
		/*!
			If the player is warming up, it really can't do anything
			Sort of a special case
		*/
		bool IsWarmingUp() const;
		void SetWarmupTime(uint32 AWarmupTime);
		
		/*!
			Defensive mode boosts defense
		*/
                bool IsInDefensiveMode() const;		
		void SetDefensiveBonus(uint32 ADefensiveBonus);

		/*!
			If we are currently performing an action we can't do anything else
			on the update
		*/
		bool IsPerformingAction() const;
		void SetPerformingAction(bool AIsPerforming);
		
		/*!
			GlobalCharacter and GlobalEnemy will use a Map of sorts
			to map strings to image animations
			This sets our characters animation
		*/
		void SetAnimation(std::string ACurrentAnimation);
		const std::string GetAnimation() const;
		
		/*!
			Specific getters for classes that inherit
		*/
		virtual std::string GetName() = 0;
		virtual std::vector<GlobalAttackPoint> GetAttackPoints() = 0;
		virtual uint32 GetHealth() = 0;
		virtual void SetHealth(uint32 hp) = 0;
		virtual uint32 GetMaxHealth() = 0;
		virtual uint32 GetSkillPoints() = 0;
		virtual void SetSkillPoints(uint32 sp) = 0;
		virtual uint32 GetMaxSkillPoints() = 0;
		virtual uint32 GetStrength() = 0;
		virtual uint32 GetIntelligence() = 0;
		virtual uint32 GetAgility() = 0;
		
		/*!
			More getters and setters
		*/
                const uint32 GetXOrigin() const { return _X_Origin; }
                const uint32 GetYOrigin() const { return _Y_Origin; }
		void SetXOrigin(int x) { _X_Origin = x; }
		void SetYOrigin(int y) { _Y_Origin = y; }
		const uint32 GetXLocation() const { return _X_Location; }
		const uint32 GetYLocation() const { return _Y_Location; }
		void SetXLocation(int x) { _X_Location = x; }
		void SetYLocation(int y) { _Y_Location = y; }
		
		/*!
			Get the movement speed in battle for this character
		*/
		virtual uint32 GetMovementSpeed() = 0;
                
                /*!
                        Getters and setters involved with totals
                */
                void SetTotalStrengthModifier(uint32 AStrengthModifier);
                uint32 GetTotalStrengthModifier();
                void SetTotalAgilityModifier(uint32 AAgilityModifier);
                uint32 GetTotalAgilityModifier();
                void SetTotalIntelligenceModifier(uint32 AIntelligenceModifier);
                uint32 GetTotalIntelligenceModifier();
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
		std::deque<uint32> _currentlySelectedMenuItem;
		//! The number of selections that must be made for an action
		uint32 _necessarySelections;
		//! The menu item we are hovering over
		uint32 _currentHoverSelection;
		//! The number of items in this menu
		uint32 _numberMenuItems;
		
	public:
		BattleUI(BattleMode * const ABattleMode);
		
		/*!
			Get the actor we are currently on
		*/
		Actor * const GetSelectedActor() const;
		
		/*!
			We clicked on an actor
		*/
		void SetActorSelected(Actor * const AWhichActor);
		
		/*!
			No actor is selected...we are now selecting an actor
		*/
		void DeselectActor();
		
		/*!
			Get other people selected
		*/
		std::list<Actor *> GetSelectedArgumentActors() const;
		
		/*!
			The actor we just selected is now an argument
		*/
		void SetActorAsArgument(Actor * const AActor);
		
		/*!
			No longer do we want this actor as an argument
		*/
		void RemoveActorAsArgument(Actor * const AActor);
		
		/*!
			Sets the number of arguments we should be allowing
		*/
		void SetNumberNecessarySelections(uint32 ANumSelections);

};

class PlayerActor : public Actor {
	private:
		//! The global character we have wrapped around
		GlobalCharacter *_wrappedCharacter;
	
	public:
		PlayerActor(GlobalCharacter * const AWrapped, BattleMode * const ABattleMode, uint32 AXLoc, uint32 AYLoc);
		~PlayerActor();
                void Update(uint32 ATimeElapsed);
                void Draw();
                
		/*!
			Get the skills from GlobalCharacter
		*/
		std::vector<GlobalSkill *> GetAttackSkills() const;
		std::vector<GlobalSkill *> GetDefenseSkills() const;
		std::vector<GlobalSkill *> GetSupportSkills() const;
		
		/*!
			More getters from GlobalCharacter
		*/
		const std::string GetName() const;
		const std::vector<GlobalAttackPoint> GetAttackPoints() const;
		uint32 GetHealth() const;
		void SetHealth(uint32 AHealth);
		uint32 GetMaxHealth() const;
		uint32 GetSkillPoints() const;
		void SetSkillPoints(uint32 ASkillPoints);
		uint32 GetMaxSkillPoints() const;
		uint32 GetStrength() const;
		uint32 GetIntelligence() const;
		uint32 GetAgility() const;
		
		uint32 GetMovementSpeed() const;
                

};

class EnemyActor : public Actor {
	private:
		//! The enemy we have wrapped around
		GlobalEnemy _wrappedEnemy;
		
	public:
		EnemyActor(GlobalEnemy AGlobalEnemy, BattleMode * const ABattleMode, uint32 AXLoc, uint32 AYLoc);
		~EnemyActor();
                void Update(uint32 ATimeElapsed);
                void Draw();

		/*!
			Has the GlobalEnemy level up to average_level
		*/
		void LevelUp(uint32 AAverageLevel);
		
		/*!
			The AI routine
		*/
		void DoAI();

		/*!
			GlobalEnemy getters
		*/
		const std::vector<GlobalSkill *> GetSkills() const;
		
		std::string GetName() const;
		const std::vector<GlobalAttackPoint> GetAttackPoints() const;
		uint32 GetHealth() const;
		void SetHealth(uint32 AHealth);
		uint32 GetMaxHealth() const;
		uint32 GetSkillPoints() const;
		void SetSkillPoints(uint32 ASkillPoints);
		uint32 GetMaxSkillPoints() const;
		uint32 GetStrength() const;
		uint32 GetIntelligence() const;
		uint32 GetAgility() const;
		
		uint32 GetMovementSpeed() const;
                
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
                
                std::string _skillName;

	public:
		Action(Actor * const AHostActor, std::vector<Actor *> AArguments, const std::string ASkillName);
		virtual ~Action();
		
		Actor * const GetHost() const;
		const std::vector<Actor *> GetArguments() const;
				
		const std::string GetSkillName() const;
};

class ScriptEvent {
        private:
                std::string _scriptName;
                Actor *_host;
                std::list<Actor *> _arguments;
        
        public:
                ScriptEvent(Actor *AHost, std::list<Actor *> AArguments, std::string AScriptName);
                ~ScriptEvent();
                void RunScript();
                
                Actor *GetHost();
};

} //end private_battle


 /******************************************************************************
	BattleMode Class

	The big kahuna
 *****************************************************************************/
class BattleMode : public hoa_engine::GameMode {
private:
	friend class hoa_data::GameData;

	std::vector<hoa_video::StillImage> _battle_images;
	std::vector<hoa_audio::MusicDescriptor> _battle_music;
	//std::vector<hoa_audio::SoundDescriptor> _battle_sound;

	//! Current list of actors 
	std::deque<PlayerActor *> _playerActors;
	
	//actors actually in battle
	std::deque<EnemyActor *> _enemyActors;
	std::deque<PlayerActor *> _PCsInBattle;
	
	//! a queue of actors trying to perform actions
	std::list<Actor *> _actionQueue;
	
        //! a queue of scripted events to perform
        std::list<ScriptEvent> _scriptQueue;
        
	//! the user interface belonging to this battle mode
	BattleUI _UserInterface;
	
	//! Is an action being performed?
	bool _performingScript;
        		
        //! Swapping information
        uint32 _numSwapCards;
        uint32 _maxSwapCards;
        uint32 _lastTimeSwapAwarded;
	
	//! Drawing methods
	void DrawBackground();
	void DrawCharacters();
	
	//! Shutdown the battle mode
	void ShutDown();
	
	//!Are we performing an action
	bool IsPerformingScript();

public:
	BattleMode();
	~BattleMode();

	//! Resets appropriate class members. Called whenever BootMode is made the active game mode.
	void Reset();
	//! Wrapper function that calls different update functions depending on the battle state.
	void Update();
	//! Wrapper function that calls different draw functions depending on the battle state.
	void Draw();
	
	//! Sets T/F whether an action is being performed
	void SetPerformingScript(bool AIsPerforming);
	
	//! Adds an actor waiting to perform an action to the queue.
	void AddToActionQueue(Actor *AActorToAdd);
	
	//! Removes an actor from the action queue (perhaps they died, et cetera)
	void RemoveFromActionQueue(Actor *AActorToRemove);
        
        //! Added a scripted event to the queue
        void AddScriptEventToQueue(ScriptEvent AEventToAdd);
        
        //! Remove all scripted events for an actor
        void RemoveScriptedEventsForActor(Actor *AActorToRemove);

	//! Returns all player actors
	std::deque<PlayerActor *> ReturnCharacters() const;
};


} // namespace hoa_battle

#endif
