///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for map mode interface.
***
*** This file contains the interface for map mode, active when the player is
*** exploring town or dungeon maps. The map environments of Allacrost are
*** quite extensive, thus this code is responsible for processing many things.
*** This includes handling all tile images, objects, sprites, map events,
*** dialogue, and more.
***
*** Each individual map is represented by it's own object of the MapMode class.
*** It is our intention in the future to retain more than one map in memory at
*** once to reduce or eliminate loading times when the player transitions
*** between maps.
*** ***************************************************************************/

#ifndef __MAP_HEADER__
#define __MAP_HEADER__

// Allacrost utilities
#include "defs.h"
#include "utils.h"

// Allacrost engines
#include "audio.h"
#include "mode_manager.h"
#include "script.h"
#include "video.h"

// Local map mode headers
#include "map_utils.h"

//! All calls to map mode are wrapped in this namespace.
namespace hoa_map {

//! An internal namespace to be used only within the map code. Don't use this namespace anywhere else!
namespace private_map {

} // namespace private_map

/** ****************************************************************************
*** \brief Handles the game execution while the player is exploring maps.
***
*** This class contains all of the structures that together compose each map.
*** Each map has a Lua script file in which the map data is permanently retained
*** and various script subroutines exist that modify the map's behavior. Keep in
*** mind that this class alone does not represent all of the data nor all of the
*** code that is used in a particular map, as the map's Lua file likely retains
*** additional information about the map that is not represented in this class.
***
*** Maps are composed by a series of tiles and objects. Tiles are 32x32 pixel
*** squares that are adjacent to one another on a map, and together make up the
*** map's background environment. Objects are variable sized entities that are
*** usually living, animated creatures (sprites), but may be something static
*** such as a large tree. Tiles and objects are drawn in multiple interwieving
*** layers to emulate a 3D environment for the game. Additionally each map has
*** a collision grid composed of 16x16 pixel elements that determine which
*** quadrants of each tile may be occupied by sprites or other objects.
***
*** Another important concept to MapMode is that of contexts. Each map has at
*** least one context and can have up to a maximum of 32 contexts. Every context
*** has its own collision grid and its own set of tiles. Map objects and sprites
*** exist in one of these context and may change their context at any time.
*** Objects and sprites can not interact with one another when they are not
*** within the same context, and typically only objects that are in the same
*** context as the map camera are visible on the screen. You can think of each
*** context as essentially its own map, and the set of contexts as a set of maps
*** that work with one another to create a cohesive environment.
***
*** Because this game mode is so complex, the MapMode class shares its
*** responsibilities with several small singleton classes that manage a
*** particular area of map code, such as tiles or objects. These sub-manager
*** classes should be viewed as extensions of the MapMode class.
*** ***************************************************************************/
class MapMode : public hoa_mode_manager::GameMode {
	friend class private_map::MapFrame;
	friend class private_map::MapObject;
	friend class private_map::PhysicalObject;
	friend class private_map::MapTreasure;
	friend class private_map::VirtualSprite;
	friend class private_map::MapSprite;
	friend class private_map::EnemySprite;
	friend class private_map::DialogueSupervisor;
	friend class private_map::TreasureSupervisor;
	friend class private_map::MapDialogue;
	friend class private_map::MapDialogueOptions;
	friend class private_map::DialogueEvent;
	friend class private_map::MapTransitionEvent;
	friend class private_map::PathMoveSpriteEvent;
	friend class private_map::RandomMoveSpriteEvent;
	friend class private_map::ScriptedEvent;
	friend class private_map::EnemyZone;
	friend class private_map::ContextZone;
	friend class private_map::ObjectSupervisor;
	friend class private_map::TileSupervisor;

	friend void hoa_defs::BindModesToLua();
public:
	//! \param filename The name of the Lua file that retains all data about the map to create
	MapMode(std::string filename);

	~MapMode();

	//! \brief Resets appropriate class members. Called whenever the MapMode object is made the active game mode.
	void Reset();

	//! \brief Updates the game and calls various sub-update functions depending on the current state of map mode.
	void Update();

	//! \brief The highest level draw function that will call the appropriate lower-level draw functions
	void Draw();

private:
	// ----- Members : Names and Identifiers -----

	/** \brief A reference to the current instance of MapMode
	*** This is used by other map clases to be able to refer to the map that they exist in.
	**/
	static MapMode *_current_map;

	//! \brief The name of the Lua file that represents the map
	std::string _map_filename;

	/** \brief The map's unique name as it is used to identify a Lua namespace table
	*** To avoid Lua naming conflicts between multiple map files, all map data is encompassed within
	*** a namespace (a Lua table) that is unique to each map.
	**/
	std::string _map_tablespace;

	//! \brief The name of the map, as it will be read by the player in the game.
	hoa_utils::ustring _map_name;

	//! \brief A pointer to the object containing all of the event information for the map
	hoa_global::GlobalEventGroup* _map_event_group;

	/** \brief The interface to the file which contains all the map's stored data and subroutines.
	*** This class generally performs a large amount of communication with this script continuously.
	*** The script remains open for as long as the MapMode object exists.
	**/
	hoa_script::ReadScriptDescriptor _map_script;

	// ----- Members : Sub-management Objects -----

	//! \brief Instance of helper class to map mode. Responsible for tile related operations.
	private_map::TileSupervisor* _tile_supervisor;

	//! \brief Instance of helper class to map mode. Responsible for object and sprite related operations.
	private_map::ObjectSupervisor* _object_supervisor;

	//! \brief Instance of helper class to map mode. Responsible for dialogue execution and display operations.
	private_map::DialogueSupervisor* _dialogue_supervisor;

	//! \brief Instance of helper class to map mode. Responsible for updating and managing active map events.
	private_map::EventSupervisor* _event_supervisor;

	//! \brief Class member object which processes all information related to treasure discovery
	private_map::TreasureSupervisor* _treasure_supervisor;

	/** \brief A script function which assists with the MapMode#Update method
	*** This function implements any custom update code that the specific map needs to be performed.
	*** The most common operation that this script function performs is to check for trigger conditions
	*** that cause map events to occur
	**/
	ScriptObject _update_function;

	/** \brief Script function which assists with the MapMode#Draw method
	*** This function allows for drawing of custom map visuals. Usually this includes lighting or
	*** other visual effects for the map environment.
	**/
	ScriptObject _draw_function;

	// ----- Members : Properties and State -----

	//! \brief The number of contexts for this map (at least 1, at most 32)
	uint8 _num_map_contexts;

	/** \brief The currently active map context
	*** This is always equal to the context of the object pointed to by the _camera member
	**/
	private_map::MAP_CONTEXT _current_context;

	/** \brief The amount of stamina
	*** This value ranges from 0 (empty) to 10000 (full). Stamina takes 10 seconds to completely fill from
	*** the empty state and 5 seconds to empty from the full state.
	**/
	uint32 _run_stamina;

	//! \brief Indicates the current state that the map is in, such as when a dialogue is taking place.
	uint8 _map_state;

	std::vector<private_map::MAP_STATE> _state_stack;

	//! \brief While true, all user input commands to map mode are ignored
	bool _ignore_input;

	//! \brief If true, the player's stamina will not drain as they run
	bool _run_forever;

	//! \brief While true, the player is not allowed to run at all.
	bool _run_disabled;

	//! \brief Indicates if dialogue icons should be drawn or not.
	//! \todo I don't think that this needs to be a static member. It should be true by default.
	static bool _show_dialogue_icons;

	// ----- Members : Map Timing and Graphics -----

	/** \brief A timer used for when the player first enters the map
	*** This timer is set to 7000 ms (7 seconds) and is used to display the map's location graphic
	*** and name at the top center of the screen. The graphic and text are faded in for the first
	*** two seconds, drawn opaquely for the next three seconds, and faded out in the final two seconds.
	**/
	hoa_system::SystemTimer _intro_timer;

	//! \brief Holds an image that represents an outline of the location, used primarily in MenuMode
	hoa_video::StillImage _location_graphic;

	//! \brief Icon which appears over NPCs who have unread dialogue
	hoa_video::AnimatedImage _new_dialogue_icon;

	//! \brief Image which underlays the stamina bar for "running"
	hoa_video::StillImage _stamina_bar_background;

	//! \brief Image which overlays the stamina bar to show that the player has unlimited running
	hoa_video::StillImage _stamina_bar_infinite_overlay;

	// ----- Members : Containers and Other Data -----

	/** \brief A pointer to the map sprite that the map camera will focus on
	*** \note Note that this member is a pointer to a map sprite, not a map object.
	*** However, this does not mean that the camera is not able to focus on non-sprite
	*** map objects. The MapMode#_virtual_focus member can be used to emulate that
	*** focus.
	**/
	private_map::VirtualSprite* _camera;

	//! \brief Retains information needed to correctly draw the next map frame
	private_map::MapFrame _draw_info;

	//! \brief The music that the map will need to make use of
	std::vector<hoa_audio::MusicDescriptor> _music;

	//! \brief The sounds that the map needs available to it
	std::vector<hoa_audio::SoundDescriptor> _sounds;

	/** \brief A container for the various foes which may appear on this map
	*** These enemies are kept at their initial stats. When they are passed to battle mode,
	*** a copy of each enemy is made and initialized there.
	**/
	std::vector<hoa_global::GlobalEnemy*> _enemies;

	// ----- Methods -----

	//! \brief Empties the state stack and places an invalid state on top
	void _ResetState();

	/** \brief Pushes a state type to the top of the state stack, making it the active state
	*** \param state The state to push onto the stack
	**/
	void _PushState(private_map::MAP_STATE state);

	//! \brief Removes the highest item in the state stack
	void _PopState();

	/** \brief Retrieves the current map state
	*** \return The top-most item on the map state stack
	**/
	private_map::MAP_STATE _CurrentState();



	//! \brief Loads all map data contained in the Lua file that defines the map
	void _Load();

	//! \brief A helper function to Update() that is called only when the map is in the explore state
	void _UpdateExplore();

	//! \brief Calculates information about how to draw the next map frame
	void _CalculateMapFrame();

	//! \brief Draws all visible map tiles and sprites to the screen
	void _DrawMapLayers();

	//! \brief Draws all GUI visuals, such as dialogue icons and the stamina bar
	void _DrawGUI();

	/** \name Lua Access Functions
	*** These methods exist to allow Lua to make function calls to examine and modify the map's state.
	**/
	//@{
	void _AddGroundObject(private_map::MapObject *obj);

	void _AddPassObject(private_map::MapObject *obj);

	void _AddSkyObject(private_map::MapObject *obj);

	void _AddZone(private_map::MapZone *zone);

	uint16 _GetGeneratedObjectID();

	// TEMP
	private_map::DialogueSupervisor* _GetDialogueSupervisor() const
		{ return _dialogue_supervisor; }

	private_map::VirtualSprite* _GetCameraFocus() const
		{ return _camera; }

	static bool _IsShowingDialogueIcons()
		{ return _show_dialogue_icons; }

	void _SetCameraFocus(private_map::VirtualSprite *sprite)
		{ _camera = sprite; }

	static void _ShowDialogueIcons( bool state )
		{ _show_dialogue_icons = state; }
	//@}
}; // class MapMode

} // namespace hoa_map;

#endif
