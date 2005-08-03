/* 
 * menu.cpp
 *	Code for Hero of Allacrost menu mode
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
#include "menu.h"
#include "audio.h"
#include "video.h"
#include "engine.h"
#include "global.h"
#include "data.h"

using namespace std;
using namespace hoa_menu::local_menu;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_engine;
using namespace hoa_global;
using namespace hoa_data;



namespace hoa_menu {

bool MENU_DEBUG = false;


MenuMode::MenuMode() {
	if (MENU_DEBUG) cout << "MENU: MenuMode constructor invoked." << endl;
	
	// Setup the coordinate system
	//VideoManager->SetCoordSys(0, 16, 0, 12, 1);
}


MenuMode::~MenuMode() {
	if (MENU_DEBUG) cout << "MENU: MenuMode destructor invoked." << endl;
	// Clean up any allocated music/images/sounds/whatever data here. Don't forget! ^_~
}


void MenuMode::Update(Uint32 time_elapsed) {

}


void MenuMode::Draw() {

}

} // namespace hoa_menu
