///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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
#include "map_sprites.h"
#include "map_dialogue.h"
#include "map_zones.h"
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

namespace hoa_map {

bool MAP_DEBUG = false;
// Initialize static class variables
MapMode *MapMode::_current_map = NULL;
MapMode *MapMode::_loading_map = NULL;
bool MapMode::_show_dialogue_icons = true;

// ****************************************************************************
// ************************** MapMode Class Functions *************************
// ****************************************************************************
// ***************************** GENERAL FUNCTIONS ****************************
// ****************************************************************************

MapMode::MapMode(string filename) :
	GameMode(),
	_map_filename(filename),
	_map_state(EXPLORE),
	_lastID(1000),
	_ignore_input(false),
	_running(false),
	_run_forever(false),
	_run_disabled(false),
	_run_stamina(10000)
{
	if (MAP_DEBUG)
		cout << "MAP: MapMode constructor invoked" << endl;
	_loading_map = this;

	mode_type = MODE_MANAGER_MAP_MODE;
	_virtual_focus = new VirtualSprite();
	_virtual_focus->SetXPosition(0, 0.0f);
	_virtual_focus->SetYPosition(0, 0.0f);
	_virtual_focus->movement_speed = NORMAL_SPEED;
	_virtual_focus->SetNoCollision(true);
	_virtual_focus->SetVisible(false);

	_dialogue_manager = new DialogueManager();

	_intro_timer.Initialize(7000, 0, this);

	// TODO: Load the map data in a seperate thread
	_Load();

	// TEMP: Load dialogue icon
	if (private_map::new_dialogue_icon.GetNumFrames() == 0) {
		std::vector<StillImage> frames;
		VideoManager->LoadMultiImageFromElementsSize(frames, "img/misc/dialogue_icon.png", 32, 32);

		for( size_t i = 0; i < frames.size(); ++i ) {
			private_map::new_dialogue_icon.AddFrame(frames[i], 100);
		}

		private_map::new_dialogue_icon.SetDimensions(2, 2);
		private_map::new_dialogue_icon.Load();
	}


	// TEMP: chest test
	ChestObject* chest = new ChestObject( "img/misc/chest1.png", 1, 3 );
	chest->SetObjectID( 7689 );
	chest->SetXPosition(111, 0);
	chest->SetYPosition(36, 0);
	chest->AddObject( 1, 50 );

	if( GlobalManager->DoesEventGroupExist( "chest_test_group" ) ) {
		if( GlobalManager->DoesEventExist( "chest_test_group", "chest_7689" ) ) {
			cout << "Exists" << endl;
			if( GlobalManager->GetEventValue( "chest_test_group", "chest_7689" ) == 1 ) {
				cout << "Cleared" << endl;
				chest->Clear();
			}
		} else {
			cout << "Doesn't exist" << endl;
			GlobalManager->GetEventGroup( "chest_test_group" )->AddNewEvent( "chest_7689", 0 );
		}
	}
	else {
		cout << "Group Doesn't exist" << endl;
		GlobalManager->AddNewEventGroup( "chest_test_group" );
	}

	_AddGroundObject( chest );
}



MapMode::~MapMode() {
	if (MAP_DEBUG) cout << "MAP: MapMode destructor invoked" << endl;

	for (uint32 i = 0; i < _music.size(); i++) {
		_music[i].FreeMusic();
	}

	for (uint32 i = 0; i < _sounds.size(); i++) {
		_sounds[i].FreeSound();
	}

	// Delete all enemy's created
	for (uint32 i = 0; i < _enemies.size(); i++) {
		delete(_enemies[i]);
	}

	// Delete all of the tile images
	for (uint32 i = 0; i < _tile_images.size(); i++) {
		delete(_tile_images[i]);
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

	delete(_dialogue_manager);

	_map_script.CloseFile();
}


// Resets appropriate class members.
void MapMode::Reset() {
	// Reset active video engine properties
	VideoManager->SetCoordSys(0.0f, SCREEN_COLS, SCREEN_ROWS, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, 0);

	if (!VideoManager->SetFont("map"))
    	cerr << "MAP ERROR: Failed to set the map font" << endl;

	// Let all map objects know that this is the current map
	MapMode::_current_map = this;

	// TEMP: This will need to be scripted later
	if (_music.size() > 0 && _music[0].IsPlaying() == false) {
		_music[0].PlayMusic();
	}

	_intro_timer.Run();
}


// Loads the map from a Lua file.
void MapMode::_Load() {
	// ---------- (1) Open map script file and read in basic map properties and tile definitions
	if (_map_script.OpenFile(_map_filename) == false) {
		return;
	}

	_map_name = MakeUnicodeString(_map_script.ReadString("map_name"));
	_location_graphic.SetFilename("img/menus/locations/" + _map_script.ReadString("location_filename"));
	if (_location_graphic.Load() == false) {
		cerr << "MAP ERROR: failed to load location graphic image: " << _location_graphic.GetFilename() << endl;
	}

	_num_tile_rows = _map_script.ReadInt("num_tile_rows");
	_num_tile_cols = _map_script.ReadInt("num_tile_cols");
	_num_grid_rows = _num_tile_rows * 2;
	_num_grid_cols = _num_tile_cols * 2;

	// Do some checking to make sure tables are of the proper size
	// NOTE: we only check that the number of rows are correct, but not the number of columns
	// NOTE: the "+ 1" is due to a bug in ReadGetTableSize that is not yet resolved
	if (_map_script.GetTableSize("map_grid") + 1 != _num_grid_rows) {
		cerr << "MAP ERROR: In MapMode::_Load(), the map_grid table had an incorrect number of rows" << endl;
		return;
	}
	if (_map_script.GetTableSize("lower_layer") + 1 != _num_tile_rows) {
		cerr << "MAP ERROR: In MapMode::_Load(), the lower_layer table had an incorrect number of rows" << endl;
		return;
	}
	if (_map_script.GetTableSize("middle_layer") + 1 != _num_tile_rows) {
		cerr << "MAP ERROR: In MapMode::_Load(), the middle_layer table had an incorrect number of rows" << endl;
		return;
	}
	if (_map_script.GetTableSize("upper_layer") + 1 != _num_tile_rows) {
		cerr << "MAP ERROR: In MapMode::_Load(), the upper_layer table had an incorrect number of rows" << endl;
		return;
	}

	// ---------- (2) Initialize all of the tile and grid mappings
	_LoadTiles();

	// ---------- (3) Load map sounds and music
	vector<string> sound_filenames;
	_map_script.ReadStringVector("sound_filenames", sound_filenames);

	for (uint32 i = 0; i < sound_filenames.size(); i++) {
		SoundDescriptor new_sound;
		if (new_sound.LoadSound(sound_filenames[i]) == true) {
			_sounds.push_back(new_sound);
		}
		else {
			cerr << "MAP ERROR: failed to load map sound: " << sound_filenames[i] << endl;
			return;
		}
	}

	vector<string> music_filenames;
	_map_script.ReadStringVector("music_filenames", music_filenames);
	for (uint32 i = 0; i < music_filenames.size(); i++) {
		MusicDescriptor new_music;
		if (new_music.LoadMusic(music_filenames[i]) == true) {
			_music.push_back(new_music);
		}
		else {
			cerr << "MAP ERROR: failed to load map music: " << music_filenames[i] << endl;
			return;
		}
	}

	// ---------- (4) Construct all enemies that may appear on this map
	vector<int32> enemy_ids;
	_map_script.ReadIntVector("enemy_ids", enemy_ids);
	for (uint32 i = 0; i < enemy_ids.size(); i++) {
		_enemies.push_back(new GlobalEnemy(enemy_ids[i]));
	}

	// ---------- (5) Call the map script's load function
	ScriptCallFunction<void>(_map_script.GetLuaState(), "Load", this);
	_update_function = _map_script.ReadFunctionPointer("Update");
	_draw_function = _map_script.ReadFunctionPointer("Draw");
} // void MapMode::_Load()



void MapMode::_LoadTiles() {
	// Contains all of the tileset filenames used (string does not contain path information or file extensions)
	vector<string> tileset_filenames;
	// A container to temporarily retain all tile images loaded for each tileset. Each inner vector contains 256 StillImages
	vector<vector<StillImage> > tileset_images;
	// Used to determine whether each tile is used by the map or not. An entry of -1 indicates that particular tile is not used
	vector<int32> tile_references;
	// Temporarily holds all animated tile images. The map key is the value of the tile index, before translation
	map<uint32, AnimatedImage> tile_animations;

	// ---------- (1) Construct the map grid (contains map traveriblity information)

	_map_script.OpenTable("map_grid");
	for (uint16 r = 0; r < _num_grid_rows; r++) {
		_map_grid.push_back(vector<uint32>());
		_map_script.ReadUIntVector(r, _map_grid.back());
	}
	_map_script.CloseTable();
	// ---------- (2) Load in the map tileset images and initialize all map tiles

	_map_script.ReadStringVector("tileset_filenames", tileset_filenames);

	// Note: each tileset image is 512x512 pixels, yielding 256 32x32 pixel tiles each
	for (uint32 i = 0; i < tileset_filenames.size(); i++) {
		// Construct the image filename from the tileset filename and create a new vector to use in the LoadMultiImage call
		string image_filename = "img/tilesets/" + tileset_filenames[i] + ".png";
		tileset_images.push_back(vector<StillImage>(TILES_PER_TILESET));

		for (uint32 j = 0; j < TILES_PER_TILESET; j++) {
			tileset_images[i][j].SetDimensions(2.0f, 2.0f);
		}

		if (VideoManager->LoadMultiImageFromNumberElements(tileset_images[i], image_filename, 16, 16) == false) {
			cerr << "MAP ERROR: MapMode::_LoadTiles() failed to load tileset image:" << image_filename << endl;
			return;
		}
	}
	// ---------- (3) Read in the map tile indeces from all three tile layers

	// First create the properly sized 2D grid of map tiles, then read the tile indeces from the map file
	for (uint32 r = 0; r < _num_tile_rows; r++) {
		_tile_grid.push_back(vector<MapTile>(_num_tile_cols));
	}

	vector<int32> table_row; // Used to temporarily store a row of integer table data

	// Read the lower_layer
	_map_script.OpenTable("lower_layer");
	for (uint32 r = 0; r < _num_tile_rows; r++) {
		table_row.clear();
		_map_script.ReadIntVector(r, table_row);
		for (uint32 c = 0; c < _num_tile_cols; c++) {
			_tile_grid[r][c].lower_layer = table_row[c];
		}
	}
	_map_script.CloseTable();

	// Read the middle_layer
	_map_script.OpenTable("middle_layer");
	for (uint32 r = 0; r < _num_tile_rows; r++) {
		table_row.clear();
		_map_script.ReadIntVector(r, table_row);
		for (uint32 c = 0; c < _num_tile_cols; c++) {
			_tile_grid[r][c].middle_layer = table_row[c];
		}
	}
	_map_script.CloseTable();

	// Read the upper_layer
	_map_script.OpenTable("upper_layer");
	for (uint32 r = 0; r < _num_tile_rows; r++) {
		table_row.clear();
		_map_script.ReadIntVector(r, table_row);
		for (uint32 c = 0; c < _num_tile_cols; c++) {
			_tile_grid[r][c].upper_layer = table_row[c];
		}
	}
	_map_script.CloseTable();

	// ---------- (4) Determine which tiles in each tileset are referenced in this map

	// Set size to be equal to the total number of tiles and initialize all entries to -1 (unreferenced)
	tile_references.assign(tileset_filenames.size() * TILES_PER_TILESET, -1);

	for (uint32 r = 0; r < _num_tile_rows; r++) {
		for (uint32 c = 0; c < _num_tile_cols; c++) {
			if (_tile_grid[r][c].lower_layer >= 0)
				tile_references[_tile_grid[r][c].lower_layer] = 0;
			if (_tile_grid[r][c].middle_layer >= 0)
				tile_references[_tile_grid[r][c].middle_layer] = 0;
			if (_tile_grid[r][c].upper_layer >= 0)
				tile_references[_tile_grid[r][c].upper_layer] = 0;
		}
	}

	// Here, we have to convert the original tile indeces defined in the map file into a new form. The original index
	// indicates the tileset where the tile is used and its location in that tileset. We need to convert those indecs
	// so that they serve as an index to the MapMode::_tile_images vector, where the tile images will soon be stored.

	// Keeps track of the next translated index number to assign
	uint32 next_index = 0;

	for (uint32 i = 0; i < tile_references.size(); i++) {
		if (tile_references[i] >= 0) {
			tile_references[i] = next_index;
			next_index++;
		}
	}

	// Now, go back and re-assign all lower, middle, and upper tile layer indeces with the translated indeces
	for (uint32 r = 0; r < _num_tile_rows; r++) {
		for (uint32 c = 0; c < _num_tile_cols; c++) {
			if (_tile_grid[r][c].lower_layer >= 0)
				_tile_grid[r][c].lower_layer = tile_references[_tile_grid[r][c].lower_layer];
			if (_tile_grid[r][c].middle_layer >= 0)
				_tile_grid[r][c].middle_layer = tile_references[_tile_grid[r][c].middle_layer];
			if (_tile_grid[r][c].upper_layer >= 0)
				_tile_grid[r][c].upper_layer = tile_references[_tile_grid[r][c].upper_layer];
		}
	}

	// ---------- (5) Parse all of the tileset definition files and create any animated tile images that are used

	ReadScriptDescriptor tileset_script; // Used to access the tileset definition file
	vector<uint32> animation_info;   // Temporarily retains the animation data (tile frame indeces and display times)

	for (uint32 i = 0; i < tileset_filenames.size(); i++) {
		if (tileset_script.OpenFile("dat/tilesets/" + tileset_filenames[i] + ".lua") == false) {
			cerr << "MAP ERROR: In MapMode::_LoadTiles(), the map failed to load because it could not open a tileset definition file: "
				<< tileset_script.GetFilename() << endl;
			return;
		}

		tileset_script.OpenTable("animated_tiles");
		for (uint32 j = 1; j <= tileset_script.GetTableSize(); j++) {
			animation_info.clear();
			tileset_script.ReadUIntVector(j, animation_info);

			// The index of the first frame in the animation. (i * TILES_PER_TILESET) factors in which tileset the frame comes from
			uint32 first_frame_index = animation_info[0] + (i * TILES_PER_TILESET);

			// Check if this animation is referenced in the map by looking at the first tile frame index. If it is not, continue on to the next animation
			if (tile_references[first_frame_index] == -1) {
				continue;
			}

			AnimatedImage new_animation;
			new_animation.SetDimensions(2.0f, 2.0f);

			// Each pair of entries in the animation info indicate the tile frame index (k) and the time (k+1)
			for (uint32 k = 0; k < animation_info.size(); k += 2) {
				new_animation.AddFrame(tileset_images[i][animation_info[k]], animation_info[k+1]);
			}

			tile_animations.insert(make_pair(first_frame_index, new_animation));
		}
		tileset_script.CloseTable();
		tileset_script.CloseFile();
	} // for (uint32 i = 0; i < tileset_filenames.size(); i++)

	// ---------- (6) Add all referenced tiles to the _tile_images vector, in the proper order

	for (uint32 i = 0; i < tileset_images.size(); i++) {
		for (uint32 j = 0; j < TILES_PER_TILESET; j++) {
			uint32 reference = (i * TILES_PER_TILESET) + j;

			if (tile_references[reference] >= 0) {
				// Add the tile as a StillImage
				if (tile_animations.find(reference) == tile_animations.end()) {
					_tile_images.push_back(new StillImage(tileset_images[i][j]));
				}

				// Add the tile as an AnimatedImage
				else {
					AnimatedImage* new_animated_tile = new AnimatedImage(tile_animations[reference]);
					_tile_images.push_back(new_animated_tile);
					_animated_tile_images.push_back(new_animated_tile);
				}
			}
		}
	}

	// Remove all tileset images. Any tiles which were not added to _tile_images will no longer exist in memory
	tileset_images.clear();
} // void MapMode::_LoadTiles()

// ****************************************************************************
// **************************** UPDATE FUNCTIONS ******************************
// ****************************************************************************

// Updates the game state when in map mode. Called from the main game loop.
void MapMode::Update() {
	_time_elapsed = SystemManager->GetUpdateTime();
	_running = false;

	// ---------- (1) Call the map's update script function
	ScriptCallFunction<void>(_update_function);

	// ---------- (2) Process user input
	if (_ignore_input == false) {
		if (_map_state == DIALOGUE)
			_dialogue_manager->Update();
		else
			_HandleInputExplore();
	}

	// ---------- (3) Update all animated tile images
	for (uint32 i = 0; i < _animated_tile_images.size(); i++) {
		_animated_tile_images[i]->Update();
	}

	// ---------- (4) Update all objects on the map
	for( uint32 i = 0; i < _zones.size(); i++ ) {
		_zones[i]->Update();
	}

	for (uint32 i = 0; i < _ground_objects.size(); i++) {
		_ground_objects[i]->Update();
	}
	for (uint32 i = 0; i < _pass_objects.size(); i++) {
		_pass_objects[i]->Update();
	}
	for (uint32 i = 0; i < _sky_objects.size(); i++) {
		_sky_objects[i]->Update();
	}

	// ---------- (5) Sort the objects so they are in the correct draw order ********
	std::sort(_ground_objects.begin(), _ground_objects.end(), MapObject_Ptr_Less());
} // void MapMode::Update()


// Updates the game status when MapMode is in the 'explore' state
void MapMode::_HandleInputExplore() {
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
	if (InputManager->MenuPress()) {
		MenuMode *MM = new MenuMode(_map_name, _location_graphic.GetFilename());
		ModeManager->Push(MM);
		return;
	}

	// If the player is trying to run, update the stamina amount
	if (InputManager->CancelState() == true && _run_disabled == false) {
		if (_run_forever) {
			_running = true;
		}
		else if (_run_stamina > _time_elapsed * 2) {
			_run_stamina -= (_time_elapsed * 2);
			_running = true;
		}
		else {
			_run_stamina = 0;
		}
	}
	else if (_run_stamina < 10000) {
		_run_stamina += _time_elapsed;
		if (_run_stamina > 10000)
			_run_stamina = 10000;
	}

	if (InputManager->ConfirmPress()) {
		MapObject* obj = _FindNearestObject(_camera);
		if (obj && (obj->GetType() == VIRTUAL_TYPE || obj->GetType() == SPRITE_TYPE)) {
			VirtualSprite *sp = reinterpret_cast<VirtualSprite*>(obj);

			if (sp->HasDialogue()) {
				sp->SaveState();
				_camera->moving = false;

				sp->moving = false;
				sp->current_action = -1;
				sp->SetDirection(VirtualSprite::CalculateOppositeDirection(_camera->GetDirection()));
				_dialogue_manager->SetCurrentDialogue(sp->GetCurrentDialogue());
				sp->NextDialogue();
				_map_state = DIALOGUE;
				return;
			}
		}
		else if ( obj && obj->GetType() == CHEST_TYPE ) {
			ChestObject *chest = reinterpret_cast<ChestObject*>(obj);

			if( !chest->IsHidden() ) {
				chest->Use();
			}
		}


	}

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
				_camera->SetDirection(NORTHWEST);
			}
			else if (InputManager->RightState()) {
				_camera->SetDirection(NORTHEAST);
			}
			else {
				_camera->SetDirection(NORTH);
			}
		}
		else if (InputManager->DownState()) {
			if (InputManager->LeftState()) {
				_camera->SetDirection(SOUTHWEST);
			}
			else if (InputManager->RightState()) {
				_camera->SetDirection(SOUTHEAST);
			}
			else {
				_camera->SetDirection(SOUTH);
			}
		}
		else if (InputManager->LeftState()) {
			_camera->SetDirection(WEST);
		}
		else if (InputManager->RightState()) {
			_camera->SetDirection(EAST);
		}
	} // if (_camera->moving == true)
} // void MapMode::_HandleInputExplore()



MapObject* MapMode::_FindNearestObject(const VirtualSprite* sprite) {
	// The edges of the collision rectangle to check
	float top, bottom, left, right;

	// ---------- (1): Using the sprite's direction, determine the area to check for other objects
	if (sprite->direction & FACING_NORTH) {
		bottom = sprite->ComputeYLocation() - sprite->coll_height;
		top = bottom - 3.0f;
		left = sprite->ComputeXLocation() - sprite->coll_half_width;
		right = sprite->ComputeXLocation() + sprite->coll_half_width;
	}
	else if (sprite->direction & FACING_SOUTH) {
		top = sprite->ComputeYLocation();
		bottom = top + 3.0f;
		left = sprite->ComputeXLocation() - sprite->coll_half_width;
		right = sprite->ComputeXLocation() + sprite->coll_half_width;
	}
	else if (sprite->direction & FACING_WEST) {
		right = sprite->ComputeXLocation() - sprite->coll_half_width;
		left = right - 3.0f;
		bottom = sprite->ComputeYLocation();
		top = bottom - sprite->coll_height;
	}
	else if (sprite->direction & FACING_EAST) {
		left = sprite->ComputeXLocation() + sprite->coll_half_width;
		right = left + 3.0f;
		bottom = sprite->ComputeYLocation();
		top = bottom - sprite->coll_height;
	}
	else {
		if (MAP_DEBUG)
			cerr << "MAP ERROR: sprite was set to invalid direction in MapMode::_FindNearestObject()" << endl;
		return NULL;
	}

	// A vector to contain objects which are valid for the sprite to interact with
	vector<MapObject*> valid_objects;
	// A pointer to the object which has been found to be the closest to the sprite within valid_objs
	MapObject* closest = NULL;

	// ---------- (2): Go through all objects and determine which (if any) are valid
	for (map<uint16, MapObject*>::iterator i = _all_objects.begin(); i != _all_objects.end(); i++) {
		MapObject* obj = i->second;
		if( obj == sprite ) //A sprite can't target itself
			continue;

		// Objects in different contexts can not interact with one another
		if (obj->context & sprite->context == 0) // Since objects can span multiple context, we check that no contexts are equal
			continue;

		// Compute the full position coordinates for the object under study
		float other_x_location = obj->ComputeXLocation();
		float other_y_location = obj->ComputeYLocation();

		// Verify that the bounding boxes overlap on the horizontal axis
		if (!(other_x_location - obj->coll_half_width > right
			|| other_x_location + obj->coll_half_width < left)) {
			// Verify that the bounding boxes overlap on the vertical axis
			if (!(other_y_location - obj->coll_height > bottom
				|| other_y_location < top )) {
				// Boxes overlap on both axes, it is a valid interaction
				valid_objects.push_back(obj);
			}
		}
	} // for (map<MapObject*>::iterator i = _all_objects.begin(); i != _all_objects.end(); i++)

	// If there are one or less objects that are valid, then we are done here.
	if (valid_objects.empty()) {
		return NULL;
	}
	else if (valid_objects.size() == 1) {
		return valid_objects[0];
	}

	// ---------- (3): Figure out which of the valid objects is the closest to the sprite
	// NOTE: For simplicity, we simply find which object has the location coordinates which are
	// closest to the sprite's coordinates using the Manhattan distance.

	// Used to hold the full position coordinates of the sprite
	float source_x = sprite->ComputeXLocation();
	float source_y = sprite->ComputeYLocation();

	closest = valid_objects[0];
	float min_distance = fabs(source_x - closest->ComputeXLocation())
		+ fabs(source_y - closest->ComputeYLocation());

	// Determine which object's position is closest to the sprite
	for (uint32 i = 1; i < valid_objects.size(); i++) {
		float dist = fabs(source_x - valid_objects[i]->ComputeXLocation())
			+ fabs(source_y - valid_objects[i]->ComputeYLocation());
		if (dist < min_distance) {
			closest = valid_objects[i];
			min_distance = dist;
		}
	}
	return closest;
} // MapObject* MapMode::_FindNearestObject(VirtualSprite* sprite)



bool MapMode::_DetectCollision(VirtualSprite* sprite) {
	// NOTE: Whether the argument pointer is valid is not checked here, since the object pointer
	// itself presumably called this function.

	// The single X,Y floating point coordinates of the sprite
	float x_location = sprite->ComputeXLocation();
	float y_location = sprite->ComputeYLocation();

	// The coordinates corresponding to the four sides of the sprite's collision rectangle (cr)
	float cr_left = x_location - sprite->coll_half_width;
	float cr_right = x_location + sprite->coll_half_width;
	float cr_top = y_location - sprite->coll_height;
	// The bottom of the sprite's collision rectangle is its y_location

	// ---------- (1): Check if the sprite's position has gone out of bounds
	if (cr_left < 0.0f || cr_top < 0.0f || cr_right >= static_cast<float>(_num_grid_cols) ||
		y_location >= static_cast<float>(_num_grid_rows)) {
		return true;
	}

	// Do not do tile or object based collision detection for this sprite if it has this member set
	if (sprite->no_collision == true) {
		return false;
	}

	// A pointer to the layer of objects to do the collision detection with
	vector<MapObject*>* objects;

	if (sprite->sky_object == false) { // Do tile collision detection for ground objects only

		// ---------- (2): Determine if the sprite's collision rectangle overlaps any unwalkable tiles
		// NOTE: Because the sprite's collision rectangle was determined to be within the map bounds,
		// the map grid tile indeces referenced in this loop are all valid entries.
		for (uint32 r = static_cast<uint32>(cr_top); r <= static_cast<uint32>(y_location); r++) {
			for (uint32 c = static_cast<uint32>(cr_left); c <= static_cast<uint32>(cr_right); c++) {
				if (_map_grid[r][c] & sprite->context > 0) { // Then this overlapping tile is unwalkable
					return true;
				}
			}
		}
		objects = &_ground_objects;
	}
	else {
		objects = &_sky_objects;
	}

	// ---------- (3): Determine if two object's collision rectangles overlap
	for (uint32 i = 0; i < objects->size(); i++) {
		// Only verify this object if it is not the same object as the sprite
		if ((*objects)[i]->object_id != sprite->object_id
			&& !(*objects)[i]->no_collision
			&& (*objects)[i]->context & sprite->context > 0 )
		{
			// Compute the full position coordinates of the other object
			float other_x_location = (*objects)[i]->ComputeXLocation();
			float other_y_location = (*objects)[i]->ComputeYLocation();;

			// Verify that the bounding boxes overlap on the horizontal axis
			if (!(other_x_location - (*objects)[i]->coll_half_width > cr_right
				|| other_x_location + (*objects)[i]->coll_half_width < cr_left)) {
				// Verify that the bounding boxes overlap on the vertical axis
				if (!(other_y_location - (*objects)[i]->coll_height > y_location || other_y_location < cr_top )) {
					// Boxes overlap on both axis, there is a colision
					if (sprite->GetType() == ENEMY_TYPE && (*objects)[i] == _camera) {
						EnemySprite *enemy = reinterpret_cast<EnemySprite*>(sprite);
						if (enemy->IsHostile()) {
							enemy->ChangeStateDead();
							BattleMode *BM = new BattleMode();
							BM->AddMusic(enemy->GetBattleMusicTheme());
							ModeManager->Push(BM);
							const vector<uint32>& enemy_party = enemy->RetrieveRandomParty();
							for (uint32 i = 0; i < enemy_party.size(); i++) {
								BM->AddEnemy(enemy_party[i]);
							}
							return false;
						}
					}

					if ((*objects)[i]->GetType() == ENEMY_TYPE && sprite == _camera) {
						EnemySprite *enemy = reinterpret_cast<EnemySprite*>((*objects)[i]);
						if (enemy->IsHostile()) {
							enemy->ChangeStateDead();
							BattleMode *BM = new BattleMode();
							BM->AddMusic(enemy->GetBattleMusicTheme());
							ModeManager->Push(BM);
							const vector<uint32>& enemy_party = enemy->RetrieveRandomParty();
							for (uint32 i = 0; i < enemy_party.size(); i++) {
								BM->AddEnemy(enemy_party[i]);
							}
							return false;
						}
					}
					return true;
				}
			}
		}
	}

	// No collision was detected
	return false;
} // bool MapMode::_DetectCollision(VirtualSprite* sprite)



void MapMode::_FindPath(const VirtualSprite* sprite, std::vector<PathNode>& path, const PathNode& dest) {
	// NOTE: Refer to the implementation of the A* algorithm to understand what all these lists and scores are
	std::vector<PathNode> open_list;
	std::vector<PathNode> closed_list;

	// The starting node of this path discovery
	PathNode source_node(static_cast<int16>(sprite->y_position), static_cast<int16>(sprite->x_position));

	// The current "best node"
	PathNode best_node;
	// Used to hold the eight adjacent nodes
	PathNode nodes[8];

	// Temporary delta variables used in calculation of a node's heuristic (h score)
	uint32 x_delta, y_delta;
	// The number of grid elements that the sprite's collision rectange spreads outward and upward
	int16 x_span, y_span;
	// The number to add to a node's g_score, depending on whether it is a lateral or diagonal movement
	int16 g_add;

	path.clear();
	x_span = static_cast<int16>(sprite->coll_half_width);
	y_span = static_cast<int16>(sprite->coll_height);

	// Check that the source node is not the same as the destination node
	if (source_node == dest) {
		if (MAP_DEBUG)
			cerr << "MAP ERROR: source node is same as destination in MapMode::_FindPath()" << endl;
		return;
	}

	// Check that the destination is valid for the sprite to move to
	if ((dest.col - x_span < 0) || (dest.row - y_span < 0) ||
		(dest.col + x_span >= (_num_grid_cols)) || (dest.row >= (_num_grid_rows))) {
		if (MAP_DEBUG)
			cerr << "MAP ERROR: sprite can not move to destination node on path because it exceeds map boundaries" << endl;
		return;
	}
	for (int16 r = dest.row - y_span; r < dest.row; r++) {
		for (int16 c = dest.col - x_span; c < dest.col + x_span; c++) {
			if (_map_grid[r][c] & sprite->context > 0) {
				if (MAP_DEBUG)
					cerr << "MAP ERROR: sprite can not move to destination node on path because one or more grid tiles are unwalkable" << endl;
				return;
			}
		}
	}

	open_list.push_back(source_node);

	while (!open_list.empty()) {
// 		sort(open_list.begin(), open_list.end(), PathNode::NodePred());
		sort(open_list.begin(), open_list.end());
		best_node = open_list.back();
		open_list.pop_back();
		closed_list.push_back(best_node);

		// Check if destination has been reached, and break out of the loop if so
		if (best_node == dest) {
			break;
		}

		// Setup the coordinates of the 8 adjacent nodes to the best node
		nodes[0].row = closed_list.back().row - 1; nodes[0].col = closed_list.back().col;
		nodes[1].row = closed_list.back().row + 1; nodes[1].col = closed_list.back().col;
		nodes[2].row = closed_list.back().row;     nodes[2].col = closed_list.back().col - 1;
		nodes[3].row = closed_list.back().row;     nodes[3].col = closed_list.back().col + 1;
		nodes[4].row = closed_list.back().row - 1; nodes[4].col = closed_list.back().col - 1;
		nodes[5].row = closed_list.back().row - 1; nodes[5].col = closed_list.back().col + 1;
		nodes[6].row = closed_list.back().row + 1; nodes[6].col = closed_list.back().col - 1;
		nodes[7].row = closed_list.back().row + 1; nodes[7].col = closed_list.back().col + 1;

		// Check the eight adjacent nodes
		for (uint8 i = 0; i < 8; ++i) {
			// ---------- (A): Check that the sprite's collision rectangle will be within the map boundaries
			if ((nodes[i].col - x_span < 0) || (nodes[i].row - y_span < 0) ||
				(nodes[i].col + x_span >= _num_grid_cols) || (nodes[i].row >= _num_grid_rows)) {
				continue;
			}

			// ---------- (B): Check that all grid nodes that the sprite's collision rectangle will overlap are walkable
			bool continue_loop = true;
			for (int16 r = nodes[i].row - y_span; r < nodes[i].row && continue_loop; r++) {
				for (int16 c = nodes[i].col - x_span; c < nodes[i].col + x_span; c++) {
					if (_map_grid[r][c] & sprite->context > 0) {
						continue_loop = false;
						break;
					}
				}
			}
			if (continue_loop == false) { // This node is invalid
				continue;
			}

			// ---------- (C): Check if the Node is already in the closed list
			if (find(closed_list.begin(), closed_list.end(), nodes[i]) != closed_list.end()) {
				continue;
			}

			// ---------- (D): If this point has been reached, the node is valid for the sprite to move to

			// If this is a lateral adjacent node, g_score is +10, otherwise diagonal adjacent node is +14
			if (i < 4)
				g_add = 10;
			else
				g_add = 14;

			// Set the node's parent and calculate its g_score
			nodes[i].parent_row = best_node.row;
			nodes[i].parent_col = best_node.col;
			nodes[i].g_score = best_node.g_score + g_add;

			// ---------- (E): Check to see if the node is already on the open list and update it if necessary
			vector<PathNode>::iterator iter = find(open_list.begin(), open_list.end(), nodes[i]);
			if (iter != open_list.end()) {
				// If its G is higher, it means that the path we are on is better, so switch the parent
				if (iter->g_score > nodes[i].g_score) {
					iter->g_score = nodes[i].g_score;
					iter->f_score = nodes[i].g_score + iter->h_score;
					iter->parent_row = nodes[i].parent_row;
					iter->parent_col = nodes[i].parent_col;
				}
			}
			// ---------- (F): Add the new node to the open list
			else {
				// Calculate the H and F score of the new node (the heuristic used is diagonal)
				x_delta = abs(dest.col - nodes[i].col);
				y_delta = abs(dest.row - nodes[i].row);
				if (x_delta > y_delta)
					nodes[i].h_score = 14 * y_delta + 10 * (x_delta - y_delta);
				else
					nodes[i].h_score = 14 * x_delta + 10 * (y_delta - x_delta);

				nodes[i].f_score = nodes[i].g_score + nodes[i].h_score;
				open_list.push_back(nodes[i]);
			}
		} // for (uint8 i = 0; i < 8; ++i)
	} // while (!open_list.empty())

	if (open_list.empty()) {
		if (MAP_DEBUG)
			cerr << "MAP ERROR: could not find path to destination" << endl;
		path.push_back( source_node );
		return;
	}

	// Add the destination node to the vector, retain its parent, and remove it from the closed list
	path.push_back(closed_list.back());
	int16 parent_row = closed_list.back().parent_row;
	int16 parent_col = closed_list.back().parent_col;
	//PathNode* parent = closed_list.back().parent;
	closed_list.pop_back();

	// Go backwards through the closed list to construct the path
	for (vector<PathNode>::iterator iter = closed_list.end() - 1; iter != closed_list.begin(); --iter) {
		if ( iter->col == parent_col && iter->row == parent_row ) { // Find the parent of the node, add it to the path, and then set a new parent node
			path.push_back(*iter);
			parent_col = iter->parent_col;
			parent_row = iter->parent_row;
		}
	}
	std::reverse( path.begin(), path.end() );
} // void MapMode::_FindPath(const VirtualSprite* sprite, std::vector<PathNode>& path, const PathNode& dest)

// ****************************************************************************
// **************************** DRAW FUNCTIONS ********************************
// ****************************************************************************

#define __MAP_CHANGE_1__
#define __MAP_CHANGE_2__


// Determines things like our starting tiles
void MapMode::_CalculateDrawInfo() {
	// TRYING TO GET RID OF PROBLEMS OF DUPLICATED LINES IN MAP
	// THIS CODE IS TEMPORAL AND NOT COMPLETELY WORKING

#ifdef __MAP_CHANGE_1__
	static float x (_draw_info.tile_x_start);
	static float y (_draw_info.tile_y_start);

//	if (VideoManager->GetWidth() == 1024 && VideoManager->GetHeight() == 768)
	{
		_draw_info.tile_x_start = x;
		_draw_info.tile_y_start = y;
	}
#endif

	// ---------- (1) Set the default starting draw positions for the tiles (top left tile)

	// The camera's position is in terms of the 16x16 grid, which needs to be converted into 32x32 coordinates.
	float camera_x = _camera->ComputeXLocation();
	float camera_y = _camera->ComputeYLocation();

	// Determine the draw coordinates of the top left corner using the camera's current position
	_draw_info.tile_x_start = 1.0f - _camera->x_offset;
	if (IsOddNumber(_camera->x_position))
		_draw_info.tile_x_start -= 1.0f;

	_draw_info.tile_y_start = 2.0f - _camera->y_offset;
	if (IsOddNumber(_camera->y_position))
		_draw_info.tile_y_start -= 1.0f;

	// By default the map draws 32 + 1 columns and 24 + 1 rows of tiles, the maximum that can fit on the screen.
	_draw_info.num_draw_cols = TILE_COLS + 1;
	_draw_info.num_draw_rows = TILE_ROWS + 1;

	// The default starting tile row and column is relative to the map camera's current position.
	_draw_info.starting_col = (_camera->x_position / 2) - HALF_TILE_COLS;
	_draw_info.starting_row = (_camera->y_position / 2) - HALF_TILE_ROWS;

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
		_draw_info.right_edge = static_cast<float>(_num_grid_cols);
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
		_draw_info.bottom_edge = static_cast<float>(_num_grid_rows);
		_draw_info.top_edge = _draw_info.bottom_edge - SCREEN_ROWS;
	}

	// Check for the conditions where the tile images align perfectly with the screen and one less row or column of tiles is drawn
	if (IsFloatInRange(_draw_info.tile_x_start, 0.999f, 1.001f)) { // Is the value approximately equal to 1.0f?
		_draw_info.num_draw_cols--;
	}
	if (IsFloatInRange(_draw_info.tile_y_start, 1.999f, 2.001f)) { // Is the value approximately equal to 2.0f?
		_draw_info.num_draw_rows--;
	}

	// TRYING TO GET RID OF PROBLEMS OF DUPLICATED LINES IN MAP
	// THIS CODE IS TEMPORAL AND NOT COMPLETELY WORKING
#ifdef __MAP_CHANGE_1__
	double y_resolution;
	double x_resolution;

	float x2 (_draw_info.tile_x_start);
	float y2 (_draw_info.tile_y_start);

//	if (VideoManager->GetWidth() == 1024 && VideoManager->GetHeight() == 768)
	{
		VideoManager->GetPixelSize(x_resolution, y_resolution);
		x_resolution = abs(x_resolution);
		y_resolution = abs(y_resolution);

		_draw_info.tile_x_start = FloorToFloatMultiple (_draw_info.tile_x_start, x_resolution);
		_draw_info.tile_y_start = FloorToFloatMultiple (_draw_info.tile_y_start, y_resolution);

		if (x2 - _draw_info.tile_x_start > x_resolution*0.5f)
			_draw_info.tile_x_start += x_resolution;
		if (y2 - _draw_info.tile_y_start > y_resolution*0.5f)
			_draw_info.tile_y_start += y_resolution;
	}
#endif

#if defined(__MAP_CHANGE_1__) && defined(__MAP_CHANGE_2__)
//	if (VideoManager->GetWidth() == 1024 && VideoManager->GetHeight() == 768)
	{
		_draw_info.left_edge = FloorToFloatMultiple (_draw_info.left_edge, x_resolution);
		_draw_info.top_edge = FloorToFloatMultiple (_draw_info.top_edge, y_resolution);

		if (camera_x - HALF_SCREEN_COLS - _draw_info.left_edge > x_resolution*0.5f)
			_draw_info.left_edge += x_resolution;
		if (camera_y - HALF_SCREEN_ROWS - _draw_info.top_edge > y_resolution*0.5f)
			_draw_info.top_edge += y_resolution;

		_draw_info.right_edge = _draw_info.left_edge + 2*SCREEN_COLS;
		_draw_info.bottom_edge = _draw_info.top_edge + 2*SCREEN_ROWS;
	}
#endif



	// Comment this out to print out debugging info about each map frame that is drawn
// 	printf("--- MAP DRAW INFO ---\n");
// 	printf("Starting row, col: [%d, %d]\n", _draw_info.starting_row, _draw_info.starting_col);
// 	printf("# draw rows, cols: [%d, %d]\n", _draw_info.num_draw_rows, _draw_info.num_draw_cols);
// 	printf("Camera position:   [%f, %f]\n", camera_x, camera_y);
// 	printf("Tile draw start:   [%f, %f]\n", _draw_info.tile_x_start, _draw_info.tile_y_start);
// 	printf("Edges (T,D,L,R):   [%f, %f, %f, %f]\n", _draw_info.top_edge, _draw_info.bottom_edge, _draw_info.left_edge, _draw_info.right_edge);
} // void MapMode::_CalculateDrawInfo()


// Public draw function called by the main game loop
void MapMode::Draw() {
	_CalculateDrawInfo();
	ScriptCallFunction<void>(_draw_function);
	_DrawGUI();
	if (_map_state == DIALOGUE) {
		_dialogue_manager->Draw();
	}
} // void MapMode::_Draw()



void MapMode::_DrawMapLayers() {
	// ---------- (1) Draw the lower tile layer
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

	// ---------- (2) Draw the middle tile layer
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

		VideoManager->MoveRelative(-static_cast<float>(_draw_info.num_draw_cols * 2), 2.0f);
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
} // void MapMode::_DrawMapLayers()



void MapMode::_DrawGUI() {
	// ---------- (1) Draw the introductory location name and graphic if necessary
	if (_intro_timer.IsFinished() == false) {
		uint32 time = _intro_timer.GetTimeExpired();
		Color blend(1.0f, 1.0f, 1.0f, 1.0f);
		if (time < 2000) { // Fade in
			blend.SetAlpha((static_cast<float>(time) / 2000.0f));
		}
		else if (time > 5000) { // Fade out
			blend.SetAlpha(1.0f - static_cast<float>(time - 5000) / 2000.0f);
		}
		VideoManager->PushState();
		VideoManager->SetCoordSys(0.0f, 1024.0f, 768.0f, 0.0f);
		VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
		VideoManager->Move(512.0f, 100.0f);
		VideoManager->DrawImage(_location_graphic, blend);
		VideoManager->MoveRelative(0.0f, -80.0f);
		VideoManager->SetTextColor(blend);
		VideoManager->DrawText(_map_name);
		VideoManager->PopState();
	}

	// ---------- (2) Draw the run stamina bar in the lower right
	float fill_size = static_cast<float>(_run_stamina) / 10000.0f;

	VideoManager->PushState();
	VideoManager->SetCoordSys(0.0f, 1024.0f, 768.0f, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_NO_BLEND, 0);
	VideoManager->Move(800, 740);

	// TEMP: draw black background rectangle; to be replaced with an image
	VideoManager->DrawRectangle(200, 10, Color::black);
	VideoManager->DrawRectangle(200 * fill_size, 10, Color(0.133f, 0.455f, 0.133f, 1.0f));
	VideoManager->PopState();
} // void MapMode::_DrawGUI()


// ****************************************************************************
// ************************* LUA BINDING FUNCTIONS ****************************
// ****************************************************************************

void MapMode::_AddGroundObject(private_map::MapObject *obj) {
	_ground_objects.push_back(obj);
	_all_objects.insert(make_pair(obj->object_id, obj));
}



void MapMode::_AddPassObject(private_map::MapObject *obj) {
	_pass_objects.push_back(obj);
	_all_objects.insert(make_pair(obj->object_id, obj));
}



void MapMode::_AddSkyObject(private_map::MapObject *obj) {
	_sky_objects.push_back(obj);
	_all_objects.insert(make_pair(obj->object_id, obj));
}

void MapMode::_AddZone(private_map::MapZone *zone) {
	_zones.push_back(zone);
}

} // namespace hoa_map
