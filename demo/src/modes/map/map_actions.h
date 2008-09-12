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

// Allacrost utilities
#include "defs.h"
#include "utils.h"

// Local map mode headers
#include "map_objects.h"

namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief An abstract class for representing a sprite action
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

	//! \brief Executes the action
	virtual void Execute() = 0;

	/** \brief Inidicates if the action is finished or not and resets the finished member if it is
	*** \return True if the action is finished, false if it is not.
	*** \note This is not a normal class member accessor method since it conditionally modifies the class state
	**/
	bool IsFinishedReset()
		{ if (!_finished) return false; else _finished = false; return true; }

	//! \name Class member access methods
	//@{
	const bool IsFinished() const
		{ return _finished; }

	bool IsForced() const
		{ return _forced; }

	void SetFinished(bool fin)
		{ _finished = fin; }

	void SetForced(bool forc)
		{ _forced = forc; }

	void SetSprite(VirtualSprite* sp)
		{ _sprite = sp; }
	//@}

protected:
	//! \brief A pointer to the map sprite that performs the action
	VirtualSprite *_sprite;

	//! \brief Set to true when the action has finished its execution
	bool _finished;

	//! \brief If true, the action should be forced to finish in order to let a dialogue continue
	bool _forced;
}; // class SpriteAction


/** ****************************************************************************
*** \brief Moves a sprite from a source position to a destination
***
*** This class enables a sprite to move between a source and a destination node.
*** Pathfinding is done between source and destination via the A* algorithm.
*** Once a path is found, it is saved and then used by the sprite. If the sprite
*** needs to traverse between the same source->destination once again, this path
*** is first checked to make sure it is still valid and if so, it will be re-used.
*** *****************************************************************************/
class ActionPathMove : public SpriteAction {
public:
	ActionPathMove(VirtualSprite* sprite) :
		SpriteAction(sprite), current_node(0)
		{}

	~ActionPathMove()
		{}

	// ----- Members -----

	//! \brief The destination coordinates for this path movement
	PathNode destination;

	//! \brief Holds the path needed to traverse from source to destination
	std::vector<PathNode> path;

	//! \brief An index to the path vector containing the node that the sprite currently occupies
	uint32 current_node;

	// ----- Methods -----

	//! \brief Moves the sprite along the path toward the destination, and computes a new path if necessary,
	void Execute();

	/** \brief Sets the destination location for this path movement action
	*** \param x The x (grid column) location to seek
	*** \param y The y (grid row) location to seek
	*** \note Calling this function will clear the path vector
	**/
	void SetDestination(int16 x, int16 y);
}; // class ActionPathMove : public SpriteAction


/** ****************************************************************************
*** \brief Action for causing random movement of sprites
***
*** This class has several parameters that can be set to alter or constrain the
*** type of random movement. These parameters include, for example, the amount of
*** time to move randomly before proceeding to the sprite's next action, whether
*** the sprite's position should be confined to a specific map zone, etc.
*** *****************************************************************************/
class ActionRandomMove : public SpriteAction {
public:
	ActionRandomMove(VirtualSprite* sprite) :
		SpriteAction(sprite), total_movement_time(10000), movement_timer(0), total_direction_time(1500), direction_timer(0) {}

	~ActionRandomMove()
		{}

	// ----- Members -----

	/** \brief The amount of time (in milliseconds) to perform random movement before ending this action
	*** Set this member to hoa_system::INFINITE_TIME in order to continue the random movement
	*** forever. The default value of this member will be set to 10 seconds if it is not specified.
	**/
	uint32 total_movement_time;

	//! \brief A timer which keeps track of how long the sprite has been in random movement
	uint32 movement_timer;

	/** \brief The amount of time (in milliseconds) that the sprite should continue moving in its current direction
	*** The default value for this timer is 1.5 seconds (1500ms).
	**/
	uint32 total_direction_time;

	//! \brief A timer which keeps track of how long the sprite has been moving around since the last change in direction.
	uint32 direction_timer;

	/** \brief A pointer to the map zone, if any, that the sprite should constrain their random movement to.
	*** \todo This feature has not yet been implemented
	**/
	MapZone* zone;

	// ----- Methods -----

	//! \brief Updates the movement timers and movement direction of the sprite
	void Execute();
}; // class ActionPathMove : public SpriteAction


/** ****************************************************************************
*** \brief Action that displays specific sprite frames for a certain period of time
***
*** This action displays a certain animation of a sprite for a specified amount of time.
*** Its primary purpose is to allow complete control over how a sprite appears to the
*** player and to show the sprite interacting with its surroundings, such as flipping
*** through a book taken from a bookshelf. Looping of these animations is also supported.
***
*** \note You <b>must</b> add at least one frame to this object. Calling the Execute()
*** function when the object contains no frame entries will cause a segmentation fault.
***
*** \note These actions can not be used with VirtualSprite objects, since this
*** class explicitly needs animation images to work and virtual sprites have no
*** images to work with.
*** ***************************************************************************/
class ActionAnimate : public SpriteAction {
public:
	ActionAnimate(VirtualSprite* sprite) :
		SpriteAction(sprite), current_frame(0), display_timer(0), loop_count(0), number_loops(0)
		{}

	~ActionAnimate()
		{}

	// ----- Members -----

	//! \brief Index to the current frame to display from the frames vector
	uint32 current_frame;

	//! \brief Used to count down the display time of the current frame
	uint32 display_timer;

	//! \brief A counter for the number of animation loops that have been performed
	int32 loop_count;

	/** \brief The number of times to loop the display of the frame set before finishing
	*** A value less than zero indicates to loop forever. Be careful with this,
	*** because that means that the action would never arrive at the "finished"
	*** state.
	***
	*** \note The default value of this member is zero, which indicates that the
	*** animations will not be looped (they will run exactly once to completion).
	**/
	int32 number_loops;

	/** \brief Holds the sprite animations to display for this action
	*** The values contained here are indeces to the sprite's animations vector
	**/
	std::vector<uint16> frames;

	/** \brief Indicates how long to display each frame
	*** The size of this vector should be equal to the size of the frames vector
	**/
	std::vector<uint32> frame_times;

	// ----- Methods -----

	/** \brief Adds a new frame to the animation set
	*** \param frame The index of the sprite's animations to display
	*** \param time The amount of time, in milliseconds, to display this frame
	**/
	void AddFrame(uint16 frame, uint32 time)
		{ frames.push_back(frame); frame_times.push_back(time); }

	//! \brief Resets all counters and timers so that the action sequence may re-start
	void Reset();

	//! \brief Updates the display timer and changes the current frame when appropriate
	void Execute();

	/** \name Lua Access Functions
	*** These functions are specifically written to enable Lua to access the members of this class
	**/
	//@{
	void SetLoopCount(int32 count)
		{ loop_count = count; }
	//@}
}; // class ActionAnimate : public SpriteAction


/** ****************************************************************************
*** \brief Action that invokes a Lua subroutine to be executed
*** \todo This class has not yet been implemented.
*** ***************************************************************************/
// class ActionScriptFunction : public SpriteAction {
// public:
// 	ActionScriptFunction()
// 		{}
//
// 	~ActionScriptFunction()
// 		{}
//
// 	void Execute()
// 		{}
// }; // class ActionScriptFunction : public SpriteAction

} // namespace private_map

} // namespace hoa_map

#endif
