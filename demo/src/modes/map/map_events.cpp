///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
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
#include "audio.h"
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
#include "battle.h"

using namespace std;

using namespace hoa_audio;
using namespace hoa_mode_manager;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_video;

using namespace hoa_battle;
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
	MapMode::CurrentInstance()->GetDialogueSupervisor()->BeginDialogue(_dialogue_id);
}



bool DialogueEvent::_Update() {
	MapDialogue* active_dialogue = MapMode::CurrentInstance()->GetDialogueSupervisor()->GetCurrentDialogue();
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


void ShopEvent::AddWare(uint32 object_id, uint32 stock) {
	_wares.insert(make_pair(object_id, stock));
}



void ShopEvent::_Start() {
	ShopMode* shop = new ShopMode();
	for (set<pair<uint32, uint32> >::iterator i = _wares.begin(); i != _wares.end(); i++) {
		shop->AddObject((*i).first, (*i).second);
	}
	ModeManager->Push(shop);
}



bool ShopEvent::_Update() {
	return true;
}

// ****************************************************************************
// ********** SoundEvent Class Functions
// ****************************************************************************

SoundEvent::SoundEvent(uint32 event_id, string sound_filename) :
	MapEvent(event_id, SOUND_EVENT)
{
	if (_sound.LoadAudio(sound_filename) == false) {
		IF_PRINT_WARNING(MAP_DEBUG) << "failed to load sound event: " << sound_filename << endl;
	}
}



SoundEvent::~SoundEvent() {
	_sound.Stop();
}



void SoundEvent::_Start() {
	_sound.Play();
}



bool SoundEvent::_Update() {
	if (_sound.GetState() == AUDIO_STATE_STOPPED) {
		// TODO: is it necessary to reset the loop counter and other properties here before returning?
		return true;
	}

	return false;
}

// ****************************************************************************
// ********** MapTransitionEvent Class Functions
// ****************************************************************************

MapTransitionEvent::MapTransitionEvent(uint32 event_id, std::string filename) :
	MapEvent(event_id, MAP_TRANSITION_EVENT),
	_transition_map_filename(filename)
{
	_fade_timer.Initialize(FADE_OUT_TIME, SYSTEM_TIMER_NO_LOOPS);
}



MapTransitionEvent::~MapTransitionEvent()
{}



void MapTransitionEvent::_Start() {
	MapMode::CurrentInstance()->PushState(STATE_SCENE);
	_fade_timer.Reset();
	_fade_timer.Run();
	// TODO: The call below is a problem because if the user pauses while this event is in progress,
	// the screen fade will continue while in pause mode (it shouldn't). I think instead we'll have
	// to perform a manual fade of the screen.
	VideoManager->FadeScreen(Color::black, _fade_timer.GetDuration());

	// TODO: fade out the map music
}



bool MapTransitionEvent::_Update() {
	_fade_timer.Update();

	if (_fade_timer.IsFinished() == true) {
		ModeManager->Pop();
		try {
			MapMode *MM = new MapMode(_transition_map_filename);
			ModeManager->Push(MM);
		} catch (luabind::error e) {
			PRINT_ERROR << "Error loading map: " << _transition_map_filename << endl;
			ScriptManager->HandleLuaError(e);
		}
		// This will fade the screen back in from black
		VideoManager->FadeScreen(Color::clear, _fade_timer.GetDuration() / 2);
		return true;
	}

	return false;
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

BattleEncounterEvent::BattleEncounterEvent(uint32 event_id, uint32 enemy_id) :
	MapEvent(event_id, BATTLE_ENCOUNTER_EVENT),
	_battle_music("mus/Confrontation.ogg"),
	_battle_background("img/backdrops/battle/desert.png")
{
	_enemy_ids.push_back(enemy_id);
}



BattleEncounterEvent::~BattleEncounterEvent() {
}



void BattleEncounterEvent::SetMusic(std::string filename) {
	_battle_music = filename;
}


void BattleEncounterEvent::SetBackground(std::string filename) {
	_battle_background = filename;
}


void BattleEncounterEvent::AddEnemy(uint32 enemy_id) {
	_enemy_ids.push_back(enemy_id);
}



void BattleEncounterEvent::AddBattleEvent(uint32 event_id) {
	_battle_event_ids.push_back(event_id);
}



void BattleEncounterEvent::_Start() {
	BattleMode* batt_mode = new BattleMode();
	for (uint32 i = 0; i < _enemy_ids.size(); i++) {
		batt_mode->AddEnemy(_enemy_ids.at(i));
	}
// 	for (uint32 i = 0; i < _battle_event_ids.size(); i++) {
// 		batt_mode->AddEvent(new BattleEvent(_battle_event_ids.at(i)));
// 	}
	batt_mode->AddMusic(_battle_music);
	batt_mode->SetBackground(_battle_background);
	ModeManager->Push(batt_mode);
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
	ReadScriptDescriptor& map_script = MapMode::CurrentInstance()->GetMapScript();
	MapMode::CurrentInstance()->OpenMapTablespace(true);
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
	_current_node(0),
	_last_x_position(0),
	_last_y_position(0)
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
	_last_x_position = _sprite->x_position;
	_last_y_position = _sprite->y_position;

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

	if (MapMode::CurrentInstance()->GetObjectSupervisor()->FindPath(_sprite, _path, _destination) == true) {
		_sprite->moving = true;
		_SetDirection();
	}
}



bool PathMoveSpriteEvent::_Update() {
	if (_path.empty() == true) {
		PRINT_ERROR << "no path to destination" << endl;
		return true;
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
	// If the sprite has moved to a new position other than the next node, adjust its direction so it is trying to move to the next node
	else if ((_sprite->x_position != _last_x_position) || (_sprite->y_position != _last_y_position)) {
		_last_x_position = _sprite->x_position;
		_last_y_position = _sprite->y_position;
		_SetDirection();
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
		if (MapMode::CurrentInstance()->GetObjectSupervisor()->AdjustSpriteAroundCollision(_sprite, coll_type, coll_obj) == false) {
			IF_PRINT_WARNING(MAP_DEBUG) << "boundary or grid collision occurred on a pre-calculated path movement" << endl;
		}
		// Wait
// 		_path.clear(); // This path is obviously not a correct one so we should trash it
// 		_sprite->ReleaseControl(this);
// 		MapMode::CurrentInstance()->GetEventSupervisor()->TerminateEvent(GetEventID());
		return;
	}

	// If the code has reached this point, then we are dealing with an object collision

	// Determine if the obstructing object is blocking the destination of this path
	bool destination_blocked = MapMode::CurrentInstance()->GetObjectSupervisor()->IsPositionOccupiedByObject(_destination.row, _destination.col, coll_obj);
	VirtualSprite* coll_sprite = NULL;

	switch (coll_obj->GetObjectType()) {
		case PHYSICAL_TYPE:
		case TREASURE_TYPE:
			// If the object is a static map object and blocking the destination, give up and terminate the event
			if (destination_blocked == true) {
				IF_PRINT_WARNING(MAP_DEBUG) << "path destination was blocked by a non-sprite map object" << endl;
				_path.clear(); // This path is obviously not a correct one so we should trash it
				_sprite->ReleaseControl(this);
				MapMode::CurrentInstance()->GetEventSupervisor()->TerminateEvent(GetEventID());
				// Note that we will retain the path (we don't clear() it), hoping that next time the object is moved

			}
			// Otherwise, try to find an alternative path around the object
			else {
				// TEMP: try a movement adjustment to get around the object
				MapMode::CurrentInstance()->GetObjectSupervisor()->AdjustSpriteAroundCollision(_sprite, coll_type, coll_obj);
				// TODO: recalculate and find an alternative path around the object
			}
			break;

		case VIRTUAL_TYPE:
		case SPRITE_TYPE:
		case ENEMY_TYPE:
			coll_sprite = dynamic_cast<VirtualSprite*>(coll_obj);
			if (destination_blocked == true) {
				// Do nothing but wait for the obstructing sprite to move out of the way
				return;

				// TODO: maybe we should use a timer here to determine if a certain number of seconds have passed while waiting for the obstructiong
				// sprite to move. If that timer expires and the destination is still blocked by the sprite, we could give up on reaching the
				// destination and terminate the path event
			}

			else {
				// TEMP: try a movement adjustment to get around the object
				MapMode::CurrentInstance()->GetObjectSupervisor()->AdjustSpriteAroundCollision(_sprite, coll_type, coll_obj);
			}
			break;

		default:
			IF_PRINT_WARNING(MAP_DEBUG) << "collision object was of an unknown object type: " << coll_obj->GetObjectType() << endl;
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



void RandomMoveSpriteEvent::_ResolveCollision(COLLISION_TYPE coll_type, MapObject* coll_obj) {
	// Try to adjust the sprite's position around the collision. If that fails, change the sprite's direction
	if (MapMode::CurrentInstance()->GetObjectSupervisor()->AdjustSpriteAroundCollision(_sprite, coll_type, coll_obj) == false) {
		_sprite->SetRandomDirection();
	}
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
	dynamic_cast<MapSprite*>(_sprite)->SetCustomAnimation(true);
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
					dynamic_cast<MapSprite*>(_sprite)->SetCustomAnimation(false);
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
