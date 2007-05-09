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
	_welcome_rendered(NULL),
	_welcome_text(
"If you have not read the MANUAL, the game controls follow:\n\
		(press any key to continue)\n\
\n\
\n\
\n\
Command Name    Default Key Map    General Purpose                                                            \n\
Up                  'up arrow'            Move sprite or cursor upwards                                        \n\
Down               'down arrow'       Move sprite or cursor downwards                                      \n\
Left                 'left arrow'         Move sprite or cursor to the left                                       \n\
Right               'right arrow'        Move sprite or cursor to the right                                     \n\
Confirm             'f'                     Confirm an action or menu command                                   \n\
Cancel              'd'                     Cancel an action or menu command                                     \n\
Menu                's'                     Display the main menu                                                    \n\
Swap                'a'                     Swap the character being displayed                                     \n\
Left Select         'w'                     Select multiple targets or page scroll up                             \n\
Right Select        'e'                     Select multiple targets or page scroll down                          \n\
Pause               'spacebar'            Pause/unpause the game                                                   \n\
Quit                'ESC'                 Quit the game                                                            \n\
Fullscreen          'Ctrl+f'              Toggles between full-screen mode and windowed mode              \n\
Quit                'Ctrl+q'              Quit the game                                                           \n\
FPS Display         'Ctrl+r'       Toggles display of the frames per second in the upper-right hand corner\n\
Screenshot         'Ctrl+s'              Takes a screenshot and saves it to 'screenshot.jpg'                    ")

{
	// Init the background window
	_window.Create(1024.0f, 600.0f);
	_window.SetPosition(0.0f, 630.0f);
	_window.Hide();
}


WelcomeScreen::~WelcomeScreen()
{
	_window.Destroy();
	if (_welcome_rendered != NULL) {
		delete(_welcome_rendered);
		_welcome_rendered = NULL;
	}
}


// Draws the welcome window on the screen if it is set visible
void WelcomeScreen::Draw()
{
	// Draw the background window
	_window.Draw();
		
	if (_window.GetState() != VIDEO_MENU_STATE_SHOWN) // Don't draw any text until the window is ready for drawing
		return;

	// Set clip region for the text and draw the visible part of it
	VideoManager->Move(512.0f, 450.0f);
	if (_welcome_rendered) {
		_welcome_rendered->Draw();
	}
}


// Shows the welcome window
void WelcomeScreen::Show()
{
	_window.Show();
	_visible = true;
	VideoManager->SetFont("default"); // Reset font

	if (!_welcome_rendered) {
		_welcome_rendered = VideoManager->RenderText(MakeUnicodeString(_welcome_text));
		if (!_welcome_rendered) {
			cerr << "BOOT ERROR: failed to render the welcome string.\n" << endl;
			exit(1);
		}
	}
}


// Hides the welcome window
void WelcomeScreen::Hide()
{
	_window.Hide();
	_visible = false;
	VideoManager->SetTextColor(Color::white); // Reset text color
	if (_welcome_rendered) {
		delete(_welcome_rendered);
		_welcome_rendered = NULL;
	}
}


// Returns true if the welcome window is set visible at the moment
bool WelcomeScreen::IsVisible()
{
	return _visible;
}


} // end hoa_boot
