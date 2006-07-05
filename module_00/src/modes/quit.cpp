///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    quit.cpp
 * \author  Tyler Olsen, roots@allacrost.org
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
#include "input.h"
#include "settings.h"
#include "boot.h"

using namespace std;
using namespace hoa_quit::private_quit;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_settings;
using namespace hoa_boot;
using namespace hoa_utils;



namespace hoa_quit {

bool QUIT_DEBUG = false;

QuitMode::QuitMode() {
	if (QUIT_DEBUG) cout << "QUIT: QuitMode constructor invoked" << endl;
	mode_type = MODE_MANAGER_QUIT_MODE;
	
	switch (SettingsManager->GetPauseVolumeAction()) {
		case SETTINGS_PAUSE_AUDIO:
			AudioManager->PauseAudio();
			break;
		case SETTINGS_ZERO_VOLUME:
			AudioManager->SetMusicVolume(0);
			AudioManager->SetSoundVolume(0);
			break;
		case SETTINGS_HALF_VOLUME:
// 			AudioManager->SetMusicVolume(static_cast<int32>(SettingsManager->music_vol * 0.5));
// 			AudioManager->SetSoundVolume(static_cast<int32>(SettingsManager->sound_vol * 0.5));
			// Note that the music_vol/sound_vol members of SettingsManager aren't changed
			break;
		// Don't need to do anything for case SETTINGS_SAME_VOLUME
	}

	// Save a copy of the current screen to use as a backdrop
	if (!VideoManager->CaptureScreen(_saved_screen)) 
		if (QUIT_DEBUG) cerr << "PAUSE: ERROR: Couldn't save the screen!" << endl;


// !@# Roots: I got rid of this code for now since CreateMenu() is defunct

//	if(!VideoManager->CreateMenu(_quit_menu, 448, 80)) // create a menu
//		cerr << "QUIT: ERROR: Couldn't create menu image!" << endl;


	// Initialize the option box
	_option_box.SetFont("default");
	_option_box.SetCellSize(150.0f, 50.0f);
	_option_box.SetSize(3, 1);
	_option_box.SetPosition(512.0f, 384.0f);
	_option_box.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_option_box.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_option_box.SetSelectMode(VIDEO_SELECT_SINGLE);
	_option_box.SetCursorOffset(-35.0f, -4.0f);
	
	vector<ustring> options;
	options.push_back(MakeWideString("Quit Game"));
	options.push_back(MakeWideString("Quit to Main Menu"));
	options.push_back(MakeWideString("Cancel"));
	
	_option_box.SetOptions(options);
	_option_box.SetSelection(QUIT_CANCEL);
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
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);

}


// Restores volume or unpauses audio, then pops itself from the game stack
void QuitMode::Update() {
	uint32 time_elapsed = SettingsManager->GetUpdateTime();
	
	// Dispatch input to option box
	
	if (InputManager->LeftPress())
		_option_box.HandleLeftKey();
	else if(InputManager->RightPress())
		_option_box.HandleRightKey();
	else if(InputManager->CancelPress())
		_option_box.HandleCancelKey();
	else if(InputManager->ConfirmPress())
		_option_box.HandleConfirmKey();
		
	// See if option box has any events
	
	int32 event = _option_box.GetEvent();
	int32 selection = _option_box.GetSelection();
		
	if(event == VIDEO_OPTION_CONFIRM)
	{
		switch(selection)
		{
			case QUIT_GAME:
				_QuitGame();
				break;
			case QUIT_TO_BOOTMENU:
				_QuitToBootMenu();
				break;			
			case QUIT_CANCEL:
				_Cancel();
				break;
			default:
			{
				if(QUIT_DEBUG)
					cerr << "QUIT ERROR: received confirm event, but option box selection was invalid" << endl;
				break;
			}
		};
	}
	else if(event == VIDEO_OPTION_CANCEL)
	{
		_Cancel();
	}
	
	// Update the option box
	_option_box.Update(time_elapsed);
	
	// Don't consume hoards of CPU time in quit mode
		
	SDL_Delay(50);
}



// Draws the saved screen, the quit prompt, the quit options, and highlights the selected option
void QuitMode::Draw() {
	// Draw the saved screen background
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	Color grayed(0.35f, 0.35f, 0.35f, 1.0f);
	VideoManager->Move(0, 0);
	VideoManager->DrawImage(_saved_screen, grayed);

	// Draw the quit menu
	
	VideoManager->Move(512, 384);
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
//!@#	VideoManager->DrawImage(_quit_menu);	

	_option_box.Draw();
}


// Quit the game completely
void QuitMode::_QuitGame()
{
	SettingsManager->ExitGame();
}


// Quit to the boot menu
void QuitMode::_QuitToBootMenu()
{
	// Restore the game audio, pop QuitMode off the stack, and push BootMode
	switch (SettingsManager->GetPauseVolumeAction()) {
		case SETTINGS_PAUSE_AUDIO:
			AudioManager->ResumeAudio();
			break;
		case SETTINGS_ZERO_VOLUME:
		case SETTINGS_HALF_VOLUME:
// 			AudioManager->SetMusicVolume(SettingsManager->music_vol);
// 			AudioManager->SetSoundVolume(SettingsManager->sound_vol);
			break;
		// We don't need to do anything for case SETTINGS_SAME_VOLUME
	}
	ModeManager->PopAll(); // Remove and free every game mode
	BootMode *BM = new BootMode();
	ModeManager->Push(BM);
}


// Cancel out of quit mode
void QuitMode::_Cancel()
{
	// The user really doesn't want to quit after all, so restore the game audio and state
	switch (SettingsManager->GetPauseVolumeAction()) {
		case SETTINGS_PAUSE_AUDIO:
			AudioManager->ResumeAudio();
			break;
		case SETTINGS_ZERO_VOLUME:
		case SETTINGS_HALF_VOLUME:
// 			AudioManager->SetMusicVolume(SettingsManager->music_vol);
// 			AudioManager->SetSoundVolume(SettingsManager->sound_vol);
			break;
		// Don't need to do anything for case SETTINGS_SAME_VOLUME
	}
	ModeManager->Pop();
}




} // namespace hoa_quit
