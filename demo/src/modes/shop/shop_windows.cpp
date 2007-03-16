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

// ****************************************************************************
// ***************************** ShopActionWindow *****************************
// ****************************************************************************

ShopActionWindow::ShopActionWindow() {
	MenuWindow::Create(800, 200, VIDEO_MENU_EDGE_ALL, VIDEO_MENU_EDGE_BOTTOM);
	MenuWindow::SetPosition(512, 500);
	MenuWindow::SetAlignment(VIDEO_X_CENTER, VIDEO_Y_BOTTOM);
	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);
	MenuWindow::Show();

	options.SetOwner(this);
	options.SetCellSize(150.0f, 50.0f);
	options.SetSize(2, 1);
	options.SetPosition(20.0f, 20.0f);
	options.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	options.SetFont("default");
	options.SetSelectMode(VIDEO_SELECT_SINGLE);
	options.SetCursorOffset(-35.0f, -4.0f);
	options.SetHorizontalWrapMode(VIDEO_WRAP_MODE_NONE);

	vector<ustring> text;
	text.push_back(MakeUnicodeString("Buy"));
	text.push_back(MakeUnicodeString("Sell"));
	options.SetOptions(text);
	options.SetSelection(0);
} // ShopActionWindow::ShopActionWindow()



ShopActionWindow::~ShopActionWindow() {
	MenuWindow::Destroy();
}



void ShopActionWindow::Update() {
	MenuWindow::Update(SystemManager->GetUpdateTime());

	if (InputManager->ConfirmPress()) {
		options.HandleConfirmKey();
		current_shop->SetState(SHOP_STATE_LIST);
	}
	else if (InputManager->CancelPress()) {
		ModeManager->Pop();
	}
	else if (InputManager->LeftPress()) {
		options.HandleLeftKey();
	}
	else if (InputManager->RightPress()) {
		options.HandleRightKey();
	}
}

// ****************************************************************************
// ***************************** ObjectListWindow *****************************
// ****************************************************************************

// ObjectList::ObjectList() {
// 	MenuWindow::Create(800, 400, VIDEO_MENU_EDGE_ALL, VIDEO_MENU_EDGE_BOTTOM);
// 	MenuWindow::SetPosition(512, 100);
// 	MenuWindow::SetAlignment(VIDEO_X_CENTER, VIDEO_Y_BOTTOM);
// 	MenuWindow::SetDisplayMode(VIDEO_MENU_INSTANT);
// 	MenuWindow::Show();
//
// 	object_list.SetOwner(this);
// 	object_list.SetCellSize(800.0f, 50.0f);
// 	object_list.SetSize(1, 6);
// 	object_list.SetPosition(512.0f, 384.0f);
// 	object_list.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
// 	object_list.SetFont("default");
// 	object_list.SetSelectMode(VIDEO_SELECT_SINGLE);
// 	object_list.SetCursorOffset(-35.0f, -4.0f);
// 	object_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_NONE);
// }

} // namespace private_shop

} // namespace hoa_shop
