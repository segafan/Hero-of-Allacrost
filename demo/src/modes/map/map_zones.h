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
*** format ( big tiles, not collision cells )
***
*** \note ZoneSection should not be used by itself, attach it to a MapZone.
*** ***************************************************************************/
class ZoneSection {
public:
	ZoneSection( uint16 s_row = 0, uint16 s_col = 0, uint16 e_row = 0, uint16 e_col = 0 )
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
	void AddSection( const ZoneSection & section )
		{ _sections.push_back( section ); }

	static bool IsInsideZone( const uint16 pos_x, const uint16 pos_y, const MapZone & zone )
	{
		
		for( std::vector< ZoneSection >::iterator i = zone._sections.begin();
			 i != zone._sections.end(); ++i )
		{
			if( pos_x >= i->start_col && pos_x <= i->end_col
				&& pos_y >= i->start_row && pos_y <= i->end_row )
			{
				return true;
			}
		}
		return false;
	}

	virtual void Update() = 0;

protected:
	std::vector< ZoneSection > _sections;
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
	MonsterZone( uint8 max_monsters, uint32 regen_time, bool constrained = true )
		: _regen_time( regen_time ), _max_monsters( max_monsters ), _time_left( 0 ),
		_active_monsters( 0 ), _constrained( constrained ) { }
	
	void Update() {
		if( _active_monsters < _max_monsters ) {
			_time_left -= time_elapsed;
			if( _time_left <= 0 ) {
				//Make monster active
				++_active_monsters;
				_time_left = _regen_time;
			}
		}	
	}

private:
	uint32 _regen_time;
	int32 _time_left;
	uint8 _max_monsters;
	uint8 _active_monsters;

	bool _constrained;

	std::vector< MonsterSprite* > _monsters;
};

}

}

#endif