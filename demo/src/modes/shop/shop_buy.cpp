///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_buy.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for buy menus of shop mode
*** ***************************************************************************/

#include "defs.h"
#include "utils.h"

#include "audio.h"
#include "input.h"
#include "video.h"

#include "global.h"

#include "shop.h"
#include "shop_buy.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_input;
using namespace hoa_video;
using namespace hoa_global;

namespace hoa_shop {

namespace private_shop {

// *****************************************************************************
// ***** ShopBuyInterface class methods
// *****************************************************************************

ShopBuyInterface::ShopBuyInterface() {

}



ShopBuyInterface::~ShopBuyInterface() {

}



void ShopBuyInterface::Initialize() {

}



void ShopBuyInterface::Update() {
	if (InputManager->ConfirmPress() || InputManager->CancelPress()) {
		ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_ROOT);
	}
}



void ShopBuyInterface::Draw() {

}

// *****************************************************************************
// ***** BuyListWindow
// *****************************************************************************

BuyListWindow::BuyListWindow() {
	MenuWindow::Create(800.0f, 400.0f, VIDEO_MENU_EDGE_LEFT | VIDEO_MENU_EDGE_RIGHT, VIDEO_MENU_EDGE_TOP | VIDEO_MENU_EDGE_BOTTOM);
	MenuWindow::SetPosition(112.0f, 584.0f);
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
// 	object_list.ClearOptions();
// 	for (uint32 i = 0; i < ShopMode::CurrentInstance()->_buy_objects.size(); i++) {
// 		AddEntry(ShopMode::CurrentInstance()->_buy_objects[i]->GetName(), ShopMode::CurrentInstance()->_buy_objects[i]->GetPrice(),
// 			ShopMode::CurrentInstance()->_buy_objects_quantities[i]
// 		);
// 	}
//
// 	object_list.SetDimensions(500.0f, 360.0f, 1, 255, 1, 6);
// 	object_list.SetSelection(0);
}



void BuyListWindow::Update() {
	MenuWindow::Update();
	object_list.Update(); // Clear any OptionBox events, since they prevent further user input

	if (InputManager->ConfirmPress()) {
// 		object_list.InputConfirm();
//
// 		int32 x = object_list.GetSelection();
// 		if (ShopMode::GetCurrentInstance()->_buy_objects_quantities[x] == 0) {
// 			ShopMode::GetCurrentInstance()->_PushAndSetState(SHOP_STATE_PROMPT);
// 			ShopMode::GetCurrentInstance()->_prompt_window->Show();
// 			ShopMode::GetCurrentInstance()->_prompt_window->prompt_text.SetDisplayText(MakeUnicodeString(
// 				"No quantity for this selection was made. Use the right and left commands to increment "
// 				"or decrement the amount of this object to purchase.")
// 			);
// 		}
// 		else {
//
// 		ShopMode::GetCurrentInstance()->_state = SHOP_STATE_CONFIRM;
// 		object_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
// 		ShopMode::GetCurrentInstance()->_info_window->SetObject(NULL);
// 		ShopMode::GetCurrentInstance()->_confirm_window->options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
// 		ShopMode::GetCurrentInstance()->_confirm_window->Show();
// 		ShopMode::GetCurrentInstance()->_shop_sounds["confirm"].Play();
// 		}
	}
	else if (InputManager->CancelPress()) {
// 		hide_options = true;
// 		ShopMode::GetCurrentInstance()->_state = SHOP_STATE_ACTION;
// 		ShopMode::GetCurrentInstance()->_action_window->action_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
// 		object_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
// 		ShopMode::GetCurrentInstance()->_info_window->SetObject(NULL);
// 		ShopMode::GetCurrentInstance()->_shop_sounds["cancel"].Play();
	}
	else if (InputManager->UpPress()) {
// 		object_list.InputUp();
// 		ShopMode::GetCurrentInstance()->_info_window->SetObject(ShopMode::GetCurrentInstance()->_buy_objects[object_list.GetSelection()]);
	}
	else if (InputManager->DownPress()) {
// 		object_list.InputDown();
// 		ShopMode::GetCurrentInstance()->_info_window->SetObject(ShopMode::GetCurrentInstance()->_buy_objects[object_list.GetSelection()]);
	}
	else if (InputManager->LeftPress()) {
// 		int32 x = object_list.GetSelection();
// 		if (ShopMode::GetCurrentInstance()->_buy_objects_quantities[x] > 0) {
// 			ShopMode::GetCurrentInstance()->_buy_objects_quantities[x]--;
// 			ShopMode::GetCurrentInstance()->_purchases_cost -= ShopMode::GetCurrentInstance()->_buy_objects[x]->GetPrice();
// 			object_list.SetOptionText(x, ShopMode::GetCurrentInstance()->_buy_objects[x]->GetName() + MakeUnicodeString("<R>") +
// 				MakeUnicodeString(NumberToString(ShopMode::GetCurrentInstance()->_buy_objects[x]->GetPrice())) + MakeUnicodeString("   x") +
// 				MakeUnicodeString(NumberToString(ShopMode::GetCurrentInstance()->_buy_objects_quantities[x]))
// 			);
// 			ShopMode::GetCurrentInstance()->_action_window->UpdateFinanceTable();
// 		}
// 		else {
// 			ShopMode::GetCurrentInstance()->_shop_sounds["cancel"].Play();
// 		}
	}
	else if (InputManager->RightPress()) {
// 		int32 x = object_list.GetSelection();
// 		if (ShopMode::GetCurrentInstance()->_buy_objects[x]->GetPrice() <= ShopMode::GetCurrentInstance()->GetTotalRemaining()) {
// 			ShopMode::GetCurrentInstance()->_buy_objects_quantities[x]++;
// 			ShopMode::GetCurrentInstance()->_purchases_cost += ShopMode::GetCurrentInstance()->_buy_objects[x]->GetPrice();
// 			ShopMode::GetCurrentInstance()->_action_window->UpdateFinanceTable();
// 			object_list.SetOptionText(x, ShopMode::GetCurrentInstance()->_buy_objects[x]->GetName() + MakeUnicodeString("<R>") +
// 				MakeUnicodeString(NumberToString(ShopMode::GetCurrentInstance()->_buy_objects[x]->GetPrice())) + MakeUnicodeString("   x") +
// 				MakeUnicodeString(NumberToString(ShopMode::GetCurrentInstance()->_buy_objects_quantities[x]))
// 			);
// 		}
// 		else {
// 			ShopMode::GetCurrentInstance()->_shop_sounds["cancel"].Play();
// 		}
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

}

}
