///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_trade.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for trade menu of shop mode
***
*** WRITE SOMETHING
*** ***************************************************************************/

#ifndef __SHOP_TRADE_HEADER__
#define __SHOP_TRADE_HEADER__

#include "defs.h"
#include "utils.h"

#include "video.h"
#include "global.h"

#include "shop_utils.h"

namespace hoa_shop {

namespace private_shop {

class TradeInterface : public ShopInterface {
public:
	TradeInterface();

	~TradeInterface();

	void Initialize();

	// TODO
	void MakeActive() {}

	void Update();

	void Draw();
}; // class TradeInterface : public ShopInterface

}

}

#endif // __SHOP_TRADE_HEADER__
