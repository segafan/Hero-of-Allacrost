///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    boot_welcome.cpp
*** \author  Philip Vorsilak, gorzuate@allacrost.org
*** \brief   Source file for the welcome screen, copied from boot_credits.cpp
*** ***************************************************************************/

#include "script.h"

#include "boot_welcome.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_script;

namespace hoa_boot {

WelcomeScreen::WelcomeScreen() :
	_visible(false),
	_welcome_text_header(
        "If you have not read the MANUAL, the game controls follow:\n"
                    "(press any key to continue)"),
	_welcome_text_body(
"Command Name       Default Key Map       General Purpose\n\n"
"Up                          up arrow                  Move sprite or cursor upwards\n"
"Down                      down arrow              Move sprite or cursor downwards\n"
"Left                        left arrow                 Move sprite or cursor to the left\n"
"Right                      right arrow                Move sprite or cursor to the right\n"
"Confirm                   F                           Confirm an action or menu command\n"
"Cancel                     D                           Cancel an action or menu command\n"
"Menu                      S                            Display the main menu\n"
"Swap                       A                            Swap the character being displayed\n"
"Left Select               W                           Select multiple targets or page scroll up\n"
"Right Select              E                            Select multiple targets or page scroll down\n"
"Pause                      spacebar                    Pause/unpause the game\n"
"Quit                       ESC                         Quit the game\n"
"Fullscreen                Ctrl+F                     Toggles between full screen mode and windowed mode\n"
"Quit                       Ctrl+Q                     Quit the game\n"
"FPS Display              Ctrl+R                     Toggles display of the frames per second drawn\n"
"Screenshot               Ctrl+S                      Takes a screenshot")
{
	// Init the background window
	_window.Create(1024.0f, 600.0f);
	_window.SetPosition(0.0f, 630.0f);
	_window.Hide();
}


WelcomeScreen::~WelcomeScreen()
{
	_window.Destroy();
}


// Draws the welcome window on the screen if it is set visible
void WelcomeScreen::Draw()
{
	// Draw the background window
	_window.Draw();
		
	if (_window.GetState() != VIDEO_MENU_STATE_SHOWN) // Don't draw any text until the window is ready for drawing
		return;
	
	// Set clip region for the text and draw the visible part of it
	VideoManager->PushState();
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_TOP, 0);
	VideoManager->Move(512.0f, 600.0f);
	_welcome_header_rendered.Draw();
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
	VideoManager->Move(220.0f, 530.0f);
	_welcome_body_rendered.Draw();
	VideoManager->SetDrawFlags(VIDEO_Y_CENTER, 0);
	VideoManager->PopState();
}


// Shows the welcome window
void WelcomeScreen::Show()
{
	_window.Show();
	_visible = true;
	VideoManager->Text()->SetDefaultFont("default"); // Reset font

	_welcome_header_rendered = TextImage(_welcome_text_header);
	_welcome_body_rendered   = TextImage(_welcome_text_body, TextStyle());
}


// Hides the welcome window
void WelcomeScreen::Hide()
{
	_window.Hide();
	_visible = false;
	VideoManager->Text()->SetDefaultTextColor(Color::white); // Reset text color
	_welcome_header_rendered.Clear();
	_welcome_body_rendered.Clear();
}


// Returns true if the welcome window is set visible at the moment
bool WelcomeScreen::IsVisible()
{
	return _visible;
}


} // end hoa_boot
