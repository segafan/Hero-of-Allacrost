/* 
 * battle.cpp
 *	Code for Hero of Allacrost battle mode
 *	(C) 2005 by Tim Hargreaves
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */
 
/*
 * The code in this file is for **** finish this ****
 */

#include "utils.h"
#include <iostream>
#include "battle.h"
#include "audio.h"
#include "video.h"
#include "engine.h"
#include "global.h"
#include "data.h"

using namespace std;
using namespace hoa_battle::local_battle;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_engine;
using namespace hoa_global;
using namespace hoa_data;


/*

To Tim: I already gave you this list yesterday, but here are the most important files for you to
look through and understand what functions/data is available to you:

audio.h, video.h, data.h, engine.h, utils.h

Within those files, you should look at the definitions of the following classes so you know what
functions you can call and what they do:

GameAudio, GameVideo, GameData, GameInput, GameMode, GameModeManager, GameSettings


The best example of a "mode" class right now is MapMode in map.*. It's pretty big and kind of ugly,
but just look at the MapMode class members and functions and ignore the other classes in there. (All
the MapMode stuff is at the bottom of both files). Good luck!


*/



namespace hoa_battle {

bool BATTLE_DEBUG = false;

/* 

To Tim: From the GameMode class that BattleMode inherits from, you already have working access to
all of the game's singleton classes so you don't need to worry about allocating pointers to them, etc.
You can find them all inside the GameMode class in engine.h, but here's a reference of them for your
convenience:

	hoa_audio::GameAudio *AudioManager;
	hoa_video::GameVideo *VideoManager;
	hoa_data::GameData *DataManager;
	GameInput *InputManager;
	GameModeManager *ModeManager;
	GameSettings *SettingsManager;

*/


BattleMode::BattleMode() {
	cerr << "BATTLE: BattleMode constructor invoked." << endl;
	
	// Load the map from the Lua data file
	//DataManager->LoadMap(this, map_id);
	
	// Setup the coordinate system
	VideoManager->SetCoordSys(0.0f, (float)SCREEN_LENGTH, 0.0f, (float)SCREEN_HEIGHT, 1);
	// To Tim: This sets up a cooriate system where "0, 0" is the top left hand corner of the screen,
	// extends to SCREEN_LENGTH and SCREEN_HEIGHT, and has 1 depth level (ie, it's 2D)
	m_iCounter = 0;
}


BattleMode::~BattleMode() {
	// Clean up any allocated music/images/sounds/whatever data here. Don't forget! ^_~
	cerr << "BATTLE: BattleMode destructor invoked." << endl;
}


void BattleMode::Update(Uint32 time_elapsed) {
	// This function is the top level function that updates the status of the game. You'll likely write
	// several sub-functions that this function calls to keep the size of the function a sane amount.
	// For example, UpdateCharacters() and UpdateEnemies(). Make these sub-functions private, because
	// nothing else should need to know about them.
	
	// time_elapsed tells us how long it's been since the last time this function was called.
	++m_iCounter;
	cerr << m_iCounter << endl;
	
	if (m_iCounter >= 1000)
		ModeManager->Pop();
}


void BattleMode::Draw() {
	// This function draws the next frame that will be displayed to the screen. Like Update(), you'll
	// probably write several sub-functions to keep the size of this function manageable. 

}

} // namespace hoa_battle
