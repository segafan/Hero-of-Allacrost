///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_leave.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for leave interface of shop mode
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
#include "shop_leave.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_input;
using namespace hoa_video;

namespace hoa_shop {

namespace private_shop {

// *****************************************************************************
// ***** LeaveInterface class methods
// *****************************************************************************

LeaveInterface::LeaveInterface() {

}



LeaveInterface::~LeaveInterface() {

}



void LeaveInterface::Initialize() {

}



void LeaveInterface::Update() {
	if (InputManager->ConfirmPress() || InputManager->CancelPress()) {
		ShopMode::CurrentInstance()->ChangeState(SHOP_STATE_ROOT);
	}
}



void LeaveInterface::Draw() {

}

} // namespace private_shop

} // namespace hoa_shop
