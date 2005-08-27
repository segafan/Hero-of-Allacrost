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

//! The rate at which tiles are animated, in milliseconds.
//! \note This will become defunct later when the video engine handles the animation.
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

//! \name Altitude Constants
//@{
//! \brief Constants for accessing the nine different alititude levels of both tiles and objects.
const uint8 ALTITUDE_0 = 0x00;
const uint8 ALTITUDE_1 = 0x01;
const uint8 ALTITUDE_2 = 0x02;
const uint8 ALTITUDE_3 = 0x04;
const uint8 ALTITUDE_4 = 0x08;
const uint8 ALTITUDE_5 = 0x10;
const uint8 ALTITUDE_6 = 0x20;
const uint8 ALTITUDE_7 = 0x40;
const uint8 ALTITUDE_8 = 0x80;
//@}

// ********************** TILE CONSTANTS **************************

//! Indicates that an event will take place when the player steps onto this tile.
const uint8 ARRIVE_EVENT  = 0x01;
//! Indicates that an event will take place when the player steps off of this tile.
const uint8 DEPART_EVENT  = 0x02;
//! Indicates that an event will take place when the player faces this tile and presses "confirm".
const uint8 CONFIRM_EVENT = 0x04;
//! Indicates a treasure is contained on this tile.
const uint8 TREASURE      = 0x08;

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
	//! Index to a lower layer tile in the MapMode tile_frames vector.
	int32 lower_layer;
	//! Index to a middle layer tile in the MapMode tile_frames vector.
	int32 middle_layer;
	//! Index to an upper layer tile in the MapMode tile_frames vector.
	int32 upper_layer;
	//! A bit-mask for indicating whether a tile is walkable on each altitude level.
	uint8 not_walkable;
	//! A bit-mask for indicating that a tile is occupied by an object.
	uint8 occupied;
	//! A bit-mask indicating various tile properties.
	uint8 properties;
}; // class MapTile


/*!****************************************************************************
 * \brief Element of a circular singlely linked list for tile frame animations.
 *
 * \note 0) This class will become defunct later, when the video engine is 
 * capable of handling all the animation.
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
	//! Retains information needed to draw the next map frame.
	private_map::MapFrame _map_info;
	//! The dialogue menu used by map mode
	hoa_video::ImageDescriptor _dialogue_menu;
	//! Contains the string of dialogue text to draw on the string when code is in the dialogue state.
	std::string _dialogue_text;
	//! When a dialogue takes place, this vector contains pointers to all of the individual speakers.
	std::vector<MapSprite*> _dialogue_speakers;
	
	/*!
	 * \brief Determines whether an object may be placed on a tile.
	 * \param row The row index of the tile to check.
	 * \param col The column index of the tile to check.
	 * \param altitude_level The altitude level of the tile to check.
	 * \return True if an object may move to the tile, false otherwise.
	 */
	bool _TileMoveable(int32 row, int32 col, uint8 altitude_level);
	/*!
	 * \brief Determines if an adjacent tile has some sort of interaction.
	 * \param row The row index of the tile to check.
	 * \param col The column index of the tile to check.
	 * \param altitude_level The altitude level of the tile to check.
	 * 
	 * An interaction may be either an event bound to the tile or another
	 * map object/sprite occupying that tile.
	 */
	void _CheckInteraction(int32 row, int32 col, uint8 altitude_level);
	/*!
	 * \brief Attempts to move a sprite in the indicated direction.
	 * \param direction The direction that the sprite wishes to move.
	 * \param *sprite A pointer to the sprite itself.
	 *
	 * This function is only called for sprites that are located in the
	 * ground object layer. Sprites are typically never in the middle 
	 * object layer, and sprites in the sky object layer don't need
	 * collision detection. This function is also never called with a
	 * virtual sprite object.
	 */
	void _GroundSpriteMove(uint32 direction, MapSprite *sprite);
	
	//! Updates the focused player sprite and processes user input.
	//! \param *player_sprite A pointer to the sprite to update.
	void _UpdatePlayer(MapSprite *player_sprite);
	//! Updates a NPC sprite.
	//! \param *npc A pointer to the sprite to update.
	void _UpdateNPC(MapSprite *npc);
	/*! 
	 *  Updates the MapMode#_virtual_sprite class member.
	 *
	 *  This function is only called when MapMode#_focused_object is MapMode#_virtual_sprite.
	 */
	void _UpdateVirtualSprite();

	//! Updates the map when in the explore state.
	void _UpdateExploreState();
	//! Updates the map when in the dialogue state.
	void _UpdateDialogueState();
	//! Updates the map when in the script state.
	void _UpdateScriptState();
	//! Updates the movement of all map NPCs.
	void _UpdateNPCMovement();

	//! Calculates information about how to draw the next map frame.
	void _GetDrawInfo();

	// TEMPORARY FUNCTIONS FOR TESTING PURPOSES >>> eventally will be defunct
	void _TEMP_CreateMap();
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

	//! Resets appropriate class members. Called whenever MapMode is made the active game mode.
	void Reset();
	//! Updates the game and calls various sub-update functions depending on the state of map mode.
	//! \param new_time_elapsed The amount of milliseconds that have elapsed since the last call to this function.
	void Update(uint32 new_time_elapsed);
	//! Handles the drawing of everything on the map and makes sub-draw function calls as appropriate.
	void Draw();
}; // class MapMode

} // namespace hoa_map;

#endif
