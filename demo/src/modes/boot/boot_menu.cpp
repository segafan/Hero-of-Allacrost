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
using namespace hoa_video;


namespace hoa_boot {

// Static BootMenu members
hoa_video::MenuWindow * BootMenu::_menu_window = 0;
BootMode * BootMenu::_boot_mode = 0;


BootMenu::BootMenu(BootMenu * parent, bool windowed, BootMode * boot_mode) :
_parent_menu(parent),
_is_windowed(windowed)
{
	// Store pointer to the boot mode (used for member function pointer calls)
	if (boot_mode != 0)
		_boot_mode = boot_mode;

	SetWindowed(_is_windowed);

	// Init the menu window if it's not available yet
	_InitWindow();
}


BootMenu::~BootMenu()
{
	if (_menu_window)
	{
		_menu_window->Destroy();
		delete _menu_window;
		_menu_window = 0;
	}
}


// Adds a new option with the desired function attached to it
void BootMenu::AddOption(const hoa_utils::ustring & text, void (BootMode::*confirm_function)(), void (BootMode::*left_function)(), void (BootMode::*right_function)(), void (BootMode::*up_function)(), void (BootMode::*down_function)())
{
	_current_menu.AddOption(text);

	// Set new size
	if (!_is_windowed)
		_current_menu.SetSize(_current_menu.GetNumOptions(), 1);
	else // windowed mode has vertical menus
		_current_menu.SetSize(1, _current_menu.GetNumOptions());

	if (_current_menu.GetNumOptions() == 1)
	{
		_current_menu.SetSelection(0);
	}

	// Add key handlers (even if they're zero!)
	_confirm_handlers.push_back(confirm_function);
	_left_handlers.push_back(left_function);
	_right_handlers.push_back(right_function);
	_up_handlers.push_back(up_function);
	_down_handlers.push_back(down_function);
}


// Changes text of the option
void BootMenu::SetOptionText(uint32 index, const hoa_utils::ustring & text)
{
	_current_menu.SetOptionText(index, text);
}

// Disables a given option
void BootMenu::EnableOption(int32 index, bool enable)
{
	_current_menu.EnableOption(index, enable);
}


// Toggles this menu to be windowed (or not windowed)
void BootMenu::SetWindowed(bool windowed)
{
	_is_windowed = windowed;

	if (!_is_windowed) // without a window
	{
		_current_menu.SetFont("default");
		_current_menu.SetCellSize(150.0f, 70.0f);
		_current_menu.SetPosition(552.0f, 50.0f);
		_current_menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
		_current_menu.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
		_current_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
		_current_menu.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
		_current_menu.SetCursorOffset(-50.0f, 28.0f);
		_current_menu.SetSize(_current_menu.GetNumOptions(), 1);
	}
	else // windowed
	{
		_current_menu.SetFont("default");
		_current_menu.SetCellSize(210.0f, 50.0f);
		_current_menu.SetPosition(512.0f, 200.0f);
		_current_menu.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
		_current_menu.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
		_current_menu.SetSelectMode(VIDEO_SELECT_SINGLE);
		_current_menu.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
		_current_menu.SetCursorOffset(-50.0f, 28.0f);
		_current_menu.SetSize(1, _current_menu.GetNumOptions());
		_current_menu.SetOwner(_menu_window);
	}
}


// Sets a new parent for this menu
void BootMenu::SetParent(BootMenu * parent)
{
	_parent_menu = parent;
}


// Sets new text density
void BootMenu::SetTextDensity(float density)
{
	if (_is_windowed)
		_current_menu.SetCellSize(210.0f, density); // Vertical
	else
		_current_menu.SetCellSize(density, 50.0f); // Horizontal
}


// Updates menu events. Call this every frame just like you would do on OptionBox!
int32 BootMenu::GetEvent()
{
	return _current_menu.GetEvent();
}


// Returns parent of the menu
BootMenu * BootMenu::GetParent() const
{
	return _parent_menu;
}


// Returns boolean if the menu is windowed or not
bool BootMenu::IsWindowed() const
{
	return _is_windowed;
}


// Returns true if the currently selected option is enabled and a confirm handler is available
bool BootMenu::IsSelectionEnabled() const
{
	return _current_menu.IsEnabled(_current_menu.GetSelection()) && 
		_confirm_handlers.at(_current_menu.GetSelection()) != 0;
}


// Draws menu on the screen
bool BootMenu::Draw()
{	
	_menu_window->Draw();

	VideoManager->EnableScissoring(false); // Scissoring is/was an issue when changing resolution. For now, disable it when drawing the menus :/
	_current_menu.Draw();
	return true;
	// NOTE: this statement does not do anything!
	VideoManager->EnableScissoring(true);
}


// Handles the confirm key
void BootMenu::ConfirmPressed()
{
	_current_menu.HandleConfirmKey();

	int32 selection = _current_menu.GetSelection();
	if (selection != -1 && !_confirm_handlers.empty())
	{
		void (BootMode::*confirm_function)() = _confirm_handlers.at(selection);
		if (confirm_function != 0)
		{
			(_boot_mode->*confirm_function)();
		}
	}
}


// Handles the left key
void BootMenu::LeftPressed()
{
	_current_menu.HandleLeftKey();

	int32 selection = _current_menu.GetSelection();
	if (selection != -1 && !_left_handlers.empty())
	{
		void (BootMode::*left_function)() = _left_handlers.at(selection);
		if (left_function != 0)
		{
			(_boot_mode->*left_function)();
		}
	}
}


// Handles the right key
void BootMenu::RightPressed()
{
	_current_menu.HandleRightKey();

	int32 selection = _current_menu.GetSelection();
	if (selection != -1)
	{
		void (BootMode::*right_function)() = _right_handlers.at(selection);
		if (right_function != 0 && !_right_handlers.empty())
		{
			(_boot_mode->*right_function)();
		}
	}
}


// Handles the up key
void BootMenu::UpPressed()
{
	_current_menu.HandleUpKey();

	int32 selection = _current_menu.GetSelection();
	if (selection != -1)
	{
		void (BootMode::*up_function)() = _up_handlers.at(selection);
		if (up_function != 0 && !_up_handlers.empty())
		{
			(_boot_mode->*up_function)();
		}
	}
}


// Handles the down key
void BootMenu::DownPressed()
{
	_current_menu.HandleDownKey();

	int32 selection = _current_menu.GetSelection();
	if (selection != -1)
	{
		void (BootMode::*down_function)() = _down_handlers.at(selection);
		if (down_function != 0 && !_down_handlers.empty())
		{
			(_boot_mode->*down_function)();
		}
	}
}


// Handles the cancel key
void BootMenu::CancelPressed()
{
	_current_menu.HandleCancelKey();
}


// Updates the menu window
void BootMenu::UpdateWindow(int32 frameTime)
{
	if (_menu_window)
		_menu_window->Update(frameTime);
}


// Shows or hides the menu window
void BootMenu::ShowWindow(bool toggle)
{
	if (toggle)
		_menu_window->Show();
	else
		_menu_window->Hide();
}


// Inits the menu window, called in the constructor of the class
void BootMenu::_InitWindow()
{
	if (_menu_window == 0)
	{
		_menu_window = new hoa_video::MenuWindow();
		_menu_window->Create(1024.0f, 400.0f);
		_menu_window->SetPosition(0.0f, 560.0f);
		_menu_window->SetDisplayMode(VIDEO_MENU_EXPAND_FROM_CENTER);
		_menu_window->Hide(); // Hide on by default
	}
}


} // end hoa_boot
