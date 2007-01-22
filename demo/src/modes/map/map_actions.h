///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_actions.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for map mode actions.
*** ***************************************************************************/

#ifndef __MAP_ACTIONS_HEADER__
#define __MAP_ACTIONS_HEADER__

#include <string>
#include <vector>

#include "defs.h"
#include "utils.h"
#include "map.h"

namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief An abstract class for sprite actions.
***
*** Map sprites can perform a variety of different actions, from movement to
*** emotional animation. This class serves as a parent class for the common
*** actions that sprites can take. The children classes are provided as a
*** convenience to the map designer and for code re-usablity. The map script
*** may also create custom actions for sprites to take in addition to the
*** actions provided in this class.
*** ***************************************************************************/
class SpriteAction {
public:
	SpriteAction(VirtualSprite *sprite) :
		_sprite(sprite), _finished(false), _forced(false) {}

	virtual ~SpriteAction()
		{}

	//! \brief Loads the data for this action from the map's data file.
	virtual void Load() = 0;

	//! \brief Executes the sprite's action.
	virtual void Execute() = 0;

	/** \brief Inidicates if the action is finished or not.
	*** \return True if the action is finished, false if it is not.
	**/
	const bool IsFinished()
		{ return _finished; }

	/** \brief Inidicates if the action is finished or not and resets the finished member if it is.
	*** \return True if the action is finished, false if it is not.
	*** \note This is not a normal class member act
	**/
	const bool IsFinishedReset()
		{ if (!_finished) return false; _finished = false; return true; }

	/** \brief This method returns if this action is forced (true) or not (false).
	*** A forced action will have to finish in order to let a dialogue continue to its next line.
	**/
	bool IsForced() const
		{ return _forced; }

	void SetFinished(bool fin)
		{ _finished = fin; }

	void SetForced(bool forc)
		{ _forced = forc; }

	void SetSprite(VirtualSprite* sp)
		{ _sprite = sp; }

protected:
	//! \brief A pointer to the map sprite that this action is performed upon.
	VirtualSprite *_sprite;

	//! \brief Set to true when the action has finished its execution.
	bool _finished;

	//! \brief This contains if the action should be forced to finish or not duri9ng a dialogue.
	bool _forced;
}; // class SpriteAction


/** ****************************************************************************
*** \brief Action involving movement between a source and destination tile.
***
*** This class enables a sprite to move between a source and a destination node.
*** Pathfinding is done between source and destination via the A* algorithm.
*** Once a path is found, it is saved and then used by the sprite. If the sprite
*** needs to traverse between the same source->destination once again, this path
*** is first checked to make sure it is still valid and if so, it is
*** automatically used once more.
*** *****************************************************************************/
class ActionPathMove : public SpriteAction {
public:
	//! \brief The destination tile of this path movement
	PathNode destination;

	//! \brief The path we need to traverse from source to destination
	std::vector<PathNode> path;

	//! \brief An index to the path vector containing the node that the sprite is currently on.
	uint32 current_node;

	ActionPathMove() :
		SpriteAction(NULL), current_node(0) {}

	ActionPathMove(VirtualSprite* sprite) :
		SpriteAction(sprite), current_node(0) {}

	~ActionPathMove()
		{}

	void Load();

	void Execute();

	void SetDestination(int16 x, int16 y)
		{ destination.col = x; destination.row = y; }
}; // class ActionPathMove : public SpriteAction


/** ****************************************************************************
*** \brief Action that displays specific sprite frames for a certain period of time.
***
*** This action displays a certain animation in a sprite for a certain amount of time.
*** It supports multiple animation + time combinations as well as looping through these
*** animations. Its primary purpose is to allow complete control over how a sprite
*** reacts to its surroundings, such as flipping through a book taken from a bookshelf.
***
*** \note The vectors ins this class should <b>always</b> be of the same size.
***
*** \note These actions can not be used with VirtualSprite objects, since this
*** class explicitly needs animation images to work and virtual sprites have no
*** sprite images to work with.
*** ***************************************************************************/
class ActionAnimate : public SpriteAction {
public:
	/** \brief The sprite animation to display for this action.
	*** This is an index to the sprite's animations vector.
	**/
	std::vector<uint16> frames;

	std::vector<uint32> timers;

	/** \brief The number of times to loop the animation before finishing.
	*** A value less than zero indicates to loop forever. Be careful with this,
	*** because that means that the action would never arrive at the "finished"
	*** state.
	***
	*** \note The default value of this member is zero, which indicates that the
	*** animations will not be looped.
	**/
	int8 loops;

	ActionAnimate(VirtualSprite* sprite) :
		SpriteAction(sprite) {}

	~ActionAnimate()
		{}

	void Load();

	void Execute();
}; // class ActionAnimate : public SpriteAction


/** ****************************************************************************
*** \brief Action that calls a Lua subroutine
***
*** ***************************************************************************/
// class ActionScriptFunction : public SpriteAction {
// public:
// 	ActionScriptFunction()
// 		{}
// 	~ActionScriptFunction()
// 		{}
//
// 	void Load(uint32 table_key)
// 		{}
// 	void Execute()
// 		{}
// };

} // namespace private_map

} // namespace hoa_map

#endif
