///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_trade.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for trade menu of shop mode
***
*** WRITE SOMETHING
*** ***************************************************************************/

#include "defs.h"
#include "utils.h"

#include "audio.h"
#include "input.h"
#include "video.h"

#include "global.h"

#include "shop.h"
#include "shop_trade.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_input;
using namespace hoa_video;
using namespace hoa_global;

namespace hoa_shop {

namespace private_shop {

// *****************************************************************************
// ***** TradeInterface class methods
// *****************************************************************************

TradeInterface::TradeInterface() {

}



TradeInterface::~TradeInterface() {

}



void TradeInterface::Initialize() {

}



void TradeInterface::MakeActive() {
}



void TradeInterface::MakeInactive() {
}



void TradeInterface::Update() {
	if (InputManager->ConfirmPress() || InputManager->CancelPress()) {
		ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_ROOT);
	}
}



void TradeInterface::Draw() {

}

} // namespace private_shop

} // namespace hoa_shop
