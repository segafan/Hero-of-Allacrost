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
#include "script.h"
#include "system.h"

// Local map mode headers
#include "map.h"
#include "map_events.h"
#include "map_objects.h"
#include "map_sprites.h"

using namespace std;

using namespace hoa_script;
using namespace hoa_system;

namespace hoa_map {

namespace private_map {

// ****************************************************************************
// ********** DialogueEvent Class Functions
// ****************************************************************************

DialogueEvent::DialogueEvent(uint32 event_id, uint32 dialogue_id) :
	MapEvent(event_id)
{
	// TODO
}



DialogueEvent::~DialogueEvent() {
	// TODO
}



void DialogueEvent::_Start() {
	// TODO
}



bool DialogueEvent::_Update() {
	// TODO
	return true;
}

// ****************************************************************************
// ********** ScriptedEvent Class Functions
// ****************************************************************************

ScriptedEvent::ScriptedEvent(uint32 event_id, uint32 start_index, uint32 update_index) :
	MapEvent(event_id)
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
	SpriteEvent(event_id, sprite),
	_current_node(0)
{
	// TODO: check that x/y coordinates are within map boundaries
	_destination.col = x_coord;
	_destination.row = y_coord;
}



PathMoveSpriteEvent::~PathMoveSpriteEvent() {
	// TODO
}



void PathMoveSpriteEvent::_Start() {
	SpriteEvent::_Start();
	_current_node = 0;
	// TODO: Check if we already have a previously computed path and if it is still valid, use it.
	// The code below automatically re-uses a path if there is one without checking if the source
	// node is the same as the sprite's current position

	if (_path.empty() == true) {
		MapMode::_current_map->_object_supervisor->FindPath(_sprite, _path, _destination);

		// If no path could be found, there's nothing more that can be done here
		if (_path.empty() == true)
			IF_PRINT_WARNING(MAP_DEBUG) << "could not discover a path to destination" << endl;
	}
}



bool PathMoveSpriteEvent::_Update() {
	// TODO: the code below needs to be optimized. We should only be doing the directional
	// readjustment after the sprite has reached the next node

	_sprite->moving = true;
	if (_sprite->y_position > _path[_current_node].row) { // Need to move toward the north
		if (_sprite->x_position > _path[_current_node].col)
			_sprite->SetDirection(MOVING_NORTHWEST);
		else if (_sprite->x_position < _path[_current_node].col)
			_sprite->SetDirection(MOVING_NORTHEAST);
		else
			_sprite->SetDirection(NORTH);
	}
	else if (_sprite->y_position < _path[_current_node].row) { // Need to move toward the south
		if (_sprite->x_position > _path[_current_node].col)
			_sprite->SetDirection(MOVING_SOUTHWEST);
		else if (_sprite->x_position < _path[_current_node].col)
			_sprite->SetDirection(MOVING_SOUTHEAST);
		else
			_sprite->SetDirection(SOUTH);
	}
	else if (_sprite->x_position > _path[_current_node].col) { // Need to move west
		_sprite->SetDirection(WEST);
	}
	else if (_sprite->x_position < _path[_current_node].col) { // Need to move east
		_sprite->SetDirection(EAST);
	}
	else { // The x and y position have reached the node, update to the next node
		_current_node++;
		if (_current_node >= _path.size()) { // Destination has been reached
			_sprite->moving = false;
			_sprite->ReleaseControl(this);
			return true;
		}
	}

	return false;
}

// ****************************************************************************
// ********** RandomMoveSpriteEvent Class Functions
// ****************************************************************************

RandomMoveSpriteEvent::RandomMoveSpriteEvent(uint32 event_id, VirtualSprite* sprite, uint32 move_time, uint32 direction_time) :
	SpriteEvent(event_id, sprite),
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

// ****************************************************************************
// ********** AnimateSpriteEvent Class Functions
// ****************************************************************************

AnimateSpriteEvent::AnimateSpriteEvent(uint32 event_id, VirtualSprite* sprite) :
	SpriteEvent(event_id, sprite),
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
			_active_events.erase(i);
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
				IF_PRINT_WARNING(MAP_DEBUG) << "can not launch child event; no event with this ID existed: " << link.child_event_id << endl;
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
