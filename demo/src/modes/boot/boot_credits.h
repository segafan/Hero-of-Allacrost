///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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

// TODO: this should all be defined in the private_boot namespace

/** ****************************************************************************
*** \brief Provides for everything that is needed for displaying the game credits.
***
*** This class is used only in boot mode.
*** *****************************************************************************/
class CreditsScreen
{
public:
	CreditsScreen();

	~CreditsScreen();

	//! Draws the credits window on the screen if it is set visible
	void Draw();

	//! Updates the credits window
	void UpdateWindow(int32 frame_time);

	//! Shows the credits window
	void Show();

	//! Hides the credits window
	void Hide();

	//! Returns true if the credits window is set visible at the moment
	bool IsVisible();

private:
	//! Window for the screen
	hoa_video::MenuWindow _window;

	//! Is the window visible or not
	bool _visible;

	//! Text Y offset
	float _text_offset_y;

	//! Text to be displayed
	std::string _credits_text;

	//! Rendered text string
	hoa_video::RenderedString* _credits_rendered;
};

} // namespace hoa_boot

#endif // __BOOT_CREDITS__
