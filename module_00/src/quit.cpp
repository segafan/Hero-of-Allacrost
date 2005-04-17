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

using namespace std;
using namespace hoa_global;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_boot;
using namespace hoa_quit::local_quit;

namespace hoa_quit {

QuitMode::QuitMode() {
	cerr << "DEBUG: QuitMode constructor called" << endl;
	
	AudioManager = GameAudio::_GetReference();
	VideoManager = GameVideo::_GetReference();
	ModeManager = GameModeManager::_GetReference();
	SettingsManager = GameSettings::_GetReference();
	
	mtype = quit_m;
	input = &(SettingsManager->InputStatus);
	
	quit_selected = QUIT_CANCEL;

	
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
	
	// Here we'll render the "Really Quit?" text to screen along with Yes - No options
}



// The destructor might possibly have to free any text textures we create...
QuitMode::~QuitMode() { 
	cerr << "DEBUG: QuitMode destructor called" << endl;
}



// Restores volume or unpauses audio, then pops itself from the game stack 
void QuitMode::Update(Uint32 time_elapsed) {

	// Move the menu selected cursor as appropriate
	if (input->left_press) {
		switch (quit_selected) {
			case QUIT_GAME:
				quit_selected = QUIT_CANCEL;
				cout << "Cancel" << endl;
				break;
			case QUIT_TO_BOOTMENU:
				quit_selected = QUIT_GAME;
				cout << "Quit Game" << endl;
				break;
			case QUIT_CANCEL:
				quit_selected = QUIT_TO_BOOTMENU;
				cout << "Quit to Bootmenu" << endl;
				break;
		}
	}
	else if (input->right_press) {
		switch (quit_selected) {
			case QUIT_GAME:
				quit_selected = QUIT_TO_BOOTMENU;
				cout << "Quit to Bootmenu" << endl;
				break;
			case QUIT_TO_BOOTMENU:
				quit_selected = QUIT_CANCEL;
				cout << "Cancel" << endl;
				break;
			case QUIT_CANCEL:
				quit_selected = QUIT_GAME;
				cout << "Quit Game" << endl;
				break;
		}
	}
	
	// The user really doesn't want to quit after all, so restore the game audio and state
	if (input->cancel_press || (input->confirm_press && quit_selected == QUIT_CANCEL)) {
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
		
		ModeManager->Pop();
	}
	
	// Restore the game audio, pop QuitMode off the stack, and push BootMode
	else if (input->confirm_press && quit_selected == QUIT_TO_BOOTMENU) {
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
		
		ModeManager->Pop();
		BootMode *BM = new BootMode();
		ModeManager->Push(BM);
	}
	
	// The user has confirmed that they want to quit.
	else if (input->confirm_press && quit_selected) {
		SettingsManager->not_done = false;
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
