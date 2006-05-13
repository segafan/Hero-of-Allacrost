///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    boot_credits.cpp
 * \author  Viljami Korhonen, mindflayer@allacrost.org
 * \brief   Source file for the Credits screen
 *****************************************************************************/
 
#include "video.h"
#include "boot_credits.h"

using namespace std;
using namespace hoa_video;
using namespace hoa_utils;

namespace hoa_boot {


CreditsScreen::CreditsScreen() :
_visible(false)
{
	// Init the credits window
	_window.Create(1024.0f, 500.0f);
	_window.SetPosition(0.0f, 630.0f);
	_window.SetDisplayMode(VIDEO_MENU_EXPAND_FROM_CENTER);
	_window.Hide();
}


CreditsScreen::~CreditsScreen()
{
	_window.Destroy();
}


// Draws the credits window on the screen if it is set visible
void CreditsScreen::Draw()
{
	_window.Draw();

	VideoManager->Move(250, 450);
	VideoManager->DrawText(MakeWideString("Credits text here!"));
}


// Updates the credits window
void CreditsScreen::UpdateWindow(int32 frameTime)
{
	_window.Update(frameTime);
}


// Shows the credits window
void CreditsScreen::Show()
{
	_window.Show();
	_visible = true;
}


// Hides the credits window
void CreditsScreen::Hide()
{
	_window.Hide();
	_visible = false;
}


// Returns true if the credits window is set visible at the moment
bool CreditsScreen::IsVisible()
{
	return _visible;
}


} // end hoa_boot
