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
*** \brief   Header file for shop mode interface
***
*** This code provides an interface for the user to purchase wares from a
*** merchant. This mode is usually entered from a map after speaking with a
*** shop keeper.
*** ***************************************************************************/

#ifndef __SHOP_HEADER__
#define __SHOP_HEADER__

#include "defs.h"
#include "utils.h"

#include "mode_manager.h"

#include "global.h"

#include "shop_utils.h"

namespace hoa_shop {

//! \brief Determines whether the code in the hoa_shop namespace should print debug statements or not.
extern bool SHOP_DEBUG;

namespace private_shop {

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
*** coordinate system used is 1024x768, and a 800x600 arrangement of three
*** menu windows (top, middle, bottom) are drawn on top of that backdrop. These
*** windows are shared beteen all states of shop mode (root, buy, sell, etc.) and
*** the contents of the windows change depending on the active state of shop mode.
***
*** \note The recommended way to create and initialize this class is to call the
*** following methods.
***
*** -# ShopMode constructor
*** -# SetShopName()
*** -# SetGreetingText()
*** -# SetPriceLevels()
*** -# AddObject() for each object to be sold
*** -# Wait for the Reset() method to be automatically called, which will finalize shop initialization
*** ***************************************************************************/
class ShopMode : public hoa_mode_manager::GameMode {
public:
	ShopMode();

	~ShopMode();

	static ShopMode* CurrentInstance()
		{ return _current_instance; }

	//! \brief Resets appropriate settings and initializes shop if appropriate. Called whenever the ShopMode object is made the active game mode.
	void Reset();

	//! \brief Handles user input and updates the shop menu.
	void Update();

	//! \brief Handles the drawing of everything on the shop menu and makes sub-draw function calls as appropriate.
	void Draw();

	/** \brief Loads data and prepares shop for initial use
	*** This function is only be called once from the Reset() method. If it is called more than
	*** once it will print a warning and refuse to execute a second time.
	**/
	void Initialize();

	/** \brief Used when an object has been selected for purchase by the player
	*** \param object A pointer to the object to add
	*** \note The buy count of the shop object added should be non-zero before this function is called.
	*** Otherwise a warning message will be printed, but the object will still be added.
	**/
	void AddObjectToBuyList(private_shop::ShopObject* object);

	/** \brief Used when the player decides not to purchase an object that was previously marked to be bought
	*** \param object A pointer to the object to remove
	*** \note The buy count of the shop object added should be zero before this function is called.
	*** Otherwise a warning message will be printed, but the object will still be removed.
	**/
	void RemoveObjectFromBuyList(private_shop::ShopObject* object);

	/** \brief Used when an object has been selected to be sold by the player
	*** \param object A pointer to the object to add
	*** \note The sell count of the shop object added should be non-zero before this function is called.
	*** Otherwise a warning message will be printed, but the object will still be added.
	**/
	void AddObjectToSellList(private_shop::ShopObject* object);

	/** \brief Used when the player decides not to sell an object that was previously marked to be sold
	*** \param object A pointer to the object to remove
	*** \note The sell count of the shop object added should be zero before this function is called.
	*** Otherwise a warning message will be printed, but the object will still be removed.
	**/
	void RemoveObjectFromSellList(private_shop::ShopObject* object);

	/** \brief Called whenever the player successfully confirms a transaction
	*** This method processes the transaction, including modifying the party's drune count, adding/removing
	*** objects from the inventory, and auto equipping/un-equipping traded equipment. It also calls appropriate
	*** methods in the various shop interfaces to update their display lists with the updated inventory contents and
	*** shop stocks.
	**/
	void CompleteTransaction();

	/** \brief Updates the costs and sales totals
	*** \param costs_amount The amount to change the purchases cost member by
	*** \param sales_amount The amount to change the sales revenue member by
	***
	*** Obviously if one wishes to only update either costs or sales but not both, pass a zero value for the
	*** appropriate argument that should not be changed. This function should only be called when necessary because
	*** it also has to update the finance text. Thus the function does not just modify integer values but in fact
	*** does have a small amount of computational overhead
	**/
	void UpdateFinances(int32 costs_amount, int32 sales_amount);

	/** \brief Changes the active state of shop mode and prepares the interface of the new state
	*** \param new_state The state to change the shop to
	**/
	void ChangeState(private_shop::SHOP_STATE new_state);

	//! \brief Returns true if the user has indicated they wish to buy or sell any items
	bool HasPreparedTransaction() const
		{ return ((_total_costs != 0) || (_total_sales != 0)); }

	//! \brief Returns the number of drunes that the party would be left with after the marked purchases and sales
	uint32 GetTotalRemaining() const
		{ return (hoa_global::GlobalManager->GetDrunes() + _total_sales - _total_costs); }

	/** \name Exported class methods
	*** The methods in this group are avaiable to be called from within Lua. Their intended use is for setting shop settings
	*** and initializing data before the shop is opened.
	**/
	//@{
	/** \brief Sets the name of the store that should be displayed to the player
	*** \param greeting The name of the shop
	*** \note This method will only work if it is called before the shop is initialized. Calling it afterwards will
	*** result in no operation and a warning message
	**/
	void SetShopName(hoa_utils::ustring name);

	/** \brief Sets the greeting message from the shop/merchant
	*** \param greeting The text
	*** \note This method will only work if it is called before the shop is initialized. Calling it afterwards will
	*** result in no operation and a warning message
	**/
	void SetGreetingText(hoa_utils::ustring greeting);

	/** \brief Sets the buy and sell price levels for the shop
	*** \param buy_level The price level to set for wares that the player would buy from the shop
	*** \param sell_level The price level to set for wares that the player would sell to the shop
	*** \note This method will only work if it is called before the shop is initialized. Calling it afterwards will
	*** result in no operation and a warning message
	**/
	void SetPriceLevels(SHOP_PRICE_LEVEL buy_level, SHOP_PRICE_LEVEL sell_level);

	/** \brief Adds a new object for the shop to sell
	*** \param object_id The id number of the object to add
	*** \param stock The amount of the object to make available for sale at the shop
	***
	*** Adding an object after the shop mode instance has already been initialized (by being made the active game state)
	*** this call will add the object but will not be visible to the player.
	**/
	void AddObject(uint32 object_id, uint32 stock);
	//@}

	/** \brief Deletes an object from the shop
	*** \param object_id The id number of the object to remove
	***
	*** This function should be used in only one specific case. This case is when the player owns this object and
	*** chooses to sell all instances of it and additionally the shop does not sell this item. Trying to remove
	*** an object that the shop sells to the player or trying to remove an object that still remains in the party's
	*** inventory will result in a warning message and the object will not be removed.
	**/
	void RemoveObject(uint32 object_id);

	//! \name Class member access functions
	//@{
	bool IsInitialized() const
		{ return _initialized; }

	private_shop::SHOP_STATE GetState() const
		{ return _state; }

	SHOP_PRICE_LEVEL GetBuyPriceLevel() const
		{ return _buy_price_level; }

	SHOP_PRICE_LEVEL GetSellPriceLevel() const
		{ return _sell_price_level; }

	uint8 GetDealTypes() const
		{ return _deal_types; }

	uint32 GetTotalCosts() const
		{ return _total_costs; }

	uint32 GetTotalSales() const
		{ return _total_sales; }

	std::map<uint32, private_shop::ShopObject>* GetShopObjects()
		{ return &_shop_objects; }

	std::map<uint32, private_shop::ShopObject*>* GetBuyList()
		{ return &_buy_list; }

	std::map<uint32, private_shop::ShopObject*>* GetSellList()
		{ return &_sell_list; }

	const std::vector<hoa_video::StillImage>& GetObjectCategoryImages() const
		{ return _object_category_images; }

	/** \brief Retrieves a shop sound object
	*** \param identifier The string identifier for the sound to retrieve
	*** \return A pointer to the SoundDescriptor, or NULL if no sound had the identifier name
	**/
	hoa_audio::SoundDescriptor* GetSound(std::string identifier);

	hoa_video::MenuWindow* GetTopWindow()
		{ return &_top_window; }

	hoa_video::MenuWindow* GetMiddleWindow()
		{ return &_middle_window; }

	hoa_video::MenuWindow* GetBottomWindow()
		{ return &_bottom_window; }
	//@}

private:
	/** \brief A reference to the current instance of ShopMode
	*** This is used by other shop clases to be able to refer to the shop that they exist in. This member
	*** is NULL when no shop is active
	**/
	static ShopMode* _current_instance;

	//! \brief Set to true only after the shop has been initialized and is ready to be used by the player
	bool _initialized;

	//! \brief Keeps track of what windows are open to determine how to handle user input.
	private_shop::SHOP_STATE _state;

	//! \brief A bit vector that represents the types of merchandise that the shop deals in (items, weapons, etc)
	uint8 _deal_types;

	//! \brief The shop's price level of objects that the player buys from the shop
	SHOP_PRICE_LEVEL _buy_price_level;

	//! \brief The shop's price level of objects that the player sells to the shop
	SHOP_PRICE_LEVEL _sell_price_level;

	//! \brief The total cost of all marked purchases.
	uint32 _total_costs;

	//! \brief The total revenue that will be earned from all marked sales.
	uint32 _total_sales;

	/** \brief A container of objects that ShopMode created itself and need to be deleted when finished
	*** These also happen to represent a list of all global objects that the shop may sell to the player
	**/
	std::vector<hoa_global::GlobalObject*> _created_objects;

	/** \brief Holds all objects that can be bought, sold, or traded in the shop
	*** The integer key to this map is the global object ID represented by the ShopObject.
	**/
	std::map<uint32, private_shop::ShopObject> _shop_objects;

	/** \brief Holds pointers to all objects that the player plans to purchase
	*** The integer key to this map is the global object ID represented by the ShopObject.
	**/
	std::map<uint32, private_shop::ShopObject*> _buy_list;

	/** \brief Holds pointers to all objects that the player plans to sell
	*** The integer key to this map is the global object ID represented by the ShopObject.
	**/
	std::map<uint32, private_shop::ShopObject*> _sell_list;

	/** \name Shopping interfaces
	*** These are class objects which are responsible for managing each state in shop mode
	**/
	//@{
	private_shop::RootInterface* _root_interface;
	private_shop::BuyInterface* _buy_interface;
	private_shop::SellInterface* _sell_interface;
	private_shop::TradeInterface* _trade_interface;
	private_shop::ConfirmInterface* _confirm_interface;
	private_shop::LeaveInterface* _leave_interface;
	//@}

	//! \brief Holds an image of the screen taken when the ShopMode instance was created
	hoa_video::StillImage _screen_backdrop;

	//! \brief Retains all icon images for each object category
	std::vector<hoa_video::StillImage> _object_category_images;

	//! \brief A map of the sounds used in shop mode
	std::map<std::string, hoa_audio::SoundDescriptor*> _shop_sounds;

	//! \brief The highest level window that contains the shop actions and finance information
	hoa_video::MenuWindow _top_window;

	//! \brief The largest window usually used to display lists of objects
	hoa_video::MenuWindow _middle_window;

	//! \brief The lowest window typically displays detailed information or additional shop options
	hoa_video::MenuWindow _bottom_window;

	//! \brief The list of options for what the player may do in shop mode
	hoa_video::OptionBox _action_options;

	//! \brief Separate text images for each action option. Displayed when _action_options are hidden
	std::vector<hoa_video::TextImage> _action_titles;

	//! \brief Table-formatted text containing the financial information about the current purchases and sales
	hoa_video::OptionBox _finance_table;

	//! \brief Image icon representing drunes (currency)
	hoa_video::StillImage _drunes_icon;
}; // class ShopMode : public hoa_mode_manager::GameMode

} // namespace hoa_shop

#endif // __SHOP_HEADER__
