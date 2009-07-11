///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_root.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for root menus of shop mode
*** ***************************************************************************/

#ifndef __SHOP_ROOT_HEADER__
#define __SHOP_ROOT_HEADER__

#include "defs.h"
#include "utils.h"

#include "video.h"
#include "global.h"

#include "shop_utils.h"

namespace hoa_shop {

namespace private_shop {

/** ****************************************************************************
*** \brief The highest level shopping interface that contains the primary menu
***
*** This interface is responsible for managing the ever-present root window which
*** contains the primary actions a user can take in shop mode, such as "buy", "sell",
*** or "trade". It also manages the display of status information about the player's
*** current transaction as well as a greeting window which gives an informational
*** overview about the shop to the player.
***
*** \note This interface is rather parculiar because its Update() and Draw() methods
*** are called on every iteration of the main game loop, regardless of what state
*** the shop is running in. When the shop is in the SHOP_STATE_ROOT state, both the
*** root window and greeting window are likewise updated and drawn and user input
*** is processed from this class. When the shop is in a different state, only the
*** root window is updated and drawn and nothing else is done by this interface.
*** ***************************************************************************/
class ShopRootInterface : public ShopInterface {
public:
	ShopRootInterface();

	~ShopRootInterface();

	// ---------- Methods ----------

	//! \brief Initializes various textual and image data based on the shop properties
	void Initialize();

	//! \brief Updates the state of GUI objects and may also process user input
	void Update();

	//! \brief Draws the root window and, if shop mode is in the correct state, the greeting window
	void Draw();

	/** \brief Sets the greeting text for the greeting window
	*** \param greeting The textual greeting
	**/
	void SetGreetingText(hoa_utils::ustring greeting);

	//! \brief Updates the text table that displays the financial information about the transaction in the root window
	void UpdateFinanceTable();

	// ---------- Members ----------
private:
	//! \brief The top-most, ever-present window in shop mode that contains the list of user actions and financial status
	RootWindow* _root_window;

	//! \brief A small window that presents an overview of information about the shop
	GreetingWindow* _greeting_window;
}; // class ShopRootInterface : public ShopInterface


/** ****************************************************************************
*** \brief The primary root window of shop mode
***
*** This window is always present on the screen and is located above all other
*** menus.
***
*** This window contains the following:
*** -# The list of actions the player may take while in the shop
*** -# A display of the financial information about the current transaction
***
*** The list of player shopping actions include the following:
*** -# Buy (objects being sold)
*** -# Sell (objects from the party's inventory)
*** -# Trade (one equipped weapon or armor for another)
*** -# Confirm (the purchase/sale/trade transaction)
*** -# Leave (shop mode and return)
*** ***************************************************************************/
class RootWindow : public hoa_video::MenuWindow {
public:
	RootWindow();

	~RootWindow();

	// ---------- Methods ----------

	//! \brief Updates the state and contents of the window
	void Update();

	//! \brief Draws the window and its contents to the screen
	void Draw();

	// ---------- Members ----------

	/** \brief The list of options for what the player may do in shop mode
	*** Each option includes the name of the object and its price.
	**/
	hoa_video::OptionBox action_options;

	//! \brief Table-formatted text containing the financial information about the current purchases and sales
	hoa_video::OptionBox finance_table;

	//! \brief Image icon representing drunes, drawn at 0.5x scale next to the finance table
	hoa_video::StillImage drunes_icon;
}; // class RootWindow : public hoa_video::MenuWindow


/** ****************************************************************************
*** \brief Displays an overview of information about the shop
***
*** This window is located directly below the root menu and is only slightly larger
*** than the root menu in size.
***
*** This window contains the following:
*** -# An introductory greeting from the merchant/shopkeeper
*** -# A list of category images indicating what wares the shop deals and doesn't deal in
*** -# The buy/sell pricing levels of the shop
***
*** \note If no greeting message or pricing levels were set for the shop prior to it
*** being initialized, this class will display a generic greeting and use standard
*** pricing levels.
*** ***************************************************************************/
class GreetingWindow : public hoa_video::MenuWindow {
public:
	GreetingWindow();

	~GreetingWindow();

	// ---------- Methods ----------

	//! \brief Updates the state and contents of the window
	void Update();

	//! \brief Draws the window and its contents to the screen
	void Draw();

	// ---------- Members ----------

	//! \brief Prints financial information in the bottom of the window
	hoa_video::TextBox greeting_text;

	//! \brief Text that indicates the price levels
	hoa_video::TextBox pricing_text;

	/** \brief Container for icon images that represent each object category that the shop deals in
	*** Categories which the shop does not deal in will have their icon set to grayscale
	**/
	std::vector<hoa_video::StillImage> category_icons;
}; // class GreetingWindow : public hoa_video::MenuWindow

} // namespace private_shop

} // namespace hoa_shop

#endif // __SHOP_ROOT_HEADER__