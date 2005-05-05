/*
 * data.cpp
 *  Data management module for Hero of Allacrost
 *  (C) 2004 by Tyler Olsen
 *
 *  This code is licensed under the GNU GPL. It is free software and you may modify it 
 *   and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *   for details.
 */


#include <iostream>
#include "data.h"
#include "audio.h"
#include "video.h"
#include "global.h"
#include "boot.h"
#include "map.h"


using namespace std;
using namespace hoa_global;
using namespace hoa_video;
using namespace hoa_audio;
using namespace hoa_utils;
using namespace hoa_map;

namespace hoa_data {

SINGLETON_INITIALIZE(GameData);

// The constructor opens lua and its associated libraries.
GameData::GameData() {
	if (DATA_DEBUG)
		cerr << "DEBUG: GameData constructor invoked." << endl;
	l_stack = lua_open();

	// Load the Lua libraries
	const luaL_reg *lib = LUALIBS;
	for (; lib->func; lib++) {
		lib->func(l_stack);      // open library
		lua_settop(l_stack, 0);  // Clear the stack
	}
	
	AudioManager = GameAudio::_GetReference();
	VideoManager = GameVideo::_GetReference();
}

// Close Lua upon destruction
GameData::~GameData() {
	if (DATA_DEBUG)
		cerr << "DEBUG: GameData destructor invoked." << endl;
	lua_close(l_stack);
}



// Get an individual boolean field from a table, assumed to be on top of the stack
bool GameData::GetTableBool (const char *key) {
	bool result;
	lua_pushstring(l_stack, key);
	lua_gettable(l_stack, -2);
	if (!lua_isboolean(l_stack, -1))
		cout << "ERROR: Invalid table field" << endl;
	result = lua_toboolean(l_stack, -1);
	lua_pop(l_stack, 1);
	return result;
}

// Get an individual integer field from a table, assumed to be on top of the stack
int GameData::GetTableInt (const char *key) {
	int result;
	lua_pushstring(l_stack, key);
	lua_gettable(l_stack, -2);
	if (!lua_isnumber(l_stack, -1))
		cout << "ERROR: Invalid table field" << endl;
	result = (int)lua_tonumber(l_stack, -1);
	lua_pop(l_stack, 1);
	return result;
}

// Get an individual floating point field from a table, assumed to be on top of the stack
float GameData::GetTableFloat (const char *key) {
	float result;
	lua_pushstring(l_stack, key);
	lua_gettable(l_stack, -2);
	if (!lua_isnumber(l_stack, -1))
		cout << "ERROR: Invalid table field" << endl;
	result = (float)lua_tonumber(l_stack, -1);
	lua_pop(l_stack, 1);
	return result;
}

// Get an individual string field from a table, assumed to be on top of the stack
string GameData::GetTableString (const char *key) {
	string result;
	lua_pushstring(l_stack, key);
	lua_gettable(l_stack, -2);
	if (!lua_isstring(l_stack, -1))
		cout << "ERROR: Invalid table field" << endl;
	result = (string)lua_tostring(l_stack, -1);
	lua_pop(l_stack, 1);
	return result;
}



bool GameData::GetGlobalBool(const char *key) {
	bool result;
	lua_getglobal(l_stack, key);
	result = (bool)lua_toboolean(l_stack, LUA_STACK_TOP);
	lua_pop(l_stack, 1);
	return result;
}

int GameData::GetGlobalInt(const char *key) {
	int result;
	lua_getglobal(l_stack, key);
	result = (int)lua_tonumber(l_stack, LUA_STACK_TOP);
	lua_pop(l_stack, 1);
	return result;
}

float GameData::GetGlobalFloat(const char *key) {
	float result;
	lua_getglobal(l_stack, key);
	result = (float)lua_tonumber(l_stack, LUA_STACK_TOP);
	lua_pop(l_stack, 1);
	return result;
}

std::string GameData::GetGlobalString(const char * key) {
	string result;
	lua_getglobal(l_stack, key);
	result = (string)lua_tostring(l_stack, LUA_STACK_TOP);
	lua_pop(l_stack, 1);
	return result;
}

// TODO L8R: Maybe turn this function into FillVector(vector*, const char*, int lua_type),
//           which will fill a custom vector?
void GameData::FillStringVector(vector<string> *vect, const char *key) {
	lua_getglobal(l_stack, key);
	if (!lua_istable(l_stack, LUA_STACK_TOP)) {
		cout << "Lua error: table " << key << " does not exist, or " << key << "ain't a table\n";
		return;
	}
	int t = lua_gettop(l_stack);
	lua_pushnil(l_stack);
	while (lua_next(l_stack, t)) {
		vect->push_back((string)lua_tostring(l_stack, LUA_STACK_TOP));
		lua_pop(l_stack, 1);
	}
}

void GameData::FillIntVector(std::vector<int> *vect, const char *key) {
	lua_getglobal(l_stack, key);
	if (!lua_istable(l_stack, LUA_STACK_TOP)) {
		cout << "Lua error: table " << key << " does not exist, or " << key << "ain't a table\n";
		return;
	}
	int t = lua_gettop(l_stack);
	lua_pushnil(l_stack);
	while (lua_next(l_stack, t)) {
		vect->push_back((int)lua_tonumber(l_stack, LUA_STACK_TOP));
		lua_pop(l_stack, 1);
	}
}


// This function initializes all the members of the GameSettings singleton class
void GameData::LoadGameSettings () {
	hoa_global::GameSettings *SettingsManager = GameSettings::_GetReference();
	const char *filename = "data/config/settings.hoa";

	if (luaL_loadfile(l_stack, filename) || lua_pcall(l_stack, 0, 0, 0))
		cout << "LUA ERROR: Could not load " << filename << " :: " << lua_tostring(l_stack, -1) << endl;

	lua_getglobal(l_stack, "video_settings");
	if (!lua_istable(l_stack, LUA_STACK_TOP))
		cout << "LUA ERROR: could not retrieve table \"video_settings\"" << endl;

	//   SettingsManager->screen_resx = GetTableInt("screen_resx");
	//   SettingsManager->screen_resy = GetTableInt("screen_resy");
	SettingsManager->full_screen = GetTableBool("full_screen");

	lua_getglobal(l_stack, "audio_settings");
	if (!lua_istable(l_stack, LUA_STACK_TOP))
		cout << "LUA ERROR: could not retrieve table \"audio_settings\"" << endl;

	SettingsManager->music_vol = GetTableInt("music_vol");
	SettingsManager->sound_vol = GetTableInt("sound_vol");

// 	lua_getglobal(l_stack, "key_settings");
// 	if (!lua_istable(l_stack, LUA_STACK_TOP))
// 		cout << "LUA ERROR: could not retrieve table \"key_settings\"" << endl;

// 	SettingsManager->InputStatus.key.up = (SDLKey)GetTableInt("up");
// 	SettingsManager->InputStatus.key.down = (SDLKey)GetTableInt("down");
// 	SettingsManager->InputStatus.key.left = (SDLKey)GetTableInt("left");
// 	SettingsManager->InputStatus.key.right = (SDLKey)GetTableInt("right");
// 	SettingsManager->InputStatus.key.confirm = (SDLKey)GetTableInt("confirm");
// 	SettingsManager->InputStatus.key.cancel = (SDLKey)GetTableInt("cancel");
// 	SettingsManager->InputStatus.key.menu = (SDLKey)GetTableInt("menu");
// 	SettingsManager->InputStatus.key.pause = (SDLKey)GetTableInt("pause");

	lua_pop(l_stack, 2); // Pop all tables from the stack before returning
}

void GameData::LoadKeyJoyState(KeyState *keystate, JoystickState *joystate) {
	const char *filename = "data/config/settings.hoa";
	if (luaL_loadfile(l_stack, filename) || lua_pcall(l_stack, 0, 0, 0))
		cout << "LUA ERROR: Could not load " << filename << " :: " << lua_tostring(l_stack, -1) << endl;
	
	lua_getglobal(l_stack, "key_settings");
	if (!lua_istable(l_stack, LUA_STACK_TOP))
		cout << "LUA ERROR: could not retrieve table \"key_settings\"" << endl;
	
	keystate->up = (SDLKey)GetTableInt("up");
	keystate->down = (SDLKey)GetTableInt("down");
	keystate->left = (SDLKey)GetTableInt("left");
	keystate->right = (SDLKey)GetTableInt("right");
	keystate->confirm = (SDLKey)GetTableInt("confirm");
	keystate->cancel = (SDLKey)GetTableInt("cancel");
	keystate->menu = (SDLKey)GetTableInt("menu");
	keystate->swap = (SDLKey)GetTableInt("swap");
	keystate->rselect = (SDLKey)GetTableInt("rselect");
	keystate->lselect = (SDLKey)GetTableInt("lselect");
	keystate->pause = (SDLKey)GetTableInt("pause");
	
	//TODO Add joystick init, after we implement joystick functionality
	
	// POP! :)
	lua_pop(l_stack, 1);
}

// This function loads all necessary variables and vectors from the boot.hoa config file
void GameData::LoadBootData(
		vector<ImageDescriptor> *boot_images,
		vector<SoundDescriptor> *boot_sound,
		vector<MusicDescriptor> *boot_music) {
	char* filename = "data/config/boot.hoa";
	if (luaL_loadfile(l_stack, filename) || lua_pcall(l_stack, 0, 0, 0))
		cout << "LUA ERROR: Could not load "<< filename << " :: " << lua_tostring(l_stack, -1) << endl;
	
	// Load the video stuff
	//
	
	ImageDescriptor im;

	// The background
	im.filename = GetGlobalString("background_image");
	im.width    = GetGlobalInt("background_image_width");
	im.height   = GetGlobalInt("background_image_height");
	boot_images->push_back(im);
	
	// The logo
	im.filename = GetGlobalString("logo_image");
	im.width    = GetGlobalInt("logo_image_width");
	im.height   = GetGlobalInt("logo_image_height");
	boot_images->push_back(im);
	
	// The menu
	im.filename = GetGlobalString("menu_image");
	im.width    = GetGlobalInt("menu_image_width");
	im.height   = GetGlobalInt("menu_image_height");
	boot_images->push_back(im);
	
	// Set up a coordinate system - now you can use the boot.hoa to set it to whatever you like
	GameVideo::_GetReference()->SetCoordSys(GetGlobalInt("coord_sys_x_left"),
					GetGlobalInt("coord_sys_x_right"),
					GetGlobalInt("coord_sys_y_bottom"),
					GetGlobalInt("coord_sys_y_top"),
					GetGlobalInt("coord_sys_nl"));
	
	// Load the audio stuff
	// Make a call to the config code that loads in two vectors of strings
	vector<string> new_music_files;
	FillStringVector(&new_music_files, "music_files");
	
	vector<string> new_sound_files;
	FillStringVector(&new_sound_files, "sound_files");
	
	// Push all our new music onto the boot_music vector
	MusicDescriptor new_music;
	
	for (int i = 0; i < new_music_files.size(); i++) {
		new_music.filename = new_music_files[i];
		boot_music->push_back(new_music);
	}
	
	SoundDescriptor new_sound;
	
	for (int i = 0; i < new_sound_files.size(); i++) {
		new_sound.filename = new_sound_files[i];
		boot_sound->push_back(new_sound);
	}
	
}

// This one loads all the tiles from img/tile/ directory and reads the map file given
// by new_map_id. The function should be called only from the MapMode class members.
void GameData::LoadMap(hoa_map::MapMode *map_mode, int new_map_id) {
	// Load the map file
	string filename = "data/maps/map";
	filename += "1";	// TEMPORARY TEMPORARY
	// filename += new_map_id;
	filename += ".hoa";
	
	if (luaL_loadfile(l_stack, filename.c_str()) || lua_pcall(l_stack, 0, 0, 0))
		cout << "LUA ERROR: Could not load "<< filename << " :: " << lua_tostring(l_stack, -1) << endl;
	
	// Setup some global map options (explanations are in map.h)
	map_mode->map_state = GetGlobalInt("map_state");	
	map_mode->random_encounters = GetGlobalBool("random_encounters");
	map_mode->encounter_rate = GetGlobalInt("encounter_rate");
	map_mode->steps_till_encounter = GaussianValue(map_mode->encounter_rate, UTILS_NO_BOUNDS, UTILS_ONLY_POSITIVE);
	map_mode->animation_rate = GetGlobalInt("animation_rate");
	map_mode->animation_counter = GetGlobalInt("animation_counter");
	map_mode->tile_count = GetGlobalInt("tile_count");
	map_mode->row_count = GetGlobalInt("row_count");
	map_mode->col_count = GetGlobalInt("col_count");
	
	// This part loads only the tiles needed by the current map. The map editor fills in the
	// corresponding Lua vector.
	vector<string> tiles_used;
	string tile_prefix = "img/tile/";	// where they're at
	FillStringVector(&tiles_used, "tiles_used");
	if (tiles_used.size() == 0) {
		cout << "Error loading map " << filename << " : No tiles specified for map!! (??)" << endl;
		//TODO Add meaningful error codes, and make LoadMap return an int
		return;
	}
	ImageDescriptor imgdsc;
	imgdsc.width = imgdsc.height = 1;
	for (int i = 0; i < tiles_used.size(); i++) {
		imgdsc.filename = tile_prefix + tiles_used[i] + ".png";
		map_mode->map_tiles.push_back(imgdsc);
		VideoManager->LoadImage(imgdsc);
	}
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
	vector<int> tbl;
	tbl.push_back(0);		// There's always at least one tile
	vector<string> basenames;	// basename("cave01a") == "cave01"
	// create the basenames vector
	for (int i = 0; i < tiles_used.size(); i++) {
		if (tiles_used[i][tiles_used[i].size()-1] >= 'a' && tiles_used[i][tiles_used[i].size()-1] <= 'z')
			basenames.push_back(tiles_used[i].substr(0, tiles_used[i].size()-1));
		else
			basenames.push_back(tiles_used[i]);
	}
	// examine the basenames, and create the table
	for (int i = 0; i < tiles_used.size() - 1; i++) {
		if (!(basenames[i] == basenames[i+1])) tbl.push_back(i+1);
	}
	tbl.push_back(tiles_used.size());
	
	for (int i = 0; i < tbl.size(); i++) cout << "In LoadMap: " << tbl[i] << endl;	/* DEBUG */
	
	// now that we have the table, we populate the tile_frames vector...
	TileFrame *tframe, *root;
	for (int i = 0; i < tbl.size() - 1; i++) {
		tframe = new TileFrame;
		root = tframe;
		for (int j = tbl[i]; j < tbl[i+1]; j++) {
			if (j > tbl[i]) {
				tframe->next = new TileFrame;
				tframe = tframe->next;
			}
			tframe->frame = j;
			tframe->next = 0;
		}
		tframe->next = root;
		map_mode->tile_frames.push_back(tframe);
	}
	
	// Now load the map itself, by loading the lower_layer, upper_layer and event_mask vectors
	vector<int> lower, upper, emask;
	FillIntVector(&lower, "lower_layer");
	FillIntVector(&upper, "upper_layer");
	FillIntVector(&emask, "event_mask");
	if (lower.size() != upper.size() || upper.size() != emask.size()) {
		cout << "ERROR: The lower_layer, upper_layer and event_mask vectors do NOT have the same size! Check the editor code, or any modifications you made to the map file " << filename << "!\n";
		// TODO Add meaningful error codes, and make LoadMap return an int
		return;
	}
	if (lower.size() != map_mode->row_count * map_mode->col_count) {
		cout << "ERROR: The actual size of the lower, upper and mask vectors is NOT EQUAL to row_count*col_count !!! BARF!\n";
		cout << "row_count = " << map_mode->row_count << "\n";
		cout << "col_count = " << map_mode->col_count << "\n";
		cout << "lower.size() = " << lower.size() << "\n";
		// TODO Add meaningful error codes, and make LoadMap return an int
		return;
	}
	int c = 0;
	MapTile t;
	for (int i = 0; i < map_mode->row_count; i++) {
		map_mode->map_layers.push_back(vector<MapTile>());
		for (int j = 0; j < map_mode->col_count; j++) {
			t.lower_layer = lower[c];
			t.upper_layer = upper[c];
			t.event_mask = emask[c];
			map_mode->map_layers[i].push_back(t);
			c++;
		}
	}
	
	// load Claudius
	map_mode->player_sprite = new PlayerSprite();
	map_mode->object_layer.push_back(map_mode->player_sprite);
	
	cout << "Kewl!\n";
}


// This function is for DEBUGGING PURPOSES ONLY! It prints the contents of the Lua stack from top to bottom.
void GameData::PrintLuaStack() {
	int type;

	cout << "DEBUG: Printing lua stack" << endl;
	for (int i = lua_gettop(l_stack); i > 0; i--) {  // Print each element starting from the top
		type = lua_type(l_stack, i);
		switch (type) {
		case LUA_TNIL:
			cout << "WARNING: NIL" << endl;
			break;
		case LUA_TBOOLEAN:
			cout << "BOOLEAN: " << lua_toboolean(l_stack, i) << endl;
			break;
		case LUA_TNUMBER:
			cout << "NUMBER:  " << lua_tonumber(l_stack, i) << endl;
			break;
		case LUA_TSTRING:
			cout << "STRING:  " << lua_tostring(l_stack, i) << endl;
			break;
		case LUA_TTABLE:
			cout << "TABLE    " << endl;
			break;
		case LUA_TFUNCTION:
			cout << "FUNCTION " << endl;
			break;
		default:
			cout << "OTHER:   " << lua_typename(l_stack, type) << endl;
			break;
		}
	}
}

} // namespace hoa_data
