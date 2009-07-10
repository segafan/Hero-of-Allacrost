///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_utils.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for shop mode utility code.
***
*** This file contains common code that is shared among the various shop mode
*** classes.
*** ***************************************************************************/

#ifndef __SHOP_UTILS_HEADER__
#define __SHOP_UTILS_HEADER__

#include "defs.h"
#include "utils.h"

namespace hoa_shop {

//! \brief Used to indicate what window has control of user input
enum SHOP_PRICE_LEVEL {
	SHOP_PRICE_INVALID   = -1,
	SHOP_PRICE_VERY_HIGH =  0,
	SHOP_PRICE_HIGH      =  1,
	SHOP_PRICE_STANDARD  =  2,
	SHOP_PRICE_LOW       =  3,
	SHOP_PRICE_VERY_LOW  =  4,
	SHOP_PRICE_TOTAL     =  5
};

namespace private_shop {

//! \brief Used to indicate what window has control of user input
enum SHOP_STATE {
	SHOP_STATE_INVALID   = -1,
	SHOP_STATE_ACTION    =  0,
	SHOP_STATE_BUY       =  1,
	SHOP_STATE_SELL      =  2,
	SHOP_STATE_CONFIRM   =  3,
	SHOP_STATE_PROMPT    =  4,
	SHOP_STATE_TOTAL     =  5
};

//! \name Object deal types
//! \brief Constants used to determine the types of merchandise that the shop deals with
//@{
const uint8 DEALS_ITEMS        = 0x01;
const uint8 DEALS_WEAPONS      = 0x02;
const uint8 DEALS_HEAD_ARMOR   = 0x04;
const uint8 DEALS_TORSO_ARMOR  = 0x08;
const uint8 DEALS_ARM_ARMOR    = 0x10;
const uint8 DEALS_LEG_ARMOR    = 0x20;
const uint8 DEALS_SHARDS       = 0x40;
const uint8 DEALS_KEY_ITEMS    = 0x80;
//@}

class ObjectList {
public:
	ObjectList();

	~ObjectList();

	std::vector<hoa_global::GlobalObject*> _all_objects;

	std::vector<hoa_global::GlobalItem*> _items;

	std::vector<hoa_global::GlobalWeapon*> _weapons;

	std::vector<hoa_global::GlobalArmor*> _head_armor;

	std::vector<hoa_global::GlobalArmor*> _torso_armor;

	std::vector<hoa_global::GlobalArmor*> _arm_armor;

	std::vector<hoa_global::GlobalArmor*> _leg_armor;

	std::vector<hoa_global::GlobalShard*> _shards;
}; // class ObjectList

} // namespace private_shop

} // namespace hoa_shop

#endif // __SHOP_UTILS_HEADER__