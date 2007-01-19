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
		_sprite(sprite), _finished(false) {}

	virtual ~SpriteAction()
		{}

	/** \brief Loads the data for this action from the map's data file.
	*** \param table_key The index of the table in the map script file that contains the action's data.
	**/
	virtual void Load(uint32 table_key) = 0;

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

	bool _forced;

	int16 _count;
protected:
	//! \brief A pointer to the map sprite that this action is performed upon.
	VirtualSprite *_sprite;

	//! \brief Set to true when the action has finished its execution.
	bool _finished;

	
};


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

	ActionPathMove(VirtualSprite* sprite, int16 count = 1, bool forced = false) :
	SpriteAction(sprite), current_node(0) {
		_forced = forced;
		_count = count;
		
	}

	~ActionPathMove()
		{}

	void Load(uint32 table_key);

	void Execute();
};


/*!****************************************************************************
 * \brief Action that displays specific sprite frames for a certain period of time.
 *
 * This type of sprite action is usually reserved for displaying emotional reactions
 * in a sprite. It specifies a series of frames and the time to display those frames.
 *
 * \note The _frame_times and _frame_indeces vectors should \c always be the same size.
 *****************************************************************************/
// class ActionAnimate : public SpriteAction {
// public:
// 	/** \brief The sprite animation to display for this action.
// 	*** Because this is a pointer, it
// 	**/
// 	hoa_video::AnimatedImage *animation;
// 	/** \brief Indicates to destroy the animation image on class destruction
// 	*** If a new animation image is created by this class, this member should
// 	*** be set to true. If it is false, however, this means that the animation
// 	*** creation/destruction is handled elsewhere, most likely in the MapSprite#_images
// 	*** vector.
// 	**/
// 	bool destroy_image;
// 	/** \brief The number of times to loop the animation before finishing.
// 	*** A value less than zero indicates to loop forever.
// 	**/
// 	int8 loop;
//
// 	ActionAnimate() {}
// 	~ActionAnimate() {}
//
// 	void Load(uint32 table_key);
// 	void Execute();
// }; // class ActionAnimate : public SpriteAction

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
