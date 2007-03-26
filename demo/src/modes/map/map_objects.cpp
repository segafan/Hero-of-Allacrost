///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_objects.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for map mode objects.
*** ***************************************************************************/

#include "utils.h"
#include "map.h"
#include "map_objects.h"
#include "map_dialogue.h"
#include "map_actions.h"
#include "audio.h"
#include "video.h"
#include "system.h"
#include "script.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_script;

namespace hoa_map {

namespace private_map {

// *****************************************************************************
// ************************ MapObject Class Functions **************************
// *****************************************************************************

MapObject::MapObject() :
	object_id(-1),
	context(-1),
	x_position(-1),
	y_position(-1),
	x_offset(0.0f),
	y_offset(0.0f),
	img_half_width(0.0f),
	img_height(0.0f),
	coll_half_width(0.0f),
	coll_height(0.0f),
	updatable(true),
	visible(true),
	no_collision(false),
	draw_on_second_pass(false)
{}



bool MapObject::DrawHelper() {
	if (visible == false)
		return false;

	// Store the full x and y position coordinates of the sprite in a single pair of variables
	float x_pos = static_cast<float>(x_position) + x_offset;
	float y_pos = static_cast<float>(y_position) + y_offset;

	// ---------- (1) Determine if the sprite is off-screen and if so, don't draw it.

	if (x_pos + img_half_width < MapMode::_current_map->_draw_info.left_edge  ||
		x_pos - img_half_width > MapMode::_current_map->_draw_info.right_edge ||
		y_pos - img_height > MapMode::_current_map->_draw_info.bottom_edge    ||
		y_pos < MapMode::_current_map->_draw_info.top_edge) {
		return false;
	}

	// ---------- (2) Calculate the drawing coordinates and move the drawing cursor

	VideoManager->Move(x_pos - MapMode::_current_map->_draw_info.left_edge, y_pos - MapMode::_current_map->_draw_info.top_edge);
	return true;
} // bool MapObject::DrawHelper()

// ****************************************************************************
// ********************* PhysicalObject Class Functions ***********************
// ****************************************************************************

PhysicalObject::PhysicalObject() :
	current_animation(0)
{
	MapObject::_object_type = PHYSICAL_TYPE;
}



PhysicalObject::~PhysicalObject() {
	animations.clear();
}



void PhysicalObject::Update() {
	if (updatable)
		animations[current_animation].Update();
}



void PhysicalObject::Draw() {
	if (MapObject::DrawHelper() == true)
		VideoManager->DrawImage(animations[current_animation]);
}

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
	_saved(false)
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



uint16 VirtualSprite::CalculateOppositeDirection(const uint16 direction) {
	switch( direction )
	{
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
		cerr << "MAP ERROR: VirtualSprite::CalculateOppositeDirection received invalid direction" << endl;
		return SOUTH;
	}
}



void VirtualSprite::Update() {
	if (!updatable) {
		return;
	}

	// If the sprite was not forced to do a certain action
	if (forced_action < 0 ) {
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

		// Move the sprite the appropriate distance in the appropriate Y direction
		switch (direction) {
			case NORTH:
				y_offset -= distance_moved; break;
			case SOUTH:
				y_offset += distance_moved; break;
			case WEST: break;
			case EAST: break;
			case NW_NORTH:
			case NW_WEST:
				y_offset -= distance_moved; break;
			case SW_SOUTH:
			case SW_WEST:
				y_offset += distance_moved; break;
			case NE_NORTH:
			case NE_EAST:
				y_offset -= distance_moved; break;
			case SE_SOUTH:
			case SE_EAST:
				y_offset += distance_moved; break;
			default:
				cerr << "MAP ERROR: sprite trying to move in an invalid direction" << endl;
				return;
		} // switch (direction)

		// Determine if the sprite may move to this new Y position
		if ( MapMode::_current_map->_DetectCollision(this) )
			y_offset = tmp_y;

		// Roll-over Y position offsets if necessary
		while (y_offset < 0.0f) {
			y_position -= 1;
			y_offset += 1.0f;
		}
		while (y_offset > 1.0f) {
			y_position += 1;
			y_offset -= 1.0f;
		}

		// Move the sprite the appropriate distance in the appropriate X direction
		switch (direction) {
			case NORTH: break;
			case SOUTH: break;
			case WEST:
				x_offset -= distance_moved; break;
			case EAST:
				x_offset += distance_moved; break;
			case NW_NORTH:
			case NW_WEST:
				x_offset -= distance_moved; break;
			case SW_SOUTH:
			case SW_WEST:
				x_offset -= distance_moved; break;
			case NE_NORTH:
			case NE_EAST:
				x_offset += distance_moved; break;
			case SE_SOUTH:
			case SE_EAST:
				x_offset += distance_moved; break;
			default:
				cerr << "MAP ERROR: sprite trying to move in an invalid direction" << endl;
				return;
		} // switch (direction)

		// Determine if the sprite may move to this new X position
		if (MapMode::_current_map->_DetectCollision(this))
			x_offset = tmp_x;

		// Roll-over X position offsets if necessary
		while (x_offset < 0.0f) {
			x_position -= 1;
			x_offset += 1.0f;
		}
		while (x_offset > 1.0f) {
			x_position += 1;
			x_offset -= 1.0f;
		}
	} // if (moving)
} // void VirtualSprite::Update()



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
}



bool VirtualSprite::LoadState() {
	if(_saved == false)
		return false;

	 direction = _saved_direction;
	 movement_speed = _saved_movement_speed;
	 moving = _saved_moving;
	 name = _saved_name;
	 current_action = _saved_current_action;

	 return true;
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
	if (MAP_DEBUG)
		cout << "MAP: MapSprite constructor invoked" << endl;

	MapObject::_object_type = SPRITE_TYPE;
	VirtualSprite::face_portrait = 0;
}


// Free all allocated images and other data
MapSprite::~MapSprite() {
	if (MAP_DEBUG)
		cout << "MAP: MapSprite destructor invoked" << endl;

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

	if (VideoManager->LoadMultiImage(frames, filename, 4, 6) == false) {
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
	animations[4].AddFrame(frames[4], frame_speed);
	animations[4].AddFrame(frames[5], frame_speed);

	animations[5].AddFrame(frames[7], frame_speed);
	animations[5].AddFrame(frames[8], frame_speed);
	animations[5].AddFrame(frames[9], frame_speed);
	animations[5].AddFrame(frames[10], frame_speed);
	animations[5].AddFrame(frames[11], frame_speed);

	animations[6].AddFrame(frames[13], frame_speed);
	animations[6].AddFrame(frames[14], frame_speed);
	animations[6].AddFrame(frames[15], frame_speed);
	animations[6].AddFrame(frames[16], frame_speed);
	animations[6].AddFrame(frames[17], frame_speed);

	animations[7].AddFrame(frames[19], frame_speed);
	animations[7].AddFrame(frames[20], frame_speed);
	animations[7].AddFrame(frames[21], frame_speed);
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
	if ( !updatable )
		return;

	// Set the sprite's animation to the standing still position if movement has just stopped
	if (!moving) {
		if (was_moving) {
			// Set the current movement animation to zero progress
			animations[current_animation].SetTimeProgress(0);
			was_moving = false;
		}

		// Determine the correct standing frame to display
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
	if (MapObject::DrawHelper() == true)
		VideoManager->DrawImage(animations[current_animation]);
}



void MapSprite::SaveState() {
	VirtualSprite::SaveState();

	_saved_was_moving = was_moving;
	_saved_walk_sound = walk_sound;
	_saved_current_animation = current_animation;
}



bool MapSprite::LoadState() {
	if( !VirtualSprite::LoadState() )
		return false;

	was_moving = _saved_was_moving;
	walk_sound = _saved_walk_sound;
	current_animation = _saved_current_animation;

	return true;
}

// *****************************************************************************
// ********************** EnemySprite Class Functions ************************
// *****************************************************************************

void EnemySprite::Draw()
{
	if( _state != DEAD ) {
		if (MapObject::DrawHelper() == true) {
			VideoManager->DrawImage(animations[current_animation],_color);
		}
	}
}

// Load in the appropriate images and other data for the sprite
bool EnemySprite::Load() {
	ScriptDescriptor sprite_script;
	if (sprite_script.OpenFile(filename, SCRIPT_READ) == false) {
		return false;
	}

	ScriptCallFunction<void>(sprite_script.GetLuaState(), "Load", this);

	string sprite_sheet = sprite_script.ReadString("sprite_sheet");
	uint32 multi_img_rows = sprite_script.ReadInt("sprite_sheet_rows");
	uint32 multi_img_cols = sprite_script.ReadInt("sprite_sheet_cols");

	uint32 frame_speed = sprite_script.ReadInt("frame_speed");

	AnimatedImage img;
	vector<StillImage> frames(multi_img_rows * multi_img_cols);
	for (uint32 i = 0; i < ( multi_img_rows * multi_img_cols ); ++i)
		frames[i].SetDimensions(img_half_width * 2, img_height);

	if (VideoManager->LoadMultiImage(frames, sprite_sheet, multi_img_rows, multi_img_cols) == false) {
		return false;
	}

	vector<int32> frames_vector;
	//Load Standing South
	sprite_script.ReadIntVector("standing_south_frames", frames_vector );
	for( size_t i = 0; i < frames_vector.size(); ++i ) {
		img.AddFrame( frames[ frames_vector[ i ] ], frame_speed);
	}
	animations.push_back(img);

	//Load Standing North
	img.Clear();
	frames_vector.clear();
	sprite_script.ReadIntVector("standing_north_frames", frames_vector );
	for( size_t i = 0; i < frames_vector.size(); ++i ) {
		img.AddFrame( frames[ frames_vector[ i ] ], frame_speed);
	}
	animations.push_back(img);

	// Load Standing West
	img.Clear();
	frames_vector.clear();
	sprite_script.ReadIntVector("standing_west_frames", frames_vector );
	for( size_t i = 0; i < frames_vector.size(); ++i ) {
		img.AddFrame( frames[ frames_vector[ i ] ], frame_speed);
	}
	animations.push_back(img);

	// Load Standing East
	img.Clear();
	frames_vector.clear();
	sprite_script.ReadIntVector("standing_east_frames", frames_vector );
	for( size_t i = 0; i < frames_vector.size(); ++i ) {
		img.AddFrame( frames[ frames_vector[ i ] ], frame_speed);
	}
	animations.push_back(img);

	// Load Walking South
	img.Clear();
	frames_vector.clear();
	sprite_script.ReadIntVector("walking_south_frames", frames_vector );
	for( size_t i = 0; i < frames_vector.size(); ++i ) {
		img.AddFrame( frames[ frames_vector[ i ] ], frame_speed);
	}
	animations.push_back(img);

	// Load Walking North
	img.Clear();
	frames_vector.clear();
	sprite_script.ReadIntVector("walking_north_frames", frames_vector );
	for( size_t i = 0; i < frames_vector.size(); ++i ) {
		img.AddFrame( frames[ frames_vector[ i ] ], frame_speed);
	}
	animations.push_back(img);

	// Load Walking West
	img.Clear();
	frames_vector.clear();
	sprite_script.ReadIntVector("walking_west_frames", frames_vector );
	for( size_t i = 0; i < frames_vector.size(); ++i ) {
		img.AddFrame( frames[ frames_vector[ i ] ], frame_speed);
	}
	animations.push_back(img);

	// Load Walking East
	img.Clear();
	frames_vector.clear();
	sprite_script.ReadIntVector("walking_east_frames", frames_vector );
	for( size_t i = 0; i < frames_vector.size(); ++i ) {
		img.AddFrame( frames[ frames_vector[ i ] ], frame_speed);
	}
	animations.push_back(img);

	for (uint32 i = 0; i < animations.size(); i++) {
		if (animations[i].Load() == false) {
			cerr << "MAP ERROR: failed to load sprite animation" << endl;
			return false;
		}
	}

	return true;
} // bool EnemySprite::Load()



void EnemySprite::Update() {
	float xdelta, ydelta;
	switch( _state )
	{
	case SPAWNING:
		_time_elapsed += hoa_system::SystemManager->GetUpdateTime();
		if( _color.GetAlpha() < 1.0f ) {
			_color.SetAlpha( ( _time_elapsed / static_cast<float>(GetTimeToSpawn()) ) * 1.0f );
		}
		else
		{
			ChangeStateHostile();
		}
		break;

	case HOSTILE:
		_time_elapsed += hoa_system::SystemManager->GetUpdateTime();

		xdelta = ComputeXLocation() - hoa_map::MapMode::_current_map->_camera->ComputeXLocation();
		ydelta = ComputeYLocation() - hoa_map::MapMode::_current_map->_camera->ComputeYLocation();

		if( !MapZone::IsInsideZone( x_position, y_position, _zone ) && _zone->IsRestraining() ) {
			SetDirection( CalculateOppositeDirection( GetDirection() ) );
		}
		else {
			if( abs(xdelta) <= _aggro_range && abs(ydelta) <= _aggro_range && MapZone::IsInsideZone( hoa_map::MapMode::_current_map->_camera->x_position, hoa_map::MapMode::_current_map->_camera->y_position, _zone ) )
			{
				if( xdelta > -0.5 && xdelta < 0.5 && ydelta < 0 )
					SetDirection( SOUTH );
				else
					if( xdelta > -0.5 && xdelta < 0.5 && ydelta > 0 )
						SetDirection( NORTH );
					else
						if( ydelta > -0.5 && ydelta < 0.5 && xdelta > 0 )
							SetDirection( WEST );
						else
							if( ydelta > -0.5 && ydelta < 0.5 && xdelta < 0 )
								SetDirection( EAST );
							else
								if( xdelta < 0 && ydelta < 0 )
									SetDirection( SOUTHEAST );
								else
									if( xdelta < 0 && ydelta > 0 )
										SetDirection( NORTHEAST );
									else
										if( xdelta > 0 && ydelta < 0 )
											SetDirection( SOUTHWEST );
										else
											SetDirection( NORTHWEST );
			}
			else {
				if( _time_elapsed >= GetTimeToChange() ) {
					SetDirection( 1 << hoa_utils::RandomBoundedInteger(0,11) );
					_time_elapsed = 0;
				}
			}
		}

		MapSprite::Update();
		break;

	default:
		break;
	}
} // void EnemySprite::Update()



} // namespace private_map

} // namespace hoa_map
