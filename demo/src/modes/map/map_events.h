///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_events.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for map mode events and event processing
***
*** Events occur on map mode to alter the state of the map, present a scene to the
*** player, or do any other custom task we require. Events may be "chained" together
*** so that one event begins as another ends. Many events are scripted, but this
*** file contains some C++ implementations of the most common types of events so
*** that these do not have to be continually re-implemented in every Lua map file.
*** ***************************************************************************/

#ifndef __MAP_EVENTS_HEADER__
#define __MAP_EVENTS_HEADER__

// Allacrost utilities
#include "defs.h"
#include "utils.h"

// Allacrost engines
#include "script.h"

namespace hoa_map {

namespace private_map {

/** ****************************************************************************
*** \brief A container class representing a link between two map events
***
*** Map events may trigger additional events to occur alongside it or following
*** it. This class represents a "link" between two events and describes how the
*** two events are linked. In an event link there is a parent event and a child
*** event. The parent and child events may begin at the same time, or the child
*** event may occur after the parent event starts, but the child will never
*** preceed the parent's start. This class only stores the event_id of the child,
*** and the link object is added as a member onto the parent event's class. When
*** the parent event gets processed, all links are examined and the children events
*** are prepared appropriately.
***
*** We use two pieces of information to determine when to start a child event relevant
*** to its parent. The first is a boolean value that indicates whether the child's
*** start is relative to the parent's start or the parent's finish. The second is a
*** time value that indicates how long to wait (in milliseconds) from the parent's
*** start/finish before starting the child event.
*** ***************************************************************************/
class EventLink {
public:
	EventLink() :
		child_event_id(0), launch_at_start(false), launch_timer(0) {}

	EventLink(uint32 child_id, bool start, uint32 time) :
		child_event_id(child_id), launch_at_start(start), launch_timer(time) {}

	~EventLink()
		{}

	//! \brief The ID of the child event in this link
	uint32 child_event_id;

	//! \brief The event will launch relative to the parent event's start if true, or its finish if false
	bool launch_at_start;

	//! \brief The amount of milliseconds to wait before launching the event (0 means launch instantly)
	uint32 launch_timer;
}; // class EventLink


/** ****************************************************************************
*** \brief An abstract class representing an event that occurs on a map
***
*** An event can be virtually anything from playing a sound to moving a sprite
*** to beginning a dialogue. Events do not necessarily inform the user (though
*** visual or audio means) that an event has occurred. They may be employed to
*** change the state of a map without the player's knowledge. This is an abstract
*** class because common types of events (such as beginning a dialogue) are implemented
*** in C++ code while Lua is used to represent not-so-common types of events.
***
*** All events have a unique non-zero ID unsigned integer value that serve to
*** distinguish the events from one another. Events can also contain any number
*** of "links" to children events, which are events which launch simultaneously
*** with or some time after the parent event. Events are processed via two
*** functions. StartEvent() begins the event, and IsEventFinished() returns true
*** when the event has finished.
*** ***************************************************************************/
class MapEvent {
	friend class EventSupervisor;
public:
	//! \param id The ID for the map event (a zero value is invalid)
	MapEvent(uint32 id) :
		_event_id(id) {}

	~MapEvent()
		{}

	/** \brief Declares a child event to be linked to this event
	*** \param child_event_id The event id of the child event
	*** \param launch_at_start The child starts relative to the start of the event if true, its finish if false
	*** \param launch_timer The number of milliseconds to wait before starting the child event
	**/
	void AddEventLink(uint32 child_event_id, bool launch_at_start, uint32 launch_timer)
		{ _event_links.push_back(EventLink(child_event_id, launch_at_start, launch_timer)); }

	/** \brief Starts the event
	*** This function is only called once per event execution
	**/
	virtual void StartEvent() = 0;

	/** \brief Checks if the event has finished and may also update the event progress
	*** \return True if the event is finished
	*** This function is called as many times as needed until the event has finished. The contents
	*** of this function may do more than simply check if the event is finished. It may also execute
	*** code for the event with the goal of eventually brining the event to a finished state.
	**/
	virtual bool IsEventFinished() = 0;

private:
	//! \brief A unique ID number for the event. A value of zero is invalid
	uint32 _event_id;

	//! \brief All child events of this class, represented by EventLink objects
	std::vector<EventLink> _event_links;
}; // class MapEvent


/** ****************************************************************************
*** \brief An event which moves a single sprite to a destination
***
*** Using event linking, it is very simple to have a single event represent
*** a sprite traveling to multiple destinations, or multiple sprites travel to
*** multiple destinations.
*** ***************************************************************************/
class MoveEvent : public MapEvent {
public:
	/** \param event_id The ID of this event
	*** \param sprite_id The ID of the sprite to move
	*** \param x_coord The X coordinate to move the sprite to
	*** \param y_coord The Y coordinate to move the sprite to
	**/
	MoveEvent(uint32 event_id, uint32 sprite_id, uint32 x_coord, uint32 y_coord);

	~MoveEvent();

	//! \brief Calculates a path for the sprite to move to the destination
	void StartEvent();

	//! \brief Returns true when the sprite has reached the destination
	bool IsEventFinished();

private:
	// TODO
}; // class MoveEvent : public MapEvent


/** ****************************************************************************
*** \brief An event which activates a dialogue on the map
***
*** Note that a dialogue may execute script actions, which would somewhat act
*** like events but technically are not events. Children events that are implemented
*** in Lua can take advantage of options selected by the player in these dialogues
*** to determine what events should follow down the event chain
*** ***************************************************************************/
class DialogueEvent : public MapEvent {
public:
	/** \param event_id The ID of this event
	*** \param dialogue_id The ID of the dialogue to execute through this event
	**/
	DialogueEvent(uint32 event_id, uint32 dialogue_id);

	~DialogueEvent();

	//! \brief Begins the dialogue
	void StartEvent();

	//! \brief Returns true when the last line of the dialogue has been read
	bool IsEventFinished();

private:
	// TODO
}; // class DialogueEvent : public MapEvent


/** ****************************************************************************
*** \brief An event which has its implementation implemented in Lua
***
*** All events that do not fall into the other categories of events will be
*** implemented here. This event uses Lua functions to implement the StartEvent()
*** and IsEventFinished() functions (all these C++ functions do is call the
*** corresponding Lua functions). Note that any type of event can be implemented
*** in Lua, including alternative implementations of the other C++ event types.
*** You should only use this event type if there is no way to implement your
*** event in the other event types provided.
*** ***************************************************************************/
class ScriptedEvent : public MapEvent {
public:
	/** \param event_id The ID of this event
	*** \param start_func_index An index in the map file's function table that references the start function
	*** \param check_func_index An index in the map file's function table that references the check function
	**/
	ScriptedEvent(uint32 event_id, uint32 start_func_index, uint32 check_func_index);

	~ScriptedEvent();

	//! \brief Calls the Lua _start_function
	void StartEvent();

	//! \brief Calls the Lua _check_function
	bool IsEventFinished();

private:
	//! \brief A pointer to the Lua function that starts the event
	ScriptObject* _start_function;

	//! \brief A pointer to the Lua function that returns a boolean value if the event is finished
	ScriptObject* _check_function;
}; // class ScriptedEvent : public MapEvent


/** ****************************************************************************
*** \brief Manages, processes, and launches map events
***
*** The EventSupervisor serves as an assistant to the MapMode class, much like the
*** other map supervisor classes. As such, this class is only created as a member
*** of the MapMode class. The first responsibility of the EventSupervisor is to
*** retain all of the MapEvent objects that have been created. The second responsibility
*** of this class is to initialize and begin the first event in a n-length chain
*** of events, where n can be equal to one or any higher interger value.
***
*** When an event chain begins, the first (base) event of the chain is started.
*** Immediately after starting the first event, the supervisor will examine its event
*** links to determine which, if any, children events begin relative to the start of
*** the base event. If they are to start a certain time after the start of the parent
*** event, they are placed in a container and their countdown timers are initialized.
*** These timers will count down on every update call to the event manager and after
*** the timers expire, these events will be launched. When an active event ends, again
*** its event links are examined to determine if any children events exist that start
*** relative to the end of the parent event.
***
*** \todo What about the case when the same event is begun when the event is already
*** active? Should we prevent the case where an event is activated twice, print a
*** warning, or allow this situation and hope the programmer knows what they are doing?
*** ***************************************************************************/
class EventSupervisor {
public:
	EventSupervisor()
		{}

	~EventSupervisor();

	/** \brief Registers a map event object with the event supervisor
	*** \param new_event A pointer to the new event
	*** \note This function should be called for all events that are created
	**/
	void RegisterEvent(MapEvent* new_event);

	/** \brief Marks a specified event as active and starts the event
	*** \param event_id The ID of the event to activate
	*** The specified event to start may be linked to several children, grandchildren, etc. events.
	*** If the event has no children, it will activate only the single event requested. Otherwise
	*** all events in the chain will become activated at the appropriate time.
	**/
	void BeginEvent(uint32 event_id);

	/** \brief Marks a specified event as active and starts the event
	*** \param event A pointer to the event to begin
	*** The specified event to start may be linked to several children, grandchildren, etc. events.
	*** If the event has no children, it will activate only the single event requested. Otherwise
	*** all events in the chain will become activated at the appropriate time.
	**/
	void BeginEvent(MapEvent* event);

	//! \brief Updates the state of all active and launch events
	void Update();

	//! \brief Returns true if any events are active
	bool HasActiveEvent() const
		{ return !_active_events.empty(); }

	//! \brief Returns true if any events are being prepared to be launched after their timers expire
	bool HasLaunchEvent() const
		{ return !_launch_events.empty(); }

	/** \brief Returns a pointer to a specified event stored by this class
	*** \param event_id The ID of the event to retrieve
	*** \return A MapEvent pointer (which may need to be casted to the proper event type), or NULL if no event was found
	**/
	MapEvent* GetEvent(uint32 event_id) const;

private:
	//! \brief A container for all map events, where the event's ID serves as the key to the std::map
	std::map<uint32, MapEvent*> _all_events;

	//! \brief A list of all events which have started but are not yet finished
	std::list<MapEvent*> _active_events;

	/** \brief A list of all events that are waiting on their launch timers to expire before being started
	*** The interger part of this std::pair is the countdown timer for this event to be launched
	**/
	std::list<std::pair<int32, MapEvent*> > _launch_events;

	/** \brief A function that is called whenever an event starts or finishes to examine that event's links
	*** \param parent_event The event that has just started or finished
	*** \param event_start The event has just started if this member is true, or if it just finished it will be false
	**/
	void _ExamineEventLinks(MapEvent* parent_event, bool event_start);
}; // class EventSupervisor

} // namespace private_map

} // namespace hoa_map

#endif // __MAP_EVENTS_HEADER__
