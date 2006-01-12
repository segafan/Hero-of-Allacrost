///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    map_actions.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 20th, 2005
 * \brief   Header file for map mode actions.
 *****************************************************************************/

#ifndef __MAP_ACTIONS_HEADER__
#define __MAP_ACTIONS_HEADER__

#include "utils.h"
#include <string>
#include <vector>
#include "defs.h"
#include "engine.h"
#include "video.h"
#include "map.h"

//! All calls to map mode are wrapped in this namespace.
namespace hoa_map {

//! An internal namespace to be used only within the boot code. Don't use this namespace anywhere else!
namespace private_map {

/*!****************************************************************************
 * \brief Abstract class for sprite actions.
 *
 * Map sprites can perform a variety of different actions, from movement to
 * emotional animation. This class serves as a parent class for the different
 * actions that sprites can take. These actions include:
 *
 * -# Intelligent pathfinding for moving between two tiles seperated by any distance
 * -# Displaying specific sprite frames for a specified period of time.
 * -# Executing code from a Lua script
 * -# Random movement
 *****************************************************************************/
class SpriteAction {
public:
	//! A pointer to the map sprite that holds this action.
	MapSprite *sprite;
	//! An identifier for the type of action (the children that inherit from this class).
	uint8 type;

	SpriteAction() {}
	~SpriteAction() {}

	//! Loads the data for this action from the map's data file.
	//! \param table_key The index of the table in the map data file that contains this action's data.
	virtual void Load(uint32 table_key) = 0;
	//! The process function executes the said action.
	virtual void Process() = 0;
};

/*!****************************************************************************
 * \brief Action involving movement between a source and destination tile.
 *
 * This class retains and processes information needed for a sprite to move between
 * a source and a destination tile. Pathfinding is done between source and
 * destination via the A* algorithm. Once a path is found, it is saved and used by
 * the sprite. If the sprite needs to traverse between the same source->destination
 * once again, this path is first checked to make sure it is still valid, and if so
 * it is automatically used once again.
 *
 *
 *****************************************************************************/
class ActionPathMove : public SpriteAction {
public:
	//! The destination tile of this path movement
	TileNode destination;
	//! The path we need to traverse from source to destination
	std::vector<TileNode> path;

	ActionPathMove() {}
	~ActionPathMove() {}

	void Load(uint32 table_key);
	void Process();

	//! Computes a new path, either because a previous path doesn't exist or it is unoptimal.
	void FindNewPath() {}
};

/*!****************************************************************************
 * \brief Action that displays specific sprite frames for a certain period of time.
 *
 * This type of sprite action is usually reserved for displaying emotional reactions
 * in a sprite. It specifies a series of frames and the time to display those frames.
 *
 * \note The _frame_times and _frame_indeces vectors should \c always be the same size.
 *****************************************************************************/
class ActionFrameDisplay : public SpriteAction {
public:
	//! The amount of time to display each frame, in milliseconds.
	std::vector<uint32> frame_times;
	//! The index in the sprite's image frame vector to display.
	std::vector<uint32> frame_indeces;

	ActionFrameDisplay() {}
	~ActionFrameDisplay() {}

	void Load(uint32 table_key) {}
	void Process() {}
};

/*!****************************************************************************
 * \brief Action that runs a Lua script.
 *
 * This kind of action is nothing more than a vector of pointers to a Lua function
 * in the map file. The Lua function is part of the map sprite's representation in
 * the Lua file. This type of action lets the sprite do virtually anything, or it
 * could even operate on other sprites or the map itself (although this could
 * cause problems if not used carefully).
 *
 *****************************************************************************/
class ActionScriptFunction : public SpriteAction {
public:
	//! The function index of the sprite object containing the function to execute.
	std::vector<uint32> function_index;

	ActionScriptFunction() {}
	~ActionScriptFunction() {}

	void Load(uint32 table_key) {}
	void Process() {}
};

/*!****************************************************************************
 * \brief Action that moves a sprite in a random direction.
 *
 * This action initiates "random movement" for a sprite. This action will likely
 * be the least-used sprite action since maps don't seem very "alive" when all
 * the sprites are just walking around randomly, but it will be appropriate to
 * use in some portions of the game.
 *
 *****************************************************************************/
class ActionRandomMove : public SpriteAction {
public:
	//! The number of times to move to a random tile.
	//! \note If this value is less than zero, random movement is continued until acted on by an outside force.
	int32 number_moves;
	//! The number of milliseconds to wait between successive moves.
	uint32 wait_time;

	ActionRandomMove() {}
	~ActionRandomMove() {}

	void Load(uint32 table_key) {}
	void Process() {}
};

} // namespace private_map

} // namespace hoa_map

#endif
