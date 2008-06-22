///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_objects.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for map mode objects.
*** *****************************************************************************/

#ifndef __MAP_OBJECTS_HEADER__
#define __MAP_OBJECTS_HEADER__

#include "utils.h"
#include "defs.h"

#include "video.h"

namespace hoa_map {

namespace private_map {

/** \name Map Object Type Constants
*** These constants are used to identify the type of map object or sprite.
**/
//@{
const uint8 PHYSICAL_TYPE = 0;
const uint8 VIRTUAL_TYPE = 1;
const uint8 SPRITE_TYPE = 2;
const uint8 ENEMY_TYPE = 3;
const uint8 TREASURE_TYPE = 4;
//@}

/** ****************************************************************************
*** \brief Abstract class that represents objects on a map
***
*** A map object can be anything from a sprite to a tree to a house. To state
*** it simply, a map object is a map image that is not tiled and may not be fixed
*** in place. Map objects are drawn in one of three layers: ground, pass, and sky
*** object layers. Every map object has a collision rectangle associated with it.
*** The collision rectangle indicates what parts of the object may not overlap with
*** other collision rectangles.
***
*** \note It is advised not to attempt to make map objects with dynamic sizes (i.e.
*** the various image frames that compose the object are all the same size). In
*** theory, dynamically sized objects are feasible to implement in maps, but they
*** are much more vulnerable to bugs
*** ***************************************************************************/
class MapObject {
public:
	/** \brief An identification number for the object as it is represented in the map file.
	*** Player sprites are assigned object ids from 5000 and above. Technically this means that
	*** a map can have no more than 5000 objects that are not player sprites, but no map should
	*** need to contain that many objects in the first place. Objects with an ID less than zero
	*** are invalid.
	**/
	int16 object_id;

	/** \brief The map context that the object currently resides in.
	*** Context helps to determine where an object "resides". For example, inside of a house or
	*** outside of a house. The context member determines if the object should be drawn or not,
	*** since objects are only drawn if they are in the same context as the map's camera.
	*** Objects can only interact with one another if they both reside in the same context.
	***
	*** \note The default value for this member is -1. A negative context indicates that the
	*** object is invalid and it does not exist anywhere. Objects with a negative context are never
	*** drawn to the screen. A value equal to zero indicates that the object is "always in
	*** context", meaning that the object will be drawn regardless of the current context. An
	*** example of where this is useful is a bridge, which shouldn't simply disappear because the
	*** player walks inside a nearby home.
	**/
	MAP_CONTEXT context;

	/** \brief Coordinates for the object's origin/position.
	*** The origin of every map object is the bottom center point of the object. These
	*** origin coordinates are used to determine where the object is on the map as well
	*** as where the objects collision rectangle lies.
	***
	*** The position coordinates are described by an integer (position) and a float (offset).
	*** The position coordinates point to the map grid tile that the object currently occupies
	*** and may range from 0 to the number of columns or rows of grid tiles on the map. The
	*** offset member will always range from 0.0f and 1.0f to indicate the exact position of
	*** the object within that tile.
	**/
	//@{
	uint16 x_position, y_position;
	float x_offset, y_offset;
	//@}

	/** \brief The half-width and height of the image, in map grid coordinates.
	*** The half_width member is indeed just that: half the width of the object's image. We keep
	*** the half width rather than the full width because the origin of the object is its bottom
	*** center, and it is more convenient to store only half the sprite's width as a result.
	***
	*** \note These members assume that the object retains the same width and height regardless
	*** of the current animation or image being drawn. If the object's image changes size, the
	*** API user must remember to change these values accordingly.
	**/
	float img_half_width, img_height;

	/** \brief Determines the collision rectangle for the object.
	*** The collision area determines what portion of the map object may not be overlapped
	*** by other objects or unwalkable regions of the map. The x and y coordinates are
	*** relative to the origin, so an x value of 0.5f means that the collision rectangle
	*** extends the length of 1/2 of a tile from the origin on both sides, and a y value
	*** of 1.0f means that the collision area exists from the origin to 1 tile's length
	*** above.
	***
	*** \note These members should always be positive. Setting these members to zero does *not*
	*** eliminate collision detection for the object, and therefore they should usually never
	*** be zero.
	**/
	float coll_half_width, coll_height;

	//! \brief When set to false, the Update() function will do nothing (default = true).
	bool updatable;

	//! \brief When set to false, the Draw() function will do nothing (default = true).
	bool visible;

	/** \brief When set to true, the object will not be examined for collision detection (default = false).
	*** Setting this member to true really has two effects. First, the object may exist anywhere on
	*** the map, including where the collision rectangles of other objects are located. Second, the
	*** object is ignored when other objects are performing their collision detection. This property
	*** is useful for virtual objects or objects with an image but no "physical form" (i.e. ghosts
	*** that other sprites may walk through). Note that while this member is set to true, the object's
	*** collision rectangle members are ignored.
	**/
	bool no_collision;

	/** \brief When set to true, objects in the ground object layer will be drawn after the pass objects
	*** \note This member is only checked for objects that exist in the ground layer. It has no meaning
	*** for objects in the pass or sky layers.
	**/
	bool draw_on_second_pass;

	std::string filename;

	// ---------- Methods

	MapObject();

	virtual ~MapObject()
		{}

	/** \brief Updates the state of an object.
	*** Many map objects may not actually have a use for this function. For example, animated objects like a
	*** tree automatically have their frames updated by the video engine, so there is no need to
	*** call this function for it. The function is only called for objects which have the UPDATEABLE bit in
	*** the MapObject#_status member set.
	**/
	virtual void Update() = 0;

	/** \brief Draws the object to the frame buffer.
	*** Objects are drawn differently depending on what type of object they are and what their current
	*** state is. This function is only called for objects that will be visible on the screen when drawn
	*** and have their VISIBLE bit in the MapObject#_status member set.
	**/
	virtual void Draw() = 0;

	/** \brief Assists with the drawing of map objects
	*** \return True if the object should be drawn, or false if it is not visible on the screen.
	***
	*** This method performs the common drawing operations of identifying whether or not the object
	*** is visible on the screen and moving the drawing cursor to its location. The children classes
	*** of this class may choose to make use of it (or not). All that needs to be done after this
	*** method returns true is to draw the object's image on the screen.
	**/
	bool DrawHelper();

	/** \brief Computes the full floating-point location coordinates of the object
	*** \return The full x or y coordinate location of the object
	***
	*** Since an object's position is stored as an integer component and an offset component, this
	*** method simply returns a single floating point value representing the full x and y positions
	*** of the object in a single variable.
	**/
	//@{
	float ComputeXLocation() const
		{ return (static_cast<float>(x_position) + x_offset); }

	float ComputeYLocation() const
		{ return (static_cast<float>(y_position) + y_offset); }
	//@}

	//! Loads the saved state of the object
	virtual void LoadSaved(){};

	/** \name Lua Access Functions
	*** These functions are specifically written for Lua binding, to enable Lua to access the
	*** members of this class.
	**/
	//@{
	void SetObjectID(int16 id = 0)
		{ object_id = id; }

	void SetContext(MAP_CONTEXT ctxt)
		{ context = ctxt; }

	void SetXPosition(uint16 x, float offset)
		{ x_position = x; x_offset = offset; }

	void SetYPosition(uint16 y, float offset)
		{ y_position = y; y_offset = offset; }

	void SetImgHalfWidth(float width)
		{ img_half_width = width; }

	void SetImgHeight(float height)
		{ img_height = height; }

	void SetCollHalfWidth(float collision)
		{ coll_half_width = collision; }

	void SetCollHeight(float collision)
		{ coll_height = collision; }

	void SetUpdatable(bool update)
		{ updatable = update; }

	void SetVisible(bool vis)
		{ visible = vis; }

	void SetNoCollision(bool coll)
		{ no_collision = coll; }

	void SetDrawOnSecondPass(bool pass)
		{ draw_on_second_pass = pass; }

	int16 GetObjectID() const
		{ return object_id; }

	MAP_CONTEXT GetContext() const
		{ return context; }

	void GetXPosition(uint16 &x, float &offset) const
		{ x = x_position; offset = x_offset; }

	void GetYPosition(uint16 &y, float &offset) const
		{ y = y_position; offset = y_offset; }

	float GetImgHalfWidth() const
		{ return img_half_width; }

	float GetImgHeight() const
		{ return img_height; }

	float GetCollHalfWidth() const
		{ return coll_half_width; }

	float GetCollHeight() const
		{ return coll_height; }

	bool IsUpdatable() const
		{ return updatable; }

	bool IsVisible() const
		{ return visible; }

	bool IsNoCollision() const
		{ return no_collision; }

	bool IsDrawOnSecondPass() const
		{ return draw_on_second_pass; }

	uint8 GetType() const
		{ return _object_type; }
	//@}

protected:
	//! \brief This holds the the type of sprite this is.
	uint8 _object_type;

}; // class MapObject


/** \brief This is a predicate used to sort MapObjects in correct draw order
*** \return True if the MapObject pointed by a should be drawn behind MapObject pointed by b
*** \note A simple '<' operator cannot be used with the sorting algorithm because it is sorting pointers.
**/
struct MapObject_Ptr_Less {
	const bool operator()(const MapObject * a, const MapObject * b) {
		return (a->y_position + a->y_offset) < (b->y_position + b->y_offset);
	}
};


/** ****************************************************************************
*** \brief Represents visible objects on the map that have no motion.
***
*** This class represents both still image and animated objects. These objects
*** are fixed in place and can not move. The object must have at least one
*** entry in its image vector, otherwise a segmentation fault will occur if the
*** Update or Draw functions are called.
***
*** \note If the object does not have any animated images, set the updatable
*** member of this class to false. Forgetting to do this will do no harm, but
*** it will do an extra function call that it shouldn't need to do.
*** ***************************************************************************/
class PhysicalObject : public MapObject {
public:
	//! \brief The index to the animations vector that contains the current image to display
	uint8 current_animation;

	/** \brief A vector containing all the object's animations.
	*** Note that these need not be actual animations. An AnimatedImage object may consist
	*** of only a single frame. Usually an object will only need a single image or animation,
	*** but a vector is used here in case others are needed.
	**/
	std::vector<hoa_video::AnimatedImage> animations;

	PhysicalObject();

	~PhysicalObject();

	//! \brief Updates the object's animation frames if it is animated.
	virtual void Update();

	//! \brief Draws the object to the screen, if it is visible.
	virtual void Draw();

	/** \name Lua Access Functions
	*** These functions are specifically written for Lua binding, to enable Lua to access the
	*** members of this class.
	**/
	//@{
	void AddAnimation(hoa_video::AnimatedImage new_img)
		{ animations.push_back(new_img); }

	void SetCurrentAnimation(uint8 current)
		{ animations[current_animation].SetTimeProgress(0); current_animation = current; }

	void SetAnimationProgress(uint32 progress)
		{ animations[current_animation].SetTimeProgress(progress); }

	uint8 GetCurrentAnimation() const
		{ return current_animation; }
	//@}
}; // class PhysicalObject : public MapObject


/** ****************************************************************************
*** \brief Represents a treasure on the map which the player may open
***
*** This class acts as an optionnaly hidden object that has to be found before
*** being able to activate. When activated it will transfer its content to the
*** player's inventory.
***
*** Image files for treasure are single row multi images where the frame ordering
*** goes from closed, to opening, to open. This means each map treasure has exactly
*** three animations, although the closed and open animations are usually single
*** frame images.
***
*** \todo Add support for more treasure features, such as locked chests, chests which
*** trigger a battle, etc.
*** ***************************************************************************/
class MapTreasure : public PhysicalObject {
	friend class TreasureMenu;
public:
	//! \brief Values for defining the 3 possible treasure animations
	enum {
		CLOSED_ANIM = 0,
		OPENING_ANIM = 1,
		OPEN_ANIM = 2
	};

	//! \brief The values pertaining to a treasure's state as a GlobalEvent
	enum {
		TREASURE_UNUSED = 0,
		TREASURE_EMPTY = 1
	};

	/** \brief Creates a MapTreasure
	*** \param image_file An image file in the form of a one row strip
	*** \param num_total_frames The total number of frame images in the multi image file
	*** \param num_closed_frames The number of frames to use as the closed animation (default value == 1)
	*** \param num_open_frames The number of frames to use as the open animation (default value == 1)
	*** \note The opening animation will be created based on the total number of frames in the image file
	*** subtracted by the number of closed and open frames. If this value is zero, then the opening animation
	*** will simply be the same as the open animation
	**/
	MapTreasure(std::string image_file, uint8 num_total_frames, uint8 num_closed_frames = 1, uint8 num_open_frames = 1);

	~MapTreasure();

	//! \brief Loads the state of the chest from the event group corresponding to the current map
	void LoadSaved();

	/** \name Lua Access Functions
	*** These functions are specifically written for Lua binding, to enable Lua to access the
	*** members of this class.
 	**/
	//@{

	//! \brief Indicates if the treasure contains any
	bool IsEmpty() const
		{ return _empty; }

	/** \brief Adds an object to the contents of the MapTreasure
	*** \param id The id of the GlobalObject to add
	*** \param number The number of the object to add (default == 1)
	*** \throw Exception An Exception object if nothing could be added to the treasure
	**/
	bool AddObject(uint32 id, uint32 number = 1);

	/** \brief Adds a number of drunes to be the chest's contents
	*** \note The overflow condition is not checked here: we just assume it will never occur
	**/
	void AddDrunes(uint32 amount)
		{ _drunes += amount; }

	//! \brief Opens the treasure, which changes the active animation and initializes the treasure menu
	void Open();

	//! \brief Changes the current animation if it has finished looping
	void Update();
	//@}

private:
	//! \brief Set to true if the contents of the treasure have been emptied out
	bool _empty;

	//! \brief The number of drunes contained in the chest
	uint32 _drunes;

	//! \brief The list of objects given to the player upon activation
	std::vector<hoa_global::GlobalObject*> _objects_list;
}; // class MapTreasure : public PhysicalObject


/** ****************************************************************************
*** \brief A container class for node information in pathfinding.
***
*** This class is used in the MapMode#_FindPath function to find an optimal
*** path from a given source to a destination.
*** *****************************************************************************/
class PathNode {
public:
	/** \brief The coordinates for this node
	*** These coordinates correspond to the MapMode#_walkable 2D vector, where
	*** each element is a 16x16 pixel space on the map.
	**/
	//@{
	int16 row, col;
	//@}

	//! \name Path Scoring Members
	//@{
	//! \brief The total score for this node (f = g + h).
	int16 f_score;

	//! \brief The score for this node relative to the source.
	int16 g_score;

	//! \brief The Manhattan distance from this node to the destination.
	int16 h_score;
	//@}

	//PathNode *parent;
	int16 parent_row, parent_col;

	PathNode() :
		row(-1), col(-1), f_score(0), g_score(0), h_score(0), parent_row( 0 ), parent_col( 0 ) {}

	PathNode(int16 r, int16 c) :
		row(r), col(c), f_score(0), g_score(0), h_score(0), parent_row( 0 ), parent_col( 0 ) {}

	//! \brief Overloaded comparison operator checks that tile.row and tile.col are equal
	bool operator==(const PathNode& that) const
		{ return ((this->row == that.row) && (this->col == that.col)); }

	//! \brief Overloaded comparison operator checks that tile.row or tile.col are not equal
	bool operator!=(const PathNode& that) const
		{ return ((this->row != that.row) || (this->col != that.col)); }

	//! \brief Overloaded comparison operator only used for path finding. It compares the two f_scores.
	bool operator<(const PathNode& that) const
		{ return this->f_score > that.f_score; }
}; // class PathNode


/** ****************************************************************************
*** \brief A helper class to MapMode responsible for all object/sprite data and operations
***
*** This class is responsible for loading, updating, and drawing all map objects
*** and map sprites, in addition to maintaining the map's collision grid and map
*** zones.
*** ***************************************************************************/
class ObjectManager {
	friend class hoa_map::MapMode;
	// TEMP for allowing context zones to access all objects
	friend class hoa_map::private_map::ContextZone;

public:
	ObjectManager();
	~ObjectManager();

	//! \brief Handles all operations on loading tilesets and tile images for the map's Lua file
	void Load(hoa_script::ReadScriptDescriptor& map_file);

	//! \brief Updates the state of all map objects and zones
	void Update();

	//! \brief Sorts objects on all three layers according to their draw order
	void SortObjects();

	/** \brief Draws the various object layers to the screen
	*** \param frame A pointer to the already computed information required to draw this frame
	*** \note These functions do not reset the coordinate system and hence depend that the proper coordinate system
	*** is already set prior to these function calls (0.0f, SCREEN_COLS, SCREEN_ROWS, 0.0f). These functions do make
	*** modifications to the blending draw flag and the draw cursor position which are not restored by the function
	*** upon its return, so take measures to retain this information before calling these functions if required.
	**/
	//@{
	void DrawGroundObjects(const MapFrame* const frame, const bool second_pass);
	void DrawPassObjects(const MapFrame* const frame);
	void DrawSkyObjects(const MapFrame* const frame);
	//@}

	// TODO
	// void SwitchContext(MAP_CONTEXT context);

	/** \brief Finds the nearest interactable object within a certain distance.
	*** \param *sprite The sprite who is trying to find its nearest object.
	*** \return A pointer to the nearest interactable map object, or NULL if no such object was found.
	***
	*** An interactable object must be in the same context as the function argument is. For an object
	*** to be valid, it's collision rectangle must be no greater than 3 grid elements from the sprite's
	*** "calling" axis, and th
	***
	**/
	private_map::MapObject* FindNearestObject(const private_map::VirtualSprite* sprite);

	/** \brief Determines if a map sprite's position is invalid because of a collision
	*** \param sprite A pointer to the map sprite to check
	*** \return True if a collision was detected, false if one was not
	***
	*** This method is invoked by the map sprite who wishes to check for its own collision. The
	*** collision detection is performed agains three types of obstacles:
	***
	*** -# Boundary conditions: where the sprite has walked off the map
	*** -# Tile collisions: where the sprite's collision rectangle overlaps with an unwalkable map grid tile.
	*** -# Object collision: where the sprite's collision rectangle overlaps that of another object's,
	***    where the object is in the same draw layer and context as the original sprite.
	***
	*** \note This function does <b>not</b> check if the MapSprite argument has its no_collision member
	*** set to false, but it <b>does</b> check that of the other MapObjects.
	**/
	bool DetectCollision(private_map::VirtualSprite* sprite);

	/** \brief Finds a path from a sprite's current position to a destination
	*** \param sprite A pointer of the sprite to find the path for
	*** \param path A reference to a vector of PathNode objects to store the path
	*** \param dest The destination coordinates
	***
	*** This algorithm uses the A* algorithm to find a path from a source to a destination.
	*** This function ignores the position of all other objects and only concerns itself with
	*** which map grid elements are walkable.
	***
	*** \note If an error is detected, the function will return an empty path argument.
	**/
	void FindPath(const private_map::VirtualSprite* sprite, std::vector<private_map::PathNode>& path, const private_map::PathNode& dest);

	MapObject* GetObject(uint32 object_id);

private:
	/** \brief The number of rows and columns in the collision gride
	*** The number of collision grid rows and columns is always equal to twice
	*** that of the number of rows and columns of tiles.
	**/
	uint16 _num_grid_rows, _num_grid_cols;

	/** \brief A 2D vector indicating which spots on the map sprites may walk on.
	*** This vector is kept seperate from the vector of tiles because each tile
	*** has 4 walkable uint32 bitflags associated with it. Note that sprite objects may
	*** come in various sizes, so not all sprites may fit through a narrow
	*** passage way.
	**/
	std::vector<std::vector<uint32> > _collision_grid;

	//! \brief Holds the most recently generated object ID number
	uint16 _lastID;

	/** \brief A map containing pointers to all of the sprites on a map.
	*** This map does not include a pointer to the MapMode#_camera nor MapMode#_virtual_focus
	*** sprites. The map key is used as the sprite's unique identifier for the map. Keys
	*** 1000 and above are reserved for map sprites that correspond to the character's party.
	**/
	std::map<uint16, MapObject*> _all_objects;

	/** \brief A container for all of the map objects located on the ground layer.
	*** The ground object layer is where most objects and sprites exist in Allacrost.
	**/
	std::vector<MapObject*> _ground_objects;

	/** \brief A container for all of the map objects located on the pass layer.
	*** The pass object layer is named so because objects on this layer can both be
	*** walked under or above by objects in the ground object layer. A good example
	*** of an object that would typically go on this layer would be a bridge. This
	*** layer usually has very few objects for the map. Also, objects on this layer
	*** are unaffected by the maps context. In other words, these objects are always
	*** drawn on the screen, regardless of the current context that the player is in.
	**/
	std::vector<private_map::MapObject*> _pass_objects;

	/** \brief A container for all of the map objects located on the sky layer.
	*** The sky object layer contains the last series of elements that are drawn on
	*** a map. These objects exist high in the sky above all other tiles and objects.
	*** Translucent clouds can make good use of this object layer, for instance.
	**/
	std::vector<private_map::MapObject*> _sky_objects;

	/** \brief A "virtual sprite" that can serve as a focus point for the camera.
	*** This sprite is not visible to the player nor does it have any collision
	*** detection properties. Usually, the camera focuses on the player's sprite
	*** rather than this object, but it is useful for scripted sequences and other
	*** things.
	**/
	private_map::VirtualSprite *_virtual_focus;

	//! \brief Container for map zones, used for various purposes such as spawning of enemies
	std::vector<private_map::MapZone*> _zones;
}; // class ObjectManager

} // namespace private_map

} // namespace hoa_map

#endif
