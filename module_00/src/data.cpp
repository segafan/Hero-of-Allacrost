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

	_AudioManager = GameAudio::_GetReference();
	_VideoManager = GameVideo::_GetReference();
}

// Close Lua upon destruction
GameData::~GameData() {
	if (DATA_DEBUG) cout << "DATA: GameData destructor invoked." << endl;
	lua_close(_l_stack);
}



// Get an individual boolean field from a table, assumed to be on top of the stack
bool GameData::_GetTableBool (const char *key) {
	bool result;
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isboolean(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	result = (0 != lua_toboolean(_l_stack, -1));
	lua_pop(_l_stack, 1);
	return result;
}

// Get an individual integer field from a table, assumed to be on top of the stack
int GameData::_GetTableInt (const char *key) {
	int32 result;
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isnumber(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	result = (int32)lua_tonumber(_l_stack, -1);
	lua_pop(_l_stack, 1);
	return result;
}

// Get an individual floating point field from a table, assumed to be on top of the stack
float GameData::_GetTableFloat (const char *key) {
	float result;
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isnumber(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	result = (float)lua_tonumber(_l_stack, -1);
	lua_pop(_l_stack, 1);
	return result;
}

// Get an individual string field from a table, assumed to be on top of the stack
string GameData::_GetTableString (const char *key) {
	string result;
	lua_pushstring(_l_stack, key);
	lua_gettable(_l_stack, -2);
	if (!lua_isstring(_l_stack, -1))
		cerr << "DATA ERROR: Invalid table field" << endl;
	result = (string)lua_tostring(_l_stack, -1);
	lua_pop(_l_stack, 1);
	return result;
}



bool GameData::_GetGlobalBool(const char *key) {
	bool result;
	lua_getglobal(_l_stack, key);
	result = (0 != lua_toboolean(_l_stack, LUA_STACK_TOP));
	lua_pop(_l_stack, 1);
	return result;
}

int GameData::_GetGlobalInt(const char *key) {
	int32 result;
	lua_getglobal(_l_stack, key);
	result = (int32)lua_tonumber(_l_stack, LUA_STACK_TOP);
	lua_pop(_l_stack, 1);
	return result;
}

float GameData::_GetGlobalFloat(const char *key) {
	float result;
	lua_getglobal(_l_stack, key);
	result = (float)lua_tonumber(_l_stack, LUA_STACK_TOP);
	lua_pop(_l_stack, 1);
	return result;
}

std::string GameData::_GetGlobalString(const char * key) {
	string result;
	lua_getglobal(_l_stack, key);
	result = (string)lua_tostring(_l_stack, LUA_STACK_TOP);
	lua_pop(_l_stack, 1);
	return result;
}

// TODO L8R: Maybe turn this function into FillVector(vector*, const char*, int lua_type),
//           which will fill a custom vector?
void GameData::_FillStringVector(vector<string> *vect, const char *key) {
	lua_getglobal(_l_stack, key);
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: table " << key << " does not exist, or " << key << "isn't a table\n";
		return;
	}
	int32 t = lua_gettop(_l_stack);
	lua_pushnil(_l_stack);
	while (lua_next(_l_stack, t)) {
		vect->push_back((string)lua_tostring(_l_stack, LUA_STACK_TOP));
		lua_pop(_l_stack, 1);
	}
}

void GameData::_FillIntVector(std::vector<int> *vect, const char *key) {
	lua_getglobal(_l_stack, key);
	if (!lua_istable(_l_stack, LUA_STACK_TOP)) {
		cerr << "DATA ERROR: table " << key << " does not exist, or " << key << "isn't a table\n";
		return;
	}
	int32 t = lua_gettop(_l_stack);
	lua_pushnil(_l_stack);
	while (lua_next(_l_stack, t)) {
		vect->push_back((int32)lua_tonumber(_l_stack, LUA_STACK_TOP));
		lua_pop(_l_stack, 1);
	}
}


// This function initializes all the members of the GameSettings singleton class
void GameData::LoadGameSettings () {
	hoa_engine::GameSettings *SettingsManager = GameSettings::_GetReference();
	const char *filename = "dat/config/settings.hoa";

	if (luaL_loadfile(_l_stack, filename) || lua_pcall(_l_stack, 0, 0, 0))
		cerr << "DATA ERROR: Could not load " << filename << " :: " << lua_tostring(_l_stack, -1) << endl;

	lua_getglobal(_l_stack, "video_settings");
	if (!lua_istable(_l_stack, LUA_STACK_TOP))
		cerr << "DATA ERROR: could not retrieve table \"video_settings\"" << endl;

	//   SettingsManager->screen_resx = GetTableInt("screen_resx");
	//   SettingsManager->screen_resy = GetTableInt("screen_resy");
	SettingsManager->SetFullScreen(_GetTableBool("full_screen"));

	lua_getglobal(_l_stack, "audio_settings");
	if (!lua_istable(_l_stack, LUA_STACK_TOP))
		cerr << "DATA ERROR: could not retrieve table \"audio_settings\"" << endl;

	SettingsManager->music_vol = _GetTableInt("music_vol");
	SettingsManager->sound_vol = _GetTableInt("sound_vol");

	lua_pop(_l_stack, 2); // Pop all tables from the stack before returning
}

void GameData::LoadKeyJoyState(KeyState *keystate, JoystickState *joystate) {
	const char *filename = "dat/config/settings.hoa";
	if (luaL_loadfile(_l_stack, filename) || lua_pcall(_l_stack, 0, 0, 0))
		cerr << "DATA ERROR: Could not load " << filename << " :: " << lua_tostring(_l_stack, -1) << endl;

	lua_getglobal(_l_stack, "key_settings");
	if (!lua_istable(_l_stack, LUA_STACK_TOP))
		cerr << "DATA ERROR: could not retrieve table \"key_settings\"" << endl;

	keystate->_up = (SDLKey)_GetTableInt("up");
	keystate->_down = (SDLKey)_GetTableInt("down");
	keystate->_left = (SDLKey)_GetTableInt("left");
	keystate->_right = (SDLKey)_GetTableInt("right");
	keystate->_confirm = (SDLKey)_GetTableInt("confirm");
	keystate->_cancel = (SDLKey)_GetTableInt("cancel");
	keystate->_menu = (SDLKey)_GetTableInt("menu");
	keystate->_swap = (SDLKey)_GetTableInt("swap");
	keystate->_left_select = (SDLKey)_GetTableInt("left_select");
	keystate->_right_select = (SDLKey)_GetTableInt("right_select");

	keystate->_pause = (SDLKey)_GetTableInt("pause");

	//TODO Add joystick init, after we implement joystick functionality

	// POP! :)
	lua_pop(_l_stack, 1);
}

// This function loads all necessary variables and vectors from the boot.hoa config file
void GameData::LoadBootData(
		vector<ImageDescriptor> *boot_images,
		vector<SoundDescriptor> *boot_sound,
		vector<MusicDescriptor> *boot_music) {
	char* filename = "dat/config/boot.hoa";
	if (luaL_loadfile(_l_stack, filename) || lua_pcall(_l_stack, 0, 0, 0))
		cerr << "DATA ERROR: Could not load "<< filename << " :: " << lua_tostring(_l_stack, -1) << endl;

	// Load the video stuff
	ImageDescriptor im;

	// The background
	im.filename = _GetGlobalString("background_image");
	im.width    = (float) _GetGlobalInt("background_image_width");
	im.height   = (float) _GetGlobalInt("background_image_height");
	boot_images->push_back(im);

	// The logo
	im.filename = _GetGlobalString("logo_image");
	im.width    = (float) _GetGlobalInt("logo_image_width");
	im.height   = (float) _GetGlobalInt("logo_image_height");
	boot_images->push_back(im);

	// The menu
	im.filename = _GetGlobalString("menu_image");
	im.width    = (float) _GetGlobalInt("menu_image_width");
	im.height   = (float) _GetGlobalInt("menu_image_height");
	boot_images->push_back(im);

	// Set up a coordinate system - now you can use the boot.hoa to set it to whatever you like
	GameVideo::_GetReference()->SetCoordSys((float)_GetGlobalInt("coord_sys_x_left"),
					(float) _GetGlobalInt("coord_sys_x_right"),
					(float) _GetGlobalInt("coord_sys_y_bottom"),
					(float) _GetGlobalInt("coord_sys_y_top"));

	// Load the audio stuff
	// Make a call to the config code that loads in two vectors of strings
	vector<string> new_music_files;
	_FillStringVector(&new_music_files, "music_files");

	vector<string> new_sound_files;
	_FillStringVector(&new_sound_files, "sound_files");

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
	// Load the map file
	string filename = "dat/maps/map";
	// filename += "1";	// TEMPORARY TEMPORARY
	filename += "1";
	filename += ".hoa";

	if (luaL_loadfile(_l_stack, filename.c_str()) || lua_pcall(_l_stack, 0, 0, 0))
		cout << "LUA ERROR: Could not load "<< filename << " :: " << lua_tostring(_l_stack, -1) << endl;

	// Setup some global map options (explanations are in map.h)
	map_mode->_map_state.push_back(_GetGlobalInt("map_state"));
	map_mode->_random_encounters = _GetGlobalBool("random_encounters");
	map_mode->_encounter_rate = _GetGlobalInt("encounter_rate");
	// this one will change:
	map_mode->_steps_till_encounter = GaussianValue(map_mode->_encounter_rate, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	map_mode->_animation_counter = _GetGlobalInt("animation_counter");
	map_mode->_row_count = _GetGlobalInt("row_count");
	map_mode->_col_count = _GetGlobalInt("col_count");

	// This part loads only the tiles needed by the current map. The map editor fills in the
	// corresponding Lua vector.
	vector<string> tiles_used;
	string tile_prefix = "img/tile/";	// where they're at
	_FillStringVector(&tiles_used, "tiles_used");
	if (tiles_used.size() == 0) {
		cerr << "DATA ERROR: loading map " << filename << " : No tiles specified for map!! (??)" << endl;
		//TODO Add meaningful error codes, and make LoadMap return an int
		return;
	}
	ImageDescriptor imgdsc;
	imgdsc.width = imgdsc.height = 1;

	_VideoManager->BeginImageLoadBatch();
	for (uint i = 0; i < tiles_used.size(); i++) {
		imgdsc.filename = tile_prefix + tiles_used[i] + ".png";
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
	_FillIntVector(&lower, "lower_layer");
	_FillIntVector(&upper, "upper_layer");
	_FillIntVector(&emask, "event_mask");
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
		map_mode->_map_layers.push_back(vector<MapTile>());
		for (int32 j = 0; j < map_mode->_col_count; j++) {
			t.lower_layer = lower[c];
			t.upper_layer = upper[c];
			t.properties = emask[c];
			map_mode->_map_layers[i].push_back(t);
			c++;
		}
	}

	// load Claudius
	// >>> SNIPE: See the new definition for the PlayerSprite constructor
	//map_mode->player_sprite = new PlayerSprite();
	//map_mode->object_layer.push_back(map_mode->player_sprite);
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
