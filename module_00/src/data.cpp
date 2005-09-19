///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    data.cpp
 * \author  Vladimir Mitrovic, snipe714@allacrost.org
 * \date    Last Updated: August 25th, 2005
 * \brief   Source file for data and scripting engine.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include "data.h"
#include "audio.h"
#include "video.h"
#include "engine.h"
#include "boot.h"
#include "map.h"


using namespace std;
using namespace hoa_engine;
using namespace hoa_video;
using namespace hoa_audio;
using namespace hoa_utils;
using namespace hoa_map;

namespace hoa_data {

bool DATA_DEBUG = false;
SINGLETON_INITIALIZE(GameData);

// The constructor opens lua and its associated libraries.
GameData::GameData() {
	if (DATA_DEBUG) cout << "DATA: GameData constructor invoked." << endl;
	_l_stack = lua_open();

	// Load the Lua libraries
	const luaL_reg *lib = LUALIBS;
	for (; lib->func; lib++) {
		lib->func(_l_stack);      // open library
		lua_settop(_l_stack, 0);  // Clear the stack
	}
}


// Close Lua upon destruction
GameData::~GameData() {
	if (DATA_DEBUG) cout << "DATA: GameData destructor invoked." << endl;
	lua_close(_l_stack);
}


// Initialize the singleton pointers
bool GameData::Initialize() {
	_AudioManager = GameAudio::GetReference();
	_VideoManager = GameVideo::GetReference();
	_SettingsManager = GameSettings::GetReference();
	return true;
}

void GameData::OpenLuaFile(const char* file_name) {
	if (luaL_loadfile(_l_stack, file_name) || lua_pcall(_l_stack, 0, 0, 0))
		cerr << "DATA ERROR: Could not load " << file_name << " :: " <<
		        lua_tostring(_l_stack, -1) << endl;
}

void GameData::OpenTable(const char* tbl_name) {
	lua_getglobal(_l_stack, tbl_name);
	if (!lua_istable(_l_stack, LUA_STACK_TOP))
		cerr << "DATA ERROR: could not retrieve table \"" << tbl_name << "\"" << endl;
}

void GameData::CloseTable() {
	lua_pop(_l_stack, 1);
}

// Get an individual boolean field from a table, assumed to be on top of the stack
void GameData::GetTableBool (const char *key, bool &ref) {
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isboolean(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	ref = (0 != lua_toboolean(_l_stack, -1));
	lua_pop(_l_stack, 1);
}

// Get an individual integer field from a table, assumed to be on top of the stack
void GameData::GetTableInt (const char *key, int32 &ref) {
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isnumber(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	ref = (int32)lua_tonumber(_l_stack, -1);
	lua_pop(_l_stack, 1);
}

// Get an individual floating point field from a table, assumed to be on top of the stack
void GameData::GetTableFloat (const char *key, float &ref) {
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isnumber(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	ref = (float)lua_tonumber(_l_stack, -1);
	lua_pop(_l_stack, 1);
}

// Get an individual string field from a table, assumed to be on top of the stack
void GameData::GetTableString (const char *key, string &ref) {
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isstring(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	ref = (string)lua_tostring(_l_stack, -1);
	lua_pop(_l_stack, 1);
}



void GameData::GetGlobalBool(const char *key, bool &ref) {
	lua_getglobal(_l_stack, key);
	ref = (0 != lua_toboolean(_l_stack, LUA_STACK_TOP));
	lua_pop(_l_stack, 1);
}

void GameData::GetGlobalInt(const char *key, int32 &ref) {
	lua_getglobal(_l_stack, key);
	ref = (int32)lua_tonumber(_l_stack, LUA_STACK_TOP);
	lua_pop(_l_stack, 1);
}

void GameData::GetGlobalFloat(const char *key, float &ref) {
	lua_getglobal(_l_stack, key);
	ref = (float)lua_tonumber(_l_stack, LUA_STACK_TOP);
	lua_pop(_l_stack, 1);
}

void GameData::GetGlobalString(const char * key, string &ref) {
	lua_getglobal(_l_stack, key);
	ref = (string)lua_tostring(_l_stack, LUA_STACK_TOP);
	lua_pop(_l_stack, 1);
}

// TODO L8R: Maybe turn this function into FillVector(vector*, const char*, int lua_type),
//           which will fill a custom vector?
void GameData::FillStringVector(const char *key, vector<string> &vect) {
	lua_getglobal(_l_stack, key);
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: table " << key << " does not exist, or " << key << "isn't a table\n";
		return;
	}
	int32 t = lua_gettop(_l_stack);
	lua_pushnil(_l_stack);
	while (lua_next(_l_stack, t)) {
		vect.push_back((string)lua_tostring(_l_stack, LUA_STACK_TOP));
		lua_pop(_l_stack, 1);
	}
}

void GameData::FillIntVector(const char *key, std::vector<int32> &vect) {
	lua_getglobal(_l_stack, key);
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: table " << key << " does not exist, or " << key << "isn't a table\n";
		return;
	}
	int32 t = lua_gettop(_l_stack);
	lua_pushnil(_l_stack);
	while (lua_next(_l_stack, t)) {
		vect.push_back((int32)lua_tonumber(_l_stack, LUA_STACK_TOP));
		lua_pop(_l_stack, 1);
	}
}


// This function initializes all the members of the GameSettings singleton class
void GameData::LoadGameSettings () {
	OpenLuaFile("dat/config/settings.hoa");
	
	OpenTable("video_settings");
	//   _SettingsManager->screen_resx = GetTableInt("screen_resx");
	//   _SettingsManager->screen_resy = GetTableInt("screen_resy");
	bool full_screen;
	GetTableBool("full_screen", full_screen);
	_SettingsManager->SetFullScreen(full_screen);
	CloseTable();

	OpenTable("audio_settings");
	int32 vol;
	GetTableInt("music_vol", vol);
	_SettingsManager->music_vol = vol;
	GetTableInt("sound_vol", vol);
	_SettingsManager->sound_vol = vol;
	CloseTable();
}

void GameData::LoadKeyJoyState(KeyState *keystate, JoystickState *joystate) {
	OpenLuaFile("dat/config/settings.hoa");
	int32 tempy;
	OpenTable("key_settings");
	GetTableInt("up", tempy);           keystate->_up = (SDLKey)tempy;
	GetTableInt("down", tempy);         keystate->_down = (SDLKey)tempy;
	GetTableInt("left", tempy);         keystate->_left = (SDLKey)tempy;
	GetTableInt("right", tempy);        keystate->_right = (SDLKey)tempy;
	GetTableInt("confirm", tempy);      keystate->_confirm = (SDLKey)tempy;
	GetTableInt("cancel", tempy);       keystate->_cancel = (SDLKey)tempy;
	GetTableInt("menu", tempy);         keystate->_menu = (SDLKey)tempy;
	GetTableInt("swap", tempy);         keystate->_swap = (SDLKey)tempy;
	GetTableInt("left_select", tempy);  keystate->_left_select = (SDLKey)tempy;
	GetTableInt("right_select", tempy); keystate->_right_select = (SDLKey)tempy;
	GetTableInt("pause", tempy);        keystate->_pause = (SDLKey)tempy;
	CloseTable();
	
	OpenTable("joystick_settings");
	GetTableInt("index", tempy);        joystate->_joy_index = (int32)tempy;
	GetTableInt("confirm", tempy);      joystate->_confirm = (uint8)tempy;
	GetTableInt("cancel", tempy);       joystate->_cancel = (uint8)tempy;
	GetTableInt("menu", tempy);         joystate->_menu = (uint8)tempy;
	GetTableInt("swap", tempy);         joystate->_swap = (uint8)tempy;
	GetTableInt("left_select", tempy);  joystate->_left_select = (uint8)tempy;
	GetTableInt("right_select", tempy); joystate->_right_select = (uint8)tempy;
	GetTableInt("pause", tempy);        joystate->_pause = (uint8)tempy;
	GetTableInt("quit", tempy);         joystate->_quit = (uint8)tempy;
	CloseTable();
}

// This function loads all necessary variables and vectors from the boot.hoa config file
void GameData::LoadBootData(
		vector<ImageDescriptor> *boot_images,
		vector<SoundDescriptor> *boot_sound,
		vector<MusicDescriptor> *boot_music) {
	OpenLuaFile("dat/config/boot.hoa");
	string s;
	int32 x1, x2, y1, y2;
	// Load the video stuff
	ImageDescriptor im;

	// The background
	GetGlobalString("background_image", s);
	im.SetFilename(s);
	GetGlobalInt("background_image_width", x1);
	GetGlobalInt("background_image_height", y1);
	im.SetDimensions((float) x1, (float) y1);
	boot_images->push_back(im);

	// The logo
	GetGlobalString("logo_image", s);
	im.SetFilename(s);
	GetGlobalInt("logo_image_width", x1);
	GetGlobalInt("logo_image_height", y1);
	im.SetDimensions((float) x1, (float) y1);
	boot_images->push_back(im);

	// The menu
	GetGlobalString("menu_image", s);
	im.SetFilename(s);
	GetGlobalInt("menu_image_width", x1);
	GetGlobalInt("menu_image_height", y1);
	im.SetDimensions((float) x1, (float) y1);
	boot_images->push_back(im);

	// Set up a coordinate system - now you can use the boot.hoa to set it to whatever you like
	GetGlobalInt("coord_sys_x_left", x1);
	GetGlobalInt("coord_sys_x_right", x2);
	GetGlobalInt("coord_sys_y_bottom", y1);
	GetGlobalInt("coord_sys_y_top", y2);
	_VideoManager->SetCoordSys((float)x1, (float) x2, (float) y1, (float) y2);

	// Load the audio stuff
	// Make a call to the config code that loads in two vectors of strings
	vector<string> new_music_files;
	FillStringVector("music_files", new_music_files);

	vector<string> new_sound_files;
	FillStringVector("sound_files", new_sound_files);

	// Push all our new music onto the boot_music vector
	MusicDescriptor new_music;

	for (uint i = 0; i < new_music_files.size(); i++) {
		new_music.filename = new_music_files[i];
		boot_music->push_back(new_music);
	}

	SoundDescriptor new_sound;
	for (uint i = 0; i < new_sound_files.size(); i++) {
		new_sound.filename = new_sound_files[i];
		boot_sound->push_back(new_sound);
	}
}

// This one loads all the tiles from img/tile/ directory and reads the map file given
// by new_map_id. The function should be called only from the MapMode class members.
void GameData::LoadMap(hoa_map::MapMode *map_mode, int32 new_map_id) {
// 	// Load the map file
	const char *filename = "dat/maps/test_map.hoa";
	OpenLuaFile(filename);

	// Setup some global map options (explanations are in map.h)
	GetGlobalBool((const char*)"random_encounters", (bool&)(map_mode->_random_encounters));
	GetGlobalInt((const char*)"encounter_rate", (int32&)(map_mode->_encounter_rate));
	// this one will change:
	map_mode->_steps_till_encounter = GaussianValue(map_mode->_encounter_rate, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	GetGlobalInt((const char*)"animation_counter", (int32&)(map_mode->_animation_counter));
	GetGlobalInt((const char*)"row_count", (int32&)(map_mode->_row_count));
	GetGlobalInt((const char*)"col_count", (int32&)(map_mode->_col_count));

	// This part loads only the tiles needed by the current map. The map editor fills in the
	// corresponding Lua vector.
	vector<string> tiles_used;
	string tile_prefix = "img/tile/";	// where they're at
	FillStringVector("tiles_used", tiles_used);
	if (tiles_used.size() == 0) {
		cerr << "DATA ERROR: loading map " << filename << " : No tiles specified for map!" << endl;
		//TODO Add meaningful error codes, and make LoadMap return an int
		return;
	}
	ImageDescriptor imgdsc;
	imgdsc.SetDimensions(1.0f, 1.0f);

	_VideoManager->BeginImageLoadBatch();
	for (uint i = 0; i < tiles_used.size(); i++) {
		imgdsc.SetFilename(tile_prefix + tiles_used[i] + ".png");
		map_mode->_map_tiles.push_back(imgdsc);
		_VideoManager->LoadImage(imgdsc);
	}
	_VideoManager->EndImageLoadBatch();

	// The following chunk-o-code checks if the tilename has a letter at the end, which means the
	// tile is a part of the animated tile (for example: cave12a, cave12b, cave12c, etc.). And
	// takes care of everything else ;)
	// First we build a table which tells us at what indices do the individual tiles start at.
	// Example:
	// "cave01", "cave02a", "cave02b", "cave02c", "city01a", "city02b", "city03"
	// would generate a table that looks like:
	// tbl[0] = 0;	// cave01
	// tbl[1] = 1;	// cave02
	// tbl[2] = 4;	// city01
	// tbl[3] = 6;	// city03
	// Oh, you get it...
	vector<int32> tbl;
	tbl.push_back(0);		// There's always at least one tile
	vector<string> basenames;	// basename("cave01a") == "cave01"
	// create the basenames vector
	for (uint32 i = 0; i < tiles_used.size(); i++) {
		if (tiles_used[i][tiles_used[i].size()-1] >= 'a' && tiles_used[i][tiles_used[i].size()-1] <= 'z')
			basenames.push_back(tiles_used[i].substr(0, tiles_used[i].size()-1));
		else
			basenames.push_back(tiles_used[i]);
	}
	// examine the basenames, and create the table
	for (int32 i = 0; i < (int32)(tiles_used.size()) - 1; i++) {
		if (!(basenames[i] == basenames[i+1])) tbl.push_back(i+1);
	}
	tbl.push_back((const int32)tiles_used.size());

	// now that we have the table, we populate the tile_frames vector...
	TileFrame *tframe, *root;
	for (int32 i = 0; i < (int32)tbl.size() - 1; i++) {
		tframe = new TileFrame;
		root = tframe;
		for (int32 j = tbl[i]; j < tbl[i+1]; j++) {
			if (j > tbl[i]) {
				tframe->next = new TileFrame;
				tframe = tframe->next;
			}
			tframe->frame = j;
			tframe->next = 0;
		}
		tframe->next = root;
		map_mode->_tile_frames.push_back(root);
	}

	// Now load the map itself, by loading the lower_layer, upper_layer and event_mask vectors
	vector<int32> lower, upper, emask;
	FillIntVector("lower_layer", lower);
	FillIntVector("upper_layer", upper);
	FillIntVector("event_mask", emask);
	if (lower.size() != upper.size() || upper.size() != emask.size()) {
		cerr << "DATA ERROR: The lower_layer, upper_layer and event_mask vectors do NOT have the same size! Check the editor code, or any modifications you made to the map file " << filename << "!\n";
		// TODO Add meaningful error codes, and make LoadMap return an int
		return;
	}
	if (lower.size() != map_mode->_row_count * map_mode->_col_count) {
		cerr << "DATA ERROR: The actual size of the lower, upper and mask vectors is NOT EQUAL to row_count*col_count !!! BARF!\n";
		cerr << "_row_count = " << map_mode->_row_count << "\n";
		cerr << "_col_count = " << map_mode->_col_count << "\n";
		cerr << "lower.size() = " << (uint32)lower.size() << "\n";
		// TODO Add meaningful error codes, and make LoadMap return an int
		return;
	}
	int32 c = 0;
	MapTile t;
	for (int32 i = 0; i < map_mode->_row_count; i++) {
		map_mode->_tile_layers.push_back(vector<MapTile>());
		for (int32 j = 0; j < map_mode->_col_count; j++) {
			t.lower_layer = lower[c];
			t.upper_layer = upper[c];
			t.properties = emask[c];
			map_mode->_tile_layers[i].push_back(t);
			c++;
		}
	}

	// Load the player's sprite
}


// This function is for DEBUGGING PURPOSES ONLY! It prints the contents of the Lua stack from top to bottom.
void GameData::PrintLuaStack() {
	int32 type;

	cout << "DEBUG: Printing lua stack" << endl;
	for (int32 i = lua_gettop(_l_stack); i > 0; i--) {  // Print each element starting from the top
		type = lua_type(_l_stack, i);
		switch (type) {
		case LUA_TNIL:
			cout << "WARNING: NIL" << endl;
			break;
		case LUA_TBOOLEAN:
			cout << "BOOLEAN: " << lua_toboolean(_l_stack, i) << endl;
			break;
		case LUA_TNUMBER:
			cout << "NUMBER:  " << lua_tonumber(_l_stack, i) << endl;
			break;
		case LUA_TSTRING:
			cout << "STRING:  " << lua_tostring(_l_stack, i) << endl;
			break;
		case LUA_TTABLE:
			cout << "TABLE    " << endl;
			break;
		case LUA_TFUNCTION:
			cout << "FUNCTION " << endl;
			break;
		default:
			cout << "OTHER:   " << lua_typename(_l_stack, type) << endl;
			break;
		}
	}
}

} // namespace hoa_data
