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

	// ---------- (1): Create the menu windows and set their properties
	_top_window.Create(800.0f, 96.0f, ~VIDEO_MENU_EDGE_BOTTOM);
	_top_window.SetPosition(112.0f, 684.0f);
	_top_window.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_top_window.SetDisplayMode(VIDEO_MENU_INSTANT);
	_top_window.Show();

	_middle_window.Create(800.0f, 400.0f, VIDEO_MENU_EDGE_ALL, VIDEO_MENU_EDGE_TOP | VIDEO_MENU_EDGE_BOTTOM);
	_middle_window.SetPosition(112.0f, 604.0f);
	_middle_window.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_middle_window.SetDisplayMode(VIDEO_MENU_INSTANT);
	_middle_window.Show();

	_bottom_window.Create(800.0f, 140.0f, ~VIDEO_MENU_EDGE_TOP);
	_bottom_window.SetPosition(112.0f, 224.0f);
	_bottom_window.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_bottom_window.SetDisplayMode(VIDEO_MENU_INSTANT);
	_bottom_window.Show();

	// (2) Create the list of shop actions
	_action_options.SetOwner(&_top_window);
	_action_options.SetPosition(80.0f, 90.0f);
	_action_options.SetDimensions(640.0f, 30.0f, 5, 1, 5, 1);
	_action_options.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_TOP);
	_action_options.SetTextStyle(TextStyle("title28"));
	_action_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_action_options.SetCursorOffset(-55.0f, 30.0f);
	_action_options.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);

	_action_options.AddOption(MakeUnicodeString("Buy"));
	_action_options.AddOption(MakeUnicodeString("Sell"));
	_action_options.AddOption(MakeUnicodeString("Trade"));
	_action_options.AddOption(MakeUnicodeString("Confirm"));
	_action_options.AddOption(MakeUnicodeString("Leave"));
	_action_options.SetSelection(0);

	// (3) Create the financial table text
	_finance_table.SetOwner(&_top_window);
	_finance_table.SetPosition(80.0f, 45.0f);
	_finance_table.SetDimensions(640.0f, 20.0f, 4, 1, 4, 1);
	_finance_table.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_finance_table.SetTextStyle(TextStyle("text20"));
	_finance_table.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	// Initialize all four options with an empty string that will be overwritten by the following method call
	for (uint32 i = 0; i < 4; i++)
		_finance_table.AddOption(ustring());
	UpdateFinances(0, 0);

	// (4) Initialize the drunes icon image
	if (_drunes_icon.Load("img/icons/drunes.png") == false)
		IF_PRINT_WARNING(SHOP_DEBUG) << "failed to load drunes image for action window" << endl;

	_root_interface = new RootInterface();
	_buy_interface = new BuyInterface();
	_sell_interface = new SellInterface();
	_trade_interface = new TradeInterface();
	_confirm_interface = new ConfirmInterface();
	// TODO
	// _leave_interface = new LeaveInterface();

	try {
		_screen_backdrop = VideoManager->CaptureScreen();
	}
	catch (Exception e) {
		IF_PRINT_WARNING(SHOP_DEBUG) << e.ToString() << endl;
	}
}



ShopMode::~ShopMode() {
	for (uint32 i = 0; i < _managed_objects.size(); i++) {
		delete(_managed_objects[i]);
	}
	_managed_objects.clear();

	delete _root_interface;
	delete _buy_interface;
	delete _sell_interface;
	delete _trade_interface;
	delete _confirm_interface;

	_top_window.Destroy();
	_middle_window.Destroy();
	_bottom_window.Destroy();

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

	// When the shop state is at the root interface ShopMode needs to process user input and possibly change state
	if (_state == SHOP_STATE_ROOT) {
		SoundDescriptor* sound = NULL; // Used to hold pointers of sound objects to play

		if (InputManager->ConfirmPress()) {
			if (_action_options.GetSelection() < 0 || _action_options.GetSelection() > 4) {
				IF_PRINT_WARNING(SHOP_DEBUG) << "invalid selection in action window: " << _action_options.GetSelection() << endl;
				_action_options.SetSelection(0);
				return;
			}

			_action_options.InputConfirm();
			sound = ShopMode::CurrentInstance()->GetSound("confirm");
			assert(sound != NULL);
			sound->Play();

			if (_action_options.GetSelection() == 0) { // Buy
				ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_BUY);
			}
			else if (_action_options.GetSelection() == 1) { // Sell
				ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_SELL);
			}
			else if (_action_options.GetSelection() == 2) { // Trade
				ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_TRADE);
			}
			else if (_action_options.GetSelection() == 3) { // Confirm
				ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_CONFIRM);
			}
			else if (_action_options.GetSelection() == 4) { // Leave
				ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_LEAVE);
			}
		}
		else if (InputManager->LeftPress()) {
			_action_options.InputLeft();
		}
		else if (InputManager->RightPress()) {
			_action_options.InputRight();
		}
	} // if (_state == SHOP_STATE_ROOT)

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
// TODO
// 		case SHOP_STATE_LEAVE:
// 			_leave_interface->Update();
// 			break;
		default:
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid shop state: " << _state << ", reseting to root state" << endl;
			_state = SHOP_STATE_ROOT;
			break;
	} // switch (_state)
}



void ShopMode::Draw() {
	// ---------- (1): Draw the background image. Set the system coordinates to the size of the window (same as the screen backdrop)
	VideoManager->SetCoordSys(0.0f, static_cast<float>(VideoManager->GetScreenWidth()), 0.0f, static_cast<float>(VideoManager->GetScreenHeight()));
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->Move(0.0f, 0.0f);
	_screen_backdrop.Draw();

	// ---------- (2): Draw all menu windows
	VideoManager->SetCoordSys(0.0f, 1024.0f, 0.0f, 768.0f); // Restore the standard shop coordinate system before drawing the shop windows
	_top_window.Draw();
	_bottom_window.Draw();
	_middle_window.Draw(); // Drawn last because the middle window has the middle upper and lower window borders attached

	// ---------- (3): Draw the contents of the top window
	VideoManager->Move(130.0f, 605.0f);
	_drunes_icon.Draw();
	VideoManager->MoveRelative(705.0f, 0.0f);
	_drunes_icon.Draw();

	switch (_state) {
		case SHOP_STATE_ROOT:
			_action_options.Draw();
			break;
		case SHOP_STATE_BUY:
			_action_options.Draw();
			break;
		case SHOP_STATE_SELL:
			_action_options.Draw();
			break;
		case SHOP_STATE_TRADE:
			_action_options.Draw();
			break;
		case SHOP_STATE_CONFIRM:
			_action_options.Draw();
			break;
// TODO
// 		case SHOP_STATE_LEAVE:
// 			_action_options.Draw();
// 			break;
		default:
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid shop state: " << _state << endl;
			break;
	}

	// TODO: This method isn't working correctly (know idea why these coordinates work). When the call is fixed, the args should be updated
	VideoManager->DrawLine(-635.0f, 32.0f, -10.0f, 32.0f, 1.0f, Color::white);

	_finance_table.Draw();

	// ---------- (4): Call the draw function on the active interface to fill the contents of the other two windows
	switch (_state) {
		case SHOP_STATE_ROOT:
			_root_interface->Draw();
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
// TODO
// 		case SHOP_STATE_LEAVE:
// 			_leave_interface->Draw();
// 			break;
		default:
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid shop state: " << _state << endl;
			break;
	}
}



void ShopMode::SetShopName(ustring name) {
	if (IsInitialized() == true) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function called after shop was already initialized" << endl;
		return;
	}

	_root_interface->SetShopName(name);
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



void ShopMode::AddObject(uint32 object_id, uint32 stock) {
	if (IsInitialized() == true) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "shop is already initialized" << endl;
		return;
	}

	if (stock == 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "added an object with a zero stock count" << endl;
		return;
	}

	if (object_id == private_global::OBJECT_ID_INVALID || object_id >= private_global::OBJECT_ID_EXCEEDS) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "attempted to add object with invalid id: " << object_id << endl;
		return;
	}

	if (_shop_objects.find(object_id) != _shop_objects.end()) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "attempted to add object that already existed: " << object_id << endl;
		return;
	}

	GlobalObject* new_object = GlobalCreateNewObject(object_id, 1);
	_managed_objects.push_back(new_object);
	ShopObject new_shop_object(new_object, true);
	new_shop_object.IncrementStockCount(stock);
	_shop_objects.insert(make_pair(object_id, new_shop_object));
}



void ShopMode::RemoveObject(uint32 object_id) {
	map<uint32, ShopObject>::iterator shop_iter = _shop_objects.find(object_id);
	if (shop_iter == _shop_objects.end()) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "attempted to remove object that did not exist: " << object_id << endl;
		return;
	}

	if (shop_iter->second.IsSoldInShop() == true) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "tried to remove object that is sold in shop: " << object_id << endl;
		return;
	}

	if (shop_iter->second.GetOwnCount() != 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object's ownership count was non-zero: " << object_id << endl;
		return;
	}

	_shop_objects.erase(shop_iter);
	_sell_list.erase(object_id);

	// TODO: call the sell interface and inform it that the object has been removed
}



void ShopMode::Initialize() {
	if (IsInitialized() == true) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "shop was already initialized previously" << endl;
		return;
	}

	_initialized = true;

	// ---------- (1): Determine what types of objects the shop deals in based on the managed object list
	for (uint32 i = 0; i < _managed_objects.size(); i++) {
		switch (_managed_objects[i]->GetObjectType()) {
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
				IF_PRINT_WARNING(SHOP_DEBUG) << "unknown object type sold in shop: " << _managed_objects[i]->GetObjectType() << endl;
				break;
		}
	}

	// ---------- (2): Add objects from the player's inventory to the list of shop objects
	map<uint32, GlobalObject*>* inventory = GlobalManager->GetInventory();
	for (map<uint32, GlobalObject*>::iterator i = inventory->begin(); i != inventory->end(); i++) {
		// Check if the object already exists in the shop list and if so, set its ownership count
		map<uint32, ShopObject>::iterator shop_obj_iter = _shop_objects.find(i->second->GetID());
		if (shop_obj_iter != _shop_objects.end()) {
			shop_obj_iter->second.IncrementOwnCount(i->second->GetCount());
		}
		// Otherwise, add the shop object to the list
		else {
			ShopObject new_shop_object(i->second, false);
			_shop_objects.insert(make_pair(i->second->GetID(), new_shop_object));
		}
	}

	// ---------- (3): Initialize pricing for all shop objects
	for (map<uint32, ShopObject>::iterator i = _shop_objects.begin(); i != _shop_objects.end(); i++) {
		i->second.SetPricing(_buy_price_level, _sell_price_level);
	}

	// ---------- (4): Load shop multimedia data
	if (ImageDescriptor::LoadMultiImageFromElementGrid(_object_category_images, "img/icons/object_categories.png", 2, 4) == false) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "failed to load category image icons" << endl;
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

	// ---------- (5): Initialize all shop interfaces
	_root_interface->Initialize();
	_buy_interface->Initialize();
	_sell_interface->Initialize();
	_trade_interface->Initialize();
	_confirm_interface->Initialize();
} // void ShopMode::Initialize()



void ShopMode::AddObjectToBuyList(ShopObject* object) {
	if (object == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function was passed a NULL argument" << endl;
		return;
	}

	if (object->GetBuyCount() == 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be added had a buy count of zero" << endl;
	}

	uint32 object_id = object->GetObject()->GetID();
	pair<map<uint32, ShopObject*>::iterator, bool> ret_val;
	ret_val = _buy_list.insert(make_pair(object_id, object));
	if (ret_val.second == false) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be added already existed in buy list" << endl;
	}
}



void ShopMode::RemoveObjectFromBuyList(ShopObject* object) {
	if (object == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function was passed a NULL argument" << endl;
		return;
	}

	if (object->GetBuyCount() > 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be removed had a buy count that was non-zero" << endl;
	}

	uint32 object_id = object->GetObject()->GetID();
	map<uint32, ShopObject*>::iterator object_entry = _buy_list.find(object_id);
	if (object_entry == _buy_list.end()) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be removed did not exist on the buy list" << endl;
	}
	else {
		_buy_list.erase(object_entry);
	}
}



void ShopMode::AddObjectToSellList(ShopObject* object) {
	if (object == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function was passed a NULL argument" << endl;
		return;
	}

	if (object->GetSellCount() == 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be added had a sell count of zero" << endl;
	}

	uint32 object_id = object->GetObject()->GetID();
	pair<map<uint32, ShopObject*>::iterator, bool> ret_val;
	ret_val = _sell_list.insert(make_pair(object_id, object));
	if (ret_val.second == false) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be added already existed in sell list" << endl;
	}
}



void ShopMode::RemoveObjectFromSellList(ShopObject* object) {
	if (object == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function was passed a NULL argument" << endl;
		return;
	}

	if (object->GetSellCount() > 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be removed had a sell count that was non-zero" << endl;
	}

	uint32 object_id = object->GetObject()->GetID();
	map<uint32, ShopObject*>::iterator object_entry = _sell_list.find(object_id);
	if (object_entry == _sell_list.end()) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "object to be removed did not exist on the sell list" << endl;
	}
	else {
		_sell_list.erase(object_entry);
	}
}



void ShopMode::CompleteTransaction() {
	for (map<uint32, ShopObject*>:: iterator i = _buy_list.begin(); i != _buy_list.end(); i++) {
		// TODO
	}

	for (map<uint32, ShopObject*>:: iterator i = _sell_list.begin(); i != _sell_list.end(); i++) {
		// TODO
	}

	map<uint32, GlobalObject*>* inventory = GlobalManager->GetInventory();
	for (map<uint32, GlobalObject*>::iterator i = inventory->begin(); i != inventory->end(); i++) {
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

	_finance_table.SetOptionText(0, MakeUnicodeString("Funds: " + NumberToString(GlobalManager->GetDrunes())));
	_finance_table.SetOptionText(1, MakeUnicodeString("Purchases: -" + NumberToString(ShopMode::CurrentInstance()->GetTotalCosts())));
	_finance_table.SetOptionText(2, MakeUnicodeString("Sales: +" + NumberToString(ShopMode::CurrentInstance()->GetTotalSales())));
	_finance_table.SetOptionText(3, MakeUnicodeString("Total: " + NumberToString(ShopMode::CurrentInstance()->GetTotalRemaining())));
}



void ShopMode::ChangeState(SHOP_STATE new_state) {
	if (_state == new_state) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "shop was already in the state to change to: " << _state << endl;
		return;
	}

	_state = new_state;

	// TEMP: shop mode should conditionally exit on leave
	if (_state == SHOP_STATE_LEAVE) {
		ModeManager->Pop();
	}
} // void ShopMode::ChangeState(SHOP_STATE new_state)



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
