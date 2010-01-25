///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_zones.h
*** \author  Guillaume Anctil, drakkoon@allacrost.org
*** \brief   Header file for map mode zones.
*** ***************************************************************************/

#ifndef __MAP_ZONES_HEADER__
#define __MAP_ZONES_HEADER__

// Allacrost utilities
#include "utils.h"
#include "defs.h"

namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief Represents a rectangular area on a map.
***
*** The area is represented by the coordinates of the top-left and bottom-right
*** corners. Both are represented in the row / column format of collision grid
*** elements. Zone sections can only include entire grid elements, not portions
*** of an element.
***
*** \note The primary intent of this class is to be able to combine several
*** ZoneSections to create a non-rectangular shape. This is how a map zone is
*** formed.
*** ***************************************************************************/
class ZoneSection {
public:
	ZoneSection(uint16 col1, uint16 row1, uint16 col2, uint16 row2) :
		top_row(row1), bottom_row(row2), left_col(col1), right_col(col2)
		{}

	//! \brief Collision grid rows for the top and bottom section of the area
	uint16 top_row, bottom_row;

	//! \brief Collision grid columns for the top and bottom section of the area
	uint16 left_col, right_col;
}; // class ZoneSection


/** ****************************************************************************
*** \brief Represents a zone on a map that can take any shape
***
*** The area is made up of many ZoneSection instances, so it can be any shape
*** (specifically, any combination of rectangular shapes). A MapZone itself
*** is not very useful, but serves as a base for other derived classes. Note
*** that a MapZone may be relevant in only certain map contexts but not others.
***
*** \note ZoneSections in the MapZone may overlap without any problem. However
*** you should try to create a MapZone using as few ZoneSections as possible to
*** improve performance.
*** ***************************************************************************/
class MapZone {
public:
	MapZone()
		{}

	virtual ~MapZone()
		{}

	/** \brief Adds a new zone section to the map zone
	*** \param section A pointer to the section to create
	***
	*** The implementation of this function is designed with Lua in mind. The
	*** ZoneSection argument is deleted before the function returns.
	***
	*** \todo Determine if the pointer deletion is necessary. Having the argument
	*** as a reference instead of a pointer would be more preferrable.
	**/
	void AddSection(ZoneSection* section);

	/** \brief Returns true if the position coordinates are located inside the zone
	*** \param pos_x The x position to check
	*** \param pos_y The y position to check
	**/
	bool IsInsideZone(uint16 pos_x, uint16 pos_y);

	//! \brief Updates the state of the zone
	virtual void Update()
		{}

protected:
	//! \brief The rectangular sections which compose the map zone
	std::vector<ZoneSection> _sections;

	/** \brief Returns random x, y position coordinates within the zone
	*** \param x A reference where to store the value of the x position
	*** \param y A reference where to store the value of the x position
	**/
	void _RandomPosition(uint16& x, uint16& y);
}; // class MapZone


/** ****************************************************************************
*** \brief Represents an area where enemy sprites spawn and roam in.
***
*** This zone will spawn enemy sprites somewhere within its boundaries. It also
*** regenerates dead enemies after a certain amount of time. The enemies can be
*** constrained within the zone area or be made free to roam the entire map
*** after spawning.
***
*** \note All time/timer members are in number of milliseconds
*** ***************************************************************************/
class EnemyZone : public MapZone {
public:
	/** \param regen_time The number of milliseconds to wait before spawning an enemy
	*** \param restrained If true, spawned enemy sprites may not leave the zone
	**/
	EnemyZone(uint32 regen_time, bool restrained);

	virtual ~EnemyZone()
		{}

	/** \brief Adds a new enemy sprite to the zone
	*** \param enemy A pointer to the EnemySprite object instance to add
	*** \param map A pointer to the MapMode instance to add the EnemySprite to
	*** \param count The number of copies of this enemy to add
	**/
	void AddEnemy(EnemySprite* enemy, MapMode* map, uint8 count = 1);

	//! \brief Decrements the number of active enemies by one
	void EnemyDead();

	//! \brief Gradually spawns enemy sprites in the zone
	void Update();

	//! \name Class Member Access Functions
	//@{
	bool IsRestrained() const
		{ return _restrained; }

	void SetRestrained(bool restrain)
		{ _restrained = restrain; }

	void SetRegenTime(uint32 rtime)
		{ _regen_time = rtime; }
	//@}

private:
	//! \brief The amount of time that should elapse before spawning the next enemy sprite
	uint32 _regen_time;

	//! \brief A timer used for the respawning of enemies within the zone
	uint32 _spawn_timer;

	//! \brief The number of enemies that are currently not in the DEAD state
	uint8 _active_enemies;

	//! \brief If true, enemies of this zone are not allowed to roam outside of the zone boundaries
	bool _restrained;

	/** \brief Contains all of the enemies that may exist in this zone.
	*** \note These sprites will be deleted by the map object manager, not the
	*** destructor of this class.
	**/
	std::vector<EnemySprite*> _enemies;
}; // class EnemyZone


/** ****************************************************************************
*** \brief Represents an area where the active map context may switch
***
*** This type of zone enables map sprites to transfer betweeen two map contexts.
*** Each zone section added is labeled as corresponding to one context or the
*** other. When a sprite stands upon a particular section, their context will
*** be set to the context of that section.
***
*** \todo In the future collision detection needs to be accounted for when two
*** objects are in the context zone but have different active map contexts.
*** Normally no collision detection is done between objects in different contexts,
*** but context zones need to be an exception to this rule.
***
*** \todo Currently the Update() function checks all ground objects to determine
*** if any context changes need to occur. This is a temporarily solution that needs
*** to be improved by the following:
***  - The class should have a container of objects currently located within the zone,
***    and check only those objects (when sprites are in motion, they can check if they
***    have stepped into a context zone there)
***  - Sky objects should also be able to change their context via context zones
***  - There should be an option for having the context zone not to apply to either the
***    ground or sky object layers
***
*** \bug MapZone::AddSection() is invalid for this function
*** If the MapZone::AddSection() function is called instead of the ContextZone
*** one (which has a different signature), then the _section_contexts vector will
*** be unequal in size to the _sections vector, and this could result in an out
*** of bounds access error later during the game's execution.
*** ***************************************************************************/
class ContextZone : public MapZone {
public:
	/** \brief The constructor requires the map contexts of the zone to be declared immediately
	*** \note These two context arguments can not be equal
	**/
	ContextZone(MAP_CONTEXT one, MAP_CONTEXT two);

	/** \brief Adds a new rectangular section to the zone
	*** \param section The section to add (the pointer will be deleted upon adding the section)
	*** \param context True indicates that the new section belongs to context one, false to context two
	***
	*** \todo Determine if the pointer deletion is necessary. Having the argument
	*** as a reference instead of a pointer would be more preferrable.
	**/
	void AddSection(ZoneSection *section, bool context);

	//! \brief Updates the active contexts of any map objects that exist within the zone
	void Update();

private:
	//! \brief The different map contexts that the context zone allows an object to transition between
	MAP_CONTEXT _context_one, _context_two;

	//! \brief Stores the context of each zone section. True indicates context one, false is context two
	std::vector<bool> _section_contexts;

	/** \brief Determines if a map object is inside the context zone
	*** \param object A pointer to the map object
	*** \return The index of the zone section where the object is located, or -1 if it is not in the zone
	**/
	int16 _IsInsideZone(MapObject* object);
}; // class ContextZone : public MapZone


/** ****************************************************************************
*** \brief Represents an area where the active map context may switch
***
***
*** ***************************************************************************/
class AudioZone : public MapZone {
public:
	AudioZone();

	~AudioZone();

private:
}; // class AudioZone : public MapZone

} // namespace private_map

} // namespace hoa_map

#endif // __MAP_ZONES_HEADER__
