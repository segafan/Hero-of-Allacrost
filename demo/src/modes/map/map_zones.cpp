///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_zones.cpp
*** \author  Guillaume Anctil, drakkoon@allacrost.org
*** \brief   Source file for map mode zones.
*** ***************************************************************************/

#include "map_zones.h"
#include "system.h"
#include "map_objects.h"

using namespace hoa_utils;

namespace hoa_map {

namespace private_map {

// *****************************************************************************
// ************************* MapZone Class Functions ***************************
// *****************************************************************************

void MapZone::AddSection( ZoneSection * section )
	{ _sections.push_back( *section ); delete section; }

bool MapZone::IsInsideZone( uint16 pos_x, uint16 pos_y, const MapZone * zone ) {
	//Verify each section of the zone to make sure the position is in bounds.
	for( std::vector< ZoneSection >::const_iterator i = zone->_sections.begin();
		 i != zone->_sections.end(); ++i )
	{
		if( pos_x >= i->start_col && pos_x <= i->end_col
			&& pos_y >= i->start_row && pos_y <= i->end_row )
		{
			return true;
		}
	}
	return false;
}

void MapZone::_RandomPosition( uint16 & x, uint16 & y ) {
	//Select a ZoneSection randomly
	uint16 i = RandomBoundedInteger( 0, _sections.size() - 1 );

	//Select a position inside that section
	x = RandomBoundedInteger(_sections[i].start_col, _sections[i].end_col);
	y = RandomBoundedInteger(_sections[i].start_row, _sections[i].end_row);
}

// *****************************************************************************
// *********************** MonsterZone Class Functions *************************
// *****************************************************************************

MonsterZone::MonsterZone(MapMode* map, uint8 max_monsters, uint32 regen_time, bool restrained) :
	_regen_time(regen_time),
	_time_elapsed(0),
	_max_monsters(max_monsters),
	_active_monsters(0),
	_restrained(restrained)
{
	_map = map;
}



void MonsterZone::Update() {
	if( _active_monsters < _max_monsters ) {
		//Add / Spawn monsters if the maximum number is not reached
		_time_elapsed += hoa_system::SystemManager->GetUpdateTime();
		if( _time_elapsed >= ( _regen_time + rand()%(_regen_time/4) ) ) {
			//If the regen time is reached ( + up to 1/4 of the regen time )
			//Spawn a new monster
			if( _monsters.size() < _max_monsters ) {
				//There are not enough MonsterSprite to show on the map
				//Create a copy from a randomly selected sprite
				if( _monsters.size() > 0 ) {
					MonsterSprite* tempMonster = new MonsterSprite( *_monsters[ rand()%_monsters.size() ] );
					tempMonster->SetObjectID( _map->_GetGeneratedObjectID() );
					tempMonster->Reset();
					_map->_AddGroundObject( tempMonster );
					_monsters.push_back( tempMonster );
				}
			}
			//Select a DEAD monster to spawn
			for( uint32 i = 0; i < _monsters.size(); i++ ) {
				if( !_monsters[i]->updatable ) {
					++_active_monsters;

					//Select a random position inside the zone where there is no collision
					uint16 x, y;
					do {
						_RandomPosition( x, y );

						_monsters[i]->SetXPosition( x, 0.5f );
						_monsters[i]->SetYPosition( y, 0.5f );
					}while( _map->_DetectCollision( _monsters[i] ) );

					//Spawn the monster at that location
					_monsters[i]->ChangeStateSpawning();
					break;
				}
			}
			//Reset timer
			_time_elapsed = 0;
		}
	}
} // void MonsterZone::Update()



void MonsterZone::AddMonster( MonsterSprite* m ) {
	_monsters.push_back( m );
	_map->_AddGroundObject( m );
	m->Load();
}



void MonsterZone::MonsterDead() {
	--_active_monsters;
}

} // namespace private_map

} // namespace hoa_map
