///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    shop_leave.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for leave interface of shop mode
***
*** WRITE SOMETHING
*** ***************************************************************************/

#ifndef __SHOP_LEAVE_HEADER__
#define __SHOP_LEAVE_HEADER__

#include "defs.h"
#include "utils.h"

#include "video.h"
#include "global.h"

#include "shop_utils.h"

namespace hoa_shop {

namespace private_shop {

class LeaveInterface : public ShopInterface {
public:
	LeaveInterface();

	~LeaveInterface();

	void Initialize();

	void Update();

	void Draw();
}; // class LeaveInterface : public ShopInterface

} // namespace private_shop

} // namespace hoa_shop

#endif // __SHOP_LEAVE_HEADER__
