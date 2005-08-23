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
#include <string>
#include <vector>
#include "defs.h"
#include "engine.h"
#include "video.h"

//! All calls to map mode are wrapped in this namespace.
namespace hoa_map {

//! An internal namespace to be used only within the boot code. Don't use this namespace anywhere else!
namespace private_map {

// *********************** OBJECT CONSTANTS **************************

//! \name Map Object Types
//@{
//! \brief Object idenfitier constants for use in the object layer.
const uint8 VIRTUAL_SPRITE = 0x00; // Sprites with no physical image
const uint8 PLAYER_SPRITE  = 0x01; // Playable character sprites
const uint8 NPC_SPRITE     = 0x02; // Regular NPC sprites
const uint8 ADV_SPRITE     = 0x04; // 'Advanced' NPC sprites with more emotion frames
const uint8 OTHER_SPRITE   = 0x08; // Sprites of non-standard sizes (small animals, etc)
const uint8 ENEMY_SPRITE   = 0x10; // Enemy sprites, various sizes
const uint8 STATIC_OBJECT  = 0x20; // A still, non-animate object
const uint8 DYNAMIC_OBJECT = 0x40; // A still and animate object
const uint8 MIDDLE_OBJECT  = 0x80; // A "middle-layer" object
//@}

// Note: The lower 8 bits of the status member for sprites use the Z_LVL_X constants

/*! 
 *  \brief The number of standard walking/standing frames for a sprite.
 *  
 *  Up and down movement require 5 animation frames, while left and right movement
 *  requires 14 animation frames. Diagonal movement uses the up/down/left/right frames.
 */
const uint32 SPRITE_STD_FRAME_COUNT = 24;

//! \name Map Sprite Speeds
//@{
//! \brief Common speeds for sprite movement.
const float VERY_SLOW_SPEED = 25.0;
const float SLOW_SPEED      = 22.5;
const float NORMAL_SPEED    = 20.0;
const float FAST_SPEED      = 17.5;
const float VERY_FAST_SPEED = 15.0;
//@}

//! \name Map Sprite Delay Times
//@{
//! \brief Common delay times in-between tile movements (in milliseconds).
const uint32 VERY_LONG_DELAY  = 500;
const uint32 LONG_DELAY       = 400;
const uint32 NORMAL_DELAY     = 300;
const uint32 SHORT_DELAY      = 200;
const uint32 VERY_SHORT_DELAY = 100;
const uint32 NO_DELAY         = 0;
//@}

//! \name Map Sprite Directions
//@{
//! \brief Constants used for sprite directions (NORTH_NW = sprite facing north, moving northwest).
const uint32 NORTH     = 0x00000100;
const uint32 SOUTH     = 0x00000200;
const uint32 WEST      = 0x00000400;
const uint32 EAST      = 0x00000800;
const uint32 NORTH_NW  = 0x00001000;
const uint32 WEST_NW   = 0x00002000;
const uint32 NORTH_NE  = 0x00004000;
const uint32 EAST_NE   = 0x00008000;
const uint32 SOUTH_SW  = 0x00010000;
const uint32 WEST_SW   = 0x00020000;
const uint32 SOUTH_SE  = 0x00040000;
const uint32 EAST_SE   = 0x00080000;
//@}

//! Used in bit-wise ops to get a sprite's current direction.
const uint32 FACE_MASK = 0x000FFF00;

//! \name Sprte Move Constants
//@{
//! \brief These are used by the MapSprite::SpriteMove() function.
const int32 MOVE_NORTH = 0;
const int32 MOVE_SOUTH = 1;
const int32 MOVE_WEST  = 2;
const int32 MOVE_EAST  = 3;
const int32 MOVE_NW    = 4;
const int32 MOVE_SW    = 5;
const int32 MOVE_NE    = 6;
const int32 MOVE_SE    = 7;
//@}
 
//! \name Sprite Status Constants
//@{
//! \brief A series of constants used for sprite status.
//! Tracks sprite step frame (right or left foot step first).
const uint32 STEP_SWAP   = 0x00100000; 
//! This is for detecting whether the sprite is currently moving.
const uint32 IN_MOTION   = 0x00200000; 
//! If this bit is set to zero, we do not update the sprite's position.
const uint32 UPDATEABLE  = 0x00400000; 
//! If this bit is set to zero, we do not draw the sprite.
const uint32 VISIBLE     = 0x00800000; 
//! If this bit is set, the sprite can be controlled by the player.
const uint32 CONTROLABLE = 0x01000000; 
//! Sprite frames are located in GameInstance singleton.
const uint32 SPR_GLOBAL  = 0x02000000; 
//@}

//! \name Sprite Animation Vector Access Constants
//@{
//! \brief These constants are used for indexing a standard sprite animation frame vector.
const int32 DOWN_STANDING  = 0;
const int32 DOWN_LSTEP1    = 1;
const int32 DOWN_LSTEP2    = 2;
const int32 DOWN_LSTEP3    = 1;
const int32 DOWN_RSTEP1    = 3;
const int32 DOWN_RSTEP2    = 4;
const int32 DOWN_RSTEP3    = 3;
const int32 UP_STANDING    = 5;
const int32 UP_LSTEP1      = 6;
const int32 UP_LSTEP2      = 7;
const int32 UP_LSTEP3      = 6;
const int32 UP_RSTEP1      = 8;
const int32 UP_RSTEP2      = 9;
const int32 UP_RSTEP3      = 8;
const int32 LEFT_STANDING  = 10;
const int32 LEFT_LSTEP1    = 11;
const int32 LEFT_LSTEP2    = 12;
const int32 LEFT_LSTEP3    = 13;
const int32 LEFT_RSTEP1    = 14;
const int32 LEFT_RSTEP2    = 15;
const int32 LEFT_RSTEP3    = 16;
const int32 RIGHT_STANDING = 17;
const int32 RIGHT_LSTEP1   = 18;
const int32 RIGHT_LSTEP2   = 19;
const int32 RIGHT_LSTEP3   = 20;
const int32 RIGHT_RSTEP1   = 21;
const int32 RIGHT_RSTEP2   = 22;
const int32 RIGHT_RSTEP3   = 23;
//@}

} // namespace private_map



/*!****************************************************************************
 * \brief Abstract class for all map objects.
 *
 * A map object can be anything from a sprite to a house. To summarize it simply,
 * a map objects is a map image that is \c not tiled. In MapMode, all objects in 
 * the map are split into the following categories:
 * 
 * - A single virtual sprite for the map (serves as a camera/focus point)
 * - Regular objects (sprites, houses, etc)
 * - "Middle layer" objects, like bridges that sprites can go both over and under.
 * - Sky objects that are drawn last and above everything else on the map.
 *
 * \note 1) The vertical or "Z" position of the object is contained in the lower
 * 8 bits of the status member. I may change this so z position is self-contained
 * in another member of this class later.
 *****************************************************************************/
class ObjectLayer {
protected:
	//! A type identifier for the object.
	uint8 object_type;
	//! The map row position for the bottom left corner of the object.
	uint32 row_pos;
	//! The map column position for the bottom left corner of the object.
	uint32 col_pos;
	//! A bit-mask for setting and detecting various conditions on the object.
	uint32 status;
	//! The vertical or "Z" position of the object.
	uint32 z_pos;
	//! A reference to the video engine singleton.
	hoa_video::GameVideo *VideoManager;

	friend class MapMode;
	friend class hoa_data::GameData;
public:
	ObjectLayer(uint8 type, uint32 row, uint32 col, uint32 status);
	~ObjectLayer();
	/*!
	 *  \brief Draws the object to the frame buffer.
	 *  \param &mf A reference to the map frame information so the object knows where to draw itself.
	 *
	 *  This function is purely virtual because objects are drawn differently depending
	 *  on what type of properties they have. For example, there are static objects, animated objects,
	 *  and sprites which have numerous frames.
	 */
	virtual void Draw(private_map::MapFrame& mf) = 0;
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
class MapSprite : public ObjectLayer {
private:
	friend class MapMode;
	friend class hoa_data::GameData;

	/*!
	 * \brief Determines what frame image should be drawn for the sprite.
	 * \return An index to the frames vector for the frame image to draw.
	 */
	uint32 FindFrame();
protected:
	//! The name of the sprite, as seen by the player in the game.
	std::string name;
	//! The base filename of the sprite, used to load various data for the sprite.
	std::string filename;
	//! The speed at which the sprite moves around the map.
	float step_speed;
	//! A counter to keep track of a sprites actual position when moving between tiles.
	float step_count;
	//! The remaining amount of time to wait before moving the sprite again (used for NPCs).
	int32 wait_time;
	//! The average amount of time a sprite should remain still between tile moves.
	uint32 delay_time;

	//! A pointer to a vector containing all the sprite's frame images.
	std::vector<hoa_video::ImageDescriptor> *frames;
	//! Retains the dialogue that the sprite has to say.
	SpriteDialogue *speech;
	/*!
	 *  \brief Draws the object to the frame buffer.
	 *  \param &mf A reference to the map frame information so the object knows where to draw itself.
	 */
	void Draw(private_map::MapFrame& mf);
public:
	MapSprite(uint8 type, uint32 row, uint32 col, uint32 stat);
	~MapSprite();

	//! Fills up the *frames vector.
	void LoadFrames();
	/*!
	 *  \brief Grabs the already loaded character data from the GameInstance singleton.
	 *  \param character The ID for the character to load data for.
	 *  
	 *  This function is only used for playable character sprites. That data is already maintained
	 *  by the GameInstance singleton so we don't want to load the data twice.
	 */
	void LoadCharacterInfo(uint32 character);

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	void SetName(std::string na) { name = na; }
	void SetFilename(std::string fn) { filename = fn; }
	void SetSpeed(float ss) { step_speed = ss; }
	void SetDelay(uint32 dt) { delay_time = dt; }
	//@}
}; // class MapSprite

/*!****************************************************************************
 * \brief A class for retaining and managing a sprite's dialogue.
 *
 * Dialogues in map mode are rather complex. We would like to have dialogues
 * between a character and an NPC, dialogues between multiple NPCs, etc. This
 * class is still in its infant stages and support for some of the more advanced
 * dialogue types has yet to be implemented.
 *****************************************************************************/
class SpriteDialogue {
private:
	//! An index to the next dialogue piece to read.
	uint32 next_read;
	//! The dialogue itself, broken into conversations and individual lines.
	std::vector<std::vector<std::string> > dialogue;
	//! A vector indicating whom is the speaker for a section of dialogue.
	std::vector<std::vector<uint32> > speaker;
	//! A boolean for each piece of dialogue, representing whether the player has read it or not.
	std::vector<bool> seen;
	//! True if the player has already read all of the sprite's dialogue.
	bool seen_all;

	friend class MapMode;
public:
	SpriteDialogue();
	~SpriteDialogue();

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	bool AllDialogueSeen() { return seen_all; }
	void LoadDialogue(std::vector<std::vector<std::string> > txt, std::vector<std::vector<uint32> > sp);
	void AddDialogue(std::vector<std::string> txt, std::vector<uint32> sp);
	void AddDialogue(std::string txt, uint32 sp);
	void ReadDialogue();
	//@}
}; // class SpriteDialogue

} // namespace hoa_map

#endif
