///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    menu.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 12th, 2005
 * \brief   Source file for menu mode interface.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include "menu.h"
#include "audio.h"
#include "video.h"
#include "engine.h"
#include "global.h"
#include "data.h"

using namespace std;
using namespace hoa_menu::private_menu;
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

	// Save the currently drawn screen
	if (!VideoManager->CaptureScreen(_saved_screen)) {
		cerr << "MENU: ERROR: Couldn't save the screen!" << endl;
	}
	
	ImageDescriptor new_menu;
	_menu_images.push_back(new_menu);
	_menu_images.push_back(new_menu);
	_menu_images.push_back(new_menu);
	_menu_images.push_back(new_menu);
	_menu_images.push_back(new_menu);
	_menu_images.push_back(new_menu);
	
	// Create menu images
	if(!VideoManager->CreateMenu(_menu_images[0], 256, 576))  // create a 256x576 menu
		cerr << "MENU: ERROR: Couldn't create menu image!" << endl;
	if(!VideoManager->CreateMenu(_menu_images[1], 256, 576)) 
		cerr << "MENU: ERROR: Couldn't create menu image!" << endl;
	if(!VideoManager->CreateMenu(_menu_images[2], 256, 576)) 
		cerr << "MENU: ERROR: Couldn't create menu image!" << endl;
	if(!VideoManager->CreateMenu(_menu_images[3], 256, 576)) 
		cerr << "MENU: ERROR: Couldn't create menu image!" << endl;
	if(!VideoManager->CreateMenu(_menu_images[4], 1024, 96)) 
		cerr << "MENU: ERROR: Couldn't create menu image!" << endl;
	if(!VideoManager->CreateMenu(_menu_images[5], 1024, 96)) 
		cerr << "MENU: ERROR: Couldn't create menu image!" << endl;
}


MenuMode::~MenuMode() {
	if (MENU_DEBUG) cout << "MENU: MenuMode destructor invoked." << endl;
	
	// Remove saved images
	VideoManager->DeleteImage(_saved_screen);
	
	for (uint32 i = 0; i < _menu_images.size(); i++) {
		VideoManager->DeleteImage(_menu_images[i]);
	}
}


// Resets appropriate class members
void MenuMode::Reset() {
	VideoManager->SetCoordSys(0, 1024, 768, 0); // Top left corner coordinates are (0,0)
	if(!VideoManager->SetFont("default")) 
    cerr << "MAP: ERROR > Couldn't set menu font!" << endl;
}


void MenuMode::Update(uint32 time_elapsed) {

	if (InputManager->CancelPress()) {
		ModeManager->Pop();
	}
}


void MenuMode::Draw() {
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	// Move to the top left corner
	VideoManager->Move(0,0);
	
	// Draw the saved screen as the menu background
	VideoManager->DrawImage(_saved_screen); 
	
	// Draw the four character menus
	VideoManager->DrawImage(_menu_images[0]);
	VideoManager->MoveRel(256, 0);
	VideoManager->DrawImage(_menu_images[1]);
	VideoManager->MoveRel(256, 0);
	VideoManager->DrawImage(_menu_images[2]);
	VideoManager->MoveRel(256, 0);
	VideoManager->DrawImage(_menu_images[3]);
	
	// Draw the bottom two menus
	VideoManager->Move(0, 576);
	VideoManager->DrawImage(_menu_images[4]);
	VideoManager->MoveRel(0, 96);
	VideoManager->DrawImage(_menu_images[5]);
	
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
	
	// Draw 1st character menu text
	if (!VideoManager->DrawText("Claudius", 32, 768 - 48))
		cerr << "MENU: ERROR > Couldn't draw text!" << endl;
		
	if (!VideoManager->DrawText("Health: 68", 32, 768 - 516))
		cerr << "MENU: ERROR > Couldn't draw text!" << endl;
	if (!VideoManager->DrawText("Skill: 23", 32, 768 - 541))
		cerr << "MENU: ERROR > Couldn't draw text!" << endl;
	if (!VideoManager->DrawText("XP to level: 498", 32, 768 - 566))
		cerr << "MENU: ERROR > Couldn't draw text!" << endl;
		
	// Draw selection menu text
	if (!VideoManager->DrawText("Inventory     Skills     Equipment     Status     Options     Save", 32, 768 - 640))
		cerr << "MENU: ERROR > Couldn't draw text!" << endl;
	
	// Draw 2nd menu text
	if (!VideoManager->DrawText("Time: 00:24:35", 32, 768 - 720))
		cerr << "MENU: ERROR > Couldn't draw text!" << endl;
	if (!VideoManager->DrawText("Bling: 4,201B", 32, 768 - 744))
		cerr << "MENU: ERROR > Couldn't draw text!" << endl;
	
}

} // namespace hoa_menu
