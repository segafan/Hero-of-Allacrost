/* 
 * quit.cpp
 *  Code for Hero of Allacrost quit mode
 *  (C) 2005 by Tyler Olsen
 *
 *  This code is licensed under the GNU GPL. It is free software and you may modify it 
 *   and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *   for details.
 */

#include <iostream>
#include "quit.h"
#include "audio.h"
#include "video.h"
#include "boot.h"

using namespace std;
using namespace hoa_quit::local_quit;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_engine;
using namespace hoa_boot;


namespace hoa_quit {

bool QUIT_DEBUG = false;

QuitMode::QuitMode() {
	if (QUIT_DEBUG) cout << "QUIT: QuitMode constructor invoked" << endl;
	
	AudioManager = GameAudio::_GetReference();
	VideoManager = GameVideo::_GetReference();
	ModeManager = GameModeManager::_GetReference();
	SettingsManager = GameSettings::_GetReference();
	
	mode_type = ENGINE_QUIT_MODE;
	
	quit_type = QUIT_CANCEL;

	
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
			// Note that the music_vol/sound_vol members of SettingsManager aren't changed
			break;
		// Don't need to do anything for case ENGINE_SAME_VOLUME
	}
	
	// Here we'll make a VideoManager call to save the current screen.
	
	// Here we'll render the "Really Quit?" text to screen along with the quit options
}



// The destructor might possibly have to free any text textures we create...
QuitMode::~QuitMode() { 
	if (QUIT_DEBUG) cout << "QUIT: QuitMode destructor invoked" << endl;
}



// Restores volume or unpauses audio, then pops itself from the game stack 
void QuitMode::Update(Uint32 time_elapsed) {

	// Move the menu selected cursor as appropriate
	if (InputManager->LeftPress()) {
		switch (quit_type) {
			case QUIT_GAME:
				quit_type = QUIT_CANCEL;
				cout << "Cancel" << endl;
				break;
			case QUIT_TO_BOOTMENU:
				quit_type = QUIT_GAME;
				cout << "Quit Game" << endl;
				break;
			case QUIT_CANCEL:
				quit_type = QUIT_TO_BOOTMENU;
				cout << "Quit to Bootmenu" << endl;
				break;
		}
	}
	else if (InputManager->RightPress()) {
		switch (quit_type) {
			case QUIT_GAME:
				quit_type = QUIT_TO_BOOTMENU;
				cout << "Quit to Bootmenu" << endl;
				break;
			case QUIT_TO_BOOTMENU:
				quit_type = QUIT_CANCEL;
				cout << "Cancel" << endl;
				break;
			case QUIT_CANCEL:
				quit_type = QUIT_GAME;
				cout << "Quit Game" << endl;
				break;
		}
	}
	
	// The user really doesn't want to quit after all, so restore the game audio and state
	if (InputManager->CancelPress() || (InputManager->ConfirmPress() && quit_type == QUIT_CANCEL)) {
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
	else if (InputManager->ConfirmPress() && quit_type == QUIT_TO_BOOTMENU) {
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
	else if (InputManager->ConfirmPress() && quit_type) {
		SettingsManager->ExitGame();
	}
}



// Draws the saved screen, the quit prompt, the quit options, and highlights the selected option
void QuitMode::Draw() { 
// 	Draw the saved screen background
// 	Apply a filter to darken it out...?
// 	Draw the quit prompt
// 	Draw the yes and no options
// 	if (quit_selected) { 
// 	  Highlight the yes option
// 	}
// 	else {
// 	  Highlight the no option
// 	}
}

} // namespace hoa_quit
