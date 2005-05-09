/* 
 * map.h
 *	Header file for Hero of Allacrost map mode
 *	(C) 2004 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */

/*
 * The code in this file is for handling the maps. This code is used whenever the player is walking around on
 *	a map (like a town or a dungeon). This includes handling tile images, sprite images, and events that
 *	occur on the map.
 *
 */
 
#ifndef __MAP_HEADER__
#define __MAP_HEADER__

#include <string>
#include <vector>
#include <list>
#include "defs.h"
#include "utils.h"
#include "engine.h"

namespace hoa_map {

// The local_map namespace is only intended for map.cpp (and possibly some map editor code) to use.
//  It holds constants and data structures that other parts of the program don't need to worry about.
//  *DO NOT* use this namespace unless you know what you are doing!
namespace local_map {

// ************************ MAP CONSTANTS ****************************

// These are elapsed time dividers used for sprite movement
const int VERY_SLOW_SPEED = 20;
const int SLOW_SPEED      = 16;
const int NORMAL_SPEED    = 12;
const int FAST_SPEED      = 8;
const int VERY_FAST_SPEED = 4;

// How many 'steps' a sprite has to take to move to the next tile
const int TILE_STEPS = 32;

// The rate at which tiles animated, in ms
const int ANIMATION_RATE = 300;

// The number of rows and columns of tiles that compose the screen
const int SCREEN_ROWS = 20;
const int SCREEN_COLS = 28;

// Constants used for describing the current state of operation during MapMode
const int EXPLORE      = 0x00000001;
const int DIALOGUE     = 0x00000002;
const int SCRIPT_EVENT = 0x00000004;

// *********************** OBJECT CONSTANTS **************************

// Different object idenfitiers for use in the object layer
const int PLAYER_SPRITE  = 1;
const int NPC_SPRITE     = 2;
const int ENEMY_SPRITE   = 3;
const int STATIC_OBJECT  = 4;
const int DYNAMIC_OBJECT = 5;

// Each object (including sprites) has a "z" value to deterime it's height.
//  The default object height is 0x8 and can range between 0x0 and 0xF (a range of 16 values)
const int DEFAULT_Z = 0x000000008;

// ********************** SPRITE CONSTANTS **************************

// A series of constants used for sprite status.
const int STEP_SWAP  = 0x00000001; // Tracks sprite step frame (right or left foot step)
const int IN_MOTION  = 0x00000002; // This is for detecting whether the sprite is currently moving
const int UPDATEABLE = 0x00000004; // If this bit is set to zero, we do not update the sprite
const int VISIBLE    = 0x00000008; // If this bit is set to zero, we do not draw the sprite

// A series of constants used for sprite directions. (NORTH_NW = sprite facing north, moving NW)
const int NORTH    = 0x00000010;
const int SOUTH    = 0x00000020;
const int WEST     = 0x00000040;
const int EAST     = 0x00000080;
const int NORTH_NW = 0x00000100;
const int WEST_NW  = 0x00000200;
const int NORTH_NE = 0x00000400;
const int EAST_NE  = 0x00000800;
const int SOUTH_SW = 0x00001000;
const int WEST_SW  = 0x00002000;
const int SOUTH_SE = 0x00004000;
const int EAST_SE  = 0x00008000;

const int FACE_MASK  = 0x0000FFF0; // Used in bit-wise ops to get the sprite direction
const int RESET_FACE = 0xFFFF000F; // Used to reset the direction the sprite is facing

// These constants are used for indexing a standard sprite animation frame vector
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

// ********************** TILE CONSTANTS **************************

// Constants used for detecting different tile properties
const int INC_HEIGHT   = 0x00000001; // Increments the height of any object that steps on it
const int DEC_HEIGHT   = 0x00000002; // Decrements the height of any object that steps on it
const int NOT_WALKABLE = 0x00000004; // Duh.
const int TREASURE     = 0x00000008; // A map treasure is located at this tile
const int EVENT        = 0x00000010; // An event occurs when player steps onto this tile
const int OCCUPIED     = 0x00000020; // Occupied by a sprite or other map object

// The next two features are in debate. They will incur greater overhead, but provide more functionality.
//const int LL_HIDDEN    = 0x00000040; // Don't draw the lower layer for this tile
//const int UL_HIDDEN    = 0x00000080; // Don't draw the upper layer for this tile



/******************************************************************************
	MapFrame class - contains info about how the current map frame is being drawn. 
		Used by map objects to determine where they should be drawn.
	
	>>>members<<<
		int c_start, r_start: The starting index of the tile column/row we need to draw
		float c_pos, r_pos:   Coordinates for setting the drawing cursor
		int c_draw, r_draw:   The number of columns and rows of tiles to draw
******************************************************************************/
class MapFrame {
public:
	int c_start, r_start;
	float c_pos, r_pos;
	int c_draw, r_draw;
};

} // namespace local_map



/******************************************************************************
	MapTile class - fills a 2D vector composing the map
	
	>>>members<<<
		int lower_layer: index to a lower layer tile in the MapMode tile_frames vector
		int upper_layer: index to an upper layer tile in the MapMode tile_frames vector
		unsigned int event_mask: a bit-wise mask indicating various tile properties
******************************************************************************/
class MapTile {
public:
	int lower_layer;
	int upper_layer;
	unsigned int event_mask;
};



/******************************************************************************
	TileFrame class - element of a circular singlely linked list for tile frame animations
	
	>>>members<<<
		int frame: holds a frame index pointing to map_tiles in MapMode class
		Tileframe *next: a pointer to the next frame
 *****************************************************************************/
class TileFrame {
public:
	int frame;
	TileFrame *next;
};



/******************************************************************************
	ObjectLayer - Abstract class for all map objects
		
	>>>members<<<
		int object_type: an identifier type for the object
		int row_pos: the map row position for the bottom left corner of the object
		int col_pos: the map col position for the bottom left corner of the object
		char ver_pos: determines the "vertical" position of an object on a map
	>>>functions<<<
		virtual void Draw(local_map::MapFrame& mf):
			A purely virtual function for drawing the object, if it even needs to be drawn
 
	>>>notes<<<
		1) An object's "vertical" position is used so that objects can be drawn over each
			other if necessary. For example, there may be a bridge that we want to be able
			to walk on or under. Range is from -128 to 127. If an object has a height >= 100,
			then the object is drawn *after* the upper layer.
 *****************************************************************************/
class ObjectLayer {
protected:
	int object_type;
	int row_pos;
	int col_pos;
	char ver_pos;
	hoa_video::GameVideo *VideoManager;
	
	friend class MapMode; // Necessary so that the MapMode class can access and change these data members
public:
	ObjectLayer();
	~ObjectLayer() {}
	
	bool operator>(const ObjectLayer& obj) const;
	bool operator<(const ObjectLayer& obj) const;
	bool operator>=(const ObjectLayer& obj) const;
	bool operator<=(const ObjectLayer& obj) const;
	bool operator==(const ObjectLayer& obj) const;
	bool operator!=(const ObjectLayer& obj) const;
	
	virtual void Draw(local_map::MapFrame& mf) = 0;
};



/******************************************************************************
	MapSprite - Abstract class for all character, NPC, enemy, and other sprites
		
	>>>members<<<
		float step_speed: determines how fast the sprite transitions between tiles
		float step_count: a counter used for tracking movement
		unsigned int status: a bit-mask for setting and detecting various conditions on the sprite
	
	>>>functions<<<
		void Draw(local_map::MapFrame& mf):
			A function for determining if a sprite needs to be drawn, and if so, drawing the sprite
		int FindFrame():
			Called by the Draw() function to figure out the sprite frame that should be drawn
	
	>>>notes<<<
		1) TO DO: Add functionality in Draw() to determine if a sprite should be drawn or not
 *****************************************************************************/
class MapSprite : public ObjectLayer {
protected:
	float step_speed;
	float step_count;
	unsigned int status;
	std::vector<hoa_video::ImageDescriptor> frames;
	
	void Draw(local_map::MapFrame& mf);
	friend class MapMode; // Necessary so that the MapMode class can access and change these data members
private:
	int FindFrame();
};



/******************************************************************************
	PlayerSprite - Manages the user-controllable player sprite on the map.
 
	>>>members<<<

	>>>functions<<<
 
	>>>notes<<<
						
 *****************************************************************************/
class PlayerSprite : public MapSprite {
	friend class MapMode; // Necessary so that the MapMode class can access and change these data members
public:
	PlayerSprite();
	~PlayerSprite();
};



/******************************************************************************
	NPCSprite Class
		This class is for managing the images needed to display and animate sprites on maps.
 
	>>>members<<<
		int wait_time: the number of milliseconds to wait till the next tile transition
		int delay_time: the average time to wait between tile transitions
	
	>>>functions<<<		 
 
	>>>notes<<<
						
 *****************************************************************************/
class NPCSprite : public MapSprite {
	friend class MapMode; // Necessary so that the MapMode class can access and change these data members
	//vector<bool, std::string> sprite_dialogue; LATER
	int wait_time;
	int delay_time;
public:
	NPCSprite(std::string name);
	~NPCSprite();
	void ConstantMovement() { delay_time = 0; }
	void DelayedMovement(int average_time) { delay_time = average_time; }
};



/******************************************************************************
	EnemySprite - for managing enemy sprites visible on the map
 
	>>>members<<<
	 
	>>>functions<<<
 
	>>>notes<<<
	
 *****************************************************************************/
// class EnemySprite : public MapSprite {
// };



/******************************************************************************
	StaticObject - map objects which are inanimate (1 frame)

	>>>members<<<
 
	>>>functions<<<		 
 
	>>>notes<<<
	
 *****************************************************************************/
// class StaticObject : public ObjectLayer {
//	 ImageDescriptor frame;
//	 
//	 friend class MapMode; // Necessary so that the MapMode class can access and change these data members
// public:
//	 StaticObject();
//	 ~StaticObject();
// 
// };



/******************************************************************************
	DynamicObject - map objects which are animate (multiple frames)

	>>>members<<<
 
	>>>functions<<<		 
 
	>>>notes<<<
						
 *****************************************************************************/
// class DynamicObject : public ObjectLayer {
//	 std::vector<ImageDescriptor> frames;
//	 
//	 friend class MapMode; // Necessary so that the MapMode class can access and change these data members
// public:
//	 DynamicObject();
//	 ~DynamicObject();
// 
// };






/******************************************************************************
	MapMode Class
 
	>>>members<<<
	 int map_id: a unique ID number for each map object
	 string mapname: the name of the map, also the location name in menu mode
	 unsigned int map_state: indicates what state the map is in (explore, dialogue, script, etc)
	 int animation_counter: millisecond counter for use in tile animation
	 
	 int tile_count: the number of tile images used in the map, not counting individual animation tiles
	 int row_count: the number of rows comprising the map
	 int col_count: the number of columns comprising the map
			
	 bool random_encounters: a boolean indicating whether the player may encounter random foes
	 int encouter_rate: the mean number of steps a player takes until they encounter a random foe
	 int steps_till_encounter: the number of steps remaining until the next encounter
 
	>>>functions<<< 
	bool TileMoveable(int row, int col):
		Determines if the tile given by the row arguments is free to move to
	void UpdateExploreState():
		When the map is in the explore state, this function is called to update the game
	void UpdateDialogueState():
		When the map is in the dialogue state, this function is called to update the game
	void UpdateScriptState():
		When the map is in the script state, this function is called to update the game
	void UpdateNPCMovement(Uint32 time_elapsed):
		Updates the position of the NPCs on the map
	void GetDrawInfo(local_map::MapFrame& mf):
		Constructs a MapFrame struct which contains details about how to draw the current map frame
	
	void Update(Uint32 new_time_elapsed):
		Called in the main game loop to update the game status
	void Draw():
		Called in the maing ame loop to draw the next frame
	
	>>>notes<<<
		1) If you change the state of random_encounters from false to true, make sure to set 
			a valid value (< 0) for steps_till_encounter. *I might change this later*
			
		2) Be careful with calling the MapMode constructor, for it changes the coordinate system of the 
			video engine without warning. Only create a new instance of this class if you plan to immediately
			push it on top of the game stack.
			
		3) Any function that starts with "Temp" is temporary and will be removed evenutally.
 *****************************************************************************/
class MapMode : public hoa_engine::GameMode {
private:
	friend class hoa_data::GameData;
	
	int map_id;
	unsigned int map_state;
	int animation_counter;
	Uint32 time_elapsed;
	
	int tile_count;	
	int row_count;
	int col_count;
	
	bool random_encounters;
	int encounter_rate;
	int steps_till_encounter;
	
	std::vector<std::vector<MapTile> > map_layers;
	std::vector<TileFrame*> tile_frames;
	std::list<ObjectLayer*> object_layer;
	PlayerSprite *player_sprite;
	
	std::vector<hoa_video::ImageDescriptor> map_tiles;	
	std::vector<hoa_audio::MusicDescriptor> map_music;
	std::vector<hoa_audio::SoundDescriptor> map_sound;
	
//	 vector<MapEvent> map_events;
//	 vector<Enemy> map_enemies;
//	 vector<EnemyGroup> m_enemy_groups;
	
	bool TileMoveable(int row, int col);
	
	void UpdateExploreState();
	void UpdatePlayerExplore();
	void UpdateNPCExplore(NPCSprite *npc);
	
	void UpdateDialogueState();
	void UpdateScriptState();
	void UpdateNPCMovement();
	
	void GetDrawInfo(local_map::MapFrame& mf);
	
	void TempCreateMap();
	void TempCreateSprites();
public: 
	std::string mapname;
	MapMode(int new_map_id);
	~MapMode();
	
	std::vector<std::vector<MapTile> > GetMapLayers();
	std::vector<hoa_video::ImageDescriptor> GetMapTiles();
	void SetTiles(int num_tiles);
	void SetRows(int num_rows);
	void SetCols(int num_cols);
	void SetMapLayers(std::vector<std::vector<MapTile> > layers);
	void SetMapTiles(std::vector<hoa_video::ImageDescriptor> tiles);
	int GetTiles();
	int GetRows();
	int GetCols();
	
	void Update(Uint32 new_time_elapsed);
	void Draw();
};

} // namespace hoa_map;

#endif
