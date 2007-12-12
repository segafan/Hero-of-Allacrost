////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    pause.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for pause mode interface.
*** ***************************************************************************/

#include <iostream>


#include "audio.h"
#include "video.h"
#include "input.h"
#include "system.h"
#include "boot.h"
#include "pause.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_boot;

namespace hoa_pause {

bool PAUSE_DEBUG = false;

//! \name Quit Options Constants
//@{
const uint8 QUIT_GAME      = 0;
const uint8 QUIT_TO_BOOT   = 1;
const uint8 QUIT_CANCEL    = 2;
//@}

PauseMode::PauseMode(bool quit_state, bool pause_audio) :
	GameMode(),
	_quit_state(quit_state),
	_audio_paused(pause_audio),
	_dim_color(0.35f, 0.35f, 0.35f, 1.0f) // A grayish opaque color
{
	mode_type = MODE_MANAGER_PAUSE_MODE;

	// Render the PAUSED string in white text
	_paused_text.SetStyle(TextStyle("default", VIDEO_TEXT_SHADOW_BLACK, Color::white));
	_paused_text.SetText(MakeUnicodeString("PAUSED"));

	// Initialize the quit options box
	_quit_options.SetFont("default");
	_quit_options.SetSize(3, 1);
	_quit_options.SetCellSize(250.0f, 50.0f);
	_quit_options.SetPosition(512.0f, 384.0f);
	_quit_options.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_quit_options.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_quit_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_quit_options.SetCursorOffset(-58.0f, 18.0f);

	_quit_options.AddOption(MakeUnicodeString("Quit Game"));
	_quit_options.AddOption(MakeUnicodeString("Quit to Main Menu"));
	_quit_options.AddOption(MakeUnicodeString("Cancel"));
	_quit_options.SetSelection(QUIT_CANCEL);
}



PauseMode::~PauseMode() {
	if (_audio_paused == true)
		AudioManager->ResumeAudio();
}



void PauseMode::Reset() {
	if (_audio_paused == true)
		AudioManager->PauseAudio();

	// Save a copy of the current screen to use as the backdrop
	try {
		_screen_capture = VideoManager->CaptureScreen();
	}
	catch(Exception e) {
		PRINT_WARNING << e.ToString() << endl;
	}

	VideoManager->SetCoordSys(0, 1024, 0, 768);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
}



void PauseMode::Update() {
	// Don't eat up 100% of the CPU time while in pause mode
	SDL_Delay(50);

	if (_quit_state == false) {
		if (InputManager->QuitPress() == true) {
			_quit_state = true;
			return;
		}
		else if (InputManager->PausePress() == true) {
			ModeManager->Pop();
			return;
		}
	}

	else {
		if (InputManager->QuitPress() == true) {
			SystemManager->ExitGame();
			return;
		}

		_quit_options.Update();
		if (InputManager->ConfirmPress() == true) {
			switch (_quit_options.GetSelection()) {
				case QUIT_CANCEL:
					ModeManager->Pop();
					break;
				case QUIT_TO_BOOT:
					ModeManager->PopAll();
					ModeManager->Push(new BootMode());
					break;
				case QUIT_GAME:
					SystemManager->ExitGame();
					break;
				default:
					IF_PRINT_WARNING(PAUSE_DEBUG) << "unknown quit option selected: " << _quit_options.GetSelection() << endl;
					break;
			}
			return;
		}

		else if (InputManager->CancelPress() == true) {
			ModeManager->Pop();
			return;
		}

		else if (InputManager->LeftPress() == true) {
			_quit_options.HandleLeftKey();
		}

		else if (InputManager->RightPress() == true) {
			_quit_options.HandleRightKey();
		}
	}
} // void PauseMode::Update()



void PauseMode::Draw() {
	VideoManager->Move(0.0f, 0.0f);
	_screen_capture.Draw(_dim_color);

	if (_quit_state == false) {
		VideoManager->Move(512.0f, 384.0f);
		VideoManager->SetDrawFlags(VIDEO_X_CENTER, 0);
		_paused_text.Draw();
		VideoManager->SetDrawFlags(VIDEO_X_LEFT, 0);
	}

	else {
		_quit_options.Draw();
	}
}

} // namespace hoa_pause
