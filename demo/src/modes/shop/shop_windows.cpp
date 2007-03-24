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
		options.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
		if (options.GetSelection() == 0) { // Buy
			current_shop->_shop_state = SHOP_STATE_LIST;
			current_shop->_list_window.hide_options = false;
			current_shop->_info_window.SetObject(current_shop->_all_objects[0]);
			current_shop->_shop_sounds["confirm"].PlaySound();
		}
		else if (options.GetSelection() == 1) { // Sell
			options.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
			current_shop->_shop_sounds["bump"].PlaySound();
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
		current_shop->_shop_sounds["coins"].PlaySound();
	}
	else if (InputManager->CancelPress()) {
		hide_options = true;
		current_shop->_shop_state = SHOP_STATE_ACTION;
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

	if (_object == NULL)
		return;

	// Draw the object's icon and name
	VideoManager->Move(350, 200);
	VideoManager->DrawImage(_object->GetIconImage());
	VideoManager->MoveRelative(60, 20);
	VideoManager->DrawText(_object->GetName());

	// Draw the object's description and stats text boxes
	description.Draw();
	properties.Draw();
}


} // namespace private_shop

} // namespace hoa_shop
