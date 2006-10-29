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
_text_offset_y(0.0f),
_credits_text(
			  "Hero of Allacrost Development Team\n"
			  "\n\n\n\n"
			  "Founder and Lead Designer\n\n"
			  "Tyler Olsen (Roots)\n"
			  "\n\n\n"
			  "Deputy Lead\n\n"
			  "Philip Vorsilak (gorzuate)\n"
			  "\n\n\n"
			  "Team Manager\n\n"
			  "Emre Motan (emrebfg)\n"
			  "\n\n\n"
			  "Programming Team\n\n"
			  "Corey Hoffstein (visage) ~ Programming lead, battle mode engine\n\n"
			  "Tyler Olsen (Roots) ~ Game engine, audio engine, map mode engine\n\n"
			  "Raj Sharma (roos) ~ Video engine\n\n"
			  "Philip Vorsilak (gorzuate) ~ Level editor\n\n"
			  "Viljami Korhonen (MindFlayer) ~ Boot mode engine\n\n"
			  "Daniel Steuernol (Steu) ~ Menu mode engine\n"
			  "\n\n\n"
			  "Artwork Team\n\n"
			  "Joe Raucci (Sylon) ~ Battle sprites, map sprites, GUI artwork\n\n"
			  "Brett Steele (Safir~Kreuz) ~ Map sprites, battle sprites, character portraits\n\n"
			  "Richard Kettering (Jetryl) ~ Map tiles, map sprites, inventory icons, location graphics\n\n"
			  "Josiah Tobin (Josiah Tobin) ~ Map tiles\n\n"
			  "Matthew James (nunvuru) ~ Website graphics, GUI artwork, Allacrost logos\n\n"
			  "Victoria Smith (alenacat) ~ Map sprites, map tiles\n\n"
			  "Jerimiah Short (BigPapaN0z) ~ Map tiles\n\n"
			  "Max Humber (zomby138) ~ Conceptual Art, Computer Graphics\n\n"
			  "\n\n\n"
			  "Music and Sound Team\n\n"
			  "Ryan Reilly (Rain) ~ Music and sound lead, soundtrack composer\n\n"
			  "Joe Rouse (Loodwig) ~ Soundtrack composer\n\n"
			  "Zhe Zhou (shizeet) ~ Sound mixer\n\n"
			  "Jamie Bremaneson (Jam) ~ Sound composer\n\n"
			  "\n\n\n"
			  "Online Services\n\n"
			  "Matthew James (nunvuru) ~ Website design\n\n"
			  "Emre Motan (emrebfg) ~ Website content, forum administration\n\n"
			  "Daniel Steuernol (Steu) ~ Wiki support\n"
			  "\n\n\n"
			  "Story\n\n"
			  "Tyler Olsen (Roots) ~ Author\n\n"
			  "Tim Hargreaves (Balthazar) ~ Editor\n"
			  "\n\n\n"
			  "Packaging\n\n"
			  "Philip Vorsilak (gorzuate) ~ Source distribution\n\n"
			  "Viljami Korhonen (MindFlayer) ~ Windows distribution\n\n"
			  "Alistair Lynn (prophile) ~ OS X distribution\n\n"
			  "(ettin) ~ Debian distribution\n\n"
			  "\n\n\n"
			  "Additional Programming\n\n"
			  "Nick Weihs (nickw) ~ Video engine\n\n"
			  "Vladimir Mitrovic (snipe714) ~ Scripting engine\n\n"
			  "Farooq Mela (CamelJockey) ~ Video engine\n\n"
			  "Kevin Martin (kev82) ~ Video engine, game engine\n\n"

			  "\n\n\n"
			  "Additional Artwork\n\n"
			  "Tyler Stroud (gloomcover) ~ Map tiles\n\n"
			  "Jason Frailey (Valdroni) ~ Concept art, map sprites\n\n"
			  "Nathan Christie (Adarias) ~ Concept art, map sprites, map tiles\n\n"
			  "(fmunoz) ~ Inventory icons\n\n"
			  "(Jarks) ~ Map tiles, inventory icons\n\n"
			  "(wayfarer) ~ Concept art, map sprites\n\n"
			  "Chris Luspo (Venndetta1) ~ Concept art\n\n"
			  "Jon Williams (Jonatron) ~ Map sprites\n\n"
			  "\n\n\n"
			  "Additional Music and Sound\n\n"
			  "Matt Dexter (Star Pilot) ~ Soundtrack composer\n\n"
			  "Jean Malary (hamiko) ~ Sound mixer\n"
			  "\n\n\n"
			  "Additional Internet Services\n\n"
			  "Tim Hargreaves (Balthazar) ~ previous website, forum administration\n\n"
			  "Felix Kastner (Biohazard) ~ previous website\n"
			  "\n\n\n"
			  "Additional Translation\n\n"
			  "Mikko Hanninen (Burnsaber) ~ Finnish prologue\n"
			  "\n\n\n"
			  "Extra Thanks\n\n"
			  "Adam Lindquist (Zorbfish) ~ Scripting engine\n\n"
			  "(Melchior)\n\n"
			  "(Egan1)"
			  )
{
	// Init the background window
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
	// Draw the background window
	_window.Draw();
	if (_window.GetState() != VIDEO_MENU_STATE_SHOWN) // Don't draw any text until the window is ready for drawing
		return;

	// Set clip region for the text and draw the visible part of it
	VideoManager->Move(512.0f, 450 + _text_offset_y);
	VideoManager->SetScissorRect(_window.GetScissorRect());
	VideoManager->EnableScissoring(true);
	VideoManager->DrawText(MakeUnicodeString(_credits_text));
	VideoManager->EnableScissoring(false);
}


// Updates the credits window
void CreditsScreen::UpdateWindow(int32 frameTime)
{
	_window.Update(frameTime);
	float color_alpha = _text_offset_y * 0.025f;
	float delta = static_cast<float>(frameTime) * 0.02f;
	_text_offset_y += delta; // Update text offset

	// Fade in the text by setting new text color with alpha value below 1.0f
	VideoManager->SetTextColor(Color(1.0f, 1.0f, 1.0f, (color_alpha > 1.0f) ? 1.0f : color_alpha));
}


// Shows the credits window
void CreditsScreen::Show()
{
	_window.Show();
	_visible = true;
	_text_offset_y = 0.0f; // Reset the text offset
	VideoManager->SetFont("default"); // Reset font
}


// Hides the credits window
void CreditsScreen::Hide()
{
	_window.Hide();
	_visible = false;
	VideoManager->SetTextColor(Color::white); // Reset text color
}


// Returns true if the credits window is set visible at the moment
bool CreditsScreen::IsVisible()
{
	return _visible;
}


} // end hoa_boot
