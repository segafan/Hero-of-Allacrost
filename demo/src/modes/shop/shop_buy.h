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

	//! \brief Shows the menu windows that are used
	void MakeActive();

	//! \brief Hides the menu windows that are used
	void MakeInactive();

	//! \brief Processes user input and updates the purchase totals as appropriate
	void Update();

	//! \brief Draws the GUI elements to the screen
	void Draw();

private:
	//! \brief Index to the active entry in both the _object_data and _object_lists containers
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
	***
	*** The size and contents of this container mimic that which is found in the _object_data container.
	**/
	std::vector<BuyList*> _object_lists;

	//! \brief Pointer to the window used for displaying the list of objects for sale
	hoa_video::MenuWindow* _list_window;

	//! \brief Pointer to the window used for displaying detailed information about a particular object
	hoa_video::MenuWindow* _info_window;

	//! \brief Contains a column of images representing each category of object sold in the shop
	hoa_video::OptionBox _category_list;

	//! \brief Header text for the object identifier list (refer to the BuyList class)
	hoa_video::OptionBox _identifier_header;

	//! \brief Header text for the properties identifier list (refer to the BuyList class)
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
*** \brief A GUI container representing a list of objects that may be bought
***
*** This container actually uses two option boxes to represent the list. The first
*** option box is on the left hand side and contains the image icon for the object
*** and the object's name. The second option box to the right lists the price, stock,
*** amount owned by the player, and amount the player indicates that they wish to
*** purchase. The number of entries in both option boxes and the number of entries in
*** the object_data vector should be the same.
*** ***************************************************************************/
class BuyList {
public:
	BuyList();

	~BuyList()
		{}

	// -------------------- Class Methods

	/** \brief Removes all entries from the option boxes
	*** \note This will also set the object_data member to NULL, so usually calling this function
	*** should be followed by invoking PopulateList() to refill the class with valid data.
	**/
	void Clear();

	/** \brief Clears and then constructs the option box data
	*** \param objects A pointer to a data vector containing the objects to populate the list with
	**/
	void PopulateList(std::vector<ShopObject*>* objects);

	//! \brief Reconstructs all option box entries from the object data
	void RefreshList();

	/** \brief Reconstructs the displayed properties of a single object
	*** \param index The index of the object data to reconstruct
	***
	*** This method refreshes only the relevant options of the properties_list and does not modify
	*** the identifier_list. The reason for this is that the identifier_list (containing the image
	*** and name of the object) never needs to be changed since that data is static. The properties,
	*** however, do require frequent change as the stock, number owned, and amount to buy can be
	*** manipulated by the user.
	**/
	void RefreshEntry(uint32 index);

	//! \brief Updates the option boxes
	void Update();

	//! \brief Draws the option boxes
	void Draw();

	// -------------------- Class Members

	//! \brief A pointer to the vector of object data that the class is to display
	std::vector<ShopObject*>* object_data;

	//! \brief Identifies objects via their icon and name
	hoa_video::OptionBox identifier_list;

	/** \brief Contains shop properties about the object: price, stock, own, and amount to buy
	***
	**/
	hoa_video::OptionBox properties_list;
}; // class BuyList

} // namespace private_shop

} // namespace hoa_shop

#endif // __SHOP_BUY_HEADER__
