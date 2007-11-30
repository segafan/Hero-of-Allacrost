///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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

#include "video.h"
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
"Command Name    Default Key Map    General Purpose\n\n"
"Up                  'up arrow'            Move sprite or cursor upwards\n\n"
"Down               'down arrow'       Move sprite or cursor downwards\n\n"
"Left                 'left arrow'         Move sprite or cursor to the left\n\n"
"Right               'right arrow'        Move sprite or cursor to the right\n\n"
"Confirm             'f'                     Confirm an action or menu command\n\n"
"Cancel              'd'                     Cancel an action or menu command\n\n"
"Menu                's'                     Display the main menu\n\n"
"Swap                'a'                     Swap the character being displayed\n\n"
"Left Select         'w'                     Select multiple targets or page scroll up\n\n"
"Right Select        'e'                     Select multiple targets or page scroll down\n\n"
"Pause               'spacebar'            Pause/unpause the game\n\n"
"Quit                'ESC'                 Quit the game\n\n"
"Fullscreen          'Ctrl+f'              Toggles between full screen mode and windowed mode\n\n"
"Quit                'Ctrl+q'              Quit the game\n\n"
"FPS Display         'Ctrl+r'       Toggles display of the frames per second in the upper right hand\n\n"
"Screenshot         'Ctrl+s'              Takes a screenshot and saves it to 'screenshot.jpg'")

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
	VideoManager->SetDrawFlags(VIDEO_Y_TOP, 0);
	VideoManager->Move(512.0f, 570.0f);
	_welcome_header_rendered.Draw();
	VideoManager->Move(512.0f, 504.0f);
	_welcome_body_rendered.Draw();
	VideoManager->SetDrawFlags(VIDEO_Y_CENTER, 0);
}


// Shows the welcome window
void WelcomeScreen::Show()
{
	_window.Show();
	_visible = true;
	VideoManager->Text()->SetDefaultFont("default"); // Reset font

	_welcome_header_rendered = TextImage(_welcome_text_header);
	_welcome_body_rendered   = TextImage(_welcome_text_body, TextStyle(), TextImage::ALIGN_LEFT);
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
