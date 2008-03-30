///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
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
#include "map_objects.h"

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
	bool IsFinishedReset()
		{ if (!_finished) return false; else _finished = false; return true; }

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

	//! \brief This contains if the action should be forced to finish or not during a dialogue.
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
	// ----- Members -----

	//! \brief The destination tile of this path movement
	PathNode destination;

	//! \brief The path we need to traverse from source to destination
	std::vector<PathNode> path;

	//! \brief An index to the path vector containing the node that the sprite is currently on.
	uint32 current_node;

	// ----- Methods ----

	ActionPathMove(VirtualSprite* sprite) :
		SpriteAction(sprite), current_node(0) {}

	~ActionPathMove()
		{}

	void Execute();

	void SetDestination(int16 x, int16 y)
		{ destination.col = x; destination.row = y; }
}; // class ActionPathMove : public SpriteAction


/** ****************************************************************************
*** \brief Action for declaring random movement of sprites
***
*** This class has several parameters that can be set to define the random movement.
*** These parameters include, for example, the amount of time to move randomly before
*** proceeding to the sprite's next action, any temporary changes in movement speed
*** during the random movement, whether the sprite's position should be confined
*** to a specific map zone, etc.
*** *****************************************************************************/
class ActionRandomMove : public SpriteAction {
public:
	// ----- Members -----
	/** \brief The amount of time to perform random movement before ending this action
	*** Set this member to hoa_system::INFINITE_TIME in order to continue the random movement
	*** forever. The default value of this member will be set to 10 seconds if it is not specified.
	**/
	uint32 total_movement_time;

	//! \brief A timer which keeps track of how long the sprite has moved about randomly
	uint32 movement_timer;

	/** \brief The amount of time (in milliseconds) that the sprite should continue moving in one direction
	*** The default value for this timer is two seconds (2000ms).
	**/
	uint32 total_direction_time;

	//! \brief A timer which keeps track of how long the sprite has been moving around since the last change in direction.
	uint32 direction_timer;

	/** \brief A pointer to the map zone, if any, that the sprite should constrain their random movement to.
	*** TODO: has not yet been implemented
	**/
	MapZone* zone;

	// ----- Methods -----

	ActionRandomMove(VirtualSprite* sprite) :
		SpriteAction(sprite), total_movement_time(10000), movement_timer(0), total_direction_time(2000), direction_timer(0) {}

	~ActionRandomMove()
		{}

	void Execute();
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
***
*** \note You MUST add at least one frame to the object if you are intending to use it.
*** Calling the Execute() function when the object contains no frame entries will cause
*** a segmentation fault.
*** ***************************************************************************/
class ActionAnimate : public SpriteAction {
public:
	/** \brief The sprite animation to display for this action.
	*** This is an index to the sprite's animations vector.
	**/
	std::vector<uint16> frames;

	/** \brief Indicates how long to display each frame
	*** The size of this vector should be equal to the size of the frames vector
	**/
	std::vector<uint32> display_times;

	//! \brief Indicates the current frame
	uint32 current_frame;

	//! \brief Used to count down the display time of the current frame
	uint32 timer;

	//! \brief A counter for the number of loops
	int8 loop_count;

	/** \brief The number of times to loop the series of frames before finishing.
	*** A value less than zero indicates to loop forever. Be careful with this,
	*** because that means that the action would never arrive at the "finished"
	*** state.
	***
	*** \note The default value of this member is zero, which indicates that the
	*** animations will not be looped.
	**/
	int8 loops;

	ActionAnimate(VirtualSprite* sprite) :
		SpriteAction(sprite), current_frame(0), timer(0), loops(0) {}

	~ActionAnimate()
		{}

	void Execute();

	void AddFrame(uint16 frame, uint32 time)
		{ frames.push_back(frame); display_times.push_back(time); }

	void SetLoops(int8 count)
		{ loops = count; }
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
