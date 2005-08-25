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

//! An internal namespace to be used only within the map code. Don't use this namespace anywhere else!
namespace private_map {

// ************************ MAP CONSTANTS ****************************

//! The rate at which tiles are animated, in ms
const uint32 ANIMATION_RATE = 300;

//! \name Screen Coordiante System Constants
//@{
//! \brief The number of rows and columns of tiles that compose the screen.
const float SCREEN_ROWS = 24.0;
const float SCREEN_COLS = 32.0;
//@}

//! \name Map State Constants
//@{
//! \brief Constants used for describing the current state of operation during map mode.
const uint32 EXPLORE      = 0x00000001;
const uint32 DIALOGUE     = 0x00000002;
const uint32 SCRIPT_EVENT = 0x00000004;
//@}

//! \name Z Level Constants
//@{
//! \brief The z value constants that determine an object's "height" on the map.
const uint32 Z_LVL1 = 0x00000001;
const uint32 Z_LVL2 = 0x00000002;
const uint32 Z_LVL3 = 0x00000004;
const uint32 Z_LVL4 = 0x00000008;
const uint32 Z_LVL5 = 0x00000010;
const uint32 Z_LVL6 = 0x00000020;
const uint32 Z_LVL7 = 0x00000040;
const uint32 Z_LVL8 = 0x00000080;
const uint32 Z_MASK = 0x000000FF;
//@}

// ********************** TILE CONSTANTS **************************

// Note: The lower 8 bits of the properties member for tiles use the z level constants.

//! \name Z Level Set Constants
//@{
//! \brief Constants for setting the z level of objects that move to certain areas on the map.
const uint32 SET_Z_LVL1  = 0x00000100;
const uint32 SET_Z_LVL2  = 0x00000200;
const uint32 SET_Z_LVL3  = 0x00000400;
const uint32 SET_Z_LVL4  = 0x00000800;
const uint32 SET_Z_LVL5  = 0x00001000;
const uint32 SET_Z_LVL6  = 0x00002000;
const uint32 SET_Z_LVL7  = 0x00004000;
const uint32 SET_Z_LVL8  = 0x00008000;
const uint32 SET_Z_MASK  = 0x0000FF00;
//@}

//! Increments the z level of the object by one (max 8).
const uint32 INC_Z_LEVEL = 0x00010000;
//! Increments the z level of the object by one (min 0).
const uint32 DEC_Z_LEVEL = 0x00020000;
//! Indicates a treasure is contained on this tile.
const uint32 TREASURE    = 0x00040000;
//! Indicates that an event will take place when the player steps onto this tile
const uint32 EVENT       = 0x00080000;

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
	int32 c_start;
	//! The starting index of the tile row to draw.
	int32 r_start;
	//! Column coordinate for setting the drawing cursor.
	float c_pos;
	//! Row coordinate for setting the drawing cursor.
	float r_pos;
	//! The number of columns of tiles that need to be drawn.
	uint32 c_draw;
	//! The number of rows of tiles that need to be drawn.
	uint32 r_draw;
}; // class MapFrame

} // namespace private_map


/*!****************************************************************************
 * \brief Class structure for representing the 2D vector that composes the map.
 *****************************************************************************/
class MapTile {
public:
	//! Index to a lower layer tile in the MapMode tile_frames vector
	int32 lower_layer;
	// Possibly might have a new layer in the future...
	// int32 middle_layer;
	//! Index to an upper layer tile in the MapMode tile_frames vector
	int32 upper_layer;
	//! A bit-mask indicating various tile properties.
	uint32 properties;
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
	int32 frame;
	//! A pointer to the next frame in the animation.
	TileFrame *next;
}; // class TileFrame

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
	uint32 _map_id;
	//! A stack indicating the various states the map code is in (ie, exploration, dialogue, script).
	std::vector<uint32> _map_state;
	//! A millisecond counter for use in tile animation.
	int32 _animation_counter;
	//! The time elapsed since the last Update() call to MapMode.
	uint32 _time_elapsed;

	//! The number of tile rows in the map.
	uint32 _row_count;
	//! The number of tile columns in the map.
	uint32 _col_count;

	//! True if this map is to have random encounters.
	bool _random_encounters;
	//! The average number of steps the player takes before encountering an enemy.
	uint32 _encounter_rate;
	//! The remaining steps until the player meets their next party of foes.
	uint32 _steps_till_encounter;

	//! A 2D vector that represents the map itself.
	std::vector<std::vector<MapTile> > _map_layers;
	//! A vector of circular singely-linked lists for each tile frame animation.
	std::vector<TileFrame*> _tile_frames;
	//! The normal set of map objects.
	std::vector<ObjectLayer*> _ground_objects;
	//! Objects that can be both walked under and above on (like bridges).
	std::vector<ObjectLayer*> _middle_objects;
	//! Objects that are drawn above everything else.
	std::vector<ObjectLayer*> _sky_objects;
	//! A pointer to the map sprite that the map should focus on.
	MapSprite *_focused_object;
	//! A "virtual sprite" that serves as a camera, available for use in each map.
	MapSprite *_virtual_sprite;

	//! A vector for miscellaneous map images.
	static std::vector<hoa_video::ImageDescriptor> _map_images;
	//! A vector containing the image for each map tile and frame.
	std::vector<hoa_video::ImageDescriptor> _map_tiles;
	//! The music that we would like available on the map.
	std::vector<hoa_audio::MusicDescriptor> _map_music;
	//! The specific sounds that the map needs available.
	std::vector<hoa_audio::SoundDescriptor> _map_sound;

//	 vector<MapEvent> _map_events;
//	std::vector<hoa_global::GEnemy> _map_enemies;

	/*!
	 * \brief Determines whether an object may be placed on a tile.
	 * \param row The row index of the tile to check.
	 * \param col The column index of the tile to check.
	 * \param z_occupied The Z level to check for an occupied object.
	 * \return True if an object may move to the tile, false otherwise.
	 */
	bool _TileMoveable(uint32 row, uint32 col, uint32 z_occupied);
	
	/*!
	 * \brief Determines if an adjacent tile has some sort of interaction
	 * \param row The row index of the tile to check.
	 * \param col The column index of the tile to check.
	 * \param z_occupied The Z level to check.
	 */
	void _CheckTile(uint32 row, uint32 col, uint32 z_occupied);
	/*!
	 * \brief Attempts to move a sprite in the indicated direction.
	 * \param direction The direction that the sprite wishes to move.
	 * \param *sprite A pointer to the sprite itself.
	 */
	void _SpriteMove(uint32 direction, MapSprite *sprite);
	//! Updates the virtual_sprite class member.
	void _UpdateVirtualSprite();

	//! Updates the map when in the explore state.
	void _UpdateExploreState();
	//! Updates the focused player sprite when in the explore state.
	//! \param *player_sprite A pointer to the sprite to update.
	void _UpdatePlayerExplore(MapSprite *player_sprite);
	//! Updates a NPC sprite when in the explore state.
	//! \param *npc A pointer to the sprite to update.
	void _UpdateNPCExplore(MapSprite *npc);

	//! Updates the map when in the dialogue state.
	void _UpdateDialogueState();
	//! Updates the map when in the script state.
	void _UpdateScriptState();
	//! Updates the movement of all map NPCs.
	void _UpdateNPCMovement();

	//! Calculates information about how to draw the next map frame.
	//! \param &mf The refereneced object to put the calculated drawing results into.
	void _GetDrawInfo(private_map::MapFrame& mf);

	// TEMPORARY FUNCTIONS FOR TESTING PURPOSES >>> eventally will be defunct
	void _TempCreateMap();
	void _TempCreateSprites();
public:
	//! The name of the map, as seen by the player in the game.
	std::string mapname;
	MapMode(uint32 new_map_id);
	MapMode(uint32 rows, uint32 cols) { _row_count = rows; _col_count = cols; }
	~MapMode();

	//! \name Map Editor Access functions
	//@{
	//! \brief Used by the map editor for accessing various map information.
	//! \note These functions might go defunct if the map editor becomes independent of the game.
	std::vector<std::vector<MapTile> > GetMapLayers() { return _map_layers; }
	std::vector<hoa_video::ImageDescriptor> GetMapTiles() { return _map_tiles; }
	void SetRows(uint32 num_rows) { _row_count = num_rows; }
	void SetCols(uint32 num_cols) { _col_count = num_cols; }
	void SetMapLayers(std::vector<std::vector<MapTile> > layers) { _map_layers = layers; }
	void SetMapTiles(std::vector<hoa_video::ImageDescriptor> tiles) { _map_tiles = tiles; }
	uint32 GetRows() { return _row_count; }
	uint32 GetCols() { return _col_count; }
	//@}

	//! Updates the game and calls various sub-update functions depending on the state of map mode.
	//! \param new_time_elapsed The amount of milliseconds that have elapsed since the last call to this function.
	void Update(uint32 new_time_elapsed);
	//! Handles the drawing of everything on the map and makes sub-draw function calls as appropriate.
	void Draw();
}; // class MapMode

} // namespace hoa_map;

#endif
