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

#include "video.h"

namespace hoa_shop {

//! \brief Used to indicate what window has control of user input
enum SHOP_PRICE_LEVEL {
	SHOP_PRICE_INVALID   = -1,
	SHOP_PRICE_VERY_GOOD =  0,
	SHOP_PRICE_GOOD      =  1,
	SHOP_PRICE_STANDARD  =  2,
	SHOP_PRICE_POOR      =  3,
	SHOP_PRICE_VERY_POOR =  4,
	SHOP_PRICE_TOTAL     =  5
};

namespace private_shop {

//! \brief Used to indicate what window has control of user input
enum SHOP_STATE {
	SHOP_STATE_INVALID   = -1,
	SHOP_STATE_ROOT      =  0,
	SHOP_STATE_BUY       =  1,
	SHOP_STATE_SELL      =  2,
	SHOP_STATE_TRADE     =  3,
	SHOP_STATE_CONFIRM   =  4,
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

/** ***************************************************************************
*** \brief Abstract class for shop interfaces
***
*** Shop interface classes are manager classes for a particular state of shop
*** mode. All interface classes inherit from this abstract class.
*** **************************************************************************/
class ShopInterface {
public:
	ShopInterface()
		{}

	virtual ~ShopInterface()
		{}

	virtual void Initialize() = 0;

	virtual void Update() = 0;

	virtual void Draw() = 0;
}; // class ShopInterface

/** ***************************************************************************
*** \brief
***
***
*** **************************************************************************/
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

/** ****************************************************************************
*** \brief Displays detailed information about a selected object
***
***
*** ***************************************************************************/
class ObjectInfoWindow : public hoa_video::MenuWindow {
public:
	ObjectInfoWindow();

	~ObjectInfoWindow();

	//! \brief Draws the window and the object properties contained within
	void Draw();

	/** \brief Sets the object that this window will display the properties of
	*** \param obj A pointer to the object to represent. NULL indicates no object.
	**/
	void SetObject(hoa_global::GlobalObject* obj);

	//! \brief A text box that holds the description text of the object
	hoa_video::TextBox description;

	//! \brief A text box that displays the object's properties, such as attack or defense ratings
	hoa_video::TextBox properties;

private:
	/** \brief A pointer to the object whose properties are to be described
	*** If this member is set to NULL, then the window will be blank. The pointer
	*** should point to an object contained within a ShopMode class, not to an
	*** object in the player's inventory or anywhere else.
	**/
	hoa_global::GlobalObject* _object;

	//COMMENT!!!!!
	bool _is_weapon;
	bool _is_armor;

	//! \brief A vector holding the list of characters capable of equipping the above object.
	std::vector<hoa_global::GlobalCharacter*> _usableBy;

	//! \brief A vector that holds an icon image for each character in the party.
	std::vector<hoa_video::StillImage> _character_icons;

	std::vector<hoa_video::StillImage> _character_icons_bw;

	/** \brief Vector that holds the +/- variance of stats for equipping above object.
	*** This vector corresponds with the _usableBy vector above. Each index represents a character
	*** that is capable of equipping this object.
	**/
	std::vector<int32> _statVariance;

	//! \brief Same as above, but this holds the variance for the meta defense/attack.
	std::vector<int32> _metaVariance;

	void _LoadCharacterIcons();

}; // class ObjectInfoWindow : public hoa_video::MenuWindow

} // namespace private_shop

} // namespace hoa_shop

#endif // __SHOP_UTILS_HEADER__