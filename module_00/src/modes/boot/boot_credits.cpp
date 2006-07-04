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
_visible(false),
_credits_text(
			  "Hero of Allacrost Development Team\n"
			  "\n"
			  "Tyler Olsen (Roots) ~ Project Leader a.k.a Dictator\n"
			  "\n"
			  "Emre Motan (emrebfg) ~ Team Manager\n"
			  "\n"
			  "Programming\n"
			  "\n"
			  "Corey Hoffstein (visage) ~ Programming Leader, Battle mode\n"
			  "Raj Sharma (roos) ~ Video Engine\n"
			  "Philip Vorsilak (gorzuate) ~ Map editor, Vice Dictator\n"
			  "Daniel Steuernol (Steu) ~ Menu mode, wikimaster\n"
			  "Viljami Korhonen (MindFlayer) ~ Boot mode\n"
			  "Adam Lindquist (Zorbfish) ~ Data and Scripting system\n"
			  "\n"
			  "\n"
			  "Graphics\n"
			  "\n"
			  "Joe Raucci (Sylon) ~ Artwork Leader\n"
			  "Brett Steele (SafirKreuz) ~ Sprite art, character portraits\n"
			  "Matthew James (nunvuru) ~ Web design, miscellaneous artwork\n"
			  "Victoria Smith (alenacat) ~ Map sprite art\n"
			  "Jerimiah Short (BigPapaN0z) ~ Map tile art\n"			  
			  "\n"
			  "\n"
			  "Audio\n"
			  "\n"
			  "Ryan Reilly (Rain) ~ Music Leader\n"
			  "Joe Rouse (Loodwig) ~ \n"
			  "Zhe #FULLNAME# (shizeet) ~ Sound effects\n"
			  "\n"
			  "\n"
			  "Special thanks to our contributors\n"
			  "\n"
			  "Richard Kettering (Jetryl) ~ Map sprites & tiles, art advisor\n"
			  "Parley #FULLNAME# (Jarks) ~ Map tiles, icons\n"			  
			  "Peter Geinitz (wayfarer) ~ Map sprites, menu design"
			  )
{
	// Init the credits window
	_window.Create(1024.0f, 600.0f);
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
	if (_window.GetState() != VIDEO_MENU_STATE_SHOWN)
		return;

	VideoManager->SetFont("default");
	VideoManager->Move(512.0f, 550.0f);
	VideoManager->DrawText(MakeWideString(_credits_text));
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
