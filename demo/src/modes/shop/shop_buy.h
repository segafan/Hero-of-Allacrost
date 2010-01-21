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
*** these objects based on their object type/category (item, weapon, etc.) and allows the
*** player to switch between views of these different categories. This interface also displays
*** information about the currently selected object such as its description, statistical
*** ratings, usable characters, etc.
*** ***************************************************************************/
class BuyInterface : public ShopInterface {
public:
	BuyInterface();

	~BuyInterface();

	//! \brief Initializes the data conatiners and GUI objects to be used
	void Initialize();

	//! \brief Sets the selected object for the ShopObjectViewer class
	void MakeActive();

	//! \brief Processes user input and sends appropriate commands to helper class objects
	void Update();

	//! \brief Draws the GUI elements to the screen
	void Draw();

private:
	//! \brief Stores the active view state of the buy interface
	SHOP_VIEW_MODE _view_mode;

	//! \brief A pointer to the currently selected object in the active list display
	ShopObject* _selected_object;

	//! \brief Retains the number of object categories for sale
	uint32 _number_categories;

	//! \brief Serves as an index to the following containers: _object_data, _category_names, _category_icons, and _list_displays
	uint32 _current_category;

	//! \brief Header text for the category field
	hoa_video::TextImage _category_header;

	//! \brief Header text for the name field
	hoa_video::TextImage _name_header;

	//! \brief Header text for the list of object properties (refer to the BuyListDisplay class)
	hoa_video::OptionBox _properties_header;

	//! \brief String representations of all category names that may
	std::vector<hoa_utils::ustring> _category_names;

	//! \brief A pointer to icon images
	std::vector<hoa_video::StillImage*> _category_icons;

	//! \brief Display manager for the current category of objects selected
	ObjectCategoryDisplay _category_display;

	/** \brief Class objects used to display the object data to the player
	*** The size and contents of this container mimic that which is found in the __object_data container.
	**/
	std::vector<BuyListDisplay*> _list_displays;

	/** \brief Contains all objects for sale sorted into various category lists
	***
	*** The minimum size this container will ever be is two and the maximum it will be is nine. The first
	*** entry (index 0) always holds the list of all objects regardless of categories. The proceeding
	*** entries will be ordered based on object type, beginning with items and ending with key items.
	*** For example, if the shop deals in weapons and shards, index 0 will hold a list of all weapons
	*** and shards, index 1 will hold a list of all weapons, and index 2 will hold a list of all shards.
	**/
	std::vector<std::vector<ShopObject*> > _object_data;

	/** \brief Changes the current category and object list that is being displayed
	*** \param left_or_right False to move the category to the left, or true for the right
	*** \return True if the _selected_object member has changed
	**/
	bool _ChangeCategory(bool left_or_right);

	/** \brief Changes the current selection in the object list
	*** \param up_or_down False to move the selection cursor up, or true to move it down
	*** \return True if the _selected_object member has changed
	**/
	bool _ChangeSelection(bool up_or_down);

	/** \brief Change the buy quantity of the current selection
	*** \param less_or_more False to decrease the quantity, true to increase it
	*** \param amount The amount to decrease/increase the quantity by (default value == 1)
	*** \return False if no quantity change could take place, true if a quantity change did occur
	*** \note Even if the function returns true, there is no guarantee that the requested amount
	*** was fully met. For example, if the function is asked to increase the buy quantity by 10 but
	*** the shop only has 6 instances of the selected object in stock, the function will increase
	*** the quantity by 6 (not 10) and return true.
	**/
	bool _ChangeQuantity(bool less_or_more, uint32 amount = 1);

	//! \brief Sets the _selected_object member based on the current display list entry
	void _SetSelectedObject();

	//! \brief Returns the total number of viewable object categories
	uint32 _GetNumberObjectCategories() const
		{ return _object_data.size(); }
}; // class BuyInterface : public ShopInterface


/** ****************************************************************************
*** \brief A display class that maintains and draws lists of objects that may be bought
***
*** The inherited _identify_list and _property_list contain several pieces of data. The
*** first contains a 0.25x size icon of the object and the object's name. Both pieces
*** of information are stored in a single column of data. The second list contains four
*** columns of data per row which are price, shop stock, amount owned (by the player), and
*** the amount the player has indicated they wish to buy.
*** ***************************************************************************/
class BuyListDisplay : public ObjectListDisplay {
public:
	BuyListDisplay();

	~BuyListDisplay()
		{}

	//! \brief Reconstructs all option box entries from the object data
	void RefreshList();

	/** \brief Reconstructs the displayed properties of a single object
	*** \param index The index of the object data to reconstruct
	***
	*** This method refreshes only the relevant options of the _property_list and does not modify
	*** the _identify_list. The reason for this is that the _identify_list contains static data that
	*** requires no changes or updates. The object's properties, however, do require frequent change
	***  as this data (such as amount to buy or sell) can be directly manipulated by the player.
	**/
	void RefreshEntry(uint32 index);
}; // class BuyListDisplay : public ObjectListDisplay

} // namespace private_shop

} // namespace hoa_shop

#endif // __SHOP_BUY_HEADER__
