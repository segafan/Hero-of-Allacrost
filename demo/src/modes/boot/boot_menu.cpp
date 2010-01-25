///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
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
using namespace hoa_video;

namespace hoa_boot {

// Static BootMenu members
BootMode* BootMenu::active_boot_mode = 0;


// Adds a new option with the desired function attached to it
void BootMenu::AddOption(const ustring& text, void (BootMode::*confirm_function)(), void (BootMode::*left_function)(),
	void (BootMode::*right_function)(), void (BootMode::*up_function)(), void (BootMode::*down_function)())
{
	OptionBox::AddOption(text);

	// Add key handlers (even if they're NULL)
	_confirm_handlers.push_back(confirm_function);
	_left_handlers.push_back(left_function);
	_right_handlers.push_back(right_function);
	_up_handlers.push_back(up_function);
	_down_handlers.push_back(down_function);
}



void BootMenu::InputConfirm() {
	OptionBox::InputConfirm();

	int32 selection = OptionBox::GetSelection();
	if (selection != -1 && !_confirm_handlers.empty()) {
		void (BootMode::*confirm_function)() = _confirm_handlers.at(selection);
		if (confirm_function != 0)
		{
			(active_boot_mode->*confirm_function)();
		}
	}
}



void BootMenu::InputLeft() {
	OptionBox::InputLeft();

	int32 selection = OptionBox::GetSelection();
	if (selection != -1 && !_left_handlers.empty())
	{
		void (BootMode::*left_function)() = _left_handlers.at(selection);
		if (left_function != 0)
		{
			(active_boot_mode->*left_function)();
		}
	}
}



void BootMenu::InputRight() {
	OptionBox::InputRight();

	int32 selection = OptionBox::GetSelection();
	if (selection != -1) {
		void (BootMode::*right_function)() = _right_handlers.at(selection);
		if (right_function != 0 && !_right_handlers.empty()) {
			(active_boot_mode->*right_function)();
		}
	}
}



void BootMenu::InputUp() {
	OptionBox::InputUp();

	int32 selection = OptionBox::GetSelection();
	if (selection != -1) {
		void (BootMode::*up_function)() = _up_handlers.at(selection);
		if (up_function != 0 && !_up_handlers.empty()) {
			(active_boot_mode->*up_function)();
		}
	}
}



void BootMenu::InputDown() {
	OptionBox::InputDown();

	int32 selection = OptionBox::GetSelection();
	if (selection != -1) {
		void (BootMode::*down_function)() = _down_handlers.at(selection);
		if (down_function != 0 && !_down_handlers.empty())
		{
			(active_boot_mode->*down_function)();
		}
	}
}

} // end hoa_boot
