///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    quit.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 12th, 2005
 * \brief   Source file for quit mode interface.
 *****************************************************************************/

/*
 * quit.cpp
 *  Code for Hero of Allacrost quit mode
 *  (C) 2005 by Tyler Olsen
 *
 *  This code is licensed under the GNU GPL. It is free software and you may modify it
 *   and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *   for details.
 */

#include "utils.h"
#include <iostream>
#include "quit.h"
#include "audio.h"
#include "video.h"
#include "boot.h"

using namespace std;
using namespace hoa_quit::private_quit;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_engine;
using namespace hoa_boot;


namespace hoa_quit {

bool QUIT_DEBUG = false;

QuitMode::QuitMode() {
	if (QUIT_DEBUG) cout << "QUIT: QuitMode constructor invoked" << endl;
	mode_type = ENGINE_QUIT_MODE;
	
	switch (SettingsManager->GetPauseVolumeAction()) {
		case ENGINE_PAUSE_AUDIO:
			AudioManager->PauseAudio();
			break;
		case ENGINE_ZERO_VOLUME:
			AudioManager->SetMusicVolume(0);
			AudioManager->SetSoundVolume(0);
			break;
		case ENGINE_HALF_VOLUME:
			AudioManager->SetMusicVolume(static_cast<int32>(SettingsManager->music_vol * 0.5));
			AudioManager->SetSoundVolume(static_cast<int32>(SettingsManager->sound_vol * 0.5));
			// Note that the music_vol/sound_vol members of SettingsManager aren't changed
			break;
		// Don't need to do anything for case ENGINE_SAME_VOLUME
	}

	// Save a copy of the current screen to use as a backdrop
	if (!VideoManager->CaptureScreen(_saved_screen)) 
		if (QUIT_DEBUG) cerr << "PAUSE: ERROR: Couldn't save the screen!" << endl;
	if(!VideoManager->CreateMenu(_quit_menu, 320, 64)) // create a 256x64 menu
		cerr << "QUIT: ERROR: Couldn't create menu image!" << endl;
}



// The destructor might possibly have to free any text textures we create...
QuitMode::~QuitMode() {
	if (QUIT_DEBUG) cout << "QUIT: QuitMode destructor invoked" << endl;
}


// Called whenever QuitMode is put on top of the stack
void QuitMode::Reset() {
	_quit_type = QUIT_CANCEL;
	
	// Setup video engine constructs.
	VideoManager->SetCoordSys(0, 1024, 0, 768);
	if(!VideoManager->SetFont("default")) 
    cerr << "MAP: ERROR > Couldn't set map font!" << endl;
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);
}


// Restores volume or unpauses audio, then pops itself from the game stack
void QuitMode::Update(uint32 time_elapsed) {

	// Move the menu selected cursor as appropriate
	if (InputManager->LeftPress()) {
		switch (_quit_type) {
			case QUIT_GAME:
				_quit_type = QUIT_CANCEL;
				cout << "Cancel" << endl;
				break;
			case QUIT_TO_BOOTMENU:
				_quit_type = QUIT_GAME;
				cout << "Quit Game" << endl;
				break;
			case QUIT_CANCEL:
				_quit_type = QUIT_TO_BOOTMENU;
				cout << "Quit to Bootmenu" << endl;
				break;
		}
	}
	else if (InputManager->RightPress()) {
		switch (_quit_type) {
			case QUIT_GAME:
				_quit_type = QUIT_TO_BOOTMENU;
				cout << "Quit to Bootmenu" << endl;
				break;
			case QUIT_TO_BOOTMENU:
				_quit_type = QUIT_CANCEL;
				cout << "Cancel" << endl;
				break;
			case QUIT_CANCEL:
				_quit_type = QUIT_GAME;
				cout << "Quit Game" << endl;
				break;
		}
	}

	// The user really doesn't want to quit after all, so restore the game audio and state
	if (InputManager->CancelPress() || (InputManager->ConfirmPress() && _quit_type == QUIT_CANCEL)) {
		switch (SettingsManager->GetPauseVolumeAction()) {
			case ENGINE_PAUSE_AUDIO:
				AudioManager->ResumeAudio();
				break;
			case ENGINE_ZERO_VOLUME:
			case ENGINE_HALF_VOLUME:
				AudioManager->SetMusicVolume(SettingsManager->music_vol);
				AudioManager->SetSoundVolume(SettingsManager->sound_vol);
				break;
			// Don't need to do anything for case ENGINE_SAME_VOLUME
		}
		ModeManager->Pop();
	}

	// Restore the game audio, pop QuitMode off the stack, and push BootMode
	else if (InputManager->ConfirmPress() && _quit_type == QUIT_TO_BOOTMENU) {
		switch (SettingsManager->GetPauseVolumeAction()) {
			case ENGINE_PAUSE_AUDIO:
				AudioManager->ResumeAudio();
				break;
			case ENGINE_ZERO_VOLUME:
			case ENGINE_HALF_VOLUME:
				AudioManager->SetMusicVolume(SettingsManager->music_vol);
				AudioManager->SetSoundVolume(SettingsManager->sound_vol);
				break;
			// We don't need to do anything for case ENGINE_SAME_VOLUME
		}
		ModeManager->PopAll(); // Remove and free every game mode
		BootMode *BM = new BootMode();
		ModeManager->Push(BM);
	}

	// The user has confirmed that they want to quit.
	else if (InputManager->ConfirmPress() && _quit_type == QUIT_GAME) {
		SettingsManager->ExitGame();
	}
}



// Draws the saved screen, the quit prompt, the quit options, and highlights the selected option
void QuitMode::Draw() {
	// Draw the saved screen background
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
	Color grayed(0.35f, 0.35f, 0.35f, 1.0f);
	VideoManager->Move(0, 0);
	VideoManager->DrawImage(_saved_screen, grayed);

	// Draw the quit menu
	VideoManager->Move((1024 - 320)/2, (768 - 32)/2);
	VideoManager->DrawImage(_quit_menu);

	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
	VideoManager->DrawText("Quit Game     Quit to Boot Menu     Cancel", 1024/2, 768/2);
}

} // namespace hoa_quit
