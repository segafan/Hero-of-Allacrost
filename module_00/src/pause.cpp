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
using namespace hoa_global;

namespace hoa_pause {

// Constructor changes audio, saves currently rendered screen, and draws "Paused" text
PauseMode::PauseMode() {
	cerr << "DEBUG: PauseMode constructor invoked" << endl;
	
	mtype = pause_m;
	
	if (SettingsManager->paused_vol_type == GLOBAL_PAUSE_AUDIO_ON_PAUSE) {
		AudioManager->PauseAudio();
		return;
	}
	
	else {
		switch (SettingsManager->paused_vol_type) {
			case GLOBAL_ZERO_VOLUME_ON_PAUSE:
				AudioManager->SetMusicVolume(0);
				AudioManager->SetSoundVolume(0);
				break;
			case GLOBAL_HALF_VOLUME_ON_PAUSE:
				AudioManager->SetMusicVolume((int)(SettingsManager->music_vol * 0.5));
				AudioManager->SetSoundVolume((int)(SettingsManager->sound_vol * 0.5));
				break;
			// We don't need to do anything for case GLOBAL_SAME_VOLUME
		}
	}
	
	// Here we'll make a VideoManager call to save the current screen.
	
	// Here we'll render the "Paused" text to appear on the screen
}



// The destructor might possibly have to free the "Paused" image...
PauseMode::~PauseMode() {
	cerr << "DEBUG: PauseMode destructor invoked" << endl;
	if (SettingsManager->paused_vol_type == GLOBAL_PAUSE_AUDIO_ON_PAUSE) {
		AudioManager->ResumeAudio();
	}
	
	else {
		switch (SettingsManager->paused_vol_type) {
			case GLOBAL_ZERO_VOLUME_ON_PAUSE:
			case GLOBAL_HALF_VOLUME_ON_PAUSE:
				AudioManager->SetMusicVolume(SettingsManager->music_vol);
				AudioManager->SetSoundVolume(SettingsManager->sound_vol);
				break;
			// We don't need to do anything for case GLOBAL_SAME_VOLUME
		}
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
