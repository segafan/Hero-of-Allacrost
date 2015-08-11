////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2010 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    save_mode.cpp
*** \author  Jacob Rudolph, rujasu@allacrost.org
*** \brief   Source file for save mode interface.
*** ***************************************************************************/

#include <iostream>
#include <sstream>

#include "audio.h"
#include "video.h"
#include "script.h"
#include "input.h"
#include "system.h"
#include "boot.h"
#include "map.h"
#include "mode_manager.h"

#include "save_mode.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_gui;
using namespace hoa_mode_manager;
using namespace hoa_input;
using namespace hoa_system;
using namespace hoa_boot;
using namespace hoa_global;
using namespace hoa_map;
using namespace hoa_script;

namespace hoa_save {

bool SAVE_DEBUG = false;

//! \name Save Options Constants
//@{
const uint8 SAVE_GAME           = 0;
const uint8 SAVE_LOAD_GAME      = 1;
const uint8 SAVE_CANCEL         = 2;
//@}

//! \name SaveMode States
//@{
const uint8 SAVE_MODE_NORMAL    = 0;
const uint8 SAVE_MODE_SAVING    = 1;
const uint8 SAVE_MODE_LOADING   = 2;
const uint8 SAVE_MODE_FADING_OUT= 3;
//@}

SaveMode::SaveMode(bool enable_saving) :
	GameMode(),
	_current_state(SAVE_MODE_NORMAL),
	_dim_color(0.35f, 0.35f, 0.35f, 1.0f), // A grayish opaque color
	_saving_enabled(enable_saving)
{
	mode_type = MODE_MANAGER_SAVE_MODE;

	_window.Create(400.0f, 500.0f);
	_window.SetPosition(312.0f, 630.0f);
	_window.SetDisplayMode(VIDEO_MENU_EXPAND_FROM_CENTER);
	_window.Hide();

	// Initialize the save options box
	_save_options.SetPosition(512.0f, 384.0f);
	_save_options.SetDimensions(250.0f, 200.0f, 1, 3, 1, 3);
	_save_options.SetTextStyle(TextStyle("title22"));

	_save_options.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_save_options.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_save_options.SetSelectMode(VIDEO_SELECT_SINGLE);
	_save_options.SetCursorOffset(-58.0f, 18.0f);

	_save_options.AddOption(UTranslate("Save Game"));
	_save_options.AddOption(UTranslate("Load Game"));
	_save_options.AddOption(UTranslate("Cancel"));
	_save_options.SetSelection(SAVE_CANCEL);

	// Initialize the save options box
	_file_list.SetPosition(512.0f, 384.0f);
	_file_list.SetDimensions(250.0f, 500.0f, 1, 7, 1, 7);
	_file_list.SetTextStyle(TextStyle("title22"));

	_file_list.SetAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);
	_file_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_file_list.SetSelectMode(VIDEO_SELECT_SINGLE);
	_file_list.SetCursorOffset(-58.0f, 18.0f);

	_file_list.AddOption(UTranslate("Cancel"));
	_file_list.AddOption(UTranslate("File #1"));
	_file_list.AddOption(UTranslate("File #2"));
	_file_list.AddOption(UTranslate("File #3"));
	_file_list.AddOption(UTranslate("File #4"));
	_file_list.AddOption(UTranslate("File #5"));
	_file_list.AddOption(UTranslate("File #6"));
	_file_list.SetSelection(0);

	if (_saving_enabled == false) {
		_save_options.EnableOption(SAVE_GAME, false);
	}

	if (_save_music.LoadAudio("mus/Save_Game.ogg") == false) {
		PRINT_ERROR << "failed to load save/load music file: " << endl;
		SystemManager->ExitGame();
		return;
	}

	_window.Show();
}



SaveMode::~SaveMode() {

}



void SaveMode::Reset() {
	// Save a copy of the current screen to use as the backdrop
	try {
		_screen_capture = VideoManager->CaptureScreen();
	}
	catch(Exception e) {
		IF_PRINT_WARNING(SAVE_DEBUG) << e.ToString() << endl;
	}

	VideoManager->SetCoordSys(0.0f, 1023.0f, 0.0f, 767.0f);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);

	_save_music.Play();
}



void SaveMode::Update() {
	if (InputManager->QuitPress() == true) {
		ModeManager->Pop();
		return;
	}

	_save_options.Update();
	_file_list.Update();

	// Screen is in the process of fading out, in order to load a game
	if (_current_state == SAVE_MODE_FADING_OUT)
	{
		// When the screen is finished fading to black, create a new map mode and fade back in
		if (!VideoManager->IsFading()) {
			ModeManager->PopAll();
			try {
				MapMode *MM = new MapMode(MakeStandardString(GlobalManager->GetLocationName()));
				ModeManager->Push(MM);
			} catch (luabind::error e) {
				PRINT_ERROR << "Map::_Load -- Error loading map " << MakeStandardString(GlobalManager->GetLocationName()) << ", returning to BootMode." << endl;
				cerr << "Exception message:" << endl;
				ScriptManager->HandleLuaError(e);
				ModeManager->Push(new BootMode());
			}
			VideoManager->FadeScreen(Color::clear, 1000);
		}
		return;
	}
	// Otherwise, it's time to start handling events.
	else if (InputManager->ConfirmPress() == true) {
		switch (_current_state) {
			case SAVE_MODE_NORMAL:
				if (_save_options.GetSelection() == SAVE_GAME && _saving_enabled) {
					_current_state = SAVE_MODE_SAVING;
				}
				else if (_save_options.GetSelection() == SAVE_LOAD_GAME) {
					_current_state = SAVE_MODE_LOADING;
				}
				else {
					ModeManager->Pop();
				}
				break;

			case SAVE_MODE_SAVING:
				if (_file_list.GetSelection() > 0) {
					// note: using int here, because uint8 will NOT work
					// do not change unless you understand this and can test it properly!
					int id = _file_list.GetSelection();
					ostringstream f;
					f << GetUserDataPath(true) + "saved_game_" << id << ".lua";
					string filename = f.str();
					cout << filename << endl;
					GlobalManager->SaveGame(filename);
				}
				else {
					_current_state = SAVE_MODE_NORMAL;
				}
				break;

			case SAVE_MODE_LOADING:
				if (_file_list.GetSelection() > 0) {
					_LoadGame( _file_list.GetSelection() );
				}
				else {
					_current_state = SAVE_MODE_NORMAL;
				}
				break;
		}
	}

	else if (InputManager->CancelPress() == true) {
		switch (_current_state) {
			case SAVE_MODE_NORMAL:
				ModeManager->Pop();
				return;

			case SAVE_MODE_SAVING: case SAVE_MODE_LOADING:
				_current_state = SAVE_MODE_NORMAL;
				break;
		}
	}

	else if (InputManager->UpPress() == true) {
		switch (_current_state) {
			case SAVE_MODE_NORMAL:
				_save_options.InputUp();
				return;

			case SAVE_MODE_SAVING: case SAVE_MODE_LOADING:
				_file_list.InputUp();
				break;
		}
	}

	else if (InputManager->DownPress() == true) {
		switch (_current_state) {
			case SAVE_MODE_NORMAL:
				_save_options.InputDown();
				return;

			case SAVE_MODE_SAVING: case SAVE_MODE_LOADING:
				_file_list.InputDown();
				break;
		}
	}
} // void SaveMode::Update()



void SaveMode::Draw() {
	// Set the coordinate system for the background and draw
	float width = _screen_capture.GetWidth();
	float height = _screen_capture.GetHeight();
	VideoManager->SetCoordSys(0, width, 0, height);
	VideoManager->Move(0.0f, 0.0f);
	_screen_capture.Draw(_dim_color);

	// Re-set the coordinate system for everything else
	VideoManager->SetCoordSys(0.0f, 1023.0f, 0.0f, 767.0f);

	_window.Draw();

	if (_current_state == SAVE_MODE_NORMAL) {
		_save_options.Draw();
	}
	else {
		_file_list.Draw();
	}
}

bool SaveMode::_LoadGame(int id) {
	ostringstream f;
	f << GetUserDataPath(true) + "saved_game_" << id << ".lua";
	string filename = f.str();

	if (DoesFileExist(filename)) {
		_current_state = SAVE_MODE_FADING_OUT;
		VideoManager->FadeScreen(Color::black, 1000);
		// TODO: stop music, if it's playing

		GlobalManager->LoadGame(filename);

		return true;
	}
	else {
		cout << "BOOT: No saved game file exists, can not load game" << endl;
		cout << filename << endl;
		return false;
	}
}

} // namespace hoa_save
