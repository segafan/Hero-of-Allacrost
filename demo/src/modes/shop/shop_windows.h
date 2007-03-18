///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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
*** ***************************************************************************/

#ifndef __SHOP_WINDOWS_HEADER__
#define __SHOP_WINDOWS_HEADER__

#include "defs.h"
#include "utils.h"
#include "global.h"
#include "video.h"

namespace hoa_shop {

namespace private_shop {

/** ****************************************************************************
*** \brief Represents the top window in shop mode which contains the main shop actions
***
*** Shop actions include "buy", "sell", etc. This window also contains financial
*** information about the party and the current purchases.
*** ***************************************************************************/
class ShopActionWindow : public hoa_video::MenuWindow {
public:
	ShopActionWindow();

	~ShopActionWindow();

	//! \brief Handles user input and updates the state of the window
	void Update();

	//! \brief Draws the window to the screen
	void Draw();

	/** \brief The list of options for what the player may do in shop mode
	*** Each option includes the name of the object and its price.
	**/
	hoa_video::OptionBox options;

	//! \brief Prints financial information in the bottom of the window
	hoa_video::TextBox text_box;
}; // class ShopActionWindow : public hoa_video::MenuWindow


/** ****************************************************************************
*** \brief A window containing a list of objects and their price
***
***
*** ***************************************************************************/
class ObjectListWindow : public hoa_video::MenuWindow {
public:
	ObjectListWindow();

	~ObjectListWindow();

	// -------------------- Class Methods

	//! \brief Removes all object entries from the list
	void Clear();

	/** \brief Adds a new entry to the option box
	*** \param name The name of the object for this entry
	*** \param price The price of the object in this entry
	**/
	void AddEntry(hoa_utils::ustring name, uint32 price);

	//! \brief Reconstructs the option box from the entries that have been added
	void ConstructList();

	//! \brief Processes user input and updates the cursor
	void Update();

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
}; // class ObjectListWindow : public hoa_video::MenuWindow


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
}; // class ObjectInfoWindow : public hoa_video::MenuWindow

} // namespace private_shop

} // namespace hoa_shop

#endif // __SHOP_WINDOWS_HEADER__
