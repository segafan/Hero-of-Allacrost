///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    battle.h
 * \author  Tim Hargreaves, balthazar@allacrost.org
 * \date    Last Updated: August 17th, 2005
 * \brief   Header file for battle mode interface.
 *
 * This code handles the game event processing and frame drawing when the user
 * is fighting a battle. 
 *****************************************************************************/

#ifndef __BATTLE_HEADER__
#define __BATTLE_HEADER__

#include "utils.h"
#include <string>
#include <vector>
#include "defs.h"
#include "engine.h"

namespace hoa_battle {

extern bool BATTLE_DEBUG;

/* The reason for this embedded namespace is so you can have constants with short and "friendly" names. Any
constants you make available inside hoa_battle namespace, you must prefix with BATTLE_ according to our code
standard (have you read it???) The only piece of code that uses the private_battle namespace should be battle.h
and battle.cpp, nothing more. So name those constants whatever you want! =D
*/
namespace private_battle {

const int32 TILE_SIZE = 64; // The virtual "tile map" that we discussed in the forums has square 64 pixel tiles
const int32 SCREEN_LENGTH = 16; // Number of tiles long the screen is
const int32 SCREEN_HEIGHT = 12; // The number of tiles high the screen is

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

	int32 num_enemies;
	int32 m_iCounter;

public:
	BattleMode();
	~BattleMode();

	void Update(uint32 time_elapsed);
	void Draw();
};


} // namespace hoa_battle

#endif
