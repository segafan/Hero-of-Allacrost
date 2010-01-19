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
*** \brief   Source file for buy interface of shop mode
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
// ***** BuyInterface class methods
// *****************************************************************************

BuyInterface::BuyInterface() :
	_list_view_active(true),
	_buy_list_view(new BuyListView())
{}



BuyInterface::~BuyInterface() {
	delete _buy_list_view;
}



void BuyInterface::Initialize() {
	_buy_list_view->Initialize();
}



void BuyInterface::MakeActive() {
	ShopMode::CurrentInstance()->ObjectViewer()->ChangeViewMode(SHOP_VIEW_MODE_LIST);
	ShopMode::CurrentInstance()->ObjectViewer()->SetSelectedObject(_buy_list_view->GetSelectedObject());
}



void BuyInterface::Update() {
	ShopObject* obj = _buy_list_view->GetSelectedObject();
	ShopObject* new_obj = NULL;

	if (_list_view_active == true) {
		if (InputManager->ConfirmPress()) {
			// TODO: Bring up an "instant purchase" confirmation menu
		}
		else if (InputManager->CancelPress()) {
			ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_ROOT);
			ShopMode::CurrentInstance()->Media()->GetSound("cancel")->Play();
		}
		else if (InputManager->MenuPress()) {
			ShopMode::CurrentInstance()->ObjectViewer()->ChangeViewMode(SHOP_VIEW_MODE_INFO);
			_list_view_active = false;
		}

		// Swap cycles through the object categories
		else if (InputManager->SwapPress() && (_buy_list_view->GetNumberObjectCategories() > 1)) {
			new_obj = _buy_list_view->ChangeCategory(true);
			ShopMode::CurrentInstance()->ObjectViewer()->SetSelectedObject(new_obj);
			ShopMode::CurrentInstance()->Media()->GetSound("confirm")->Play();
		}

		// Up/down changes the selected object in the current list
		else if (InputManager->UpPress()) {
			new_obj = _buy_list_view->ChangeSelection(false);
			if (new_obj != obj) {
				ShopMode::CurrentInstance()->ObjectViewer()->SetSelectedObject(new_obj);
				ShopMode::CurrentInstance()->Media()->GetSound("confirm")->Play();
			}
		}
		else if (InputManager->DownPress()) {
			new_obj = _buy_list_view->ChangeSelection(true);
			if (new_obj != obj) {
				ShopMode::CurrentInstance()->ObjectViewer()->SetSelectedObject(new_obj);
				ShopMode::CurrentInstance()->Media()->GetSound("confirm")->Play();
			}
		}

		// Left/right change the quantity of the object to buy
		else if (InputManager->LeftPress()) {
			if (_buy_list_view->ChangeQuantity(false) == true) {
				ShopMode::CurrentInstance()->Media()->GetSound("confirm")->Play();
			}
			else {
				ShopMode::CurrentInstance()->Media()->GetSound("bump")->Play();
			}
		}
		else if (InputManager->RightPress()) {
			if (_buy_list_view->ChangeQuantity(true) == true) {
				ShopMode::CurrentInstance()->Media()->GetSound("confirm")->Play();
			}
			else {
				ShopMode::CurrentInstance()->Media()->GetSound("bump")->Play();
			}
		}

		// Left select/right select change the quantity of the object to buy by 10 at a time
		else if (InputManager->LeftSelectPress()) {
			if (_buy_list_view->ChangeQuantity(false, 10) == true) {
				ShopMode::CurrentInstance()->Media()->GetSound("confirm")->Play();
			}
			else {
				ShopMode::CurrentInstance()->Media()->GetSound("bump")->Play();
			}
		}
		else if (InputManager->RightSelectPress()) {
			if (_buy_list_view->ChangeQuantity(true, 10) == true) {
				ShopMode::CurrentInstance()->Media()->GetSound("confirm")->Play();
			}
			else {
				ShopMode::CurrentInstance()->Media()->GetSound("bump")->Play();
			}
		}

		_buy_list_view->Update();
	} // if (_list_view_active == true)

	else {
		if (InputManager->MenuPress()) {
			ShopMode::CurrentInstance()->ObjectViewer()->ChangeViewMode(SHOP_VIEW_MODE_LIST);
			_list_view_active = true;
		}
	}

	ShopMode::CurrentInstance()->ObjectViewer()->Update();
}



void BuyInterface::Draw() {
	if (_list_view_active == true) {
		_buy_list_view->Draw();
	}

	ShopMode::CurrentInstance()->ObjectViewer()->Draw();
}

// *****************************************************************************
// ***** BuyCategoryDisplay class methods
// *****************************************************************************

BuyCategoryDisplay::BuyCategoryDisplay() :
	ObjectCategoryDisplay()
{
	_category_text.SetOwner(ShopMode::CurrentInstance()->GetMiddleWindow());
	_category_text.SetPosition(25.0f, 175.0f);
	_category_text.SetDimensions(125.0f, 30.0f);
	_category_text.SetTextStyle(TextStyle("text22"));
	_category_text.SetDisplayMode(VIDEO_TEXT_FADELINE);
	_category_text.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_category_text.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
}



void BuyCategoryDisplay::Draw() {
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->Move(200.0f, 410.0f);

	// TODO: use animation timer to fade beween current and last category icons
	if (_category_icon != NULL) {
		_category_icon->Draw();
	}

	_category_text.Draw();
}

// *****************************************************************************
// ***** BuyListDisplay class methods
// *****************************************************************************

BuyListDisplay::BuyListDisplay() :
	ObjectListDisplay()
{
	_identify_list.SetOwner(ShopMode::CurrentInstance()->GetMiddleWindow());
	_identify_list.SetPosition(180.0f, 330.0f);
	_identify_list.SetDimensions(300.0f, 300.0f, 1, 255, 1, 8);
	_identify_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_identify_list.SetTextStyle(TextStyle("text22"));
	_identify_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
	_identify_list.SetSelectMode(VIDEO_SELECT_SINGLE);
	_identify_list.SetCursorOffset(-50.0f, 20.0f);
	_identify_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_NONE);
	_identify_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);

	_property_list.SetOwner(ShopMode::CurrentInstance()->GetMiddleWindow());
	_property_list.SetPosition(480.0f, 330.0f);
	_property_list.SetDimensions(300.0f, 300.0f, 4, 255, 4, 8);
	_property_list.SetOptionAlignment(VIDEO_X_RIGHT, VIDEO_Y_CENTER);
	_property_list.SetTextStyle(TextStyle("text22"));
	_property_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	_property_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_NONE);
	_property_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
}



void BuyListDisplay::RefreshList() {
	if (_objects == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "no object data is available" << endl;
		return;
	}

	_identify_list.ClearOptions();
	_property_list.ClearOptions();

	ShopObject* shop_obj = NULL;
	for (uint32 i = 0; i < _objects->size(); i++) {
		shop_obj = (*_objects)[i];
		// Add an entry with the icon image of the object (scaled down by 4x to 30x30 pixels) followed by the object name
		_identify_list.AddOption(MakeUnicodeString("<" + shop_obj->GetObject()->GetIconImage().GetFilename() + "><30>")
			+ shop_obj->GetObject()->GetName());
		_identify_list.GetEmbeddedImage(i)->SetDimensions(30.0f, 30.0f);

		// Add an option for each object property in the order of: price, stock, number owned, and amount to buy
		_property_list.AddOption(MakeUnicodeString(NumberToString(shop_obj->GetBuyPrice())));
		_property_list.AddOption(MakeUnicodeString("x" + NumberToString(shop_obj->GetStockCount())));
		_property_list.AddOption(MakeUnicodeString("x" + NumberToString(shop_obj->GetOwnCount())));
		_property_list.AddOption(MakeUnicodeString("x" + NumberToString(shop_obj->GetBuyCount())));
	}

	_identify_list.SetSelection(0);
	_property_list.SetSelection(0);
}



void BuyListDisplay::RefreshEntry(uint32 index) {
	const uint8 NUMBER_BUY_PROPERTIES = 4; // How many columns of data are in a single row of the _property_list

	if (_objects == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "no object data is available" << endl;
		return;
	}
	if (index >= _objects->size()) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "index argument was out of range: " << index << endl;
		return;
	}

	// Update only the stock, number owned, and amount to sell. The price does not require updating
	ShopObject* shop_obj = (*_objects)[index];
	_property_list.SetOptionText((index * NUMBER_BUY_PROPERTIES) + 1, MakeUnicodeString("x" + NumberToString(shop_obj->GetStockCount())));
	_property_list.SetOptionText((index * NUMBER_BUY_PROPERTIES) + 2, MakeUnicodeString("x" + NumberToString(shop_obj->GetOwnCount())));
	_property_list.SetOptionText((index * NUMBER_BUY_PROPERTIES) + 3, MakeUnicodeString("x" + NumberToString(shop_obj->GetBuyCount())));
}

// *****************************************************************************
// ***** BuyListView class methods
// *****************************************************************************

BuyListView::BuyListView() :
	current_category(0)
{
	category_header.SetStyle(TextStyle("title24"));
	category_header.SetText(MakeUnicodeString("Category"));

	name_header.SetStyle(TextStyle("title24"));
	name_header.SetText(MakeUnicodeString("Name"));

	properties_header.SetPosition(480.0f, 390.0f);
	properties_header.SetDimensions(300.0f, 30.0f, 4, 1, 4, 1);
	properties_header.SetOptionAlignment(VIDEO_X_RIGHT, VIDEO_Y_CENTER);
	properties_header.SetTextStyle(TextStyle("title24"));
	properties_header.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
	properties_header.AddOption(MakeUnicodeString("Price"));
	properties_header.AddOption(MakeUnicodeString("Stock"));
	properties_header.AddOption(MakeUnicodeString("Own"));
	properties_header.AddOption(MakeUnicodeString("Buy"));
}



BuyListView::~BuyListView() {
	for (uint32 i = 0; i < list_displays.size(); i++) {
		delete list_displays[i];
	}
}



void BuyListView::Initialize() {
	properties_header.SetOwner(ShopMode::CurrentInstance()->GetMiddleWindow());

	// ---------- (1): Load all category names and icon images to be used
	// The number of categories of objects that the shop has for sale
	uint8 num_obj_categories = ShopMode::CurrentInstance()->Media()->GetObjectCategoryNames()->size();

	category_names = *(ShopMode::CurrentInstance()->Media()->GetObjectCategoryNames());
	vector<StillImage>* cat_icons = ShopMode::CurrentInstance()->Media()->GetObjectCategoryIcons();
	for (uint32 i = 0; i < cat_icons->size(); i++) {
		category_icons.push_back(&(cat_icons->at(i)));
	}

	// Set the initial category to the last category that was added (this is usually "All Wares")
	current_category = num_obj_categories - 1;
	// Initialize the category display with the initial category
	category_display.ChangeCategory(category_names[current_category], category_icons[current_category]);

	// ---------- (2): Populating the object_data container and determine category index mappings
	for (uint32 i = 0; i < num_obj_categories; i++) {
		object_data.push_back(vector<ShopObject*>());
	}

	// Bit-vector that indicates what types of objects are sold in the shop
	uint8 deal_types = ShopMode::CurrentInstance()->GetDealTypes();
	// Holds the index to the object_data vector where the container for a specific object type is located
	vector<uint32> type_index(GLOBAL_OBJECT_TOTAL, 0);
	// Used to set the appropriate data in the type_index vector
	uint32 next_index = 0;
	// Used to do a bit-by-bit analysis of the deal_types variable
	uint8 bit_x = 0x01;

	// This loop determines where each type of object should be placed in the object_data container. For example,
	// if the available categories in the shop are items, weapons, shards, and all wares, the size of object_data
	// will be four. But when we go to add an object of one of these types into the object_data container, we need
	// to know the correct index for each type of object. These indeces are stored in the type_index vector. The
	// size of this vector is the number of object types, so it becomes simple to map each object type to its correct
	// location in object_data.
	for (uint8 i = 0; i < GLOBAL_OBJECT_TOTAL; i++, bit_x <<= 1) {
		// Check if the type is available by doing a bit-wise comparison
		if (deal_types & bit_x) {
			type_index[i] = next_index++;
		}
	}

	// ---------- (3): Populate the object_data containers
	// Used to temporarily hold a pointer to a valid shop object
	ShopObject* obj = NULL;
	// Pointer to the container of all objects that are bought/sold/traded in the ship
	map<uint32, ShopObject>* shop_objects = ShopMode::CurrentInstance()->GetShopObjects();

	for (map<uint32, ShopObject>::iterator i = shop_objects->begin(); i != shop_objects->end(); i++) {
		obj = &(i->second);

		if (obj->IsSoldInShop() == true) {
			switch (obj->GetObject()->GetObjectType()) {
				case GLOBAL_OBJECT_ITEM:
					object_data[type_index[0]].push_back(obj);
					break;
				case GLOBAL_OBJECT_WEAPON:
					object_data[type_index[1]].push_back(obj);
					break;
				case GLOBAL_OBJECT_HEAD_ARMOR:
					object_data[type_index[2]].push_back(obj);
					break;
				case GLOBAL_OBJECT_TORSO_ARMOR:
					object_data[type_index[3]].push_back(obj);
					break;
				case GLOBAL_OBJECT_ARM_ARMOR:
					object_data[type_index[4]].push_back(obj);
					break;
				case GLOBAL_OBJECT_LEG_ARMOR:
					object_data[type_index[5]].push_back(obj);
					break;
				case GLOBAL_OBJECT_SHARD:
					object_data[type_index[6]].push_back(obj);
					break;
				case GLOBAL_OBJECT_KEY_ITEM:
					object_data[type_index[7]].push_back(obj);
					break;
				default:
					IF_PRINT_WARNING(SHOP_DEBUG) << "added object of unknown type: " << obj->GetObject()->GetObjectType() << endl;
					break;
			}

			// If there is an "All Wares" category, make sure the object gets added there as well
			if (num_obj_categories > 1) {
				object_data.back().push_back(obj);
			}
		}
	}

	// ---------- (4): Create the buy displays using the object data that is now ready
	for (uint32 i = 0; i < object_data.size(); i++) {
		BuyListDisplay* new_list = new BuyListDisplay();
		new_list->PopulateList(&(object_data[i]));
		list_displays.push_back(new_list);
	}
} // void BuyListView::Initialize()



void BuyListView::Update() {
	category_display.Update();
	list_displays[current_category]->Update();
}



void BuyListView::Draw() {
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_BOTTOM, 0);
	VideoManager->Move(200.0f, 558.0f);
	category_header.Draw();

	VideoManager->SetDrawFlags(VIDEO_X_LEFT, 0);
	VideoManager->MoveRelative(95.0f, 0.0f);
	name_header.Draw();

	properties_header.Draw();
	category_display.Draw();
	list_displays[current_category]->Draw();
}



ShopObject* BuyListView::ChangeCategory(bool left_or_right) {
	if (GetNumberObjectCategories() == 1) {
		return GetSelectedObject();
	}

	if (left_or_right == false) {
		current_category = (current_category == 0) ? (object_data.size() -1) : (current_category - 1);
	}
	else {
		current_category = (current_category >= (object_data.size() - 1)) ? 0 : (current_category + 1);
	}

	category_display.ChangeCategory(category_names[current_category], category_icons[current_category]);
	// Refresh all entries in the newly selected list is required because all objects are available
	// in two categories (their standard type and the "All Wares" category)
	list_displays[current_category]->RefreshAllEntries();
	return GetSelectedObject();
}



ShopObject* BuyListView::ChangeSelection(bool up_or_down) {
	BuyListDisplay* selected_list = list_displays[current_category];

	if (up_or_down == false) {
		selected_list->GetIdentifyList().InputUp();
		selected_list->GetPropertyList().InputUp();
	}
	else {
		selected_list->GetIdentifyList().InputDown();
		selected_list->GetPropertyList().InputDown();
	}

	return GetSelectedObject();
}



bool BuyListView::ChangeQuantity(bool less_or_more, uint32 amount) {
	ShopObject* selected_object = GetSelectedObject();
	// Holds the amount that the quantity will actually increase or decrease by. May be less than the
	// amount requested if there is an limitation such as shop stock or available funds
	uint32 change_amount = amount;

	if (less_or_more == false) {
		if (selected_object->GetBuyCount() == 0) {
			return false;
		}

		if (amount > selected_object->GetBuyCount()) {
			change_amount = selected_object->GetBuyCount();
		}
		selected_object->DecrementBuyCount(change_amount);
		ShopMode::CurrentInstance()->UpdateFinances(-(selected_object->GetBuyPrice() * change_amount), 0);
		list_displays[current_category]->RefreshEntry(list_displays[current_category]->GetIdentifyList().GetSelection());
		return true;
	}
	else {
		// Make sure that there is at least one more object in stock and the player has enough funds to purchase it
		if ((selected_object->GetBuyCount() >= selected_object->GetStockCount()) ||
			(selected_object->GetBuyPrice() > ShopMode::CurrentInstance()->GetTotalRemaining()))
		{
			return false;
		}

		// Determine if there's enough of the object in stock to buy. If not, buy as many left as possible
		if ((selected_object->GetStockCount() - selected_object->GetBuyCount()) < change_amount) {
			change_amount = selected_object->GetStockCount() - selected_object->GetBuyCount();
		}
		// Determine how many of the possible amount to buy the player can actually afford
		int32 total_cost = change_amount * selected_object->GetBuyPrice();
		while (total_cost > static_cast<int32>(ShopMode::CurrentInstance()->GetTotalRemaining())) {
			change_amount--;
			total_cost -= selected_object->GetBuyPrice();
		}

		selected_object->IncrementBuyCount(change_amount);
		ShopMode::CurrentInstance()->UpdateFinances((selected_object->GetBuyPrice() * change_amount), 0);
		list_displays[current_category]->RefreshEntry(list_displays[current_category]->GetIdentifyList().GetSelection());
		return true;
	}
}



ShopObject* BuyListView::GetSelectedObject() const {
	BuyListDisplay* selected_list = list_displays[current_category];
	uint32 selected_entry = selected_list->GetIdentifyList().GetSelection();
	return object_data[current_category][selected_entry];
}

} // namespace private_shop

} // namespace hoa_shop
