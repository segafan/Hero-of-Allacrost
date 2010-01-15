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
*** \brief   Header file for buy interface of shop mode
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

/** ****************************************************************************
*** \brief Manages the shop when it is in buy mode and enables the player to view and purchase wares
***
*** This interface displays the list of objects that are available for sale. It organizes
*** these objects based on their object type (item, weapon, etc.) and allows the player
*** to switch between views of these different categories.
*** ***************************************************************************/
class BuyInterface : public ShopInterface {
public:
	BuyInterface();

	~BuyInterface();

	//! \brief Initializes the data conatiners and GUI objects to be used
	void Initialize();

	//! \brief Processes user input and updates the purchase totals as appropriate
	void Update();

	//! \brief Draws the GUI elements to the screen
	void Draw();

private:
	//! \brief Index to the active entry in both the _object_data and _object_displays containers
	uint32 _current_datalist;

	/** \brief Contains all objects for sale sorted into various category lists
	***
	*** The minimum size this container will ever be is two and the maximum it will be is nine. The first
	*** entry (index 0) always holds the list of all objects regardless of categories. The proceeding
	*** entries will be ordered based on object type, beginning with items and ending with key items.
	*** For example, if the shop deals in weapons and shards, index 0 will hold a list of all weapons
	*** and shards, index 1 will hold a list of all weapons, and index 2 will hold a list of all shards.
	**/
	std::vector<std::vector<ShopObject*> > _object_data;

	/** \brief Class objects used to display the object data to the player
	*** The size and contents of this container mimic that which is found in the _object_data container.
	**/
	std::vector<BuyDisplay*> _object_displays;

	//! \brief Contains a column of images representing each category of object sold in the shop
	hoa_video::OptionBox _category_list;

	//! \brief Header text for the object identifier list (refer to the BuyDisplay class)
	hoa_video::OptionBox _identifier_header;

	//! \brief Header text for the properties identifier list (refer to the BuyDisplay class)
	hoa_video::OptionBox _properties_header;

	// ---------- Methods ----------
	//! \brief Returns the number of object categories displayed by the buy interface
	uint32 GetNumberObjectCategories() const
		{ return _object_data.size(); }

	//! \brief Returns true if the buy interface includes an "All" category for displaying wares
	bool _HasAllCategory() const
		{ return (_object_data.size() > 1); }

	//! \brief Used to update the category icons to show the unselected categories in gray
	void _UpdateSelectedCategory();
}; // class BuyInterface : public ShopInterface


/** ****************************************************************************
*** \brief A GUI display of the list of objects that may be bought
*** ***************************************************************************/
class BuyDisplay : public ListDisplay {
public:
	BuyDisplay();

	~BuyDisplay()
		{}

	//! \brief Reconstructs all option box entries from the object data
	void RefreshList();

	/** \brief Reconstructs the displayed properties of a single object
	*** \param index The index of the object data to reconstruct
	**/
	void RefreshEntry(uint32 index);
}; // class BuyDisplay : public ListDisplay

} // namespace private_shop

} // namespace hoa_shop

#endif // __SHOP_BUY_HEADER__
