///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for shop mode interface.
***
*** This code provides an interface for the user to purchase wares from a
*** merchant. This mode is usually entered from a map after speaking with a
*** store owner.
***
*** \todo Allow shops to have a status flag indicating if their prices (for buy
*** and for sale) are higher or lower than the common prices.
***
*** \todo Implement feature for shops to have limited quantity of wares to buy,
*** as well as the ability to re-generate more quantities as the time passes
*** between shops.
*** ***************************************************************************/

#ifndef __SHOP_HEADER__
#define __SHOP_HEADER__

#include "defs.h"
#include "utils.h"

#include "global.h"

#include "mode_manager.h"
#include "shop_windows.h"

namespace hoa_shop {

//! \brief Determines whether the code in the hoa_shop namespace should print debug statements or not.
extern bool SHOP_DEBUG;

namespace private_shop {

//! \brief Used to indicate what window has control of user input
enum SHOP_STATE {
	SHOP_STATE_INVALID   = -1,
	SHOP_STATE_ACTION    =  0,
	SHOP_STATE_BUY       =  1,
	SHOP_STATE_SELL      =  2,
	SHOP_STATE_CONFIRM   =  3,
	SHOP_STATE_PROMPT    =  4,
	SHOP_STATE_TOTAL     =  5
};


/** \brief A pointer to the currently active shop mode
*** This is used by the various shop classes so that they can refer back to the main class from which
*** they are a part of. This member is initially set to NULL. It is set whenever the ShopMode
*** constructor is invoked, and reset back to NULL when the ShopMode destructor is invoked.
**/
extern ShopMode* current_shop;

} // namespace private_shop


/** ****************************************************************************
*** \brief Handles the game execution while the player is shopping.
***
*** ShopMode allows the player to purchase items, weapons, armor, and other
*** objects. ShopMode consists of a captured screenshot which forms the
*** background image, upon which a series of menu windows are drawn. The
*** background image is of size 1024x768, and a 800x600 arrangement of windows
*** is drawn ontop of the middle of that image.
***
*** \note Shop states are a little confusing because of prompt/confirmation
*** windows. These windows appear when, for instance, the player attempts an
*** invalid operation (buy x0 quantity of an item). When this occurs, we make
*** the prompt window the current state, but also save the buy state because
*** we still want to draw the list of wares to purchase.
*** ***************************************************************************/
class ShopMode : public hoa_mode_manager::GameMode {
	friend class private_shop::ShopActionWindow;
	friend class private_shop::BuyListWindow;
	friend class private_shop::SellListWindow;
	friend class private_shop::ObjectInfoWindow;
	friend class private_shop::ConfirmWindow;
	friend class private_shop::PromptWindow;
public:
	ShopMode();

	~ShopMode();

	/** \brief Resets appropriate settings. Called whenever the ShopMode object is made the active game mode.
	*** This function additionally constructs the inventory menu from the object list. Therefore, if you add
	*** an object to the inventory it won't be seen in the list until this function is called.
	**/
	void Reset();

	//! \brief Handles user input and updates the shop menu.
	void Update();

	//! \brief Handles the drawing of everything on the shop menu and makes sub-draw function calls as appropriate.
	void Draw();

	/** \brief Adds a new object for the shop to sell
	*** \param object_id The id number of the object to add
	***
	*** You should only specify id numbers that correspond to items, weapons, or armor
	*** The newly added object won't be seen in the shop menu until the Reset() function is called.
	**/
	void AddObject(uint32 object_id);

	//! \name Class member access functions
	//@{
	uint32 GetPurchaseCost()
		{ return _purchases_cost; }

	uint32 GetSalesRevenue()
		{ return _sales_revenue; }

	//! \brief Returns the quantity: current drunes - total costs + total sales
	uint32 GetTotalRemaining()
		{ return hoa_global::GlobalManager->GetDrunes() - _purchases_cost + _sales_revenue; }
	//@}

private:
	//! \brief Keeps track of what windows are open to determine how to handle user input.
	private_shop::SHOP_STATE _state;

	//! \brief Use to retain the previous value of the _state member
	private_shop::SHOP_STATE _saved_state;

	//! \brief The total cost of all marked purchases.
	uint32 _purchases_cost;

	//! \brief The total revenue that will be earned from all marked sales.
	uint32 _sales_revenue;

	//! \brief An image of the last frame shown on the screen before ShopMode was created.
	hoa_video::StillImage _saved_screen;

	/** \brief Contains the ids of all objects which are sold in this shop
	*** The map key is the object id and the value is not used for anything (currently).
	**/
	std::map<uint32, uint32> _object_map;

	//! \brief A map of the sounds used in shop mode
	std::map<std::string, hoa_audio::SoundDescriptor> _shop_sounds;

	/** \brief Contains all of the objects
	*** \note This container is temporary, and will be replaced with multiple containers (for each
	*** type of object) at a later time.
	**/
	std::vector<hoa_global::GlobalObject*> _buy_objects;

	/** \brief Contains all of the items
	*** \note This container is temporary, and will be replaced with multiple containers (for each
	*** type of object) at a later time.
	**/
	std::vector<hoa_global::GlobalObject*> _current_inv;

	/** \brief Contains quantities corresponding to _all_objects
	**/
	std::vector<uint32> _buy_objects_quantities;

	/** \brief Contains quantities corresponding to current inventory
	**/
	std::vector<uint32> _sell_objects_quantities;

	//! \name Shopping menu windows
	//@{
	//! \brief The top window containing the shop actions (buy, sell, etc).
	private_shop::ShopActionWindow _action_window;

	//! \brief The window containing the list of wares for sale
	private_shop::BuyListWindow _buy_window;

	//! \brief The window containing the list of wares for sale
	private_shop::SellListWindow _sell_window;

	//! \brief The window that provides a detailed description of the selected object in the list window
	private_shop::ObjectInfoWindow _info_window;

	//! \brief A window to confirm various information about user requested actions
	private_shop::ConfirmWindow _confirm_window;

	//! \brief A window to prompt the user with information about their action or selection
	private_shop::PromptWindow _prompt_window;
	//@}

	// ---------- Private methods
	/** \brief Saves the current state and then changes the current state
	*** \param new_state The new state to make the current shop state
	*** \note Only use this function to change the state if you intend to
	*** return to the saved state when the new state finishes.
	**/
	void _PushAndSetState(private_shop::SHOP_STATE new_state)
		{ _saved_state = _state; _state = new_state; }

	//! \brief Retrieves the previously saved state
	void _PopState()
		{ _state = _saved_state; _saved_state = private_shop::SHOP_STATE_INVALID; }
}; // class ShopMode : public hoa_mode_manager::GameMode

} // namespace hoa_shop

#endif // __SHOP_HEADER__
