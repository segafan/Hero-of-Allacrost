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
	_text_offset_y(0.0f),
	_credits_rendered(NULL)
{
	// Init the background window
	_window.Create(1024.0f, 600.0f);
	_window.SetPosition(0.0f, 630.0f);
	_window.SetDisplayMode(VIDEO_MENU_EXPAND_FROM_CENTER);
	_window.Hide();

	// Load the credits from the Lua file
	ScriptDescriptor credits_file;
	if (credits_file.OpenFile("dat/credits.lua", SCRIPT_READ) == false) {
		cerr << "BOOT ERROR: failed to open the credits file" << endl;
		exit(1);
	}
	_credits_text = credits_file.ReadString("credits_text");
	credits_file.CloseFile();
}


CreditsScreen::~CreditsScreen()
{
	_window.Destroy();
	if (_credits_rendered != NULL) {
		delete(_credits_rendered);
		_credits_rendered = NULL;
	}
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
	if (_credits_rendered)
	{
		// Fade in the text by setting new color with alpha value below 1.0f
		float color_alpha = _text_offset_y * 0.025f;
		if (color_alpha > 1.0f)
			color_alpha = 1.0f;
		GLfloat modulation[] = {1.0f, 1.0f, 1.0f, color_alpha};
		glColor4fv(modulation);
		_credits_rendered->Draw();
	}
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

	if (!_credits_rendered) {
		_credits_rendered = VideoManager->RenderText(MakeUnicodeString(_credits_text));
		if (!_credits_rendered) {
			cerr << "BOOT ERROR: failed to render the credits string.\n" << endl;
			exit(1);
		}
	}
}


// Hides the credits window
void CreditsScreen::Hide()
{
	_window.Hide();
	_visible = false;
	glColor4fv(&Color::white[0]);
	if (_credits_rendered) {
		delete(_credits_rendered);
		_credits_rendered = NULL;
	}
}


// Returns true if the credits window is set visible at the moment
bool CreditsScreen::IsVisible()
{
	return _visible;
}


} // end hoa_boot
