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

	//! \brief Processes user input and sends appropriate commands to helper class objects
	void Update();

	//! \brief Draws the GUI elements to the screen
	void Draw();

private:
	/** \brief When true, the list view will be updated and drawn
	*** The value of this member coincides with the active view state of the BuyObjectView class
	**/
	bool _list_view_active;

	//! \brief A helper class object for displaying categories and lists of objects for sale
	BuyListView* _buy_list_view;

	//! \brief A helper class object for displaying information about a particular object
	BuyObjectView* _buy_object_view;
}; // class BuyInterface : public ShopInterface


/** ****************************************************************************
*** \brief A display class that draws the current object category icon and text
***
*** The contents of this class are drawn to the left side of the middle window.
*** When a new category is selected to be displayed, a gradual transition animation
*** takes place. The previous category name text will immediately disappear and
*** the new name text line will fade in. The new and old category icons fade in
*** and out with each other. This animation is best illustrated as a timeline, where
*** (t) represents the amount of time the entire transition takes.
***
*** -# 0      : old icon displayed completely, begins transparency fade
*** -# (t/3)  : old icon is 1/2 transparent, new icon begins fading in
*** -# (2t/3) : old icon is completely transparent, new icon is 1/2 transparent
*** -# (t)    : new icon is displayed completely
*** ***************************************************************************/
class BuyCategoryDisplay : public ObjectCategoryDisplay {
public:
	BuyCategoryDisplay();

	~BuyCategoryDisplay()
		{}

	//! \brief Draws the category name and icon to the screen
	void Draw();
}; // class BuyListDisplay : public ObjectCategoryDisplay


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


/** ****************************************************************************
*** \brief Manages all data and graphics for showing object lists and categories
***
*** This class represents the view of the middle menu window in buy mode when that
*** window contains the current object category and a list of objects for sale.
*** The primary purpose of this class is to serve as a helper to the BuyInterface
*** class and to keep the code organized.
*** ***************************************************************************/
class BuyListView {
public:
	BuyListView();

	~BuyListView();

	// ---------- Methods

	//! \brief Finishes initialization of any data or settings that could not be completed in constructor
	void Initialize();

	//! \brief Updates the object category and list displays
	void Update();

	//! \brief Draws the contents to the screen
	void Draw();

	/** \brief Changes the current category and object list that is being displayed
	*** \param left_or_right False to move the category to the left, or true for the right
	*** \return A pointer to the ShopObject selected after the category change is performed
	**/
	ShopObject* ChangeCategory(bool left_or_right);

	/** \brief Changes the current selection in the object list
	*** \param up_or_down False to move the selection cursor up, or true to move it down
	*** \return A pointer to the ShopObject selected after the selection change is performed
	**/
	ShopObject* ChangeSelection(bool up_or_down);

	/** \brief Change the buy quantity of the current selection
	*** \param less_or_more False to decrease the quantity, true to increase it
	*** \param amount The amount to decrease/increase the quantity by (default value == 1)
	*** \return False if no quantity change could take place, true if a quantity change did occur
	*** \note Even if the function returns true, there is no guarantee that the requested amount
	*** was fully met. For example, if the function is asked to increase the buy quantity by 10 but
	*** the shop only has 6 instances of the selected object in stock, the function will increase
	*** the quantity by 6 (not 10) and return true.
	**/
	bool ChangeQuantity(bool less_or_more, uint32 amount = 1);

	/** \brief Returns a pointer to the currently selected object in the view
	*** \note This method should never return NULL as long as its called after the Initialize() method has
	*** been called once before.
	**/
	ShopObject* GetSelectedObject() const;

	//! \brief Returns the total number of viewable object categories
	uint32 GetNumberObjectCategories() const
		{ return object_data.size(); }

	// ---------- Members

	//! \brief Serves as an index to the following containers: object_data, category_names, category_icons, and list_displays
	uint32 current_category;

	//! \brief Header text for the category field
	hoa_video::TextImage category_header;

	//! \brief Header text for the name field
	hoa_video::TextImage name_header;

	//! \brief Header text for the list of object properties (refer to the BuyListDisplay class)
	hoa_video::OptionBox properties_header;

	//! \brief String representations of all category names that may
	std::vector<hoa_utils::ustring> category_names;

	//! \brief A pointer to icon images
	std::vector<const hoa_video::StillImage*> category_icons;

	//! \brief Display manager for the current category of objects selected
	BuyCategoryDisplay category_display;

	/** \brief Class objects used to display the object data to the player
	*** The size and contents of this container mimic that which is found in the _object_data container.
	**/
	std::vector<BuyListDisplay*> list_displays;

	/** \brief Contains all objects for sale sorted into various category lists
	***
	*** The minimum size this container will ever be is two and the maximum it will be is nine. The first
	*** entry (index 0) always holds the list of all objects regardless of categories. The proceeding
	*** entries will be ordered based on object type, beginning with items and ending with key items.
	*** For example, if the shop deals in weapons and shards, index 0 will hold a list of all weapons
	*** and shards, index 1 will hold a list of all weapons, and index 2 will hold a list of all shards.
	**/
	std::vector<std::vector<ShopObject*> > object_data;
}; // class BuyListView


/** ****************************************************************************
*** \brief Manages all data and graphics showing summary information about an object
***
*** This class shows summary information about the currently selected object. It displays
*** this information in one of two display modes. The first mode uses only the lower window
*** and shows only a limited amount of information. This is the default view mode that the
*** player sees and this information is visible in tandem with the graphics displayed by the
*** BuyListView class. The second view mode is entered if the player requests to see all the
*** details about the selected object. In this view mode, the BuyListView class' graphics are
*** hidden and this class displays the complete set of information about the selected object
*** using both the middle and lower menu windows. The type of information which is displayed is
*** different depending upon what type of object is selected. The primary purpose of this class
*** is to serve as a helper to the BuyInterface class and to keep the code organized.
***
*** \todo Currently this class treats the display of shards the same as it does for items
*** and key items. This should be changed once shard properties have a more clear definition
*** in the game design.
***
*** \todo Status icons are not currently displayed as there are no status effects implemented
*** in the game yet. Once status effects are functional and graphics are ready to represent
*** them, this code should be updated.
***
*** \todo The character sprites are not yet animated in the display. This should be changed
*** once the appropriate set of sprite frames are available for this menu.
*** ***************************************************************************/
class BuyObjectView {
public:
	BuyObjectView();

	~BuyObjectView()
		{}

	/** ****************************************************************************
	*** \brief List of view states that this class may execute in
	*** - VIEW_SUMMARY: default view mode, displays information summary in bottom window
	*** - VIEW_COMPLETE: displays all available information in both middle and bottom windows
	*** ***************************************************************************/
	enum ObjectViewState {
		VIEW_SUMMARY,
		VIEW_COMPLETE
	};

	// ---------- Methods

	//! \brief Finishes initialization of any data or settings that could not be completed in constructor
	void Initialize();

	//! \brief Updates sprite animations and other graphics as necessary
	void Update();

	//! \brief Draws the contents to the screen
	void Draw();

	/** \brief Changes the selected object to the function argument
	*** \param object A pointer to the shop object to change (NULL-safe, but will result in a warning message)
	*** \note The code using this class should always use this function to change the selected object rather than
	*** changing the public selected_object member directly. This method updates all of the relevent graphics and
	*** data displays in addition to changing the pointer member.
	**/
	void SetSelectedObject(ShopObject* object);

	/** \brief Changes the view mode to use only the lower window to display an information summary
	*** If the class is already in this state, a warning message will be printed and no change will take place.
	**/
	void ChangeViewSummary();

	/** \brief Changes the view mode to use both middle and lower windows to display all possible information
	*** If the class is already in this state, a warning message will be printed and no change will take place.
	**/
	void ChangeViewComplete();

	// ---------- Members

	//! \brief Holds the current view state of this class
	ObjectViewState view_state;

	//! \brief A pointer to the object who's information should be displayed
	ShopObject* selected_object;

	//! \brief The name of the selected object
	hoa_video::TextImage object_name;

	//! \brief The textual description of the object to display
	hoa_video::TextBox object_description;

	//! \brief For weapons and armor, header text identifying the physical and metaphysical ratings
	hoa_video::TextImage phys_header, meta_header;

	//! \brief For weapons and armor, a rendering of the physical and metaphysical attack/defense ratings
	hoa_video::TextImage phys_rating, meta_rating;

	//! \brief For weapons and armor, an icon image of a shard socket
	hoa_video::StillImage socket_icon;

	//! \brief For weapons and armor, text indicating how many sockets the selected equipment has available
	hoa_video::TextImage socket_text;

	//! \brief Icon images representing each of the game's elements
	std::vector<hoa_video::StillImage> elemental_icons;

	//! \brief Icon images representing each of the game's status effects
	std::vector<hoa_video::StillImage> status_icons;

	//! \brief Sprite images of all characters currently in the party
	std::vector<hoa_video::StillImage> character_sprites;

	//! \brief For weapons and armor, text to indicate an increase or decrease in phys/meta stat value for each party character
	std::vector<hoa_video::TextImage> stat_inc_text, stat_dec_text;

	//! \brief For weapons and armor, icon image that represents when a character already has the object equipped
	hoa_video::StillImage equip_icon;
}; // class BuyObjectView

} // namespace private_shop

} // namespace hoa_shop

#endif // __SHOP_BUY_HEADER__
