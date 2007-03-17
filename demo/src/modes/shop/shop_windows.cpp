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
#include "video.h"
#include "input.h"
#include "system.h"
#include "mode_manager.h"

#include "shop.h"
#include "shop_windows.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_mode_manager;

namespace hoa_shop {

namespace private_shop {

// *****************************************************************************
// ***************************** ShopActionWindow ******************************
// *****************************************************************************

ShopActionWindow::ShopActionWindow() {
	MenuWindow::Create(200, 600, ~VIDEO_MENU_EDGE_RIGHT);
	MenuWindow::SetPosition(112, 684);
	MenuWindow::SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);
	MenuWindow::Show();

	options.SetOwner(this);
	options.SetCellSize(150.0f, 50.0f);
	options.SetSize(1, 3); // One column, numerous rows
	options.SetPosition(25.0f, 600.0f);
	options.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	options.SetFont("default");
	options.SetSelectMode(VIDEO_SELECT_SINGLE);
	options.SetCursorOffset(-5.0f, -5.0f);
	options.SetVerticalWrapMode(VIDEO_WRAP_MODE_NONE);

	vector<ustring> text;
	text.push_back(MakeUnicodeString("Buy"));
	text.push_back(MakeUnicodeString("Sell"));
	text.push_back(MakeUnicodeString("Exit"));
	options.SetOptions(text);
	options.SetSelection(0);
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
		}
		else if (options.GetSelection() == 1) { // Sell

		}
		else if (options.GetSelection() == 2) { // Exit
			ModeManager->Pop();
		}
		else {
			if (SHOP_DEBUG)
				cerr << "SHOP WARNING: invalid selection in action window: " << options.GetSelection() << endl;
			ModeManager->Pop();
		}
	}
	else if (InputManager->CancelPress()) {
		ModeManager->Pop();
	}
	else if (InputManager->UpPress()) {
		options.HandleUpKey();
	}
	else if (InputManager->DownPress()) {
		options.HandleDownKey();
	}
}



void ShopActionWindow::Draw() {
	hoa_video::MenuWindow::Draw();
	options.Draw();
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
	object_list.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	object_list.SetFont("default");
	object_list.SetSelectMode(VIDEO_SELECT_SINGLE);
	object_list.SetCursorOffset(-35.0f, -4.0f);
	object_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_NONE);
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
	}
	else if (InputManager->CancelPress()) {
		ModeManager->Pop();
	}
	else if (InputManager->UpPress()) {
		object_list.HandleUpKey();
	}
	else if (InputManager->DownPress()) {
		object_list.HandleDownKey();
	}
}



void ObjectListWindow::Draw() {
	MenuWindow::Draw();
	if (option_text.empty() == false)
		object_list.Draw();
}

// *****************************************************************************
// ***************************** ObjectInfoWindow ******************************
// *****************************************************************************

ObjectInfoWindow::ObjectInfoWindow() {
	MenuWindow::Create(600, 200, ~VIDEO_MENU_EDGE_TOP);
	MenuWindow::SetPosition(312, 284);
	MenuWindow::SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);
	MenuWindow::Show();
}



ObjectInfoWindow::~ObjectInfoWindow() {
	MenuWindow::Destroy();
}



void ObjectInfoWindow::Draw() {
	MenuWindow::Draw();

	if (object == NULL)
		return;

	// Draw the object's name, icon, description, and stats

}


} // namespace private_shop

} // namespace hoa_shop
