/*
 * data.cpp
 *  Data management module for Hero of Allacrost
 *  (C) 2004 by Tyler Olsen
 *
 *  This code is licensed under the GNU GPL. It is free software and you may modify it 
 *   and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *   for details.
 */


#include "data.h"
#include <iostream>

using namespace std;
using namespace hoa_global;
using namespace hoa_video;
using namespace hoa_audio;
using namespace hoa_utils;

namespace hoa_data {

SINGLETON1(GameData);

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


// This function initializes all the members of the GameSettings singleton class
void GameData::LoadGameSettings () {
	hoa_utils::Singleton<hoa_global::GameSettings> SettingsManager;
	const char *filename = "data/config/settings.hoa";

	if (luaL_loadfile(l_stack, "data/config/settings.hoa") || lua_pcall(l_stack, 0, 0, 0))
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

	lua_getglobal(l_stack, "key_settings");
	if (!lua_istable(l_stack, LUA_STACK_TOP))
		cout << "LUA ERROR: could not retrieve table \"key_settings\"" << endl;

	SettingsManager->InputStatus.key.up = (SDLKey)GetTableInt("up");
	SettingsManager->InputStatus.key.down = (SDLKey)GetTableInt("down");
	SettingsManager->InputStatus.key.left = (SDLKey)GetTableInt("left");
	SettingsManager->InputStatus.key.right = (SDLKey)GetTableInt("right");
	SettingsManager->InputStatus.key.confirm = (SDLKey)GetTableInt("confirm");
	SettingsManager->InputStatus.key.cancel = (SDLKey)GetTableInt("cancel");
	SettingsManager->InputStatus.key.menu = (SDLKey)GetTableInt("menu");
	SettingsManager->InputStatus.key.pause = (SDLKey)GetTableInt("pause");

	lua_pop(l_stack, 3); // Pop all 3 tables from the stack before returning
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
	
	VideoManager->LoadImage(im);
	boot_images->push_back(im);
	
	// The logo
	im.filename = GetGlobalString("logo_image");
	im.width    = GetGlobalInt("logo_image_width");
	im.height   = GetGlobalInt("logo_image_height");
	
	VideoManager->LoadImage(im);
	boot_images->push_back(im);
	
	// The menu
	im.filename = GetGlobalString("menu_image");
	im.width    = GetGlobalInt("menu_image_width");
	im.height   = GetGlobalInt("menu_image_height");
	
	VideoManager->LoadImage(im);
	boot_images->push_back(im);
	
	// Set up a coordinate system - now you can use the boot.hoa to set it to whatever you like
	VideoManager->SetCoordSys(GetGlobalInt("coord_sys_x_left"),
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
		AudioManager->LoadMusic(boot_music->back());
	}
	
	SoundDescriptor new_sound;
	
	for (int i = 0; i < new_sound_files.size(); i++) {
		new_sound.filename = new_sound_files[i];
		boot_sound->push_back(new_sound);
		AudioManager->LoadSound(boot_sound->back());
	}
	
	AudioManager->PlayMusic((*boot_music)[0], AUDIO_NO_FADE, AUDIO_LOOP_FOREVER);
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
