///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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
// ***************************** ShopActionWindow ******************************
// *****************************************************************************

ShopActionWindow::ShopActionWindow() {
	// (1) Initialize the window
	MenuWindow::Create(200, 600, ~VIDEO_MENU_EDGE_RIGHT);
	MenuWindow::SetPosition(112, 684);
	MenuWindow::SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);
	MenuWindow::Show();

	// (2) Initialize the list of actions
	options.SetOwner(this);
	options.SetPosition(25.0f, 600.0f);
	options.SetSize(1, 3); // One column, numerous rows
	options.SetCellSize(150.0f, 50.0f);
	options.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	options.SetFont("default");
	options.SetSelectMode(VIDEO_SELECT_SINGLE);
	options.SetCursorOffset(-50.0f, 20.0f);
	options.SetVerticalWrapMode(VIDEO_WRAP_MODE_NONE);

	vector<ustring> text;
	text.push_back(MakeUnicodeString("Buy"));
	text.push_back(MakeUnicodeString("Sell"));
	text.push_back(MakeUnicodeString("Leave"));
	options.SetOptions(text);
	options.SetSelection(0);

	// (3) Initialize the financial text box
	text_box.SetOwner(this);
	text_box.SetPosition(25.0f, 100.0f);
	text_box.SetDimensions(150.0f, 50.0f);
	text_box.SetDisplaySpeed(30);
	text_box.SetFont("default");
	text_box.SetDisplayMode(VIDEO_TEXT_INSTANT);
	text_box.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	UpdateFinanceText();
} // ShopActionWindow::ShopActionWindow()



ShopActionWindow::~ShopActionWindow() {
	MenuWindow::Destroy();
}



void ShopActionWindow::Update() {
	MenuWindow::Update(SystemManager->GetUpdateTime());
	options.GetEvent(); // clear any events, since they prevent cursor movement

	if (InputManager->ConfirmPress()) {
		options.HandleConfirmKey();
		if (options.GetSelection() == 0) { // Buy
			options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
			current_shop->_state = SHOP_STATE_LIST;
			current_shop->_list_window.hide_options = false;
			current_shop->_info_window.SetObject(current_shop->_all_objects[0]);
			current_shop->_shop_sounds["confirm"].PlaySound();
		}
		else if (options.GetSelection() == 1) { // Sell
			if (GlobalManager->GetInventory()->empty() == false) {
				current_shop->_sell_window.UpdateSellList();
				current_shop->_sell_window.object_list.SetSelection(0);
				current_shop->_state = SHOP_STATE_SELL;
				current_shop->_sell_window.hide_options = false;
				current_shop->_info_window.SetObject(current_shop->_sell_window.current_inv[0]);
				options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
				current_shop->_shop_sounds["confirm"].PlaySound();
			}
			else {
				current_shop->_shop_sounds["cancel"].PlaySound();
			}
		}
		else if (options.GetSelection() == 2) { // Exit
			ModeManager->Pop();
			current_shop->_shop_sounds["cancel"].PlaySound();
		}
		else {
			if (SHOP_DEBUG)
				cerr << "SHOP WARNING: invalid selection in action window: " << options.GetSelection() << endl;
			ModeManager->Pop();
		}
	}
	else if (InputManager->CancelPress()) {
		ModeManager->Pop();
		current_shop->_shop_sounds["cancel"].PlaySound();
	}
	else if (InputManager->UpPress()) {
		options.HandleUpKey();
	}
	else if (InputManager->DownPress()) {
		options.HandleDownKey();
	}
}



void ShopActionWindow::UpdateFinanceText() {
	text_box.SetDisplayText(MakeUnicodeString(
		"Drunes: " + NumberToString(GlobalManager->GetFunds())
	));
}



void ShopActionWindow::Draw() {
	hoa_video::MenuWindow::Draw();
	options.Draw();
	text_box.Draw();
}

// *****************************************************************************
// ***************************** ObjectListWindow ******************************
// *****************************************************************************

ObjectListWindow::ObjectListWindow() {
	MenuWindow::Create(600, 400, VIDEO_MENU_EDGE_ALL, VIDEO_MENU_EDGE_LEFT | VIDEO_MENU_EDGE_BOTTOM);
	MenuWindow::SetPosition(312, 684);
	MenuWindow::SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);
	MenuWindow::Show();

	object_list.SetOwner(this);
	object_list.SetCellSize(500.0f, 50.0f);
	object_list.SetPosition(50.0f, 400.0f);
	object_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	object_list.SetFont("default");
	object_list.SetSelectMode(VIDEO_SELECT_SINGLE);
	object_list.SetCursorOffset(-50.0f, 20.0f);
	object_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_NONE);

	hide_options = true;
}



ObjectListWindow::~ObjectListWindow() {
	MenuWindow::Destroy();
}



void ObjectListWindow::Clear() {
	option_text.clear();
	object_list.SetOptions(option_text);
}



void ObjectListWindow::AddEntry(hoa_utils::ustring name, uint32 price) {
	option_text.push_back(name + MakeUnicodeString("<R>") + MakeUnicodeString(NumberToString(price)));
}



void ObjectListWindow::ConstructList() {
	object_list.SetSize(1, option_text.size());
	object_list.SetOptions(option_text);
	object_list.SetSelection(0);
}



void ObjectListWindow::Update() {
	MenuWindow::Update(SystemManager->GetUpdateTime());
	object_list.GetEvent(); // clear any events, since they prevent cursor movement

	if (InputManager->ConfirmPress()) {
		object_list.HandleConfirmKey();
		current_shop->_confirm_window.SetObject(current_shop->_all_objects[object_list.GetSelection()]);
		current_shop->_state = SHOP_STATE_CONFIRM;
		current_shop->_shop_sounds["confirm"].PlaySound();
	}
	else if (InputManager->CancelPress()) {
		hide_options = true;
		current_shop->_state = SHOP_STATE_ACTION;
		current_shop->_action_window.options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		current_shop->_info_window.SetObject(NULL);
		current_shop->_shop_sounds["cancel"].PlaySound();
	}
	else if (InputManager->UpPress()) {
		object_list.HandleUpKey();
		current_shop->_info_window.SetObject(current_shop->_all_objects[object_list.GetSelection()]);
	}
	else if (InputManager->DownPress()) {
		object_list.HandleDownKey();
		current_shop->_info_window.SetObject(current_shop->_all_objects[object_list.GetSelection()]);
	}
}



void ObjectListWindow::Draw() {
	MenuWindow::Draw();

	if (hide_options == false && option_text.empty() == false)
		object_list.Draw();
}

// *****************************************************************************
// *************************** ObjectSellListWindow ****************************
// *****************************************************************************

ObjectSellListWindow::ObjectSellListWindow() {
	MenuWindow::Create(600, 400, VIDEO_MENU_EDGE_ALL, VIDEO_MENU_EDGE_LEFT | VIDEO_MENU_EDGE_BOTTOM);
	MenuWindow::SetPosition(312, 684);
	MenuWindow::SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);
	MenuWindow::Show();

	object_list.SetOwner(this);
	object_list.SetCellSize(500.0f, 50.0f);
	object_list.SetPosition(50.0f, 400.0f);
	object_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	object_list.SetFont("default");
	object_list.SetSelectMode(VIDEO_SELECT_SINGLE);
	object_list.SetCursorOffset(-50.0f, 20.0f);
	object_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_NONE);

	hide_options = true;
}



ObjectSellListWindow::~ObjectSellListWindow() {
	MenuWindow::Destroy();
}



void ObjectSellListWindow::Clear() {
	option_text.clear();
	current_inv.clear();
	object_list.SetOptions(option_text);
}



void ObjectSellListWindow::AddEntry(hoa_utils::ustring name, uint32 count, uint32 price) {
	option_text.push_back(name + MakeUnicodeString(" x") + MakeUnicodeString(NumberToString(count)) + MakeUnicodeString("<R>") + MakeUnicodeString(NumberToString(price)));
}



void ObjectSellListWindow::Update() {
	MenuWindow::Update(SystemManager->GetUpdateTime());
	object_list.GetEvent(); // clear any events, since they prevent cursor movement

	if (InputManager->ConfirmPress()) {
		object_list.HandleConfirmKey();
		current_shop->_confirm_sell_window.SetObject(current_inv[object_list.GetSelection()]);
		object_list.SetSelection(0);
		current_shop->_state = SHOP_STATE_CONFIRM_SELL;
		current_shop->_shop_sounds["confirm"].PlaySound();
	}
	else if (InputManager->CancelPress()) {
		hide_options = true;
		current_shop->_state = SHOP_STATE_ACTION;
		current_shop->_action_window.options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
		current_shop->_info_window.SetObject(NULL);
		current_shop->_shop_sounds["cancel"].PlaySound();
	}
	else if (InputManager->UpPress()) {
		object_list.HandleUpKey();
		current_shop->_info_window.SetObject(current_inv[object_list.GetSelection()]);
	}
	else if (InputManager->DownPress()) {
		object_list.HandleDownKey();
		current_shop->_info_window.SetObject(current_inv[object_list.GetSelection()]);
	}
}



void ObjectSellListWindow::UpdateSellList() {
	Clear();
	std::map<uint32, GlobalObject*>* inv = GlobalManager->GetInventory();
	std::map<uint32, GlobalObject*>::iterator iter;

	for (iter = inv->begin(); iter != inv->end(); iter++) {
		current_inv.push_back(iter->second);
		AddEntry(iter->second->GetName(), iter->second->GetCount(), iter->second->GetPrice() / 2);
	}
	object_list.SetSize(1, option_text.size());
	object_list.SetOptions(option_text);
}



void ObjectSellListWindow::Draw() {
	MenuWindow::Draw();

	if (hide_options == false && option_text.empty() == false)
		object_list.Draw();
}

// *****************************************************************************
// ***************************** ObjectInfoWindow ******************************
// *****************************************************************************

ObjectInfoWindow::ObjectInfoWindow() {
	// (1) Create the info window in the bottom right-hand section of the screen
	MenuWindow::Create(600, 216, ~VIDEO_MENU_EDGE_TOP);
	MenuWindow::SetPosition(312, 300);
	MenuWindow::SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);
	MenuWindow::Show();

	// (2) Initialize the object to NULL, so that no information is displayed
	_object = NULL;

	// (3) Initialize the description text box in the lower section of the window
	description.SetOwner(this);
	description.SetPosition(25.0f, 100.0f);
	description.SetDimensions(550.0f, 80.0f);
	description.SetDisplaySpeed(30);
	description.SetFont("default");
	description.SetDisplayMode(VIDEO_TEXT_INSTANT);
	description.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);

	// (4) Initialize the properties text box in the upper right section of the window
	properties.SetOwner(this);
	properties.SetPosition(50.0f, 150.0f);
	properties.SetDimensions(300.0f, 80.0f);
	properties.SetDisplaySpeed(30);
	properties.SetFont("default");
	properties.SetDisplayMode(VIDEO_TEXT_INSTANT);
	properties.SetTextAlignment(VIDEO_X_RIGHT, VIDEO_Y_TOP);
}



ObjectInfoWindow::~ObjectInfoWindow() {
	MenuWindow::Destroy();
}



void ObjectInfoWindow::SetObject(GlobalObject* obj) {
	_object = obj;
	if (obj == NULL) {
		description.ClearText();
		properties.ClearText();
		return;
	}

	description.SetDisplayText(_object->GetDescription());

	// Determine what properties to display depending on what type of object this is
	switch (obj->GetType()) {
		case GLOBAL_OBJECT_WEAPON:
			GlobalWeapon *weapon;
			weapon = dynamic_cast<GlobalWeapon*>(obj);
			properties.SetDisplayText(
				"PHYS ATK: " + NumberToString(weapon->GetPhysicalAttack()) + "\n" +
				"META ATK: " + NumberToString(weapon->GetMetaphysicalAttack())
			);
			break;
		case GLOBAL_OBJECT_HEAD_ARMOR:
		case GLOBAL_OBJECT_TORSO_ARMOR:
		case GLOBAL_OBJECT_ARM_ARMOR:
		case GLOBAL_OBJECT_LEG_ARMOR:
			GlobalArmor *armor;
			armor = dynamic_cast<GlobalArmor*>(obj);
			properties.SetDisplayText(
				"PHYS DEF: " + NumberToString(armor->GetPhysicalDefense()) + "\n" +
				"META DEF: " + NumberToString(armor->GetMetaphysicalDefense())
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

	// Draw the object's icon and name
	VideoManager->Move(350, 200);
	VideoManager->DrawImage(_object->GetIconImage());
	VideoManager->MoveRelative(60, 20);
	VideoManager->DrawText(_object->GetName());

	// Draw the object's description and stats text boxes
	description.Draw();
	properties.Draw();
}

// *****************************************************************************
// ****************************** ConfirmWindow ********************************
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
	options.SetSize(2, 1); // Two columns, one row
	options.SetCellSize(150.0f, 50.0f);
	options.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	options.SetFont("default");
	options.SetSelectMode(VIDEO_SELECT_SINGLE);
	options.SetCursorOffset(-50.0f, 20.0f);
	options.SetVerticalWrapMode(VIDEO_WRAP_MODE_NONE);

	vector<ustring> text;
	text.push_back(MakeUnicodeString("Confirm"));
	text.push_back(MakeUnicodeString("Cancel"));
	options.SetOptions(text);
	options.SetSelection(0);

	_object = NULL;
}



ConfirmWindow::~ConfirmWindow() {
	MenuWindow::Destroy();
}



void ConfirmWindow::Update() {
	options.GetEvent();

	if (InputManager->LeftPress()) {
		options.HandleLeftKey();
	}
	else if (InputManager->RightPress()) {
		options.HandleRightKey();
	}

	if (InputManager->CancelPress()) {
		current_shop->_shop_sounds["cancel"].PlaySound();
		SetObject(NULL);
		options.SetSelection(0);
		current_shop->_state = SHOP_STATE_LIST;
	}
	else if (InputManager->ConfirmPress()) {
		if (options.GetSelection() == 0) { // Confirm purchase
			if (GlobalManager->GetFunds() >= _object->GetPrice()) {
				// Add to global inventory
				GlobalManager->AddToInventory(_object->GetID());
				GlobalManager->SubtractFunds(_object->GetPrice());
				current_shop->_shop_sounds["coins"].PlaySound();
				current_shop->_action_window.UpdateFinanceText();
			}
			else {
				current_shop->_shop_sounds["bump"].PlaySound();
			}
			current_shop->_state = SHOP_STATE_LIST;
		}
		else {
			current_shop->_shop_sounds["cancel"].PlaySound();
		}

		// Return to previous window
		options.SetSelection(0);
		SetObject(NULL);
		current_shop->_state = SHOP_STATE_LIST;
	}
}



void ConfirmWindow::Draw() {
	if (_object == NULL)
		return;

	MenuWindow::Draw();
	options.Draw();
	VideoManager->PushState();
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->Move(512, 450);
	VideoManager->DrawText("Make this purchase?");
	VideoManager->PopState();
}



void ConfirmWindow::SetObject(GlobalObject* obj) {
	_object = obj;

	if (_object == NULL)
		MenuWindow::Hide();
	else
		MenuWindow::Show();
}

// *****************************************************************************
// **************************** SellConfirmWindow ******************************
// *****************************************************************************

SellConfirmWindow::SellConfirmWindow() {
	// (1) Create the confirmation window in the center of the screen
	MenuWindow::Create(400, 200);
	MenuWindow::SetPosition(512, 384);
	MenuWindow::SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);

	// (2) Initialize the option list
	options.SetOwner(this);
	options.SetPosition(100.0f, 100.0f);
	options.SetSize(2, 1); // Two columns, one row
	options.SetCellSize(150.0f, 50.0f);
	options.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	options.SetFont("default");
	options.SetSelectMode(VIDEO_SELECT_SINGLE);
	options.SetCursorOffset(-50.0f, 20.0f);
	options.SetVerticalWrapMode(VIDEO_WRAP_MODE_NONE);

	vector<ustring> text;
	text.push_back(MakeUnicodeString("Confirm"));
	text.push_back(MakeUnicodeString("Cancel"));
	options.SetOptions(text);
	options.SetSelection(0);

	_object_id = 0;
}



SellConfirmWindow::~SellConfirmWindow() {
	MenuWindow::Destroy();
}



void SellConfirmWindow::Update() {
	options.GetEvent();

	if (InputManager->LeftPress()) {
		options.HandleLeftKey();
	}
	else if (InputManager->RightPress()) {
		options.HandleRightKey();
	}

	if (InputManager->CancelPress()) {
		current_shop->_shop_sounds["cancel"].PlaySound();
		SetObject(NULL);
		current_shop->_info_window.SetObject(NULL);
		options.SetSelection(0);
		current_shop->_state = SHOP_STATE_SELL;
	}
	else if (InputManager->ConfirmPress()) {
		if (options.GetSelection() == 0) { // Confirm sale
			std::map<uint32, GlobalObject*>::iterator iter = GlobalManager->GetInventory()->find(_object_id);
			GlobalManager->AddFunds(iter->second->GetPrice() / 2);
			GlobalManager->DecrementObjectCount(_object_id, 1);
			current_shop->_shop_sounds["coins"].PlaySound();
			current_shop->_action_window.UpdateFinanceText();
			current_shop->_sell_window.UpdateSellList();
			current_shop->_sell_window.object_list.SetSelection(0);
			if (GlobalManager->GetInventory()->empty() == true) {
				current_shop->_state = SHOP_STATE_ACTION;
			}
			else {
				current_shop->_state = SHOP_STATE_SELL;
			}
		}
		else {
			current_shop->_shop_sounds["cancel"].PlaySound();
			current_shop->_state = SHOP_STATE_SELL;
		}

		// Return to previous window
		options.SetSelection(0);
		SetObject(NULL);
		current_shop->_info_window.SetObject(NULL);
	}
}



void SellConfirmWindow::Draw() {
	if (_object_id == 0)
		return;

	MenuWindow::Draw();
	options.Draw();
	VideoManager->PushState();
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->Move(512, 450);
	VideoManager->DrawText("Sell this item?");
	VideoManager->PopState();
}



void SellConfirmWindow::SetObject(GlobalObject* obj) {
	
	if (obj == NULL) {
		_object_id = 0;
		MenuWindow::Hide();
	}
	else {
		_object_id = obj->GetID();
		MenuWindow::Show();
	}
}

} // namespace private_shop

} // namespace hoa_shop
