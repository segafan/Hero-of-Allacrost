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
// ***** BuyInterface class methods
// *****************************************************************************

BuyInterface::BuyInterface() :
	_current_datalist(0),
	_list_window(NULL),
	_info_window(NULL)
{
	_list_window = ShopMode::CurrentInstance()->GetListWindow();
	_info_window = ShopMode::CurrentInstance()->GetInfoWindow();
}



BuyInterface::~BuyInterface() {
	for (uint32 i = 0; i < _object_lists.size(); i++) {
		delete _object_lists[i];
	}
}



void BuyInterface::Initialize() {
	// Used to temporarily hold a pointer to a valid shop object
	ShopObject* obj = NULL;
	// Pointer to the container of all objects that are bought/sold/traded in the ship
	map<uint32, ShopObject>* shop_objects = ShopMode::CurrentInstance()->GetShopObjects();
	// Bit-vector that indicates what types of objects are sold in the shop
	uint8 obj_types = ShopMode::CurrentInstance()->GetDealTypes();
	// Holds the index within the _object_data vector where the container for a specific object type is
	vector<uint32> type_index(8, 0);

	// ---------- (1): Populating the _object_data container with an entry for each type of object dealt in the shop
	_object_data.push_back(vector<ShopObject*>()); // This first entry represents all objects
	uint32 next_index = 1; // Used to set the appropriate data in the type_index vector
	uint8 bit_x = 0x01; // Used to do a bit-by-bit analysis of the obj_types variable
	for (uint32 i = 0; i < type_index.size(); i++, bit_x <<= 1) {
		// Check if the type is available by doing a bit-wise comparison
		if (obj_types & bit_x) {
			_object_data.push_back(vector<ShopObject*>());
			type_index[i] = next_index;
			next_index++;
		}
	}

	// ---------- (2): Populate the object containers
	for (map<uint32, ShopObject>::iterator i = shop_objects->begin(); i != shop_objects->end(); i++) {
		obj = &(i->second);

		if (obj->IsSoldInShop() == true) {
			_object_data[0].push_back(obj);

			switch (obj->GetObject()->GetObjectType()) {
				case GLOBAL_OBJECT_ITEM:
					_object_data[type_index[0]].push_back(obj);
					break;
				case GLOBAL_OBJECT_WEAPON:
					_object_data[type_index[1]].push_back(obj);
					break;
				case GLOBAL_OBJECT_HEAD_ARMOR:
					_object_data[type_index[2]].push_back(obj);
					break;
				case GLOBAL_OBJECT_TORSO_ARMOR:
					_object_data[type_index[3]].push_back(obj);
					break;
				case GLOBAL_OBJECT_ARM_ARMOR:
					_object_data[type_index[4]].push_back(obj);
					break;
				case GLOBAL_OBJECT_LEG_ARMOR:
					_object_data[type_index[5]].push_back(obj);
					break;
				case GLOBAL_OBJECT_SHARD:
					_object_data[type_index[6]].push_back(obj);
					break;
				case GLOBAL_OBJECT_KEY_ITEM:
					_object_data[type_index[7]].push_back(obj);
					break;
				default:
					IF_PRINT_WARNING(SHOP_DEBUG) << "added object of unknown type: " << obj->GetObject()->GetObjectType() << endl;
					break;
			}
		}
	}

	// ---------- (3): Create the buy object lists using the object data that is now ready
	for (uint32 i = 0; i < _object_data.size(); i++) {
		BuyObjectList* new_list = new BuyObjectList();
		new_list->identifier_list.SetOwner(_list_window);
		new_list->properties_list.SetOwner(_list_window);
		new_list->PopulateList(&(_object_data[i]));

		_object_lists.push_back(new_list);
	}

	// ---------- (4): Initialize the list headers and object type icons
	// TODO: _object_types, _identifier_header, _properties_header
}



void BuyInterface::MakeActive() {
	_list_window->Show();
	_info_window->Show();

}



void BuyInterface::MakeInactive() {
	_list_window->Hide();
	_info_window->Hide();
}



void BuyInterface::Update() {
	if (InputManager->ConfirmPress() || InputManager->CancelPress()) {
		ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_ROOT);
	}

	if (InputManager->ConfirmPress()) {

	}
	else if (InputManager->CancelPress()) {

	}
	else if (InputManager->UpPress()) {

	}
	else if (InputManager->DownPress()) {

	}
	else if (InputManager->LeftPress()) {

	}
	else if (InputManager->RightPress()) {

	}
	else if (InputManager->LeftSelectPress()) {
		_current_datalist = (_current_datalist == 0) ? (_object_data.size() - 1) : (_current_datalist - 1);
	}
	else if (InputManager->RightSelectPress()) {
		_current_datalist = (_current_datalist >= (_object_data.size() - 1)) ? 0 : (_current_datalist + 1);
	}
}



void BuyInterface::Draw() {
	_list_window->Draw();
	_object_lists[_current_datalist]->Draw();

	_info_window->Draw();
}

// *****************************************************************************
// ***** BuyObjectList class methods
// *****************************************************************************

BuyObjectList::BuyObjectList() :
	object_data(NULL)
{
	identifier_list.SetPosition(100.0f, 362.0f);
	identifier_list.SetDimensions(400.0f, 300.0f, 1, 255, 1, 8);
	identifier_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	identifier_list.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	identifier_list.SetSelectMode(VIDEO_SELECT_SINGLE);
	identifier_list.SetCursorOffset(-50.0f, 20.0f);
	identifier_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_NONE);
	identifier_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);

	properties_list.SetPosition(500.0f, 362.0f);
	properties_list.SetDimensions(250.0f, 300.0f, 4, 255, 4, 8);
	properties_list.SetOptionAlignment(VIDEO_X_RIGHT, VIDEO_Y_CENTER);
	properties_list.SetTextStyle(VideoManager->Text()->GetDefaultStyle());
	properties_list.SetSelectMode(VIDEO_SELECT_SINGLE);
	properties_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_NONE);
	properties_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);

	// NOTE: Both the identifier and properties lists will have SetOwner() called for the menu
	// window that they exist on. This is done by the BuyInterface class shortly after this
	// constructor returns
}



void BuyObjectList::Clear() {
	object_data = NULL;
	identifier_list.ClearOptions();
	properties_list.ClearOptions();
}



void BuyObjectList::PopulateList(vector<ShopObject*>* objects) {
	if (objects == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function was given a NULL pointer argument" << endl;
		return;
	}

	object_data = objects;
	RefreshList();
}



void BuyObjectList::RefreshList() {
	if (object_data == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "no object data is available" << endl;
		return;
	}

	identifier_list.ClearOptions();
	properties_list.ClearOptions();

	ShopObject* shop_obj = NULL;
	for (uint32 i = 0; i < object_data->size(); i++) {
		shop_obj = (*object_data)[i];
		// Add an entry with the icon image of the object (scaled down by 4x to 30x30 pixels) followed by the object name
		identifier_list.AddOption(MakeUnicodeString("<" + shop_obj->GetObject()->GetIconImage().GetFilename() + ">       ")
			+ shop_obj->GetObject()->GetName());
		identifier_list.GetEmbeddedImage(i)->SetDimensions(30.0f, 30.0f);

		// Add an option for each object property in the order of: price, stock, number owned, and amount to buy
		properties_list.AddOption(MakeUnicodeString(NumberToString(shop_obj->GetBuyPrice())));
		properties_list.AddOption(MakeUnicodeString("x" + NumberToString(shop_obj->GetStockCount())));
		properties_list.AddOption(MakeUnicodeString("x" + NumberToString(shop_obj->GetOwnCount())));
		properties_list.AddOption(MakeUnicodeString("x" + NumberToString(shop_obj->GetBuyCount())));
	}

	identifier_list.SetSelection(0);
	properties_list.SetSelection(0);
}



void BuyObjectList::RefreshEntryProperties(uint32 index) {
	if (object_data == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "no object data is available" << endl;
		return;
	}

	if (index >= object_data->size()) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "index argument was out of range: " << index << endl;
		return;
	}

	ShopObject* shop_obj = (*object_data)[index];
	// Update only the stock, number owned, and amount to buy. The price option should not require updating
	properties_list.SetOptionText((index * 4) + 1, MakeUnicodeString("x" + NumberToString(shop_obj->GetStockCount())));
	properties_list.SetOptionText((index * 4) + 2, MakeUnicodeString("x" + NumberToString(shop_obj->GetOwnCount())));
	properties_list.SetOptionText((index * 4) + 3, MakeUnicodeString("x" + NumberToString(shop_obj->GetBuyCount())));
}


void BuyObjectList::Update() {
	identifier_list.Update();
	properties_list.Update();
}



void BuyObjectList::Draw() {
	identifier_list.Draw();
	properties_list.Draw();
}

} // namespace private_shop

} // namespace hoa_shop
