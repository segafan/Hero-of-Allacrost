///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_events.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for map mode events and event processing.
*** ***************************************************************************/

// Allacrost engines
#include "mode_manager.h"
#include "script.h"
#include "system.h"
#include "video.h"

// Local map mode headers
#include "map.h"
#include "map_events.h"
#include "map_objects.h"
#include "map_sprites.h"

// Other mode headers
#include "shop.h"

using namespace std;

using namespace hoa_mode_manager;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_video;

using namespace hoa_shop;

namespace hoa_map {

namespace private_map {

// ****************************************************************************
// ********** DialogueEvent Class Functions
// ****************************************************************************

DialogueEvent::DialogueEvent(uint32 event_id, uint32 dialogue_id) :
	MapEvent(event_id, DIALOGUE_EVENT),
	_dialogue_id(dialogue_id)
{}



DialogueEvent::~DialogueEvent()
{}



void DialogueEvent::_Start() {
	MapMode::_current_map->_dialogue_supervisor->BeginDialogue(_dialogue_id);
}



bool DialogueEvent::_Update() {
	MapDialogue* active_dialogue = MapMode::_current_map->_dialogue_supervisor->GetCurrentDialogue();
	if (active_dialogue != NULL && active_dialogue->GetDialogueID() == _dialogue_id)
		return true;
	else
		return false;
}

// ****************************************************************************
// ********** ShopEvent Class Functions
// ****************************************************************************

ShopEvent::ShopEvent(uint32 event_id) :
	MapEvent(event_id, SHOP_EVENT)
{}



ShopEvent::~ShopEvent()
{}


void ShopEvent::AddWare(uint32 object_id) {
	_ware_ids.insert(object_id);
}



void ShopEvent::_Start() {
	ShopMode* shop = new ShopMode();
	for (set<uint32>::iterator i = _ware_ids.begin(); i != _ware_ids.end(); i++) {
		shop->AddObject(*i);
	}
	ModeManager->Push(shop);
}



bool ShopEvent::_Update() {
	return true;
}

// ****************************************************************************
// ********** SoundEvent Class Functions
// ****************************************************************************

SoundEvent::SoundEvent(uint32 event_id) :
	MapEvent(event_id, SOUND_EVENT)
{
	// TODO
}



SoundEvent::~SoundEvent() {
	// TODO
}



void SoundEvent::_Start() {
	// TODO
}



bool SoundEvent::_Update() {
	// TODO
	return true;
}

// ****************************************************************************
// ********** MapTransitionEvent Class Functions
// ****************************************************************************

MapTransitionEvent::MapTransitionEvent(uint32 event_id, std::string filename) :
	MapEvent(event_id, MAP_TRANSITION_EVENT),
	_transition_filename(filename),
	_fade_timer(0)
{}



MapTransitionEvent::~MapTransitionEvent()
{}



void MapTransitionEvent::_Start() {
	MapMode::_current_map->_PushState(STATE_SCENE);
	_fade_timer = 0;
	VideoManager->FadeScreen(Color::black, FADE_OUT_TIME);
	// TODO: fade out the current map music
}



bool MapTransitionEvent::_Update() {
	while (_fade_timer < FADE_OUT_TIME) {
		_fade_timer += SystemManager->GetUpdateTime();
		return false;
	}

	ModeManager->Pop();
	try {
		MapMode *MM = new MapMode(_transition_filename);
		ModeManager->Push(MM);
	} catch (luabind::error e) {
		PRINT_ERROR << "Error loading map: " << _transition_filename << endl;
		ScriptManager->HandleLuaError(e);
	}
	VideoManager->FadeScreen(Color::clear, FADE_OUT_TIME / 2);
	return true;
}

// ****************************************************************************
// ********** JoinPartyEvent Class Functions
// ****************************************************************************

JoinPartyEvent::JoinPartyEvent(uint32 event_id) :
	MapEvent(event_id, JOIN_PARTY_EVENT)
{
	// TODO
}



JoinPartyEvent::~JoinPartyEvent() {
	// TODO
}



void JoinPartyEvent::_Start() {
	// TODO
}



bool JoinPartyEvent::_Update() {
	// TODO
	return true;
}

// ****************************************************************************
// ********** BattleEncounterEvent Class Functions
// ****************************************************************************

BattleEncounterEvent::BattleEncounterEvent(uint32 event_id) :
	MapEvent(event_id, BATTLE_ENCOUNTER_EVENT)
{
	// TODO
}



BattleEncounterEvent::~BattleEncounterEvent() {
	// TODO
}



void BattleEncounterEvent::_Start() {
	// TODO
}



bool BattleEncounterEvent::_Update() {
	// TODO
	return true;
}

// ****************************************************************************
// ********** ScriptedEvent Class Functions
// ****************************************************************************

ScriptedEvent::ScriptedEvent(uint32 event_id, uint32 start_index, uint32 update_index) :
	MapEvent(event_id, SCRIPTED_EVENT)
{
	ReadScriptDescriptor& map_script = MapMode::_current_map->_map_script;
	map_script.OpenTable(MapMode::_current_map->_map_tablespace, true);
	map_script.OpenTable("map_functions");
	_start_function = map_script.ReadFunctionPointer(start_index);
	_update_function = map_script.ReadFunctionPointer(update_index);
	map_script.CloseTable();
	map_script.CloseTable();
}



ScriptedEvent::~ScriptedEvent()
{}



void ScriptedEvent::_Start() {
	ScriptCallFunction<void>(_start_function);
}



bool ScriptedEvent::_Update() {
	return ScriptCallFunction<bool>(_update_function);
}

// ****************************************************************************
// ********** PathMoveSpriteEvent Class Functions
// ****************************************************************************

PathMoveSpriteEvent::PathMoveSpriteEvent(uint32 event_id, VirtualSprite* sprite, uint32 x_coord, uint32 y_coord) :
	SpriteEvent(event_id, PATH_MOVE_SPRITE_EVENT, sprite),
	_source_col(-1),
	_source_row(-1),
	_current_node(0)
{
	// TODO: check that x/y coordinates are within map boundaries
	_destination.col = x_coord;
	_destination.row = y_coord;
}



PathMoveSpriteEvent::~PathMoveSpriteEvent() {
	_path.clear();
}



void PathMoveSpriteEvent::_Start() {
	SpriteEvent::_Start();
	_current_node = 0;

	// If a path already exists and the current position of the sprite is the same as the source position for this path,
	// then we will re-use it and not bother to compute a new path.
	if ((_path.empty() == false) && (_source_col == _sprite->x_position) && (_source_row == _sprite->y_position)) {
		return;
	}

	// Set the source position for this new path to the sprite's current location and compute the new path
	_source_col = _sprite->x_position;
	_source_row = _sprite->y_position;
	if (_source_col < 0 || _source_row < 0) {
		// TODO: Also check if the source position is beyond the maximum row/col map boundaries
		PRINT_ERROR << "sprite position is invalid" << endl;
		return;
	}

	if (MapMode::_current_map->_object_supervisor->FindPath(_sprite, _path, _destination) == true) {
		_sprite->moving = true;
		_SetDirection();
	}
}



bool PathMoveSpriteEvent::_Update() {
	if (_path.empty() == true) {
		PRINT_ERROR << "no path to destination" << endl;
		return true;
	}

	// This condition may happen if a collision halted the sprite's movement
	if (_sprite->moving == false) {
		_sprite->moving = true;
	}

	// Check if the sprite has arrived at the position of the current node
	if (_sprite->x_position == _path[_current_node].col && _sprite->y_position == _path[_current_node].row) {
		_current_node++;

		// When the current node index is at the end of the path, the event is finished
		if (_current_node >= _path.size() - 1) {
			_sprite->moving = false;
			_sprite->ReleaseControl(this);
			return true;
		}
		else {
			_SetDirection();
		}
	}

	return false;
}



void PathMoveSpriteEvent::_SetDirection() {
	uint16 direction = 0;

	if (_sprite->y_position > _path[_current_node].row) { // Need to move north
		direction |= NORTH;
	}
	else if (_sprite->y_position < _path[_current_node].row) { // Need to move south
		direction |= SOUTH;
	}

	if (_sprite->x_position > _path[_current_node].col) { // Need to move west
		direction |= WEST;
	}
	else if (_sprite->x_position < _path[_current_node].col) { // // Need to move east
		direction |= EAST;
	}

	// Determine if the sprite should move diagonally to the next node
	if ((direction & (NORTH | SOUTH)) && (direction & (WEST | EAST))) {
		switch (direction) {
			case (NORTH | WEST):
				direction = MOVING_NORTHWEST;
				break;
			case (NORTH | EAST):
				direction = MOVING_NORTHEAST;
				break;
			case (SOUTH | WEST):
				direction = MOVING_SOUTHWEST;
				break;
			case (SOUTH | EAST):
				direction = MOVING_SOUTHEAST;
				break;
		}
	}

	_sprite->SetDirection(direction);
}



void PathMoveSpriteEvent::_ResolveCollision(COLLISION_TYPE coll_type, MapObject* coll_obj) {
	// Boundary and grid collisions should not occur on a pre-calculated path. If these conditions do occur,
	// we terminate the path event immediately. The conditions may occur if, for some reason, the map's boundaries
	// or collision grid are modified after the path is calculated
	if (coll_type == BOUNDARY_COLLISION || coll_type == GRID_COLLISION) {
		IF_PRINT_WARNING(MAP_DEBUG) << "boundary or grid collision occurred on a pre-calculated path movement" << endl;

		_path.clear(); // This path is obviously not a correct one so we should trash it
		_sprite->ReleaseControl(this);
		MapMode::_current_map->_event_supervisor->TerminateEvent(GetEventID());
		return;
	}

	// If the code has reached this point, then we are dealing with an object collision

	// Determine if the obstructing object is blocking the destination of the path
	bool destination_blocked = MapMode::_current_map->_object_supervisor->IsPositionOccupiedByObject(_destination.row, _destination.col, coll_obj);

	switch (coll_obj->GetObjectType()) {
		case PHYSICAL_TYPE:
		case TREASURE_TYPE:
			// If the object is a static map object and blocking the destination, give up and terminate the event
			if (destination_blocked == true) {
				// Note that we will retain the path (we don't clear() it), hoping that next time the object is moved
				_sprite->ReleaseControl(this);
				MapMode::_current_map->_event_supervisor->TerminateEvent(GetEventID());
			}
			// Otherwise, try to find an alternative path around the object
			else {
				// TEMP: Right now we just give up trying to complete the path because our pathfinding algorithm doesn't account for objects yet
				_sprite->ReleaseControl(this);
				MapMode::_current_map->_event_supervisor->TerminateEvent(GetEventID());

				// TODO: recalculate and find an alternative path around the object
			}
			return;

		case VIRTUAL_TYPE:
		case SPRITE_TYPE:
		case ENEMY_TYPE:
			VirtualSprite* coll_sprite = dynamic_cast<VirtualSprite*>(coll_obj);
			// The object is a sprite and blocking the destination. How we resolve the situation depends upon whether or not the sprite is moving
			if (destination_blocked == true) {
				if (coll_sprite->moving == true) {
					// The obstructing sprite is moving so hopefully it will get out of the way eventually. We will wait for it to do so
				}
				else {
					// The obstructing sprite is not moving so give up trying to reach the destination
					_sprite->ReleaseControl(this);
					MapMode::_current_map->_event_supervisor->TerminateEvent(GetEventID());
				}
			}

			else {
				if (coll_sprite->moving == true) {
					// TEMP: The obstructing sprite is moving so hopefully it will get out of the way eventually. We will wait for it to do so

					// TODO: Re-calculate and find a path around the object
				}

				else {
					// TEMP: The obstructing sprite is not moving so give up trying to reach the destination
					_sprite->ReleaseControl(this);
					MapMode::_current_map->_event_supervisor->TerminateEvent(GetEventID());

					// TODO: Re-calculate and find a path around the object
				}
			}
			return;
	}
} // void PathMoveSpriteEvent::_ResolveCollision(COLLISION_TYPE coll_type, MapObject* coll_obj)

// ****************************************************************************
// ********** RandomMoveSpriteEvent Class Functions
// ****************************************************************************

RandomMoveSpriteEvent::RandomMoveSpriteEvent(uint32 event_id, VirtualSprite* sprite, uint32 move_time, uint32 direction_time) :
	SpriteEvent(event_id, RANDOM_MOVE_SPRITE_EVENT, sprite),
	_total_movement_time(move_time),
	_total_direction_time(direction_time),
	_movement_timer(0),
	_direction_timer(0)
{}



RandomMoveSpriteEvent::~RandomMoveSpriteEvent()
{}



void RandomMoveSpriteEvent::_Start() {
	SpriteEvent::_Start();
	_sprite->SetRandomDirection();
	_sprite->moving = true;
}



bool RandomMoveSpriteEvent::_Update() {
	_direction_timer += SystemManager->GetUpdateTime();
	_movement_timer += SystemManager->GetUpdateTime();

	// Check if we should change the sprite's direction
	if (_direction_timer >= _total_direction_time) {
		_direction_timer -= _total_direction_time;
		_sprite->SetRandomDirection();
	}

	if (_movement_timer >= _total_movement_time) {
		_movement_timer = 0;
		_sprite->moving = false;
		_sprite->ReleaseControl(this);
		return true;
	}

	return false;
}



void RandomMoveSpriteEvent::_ResolveCollision() {
	_sprite->SetRandomDirection();
	_sprite->moving = true;
}

// ****************************************************************************
// ********** AnimateSpriteEvent Class Functions
// ****************************************************************************

AnimateSpriteEvent::AnimateSpriteEvent(uint32 event_id, VirtualSprite* sprite) :
	SpriteEvent(event_id, ANIMATE_SPRITE_EVENT, sprite),
	_current_frame(0),
	_display_timer(0),
	_loop_count(0),
	_number_loops(0)
{}



AnimateSpriteEvent::~AnimateSpriteEvent()
{}



void AnimateSpriteEvent::_Start() {
	SpriteEvent::_Start();
	_current_frame = 0;
	_display_timer = 0;
	_loop_count = 0;
	dynamic_cast<MapSprite*>(_sprite)->SetCurrentAnimation(static_cast<uint8>(_frames[_current_frame]));
}



bool AnimateSpriteEvent::_Update() {
	_display_timer += SystemManager->GetUpdateTime();

	if (_display_timer > _frame_times[_current_frame]) {
		_display_timer = 0;
		_current_frame++;

		// Check if we are past the final frame to display in the loop
		if (_current_frame >= _frames.size()) {
			_current_frame = 0;

			// If this animation is not infinitely looped, increment the loop counter
			if (_number_loops >= 0) {
				_loop_count++;
				if (_loop_count > _number_loops) {
					_loop_count = 0;
					_sprite->ReleaseControl(this);
					return true;
				 }
			}
		}

		dynamic_cast<MapSprite*>(_sprite)->SetCurrentAnimation(static_cast<uint8>(_frames[_current_frame]));
	}

	return false;
}


// ****************************************************************************
// ********** EventSupervisor Class Functions
// ****************************************************************************

EventSupervisor::~EventSupervisor() {
	_active_events.clear();
	_launch_events.clear();

	for (map<uint32, MapEvent*>::iterator i = _all_events.begin(); i != _all_events.end(); i++) {
		delete i->second;
	}
	_all_events.clear();
}



void EventSupervisor::RegisterEvent(MapEvent* new_event) {
	if (new_event == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "function argument was NULL" << endl;
		return;
	}

	if (GetEvent(new_event->_event_id) != NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "event with this ID already existed: " << new_event->_event_id << endl;
		return;
	}

	_all_events.insert(make_pair(new_event->_event_id, new_event));
}



void EventSupervisor::StartEvent(uint32 event_id) {
	MapEvent* event = GetEvent(event_id);
	if (event == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "no event with this ID existed: " << event_id << endl;
		return;
	}

	StartEvent(event);
}



void EventSupervisor::StartEvent(MapEvent* event) {
	if (event == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "NULL argument passed to function" << endl;
		return;
	}

	_active_events.push_back(event);
	event->_Start();
	_ExamineEventLinks(event, true);
}



void EventSupervisor::PauseEvent(uint32 event_id) {
	for (list<MapEvent*>::iterator i = _active_events.begin(); i != _active_events.end(); i++) {
		if ((*i)->_event_id == event_id) {
			_paused_events.push_back(*i);
			_active_events.erase(i);
			return;
		}
	}

	IF_PRINT_WARNING(MAP_DEBUG) << "operation failed because no active event was found corresponding to event id: " << event_id << endl;
}



void EventSupervisor::ResumeEvent(uint32 event_id) {
	for (list<MapEvent*>::iterator i = _paused_events.begin(); i != _paused_events.end(); i++) {
		if ((*i)->_event_id == event_id) {
			_active_events.push_back(*i);
			_paused_events.erase(i);
			return;
		}
	}

	IF_PRINT_WARNING(MAP_DEBUG) << "operation failed because no paused event was found corresponding to event id: " << event_id << endl;
}



void EventSupervisor::TerminateEvent(uint32 event_id) {
	// TODO: what if the event is in the active queue in more than one location?
	for (list<MapEvent*>::iterator i = _active_events.begin(); i != _active_events.end(); i++) {
		if ((*i)->_event_id == event_id) {
			MapEvent* terminated_event = *i;
			i = _active_events.erase(i);
			// We examine the event links only after the event has been removed from the active list
			_ExamineEventLinks(terminated_event, false);
			return;
		}
	}

	IF_PRINT_WARNING(MAP_DEBUG) << "attempted to terminate an event that was not active, id: " << event_id << endl;
}



void EventSupervisor::Update() {
	// Update all launch event timers and start all events whose timers have finished
	for (list<pair<int32, MapEvent*> >::iterator i = _launch_events.begin(); i != _launch_events.end();) {
		i->first -= SystemManager->GetUpdateTime();

		if (i->first <= 0) { // Timer has expired
			MapEvent* start_event = i->second;
			i = _launch_events.erase(i);
			// We begin the event only after it has been removed from the launch list
			StartEvent(start_event);
		}
		else
			++i;
	}

	// Check for active events which have finished
	for (list<MapEvent*>::iterator i = _active_events.begin(); i != _active_events.end();) {
		if ((*i)->_Update() == true) {
			MapEvent* finished_event = *i;
			i = _active_events.erase(i);
			// We examine the event links only after the event has been removed from the active list
			_ExamineEventLinks(finished_event, false);
		}
		else
			++i;
	}
}



bool EventSupervisor::IsEventActive(uint32 event_id) const {
	for (list<MapEvent*>::const_iterator i = _active_events.begin(); i != _active_events.end(); i++) {
		if ((*i)->_event_id == event_id) {
			return true;
		}
	}
	return false;
}



MapEvent* EventSupervisor::GetEvent(uint32 event_id) const {
	map<uint32, MapEvent*>::const_iterator i = _all_events.find(event_id);

	if (i == _all_events.end())
		return NULL;
	else
		return i->second;
}



void EventSupervisor::_ExamineEventLinks(MapEvent* parent_event, bool event_start) {
	for (uint32 i = 0; i < parent_event->_event_links.size(); i++) {
		EventLink& link = parent_event->_event_links[i];

		// Case 1: Start/finish launch member is not equal to the start/finish status of the parent event, so ignore this link
		if (link.launch_at_start != event_start) {
			continue;
		}
		// Case 2: The child event is to be launched immediately
		else if (link.launch_timer == 0) {
			StartEvent(link.child_event_id);
		}
		// Case 3: The child event has a timer associated with it and needs to be placed in the event launch container
		else {
			MapEvent* child = GetEvent(link.child_event_id);
			if (child == NULL) {
				IF_PRINT_WARNING(MAP_DEBUG) << "can not launch child event, no event with this ID existed: " << link.child_event_id << endl;
				continue;
			}
			else {
				_launch_events.push_back(make_pair(static_cast<int32>(link.launch_timer), child));
			}
		}
	}
}

} // namespace private_map

} // namespace hoa_map
