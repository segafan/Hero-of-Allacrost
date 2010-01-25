///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_confirm.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for confirm menu of shop mode
***
*** WRITE SOMETHING
*** ***************************************************************************/

#include "defs.h"
#include "utils.h"

#include "audio.h"
#include "input.h"
#include "video.h"
#include "global.h"

#include "shop.h"
#include "shop_confirm.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_input;
using namespace hoa_video;

namespace hoa_shop {

namespace private_shop {

// *****************************************************************************
// ***** ConfirmInterface class methods
// *****************************************************************************

ConfirmInterface::ConfirmInterface() {

}



ConfirmInterface::~ConfirmInterface() {

}



void ConfirmInterface::Initialize() {

}



void ConfirmInterface::Update() {
	if (InputManager->ConfirmPress() || InputManager->CancelPress()) {
		ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_ROOT);
	}
}



void ConfirmInterface::Draw() {

}

// *****************************************************************************
// ***** ConfirmWindow
// *****************************************************************************

ConfirmWindow::ConfirmWindow() {
	// (1) Create the confirmation window in the center of the screen
	MenuWindow::Create(400.0f, 200.0f);
	MenuWindow::SetPosition(512.0f, 384.0f);
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
// 		ShopMode::GetCurrentInstance()->_shop_sounds["cancel"].Play();
// 		options.SetSelection(0);
// 		ShopMode::GetCurrentInstance()->_action_window->action_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
// 		options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
// 		ShopMode::GetCurrentInstance()->_state = SHOP_STATE_ACTION;
	}
	else if (InputManager->ConfirmPress()) {
// 		if (options.GetSelection() == 0) { // Confirm purchase
// 			for (uint32 ctr = 0; ctr < ShopMode::GetCurrentInstance()->_current_inv.size(); ctr++) {
// 				if (ShopMode::GetCurrentInstance()->_sell_objects_quantities[ctr] > 0) {
// 					GlobalManager->DecrementObjectCount(ShopMode::GetCurrentInstance()->_current_inv[ctr]->GetID(), ShopMode::GetCurrentInstance()->_sell_objects_quantities[ctr]);
// 				}
// 			}
//
// 			for (uint32 ctr = 0; ctr < ShopMode::GetCurrentInstance()->_buy_objects.size(); ctr++) {
// 				if (ShopMode::GetCurrentInstance()->_buy_objects_quantities[ctr] > 0) {
// 					GlobalManager->AddToInventory(ShopMode::GetCurrentInstance()->_buy_objects[ctr]->GetID(), ShopMode::GetCurrentInstance()->_buy_objects_quantities[ctr]);
// 				}
// 				ShopMode::GetCurrentInstance()->_buy_objects_quantities[ctr] = 0;
// 			}
//
// 			map<uint32, GlobalObject*>* inv = GlobalManager->GetInventory();
// 			map<uint32, GlobalObject*>::iterator iter;
//
// 			ShopMode::GetCurrentInstance()->_sell_objects_quantities.clear();
// 			for (iter = inv->begin(); iter != inv->end(); iter++) {
// 				ShopMode::GetCurrentInstance()->_sell_objects_quantities.push_back(0);
// 			}
//
// 			GlobalManager->SubtractDrunes(ShopMode::GetCurrentInstance()->GetPurchaseCost());
// 			GlobalManager->AddDrunes(ShopMode::GetCurrentInstance()->GetSalesRevenue());
// 			ShopMode::GetCurrentInstance()->_purchases_cost = 0;
// 			ShopMode::GetCurrentInstance()->_sales_revenue = 0;
// 			ShopMode::GetCurrentInstance()->_shop_sounds["coins"].Play();
// 			ShopMode::GetCurrentInstance()->_action_window->UpdateFinanceTable();
// 			ShopMode::GetCurrentInstance()->_info_window->SetObject(NULL);
// 			ShopMode::GetCurrentInstance()->_buy_window->RefreshList();
// 			ShopMode::GetCurrentInstance()->_sell_window->UpdateSellList();
// 			ShopMode::GetCurrentInstance()->_state = SHOP_STATE_BUY;
// 		}
// 		else { // Cancel purchase
// 			ShopMode::GetCurrentInstance()->_shop_sounds["cancel"].Play();
// 		}
//
// 		// Return to previous window
// 		options.SetSelection(0);
// 		ShopMode::GetCurrentInstance()->_action_window->action_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
// 		options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
// 		ShopMode::GetCurrentInstance()->_PopState();
	}
} // void ConfirmWindow::Update()



void ConfirmWindow::Draw() {
	MenuWindow::Draw();
	options.Draw();

	VideoManager->PushState();
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->Move(512.0f, 450.0f);
	VideoManager->Text()->Draw("Finalize transactions?");
	VideoManager->PopState();
}

}

}
