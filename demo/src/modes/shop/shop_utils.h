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
*** This file contains utility code that is shared among the various shop mode
*** classes.
*** ***************************************************************************/

#ifndef __SHOP_UTILS_HEADER__
#define __SHOP_UTILS_HEADER__

#include "defs.h"
#include "utils.h"

#include "system.h"
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
	SHOP_STATE_LEAVE     =  5,
	SHOP_STATE_TOTAL     =  6
};

//! \name Price multipliers
//! \brief These values are multiplied by an object's standard price to get the price for the desired price level
//@{
const float BUY_PRICE_VERY_GOOD   = 1.2f;
const float BUY_PRICE_GOOD        = 1.4f;
const float BUY_PRICE_STANDARD    = 1.6f;
const float BUY_PRICE_POOR        = 1.8f;
const float BUY_PRICE_VERY_POOR   = 2.0f;

const float SELL_PRICE_VERY_GOOD  = 0.9f;
const float SELL_PRICE_GOOD       = 0.8f;
const float SELL_PRICE_STANDARD   = 0.7f;
const float SELL_PRICE_POOR       = 0.6f;
const float SELL_PRICE_VERY_POOR  = 0.5f;
//@}

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
*** mode. All interface classes inherit from this abstract class. All interfaces
*** are initialized only once after the ShopMode class finishes its own initialization
*** routine.
*** **************************************************************************/
class ShopInterface {
public:
	ShopInterface()
		{}

	virtual ~ShopInterface()
		{}

	//! \brief Performs any initialization that could not be done when the class was constructed
	virtual void Initialize() = 0;

	//! \brief Updates the state of the interface and operates on user input
	virtual void Update() = 0;

	//! \brief Draws the interface's contents to the screen
	virtual void Draw() = 0;
}; // class ShopInterface


/** ***************************************************************************
*** \brief Represents objects that are bought, sold, and traded within the shop
***
*** This class wraps around a GlobalObject and uses additional members that are
*** properties of the object specific to shopping. The ShopMode class maintains
*** containers of these objects and the various interfaces perform modifications
*** to their properties.
***
*** \note Be careful with assigning the GlobalObject pointer in the class constructor.
*** The object pointed to, if it exists in the global party inventory, will be deleted
*** if all counts to this member are removed from the inventory. Therefore never use
*** a GlobalObject inventory pointer if you don't have to (use the ones that ShopMode
*** creates for all objects being sold in the shop) and if a sell count goes to zero,
*** delete the corresponding ShopObject.
***
*** \note The data in this class is used to determine if this object should be visible
*** in buy and/or sell lists.
*** **************************************************************************/
class ShopObject {
public:
	/** \param object A pointer to a valid GlobalObject instance that the shop object will represent
	*** \param sold_by_shop True if this object is offered for sale by the shop
	**/
	ShopObject(hoa_global::GlobalObject* object, bool sold_by_shop);

	~ShopObject()
		{}

	/** \brief Sets the buy and sell prices for the object
	*** \param buy_level The buy level of the shop that will determine its buy price
	*** \param sell_level The sell level of the shop that will determine its sell price
	**/
	void SetPricing(SHOP_PRICE_LEVEL buy_level, SHOP_PRICE_LEVEL sell_level);

	//! \name Class member accessor functions
	//@{
	hoa_global::GlobalObject* GetObject() const
		{ return _object; }

	bool IsSoldInShop() const
		{ return _sold_in_shop; }

	uint32 GetBuyPrice() const
		{ return _buy_price; }

	uint32 GetSellPrice() const
		{ return _sell_price; }

	uint32 GetOwnCount() const
		{ return _own_count; }

	uint32 GetStockCount() const
		{ return _stock_count; }

	uint32 GetBuyCount() const
		{ return _buy_count; }

	uint32 GetSellCount() const
		{ return _sell_count; }

	void ResetBuyCount()
		{ _buy_count = 0; }

	void ResetSellCount()
		{ _sell_count = 0; }
	//@}

	/** \name Increment and Decrement Functions
	*** \brief Increments or decrements the value of the various count members
	*** \param inc/dec The amount to decrement the count by (default value == 1)
	***
	*** These functions increment or decrement the respective count members. Checks are performed
	*** to prevent error conditions from occurring. For example, the buy count can not be greater
	*** than the stock count and the sell count can not be greater than the own count. None of the
	*** count members will be allowed to decrement below zero. Overflow conditions however are not
	*** checked. Should any error condition occur, a warning message will be printed and the value
	*** of the count member will not be modified.
	**/
	//@{
	void IncrementOwnCount(uint32 inc = 1);
	void IncrementStockCount(uint32 inc = 1);
	void IncrementBuyCount(uint32 inc = 1);
	void IncrementSellCount(uint32 inc = 1);
	void DecrementOwnCount(uint32 dec = 1);
	void DecrementStockCount(uint32 dec = 1);
	void DecrementBuyCount(uint32 dec = 1);
	void DecrementSellCount(uint32 dec = 1);
	//@}

private:
	//! \brief A pointer to the global object represented by this
	hoa_global::GlobalObject* _object;

	//! \brief Set to true if the player is able to buy this object from the shop
	bool _sold_in_shop;

	//! \brief The price that the player must pay to buy this object from the shop
	uint32 _buy_price;

	//! \brief The return that the player will receive for selling this object to the shop
	uint32 _sell_price;

	//! \brief The number of this object that the player's party currently owns
	uint32 _own_count;

	//! \brief The stock of this object that the shop
	uint32 _stock_count;

	//! \brief The amount of this object that the player plans to purchase
	uint32 _buy_count;

	//! \brief The amount of this object that the player plans to sell
	uint32 _sell_count;
}; // class ShopObject


/** ****************************************************************************
*** \brief An abstract class for displaying the current selected object category
***
*** The contents of this class are used to display an icon image and name. This
*** class is intended to be used to display only one single category and is not
*** recommended to be used if you wish to display multiple categories in a list.
*** This class also enables a smooth transition when switching from one category
*** to the next. It does so by a general visual transformation from the former
*** category icon to the next and an animation of the category text changing.
***
*** \note In the Draw() method implementation of the inheriting class, take care
*** to check that the icon image pointer members are not NULL before attempting
*** to draw them.
*** ***************************************************************************/
class ObjectCategoryDisplay {
public:
	ObjectCategoryDisplay();

	virtual ~ObjectCategoryDisplay();

	/** \brief Sets the amount of time that it should take to fully transition between categories
	*** \param time The amount of time, in milliseconds
	*** \note The default time is defined by the value of _DEFAULT_TRANSITION_TIME. You only need to
	*** call this method if you do not wish to use the default value.
	**/
	void SetTransitionTime(uint32 time);

	/** \brief Sets the new category
	*** \param name The text to display that represents the category's name
	*** \param icon A pointer to the new image to represent the category's icon
	*** \note This function does not check whether or not the new name/icon are not
	*** the same as the previous name/icon
	*** \note It is safe to pass a NULL pointer for the icon argument, but a warning message
	*** will be printed if this is the case.
	**/
	void ChangeCategory(hoa_utils::ustring& name, const hoa_video::StillImage* icon);

	//! \brief Must be called so that the TextBox can proceed
	void Update();

	//! \brief Draws the graphics and text to the screen
	virtual void Draw() = 0;

protected:
	//! \brief A pointer to an icon image representing the current category
	const hoa_video::StillImage* _category_icon;

	//! \brief A pointer to the icon image that represents the previous category
	const hoa_video::StillImage* _last_icon;

	//! \brief The name/description text of the current category
	hoa_video::TextBox _category_text;

	//! \brief A timer used to track the progress of category transitions
	hoa_system::SystemTimer _transition_timer;
}; // class ObjectCategoryDisplay


/** ****************************************************************************
*** \brief An abstract class for displaying a list of shop objects
***
*** This class is used to display a list of shop objects to the user along with
*** certain properties. It uses two OptionBox objects to achieve this, which are placed
*** side by side. The left option box contains indentifying information of the shop
*** object while the right option box contains specific object properties. Both OptionBox
*** objects have the same number of rows (a row represents a single object) but usually a
*** different number of columns.
***
*** It is up to the deriving class to determine what information that the OptionBoxes ultimately
*** display and how that information is displayed. The deriving class constructor should set the
*** the properties of the OptionBox objects to display them as desired.
*** ***************************************************************************/
class ObjectListDisplay {
public:
	ObjectListDisplay() : _objects(NULL)
		{}

	virtual ~ObjectListDisplay()
		{ Clear(); }

	/** \brief Removes all entries from the option boxes
	*** \note This will also set the _objects member to NULL, so usually calling this function
	*** should be followed by invoking PopulateList() to refill the class with valid data.
	**/
	void Clear();

	/** \brief Clears and then constructs the option box data
	*** \param objects A pointer to a data vector containing the objects to populate the list with
	**/
	void PopulateList(std::vector<ShopObject*>* objects);

	//! \brief Reconstructs all option box entries from the object data
	virtual void RefreshList() = 0;

	/** \brief Reconstructs the displayed properties of a single object
	*** \param index The index of the object data to reconstruct
	**/
	virtual void RefreshEntry(uint32 index) = 0;

	/** \brief Reconstructs the displayed properties of all objects in the list
	*** The difference between this method and the RefreshList() method is that this method only
	*** operates on the object's properties, whereas RefreshList() also works on the identification
	*** data. This method is less costly than reconstructing the entire list.
	**/
	virtual void RefreshAllEntries();

	//! \brief Updates the option boxes
	void Update();

	//! \brief Draws the option boxes
	void Draw();

	hoa_video::OptionBox& GetIdentifyList()
		{ return _identify_list; }

	hoa_video::OptionBox& GetPropertyList()
		{ return _property_list; }

protected:
	//! \brief A pointer to the vector of object data that the class is to display
	std::vector<ShopObject*>* _objects;

	//! \brief Contains identification information about each objects such as graphical icon and name
	hoa_video::OptionBox _identify_list;

	//! \brief Contains properties about the object such as price, stock, amount owned, or amount to buy/sell
	hoa_video::OptionBox _property_list;
}; // class ObjectListDisplay


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
