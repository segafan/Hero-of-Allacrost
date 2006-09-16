///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    map.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \brief   Source file for map mode interface.
 *****************************************************************************/

#include "utils.h"
#include "map.h"
#include "map_objects.h"
#include "map_dialogue.h"
#include "audio.h"
#include "video.h"
#include "gui.h"
#include "global.h"
#include "data.h"
#include "input.h"
#include "system.h"
#include "battle.h"
#include "menu.h"

using namespace std;
using namespace hoa_map::private_map;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_input;
using namespace hoa_global;
using namespace hoa_data;
using namespace hoa_battle;
using namespace hoa_menu;

namespace hoa_map {

bool MAP_DEBUG = false;

// ****************************************************************************
// ************************** MapMode Class Functions *************************
// ****************************************************************************
// ***************************** GENERAL FUNCTIONS ****************************
// ****************************************************************************

MapMode::MapMode() : _fade_to_battle_mode(false) {
	if (MAP_DEBUG) cout << "MAP: MapMode constructor invoked" << endl;

	mode_type = MODE_MANAGER_MAP_MODE;
	_map_state = EXPLORE;

	_map_camera = new MapSprite();
	_map_camera->SetObjectType(MAP_CAMERA);
	_map_camera->SetRowPosition(20);
	_map_camera->SetColPosition(20);
	_map_camera->SetStatus(0);
	_map_camera->SetStepSpeed(NORMAL_SPEED);

	_ground_objects.push_back(_map_camera);
	// Loads all the map data
	LoadMap();

	SoundDescriptor battle1;
	battle1.LoadSound("snd/battle_encounter_01.ogg");
	SoundDescriptor battle2;
	battle2.LoadSound("snd/battle_encounter_02.ogg");
	SoundDescriptor battle3;
	battle3.LoadSound("snd/battle_encounter_03.ogg");
	_battle_sounds.push_back(battle1);
	_battle_sounds.push_back(battle2);
	_battle_sounds.push_back(battle3);
}



MapMode::~MapMode() {
	if (MAP_DEBUG) cout << "MAP: MapMode destructor invoked" << endl;

	for (uint32 i = 0; i < _map_music.size(); i++) {
		_map_music[i].FreeMusic();
	}

	// Delete all of the tile images
	for (uint32 i = 0; i < _tile_images.size(); i++) {
		VideoManager->DeleteImage(*(_tile_images[i]));
	}

	// Delete all of the objects
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		delete(_ground_objects[i]);
	}

	// Free up the dialogue window
	VideoManager->DeleteImage(_dialogue_box);
	_dialogue_window.Destroy();
}


// Resets appropriate class members.
void MapMode::Reset() {
	// Reset active video engine properties
	VideoManager->SetCoordSys(0.0f, SCREEN_COLS, SCREEN_ROWS, 0.0f);
	//VideoManager->SetCoordSys(-SCREEN_COLS/2.0f, SCREEN_COLS/2.0f, -SCREEN_ROWS/2.0f, SCREEN_ROWS/2.0f);
	if(!VideoManager->SetFont("default"))
    cerr << "MAP: ERROR > Couldn't set map font!" << endl;

	// Let all map objects know that this is the current map
	MapObject::current_map = this;

	if (_map_music[0].GetMusicState() != AUDIO_STATE_PLAYING) {
		_map_music[0].PlayMusic();
	}
}


// Loads the map from a Lua file.
void MapMode::LoadMap() {
	// TEMP: load point light
	_lighting_overlay.SetFilename("img/misc/torch_light_mask.png");
	_lighting_overlay.SetDimensions(8, 8);
	if (VideoManager->LoadImage(_lighting_overlay) == false) {
		exit(1);
	}


	_map_music.push_back(MusicDescriptor());
	_map_music[0].LoadMusic("mus/Seeking_New_Worlds.ogg");

	// *********** (1) Setup GUI items in 1024x768 coordinate system ************
	VideoManager->PushState();
	VideoManager->SetCoordSys(0, 1024, 768, 0);
	_dialogue_window.Create(1024.0f, 256.0f);
	_dialogue_window.SetPosition(0.0f, 512.0f);
// 	_dialogue_window.SetDisplayMode(VIDEO_MENU_EXPAND_FROM_CENTER);

	_dialogue_box.SetFilename("img/menus/dialogue_box.png");
	VideoManager->LoadImage(_dialogue_box);
	_dialogue_nameplate.SetFilename("img/menus/dialogue_nameplate.png");
	VideoManager->LoadImage(_dialogue_nameplate);

	_dialogue_textbox.SetDisplaySpeed(30);
	_dialogue_textbox.SetPosition(300.0f, 768.0f - 180.0f);
	_dialogue_textbox.SetDimensions(1024.0f - 300.0f - 60.0f, 180.0f - 70.0f);
	_dialogue_textbox.SetFont("map");
	_dialogue_textbox.SetDisplayMode(VIDEO_TEXT_FADECHAR);
	_dialogue_textbox.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	VideoManager->PopState();

	// ************* (2) Open data file and begin processing data ***************
	_map_data.OpenFile("dat/maps/desert_cave.lua");
	_random_encounters = _map_data.ReadBool("random_encounters");
	if (_random_encounters) {
		_encounter_rate = _map_data.ReadInt("encounter_rate");
		_steps_till_encounter = GaussianRandomValue(_encounter_rate, 5.0f);
	}
	else {
		// Set some decent default values, just in case a script turns random encounters on later
		_encounter_rate = 10;
		_steps_till_encounter = 10;
	}

	_row_count = _map_data.ReadInt("row_count");
	_col_count = _map_data.ReadInt("col_count");

	// ********************** (3) Load in tile filenames ************************
	vector<string> tile_filenames;
	_map_data.FillStringVector("tile_filenames", tile_filenames);
	for (uint32 i = 0; i < tile_filenames.size(); i++) {
		// Prepend the pathname and append the file extension for all the file names
		tile_filenames[i] = "img/tiles/" + tile_filenames[i] + ".png";
	}

	// ******************** (4) Setup tile image mappings ***********************
	vector<int32> tile_mappings;
	_map_data.OpenTable("tile_mappings");
	int32 mapping_count = tile_filenames.size(); // TMP
	//int32 mapping_count = _map_data.GetTableSize("tile_mappings");
	//cout << "mapping_count == " << mapping_count << endl;
	for (uint32 i = 0; i < static_cast<uint32>(mapping_count); i++) {
		_map_data.FillIntVector(i, tile_mappings);

		if (tile_mappings.size() == 1) { // Then add a new static image
			StillImage *static_tile = new StillImage();
			static_tile->SetDimensions(1.0f, 1.0f);
			static_tile->SetFilename(tile_filenames[tile_mappings[0]]);
			_tile_images.push_back(static_tile);
		}
		else { // Create a new dynamic image
			//_tile_images.push_back(&animate_tile);
			for (uint32 j = 0; j < tile_mappings.size(); j += 2) {
//				(dynamic_cast<AnimatedImage>(_tile_images.back())).AddFrame
//				 (tile_filenames[tile_mappings[j]], tile_mappings[j+1]);
				// NOTE: Find a cleaner way to do this later...
			}
		}
		tile_mappings.clear();
	}
	_map_data.CloseTable();

	// **************** (5) Load all tile images from memory ********************
	VideoManager->BeginImageLoadBatch();
	for (uint32 i = 0; i < _tile_images.size(); i++) {
		if (!_tile_images[i]->Load()) {
			cerr << "MAP ERROR: Failed to load tile image " << endl;
		}
	}
	VideoManager->EndImageLoadBatch();

	// ******************** (6) Create the 2D tile map *************************
	MapTile tmp_tile;
	for (uint32 r = 0; r < _row_count; r++) {
		_tile_layers.push_back(vector <MapTile>(_col_count));
	}

	vector<int32> properties;

	_map_data.OpenTable("lower_layer");
	for (uint32 r = 0; r < _row_count; r++) {
		_map_data.FillIntVector(r, properties);

		for (uint32 c = 0; c < _col_count; c++) {
			_tile_layers[r][c].lower_layer = static_cast<int16>(properties[c]);
		}

		properties.clear();
	}
	_map_data.CloseTable();

	_map_data.OpenTable("middle_layer");
	for (uint32 r = 0; r < _row_count; r++) {
		_map_data.FillIntVector(r, properties);

		for (uint32 c = 0; c < _col_count; c++) {
			_tile_layers[r][c].middle_layer = static_cast<int16>(properties[c]);
		}

		properties.clear();
	}
	_map_data.CloseTable();

	_map_data.OpenTable("upper_layer");
	for (uint32 r = 0; r < _row_count; r++) {
		_map_data.FillIntVector(r, properties);

		for (uint32 c = 0; c < _col_count; c++) {
			_tile_layers[r][c].upper_layer = static_cast<int16>(properties[c]);
		}

		properties.clear();
	}
	_map_data.CloseTable();

	_map_data.OpenTable("tile_walkable");
	for (uint32 r = 0; r < _row_count; r++) {
		_map_data.FillIntVector(r, properties);

		for (uint32 c = 0; c < _col_count; c++) {
			_tile_layers[r][c].walkable = static_cast<uint8>(properties[c]);
		}

		properties.clear();
	}
	_map_data.CloseTable();

	// The occupied member of tiles are not set until we place map objects

	_map_data.CloseFile();

	if (_map_data.GetError() != DATA_NO_ERRORS) {
		cout << "MAP ERROR: some error occured during reading of map file" << endl;
	}

	MapSprite *sp;
	SpriteDialogue *sd;
	ActionPathMove *sa;
	ActionFrameDisplay *sf;

	// Load player sprite and rest of map objects
	sp = new MapSprite();
	sp->SetName(MakeUnicodeString("Claudius"));
	sp->SetID(0);
	sp->SetObjectType(PLAYER_SPRITE);
        sp->SetRowPosition(12);
	sp->SetColPosition(12);
	sp->SetStepSpeed(NORMAL_SPEED);
	sp->SetStatus(UPDATEABLE | VISIBLE | ALWAYS_IN_CONTEXT);
	sp->SetFilename("img/sprites/map/claudius");
	sp->SetPortrait("img/portraits/map/claudius.png");
	sp->SetDirection(SOUTH);
	sp->LoadFrames();
	_tile_layers[sp->GetColPosition()][sp->GetRowPosition()].occupied = 1;
	_ground_objects.push_back(sp);
	_sprites[sp->sprite_id] = sp;
	_focused_object = sp;

	sp = new MapSprite();
	sp->SetName(MakeUnicodeString("Laila"));
	sp->SetID(1);
	sp->SetObjectType(NPC_SPRITE);
	sp->SetRowPosition(4);
	sp->SetColPosition(4);
	sp->SetStepSpeed(NORMAL_SPEED);
	sp->SetStatus(UPDATEABLE | VISIBLE | ALWAYS_IN_CONTEXT);
	sp->SetFilename("img/sprites/map/laila");
	sp->SetPortrait("img/portraits/map/laila.png");
	sp->SetDirection(SOUTH);
	sp->LoadFrames();
	_tile_layers[sp->GetColPosition()][sp->GetRowPosition()].occupied = 1;

	sd = new SpriteDialogue();
	sd->text.push_back(MakeUnicodeString("It's really dark in here isn't it? I wonder how much longer our torches will last us..."));
	sd->speakers.push_back(1); // NPC speaks
	sp->dialogues.push_back(sd);

	sd = new SpriteDialogue();
	sd->text.push_back(MakeUnicodeString("If only we had more art, maybe the designers would have put in an exit in this cave!"));
	sd->speakers.push_back(1); // NPC speaks
	sd->text.push_back(MakeUnicodeString("Well, they're really under staffed in the art department. We really can't blame them too much."));
	sd->speakers.push_back(0); // Player speaks
	sp->dialogues.push_back(sd);

	sd = new SpriteDialogue();
	sd->text.push_back(MakeUnicodeString("Did you know that you can toggle off random encounters by pressing the swap key (default: a)?"));
	sd->speakers.push_back(1); // NPC speaks
	sp->dialogues.push_back(sd);

	sa = new ActionPathMove();
	sa->destination.row = 4;
	sa->destination.col = 16;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	sa = new ActionPathMove();
	sa->destination.row = 12;
	sa->destination.col = 16;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	sa = new ActionPathMove();
	sa->destination.row = 8;
	sa->destination.col = 4;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	sa = new ActionPathMove();
	sa->destination.row = 4;
	sa->destination.col = 4;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	_ground_objects.push_back(sp);
	_sprites[sp->sprite_id] = sp;

	sp = new MapSprite();
	sp->SetName(MakeUnicodeString("Marcus"));
	sp->SetID(2);
	sp->SetObjectType(NPC_SPRITE);
	sp->SetRowPosition(18);
	sp->SetColPosition(21);
	sp->SetStepSpeed(SLOW_SPEED);
	sp->SetStatus(UPDATEABLE | VISIBLE | ALWAYS_IN_CONTEXT);
	sp->SetFilename("img/sprites/map/marcus");
	sp->SetPortrait("img/portraits/map/marcus.png");
	sp->SetDirection(WEST);
	sp->LoadFrames();
	_tile_layers[sp->GetColPosition()][sp->GetRowPosition()].occupied = 1;

	sd = new SpriteDialogue();
	sd->text.push_back(MakeUnicodeString("My moustache tickles me."));
	sd->speakers.push_back(2); // NPC speaks
	sd->text.push_back(MakeUnicodeString("Why don't you shave it off then? Or at least trim it..."));
	sd->speakers.push_back(0); // Player speaks
	sd->text.push_back(MakeUnicodeString("Because moustaches are fashionable these days. I have to keep up with the times!"));
	sd->speakers.push_back(2); // NPC speaks
	sd->text.push_back(MakeUnicodeString("....."));
	sd->speakers.push_back(0); // NPC speaks
	sd->text.push_back(MakeUnicodeString("Claudius, I am your father!"));
	sd->speakers.push_back(2); // NPC speaks
	sd->text.push_back(MakeUnicodeString("I...know dad. Why are you wearing that black mask and breathing heavily when you say that?"));
	sd->speakers.push_back(0); // Player speaks
	sp->dialogues.push_back(sd);

	sa = new ActionPathMove();
	sa->destination.row = 25;
	sa->destination.col = 11;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	sa = new ActionPathMove();
	sa->destination.row = 12;
	sa->destination.col = 9;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	sa = new ActionPathMove();
	sa->destination.row = 8;
	sa->destination.col = 30;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	sa = new ActionPathMove();
	sa->destination.row = 26;
	sa->destination.col = 27;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	sf = new ActionFrameDisplay();
	sf->display_time = 2000;
	sf->remaining_time = 2000;
	sf->frame_index = EAST;
	sf->sprite = sp;
	sp->actions.push_back(sf);

	_ground_objects.push_back(sp);
	_sprites[sp->sprite_id] = sp;

	sp = new MapSprite();
	sp->SetName(MakeUnicodeString("Vanica"));
	sp->SetID(3);
	sp->SetObjectType(NPC_SPRITE);
	sp->SetRowPosition(24);
	sp->SetColPosition(6);
	sp->SetStepSpeed(FAST_SPEED);
	sp->SetStatus(UPDATEABLE | VISIBLE | ALWAYS_IN_CONTEXT);
	sp->SetFilename("img/sprites/map/vanica");
	sp->SetPortrait("img/portraits/map/vanica.png");
	sp->SetDirection(EAST);
	sp->LoadFrames();
	_tile_layers[sp->GetColPosition()][sp->GetRowPosition()].occupied = 1;

	sd = new SpriteDialogue();
	sd->text.push_back(MakeUnicodeString("I hope they put me in a star role in the game. I may not be at the peak of my youth, but I've been studying taichi lately you know!"));
	sd->speakers.push_back(3); // NPC speaks
	sp->dialogues.push_back(sd);

	sa = new ActionPathMove();
	sa->destination.row = 8;
	sa->destination.col = 5;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	sa = new ActionPathMove();
	sa->destination.row = 7;
	sa->destination.col = 13;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	sf = new ActionFrameDisplay();
	sf->display_time = 2500;
	sf->remaining_time = 2500;
	sf->frame_index = NORTH;
	sf->sprite = sp;
	sp->actions.push_back(sf);

	sa = new ActionPathMove();
	sa->destination.row = 24;
	sa->destination.col = 6;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	_ground_objects.push_back(sp);
	_sprites[sp->sprite_id] = sp;

	sp = new MapSprite();
	sp->SetName(MakeUnicodeString("Woman in Rags"));
	sp->SetID(4);
	sp->SetObjectType(NPC_SPRITE);
	sp->SetRowPosition(32);
	sp->SetColPosition(36);
	sp->SetStepSpeed(NORMAL_SPEED);
	sp->SetStatus(UPDATEABLE | VISIBLE | ALWAYS_IN_CONTEXT);
	sp->SetFilename("img/sprites/map/rags_woman");
	sp->SetDirection(NORTH);
	sp->LoadFrames();
	_tile_layers[sp->GetColPosition()][sp->GetRowPosition()].occupied = 1;

	sd = new SpriteDialogue();
	sd->text.push_back(MakeUnicodeString("Is there no exit out of this stinking..... hey, why don't I have a portrait?!"));
	sd->speakers.push_back(4); // NPC speaks
	sd->text.push_back(MakeUnicodeString("Probably because you're just a normal non playable character with no special role. I mean look at you, you're dressed in rags."));
	sd->speakers.push_back(0); // Player speaks
	sd->text.push_back(MakeUnicodeString("They can't do this to me! Just wait until the director hears from my agent! I've been nominated for six academy awards, how dare they disgrace me like this!"));
	sd->speakers.push_back(4); // NPC speaks
	sd->text.push_back(MakeUnicodeString("..... (I wonder who she thinks she is)"));
	sd->speakers.push_back(0); // NPC speaks
	sp->dialogues.push_back(sd);

	sa = new ActionPathMove();
	sa->destination.row = 32;
	sa->destination.col = 26;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	sa = new ActionPathMove();
	sa->destination.row = 32;
	sa->destination.col = 36;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	sa = new ActionPathMove();
	sa->destination.row = 32;
	sa->destination.col = 26;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	sa = new ActionPathMove();
	sa->destination.row = 32;
	sa->destination.col = 36;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	sa = new ActionPathMove();
	sa->destination.row = 32;
	sa->destination.col = 26;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	sa = new ActionPathMove();
	sa->destination.row = 32;
	sa->destination.col = 36;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	sa = new ActionPathMove();
	sa->destination.row = 32;
	sa->destination.col = 31;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	// Wait here, looking west
	sf = new ActionFrameDisplay();
	sf->display_time = 1240;
	sf->remaining_time = 1240;
	sf->frame_index = WEST;
	sf->sprite = sp;
	sp->actions.push_back(sf);

	sa = new ActionPathMove();
	sa->destination.row = 38;
	sa->destination.col = 33;
	sa->sprite = sp;
	sp->actions.push_back(sa);

	// Wait again, looking south
	sf = new ActionFrameDisplay();
	sf->display_time = 3200;
	sf->remaining_time = 3200;
	sf->frame_index = SOUTH;
	sf->sprite = sp;
	sp->actions.push_back(sf);

	_ground_objects.push_back(sp);
	_sprites[sp->sprite_id] = sp;

	speed_double = false;
	_focused_object->step_speed /= 2;
} // _LoadMap()



// Returns true if an object can be moved to the tile.
bool MapMode::_TileMoveable(const private_map::TileCheck& tcheck) {
	// Check that the row and col indeces are valid and not outside the map
	// (By the way, the top row is never walkable on a map so we don't check that)
	if (tcheck.row < 1 || tcheck.col < 0 || tcheck.row >= _row_count || tcheck.col >= _col_count) {
		return false;
	}

	// If the focused object is the virtual sprite, there's nothing left to check
	if (_focused_object == _map_camera) {
		return true;
	}

	// Check that the tile is walkable
	if (!_tile_layers[tcheck.row][tcheck.col].walkable) {
		return false;
	}

	// Don't allow diagonal movement if any nearby component x, y tiles are unwalkable
	switch (tcheck.direction) {
		case NORTH:
		case SOUTH:
		case WEST:
		case EAST:
			break;
		case NORTHWEST:
		case NW_NORTH:
		case NW_WEST:
			if (!(_tile_layers[tcheck.row][tcheck.col + 1].walkable && _tile_layers[tcheck.row + 1][tcheck.col].walkable)) {
				return false;
			}
			break;
		case SOUTHWEST:
		case SW_SOUTH:
		case SW_WEST:
			if (!(_tile_layers[tcheck.row][tcheck.col + 1].walkable && _tile_layers[tcheck.row - 1][tcheck.col].walkable)) {
				return false;
			}
			break;
		case NORTHEAST:
		case NE_NORTH:
		case NE_EAST:
			if (!(_tile_layers[tcheck.row][tcheck.col - 1].walkable && _tile_layers[tcheck.row + 1][tcheck.col].walkable)) {
				return false;
			}
			break;
		case SOUTHEAST:
		case SE_SOUTH:
		case SE_EAST:
			if (!(_tile_layers[tcheck.row][tcheck.col - 1].walkable && _tile_layers[tcheck.row - 1][tcheck.col].walkable)) {
				return false;
			}
			break;
		default:
			if (MAP_DEBUG) cerr << "MAP: WARNING: Called MapMode::_TileMoveable() with an invalid direction" << endl;
			return false;
	}

	// Check that no other objects occupy this tile
	if (_tile_layers[tcheck.row][tcheck.col].occupied) {
		return false;
	}

	return true;
}



// Searches the list of map objects to find the object occupying a tile.
MapObject* MapMode::_FindTileOccupant(const TileCheck& tcheck) {
	// TODO: Use a more sophisticated search algorithm here
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		if (_ground_objects[i]->row_position == tcheck.row &&
				_ground_objects[i]->col_position == tcheck.col) {
			return _ground_objects[i];
		}
	}
	return NULL;
} // MapMode::_FindTileOccupant()



bool MapMode::_IsNodeInList(const TileCheck& node, list<TileNode> &node_list) {
	for (list<TileNode>::iterator i = node_list.begin(); i != node_list.end(); i++) {
		if (node.row == i->row && node.col == i->col) {
			return true;
		}
	}
	return false;
}


TileNode* MapMode::_FindNodeInList(const TileCheck& node, list<TileNode> &node_list) {
	for (list<TileNode>::iterator i = node_list.begin(); i != node_list.end(); i++) {
		if (node.row == i->row && node.col == i->col) {
			return &(*i);
		}
	}
	return NULL;
}



// Finds a path for a sprite to take, using the A* algorithm.
void MapMode::_FindPath(TileNode destination, vector<TileNode> &path,
		const MapSprite* sprite) {
	// The tiles that we are considering for the next move
	list<TileNode> open_list;
	// The tiles which have already been visited once.
	list<TileNode> closed_list;
	// Used to examine if a path is valid
	TileCheck tcheck;
	// A new node to construct and add to the path
	TileNode new_node;
	// Used to temporarily hold a pointer to a node in a list
	TileNode *list_node = NULL;

	// Check if the destination is occupied; if it is, keep changing the
	// destination tile to the next closest tile to the source, until we find
	// one that is walkable and not occupied.
	while (!_tile_layers[destination.row][destination.col].walkable || 
		   (_tile_layers[destination.row][destination.col].occupied &&
			(sprite->row_position != destination.row ||
			 sprite->col_position != destination.col))) {
		if (destination.row > path[0].row)
			destination.row--;
		else if (destination.row < path[0].row)
			destination.row++;
		if (destination.col > path[0].col)
			destination.col--;
		else if (destination.col < path[0].col)
			destination.col++;
	}

	// Check that the source is not equal to the destination
	if (path[0].row == destination.row && path[0].col == destination.col) {
		path.clear();
		return;
	}

	// Push the node that the sprite is currently standing on to the closed list
	closed_list.push_back(path[0]);

	// Find a path until the current node is equal to the destination
	while (closed_list.back() != destination) {
		// Check bottom left tile
		tcheck.direction = SOUTHWEST;
		tcheck.row = closed_list.back().row + 1;
		tcheck.col = closed_list.back().col - 1;
		if (_TileMoveable(tcheck) && (!_IsNodeInList(tcheck, closed_list))) {
			list_node = _FindNodeInList(tcheck, open_list);
			if ((list_node != NULL) && (list_node->g_score > closed_list.back().g_score + 14)) { // Check the node for a better g score
				list_node->g_score = closed_list.back().g_score + 14;
				list_node->parent = &(closed_list.back());
			}
			else if (list_node == NULL) { // Add new node to open list
				new_node.row = tcheck.row;
				new_node.col = tcheck.col;
				new_node.parent = &(closed_list.back());
				new_node.g_score = new_node.parent->g_score + 14;
				new_node.h_score = 10 * (abs(destination.row - new_node.row) + abs(destination.col - new_node.col));
				new_node.f_score = new_node.g_score + new_node.h_score;
				open_list.push_back(new_node);
			}
		}

		// Check left tile
		tcheck.direction = WEST;
		tcheck.row = closed_list.back().row;
		tcheck.col = closed_list.back().col - 1;
		if (_TileMoveable(tcheck) && (!_IsNodeInList(tcheck, closed_list))) {
			list_node = _FindNodeInList(tcheck, open_list);
			if ((list_node != NULL) && (list_node->g_score > closed_list.back().g_score + 10)) { // Check the node for a better g score
				list_node->g_score = closed_list.back().g_score + 10;
				list_node->parent = &(closed_list.back());
			}
			else if (list_node == NULL) { // Add new node to open list
				new_node.row = tcheck.row;
				new_node.col = tcheck.col;
				new_node.parent = &(closed_list.back());
				new_node.g_score = new_node.parent->g_score + 10;
				new_node.h_score = 10 * (abs(destination.row - new_node.row) + abs(destination.col - new_node.col));
				new_node.f_score = new_node.g_score + new_node.h_score;
				open_list.push_back(new_node);
			}
		}

		// Check top left tile
		tcheck.direction = NORTHWEST;
		tcheck.row = closed_list.back().row - 1;
		tcheck.col = closed_list.back().col - 1;
		if (_TileMoveable(tcheck) && (!_IsNodeInList(tcheck, closed_list))) {
			list_node = _FindNodeInList(tcheck, open_list);
			if ((list_node != NULL) && (list_node->g_score > closed_list.back().g_score + 14)) { // Check the node for a better g score
				list_node->g_score = closed_list.back().g_score + 14;
				list_node->parent = &(closed_list.back());
			}
			else if (list_node == NULL) { // Add new node to open list
				new_node.row = tcheck.row;
				new_node.col = tcheck.col;
				new_node.parent = &(closed_list.back());
				new_node.g_score = new_node.parent->g_score + 14;
				new_node.h_score = 10 * (abs(destination.row - new_node.row) + abs(destination.col - new_node.col));
				new_node.f_score = new_node.g_score + new_node.h_score;
				open_list.push_back(new_node);
			}
		}

		// Check top tile
		tcheck.direction = NORTH;
		tcheck.row = closed_list.back().row - 1;
		tcheck.col = closed_list.back().col;
		if (_TileMoveable(tcheck) && (!_IsNodeInList(tcheck, closed_list))) {
			list_node = _FindNodeInList(tcheck, open_list);
			if ((list_node != NULL) && (list_node->g_score > closed_list.back().g_score + 10)) { // Check the node for a better g score
				list_node->g_score = closed_list.back().g_score + 10;
				list_node->parent = &(closed_list.back());
			}
			else if (list_node == NULL) { // Add new node to open list
				new_node.row = tcheck.row;
				new_node.col = tcheck.col;
				new_node.parent = &(closed_list.back());
				new_node.g_score = new_node.parent->g_score + 10;
				new_node.h_score = 10 * (abs(destination.row - new_node.row) + abs(destination.col - new_node.col));
				new_node.f_score = new_node.g_score + new_node.h_score;
				open_list.push_back(new_node);
			}
		}

		// Check top right tile
		tcheck.direction = NORTHEAST;
		tcheck.row = closed_list.back().row - 1;
		tcheck.col = closed_list.back().col + 1;
		if (_TileMoveable(tcheck) && (!_IsNodeInList(tcheck, closed_list))) {
			list_node = _FindNodeInList(tcheck, open_list);
			if ((list_node != NULL) && (list_node->g_score > closed_list.back().g_score + 14)) { // Check the node for a better g score
				list_node->g_score = closed_list.back().g_score + 14;
				list_node->parent = &(closed_list.back());
			}
			else if (list_node == NULL) { // Add new node to open list
				new_node.row = tcheck.row;
				new_node.col = tcheck.col;
				new_node.parent = &(closed_list.back());
				new_node.g_score = new_node.parent->g_score + 14;
				new_node.h_score = 10 * (abs(destination.row - new_node.row) + abs(destination.col - new_node.col));
				new_node.f_score = new_node.g_score + new_node.h_score;
				open_list.push_back(new_node);
			}
		}

		// Check right tile
		tcheck.direction = EAST;
		tcheck.row = closed_list.back().row;
		tcheck.col = closed_list.back().col + 1;
		if (_TileMoveable(tcheck) && (!_IsNodeInList(tcheck, closed_list))) {
			list_node = _FindNodeInList(tcheck, open_list);
			if ((list_node != NULL) && (list_node->g_score > closed_list.back().g_score + 10)) { // Check the node for a better g score
				list_node->g_score = closed_list.back().g_score + 10;
				list_node->parent = &(closed_list.back());
			}
			else if (list_node == NULL) { // Add new node to open list
				new_node.row = tcheck.row;
				new_node.col = tcheck.col;
				new_node.parent = &(closed_list.back());
				new_node.g_score = new_node.parent->g_score + 10;
				new_node.h_score = 10 * (abs(destination.row - new_node.row) + abs(destination.col - new_node.col));
				new_node.f_score = new_node.g_score + new_node.h_score;
				open_list.push_back(new_node);
			}
		}

		// Check bottom right tile
		tcheck.direction = SOUTHEAST;
		tcheck.row = closed_list.back().row + 1;
		tcheck.col = closed_list.back().col + 1;
		if (_TileMoveable(tcheck) && (!_IsNodeInList(tcheck, closed_list))) {
			list_node = _FindNodeInList(tcheck, open_list);
			if ((list_node != NULL) && (list_node->g_score > closed_list.back().g_score + 14)) { // Check the node for a better g score
				list_node->g_score = closed_list.back().g_score + 14;
				list_node->parent = &(closed_list.back());
			}
			else if (list_node == NULL) { // Add new node to open list
				new_node.row = tcheck.row;
				new_node.col = tcheck.col;
				new_node.parent = &(closed_list.back());
				new_node.g_score = new_node.parent->g_score + 14;
				new_node.h_score = 10 * (abs(destination.row - new_node.row) + abs(destination.col - new_node.col));
				new_node.f_score = new_node.g_score + new_node.h_score;
				open_list.push_back(new_node);
			}
		}

		// Check bottom tile
		tcheck.direction = SOUTH;
		tcheck.row = closed_list.back().row + 1;
		tcheck.col = closed_list.back().col;
		if (_TileMoveable(tcheck) && (!_IsNodeInList(tcheck, closed_list))) {
			list_node = _FindNodeInList(tcheck, open_list);
			if ((list_node != NULL) && (list_node->g_score > closed_list.back().g_score + 10)) { // Check the node for a better g score
				list_node->g_score = closed_list.back().g_score + 10;
				list_node->parent = &(closed_list.back());
			}
			else if (list_node == NULL) { // Add new node to open list
				new_node.row = tcheck.row;
				new_node.col = tcheck.col;
				new_node.parent = &(closed_list.back());
				new_node.g_score = new_node.parent->g_score + 10;
				new_node.h_score = 10 * (abs(destination.row - new_node.row) + abs(destination.col - new_node.col));
				new_node.f_score = new_node.g_score + new_node.h_score;
				open_list.push_back(new_node);
			}
		}

		// If there are no nodes on the open list, a path couldn't be found
		if (open_list.empty()) {
			cerr << "MAP ERROR: Couldn't find a path between two nodes" << endl;
			return;
		}

		// Find the node on the open list with the lowest F score, and move it to the closed list
		list<TileNode>::iterator best_move = open_list.begin();
		for (list<TileNode>::iterator i = open_list.begin(); i != open_list.end(); i++) {
			if (i->f_score < best_move->f_score) {
				best_move = i;
			}
		}
		closed_list.push_back(*best_move);
		open_list.erase(best_move);
	} // while (destination != end of closed list)

	// Save the new path by tracing it backwards
	path.clear();
	open_list.clear();
	list_node = &(closed_list.back());

	// Reverse sort the closed_list into the open list
	// First find the number of elements
	while (list_node->parent != NULL) {
		open_list.push_back(*list_node);
		list_node = list_node->parent;
	}
	// Now put the open list elements into the path vector
	while (!open_list.empty()) {
		path.push_back(open_list.back());
		open_list.pop_back();
	}
}


// Processes user update and camera movement. Only called when the map is focused on the virtual sprite
void MapMode::_UpdateVirtualSprite() {

} // MapMode::_UpdateVirtualSprite()

// ****************************************************************************
// **************************** UPDATE FUNCTIONS ******************************
// ****************************************************************************

// Updates the game state when in map mode. Called from the main game loop.
void MapMode::Update() {
	_time_elapsed = SystemManager->GetUpdateTime();
        
        
	// ***************** (1) Process user input **************
	switch (_map_state) {
		case EXPLORE:
			_UpdateExplore();
			break;
		case DIALOGUE:
			_UpdateDialogue();
			break;
		default:
			_UpdateExplore();
			break;
	}

	// ***************** (2) Update all objects on the map **************
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		if ((_ground_objects[i])->status & UPDATEABLE) {
			_ground_objects[i]->Update();
		}
	}

	// ************ (3) Sort the objects so they are in the correct draw order ********
	// Note: this sorting algorithm will be optimized at a later time
	for (uint32 i = 1; i < _ground_objects.size(); i++) {
		MapObject *tmp = _ground_objects[i];
		int32 j = static_cast<int32>(i) - 1;
		while (j >= 0 && (_ground_objects[j])->row_position > tmp->row_position) {
			_ground_objects[j+1] = _ground_objects[j];
			j--;
		}
		_ground_objects[j+1] = tmp;
	}


}



// Updates the game status when MapMode is in the 'explore' state
void MapMode::_UpdateExplore() {
	bool user_move = false;
	uint32 move_direction;

	if (InputManager->SwapPress()) {
		_random_encounters = !_random_encounters;
	}
	// Toggle run versus walk
	if (InputManager->CancelPress()) {
		if (speed_double) {
			_focused_object->step_speed /= 2;
			speed_double = false;
		}
		else {
			_focused_object->step_speed *= 2;
			speed_double = true;
		}
	}

	// Check for menu press events
	if (InputManager->MenuPress()) {
		MenuMode *MM = new MenuMode();
		ModeManager->Push(MM);
		return;
	}

	//  Check if the focused object is moving. If so, only process swap events from the user
	if (_focused_object->status & IN_MOTION) {
		return;
	}

	// Process confirm events from the user
	if (InputManager->ConfirmPress()) {
		// Check for a sprite present within the space of one tile
		int16 check_row;
		int16 check_col;
		if (_focused_object->direction & (WEST | NW_WEST | SW_WEST)) {
			check_row = _focused_object->row_position;
			check_col = _focused_object->col_position - 1;
		}
		else if (_focused_object->direction & (EAST | NE_EAST | SE_EAST)) {
			check_row = _focused_object->row_position;
			check_col = _focused_object->col_position + 1;
		}
		else if (_focused_object->direction & (NORTH | NW_NORTH | NE_NORTH)) {
			check_row = _focused_object->row_position - 1;
			check_col = _focused_object->col_position;
		}
		else { // then => (_focused_object->_direction & (SOUTH | SW_SOUTH | SE_SOUTH)) == true
			check_row = _focused_object->row_position + 1;
			check_col = _focused_object->col_position;
		}

		if (check_row < 0 || check_col < 0 || check_row > _row_count || check_col >_col_count) {
			return;
		}

		MapSprite *sprite = NULL;
		if (_tile_layers[check_row][check_col].occupied) {
			for (uint32 i = 0; i < _ground_objects.size(); i++) {
				if (_ground_objects[i]->row_position == check_row && _ground_objects[i]->col_position == check_col) {
					sprite = dynamic_cast<MapSprite*>(_ground_objects[i]);
					break;
				}
			}
			if (MAP_DEBUG && sprite == NULL) {
				cerr << "MAP ERROR: could not find sprite that should be occupying tile" << endl;
			}
		}

		if (sprite == NULL) {
			float first_row = static_cast<float>(check_row) +  _focused_object->row_offset;
			float first_col = static_cast<float>(check_col) +  _focused_object->col_offset;
			for (uint32 i = 0; i < _ground_objects.size(); i++) {
				MapSprite *second = dynamic_cast<MapSprite*>(_ground_objects[i]);
				float second_row = static_cast<float>(second->row_position) + second->row_offset;
				float second_col = static_cast<float>(second->col_position) + second->col_offset;

				if (second_row <= first_row + 1.0f  && second_row >= first_row - 1.0f &&
					second_col <= first_col + 1.0f  && second_col >= first_col - 1.0f) {
					sprite = second;
					break;
				}
			}
		}

		if (sprite != NULL) {
			if (sprite->dialogues.size() != 0) {
				_map_state = DIALOGUE;
				sprite->SaveState();
				if (_focused_object->direction & (SOUTH | SW_SOUTH | SE_SOUTH)) {
					sprite->frame = UP_STANDING;
				}
				else if (_focused_object->direction & (NORTH | NW_NORTH | NE_NORTH)) {
					sprite->frame = DOWN_STANDING;
				}
				else if (_focused_object->direction & (EAST | NE_EAST | SE_EAST)) {
					sprite->frame = LEFT_STANDING;
				}
				else {
					sprite->frame = RIGHT_STANDING;
				}
				_dialogue_window.Show();
				_current_dialogue = dynamic_cast<MapDialogue*>(sprite->dialogues[sprite->next_conversation]);
				_dialogue_textbox.SetDisplayText(_current_dialogue->text[_current_dialogue->current_line]);
// 				if (sprite->dialogues[sprite->next_conversation]->speaking_action == NULL) {
// 					cout << 3.5 << endl;
// 					sprite->SaveState();
// 				}
			}
		}

		return;
	} // if (InputManager->ConfirmPress())

	// Handle movement input from user
	// Handle west, northwest, and southwest movement
	if (InputManager->LeftState()) {
		user_move = true;
		if (InputManager->UpState()) {
			move_direction = NW_NORTH;
		}
		else if (InputManager->DownState()) {
			move_direction = SW_SOUTH;
		}
		else {
			move_direction = WEST;
		}
	}
	// Handle east, northeast, and southeast movement
	else if (InputManager->RightState()) {
		user_move = true;
		if (InputManager->UpState())
			move_direction = NE_NORTH;
		else if (InputManager->DownState())
			move_direction = SE_SOUTH;
		else
			move_direction = EAST;
	}
	// Handle north movement
	else if (InputManager->UpState()) {
		user_move = true;
		move_direction = NORTH;
	}
	// Handle south movement
	else if (InputManager->DownState()) {
		user_move = true;
		move_direction = SOUTH;
	}

	// Do the fade to battle mode
	if (_fade_to_battle_mode)
	{
		// Only start battle mode once the fade is done.
		if (!VideoManager->IsFading())
		{
			// clear fade instantly
			VideoManager->FadeScreen(Color::clear, 0.0f);
			_fade_to_battle_mode = false;
			BattleMode *BM = new BattleMode();
			ModeManager->Push(BM);
		}
		return;
	}

	if (user_move) {
		if (_random_encounters) {
			_steps_till_encounter--;
			if (_steps_till_encounter == 0) {
				VideoManager->FadeScreen(Color::black, 1.0f);
				// play battle sfx
				_map_music[0].StopMusic();
				_battle_sounds[RandomBoundedInteger(0, 2)].PlaySound();
				
				_fade_to_battle_mode = true;
				_steps_till_encounter = GaussianRandomValue(_encounter_rate, 2.5f);
			}
		}
		_focused_object->Move(move_direction);
		// The move may be successful, or it may not be.
	}
}



// Updates the game status when MapMode is in the 'dialogue' state
void MapMode::_UpdateDialogue() {
	_dialogue_window.Update(_time_elapsed);
	_dialogue_textbox.Update(_time_elapsed);

	if (InputManager->ConfirmPress()) {
		if (!_dialogue_textbox.IsFinished()) {
			_dialogue_textbox.ForceFinish();
		}
		else {
			bool not_finished = _current_dialogue->ReadNextLine();

			if (!not_finished) {
				_dialogue_window.Hide();
				_map_state = EXPLORE;
				// Restore the status of the map sprites
				for (uint32 i = 0; i < _current_dialogue->speakers.size(); i++) {
					_sprites[_current_dialogue->speakers[i]]->RestoreState();
				}
				_sprites[1]->UpdateConversationCounter();
				_current_dialogue = NULL;
			}
			else { // Otherwise, the dialogue is automatically updated to the next line
				_dialogue_textbox.SetDisplayText(_current_dialogue->text[_current_dialogue->current_line]);
			}
		}
	}
}




// ****************************************************************************
// **************************** DRAW FUNCTIONS ********************************
// ****************************************************************************


// Determines things like our starting tiles
void MapMode::_GetDrawInfo() {
	// ************* (1) Set the default drawing positions for the tiles ****************
	// Begin drawing from the top left corner
	_draw_info.c_pos = -0.5f;
	_draw_info.r_pos = 0.5f;

	// By default draw 32 + 1 columns and 24 + 1 rows
	_draw_info.c_draw = static_cast<uint8>(SCREEN_COLS) + 1;
	_draw_info.r_draw = static_cast<uint8>(SCREEN_ROWS) + 1;

	// The default starting tile row and column is relative to the focused sprite's current position.
	_draw_info.c_start = static_cast<int16>(_focused_object->col_position - (static_cast<int32>(SCREEN_COLS) / 2));
	_draw_info.r_start = static_cast<int16>(_focused_object->row_position - (static_cast<int32>(SCREEN_ROWS) / 2));

	// *** (2) Modify drawing positions if focused sprite is currently moving ***

	if (_focused_object->status & IN_MOTION) {
		float offset = _focused_object->step_count / _focused_object->step_speed;
		if (_focused_object->direction & (WEST | NW_NORTH | NW_WEST | SW_SOUTH | SW_WEST)) {
			if (offset < 0.5f) {
				_draw_info.c_pos += offset;
				_draw_info.c_start++;
			}
			else {
				_draw_info.c_pos -= 1.0f - offset;
			}
		}
		else if (_focused_object->direction & (EAST | NE_NORTH | NE_EAST | SE_SOUTH | SE_EAST)) {
			if (offset < 0.5f) {
				_draw_info.c_pos -= offset;
				_draw_info.c_start--;
			}
			else {
				_draw_info.c_pos += 1.0f - offset;
			}
		}

		if (_focused_object->direction & (NORTH | NW_WEST | NW_NORTH | NE_EAST | NE_NORTH)) {
			if (offset < 0.5f) {
				_draw_info.r_pos += offset;
				_draw_info.r_start++;
			}
			else {
				_draw_info.r_pos -= 1.0f - offset;
			}
		}
		else if (_focused_object->direction & (SOUTH | SW_WEST | SW_SOUTH | SE_EAST | SE_SOUTH)) {
			if (offset < 0.5f) {
				_draw_info.r_pos -= offset;
				_draw_info.r_start--;
			}
			else {
				_draw_info.r_pos += 1.0f - offset;
			}
		}
	}

	// *********************** (3) Check for special conditions **************************

	// Usually the map moves around the player, but when we encounter the edges of the map we
	// want the player to move around the map.

	// Exceeds the far-left side of the map
	if (_draw_info.c_start < 0) {
		_draw_info.c_start = 0;
		_draw_info.c_pos = 0.0f;
	}
	// Exceeds the far-right side of the map
	else if (_draw_info.c_start >= _col_count - static_cast<int32>(SCREEN_COLS)) {
		_draw_info.c_start = static_cast<int16>(_col_count - static_cast<int32>(SCREEN_COLS));
		_draw_info.c_pos = 0.0f;
	}
	// If our column position is exactly on the left edge of the screen, we draw one less column of tiles
	if (_draw_info.c_pos == 0.0f) {
		_draw_info.c_draw--;
	}

	// Exceeds the far-north side of the map
	if (_draw_info.r_start < 0) {
		_draw_info.r_start = 0;
		_draw_info.r_pos = 1.0f;
	}
	// Exceeds the far-south side of the map
	else if (_draw_info.r_start >= _row_count - static_cast<int32>(SCREEN_ROWS)) {
		_draw_info.r_start = static_cast<int16>(_row_count - static_cast<int32>(SCREEN_ROWS));
		_draw_info.r_pos = 1.0f;
	}
	// If the row position is exactly on the top of the screen, draw one less row of tiles
	if (_draw_info.r_pos == 1.0f) {
		_draw_info.r_draw--;
	}
        
} // MapMode::_GetDrawInfo()


// Public draw function called by the main game loop
void MapMode::Draw() {
	// Calculate all the information we need for drawing this map frame
	_GetDrawInfo();

	// TEMP: Darken the cave scene
	VideoManager->EnableSceneLighting(Color(0.75f, 0.75f, 0.75f, 1.0f)); // black, 75% transparent
	VideoManager->EnablePointLights();

	// ************** (1) Draw the lower tile layer *************
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_NO_BLEND, 0);
	VideoManager->Move(_draw_info.c_pos, _draw_info.r_pos);
	for (uint32 r = static_cast<uint32>(_draw_info.r_start);
	     r < static_cast<uint32>(_draw_info.r_start) + _draw_info.r_draw; r++) {
		for (uint32 c = static_cast<uint32>(_draw_info.c_start);
		     c < static_cast<uint32>(_draw_info.c_start) + _draw_info.c_draw; c++) {
			if (_tile_layers[r][c].lower_layer >= 0) { // Then a lower layer tile exists and we should draw it
				_tile_images[_tile_layers[r][c].lower_layer]->Draw();
			}
			VideoManager->MoveRelative(1.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(_draw_info.c_draw), 1.0f);
	}

	// ************** (2) Draw the middle tile layer *************
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->Move(_draw_info.c_pos, _draw_info.r_pos);
	for (uint32 r = static_cast<uint32>(_draw_info.r_start);
	     r < static_cast<uint32>(_draw_info.r_start) + _draw_info.r_draw; r++) {
		for (uint32 c = static_cast<uint32>(_draw_info.c_start);
		     c < static_cast<uint32>(_draw_info.c_start) + _draw_info.c_draw; c++) {
			if (_tile_layers[r][c].middle_layer >= 0) { // Then a middle layer tile exists and we should draw it
				_tile_images[_tile_layers[r][c].middle_layer]->Draw();
			}
			VideoManager->MoveRelative(1.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(_draw_info.c_draw), 1.0f);
	}

	// ************** (3) Draw the ground object layer (first pass) *************
        VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
                // Only draw objects that are visible
		if (_ground_objects[i]->status & VISIBLE) {
			(_ground_objects[i])->Draw();
			(_ground_objects[i])->DrawLight();
		}
	}

	// ************** (4) Draw the middle object layer *************
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	for (uint32 i = 0; i < _middle_objects.size(); i++) {
		// Only draw objects that are visible
		if (_middle_objects[i]->status & VISIBLE) {
			(_middle_objects[i])->Draw();
		}
	}

	// ************** (5) Draw the ground object layer (second pass) *************
/*
 	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
 	for (uint32 i = 0; i < _ground_objects.size(); i++) {
 		// The following function call only draws the object if it is visible on the screen.
 		(_ground_objects[i])->Draw();
 	}
*/
	// ************** (6) Draw the upper tile layer *************
	VideoManager->Move(_draw_info.c_pos, _draw_info.r_pos);
	for (int32 r = _draw_info.r_start; r < _draw_info.r_start + _draw_info.r_draw; r++) {
		for (int32 c = _draw_info.c_start; c < _draw_info.c_start + _draw_info.c_draw; c++) {
			if (_tile_layers[r][c].upper_layer >= 0) // Then an upper layer tile exists and we should draw it;
				_tile_images[_tile_layers[r][c].upper_layer]->Draw();
			VideoManager->MoveRelative(1.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(_draw_info.c_draw), 1.0f);
	}

	// ************* (7) Draw the sky object layer **********
	VideoManager->SetDrawFlags(VIDEO_NO_BLEND, 0);
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	for (uint32 i = 0; i < _sky_objects.size(); i++) {
		// Only draw objects that are visible
		if (_sky_objects[i]->status & VISIBLE) {
			(_sky_objects[i])->Draw();
		}
	}

	VideoManager->ApplyLightingOverlay();

	// Disable lighting for dialogue menus and GUI
	VideoManager->DisableSceneLighting();
	VideoManager->DisablePointLights();

	// ************** (8) Draw the dialogue menu and text *************
	if (_map_state == DIALOGUE) {
		VideoManager->PushState();
		VideoManager->SetCoordSys(0, 1024, 768, 0);
		VideoManager->Move(0.0f, 768.0f);
		_dialogue_box.Draw();
		VideoManager->MoveRelative(47.0f, -42.0f);
		_dialogue_nameplate.Draw();

		VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, 0);
		VideoManager->SetTextColor(Color(Color::black));
		VideoManager->SetFont("map");
		VideoManager->MoveRelative(120.0f, -6.0f);
		VideoManager->DrawText(_sprites[_current_dialogue->speakers[_current_dialogue->current_line]]->name);
		if (_sprites[_current_dialogue->speakers[_current_dialogue->current_line]]->portrait != NULL) {
			VideoManager->MoveRelative(0.0f, -26.0f);
			_sprites[_current_dialogue->speakers[_current_dialogue->current_line]]->portrait->Draw();
		}
		_dialogue_textbox.Draw();
		VideoManager->PopState();
	}

	return;
} // MapMode::_Draw()

} // namespace hoa_map
