///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
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

#include "script.h"

#include "boot_credits.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_script;

namespace hoa_boot {

namespace private_boot {

CreditsScreen::CreditsScreen() :
	_visible(false),
	_scroll_offset(0.0f)
{
	// Init the background window
	_window.Create(1024.0f, 600.0f);
	_window.SetPosition(0.0f, 630.0f);
	_window.SetDisplayMode(VIDEO_MENU_EXPAND_FROM_CENTER);
	_window.Hide();

	// Load the credits text from the Lua file
	ReadScriptDescriptor credits_file;
	if (credits_file.OpenFile("dat/credits.lua") == false) {
		IF_PRINT_WARNING(BOOT_DEBUG) << "failed to open the Lua credits file" << endl;
	}
	_credits_text = MakeUnicodeString(credits_file.ReadString("credits_text"));
	credits_file.CloseFile();

	// Use the default text style for the credits
	_credits_rendered.SetStyle(TextStyle());
}



CreditsScreen::~CreditsScreen() {
	_window.Destroy();
}



void CreditsScreen::Draw() {
	_window.Draw();
	// Don't draw any text until the window is fully shown
	if (_window.GetState() != VIDEO_MENU_STATE_SHOWN)
		return;

	// Set clip region for the text and draw the visible part of it
	VideoManager->Move(512.0f, 384.0f + _scroll_offset);
	// TODO: This returns a bad scissor rect, video engine bug likely
// 	VideoManager->SetScissorRect(_window.GetScissorRect());
	VideoManager->SetScissorRect(ScreenRect(0, 50, 1024, 550));
	VideoManager->EnableScissoring();

	// Fade in the text by setting new color with alpha value below 1.0f
	float color_alpha = _scroll_offset * 0.025f;
	if (color_alpha > 1.0f)
		color_alpha = 1.0f;
	Color modulated(1.0f, 1.0f, 1.0f, color_alpha);

	VideoManager->SetDrawFlags(VIDEO_Y_TOP, 0);
	_credits_rendered.Draw(modulated);
	VideoManager->SetDrawFlags(VIDEO_Y_CENTER, 0);

	VideoManager->DisableScissoring();
}



void CreditsScreen::Update(uint32 time) {
	_window.Update(time);
	_scroll_offset += static_cast<float>(time) * 0.025f; // Update the scroll offset
}



void CreditsScreen::Show() {
	_window.Show();
	_visible = true;
	_scroll_offset = 0.0f;
	_credits_rendered.SetText(_credits_text);
}



void CreditsScreen::Hide() {
	_window.Hide();
	_visible = false;

	// Remove rendered text image from memory. We reconstruct it if the credits are shown once more.
	_credits_rendered.Clear();
}

} // namespace private_boot

} // namespace hoa_boot
