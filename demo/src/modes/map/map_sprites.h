///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_sprites.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for map mode sprites.
*** *****************************************************************************/

#ifndef __MAP_SPRITES_HEADER__
#define __MAP_SPRITES_HEADER__

#include "utils.h"
#include "defs.h"
#include "video.h"
#include "map_actions.h"
#include "map_dialogue.h"
#include "map_objects.h"
#include "map_zones.h"

namespace hoa_map {

namespace private_map {

// *********************** SPRITE CONSTANTS **************************

/** \name Map Sprite Speeds
*** \brief Common speeds for sprite movement.
*** These values are the time (in milliseconds) that it takes a sprite to walk
*** the distance of one map grid (16 pixels).
**/
//@{
const float VERY_SLOW_SPEED = 200.0f;
const float SLOW_SPEED      = 175.0f;
const float NORMAL_SPEED    = 150.0f;
const float FAST_SPEED      = 125.0f;
const float VERY_FAST_SPEED = 100.0f;
//@}

/** \name Sprite Direction Constants
*** \brief Constants used for setting and determining sprite directions
*** Sprites are allowed to travel in eight different directions, however the sprite itself
*** can only be facing one of four ways: north, south, east, or west. Because of this, it
*** is possible to travel, for instance, northwest facing north <i>or</i> northwest facing west.
*** The "NW_NORTH" constant means that the sprite is traveling to the northwest and is
*** facing towards the north.
***
*** \note The set of "FACING_DIRECTION" and "MOVING_DIRECTION" constants are only meant to be
*** used as shorthands. You shouldn't assign the MapSprite#direction member to any of these values.
**/
//@{
const uint16 NORTH     = 0x0001;
const uint16 SOUTH     = 0x0002;
const uint16 WEST      = 0x0004;
const uint16 EAST      = 0x0008;
const uint16 NW_NORTH  = 0x0010;
const uint16 NW_WEST   = 0x0020;
const uint16 NE_NORTH  = 0x0040;
const uint16 NE_EAST   = 0x0080;
const uint16 SW_SOUTH  = 0x0100;
const uint16 SW_WEST   = 0x0200;
const uint16 SE_SOUTH  = 0x0400;
const uint16 SE_EAST   = 0x0800;

const uint16 NORTHWEST = NW_NORTH | NW_WEST;
const uint16 NORTHEAST = NE_NORTH | NE_EAST;
const uint16 SOUTHWEST = SW_SOUTH | SW_WEST;
const uint16 SOUTHEAST = SE_SOUTH | SE_EAST;

const uint16 FACING_NORTH = NORTH | NW_NORTH | NE_NORTH;
const uint16 FACING_SOUTH = SOUTH | SW_SOUTH | SE_SOUTH;
const uint16 FACING_WEST = WEST | NW_WEST | SW_WEST;
const uint16 FACING_EAST = EAST | NE_EAST | SE_EAST;

const uint16 LATERAL_MOVEMENT = NORTH | SOUTH | EAST | WEST;
const uint16 DIAGONAL_MOVEMENT = NORTHWEST | NORTHEAST | SOUTHWEST | SOUTHEAST;
//@}

/** \name Map Sprite Animation Constants
*** These constants are used to index the MapSprite#animations vector to display the correct
*** animation. The first 8 entries in this vector always represent the same sets of animations
*** for each map sprite.
**/
//@{
const uint32 ANIM_STANDING_SOUTH = 0;
const uint32 ANIM_STANDING_NORTH = 1;
const uint32 ANIM_STANDING_WEST  = 2;
const uint32 ANIM_STANDING_EAST  = 3;
const uint32 ANIM_WALKING_SOUTH  = 4;
const uint32 ANIM_WALKING_NORTH  = 5;
const uint32 ANIM_WALKING_WEST   = 6;
const uint32 ANIM_WALKING_EAST   = 7;
const uint32 ANIM_RUNNING_SOUTH  = 8;
const uint32 ANIM_RUNNING_NORTH  = 9;
const uint32 ANIM_RUNNING_WEST   = 10;
const uint32 ANIM_RUNNING_EAST   = 11;
//@}

/** ****************************************************************************
*** \brief An invisible and possible mobile sprite on a map
***
*** The VirtualSprite is a special type of MapObject because it has no physical
*** form (no image). Virtual sprites may be manipulated to move around on the screen,
*** or they may remain stationary. VirtualSprites do take collision detection into account
*** by default, unless the no_collision member is set to true. Here are some examples of
*** where virtual sprites may be of use:
***
*** - As a mobile focusing point for the map camera
*** - As an impassible map location for ground objects in a specific context only
*** - To set impassible locations for objects in the sky layer
*** ***************************************************************************/
class VirtualSprite : public MapObject {
public:
	/** \brief A bit-mask for the sprite's draw orientation and direction vector.
	*** This member determines both where to move the sprite (8 directions) and
	*** which way the sprite is facing (4 directions). See the Sprite Directions
	*** series of constants for the values that this member may be set to.
	**/
	uint16 direction;

	//! \brief The speed at which the sprite moves around the map.
	float movement_speed;

	/** \brief Set to true when the sprite is currently moving.
	*** \note This does not necessarily mean that the sprite actually is moving, but rather that
	*** the sprite is <i>trying</i> to move in a certain direction.
	**/
	bool moving;

	//! \brief Set to true when the sprite is running rather than just walking
	bool is_running;

	/** \brief When set to true, indicates that the object exists on the sky object layer (default = false).
	*** This member is necessary for collision detection purposes. When a sprite needs to detect
	*** if it has encountered a collision, that collision must be examined with other objects on
	*** the appropriate layer (the ground or sky layer).
	**/
	bool sky_object;

	//! \brief The name of the sprite, as seen by the player in the game.
	hoa_utils::ustring name;

	//! \brief A pointer to the face portrait of the sprite, as seen in dialogues and menus.
	hoa_video::StillImage* face_portrait;

	//! \brief Set to false if the sprite contains dialogue that has not been seen by the player
	bool seen_all_dialogue;

	//! \brief True is sprite contains active dialogue.
	bool has_active_dialogue;

	/** \brief An index to the actions vector, representing the current sprite action being performed.
	*** A negative value indicates that the sprite is taking no action. If the sprite has no entries
	*** in its actions vector, this member should remain negative, otherwise a segmentation fault
	*** will occur.
	**/
	int8 current_action;

	// TODO: change how forced action work
	int8 forced_action;

	//! \brief A container for all of the actions this sprite performs.
	std::vector<SpriteAction*> actions;

	/** \name Saved state attributes
	*** These attributes are used to save and load the state of a VirtualSprite
	**/
	//@{
	//! \brief This indicates if a state was saved or not.
	bool _saved;
	uint16 _saved_direction;
	float _saved_movement_speed;
	bool _saved_moving;
	hoa_utils::ustring _saved_name;
	int8 _saved_current_action;
	//@}

	//! \brief This vector contains all the dialogues of the sprite
	std::vector<MapDialogue*> dialogues;

	/** \brief An index to the dialogues vector, representing the current sprite dialogue to
	*** display when talked to by the player. A negative value indicates that the sprite has no dialogue.
	*** \note If the sprite has no entries in its dialogues vector, this member should remain negative,
	*** otherwise a segmentation fault will occur.
	**/
	int16 _current_dialogue;

	//! \brief Indicates if the icon indicating that there is a dialogue available should be drawn or not.
	bool _show_dialogue_icon;

	//! \brief Used to fade the dialogue icon according to distance 
	hoa_video::Color _dialogue_icon_color;

	// -------------------- Public methods

	VirtualSprite();

	~VirtualSprite();

	//! \brief Updates the virtual object's position if it is moving, otherwise does nothing.
	virtual void Update();

	//! \brief Draws a dialogue icon over the virtual sprite if it has to.
	virtual void Draw();

	/** \name Lua Access Functions
	*** These functions are specifically written for Lua binding, to enable Lua to access the
	*** members of this class.
	**/
	//@{
	/** \note This method takes into account the current direction when setting the new direction
	*** in the case of diagonal movement. For example, if the sprite is currently facing north
	*** and this function indicates that the sprite should move northwest, it will face north
	*** during the northwest movement.
	**/
	void SetDirection(uint16 dir);

	void SetMovementSpeed(float speed)
		{ movement_speed = speed; }

	uint16 GetDirection() const
		{ return direction; }

	float GetMovementSpeed() const
		{ return movement_speed; }

	void SetFacePortrait(std::string pn);
	//@}

	/** \brief This method will save the state of a sprite.
	*** Attributes saved: direction, speed, moving state, name, current action.
	**/
	virtual void SaveState();

	/** \brief This method will load the saved state of a sprite.
	*** Attributes loaded: direction, speed, moving state, name, current action.
	*** \return false if there was no saved state, true otherwise.
	**/
	virtual bool LoadState();

	//! \brief Examines all dialogue owned by the sprite and sets the appropriate value of VirtualSprite#seen_all_dialogue
	void UpdateSeenDialogue();

	//! \brief Examines all dialogue owned by the sprite and sets the appropriate value of VirtualSprite#has_active_dialogue
	void UpdateActiveDialogue();

	/** \name Dialogue control methods
	*** These methods are used to add and control which dialogue should the sprite speak.
	**/
	//@{
	void AddDialogue(MapDialogue* md);

	bool HasDialogue() const
		//{ return (dialogues.size() > 0); }
		{ if(dialogues.size() > 0) return has_active_dialogue; else return false; }

	MapDialogue* GetCurrentDialogue() const
		{ return dialogues[_current_dialogue]; }

	void SetDialogue(const int16 dialogue)
		{ if (static_cast<uint16>(dialogue) >= dialogues.size()) return; else _current_dialogue = dialogue; }

	void NextDialogue()
		{ do { _current_dialogue++; if (static_cast<uint16>(_current_dialogue) >= dialogues.size()) _current_dialogue = 0; }
			while (dialogues[_current_dialogue]->IsActive() == false); }

	int16 GetNumDialogues() const
		{ return dialogues.size(); }

	void ShowDialogueIcon(bool state)
		{ _show_dialogue_icon = state; }

	bool IsShowingDialogueIcon() const
		{ return _show_dialogue_icon; }
	//@}

	/** \brief Adds a new action for the sprite to process onto the end of the sprite's action list
	*** \param act A pointer to the instantiated SpriteAction object to use
	**/
	void AddAction(SpriteAction* act)
		{ act->SetSprite(this); actions.push_back(act); }

	/** \brief This static class function returns the opposite direction of the direction given in parameter.
	*** \note This is mostly used as an helper function to make sprites face each other.
	**/
	static uint16 CalculateOppositeDirection(const uint16 direction);

	/** \brief Sets the sprite's direction to a random value
	*** This function is used mostly for the ActionRandomMove class.
	**/
	void SetRandomDirection();
}; // class VirtualSprite : public MapObject


/** ****************************************************************************
*** \brief A mobile map object with which the player can interact with.
***
*** Map sprites are animate, mobile, living map objects. Although there is
*** but this single class to represent all the map sprites in the game, they can
*** divided into types such as NPCs, friendly creatures, and enemies. The fact
*** that there is only one class for representing several sprite types is the
*** reason why many of the class members are pointers. For example, we don't
*** need dialogue for a dog sprite.
*** ***************************************************************************/
class MapSprite : public VirtualSprite {
public:
	//! \brief Holds the previous value of VirtualSprite#moving from the last call to MapSprite#Update().
	bool was_moving;

	//! \brief Set to true if the sprite has running animations loaded
	bool has_running_anim;

	/** \brief The sound that will play when the sprite walks.
	*** This member references the MapMode#_map_sounds vector as the sound to play. If this member
	*** is less than zero, no sound is played when the object is walking.
	**/
	int8 walk_sound;

	//! \brief The index to the animations vector containing the current sprite image to display
	uint8 current_animation;

	/** \brief A vector containing all the sprite's various animations.
	*** The first four entries in this vector are the walking animation frames.
	*** They are ordered from index 0 to 3 as: down, up, left, right. Additional
	*** animations may follow.
	**/
	std::vector<hoa_video::AnimatedImage> animations;

	/** \name Saved state attributes
	*** These attributes are used to save and load the state of a VirtualSprite
	**/
	//@{
	bool _saved_was_moving;
	int8 _saved_walk_sound;
	uint8 _saved_current_animation;
	//@}

	// -------------------------------- Methods --------------------------------

	MapSprite();

	~MapSprite();

	/** \brief Loads the image containing the standard animations for the sprite
	*** \param filename The name of the image file holding the standard walking animations
	*** \return False if there was a problem loading the sprite.
	**/
	bool LoadStandardAnimations(std::string filename);

	/** \brief Loads the image containing the running animations for the sprite
	*** \param filename The name of the image file
	*** \return False if the animations were not created successfully.
	**/
	bool LoadRunningAnimations(std::string filename);

	//! \brief Updates the sprite's position and state.
	virtual void Update();

	//! \brief Draws the sprite frame in the appropriate position on the screen, if it is visible.
	virtual void Draw();

	/** \name Lua Access Functions
	*** These functions are specifically written for Lua binding, to enable Lua to access the
	*** members of this class.
	**/
	//@{
	void SetName(std::string na)
		{ name = hoa_utils::MakeUnicodeString(na); }

	void SetWalkSound(int8 sound)
		{ walk_sound = sound; }

	void SetCurrentAnimation(uint8 anim)
		{ current_animation = anim; }

	int8 GetWalkSound() const
		{ return walk_sound; }

	uint8 GetCurrentAnimation() const
		{ return current_animation; }
	//@}

	/** \brief This method will save the state of a sprite.
	*** Attributes saved: direction, speed, moving state, name, current action,
	*** current animation, current walk sound.
	**/
	virtual void SaveState();

	/** \brief This method will load the saved state of a sprite.
	*** Attributes loaded: direction, speed, moving state, name, current action,
	*** current animation, current walk sound.
	*** \return false if there was no saved state, true otherwise.
	**/
	virtual bool LoadState();
}; // class MapSprite : public VirtualSprite


/** ****************************************************************************
*** \brief A mobile map object that induces a battle to occur if the player touches it
***
*** There are really two types of enemy sprites. The first type behave just like
*** map sprites and can have scripted movement sequences. The second type belong
*** to EnemyZones, where they fade into existence and pursue after the player's
*** sprite should the player enter the zone.
***
*** An enemy sprite in a zone can be in one of 3 states: SPAWNING, HOSTILE or DEAD.
*** In the spawning state, the enemy becomes gradually visible, is immobile, and
*** cannot be touched or attacked. In the hostile state, the enemies roams the map
*** and will cause a battle if touched by the player. In the dead state, the enemy
*** is invisible and waits for the EnemyZone to reset it in another position, so
*** that it may spawn once more.
*** ***************************************************************************/
class EnemySprite : public MapSprite {
private:
	//! \brief The states that the enemy sprite may be in
	enum STATE {
		SPAWNING,
		HOSTILE,
		DEAD
	};

public:
	//! \brief The default constructor which typically requires that the user make several additional calls to setup the sprite properties
	EnemySprite();
	
	//! \brief A constructor for when the enemy sprite is stored in the definition of a single file
	EnemySprite(std::string file);

	//! \brief Loads the enemy's data from a file and returns true if it was successful
	bool Load();

	//! \brief Resets various members of the class so that the enemy is dead, invisible, and does not produce a collision
	void Reset();

	//! \brief Updates the sprite's position and state.
	virtual void Update();

	//! \brief Draws the sprite frame in the appropriate position on the screen, if it is visible.
	virtual void Draw();

	// TODO: eventually I would like the ability for Lua to pass in a table of ints to the AddEnemyParty function, but because I'm not quite
	// sure how to do that yet, I'm writing several smaller functions so we can just get this demo released.

	// void AddEnemyParty(std::vector<uint32>& party);

	/** \brief Adds a new empty vector to the _enemy_parties member
	*** \note Make sure to populate this vector by adding at least one enemy!
	**/
	void NewEnemyParty()
		{ _enemy_parties.push_back(std::vector<uint32>()); }

	/** \brief Adds an enemy with the specified ID to the last party in _enemy_parties
	*** \param enemy_id The ID of the enemy to add
	*** \note MapMode should have already loaded a GlobalEnemy with this ID and retained it within the MapMode#_enemies member.
	*** If this is not the case, this function will print a warning message.
	**/
	void AddEnemy(uint32 enemy_id);

	//! \brief Returns a reference to a random party of enemies
	const std::vector<uint32>& RetrieveRandomParty();

	//! \name Class Member Access Functions
	//@{
	float GetAggroRange() const
		{ return _aggro_range; }

	uint32 GetTimeToChange() const
		{ return _time_dir_change; }

	uint32 GetTimeToSpawn() const
		{ return _time_to_spawn; }

	std::string GetBattleMusicTheme() const
		{ return _music_theme; }

	bool IsDead() const
		{ return _state == DEAD; }

	bool IsSpawning() const
		{ return _state == SPAWNING; }

	bool IsHostile() const
		{ return _state == HOSTILE; }

	void SetZone(EnemyZone* zone)
		{ _zone = zone; }

	void SetAggroRange(float range)
		{ _aggro_range = range; }

	void SetTimeToChange(uint32 time)
		{ _time_dir_change = time; }

	void SetTimeToSpawn(uint32 time)
		{ _time_to_spawn = time; }

	void SetBattleMusicTheme(const std::string & music_theme)
		{ _music_theme = music_theme; }

	void ChangeStateDead() { 
		Reset(); 
		if( _zone ) 
			_zone->EnemyDead(); 
	}

	void ChangeStateSpawning()
		{ updatable = true; _state = SPAWNING; no_collision = false; }

	void ChangeStateHostile()
		{ updatable = true; _state = HOSTILE; no_collision = false; _color.SetAlpha(1.0); }
	//@}

private:
	//! \brief The zone that the enemy sprite belongs to
	private_map::EnemyZone* _zone;

	//! \brief Used to gradually fade in the sprite as it is spawning by adjusting the alpha channel
	hoa_video::Color _color;

	//! \brief A timer used for spawning
	uint32 _time_elapsed;

	//! \brief The state that the enemy sprite is in
	STATE _state;

	//! \brief A value which determines how close the player needs to be for the enemy to aggressively seek to confront it
	float _aggro_range;

	//! \brief ???
	uint32 _time_dir_change;

	//! \brief ???
	uint32 _time_to_spawn;

	//! \brief Indicates if the enemy is outside of its zone. If it is, it won't change direction until it gets back in.
	bool _out_of_zone;

	//! \brief The default battle music theme for the monster
	std::string _music_theme;

	/** \brief Contains the possible groups of enemies that may appear in a battle should the player encounter this enemy sprite
	*** The numbers contained within this member are ID numbers for the enemy. If the
	**/
	std::vector<std::vector<uint32> > _enemy_parties;
}; // class EnemySprite : public MapSprite

} // namespace private_map

} // namespace hoa_map

#endif // __MAP_SPRITES_HEADER__
