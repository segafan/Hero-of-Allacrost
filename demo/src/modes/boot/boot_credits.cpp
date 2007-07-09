///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    boot_credits.cpp
*** \author  Viljami Korhonen, mindflayer@allacrost.org
*** \brief   Source file for the credits screen
*** ***************************************************************************/

#include "video.h"
#include "script.h"

#include "boot_credits.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_script;

namespace hoa_boot {

CreditsScreen::CreditsScreen() :
	_visible(false),
	_text_offset_y(0.0f)
{
	// Init the background window
	_window.Create(1024.0f, 600.0f);
	_window.SetPosition(0.0f, 630.0f);
	_window.SetDisplayMode(VIDEO_MENU_EXPAND_FROM_CENTER);
	_window.Hide();

	// Load the credits from the Lua file
	ReadScriptDescriptor credits_file;
	if (credits_file.OpenFile("dat/credits.lua") == false) {
		cerr << "BOOT ERROR: failed to open the credits file" << endl;
		exit(1);
	}
	_credits_text = MakeUnicodeString(credits_file.ReadString("credits_text"));
	credits_file.CloseFile();
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
	VideoManager->Move(512.0f, 384.0f + _text_offset_y);
	VideoManager->SetScissorRect(_window.GetScissorRect());
	VideoManager->EnableScissoring(true);

	// Fade in the text by setting new color with alpha value below 1.0f
	float color_alpha = _text_offset_y * 0.025f;
	if (color_alpha > 1.0f)
		color_alpha = 1.0f;
	Color modulated(1.0f, 1.0f, 1.0f, color_alpha);

	VideoManager->SetDrawFlags(VIDEO_Y_TOP, 0);
	VideoManager->DrawImage(_credits_rendered, modulated);
	VideoManager->SetDrawFlags(VIDEO_Y_CENTER, 0);

	VideoManager->EnableScissoring(false);
}


// Updates the credits window
void CreditsScreen::UpdateWindow(int32 frameTime)
{
	_window.Update(frameTime);
	float delta = static_cast<float>(frameTime) * 0.02f;
	_text_offset_y += delta; // Update text offset
}


// Shows the credits window
void CreditsScreen::Show()
{
	_window.Show();
	_visible = true;
	_text_offset_y = 0.0f; // Reset the text offset
	VideoManager->SetFont("default"); // Reset font
	VideoManager->SetTextColor(Color::white); // Reset text color
	_credits_rendered.SetText(_credits_text);
}


// Hides the credits window
void CreditsScreen::Hide()
{
	_window.Hide();
	_visible = false;

	// Explicit clear textures when leaving credits mode
	_credits_rendered.Clear();
}


// Returns true if the credits window is set visible at the moment
bool CreditsScreen::IsVisible()
{
	return _visible;
}


} // end hoa_boot
