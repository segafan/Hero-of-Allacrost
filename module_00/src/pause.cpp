///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    pause.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 12th, 2005
 * \brief   Source file for pause mode interface.
 *****************************************************************************/

#include "utils.h"
#include <iostream>
#include "pause.h"
#include "audio.h"
#include "video.h"

using namespace std;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_engine;

namespace hoa_pause {

bool PAUSE_DEBUG = false;

// Constructor changes audio, saves currently rendered screen, and draws "Paused" text
PauseMode::PauseMode() {
	if (PAUSE_DEBUG) cout << "PAUSE: PauseMode constructor invoked" << endl;

	mode_type = ENGINE_PAUSE_MODE;
	unsigned char volume_action = SettingsManager->GetPauseVolumeAction();

	// Adjust the volume while in paused mode acordingly
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
			break;
		// Don't need to do anything for the case ENGINE_SAME_VOLUME
	}

	// Here make a VideoManager call to save the current screen.
	if (!VideoManager->CaptureScreen(_saved_screen)) {
		if (PAUSE_DEBUG) cerr << "PAUSE: ERROR: Couldn't save the screen!" << endl;
	}
	VideoManager->SetCoordSys(0, 1024, 0, 768);
	VideoManager->Move(0,0);
}



// The destructor might possibly have to free the "Paused" image...
PauseMode::~PauseMode() {
	if (PAUSE_DEBUG) cout << "PAUSE: PauseMode destructor invoked" << endl;

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


	// Release the saved screen frame.
	VideoManager->DeleteImage(_saved_screen);
	
	// TEMPORARY: Because the Video Manager doesn't save the stat of the coordinate system
	//VideoManager->SetCoordSys(-14, 14, -10, 10);
	
	
	// Note that we *DON'T* pop the top of the game mode stack, because
	// GameSettings::KeyEventHandler() does it for us (hence this destructor gets called by it)
}



// Doesn't do a thing. This is all handled in GameSettings::KeyEventHandler()
void PauseMode::Update(uint32 time_elapsed) { }



// Nothing to draw since the screen never changes in pause mode (maybe we should call SDL_Delay here?)
void PauseMode::Draw() {

	// Draw the background image
	VideoManager->DrawImage(_saved_screen);
	// Render the "Paused" text to appear on the center of the screen
	VideoManager->DrawText("Paused", 512, 384);
}

} // namespace hoa_pause
