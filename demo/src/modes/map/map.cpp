///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for map mode interface.
*** ***************************************************************************/

#include "utils.h"
#include "map.h"
#include "map_objects.h"
#include "map_dialogue.h"
#include "audio.h"
#include "video.h"
#include "global.h"
#include "script.h"
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
using namespace hoa_script;
using namespace hoa_battle;
using namespace hoa_menu;
using namespace luabind;

namespace hoa_map {

bool MAP_DEBUG = false;
// Initialize static class variable
MapMode *MapMode::_current_map = NULL;

// ****************************************************************************
// ************************** MapMode Class Functions *************************
// ****************************************************************************
// ***************************** GENERAL FUNCTIONS ****************************
// ****************************************************************************

MapMode::MapMode() {
	if (MAP_DEBUG) cout << "MAP: MapMode constructor invoked" << endl;

	mode_type = MODE_MANAGER_MAP_MODE;

	_map_state = EXPLORE;

	_virtual_focus = new VirtualSprite();
	_virtual_focus->SetXPosition(0, 0.0f);
	_virtual_focus->SetYPosition(0, 0.0f);
	_virtual_focus->movement_speed = NORMAL_SPEED;
	_virtual_focus->SetNoCollision(true);
	_virtual_focus->SetVisible(false);

	// TODO: Load the map data in a seperate thread
	Load();
}



MapMode::~MapMode() {
	if (MAP_DEBUG) cout << "MAP: MapMode destructor invoked" << endl;

	for (uint32 i = 0; i < _music.size(); i++) {
		_music[i].FreeMusic();
	}

	for (uint32 i = 0; i < _sounds.size(); i++) {
		_sounds[i].FreeSound();
	}

	// Delete all of the tile images
	for (uint32 i = 0; i < _tile_images.size(); i++) {
		VideoManager->DeleteImage(*(_tile_images[i]));
	}

	// Delete all of the map objects
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		delete(_ground_objects[i]);
	}
	for (uint32 i = 0; i < _pass_objects.size(); i++) {
		delete(_pass_objects[i]);
	}
	for (uint32 i = 0; i < _sky_objects.size(); i++) {
		delete(_sky_objects[i]);
	}
	delete(_virtual_focus);

	// Free up the dialogue window
	VideoManager->DeleteImage(_dialogue_box);
	_dialogue_window.Destroy();
}


// Resets appropriate class members.
void MapMode::Reset() {
	// Reset active video engine properties
	VideoManager->SetCoordSys(0.0f, SCREEN_COLS, SCREEN_ROWS, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, 0);

	if (!VideoManager->SetFont("default"))
    	cerr << "MAP ERROR: Failed to set the map font" << endl;

	// Let all map objects know that this is the current map
	MapMode::_current_map = this;

	// TEMP: This will need to be scripted later
	if (_music.size() > 0 && _music[0].GetMusicState() != AUDIO_STATE_PLAYING) {
		_music[0].PlayMusic();
	}
}


// Loads the map from a Lua file.
void MapMode::Load() {
	// TEMP: All of this is temporary, and will be replaced later
	_map_filename = "dat/maps/nofile.lua";

	// ---------- (1) Open map script file and begin loading data
	_num_tile_rows = 50;
	_num_tile_cols = 60;

	// ---------- (2) Load in the map tileset images
	vector<string> tileset_filenames;
	vector<StillImage> tile_images;

	tileset_filenames.push_back("img/tiles/ll_floor1.png");
	tileset_filenames.push_back("img/tiles/ll_floor2.png");
	tileset_filenames.push_back("img/tiles/ll_floor_horizontal_sand_left.png");
	tileset_filenames.push_back("img/tiles/ll_floor_horizontal_sand_right.png");
	tileset_filenames.push_back("img/tiles/ll_floor3.png");
	tileset_filenames.push_back("img/tiles/ol_rock_02.png");
	tileset_filenames.push_back("img/tiles/ol_rock_03.png");

	for (uint32 i = 0; i < tileset_filenames.size(); i++) {
		StillImage *new_img = new StillImage();
		new_img->SetFilename(tileset_filenames[i]);
		new_img->SetDimensions(2.0f, 2.0f);
		if (new_img->Load() == false)
			cerr << "MAP ERROR: tile image " << new_img->GetFilename() << " failed to load" << endl;
		_tile_images.push_back(new_img);
	}

	// ---------- (3) Setup the tile grid and map grid
	for (uint32 r = 0; r < _num_tile_rows; r++) {
		_tile_grid.push_back(vector<MapTile>());
		for (uint32 c = 0; c < _num_tile_cols; c++) {
			_tile_grid[r].push_back(MapTile());
			_tile_grid[r][c].lower_layer = (r + c) % 5;
			if ((r + c) % 35 == 0)
				_tile_grid[r][c].middle_layer = 5;
			else if ((r + c) % 47 == 0)
				_tile_grid[r][c].middle_layer = 6;
			else
				_tile_grid[r][c].middle_layer = -1;
			_tile_grid[r][c].upper_layer = -1;
		}
	}

	for (uint16 r = 0; r < _num_tile_rows * 2; r++) {
		_map_grid.push_back(vector<bool>(_num_tile_cols * 2, false));
	}

	// Uncomment this loop to test out tile-collision detection
// 	for (uint16 r = 0; r < _num_tile_rows * 2; r++) {
// 		for (uint16 c = 0; c < _num_tile_cols * 2; c++) {
// 			if ((r + c) % 70 == 0) {
// 				_map_grid[r][c] = true;
// 			}
// 		}
// 	}

	MapSprite *sp;

	// Load player sprite and rest of map objects
	sp = new MapSprite();
	sp->name = MakeUnicodeString("Claudius");
	sp->SetObjectID(555);
	sp->SetContext(1);
	sp->SetXPosition(55, 0.5f);
	sp->SetYPosition(55, 0.5f);
	sp->SetCollHalfWidth(1.0f);
	sp->SetCollHeight(2.0f);
	sp->img_half_width = 1.0f;
	sp->img_height = 4.0f;
	sp->movement_speed = NORMAL_SPEED;
	sp->direction = SOUTH;
	if (sp->Load() == false)
		return;
	_ground_objects.push_back(sp);
	_camera = sp;

	// ---------- (1) Setup GUI items (in a 1024x768 coordinate system)
	VideoManager->PushState();
	VideoManager->SetCoordSys(0, 1024, 768, 0);
	_dialogue_window.Create(1024.0f, 256.0f);
	_dialogue_window.SetPosition(0.0f, 512.0f);
	_dialogue_window.SetDisplayMode(VIDEO_MENU_EXPAND_FROM_CENTER);

	_dialogue_box.SetFilename("img/menus/dialogue_box.png");
	if (_dialogue_box.Load() == false)
		cerr << "MAP ERROR: failed to load image: " << _dialogue_box.GetFilename() << endl;

	_dialogue_nameplate.SetFilename("img/menus/dialogue_nameplate.png");
	if (_dialogue_nameplate.Load() == false) 
		cerr << "MAP ERROR: failed to load image: " << _dialogue_nameplate.GetFilename() << endl;

	_dialogue_textbox.SetDisplaySpeed(30);
	_dialogue_textbox.SetPosition(300.0f, 768.0f - 180.0f);
	_dialogue_textbox.SetDimensions(1024.0f - 300.0f - 60.0f, 180.0f - 70.0f);
	_dialogue_textbox.SetFont("default");
	_dialogue_textbox.SetDisplayMode(VIDEO_TEXT_FADECHAR);
	_dialogue_textbox.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	VideoManager->PopState();
} // Load()

// ****************************************************************************
// **************************** UPDATE FUNCTIONS ******************************
// ****************************************************************************

// Updates the game state when in map mode. Called from the main game loop.
void MapMode::Update() {
	_time_elapsed = SystemManager->GetUpdateTime();

	// ---------- (1) Process user input

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

	// ---------- (2) Update all animated tile images

	// TODO

	// ---------- (3) Update all objects on the map

	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		_ground_objects[i]->Update();
	}
	for (uint32 i = 0; i < _pass_objects.size(); i++) {
		_pass_objects[i]->Update();
	}
	for (uint32 i = 0; i < _sky_objects.size(); i++) {
		_sky_objects[i]->Update();
	}

	// ---------- (4) Sort the objects so they are in the correct draw order ********

	//! \todo Optimize this sorting algorithm
// 	for (uint32 i = 1; i < _ground_objects.size(); i++) {
// 		MapObject *tmp = _ground_objects[i];
// 		int32 j = static_cast<int32>(i) - 1;
// 		while (j >= 0 && (_ground_objects[j])->row_position > tmp->row_position) {
// 			_ground_objects[j+1] = _ground_objects[j];
// 			j--;
// 		}
// 		_ground_objects[j+1] = tmp;
// 	}
} // void MapMode::Update()


// Updates the game status when MapMode is in the 'explore' state
void MapMode::_UpdateExplore() {
	// Do the fade to battle mode
	// Doing this first should prevent user input
// 	if (_fade_to_battle_mode) {
// 		// Only start battle mode once the fade is done.
// 		if (!VideoManager->IsFading()) {
// 			// Clear fade instantly
// 			VideoManager->FadeScreen(Color::clear, 0.0f);
// 			_fade_to_battle_mode = false;
// 			BattleMode *BM = new BattleMode();
// 			ModeManager->Push(BM);
// 		}
// 		return;
// 	}

	// Go to menu mode if the user requested it
// 	if (InputManager->MenuPress()) {
// 		MenuMode *MM = new MenuMode();
// 		ModeManager->Push(MM);
// 		return;
// 	}

	// TEMPORARY: disable random encounters
// 	if (InputManager->SwapPress()) {
// 		_random_encounters = !_random_encounters;
// 	}

	// Toggle running versus walking
// 	if (InputManager->CancelPress()) {
// 		if (speed_double) {
// 			_focused_object->step_speed /= 2;
// 			speed_double = false;
// 		}
// 		else {
// 			_focused_object->step_speed *= 2;
// 			speed_double = true;
// 		}
// 	}

// 	if (InputManager->ConfirmPress()) {
// 		// Check for a sprite present within the space of one tile
// 		int16 check_row, check_col;
// 		if (_camera->direction & FACING_WEST) {
// 
// 		}
// 		else if (_camera->direction & FACING_EAST) {
// 
// 		}
// 		else if (_camera->direction & FACING_NORTH) {
// 
// 		}
// 		else { // then => (_camera->direction & FACING_SOUTH)) == true
// 
// 		}
// 
// 		return;
// 	} // if (InputManager->ConfirmPress())

	// Detect and handle movement input from the user
	if (InputManager->UpState() || InputManager->DownState() || InputManager->LeftState() || InputManager->RightState()) {
		_camera->moving = true;
	}
	else {
		_camera->moving = false;
	}

	// Determine the direction of movement. Priority of movement is given to: up, down, left, right.
	// In the case of diagonal movement, the direction that the sprite should face also needs to be
	// deduced.
	if (_camera->moving == true) {
		if (InputManager->UpState()) {
			if (InputManager->LeftState()) {
				// The sprite is moving northwest: determine if it should be facing north or west
				if (_camera->direction == (NW_NORTH | NORTH | NE_NORTH | NE_EAST | EAST | SE_EAST))
					_camera->direction = NW_NORTH;
				else
					_camera->direction = NW_WEST;
			}
			else if (InputManager->RightState()) {
				// The sprite is moving northeast: determine if it should be facing north or east
				if (_camera->direction == (NE_NORTH | NORTH | NW_NORTH | NW_WEST | WEST | SW_WEST))
					_camera->direction = NE_NORTH;
				else
					_camera->direction = NE_EAST;
			}
			else {
				_camera->direction = NORTH;
			}
		}
		else if (InputManager->DownState()) {
			if (InputManager->LeftState()) {
				// The sprite is moving southwest: determine if it should be facing south or west
				if (_camera->direction == (SW_SOUTH | SOUTH | SE_SOUTH | SE_EAST | EAST | NE_EAST))
					_camera->direction = SW_SOUTH;
				else
					_camera->direction = SW_WEST;
			}
			else if (InputManager->RightState()) {
				// The sprite is moving southeast: determine if it should be facing south or east
				if (_camera->direction == (SE_SOUTH | SOUTH | SW_SOUTH | SW_WEST | WEST | NW_WEST))
					_camera->direction = SE_SOUTH;
				else
					_camera->direction = SE_EAST;
			}
			else {
				_camera->direction = SOUTH;
			}
		}
		else if (InputManager->LeftState()) {
			_camera->direction = WEST;
		}
		else if (InputManager->RightState()) {
			_camera->direction = EAST;
		}

		// TEMP: random encounters will be removed at some point
// 		if (_random_encounters) {
			// todo: subtract the encounter timer based on the update time, and start a battle if
			// the time expired
// 		}
	} // if (_camera->moving == true)
} // void MapMode::_UpdateExplore()


// Updates the game status when MapMode is in the 'dialogue' state
void MapMode::_UpdateDialogue() {
// 	_dialogue_window.Update(_time_elapsed);
// 	_dialogue_textbox.Update(_time_elapsed);
// 
// 	if (InputManager->ConfirmPress()) {
// 		if (!_dialogue_textbox.IsFinished()) {
// 			_dialogue_textbox.ForceFinish();
// 		}
// 		else {
// 			bool not_finished = _current_dialogue->ReadNextLine();
// 
// 			if (!not_finished) {
// 				_dialogue_window.Hide();
// 				_map_state = EXPLORE;
// 				// Restore the status of the map sprites
// 				for (uint32 i = 0; i < _current_dialogue->speakers.size(); i++) {
// 					_sprites[_current_dialogue->speakers[i]]->RestoreState();
// 				}
// 				_sprites[1]->UpdateConversationCounter();
// 				_current_dialogue = NULL;
// 			}
// 			else { // Otherwise, the dialogue is automatically updated to the next line
// 				_dialogue_textbox.SetDisplayText(_current_dialogue->text[_current_dialogue->current_line]);
// 			}
// 		}
// 	}
} // void MapMode::_UpdateDialogue()


bool MapMode::_DetectCollision(VirtualSprite* sprite) {
	// NOTE: Whether the argument pointer is valid is not checked here, since the object pointer
	// itself presumably called this function.

	// The single X,Y floating point coordinates of the sprite
	float x_location = static_cast<float>(sprite->x_position) + sprite->x_offset;
	float y_location = static_cast<float>(sprite->y_position) + sprite->y_offset;

	// The coordinates corresponding to the four sides of the sprite's collision rectangle (cr)
	float cr_left = x_location - sprite->coll_half_width;
	float cr_right = x_location + sprite->coll_half_width;
	float cr_top = y_location - sprite->coll_height;
	// The bottom of the sprite's collision rectangle is its y_location

	// ---------- (1): Check if the sprite's position has gone out of bounds

	if (cr_left < 0.0f || cr_top < 0.0f || cr_right >= static_cast<float>(_num_tile_cols * 2) ||
		y_location >= static_cast<float>(_num_tile_rows * 2)) {
		return true;
	}

	// Do not do tile or object based collision detection if this member is set
	if (sprite->no_collision == true) {
		return false;
	}

	// ---------- (2): Determine if the sprite's collision rectangle overlaps any unwalkable tiles

	// NOTE: Because the sprite's collision rectangle was determined to be within the map bounds,
	// the map grid tile indeces referenced in this loop are all valid entries.

	for (uint32 r = static_cast<uint32>(cr_top); r <= static_cast<uint32>(y_location); r++) {
		for (uint32 c = static_cast<uint32>(cr_left); c <= static_cast<uint32>(cr_right); c++) {
			if (_map_grid[r][c] == true) { // Then this overlapping tile is unwalkable
				return true;
			}
		}
	}

	// ---------- (3): Determine if two sprite's collision rectangles overlap

	// TODO

	// No collision was detected
	return false;
} // bool MapMode::_DetectCollision(MapSprite* sprite)

// ****************************************************************************
// **************************** DRAW FUNCTIONS ********************************
// ****************************************************************************

// Determines things like our starting tiles
void MapMode::_CalculateDrawInfo() {
	// ---------- (1) Set the default starting draw positions for the tiles (top left tile)

	// The camera's position is in terms of the 16x16 grid, which needs to be converted into 32x32 coordinates.
	float camera_x = static_cast<float>(_camera->x_position) + _camera->x_offset;
	float camera_y = static_cast<float>(_camera->y_position) + _camera->y_offset;

	// Determine the draw coordinates of the top left corner using the camera's current position
	_draw_info.tile_x_start = 1.0f - _camera->x_offset;
	if (IsOddNumber(_camera->x_position))
		_draw_info.tile_x_start += -1.0f;

	_draw_info.tile_y_start = 2.0f - _camera->y_offset;
	if (IsOddNumber(_camera->y_position))
		_draw_info.tile_y_start += -1.0;

	// By default the map draws 32 + 1 columns and 24 + 1 rows of tiles, the maximum that can fit on the screen.
	_draw_info.num_draw_cols = TILE_COLS + 1;
	_draw_info.num_draw_rows = TILE_ROWS + 1;

	// The default starting tile row and column is relative to the map camera's current position.
	_draw_info.starting_col = static_cast<int16>(camera_x / 2.0f) - HALF_TILE_COLS;
	_draw_info.starting_row = static_cast<int16>(camera_y / 2.0f) - HALF_TILE_ROWS;

	// ---------- (2) Determine the coordinates for the screen edges on the map grid

	_draw_info.top_edge    = camera_y - HALF_SCREEN_ROWS;
	_draw_info.bottom_edge = camera_y + HALF_SCREEN_ROWS;
	_draw_info.left_edge   = camera_x - HALF_SCREEN_COLS;
	_draw_info.right_edge  = camera_x + HALF_SCREEN_COLS;

	// ---------- (3) Check for special conditions that modify the drawing state

	// Usually the map centers on the camera's position, but when the camera becomes close to
	// the edges of the map, we need to modify the drawing properties of the frame.

	// Camera exceeds the left boundary of the map
	if (_draw_info.starting_col < 0) {
		_draw_info.starting_col = 0;
		_draw_info.tile_x_start = 1.0f;
		_draw_info.left_edge = 0.0f;
		_draw_info.right_edge = SCREEN_COLS;
	}
	// Camera exceeds the right boundary of the map
	else if (_draw_info.starting_col + TILE_COLS >= _num_tile_cols) {
		_draw_info.starting_col = static_cast<int16>(_num_tile_cols - TILE_COLS);
		_draw_info.tile_x_start = 1.0f;
		_draw_info.right_edge = static_cast<float>(_num_tile_cols * 2);
		_draw_info.left_edge = _draw_info.right_edge - SCREEN_COLS;
	}

	// Camera exceeds the top boundary of the map
	if (_draw_info.starting_row < 0) {
		_draw_info.starting_row = 0;
		_draw_info.tile_y_start = 2.0f;
		_draw_info.top_edge = 0.0f;
		_draw_info.bottom_edge = SCREEN_ROWS;
	}
	// Camera exceeds the bottom boundary of the map
	else if (_draw_info.starting_row + TILE_ROWS >= _num_tile_rows) {
		_draw_info.starting_row = static_cast<int16>(_num_tile_rows - TILE_ROWS);
		_draw_info.tile_y_start = 2.0f;
		_draw_info.bottom_edge = static_cast<float>(_num_tile_rows * 2);
		_draw_info.top_edge = _draw_info.bottom_edge - SCREEN_ROWS;
	}

	// Check for the conditions where the tile images align perfectly with the screen and one less row or column of tiles is drawn
	if (IsFloatInRange(_draw_info.tile_x_start, 0.999, 1.001)) { // Is the value approximately equal to 1.0f?
		_draw_info.num_draw_cols--;
	}
	if (IsFloatInRange(_draw_info.tile_y_start, 1.999, 2.001)) { // Is the value approximately equal to 2.0f?
		_draw_info.num_draw_rows--;
	}

// 	printf("--- DRAW INFO ---\n");
// 	printf("Starting row, col: [%d, %d]\n", _draw_info.starting_row, _draw_info.starting_col);
// 	printf("# draw rows, cols: [%d, %d]\n", _draw_info.num_draw_rows, _draw_info.num_draw_cols);
// 	printf("Camera position:   [%f, %f]\n", camera_x, camera_y);
// 	printf("Tile draw start:   [%f, %f]\n", _draw_info.tile_x_start, _draw_info.tile_y_start);
// 	printf("Edges (T,D,L,R):   [%f, %f, %f, %f]\n", _draw_info.top_edge, _draw_info.bottom_edge, _draw_info.left_edge, _draw_info.right_edge);
} // void MapMode::_CalculateDrawInfo()


// Public draw function called by the main game loop
void MapMode::Draw() {
	_CalculateDrawInfo();

	// ---------- (1) Call Lua to determine if any lighting, etc. needs to be done before drawing
	// TODO
	// ---------- (2) Draw the lower tile layer
	VideoManager->SetDrawFlags(VIDEO_NO_BLEND, 0);
	VideoManager->Move(_draw_info.tile_x_start, _draw_info.tile_y_start);
	for (uint32 r = static_cast<uint32>(_draw_info.starting_row);
			r < static_cast<uint32>(_draw_info.starting_row + _draw_info.num_draw_rows); r++) {
		for (uint32 c = static_cast<uint32>(_draw_info.starting_col);
				c < static_cast<uint32>(_draw_info.starting_col + _draw_info.num_draw_cols); c++) {
			if (_tile_grid[r][c].lower_layer >= 0) { // Draw a tile image if it exists at this location
				_tile_images[_tile_grid[r][c].lower_layer]->Draw();
			}
			VideoManager->MoveRelative(2.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(_draw_info.num_draw_cols * 2), 2.0f);
	}

	// ---------- (2) Draw the middle tile layer *************
	VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
	VideoManager->Move(_draw_info.tile_x_start, _draw_info.tile_y_start);
	for (uint32 r = static_cast<uint32>(_draw_info.starting_row);
			r < static_cast<uint32>(_draw_info.starting_row + _draw_info.num_draw_rows); r++) {

		for (uint32 c = static_cast<uint32>(_draw_info.starting_col);
				c < static_cast<uint32>(_draw_info.starting_col + _draw_info.num_draw_cols); c++) {
			if (_tile_grid[r][c].middle_layer >= 0) { // Draw a tile image if it exists at this location
				_tile_images[_tile_grid[r][c].middle_layer]->Draw();
			}
			VideoManager->MoveRelative(2.0f, 0.0f);
		}

		VideoManager->MoveRelative(static_cast<float>(_draw_info.num_draw_cols * -2), 2.0f);
	}

	// ---------- (3) Draw the ground object layer (first pass)
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		if (_ground_objects[i]->draw_on_second_pass == false) {
			_ground_objects[i]->Draw();
		}
	}

	// ---------- (4) Draw the pass object layer
	for (uint32 i = 0; i < _pass_objects.size(); i++) {
		_pass_objects[i]->Draw();
	}

	// ---------- (5) Draw the ground object layer (second pass)
	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		if (_ground_objects[i]->draw_on_second_pass == true) {
			_ground_objects[i]->Draw();
		}
	}

	// ---------- (6) Draw the upper tile layer
	VideoManager->Move(_draw_info.tile_x_start, _draw_info.tile_y_start);
	for (uint32 r = static_cast<uint32>(_draw_info.starting_row);
			r < static_cast<uint32>(_draw_info.starting_row + _draw_info.num_draw_rows); r++) {
		for (uint32 c = static_cast<uint32>(_draw_info.starting_col);
				c < static_cast<uint32>(_draw_info.starting_col + _draw_info.num_draw_cols); c++) {
			if (_tile_grid[r][c].upper_layer >= 0) { // Draw a tile image if it exists at this location
				_tile_images[_tile_grid[r][c].upper_layer]->Draw();
			}
			VideoManager->MoveRelative(2.0f, 0.0f);
		}
		VideoManager->MoveRelative(-static_cast<float>(_draw_info.num_draw_cols * 2), 2.0f);
	}

	// ---------- (7) Draw the sky object layer
	for (uint32 i = 0; i < _sky_objects.size(); i++) {
		_sky_objects[i]->Draw();
	}

	// ---------- (8) Call Lua to determine if any lighting, etc. needs to be done after drawing
	// TODO

	// ---------- (9) Draw the dialogue menu and text if necessary
	if (_map_state == DIALOGUE) {
		// TODO
	}

} // void MapMode::_Draw()

} // namespace hoa_map
