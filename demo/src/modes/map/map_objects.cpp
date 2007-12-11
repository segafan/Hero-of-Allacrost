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

#include <algorithm> //For std::replace in the MapTreasure saving code

#include "utils.h"

#include "audio.h"
#include "system.h"

#include "global.h"

#include "map.h"
#include "map_objects.h"
#include "map_dialogue.h"
#include "map_actions.h"

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
		animations[current_animation].Draw();
}

// *****************************************************************************
// *********************** MapTreasure Class Functions *************************
// *****************************************************************************

MapTreasure::MapTreasure(string image_file, uint8 num_total_frames, uint8 num_closed_frames, uint8 num_open_frames) :
	_empty(false),
	_drunes(0)
{
	MapObject::_object_type = TREASURE_TYPE;
	const uint32 DEFAULT_FRAME_TIME = 10; // The default number of milliseconds for frame animations
	std::vector<StillImage> frames;

	// (1) Load a the single row, multi column multi image containing all the treasure frames
	if (ImageDescriptor::LoadMultiImageFromElementGrid(frames, image_file, 1, num_total_frames) == false ) {
		cerr << "File: " << filename << " could not be loaded correctly." << endl;
		// TODO: throw exception
		return;
	}
	// Update the frame image sizes to work in the MapMode coordinate system
	for (uint32 i = 0; i < frames.size(); i++) {
		frames[i].SetWidth(frames[i].GetWidth() / HALF_TILE_COLS);
		frames[i].SetHeight(frames[i].GetHeight() / HALF_TILE_COLS);
	}

	// (2) Now that we know the total number of frames in the image, make sure the frame count arguments make sense
	if (num_closed_frames == 0 || num_open_frames == 0 || num_closed_frames >= num_total_frames || num_open_frames >= num_total_frames) {
		// TODO: throw exception
		return;
	}

	// (3) Dissect the frames and create the closed, opening, and open animations Create the close and opening animations for the chest
	hoa_video::AnimatedImage closed_anim;
	for (uint8 i = 0; i < num_closed_frames; i++) {
		closed_anim.AddFrame(frames[i], DEFAULT_FRAME_TIME);
	}

	hoa_video::AnimatedImage open_anim;
	for (uint8 i = num_total_frames - num_open_frames; i < num_total_frames; i++) {
		open_anim.AddFrame(frames[i], DEFAULT_FRAME_TIME);
	}

	hoa_video::AnimatedImage opening_anim;
	opening_anim.SetNumberLoops(0); // Only loop the opening animation once, not infinitely
	if (num_total_frames - num_closed_frames - num_open_frames <= 0) {
		opening_anim = open_anim;
	}
	else {
		for (uint8 i = 0 + num_closed_frames; i < num_total_frames - num_open_frames; i++) {
			opening_anim.AddFrame(frames[i], DEFAULT_FRAME_TIME);
		}
	}

	AddAnimation(closed_anim);
	AddAnimation(opening_anim);
	AddAnimation(open_anim);

	// (4) Set the collision rectangle according to the dimensions of the first frame
	SetCollHalfWidth(frames[0].GetWidth() / 2.0f);
	SetCollHeight(frames[0].GetHeight());
} // MapTreasure::MapTreasure(string image_file, uint8 num_closed_frames = 1, uint8 num_open_frames = 1)



MapTreasure::~MapTreasure() {
	for (uint32 i = 0; i < _objects_list.size(); i++) {
		delete _objects_list[i];
	}
}



void MapTreasure::LoadSaved() {
	//Add an event in the group having the ObjectID of the chest as name
	string event_name = "chest_" + NumberToString(GetObjectID());
	if (MapMode::_loading_map->_map_event_group->DoesEventExist(event_name)) {
		if (MapMode::_loading_map->_map_event_group->GetEvent(event_name) == TREASURE_EMPTY) {
			SetCurrentAnimation(OPEN_ANIM);
			_drunes = 0;
			for (uint32 i = 0; i < _objects_list.size(); i++)
				delete _objects_list[i];
			_objects_list.clear();
			_empty = true;
		}
	}
}



bool MapTreasure::AddObject(uint32 id, uint32 number) {
	hoa_global::GlobalObject* obj = GlobalCreateNewObject(id, number);

	if (obj == NULL) {
		IF_PRINT_WARNING(MAP_DEBUG) << "invalid object id argument passed to function" << endl;
		return false;
	}

	_objects_list.push_back(obj);
	return true;
}



void MapTreasure::Update() {
	PhysicalObject::Update();
	if (current_animation == OPENING_ANIM && animations[OPENING_ANIM].IsLoopsFinished() == true) {
		SetCurrentAnimation(OPEN_ANIM);
	}
}



void MapTreasure::Open() {
	if (_empty == true) {
		IF_PRINT_WARNING(MAP_DEBUG) << "attempted to open an empty map treasure" << endl;
		return;
	}

	SetCurrentAnimation(OPENING_ANIM);

	// Add an event to the map group indicating that the chest has now been opened
	string event_name = "chest_" + NumberToString(GetObjectID());

	if (MapMode::_current_map->_map_event_group->DoesEventExist(event_name) == true) {
		MapMode::_current_map->_map_event_group->SetEvent(event_name, TREASURE_EMPTY);
	}
	else {
		MapMode::_current_map->_map_event_group->AddNewEvent(event_name, TREASURE_EMPTY);
	}

	// Initialize the treasure menu to display the contents of the open treasure
	MapMode::_current_map->_treasure_menu->Initialize(this);
}

} // namespace private_map

} // namespace hoa_map
