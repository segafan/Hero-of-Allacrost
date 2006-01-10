///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    map_objects.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 20th, 2005
 * \brief   Header file for map mode objects.
 *****************************************************************************/

#ifndef __MAP_OBJECTS_HEADER__
#define __MAP_OBJECTS_HEADER__

#include "utils.h"
#include "defs.h"
#include "engine.h"
#include "video.h"
#include "map_actions.h"
#include "map_dialogue.h"

//! All calls to map mode are wrapped in this namespace.
namespace hoa_map {

//! An internal namespace to be used only within the boot code. Don't use this namespace anywhere else!
namespace private_map {

// *********************** OBJECT CONSTANTS **************************

//! \name Map Object Types
//! \brief Object idenfitier constants for use in the object layer.
//@{
//! The default type value of an object.
const uint8 EMPTY_OBJECT     = 0x00;
//! A "virtual sprite" with no physical image, used as a focus for the map's camera.
const uint8 MAP_CAMERA       = 0x01;
//! The player-controlled character sprite.
const uint8 PLAYER_SPRITE    = 0x02;
//! Standard NPC sprites. May also be party members.
const uint8 NPC_SPRITE       = 0x04;
//! Sprites of non-standard sizes (small animals, etc)
const uint8 OTHER_SPRITE     = 0x08;
//! Enemy sprites, various sizes
const uint8 ENEMY_SPRITE     = 0x10;
//! A still, non-animate object
const uint8 STATIC_OBJECT    = 0x20;
//! A still and animate object
const uint8 DYNAMIC_OBJECT   = 0x40;
//! A "middle-layer" object, such as a bridge.
const uint8 MIDDLE_OBJECT    = 0x80;
//@}

// *********************** SPRITE CONSTANTS **************************

/*!
 *  \brief The number of standard walking/standing frames for a sprite.
 *
 *  Up and down movement require 5 animation frames each, while left and right movement
 *  requires 7 animation frames. Diagonal movement uses the up/down/left/right frames.
 */
const uint32 SPRITE_STD_FRAME_COUNT = 24;

//! \name Map Sprite Speeds
//! \brief Common speeds for sprite movement.
//! These values are essentially the time (in milliseconds) that it takes a sprite to walk one tile.
//@{
const float VERY_SLOW_SPEED = 1000.0f;
const float SLOW_SPEED      = 800.0f;
const float NORMAL_SPEED    = 400.0f;
const float FAST_SPEED      = 300.0f;
const float VERY_FAST_SPEED = 200.0f;
//@}

//! \name Map Sprite Delay Times
//! \brief Common delay times in-between tile movements (in milliseconds).
//@{
const uint32 VERY_LONG_DELAY  = 500;
const uint32 LONG_DELAY       = 400;
const uint32 NORMAL_DELAY     = 300;
const uint32 SHORT_DELAY      = 200;
const uint32 VERY_SHORT_DELAY = 100;
const uint32 NO_DELAY         = 0;
//@}

//! \name Map Sprite Directions
//! \brief Constants used for sprite directions (NW_NORTH = sprite facing north, moving northwest).
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
const uint16 NORTHWEST = 0x1000;
const uint16 NORTHEAST = 0x2000;
const uint16 SOUTHWEST = 0x4000;
const uint16 SOUTHEAST = 0x8000;
const uint16 LATERAL   = 0x000F;
const uint16 DIAGONAL  = 0x0FF0;
//@}

//! \name Sprite Status Constants
//! \brief A series of constants used for sprite status.
//@{
//! Tracks sprite step frame (right or left foot step first).
const uint16 STEP_SWAP          = 0x0001;
//! This is for detecting whether the sprite is currently moving.
const uint16 IN_MOTION          = 0x0002;
//! If this bit is set to zero, we do not update the sprite.
const uint16 UPDATEABLE         = 0x0004;
//! If this bit is set to zero, we do not draw the sprite.
const uint16 VISIBLE            = 0x0008;
//! If this bit is set to one, we continue to draw this object even when it is not in context.
const uint16 ALWAYS_IN_CONTEXT  = 0x0010;
//@}

/*! \name Sprite Animation Vector Access Constants
 *  \brief These constants are used for indexing a standard sprite animation frame vector.
 *
 *  Every sprite has a set of 20 frames for standard movement. There are 5 frames per direction
 *  (north, south, west, east). 8-directional movement is supported with these 20 frames by
 *  simply displaying the "west" frames when the sprite is moving south west. The frame order for
 *  a simple step forward is the following:
 *
 *  - FRAME_0 (start position, standing)
 *  - FRAME_1 (left foot forward)
 *  - FRAME_2 (left foot forward)
 *  - FRAME_3 (neutral position)
 *  - FRAME_4 (right foot forward)
 *  - FRAME_5 (right foot forward)
 *  - FRAME_0 (stop position, standing)
 */
//@{
const uint8 DOWN_STANDING  = 0;
const uint8 DOWN_NEUTRAL   = 1;
const uint8 DOWN_LSTEP1    = 2;
const uint8 DOWN_LSTEP2    = 3;
const uint8 DOWN_RSTEP1    = 4;
const uint8 DOWN_RSTEP2    = 5;
const uint8 UP_STANDING    = 6;
const uint8 UP_NEUTRAL     = 7;
const uint8 UP_LSTEP1      = 8;
const uint8 UP_LSTEP2      = 9;
const uint8 UP_RSTEP1      = 10;
const uint8 UP_RSTEP2      = 11;
const uint8 LEFT_STANDING  = 12;
const uint8 LEFT_NEUTRAL   = 13;
const uint8 LEFT_LSTEP1    = 14;
const uint8 LEFT_LSTEP2    = 15;
const uint8 LEFT_RSTEP1    = 16;
const uint8 LEFT_RSTEP2    = 17;
const uint8 RIGHT_STANDING = 18;
const uint8 RIGHT_NEUTRAL  = 19;
const uint8 RIGHT_LSTEP1   = 20;
const uint8 RIGHT_LSTEP2   = 21;
const uint8 RIGHT_RSTEP1   = 22;
const uint8 RIGHT_RSTEP2   = 23;
//@}

/*!****************************************************************************
 * \brief Abstract class for all map objects.
 *
 * A map object can be anything from a sprite to a house. To summarize it simply,
 * a map objects is a map image that is \c not tiled. In MapMode, all objects in
 * the map are split into the following categories:
 *
 * - A single virtual sprite for the map (serves as a camera/focus point)
 * - Ground layer objects (sprites, houses, etc)
 * - "Middle layer" objects, like bridges that sprites can go both over and under.
 * - Sky objects that are drawn above everything else on the map.
 *****************************************************************************/
class MapObject {
public:
	//! A type identifier for the object.
	uint8 object_type;
	//! An identification number for the object as it is represented in the map file.
	uint32 object_id;
	//! The map row position for the bottom left corner of the object.
	int16 row_position;
	//! The map column position for the bottom left corner of the object.
	int16 col_position;
	//! The width of the object's image, in number of tiles.
	uint8 obj_width;
	//! The height of the object's image, in number of tiles.
	uint8 obj_height;
	//! The altitude of the object.
	//! \note This member is only used for objects in the ground object layer.
	uint8 altitude;
	//! A bit-mask for setting and detecting various conditions on the object.
	uint16 status;
	//! The map context for the object (determines whether sprites are inside, outside, etc.)
	uint8 context;

	//! A pointer to the currently active instance of MapMode.
	static MapMode *CurrentMap;

	MapObject();
	~MapObject();

	/*! \brief Member Access functions
	 *
	 *  These functions are for setting and retrieving the values of the members for the MapObject class.
	 *  Almost always after constructing a new MapObject object, you should invoke these functions to set
	 *  all the properties of the object. Otherwise the object won't do anything useful.
	 */
	//@{
	void SetObjectType(uint8 type) { object_type = type; }
	void SetRowPosition(int16 row) { row_position = row; }
	void SetColPosition(int16 col) { col_position = col; }
	void SetAltitude(uint8 alt) { altitude = alt; }
	void SetStatus(uint16 stat) { status = stat; }

	uint8 GetObjectType() { return object_type; }
	int16 GetRowPosition() { return row_position; }
	int16 GetColPosition() { return col_position; }
	uint8 GetAltitude() { return altitude; }
	uint16 GetStatus() {return status; }
	//@}

	/*!
	 *  \brief Updates the state of an object on a map.
	 *
	 *  Many map objects may not actually have a use for this function. For example, animated objects like a
	 *  tree automatically have their frames updated by the video engine, so there is no need to
	 *  call this function for it. The function is only called for objects which have the UPDATEABLE bit in
	 *  the MapObject#_status member set.
	 */
	virtual void Update() = 0;
	/*!
	 *  \brief Draws the object to the frame buffer.
	 *
	 *  Objects are drawn differently depending on what type of object they are and what their current
	 *  state is. This function is only called for objects that will be visible on the screen when drawn
	 *  and have their VISIBLE bit in the MapObject#_status member set.
	 */
	virtual void Draw() = 0;
};

/*!****************************************************************************
 * \brief A mobile map object with which the player can interact with.
 *
 * Map sprites are basically animate, "living" map objects. Although there is
 * but a single class to represent all the map sprites in the game, they can
 * actually be divided into the following categories:
 *
 * - Virtual sprites (a controllable, invisible map camera of sorts)
 * - Playable character sprites (those that are in the player's party)
 * - NPC (non-playable character) sprites.
 * - Enemy sprites
 * - Other sprites (like animals, etc.)
 *
 * It is easier to manage a single sprite class rather than several classes for
 * each sprite type. The fact that there is only one class for representing
 * several sprite types is the reason why many of these class members are pointers.
 * For example, we don't need dialogue for a dog sprite, so we don't want to waste
 * unnecessary space.
 *
 * \note 1) The vertical or "Z" position of the object is contained in the lower
 * 8 bits of the status member. I may change this so z position is self-contained
 * in another member of this class later.
 *****************************************************************************/
class MapSprite : public MapObject {
public:
	//! The name of the sprite, as seen by the player in the game.
	hoa_utils::ustring name;
	//! The base filename of the sprite, used to load various data for the sprite.
	std::string filename;
	//! A bit-mask for various information regarding the sprite's draw orientation.
	uint16 direction;
	//! The speed at which the sprite moves around the map.
	float step_speed;
	//! A counter to keep track of a sprites actual position when moving between tiles.
	float step_count;
	//! The tile column position plus an offset between 0.0 and 1.0 (used for sprite movement).
	float x_position;
	//! The tile row position plus an offset between 0.0 and 1.0 (used for sprite movement).
	float y_position;
	//! The remaining amount of time to wait before moving the sprite again (used for NPCs).
	int32 wait_time;
	//! The average amount of time a sprite should remain still between tile moves.
	uint32 delay_time;
	//! Retains the frame index that should be drawn on the next frame.
	uint8 frame;
	//! True if all the dialogue of this sprite has been seen by the player.
	bool seen_all_dialogue;

	/*! \name Saved State Members
	 *  \brief Used to temporarily retain some state properties of a sprite.
	 *  These members are primarly used to store and restore the state of a sprite that is
	 *  interrupted from a dialogue.
	 */
	//@{
	uint16 saved_direction;
	uint16 saved_status;
	uint8 saved_frame;
	//@}

	//! A container for all of the actions this sprite performs.
	std::vector<SpriteAction*> actions;
	//! An index into the _actions vector, representing the current sprite action.
	uint8 current_action;


	/*! \brief A pointer to a vector containing all the sprite's frame images.
	 *
	 *  Frames 0-23 are the standard animation frames. It is safe to assume that for all sprites
	 *  (other than the virtual sprite), these indeces exist. Every frame past index number 23
	 *  are special frames, used for map-specific events and scripts.
	 */
	std::vector<hoa_video::StillImage> frames;

	/*! \brief Retains and manages all the possible conversations one may have with this sprite.
	 *
	 *  It is legal for a sprite to have no dialogue at all. The player controlled sprite obviously
	 *  can't have any dialogue, because the player can't initiate a conversation with him/herself.
	 */
	SpriteDialogue dialogue;
//	//! Retain the dialogues that correspond to the sprite.
//	std::vector<MapDialogue> _dialogues;
//	//! A pointer to the next dialogue for the user to read.
//	uint8 _next_dialogue;

	/*! \brief Determines the standard animation frame to draw for the sprite.
	 *
	 *  Note that special frames (frames not part of the standard walk/stand animation) are of no
	 *  concern to this function. This simply finds the standard animation frame for normal sprite
	 *  operation.
	 */
	void FindFrame();

	MapSprite();
	~MapSprite();

	// TODO: bool SeenAllDialogue() { return _see_all_dialogue; }

	//! Fills up the frames vector and loads the sprite image frames.
	void LoadFrames();

	/*!
	 * \brief Attempts to move a sprite in an indicated direction.
	 * \param direction The direction in which the sprite wishes to move.
	 *
	 * This function is only called for sprites that are located in the
	 * ground object layer. It finds the tile to move to based on the direction that
	 * the sprite is currently facing, and won't move the sprite if the tile is occupied.
	 */
	void Move(uint16 direction);

	/*!
	 * \brief Adds a new dialogue to the sprite.
	 * \param new_dia The new dialogue to add to the end of the MapSprite#_dialogues vector
	 *
	 * Naturally, this will automatically set the MapSprite#_seen_all_dialogue flag to false.
	 */
	void AddDialogue(std::vector<std::string> new_dia);
	//! Updates the dialogue pointer when a dialogue finishes.
// 	void FinishedDialogue() { next_dialogue = static_cast<uint8>((_next_dialogue + 1) % _dialogues.size()); }

	//! Saves the status and frame of the sprite (primarily used for dialogues).
	void SaveState();
	//! Restores the status and frame of the sprite (primarily used for dialogues).
	void RestoreState();

	//! Updates the sprite position and state.
	void Update();
	//! Draws the appropriate sprite frame in the appropriate position on the screen, if at all.
	void Draw();

	/*! \brief Member Access functions
	 *
	 *  These functions are for setting and retrieving the values of the members for the MapSprite class.
	 *  Almost always after constructing a new MapSprite object, you should invoke these functions to set
	 *  all the properties of the object. Otherwise the sprite won't do anything useful.
	 *
	 *  \note Not all members of this class have member access functions, because they are not
	 */
	//@{
	void SetName(hoa_utils::ustring na) { name = na; }
	void SetFilename(std::string fn) { filename = fn; }
	void SetDirection(uint16 di) { direction = di; }
	void SetStepSpeed(float sp) { step_speed = sp; }
	void SetDelayTime(uint32 delay) { delay_time = delay; }

	hoa_utils::ustring GetName() { return name; }
	std::string GetFilename() { return filename; }
	uint16 GetDirection() { return direction; }
	float GetStepSpeed() { return step_speed; }
	uint32 GetDelayTime() { return delay_time; }
	//@}
}; // class MapSprite

} // namespace private_map

} // namespace hoa_map

#endif
