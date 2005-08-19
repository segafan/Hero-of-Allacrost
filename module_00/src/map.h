///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    map.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 19th, 2005
 * \brief   Header file for map mode interface.
 *
 * This code handles the game event processing and frame drawing when the user
 * is in map mode (when the user is exploring town or dungeon maps). This
 * includes handling of tile images, sprites, and events that occur on the map.
 *
 * Each individual map is represented by it's own object
 * of the MapMode class. At this time, the intention is to keep the three most
 * recently accessed maps in memory so there is no loading time when the player
 * backtraces his or her steps. When a new map is loaded and there are already
 * three
 * 
 * \note Because this file and map.cpp are already so huge (and still have a lot
 * of growth planned), the contents of these files may be split up in the near
 * future.
 *****************************************************************************/

#ifndef __MAP_HEADER__
#define __MAP_HEADER__

#include "utils.h"
#include <string>
#include <vector>
#include "defs.h"
#include "engine.h"
#include "video.h"

//! All calls to map mode are wrapped in this namespace.
namespace hoa_map {

//! Determines whether the code in the hoa_map namespace should print debug statements or not.
extern bool MAP_DEBUG;

//! An internal namespace to be used only within the boot code. Don't use this namespace anywhere else!
namespace private_map {

// ************************ MAP CONSTANTS ****************************

//! The rate at which tiles are animated, in ms
const int ANIMATION_RATE = 300;

//! \name Screen Coordiante System Constants
//@{
//! \brief The number of rows and columns of tiles that compose the screen.
const int SCREEN_ROWS = 20;
const int SCREEN_COLS = 28;
//@}

//! \name Map State Constants
//@{
//! \brief Constants used for describing the current state of operation during map mode.
const int EXPLORE      = 0x00000001;
const int DIALOGUE     = 0x00000002;
const int SCRIPT_EVENT = 0x00000004;
//@}

//! \name Z Level Constants
//@{
//! \brief The z value constants that determine an object's "height" on the map.
const uint Z_LVL1 = 0x00000001;
const uint Z_LVL2 = 0x00000002;
const uint Z_LVL3 = 0x00000004;
const uint Z_LVL4 = 0x00000008;
const uint Z_LVL5 = 0x00000010;
const uint Z_LVL6 = 0x00000020;
const uint Z_LVL7 = 0x00000040;
const uint Z_LVL8 = 0x00000080;
const uint Z_MASK = 0x000000FF;
//@}

// ********************** TILE CONSTANTS **************************

// Note: The lower 8 bits of the properties member for tiles use the z level constants.

//! \name Z Level Set Constants
//@{
//! \brief Constants for setting the z level of objects that move to certain areas on the map.
const uint SET_Z_LVL1  = 0x00000100;
const uint SET_Z_LVL2  = 0x00000200;
const uint SET_Z_LVL3  = 0x00000400;
const uint SET_Z_LVL4  = 0x00000800;
const uint SET_Z_LVL5  = 0x00001000;
const uint SET_Z_LVL6  = 0x00002000;
const uint SET_Z_LVL7  = 0x00004000;
const uint SET_Z_LVL8  = 0x00008000;
const uint SET_Z_MASK  = 0x0000FF00;
//@}

//! Increments the z level of the object by one (max 8).
const uint INC_Z_LEVEL = 0x00010000;
//! Increments the z level of the object by one (min 0).
const uint DEC_Z_LEVEL = 0x00020000;
//! Indicates a treasure is contained on this tile.
const uint TREASURE    = 0x00040000;
//! Indicates that an event will take place when the player steps onto this tile
const uint EVENT       = 0x00080000;

// *********************** OBJECT CONSTANTS **************************

//! \name Map Object Types
//@{
//! \brief Object idenfitier constants for use in the object layer.
const unsigned char VIRTUAL_SPRITE = 0x00; // Sprites with no physical image
const unsigned char PLAYER_SPRITE  = 0x01; // Playable character sprites
const unsigned char NPC_SPRITE     = 0x02; // Regular NPC sprites
const unsigned char ADV_SPRITE     = 0x04; // 'Advanced' NPC sprites with more emotion frames
const unsigned char OTHER_SPRITE   = 0x08; // Sprites of non-standard sizes (small animals, etc)
const unsigned char ENEMY_SPRITE   = 0x10; // Enemy sprites, various sizes
const unsigned char STATIC_OBJECT  = 0x20; // A still, non-animate object
const unsigned char DYNAMIC_OBJECT = 0x40; // A still and animate object
const unsigned char MIDDLE_OBJECT  = 0x80; // A "middle-layer" object
//@}

// ********************** SPRITE CONSTANTS **************************

// Note: The lower 8 bits of the status member for sprites use the Z_LVL_X constants

/*! 
 *  \brief The number of standard walking/standing frames for a sprite.
 *  
 *  Up and down movement require 5 animation frames, while left and right movement
 *  requires 14 animation frames. Diagonal movement uses the up/down/left/right frames.
 */
const uint SPRITE_STD_FRAME_COUNT = 24;

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
const uint VERY_LONG_DELAY  = 500;
const uint LONG_DELAY       = 400;
const uint NORMAL_DELAY     = 300;
const uint SHORT_DELAY      = 200;
const uint VERY_SHORT_DELAY = 100;
const uint NO_DELAY         = 0;
//@}

//! \name Map Sprite Directions
//@{
//! \brief Constants used for sprite directions (NORTH_NW = sprite facing north, moving northwest).
const uint NORTH     = 0x00000100;
const uint SOUTH     = 0x00000200;
const uint WEST      = 0x00000400;
const uint EAST      = 0x00000800;
const uint NORTH_NW  = 0x00001000;
const uint WEST_NW   = 0x00002000;
const uint NORTH_NE  = 0x00004000;
const uint EAST_NE   = 0x00008000;
const uint SOUTH_SW  = 0x00010000;
const uint WEST_SW   = 0x00020000;
const uint SOUTH_SE  = 0x00040000;
const uint EAST_SE   = 0x00080000;
//@}

//! Used in bit-wise ops to get a sprite's current direction.
const uint FACE_MASK = 0x000FFF00;

//! \name Sprte Move Constants
//@{
//! \brief These are used by the MapSprite::SpriteMove() function.
const int MOVE_NORTH = 0;
const int MOVE_SOUTH = 1;
const int MOVE_WEST  = 2;
const int MOVE_EAST  = 3;
const int MOVE_NW    = 4;
const int MOVE_SW    = 5;
const int MOVE_NE    = 6;
const int MOVE_SE    = 7;
//@}
 
//! \name Sprite Status Constants
//@{
//! \brief A series of constants used for sprite status.
//! Tracks sprite step frame (right or left foot step first).
const uint STEP_SWAP   = 0x00100000; 
//! This is for detecting whether the sprite is currently moving.
const uint IN_MOTION   = 0x00200000; 
//! If this bit is set to zero, we do not update the sprite's position.
const uint UPDATEABLE  = 0x00400000; 
//! If this bit is set to zero, we do not draw the sprite.
const uint VISIBLE     = 0x00800000; 
//! If this bit is set, the sprite can be controlled by the player.
const uint CONTROLABLE = 0x01000000; 
//! Sprite frames are located in GameInstance singleton.
const uint SPR_GLOBAL  = 0x02000000; 
//@}

//! \name Sprite Animation Vector Access Constants
//@{
//! \brief These constants are used for indexing a standard sprite animation frame vector.
const int DOWN_STANDING  = 0;
const int DOWN_LSTEP1    = 1;
const int DOWN_LSTEP2    = 2;
const int DOWN_LSTEP3    = 1;
const int DOWN_RSTEP1    = 3;
const int DOWN_RSTEP2    = 4;
const int DOWN_RSTEP3    = 3;
const int UP_STANDING    = 5;
const int UP_LSTEP1      = 6;
const int UP_LSTEP2      = 7;
const int UP_LSTEP3      = 6;
const int UP_RSTEP1      = 8;
const int UP_RSTEP2      = 9;
const int UP_RSTEP3      = 8;
const int LEFT_STANDING  = 10;
const int LEFT_LSTEP1    = 11;
const int LEFT_LSTEP2    = 12;
const int LEFT_LSTEP3    = 13;
const int LEFT_RSTEP1    = 14;
const int LEFT_RSTEP2    = 15;
const int LEFT_RSTEP3    = 16;
const int RIGHT_STANDING = 17;
const int RIGHT_LSTEP1   = 18;
const int RIGHT_LSTEP2   = 19;
const int RIGHT_LSTEP3   = 20;
const int RIGHT_RSTEP1   = 21;
const int RIGHT_RSTEP2   = 22;
const int RIGHT_RSTEP3   = 23;
//@}

/*!****************************************************************************
 * \brief Retains information about how the next map frame should be drawn.
 *
 * This class is used by map objects to determine where (and if) they should
 * be drawn.
 * 
 * \note 1) The MapMode class keeps an active object of this class with the latest
 * information about the map, so you should never need to create an instance of it.
 *****************************************************************************/
class MapFrame {
public:
	//! The starting index of the tile column to draw.
	int c_start;
	//! The starting index of the tile row to draw.
	int r_start;
	//! Column coordinate for setting the drawing cursor.
	float c_pos;
	//! Row coordinate for setting the drawing cursor.
	float r_pos;
	//! The number of columns of tiles that need to be drawn.
	int c_draw;
	//! The number of rows of tiles that need to be drawn.
	int r_draw;
}; // class MapFrame

} // namespace private_map


/*!****************************************************************************
 * \brief Class structure for representing the 2D vector that composes the map.
 *****************************************************************************/
class MapTile {
public:
	//! Index to a lower layer tile in the MapMode tile_frames vector
	int lower_layer;
	//! Index to an upper layer tile in the MapMode tile_frames vector
	int upper_layer;
	//! A bit-mask indicating various tile properties.
	uint properties;
}; // class MapTile


/*!****************************************************************************
 * \brief Element of a circular singlely linked list for tile frame animations.
 *
 * \note 1) Obviously, not all tiles are animated. For those that aren't, the
 * list will only contain one item and the next pointer will point to itself.
 *****************************************************************************/
class TileFrame {
public:
	//! Holds a frame index pointing to map_tiles in the MapMode class.
	int frame;
	//! A pointer to the next frame in the animation.
	TileFrame *next;
}; // class TileFrame


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
	unsigned char object_type;
	//! The map row position for the bottom left corner of the object.
	uint row_pos;
	//! The map column position for the bottom left corner of the object.
	uint col_pos;
	//! A bit-mask for setting and detecting various conditions on the object.
	uint status;
	//! A reference to the video engine singleton.
	hoa_video::GameVideo *VideoManager;

	friend class MapMode;
	friend class hoa_data::GameData;
public:
	ObjectLayer(unsigned char type, uint row, uint col, uint status);
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
	int FindFrame();
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
	int wait_time;
	//! The average amount of time a sprite should remain still between tile moves.
	uint delay_time;

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
	MapSprite(unsigned char type, uint row, uint col, uint stat);
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
	void LoadCharacterInfo(uint character);

	//! \name Public Member Access Functions
	//@{
	//! \brief Used for setting and getting the values of the various class members.
	void SetName(std::string na) { name = na; }
	void SetFilename(std::string fn) { filename = fn; }
	void SetSpeed(float ss) { step_speed = ss; }
	void SetDelay(uint dt) { delay_time = dt; }
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
	uint next_read;
	//! The dialogue itself, broken into conversations and individual lines.
	std::vector<std::vector<std::string> > dialogue;
	//! A vector indicating whom is the speaker for a section of dialogue.
	std::vector<std::vector<uint> > speaker;
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
	void LoadDialogue(std::vector<std::vector<std::string> > txt, std::vector<std::vector<uint> > sp);
	void AddDialogue(std::vector<std::string> txt, std::vector<uint> sp);
	void AddDialogue(std::string txt, uint sp);
	void ReadDialogue();
	//@}
}; // class SpriteDialogue

/*!****************************************************************************
 * \brief Handles everything that needs to be done when the player is exploring maps.
 *
 * This code in this class and its respective partner classes is arguably one of the
 * most complex pieces of the game to date. Basic functionality in this class has been
 * working for a while, but we still have much work to do here (namely, integrating
 * map scripts). I intend to more fully document the primary operational features of
 * this class at a later time, but I would like to wait until it is in a more finalized
 * state before I do so.
 *
 * \note 1) If you change the state of random_encounters from false to true, make 
 * sure to set a valid value (< 0) for steps_till_encounter. *I might change this later*
 * 
 * \note 2) Be careful with calling the MapMode constructor, for it changes the coordinate 
 * system of the video engine without warning. Only create a new instance of this class if
 * you plan to immediately push it on top of the game stack.
 *****************************************************************************/
class MapMode : public hoa_engine::GameMode {
private:
	friend class hoa_data::GameData;

	//! A unique ID value for the map.
	int map_id;
	//! A stack indicating the various states the map code is in (ie, exploration, dialogue, script).
	std::vector<uint> map_state;
	//! A millisecond counter for use in tile animation.
	int animation_counter;
	//! The time elapsed since the last Update() call to MapMode.
	Uint32 time_elapsed;

	//! The number of tile rows in the map.
	int row_count;
	//! The number of tile columns in the map.
	int col_count;

	//! True if this map is to have random encounters.
	bool random_encounters;
	//! The average number of steps the player takes before encountering an enemy.
	int encounter_rate;
	//! The remaining steps until the player meets their next party of foes.
	int steps_till_encounter;

	//! A 2D vector that represents the map itself.
	std::vector<std::vector<MapTile> > map_layers;
	//! A vector of circular singely-linked lists for each tile frame animation.
	std::vector<TileFrame*> tile_frames;
	//! The normal set of map objects.
	std::vector<ObjectLayer*> object_layer;
	//! A pointer to the map sprite that the map should focus on.
	MapSprite *focused_object;
	//! A "virtual sprite" that serves as a camera, available for use in each map.
	MapSprite *virtual_sprite;

	//! A vector for miscellaneous map images.
	static std::vector<hoa_video::ImageDescriptor> map_images;
	//! A vector containing the image for each map tile and frame.
	std::vector<hoa_video::ImageDescriptor> map_tiles;
	//! The music that we would like available on the map.
	std::vector<hoa_audio::MusicDescriptor> map_music;
	//! The specific sounds that the map needs available.
	std::vector<hoa_audio::SoundDescriptor> map_sound;

//	 vector<MapEvent> map_events;
//	std::vector<hoa_global::GEnemy> map_enemies;

	/*!
	 * \brief
	 * \param direction
	 */
	bool TileMoveable(int row, int col, uint z_occupied);
	/*!
	 * \brief
	 * \param direction
	 */
	void SpriteMove(int direction, MapSprite *sprite);
	//! Updates the virtual_sprite class member.
	void UpdateVirtualSprite();

	//! Updates the map when in the explore state.
	void UpdateExploreState();
	//! Updates the focused player sprite when in the explore state.
	//! \param *player_sprite A pointer to the sprite to update.
	void UpdatePlayerExplore(MapSprite *player_sprite);
	//! Updates a NPC sprite when in the explore state.
	//! \param *npc A pointer to the sprite to update.
	void UpdateNPCExplore(MapSprite *npc);

	//! Updates the map when in the dialogue state.
	void UpdateDialogueState();
	//! Updates the map when in the script state.
	void UpdateScriptState();
	//! Updates the movement of all map NPCs.
	void UpdateNPCMovement();

	//! Calculates information about how to draw the next map frame.
	//! \param &mf The refereneced object to put the calculated drawing results into.
	void GetDrawInfo(private_map::MapFrame& mf);

	// TEMPORARY FUNCTIONS FOR TESTING PURPOSES >>> eventally will be defunct
	void TempCreateMap();
	void TempCreateSprites();
public:
	//! The name of the map, as seen by the player in the game.
	std::string mapname;
	MapMode(int new_map_id);
	MapMode(int rows, int cols) { row_count = rows; col_count = cols; }
	~MapMode();

	//! \name Map Editor Access functions
	//@{
	//! \brief Used by the map editor for accessing various map information.
	//! \note These functions might go defunct if the map editor becomes independent of the game.
	std::vector<std::vector<MapTile> > GetMapLayers() { return map_layers; }
	std::vector<hoa_video::ImageDescriptor> GetMapTiles() { return map_tiles; }
	void SetRows(int num_rows) { row_count = num_rows; }
	void SetCols(int num_cols) { col_count = num_cols; }
	void SetMapLayers(std::vector<std::vector<MapTile> > layers) { map_layers = layers; }
	void SetMapTiles(std::vector<hoa_video::ImageDescriptor> tiles) { map_tiles = tiles; }
	int GetRows() { return row_count; }
	int GetCols() { return col_count; }
	//@}

	//! Updates the game and calls various sub-update functions depending on the state of map mode.
	//! \param new_time_elapsed The amount of milliseconds that have elapsed since the last call to this function.
	void Update(Uint32 new_time_elapsed);
	//! Handles the drawing of everything on the map and makes sub-draw function calls as appropriate.
	void Draw();
}; // class MapMode

} // namespace hoa_map;

#endif
