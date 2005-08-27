///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    map_objects.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 20th, 2005
 * \brief   Source file for map mode objects.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include "map.h"
#include "map_objects.h"
#include "audio.h"
#include "video.h"
#include "global.h"
#include "data.h"
#include "battle.h"
#include "menu.h"

using namespace std;
using namespace hoa_map::private_map;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_engine;
using namespace hoa_global;
using namespace hoa_data;
using namespace hoa_battle;
using namespace hoa_menu;

namespace hoa_map {

// ****************************************************************************
// *********************** ObjectLayer Class Functions ************************
// ****************************************************************************

// Initialize the members and setup the pointer to the GameVideo class
ObjectLayer::ObjectLayer(uint8 type, uint32 row, uint32 col, uint8 alt, uint16 stat) {
	_object_type = type;
	_row_pos = row;
	_col_pos = col;
	_altitude = alt;
	_status = stat;
	VideoManager = hoa_video::GameVideo::_GetReference();
}


// Destructor
ObjectLayer::~ObjectLayer() {}

// ****************************************************************************
// ************************ MapSprite Class Functions *************************
// ****************************************************************************

// Constructor for critical class members. Other members are initialized via support functions
MapSprite::MapSprite(uint8 type, uint32 row, uint32 col, uint8 alt, uint16 stat)
                     : ObjectLayer(type, row, col, alt, stat) {
	if (MAP_DEBUG) cout << "MAP: MapSprite constructor invoked" << endl;
	_step_speed = NORMAL_SPEED;
	_step_count = 0;
	_delay_time = NORMAL_DELAY;
	_wait_time = 0;
	_name = "";
	_filename = "";
	_frames = NULL;
	_speech = new SpriteDialogue();
}


// Free all the frames from memory
MapSprite::~MapSprite() {
	if (MAP_DEBUG) cout << "MAP: MapSprite destructor invoked" << endl;
	
	// Character sprite frames are kept globally, so don't delete them.
	if (_object_type == CHARACTER_SPRITE) { 
		return;
	}

	for (uint32 i = 0; i < _frames->size(); i++) {
		VideoManager->DeleteImage((*_frames)[i]);
	}
	delete _frames;

	if (_speech != NULL) {
		delete _speech;
	}
}


// Load the appropriate number of image frames for the sprite
void MapSprite::LoadFrames() {
	ImageDescriptor imd;

	// Prepare standard sprite animation frames (24 count)
	_frames = new vector<ImageDescriptor>;
	imd.SetDimensions(1.0f, 2.0f);

	imd.SetFilename(_filename + "_d1.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_d2.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_d3.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_d4.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_d5.png");
	_frames->push_back(imd);

	imd.SetFilename(_filename + "_u1.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_u2.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_u3.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_u4.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_u5.png");
	_frames->push_back(imd);

	imd.SetFilename(_filename + "_l1.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_l2.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_l3.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_l4.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_l5.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_l6.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_l7.png");
	_frames->push_back(imd);

	imd.SetFilename(_filename + "_r1.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_r2.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_r3.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_r4.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_r5.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_r6.png");
	_frames->push_back(imd);
	imd.SetFilename(_filename + "_r7.png");
	_frames->push_back(imd);

//	// Prepare additional extra frames if the sprite is not a regular NPC
//	if (_status & (CHARACTER_SPRITE | ADV_SPRITE)) {
//
//		// Prepare even -more- extra frames if the sprite is a character
//		if (_status & CHARACTER_SPRITE) {
//
//		}
//	}

	// Load a

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < _frames->size(); i++) {
		VideoManager->LoadImage((*_frames)[i]);
	}
	VideoManager->EndImageLoadBatch();
}


// Loads the frames and other info from the GameInstance singleton
void MapSprite::LoadCharacterInfo(uint32 character) {
	GCharacter *pchar = GameInstance::_GetReference()->GetCharacter(character);
	_name = pchar->GetName();
	_filename = "img/sprite/" + pchar->GetFilename();
	_frames = pchar->GetMapFrames();
}


// Find the frame image that should be drawn
uint32 MapSprite::_FindFrame() {
	uint32 draw_frame; // The frame index that we should draw

	// Depending on the direction the sprite is facing and the step_count, select the correct frame to draw
	switch (_direction) {
		case SOUTH:
		case SOUTH_SW:
		case SOUTH_SE:
			if (_step_count < (0.25 * _step_speed)) {
				draw_frame = DOWN_STANDING;
			}
			else if (_step_count < (0.50 * _step_speed)) {
				if (_status & STEP_SWAP)
					draw_frame = DOWN_RSTEP1;
				else
					draw_frame = DOWN_LSTEP1;
			}
			else if (_step_count < (0.75 * _step_speed)) {
				if (_status & STEP_SWAP)
					draw_frame = DOWN_RSTEP2;
				else
					draw_frame = DOWN_LSTEP2;
			}
			else { // (_step_count < _step_speed) == true
				if (_status & STEP_SWAP)
					draw_frame = DOWN_RSTEP3;
				else
					draw_frame = DOWN_LSTEP3;
			}
			break;
		case NORTH:
		case NORTH_NW:
		case NORTH_NE:
			if (_step_count < (0.25 * _step_speed)) {
				draw_frame = UP_STANDING;
			}
			else if (_step_count < (0.50 * _step_speed)) {
				if (_status & STEP_SWAP)
					draw_frame = UP_RSTEP1;
				else
					draw_frame = UP_LSTEP1;
			}
			else if (_step_count < (0.75 * _step_speed)) {
				if (_status & STEP_SWAP)
					draw_frame = UP_RSTEP2;
				else
					draw_frame = UP_LSTEP2;
			}
			else { // (_step_count < _step_speed) == true
				if (_status & STEP_SWAP)
					draw_frame = UP_RSTEP3;
				else
					draw_frame = UP_LSTEP3;
			}
			break;
		case WEST:
		case WEST_NW:
		case WEST_SW:
			if (_step_count < (0.25 * _step_speed)) {
				draw_frame = LEFT_STANDING;
			}
			else if (_step_count < (0.50 * _step_speed)) {
				if (_status & STEP_SWAP)
					draw_frame = LEFT_RSTEP1;
				else
					draw_frame = LEFT_LSTEP1;
			}
			else if (_step_count < (0.75 * _step_speed)) {
				if (_status & STEP_SWAP)
					draw_frame = LEFT_RSTEP2;
				else
					draw_frame = LEFT_LSTEP2;
			}
			else { // (step_count < step_speed) == true
				if (_status & STEP_SWAP)
					draw_frame = LEFT_RSTEP3;
				else
					draw_frame = LEFT_LSTEP3;
			}
			break;
		case EAST:
		case EAST_NE:
		case EAST_SE:
			if (_step_count < (0.25 * _step_speed)) {
				draw_frame = RIGHT_STANDING;
			}
			else if (_step_count < (0.50 * _step_speed)) {
				if (_status & STEP_SWAP)
					draw_frame = RIGHT_RSTEP1;
				else
					draw_frame = RIGHT_LSTEP1;
			}
			else if (_step_count < (0.75 * _step_speed)) {
				if (_status & STEP_SWAP)
					draw_frame = RIGHT_RSTEP2;
				else
					draw_frame = RIGHT_LSTEP2;
			}
			else { // (_step_count < _step_speed) == true
				if (_status & STEP_SWAP)
					draw_frame = RIGHT_RSTEP3;
				else
					draw_frame = RIGHT_LSTEP3;
			}
			break;
	}
	return draw_frame;
}



// Draw the appropriate sprite frame on the correct position on the screen
void MapSprite::Draw(MapFrame& mf) {
	float x_pos = 0.0;  // The x and y cursor position to draw the sprite to
	float y_pos = 0.0;
	uint32 draw_frame = 0; // The sprite frame index to draw

	// Set the default x and y position (true positions when sprite is not in motion)
	x_pos = mf.c_pos + (static_cast<float>(_col_pos) - static_cast<float>(mf.c_start));
	y_pos = mf.r_pos + (static_cast<float>(mf.r_start) - static_cast<float>(_row_pos));

	// When the sprite is in motion, we have to off-set the step positions
	if (_status & IN_MOTION) {
		switch (_direction) {
			case EAST:
				x_pos -= (_step_speed - _step_count) / _step_speed;
				break;
			case WEST:
				x_pos += (_step_speed - _step_count) / _step_speed;
				break;
			case NORTH:
				y_pos -= (_step_speed - _step_count) / _step_speed;
				break;
			case SOUTH:
				y_pos += (_step_speed - _step_count) / _step_speed;
				break;
			case NORTH_NW:
			case WEST_NW:
				x_pos += (_step_speed - _step_count) / _step_speed;
				y_pos -= (_step_speed - _step_count) / _step_speed;
				break;
			case SOUTH_SW:
			case WEST_SW:
				x_pos += (_step_speed - _step_count) / _step_speed;
				y_pos += (_step_speed - _step_count) / _step_speed;
				break;
			case NORTH_NE:
			case EAST_NE:
				x_pos -= (_step_speed - _step_count) / _step_speed;
				y_pos -= (_step_speed - _step_count) / _step_speed;
				break;
			case SOUTH_SE:
			case EAST_SE:
				x_pos -= (_step_speed - _step_count) / _step_speed;
				y_pos += (_step_speed - _step_count) / _step_speed;
				break;
		}
	}


	draw_frame = _FindFrame();
	VideoManager->Move(x_pos, y_pos);
	VideoManager->DrawImage((*_frames)[draw_frame]);
}

// ****************************************************************************
// ********************* SpriteDialouge Class Functions ***********************
// ****************************************************************************

SpriteDialogue::SpriteDialogue() {
	if (MAP_DEBUG) cout << "MAP: SpriteDialogue constructor invoked" << endl;
	_next_read = 0;
// 	_seen_all = true;
}


SpriteDialogue::~SpriteDialogue() {
	if (MAP_DEBUG) cout << "MAP: SpriteDialogue destructor invoked" << endl;
}

// Load a new set of dialogue
void SpriteDialogue::LoadDialogue(vector<vector<string> > txt) {
	_conversations = txt;
// 	_speaker = sp;
// 	for (uint32 i = 0; i < _dialogue.size(); i++) {
// 		_seen.push_back(false);
// 	}
// 	_seen_all = false;
	
}

// Add a new line of dialogue
void SpriteDialogue::AddDialogue(vector<string> txt) {
	_conversations.push_back(txt);
// 	_speaker.push_back(sp);
// 	_seen.push_back(false);

// 	if (_seen_all) { // Set the next read to point to the new dialogue
// 		_next_read = _dialogue.size() - 1;
// 	}

// 	_seen_all = false;
}


// Add a new line of dialogue, for one-part speeches
void SpriteDialogue::AddDialogue(string txt) {
	vector<string> new_txt(1, txt);
// 	vector<uint32> new_sp(1, sp);
	_conversations.push_back(new_txt);
// 	_speaker.push_back(new_sp);

// 	if (_seen_all) {
// 		_next_read = _dialogue.size() - 1;
// 	}
// 
// 	_seen_all = false;
}

} // namespace hoa_map
