///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    boot_menu.cpp
 * \author  Viljami Korhonen, mindflayer@allacrost.org
 * \brief   Source file for the boot-mode menu
 *****************************************************************************/

#include "boot_menu.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_global;
using namespace hoa_data;
using namespace hoa_map;
using namespace hoa_battle; // tmp

namespace hoa_boot {


BootMenu::BootMenu(BootMenu * parent) :
_parent(parent)
{
	// Add this as a new child to the parent
	if (_parent != 0)
	{
		_parent->_childs.push_back(this);
	}

	_childs.clear();
	_confirm_handlers.clear();
}


BootMenu::~BootMenu()
{
}



} // end hoa_boot
