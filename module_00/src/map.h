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
#include "gui.h"

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
//! \brief The number of rows and columns of tiles that compose the screen.
//@{
const float SCREEN_ROWS = 24.0f;
const float SCREEN_COLS = 32.0f;
//@}

//! \name Map State Constants
//! \brief Constants used for describing the current state of operation during map mode.
//@{
const uint8 EXPLORE      = 0x00000001;
const uint8 DIALOGUE     = 0x00000002;
const uint8 SCRIPT_EVENT = 0x00000004;
//@}

//! \name Altitude Constants
//! \brief Constants for accessing the eight different alititude levels of both tiles and objects.
//! \note Higher numbers correspond to higher altitudes.
//@{
const uint8 ALTITUDE_1 = 0x01;
const uint8 ALTITUDE_2 = 0x02;
const uint8 ALTITUDE_3 = 0x04;
const uint8 ALTITUDE_4 = 0x08;
const uint8 ALTITUDE_5 = 0x10;
const uint8 ALTITUDE_6 = 0x20;
const uint8 ALTITUDE_7 = 0x40;
const uint8 ALTITUDE_8 = 0x80;
//@}

//! \name Interaction Type Constants
//! \brief Types of interactions that can occur on a specific tile
//@{
const uint32 NO_INTERACTION     = 0;
const uint32 SPRITE_INTERACTION = 1;
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
	int16 c_start;
	//! The starting index of the tile row to draw.
	int16 r_start;
	//! Column coordinate for setting the drawing cursor.
	float c_pos;
	//! Row coordinate for setting the drawing cursor.
	float r_pos;
	//! The number of columns of tiles that need to be drawn.
	uint8 c_draw;
	//! The number of rows of tiles that need to be drawn.
	uint8 r_draw;
}; // class MapFrame

/*!****************************************************************************
 * \brief A container class for storing information when a tile needs to be examined.
 *
 * This class is used in the MapMode#_TileMoveable() and MapMode#_CheckInteraction() 
 * functions to do appropriate tile and object interaction checking.
 *****************************************************************************/
class TileCheck {
public:
	//! The row index of the tile to check.
	int16 row;
	//! The column index of the tile to check.
	int16 col;
	//! The altitude level of the tile to check.
	uint8 altitude;
	//! The direction of the movement (this may go defunct). 
	uint16 direction;
}; // class TileCheck

/*!****************************************************************************
 * \brief A container class for node information in pathfinding
 *
 * This class is used in the MapMode#_FindPath() to find an optimal path from
 * a source node to a destination node.
 *****************************************************************************/
class TileNode {
public:
	//! The row index of the node tile.
	int16 row;
	//! The column index of the node tile
	int16 col;
	//! The total score for this node.
	int16 f_score;
	//! The score for this node relative to the source.
	int16 g_score;
	//! The estimated score for this node relative to the destination.
	int16 h_score;
}; // class TileNode

} // namespace private_map


/*!****************************************************************************
 * \brief Class structure for representing the 2D vector that composes the map.
 *****************************************************************************/
class MapTile {
public:
	//! Index to a lower layer tile in the MapMode tile_frames vector.
	int16 lower_layer;
	//! Index to a middle layer tile in the MapMode tile_frames vector.
	int16 middle_layer;
	//! Index to an upper layer tile in the MapMode tile_frames vector.
	int16 upper_layer;
	//! A bit-mask for indicating whether a tile is walkable on each altitude level.
	uint8 walkable;
	//! A bit-mask for indicating that a tile is occupied by an object.
	uint8 occupied;
	//! A bit-mask indicating various tile properties.
	uint8 properties;
	//! The map event (if any) registered to this tile.
	int16 event;
	
	MapTile() 
		{ lower_layer = 0; middle_layer = 0; upper_layer = 0; 
		  walkable = 0; occupied = 0; properties = 0; event = 0; }
}; // class MapTile

/*!****************************************************************************
 * \brief Handles everything that needs to be done when the player is exploring maps.
 *
 * The code in this class and its respective partner classes is arguably one of the
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
public:
	MapMode();
	~MapMode();
	
	//! Resets appropriate class members. Called whenever MapMode is made the active game mode.
	void Reset();
	//! Updates the game and calls various sub-update functions depending on the state of map mode.
	//! \param new_time_elapsed The amount of milliseconds that have elapsed since the last call to this function.
	void Update(uint32 new_time_elapsed);
	//! Handles the drawing of everything on the map and makes sub-draw function calls as appropriate.
	void Draw();
	
	//! Fills in all the map structures from a Lua data file.
	void _LoadMap();
private:
	friend class MapSprite;
	
	//! The name of the map, as will be read by the player in-game.
	hoa_utils::ustring _map_name;
	//! Indicates a special conditions that the map is in (e.g. a dialogue is taking place)
	uint8 _map_state;
	//! The time elapsed since the last Update() call to MapMode.
	uint32 _time_elapsed;
	//! The number of tile rows in the map.
	uint16 _row_count;
	//! The number of tile columns in the map.
	uint16 _col_count;
	//! True if this map is to have random encounters.
	bool _random_encounters;
	//! The average number of steps the player takes before encountering an enemy.
	uint32 _encounter_rate;
	//! The remaining steps until the player meets their next party of foes.
	uint32 _steps_till_encounter;

	//! A 2D vector that represents the map itself.
	std::vector<std::vector<MapTile> > _tile_layers;
	//! The normal set of map objects.
	std::vector<MapObject*> _ground_objects;
	//! Objects that can be both walked under and above on (like bridges).
	std::vector<MapObject*> _middle_objects;
	//! Objects that are drawn above everything else.
	std::vector<MapObject*> _sky_objects;
	//! A pointer to the map sprite that the map should focus on.
	MapSprite *_focused_object;
	//! A "virtual sprite" that serves as a camera, available for use in each map.
	MapSprite *_virtual_sprite;
	
	//! Retains information needed to draw the next map frame.
	private_map::MapFrame _draw_info;
	
	//! A vector containing the image for each map tile and frame.
	std::vector<hoa_video::StaticImage> _map_tiles;
	//! The music that we would like available on the map.
	std::vector<hoa_audio::MusicDescriptor> _map_music;
	//! The specific sounds that the map needs available.
	std::vector<hoa_audio::SoundDescriptor> _map_sound;
	
	//! The window for sprite dialogues.
	hoa_video::MenuWindow _dialogue_window;
	//! The textbox for sprite dialogues.
	hoa_video::TextBox _dialogue_textbox;
	//! A vector of all the NPC map sprites that participate in this dialogue
	std::vector<MapSprite*> _dialogue_speakers;
	//! The dialogue menu used by map mode.
	hoa_video::StaticImage _dialogue_menu;
	//! A pointer to the lines of the current dialogue.
	std::vector<std::string> *_dialogue_text;
	//! The index to the current line of dialogue.
	uint32 _dialogue_line;
	
//	 vector<MapEvent> _map_events;
//	std::vector<hoa_global::GEnemy> _map_enemies;
	
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

	
	/*!
	 * \brief Determines whether an object may be placed on a tile.
	 * \param row The row index of the tile to check.
	 * \param col The column index of the tile to check.
	 * \param altitude_level The altitude level of the tile to check.
	 * \return True if an object may move to the tile, false otherwise.
	 */
	bool _TileMoveable(const private_map::TileCheck& tcheck);
	/*!
	 * \brief Determines if an adjacent tile has some sort of interaction.
	 * \param &tcheck Contains information about the tile row, column and altitude to check.
	 * \return A constant that indicates what type of interaction is found on the tile.
	 * 
	 * An interaction may be either an event bound to the tile or another
	 * map object/sprite occupying that tile.
	 */
	uint32 _CheckInteraction(const private_map::TileCheck& tcheck);
	/*!
	 * \brief Determine which object is occuping a given tile
	 * \param &tcheck Contains information about the occupied tile in question.
	 * \return A pointer to the map object in question. Returns null if no occupant is found.
	 */
	MapObject* _FindTileOccupant(const private_map::TileCheck& tcheck);
	/*!
	 * \brief Uses the A* algorithm to find a path from a source to a destination.
	 * \param *sprite A pointer to the sprite who will use the path (also contains the source location).
	 * \param &tdest The destination tile information, including row, column, and altitude information.
	 */
	void _FindPath(const MapSprite* sprite, const private_map::TileNode& destination);
}; // class MapMode

} // namespace hoa_map;

#endif
