///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_sprites.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for map mode sprites.
*** ***************************************************************************/

#include "utils.h"
#include "map.h"
#include "map_sprites.h"
#include "map_objects.h"
#include "map_dialogue.h"
#include "map_actions.h"

#include "audio.h"
#include "video.h"
#include "script.h"
#include "system.h"
#include "global.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_global;

namespace hoa_map {

namespace private_map {

// Initialized in MapMode constructor
hoa_video::AnimatedImage new_dialogue_icon;

// ****************************************************************************
// ********************* VirtualSprite Class Functions ************************
// ****************************************************************************

VirtualSprite::VirtualSprite() :
	direction(SOUTH),
	movement_speed(NORMAL_SPEED),
	moving(false),
	sky_object(false),
	face_portrait( NULL ),
	current_action(-1),
	forced_action(-1),
	_saved(false),
	_current_dialogue(0),
	_show_dialogue_icon(true),
	_dialogue_icon_color( 1.0f, 1.0f, 1.0f, 0.0f )
{
	MapObject::_object_type = VIRTUAL_TYPE;
}



VirtualSprite::~VirtualSprite() {
	if (face_portrait != NULL) {
		VideoManager->DeleteImage(*face_portrait);
		face_portrait = NULL;
	}

	for (uint32 i = 0; i < actions.size(); i++) {
		delete actions[i];
	}
	actions.clear();

	for(uint32 i = 0; i < dialogues.size(); ++i) {
		delete dialogues[i];
	}
	dialogues.clear();
}


void VirtualSprite::UpdateSeenDialogue() {
	// Check all dialogues for any which have not yet been read
	for (uint32 i = 0; i < dialogues.size(); i++) {
		if (dialogues[i]->HasAlreadySeen() == false) {
			seen_all_dialogue = false;
			return;
		}
	}

	seen_all_dialogue = true;
}



uint16 VirtualSprite::CalculateOppositeDirection(const uint16 direction) {
	switch (direction) {
	case NORTH:     return SOUTH;
	case SOUTH:     return NORTH;
	case WEST:      return EAST;
	case EAST:      return WEST;
	case NW_NORTH:  return SE_SOUTH;
	case NW_WEST:   return SE_EAST;
	case NE_NORTH:  return SW_SOUTH;
	case NE_EAST:   return SW_WEST;
	case SW_SOUTH:  return NE_NORTH;
	case SW_WEST:   return NE_EAST;
	case SE_SOUTH:  return NW_NORTH;
	case SE_EAST:   return NW_WEST;
	default:
		if (MAP_DEBUG)
			cerr << "MAP WARNING: VirtualSprite::CalculateOppositeDirection received invalid direction" << endl;
		return SOUTH;
	}
}



void VirtualSprite::Update() {
	//Update the alpha of the dialogue icon according to it's distance from the player to make it fade away
	const float DIALOGUE_ICON_VISIBLE_RANGE = 30.0f;
	float icon_alpha = 1.0f - ( abs( ComputeXLocation() - MapMode::_current_map->_camera->ComputeXLocation() ) + abs( ComputeYLocation() - MapMode::_current_map->_camera->ComputeYLocation() ) ) / DIALOGUE_ICON_VISIBLE_RANGE;
	if( icon_alpha < 0 )
		icon_alpha = 0;
	_dialogue_icon_color.SetAlpha( icon_alpha );

	new_dialogue_icon.Update();
	if (!updatable) {
		return;
	}

	// If the sprite was not forced to do a certain action
	if (forced_action < 0) {
		// Execute the sprite's action and if it is finished, update the action counter
		if (current_action >= 0) {
			actions[current_action]->Execute();
			if (actions[current_action]->IsFinishedReset()) {
				current_action++;
				if (static_cast<uint8>(current_action) >= actions.size())
					current_action = 0;
			}
		}
	}

	if (moving) {
		// Save the previous sprite's position temporarily
		float tmp_x = x_offset;
		float tmp_y = y_offset;

		float distance_moved = static_cast<float>(MapMode::_current_map->_time_elapsed) / movement_speed;
		// If this object is the map's focused and the player is running, increase the distance moved
		if (MapMode::_current_map->_camera == this && MapMode::_current_map->_running == true)
			distance_moved *= 2.0f;
		// If the movement is diagonal, decrease the lateral movement distance by sin(45 degress)
		if (direction & DIAGONAL_MOVEMENT)
			distance_moved *= 0.707f;

		// Move the sprite the appropriate distance in the appropriate Y direction
		if (direction & (NORTH | NORTHWEST | NORTHEAST))
			y_offset -= distance_moved;
		else if (direction & (SOUTH | SOUTHWEST | SOUTHEAST))
			y_offset += distance_moved;

		// Determine if the sprite may move to this new Y position
		if (MapMode::_current_map->_DetectCollision(this)) {
			y_offset = tmp_y;
		}
		else {
			// Roll-over Y position offsets if necessary
			while (y_offset < 0.0f) {
				y_position -= 1;
				y_offset += 1.0f;
			}
			while (y_offset > 1.0f) {
				y_position += 1;
				y_offset -= 1.0f;
			}
		}

		// Move the sprite the appropriate distance in the appropriate X direction
		if (direction & (WEST | NORTHWEST | SOUTHWEST))
			x_offset -= distance_moved;
		else if (direction & (EAST | NORTHEAST | SOUTHEAST))
			x_offset += distance_moved;


		// Determine if the sprite may move to this new X position
		if (MapMode::_current_map->_DetectCollision(this)) {
			x_offset = tmp_x;
		}
		else {
			// Roll-over X position offsets if necessary
			while (x_offset < 0.0f) {
				x_position -= 1;
				x_offset += 1.0f;
			}
			while (x_offset > 1.0f) {
				x_position += 1;
				x_offset -= 1.0f;
			}
		}
	} // if (moving)
} // void VirtualSprite::Update()



void VirtualSprite::Draw() {
	if (HasDialogue()) {
		if (IsShowingDialogueIcon() && MapMode::_IsShowingDialogueIcons() && seen_all_dialogue == false) {
			VideoManager->MoveRelative(0, -GetImgHeight());
			VideoManager->DrawImage(new_dialogue_icon, _dialogue_icon_color);
		}
	}
}



void VirtualSprite::SetDirection(uint16 dir) {
	// If the direction is a lateral one, simply set it and return
	if (dir & (NORTH | SOUTH | EAST | WEST)) {
		direction = dir;
		return;
	}

	// Otherwise the direction is diagonal, and we must figure out which way the sprite should face.
	if (dir & NORTHWEST) {
		if (direction & (FACING_NORTH | FACING_EAST))
			direction = NW_NORTH;
		else
			direction = NW_WEST;
	}
	else if (dir & SOUTHWEST) {
		if (direction & (FACING_SOUTH | FACING_EAST))
			direction = SW_SOUTH;
		else
			direction = SW_WEST;
	}
	else if (dir & NORTHEAST) {
		if (direction & (FACING_NORTH | FACING_WEST))
			direction = NE_NORTH;
		else
			direction = NE_EAST;
	}
	else if (dir & SOUTHEAST) {
		if (direction & (FACING_SOUTH | FACING_WEST))
			direction = SE_SOUTH;
		else
			direction = SE_EAST;
	}
	else { // Invalid
		if (MAP_DEBUG)
			fprintf(stderr, "ERROR: in VirtualSprite::SetDirection tried to set an invalid direction (%d)\n", dir);
	}
} // void VirtualSprite::SetDirection(uint16 dir)



void VirtualSprite::SetFacePortrait(std::string pn) {
	if (face_portrait != NULL) {
		delete face_portrait;
	}

	face_portrait = new StillImage();
	face_portrait->SetFilename(pn);
	VideoManager->LoadImage(*face_portrait);
}



void VirtualSprite::SaveState() {
	_saved = true;

	_saved_direction = direction;
	_saved_movement_speed = movement_speed;
	_saved_moving = moving;
	_saved_name = name;
	_saved_current_action = current_action;
	// TEMP
	//updatable = false;
}



bool VirtualSprite::LoadState() {
	if (_saved == false)
		return false;

	 direction = _saved_direction;
	 movement_speed = _saved_movement_speed;
	 moving = _saved_moving;
	 name = _saved_name;
	 current_action = _saved_current_action;

	// TEMP
	//updatable = true;

	 return true;
}



void VirtualSprite::SetRandomDirection() {
	switch (RandomBoundedInteger(1, 8)) {
		case 1:
			SetDirection(NORTH);
			break;
		case 2:
			SetDirection(SOUTH);
			break;
		case 3:
			SetDirection(EAST);
			break;
		case 4:
			SetDirection(WEST);
			break;
		case 5:
			SetDirection(NORTHEAST);
			break;
		case 6:
			SetDirection(NORTHWEST);
			break;
		case 7:
			SetDirection(SOUTHEAST);
			break;
		case 8:
			SetDirection(SOUTHWEST);
			break;
		default:
			if (MAP_DEBUG)
				cerr << "MAP WARNING: In VirtualSprite::SetRandomDirection(), invalid direction was picked" << endl;
	}
}

// ****************************************************************************
// ************************ MapSprite Class Functions *************************
// ****************************************************************************

// Constructor for critical class members. Other members are initialized via support functions
MapSprite::MapSprite() :
	was_moving(false),
	walk_sound(-1),
	current_animation(ANIM_STANDING_SOUTH)
{
	MapObject::_object_type = SPRITE_TYPE;
	VirtualSprite::face_portrait = 0;
}


// Free all allocated images and other data
MapSprite::~MapSprite() {
	for (uint32 i = 0; i < animations.size(); i++) {
		VideoManager->DeleteImage(animations[i]);
	}
	animations.clear();

	if (face_portrait != NULL) {
		VideoManager->DeleteImage(*face_portrait);
		face_portrait = NULL;
	}

// 	for (uint32 i = 0; i < dialogues.size(); i++) {
// 		delete(dialogues[i]);
// 	}
}


// Load in the appropriate images and other data for the sprite
bool MapSprite::LoadStandardAnimations(std::string filename) {

	// The speed to display each frame in the walking animation
	uint32 frame_speed = static_cast<uint32>(movement_speed / 10.0f);

	// Prepare the four standing and four walking animations
	for (uint8 i = 0; i < 8; i++)
		animations.push_back(AnimatedImage());

	// Load the multi-image, containing 24 frames total
	vector<StillImage> frames(24);
	for (uint8 i = 0; i < 24; i++)
		frames[i].SetDimensions(img_half_width * 2, img_height);

	if (VideoManager->LoadMultiImageFromNumberElements(frames, filename, 4, 6) == false) {
		return false;
	}

	// Add standing frames to animations
	animations[0].AddFrame(frames[0], frame_speed);
	animations[1].AddFrame(frames[6], frame_speed);
	animations[2].AddFrame(frames[12], frame_speed);
	animations[3].AddFrame(frames[18], frame_speed);

	// Add walking frames to animations
	animations[4].AddFrame(frames[1], frame_speed);
	animations[4].AddFrame(frames[2], frame_speed);
	animations[4].AddFrame(frames[3], frame_speed);
	animations[4].AddFrame(frames[1], frame_speed);
	animations[4].AddFrame(frames[4], frame_speed);
	animations[4].AddFrame(frames[5], frame_speed);

	animations[5].AddFrame(frames[7], frame_speed);
	animations[5].AddFrame(frames[8], frame_speed);
	animations[5].AddFrame(frames[9], frame_speed);
	animations[5].AddFrame(frames[7], frame_speed);
	animations[5].AddFrame(frames[10], frame_speed);
	animations[5].AddFrame(frames[11], frame_speed);

	animations[6].AddFrame(frames[13], frame_speed);
	animations[6].AddFrame(frames[14], frame_speed);
	animations[6].AddFrame(frames[15], frame_speed);
	animations[6].AddFrame(frames[13], frame_speed);
	animations[6].AddFrame(frames[16], frame_speed);
	animations[6].AddFrame(frames[17], frame_speed);

	animations[7].AddFrame(frames[19], frame_speed);
	animations[7].AddFrame(frames[20], frame_speed);
	animations[7].AddFrame(frames[21], frame_speed);
	animations[7].AddFrame(frames[19], frame_speed);
	animations[7].AddFrame(frames[22], frame_speed);
	animations[7].AddFrame(frames[23], frame_speed);

	for (uint32 i = 0; i < animations.size(); i++) {
		if (animations[i].Load() == false) {
			cerr << "MAP ERROR: failed to load sprite animation" << endl;
			return false;
		}
	}

	return true;
} // bool MapSprite::LoadStandardAnimations(std::string filename)


// Updates the state of the sprite
void MapSprite::Update() {
// 	if (!updatable)
// 		return;

	// Set the sprite's animation to the standing still position if movement has just stopped
	if (!moving) {
		if (was_moving) {
			// Set the current movement animation to zero progress
			animations[current_animation].SetTimeProgress(0);
			was_moving = false;
		}

		// Determine the correct standing frame to display
		if (current_action == -1) {
			if (direction & FACING_NORTH) {
				current_animation = ANIM_STANDING_NORTH;
			}
			else if (direction & FACING_SOUTH) {
				current_animation = ANIM_STANDING_SOUTH;
			}
			else if (direction & FACING_WEST) {
				current_animation = ANIM_STANDING_WEST;
			}
			else if (direction & FACING_EAST) {
				current_animation = ANIM_STANDING_EAST;
			}
			else {
				cerr << "MAP ERROR: could not find proper standing animation to draw" << endl;
			}
		}
	} // if (!moving)

	// This call will update the sprite's position and perform collision detection
	VirtualSprite::Update();

	if (moving) {
		// Save the previous animation
		uint8 last_animation = current_animation;

		// Determine the correct animation to display
		if (direction & FACING_NORTH) {
			current_animation = ANIM_WALKING_NORTH;
		}
		else if (direction & FACING_SOUTH) {
			current_animation = ANIM_WALKING_SOUTH;
		}
		else if (direction & FACING_WEST) {
			current_animation = ANIM_WALKING_WEST;
		}
		else if (direction & FACING_EAST) {
			current_animation = ANIM_WALKING_EAST;
		}
		else {
			cerr << "MAP ERROR: could not find proper movement animation to draw" << endl;
		}

		// If the direction of movement changed in mid-flight, update the animation timer on the
		// new animated image to reflect the old, so the walking animations do not appear to
		// "start and stop" whenever the direction is changed.
		if (current_animation != last_animation) {
			animations[current_animation].SetTimeProgress(animations[last_animation].GetTimeProgress());
			animations[last_animation].SetTimeProgress(0);
		}
		animations[current_animation].Update();

		was_moving = true;
	} // if (moving)
} // void MapSprite::Update()


// Draw the appropriate sprite frame at the correct position on the screen
void MapSprite::Draw() {
	if (MapObject::DrawHelper() == true) {
		VideoManager->DrawImage(animations[current_animation]);
		VirtualSprite::Draw();
	}
}



void MapSprite::SaveState() {
	VirtualSprite::SaveState();

	_saved_was_moving = was_moving;
	_saved_walk_sound = walk_sound;
	_saved_current_animation = current_animation;
}



bool MapSprite::LoadState() {
	if (!VirtualSprite::LoadState())
		return false;

	was_moving = _saved_was_moving;
	walk_sound = _saved_walk_sound;
	current_animation = _saved_current_animation;

	return true;
}

// *****************************************************************************
// ********************** EnemySprite Class Functions ************************
// *****************************************************************************

EnemySprite::EnemySprite() :
	_zone(NULL),
	_color(1.0f, 1.0f, 1.0f, 0.0f),
	_aggro_range(8.0f),
	_time_dir_change(2500),
	_time_to_spawn(3500),
	_music_theme("")
{
	filename = "";
	MapObject::_object_type = ENEMY_TYPE;
	moving = true;
	Reset();
}

EnemySprite::EnemySprite(std::string file) :
	_zone(NULL),
	_color(1.0f, 1.0f, 1.0f, 0.0f),
	_aggro_range(8.0f),
	_time_dir_change(2500),
	_time_to_spawn(3500)
{
	filename = file;
	MapObject::_object_type = ENEMY_TYPE;
	moving = true;
	Reset();
}


// Load in the appropriate images and other data for the sprite from a Lua file
bool EnemySprite::Load() {
	ReadScriptDescriptor sprite_script;
	if (sprite_script.OpenFile(filename) == false) {
		return false;
	}

	ScriptCallFunction<void>(sprite_script.GetLuaState(), "Load", this);
	string sprite_sheet = sprite_script.ReadString("sprite_sheet");
	return MapSprite::LoadStandardAnimations(sprite_sheet);
}



void EnemySprite::Reset() {
	updatable = false;
	no_collision = true;
	_state = DEAD;
	_time_elapsed = 0;
	_color.SetAlpha(0.0f);
	_out_of_zone = false;
}



void EnemySprite::AddEnemy(uint32 enemy_id) {
	if (_enemy_parties.empty()) {
		if (MAP_DEBUG) {
			cerr << "MAP WARNING: In EnemySprite::AddEnemy, can not add new enemy when no parties have been declared" << endl;
		}
		return;
	}

	_enemy_parties.back().push_back(enemy_id);

	// Make sure that the GlobalEnemy has already been created for this enemy_id
	if (MAP_DEBUG) {
		bool found = false;
		for (uint32 i = 0; i < MapMode::_loading_map->_enemies.size(); i++) {
			if (MapMode::_loading_map->_enemies[i]->GetID() == enemy_id) {
				found = true;
				break;
			}
		}

		if (found == false) {
			cerr << "MAP WARNING: In EnemySprite::AddEnemy, enemy to add has id " << enemy_id
				<< ", which does not exist in MapMode::_enemies" << endl;
		}
	}
}



const std::vector<uint32>& EnemySprite::RetrieveRandomParty() {
	if (_enemy_parties.empty()) {
		cerr << "MAP ERROR: Called EnemySprite::RetreiveRandomParty when no enemy parties existed!" << endl;
		exit(1);
	}

	return _enemy_parties[rand() % _enemy_parties.size()];
}



void EnemySprite::Update() {
	if( current_action != -1 ) {
		MapSprite::Update();
		return;
	}

	switch (_state) {
		// Gradually increase the alpha while the sprite is fading in during spawning
		case SPAWNING:
			_time_elapsed += SystemManager->GetUpdateTime();
			if (_color.GetAlpha() < 1.0f) {
				_color.SetAlpha((_time_elapsed / static_cast<float>(_time_to_spawn)) * 1.0f);
			}
			else {
				ChangeStateHostile();
			}
			break;

		// Set the sprite's direction so that it seeks to collide with the map camera's position
		case HOSTILE:
			// Holds the x and y deltas between the sprite and map camera coordinate pairs
			float xdelta, ydelta;
			_time_elapsed += SystemManager->GetUpdateTime();

			xdelta = ComputeXLocation() - MapMode::_current_map->_camera->ComputeXLocation();
			ydelta = ComputeYLocation() - MapMode::_current_map->_camera->ComputeYLocation();

			// If the sprite has moved outside of its zone and it should not, reverse the sprite's direction
			if ( _zone != NULL && _zone->IsInsideZone(x_position, y_position) == false && _zone->IsRestraining() ) {
				// Make sure it wasn't already out (stuck on boundaries fix)
				if( !_out_of_zone )
				{
					SetDirection(CalculateOppositeDirection(GetDirection()));
					// The sprite is now finding its way back into the zone
					_out_of_zone = true;
				}
			}
			// Otherwise, determine the direction that the sprite should move if the camera is within the sprite's aggression range
			else {
				_out_of_zone = false;

				// Enemies will only aggro if the camera is inside the zone, or the zone is non-restrictive
				// The order of comparaisons here is important, the NULL check MUST come before the rest or a null pointer exception could happen if no zone is registered
				if ( _zone == NULL || ( abs(xdelta) <= _aggro_range && abs(ydelta) <= _aggro_range 
					 && (!_zone->IsRestraining() || _zone->IsInsideZone(MapMode::_current_map->_camera->x_position, MapMode::_current_map->_camera->y_position)) ) )
				{
					if (xdelta > -0.5 && xdelta < 0.5 && ydelta < 0)
						SetDirection(SOUTH);
					else if (xdelta > -0.5 && xdelta < 0.5 && ydelta > 0)
						SetDirection(NORTH);
					else if (ydelta > -0.5 && ydelta < 0.5 && xdelta > 0)
						SetDirection(WEST);
					else if (ydelta > -0.5 && ydelta < 0.5 && xdelta < 0)
						SetDirection(EAST);
					else if (xdelta < 0 && ydelta < 0)
						SetDirection(SOUTHEAST);
					else if (xdelta < 0 && ydelta > 0)
						SetDirection(NORTHEAST);
					else if (xdelta > 0 && ydelta < 0)
						SetDirection(SOUTHWEST);
					else
						SetDirection(NORTHWEST);
				}
				// If the sprite is not within the aggression range, pick a random direction to move
				else {
					if (_time_elapsed >= GetTimeToChange()) {
						// TODO: needs comment
						SetDirection(1 << hoa_utils::RandomBoundedInteger(0,11));
						_time_elapsed = 0;
					}
				}
			}

			MapSprite::Update();
			break;

		// Do nothing if the sprite is in the DEAD state, or any other state
		case DEAD:
		default:
			break;
	}
} // void EnemySprite::Update()



void EnemySprite::Draw() {
	// Otherwise, only draw it if it is not in the DEAD state
	if (_state != DEAD && MapObject::DrawHelper() == true) {
		VideoManager->DrawImage(animations[current_animation], _color);
		return;
	}
}

} // namespace private_map

} // namespace hoa_map
