///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
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

// Allacrost engines
#include "system.h"

// Local map mode headers
#include "map.h"
#include "map_objects.h"
#include "map_sprites.h"
#include "map_zones.h"

using namespace std;
using namespace hoa_utils;

namespace hoa_map {

namespace private_map {

// *****************************************************************************
// ********** MapZone Class Functions
// *****************************************************************************

void MapZone::AddSection(ZoneSection* section) {
	if (section == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function argument was NULL" << endl;
		return;
	}

	_sections.push_back(*section);
	delete section;
}



bool MapZone::IsInsideZone(uint16 pos_x, uint16 pos_y) {
	// Verify each section of the zone to make sure the position is in bounds.
	for (vector<ZoneSection>::const_iterator i = _sections.begin(); i != _sections.end(); ++i) {
		if (pos_x >= i->left_col && pos_x <= i->right_col &&
			pos_y >= i->top_row && pos_y <= i->bottom_row)
		{
			return true;
		}
	}
	return false;
}



void MapZone::_RandomPosition(uint16& x, uint16& y) {
	// Select a random ZoneSection
	uint16 i = RandomBoundedInteger(0, _sections.size() - 1);

	// Select a random x and y position inside that section
	x = RandomBoundedInteger(_sections[i].left_col, _sections[i].right_col);
	y = RandomBoundedInteger(_sections[i].top_row, _sections[i].bottom_row);
}

// *****************************************************************************
// ********** EnemyZone Class Functions
// *****************************************************************************

EnemyZone::EnemyZone(uint32 regen_time, bool restrained) :
	_regen_time(regen_time),
	_spawn_timer(0),
	_active_enemies(0),
	_restrained(restrained)
{}



void EnemyZone::AddEnemy(EnemySprite* enemy, MapMode* map, uint8 count) {
	if (count == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function called with a count argument equal to zero" << endl;
		return;
	}

	// Prepare the first enemy
	enemy->SetZone(this);
	map->AddGroundObject(enemy);
	_enemies.push_back(enemy);

	// Create any additional copies of the enemy and add them as well
	for (uint8 i = 1; i < count; i++) {
		EnemySprite* copy = new EnemySprite(*enemy);
		copy->SetObjectID(map->GetObjectSupervisor()->GenerateObjectID());
		// Add a 10% random margin of error to make enemies look less synchronized
		copy->SetTimeToChange(static_cast<uint32>(copy->GetTimeToChange() * (1 + RandomFloat() * 10)));
		copy->Reset();
		map->AddGroundObject(copy);
		_enemies.push_back(copy);
	}
}



void EnemyZone::EnemyDead() {
	if (_active_enemies == 0) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function called when no enemies were active" << endl;
	}
	else {
		--_active_enemies;
	}
}



void EnemyZone::Update() {
	// When spawning an enemy in a random zone location, sometimes it is occupied by another
	// object or that section is unwalkable. We try only a few different spawn locations before
	// giving up and waiting for the next call to Update(). Otherwise this function could
	// potentially take a noticable amount of time to complete
	const int8 SPAWN_RETRIES = 5;

	if (_enemies.empty() == true)
		return;

	// Spawn new enemies only if there is at least one enemy that is not active
	if (_active_enemies == _enemies.size())
		return;

	// Update the regeneration timer and return if the spawn time has not yet been reached
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


	uint16 x, y; // Used to retain random position coordinates in the zone
	int8 retries = SPAWN_RETRIES; // Number of times to try finding a valid spawning location
	bool collision; // Holds the result of a collision detection check

	// Select a random position inside the zone to place the spawning enemy
	// If there is a collision, retry a different location
	_enemies[index]->no_collision = false;
	do {
		_RandomPosition(x, y);
		_enemies[index]->SetXPosition(x, 0.0f);
		_enemies[index]->SetYPosition(y, 0.0f);
		collision = MapMode::CurrentInstance()->GetObjectSupervisor()->DetectCollision(_enemies[index], NULL);
	} while (collision && --retries > 0);

	// If we didn't find a suitable spawning location, reset the collision info
	// on the enemy sprite and we will retry on the next call to this function
	if (collision) {
		_enemies[index]->no_collision = true;
	}
	// Otherwise, spawn the enemy and reset the spawn timer
	else {
		_spawn_timer = 0;
		_enemies[index]->ChangeStateSpawning();
		_active_enemies++;
	}
} // void EnemyZone::Update()

// *****************************************************************************
// ********** ContextZone Class Functions
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
	if (section == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function argument was NULL" << endl;
		return;
	}

	_sections.push_back(*section);
	_section_contexts.push_back(context);
	delete section;
}



void ContextZone::Update() {
	int16 index;

	// Check every ground object and determine if its context should be changed by this zone
	for (std::vector<MapObject*>::iterator i = MapMode::CurrentInstance()->GetObjectSupervisor()->_ground_objects.begin();
		i != MapMode::CurrentInstance()->GetObjectSupervisor()->_ground_objects.end(); i++)
	{
		// If the object does not have a context equal to one of the two switching contexts, do not examine it further
		if ((*i)->GetContext() != _context_one && (*i)->GetContext() != _context_two) {
			continue;
		}

		// If the object is inside the zone, set their context to that zone's context
		// (This may result in no change from the object's current context depending on the zone section)
		index = _IsInsideZone(*i);
		if (index >= 0) {
			(*i)->SetContext(_section_contexts[index] ? _context_one : _context_two);
		}
	}
}



int16 ContextZone::_IsInsideZone(MapObject* object) {
	// NOTE: argument is not checked here for performance reasons

	// Check each section of the zone to see if the object is located within
	for (uint16 i = 0; i < _sections.size(); i++) {
		if (object->x_position >= _sections[i].left_col && object->x_position <= _sections[i].right_col &&
			object->y_position >= _sections[i].top_row && object->y_position <= _sections[i].bottom_row)
		{
			return i;
		}
	}
	return -1;
}

} // namespace private_map

} // namespace hoa_map
