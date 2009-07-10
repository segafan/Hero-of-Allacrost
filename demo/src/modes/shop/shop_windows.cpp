///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>

#include "utils.h"

#include "audio.h"
#include "video.h"
#include "input.h"
#include "system.h"

#include "global.h"

#include "mode_manager.h"
#include "menu.h"
#include "shop.h"
#include "shop_windows.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_global;
using namespace hoa_mode_manager;

namespace hoa_shop {

namespace private_shop {

// *****************************************************************************
// ***** ShopActionWindow
// *****************************************************************************

ShopActionWindow::ShopActionWindow() {
	// (1) Initialize the window
	MenuWindow::Create(800, 80, ~VIDEO_MENU_EDGE_BOTTOM);
	MenuWindow::SetPosition(112, 684);
	MenuWindow::SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);
	MenuWindow::Show();

	// (2) Initialize the list of actions
	action_options.SetOwner(this);
	action_options.SetPosition(40.0f, 60.0f);
	action_options.SetDimensions(720.0f, 20.0f, 5, 1, 5, 1);
	action_options.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	action_options.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	action_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	action_options.SetCursorOffset(-50.0f, 20.0f);
	action_options.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);

	action_options.AddOption(MakeUnicodeString("Buy"));
	action_options.AddOption(MakeUnicodeString("Sell"));
	action_options.AddOption(MakeUnicodeString("Trade"));
	action_options.AddOption(MakeUnicodeString("Confirm"));
	action_options.AddOption(MakeUnicodeString("Leave"));
	action_options.SetSelection(0);

	// (3) Initialize the financial table text
	finance_table.SetOwner(this);
	finance_table.SetPosition(80.0f, 30.0f);
	finance_table.SetDimensions(680.0f, 20.0f, 4, 1, 4, 1);
	finance_table.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	finance_table.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	finance_table.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	// Initialize all four options with an empty string that will be overwritten by the following method call
	for (uint32 i = 0; i < 4; i++)
		finance_table.AddOption(MakeUnicodeString(""));
	UpdateFinanceTable();

	// (4) Initialize the drunes icon image
	if (drunes_icon.Load("img/icons/drunes.png") == false)
		IF_PRINT_WARNING(SHOP_DEBUG) << "failed to load drunes image for action window" << endl;
	drunes_icon.SetDimensions(30.0f, 30.0f);
} // ShopActionWindow::ShopActionWindow()



ShopActionWindow::~ShopActionWindow() {
	MenuWindow::Destroy();
}



void ShopActionWindow::Update() {
	MenuWindow::Update(SystemManager->GetUpdateTime());
	action_options.Update(); // Clear any OptionBox events, since they prevent further user input

	if (InputManager->ConfirmPress()) {
		action_options.InputConfirm();
		if (action_options.GetSelection() == 0) { // Buy
			action_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			current_shop->_info_window->SetObject(current_shop->_buy_objects[0]);
			current_shop->_buy_window->hide_options = false;
			current_shop->_buy_window->object_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			current_shop->_state = SHOP_STATE_BUY;
			current_shop->_shop_sounds["confirm"].Play();
		}
		else if (action_options.GetSelection() == 1) { // Sell
			// TODO: Inventory empty check is not sufficient here because if the inventory only consists
			// of key items, then the sell menu should not come up
			if (GlobalManager->GetInventory()->empty() == true) {
				current_shop->_PushAndSetState(SHOP_STATE_PROMPT);
				current_shop->_prompt_window->Show();
				current_shop->_prompt_window->prompt_text.SetDisplayText(MakeUnicodeString(
					"The party's inventory is empty. There is nothing you can offer to sell.")
				);
				current_shop->_shop_sounds["cancel"].Play();
			}
			else {
				action_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				current_shop->_info_window->SetObject(current_shop->_current_inv[0]);
				current_shop->_sell_window->UpdateSellList();
				current_shop->_sell_window->object_list.SetSelection(0);
				current_shop->_sell_window->hide_options = false;
				current_shop->_sell_window->object_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				current_shop->_state = SHOP_STATE_SELL;
				current_shop->_shop_sounds["confirm"].Play();
			}
		}
		else if (action_options.GetSelection() == 2) { // Trade
			// TODO
		}
		else if (action_options.GetSelection() == 3) { // Confirm
			if (current_shop->HasPreparedTransaction() == false) {
				current_shop->_PushAndSetState(SHOP_STATE_PROMPT);
				current_shop->_prompt_window->Show();
				current_shop->_prompt_window->prompt_text.SetDisplayText(MakeUnicodeString(
					"You have not offered to make any purchases or sales.")
				);
				current_shop->_shop_sounds["cancel"].Play();
			}
			else {
				action_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
				current_shop->_state = SHOP_STATE_CONFIRM;
				current_shop->_confirm_window->options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				current_shop->_confirm_window->Show();
			}
		}
		else if (action_options.GetSelection() == 4) { // Leave shop
			ModeManager->Pop();
			current_shop->_shop_sounds["cancel"].Play();
		}
		else {
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid selection in action window: " << action_options.GetSelection() << endl;
			ModeManager->Pop();
		}
	}
	else if (InputManager->CancelPress()) {
		ModeManager->Pop();
		current_shop->_shop_sounds["cancel"].Play();
	}
	else if (InputManager->LeftPress()) {
		action_options.InputLeft();
	}
	else if (InputManager->RightPress()) {
		action_options.InputRight();
	}
}



void ShopActionWindow::UpdateFinanceTable() {
	finance_table.SetOptionText(0, MakeUnicodeString("Funds: " + NumberToString(GlobalManager->GetDrunes())));
	finance_table.SetOptionText(1, MakeUnicodeString("Purchases: -" + NumberToString(current_shop->GetPurchaseCost())));
	finance_table.SetOptionText(2, MakeUnicodeString("Sales: +" + NumberToString(current_shop->GetSalesRevenue())));
	finance_table.SetOptionText(3, MakeUnicodeString("Total: " + NumberToString(current_shop->GetTotalRemaining())));
}



void ShopActionWindow::Draw() {
	MenuWindow::Draw();
	action_options.Draw();
	finance_table.Draw();
	VideoManager->Move(150.0f, 610.0f);
	drunes_icon.Draw();
}

// *****************************************************************************
// ***** ShopGreetingWindow class methods
// *****************************************************************************

ShopGreetingWindow::ShopGreetingWindow() {
	// (1) Initialize the window
	MenuWindow::Create(800, 200, VIDEO_MENU_EDGE_ALL, VIDEO_MENU_EDGE_TOP);
	MenuWindow::SetPosition(112, 612);
	MenuWindow::SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);
	MenuWindow::Show();

	// (2) Initialize the greeting textbox
	greeting_text.SetOwner(this);
	greeting_text.SetPosition(40.0f, 190.0f);
	greeting_text.SetDimensions(720.0f, 25.0f);
	greeting_text.SetTextStyle(TextStyle());
	greeting_text.SetDisplaySpeed(30);
	greeting_text.SetDisplayMode(VIDEO_TEXT_INSTANT);
	greeting_text.SetTextAlignment(VIDEO_X_CENTER, VIDEO_Y_TOP);
	greeting_text.SetDisplayText(MakeUnicodeString("Welcome! Take a look around.")); // Default greeting, will usually be overwritten

	// (3) Initialize the price level textbox
	pricing_text.SetOwner(this);
	pricing_text.SetPosition(40.0f, 65.0f);
	pricing_text.SetDimensions(720.0f, 50.0f);
	pricing_text.SetTextStyle(TextStyle());
	pricing_text.SetDisplaySpeed(30);
	pricing_text.SetDisplayMode(VIDEO_TEXT_INSTANT);
	pricing_text.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	SetPriceLevelText(SHOP_PRICE_STANDARD, SHOP_PRICE_STANDARD); // Default price levels, will usually be overwritten
} // ShopActionWindow::ShopActionWindow()



ShopGreetingWindow::~ShopGreetingWindow() {
	MenuWindow::Destroy();
} // ShopActionWindow::ShopActionWindow()



void ShopGreetingWindow::Update() {
	MenuWindow::Update();
	greeting_text.Update();
	pricing_text.Update();
}



void ShopGreetingWindow::Draw() {
	MenuWindow::Draw();
	greeting_text.Draw();
	pricing_text.Draw();
	VideoManager->Move(200.0f, 500.0f);
	for (uint32 i = 0; i < category_icons.size(); i++) {
		category_icons[i].Draw();
		VideoManager->MoveRelative(80.0f, 0.0f);
	}
}



void ShopGreetingWindow::SetGreetingText(hoa_utils::ustring greeting) {
	greeting_text.SetDisplayText(greeting);
}



void ShopGreetingWindow::SetPriceLevelText(SHOP_PRICE_LEVEL buy_level, SHOP_PRICE_LEVEL sell_level) {
	const ustring VERY_HIGH = MakeUnicodeString("very high");
	const ustring HIGH = MakeUnicodeString("high");
	const ustring STANDARD = MakeUnicodeString("standard");
	const ustring LOW = MakeUnicodeString("low");
	const ustring VERY_LOW = MakeUnicodeString("very low");

	ustring buy_price, sell_price;
	switch (buy_level) {
		case SHOP_PRICE_VERY_HIGH:
			buy_price = VERY_HIGH;
			break;
		case SHOP_PRICE_HIGH:
			buy_price = HIGH;
			break;
		case SHOP_PRICE_STANDARD:
			buy_price = STANDARD;
			break;
		case SHOP_PRICE_LOW:
			buy_price = LOW;
			break;
		case SHOP_PRICE_VERY_LOW:
			buy_price = VERY_LOW;
			break;
		default:
			buy_price = STANDARD;
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid buy_level argument: " << buy_level << endl;
			break;
	}

	switch (sell_level) {
		case SHOP_PRICE_VERY_HIGH:
			sell_price = VERY_HIGH;
			break;
		case SHOP_PRICE_HIGH:
			sell_price = HIGH;
			break;
		case SHOP_PRICE_STANDARD:
			sell_price = STANDARD;
			break;
		case SHOP_PRICE_LOW:
			sell_price = LOW;
			break;
		case SHOP_PRICE_VERY_LOW:
			sell_price = VERY_LOW;
			break;
		default:
			sell_price = STANDARD;
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid sell_level argument: " << sell_level << endl;
			break;
	}

	pricing_text.SetDisplayText(MakeUnicodeString("Merchan't buy prices are ") + buy_price + MakeUnicodeString(".\nMerchant's sell prices are ") + sell_price + MakeUnicodeString("."));
} // void ShopGreetingWindow::SetPriceLevelText(SHOP_PRICE_LEVEL buy_level, SHOP_PRICE_LEVEL sell_level)



void ShopGreetingWindow::SetCategoryIcons(uint8 deal_types, vector<StillImage>& deal_images) {
	category_icons = deal_images;

	if ((deal_types & DEALS_ITEMS) == false) {
		category_icons[0].EnableGrayScale();
	}
	if ((deal_types & DEALS_WEAPONS) == false) {
		category_icons[1].EnableGrayScale();
	}
	if ((deal_types & DEALS_HEAD_ARMOR) == false) {
		category_icons[2].EnableGrayScale();
	}
	if ((deal_types & DEALS_TORSO_ARMOR) == false) {
		category_icons[3].EnableGrayScale();
	}
	if ((deal_types & DEALS_ARM_ARMOR) == false) {
		category_icons[4].EnableGrayScale();
	}
	if ((deal_types & DEALS_LEG_ARMOR) == false) {
		category_icons[5].EnableGrayScale();
	}
	if ((deal_types & DEALS_SHARDS) == false) {
		category_icons[6].EnableGrayScale();
	}
	if ((deal_types & DEALS_KEY_ITEMS) == false) {
		category_icons[7].EnableGrayScale();
	}
}

// *****************************************************************************
// ***** BuyListWindow
// *****************************************************************************

BuyListWindow::BuyListWindow() {
	MenuWindow::Create(800, 400, VIDEO_MENU_EDGE_LEFT | VIDEO_MENU_EDGE_RIGHT, VIDEO_MENU_EDGE_TOP | VIDEO_MENU_EDGE_BOTTOM);
	MenuWindow::SetPosition(112, 584);
	MenuWindow::SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);
	MenuWindow::Show();

	object_list.SetOwner(this);
	object_list.SetPosition(35.0f, 362.0f);
	object_list.SetDimensions(500.0f, 360.0f, 1, 255, 1, 6);
	object_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	object_list.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	object_list.SetSelectMode(VIDEO_SELECT_SINGLE);
	object_list.SetCursorOffset(-50.0f, 20.0f);
	object_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_NONE);

	hide_options = true;
}



BuyListWindow::~BuyListWindow() {
	MenuWindow::Destroy();
}



void BuyListWindow::AddEntry(hoa_utils::ustring name, uint32 price, uint32 quantity) {
	object_list.AddOption(name + MakeUnicodeString("<R>") + MakeUnicodeString(NumberToString(price)) +
		MakeUnicodeString("   x") + MakeUnicodeString(NumberToString(quantity))
	);
}



void BuyListWindow::RefreshList() {
	object_list.ClearOptions();
	for (uint32 i = 0; i < current_shop->_buy_objects.size(); i++) {
		AddEntry(current_shop->_buy_objects[i]->GetName(), current_shop->_buy_objects[i]->GetPrice(),
			current_shop->_buy_objects_quantities[i]
		);
	}
// 	object_list.SetSize(1, object_list.GetNumberOptions());
	object_list.SetDimensions(500.0f, 360.0f, 1, 255, 1, 6);
	object_list.SetSelection(0);
}



void BuyListWindow::Update() {
	MenuWindow::Update(SystemManager->GetUpdateTime());
	object_list.Update(); // Clear any OptionBox events, since they prevent further user input

	if (InputManager->ConfirmPress()) {
		object_list.InputConfirm();

		int32 x = object_list.GetSelection();
		if (current_shop->_buy_objects_quantities[x] == 0) {
			current_shop->_PushAndSetState(SHOP_STATE_PROMPT);
			current_shop->_prompt_window->Show();
			current_shop->_prompt_window->prompt_text.SetDisplayText(MakeUnicodeString(
				"No quantity for this selection was made. Use the right and left commands to increment "
				"or decrement the amount of this object to purchase.")
			);
		}
		else {

		current_shop->_state = SHOP_STATE_CONFIRM;
		object_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		current_shop->_info_window->SetObject(NULL);
		current_shop->_confirm_window->options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		current_shop->_confirm_window->Show();
		current_shop->_shop_sounds["confirm"].Play();
		}
	}
	else if (InputManager->CancelPress()) {
		hide_options = true;
		current_shop->_state = SHOP_STATE_ACTION;
		current_shop->_action_window->action_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		object_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		current_shop->_info_window->SetObject(NULL);
		current_shop->_shop_sounds["cancel"].Play();
	}
	else if (InputManager->UpPress()) {
		object_list.InputUp();
		current_shop->_info_window->SetObject(current_shop->_buy_objects[object_list.GetSelection()]);
	}
	else if (InputManager->DownPress()) {
		object_list.InputDown();
		current_shop->_info_window->SetObject(current_shop->_buy_objects[object_list.GetSelection()]);
	}
	else if (InputManager->LeftPress()) {
		int32 x = object_list.GetSelection();
		if (current_shop->_buy_objects_quantities[x] > 0) {
			current_shop->_buy_objects_quantities[x]--;
			current_shop->_purchases_cost -= current_shop->_buy_objects[x]->GetPrice();
			object_list.SetOptionText(x, current_shop->_buy_objects[x]->GetName() + MakeUnicodeString("<R>") +
				MakeUnicodeString(NumberToString(current_shop->_buy_objects[x]->GetPrice())) + MakeUnicodeString("   x") +
				MakeUnicodeString(NumberToString(current_shop->_buy_objects_quantities[x]))
			);
			current_shop->_action_window->UpdateFinanceTable();
		}
		else {
			current_shop->_shop_sounds["cancel"].Play();
		}
	}
	else if (InputManager->RightPress()) {
		int32 x = object_list.GetSelection();
		if (current_shop->_buy_objects[x]->GetPrice() <= current_shop->GetTotalRemaining()) {
			current_shop->_buy_objects_quantities[x]++;
			current_shop->_purchases_cost += current_shop->_buy_objects[x]->GetPrice();
			current_shop->_action_window->UpdateFinanceTable();
			object_list.SetOptionText(x, current_shop->_buy_objects[x]->GetName() + MakeUnicodeString("<R>") +
				MakeUnicodeString(NumberToString(current_shop->_buy_objects[x]->GetPrice())) + MakeUnicodeString("   x") +
				MakeUnicodeString(NumberToString(current_shop->_buy_objects_quantities[x]))
			);
		}
		else {
			current_shop->_shop_sounds["cancel"].Play();
		}
	}
} // void BuyListWindow::Update()



void BuyListWindow::Draw() {
	MenuWindow::Draw();

	if (hide_options == false && object_list.GetNumberOptions() > 0) {
		object_list.Draw();
	}
	else {
		cout << "BuyList object_list not drawn!" << endl;
	}
}

// *****************************************************************************
// ***** SellListWindow
// *****************************************************************************

SellListWindow::SellListWindow() {
	MenuWindow::Create(800, 400, VIDEO_MENU_EDGE_LEFT | VIDEO_MENU_EDGE_RIGHT, VIDEO_MENU_EDGE_TOP | VIDEO_MENU_EDGE_BOTTOM);
	MenuWindow::SetPosition(112, 584);
	MenuWindow::SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);
	MenuWindow::Show();

	object_list.SetOwner(this);
	object_list.SetPosition(50.0f, 350.0f);
	object_list.SetDimensions(500.0f, 300.0f, 1, 6, 1, 6);
	object_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	object_list.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	object_list.SetSelectMode(VIDEO_SELECT_SINGLE);
	object_list.SetCursorOffset(-50.0f, 20.0f);
	object_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
	object_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_NONE);

	hide_options = true;
}



SellListWindow::~SellListWindow() {
	MenuWindow::Destroy();
}



void SellListWindow::Clear() {
	option_text.clear();
	current_shop->_current_inv.clear();
	object_list.SetOptions(option_text);
}



void SellListWindow::AddEntry(hoa_utils::ustring name, uint32 count, uint32 price, uint32 sell_count) {
	string text = MakeStandardString(name) + "<R>" + NumberToString(count) + "      x" + NumberToString(sell_count) + "       " + NumberToString(price);
	option_text.push_back(MakeUnicodeString(text));
}



void SellListWindow::Update() {
	MenuWindow::Update(SystemManager->GetUpdateTime());
	object_list.Update(); // Clear any OptionBox events, since they prevent further user input

	if (InputManager->ConfirmPress()) {
		object_list.InputConfirm();
		int32 x = object_list.GetSelection();

		if (current_shop->_sell_objects_quantities[x] == 0) {
			current_shop->_PushAndSetState(SHOP_STATE_PROMPT);
			current_shop->_prompt_window->Show();
			current_shop->_prompt_window->prompt_text.SetDisplayText(MakeUnicodeString(
				"No quantity for this selection was made. Use the right and left commands to increment "
				"or decrement the amount of this object to offer for sale.")
			);
		}
		else {
			hide_options = true;
			object_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			current_shop->_state = SHOP_STATE_CONFIRM;
			current_shop->_confirm_window->options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			current_shop->_confirm_window->Show();
			current_shop->_shop_sounds["confirm"].Play();
		}
	}
	else if (InputManager->CancelPress()) {
		hide_options = true;
		current_shop->_state = SHOP_STATE_ACTION;
		current_shop->_action_window->action_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		object_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		current_shop->_info_window->SetObject(NULL);
		current_shop->_shop_sounds["cancel"].Play();
	}
	else if (InputManager->UpPress()) {
		object_list.InputUp();
		current_shop->_info_window->SetObject(current_shop->_current_inv[object_list.GetSelection()]);
	}
	else if (InputManager->DownPress()) {
		object_list.InputDown();
		current_shop->_info_window->SetObject(current_shop->_current_inv[object_list.GetSelection()]);
	}
	else if (InputManager->LeftPress()) {
		int32 x = object_list.GetSelection();
		if (current_shop->_sell_objects_quantities[x] > 0) {
			current_shop->_sell_objects_quantities[x]--;
			current_shop->_sales_revenue -= (current_shop->_current_inv[x]->GetPrice() / 2);
			current_shop->_action_window->UpdateFinanceTable();
		}
		else {
			current_shop->_shop_sounds["cancel"].Play();
		}
	}
	else if (InputManager->RightPress()) {
		uint32 x = object_list.GetSelection();
		if (current_shop->_current_inv[x]->GetCount() > static_cast<uint32>(current_shop->_sell_objects_quantities[x])) {
			current_shop->_sell_objects_quantities[x]++;
			current_shop->_sales_revenue += (current_shop->_current_inv[x]->GetPrice() / 2);
			current_shop->_action_window->UpdateFinanceTable();
		}
		else {
			current_shop->_shop_sounds["cancel"].Play();
		}
	}

	UpdateSellList();
}



void SellListWindow::UpdateSellList() {
	Clear();
	map<uint32, GlobalObject*>* inv = GlobalManager->GetInventory();
	map<uint32, GlobalObject*>::iterator iter;

	uint32 x = 0;
	for (iter = inv->begin(); iter != inv->end(); iter++) {
		current_shop->_current_inv.push_back(iter->second);
		AddEntry(iter->second->GetName(), iter->second->GetCount(), iter->second->GetPrice() / 2, current_shop->_sell_objects_quantities[x]);
		x++;
	}
	object_list.SetOptions(option_text);
}



void SellListWindow::Draw() {
	MenuWindow::Draw();

	if (hide_options == false && object_list.GetNumberOptions() != 0) {
		object_list.Draw();
		VideoManager->Move(375, 640);
		VideoManager->Text()->Draw(MakeUnicodeString("Item                                                                     Inv   Sell   Price"));
	}
}

// *****************************************************************************
// ***** ObjectInfoWindow
// ***************************************************************************

ObjectInfoWindow::ObjectInfoWindow() {
	_is_weapon = false;
	_is_armor  = false;

	// (1) Create the info window in the bottom right-hand section of the screen
	MenuWindow::Create(800, 300, ~VIDEO_MENU_EDGE_TOP);
	MenuWindow::SetPosition(112, 184);
	MenuWindow::SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);
	MenuWindow::Show();

	// (2) Initialize the object to NULL, so that no information is displayed
	_object = NULL;

	// (3) Initialize the description text box in the lower section of the window
	description.SetOwner(this);
	description.SetPosition(25.0f, 150.0f);
	description.SetDimensions(550.0f, 80.0f);
	description.SetDisplaySpeed(30);
	description.SetTextStyle(TextStyle());
	description.SetDisplayMode(VIDEO_TEXT_INSTANT);
	description.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);

	// (4) Initialize the properties text box in the upper right section of the window
	properties.SetOwner(this);
	properties.SetPosition(450.0f, 217.0f);
	properties.SetDimensions(300.0f, 80.0f);
	properties.SetDisplaySpeed(30);
	properties.SetTextStyle(TextStyle());
	properties.SetDisplayMode(VIDEO_TEXT_INSTANT);
	properties.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);

	// (5) Load character icons
	_LoadCharacterIcons();
}



ObjectInfoWindow::~ObjectInfoWindow() {
	MenuWindow::Destroy();
}



void ObjectInfoWindow::SetObject(GlobalObject* obj) {
	_object = obj;
	_is_weapon = false;
	_is_armor  = false;

	_usableBy.clear();
	_statVariance.clear();
	_metaVariance.clear();

	if (obj == NULL) {
		description.ClearText();
		properties.ClearText();
		return;
	}

	if (obj->GetObjectType() == GLOBAL_OBJECT_WEAPON ||
	    obj->GetObjectType() == GLOBAL_OBJECT_HEAD_ARMOR ||
	    obj->GetObjectType() == GLOBAL_OBJECT_TORSO_ARMOR ||
	    obj->GetObjectType() == GLOBAL_OBJECT_ARM_ARMOR ||
	    obj->GetObjectType() == GLOBAL_OBJECT_LEG_ARMOR) {
		uint32 partysize = GlobalManager->GetActiveParty()->GetPartySize();
		GlobalCharacter* ch;

		if(obj->GetObjectType() == GLOBAL_OBJECT_WEAPON) {
			_is_weapon = true;
		}
		else {
			_is_armor = true;
		}

		for(uint32 i = 0; i < partysize; i++) {
			ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(i));

			// If the currently selected character can equip this weapon, calculate the +/- effects the weapon/armor has on stats.
			if(static_cast<GlobalArmor*>(obj)->GetUsableBy() & ch->GetID()) {
				int32 variance = 0;
				int32 metaVariance = 0;
				switch(obj->GetObjectType()) {
					case GLOBAL_OBJECT_WEAPON:
						variance = static_cast<GlobalWeapon*>(obj)->GetPhysicalAttack() - ch->GetWeaponEquipped()->GetPhysicalAttack();
						metaVariance = static_cast<GlobalWeapon*>(obj)->GetMetaphysicalAttack() - ch->GetWeaponEquipped()->GetMetaphysicalAttack();
						break;
					case GLOBAL_OBJECT_ARM_ARMOR:
						variance = static_cast<GlobalArmor*>(obj)->GetPhysicalDefense() - ch->GetArmArmorEquipped()->GetPhysicalDefense();
						metaVariance = static_cast<GlobalArmor*>(obj)->GetMetaphysicalDefense() - ch->GetArmArmorEquipped()->GetMetaphysicalDefense();
						break;
					case GLOBAL_OBJECT_TORSO_ARMOR:
						variance = static_cast<GlobalArmor*>(obj)->GetPhysicalDefense() - ch->GetTorsoArmorEquipped()->GetPhysicalDefense();
						metaVariance = static_cast<GlobalArmor*>(obj)->GetMetaphysicalDefense() - ch->GetTorsoArmorEquipped()->GetMetaphysicalDefense();
						break;
					case GLOBAL_OBJECT_HEAD_ARMOR:
						variance = static_cast<GlobalArmor*>(obj)->GetPhysicalDefense() - ch->GetHeadArmorEquipped()->GetPhysicalDefense();
						metaVariance = static_cast<GlobalArmor*>(obj)->GetMetaphysicalDefense() - ch->GetHeadArmorEquipped()->GetMetaphysicalDefense();
					 	break;
					case GLOBAL_OBJECT_LEG_ARMOR:
						variance = static_cast<GlobalArmor*>(obj)->GetPhysicalDefense() - ch->GetLegArmorEquipped()->GetPhysicalDefense();
						metaVariance = static_cast<GlobalArmor*>(obj)->GetMetaphysicalDefense() - ch->GetLegArmorEquipped()->GetMetaphysicalDefense();
						break;
					default: break;
				}

				// Put variance info into the corresponding vectors for currently selected character's index.
				_usableBy.push_back(ch);
				_statVariance.push_back(variance);
				_metaVariance.push_back(metaVariance);
			}
			else {
				_usableBy.push_back(NULL);
				_statVariance.push_back(0);
				_metaVariance.push_back(0);
			}
		}
	}

	description.SetDisplayText(_object->GetDescription());

	// Determine what properties to display depending on what type of object this is
	switch (obj->GetObjectType()) {
		case GLOBAL_OBJECT_WEAPON:
			GlobalWeapon *weapon;
			weapon = dynamic_cast<GlobalWeapon*>(obj);
			properties.SetDisplayText("PHYS ATK: " + NumberToString(weapon->GetPhysicalAttack()) + "\n" + "META ATK: " + NumberToString(weapon->GetMetaphysicalAttack()) + "\n" +
				"Equippable by: "
			);
			break;
		case GLOBAL_OBJECT_HEAD_ARMOR:
		case GLOBAL_OBJECT_TORSO_ARMOR:
		case GLOBAL_OBJECT_ARM_ARMOR:
		case GLOBAL_OBJECT_LEG_ARMOR:
			GlobalArmor *armor;
			armor = dynamic_cast<GlobalArmor*>(obj);
			properties.SetDisplayText("           DEF: " + NumberToString(armor->GetPhysicalDefense()) + "\n" + "META DEF: " + NumberToString(armor->GetMetaphysicalDefense())
			);
			break;
		default:
			properties.ClearText();
			break;
	}
} // void ObjectInfoWindow::SetObject(GlobalObject* obj)



void ObjectInfoWindow::Draw() {
	MenuWindow::Draw();
	if (_object == NULL) {
		return;
	}

	VideoManager->Move(350, 240);
	// Draw the object's icon and name
	_object->GetIconImage().Draw();
	VideoManager->MoveRelative(60, 20);
	VideoManager->Text()->Draw(_object->GetName());

	if(_is_weapon || _is_armor) {
		hoa_utils::ustring atk_or_def;
		if(_is_weapon) atk_or_def = MakeUnicodeString("ATK:");
		if(_is_armor)  atk_or_def = MakeUnicodeString("DEF:");

		VideoManager->Move(335,110);

		for(uint32 i = 0; i < /*CHANGE TO PARTYSIZE*/_usableBy.size(); i++) {

			// if selected character is able to equip this item
			if(_usableBy[i] != NULL) {
				//VideoManager->Text()->Draw(_usableBy[i]->GetName());
				_character_icons[i].Draw();
				VideoManager->MoveRelative(47, 32);

				VideoManager->Text()->Draw(atk_or_def, TextStyle("default", Color::white, VIDEO_TEXT_SHADOW_DARK));
				VideoManager->MoveRelative(47, 0);
				if(_statVariance[i] > 0) {
					VideoManager->Text()->Draw("+" + NumberToString(_statVariance[i]), TextStyle("default", Color::green, VIDEO_TEXT_SHADOW_DARK));
				}
				else if(_statVariance[i] == 0) {
					VideoManager->Text()->Draw("+" + NumberToString(_statVariance[i]), TextStyle("default", Color::gray, VIDEO_TEXT_SHADOW_DARK) );
				}
				else if(_statVariance[i] < 0) {
					VideoManager->MoveRelative(2, 0); // OCD + Allignment problem :)
					VideoManager->Text()->Draw(NumberToString(_statVariance[i]), TextStyle("default", Color::red, VIDEO_TEXT_SHADOW_DARK) );
					VideoManager->MoveRelative(-2, 0);
				}

				VideoManager->MoveRelative(-47, -32);
				VideoManager->Text()->Draw("MET: ", TextStyle("default", Color::white, VIDEO_TEXT_SHADOW_DARK));
				VideoManager->MoveRelative(47, 0);

				if(_metaVariance[i] > 0) {
					VideoManager->Text()->Draw("+" + NumberToString(_metaVariance[i]), TextStyle("default", Color::green, VIDEO_TEXT_SHADOW_DARK));
				}
				else if(_metaVariance[i] == 0) {
					VideoManager->Text()->Draw("+" + NumberToString(_metaVariance[i]), TextStyle("default", Color::gray, VIDEO_TEXT_SHADOW_DARK) );
				}
				else if(_metaVariance[i] < 0) {
					VideoManager->MoveRelative(2, 0); // OCD + Allignment problem :)
					VideoManager->Text()->Draw(NumberToString(_metaVariance[i]), TextStyle("default", Color::red, VIDEO_TEXT_SHADOW_DARK) );
					VideoManager->MoveRelative(-2, 0);
				}
				VideoManager->MoveRelative(30, 0);
			}
			// if selected character can't equip this item
			else {
				_character_icons_bw[i].Draw();
				VideoManager->MoveRelative(124, 0);
			}


		}
	}
	// Draw the object's description and stats text boxes
	description.Draw();
	properties.Draw();
}


void ObjectInfoWindow::_LoadCharacterIcons() {
	uint32 partysize = GlobalManager->GetActiveParty()->GetPartySize();		// Number of characters in party
	GlobalCharacter* ch;  													// Used to point to individual character
	hoa_video::StillImage icon;												// Stores color icon image
	hoa_video::StillImage icon_bw;											// Store b&w icon image

	for(uint32 i = 0; i < partysize; i++) {
			ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(i));

			// load color character icon
			if (icon.Load("img/icons/actors/characters/" + ch->GetFilename() + ".png", 45, 45) == false)
				cerr << "SHOPMODE: Couldn't load character icon: " + ch->GetFilename() + ".png" << endl;

			// load black and white character icon
			if (icon_bw.Load("img/icons/actors/characters/" + ch->GetFilename() + "_bw.png", 45, 45) == false)
				cerr << "SHOPMODE: Couldn't load character icon: " + ch->GetFilename() + "_bw.png" << endl;

			// put color and black and white icons into appropiate vectors
			_character_icons.push_back(icon);
			_character_icons_bw.push_back(icon_bw);
	}
}

// *****************************************************************************
// ***** ConfirmWindow
// *****************************************************************************

ConfirmWindow::ConfirmWindow() {
	// (1) Create the confirmation window in the center of the screen
	MenuWindow::Create(400, 200);
	MenuWindow::SetPosition(512, 384);
	MenuWindow::SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);

	// (2) Initialize the option list
	options.SetOwner(this);
	options.SetPosition(100.0f, 100.0f);
	options.SetDimensions(300.0f, 50.0f, 2, 1, 2, 1);
	options.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	options.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	options.SetSelectMode(VIDEO_SELECT_SINGLE);
	options.SetCursorOffset(-50.0f, 20.0f);
	options.SetVerticalWrapMode(VIDEO_WRAP_MODE_NONE);

	vector<ustring> text;
	text.push_back(MakeUnicodeString("Confirm"));
	text.push_back(MakeUnicodeString("Cancel"));
	options.SetOptions(text);
	options.SetSelection(0);
}



ConfirmWindow::~ConfirmWindow() {
	MenuWindow::Destroy();
}



void ConfirmWindow::Update() {
	options.Update();

	if (InputManager->LeftPress()) {
		options.InputLeft();
	}
	else if (InputManager->RightPress()) {
		options.InputRight();
	}

	if (InputManager->CancelPress()) {
		current_shop->_shop_sounds["cancel"].Play();
		options.SetSelection(0);
		current_shop->_action_window->action_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		current_shop->_state = SHOP_STATE_ACTION;
	}
	else if (InputManager->ConfirmPress()) {
		if (options.GetSelection() == 0) { // Confirm purchase
			for (uint32 ctr = 0; ctr < current_shop->_current_inv.size(); ctr++) {
				if (current_shop->_sell_objects_quantities[ctr] > 0) {
					GlobalManager->DecrementObjectCount(current_shop->_current_inv[ctr]->GetID(), current_shop->_sell_objects_quantities[ctr]);
				}
			}

			for (uint32 ctr = 0; ctr < current_shop->_buy_objects.size(); ctr++) {
				if (current_shop->_buy_objects_quantities[ctr] > 0) {
					GlobalManager->AddToInventory(current_shop->_buy_objects[ctr]->GetID(), current_shop->_buy_objects_quantities[ctr]);
				}
				current_shop->_buy_objects_quantities[ctr] = 0;
			}

			map<uint32, GlobalObject*>* inv = GlobalManager->GetInventory();
			map<uint32, GlobalObject*>::iterator iter;

			current_shop->_sell_objects_quantities.clear();
			for (iter = inv->begin(); iter != inv->end(); iter++) {
				current_shop->_sell_objects_quantities.push_back(0);
			}

			GlobalManager->SubtractDrunes(current_shop->GetPurchaseCost());
			GlobalManager->AddDrunes(current_shop->GetSalesRevenue());
			current_shop->_purchases_cost = 0;
			current_shop->_sales_revenue = 0;
			current_shop->_shop_sounds["coins"].Play();
			current_shop->_action_window->UpdateFinanceTable();
			current_shop->_info_window->SetObject(NULL);
			current_shop->_buy_window->RefreshList();
			current_shop->_sell_window->UpdateSellList();
			current_shop->_state = SHOP_STATE_BUY;
		}
		else { // Cancel purchase
			current_shop->_shop_sounds["cancel"].Play();
		}

		// Return to previous window
		options.SetSelection(0);
		current_shop->_action_window->action_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		current_shop->_PopState();
	}
} // void ConfirmWindow::Update()



void ConfirmWindow::Draw() {
	MenuWindow::Draw();
	options.Draw();

	VideoManager->PushState();
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->Move(512, 450);
	VideoManager->Text()->Draw("Finalize transactions?");
	VideoManager->PopState();
}

// *****************************************************************************
// ***** PromptWindow
// *****************************************************************************

PromptWindow::PromptWindow() {
	// (1) Create the confirmation window in the center of the screen
	MenuWindow::Create(400, 200);
	MenuWindow::SetPosition(512, 384);
	MenuWindow::SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);

	// (2) Initialize the textbox list
	prompt_text.SetOwner(this);
	prompt_text.SetPosition(25.0f, 175.0f);
	prompt_text.SetDimensions(360.0f, 160.0f);
	prompt_text.SetDisplaySpeed(30);
	prompt_text.SetTextStyle(TextStyle());
	prompt_text.SetDisplayMode(VIDEO_TEXT_INSTANT);
	prompt_text.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
}



PromptWindow::~PromptWindow() {
	MenuWindow::Destroy();
}



void PromptWindow::Update() {
	prompt_text.Update();

	if (InputManager->ConfirmPress() || InputManager->CancelPress()) {
		current_shop->_PopState();
		prompt_text.ClearText();
	}
}



void PromptWindow::Draw() {
	MenuWindow::Draw();
	prompt_text.Draw();
}

} // namespace private_shop

} // namespace hoa_shop
