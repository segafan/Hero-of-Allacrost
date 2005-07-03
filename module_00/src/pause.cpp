/* 
 * pause.cpp
 *  Code for Hero of Allacrost paused mode
 *  (C) 2005 by Tyler Olsen
 *
 *  This code is licensed under the GNU GPL. It is free software and you may modify it 
 *   and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *   for details.
 */

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
			AudioManager->SetMusicVolume((int)(SettingsManager->music_vol * 0.5));
			AudioManager->SetSoundVolume((int)(SettingsManager->sound_vol * 0.5));
			break;
		// Don't need to do anything for the case ENGINE_SAME_VOLUME
	}
	
	// Here make a VideoManager call to save the current screen.
	
	// Here render the "Paused" text to appear on the screen
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

	
	// Here we'll need to release the saved screen that VideoManager is holding onto.
		
	// Note that we *DON'T* pop the top of the game mode stack, because 
	// GameSettings::KeyEventHandler() does it for us (hence this destructor gets called by it)
}



// Doesn't do a thing. This is all handled in GameSettings::KeyEventHandler()
void PauseMode::Update(Uint32 time_elapsed) { }



// Nothing to draw since the screen never changes in pause mode (maybe we should call SDL_Delay here?)
void PauseMode::Draw() { }

} // namespace hoa_pause
