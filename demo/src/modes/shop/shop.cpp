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

	_purchases_cost = 0;
	_sales_revenue = 0;

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
	VideoManager->SetFont("default");
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->SetTextColor(Color::white);

	// Temporary code
	_all_objects.clear();
	_all_objects.push_back(new GlobalItem(1));
	_all_objects.push_back(new GlobalWeapon(10001));
	_all_objects.push_back(new GlobalWeapon(10002));
	_all_objects.push_back(new GlobalArmor(20002));
	_all_objects.push_back(new GlobalArmor(30002));
	_all_objects.push_back(new GlobalArmor(40002));
	_all_objects.push_back(new GlobalArmor(50001));
	// End temp code

	_all_objects_quantities.clear();
	for (uint32 ctr = 0; ctr < _all_objects.size(); ctr++) {
		_all_objects_quantities.push_back(0);
	}

	std::map<uint32, GlobalObject*>* inv = GlobalManager->GetInventory();
	std::map<uint32, GlobalObject*>::iterator iter;

	_sell_objects_quantities.clear();
	for (iter = inv->begin(); iter != inv->end(); iter++) {
		_sell_objects_quantities.push_back(0);
	}

	_action_window.UpdateFinanceText();

	_list_window.RefreshList();

	_sell_window.UpdateSellList();
	if (_sell_window.object_list.GetNumberOptions() > 0) {
		_sell_window.object_list.SetSelection(0);
	}

	SoundDescriptor snd;
	_shop_sounds["confirm"] = SoundDescriptor();
	_shop_sounds["cancel"] = SoundDescriptor();
	_shop_sounds["coins"] = SoundDescriptor();
	_shop_sounds["bump"] = SoundDescriptor();

	_shop_sounds["confirm"].LoadAudio("snd/confirm.wav");
	_shop_sounds["cancel"].LoadAudio("snd/cancel.wav");
	_shop_sounds["coins"].LoadAudio("snd/coins.wav");
	_shop_sounds["bump"].LoadAudio("snd/bump.wav");
}



void ShopMode::Update() {
	switch (_state) {
		case SHOP_STATE_ACTION:
			_action_window.Update();
			break;
		case SHOP_STATE_LIST:
			_list_window.Update();
			break;
		case SHOP_STATE_SELL:
			_sell_window.Update();
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
	int32 width = VideoManager->GetScreenWidth();
	int32 height = VideoManager->GetScreenHeight();
	VideoManager->SetCoordSys(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

	VideoManager->Move(0,0);
	_saved_screen.Draw();
	
	// Restore the Coordinate system (that one will be shop mode coodinate system?)
	VideoManager->SetCoordSys(0.0f, 1024.0f, 0.0f, 768.0f);

	_action_window.Draw();
	_info_window.Draw();
	_sell_window.Draw();
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

uint32 ShopMode::GetTotalRemaining() {
	return GlobalManager->GetDrunes() - _purchases_cost + _sales_revenue;
}

} // namespace hoa_shop
