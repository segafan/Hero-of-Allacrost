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

// Local map mode headers
#include "map_utils.h"
#include "map_sprites.h"

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

	uint32 GetEventID() const
		{ return _event_id; }

	/** \brief Declares a child event to be linked to this event
	*** \param child_event_id The event id of the child event
	*** \param launch_at_start The child starts relative to the start of the event if true, its finish if false
	*** \param launch_timer The number of milliseconds to wait before starting the child event
	**/
	void AddEventLink(uint32 child_event_id, bool launch_at_start, uint32 launch_timer)
		{ _event_links.push_back(EventLink(child_event_id, launch_at_start, launch_timer)); }

protected:
	/** \brief Starts the event
	*** This function is only called once per event execution
	**/
	virtual void _Start() = 0;

	/** \brief Updates the event progress and checks if the event has finished
	*** \return True if the event is finished
	*** This function is called as many times as needed until the event has finished. The contents
	*** of this function may do more than simply check if the event is finished. It may also execute
	*** code for the event with the goal of eventually brining the event to a finished state.
	**/
	virtual bool _Update() = 0;

private:
	//! \brief A unique ID number for the event. A value of zero is invalid
	uint32 _event_id;

	//! \brief All child events of this class, represented by EventLink objects
	std::vector<EventLink> _event_links;
}; // class MapEvent


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

protected:
	//! \brief Begins the dialogue
	void _Start();

	//! \brief Returns true when the last line of the dialogue has been read
	bool _Update();

private:
	// TODO
}; // class DialogueEvent : public MapEvent


/** ****************************************************************************
*** \brief An event with its _Start and _Update functions implemented in Lua.
***
*** All events that do not fall into the other categories of events will be
*** implemented here. This event uses Lua functions to implement the _StartEvent()
*** and _Update() functions (all these C++ functions do is call the
*** corresponding Lua functions). Note that any type of event can be implemented
*** in Lua, including alternative implementations of the other C++ event types.
*** You should only use this event type if there is no way to implement your
*** event in the other event types provided.
*** ***************************************************************************/
class ScriptedEvent : public MapEvent {
public:
	/** \param event_id The ID of this event
	*** \param start_index An index in the map file's function table that references the start function
	*** \param check_index An index in the map file's function table that references the check function
	**/
	ScriptedEvent(uint32 event_id, uint32 start_index, uint32 check_index);

	~ScriptedEvent();

protected:
	//! \brief Calls the Lua _start_function
	void _Start();

	//! \brief Calls the Lua _check_function
	bool _Update();

private:
	//! \brief A pointer to the Lua function that starts the event
	ScriptObject _start_function;

	//! \brief A pointer to the Lua function that returns a boolean value if the event is finished
	ScriptObject _update_function;
}; // class ScriptedEvent : public MapEvent


/** ****************************************************************************
*** \brief An abstract event class that represents an event controlling a sprite
***
*** Sprite events are special types of events that control a sprite (of any type)
*** on a map. Technically they are more like controllers than events, in that they
*** take control of a sprite and direct how its state should change, whether that
*** be their direction, movement, and/or display. All sprite events are connected
*** to one (and only one) sprite. When the event takes control over the sprite,
*** it notifies the sprite object which grabs a pointer to the SpriteEvent.
***
*** For a deriving class to be implemented properly, it must do two things.
*** # In the _Start method, call SpriteEvent::_Start() before any other code
*** # Before returning true in the _Update() method, call _sprite->ReleaseControl(this)
***
*** \todo Should we also have a constructor that takes a sprite's integer ID?
*** ***************************************************************************/
class SpriteEvent : public MapEvent {
public:
	/** \param event_id The ID of this event
	*** \param sprite A pointer to the sprite that this event will control
	**/
	SpriteEvent(uint32 event_id, VirtualSprite* sprite) :
		MapEvent(event_id), _sprite(sprite) {}

	~SpriteEvent()
		{}

protected:
	//! \brief Acquires control of the sprite that the event will operate on
	void _Start()
		{ _sprite->AcquireControl(this); }

	//! \brief Updates the state of the sprite and returns true if the event is finished
	virtual bool _Update() = 0;

	//! \brief A pointer to the map sprite that the event controls
	VirtualSprite* _sprite;
}; // class SpriteEvent : public MapEvent


/** ****************************************************************************
*** \brief An event which moves a single sprite to a destination
***
*** Using event linking, it is very simple to have a single event represent
*** a sprite traveling to multiple destinations, or multiple sprites travel to
*** multiple destinations.
***
*** \todo Should we write a public function to allow the path destination to change?
*** ***************************************************************************/
class PathMoveSpriteEvent : public SpriteEvent {
public:
	/** \param event_id The ID of this event
	*** \param sprite A pointer to the sprite to move
	*** \param x_coord The X coordinate to move the sprite to
	*** \param y_coord The Y coordinate to move the sprite to
	**/
	PathMoveSpriteEvent(uint32 event_id, VirtualSprite* sprite, uint32 x_coord, uint32 y_coord);

	~PathMoveSpriteEvent();

protected:
	//! \brief Calculates a path for the sprite to move to the destination
	void _Start();

	//! \brief Returns true when the sprite has reached the destination
	bool _Update();

	//! \brief The destination coordinates for this path movement
	PathNode _destination;

	//! \brief Holds the path needed to traverse from source to destination
	std::vector<PathNode> _path;

	//! \brief An index to the path vector containing the node that the sprite currently occupies
	uint32 _current_node;
}; // class PathMoveSpriteEvent : public SpriteEvent


/** ****************************************************************************
*** \brief An event which randomizes movement of a sprite
*** ***************************************************************************/
class RandomMoveSpriteEvent : public SpriteEvent {
public:
	/** \param event_id The ID of this event
	*** \param sprite A pointer to the sprite to move
	*** \param move_time The total amount of time that this event should take
	*** \param direction_time The amount of time to wait before changing the sprite's direction randomly
	**/
	RandomMoveSpriteEvent(uint32 event_id, VirtualSprite* sprite, uint32 move_time = 10000, uint32 direction_time = 2000);

	~RandomMoveSpriteEvent();

protected:
	//! \brief Calculates a path for the sprite to move to the destination
	void _Start();

	//! \brief Returns true when the sprite has reached the destination
	bool _Update();

	/** \brief The amount of time (in milliseconds) to perform random movement before ending this action
	*** Set this member to hoa_system::INFINITE_TIME in order to continue the random movement
	*** forever. The default value of this member will be set to 10 seconds if it is not specified.
	**/
	uint32 _total_movement_time;

	/** \brief The amount of time (in milliseconds) that the sprite should continue moving in its current direction
	*** The default value for this timer is 1.5 seconds (1500ms).
	**/
	uint32 _total_direction_time;

	//! \brief A timer which keeps track of how long the sprite has been in random movement
	uint32 _movement_timer;

	//! \brief A timer which keeps track of how long the sprite has been moving around since the last change in direction.
	uint32 _direction_timer;
}; // class RandomMoveSpriteEvent : public SpriteEvent


/** ****************************************************************************
*** \brief Displays specific sprite frames for a certain period of time
***
*** This event displays a certain animation of a sprite for a specified amount of time.
*** Its primary purpose is to allow complete control over how a sprite appears to the
*** player and to show the sprite interacting with its surroundings, such as flipping
*** through a book taken from a bookshelf. Looping of these animations is also supported.
***
*** \note You <b>must</b> add at least one frame to this object
***
*** \note These actions can not be used with VirtualSprite objects, since this
*** class explicitly needs animation images to work and virtual sprites have no
*** images.
*** ***************************************************************************/
class AnimateSpriteEvent : public SpriteEvent {
public:
	/** \param event_id The ID of this event
	*** \param sprite A pointer to the sprite to move
	**/
	AnimateSpriteEvent(uint32 event_id, VirtualSprite* sprite);

	~AnimateSpriteEvent();

	/** \brief Adds a new frame to the animation set
	*** \param frame The index of the sprite's animations to display
	*** \param time The amount of time, in milliseconds, to display this frame
	**/
	void AddFrame(uint16 frame, uint32 time)
		{ _frames.push_back(frame); _frame_times.push_back(time); }

	/** \brief Sets the loop
	***
	**/
	void SetLoopCount(int32 count)
		{ _loop_count = count; }

protected:
	//! \brief Calculates a path for the sprite to move to the destination
	void _Start();

	//! \brief Returns true when the sprite has reached the destination
	bool _Update();

	//! \brief Index to the current frame to display from the frames vector
	uint32 _current_frame;

	//! \brief Used to count down the display time of the current frame
	uint32 _display_timer;

	//! \brief A counter for the number of animation loops that have been performed
	int32 _loop_count;

	/** \brief The number of times to loop the display of the frame set before finishing
	*** A value less than zero indicates to loop forever. Be careful with this,
	*** because that means that the action would never arrive at the "finished"
	*** state.
	***
	*** \note The default value of this member is zero, which indicates that the
	*** animations will not be looped (they will run exactly once to completion).
	**/
	int32 _number_loops;

	/** \brief Holds the sprite animations to display for this action
	*** The values contained here are indeces to the sprite's animations vector
	**/
	std::vector<uint16> _frames;

	/** \brief Indicates how long to display each frame
	*** The size of this vector should be equal to the size of the frames vector
	**/
	std::vector<uint32> _frame_times;
}; // class AnimateSpriteEvent : public SpriteEvent


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
	void StartEvent(uint32 event_id);

	/** \brief Marks a specified event as active and starts the event
	*** \param event A pointer to the event to begin
	*** The specified event to start may be linked to several children, grandchildren, etc. events.
	*** If the event has no children, it will activate only the single event requested. Otherwise
	*** all events in the chain will become activated at the appropriate time.
	**/
	void StartEvent(MapEvent* event);

	/** \brief Pauses an active event by preventing the event from updating
	*** \param event_id The ID of the active event to pause
	*** If the event corresponding to the ID is not active, a warning will be issued and no change
	*** will occur.
	**/
	void PauseEvent(uint32 event_id);

	/** \brief Resumes a pausd evend
	*** \param event_id The ID of the active event to resume
	*** If the event corresponding to the ID is not paused, a warning will be issued and no change
	*** will occur.
	**/
	void ResumeEvent(uint32 event_id);

	/** \brief Terminates an event if it is active
	*** \param event_id The ID of the event to terminate
	*** \note If there is no active event that corresponds to the event ID, the function will do nothing.
	*** \note This function will <b>not</b> terminate any of the event's children. All children that launch from this
	*** event's start will remain in the active or launch event containers. Any children that launch after the event's
	*** finish will not be processed.
	*** \note Use of this function is atypical and should be avoided. Termination of certain events before their completion
	*** can lead to memory leaks, errors, and other problems. Make sure that the event you are terminating will not cause
	*** any of these conditions.
	**/
	void TerminateEvent(uint32 event_id);

	//! \brief Updates the state of all active and launch events
	void Update();

	/** \brief Determines if a chosen event is active
	*** \param event_id The ID of the event to check
	*** \return True if the event is active, false if it is not or the event could not be found
	**/
	bool IsEventActive(uint32 event_id) const;

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

	//! \brief A list of all events which have been paused
	std::list<MapEvent*> _paused_events;

	/** \brief A function that is called whenever an event starts or finishes to examine that event's links
	*** \param parent_event The event that has just started or finished
	*** \param event_start The event has just started if this member is true, or if it just finished it will be false
	**/
	void _ExamineEventLinks(MapEvent* parent_event, bool event_start);
}; // class EventSupervisor

} // namespace private_map

} // namespace hoa_map

#endif // __MAP_EVENTS_HEADER__