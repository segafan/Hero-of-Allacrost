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

#include "audio.h"
#include "script.h"
#include "input.h"
#include "system.h"

#include "global.h"

#include "map.h"
#include "map_dialogue.h"
#include "map_objects.h"
#include "map_sprites.h"
#include "map_tiles.h"
#include "map_zones.h"
#include "menu.h"
#include "pause.h"

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
using namespace hoa_menu;
using namespace hoa_pause;

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
	_num_map_contexts(0),
	_ignore_input(false),
	_run_forever(false),
	_run_disabled(false),
	_run_stamina(10000)
{
	_loading_map = this;

	// Modify the filename so that is only consists of alphanumeric characters and underscores, and thus will be a valid identifier name in Lua
	string event_group_name = _map_filename;
	std::replace(event_group_name.begin(), event_group_name.end(), '/', '_');
	std::replace(event_group_name.begin(), event_group_name.end(), '.', '_');

	if (GlobalManager->DoesEventGroupExist(event_group_name) == false) {
		GlobalManager->AddNewEventGroup(event_group_name);
	}
	_map_event_group = GlobalManager->GetEventGroup(event_group_name);

	mode_type = MODE_MANAGER_MAP_MODE;

	_tile_manager = new TileManager();
	_object_manager = new ObjectManager();
	_dialogue_manager = new DialogueManager();
	_treasure_menu = new TreasureMenu();

	_intro_timer.Initialize(7000, 0, this);

	// TODO: Load the map data in a seperate thread
	_Load();

	// TEMP: Load dialogue icon
	_new_dialogue_icon.SetDimensions(2, 2);
	vector<uint32> timings(16, 100);

	if (_new_dialogue_icon.LoadFromFrameSize("img/misc/dialogue_icon.png", timings, 32, 32) == false)
		IF_PRINT_WARNING(MAP_DEBUG) << "new dialogue icon load failure" << endl;

	if (_stamina_bar_background.Load("img/misc/stamina_bar_background.png", 227, 24) == false)
		IF_PRINT_WARNING(MAP_DEBUG) << "run-stamina bar background image load failure" << endl;
		
	if (_stamina_bar_infinite_overlay.Load("img/misc/stamina_bar_infinite_overlay.png", 227, 24) == false)
		IF_PRINT_WARNING(MAP_DEBUG) << "run-stamina bar infinity image load failure" << endl;
}



MapMode::~MapMode() {
	for (uint32 i = 0; i < _music.size(); i++) {
		_music[i].FreeAudio();
	}

	for (uint32 i = 0; i < _sounds.size(); i++) {
		_sounds[i].FreeAudio();
	}

	// Delete all enemy's created
	for (uint32 i = 0; i < _enemies.size(); i++) {
		delete(_enemies[i]);
	}

	delete(_tile_manager);
	delete(_object_manager);
	delete(_dialogue_manager);
	delete(_treasure_menu);

	// free dialogue icon
	_new_dialogue_icon.Clear();

	_map_script.CloseFile();
}


// Resets appropriate class members.
void MapMode::Reset() {
	// Reset active video engine properties
	VideoManager->SetCoordSys(0.0f, SCREEN_COLS, SCREEN_ROWS, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, 0);

	// Let all map objects know that this is the current map
	MapMode::_current_map = this;

	// ------------ (8) Set values in the global manager so when the game is saved it has necessary information
	GlobalManager->SetLocation(MakeUnicodeString(_map_filename), _location_graphic.GetFilename());

	// TEMP: This will need to be scripted later
	if (_music.size() > 0 && _music.back().GetState() != AUDIO_STATE_PLAYING) {
		_music.back().Play();
	}

	_intro_timer.Run();
}


// Loads the map from a Lua file.
void MapMode::_Load() {
	// ---------- (1) Create a new GlobalEvent group for this map, if one does not already exist
	string group_name = _map_filename;

	// ---------- (2) Open map script file and read in basic map properties and tile definitions
	if (_map_script.OpenFile(_map_filename) == false) {
		return;
	}

	// Open the map tablespace (named after the map filename)
	// first see if the filename has an extension, and then strip the directories
	int32 period = _map_filename.find(".");
	int32 last_slash = _map_filename.find_last_of("/");
	_map_tablespace = _map_filename.substr(last_slash + 1, period - (last_slash + 1));
	_map_script.OpenTable(_map_tablespace);

	_map_name = MakeUnicodeString(_map_script.ReadString("map_name"));
	if (_location_graphic.Load("img/menus/locations/" + _map_script.ReadString("location_filename")) == false) {
		cerr << "MAP ERROR: failed to load location graphic image: " << _location_graphic.GetFilename() << endl;
	}

	_num_map_contexts = _map_script.ReadUInt("num_map_contexts");

	// ---------- (3) Initialize all of the tile and grid mappings
	_tile_manager->Load(_map_script, this);
	_object_manager->Load(_map_script);

	// ---------- (4) Load map sounds and music
	vector<string> sound_filenames;
	_map_script.ReadStringVector("sound_filenames", sound_filenames);

	for (uint32 i = 0; i < sound_filenames.size(); i++) {
		_sounds.push_back(SoundDescriptor());
		if (_sounds.back().LoadAudio(sound_filenames[i]) == false) {
			cerr << "MAP ERROR: failed to load map sound: " << sound_filenames[i] << endl;
			return;
		}
	}

	vector<string> music_filenames;
	_map_script.ReadStringVector("music_filenames", music_filenames);
	for (uint32 i = 0; i < music_filenames.size(); i++) {
		_music.push_back(MusicDescriptor());
		if (_music.back().LoadAudio(music_filenames[i]) == false) {
			cerr << "MAP ERROR: failed to load map music: " << music_filenames[i] << endl;
			return;
		}
	}

	// ---------- (5) Construct all enemies that may appear on this map
	vector<int32> enemy_ids;
	_map_script.ReadIntVector("enemy_ids", enemy_ids);
	for (uint32 i = 0; i < enemy_ids.size(); i++) {
		_enemies.push_back(new GlobalEnemy(enemy_ids[i]));
	}

	// ---------- (6) Call the map script's load function
	ScriptObject map_table(luabind::from_stack(_map_script.GetLuaState(), hoa_script::private_script::STACK_TOP));
	ScriptObject function = map_table["Load"];
	ScriptCallFunction<void>(function, this);

	_update_function = _map_script.ReadFunctionPointer("Update");
	_draw_function = _map_script.ReadFunctionPointer("Draw");

	_map_script.CloseAllTables();
} // void MapMode::_Load()

// ****************************************************************************
// **************************** UPDATE FUNCTIONS ******************************
// ****************************************************************************

// Updates the game state when in map mode. Called from the main game loop.
void MapMode::Update() {
	if (InputManager->QuitPress() == true) {
		ModeManager->Push(new PauseMode(true));
		return;
	}
	else if (InputManager->PausePress() == true) {
		ModeManager->Push(new PauseMode(false));
		return;
	}

	_time_elapsed = SystemManager->GetUpdateTime();

	// ---------- (1) Call the map's update script function
	ScriptCallFunction<void>(_update_function);

	// ---------- (2) Process user input
	if (_ignore_input == false) {
		if (_map_state == DIALOGUE)
			_dialogue_manager->Update();
		else if (_treasure_menu->IsActive() == true)
			_treasure_menu->Update();
		else
			_HandleInputExplore();
	}

	// ---------- (3) Update all animated tile images
	_tile_manager->Update();

	// ---------- (4) Update all zones and objects on the map
	if (_treasure_menu->IsActive() == false) {
		_object_manager->Update();
	}
	_object_manager->SortObjects();
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

	// Allow the player to run if they have enough stamina, and update the stamina amount
	_camera->is_running = false;
	if (InputManager->CancelState() == true && _run_disabled == false) {
		if (_run_forever) {
			_camera->is_running = true;
		}
		else if (_run_stamina > _time_elapsed * 2) {
			_run_stamina -= (_time_elapsed * 2);
			_camera->is_running = true;
		}
		else {
			_run_stamina = 0;
		}
	}
	// Regenerate the stamina at 1/2 the consumption rate
	else if (_run_stamina < 10000) {
		_run_stamina += _time_elapsed;
		if (_run_stamina > 10000)
			_run_stamina = 10000;
	}

	if (InputManager->ConfirmPress()) {
		MapObject* obj = _object_manager->FindNearestObject(_camera);
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
		else if (obj && obj->GetType() == TREASURE_TYPE) {
			MapTreasure* chest = reinterpret_cast<MapTreasure*>(obj);

			if (chest->IsEmpty() == false) {
				chest->Open();
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
	else if (_draw_info.starting_col + TILE_COLS >= _tile_manager->_num_tile_cols) {
		_draw_info.starting_col = static_cast<int16>(_tile_manager->_num_tile_cols - TILE_COLS);
		_draw_info.tile_x_start = 1.0f;
		_draw_info.right_edge = static_cast<float>(_object_manager->_num_grid_cols);
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
	else if (_draw_info.starting_row + TILE_ROWS >= _tile_manager->_num_tile_rows) {
		_draw_info.starting_row = static_cast<int16>(_tile_manager->_num_tile_rows - TILE_ROWS);
		_draw_info.tile_y_start = 2.0f;
		_draw_info.bottom_edge = static_cast<float>(_object_manager->_num_grid_rows);
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
	float y_resolution;
	float x_resolution;

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
	VideoManager->SetCoordSys(0.0f, SCREEN_COLS, SCREEN_ROWS, 0.0f);

	// ---------- (1) Draw the lower tile layer
	_tile_manager->DrawLowerLayer(&_draw_info);
	// ---------- (2) Draw the middle tile layer
	_tile_manager->DrawMiddleLayer(&_draw_info);
	// ---------- (3) Draw the ground object layer (first pass)
	_object_manager->DrawGroundObjects(&_draw_info, false);
	// ---------- (4) Draw the pass object layer
	_object_manager->DrawPassObjects(&_draw_info);
	// ---------- (5) Draw the ground object layer (second pass)
	_object_manager->DrawGroundObjects(&_draw_info, true);
	// ---------- (6) Draw the upper tile layer
	_tile_manager->DrawUpperLayer(&_draw_info);
	// ---------- (7) Draw the sky object layer
	_object_manager->DrawSkyObjects(&_draw_info);
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
		_location_graphic.Draw(blend);
		VideoManager->MoveRelative(0.0f, -80.0f);
		VideoManager->Text()->Draw(_map_name, TextStyle("map", blend, VIDEO_TEXT_SHADOW_DARK));
		VideoManager->PopState();
	}

	// ---------- (2) Draw the run stamina bar in the lower right
	float fill_size = static_cast<float>(_run_stamina) / 10000.0f;

	VideoManager->PushState();
	VideoManager->SetCoordSys(0.0f, 1024.0f, 768.0f, 0.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);

	//draw the background image
	VideoManager->Move(780, 747);
	_stamina_bar_background.Draw();
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_NO_BLEND, 0);

	VideoManager->Move(800, 740);
	//VideoManager->DrawRectangle(200, 10, Color::black);
	//VideoManager->DrawRectangle(200 * fill_size, 10, Color(0.133f, 0.455f, 0.133f, 1.0f));
	VideoManager->DrawRectangle(200 * fill_size, 10, Color(0.0196f, 0.207f, 0.0196f, 1.0f));
	
	//code to shade the bar with a faux lighting effect
	VideoManager->Move(800,739); // dark green
	VideoManager->DrawRectangle(200 * fill_size, 2, Color(0.274f, 0.298f, 0.274f, 1.0f));
	VideoManager->Move(800, 737); //darkish green
	VideoManager->DrawRectangle(200 * fill_size, 7, Color(0.352f, 0.4f, 0.352f, 1.0f));

	if ((200 * fill_size) >= 4){  //Only do this if the bar is at least 4 pixels long
	VideoManager->Move(801, 739); //darkish green
	VideoManager->DrawRectangle((200 * fill_size) -2, 1, Color(0.352f, 0.4f, 0.352f, 1.0f));	
	}
	
	if ((200 * fill_size) >= 4){  //Only do this if the bar is at least 4 pixels long
	VideoManager->Move(801, 738); //medium green
	VideoManager->DrawRectangle(1, 2, Color(0.0509f, 0.556f, 0.0509f, 1.0f));
	
	VideoManager->Move(800 + (fill_size * 200 - 2), 738); //medium green //automatically reposition to be at moving endcap
	VideoManager->DrawRectangle(1, 2, Color(0.0509f, 0.556f, 0.0509f, 1.0f));
	}
	
	VideoManager->Move(800, 736); //medium green
	VideoManager->DrawRectangle(200 * fill_size, 5, Color(0.0509f, 0.556f, 0.0509f, 1.0f));
	
	
	
	if ((200 * fill_size) >= 4){  //Only do this if the bar is at least 4 pixels long
	VideoManager->Move(801, 735); //light green
	VideoManager->DrawRectangle(1, 1, Color(0.419f, 0.894f, 0.0f, 1.0f));
	
	VideoManager->Move(800 + (fill_size * 200 - 2), 735); //light green //automatically reposition to be at moving endcap
	VideoManager->DrawRectangle(1, 1, Color(0.419f, 0.894f, 0.0f, 1.0f));
	
	VideoManager->Move(800, 734); //light green
	VideoManager->DrawRectangle(200 * fill_size, 2, Color(0.419f, 0.894f, 0.0f, 1.0f));
	}



	if ((200 * fill_size) >= 6){  //Only do this if the bar is at least 4 pixels long
	VideoManager->Move(802, 733); //bright yellow highlight
	VideoManager->DrawRectangle((200 * fill_size) - 4, 1, Color(0.937f, 1.0f, 0.725f, 1.0f));
	}



	if (_run_forever){ //then display this fact to the player
	//first change the video mode so we can do alpha channels
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);

	//then draw the infinity symbol
	VideoManager->Move(780, 747);
	_stamina_bar_infinite_overlay.Draw();
	}
	
	VideoManager->PopState();

	// ---------- (3) Draw the treasure menu
	if (_treasure_menu->IsActive() == true)
		_treasure_menu->Draw();
} // void MapMode::_DrawGUI()


// ****************************************************************************
// ************************* LUA BINDING FUNCTIONS ****************************
// ****************************************************************************

void MapMode::_AddGroundObject(MapObject *obj) {
	_object_manager->_ground_objects.push_back(obj);
	_object_manager->_all_objects.insert(make_pair(obj->object_id, obj));
}



void MapMode::_AddPassObject(MapObject *obj) {
	_object_manager->_pass_objects.push_back(obj);
	_object_manager->_all_objects.insert(make_pair(obj->object_id, obj));
}



void MapMode::_AddSkyObject(MapObject *obj) {
	_object_manager->_sky_objects.push_back(obj);
	_object_manager->_all_objects.insert(make_pair(obj->object_id, obj));
}



void MapMode::_AddZone(MapZone *zone) {
	_object_manager->_zones.push_back(zone);
}



void MapMode::_SetCameraFocus(VirtualSprite *sprite) {
	_camera = sprite;
}



VirtualSprite* MapMode::_GetCameraFocus() const {
	return _camera;
}


uint16 MapMode::_GetGeneratedObjectID() {
	return ++(_object_manager->_lastID);
}

} // namespace hoa_map
