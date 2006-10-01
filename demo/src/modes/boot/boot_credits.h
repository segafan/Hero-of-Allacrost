///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    boot_credits.h
 * \author  Viljami Korhonen, mindflayer@allacrost.org
 * \brief   Header file for the Credits screen
 *
 * This file includes a simple version of the credits screen class
 * used in the boot mode. 
 *****************************************************************************/
 
#ifndef __BOOT_CREDITS__
#define __BOOT_CREDITS__

#include <string>
#include "video.h"


//! All calls to boot mode are wrapped in this namespace.
namespace hoa_boot {


/*!****************************************************************************
 *  \brief The CreditsScreen-class provides everything that's needed for simple
 * Credits-screen effects. Used only in the boot mode.
 *****************************************************************************/
class CreditsScreen
{
public:
	//! Constructor
	CreditsScreen();

	//! Destructor
	~CreditsScreen();

	//! Draws the credits window on the screen if it is set visible
	void Draw();

	//! Updates the credits window
	void UpdateWindow(int32 frameTime);

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
	const std::string _credits_text;
};


} // end hoa_boot


#endif


