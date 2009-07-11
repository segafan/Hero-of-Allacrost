///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_sell.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for sell menu of shop mode
***
*** WRITE SOMETHING
*** ***************************************************************************/

#ifndef __SHOP_SELL_HEADER__
#define __SHOP_SELL_HEADER__

#include "defs.h"
#include "utils.h"

#include "video.h"
#include "global.h"

#include "shop_utils.h"

namespace hoa_shop {

namespace private_shop {

class ShopSellInterface : public ShopInterface {
public:
	ShopSellInterface();

	~ShopSellInterface();

	void Initialize();

	void Update();

	void Draw();
}; // class ShopSellInterface : public ShopInterface

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

}

}

#endif // __SHOP_SELL_HEADER__
