/* 
 * hoa_boot.cpp
 *	Code for Hero of Allacrost boot menu
 *	(C) 2004 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */

#include <iostream>
#include "boot.h"
#include "audio.h"
#include "video.h"
#include "data.h"
#include "map.h"


using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_engine;
using namespace hoa_data;
using namespace hoa_map;



namespace hoa_boot {

// ****************************************************************************
// *************************** GENERAL FUNCTIONS ******************************
// ****************************************************************************

// The constructor initializes variables and sets up the path names of the boot images
BootMode::BootMode() {
	cerr << "DEBUG: BootMode's constructor invoked." << endl;
	
	mtype = boot_m;
	
	vmenu_index.push_back(LOAD_MENU);
	menu_hidden = false;
	
	DataManager->LoadBootData(&boot_images, &boot_sound, &boot_music);
	
	VideoManager->SetCoordSys(0, 1024, 0, 768, 1); // TEMPORARY
	
 	for (int i = 0; i < boot_images.size(); i++) {
 		VideoManager->LoadImage(boot_images[i]);
 	}
	for (int i = 0; i < boot_music.size(); i++) {
		AudioManager->LoadMusic(boot_music[i]);
	}
	for (int i = 0; i < boot_sound.size(); i++) {
		AudioManager->LoadSound(boot_sound[i]);
	}
	
	AudioManager->PlayMusic(boot_music[0], AUDIO_NO_FADE, AUDIO_LOOP_FOREVER);
}



// The destructor frees all used music, sounds, and images.
BootMode::~BootMode() {
	cerr << "DEBUG: BootMode's destructor invoked." << endl;
	
	for (int i = 0; i < boot_music.size(); i++)
		AudioManager->FreeMusic(boot_music[i]);
	for (int i = 0; i < boot_sound.size(); i++)
		AudioManager->FreeSound(boot_sound[i]);
	for (int i = 0; i < boot_images.size(); i++)
		VideoManager->DeleteImage(boot_images[i]);
}





// Animates the logo when the boot mode starts up. Should not be called before LoadBootImages.
void BootMode::AnimateLogo() {
	cerr << "TEMP: Do nothing" << endl;
	// Write a series of image moves/rotations to animate the logo here.
}




// Redefines the change_key reference. Waits indefinitely for user to press any key.
void BootMode::RedefineKey(SDLKey& change_key) {
	SDL_Event event;
	
// 	KeyState *key_set = &(SettingsManager->InputStatus.key); // A shortcut
	
// 	while (SDL_WaitEvent(&event)) { // Waits indefinitely for events to occur.
// 		switch (event.type) {
// 			case SDL_KEYDOWN:
// 				// The following series of if statements ensure mutual exclusion of the key map
// 				if (key_set->up == event.key.keysym.sym) {
// 					key_set->up = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				if (key_set->down == event.key.keysym.sym) {
// 					key_set->down = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				if (key_set->left == event.key.keysym.sym) {
// 					key_set->left = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				if (key_set->right == event.key.keysym.sym) {
// 					key_set->right = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				if (key_set->confirm == event.key.keysym.sym) {
// 					key_set->confirm = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				if (key_set->cancel == event.key.keysym.sym) {
// 					key_set->cancel = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				if (key_set->menu == event.key.keysym.sym) {
// 					key_set->menu = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				if (key_set->pause == event.key.keysym.sym) {
// 					key_set->pause = change_key;
// 					change_key = event.key.keysym.sym;
// 					return;
// 				}
// 				change_key = event.key.keysym.sym;
// 				return;
// 		}
// 	}
}


// ****************************************************************************
// **************************** UPDATE FUNCTIONS ******************************
// ****************************************************************************


// This is called once every frame iteration to update the status of the game
void BootMode::Update(Uint32 time_elapsed) {
	// If our menu is hidden, we don't do anything until a user event occurs.
	if (menu_hidden) {
		if (InputManager->ConfirmPress() || InputManager->CancelPress() || InputManager->MenuPress() || 
				InputManager->SwapPress() || InputManager->LSelectPress() || InputManager->RSelectPress() ||
				InputManager->UpPress() || InputManager->DownPress() || InputManager->LeftPress() ||
				InputManager->RightPress())
			menu_hidden = false;
		return;
	}
	
	switch (vmenu_index[0]) {
		case NEW_MENU: 
			UpdateNewMenu();
			break;
		case LOAD_MENU: 
			UpdateLoadMenu();
			break;
		case OPTIONS_MENU:
			UpdateOptionsMenu();
			break;
		case CREDITS_MENU:
			UpdateCreditsMenu();
			break;
		case HIDE_MENU:
			UpdateHideMenu();
			break;
		case QUIT_MENU:
			UpdateQuitMenu();
			break;
	}
}



// Handles events when NEW_MENU is selected. 
void BootMode::UpdateNewMenu() {
	// Left and right flags are mutually exclusive. We ignore right if both are pressed.
	if (InputManager->LeftPress()) {
		vmenu_index[0] = QUIT_MENU;
		cout << "QUIT MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (InputManager->RightPress()) {
		vmenu_index[0] = LOAD_MENU;
		cout << "LOAD MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	
	if (InputManager->ConfirmPress()) {
		AudioManager->PlaySound(boot_sound[2], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // game loading sound
		cout << "*Starting new game!" << endl;
		// Remove the boot mode from the top of the stack
		
		MapMode *MM = new MapMode(-1);
		ModeManager->Pop();
		ModeManager->Push(MM);
	}
}



// Handles events when LOAD_MENU is selected. Two levels of sub-menus here.
void BootMode::UpdateLoadMenu() {
	// Handle main menu events when all submenus are closed
	if (vmenu_index.size() == 1) {
		// Left and right flags are mutually exclusive. We ignore right if both are pressed.
		if (InputManager->LeftPress()) {
			vmenu_index[0] = NEW_MENU;
			cout << "NEW MENU" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		else if (InputManager->RightPress()) {
			vmenu_index[0] = OPTIONS_MENU;
			cout << "OPTIONS MENU" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		
		if (InputManager->ConfirmPress()) {
			vmenu_index.push_back(0);
			cout << "*Entering Saved Game Selection Screen" << endl;
			// Load the list of saved games
			AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
			// Load a vector of the saved games here
			BattleMode *BM = new BattleMode();
			ModeManager->Pop();
			ModeManager->Push(BM);
		}
		
		return;
	} // if (vmenu_index.size() == 1)
	
	
	// Handle submenu level 1 events. This is the "saved game selection screen" submenu
	if (vmenu_index.size() == 2) {
		if (InputManager->CancelPress()) {
			AudioManager->PlaySound(boot_sound[1], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // cancel sound
			vmenu_index.pop_back();
			cout << "*Exiting Saved game selection screen" << endl;
			return;
		}
		if (InputManager->ConfirmPress()) {
			vmenu_index.push_back(0);
			// Load the specific saved game data screen
			cout << "*Entering saved game confirmation screen" << endl;
			AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
			// Call a config routine to load details about the selected saved game
			return;
		}
		
		// Change selected game. Up and down key presses mutality exclusive. Up has higher priority. 
		if (InputManager->UpPress()) {
			if (vmenu_index[1] != 0) 
				vmenu_index[1] = vmenu_index[1] - 1;
			else
				vmenu_index[1] = 5; // 5 = number of saved games (TEMP)
			cout << "*Saved game " << vmenu_index[1] << " selected." << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		if (InputManager->DownPress()) {
			if (vmenu_index[1] != 5) // 5 = num_saved_games (TEMP)
				vmenu_index[1] = vmenu_index[1] + 1;
			else
				vmenu_index[1] = 0; 
			cout << "*Saved game " << vmenu_index[1] << " selected." << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		}
		
		return;
	} // if (vmenu_index.size() == 1)
	
	
	// Handle submenu level 2 events. This is the "saved game confirmation screen" submenu
	if (vmenu_index.size() == 3) {
		if (InputManager->CancelPress()) {
			AudioManager->PlaySound(boot_sound[1], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // cancel sound
			vmenu_index.pop_back();
			cout << "*Exiting saved game confirmation screen" << endl;
		}
		else if (InputManager->ConfirmPress()) {
			cout << "*Loading saved game!" << endl;
			AudioManager->PlaySound(boot_sound[2], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // game loading sound
			// Load the game
		}
	} // if (vmenu_index.size() == 2)
	
}


// Updates when in options menu. Calls other update handling function. Two levels of submenus
void BootMode::UpdateOptionsMenu() {
	// Handle events when we are in level 0 menu
	if (vmenu_index.size() == 1) {
		if (InputManager->LeftPress()) {
			vmenu_index[0] = LOAD_MENU;
			cout << "LOAD MENU" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		else if (InputManager->RightPress()) {
			vmenu_index[0] = CREDITS_MENU;
			cout << "CREDITS_MENU" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		
		if (InputManager->ConfirmPress()) {
			vmenu_index.push_back(0); // Left-Right main options menu
			vmenu_index.push_back(0); // Up-Down specific options menu
			cout << "*Entering options menu!" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		}
		return;
	}
	
	// Level 1 submenu
	if (vmenu_index.size() == 3) {
		if (InputManager->CancelPress()) {
			vmenu_index.pop_back();
			vmenu_index.pop_back();
			cout << "*Exiting options menu" << endl;
			AudioManager->PlaySound(boot_sound[1], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // cancel sound
			return;
		}
		
		switch (vmenu_index[1]) {
			case VIDEO_OP:
				UpdateVideoOptions();
				break;
			case AUDIO_OP:
				UpdateAudioOptions();
				break;
			case LANGUAGE_OP:
				UpdateLanguageOptions();
				break;
			case KEYS_OP:
				UpdateKeyOptions();
				break;
			case JOYSTICK_OP:
				UpdateJoystickOptions();
				break;
		}
	}
}



// Handles updates when the credits menu is selected. There is one sub-menu level.
void BootMode::UpdateCreditsMenu() {
	// Handle events when we are in level 0 menu
	if (vmenu_index.size() == 1) {
		// Left and right flags are mutually exclusive. We ignore right if both are pressed.
		if (InputManager->LeftPress()) {
			vmenu_index[0] = OPTIONS_MENU;
			cout << "OPTIONS MENU" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		else if (InputManager->RightPress()) {
			cout << "HIDE_MENU" << endl;
			vmenu_index[0] = HIDE_MENU;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		
		if (InputManager->ConfirmPress()) {
			vmenu_index.push_back(0);
			cout << "*Viewing credits now!" << endl;
			// Load in the credits text.
			AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
		}
		return;
	}
	
	// Level 1 submenu
	if (InputManager->ConfirmPress() || InputManager->CancelPress()) {
		vmenu_index.pop_back();
		cout << "*Exiting credits" << endl;
	}
	// else() { We update the credits animation position? }
}



// Handles updates when hide menu is selected. No sub-menu levels here.
void BootMode::UpdateHideMenu() {
	if (InputManager->LeftPress()) {
		vmenu_index[0] = CREDITS_MENU;
		cout << "CREDITS_MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (InputManager->RightPress()) {
		vmenu_index[0] = QUIT_MENU;
		cout << "QUIT_MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound	 
	}
	else if (InputManager->ConfirmPress()) {
		menu_hidden = true;
	}
}



// Handles updates when quit menu is selected. No sub-menu levels here
void BootMode::UpdateQuitMenu() {
	if (InputManager->LeftPress()) {
		vmenu_index[0] = HIDE_MENU;
		cout << "HIDE_MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (InputManager->RightPress()) {
		vmenu_index[0] = NEW_MENU;
		cout << "NEW_MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound	 
	}
	else if (InputManager->ConfirmPress()) {
		SettingsManager->ExitGame();
	}
}



// Handle updates when video options menu is selected. One sub-menu level.
void BootMode::UpdateVideoOptions() {
	if (InputManager->LeftPress()) {
		vmenu_index[1] = JOYSTICK_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: JOYSTICK MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	else if (InputManager->RightPress()) {
		vmenu_index[1] = AUDIO_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: AUDIO MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	
	if (InputManager->UpPress()) {
		if (vmenu_index[2] != 0)
			vmenu_index[2] = vmenu_index[2] - 1;
		else
			vmenu_index[2] = 4;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (InputManager->DownPress()) {
		if (vmenu_index[2] != 4)
			vmenu_index[2] = vmenu_index[2] + 1;
		else
			vmenu_index[2] = 0;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	
	if (InputManager->ConfirmPress()) {
		switch (vmenu_index[2]) {
			case 0:
				// Toggle fullscreen
				break;
			case 1:
				// Change video resolution
				break;
			case 2:
				// Change other video setting
				break;
			case 3:
				// Change other video setting
				break;
			case 4:
				// Change other video setting
				break;
		}
	}
}



// Update code for audio options menu. Uses two sub-menu levels.
void BootMode::UpdateAudioOptions() {
	if (vmenu_index.size() == 3) {
		if (InputManager->LeftPress()) {
			vmenu_index[1] = VIDEO_OP;
			vmenu_index[2] = 0;
			cout << "OPTIONS: VIDEO MENU" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		else if (InputManager->RightPress()) {
			vmenu_index[1] = LANGUAGE_OP;
			vmenu_index[2] = 0;
			cout << "OPTIONS: LANGUAGE MENU" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		
		if (InputManager->UpPress()) {
			if (vmenu_index[2] != 0)
				vmenu_index[2] = vmenu_index[2] - 1;
			else
				vmenu_index[2] = 1;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		}
		else if (InputManager->DownPress()) {
			if (vmenu_index[2] != 1)
				vmenu_index[2] = vmenu_index[2] + 1;
			else
				vmenu_index[2] = 0;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		}
		
		if (InputManager->ConfirmPress()) {
			vmenu_index.push_back(0);
		}
		return;
	}
	
	// Changing volume levels
	if (InputManager->CancelPress()) {
		vmenu_index.pop_back();
		AudioManager->PlaySound(boot_sound[1], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // cancel sound
		return;
	}
	
	if (vmenu_index[2] == 0) { // Change music volume
		if (InputManager->LeftState()) { // Decrease music volume

		}
		if (InputManager->RightState()) { // Increase music volume
		
		}
	}
	else if (vmenu_index[2] == 1) { // Change sound volume
		if (InputManager->LeftState()) { // Decrease music volume
			
		}
		if (InputManager->RightState()) { // Increase music volume
		
		}
	}
}



// Handles language options menu updates. One sub-menu level.
void BootMode::UpdateLanguageOptions() {
	if (InputManager->LeftPress()) {
		vmenu_index[1] = AUDIO_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: AUDIO MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	else if (InputManager->RightPress()) {
		vmenu_index[1] = KEYS_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: KEY SETTINGS MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	
	if (InputManager->UpPress()) {
		if (vmenu_index[2] != 0)
			vmenu_index[2] = vmenu_index[2] - 1;
		else
			vmenu_index[2] = 2;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (InputManager->DownPress()) {
		if (vmenu_index[2] != 2)
			vmenu_index[2] = vmenu_index[2] + 1;
		else
			vmenu_index[2] = 0;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	
	if (InputManager->ConfirmPress()) {
		switch (vmenu_index[2]) {
			case 0:
				// Change language to English
				break;
			case 1:
				// Change language to German
				break;
			case 2:
				// Change language to Spanish
				break;
		}
	}
}



// Handle updates when key settings options menu is selected. One sub-menu.
void BootMode::UpdateKeyOptions() {
	if (InputManager->LeftPress()) {
		vmenu_index[1] = LANGUAGE_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: LANGUAGE MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	else if (InputManager->RightPress()) {
		vmenu_index[1] = JOYSTICK_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: JOYSTICK MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	
	if (InputManager->UpPress()) {
		if (vmenu_index[2] != 0)
			vmenu_index[2] = vmenu_index[2] - 1;
		else
			vmenu_index[2] = 7;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (InputManager->DownPress()) {
		if (vmenu_index[2] != 7)
			vmenu_index[2] = vmenu_index[2] + 1;
		else
			vmenu_index[2] = 0;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	
	if (InputManager->ConfirmPress()) {
		switch (vmenu_index[2]) {
			case 0: // Change up key
				//RedefineKey(InputManager->Key.up&);
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 1: // Change down key
				//RedefineKey(&(InputManager->Key.down));
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 2: // Change left key
				//RedefineKey(&(InputManager->Key.left));
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 3: // Change right key
				//RedefineKey(&(InputManager->Key.right));
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 4: // Change confirm key
				//RedefineKey(&(InputManager->Key.confirm));
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 5: // Change cancel key
				//RedefineKey(&(InputManager->Key.cancel));
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 6: // Change menu key
				//RedefineKey(&(InputManager->Key.menu));
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 7: // Change pause key
				//RedefineKey(&(InputManager->Key.pause));
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
		}
	}
}



// Handle updates when joystick options menu is selected. One sub-menu.
void BootMode::UpdateJoystickOptions() {
	if (InputManager->LeftPress()) {
		vmenu_index[1] = KEYS_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: KEY SETTINGS MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	else if (InputManager->RightPress()) {
		vmenu_index[1] = VIDEO_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: VIDEO MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	
// This function is incomplete for now. It will be very much like the UpdateKeyOptions when finished.
}


// ****************************************************************************
// ***************************** DRAW FUNCTIONS *******************************
// ****************************************************************************


// Draws our next frame to the video back buffer
void BootMode::Draw() {
	// Draw the backdrop image
	VideoManager->SelectLayer(1);
	
	//VideoManager->Move(0, 0);
	//VideoManager->Move(-1024/2, 0);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_NO_BLEND, 0);
	VideoManager->DrawImage(boot_images[0]);
	
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_TOP, VIDEO_BLEND, 0);
	VideoManager->Move(1024/2, 50);
	VideoManager->DrawImage(boot_images[2]);
	
	VideoManager->Move(1024/2, 575);
	VideoManager->DrawImage(boot_images[1]);
	
	if (menu_hidden) { // Then we are already done!
		return;
	}
	
	// Draw logo at top center of screen
		
	// Draw plain main menu at bottom center.
		
	// Draw text in menus depending on language
	return;
}


// Draws a window displaying summary info of all the saved games on the system
void BootMode::DrawLoadMenu() {
	cout << "TEMP: DrawLoadMenu() invoked" << endl;
}


// Draws the menu screen for the selected game and displays a confirmation dialogue
void BootMode::DrawLoadGame() {
	cout << "TEMP: DrawLoadGame() invoked." << endl;
}

// Draws the video options menu.
void BootMode::DrawVideoOptions() {
	cout << "TEMP: DrawVideoOptions() invoked." << endl;
	// Draw a full screen checkbox and the various video modes available
}

// Draws the audio options menu.
void BootMode::DrawAudioOptions() {
	cout << "TEMP: DrawAudioOptions() invoked." << endl;
	// Draw a music and sound volume bar thing
}

// Draws the language options menu.
void BootMode::DrawLanguageOptions() {
	cout << "TEMP: DrawLanguageOptions() invoked." << endl;
	// Draw a list of all the languages available.
}

// Draws the key options menu.
void BootMode::DrawKeyOptions() {
	cout << "TEMP: DrawKeyOptions() invoked." << endl;
	// Draw a list of all the function->key mappings. I'll need strings for all the keysyms...
}

// Draws the joystick options menu.
void BootMode::DrawJoystickOptions() {
	cout << "TEMP: DrawJoystickOptions() invoked." << endl;
	// This will be implemented later.
}

// Draws the credits menu.
void BootMode::DrawCredits() {
	cout << "TEMP: DrawCredits() invoked." << endl;
	// Draw a text box with our names and junk. Later will be made into a specify animation.
}


}
