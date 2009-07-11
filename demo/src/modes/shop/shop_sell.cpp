///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_sell.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for sell menu of shop mode
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
#include "shop_sell.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_input;
using namespace hoa_video;
using namespace hoa_global;

namespace hoa_shop {

namespace private_shop {

// *****************************************************************************
// ***** ShopSellInterface class methods
// *****************************************************************************

ShopSellInterface::ShopSellInterface() {

}



ShopSellInterface::~ShopSellInterface() {

}



void ShopSellInterface::Initialize() {

}



void ShopSellInterface::Update() {
	if (InputManager->ConfirmPress() || InputManager->CancelPress()) {
		ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_ROOT);
	}
}



void ShopSellInterface::Draw() {

}

// *****************************************************************************
// ***** SellListWindow
// *****************************************************************************

SellListWindow::SellListWindow() {
	MenuWindow::Create(800.0f, 400.0f, VIDEO_MENU_EDGE_LEFT | VIDEO_MENU_EDGE_RIGHT, VIDEO_MENU_EDGE_TOP | VIDEO_MENU_EDGE_BOTTOM);
	MenuWindow::SetPosition(112.0f, 584.0f);
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
// 	option_text.clear();
// 	ShopMode::CurrentInstance()->_current_inv.clear();
// 	object_list.SetOptions(option_text);
}



void SellListWindow::AddEntry(hoa_utils::ustring name, uint32 count, uint32 price, uint32 sell_count) {
	string text = MakeStandardString(name) + "<R>" + NumberToString(count) + "      x" + NumberToString(sell_count) + "       " + NumberToString(price);
	option_text.push_back(MakeUnicodeString(text));
}



void SellListWindow::Update() {
	MenuWindow::Update();
	object_list.Update(); // Clear any OptionBox events, since they prevent further user input

	if (InputManager->ConfirmPress()) {
// 		object_list.InputConfirm();
// 		int32 x = object_list.GetSelection();
//
// 		if (ShopMode::CurrentInstance()->_sell_objects_quantities[x] == 0) {
// 			ShopMode::CurrentInstance()->_PushAndSetState(SHOP_STATE_PROMPT);
// 			ShopMode::CurrentInstance()->_prompt_window->Show();
// 			ShopMode::CurrentInstance()->_prompt_window->prompt_text.SetDisplayText(MakeUnicodeString(
// 				"No quantity for this selection was made. Use the right and left commands to increment "
// 				"or decrement the amount of this object to offer for sale.")
// 			);
// 		}
// 		else {
// 			hide_options = true;
// 			object_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
// 			ShopMode::CurrentInstance()->_state = SHOP_STATE_CONFIRM;
// 			ShopMode::CurrentInstance()->_confirm_window->options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
// 			ShopMode::CurrentInstance()->_confirm_window->Show();
// 			ShopMode::CurrentInstance()->_shop_sounds["confirm"].Play();
// 		}
	}
	else if (InputManager->CancelPress()) {
// 		hide_options = true;
// 		ShopMode::CurrentInstance()->_state = SHOP_STATE_ACTION;
// 		ShopMode::CurrentInstance()->_action_window->action_options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
// 		object_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
// 		ShopMode::CurrentInstance()->_info_window->SetObject(NULL);
// 		ShopMode::CurrentInstance()->_shop_sounds["cancel"].Play();
	}
	else if (InputManager->UpPress()) {
// 		object_list.InputUp();
// 		ShopMode::CurrentInstance()->_info_window->SetObject(ShopMode::CurrentInstance()->_current_inv[object_list.GetSelection()]);
	}
	else if (InputManager->DownPress()) {
// 		object_list.InputDown();
// 		ShopMode::CurrentInstance()->_info_window->SetObject(ShopMode::CurrentInstance()->_current_inv[object_list.GetSelection()]);
	}
	else if (InputManager->LeftPress()) {
// 		int32 x = object_list.GetSelection();
// 		if (ShopMode::CurrentInstance()->_sell_objects_quantities[x] > 0) {
// 			ShopMode::CurrentInstance()->_sell_objects_quantities[x]--;
// 			ShopMode::CurrentInstance()->_sales_revenue -= (ShopMode::CurrentInstance()->_current_inv[x]->GetPrice() / 2);
// 			ShopMode::CurrentInstance()->_action_window->UpdateFinanceTable();
// 		}
// 		else {
// 			ShopMode::CurrentInstance()->_shop_sounds["cancel"].Play();
// 		}
	}
	else if (InputManager->RightPress()) {
// 		uint32 x = object_list.GetSelection();
// 		if (ShopMode::CurrentInstance()->_current_inv[x]->GetCount() > static_cast<uint32>(ShopMode::CurrentInstance()->_sell_objects_quantities[x])) {
// 			ShopMode::CurrentInstance()->_sell_objects_quantities[x]++;
// 			ShopMode::CurrentInstance()->_sales_revenue += (ShopMode::CurrentInstance()->_current_inv[x]->GetPrice() / 2);
// 			ShopMode::CurrentInstance()->_action_window->UpdateFinanceTable();
// 		}
// 		else {
// 			ShopMode::CurrentInstance()->_shop_sounds["cancel"].Play();
// 		}
	}

	UpdateSellList();
}



void SellListWindow::UpdateSellList() {
// 	Clear();
// 	map<uint32, GlobalObject*>* inv = GlobalManager->GetInventory();
// 	map<uint32, GlobalObject*>::iterator iter;
//
// 	uint32 x = 0;
// 	for (iter = inv->begin(); iter != inv->end(); iter++) {
// 		ShopMode::CurrentInstance()->_current_inv.push_back(iter->second);
// 		AddEntry(iter->second->GetName(), iter->second->GetCount(), iter->second->GetPrice() / 2, ShopMode::CurrentInstance()->_sell_objects_quantities[x]);
// 		x++;
// 	}
// 	object_list.SetOptions(option_text);
}



void SellListWindow::Draw() {
	MenuWindow::Draw();

	if (hide_options == false && object_list.GetNumberOptions() != 0) {
		object_list.Draw();
		VideoManager->Move(375.0f, 640.0f);
		VideoManager->Text()->Draw(MakeUnicodeString("Item                                                                     Inv   Sell   Price"));
	}
}

}

}
