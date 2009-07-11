///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_buy.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for buy menus of shop mode
*** ***************************************************************************/

#ifndef __SHOP_BUY_HEADER__
#define __SHOP_BUY_HEADER__

#include "defs.h"
#include "utils.h"

#include "video.h"
#include "global.h"

#include "shop_utils.h"

namespace hoa_shop {

namespace private_shop {

class ShopBuyInterface : public ShopInterface {
public:
	ShopBuyInterface();

	~ShopBuyInterface();

	void Initialize();

	void Update();

	void Draw();
}; // class ShopBuyInterface : public ShopInterface

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


}

}

#endif // __SHOP_BUY_HEADER__
