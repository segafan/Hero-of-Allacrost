///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_root.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for root menus of shop mode
*** ***************************************************************************/

#include <iostream>

#include "utils.h"

#include "audio.h"
#include "video.h"
#include "input.h"
#include "mode_manager.h"
#include "system.h"

#include "global.h"

#include "shop.h"
#include "shop_root.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_input;
using namespace hoa_mode_manager;
using namespace hoa_system;
using namespace hoa_global;

namespace hoa_shop {

namespace private_shop {

// *****************************************************************************
// ***** RootInterface class methods
// *****************************************************************************

RootInterface::RootInterface() {
	_root_window = new RootWindow();
	_greeting_window = new GreetingWindow();
}



RootInterface::~RootInterface() {
	delete _root_window;
	delete _greeting_window;
}



void RootInterface::Initialize() {
	// Text constants that represent the various price levels
	const ustring VERY_GOOD = MakeUnicodeString("very good");
	const ustring GOOD = MakeUnicodeString("good");
	const ustring STANDARD = MakeUnicodeString("standard");
	const ustring POOR = MakeUnicodeString("poor");
	const ustring VERY_POOR = MakeUnicodeString("very poor");

	// ----- (1): Initialize the finance table text
	UpdateFinanceTable();

	// ----- (2): Retrieve copies of each category icon and enable grayscale for unused categories
	uint8 deal_types = ShopMode::CurrentInstance()->GetDealTypes();
	_greeting_window->category_icons = ShopMode::CurrentInstance()->GetObjectCategoryImages();

	if ((deal_types & DEALS_ITEMS) == false) {
		_greeting_window->category_icons[0].EnableGrayScale();
	}
	if ((deal_types & DEALS_WEAPONS) == false) {
		_greeting_window->category_icons[1].EnableGrayScale();
	}
	if ((deal_types & DEALS_HEAD_ARMOR) == false) {
		_greeting_window->category_icons[2].EnableGrayScale();
	}
	if ((deal_types & DEALS_TORSO_ARMOR) == false) {
		_greeting_window->category_icons[3].EnableGrayScale();
	}
	if ((deal_types & DEALS_ARM_ARMOR) == false) {
		_greeting_window->category_icons[4].EnableGrayScale();
	}
	if ((deal_types & DEALS_LEG_ARMOR) == false) {
		_greeting_window->category_icons[5].EnableGrayScale();
	}
	if ((deal_types & DEALS_SHARDS) == false) {
		_greeting_window->category_icons[6].EnableGrayScale();
	}
	if ((deal_types & DEALS_KEY_ITEMS) == false) {
		_greeting_window->category_icons[7].EnableGrayScale();
	}

	// ----- (3): Initialize the shop pricing text based on the buy/sell price levels
	ustring buy_text, sell_text;

	switch (ShopMode::CurrentInstance()->GetBuyPriceLevel()) {
		case SHOP_PRICE_VERY_GOOD:
			buy_text = VERY_GOOD;
			break;
		case SHOP_PRICE_GOOD:
			buy_text = GOOD;
			break;
		case SHOP_PRICE_STANDARD:
			buy_text = STANDARD;
			break;
		case SHOP_PRICE_POOR:
			buy_text = POOR;
			break;
		case SHOP_PRICE_VERY_POOR:
			buy_text = VERY_POOR;
			break;
		default:
			buy_text = STANDARD;
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid buy_level argument: " << ShopMode::CurrentInstance()->GetBuyPriceLevel() << endl;
			break;
	}

	switch (ShopMode::CurrentInstance()->GetSellPriceLevel()) {
		case SHOP_PRICE_VERY_GOOD:
			sell_text = VERY_GOOD;
			break;
		case SHOP_PRICE_GOOD:
			sell_text = GOOD;
			break;
		case SHOP_PRICE_STANDARD:
			sell_text = STANDARD;
			break;
		case SHOP_PRICE_POOR:
			sell_text = POOR;
			break;
		case SHOP_PRICE_VERY_POOR:
			sell_text = VERY_POOR;
			break;
		default:
			sell_text = STANDARD;
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid sell_level argument: " << ShopMode::CurrentInstance()->GetSellPriceLevel() << endl;
			break;
	}

	_greeting_window->pricing_text.SetDisplayText(MakeUnicodeString("Merchant's buy prices are ") + buy_text + MakeUnicodeString(".\n") +
		MakeUnicodeString("Merchant's sell prices are ") + sell_text + MakeUnicodeString("."));
} // void RootInterface::Initialize()



void RootInterface::MakeActive() {
	_greeting_window->Show();
	_root_window->action_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
}



void RootInterface::MakeInactive() {
	_greeting_window->Hide();
	_root_window->action_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}



void RootInterface::Update() {
	_root_window->Update();

	if (ShopMode::CurrentInstance()->GetState() != SHOP_STATE_ROOT) {
		return;
	}

	_greeting_window->Update();

	// ----- Process User Input
	SoundDescriptor* sound = NULL; // Used to hold pointers of sound objects to play

	if (InputManager->ConfirmPress()) {
		if (_root_window->action_options.GetSelection() < 0 || _root_window->action_options.GetSelection() > 4) {
			IF_PRINT_WARNING(SHOP_DEBUG) << "invalid selection in action window: " << _root_window->action_options.GetSelection() << endl;
			_root_window->action_options.SetSelection(0);
			return;
		}

		_root_window->action_options.InputConfirm();
		_root_window->action_options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		sound = ShopMode::CurrentInstance()->GetSound("confirm");
		assert(sound != NULL);
		sound->Play();

		if (_root_window->action_options.GetSelection() == 0) { // Buy
			ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_BUY);
		}
		else if (_root_window->action_options.GetSelection() == 1) { // Sell
			ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_SELL);
		}
		else if (_root_window->action_options.GetSelection() == 2) { // Trade
			ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_TRADE);
		}
		else if (_root_window->action_options.GetSelection() == 3) { // Confirm
			ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_CONFIRM);
		}
		else if (_root_window->action_options.GetSelection() == 4) { // Leave shop
			ModeManager->Pop();
		}
	}
	else if (InputManager->LeftPress()) {
		_root_window->action_options.InputLeft();
	}
	else if (InputManager->RightPress()) {
		_root_window->action_options.InputRight();
	}
}



void RootInterface::Draw() {
	_root_window->Draw();

	if (ShopMode::CurrentInstance()->GetState() != SHOP_STATE_ROOT) {
		return;
	}

	_greeting_window->Draw();
}



void RootInterface::SetGreetingText(hoa_utils::ustring greeting) {
	_greeting_window->greeting_text.SetDisplayText(greeting);
}



void RootInterface::UpdateFinanceTable() {
	_root_window->finance_table.SetOptionText(0, MakeUnicodeString("Funds: " + NumberToString(GlobalManager->GetDrunes())));
	_root_window->finance_table.SetOptionText(1, MakeUnicodeString("Purchases: -" + NumberToString(ShopMode::CurrentInstance()->GetTotalCosts())));
	_root_window->finance_table.SetOptionText(2, MakeUnicodeString("Sales: +" + NumberToString(ShopMode::CurrentInstance()->GetTotalSales())));
	_root_window->finance_table.SetOptionText(3, MakeUnicodeString("Total: " + NumberToString(ShopMode::CurrentInstance()->GetTotalRemaining())));
}

// *****************************************************************************
// ***** RootWindow class methods
// *****************************************************************************

RootWindow::RootWindow() {
	// (1) Initialize the window
	MenuWindow::Create(800.0f, 80.0f, ~VIDEO_MENU_EDGE_BOTTOM);
	MenuWindow::SetPosition(112.0f, 684.0f);
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
		finance_table.AddOption(ustring());

	// (4) Initialize the drunes icon image
	if (drunes_icon.Load("img/icons/drunes.png") == false)
		IF_PRINT_WARNING(SHOP_DEBUG) << "failed to load drunes image for action window" << endl;
	drunes_icon.SetDimensions(30.0f, 30.0f);
} // RootWindow::RootWindow()



RootWindow::~RootWindow() {
	MenuWindow::Destroy();
}



void RootWindow::Update() {
	MenuWindow::Update();
	action_options.Update(); // Clear any OptionBox events, since they prevent further user input
}



void RootWindow::Draw() {
	MenuWindow::Draw();
	action_options.Draw();
	finance_table.Draw();

	VideoManager->Move(150.0f, 610.0f);
	drunes_icon.Draw();
}

// *****************************************************************************
// ***** GreetingWindow class methods
// *****************************************************************************

GreetingWindow::GreetingWindow() {
	// (1) Initialize the window
	MenuWindow::Create(800.0f, 200.0f, VIDEO_MENU_EDGE_ALL, VIDEO_MENU_EDGE_TOP);
	MenuWindow::SetPosition(112.0f, 612.0f);
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
	greeting_text.SetDisplayText(MakeUnicodeString("Welcome! Take a look around.")); // Default greeting, should usually be overwritten

	// (3) Initialize the price level textbox
	pricing_text.SetOwner(this);
	pricing_text.SetPosition(40.0f, 65.0f);
	pricing_text.SetDimensions(720.0f, 50.0f);
	pricing_text.SetTextStyle(TextStyle());
	pricing_text.SetDisplaySpeed(30);
	pricing_text.SetDisplayMode(VIDEO_TEXT_INSTANT);
	pricing_text.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
}



GreetingWindow::~GreetingWindow() {
	MenuWindow::Destroy();
}



void GreetingWindow::Update() {
	MenuWindow::Update();
	greeting_text.Update();
	pricing_text.Update();
}



void GreetingWindow::Draw() {
	MenuWindow::Draw();
	greeting_text.Draw();
	pricing_text.Draw();

	VideoManager->Move(200.0f, 500.0f);
	for (uint32 i = 0; i < category_icons.size(); i++) {
		category_icons[i].Draw();
		VideoManager->MoveRelative(80.0f, 0.0f);
	}
}

} // namespace private_shop

} // namespace hoa_shop
