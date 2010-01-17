///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_utils.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for shop mode utility code.
***
*** This file contains utility code that is shared among the various shop mode
*** classes.
*** ***************************************************************************/

#include "video.h"

#include "global.h"

#include "shop_utils.h"
#include "shop.h"

using namespace std;

using namespace hoa_utils;

using namespace hoa_system;
using namespace hoa_video;

using namespace hoa_global;

namespace hoa_shop {

namespace private_shop {

// *****************************************************************************
// ***** ShopObject class methods
// *****************************************************************************

ShopObject::ShopObject(hoa_global::GlobalObject* object, bool sold_by_shop) :
	_object(object),
	_sold_in_shop(sold_by_shop),
	_buy_price(0),
	_sell_price(0),
	_own_count(0),
	_stock_count(0),
	_buy_count(0),
	_sell_count(0)
{
	assert(_object != NULL);
}



void ShopObject::SetPricing(SHOP_PRICE_LEVEL buy_level, SHOP_PRICE_LEVEL sell_level) {
	_buy_price = _object->GetPrice();
	_sell_price = _object->GetPrice();

	switch (buy_level) {
		case SHOP_PRICE_VERY_GOOD:
			_buy_price *= BUY_PRICE_VERY_GOOD;
			break;
		case SHOP_PRICE_GOOD:
			_buy_price *= BUY_PRICE_GOOD;
			break;
		case SHOP_PRICE_STANDARD:
			_buy_price *= BUY_PRICE_STANDARD;
			break;
		case SHOP_PRICE_POOR:
			_buy_price *= BUY_PRICE_POOR;
			break;
		case SHOP_PRICE_VERY_POOR:
			_buy_price *= BUY_PRICE_VERY_POOR;
			break;
		default:
			IF_PRINT_WARNING(SHOP_DEBUG) << "unknown buy level: " << buy_level << endl;
	}

	switch (sell_level) {
		case SHOP_PRICE_VERY_GOOD:
			_sell_price *= SELL_PRICE_VERY_GOOD;
			break;
		case SHOP_PRICE_GOOD:
			_sell_price *= SELL_PRICE_GOOD;
			break;
		case SHOP_PRICE_STANDARD:
			_sell_price *= SELL_PRICE_STANDARD;
			break;
		case SHOP_PRICE_POOR:
			_sell_price *= SELL_PRICE_POOR;
			break;
		case SHOP_PRICE_VERY_POOR:
			_sell_price *= SELL_PRICE_VERY_POOR;
			break;
		default:
			IF_PRINT_WARNING(SHOP_DEBUG) << "unknown sell level: " << sell_level << endl;
	}
}



void ShopObject::IncrementOwnCount(uint32 inc) {
	_own_count += inc;
}



void ShopObject::IncrementStockCount(uint32 inc) {
	_stock_count += inc;
}



void ShopObject::IncrementBuyCount(uint32 inc) {
	uint32 old_count = _buy_count;
	if (inc == 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function received an argument with a value of zero" << endl;
		return;
	}

	_buy_count += inc;
	if (_stock_count < _buy_count) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "incremented buy count beyond the amount available in stock" << endl;
		_buy_count = old_count;
		return;
	}
	if (old_count == 0) {
		ShopMode::CurrentInstance()->AddObjectToBuyList(this);
	}
}



void ShopObject::IncrementSellCount(uint32 inc) {
	uint32 old_count = _sell_count;
	if (inc == 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function received an argument with a value of zero" << endl;
		return;
	}

	_sell_count += inc;
	if (_sell_count > _own_count) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "incremented sell count beyond the amount available to be sold" << endl;
		_sell_count -= inc;
		return;
	}
	if (old_count == 0) {
		ShopMode::CurrentInstance()->AddObjectToSellList(this);
	}
}



void ShopObject::DecrementOwnCount(uint32 dec) {
	if (dec > _own_count) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "attempted to decrement own count below zero" << endl;
		return;
	}

	_own_count -= dec;

	if (_own_count < _sell_count) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "decremented own count below that of the sell count" << endl;
		_own_count += dec;
	}
}



void ShopObject::DecrementStockCount(uint32 dec) {
	if (dec > _stock_count) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "attempted to decrement stock count below zero" << endl;
		return;
	}

	_stock_count -= dec;

	if (_stock_count < _buy_count) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "decremented stock count below that of the buy count" << endl;
		_stock_count += dec;
	}
}



void ShopObject::DecrementBuyCount(uint32 dec) {
	if (dec == 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function received an argument with a value of zero" << endl;
		return;
	}

	if (dec > _buy_count) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "attempted to decrement buy count below zero" << endl;
		return;
	}

	_buy_count -= dec;
	if (_buy_count == 0) {
		ShopMode::CurrentInstance()->RemoveObjectFromBuyList(this);
	}
}



void ShopObject::DecrementSellCount(uint32 dec) {
	if (dec == 0) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function received an argument with a value of zero" << endl;
		return;
	}

	if (dec > _sell_count) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "attempted to decrement sell count below zero" << endl;
		return;
	}

	_sell_count -= dec;
	if (_sell_count == 0) {
		ShopMode::CurrentInstance()->RemoveObjectFromSellList(this);
	}
}

// *****************************************************************************
// ***** ObjectCategoryDisplay class methods
// *****************************************************************************

ObjectCategoryDisplay::ObjectCategoryDisplay() :
	_category_icon(NULL),
	_last_icon(NULL)
{
	// The default time it takes to transition graphics/text to a new category (in milliseconds)
	const uint32 DEFAULT_TRANSITION_TIME = 500;

	_transition_timer.Initialize(DEFAULT_TRANSITION_TIME, 0, ShopMode::CurrentInstance());
	_category_text.SetDisplaySpeed(static_cast<float>(DEFAULT_TRANSITION_TIME));
}



ObjectCategoryDisplay::~ObjectCategoryDisplay() {
	_category_icon = NULL;
	_last_icon = NULL;
}



void ObjectCategoryDisplay::SetTransitionTime(uint32 time) {
	// Transition times can only be set when the timer is in the initial state
	if (_transition_timer.IsInitial() == false) {
		_transition_timer.Reset();
	}

	_transition_timer.SetDuration(time);
	_category_text.SetDisplaySpeed(static_cast<float>(time) / 1000.0f);
}



void ObjectCategoryDisplay::ChangeCategory(ustring& name, const StillImage* icon) {
	if (icon == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function's icon argument was passed a NULL pointer" << endl;
	}

	_last_icon = _category_icon;
	_category_icon = icon;

	_category_text.SetDisplayText(name);

	_transition_timer.Reset();
	_transition_timer.Run();
}



void ObjectCategoryDisplay::Update() {
	_category_text.Update();
}

// *****************************************************************************
// ***** ObjectListDisplay class methods
// *****************************************************************************

void ObjectListDisplay::Clear() {
	_objects = NULL;
	_identify_list.ClearOptions();
	_property_list.ClearOptions();
}



void ObjectListDisplay::PopulateList(vector<ShopObject*>* objects) {
	if (objects == NULL) {
		IF_PRINT_WARNING(SHOP_DEBUG) << "function was given a NULL pointer argument" << endl;
		return;
	}

	_objects = objects;
	RefreshList();
}



void ObjectListDisplay::RefreshAllEntries() {
	for (uint32 i = 0; i < _objects->size(); i++) {
		RefreshEntry(i);
	}
}



void ObjectListDisplay::Update() {
	_identify_list.Update();
	_property_list.Update();
}



void ObjectListDisplay::Draw() {
	_identify_list.Draw();
	_property_list.Draw();
}

} // namespace private_shop

} // namespace hoa_shop
