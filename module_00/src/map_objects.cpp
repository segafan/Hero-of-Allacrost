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
ObjectLayer::ObjectLayer(uint8 type, uint32 row, uint32 col, uint32 stat) {
	object_type = type;
	row_pos = row;
	col_pos = col;
	status = stat;
	VideoManager = hoa_video::GameVideo::_GetReference();
}


// Destructor
ObjectLayer::~ObjectLayer() {}

// ****************************************************************************
// ************************ MapSprite Class Functions *************************
// ****************************************************************************

// Constructor for critical class members. Other members are initialized via support functions
MapSprite::MapSprite(uint8 type, uint32 row, uint32 col, uint32 stat)
                     : ObjectLayer(type, row, col, stat) {
	if (MAP_DEBUG) cout << "MAP: MapSprite constructor invoked" << endl;
	step_speed = NORMAL_SPEED;
	step_count = 0;
	delay_time = NORMAL_DELAY;
	wait_time = 0;
	name = "";
	filename = "";
	frames = NULL;
	speech = new SpriteDialogue();
}


// Free all the frames from memory
MapSprite::~MapSprite() {
	if (MAP_DEBUG) cout << "MAP: MapSprite destructor invoked" << endl;
	if (status & SPR_GLOBAL) { // Don't delete these sprite frames
		return;
	}

	for (uint32 i = 0; i < frames->size(); i++) {
		VideoManager->DeleteImage((*frames)[i]);
	}
	delete frames;

	if (speech != NULL) {
		delete speech;
	}
}


// Load the appropriate number of image frames for the sprite
void MapSprite::LoadFrames() {
	ImageDescriptor imd;

	// Prepare standard sprite animation frames (24 count)
	frames = new vector<ImageDescriptor>;
	imd.width = 1;
	imd.height = 2;

	imd.filename = filename + "_d1.png";
	frames->push_back(imd);
	imd.filename = filename + "_d2.png";
	frames->push_back(imd);
	imd.filename = filename + "_d3.png";
	frames->push_back(imd);
	imd.filename = filename + "_d4.png";
	frames->push_back(imd);
	imd.filename = filename + "_d5.png";
	frames->push_back(imd);

	imd.filename = filename + "_u1.png";
	frames->push_back(imd);
	imd.filename = filename + "_u2.png";
	frames->push_back(imd);
	imd.filename = filename + "_u3.png";
	frames->push_back(imd);
	imd.filename = filename + "_u4.png";
	frames->push_back(imd);
	imd.filename = filename + "_u5.png";
	frames->push_back(imd);

	imd.filename = filename + "_l1.png";
	frames->push_back(imd);
	imd.filename = filename + "_l2.png";
	frames->push_back(imd);
	imd.filename = filename + "_l3.png";
	frames->push_back(imd);
	imd.filename = filename + "_l4.png";
	frames->push_back(imd);
	imd.filename = filename + "_l5.png";
	frames->push_back(imd);
	imd.filename = filename + "_l6.png";
	frames->push_back(imd);
	imd.filename = filename + "_l7.png";
	frames->push_back(imd);

	imd.filename = filename + "_r1.png";
	frames->push_back(imd);
	imd.filename = filename + "_r2.png";
	frames->push_back(imd);
	imd.filename = filename + "_r3.png";
	frames->push_back(imd);
	imd.filename = filename + "_r4.png";
	frames->push_back(imd);
	imd.filename = filename + "_r5.png";
	frames->push_back(imd);
	imd.filename = filename + "_r6.png";
	frames->push_back(imd);
	imd.filename = filename + "_r7.png";
	frames->push_back(imd);

//	// Prepare additional extra frames if the sprite is not a regular NPC
//	if (status & (PLAYER_SPRITE | ADV_SPRITE)) {
//
//		// Prepare even -more- extra frames if the sprite is a character
//		if (status & PLAYER_SPRITE) {
//
//		}
//	}

	// Load a

	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < frames->size(); i++) {
		VideoManager->LoadImage((*frames)[i]);
	}
	VideoManager->EndImageLoadBatch();
}


// Loads the frames and other info from the GameInstance singleton
void MapSprite::LoadCharacterInfo(uint32 character) {
	GCharacter *pchar = GameInstance::_GetReference()->GetCharacter(character);
	name = pchar->GetName();
	filename = "img/sprite/" + pchar->GetFilename();
	status |= SPR_GLOBAL; // Safety so we don't accdidentally delete the sprite frames
	frames = pchar->GetMapFrames();
// 	VideoManager->DrawImage((*frames)[0]);
// 	cout << "Frames[0].filename = " << (*frames)[0].filename << endl;
}


// Find the frame image that should be drawn
uint32 MapSprite::FindFrame() {
	uint32 draw_frame; // The frame index that we should draw

	// Depending on the direction the sprite is facing and the step_count, select the correct frame to draw
	switch (status & FACE_MASK) {
		case SOUTH:
		case SOUTH_SW:
		case SOUTH_SE:
			if (step_count < (0.25 * step_speed)) {
				draw_frame = DOWN_STANDING;
			}
			else if (step_count < (0.50 * step_speed)) {
				if (status & STEP_SWAP)
					draw_frame = DOWN_RSTEP1;
				else
					draw_frame = DOWN_LSTEP1;
			}
			else if (step_count < (0.75 * step_speed)) {
				if (status & STEP_SWAP)
					draw_frame = DOWN_RSTEP2;
				else
					draw_frame = DOWN_LSTEP2;
			}
			else { // (step_count < step_speed) == true
				if (status & STEP_SWAP)
					draw_frame = DOWN_RSTEP3;
				else
					draw_frame = DOWN_LSTEP3;
			}
			break;
		case NORTH:
		case NORTH_NW:
		case NORTH_NE:
			if (step_count < (0.25 * step_speed)) {
				draw_frame = UP_STANDING;
			}
			else if (step_count < (0.50 * step_speed)) {
				if (status & STEP_SWAP)
					draw_frame = UP_RSTEP1;
				else
					draw_frame = UP_LSTEP1;
			}
			else if (step_count < (0.75 * step_speed)) {
				if (status & STEP_SWAP)
					draw_frame = UP_RSTEP2;
				else
					draw_frame = UP_LSTEP2;
			}
			else { // (step_count < step_speed) == true
				if (status & STEP_SWAP)
					draw_frame = UP_RSTEP3;
				else
					draw_frame = UP_LSTEP3;
			}
			break;
		case WEST:
		case WEST_NW:
		case WEST_SW:
			if (step_count < (0.25 * step_speed)) {
				draw_frame = LEFT_STANDING;
			}
			else if (step_count < (0.50 * step_speed)) {
				if (status & STEP_SWAP)
					draw_frame = LEFT_RSTEP1;
				else
					draw_frame = LEFT_LSTEP1;
			}
			else if (step_count < (0.75 * step_speed)) {
				if (status & STEP_SWAP)
					draw_frame = LEFT_RSTEP2;
				else
					draw_frame = LEFT_LSTEP2;
			}
			else { // (step_count < step_speed) == true
				if (status & STEP_SWAP)
					draw_frame = LEFT_RSTEP3;
				else
					draw_frame = LEFT_LSTEP3;
			}
			break;
		case EAST:
		case EAST_NE:
		case EAST_SE:
			if (step_count < (0.25 * step_speed)) {
				draw_frame = RIGHT_STANDING;
			}
			else if (step_count < (0.50 * step_speed)) {
				if (status & STEP_SWAP)
					draw_frame = RIGHT_RSTEP1;
				else
					draw_frame = RIGHT_LSTEP1;
			}
			else if (step_count < (0.75 * step_speed)) {
				if (status & STEP_SWAP)
					draw_frame = RIGHT_RSTEP2;
				else
					draw_frame = RIGHT_LSTEP2;
			}
			else { // (step_count < step_speed) == true
				if (status & STEP_SWAP)
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
	x_pos = mf.c_pos + (static_cast<float>(col_pos) - static_cast<float>(mf.c_start));
	y_pos = mf.r_pos + (static_cast<float>(mf.r_start) - static_cast<float>(row_pos));

	// When we are in motion, we have to off-set the step positions
	if (status & IN_MOTION) {
		switch (status & FACE_MASK) {
			case EAST:
				x_pos -= (step_speed - step_count) / step_speed;
				break;
			case WEST:
				x_pos += (step_speed - step_count) / step_speed;
				break;
			case NORTH:
				y_pos -= (step_speed - step_count) / step_speed;
				break;
			case SOUTH:
				y_pos += (step_speed - step_count) / step_speed;
				break;
			case NORTH_NW:
			case WEST_NW:
				x_pos += (step_speed - step_count) / step_speed;
				y_pos -= (step_speed - step_count) / step_speed;
				break;
			case SOUTH_SW:
			case WEST_SW:
				x_pos += (step_speed - step_count) / step_speed;
				y_pos += (step_speed - step_count) / step_speed;
				break;
			case NORTH_NE:
			case EAST_NE:
				x_pos -= (step_speed - step_count) / step_speed;
				y_pos -= (step_speed - step_count) / step_speed;
				break;
			case SOUTH_SE:
			case EAST_SE:
				x_pos -= (step_speed - step_count) / step_speed;
				y_pos += (step_speed - step_count) / step_speed;
				break;
		}
	}


	draw_frame = FindFrame();
	VideoManager->Move(x_pos, y_pos);
	VideoManager->DrawImage((*frames)[draw_frame]);
}

// ****************************************************************************
// ********************* SpriteDialouge Class Functions ***********************
// ****************************************************************************

SpriteDialogue::SpriteDialogue() {
	if (MAP_DEBUG) cout << "MAP: SpriteDialogue constructor invoked" << endl;
	seen_all = true;
}


SpriteDialogue::~SpriteDialogue() {
	if (MAP_DEBUG) cout << "MAP: SpriteDialogue destructor invoked" << endl;
}

// Load a new set of dialogue
void SpriteDialogue::LoadDialogue(vector<vector<string> > txt, vector<vector<uint32> > sp) {
	dialogue = txt;
	speaker = sp;
	for (uint32 i = 0; i < dialogue.size(); i++) {
		seen.push_back(false);
	}
	seen_all = false;
	next_read = 0;
}

// Add a new line of dialogue
void SpriteDialogue::AddDialogue(vector<string> txt, vector<uint32> sp) {
	dialogue.push_back(txt);
	speaker.push_back(sp);
	seen.push_back(false);

	if (seen_all) { // Set the next read to point to the new dialogue
		next_read = dialogue.size() - 1;
	}

	seen_all = false;
}


// Add a new line of dialogue, for one-part speeches
void SpriteDialogue::AddDialogue(string txt, uint32 sp) {
	vector<string> new_txt(1, txt);
	vector<uint32> new_sp(1, sp);
	dialogue.push_back(new_txt);
	speaker.push_back(new_sp);

	if (seen_all) {
		next_read = dialogue.size() - 1;
	}

	seen_all = false;
}

} // namespace hoa_map
