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
#include "mode_manager.h"

#include "global.h"
#include "video.h"
#include "audio.h"

namespace hoa_battle {

extern bool BATTLE_DEBUG;

class BattleStatTypes {
        public:
                int32 volt; //strong against water, weak against earth
                int32 earth; //strong against volt, weak against fire
                int32 water; //strong against fire, weak against volt
                int32 fire; //strong against earth, weak against water
                
                int32 piercing;
                int32 slashing;
                int32 bludgeoning;
};

enum StatusSeverity {
        LESSER = 0,
        NORMAL,
        GREATER,
        ULTIMATE
};

/***

   Begin private namespace

***/
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
		private_battle::Actor *_host;
		//! The name of the effect
		std::string _effect_name;
		//! The length the effect will last
		uint32 _TTL;
                
                StatusSeverity _severeness;
                
		/*! How often the effect does something
		     -1 for update once
		*/
                bool _can_move;
                uint32 _health_modifier;
                uint32 _skill_point_modifier;
                
                uint32 _strength_modifier;
                uint32 _intelligence_modifier;
                uint32 _agility_modifier;
                
                //how often to update the effect
		uint32 _update_length;
		//! How old the effect is
		uint32 _age;
		//! When the last update was
		uint32 _last_update;
                //! How many times this effect updated on the player
                uint32 _times_updated;
		
		void _SubtractTTL(uint32 dt);
		
	public:
		ActorEffect(private_battle::Actor * const AHost, std::string AEffectName, StatusSeverity AHowSevere,
                                uint32 ATTL, bool ACanMove, uint32 AHealthModifier, 
                                uint32 ASkillPointModifier, uint32 AStrengthModifier, 
                                uint32 AIntelligenceModifier, uint32 AAgilityModifier, 
                                uint32 AUpdateLength);
		virtual ~ActorEffect();
		
                //! Update the effect
		void Update(uint32 ATimeElapsed);
                
		/*!
                       Some standard Getters
                */
                //@{
                uint32 GetTTL() const;
		private_battle::Actor * const GetHost() const;
		std::string GetEffectName() const;
		uint32 GetUpdateLength() const;
		uint32 GetLastUpdate() const;
                
                bool CanMove() const;
                uint32 GetHealthModifier() const;
                uint32 GetSkillPointModifier() const;
                uint32 GetStrengthModifier() const;
                uint32 GetIntelligenceModifier() const;
                uint32 GetAgilityModifier() const;
                //@}
                
		void SetLastUpdate(uint32 ALastUpdate);
		
                //! Undo the effect on the host
		void UndoEffect() const;
};

/**
	Actor is the general entity partaking in battle.  It will be inherited by player actors and enemy actors.
*/
class Actor {
	private:
		//! The mode we belong to
		BattleMode *_owner_battle_mode;
		//! The original X location of the actor
		uint32 _x_origin;
		//! The original Y location of the actor
		uint32 _y_origin;
		//! The X location of the actor on the battle grid
		double _x_location;
		//! The Y location of the actor on the battle grid
		double _y_location;
		//! A list of effects and ailments on the character
		std::deque<ActorEffect> _effects;
		//! The maximum stamina
		uint32 _max_skill_points;
		//! The remaining level of stamina
		uint32 _current_skill_points;
		//! Tells whether the character can move (frozen, burned, et cetera)
		bool _is_move_capable;
		//! Tells if the character is alive or dead
		bool _is_alive;
                //! Is the character attacking or queued to?
                bool _is_queued_to_perform;
                //! Are we warming up for the action?
                uint32 _warmup_time;
                //! Are we cooling down from an action?
                uint32 _cooldown_time;
                //! Do we have a defensive mode bonus?  how much?
                uint32 _defensive_mode_bonus;
                
                //! The sum of all modifiers from effects
                uint32 _total_strength_modifier;
                uint32 _total_agility_modifier;
                uint32 _total_intelligence_modifier;
                
        protected:
                uint32 _TEMP_total_time_damaged;
                uint32 _TEMP_damage_dealt;
		
	public:
		Actor(BattleMode *ABattleMode, uint32 AXLocation, uint32 AYLocation);
		
		virtual ~Actor();
		virtual void Update(uint32 ATimeElapsed) = 0;
		virtual void Draw() = 0;
		
		/*!
			\brief Stuff relating to, you know, death
		*/
		void Die();
		bool IsAlive() const;
		
                /*!
                        \brief Get the mode we are currently fighting in
                */
		BattleMode *GetOwnerBattleMode() const;
		
		/*!
			\brief Manage effects that the player is feeling
		*/
                //@{
		void UpdateEffects(uint32 ATimeElapsed);
		void PushEffect(const ActorEffect AEffect);
                //@}
		
		/*!
			\brief Is the player frozen, asleep, et cetera?
		*/
                //@{
		bool IsMoveCapable() const;
		void SetMoveCapable(bool AMoveCapable);
                //@}
                
                /*!
                        \brief Is the character already performing an action?
                */
                //@{
                bool IsQueuedToPerform() const;
                void SetQueuedToPerform(bool AQueuedToPerform);
                //@}
		
		/*!
			\brief If the player is warming up, it really can't do anything. Sort of a special case
		*/
                //@{
		bool IsWarmingUp() const;
		void SetWarmupTime(uint32 AWarmupTime);
                //@}
		
		/*!
			\brief Defensive mode boosts defense
		*/
                //@{
                bool IsInDefensiveMode() const;		
		void SetDefensiveBonus(uint32 ADefensiveBonus);
                //@}
                
                virtual void SetAnimation(std::string AAnimation);
		
		/*!
			\brief Specific getters for classes that inherit
		*/
                //@{
		virtual const std::string GetName() const = 0;
		virtual const std::vector<hoa_global::GlobalAttackPoint*> GetAttackPoints() const = 0;
		virtual uint32 GetHealth() const = 0;
		virtual void SetHealth(uint32 hp) = 0;
		virtual uint32 GetMaxHealth() const = 0;
		virtual uint32 GetSkillPoints() const = 0;
		virtual void SetSkillPoints(uint32 sp) = 0;
		virtual uint32 GetMaxSkillPoints() const = 0;
		virtual uint32 GetStrength() const = 0;
		virtual uint32 GetIntelligence() const = 0;
		virtual uint32 GetAgility() const = 0;
                //@}
		
		/*!
			\brief More getters and setters
		*/
                //@{
                const uint32 GetXOrigin() const { return _x_origin; }
                const uint32 GetYOrigin() const { return _y_origin; }
		void SetXOrigin(uint32 x) { _x_origin = x; }
		void SetYOrigin(uint32 y) { _y_origin = y; }
		const double GetXLocation() const { return _x_location; }
		const double GetYLocation() const { return _y_location; }
		void SetXLocation(double x) { _x_location = x; }
		void SetYLocation(double y) { _y_location = y; }
		//@}
                
		/*!
			\brief Get the movement speed in battle for this character
		*/
		virtual uint32 GetMovementSpeed() const = 0;
                
                /*!
                        \brief Getters and setters involved with totals
                */
                //@{
                void SetTotalStrengthModifier(uint32 AStrengthModifier);
                uint32 GetTotalStrengthModifier();
                void SetTotalAgilityModifier(uint32 AAgilityModifier);
                uint32 GetTotalAgilityModifier();
                void SetTotalIntelligenceModifier(uint32 AIntelligenceModifier);
                uint32 GetTotalIntelligenceModifier();
                //@}
                
                void TEMP_Deal_Damage(uint32 damage);
};

/*!
	The user interface for the battle mode
*/
class BattleUI {
	private:
                enum CURSOR_STATE  {
                        CURSOR_ON_PLAYER_CHARACTERS = 0,
                        CURSOR_ON_ENEMY_CHARACTERS = 1,
                        CURSOR_ON_MENU = 2,
                        CURSOR_ON_SUB_MENU = 3,
                        CURSOR_SELECT_TARGET = 4,
                        CURSOR_ON_SELECT_MAP = 5
                };
                
		//! The battlemode we belong to
		BattleMode *_battle_mode;
		//! The current actor we have clicked on
		PlayerActor *_currently_selected_player_actor;
                //! Character index of the currently selected actor
                int32 _actor_index;
                //! Argument selector
                int32 _argument_actor_index;
		//! The actors we have selected as arguments
		std::deque<Actor *> _currently_selected_argument_actors;
		//! A stack of menu selections we have gone through
		std::deque<uint32> _currently_selected_menu_item;
		//! The number of selections that must be made for an action
		uint32 _necessary_selections;
		//! The menu item we are hovering over
		uint32 _current_hover_selection;
                //! The current MAP we are pointing to
                uint32 _current_map_selection;
		//! The number of items in this menu
		uint32 _number_menu_items;
                //! The state of our cursor
                CURSOR_STATE _cursor_state;
                //! The general option box
                hoa_video::OptionBox _general_menu;
                //! The cursor location in the _general_menu.  For pure hackery reasons only
                uint32 _general_menu_cursor_location;
                //! The sub menu.  Recreated every time it is chosen
                hoa_video::OptionBox *_sub_menu;
                hoa_video::MenuWindow *_sub_menu_window;
                //! The "loser" - menu
                hoa_video::OptionBox _battle_lose_menu;
                //! The selected cursor
                hoa_video::StillImage _player_selector_image;
		
	public:
		BattleUI(BattleMode * const ABattleMode);
		~BattleUI();
		/*!
			\brief Get the actor we are currently on
		*/
		Actor * const GetSelectedActor() const;
		
		/*!
			\brief We clicked on an actor
		*/
		void SetPlayerActorSelected(PlayerActor * const AWhichActor);
		
		/*!
			\brief No actor is selected...we are now selecting an actor
		*/
		void DeselectPlayerActor();
		
		/*!
			\brief Get other people selected
		*/
		std::deque<Actor *> GetSelectedArgumentActors() const;
		
		/*!
			\brief The actor we just selected is now an argument
		*/
		void SetActorAsArgument(Actor * const AActor);
		
		/*!
			\brief Sets the number of arguments we should be allowing
		*/
		void SetNumberNecessarySelections(uint32 ANumSelections);

                /*!
                        \brief The player lost.  Show them the menu
                */
                void ShowPlayerDefeatMenu();
                
                /*!
                        \brief The player won. Show them their loot
                */
                void ShowPlayerVictoryMenu();
                
                /*!
                        \brief Draw the GUI images
                */
                void Draw();
                void DrawTopElements();
                
                /*!
                        \brief Update player info
                */
                void Update(uint32 AUpdateTime);
};

class PlayerActor : public Actor {
	private:
		//! The global character we have wrapped around
		hoa_global::GlobalCharacter *_wrapped_character;
                //! The current animation to draw
                hoa_video::AnimatedImage _current_animation;
	
	public:
		PlayerActor(hoa_global::GlobalCharacter * const AWrapped, BattleMode * const ABattleMode, uint32 AXLoc, uint32 AYLoc);
		~PlayerActor();
                void Update(uint32 ATimeElapsed);
                void Draw();
                
		/*!
			\brief Get the skills from GlobalCharacter
		*/
                //@{
		std::vector<hoa_global::GlobalSkill *> GetAttackSkills() const;
		std::vector<hoa_global::GlobalSkill *> GetDefenseSkills() const;
		std::vector<hoa_global::GlobalSkill *> GetSupportSkills() const;
		//@}
                
		/*!
			\brief More getters from GlobalCharacter
		*/
                //@{
		const std::string GetName() const;
		const std::vector<hoa_global::GlobalAttackPoint*> GetAttackPoints() const;
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
                
                void SetAnimation(std::string ACurrentAnimation);
                //@}
                

};

class EnemyActor : public Actor {
	private:
		//! The enemy we have wrapped around
		hoa_global::GlobalEnemy _wrapped_enemy;
		
	public:
		EnemyActor(hoa_global::GlobalEnemy AGlobalEnemy, BattleMode * const ABattleMode, uint32 AXLoc, uint32 AYLoc);
		~EnemyActor();
                void Update(uint32 ATimeElapsed);
                void Draw();

		/*!
			\brief Has the GlobalEnemy level up to average_level
		*/
		void LevelUp(uint32 AAverageLevel);
		
		/*!
			\brief The AI routine
		*/
		void DoAI();

		/*!
			\brief GlobalEnemy getters
		*/
                //@{
		const std::vector<hoa_global::GlobalSkill *> GetSkills() const;
		
		const std::string GetName() const;
		const std::vector<hoa_global::GlobalAttackPoint*> GetAttackPoints() const;
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
                //@}
                
};


/*!
	Action is fairly self explainatory.  They are verbs an actor can take:
	use item, use a skill, swap with another actor on their team, et cetera.
*/

class ScriptEvent {
        private:
                std::string _script_name;
                Actor *_host;
                std::deque<Actor *> _arguments;
        
        public:
                ScriptEvent(Actor *AHost, std::deque<Actor *> AArguments, std::string AScriptName);
                ~ScriptEvent();
                void RunScript();
                
                Actor *GetHost();
};

} //end private_battle


 /******************************************************************************
	BattleMode Class

	The big kahuna
 *****************************************************************************/
class BattleMode : public hoa_mode_manager::GameMode {
private:
	friend class hoa_data::GameData;

	std::vector<hoa_video::StillImage> _battle_images;
	std::vector<hoa_audio::MusicDescriptor> _battle_music;
	//std::vector<hoa_audio::SoundDescriptor> _battle_sound;

	//! The music that is played during the battle

	//! Current list of actors 
	std::deque<private_battle::PlayerActor *> _player_actors;
	
        //! The global enemies used in this battle
        //   Used for restoring the battle mode
        std::deque<hoa_global::GlobalEnemy> _global_enemies;
        
	//actors actually in battle
	std::deque<private_battle::EnemyActor *> _enemy_actors;
	std::deque<private_battle::PlayerActor *> _players_characters_in_battle;
	
        //! a queue of scripted events to perform
        std::list<private_battle::ScriptEvent> _script_queue;
        
	//! the user interface belonging to this battle mode
	private_battle::BattleUI _user_interface;
        
	//! Is the battle over
	bool _battle_over;

	//! if _battle_over == true the battle was either won or lost
	bool _victorious_battle;
	
	//! Is an action being performed?
	bool _performing_script;
        		
        //! Swapping information
        uint32 _num_swap_cards;
        uint32 _max_swap_cards;
        uint32 _last_time_swap_awarded;
	
	//! Drawing methods
	void _DrawBackground();
	void _DrawCharacters();
	
	//! Shutdown the battle mode
	void _ShutDown();
	
	//!Are we performing an action
	bool _IsPerformingScript();
        
        void _TEMP_LoadTestData();
        
        void _BuildPlayerCharacters();
        void _BuildEnemyActors();

public:
        static int MAX_PLAYER_CHARACTERS_IN_BATTLE;
        static int MAX_ENEMY_CHARACTERS_IN_BATTLE;
        
	BattleMode();
	~BattleMode();

	//! \brief Resets appropriate class members. Called whenever BootMode is made the active game mode.
	void Reset();
	//! \brief Wrapper function that calls different update functions depending on the battle state.
	void Update();
	//! \brief Wrapper function that calls different draw functions depending on the battle state.
	void Draw();
	
	//! \brief Sets T/F whether an action is being performed
	void SetPerformingScript(bool AIsPerforming);
        
        //! \brief Added a scripted event to the queue
        void AddScriptEventToQueue(private_battle::ScriptEvent AEventToAdd);
        
        //! \brief Remove all scripted events for an actor
        void RemoveScriptedEventsForActor(private_battle::Actor *AActorToRemove);

	//! \brief Returns all player actors
	std::deque<private_battle::PlayerActor *> ReturnCharacters() const;
        
        //! \brief The number of players alive
        uint32 NumberOfPlayerCharactersAlive();
        
	//! Is the battle over?
	bool IsBattleOver();
	//! Was the battle victorious?
	bool IsVictorious();
        //! \brief Victory stuff
        void PlayerVictory();
        //! \brief Defeat stuff
        void PlayerDefeat();
        
        uint32 GetNumberOfPlayerCharacters();
        uint32 GetNumberOfEnemyActors();
        int32 GetIndexOfFirstAliveEnemy();
        int32 GetIndexOfFirstIdleCharacter();
        
        //! \brief Return the player character at the deque location 'index'
        private_battle::PlayerActor* GetPlayerCharacterAt(uint32 AIndex) const;
        private_battle::EnemyActor* BattleMode::GetEnemyActorAt(uint32 AIndex) const;
        
        //! \brief Returns the index of a player character 
        int32 IndexLocationOfPlayerCharacter(private_battle::PlayerActor * const AActor);
        
        //! \brief Swap a character from _player_actors to _player_actors_in_battle
        // This may become more complicated if it is done in a wierd graphical manner
        void SwapCharacters(private_battle::PlayerActor *AActorToRemove, private_battle::PlayerActor *AActorToAdd);
};


} // namespace hoa_battle

#endif
