///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    boot_welcome.h
*** \author  Philip Vorsilak, gorzuate@allacrost.org
*** \brief   Header file for the Welcome screen, copied from boot_credits.h
***
*** This file defines the welcome screen class that is used by boot mode.
*** ***************************************************************************/

#ifndef __BOOT_WELCOME__
#define __BOOT_WELCOME__

#include <string>

#include "video.h"
#include "text.h"

namespace hoa_boot {

// TODO: this should all be defined in the private_boot namespace

/** ****************************************************************************
*** \brief Provides for everything that is needed for displaying the game 
***        welcome message.
***
*** This class is used only in boot mode.
*** *****************************************************************************/
class WelcomeScreen
{
public:
	WelcomeScreen();

	~WelcomeScreen();

	//! Draws the welcome window on the screen if it is set visible
	void Draw();

	//! Shows the welcome window
	void Show();

	//! Hides the welcome window
	void Hide();

	//! Returns true if the welcome window is set visible at the moment
	bool IsVisible();

private:
	//! Window for the screen
	hoa_video::MenuWindow _window;

	//! Is the window visible or not
	bool _visible;

	//! Rendered text header string
	hoa_video::RenderedText _welcome_header_rendered;

	//! Rendered text body string
	hoa_video::RenderedText _welcome_body_rendered;

	//! Text header to be displayed
	std::string _welcome_text_header;

	//! Text body to be displayed
	std::string _welcome_text_body;
};

} // namespace hoa_boot

#endif // __BOOT_WELCOME__
