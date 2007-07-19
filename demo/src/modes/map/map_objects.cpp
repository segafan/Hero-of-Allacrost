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
#include "script.h"
#include "system.h"
#include "global.h"

#include <algorithm> //For std::replace in the ChestObject saving code

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_script;
using namespace hoa_system;
using namespace hoa_global;

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

// *****************************************************************************
// *********************** ChestObject Class Functions *************************
// *****************************************************************************

// These constants are used to identify the default settings of a ChestObject
const uint8 ChestObject::NB_FRAMES_CLOSED_DEFAULT = 1;
const uint8 ChestObject::NB_FRAMES_OPENING_DEFAULT = 3;;

const uint32 ChestObject::HIDE_FORCE_DEFAULT = 0;	//! By default a ChestObject will not be hidden


ChestObject::ChestObject( std::string image_file, uint8 nb_frames_closed, uint8 nb_frames_opening, uint32 hide_force )
	: _hide_force( hide_force )
{
	MapObject::_object_type = CHEST_TYPE;

	const uint32 DEFAULT_FRAME_TIME = 10;

	std::vector<StillImage> frames;

	//--(1)-- Load a 1 row image strip of the length of all the needed frames
	if( VideoManager->LoadMultiImageFromNumberElements( frames, image_file, 1, nb_frames_closed + nb_frames_opening ) == false ) {
		cerr << "File: " << filename << " could not be loaded correctly." << endl;
		exit( 1 );
	}

	//--(2)-- Update the frames size to the map coords size
	for( size_t i = 0; i < frames.size(); ++i ) {
		frames[i].SetWidth( frames[i].GetWidth() / HALF_TILE_COLS );
		//Why ho why does it work like this!! It's so counter-intuitive
		frames[i].SetHeight( frames[i].GetHeight() / HALF_TILE_COLS );
	}

	//--(3)-- Create the two needed animations
	hoa_video::AnimatedImage closed_anim;
	closed_anim.SetNumberLoops( 0 );

	//Make sure that a frame number was set
	if( nb_frames_closed > 0 ) {
		for( uint8 i = 0; i < nb_frames_closed; ++i ) {
			closed_anim.AddFrame( frames[ i ], DEFAULT_FRAME_TIME );
		}
	}
	else {
		//Check if the other animation has frames set and use it instead
		if( nb_frames_opening > 0 ) {
			for( uint8 i = 0; i < nb_frames_opening; ++i ) {
				closed_anim.AddFrame( frames[ i ], DEFAULT_FRAME_TIME );
			}
		} else {
			//No frames set, we can't do anything
			cerr << "File: " << filename << " could not be loaded correctly. No frames set as closed ChestObject animation." << endl;
			exit( 1 );
		}
	}

	hoa_video::AnimatedImage opening_anim;
	opening_anim.SetNumberLoops( 0 );

	//Make sure that a frame number was set
	if( nb_frames_opening > 0 ) {
		for( uint8 i = nb_frames_closed; i < ( nb_frames_opening + nb_frames_closed ); ++i ) {
			opening_anim.AddFrame( frames[ i ], DEFAULT_FRAME_TIME );
		}
	} else {
		//Check if the other animation has frames set and use it instead
		if( nb_frames_closed > 0 ) {
			for( uint8 i = 0; i < nb_frames_closed; ++i ) {
				opening_anim.AddFrame( frames[ i ], DEFAULT_FRAME_TIME );
			}
		} else {
			//No frames set, we can't do anything
			cerr << "File: " << filename << " could not be loaded correctly. No frames set as opening ChestObject animation." << endl;
			exit( 1 );
		}
	}

	//--(4)-- Add the animations to the parent PhysicalObject
	AddAnimation( closed_anim );
	AddAnimation( opening_anim );

	//--(5)-- Set the parent's MapObject params
	SetCollHalfWidth( frames[0].GetWidth() / 2.0f );
	SetCollHeight( frames[0].GetHeight() );
}

ChestObject::~ChestObject() {
	for( size_t i = 0; i < _objects_list.size(); ++i ) {
		delete _objects_list[i];
	}
}

bool ChestObject::UpdateHideForce( uint32 force_applied ) {
	if( _hide_force - force_applied < 0 ) {
		_hide_force = 0;
		return true;
	}
	else {
		_hide_force -= force_applied;
		return false;
	}
}



bool ChestObject::AddObject( uint32 id, uint32 number ) {
	hoa_global::GlobalObject * obj = GlobalCreateNewObject( id, number );

	if( !obj )
	{
		if( MAP_DEBUG )
			cerr << "MAP WARNING: ChestObject::AddObject() called with invalid item id" << endl;
		return false;
	}
	else {
		_objects_list.push_back( obj );
	}
	return true;
}



void ChestObject::Use() {

	//TODO: Create a class to show items gained in the map code
	if( !IsUsed() ) {
		for( size_t i = 0; i < _objects_list.size(); ++i ) {
			GlobalManager->AddToInventory( _objects_list[i] );
		}

		SetCurrentAnimation( OPENING_CHEST_ANIM );

		_objects_list.clear();

		//Create an Event Group called "path_to_map_file_lua_chests"
		std::string group_name = MapMode::_current_map->_map_filename + "_chests";
		std::replace( group_name.begin(), group_name.end(), '/', '_' );
		std::replace( group_name.begin(), group_name.end(), '.', '_' );

		if( !GlobalManager->DoesEventGroupExist( group_name ) ) {
			GlobalManager->AddNewEventGroup( group_name );
		}

		//Add an event in the group having the ObjectID of the chest as name
		std::string event_name;
		DataToString( event_name, GetObjectID() );
		if( GlobalManager->DoesEventExist( group_name, event_name ) ) {
			GlobalManager->GetEventGroup( group_name )->SetEvent( event_name, CHEST_USED );
		} else {
			GlobalManager->GetEventGroup( group_name )->AddNewEvent( event_name, CHEST_USED );
		}
	}
}



void ChestObject::LoadSaved() {
	//Check for an Event Group called "path_to_map_file_lua_chests"
	std::string group_name = MapMode::_loading_map->_map_filename + "_chests";
	std::replace( group_name.begin(), group_name.end(), '/', '_' );
	std::replace( group_name.begin(), group_name.end(), '.', '_' );

	if( GlobalManager->DoesEventGroupExist( group_name ) ) {
		//Add an event in the group having the ObjectID of the chest as name
		std::string event_name;
		DataToString( event_name, GetObjectID() );
		if( GlobalManager->DoesEventExist( group_name, event_name ) ) {
			if( GlobalManager->GetEventValue( group_name, event_name ) == CHEST_USED ) {
				Clear();
			}
		}
	}
}


} // namespace private_map

} // namespace hoa_map
