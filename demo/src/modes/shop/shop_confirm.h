///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_confirm.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for confirm menu of shop mode
***
*** WRITE SOMETHING
*** ***************************************************************************/

#ifndef __SHOP_CONFIRM_HEADER__
#define __SHOP_CONFIRM_HEADER__

#include "defs.h"
#include "utils.h"

#include "video.h"
#include "global.h"

#include "shop_utils.h"

namespace hoa_shop {

namespace private_shop {

class ShopConfirmInterface : public ShopInterface {
public:
	ShopConfirmInterface();

	~ShopConfirmInterface();

	void Initialize();

	void Update();

	void Draw();
}; // class ShopConfirmInterface : public ShopInterface

/** ****************************************************************************
*** \brief Displays the object's icon, name, and a sale confirmation message
***
*** This window is currently being used for the shopping cart functionality.
*** When confirmed, all buy/sell transactions are finalized.
*** ***************************************************************************/
class ConfirmWindow : public hoa_video::MenuWindow {
public:
	ConfirmWindow();

	~ConfirmWindow();

	//! \brief Updates the option box
	void Update();

	//! \brief Draws the window and the object properties contained within
	void Draw();

	//! \brief Options for the user to confirm or reject the sale
	hoa_video::OptionBox options;
}; // class ConfirmWindow : public hoa_video::MenuWindow

}

}

#endif // __SHOP_CONFIRM_HEADER__
