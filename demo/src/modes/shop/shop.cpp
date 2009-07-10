///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
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

#include "audio.h"
#include "video.h"
#include "input.h"
#include "system.h"

#include "global.h"

#include "mode_manager.h"
#include "pause.h"

#include "shop.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_mode_manager;
using namespace hoa_shop::private_shop;
using namespace hoa_pause;

namespace hoa_shop {

bool SHOP_DEBUG = false;
ShopMode* private_shop::current_shop = NULL;



ShopMode::ShopMode() :
	_state(SHOP_STATE_ACTION),
	_deal_types(0),
	_purchases_cost(0),
	_sales_revenue(0),
	_action_window(NULL),
	_buy_window(NULL),
	_sell_window(NULL),
	_info_window(NULL),
	_confirm_window(NULL),
	_prompt_window(NULL)
{
	mode_type = MODE_MANAGER_SHOP_MODE;
	private_shop::current_shop = this;

	_action_window = new ShopActionWindow();
	_greeting_window = new ShopGreetingWindow();
	_buy_window = new BuyListWindow();
	_sell_window = new SellListWindow();
	_info_window = new ObjectInfoWindow();
	_confirm_window = new ConfirmWindow();
	_prompt_window = new PromptWindow();

	try {
		_saved_screen = VideoManager->CaptureScreen();
	}
	catch (Exception e) {
		IF_PRINT_WARNING(SHOP_DEBUG) << e.ToString() << endl;
	}
}



ShopMode::~ShopMode() {
	for (uint32 i = 0; i < _buy_objects.size(); i++) {
		delete(_buy_objects[i]);
	}
	_buy_objects.clear();

	delete _action_window;
	delete _greeting_window;
	delete _buy_window;
	delete _sell_window;
	delete _info_window;
	delete _confirm_window;
	delete _prompt_window;

	private_shop::current_shop = NULL;
}


// Called whenever ShopMode is put on top of the game mode stack
void ShopMode::Reset() {
	// Setup video engine constructs
	VideoManager->SetCoordSys(0, 1024, 0, 768);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->Text()->SetDefaultFont("default");
	VideoManager->Text()->SetDefaultTextColor(Color::white);

	_purchases_cost = 0;
	_sales_revenue = 0;

	_buy_objects_quantities.clear();
	for (uint32 i = 0; i < _buy_objects.size(); i++) {
		_buy_objects_quantities.push_back(0);
		switch (_buy_objects[i]->GetObjectType()) {
			case GLOBAL_OBJECT_ITEM:
				_deal_types |= DEALS_ITEMS;
				break;
			case GLOBAL_OBJECT_WEAPON:
				_deal_types |= DEALS_WEAPONS;
				break;
			case GLOBAL_OBJECT_HEAD_ARMOR:
				_deal_types |= DEALS_HEAD_ARMOR;
				break;
			case GLOBAL_OBJECT_TORSO_ARMOR:
				_deal_types |= DEALS_TORSO_ARMOR;
				break;
			case GLOBAL_OBJECT_ARM_ARMOR:
				_deal_types |= DEALS_ARM_ARMOR;
				break;
			case GLOBAL_OBJECT_LEG_ARMOR:
				_deal_types |= DEALS_LEG_ARMOR;
				break;
			case GLOBAL_OBJECT_SHARD:
				_deal_types |= DEALS_SHARDS;
				break;
			case GLOBAL_OBJECT_KEY_ITEM:
				_deal_types |= DEALS_KEY_ITEMS;
				break;
			default:
				IF_PRINT_WARNING(SHOP_DEBUG) << "unknown object type sold in shop: " << _buy_objects[i]->GetObjectType() << endl;
				break;
		}
	}

	if (ImageDescriptor::LoadMultiImageFromElementGrid(_object_category_images, "img/icons/object_categories.png", 2, 4) == false) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "failed to load category images: img/icons/object_categories.png" << endl;
		return;
	}
	_greeting_window->SetCategoryIcons(_deal_types, _object_category_images);

	map<uint32, GlobalObject*>* inv = GlobalManager->GetInventory();
	map<uint32, GlobalObject*>::iterator iter;

	_sell_objects_quantities.clear();
	for (iter = inv->begin(); iter != inv->end(); iter++) {
		_sell_objects_quantities.push_back(0);
	}

	_buy_window->RefreshList();
	_sell_window->UpdateSellList();

	if (_sell_window->object_list.GetNumberOptions() > 0) {
		_sell_window->object_list.SetSelection(0);
	}

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
	if (InputManager->QuitPress() == true) {
		ModeManager->Push(new PauseMode(true));
		return;
	}
	else if (InputManager->PausePress() == true) {
		ModeManager->Push(new PauseMode(false));
		return;
	}

	switch (_state) {
		case SHOP_STATE_ACTION:
			_action_window->Update();
			_greeting_window->Update();
			break;
		case SHOP_STATE_BUY:
			_buy_window->Update();
			break;
		case SHOP_STATE_SELL:
			_sell_window->Update();
			break;
		case SHOP_STATE_CONFIRM:
			_confirm_window->Update();
			break;
		case SHOP_STATE_PROMPT:
			_prompt_window->Update();
			break;
		default:
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid shop state: " << _state << ", reseting to initial state" << endl;
			_state = SHOP_STATE_ACTION;
			break;
	} // switch (shop_state)
}



void ShopMode::Draw() {
	// Draw the background image
	// Set the system coordinates to the size of the window (same with the saved-screen)
	int32 width = VideoManager->GetScreenWidth();
	int32 height = VideoManager->GetScreenHeight();
	VideoManager->SetCoordSys(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

	VideoManager->Move(0, 0);
	_saved_screen.Draw();

	// Restore the standard shop coordinate system before drawing the shop windows
	VideoManager->SetCoordSys(0.0f, 1024.0f, 0.0f, 768.0f);

	_action_window->Draw();
	_greeting_window->Draw();
// 	_info_window->Draw();

// 	// Determine if we should draw the buy window, sell window, or an empty menu window
// 	// NOTE: we should never see both buy and sell states in the state/saved_state member.
// 	if (_state == SHOP_STATE_BUY || _saved_state == SHOP_STATE_BUY) {
// 		_buy_window->Draw();
// 	}
// 	else if (_state == SHOP_STATE_SELL || _saved_state == SHOP_STATE_SELL) {
// 		_sell_window->Draw();
// 	}
// 	else {
// 		_buy_window->MenuWindow::Draw();
// 	}
//
// 	if (_state == SHOP_STATE_CONFIRM) {
// 		_confirm_window->Draw();
// 	}
// 	else if (_state == SHOP_STATE_PROMPT) {
// 		_prompt_window->Draw();
// 	}
}



void ShopMode::AddObject(uint32 object_id) {
	if (object_id == 0 || object_id > 60000) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "attempted to add object with invalid id: " << object_id << endl;
		return;
	}

	if (_object_map.find(object_id) != _object_map.end()) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "attempted to add object that was already in the object list: " << object_id << endl;
		return;
	}

	_object_map.insert(make_pair(object_id, 0));
	_buy_objects.push_back(GlobalCreateNewObject(object_id, 1));
}

} // namespace hoa_shop
