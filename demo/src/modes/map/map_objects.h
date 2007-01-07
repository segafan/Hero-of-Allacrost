///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_objects.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for map mode objects.
*** *****************************************************************************/

#ifndef __MAP_OBJECTS_HEADER__
#define __MAP_OBJECTS_HEADER__

#include "utils.h"
#include "defs.h"
#include "video.h"
#include "map_actions.h"
#include "map_dialogue.h"

//! \brief All calls to map mode are wrapped in this namespace.
namespace hoa_map {

//! \brief An internal namespace to be used only within the boot code. Don't use this namespace anywhere else!
namespace private_map {

// *********************** SPRITE CONSTANTS **************************

/** \name Map Sprite Speeds
*** \brief Common speeds for sprite movement.
*** These values are the time (in milliseconds) that it takes a sprite to walk
*** the distance of a single tile (32 pixels).
**/
//@{
const float VERY_SLOW_SPEED = 1000.0f;
const float SLOW_SPEED      = 800.0f;
const float NORMAL_SPEED    = 400.0f;
const float FAST_SPEED      = 300.0f;
const float VERY_FAST_SPEED = 100.0f;
//@}

/** \name Sprite Directions
*** \brief Constants used for sprite directions
*** Sprites are allowed to travel in eight different directions, however the sprite itself
*** can only be facing one of four ways: north, south, east, or west. Because of this, it
*** is possible to travel, for instance, northwest facing north or northwest facing west.
*** The "NW_NORTH" constant means that the sprite is traveling to the northwest and is
*** facing towards the north.
**/
//@{
const uint16 NORTH     = 0x0001;
const uint16 SOUTH     = 0x0002;
const uint16 WEST      = 0x0004;
const uint16 EAST      = 0x0008;
const uint16 NW_NORTH  = 0x0010;
const uint16 NW_WEST   = 0x0020;
const uint16 NE_NORTH  = 0x0040;
const uint16 NE_EAST   = 0x0080;
const uint16 SW_SOUTH  = 0x0100;
const uint16 SW_WEST   = 0x0200;
const uint16 SE_SOUTH  = 0x0400;
const uint16 SE_EAST   = 0x0800;
//@}

const uint16 FACING_NORTH = NORTH | NW_NORTH | NE_NORTH;
const uint16 FACING_SOUTH = SOUTH | SW_SOUTH | SE_SOUTH;
const uint16 FACING_WEST = WEST | NW_WEST | SW_WEST;
const uint16 FACING_EAST = EAST | NE_EAST | SE_EAST;

const uint16 MOVING_NORTH = NORTH;
const uint16 MOVING_SOUTH = SOUTH;
const uint16 MOVING_WEST = WEST;
const uint16 MOVING_EAST = EAST;
const uint16 MOVING_NORTHWEST = NW_NORTH | NW_WEST;
const uint16 MOVING_NORTHEAST = NE_NORTH | NE_EAST;
const uint16 MOVING_SOUTHWEST = SW_SOUTH | SW_WEST;
const uint16 MOVING_SOUTHEAST = SE_SOUTH | SE_EAST;

const uint32 ANIM_STANDING_SOUTH = 0;
const uint32 ANIM_STANDING_NORTH = 1;
const uint32 ANIM_STANDING_WEST  = 2;
const uint32 ANIM_STANDING_EAST  = 3;
const uint32 ANIM_WALKING_SOUTH  = 4;
const uint32 ANIM_WALKING_NORTH  = 5;
const uint32 ANIM_WALKING_WEST   = 6;
const uint32 ANIM_WALKING_EAST   = 7;

/** ****************************************************************************
*** \brief Abstract class that represents objects on a map
***
*** A map object can be anything from a sprite to a house. To summarize it simply,
*** a map object is a map image that is not tiled and not fixed in place. Map
*** objects are drawn in one of three layers: ground, pass, and sky object layers.
*** There are many different types of map objects, defined by the Map Object Types
*** constants above.
*** ***************************************************************************/
class MapObject {
public:
	/** \brief An identification number for the object as it is represented in the map file.
	*** Player sprites are assigned object ids from 5000 and above. Technically this means that
	*** a map can have no more than 5000 objects that are not player sprites, but no map should
	*** need to contain that many objects in the first place. Objects with an ID less than zero
	*** are invalid.
	**/
	int16 object_id;

	/** \brief The map context that the object currently resides in.
	*** Context helps to determine where an object is. Objects can only interact with one another
	*** if they both reside in the same context.
	***
	**/
	uint8 context;

	/** \brief Coordinates for the object's origin/position.
	*** The origin of every map object is the bottom center point of the object. These
	*** origin coordinates are used to determine where the object is on the map as well
	*** as where the objects collision area lies. One map grid tile is equal to 1.0f on this
	*** coordinate system, with the x, y origin of 0.0f, 0.0f lying in the upper-left
	*** hand corner of the map.
	*** 
	*** The position coordinates are described by an integer and an offset. The offset will always
	*** range from 0.0f and 1.0f.
	**/
	//@{
	uint16 x_position, y_position;
	float x_offset, y_offset;
	//@}

	/** \brief The half-width and height of the image, in map grid coordinates.
	*** The half_width member is indeed just that: half the width of the sprite. We keep the half
	*** width and not the full width because the origin of the sprite is the bottom center, and
	*** it is more convenient to store only half the sprite's width as a result
	**/
	float half_width, height;

	//! \brief When set to false, the Update() function will do nothing
	bool updatable;

	//! \brief When set to false, the Draw() function will do nothing
	bool visible;

	/** \brief When set to true, the object will always be drawn regardless of the map's current context
	*** \note If visible is set to false however, the object will not be drawn.
	**/
	bool always_in_context;

	/** \brief When set to true, objects in the ground object layer will be drawn after the pass objects
	*** This member is only checked for ground objects. It has no meaning for objects in the pass or 
	*** sky layers.
	**/
	bool draw_on_second_pass;

	/** \brief Determines the collision rectangle for the object.
	*** The collision area determines what portion of the map object may not be overlapped
	*** by other objects or unwalkable regions of the map. The x and y coordinates are
	*** relative to the origin, so an x value of 0.5f means that the collision rectangle
	*** extends the length of 1/2 of a tile from the origin on both sides, and a y value
	*** of 1.0f means that the collision area exists from the origin to 1 tile's length
	*** above.
	***
	*** \note If these members are set to zero, then the object will not have a collision
	*** rectangle. This is useful for immaterial objects such as ghosts, which can pass
	*** through objects.
	**/
	float coll_half_width, coll_height;

	MapObject();
	virtual ~MapObject()
		{}

	/** \brief Updates the state of an object.
	*** Many map objects may not actually have a use for this function. For example, animated objects like a
	*** tree automatically have their frames updated by the video engine, so there is no need to
	*** call this function for it. The function is only called for objects which have the UPDATEABLE bit in
	*** the MapObject#_status member set.
	**/
	virtual void Update() = 0;

	/** \brief Draws the object to the frame buffer.
	*** Objects are drawn differently depending on what type of object they are and what their current
	*** state is. This function is only called for objects that will be visible on the screen when drawn
	*** and have their VISIBLE bit in the MapObject#_status member set.
	**/
	virtual void Draw() = 0;

	void SetObjectID(int16 id)
		{ object_id = id; }

	void SetContext(uint8 ctxt)
		{ context = ctxt; }

	void SetXPosition(uint16 x, float offset)
		{ x_position = x; x_offset = offset; }

	void SetYPosition(uint16 y, float offset)
		{ y_position = y; y_offset = offset; }

	void SetXCollision(float collision)
		{ coll_half_width = collision; }

	void SetYCollision(float collision)
		{ coll_height = collision; }

	void SetUpdatable(bool update)
		{ updatable = update; }

	void SetVisible(bool vis)
		{ visible = vis; }

	void SetAlwaysInContext(bool in_context)
		{ always_in_context = in_context; }

	void SetDrawOnSecondPass(bool pass)
		{ draw_on_second_pass = pass; }
}; // class MapObject


/** ****************************************************************************
*** \brief A mobile map object with which the player can interact with.
***
*** Map sprites are animate, mobile, living map objects. Although there is
*** but this single class to represent all the map sprites in the game, they can
*** divided into types such as NPCs, friendly creatures, and enemies. The fact
*** that there is only one class for representing several sprite types is the
*** reason why many of these class members are pointers. For example, we don't
*** need dialogue for a dog sprite.
*** ***************************************************************************/
class MapSprite : public MapObject {
public:
	//! \brief The name of the sprite, as seen by the player in the game.
	hoa_utils::ustring name;

	/** \brief A bit-mask for the sprite's draw orientation and direction vector.
	*** This member determines both where to move the sprite (8 directions) and
	*** which way the sprite is facing (4 directions). See the Sprite Directions
	*** series of constants for the values that this member may be set to.
	**/
	uint16 direction;

	//! \brief The speed at which the sprite moves around the map.
	float movement_speed;

	/** \brief Set to true when the sprite is currently moving.
	*** \note This does not necessarily mean that the sprite actually is moving, but rather that
	*** the sprite is <i>trying</i> to move in a certain direction.
	**/
	bool moving;

	//! \brief Holds the previous value of MapSprite#moving from the last call to MapSprite#Update().
	bool was_moving;

	/** \brief The sound that will play when the sprite walks.
	*** This member references the MapMode#_map_sounds vector as the sound to play. If
	*** this member is less than zero, no sound is played when objects walk upon this
	*** tile.
	**/
	int8 walk_sound;

	/** \brief A pointer to a vector containing all the sprite's animation frames.
	*** The first four entries in this vector are the walking animation frames.
	*** They are ordered from index 0 to 3 as: down, up, left, right. Additional
	*** animations may follow.
	**/
	std::vector<hoa_video::AnimatedImage> animations;

	//! \brief The index to the animations vector containing the current sprite image to display
	uint8 current_animation;

	//! \brief A pointer to the face portrait of the sprite, as seen in dialogues and menus.
	hoa_video::StillImage* face_portrait;

	// -------------------------------- Methods --------------------------------

	MapSprite();
	~MapSprite();

	/** \brief Fills up the frames vector and loads the sprite image frames.
	*** \return False if there was a problem loading the sprite.
	**/
	bool Load();

	//! \brief Updates the sprite's position and state.
	void Update();

	//! \brief Draws the sprite frame in the appropriate position on the screen, if it is visible.
	void Draw();

	/** \brief Attempts to move a sprite in an indicated direction.
	*** \param direction The direction in which the sprite wishes to move.
	***
	*** This function is only called for sprites that are located in the
	*** ground object layer. It finds the tile to move to based on the direction that
	*** the sprite is currently facing, and won't move the sprite if the tile is occupied.
	**/
	// void Move(uint16 direction);
}; // class MapSprite : public MapObject

} // namespace private_map

} // namespace hoa_map

#endif
