/* 
 * battle.h
 *	Header file for Hero of Allacrost battle mode
 *	(C) 2005 by Tim Hargreaves
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */

/*
 * The code in this file is for ****finish this****
 */
 
 
/*

To Tim: Here are your skeleton files. This sets up all the basic crap you'll need, now you just have to
make the crap do something useful. :) I think your first task should be drawing static objects on the screen 
(since we have an objective of include a battle screenshot in our next released screens).


Unfortunately, even if we have the code we lack the art. :P So you might need to make some really crappy
placeholder graphics for the time being. Also unfortunately a lot of the classes that BattleMode will be
using (for representing characters and enemies) will be in global.*, which have not been very refined yet.
I'm working on that file though, so hopefully we can work together on it and figure out what it needs.


For the battle code initialization, what I was thinking of doing was in MapMode I'll figure out what 
enemies to have in the battle and how many, and then I will pass you a vector of enemy objects in the 
battle mode constructor or something. Once you've got that, you'll have to figure out how to place the
enemies on the enemy grid area of the battle map. ^_~



One final note. So that you can get to BattleMode to test it when you're actually running the game, I'm
going to add a temporary shortcut in the BootMode code that will load BattleMode. I'll tell you what that
is once I write it.

*/
 
#ifndef __BATTLE_HEADER__
#define __BATTLE_HEADER__

#include <string>
#include <vector>
#include "defs.h"
#include "utils.h"
#include "engine.h"

namespace hoa_battle {


/* The reason for this embedded namespace is so you can have constants with short and "friendly" names. Any
constants you make available inside hoa_battle namespace, you must prefix with BATTLE_ according to our code
standard (have you read it???) The only piece of code that uses the local_battle namespace should be battle.h
and battle.cpp, nothing more. So name those constants whatever you want! =D
*/
namespace local_battle {

const int TILE_SIZE = 64; // The virtual "tile map" that we discussed in the forums has square 64 pixel tiles
const int SCREEN_LENGTH = 16; // Number of tiles long the screen is
const int SCREEN_HEIGHT = 12; // The number of tiles high the screen is
 
}
 
 /******************************************************************************
	BattleMode Class
		
	>>>members<<<

	>>>functions<<<

	>>>notes<<<
 *****************************************************************************/
class BattleMode : public hoa_engine::GameMode {
private:
	friend class hoa_data::GameData;
	
	std::vector<hoa_video::ImageDescriptor> battle_images;	
	std::vector<hoa_audio::MusicDescriptor> battle_music;
	std::vector<hoa_audio::SoundDescriptor> battle_sound;
	
	int num_enemies;
	int m_iCounter;
	
public: 
	BattleMode();
	~BattleMode();
	
	void Update(Uint32 time_elapsed);
	void Draw();
};


} // namespace hoa_battle

#endif
