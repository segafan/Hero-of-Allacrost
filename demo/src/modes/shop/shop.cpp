///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for shop mode interface.
***
*** This code provides an interface for the user to purchase wares from a
*** merchant. This mode is usually entered from a map after discussing with a
*** store owner.
*** ***************************************************************************/

#include <iostream>

#include "defs.h"
#include "utils.h"
#include "shop.h"

#include "audio.h"
#include "video.h"
#include "input.h"
#include "system.h"
#include "global.h"
#include "mode_manager.h"

using namespace std;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_mode_manager;
using namespace hoa_shop::private_shop;

namespace hoa_shop {

bool SHOP_DEBUG = false;

namespace private_shop {

ShopMode* current_shop = NULL;

} // namespace private_shop

ShopMode::ShopMode() {
	if (SHOP_DEBUG)
		cout << "SHOP: ShopMode constructor invoked" << endl;

	mode_type = MODE_MANAGER_SHOP_MODE;
	private_shop::current_shop = this;
	_state = SHOP_STATE_ACTION;

	if (VideoManager->CaptureScreen(_saved_screen) == false) {
		if (SHOP_DEBUG)
			cerr << "SHOP ERROR: Failed to capture saved screen" << endl;
	}
}



ShopMode::~ShopMode() {
	if (SHOP_DEBUG)
		cout << "SHOP: ShopMode destructor invoked" << endl;

	VideoManager->DeleteImage(_saved_screen);

	for (uint32 i = 0; i < _all_objects.size(); i++) {
		delete(_all_objects[i]);
	}
	_all_objects.clear();

	private_shop::current_shop = NULL;
}


// Called whenever ShopMode is put on top of the stack
void ShopMode::Reset() {
	// Setup video engine constructs
	VideoManager->SetCoordSys(0, 1024, 0, 768);
	if (VideoManager->SetFont("default") == false)
    	cerr << "SHOP ERROR: failed to set font" << endl;
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->SetTextColor(Color::white);

	// Everything is temporary code from here to the end of this function
	_all_objects.push_back(new GlobalItem(1));
	_all_objects.push_back(new GlobalWeapon(10001));
	_all_objects.push_back(new GlobalWeapon(10002));
// 	_all_objects.push_back(new GlobalArmor(20001));
	_all_objects.push_back(new GlobalArmor(20002));
// 	_all_objects.push_back(new GlobalArmor(30001));
	_all_objects.push_back(new GlobalArmor(30002));
// 	_all_objects.push_back(new GlobalArmor(40001));
	_all_objects.push_back(new GlobalArmor(40002));
	_all_objects.push_back(new GlobalArmor(50001));

	GlobalManager->SetFunds(500);
	_action_window.UpdateFinanceText();

	for (uint32 i = 0; i < _all_objects.size(); i++) {
		_list_window.AddEntry(_all_objects[i]->GetName(), _all_objects[i]->GetPrice());
	}
	_list_window.ConstructList();

	SoundDescriptor snd;
	_shop_sounds["confirm"] = SoundDescriptor();
	_shop_sounds["cancel"] = SoundDescriptor();
	_shop_sounds["coins"] = SoundDescriptor();
	_shop_sounds["bump"] = SoundDescriptor();

	_shop_sounds["confirm"].LoadSound("snd/confirm.wav");
	_shop_sounds["cancel"].LoadSound("snd/cancel.wav");
	_shop_sounds["coins"].LoadSound("snd/coins.wav");
	_shop_sounds["bump"].LoadSound("snd/bump.wav");
}



void ShopMode::Update() {
	switch (_state) {
		case SHOP_STATE_ACTION:
			_action_window.Update();
			break;
		case SHOP_STATE_LIST:
			_list_window.Update();
			break;
		case SHOP_STATE_CONFIRM:
			_confirm_window.Update();
			break;
		default:
			if (SHOP_DEBUG)
				cerr << "SHOP WARNING: invalid shop state: " << _state << ", reseting to initial state" << endl;
			_state = SHOP_STATE_ACTION;
			return;
	} // switch (shop_state)
} // void ShopMode::Update()



void ShopMode::Draw() {
	// Draw the background image
	// For that, set the system coordinates to the size of the window (same with the saved-screen)
	int32 width = VideoManager->GetWidth();
	int32 height = VideoManager->GetHeight();
	VideoManager->SetCoordSys (0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

	VideoManager->Move(0,0);
	VideoManager->DrawImage(_saved_screen);
	
	// Restore the Coordinate system (that one will be shop mode coodinate system?)
	VideoManager->SetCoordSys (0.0f, 1024.0f, 0.0f, 768.0f);

	_action_window.Draw();
	_info_window.Draw();
	_list_window.Draw();
	_confirm_window.Draw();
}



void ShopMode::AddObject(uint32 object_id) {
	if (object_id == 0 || object_id > 60000) {
		if (SHOP_DEBUG)
			cerr << "SHOP WARNING: attempted to add object with invalid id: " << object_id << endl;
		return;
	}

	if (_object_map.find(object_id) != _object_map.end()) {
		if (SHOP_DEBUG)
			cerr << "SHOP WARNING: attempted to add object that was already in the object list: " << object_id << endl;
		return;
	}

	_object_map.insert(make_pair(object_id, 0));
}

} // namespace hoa_shop
