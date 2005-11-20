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
#include "map_dialogue.h"

//! All calls to map mode are wrapped in this namespace.
namespace hoa_map {

//! An internal namespace to be used only within the boot code. Don't use this namespace anywhere else!
namespace private_map {

// *********************** OBJECT CONSTANTS **************************

//! \name Map Object Types
//! \brief Object idenfitier constants for use in the object layer.
//@{
//! Sprites with no physical image, used mainly as a focus for the map's camera.
const uint8 VIRTUAL_SPRITE   = 0x01; 
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

/*! 
 *  \brief The number of standard walking/standing frames for a sprite.
 *  
 *  Up and down movement require 5 animation frames each, while left and right movement
 *  requires 7 animation frames. Diagonal movement uses the up/down/left/right frames.
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
const uint16 NORTH     = 0x0001;
const uint16 SOUTH     = 0x0002;
const uint16 WEST      = 0x0004;
const uint16 EAST      = 0x0008;
const uint16 NORTH_NW  = 0x0010;
const uint16 WEST_NW   = 0x0020;
const uint16 NORTH_NE  = 0x0040;
const uint16 EAST_NE   = 0x0080;
const uint16 SOUTH_SW  = 0x0100;
const uint16 WEST_SW   = 0x0200;
const uint16 SOUTH_SE  = 0x0400;
const uint16 EAST_SE   = 0x0800;
const uint16 NORTHWEST = 0x1000;
const uint16 NORTHEAST = 0x2000;
const uint16 SOUTHWEST = 0x4000;
const uint16 SOUTHEAST = 0x8000;
const uint16 LATERAL   = 0x000F;
const uint16 DIAGONAL  = 0x0FF0;
//@}
 
//! \name Sprite Status Constants
//@{
//! \brief A series of constants used for sprite status.
//! Tracks sprite step frame (right or left foot step first).
const uint16 STEP_SWAP   = 0x0001; 
//! This is for detecting whether the sprite is currently moving.
const uint16 IN_MOTION   = 0x0002; 
//! If this bit is set to zero, we do not update the sprite.
const uint16 UPDATEABLE  = 0x0004; 
//! If this bit is set to zero, we do not draw the sprite.
const uint16 VISIBLE     = 0x0008; 
//! Only sprites that are in context with the player can be seen and interacted with.
const uint16 IN_CONTEXT  = 0x0010; 
//@}

//! \name Sprite Animation Vector Access Constants
//@{
//! \brief These constants are used for indexing a standard sprite animation frame vector.
const uint8 DOWN_STANDING  = 0;
const uint8 DOWN_LSTEP1    = 1;
const uint8 DOWN_LSTEP2    = 2;
const uint8 DOWN_LSTEP3    = 1;
const uint8 DOWN_RSTEP1    = 3;
const uint8 DOWN_RSTEP2    = 4;
const uint8 DOWN_RSTEP3    = 3;
const uint8 UP_STANDING    = 5;
const uint8 UP_LSTEP1      = 6;
const uint8 UP_LSTEP2      = 7;
const uint8 UP_LSTEP3      = 6;
const uint8 UP_RSTEP1      = 8;
const uint8 UP_RSTEP2      = 9;
const uint8 UP_RSTEP3      = 8;
const uint8 LEFT_STANDING  = 10;
const uint8 LEFT_LSTEP1    = 11;
const uint8 LEFT_LSTEP2    = 12;
const uint8 LEFT_LSTEP3    = 13;
const uint8 LEFT_RSTEP1    = 14;
const uint8 LEFT_RSTEP2    = 15;
const uint8 LEFT_RSTEP3    = 16;
const uint8 RIGHT_STANDING = 17;
const uint8 RIGHT_LSTEP1   = 18;
const uint8 RIGHT_LSTEP2   = 19;
const uint8 RIGHT_LSTEP3   = 20;
const uint8 RIGHT_RSTEP1   = 21;
const uint8 RIGHT_RSTEP2   = 22;
const uint8 RIGHT_RSTEP3   = 23;
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
class MapObject {
protected:
	//! A type identifier for the object.
	uint8 _object_type;
	//! The map row position for the bottom left corner of the object.
	int16 _row_pos;
	//! The map column position for the bottom left corner of the object.
	int16 _col_pos;
	//! The altitude of the object.
	//! \note This member is really only useful for objects in the ground object layer.
	uint8 _altitude;
	//! A bit-mask for setting and detecting various conditions on the object.
	uint16 _status;
	
	//! A pointer to the currently active instance of MapMode.
	static MapMode *CurrentMap;

	friend class MapMode;
	friend class hoa_data::GameData;
public:
	MapObject(uint8 type, uint32 row, uint32 col, uint8 alt, uint16 status);
	~MapObject();
	/*!
	 *  \brief Draws the object to the frame buffer.
	 *  \param &mf A reference to the map frame information so the object knows where to draw itself.
	 *
	 *  This function is purely virtual because objects are drawn differently depending
	 *  on what type of properties they have. For example, there are static objects, animated objects,
	 *  and sprites which have numerous frames.
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
private:
	friend class MapMode;
	friend class hoa_data::GameData;

	//! The name of the sprite, as seen by the player in the game.
	std::string _name;
	//! The base filename of the sprite, used to load various data for the sprite.
	std::string _filename;
	//! A bit-mask for various information regarding the sprite's draw orientation.
	uint16 _direction;
	//! The speed at which the sprite moves around the map.
	float _step_speed;
	//! A counter to keep track of a sprites actual position when moving between tiles.
	float _step_count;
	//! The remaining amount of time to wait before moving the sprite again (used for NPCs).
	int32 _wait_time;
	//! The average amount of time a sprite should remain still between tile moves.
	uint32 _delay_time;
	//! Retains the frame index that should be drawn on the next frame.
	uint8 _frame;
	//! True if all the dialogue of this sprite has been seen by the player.
	bool _seen_all_dialogue;
	
	//! \name Saved State Members
	//@{
	//! \brief Used to temporarily retain properties of the sprite.
	uint16 _saved_direction;
	uint16 _saved_status;
	uint8 _saved_frame;
	//@}
	
	
	/*! A pointer to a vector containing all the sprite's frame images.
	 * 
	 * \note The reason this is a pointer is because sprite frames may already be loaded and retained
	 * elsewhere. For example, playable character sprites can be found in the GameInstance singleton
	 * in the GCharacter class for each character.
	 */
	std::vector<hoa_video::StillImage> *_frames;
	//! Retain the dialogues that correspond to the sprite.
	std::vector<MapDialogue> _dialogues;
	//! A pointer to the next dialogue for the user to read.
	uint8 _next_dialogue;
	
	//! Finds the appropriate frame to draw for the sprite.
	void _FindFrame();
	//! Computes a random direction for movement.
	//! \return The random direction.
	uint16 _RandomDirection();
public:
	MapSprite(uint8 type, uint32 row, uint32 col, uint8 alt, uint16 stat);
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
	
	/*!
	 * \brief Attempts to move a sprite on the ground in the indicated direction.
	 * \param direction The direction in which the sprite wishes to move.
	 *
	 * This function is only called for sprites that are located in the
	 * ground object layer. It finds the tile to move to based on the direction that
	 * the sprite is currently facing, and won't move the sprite if the tile is occupied.
	 */
	void GroundMove(uint16 direction);
	/*!
	 * \brief Moves a sprite in the air in the indicated direction.
	 * \param direction The direction in which the sprite wishes to move.
	 *
	 * This function is only called for sprites that are located in the
	 * ground object layer. It finds the tile to move to based on the direction
	 * that the sprite is currently facing. It will always successfully move the 
	 * sprite since there is no collision detection in the sky layer.
	 */
	void SkyMove(uint16 direction);
	
	/*!
	 * \brief Adds a new dialogue to the sprite.
	 * \param new_dia The new dialogue to add to the end of the MapSprite#_dialogues vector
	 *
	 * Naturally, this will automatically set the MapSprite#_seen_all_dialogue flag to false.
	 */
	void AddDialogue(std::vector<std::string> new_dia);
	//! Updates the dialogue pointer when a dialogue finishes.
	void FinishedDialogue() { _next_dialogue = static_cast<uint8>((_next_dialogue + 1) % _dialogues.size()); }
	
	//! Saves the status and frame of the sprite (primarily used for dialogues).
	void SaveState();
	//! Restores the status and frame of the sprite (primarily used for dialogues).
	void RestoreState();
	
	//! Updates the sprite position and state.
	void Update();
	//! Draws the appropriate sprite frame in the appropriate position on the screen, if at all.
	void Draw();
	
	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	void SetName(std::string na) { _name = na; }
	void SetFilename(std::string fn) { _filename = fn; }
	void SetSpeed(float ss) { _step_speed = ss; }
	void SetDelay(uint32 dt) { _delay_time = dt; }
	//@}
}; // class MapSprite

} // namespace hoa_map

#endif
