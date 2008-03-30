////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software and
// you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    battle_actions.h
*** \author  Viljami Korhonen, mindflayer@allacrost.org
*** \author  Andy Gardner, chopperdave@allacrost.org
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for actions that occur in battles.
***
*** Actions are events that are carried out by actors and include the execution
*** of skills or the use of items.
*** ***************************************************************************/

#ifndef __BATTLE_ACTIONS_HEADER__
#define __BATTLE_ACTIONS_HEADER__

#include "utils.h"
#include "defs.h"

#include "video.h"

#include "global.h"

namespace hoa_battle {

namespace private_battle {

/** ****************************************************************************
*** \brief Representation of a single action to be executed in battle
***
*** This is an abstract base class for all action classes to inherit from.
*** Actions are executed one at a time in a FIFO queue by BattleMode. Some
*** actions may also be continuous, in that they apply an effect on the target
*** for a limited period of time. For example, a skill which temporarily boosts
*** the strength of its target.
*** ***************************************************************************/
class BattleAction {
public:
	BattleAction(BattleActor* source, BattleActor* target, hoa_global::GlobalAttackPoint* attack_point);

	virtual ~BattleAction()
		{}

	//! \brief Updates the script
	void Update();

	//! \brief Makes sure the the target is valid, and if not it selects a new one
	void VerifyValidTarget(BattleActor* source, BattleActor* &target);

	//! \brief Executes the script
	virtual void RunScript() = 0;

	//! \brief Returns true if this action consumes an item
	virtual bool IsItemAction() const = 0;

	//! \brief Sets _should_be_removed to true so this action will be removed from the queue
	void MarkForRemoval()
		{ _should_be_removed = true; }

	/*! \brief Returns the value of _should_be_removed
	 *	\return If true, the action should be removed from the queue
	 */
	bool ShouldBeRemoved()
		{ return _should_be_removed; }

	//! \name Class member access functions
	//@{
	BattleActor* GetSource()
		{ return _source; }

	BattleActor* GetTarget()
		{ return _target; }

	hoa_system::SystemTimer* GetWarmUpTime()
		{ return &_warm_up_time; }
	//@}

protected:
	//! \brief The rendered text for the name of the action
	hoa_video::TextImage _script_name;

	//! \brief The actor whom is initiating this action
	BattleActor* _source;

	//! \brief The targets of the script
	//! \todo This should be changed to a GlobalTarget class pointer
	BattleActor* _target;

	//! \brief The selected attack point (if applicable)
	//! \todo This should be removed when the _target member is changed
	hoa_global::GlobalAttackPoint* _attack_point;

	//! \brief The amount of time to wait to execute the script
	hoa_system::SystemTimer _warm_up_time;

	//! \brief If true, the script needs to be removed
	bool _should_be_removed;
}; // class BattleAction


/** ****************************************************************************
*** \brief A battle action which involves the exectuion of an actor's skill
***
*** This class invokes the execution of a GlobalSkill contained by the source
*** actor. When the action is finished, any SP required to use the skill is
*** subtracted from the source actor.
*** ***************************************************************************/
class SkillAction : public BattleAction {
public:
	SkillAction(BattleActor* source, BattleActor* target, hoa_global::GlobalSkill* skill, hoa_global::GlobalAttackPoint* attack_point = NULL);

	void RunScript();

	bool IsItemAction() const
		{ return false; }

	hoa_global::GlobalSkill* GetSkill()
		{ return _skill; }

private:
	//! \brief Pointer to the skill attached to this script (for skill events only)
	hoa_global::GlobalSkill* _skill;
}; // class SkillAction : public BattleAction


/** ****************************************************************************
*** \brief A battle action which involves the use of an item
***
*** This class invokes the usage of a GlobalItem. The item's count is decremented
*** as soon as the action goes into the FIFO queue. After the action is executed,
*** the item is removed if its count has become zero. If the action is removed
*** from the queue before it is executed (because the source actor perished, or
*** the battle ended, or other circumstances), then the item's count is 
*** incremented back to its original value since it was not used.
*** ***************************************************************************/
class ItemAction : public BattleAction {
public:
	ItemAction(BattleActor* source, BattleActor* target, hoa_global::GlobalItem* item, hoa_global::GlobalAttackPoint* attack_point = NULL);

	void RunScript();

	bool IsItemAction() const
		{ return true; }

	hoa_global::GlobalItem* GetItem()
		{ return _item; }

private:
	//! \brief Pointer to the item attached to this script (for item events only)
	hoa_global::GlobalItem* _item;
}; // class ItemAction : public BattleAction

} // namespace private_battle

} // namespace hoa_battle

#endif // __BATTLE_ACTIONS_HEADER__
