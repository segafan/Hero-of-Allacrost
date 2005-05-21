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
 
/* >>>>> STUFF TO IMPLEMENT (05/13/2005) <<<<<
 * 
 * > Fix drawing error with map objects
 * > 8-layer 'heights' with individual collision detection
 * > MapMode(int rows, int cols) constructor for MapEditor
 * > Create map state stack instead of single map state variable
 * > Ability for map to follow *any* object, not just the player sprite
 * > "Virtual sprite" class for allowing an un-drawable focal point for the map
 *
*/ 
 
#ifndef __MAP_HEADER__
#define __MAP_HEADER__

#include <string>
#include <vector>
#include "defs.h"
#include "engine.h"
#include "utils.h"

namespace hoa_map {

// The local_map namespace is only intended for map.cpp (and possibly some map editor code) to use.
//  It holds constants and data structures that other parts of the program don't need to worry about.
//  *DO NOT* use this namespace unless you know what you are doing!
namespace local_map {

// ************************ MAP CONSTANTS ****************************

// The rate at which tiles are animated, in ms
const int ANIMATION_RATE = 300;

// The number of rows and columns of tiles that compose the screen
const int SCREEN_ROWS = 20;
const int SCREEN_COLS = 28;

// Constants used for describing the current state of operation during MapMode
const int EXPLORE      = 0x00000001;
const int DIALOGUE     = 0x00000002;
const int SCRIPT_EVENT = 0x00000004;

// Each object (sprites, etc.) has a z value that store's it's "height" on the map
const unsigned int Z_LVL1 = 0x00000001;
const unsigned int Z_LVL2 = 0x00000002;
const unsigned int Z_LVL3 = 0x00000004;
const unsigned int Z_LVL4 = 0x00000008;
const unsigned int Z_LVL5 = 0x00000010;
const unsigned int Z_LVL6 = 0x00000020;
const unsigned int Z_LVL7 = 0x00000040;
const unsigned int Z_LVL8 = 0x00000080;
const unsigned int Z_MASK = 0x000000FF;

// *********************** OBJECT CONSTANTS **************************

// Different object idenfitiers for use in the object layer
const int PLAYER_SPRITE  = 0x00000001;
const int NPC_SPRITE     = 0x00000002;
const int ENEMY_SPRITE   = 0x00000004;
const int VIRTUAL_SPRITE  = 0x00000008;
const int STATIC_OBJECT  = 0x00000010;
const int DYNAMIC_OBJECT = 0x00000020;

// ********************** SPRITE CONSTANTS **************************

// Note: The lower 8 bits for sprites use the Z_LEVEL_X constants

// These are common speeds for sprite movement
const float VERY_SLOW_SPEED = 30;
const float SLOW_SPEED      = 25;
const float NORMAL_SPEED    = 20;
const float FAST_SPEED      = 15;
const float VERY_FAST_SPEED = 10;

// A series of constants used for sprite directions. (NORTH_NW = sprite facing north, moving NW)
const unsigned int NORTH    = 0x00000100;
const unsigned int SOUTH    = 0x00000200;
const unsigned int WEST     = 0x00000400;
const unsigned int EAST     = 0x00000800;
const unsigned int NORTH_NW = 0x00001000;
const unsigned int WEST_NW  = 0x00002000;
const unsigned int NORTH_NE = 0x00004000;
const unsigned int EAST_NE  = 0x00008000;
const unsigned int SOUTH_SW = 0x00010000;
const unsigned int WEST_SW  = 0x00020000;
const unsigned int SOUTH_SE = 0x00040000;
const unsigned int EAST_SE  = 0x00080000;

const unsigned int FACE_MASK  = 0x000FFF00; // Used in bit-wise ops to get the sprite direction

// These are used by the MapSprite::SpriteMove() function
const int MOVE_NORTH = 0;
const int MOVE_SOUTH = 1;
const int MOVE_WEST  = 2;
const int MOVE_EAST  = 3;
const int MOVE_NW    = 4;
const int MOVE_SW    = 5;
const int MOVE_NE    = 6;
const int MOVE_SE    = 7;

// A series of constants used for sprite status.
const unsigned int STEP_SWAP   = 0x00100000; // Tracks sprite step frame (right or left foot step)
const unsigned int IN_MOTION   = 0x00200000; // This is for detecting whether the sprite is currently moving
const unsigned int UPDATEABLE  = 0x00400000; // If this bit is set to zero, we do not update the sprite
const unsigned int VISIBLE     = 0x00800000; // If this bit is set to zero, we do not draw the sprite
const unsigned int CONTROLABLE = 0x01000000; // If this bit is set, the sprite can be controlled by the player

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

// Sets the z level of the object that steps on it
const unsigned int SET_Z_LVL1 = 0x00000100;
const unsigned int SET_Z_LVL2 = 0x00000200;
const unsigned int SET_Z_LVL3 = 0x00000400;
const unsigned int SET_Z_LVL4 = 0x00000800;
const unsigned int SET_Z_LVL5 = 0x00001000;
const unsigned int SET_Z_LVL6 = 0x00002000; 
const unsigned int SET_Z_LVL7 = 0x00004000;
const unsigned int SET_Z_LVL8 = 0x00008000;

const unsigned int SET_Z_MASK = 0x0000FF00;

const unsigned int INC_Z_LEVEL   = 0x00010000; // Increments the z level of the object on this tile
const unsigned int DEC_Z_LEVEL   = 0x00020000; // Decrements the z level of the object on this tile
const unsigned int TREASURE      = 0x00040000; // A map treasure is located at this tile
const unsigned int EVENT         = 0x00080000; // An event occurs when player steps onto this tile



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
		unsigned int properties: a bit-wise mask indicating various tile properties
******************************************************************************/
class MapTile {
public:
	int lower_layer;
	int upper_layer;
	unsigned int properties;
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
		unsigned int object_type: an identifier type for the object
		int row_pos: the map row position for the bottom left corner of the object
		int col_pos: the map col position for the bottom left corner of the object
		unsigned int status: a bit-mask for setting and detecting various conditions on the sprite
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
	unsigned char object_type;
	int row_pos;
	int col_pos;
	hoa_video::GameVideo *VideoManager;
	
	friend class MapMode; // Necessary so that the MapMode class can access and change these data members
public:
	ObjectLayer(unsigned int type, int row, int col);
	~ObjectLayer();
	virtual void Draw(local_map::MapFrame& mf) = 0;
};



/******************************************************************************
	MobileObject - A moveable, controllable object on the map
		
	>>>members<<<
		float step_speed: determines how fast the object can move around the map
		float step_count: a counter used for tracking movement
		unsigned int status: bit-mask containing various state information about the object
	
	>>>functions<<<
	
	>>>notes<<<
		1) This class can be used to create an invisible "camera" that the map focuses on.
		
		2) A single, unique object of this class is created for every map, regardless of 
			whether or not it is used.
 *****************************************************************************/
class MobileObject : public ObjectLayer {
private:
	friend class MapMode;
	
protected:
	float step_speed;
	float step_count;
	unsigned int status;
public:
	MobileObject(float ss, unsigned int type, int row, int col);
	~MobileObject();
	void Draw(local_map::MapFrame& mf) {}
};


/******************************************************************************
	MapSprite - Abstract class for all character, NPC, enemy, and other sprites
		
	>>>members<<<
		vector <> frames: holds ImageDescriptors for all the sprite frame images
	
	>>>functions<<<
		int FindFrame():
			Called by the Draw() function to figure out the sprite frame that should be drawn
		void Draw(local_map::MapFrame& mf):
			A function for determining if a sprite needs to be drawn, and if so, drawing the sprite
	
	>>>notes<<<
		1) TO DO: Add functionality in Draw() to determine if a sprite should be drawn or not

 *****************************************************************************/
class MapSprite : public MobileObject {
private:
	int FindFrame();
	friend class MapMode;
protected:
	std::vector<hoa_video::ImageDescriptor> frames;
	
	void Draw(local_map::MapFrame& mf);
public:
	MapSprite(unsigned int stat, float ss, unsigned int type, int row, int col);
	~MapSprite();
};

 
 
 
/******************************************************************************
	PlayerSprite - Manages the user-controllable player sprite on the map.
 
	>>>members<<<

	>>>functions<<<
 
	>>>notes<<<
						
 *****************************************************************************/
class PlayerSprite : public MapSprite {
	friend class MapMode;
public:
	PlayerSprite(unsigned int stat, float ss, unsigned int type, int row, int col);
	~PlayerSprite();
};



/******************************************************************************
	NPCSprite Class
		This class is for managing the images needed to display and animate sprites on maps
 
	>>>members<<<
		int wait_time: the number of milliseconds to wait till the next tile transition
		int delay_time: the average time to wait between tile transitions
	
	>>>functions<<<
		void DelayedMovement(int average_time): 
			Sets the mean time between sprite tile transitions (zero is constant movement)
 
	>>>notes<<<
						
 *****************************************************************************/
class NPCSprite : public MapSprite {
private:
	friend class MapMode;
	//vector<bool, std::string> sprite_dialogue; LATER
	int wait_time;
	int delay_time;
	
public:
	NPCSprite(std::string name, int d_time, unsigned int stat, float ss, unsigned int type, int row, int col);
	~NPCSprite();
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
//	 friend class MapMode;
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
//	 friend class MapMode;
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
	 vector<unsigned int> map_state: indicates what state the map is in (explore, dialogue, script, etc)
	 int animation_counter: millisecond counter for use in tile animation
	 
	 int tile_count: the number of tile images used in the map, not counting individual animation tiles
	 int row_count: the number of rows comprising the map
	 int col_count: the number of columns comprising the map
			
	 bool random_encounters: a boolean indicating whether the player may encounter random foes
	 int encouter_rate: the mean number of steps a player takes until they encounter a random foe
	 int steps_till_encounter: the number of steps remaining until the next encounter
	 
	 MobileObject *focused_object: the object that the map camera is focused on
	 MobileObject *virtual_sprite: serves as an invisible focal point for the camera
 
	>>>functions<<< 
	bool TileMoveable(int row, int col):
		Determines if the tile given by the row arguments is free to move to
	void MoveSprite(MobileObject *sprite, int direction):
		Will move a sprite to a new tile and, if successful, change all necessary sprite and tile properties
	void UpdateVirtualSprite():
		Called if the focused object is a virtual sprite. Handles user input and virtual sprite movement
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
	std::vector<unsigned int> map_state;
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
	std::vector<ObjectLayer*> object_layer;
	MobileObject *focused_object;
	MobileObject *virtual_sprite;
	
	std::vector<hoa_video::ImageDescriptor> map_tiles;	
	std::vector<hoa_audio::MusicDescriptor> map_music;
	std::vector<hoa_audio::SoundDescriptor> map_sound;
	
//	 vector<MapEvent> map_events;
//	 vector<Enemy> map_enemies;
	
	bool TileMoveable(int row, int col, unsigned int z_occupied);
	void SpriteMove(int direction, MobileObject *sprite);
	void UpdateVirtualSprite();
	
	void UpdateExploreState();
	void UpdatePlayerExplore(PlayerSprite *player_sprite);
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
	MapMode(int rows, int cols) { row_count = rows; col_count = cols; }
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
