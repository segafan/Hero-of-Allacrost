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

#include "utils.h"
#include <string>
#include <vector>
#include "defs.h"
#include "engine.h"
#include "video.h"

namespace hoa_map {

extern bool MAP_DEBUG;

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
const uint Z_LVL1 = 0x00000001;
const uint Z_LVL2 = 0x00000002;
const uint Z_LVL3 = 0x00000004;
const uint Z_LVL4 = 0x00000008;
const uint Z_LVL5 = 0x00000010;
const uint Z_LVL6 = 0x00000020;
const uint Z_LVL7 = 0x00000040;
const uint Z_LVL8 = 0x00000080;
const uint Z_MASK = 0x000000FF;

// ********************** TILE CONSTANTS **************************

// Note: The lower 8 bits of the properties member for tiles use the Z_LVL_X constants

// Sets the z level of the object that steps on it
const uint SET_Z_LVL1  = 0x00000100;
const uint SET_Z_LVL2  = 0x00000200;
const uint SET_Z_LVL3  = 0x00000400;
const uint SET_Z_LVL4  = 0x00000800;
const uint SET_Z_LVL5  = 0x00001000;
const uint SET_Z_LVL6  = 0x00002000; 
const uint SET_Z_LVL7  = 0x00004000;
const uint SET_Z_LVL8  = 0x00008000;
const uint SET_Z_MASK  = 0x0000FF00;

const uint INC_Z_LEVEL = 0x00010000; // Increments the z level of the object on this tile
const uint DEC_Z_LEVEL = 0x00020000; // Decrements the z level of the object on this tile
const uint TREASURE    = 0x00040000; // A map treasure is located at this tile
const uint EVENT       = 0x00080000; // An event occurs when player steps onto this tile

// *********************** OBJECT CONSTANTS **************************

// Different object idenfitiers for use in the object layer
const unsigned char VIRTUAL_SPRITE = 0x00; // Sprites with no physical image
const unsigned char PLAYER_SPRITE  = 0x01; // Playable character sprites
const unsigned char NPC_SPRITE     = 0x02; // Regular NPC sprites
const unsigned char ADV_SPRITE     = 0x04; // 'Advanced' NPC sprites with more emotion frames
const unsigned char OTHER_SPRITE   = 0x08; // Sprites of non-standard sizes (small animals, etc)
const unsigned char ENEMY_SPRITE   = 0x10; // Enemy sprites, various sizes
const unsigned char STATIC_OBJECT  = 0x20; // A still, non-animate object
const unsigned char DYNAMIC_OBJECT = 0x40; // A still and animate object
const unsigned char MIDDLE_OBJECT  = 0x80; // A "middle-layer" object

// ********************** SPRITE CONSTANTS **************************

// Note: The lower 8 bits of the status member for sprites use the Z_LVL_X constants

const uint SPRITE_STD_FRAME_COUNT = 24;

// These are common speeds for sprite movement
const float VERY_SLOW_SPEED = 25.0;
const float SLOW_SPEED      = 22.5;
const float NORMAL_SPEED    = 20.0;
const float FAST_SPEED      = 17.5;
const float VERY_FAST_SPEED = 15.0;

// These are common delay times inbetween movements (numbers are in terms of milliseconds)
const uint VERY_LONG_DELAY  = 500;
const uint LONG_DELAY       = 400;
const uint NORMAL_DELAY     = 300;
const uint SHORT_DELAY      = 200;
const uint VERY_SHORT_DELAY = 100;
const uint NO_DELAY         = 0;

// A series of constants used for sprite directions. (NORTH_NW = sprite facing north, moving NW)
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
const uint FACE_MASK = 0x000FFF00; // Used in bit-wise ops to get the sprite direction

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
const uint STEP_SWAP   = 0x00100000; // Tracks sprite step frame (right or left foot step)
const uint IN_MOTION   = 0x00200000; // This is for detecting whether the sprite is currently moving
const uint UPDATEABLE  = 0x00400000; // If this bit is set to zero, we do not update the sprite
const uint VISIBLE     = 0x00800000; // If this bit is set to zero, we do not draw the sprite
const uint CONTROLABLE = 0x01000000; // If this bit is set, the sprite can be controlled by the player
const uint SPR_GLOBAL  = 0x02000000; // Sprite frames are located in GameInstance singleton

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
		uint properties: a bit-wise mask indicating various tile properties
******************************************************************************/
class MapTile {
public:
	int lower_layer;
	int upper_layer;
	uint properties;
};


/******************************************************************************
	TileFrame class - element of a circular singlely linked list for tile frame animations
	
	>>>members<<<
		int frame: holds a frame index pointing to map_tiles in the MapMode class
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
		unsigned char object_type: an identifier type for the object
		uint row_pos: the map row position for the bottom left corner of the object
		uint col_pos: the map col position for the bottom left corner of the object
		uint status: a bit-mask for setting and detecting various conditions on the object

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
	uint row_pos;
	uint col_pos;
	uint status;
	hoa_video::GameVideo *VideoManager;
	
	friend class MapMode;
	friend class hoa_data::GameData;
public:
	ObjectLayer(unsigned char type, uint row, uint col, uint status);
	~ObjectLayer();
	virtual void Draw(local_map::MapFrame& mf) = 0;
};


/******************************************************************************
	MapSprite - A moveable, controllable object on the map
		
	>>>members<<<
		float step_speed: determines how fast the object can move around the map
		float step_count: a counter used for tracking movement
	
	>>>functions<<<
	
	>>>notes<<<
		1) This class has a lot of different constructors for different types of map sprites.
				Briefly, the constructors serve the following types of sprite creatiion:
					- Sprites whose frames are loaded in the InstanceManager singleton
					- Standard sprites (mostly NPCs)
					- Virtual sprites who have no frames
	
		2) This class can be used to create an invisible "camera" that the map focuses on.
				That's basically what we refer to as a 'virtual sprite' and there's a special
				constructor for making one.
		
		2)
 *****************************************************************************/
class MapSprite : public ObjectLayer {
private:
	friend class MapMode;
	friend class hoa_data::GameData;
	
	int FindFrame();
protected:
	std::string name;
	std::string filename;
	float step_speed;
	float step_count;
	int wait_time;
	uint delay_time;
	
	std::vector<hoa_video::ImageDescriptor> *frames;
	SpriteDialogue *speech;
	void Draw(local_map::MapFrame& mf);
public:
	MapSprite(unsigned char type, uint row, uint col, uint stat);
	~MapSprite();
	
	void LoadFrames();
	void LoadCharacterInfo(uint character);
	
	void SetName(std::string na) { name = na; }
	void SetFilename(std::string fn) { filename = fn; }
	void SetSpeed(float ss) { step_speed = ss; }
	void SetDelay(uint dt) { delay_time = dt; }
};


/******************************************************************************
	SpriteDialoge class - container class for sprite dialoge
	
	>>>members<<<
		uint next_read: index to next piece of dialogue to read
		vector<vector<string> > dialogue: the text of the dialogue
		vector<vector<uint> > speaker: who will be saying each part of the dialogue
		vector<bool> seen: whether the player has seen this text or not
		bool seen_all: true if the player has read all the dialogue
		
	>>>functions<<<
		bool ReadAllDialogue(): returns seen_all
		
	>>>notes<<<
		1) Still need to add support for "interactive" dialogues. That is, the NPC says
				something, the character says something back.
 *****************************************************************************/
class SpriteDialogue {
private:
	uint next_read;
	std::vector<std::vector<std::string> > dialogue;
	std::vector<std::vector<uint> > speaker;
	std::vector<bool> seen;
	bool seen_all;
	
	friend class MapMode;
public:
	SpriteDialogue();
	~SpriteDialogue();
	
	bool AllDialogueSeen() { return seen_all; }
	void LoadDialogue(std::vector<std::vector<std::string> > txt, std::vector<std::vector<uint> > sp);
	void AddDialogue(std::vector<std::string> txt, std::vector<uint> sp);
	void AddDialogue(std::string txt, uint sp);
	void ReadDialogue();
};


/******************************************************************************
	MapMode Class
 
	>>>members<<<
	 int map_id: a unique ID number for each map object
	 string mapname: the name of the map, also the location name in menu mode
	 vector<uint> map_state: indicates what state the map is in (explore, dialogue, script, etc)
	 int animation_counter: millisecond counter for use in tile animation
	 
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
	std::vector<uint> map_state;
	int animation_counter;
	Uint32 time_elapsed;
	
	int row_count;
	int col_count;
	
	bool random_encounters;
	int encounter_rate;
	int steps_till_encounter;
	
	std::vector<std::vector<MapTile> > map_layers;
	std::vector<TileFrame*> tile_frames;
	std::vector<ObjectLayer*> object_layer;
	MapSprite *focused_object;
	MapSprite *virtual_sprite;
	
	static std::vector<hoa_video::ImageDescriptor> map_images;
	std::vector<hoa_video::ImageDescriptor> map_tiles;
	std::vector<hoa_audio::MusicDescriptor> map_music;
	std::vector<hoa_audio::SoundDescriptor> map_sound;

	
//	 vector<MapEvent> map_events;
//	std::vector<hoa_global::GEnemy> map_enemies;
	
	bool TileMoveable(int row, int col, uint z_occupied);
	void SpriteMove(int direction, MapSprite *sprite);
	void UpdateVirtualSprite();
	
	void UpdateExploreState();
	void UpdatePlayerExplore(MapSprite *player_sprite);
	void UpdateNPCExplore(MapSprite *npc);
	
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
	
	std::vector<std::vector<MapTile> > GetMapLayers() { return map_layers; }
	std::vector<hoa_video::ImageDescriptor> GetMapTiles() { return map_tiles; }
	void SetRows(int num_rows) { row_count = num_rows; }
	void SetCols(int num_cols) { col_count = num_cols; }
	void SetMapLayers(std::vector<std::vector<MapTile> > layers) { map_layers = layers; }
	void SetMapTiles(std::vector<hoa_video::ImageDescriptor> tiles) { map_tiles = tiles; }
	int GetRows() { return row_count; }
	int GetCols() { return col_count; }
	
	void Update(Uint32 new_time_elapsed);
	void Draw();
};

} // namespace hoa_map;

#endif
