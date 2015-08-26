///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2015 by The Allacrost Project
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
*** to beginning a dialogue. Events do not necessarily inform the user (through
*** visual or audio means) that an event has occurred. They may be employed to
*** change the state of a map without the player's knowledge. This is an abstract
*** class because common types of events (such as beginning a dialogue) are implemented
*** in C++ code while Lua is used to represent not-so-common types of events.
***
*** All events have a unique, non-zero, unsigned integer value that serve to
*** distinguish the events from one another (an ID number). Events can also contain any
*** number of "links" to children events, which are events which launch simultaneously
*** with or some time after the parent event. Events are processed via two
*** functions. _Start() is called only one when the event begins. _Update() is called
*** once for every iteration of the main game loop until this function returns a true
*** value, indicating that the event is finished.
*** ***************************************************************************/
class MapEvent {
	friend class EventSupervisor;
public:
	uint32 GetEventID() const
		{ return _event_id; }

	EVENT_TYPE GetEventType() const
		{ return _event_type; }

	/** \brief Declares a child event to be launched immediately at the start of this event
	*** \param child_event_id The event id of the child event
	**/
	void AddEventLinkAtStart(uint32 child_event_id)
		{ _AddEventLink(child_event_id, true, 0); }

	/** \brief Declares a child event to be launched after the start of this event
	*** \param child_event_id The event id of the child event
	*** \param launch_time The number of milliseconds to wait before launching the child event
	**/
	void AddEventLinkAtStart(uint32 child_event_id, uint32 launch_time)
		{ _AddEventLink(child_event_id, true, launch_time); }

	/** \brief Declares a child event to be launched immediately at the end of this event
	*** \param child_event_id The event id of the child event
	**/
	void AddEventLinkAtEnd(uint32 child_event_id)
		{ _AddEventLink(child_event_id, false, 0); }

	/** \brief Declares a child event to be launched after the end of this event
	*** \param child_event_id The event id of the child event
	*** \param launch_time The number of milliseconds to wait before launching the child event
	**/
	void AddEventLinkAtEnd(uint32 child_event_id, uint32 launch_time)
		{ _AddEventLink(child_event_id, false, launch_time); }

protected:
	//! \param id The ID for the map event (a zero value is invalid)
	MapEvent(uint32 id, EVENT_TYPE type) :
		_event_id(id), _event_type(type) {}

	virtual ~MapEvent()
		{}

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

	/** \brief Declares a child event to be linked to this event
	*** \param child_event_id The event id of the child event
	*** \param launch_at_start The child starts relative to the start of the event if true, its finish if false
	*** \param launch_time The number of milliseconds to wait before launching the child event
	**/
	void _AddEventLink(uint32 child_event_id, bool launch_at_start, uint32 launch_time)
		{ _event_links.push_back(EventLink(child_event_id, launch_at_start, launch_time)); }

private:
	//! \brief A unique ID number for the event. A value of zero is invalid
	uint32 _event_id;

	//! \brief Identifier for the class type of this event
	EVENT_TYPE _event_type;

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
***
*** Sometimes you may want a dialogue event to stop the camera from moving, especially
*** if it is the first event in an event chain. When this behavior is desired, call the
*** StopCameraMovement() method after creating the event object.
*** ***************************************************************************/
class DialogueEvent : public MapEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param dialogue_id The ID of the dialogue to execute through this event
	*** \return A pointer to the instance of the event created
	**/
	static DialogueEvent* Create(uint32 event_id, uint32 dialogue_id);

	//! \brief Toggles whether or not camera movement should be stopped when the dialogue begins
	void SetStopCameraMovement(bool stop)
		{ _stop_camera_movement = stop; }

protected:
	DialogueEvent(uint32 event_id, uint32 dialogue_id);

	~DialogueEvent()
		{}

	//! \brief The ID of the dialogue to invoke
	uint32 _dialogue_id;

	//! \brief When true, any camera movement will be stopped when the event begins
	bool _stop_camera_movement;

	//! \brief Begins the dialogue
	void _Start();

	//! \brief Returns true when the last line of the dialogue has been read
	bool _Update();
}; // class DialogueEvent : public MapEvent


/** ****************************************************************************
*** \brief An event that creates an instance of ShopMode when started
***
*** \todo Several future shop mode features will likely need to be added to this
*** class. This includes limited availability of objects, market pricing, etc.
*** ***************************************************************************/
class ShopEvent : public MapEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \return A pointer to the instance of the event created
	**/
	static ShopEvent* Create(uint32 event_id);

	/** \brief Adds a ware to the list of objects for sale
	*** \param object_id The ID of the GlobalObject to make available for purchase
	*** \param stock The amount of the object to make available for purchase
	*** \note All wares must be added before the _Start() method is called to ensure
	*** that the wares actually appear in shop mode.
	**/
	void AddWare(uint32 object_id, uint32 stock);

protected:
	//! \param event_id The ID of this event
	ShopEvent(uint32 event_id);

	~ShopEvent()
		{}

	//! \brief The GlobalObject IDs and stock count of all objects to be sold in the shop
	std::set<std::pair<uint32, uint32> > _wares;

	//! \brief Creates an instance of ShopMode and pushes it to the game mode stack
	void _Start();

	//! \brief Performs no operation (returns true)
	bool _Update();
}; // class ShopEvent : public MapEvent


/** ****************************************************************************
*** \brief Plays a sound. The event finishes when the sound stops
***
*** The suggested usage for initializing an object of this class is the following:
*** -# Call the class constructor
*** -# Call the GetSound() function to retrieve the SoundDescriptor object
*** -# Call SoundDescriptor methods to set the desired properties of the sound (looping, attenuation, etc)
***
*** After these steps are performed the event is ready to launch. The default properties
*** of the sound are the same as are in the default constructor of the SoundDescriptor
*** class. This includes no looping and no distance attenuation. The event will finish when the
*** sound finishes playing (when the sound state is AUDIO_STATE_STOPPED). Note that if looping is set
*** to infinite, the sound will never enter this state. It is possible to prematurely terminate this
*** event by calling the GetSound() method and invoking Stop() on the SoundDescriptor object that is
*** returned.
***
*** \note The MapMode class has a container of SoundDescriptor objects which should include all of
*** the sounds that may be used on a given map. This means that when a SoundEvent is created, the
*** sound file data will already be loaded by the audio engine.
***
*** \todo Support sounds with a position that employ distance attenuation. Perhaps
*** another derived class would be ideal to implement this, since sounds could possibly
*** be mobile (attached to sprites).
*** ***************************************************************************/
class SoundEvent : public MapEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sound_filename The name of the sound file to load
	*** \return A pointer to the instance of the event created
	**/
	static SoundEvent* Create(uint32 event_id, std::string sound_filename);

	//! \brief Accessor which allows the properties of the sound to be customized
	hoa_audio::SoundDescriptor& GetSound()
		{ return _sound; }

protected:
	SoundEvent(uint32 event_id, std::string sound_filename);

	~SoundEvent();

	//! \brief The sound that this event will play
	hoa_audio::SoundDescriptor _sound;

	//! \brief Begins playback of the sound
	void _Start();

	//! \brief Returns true when the sound has finished playing, or finished looping
	bool _Update();
}; // class SoundEvent : public MapEvent


/** ****************************************************************************
*** \brief Event for switching from one map to another, with fading.
***
***
*** ***************************************************************************/
class MapTransitionEvent : public MapEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param filename The name of the map file to transition to
	*** \return A pointer to the instance of the event created
	**/
	static MapTransitionEvent* Create(uint32 event_id, std::string filename);

protected:
	MapTransitionEvent(uint32 event_id, std::string filneame);

	~MapTransitionEvent()
		{}

	//! \brief The filename of the map to transition to
	std::string _transition_map_filename;

	//! \brief A timer used for fading out the current map
	hoa_system::SystemTimer _fade_timer;

	//! \brief Begins the transition process by fading out the screen and music
	void _Start();

	//! \brief Once the fading process completes, creates the new map mode to transition to
	bool _Update();
}; // class MapTransitionEvent : public MapEvent


/** ****************************************************************************
*** \brief
***
***
*** ***************************************************************************/
class JoinPartyEvent : public MapEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \return A pointer to the instance of the event created
	**/
	static JoinPartyEvent* Create(uint32 event_id);

protected:
	/** \param event_id The ID of this event
	**/
	JoinPartyEvent(uint32 event_id);

	~JoinPartyEvent();

	//! \brief
	void _Start();

	//! \brief
	bool _Update();
}; // class JoinPartyEvent : public MapEvent


/** ****************************************************************************
*** \brief Instantly starts a battle.
***
***
*** ***************************************************************************/
class BattleEncounterEvent : public MapEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \return A pointer to the instance of the event created
	**/
	static BattleEncounterEvent* Create(uint32 event_id);

	void SetMusic(std::string filename);

	void SetBackground(std::string filename);

	void AddEnemy(uint32 enemy_id);

protected:
	BattleEncounterEvent(uint32 event_id);

	~BattleEncounterEvent();

	//! \brief ID numbers for enemies to generate
	std::vector<uint32> _enemy_ids;

	//! \brief Filename for battle music
	std::string _battle_music;

	//! \brief Filename for battle background
	std::string _battle_background;

	//! \brief Starts the battle
	void _Start();

	//! \brief Currently does nothing
	bool _Update();
}; // class BattleEncounterEvent : public MapEvent


/** ****************************************************************************
*** \brief An event with its _Start() and _Update() functions implemented in Lua.
***
*** All events that do not fall into the other categories of events will be
*** implemented here. This event uses Lua functions to implement the _Start()
*** and _Update() functions (the C++ functions implemented here just call the
*** corresponding Lua functions). Note that any type of event can be implemented
*** in Lua, including alternative implementations of the other C++ event types.
*** You should use this event type only when the other event classes do not meet
*** your needs.
*** ***************************************************************************/
class CustomEvent : public MapEvent {
public:
	CustomEvent(const CustomEvent& copy);

	CustomEvent& operator=(const CustomEvent& copy);

	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param start_name The name of the start function to call
	*** \param update_name The name of the update function to call
	*** \return A pointer to the instance of the event created
	***
	*** \note Passing an empty string for either the start_name or update_name will result in
	*** no corresponding function defined. If no update function is defined, the call to _Update()
	*** will always return true, meaning that this event will end immediately after it starts.
	*** If both names are given empty string arguments, the event effectively does nothing and a
	*** warning message is printed out for this case.
	**/
	static CustomEvent* Create(uint32 event_id, std::string start_name, std::string update_name);

protected:
	CustomEvent(uint32 event_id, std::string start_name, std::string update_name);

	~CustomEvent();

	//! \brief A pointer to the Lua function that starts the event
	ScriptObject* _start_function;

	//! \brief A pointer to the Lua function that updates the status of an event, returning true when the event is finished
	ScriptObject* _update_function;

	//! \brief Calls the Lua _start_function if one was defined
	void _Start();

	//! \brief Calls the Lua _update_function. If no update function was defined, immediately returns true
	bool _Update();
}; // class CustomEvent : public MapEvent


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
*** \note It is important to keep in mind that all map sprites have their update
*** function called before map events are updated. This can have implications for
*** changing some members of the sprite object inside the _Start() and _Update() methods
*** as these methods are called <i>after</i> the sprite's own Update() method. Keep
*** this property in mind when designing a derived sprite event class.
*** ***************************************************************************/
class SpriteEvent : public MapEvent {
protected:
	/** \param event_id The ID of this event
	*** \param event_type The type of this event
	*** \param sprite A pointer to the sprite that this event will control
	**/
	SpriteEvent(uint32 event_id, EVENT_TYPE event_type, VirtualSprite* sprite);

	virtual ~SpriteEvent()
		{}

	//! \brief A pointer to the map sprite that the event controls
	VirtualSprite* _sprite;

	//! \brief Acquires control of the sprite that the event will operate on
	virtual void _Start()
		{ _sprite->AcquireControl(this); }

	//! \brief Updates the state of the sprite and returns true if the event is finished
	virtual bool _Update() = 0;
}; // class SpriteEvent : public MapEvent


/** ****************************************************************************
*** \brief A simple event used to set the direction of a sprite
***
*** This event finishes immediately after it starts, as all that it performs is
*** to set the direction of a sprite in a particular orientation. Normally such
*** a minor event would be better suited as a CustomEvent with no update function,
*** but because a set direction operation is so common, it makes sense to create a
*** specific event for it for convenience.
***
*** \note The only directions you should set in the class constructor are: NORTH,
*** SOUTH, EAST, and WEST. This event is used when a sprite is stationary, so
*** the other types of directions (which also infer movement) are unnecessary.
*** Using a direction other than these four will result in a warning being printed.
*** ***************************************************************************/
class ChangeDirectionSpriteEvent : public SpriteEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite A pointer to the sprite to enact the event on
	*** \param direction The direction in which to turn the sprite
	*** \return A pointer to the instance of the event created
	**/
	static ChangeDirectionSpriteEvent* Create(uint32 event_id, VirtualSprite* sprite, uint16 direction);

	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite_id The ID of the sprite to enact the event on
	*** \param direction The direction in which to turn the sprite
	*** \return A pointer to the instance of the event created
	**/
	static ChangeDirectionSpriteEvent* Create(uint32 event_id, uint16 sprite_id, uint16 direction);

protected:
	ChangeDirectionSpriteEvent(uint32 event_id, VirtualSprite* sprite, uint16 direction);

	~ChangeDirectionSpriteEvent()
		{}

	//! \brief Retains the direction to move the sprite when the event starts
	uint16 _direction;

	//! \brief Immediately changes the sprite's direction
	void _Start();

	//! \brief Always returns true immediately, terminating the event
	bool _Update();
}; // class ChangeDirectionSpriteEvent : public SpriteEvent


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
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite_id The ID of the sprite to enact the event on
	*** \param sprite A pointer to the sprite to enact the event on
	*** \return A pointer to the instance of the event created
	**/
	static AnimateSpriteEvent* Create(uint32 event_id, VirtualSprite* sprite);

	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite_id The ID of the sprite to enact the event on
	*** \return A pointer to the instance of the event created
	**/
	static AnimateSpriteEvent* Create(uint32 event_id, uint16 sprite_id);

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
	/** \param event_id The ID of this event
	*** \param sprite A pointer to the sprite to move
	**/
	AnimateSpriteEvent(uint32 event_id, VirtualSprite* sprite);

	~AnimateSpriteEvent();

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

	//! \brief Calculates a path for the sprite to move to the destination
	void _Start();

	//! \brief Returns true when the sprite has reached the destination
	bool _Update();
}; // class AnimateSpriteEvent : public SpriteEvent


/** ****************************************************************************
*** \brief An event which randomizes movement of a sprite
*** ***************************************************************************/
class RandomMoveSpriteEvent : public SpriteEvent {
	friend class VirtualSprite;

public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite A pointer to the sprite to enact the event on
	*** \param move_time The total amount of time that this event should take
	*** \param direction_time The amount of time to wait before changing the sprite's direction randomly
	*** \return A pointer to the instance of the event created
	**/
	static RandomMoveSpriteEvent* Create(uint32 event_id, VirtualSprite* sprite, uint32 move_time, uint32 direction_time);

	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite_id The ID of the sprite to enact the event on
	*** \param move_time The total amount of time that this event should take
	*** \param direction_time The amount of time to wait before changing the sprite's direction randomly
	*** \return A pointer to the instance of the event created
	**/
	static RandomMoveSpriteEvent* Create(uint32 event_id, uint16 sprite_id, uint32 move_time, uint32 direction_time);

protected:
	RandomMoveSpriteEvent(uint32 event_id, VirtualSprite* sprite, uint32 move_time, uint32 direction_time);

	~RandomMoveSpriteEvent();

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

	//! \brief Calculates a path for the sprite to move to the destination
	void _Start();

	//! \brief Returns true when the sprite has reached the destination
	bool _Update();

	/** \brief Tries to adjust the sprite's position around the collision. Will randomally change the sprite's direction if that fails.
	*** \param coll_type The type of collision that has occurred
	*** \param coll_obj A pointer to the MapObject that the sprite has collided with, if any
	**/
	void _ResolveCollision(COLLISION_TYPE coll_type, MapObject* coll_obj);
}; // class RandomMoveSpriteEvent : public SpriteEvent


/** ****************************************************************************
*** \brief An event which moves a sprite to a destination
***
*** This class allows for both absolute and relative destinations. Absolute destinations
*** are defined by specifying an X,Y coordinate on the map to move the sprite to. A relative
*** destination is the change in the X and Y directions to move the sprite from their current
*** position. The default destination type is absolute.
***
*** Using event linking, it is very simple to create an event chain where a sprite
*** travels between multiple destinations, or multiple sprites travel to multiple
*** destinations.
*** ***************************************************************************/
class PathMoveSpriteEvent : public SpriteEvent {
	friend class VirtualSprite;

public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite A pointer to the sprite to move
	*** \param x_coord The X coordinate to move the sprite to
	*** \param y_coord The Y coordinate to move the sprite to
	*** \return A pointer to the instance of the event created
	**/
	static PathMoveSpriteEvent* Create(uint32 event_id, VirtualSprite* sprite, int16 x_coord, int16 y_coord);

	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite_id The ID of the sprite that is to be moved
	*** \param x_coord The X coordinate to move the sprite to
	*** \param y_coord The Y coordinate to move the sprite to
	*** \return A pointer to the instance of the event created
	**/
	static PathMoveSpriteEvent* Create(uint32 event_id, uint16 sprite_id, int16 x_coord, int16 y_coord);

	/** \brief Used to toggle whether or not the destination provided in the constructor is relative or absolute
	*** \note Any previous existing paths are cleared when this function is called. If this function is called when
	*** the event is active, no change will take place.
	**/
	void SetRelativeDestination(bool relative);

	/** \brief Used to change the destination coordinates after the class object has been constructed
	*** \param x_coord The X coordinate to move the sprite to
	*** \param y_coord The Y coordinate to move the sprite to
	*** \note Any previous existing paths are cleared when this function is called. If this function is called when
	*** the event is active, no change will take place.
	**/
	void SetDestination(int16 x_coord, int16 y_coord);

	/** \brief Optionally indicates the facing direction to set for the sprite after the movement is complete
	*** \note The only directions you should set in the class constructor are: NORTH, SOUTH, EAST, and WEST.
	*** The other types of directions (which also infer movement) are unnecessary. Using a direction other than
	*** these four will result in a warning being printed.
	**/
	void SetFinalDirection(uint16 direction);

protected:
	PathMoveSpriteEvent(uint32 event_id, VirtualSprite* sprite, int16 x_coord, int16 y_coord);

	~PathMoveSpriteEvent()
		{}

	//! \brief When true, the destination coordinates are relative to the current position of the sprite. Otherwise the destination is absolute.
	bool _relative_destination;

	//! \brief Stores the source coordinates for the path movement (the sprite's position when the event is started).
	int16 _source_col, _source_row;

	//! \brief Stores the destination coordinates for the path movement. These may be either absolute or relative coordinates.
	int16 _destination_col, _destination_row;

	//! \brief Used to store the previous coordinates of the sprite during path movement, so as to set the proper direction of the sprite as it moves
	uint16 _last_x_position, _last_y_position;

	//! \brief Allows the ability of the event to optionally set the direction that the sprite should face after completing the path movement
	uint16 _final_direction;

	//! \brief An index to the path vector containing the node that the sprite currently occupies
	uint32 _current_node;

	//! \brief Holds the final destination coordinates for the path movement
	PathNode _destination_node;

	//! \brief Holds the path needed to traverse from source to destination
	std::vector<PathNode> _path;

	//! \brief Calculates a path for the sprite to move to the destination
	void _Start();

	//! \brief Returns true when the sprite has reached the destination
	bool _Update();

	//! \brief Sets the correct direction for the sprite to move to the next node in the path
	void _SetSpriteDirection();

	/** \brief Determines an appropriate resolution when the sprite collides with an obstruction
	*** \param coll_type The type of collision that has occurred
	*** \param coll_obj A pointer to the MapObject that the sprite has collided with, if any
	**/
	void _ResolveCollision(COLLISION_TYPE coll_type, MapObject* coll_obj);
}; // class PathMoveSpriteEvent : public SpriteEvent


/** ****************************************************************************
*** \brief A custom event which operates on a sprite
***
*** This class is a cross between a SpriteEvent and CustomEvent class. The class
*** inherits from SpriteEvent, but not from CustomEvent. The key feature of this
*** class is that it passes a pointer to a VirtualSprite object in the argument
*** list when it makes its Lua function calls. The Lua functions are then able
*** to take any allowable action on the sprite object. Otherwise, this class
*** behaves just like a standard CustomEvent class.
*** ***************************************************************************/
class CustomSpriteEvent : public SpriteEvent {
public:
	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite A pointer to the sprite that will be passed to the Lua script functions
	*** \param start_name The name of the start function to call
	*** \param update_name The name of the update function to call
	*** \return A pointer to the instance of the event created
	***
	*** \note Passing an empty string for either the start_name or update_name will result in
	*** no corresponding function defined. If no update function is defined, the call to _Update()
	*** will always return true, meaning that this event will end immediately after it starts.
	*** If both names are given empty string arguments, the event effectively does nothing and a
	*** warning message is printed out for this case.
	**/
	static CustomSpriteEvent* Create(uint32 event_id, VirtualSprite* sprite, std::string start_name, std::string update_name);

	/** \brief Creates an instance of the class and registers it with the event supervisor
	*** \param event_id The ID of this event
	*** \param sprite_id The id of the sprite that will be passed to the Lua script functions
	*** \param start_name The name of the start function to call
	*** \param update_name The name of the update function to call
	*** \return A pointer to the instance of the event created
	***
	*** \note Passing an empty string for either the start_name or update_name will result in
	*** no corresponding function defined. If no update function is defined, the call to _Update()
	*** will always return true, meaning that this event will end immediately after it starts.
	*** If both names are given empty string arguments, the event effectively does nothing and a
	*** warning message is printed out for this case.
	**/
	static CustomSpriteEvent* Create(uint32 event_id, uint16 sprite_id, std::string start_name, std::string update_name);

	CustomSpriteEvent(const CustomSpriteEvent& copy);

	CustomSpriteEvent& operator=(const CustomSpriteEvent& copy);

protected:
	CustomSpriteEvent(uint32 event_id, VirtualSprite* sprite, std::string start_name, std::string check_name);

	~CustomSpriteEvent();

	//! \brief A pointer to the Lua function that starts the event
	ScriptObject* _start_function;

	//! \brief A pointer to the Lua function that returns a boolean value if the event is finished
	ScriptObject* _update_function;

	//! \brief Calls the Lua _start_function, if one was defined
	void _Start();

	//! \brief Calls the Lua _update_function. If no update function was defined, does nothing and returns true
	bool _Update();
}; // class CustomSpriteEvent : public SpriteEvent


/** ****************************************************************************
*** \brief Manages, processes, and launches map events
***
*** The EventSupervisor serves as an assistant to the MapMode class, much like the
*** other map supervisor classes. As such, this class is only instantiated as a member
*** of the MapMode class. The first responsibility of the EventSupervisor is to
*** retain all of the MapEvent objects that have been created. The second responsibility
*** of this class is to start and run events to their completion. Because events may be
*** linked together in a chain, starting one event may cause several other chains to start
*** as well. Each event can be thought of as the base of a n-length event chain, where n
*** is an integer greater than or equal to one.
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

	/** \brief Marks a specified event as active and immediately starts the event
	*** \param event_id The ID of the event to start
	**/
	void StartEvent(uint32 event_id);

	/** \brief Marks a specified event as active and immediately starts the event
	*** \param event A pointer to the event to begin
	**/
	void StartEvent(MapEvent* event);

	/** \brief Begins an event after a specified wait period expires
	*** \param event_id The ID of the event to activate
	*** \param wait_time The number of milliseconds to wait before starting the event
	*** \note Passing a zero value for wait_time will result in a warning message and start the
	*** event immediately. If you wish to start the event immediately, use the version of StartEvent
	*** that does not require a wait_time to be specified.
	**/
	void StartEvent(uint32 event_id, uint32 wait_time);

	/** \brief Begins an event after a specified wait period expires
	*** \param event A pointer to the event to start
	*** \param wait_time The number of milliseconds to wait before starting the event
	*** \note Passing a zero value for wait_time will result in a warning message and start the
	*** event immediately. If you wish to start the event immediately, use the version of StartEvent
	*** that does not require a wait_time to be specified.
	**/
	void StartEvent(MapEvent* event, uint32 wait_time);

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
	*** \note This function will <b>not</b> terminate or prevent the launching of any of the event's children.
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
