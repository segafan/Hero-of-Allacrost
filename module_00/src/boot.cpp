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

using namespace std;
using namespace hoa_global;
using namespace hoa_video;
using namespace hoa_audio;
using namespace hoa_utils;
using namespace hoa_map;



namespace hoa_boot {

// ********************************* GENERAL FUNCTIONS ********************************



// The constructor initializes variables and sets up the path names of the boot images
BootMode::BootMode() {
	cerr << "DEBUG: BootMode's constructor invoked." << endl;
	mtype = boot_m;
	input = &(SettingsManager->InputStatus);
	
	vmenu_index.push_back(LOAD_MENU);
	menu_hidden = false;
	
	DataManager->LoadBootData(&boot_images, &boot_sound, &boot_music);
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
	
	KeyState *key_set = &(SettingsManager->InputStatus.key); // A shortcut
	
	while (SDL_WaitEvent(&event)) { // Waits indefinitely for events to occur.
		switch (event.type) {
			case SDL_KEYDOWN:
				// The following series of if statements ensure mutual exclusion of the key map
				if (key_set->up == event.key.keysym.sym) {
					key_set->up = change_key;
					change_key = event.key.keysym.sym;
					return;
				}
				if (key_set->down == event.key.keysym.sym) {
					key_set->down = change_key;
					change_key = event.key.keysym.sym;
					return;
				}
				if (key_set->left == event.key.keysym.sym) {
					key_set->left = change_key;
					change_key = event.key.keysym.sym;
					return;
				}
				if (key_set->right == event.key.keysym.sym) {
					key_set->right = change_key;
					change_key = event.key.keysym.sym;
					return;
				}
				if (key_set->confirm == event.key.keysym.sym) {
					key_set->confirm = change_key;
					change_key = event.key.keysym.sym;
					return;
				}
				if (key_set->cancel == event.key.keysym.sym) {
					key_set->cancel = change_key;
					change_key = event.key.keysym.sym;
					return;
				}
				if (key_set->menu == event.key.keysym.sym) {
					key_set->menu = change_key;
					change_key = event.key.keysym.sym;
					return;
				}
				if (key_set->pause == event.key.keysym.sym) {
					key_set->pause = change_key;
					change_key = event.key.keysym.sym;
					return;
				}
				change_key = event.key.keysym.sym;
				return;
		}
	}
}



// ********************************* UPDATE FUNCTIONS ********************************



// This is called once every frame iteration to update the status of the game
void BootMode::Update(Uint32 time_elapsed) {
	// If our menu is hidden, we don't do anything until a user event occurs.
	if (menu_hidden) {
		if (input->confirm_press || input->cancel_press || input->menu_press ||
				input->pause_press	 || input->up_press		 || input->down_press ||
				input->left_press		|| input->right_press)
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
	if (input->left_press) {
		vmenu_index[0] = QUIT_MENU;
		cout << "QUIT MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (input->right_press) {
		vmenu_index[0] = LOAD_MENU;
		cout << "LOAD MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	
	if (input->confirm_press) {
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
		if (input->left_press) {
			vmenu_index[0] = NEW_MENU;
			cout << "NEW MENU" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		else if (input->right_press) {
			vmenu_index[0] = OPTIONS_MENU;
			cout << "OPTIONS MENU" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		
		if (input->confirm_press) {
			vmenu_index.push_back(0);
			cout << "*Entering Saved Game Selection Screen" << endl;
			// Load the list of saved games
			AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
			// Load a vector of the saved games here
		}
		
		return;
	} // if (vmenu_index.size() == 1)
	
	
	// Handle submenu level 1 events. This is the "saved game selection screen" submenu
	if (vmenu_index.size() == 2) {
		if (input->cancel_press) {
			AudioManager->PlaySound(boot_sound[1], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // cancel sound
			vmenu_index.pop_back();
			cout << "*Exiting Saved game selection screen" << endl;
			return;
		}
		if (input->confirm_press) {
			vmenu_index.push_back(0);
			// Load the specific saved game data screen
			cout << "*Entering saved game confirmation screen" << endl;
			AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
			// Call a config routine to load details about the selected saved game
			return;
		}
		
		// Change selected game. Up and down key presses mutality exclusive. Up has higher priority. 
		if (input->up_press) {
			if (vmenu_index[1] != 0) 
				vmenu_index[1] = vmenu_index[1] - 1;
			else
				vmenu_index[1] = 5; // 5 = number of saved games (TEMP)
			cout << "*Saved game " << vmenu_index[1] << " selected." << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		if (input->down_press) {
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
		if (input->cancel_press) {
			AudioManager->PlaySound(boot_sound[1], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // cancel sound
			vmenu_index.pop_back();
			cout << "*Exiting saved game confirmation screen" << endl;
		}
		else if (input->confirm_press) {
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
		if (input->left_press) {
			vmenu_index[0] = LOAD_MENU;
			cout << "LOAD MENU" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		else if (input->right_press) {
			vmenu_index[0] = CREDITS_MENU;
			cout << "CREDITS_MENU" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		
		if (input->confirm_press) {
			vmenu_index.push_back(0); // Left-Right main options menu
			vmenu_index.push_back(0); // Up-Down specific options menu
			cout << "*Entering options menu!" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		}
		return;
	}
	
	// Level 1 submenu
	if (vmenu_index.size() == 3) {
		if (input->cancel_press) {
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
		if (input->left_press) {
			vmenu_index[0] = OPTIONS_MENU;
			cout << "OPTIONS MENU" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		else if (input->right_press) {
			cout << "HIDE_MENU" << endl;
			vmenu_index[0] = HIDE_MENU;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		
		if (input->confirm_press) {
			vmenu_index.push_back(0);
			cout << "*Viewing credits now!" << endl;
			// Load in the credits text.
			AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
		}
		return;
	}
	
	// Level 1 submenu
	if (input->confirm_press || input->cancel_press) {
		vmenu_index.pop_back();
		cout << "*Exiting credits" << endl;
	}
	// else() { We update the credits animation position? }
}



// Handles updates when hide menu is selected. No sub-menu levels here.
void BootMode::UpdateHideMenu() {
	if (input->left_press) {
		vmenu_index[0] = CREDITS_MENU;
		cout << "CREDITS_MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (input->right_press) {
		vmenu_index[0] = QUIT_MENU;
		cout << "QUIT_MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound	 
	}
	else if (input->confirm_press) {
		menu_hidden = true;
	}
}



// Handles updates when quit menu is selected. No sub-menu levels here
void BootMode::UpdateQuitMenu() {
	if (input->left_press) {
		vmenu_index[0] = HIDE_MENU;
		cout << "HIDE_MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (input->right_press) {
		vmenu_index[0] = NEW_MENU;
		cout << "NEW_MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound	 
	}
	else if (input->confirm_press) {
		SettingsManager->not_done = false;
	}
}



// Handle updates when video options menu is selected. One sub-menu level.
void BootMode::UpdateVideoOptions() {
	if (input->left_press) {
		vmenu_index[1] = JOYSTICK_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: JOYSTICK MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	else if (input->right_press) {
		vmenu_index[1] = AUDIO_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: AUDIO MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	
	if (input->up_press) {
		if (vmenu_index[2] != 0)
			vmenu_index[2] = vmenu_index[2] - 1;
		else
			vmenu_index[2] = 4;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (input->down_press) {
		if (vmenu_index[2] != 4)
			vmenu_index[2] = vmenu_index[2] + 1;
		else
			vmenu_index[2] = 0;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	
	if (input->confirm_press) {
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
		if (input->left_press) {
			vmenu_index[1] = VIDEO_OP;
			vmenu_index[2] = 0;
			cout << "OPTIONS: VIDEO MENU" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		else if (input->right_press) {
			vmenu_index[1] = LANGUAGE_OP;
			vmenu_index[2] = 0;
			cout << "OPTIONS: LANGUAGE MENU" << endl;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
			return;
		}
		
		if (input->up_press) {
			if (vmenu_index[2] != 0)
				vmenu_index[2] = vmenu_index[2] - 1;
			else
				vmenu_index[2] = 1;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		}
		else if (input->down_press) {
			if (vmenu_index[2] != 1)
				vmenu_index[2] = vmenu_index[2] + 1;
			else
				vmenu_index[2] = 0;
			AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		}
		
		if (input->confirm_press) {
			vmenu_index.push_back(0);
		}
		return;
	}
	
	// Changing volume levels
	if (input->cancel_press) {
		vmenu_index.pop_back();
		AudioManager->PlaySound(boot_sound[1], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // cancel sound
		return;
	}
	
	if (vmenu_index[2] == 0) { // Change music volume
		if (input->left_state) { // Decrease music volume

		}
		if (input->right_state) { // Increase music volume
		
		}
	}
	else if (vmenu_index[2] == 1) { // Change sound volume
		if (input->left_state) { // Decrease music volume
			
		}
		if (input->right_state) { // Increase music volume
		
		}
	}
}



// Handles language options menu updates. One sub-menu level.
void BootMode::UpdateLanguageOptions() {
	if (input->left_press) {
		vmenu_index[1] = AUDIO_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: AUDIO MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	else if (input->right_press) {
		vmenu_index[1] = KEYS_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: KEY SETTINGS MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	
	if (input->up_press) {
		if (vmenu_index[2] != 0)
			vmenu_index[2] = vmenu_index[2] - 1;
		else
			vmenu_index[2] = 2;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (input->down_press) {
		if (vmenu_index[2] != 2)
			vmenu_index[2] = vmenu_index[2] + 1;
		else
			vmenu_index[2] = 0;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	
	if (input->confirm_press) {
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
	if (input->left_press) {
		vmenu_index[1] = LANGUAGE_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: LANGUAGE MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	else if (input->right_press) {
		vmenu_index[1] = JOYSTICK_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: JOYSTICK MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	
	if (input->up_press) {
		if (vmenu_index[2] != 0)
			vmenu_index[2] = vmenu_index[2] - 1;
		else
			vmenu_index[2] = 7;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	else if (input->down_press) {
		if (vmenu_index[2] != 7)
			vmenu_index[2] = vmenu_index[2] + 1;
		else
			vmenu_index[2] = 0;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
	}
	
	if (input->confirm_press) {
		switch (vmenu_index[2]) {
			case 0: // Change up key
				//RedefineKey(input->key.up&);
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 1: // Change down key
				//RedefineKey(&(input->key.down));
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 2: // Change left key
				//RedefineKey(&(input->key.left));
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 3: // Change right key
				//RedefineKey(&(input->key.right));
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 4: // Change confirm key
				//RedefineKey(&(input->key.confirm));
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 5: // Change cancel key
				//RedefineKey(&(input->key.cancel));
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 6: // Change menu key
				//RedefineKey(&(input->key.menu));
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
			case 7: // Change pause key
				//RedefineKey(&(input->key.pause));
				AudioManager->PlaySound(boot_sound[0], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // confirm sound
				break;
		}
	}
}



// Handle updates when joystick options menu is selected. One sub-menu.
void BootMode::UpdateJoystickOptions() {
	if (input->left_press) {
		vmenu_index[1] = KEYS_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: KEY SETTINGS MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	else if (input->right_press) {
		vmenu_index[1] = VIDEO_OP;
		vmenu_index[2] = 0;
		cout << "OPTIONS: VIDEO MENU" << endl;
		AudioManager->PlaySound(boot_sound[3], AUDIO_NO_FADE, AUDIO_LOOP_ONCE); // move sound
		return;
	}
	
// This function is incomplete for now. It will be very much like the UpdateKeyOptions when finished.
}



// ********************************* DRAW FUNCTIONS ********************************



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
