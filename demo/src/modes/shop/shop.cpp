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
#include "shop_root.h"
#include "shop_buy.h"
#include "shop_sell.h"
#include "shop_trade.h"
#include "shop_confirm.h"

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
// Initialize static class variables
ShopMode* ShopMode::_current_instance = NULL;


ShopMode::ShopMode() :
	_initialized(false),
	_state(SHOP_STATE_ROOT),
	_deal_types(0),
	_buy_price_level(SHOP_PRICE_STANDARD),
	_sell_price_level(SHOP_PRICE_STANDARD),
	_total_costs(0),
	_total_sales(0),
	_root_interface(NULL),
	_buy_interface(NULL),
	_sell_interface(NULL),
	_trade_interface(NULL),
	_confirm_interface(NULL)
{
	mode_type = MODE_MANAGER_SHOP_MODE;
	_current_instance = this;

	_root_interface = new ShopRootInterface();
	_buy_interface = new ShopBuyInterface();
	_sell_interface = new ShopSellInterface();
	_trade_interface = new ShopTradeInterface();
	_confirm_interface = new ShopConfirmInterface();

	try {
		_saved_screen = VideoManager->CaptureScreen();
	}
	catch (Exception e) {
		IF_PRINT_WARNING(SHOP_DEBUG) << e.ToString() << endl;
	}
}



ShopMode::~ShopMode() {
	for (uint32 i = 0; i < _created_objects.size(); i++) {
		delete(_created_objects[i]);
	}
	_created_objects.clear();

	delete _root_interface;
	delete _buy_interface;
	delete _sell_interface;
	delete _trade_interface;
	delete _confirm_interface;

	if (_current_instance == this)
		_current_instance = NULL;
}



void ShopMode::Reset() {
	// Setup video engine constructs
	VideoManager->SetCoordSys(0.0f, 1024.0f, 0.0f, 768.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->Text()->SetDefaultFont("default");
	VideoManager->Text()->SetDefaultTextColor(Color::white);

	_current_instance = this;
	if (IsInitialized() == false)
		Initialize();
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
		case SHOP_STATE_ROOT:
			_root_interface->Update();
			break;
		case SHOP_STATE_BUY:
			_buy_interface->Update();
			break;
		case SHOP_STATE_SELL:
			_sell_interface->Update();
			break;
		case SHOP_STATE_TRADE:
			_trade_interface->Update();
			break;
		case SHOP_STATE_CONFIRM:
			_confirm_interface->Update();
			break;
		default:
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid shop state: " << _state << ", reseting to root state" << endl;
			_state = SHOP_STATE_ROOT;
			break;
	} // switch (_state)
}



void ShopMode::Draw() {
	// Draw the background image
	// Set the system coordinates to the size of the window (same with the saved-screen)
	int32 width = VideoManager->GetScreenWidth();
	int32 height = VideoManager->GetScreenHeight();
	VideoManager->SetCoordSys(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

	VideoManager->Move(0.0f, 0.0f);
	_saved_screen.Draw();

	// Restore the standard shop coordinate system before drawing the shop windows
	VideoManager->SetCoordSys(0.0f, 1024.0f, 0.0f, 768.0f);

	// The root interface is always drawn regardless of the shop's state
	_root_interface->Draw();
	switch (_state) {
		case SHOP_STATE_ROOT:
			break;
		case SHOP_STATE_BUY:
			_buy_interface->Draw();
			break;
		case SHOP_STATE_SELL:
			_sell_interface->Draw();
			break;
		case SHOP_STATE_TRADE:
			_trade_interface->Draw();
			break;
		case SHOP_STATE_CONFIRM:
			_confirm_interface->Draw();
			break;
		default:
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid shop state: " << _state << endl;
			break;
	}
}



void ShopMode::SetGreetingText(ustring greeting) {
	if (IsInitialized() == true) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function called after shop was already initialized" << endl;
		return;
	}

	_root_interface->SetGreetingText(greeting);
}



void ShopMode::SetPriceLevels(SHOP_PRICE_LEVEL buy_level, SHOP_PRICE_LEVEL sell_level) {
	if (IsInitialized() == true) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function called after shop was already initialized" << endl;
		return;
	}

	_buy_price_level = buy_level;
	_sell_price_level = sell_level;
}



void ShopMode::AddObject(uint32 object_id) {
	if (IsInitialized() == true) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "shop is already initialized" << endl;
		return;
	}

	if (object_id == private_global::OBJECT_ID_INVALID || object_id >= private_global::OBJECT_ID_EXCEEDS) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "attempted to add object with invalid id: " << object_id << endl;
		return;
	}

	if (_shop_objects.find(object_id) != _shop_objects.end()) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "attempted to add object that was already in the object list: " << object_id << endl;
		return;
	}

	GlobalObject* new_object = GlobalCreateNewObject(object_id, 1);
	_created_objects.push_back(new_object);
	ShopObject new_shop_object(new_object, true);
	_shop_objects.insert(make_pair(object_id, new_shop_object));
}



void ShopMode::Initialize() {
	if (IsInitialized() == true) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "shop was already initialized previously" << endl;
		return;
	}

	_initialized = true;

	for (uint32 i = 0; i < _created_objects.size(); i++) {
		switch (_created_objects[i]->GetObjectType()) {
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
				IF_PRINT_WARNING(SHOP_DEBUG) << "unknown object type sold in shop: " << _created_objects[i]->GetObjectType() << endl;
				break;
		}
	}

	if (ImageDescriptor::LoadMultiImageFromElementGrid(_object_category_images, "img/icons/object_categories.png", 2, 4) == false) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "failed to load category images: img/icons/object_categories.png" << endl;
		return;
	}

	_shop_sounds["confirm"] = new SoundDescriptor();
	_shop_sounds["cancel"] = new SoundDescriptor();
	_shop_sounds["coins"] = new SoundDescriptor();
	_shop_sounds["bump"] = new SoundDescriptor();

	_shop_sounds["confirm"]->LoadAudio("snd/confirm.wav");
	_shop_sounds["cancel"]->LoadAudio("snd/cancel.wav");
	_shop_sounds["coins"]->LoadAudio("snd/coins.wav");
	_shop_sounds["bump"]->LoadAudio("snd/bump.wav");

	_root_interface->Initialize();
	_buy_interface->Initialize();
	_sell_interface->Initialize();
	_trade_interface->Initialize();
	_confirm_interface->Initialize();
}



void ShopMode::CompleteTransaction() {
	for (map<uint32, ShopObject*>:: iterator i = _buy_objects.begin(); i != _buy_objects.end(); i++) {
		// TODO
	}

	for (map<uint32, ShopObject*>:: iterator i = _sell_objects.begin(); i != _sell_objects.end(); i++) {
		// TODO
	}

	map<uint32, GlobalObject*>* inv = GlobalManager->GetInventory();
	map<uint32, GlobalObject*>::iterator iter;
	for (iter = inv->begin(); iter != inv->end(); iter++) {
		// TODO
	}
}



void ShopMode::UpdateFinances(int32 costs_amount, int32 sales_amount) {
	int32 updated_costs = _total_costs + costs_amount;
	int32 updated_sales = _total_sales + sales_amount;

	if (updated_costs < 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "updated amount causes costs to become negative: " << costs_amount << endl;
		return;
	}
	if (updated_sales < 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "updated amount causes sales to become negative: " << sales_amount << endl;
		return;
	}
	if ((static_cast<int32>(GlobalManager->GetDrunes()) + updated_sales - updated_costs) < 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "updated costs and sales values cause negative balance: " << costs_amount << ", " << sales_amount << endl;
		return;
	}

	_total_costs = static_cast<uint32>(updated_costs);
	_total_sales = static_cast<uint32>(updated_sales);
	_root_interface->UpdateFinanceTable();
}


void ShopMode::ChangeState(SHOP_STATE new_state) {
	_state = new_state;
}



SoundDescriptor* ShopMode::GetSound(string identifier) {
	map<string, SoundDescriptor*>::iterator sound = _shop_sounds.find(identifier);
	if (sound != _shop_sounds.end()) {
		return sound->second;
	}
	else {
		return NULL;
	}
}

} // namespace hoa_shop
