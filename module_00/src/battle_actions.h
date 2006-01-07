///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    battle_skillactions.h
 * \author  Corey Hoffstein, visage@allacrost.org
 * \date    Last Updated: December 15, 2005
 * \brief   Header file for battle mode interface.
 *
 * Defines skill actions that may be used to customize skills
 *****************************************************************************/

#ifndef __BATTLE_ACTIONS_HEADER__
#define __BATTLE_ACTIONS_HEADER__

#include "battle.h"
#include "defs.h"

using namespace hoa_global;

namespace hoa_battle {

/*!
	Battle Actions are Action breakdowns that can be arranged to create a 
	schmorgusborg of different effects.  If a battle action is concurrent 
	then it is assigned to the owning battle mode to take care of.
*/
class BattleAction {
	private:
		//! Is it concurrent?
		bool _isConcurrent;
		//! The skill we belong to (potential to be NULL)
		GSkill *_skill;
		//! The host we belong to (must be assigned)
		Actor *_host;
		//! The arguments we have (potential to be size 0)
		std::vector<Actor *> _arguments;
	public:
		BattleAction() : _isConcurrent(false), _skill(NULL), _host(NULL) {}
		virtual ~BattleAction() {}
		
		/*!
			Assigns all the values
		*/
		void Initialize(GSkill *sk, Actor *a, std::vector<Actor *> arguments);
		virtual void Update(uint32 dt) = 0;
		
		/*!
			Some mutators and getters
		*/
		void SetConcurrent(bool concur) { _isConcurrent = concur; }
		bool IsConcurrent() { return _isConcurrent; }
		GSkill * GetSkill() { return _skill; }
		Actor * GetHost() { return _host; }
		std::vector<Actor *> GetArguments() { return _arguments; }
};

/*!
	An action related to moving on the screen
*/
class MoveAction : public BattleAction {
	private:
		const int _moveX;
		const int _moveY;
	public:
		MoveAction(const int x, const int y) : _moveX(x), _moveY(y) {} 
		virtual ~MoveAction() {}
		int GetX() const;
		int GetY() const;
		virtual void Update(uint32 dt) = 0;
};

/*!
	A move where the given X and Y are relative to your current location
*/
class MoveRelativeToCurrentLocation : public MoveAction {
	public:
		MoveRelativeToCurrentLocation(const int x, const int y) : MoveAction(x, y) {}
		void Update(uint32 dt);
};

/*!
	A move where the given X and Y are relative to your starting position
*/
class MoveRelativeToOrigin : public MoveAction {
	private:
	public:
		MoveRelativeToOrigin(const int x, const int y) : MoveAction(x, y) {}
		void Update(uint32 dt);
};

/*!
	A move where the given X and Y are relative to another position, RX and RY
*/
class MoveRelativeToPosition : public MoveAction {
	private:
		int _relativeX;
		int _relativeY;
	public:
		MoveRelativeToPosition(const int x, const int y, const int rx, const int ry) : MoveAction(x,y), _relativeX(rx), _relativeY(ry) {}
		void Update(uint32 dt);
};

/*!
	A move where the given X and Y are absolute on the screen
*/
class MoveAbsolute : public MoveAction {
	public:
		MoveAbsolute(const int x, const int y) : MoveAction(x, y) {}
		void Update(uint32 dt);
};

/*!
	Performs a skill (data wise)
*/
class PerformSkill : public BattleAction {
	public:
		PerformSkill() {}
		void Update(uint32 dt);
};

/*!
	Has a character perform an animation
*/
class PlayCharacterAnimation : public BattleAction {
	private:
		std::string _animation;
	public:
		PlayCharacterAnimation(std::string an) : _animation(an) {}
		void Update(uint32 dt);
};

/*!
	Begins a visual effect
*/
class PerformVisualEffect : public BattleAction {
	private:
		std::string _ve;
	public:
		PerformVisualEffect(std::string ve) : _ve(ve) {}
		void Update(uint32 dt);
};

/*!
	Performs an audio effect
*/
class PerformAudioEffect : public BattleAction {
	private:
		std::string _ae;
	public:
		PerformAudioEffect(std::string ae) : _ae(ae) {}
		void Update(uint32 dt);
};

/*!
	Displays on the screen the effects of the most recent action with the skill
*/
class DisplaySkillEffects : public BattleAction {
	public:
		DisplaySkillEffects(){}
		void Update(uint32 dt);
};

/*!
	Retreats a character off of the screen
*/
class RetreatAction : public BattleAction {
	private:
	public:
		void Update(uint32 dt);
};

/*!
	Used to signify the end of a skill, and clean up all things related to that 
	skill
*/
class FinishSkill : public BattleAction {
	public:
		void Update(uint32 dt);
};

}

#endif