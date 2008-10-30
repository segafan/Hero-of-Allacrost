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

#include "map_events.h"

// Allacrost engines
#include "system.h"

using namespace std;

using namespace hoa_system;

namespace hoa_map {

namespace private_map {

// ****************************************************************************
// ********** MoveEvent Class Functions
// ****************************************************************************

MoveEvent::MoveEvent(uint32 event_id, uint32 sprite_id, uint32 x_coord, uint32 y_coord) :
	MapEvent(event_id)
{
	// TODO
}



MoveEvent::~MoveEvent() {
	// TODO
}



void MoveEvent::StartEvent() {
	// TODO
}



bool MoveEvent::IsEventFinished() {
	// TODO
	return true;
}

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



void DialogueEvent::StartEvent() {
	// TODO
}



bool DialogueEvent::IsEventFinished() {
	// TODO
	return true;
}

// ****************************************************************************
// ********** ScriptedEvent Class Functions
// ****************************************************************************

ScriptedEvent::ScriptedEvent(uint32 event_id, uint32 start_func_index, uint32 check_func_index) :
	MapEvent(event_id)
{
	// TODO
}



ScriptedEvent::~ScriptedEvent() {
	if (_start_function != NULL)
		delete _start_function;

	if (_check_function != NULL)
		delete _check_function;
}



void ScriptedEvent::StartEvent() {
	// TODO
}



bool ScriptedEvent::IsEventFinished() {
	// TODO
	return true;
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



void EventSupervisor::BeginEvent(uint32 event_id) {
	MapEvent* event = GetEvent(event_id);
	if (event == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "no event with this ID existed: " << event_id << endl;
		return;
	}

	BeginEvent(event);
}



void EventSupervisor::BeginEvent(MapEvent* event) {
	if (event == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "NULL argument passed to function" << endl;
		return;
	}

	_all_events.insert(make_pair(event->_event_id, event));
	event->StartEvent();
	_ExamineEventLinks(event, true);
}




void EventSupervisor::Update() {
	// Update all launch event timers and start all events whose timers have finished
	for (list<pair<int32, MapEvent*> >::iterator i = _launch_events.begin(); i != _launch_events.end(); i++) {
		i->first -= SystemManager->GetUpdateTime();

		if (i->first <= 0) { // Timer has expired
			MapEvent* start_event = i->second;
			i = _launch_events.erase(i);
			// We begin the event only after it has been removed from the launch list
			BeginEvent(start_event);
		}
	}

	// Check for active events which have finished
	for (list<MapEvent*>::iterator i = _active_events.begin(); i != _active_events.end(); i++) {
		if ((*i)->IsEventFinished() == true) {
			MapEvent* finished_event = *i;
			i = _active_events.erase(i);
			// We examine the event links only after the event has been removed from the active list
			_ExamineEventLinks(finished_event, false);
		}
	}
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
			BeginEvent(link.child_event_id);
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
