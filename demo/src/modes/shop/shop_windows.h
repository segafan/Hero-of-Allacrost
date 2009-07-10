///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_windows.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for menu windows in shop mode.
***
*** This code contains classes that represent and manage the various menu
*** windows when the player is in shop mode. The list of wares for sale and
*** shop actions (buy/sell) are two examples of shop menu windows.
***
*** \todo I (Roots) think that we should combine the buy and sell windows into
*** one, since each window is the same size and only the contents of that window
*** change between buy and sell. It should help us re-use code.
***
*** \todo The confirm and prompt windows should also be combined since they are
*** about the same size.
*** ***************************************************************************/

#ifndef __SHOP_WINDOWS_HEADER__
#define __SHOP_WINDOWS_HEADER__

#include "defs.h"
#include "utils.h"

#include "video.h"

#include "global.h"

#include "shop_utils.h"

namespace hoa_shop {



namespace private_shop {

/** ****************************************************************************
*** \brief Represents the main window in shop mode which contains the shop actions
***
*** Shop actions include "buy", "sell", etc. This window also contains financial
*** information about the party and marked purchases. This window is located at
*** the top of the group of windows and is shown on a permanent basis.
***
*** \todo Retrieve the name and location graphic of the most recent map mode
*** (highest on the game stack) for when the user enters menu mode through shop
*** mode.
*** ***************************************************************************/
class ShopActionWindow : public hoa_video::MenuWindow {
public:
	ShopActionWindow();

	~ShopActionWindow();

	//! \brief Handles user input and updates the state of the window
	void Update();

	//! \brief Updates the text box that displays the financial information about the transaction
	void UpdateFinanceText();

	//! \brief Updates the text table that displays the financial information about the transaction
	void UpdateFinanceTable();

	//! \brief Draws the window to the screen
	void Draw();

	/** \brief The list of options for what the player may do in shop mode
	*** Each option includes the name of the object and its price.
	**/
	hoa_video::OptionBox action_options;

	//! \brief Table-formatted text containing the financial information about the current purchases and sales
	hoa_video::OptionBox finance_table;

	//! \brief Image icon representing drunes, drawn at 0.5x scale next to the finance table
	hoa_video::StillImage drunes_icon;
}; // class ShopActionWindow : public hoa_video::MenuWindow


/** ****************************************************************************
*** \brief Displays textual information about the shop at the highest menu level
***
*** This window displays a brief introductory greeting from the shop keeper and
*** informs the player of the fairness in their buy/sell prices. It is located
*** directly below the action window and is only displayed when the player is
*** at the highest level of the shop menu hiearchy. After creating this class,
*** the greeting text and price level text need to be set. If they are not set,
*** then a generic greeting or text for a standard price level will be used.
*** ***************************************************************************/
class ShopGreetingWindow : public hoa_video::MenuWindow {
public:
	ShopGreetingWindow();

	~ShopGreetingWindow();

	//! \brief Updates the state of the window
	void Update();

	//! \brief Draws the window to the screen
	void Draw();

	/** \brief Sets the greeting text of the merchant
	*** \param greeting The textual greeting
	**/
	void SetGreetingText(hoa_utils::ustring greeting);

	/** \brief Sets the text representing the buy/sell price levels of the merchant
	*** \param buy_level The pricing level for the merchandise to be bought
	*** \param sell_level The pricing level for anything the player wishes to sell
	**/
	void SetPriceLevelText(SHOP_PRICE_LEVEL buy_level, SHOP_PRICE_LEVEL sell_level);

	/** \brief Determines what type of merchandise is purchased and sold at this shop
	*** \param deal_types A bit vector indicating which types of objects are dealt with at the shop
	*** \param deal_images A vector of images representing each type of dealing
	***
	*** This method will copy the deal_images vector to the category_icons vector. Then any unused
	*** category images as determined by the deals_type parameter will have grayscale enabled.
	**/
	void SetCategoryIcons(uint8 deal_types, std::vector<hoa_video::StillImage>& deal_images);

	//! \brief Prints financial information in the bottom of the window
	hoa_video::TextBox greeting_text;

	//! \brief Text that indicates the price levels
	hoa_video::TextBox pricing_text;

	//! \brief Container for icon images that represent each object category that the shop deals in
	std::vector<hoa_video::StillImage> category_icons;
}; // class ShopGreetingWindow : public hoa_video::MenuWindow


/** ****************************************************************************
*** \brief A window containing a list of objects and their price
***
*** \todo Allow the player to browse via object category (all, items, weapons,
*** etc).
*** ***************************************************************************/
class BuyListWindow : public hoa_video::MenuWindow {
public:
	BuyListWindow();

	~BuyListWindow();

	// -------------------- Class Methods

	//! \brief Removes all entries from the list of objects for sale
	void Clear();

	/** \brief Adds a new entry to the option box
	*** \param name The name of the object for this entry
	*** \param price The price of the object in this entry
	*** \param quantity The maximum quantity of this object that are available to buy
	**/
	void AddEntry(hoa_utils::ustring name, uint32 price, uint32 quantity);

	//! \brief Reconstructs the option box from the entries that have been added
	void ConstructList();

	//! \brief Refreshes the option box and all entries
	void RefreshList();

	//! \brief Processes user input and updates the cursor
	void Update();

	//! \brief Draws the object list window and options to the screen
	void Draw();

	// -------------------- Class Members

	//! \brief When set to true, the OptionBox will not be drawn for this window
	bool hide_options;

	/** \brief Contains the list of objects for sale
	*** Each option includes the name of the object and its price.
	**/
	hoa_video::OptionBox object_list;
}; // class BuyListWindow : public hoa_video::MenuWindow


/** ****************************************************************************
*** \brief A window containing a list of current inventory and selling price
***
***
*** ***************************************************************************/
class SellListWindow : public hoa_video::MenuWindow {
public:
	SellListWindow();

	~SellListWindow();

	// -------------------- Class Methods

	//! \brief Removes all object entries from the list
	void Clear();

	/** \brief Adds a new entry to the option box
	*** \param name The name of the object for this entry
	*** \param count The number of objects in the current inventory
	*** \param price The price of the object in this entry
	**/
	void AddEntry(hoa_utils::ustring name, uint32 count, uint32 price, uint32 sell_count);

	//! \brief Processes user input and updates the cursor
	void Update();

	//! \brief Refreshes list of sellable items
	void UpdateSellList();

	//! \brief Draws the object list window and options to the screen
	void Draw();

	// -------------------- Class Members

	//! \brief When set to true, the OptionBox will not be drawn for this window
	bool hide_options;

	//! \brief Contains the text that forms each option in the list
	std::vector<hoa_utils::ustring> option_text;

	/** \brief Contains the list of objects for sale
	*** Each option includes the name of the object and its price.
	**/
	hoa_video::OptionBox object_list;

	hoa_video::TextBox list_header;
}; // class SellListWindow : public hoa_video::MenuWindow


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



/** ****************************************************************************
*** \brief Displays the object's icon, name, and a sale confirmation message
***
*** This window is currently being used for the shopping cart functionality.
*** When confirmed, all buy/sell transactions are finalized.
*** ***************************************************************************/
class ConfirmWindow : public hoa_video::MenuWindow {
public:
	ConfirmWindow();

	~ConfirmWindow();

	//! \brief Updates the option box
	void Update();

	//! \brief Draws the window and the object properties contained within
	void Draw();

	//! \brief Options for the user to confirm or reject the sale
	hoa_video::OptionBox options;
}; // class ConfirmWindow : public hoa_video::MenuWindow



/** ****************************************************************************
*** \brief Displays a small window presenting the user with information
***
*** This is typically used to indicate a user's incorrect action. For example,
*** when they try to go to the sell menu when their inventory is empty, or they
*** try to make a transaction with no sales or purchases.
*** ***************************************************************************/
class PromptWindow : public hoa_video::MenuWindow {
public:
	PromptWindow();

	~PromptWindow();

	//! \brief Processes user input
	void Update();

	//! \brief Draws the window and the object properties contained within
	void Draw();

	//! \brief The text to prompt the user with
	hoa_video::TextBox prompt_text;
}; // class PromptWindow : public hoa_video::MenuWindow

} // namespace private_shop

} // namespace hoa_shop

#endif // __SHOP_WINDOWS_HEADER__
