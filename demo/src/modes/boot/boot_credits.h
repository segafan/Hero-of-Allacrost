///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    boot_credits.h
*** \author  Viljami Korhonen, mindflayer@allacrost.org
*** \brief   Header file for the Credits screen
***
*** This file defines the credits screen class that is used by boot mode.
*** ***************************************************************************/

#ifndef __BOOT_CREDITS__
#define __BOOT_CREDITS__

#include <string>

#include "video.h"

namespace hoa_boot {

namespace private_boot {

/** ****************************************************************************
*** \brief Provides for everything that is needed for displaying the game credits
*** ***************************************************************************/
class CreditsScreen {
public:
	CreditsScreen();

	~CreditsScreen();

	//! \brief Draws the credits window on the screen if it is set visible
	void Draw();

	/** \brief Updates the credits window
	*** \param time The number of milliseconds to update the credits screen by
	**/
	void Update(uint32 time);

	//! \brief Shows the credits window
	void Show();

	//! \brief Hides the credits window
	void Hide();

	//! \brief Returns true if the credits window is visible (not hidden)
	bool IsVisible()
		{ return _visible; }

private:
	//! \brief The MenuWindow for the credits backdrop
	hoa_video::MenuWindow _window;

	//! \brief Retains the credits text to be displayed
	hoa_utils::ustring _credits_text;

	//! \brief The rendered text of the credits
	hoa_video::TextImage _credits_rendered;

	//! \brief Is the window visible or not
	bool _visible;

	//! \brief The vertical offset for the scrolling credits text
	float _scroll_offset;
}; // class CreditsScreen

} // namespace private_boot

} // namespace hoa_boot

#endif // __BOOT_CREDITS__
