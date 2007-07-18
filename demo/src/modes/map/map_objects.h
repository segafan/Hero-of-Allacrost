///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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
const uint8 CHEST_TYPE = 4;
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
	uint32 context;

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

	/** \name Lua Access Functions
	*** These functions are specifically written for Lua binding, to enable Lua to access the
	*** members of this class.
	**/
	//@{
	void SetObjectID(int16 id = 0)
		{ object_id = id; }

	void SetContext(uint32 ctxt)
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

	uint32 GetContext() const
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
	void Update();

	//! \brief Draws the object to the screen, if it is visible.
	void Draw();

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
 *** \brief Represents an physical object which can give other objects to the player when activated
 ***
 *** This class acts as an optionnaly hidden object that has to be found before
 *** being able to activate. When activated it will transfer its content to the
 *** player's inventory.
 ***
 *** ***************************************************************************/
class ChestObject : public PhysicalObject {
public:
	/** \name ChestObject defaults constants
 	*** These constants are used to identify the default settings of a ChestObject
	**/
	//@{
	static const uint8 NB_FRAMES_CLOSED_DEFAULT;
	static const uint8 NB_FRAMES_OPENING_DEFAULT;

	static const uint32 HIDE_FORCE_DEFAULT;
	//@}

	//! Values indicating which animation is which, should not be changed!
	enum {
		  CLOSED_CHEST_ANIM = 0
  		, OPENING_CHEST_ANIM = 1
	};


	//TODO: that
	/** \brief Creates a ChestObject
	**/
	ChestObject( std::string image_file,
	             uint8 nb_frames_closed = NB_FRAMES_CLOSED_DEFAULT,
	             uint8 nb_frames_opening = NB_FRAMES_OPENING_DEFAULT,
	             uint32 hide_force = HIDE_FORCE_DEFAULT );

	~ChestObject();

	/** \name Lua Access Functions
	*** These functions are specifically written for Lua binding, to enable Lua to access the
	*** members of this class.
 	**/
	//@{

	/** \brief Applies a number of sensing force to the object to make it appear
	*** \param force_applied The amount of sensing forced applied
	*** \return false if the object is still hidden, true if it was discovered
	**/
	bool UpdateHideForce( uint32 force_applied );

	//! Sets the hiding force required to discover the object
	void SetHidingForce( uint32 force )
		{ _hide_force = force; }

	//! Returns the hiding force left in order to discover the object
	uint32 GetHidingForce() const
		{ return _hide_force; }

	//! Indicates if the object is hidden or not
	bool IsHidden() const
		{ return _hide_force > 0; }

	//! Indicates if the object was used by the player
	bool IsUsed() const
		{ return _objects_list.empty(); }

	/** \brief Adds an object tot he contents of the ChestObject
	*** \param id The id of the object to add
	*** \param number The number of the object to add
	*** \return false if the id is not valid
	**/
	bool AddObject( uint32 id, uint32 number );

	//! Transfers the ChestObject's content to the player's inventory
	void Use();

	//@}
private:
	//! The remaining hiding force of the object
	uint32 _hide_force;

	//! The list of objects given to the player upon activation
	std::vector< hoa_global::GlobalObject* > _objects_list;

}; // class ChestObject : public PhysicalObject

} // namespace private_map

} // namespace hoa_map

#endif
