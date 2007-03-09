///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
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

#include "utils.h"
#include "defs.h"


namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief Class that represents a rectangular area on a map.
***
*** The area is represented by the top-left corner as the start and the bottom
*** right corner as the end of the area. Both are represented in the row / column
*** format ( small tiles, collision cells )
***
*** \note ZoneSection should not be used by itself, attach it to a MapZone.
*** ***************************************************************************/
class ZoneSection {
public:
	ZoneSection( uint16 s_col, uint16 s_row, uint16 e_col, uint16 e_row )
		: start_row( s_row ), start_col( s_col ), end_row( e_row ), end_col( e_col ) {}

	uint16 start_row, start_col;
	uint16 end_row, end_col;
};

/** ***************************************************************************
*** \brief Abstract class that represents a special zone on a map.
***
*** The area is made up of many ZoneSection, so it can be more than rectangular.
*** This class can be derived to create poisonous zones, etc.
***
*** \note ZoneSections in the MapZone can overlap without creating problems.
*** ***************************************************************************/
class MapZone {
public:
	MapZone(){};
	virtual ~MapZone(){};
	void AddSection( ZoneSection * section );

	static bool IsInsideZone( uint16 pos_x, uint16 pos_y, const MapZone * zone );

	virtual void Update() = 0;

protected:
	void _RandomPosition( uint16 & x, uint16 & y );

	std::vector< ZoneSection > _sections;
	MapMode* _map;
};

/** ****************************************************************************
*** \brief Class that represents an area where monsters spawn and roam in.
***
*** This class makes a zone regenerate dead monsters after a certain amount of
*** time. The monsters can be constrained in the zone area or be free to roam
*** the whole map after spawning.
*** ***************************************************************************/
class MonsterZone : public MapZone {
public:
	MonsterZone(MapMode* map, uint8 max_monsters, uint32 regen_time, bool restrained);

	virtual ~MonsterZone()
		{}

	void Update();

	void AddMonster(MonsterSprite* m);

	void MonsterDead();

	bool IsRestraining() const
		{ return _restrained; }

private:
	uint32 _regen_time;
	uint32 _time_elapsed;
	uint8 _max_monsters;
	uint8 _active_monsters;

	bool _restrained;

	std::vector<MonsterSprite*> _monsters;
};

} // namespace private_map

} // namespace hoa_map

#endif // __MAP_ZONES_HEADER__
