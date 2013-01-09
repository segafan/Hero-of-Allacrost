///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2013 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    test.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for test mode code
*** **************************************************************************/

#include "test.h"

#include "input.h"
#include "mode_manager.h"
#include "pause.h"

using namespace std;
using namespace hoa_gui;
using namespace hoa_input;
using namespace hoa_mode_manager;
using namespace hoa_pause;

namespace hoa_test {

bool TEST_DEBUG = false;



TestMode::TestMode() :
	GameMode(MODE_MANAGER_TEST_MODE),
	_run_test_immediately(false),
	_test_number(INVALID_TEST),
	_user_focus(SELECTING_CATEGORY),
	_test_list(NULL)
{}



TestMode::TestMode(uint32 test_number) :
	GameMode(MODE_MANAGER_TEST_MODE),
	_run_test_immediately(false),
	_test_number(INVALID_TEST),
	_user_focus(SELECTING_CATEGORY),
	_test_list(NULL)
{}


TestMode::~TestMode() {
	if (_main_script.IsFileOpen() == true)
		_main_script.CloseFile();
}



void TestMode::Reset() {
	_ReloadMainScript();

	if (_run_test_immediately == true) {
		_ExecuteTest();
	}
	_run_test_immediately = false;
}



void TestMode::Update() {
	if (InputManager->QuitPress() == true) {
		ModeManager->Push(new PauseMode(true));
		return;
	}

	if (_user_focus == SELECTING_CATEGORY) {
		// TODO
	}
	else if (_user_focus == SELECTING_TEST) {
		// TODO
	}
}



void TestMode::Draw() {
	_category_window.Draw();
	_test_window.Draw();
	_description_window.Draw();
	_category_list.Draw();
	if (_test_list != NULL)
		_test_list->Draw();
	_description_text.Draw();
}



void TestMode::_ReloadMainScript() {
	if (_main_script.IsFileOpen() == true) {
		_main_script.CloseFile();
	}

	if (_main_script.OpenFile(TEST_MAIN_FILENAME) == false) {
		PRINT_ERROR << "Failed to open main test script file: " << TEST_MAIN_FILENAME << endl;
	}
}



void TestMode::_ExecuteTest() {
	// TODO
}



void TestMode::_UpdateDescription() {
	_description_text.ClearText();

	// TODO: set the text for the selected test or test category
}

} // namespace hoa_test
