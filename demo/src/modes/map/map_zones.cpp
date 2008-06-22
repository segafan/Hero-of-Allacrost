///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
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

#include "system.h"

#include "map.h"
#include "map_objects.h"
#include "map_sprites.h"
#include "map_zones.h"

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
	for (vector<ZoneSection>::const_iterator i = _sections.begin(); i != _sections.end(); ++i) {
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
		IF_PRINT_WARNING(MAP_DEBUG) << "function called with a zero count for the enemy" << endl;
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
		collision = MapMode::_current_map->_object_manager->DetectCollision(_enemies[index]);
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

// *****************************************************************************
// *********************** ContextZone Class Functions *************************
// *****************************************************************************

ContextZone::ContextZone(MAP_CONTEXT one, MAP_CONTEXT two) :
	_context_one(one),
	_context_two(two)
{
	if (_context_one == _context_two) {
		PRINT_ERROR << "tried to create a ContextZone with two equal context values: " << _context_one << endl;
		exit(1);
	}
}


void ContextZone::AddSection(ZoneSection* section, bool context) {
	_sections.push_back(*section);
	_section_contexts.push_back(context);
	delete section;
}



void ContextZone::Update()
{
	int16 index;

	// For every ground object, if it is within the zone and its current context is equal to either context one or
	// context two
	for (std::vector<MapObject*>::iterator i = MapMode::_current_map->_object_manager->_ground_objects.begin();
		i != MapMode::_current_map->_object_manager->_ground_objects.end(); i++)
	{
		// If the object does not have a context equal to one of the two switching contexts, do not examine it further
		if ((*i)->GetContext() != _context_one && (*i)->GetContext() != _context_two) {
			continue;
		}

		// Determine if the object is inside either zone. If so, set their context to that zone's context
		// (This may resultin no change from the object's current context)
		index = _IsInsideZone(*i);
		if (index >= 0) {
			(*i)->SetContext(_section_contexts[index] ? _context_one : _context_two);
		}
	}
}



int16 ContextZone::_IsInsideZone(MapObject* object) {
		// Verify each section of the zone to make sure the position is in bounds.
	for (uint16 i = 0; i < _sections.size(); i++) {
		if (object->x_position >= _sections[i].start_col && object->x_position <= _sections[i].end_col &&
			object->y_position >= _sections[i].start_row && object->y_position <= _sections[i].end_row )
		{
			return i;
		}
	}
	return -1;
}

} // namespace private_map

} // namespace hoa_map
