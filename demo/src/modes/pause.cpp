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

#include "utils.h"
#include <iostream>
#include "pause.h"
#include "audio.h"
#include "video.h"
#include "input.h"
#include "system.h"

using namespace std;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_system;

namespace hoa_pause {

bool PAUSE_DEBUG = false;

// Constructor changes audio, saves currently rendered screen, and draws "Paused" text
PauseMode::PauseMode() {
	if (PAUSE_DEBUG) cout << "PAUSE: PauseMode constructor invoked" << endl;

	mode_type = MODE_MANAGER_PAUSE_MODE;
	// Adjust the volume while in paused mode acordingly
// 	switch (SystemManager->GetPauseVolumeAction()) {
// 		case SETTINGS_PAUSE_AUDIO:
// 			AudioManager->PauseAudio();
// 			break;
// 		case SETTINGS_ZERO_VOLUME:
// 			AudioManager->SetMusicVolume(0);
// 			AudioManager->SetSoundVolume(0);
// 			break;
// 		case SETTINGS_HALF_VOLUME:
// // 			AudioManager->SetMusicVolume(static_cast<int32>(SystemManager->music_vol * 0.5));
// // 			AudioManager->SetSoundVolume(static_cast<int32>(SystemManager->sound_vol * 0.5));
// 			break;
// 		// Don't need to do anything for the case SETTINGS_SAME_VOLUME
// 	}

	// Save a copy of the current screen to use as a backdrop
	if (!VideoManager->CaptureScreen(_saved_screen)) 
		if (PAUSE_DEBUG) cerr << "PAUSE: ERROR: Couldn't save the screen!" << endl;
}



// The destructor might possibly have to free the "Paused" image...
PauseMode::~PauseMode() {
	if (PAUSE_DEBUG) cout << "PAUSE: PauseMode destructor invoked" << endl;

// 	// Restore the game audio/volume levels appropriately
// 	switch (SystemManager->GetPauseVolumeAction()) {
// 		case SETTINGS_PAUSE_AUDIO:
// 			AudioManager->ResumeAudio();
// 			break;
// 		case SETTINGS_ZERO_VOLUME:
// 		case SETTINGS_HALF_VOLUME:
// // 			AudioManager->SetMusicVolume(SystemManager->music_vol);
// // 			AudioManager->SetSoundVolume(SystemManager->sound_vol);
// 			break;
// 		default: // Don't need to do anything for case SETTINGS_SAME_VOLUME
// 			break;
// 		
// 	}

	// Release the saved screen frame.
	VideoManager->DeleteImage(_saved_screen);
}


// Called whenever PauseMode is put on top of the stack
void PauseMode::Reset() {
	// Setup video engine constructs.
	VideoManager->SetCoordSys(0, 1024, 0, 768);
	if(!VideoManager->SetFont("default")) 
    cerr << "MAP: ERROR > Couldn't set map font!" << endl;
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
}


// The true logic is all handled in GameSystem::KeyEventHandler() instead of this function.
void PauseMode::Update() { 
	// Don't eat up 100% of the CPU time when we're in pause mode
	SDL_Delay(50);
}



// Nothing to draw since the screen never changes in pause mode (maybe we should call SDL_Delay here?)
void PauseMode::Draw() {
	// Draw the background image
	// For that, set the system coordinates to the size of the window (same with the save-screen)
	int32 width = VideoManager->GetScreenWidth();
	int32 height = VideoManager->GetScreenHeight();
	VideoManager->SetCoordSys(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

	Color grayed(0.35f, 0.35f, 0.35f, 1.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, 0);
	VideoManager->Move(0,0);
	VideoManager->DrawImage(_saved_screen, grayed);
	
	// Render the "Paused" text to appear on the center of the screen
	// Restore the Coordinate system (that one is quit mode coodinate system)
	VideoManager->SetCoordSys (0.0f, 1024.0f, 0.0f, 768.0f);

	VideoManager->SetDrawFlags(VIDEO_X_CENTER, 0);
	VideoManager->Move(512, 384);
	VideoManager->DrawText("Paused");
}

} // namespace hoa_pause
