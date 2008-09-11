///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
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
*** *****************************************************************************/

#ifndef __MAP_ZONES_HEADER__
#define __MAP_ZONES_HEADER__

// Allacrost utilities
#include "utils.h"
#include "defs.h"


namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief Class that represents a rectangular area on a map.
***
*** The area is represented by the top-left corner as the start and the bottom
*** right corner as the end of the area. Both are represented in the row / column
*** format (small tiles, collision cells).
***
*** \note ZoneSection should not be used by itself. Attach it to a MapZone.
*** ***************************************************************************/
class ZoneSection {
public:
	ZoneSection(uint16 s_col, uint16 s_row, uint16 e_col, uint16 e_row) :
		start_row(s_row), start_col(s_col), end_row(e_row), end_col(e_col)
		{}

	//! \brief Coordinates of the top-left tile corner of this zone.
	uint16 start_row, start_col;

	//! \brief Coordinates of the bottom-right tile corner of this zone.
	uint16 end_row, end_col;
}; // class ZoneSection


/** ***************************************************************************
*** \brief A class that represents a special zone on a map.
***
*** The area is made up of many ZoneSection instances, so it can be any shape.
*** This class can be derived to create enemy zones, poisonous zones, etc.
***
*** \note ZoneSections in the MapZone may overlap without any problem.
*** ***************************************************************************/
class MapZone {
public:
	MapZone()
		{}

	virtual ~MapZone()
		{}

	void AddSection(ZoneSection * section);

	/** \brief Returns true if the position coordinates are located inside the zone
	*** \param pos_x The x position to check
	*** \param pos_y The y position to check
	**/
	bool IsInsideZone(uint16 pos_x, uint16 pos_y);

	//! \brief Updates the state of the zone and the state of any objects in the zone
	virtual void Update()
		{}

protected:
	//! \brief The rectangular sections which compose the map zone
	std::vector<ZoneSection> _sections;

	/** \brief Returns random x, y position coordinates within the zone
	*** \param &x A reference where to store the value of the x position
	*** \param &y A reference where to store the value of the x position
	**/
	void _RandomPosition(uint16& x, uint16& y);
}; // class MapZone


/** ****************************************************************************
*** \brief Class that represents an area where enemies spawn and roam in.
***
*** This class makes a zone regenerate dead enemies after a certain amount of
*** time. The enemies can be constrained in the zone area or be free to roam
*** the whole map after spawning.
*** ***************************************************************************/
class EnemyZone : public MapZone {
public:
	EnemyZone(uint32 regen_time, bool restrained);

	virtual ~EnemyZone()
		{}

	/** \brief Adds a new enemy sprite to the zone
	*** \param new_enemy A pointer to the EnemySprite object instance to add
	*** \param map A pointer to the MapMode instance to add the EnemySprite to
	*** \param count The number of copies of this enemy to add
	**/
	void AddEnemy(EnemySprite* new_enemy, MapMode* map, uint8 count = 1);

	//! \brief Decrements the number of active enemies by one
	void EnemyDead()
		{ --_active_enemies; }

	//! \brief Gradually spawns enemy sprites in the zone
	void Update();

	//! \name Class Member Access Functions
	//@{
	bool IsRestraining() const
		{ return _restrained; }
	//@}

private:
	//! \brief The amount of time that should elapse before spawning the next enemy sprite
	uint32 _regen_time;

	//! \brief A timer used for the respawning of enemies in the zone
	uint32 _spawn_timer;

	//! \brief The number of enemies that are currently not in the DEAD state
	uint8 _active_enemies;

	//! \brief If true, enemies of this zone are not allowed to roam outside of the zone boundaries
	bool _restrained;

	//! \brief Contains all of the enemies that may exist in this zone.
	std::vector<EnemySprite*> _enemies;
}; // class EnemyZone


/** ****************************************************************************
*** \brief Class that represents an area where the active map context may switch
***
*** This type of zone enables map sprites to transfer betweeen two map contexts.
*** Each zone section added is labeled as corresponding to one context or the
*** other. When a sprite stands upon a particular section, their context will
*** be set to the context of that section.
***
*** \todo In the future collision detection needs to be accounted for when two
*** objects are in the context zone but have different active map contexts.
*** Normally no collision detection is done between objects in different cases,
*** but context zones need to be an exception to this rule.
***
*** \bug If the MapZone::AddSection() function is called instead of the ContextZone
*** specific one, then the _section_contexts vector will be unequal in size to the
*** _sections vector, and this will likely result in an out of bounds access error.
*** ***************************************************************************/
class ContextZone : public MapZone {
public:
	/** \brief The constructor requires the map contexts of the zone to be declared immediately
	*** \note These two context arguments can not be equal
	**/
	ContextZone(MAP_CONTEXT one, MAP_CONTEXT two);

	/** \brief Adds a new rectangular section to the zone
	*** \param section The section to add
	*** \param context True indicates the section belongs to context one, false to context two
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

} // namespace private_map

} // namespace hoa_map

#endif // __MAP_ZONES_HEADER__
