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

extern bool MAP_DEBUG;

using namespace std;

using namespace hoa_utils;

namespace hoa_map {

namespace private_map {

// *****************************************************************************
// ************************* MapZone Class Functions ***************************
// *****************************************************************************

void MapZone::AddSection(ZoneSection* section) {
	_sections.push_back(*section);
	delete section;
}



bool MapZone::IsInsideZone(uint16 pos_x, uint16 pos_y) {
	// Verify each section of the zone to make sure the position is in bounds.
	for (std::vector<ZoneSection>::const_iterator i = _sections.begin(); i != _sections.end(); ++i) {
		if (pos_x >= i->start_col && pos_x <= i->end_col &&
			pos_y >= i->start_row && pos_y <= i->end_row )
		{
			return true;
		}
	}
	return false;
}



void MapZone::_RandomPosition(uint16& x, uint16& y) {
	// Select a ZoneSection randomly
	uint16 i = RandomBoundedInteger(0, _sections.size() - 1);

	// Select a position inside that section
	x = RandomBoundedInteger(_sections[i].start_col, _sections[i].end_col);
	y = RandomBoundedInteger(_sections[i].start_row, _sections[i].end_row);
}

// *****************************************************************************
// *********************** EnemyZone Class Functions *************************
// *****************************************************************************

EnemyZone::EnemyZone(uint32 regen_time, bool restrained) :
	_regen_time(regen_time),
	_spawn_timer(0),
	_active_enemies(0),
	_restrained(restrained)
{}



void EnemyZone::AddEnemy(EnemySprite* enemy, MapMode* map, uint8 count) {
	if (count == 0) {
		// NOTE: The EnemySprite pointer passed in is not deleted in this case
		if (MAP_DEBUG)
			cerr << "MAP WARNING: EnemyZone::AddEnemy called with a zero count for the enemy" << endl;
		return;
	}

	// Prepare the first enemy
	enemy->SetZone(this);
	map->_AddGroundObject(enemy);
	_enemies.push_back(enemy);

	// Create any additional copies of the enemy and add them as well
	uint8 remaining = count - 1;
	while (remaining > 0) {
		EnemySprite* copy = new EnemySprite(*enemy);
		copy->SetObjectID(map->_GetGeneratedObjectID() );
		//Add a 10% random margin of error to make enemies look less sync'ed
		copy->SetTimeToChange( (uint32)(copy->GetTimeToChange() * ( 1 + RandomFloat()*10 ) ) );
		copy->Reset();
		map->_AddGroundObject(copy);
		_enemies.push_back(copy);
		remaining--;
	}
}



void EnemyZone::EnemyDead() {
	--_active_enemies;
}



void EnemyZone::Update() {
	// Spawn new enemies only if there is at least one enemy that is not active
	if (_active_enemies == _enemies.size()) {
		return;
	}

	// Return if the regenenration time has not been reached, return
	_spawn_timer += hoa_system::SystemManager->GetUpdateTime();
	if (_spawn_timer < _regen_time) {
		return;
	}

	// Otherwise, select a DEAD enemy to spawn
	uint32 index = 0;
	for (uint32 i = 0; i < _enemies.size(); i++) {
		if (_enemies[i]->IsDead() == true) {
			index = i;
			break;
		}
	}

	// Used to retain random position coordinates in the zone
	uint16 x, y;
	// Number of times to try to place the enemy in the zone (arbitrarly set to 5 tries)
	int8 retries = 5;
	// Holds the result of a collision detection check
	bool collision;

	// Select a random position inside the zone to place the spawning enemy, and make sure that there is no collision
	_enemies[index]->no_collision = false;
	do {
		_RandomPosition(x, y);
		_enemies[index]->SetXPosition(x, 0.0f);
		_enemies[index]->SetYPosition(y, 0.0f);
		collision = MapMode::_current_map->_DetectCollision(_enemies[index]);
	} while (collision && --retries > 0);

	// If there is still a collision, reset the collision info on the enemy and retry on the next frame update
	if (collision) {
		_enemies[index]->no_collision = true;
		return;
	}

	// Otherwise, spawn the enemy and reset the spawn timer
	_spawn_timer = 0;
	_enemies[index]->ChangeStateSpawning();
	_active_enemies++;
} // void EnemyZone::Update()

} // namespace private_map

} // namespace hoa_map
