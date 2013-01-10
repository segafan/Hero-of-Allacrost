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

#include "audio.h"
#include "input.h"
#include "mode_manager.h"
#include "script.h"
#include "video.h"
#include "pause.h"

using namespace std;
using namespace hoa_utils;

using namespace hoa_audio;
using namespace hoa_input;
using namespace hoa_mode_manager;
using namespace hoa_script;
using namespace hoa_video;

using namespace hoa_gui;
using namespace hoa_pause;
using namespace hoa_test::private_test;

namespace hoa_test {

bool TEST_DEBUG = false;



TestMode::TestMode() :
	GameMode(MODE_MANAGER_TEST_MODE),
	_run_test_immediately(false),
	_test_number(INVALID_TEST),
	_user_focus(SELECTING_CATEGORY),
	_test_list(NULL)
{
	_Initialize();
}



TestMode::TestMode(uint32 test_number) :
	GameMode(MODE_MANAGER_TEST_MODE),
	_run_test_immediately(false),
	_test_number(INVALID_TEST),
	_user_focus(SELECTING_CATEGORY),
	_test_list(NULL)
{
	_Initialize();
}



TestMode::~TestMode() {
	for (uint32 i = 0; i < _all_test_lists.size(); i++) {
		if (_all_test_lists[i] != NULL)
			delete _all_test_lists[i];
	}
	_all_test_lists.clear();
	_test_list = NULL;

	_category_window.Destroy();
	_test_window.Destroy();
	_description_window.Destroy();
}



void TestMode::Reset() {
	VideoManager->SetStandardCoordSys();
	VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);

	if (_test_data.empty() == true)
		_ReloadTestData();

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

	_category_list.Update();
	if (_test_list != NULL)
		_test_list->Update();

	if (_user_focus == SELECTING_CATEGORY) {
		// Note: We do not allow the user focus to change to SELECTING_TEST if the selected category has
		// no tests defined
		if (InputManager->ConfirmPress()) {
			if (_test_list != NULL) {
				_category_list.InputConfirm();
				_user_focus = SELECTING_TEST;
				_SetDescriptionText();
			}
			else {
				// TODO: Play an "invalid command" sound
			}
		}
		else if (InputManager->UpPress()) {
			_category_list.InputUp();
			_SetDescriptionText();
		}
		else if (InputManager->DownPress()) {
			_category_list.InputDown();
			_SetDescriptionText();
		}
	}
	else if (_user_focus == SELECTING_TEST) {
		if (InputManager->ConfirmPress()) {
			_test_list->InputConfirm();
			_ExecuteTest();
		}
		if (InputManager->CancelPress()) {
			_test_list->InputCancel();
			_user_focus = SELECTING_CATEGORY;
			_SetDescriptionText();
		}
		else if (InputManager->UpPress()) {
			_test_list->InputUp();
			_SetDescriptionText();
		}
		else if (InputManager->DownPress()) {
			_test_list->InputDown();
			_SetDescriptionText();
		}
	}
}



void TestMode::Draw() {
	_category_window.Draw();
	_test_window.Draw();
	_description_window.Draw();

	_category_list.Draw();
	if (_test_list != NULL) {
		_test_list->Draw();
	}
	else {
		VideoManager->PushState();
		VideoManager->SetDrawFlags(VIDEO_X_CENTER, VIDEO_Y_CENTER, 0);
		// Move the draw cursor to the middle of the test window
		VideoManager->Move(612.0f, 300.0f);
		_no_tests_message.Draw();
		VideoManager->PopState();
	}
	_description_text.Draw();
}



void TestMode::_Initialize() {
	_no_tests_message.SetStyle(TextStyle("text22"));
	_no_tests_message.SetText("No tests are defined for this test category.");
	
	_category_window.Create(400.0f, 600.0f);
	_category_window.SetPosition(0.0f, 0.0f);
	_category_window.SetDisplayMode(VIDEO_MENU_INSTANT);
	_category_window.Show();

	_test_window.Create(624.0f, 600.0f);
	_test_window.SetPosition(400.0f, 0.0f);
	_test_window.SetDisplayMode(VIDEO_MENU_INSTANT);
	_test_window.Show();

	_description_window.Create(1024.0f, 168.0f);
	_description_window.SetPosition(0.0f, 600.0f);
	_description_window.SetDisplayMode(VIDEO_MENU_INSTANT);
	_description_window.Show();

	_category_list.SetOwner(&_category_window);
	_category_list.SetPosition(50.0f, 20.0f);
	_category_list.SetDimensions(360.0f, 560.0f, 1, 60, 1, 12);
	_category_list.SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_category_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
	_category_list.SetTextStyle(TextStyle("title22"));
	_category_list.SetCursorOffset(-50.0f, -20.0f);
	_category_list.SetSelectMode(VIDEO_SELECT_SINGLE);
	_category_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);

	_description_text.SetOwner(&_description_window);
	_description_text.SetPosition(20.0f, 20.0f);
	_description_text.SetDimensions(980.0f, 100.0f);
	_description_text.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
	_description_text.SetTextStyle(TextStyle("text20"));
	_description_text.SetDisplayMode(VIDEO_TEXT_INSTANT);
}



void TestMode::_ReloadTestData() {
	// TODO: this function needs some additional error checking to make sure that all of the data
	// being read from the script files is present and accurate
	
	// A handle to the main test file that contains the information for each test category
	ReadScriptDescriptor main_script;
	// A handle to each test file for each category
	ReadScriptDescriptor test_script;

	_test_data.clear();

	// ----- (1): Open the main test file and retrieve the list of test categories
	if (main_script.IsFileOpen() == true) {
		main_script.CloseFile();
	}

	if (main_script.OpenFile(TEST_MAIN_FILENAME) == false) {
		PRINT_ERROR << "Failed to open main test script file: " << TEST_MAIN_FILENAME << endl;
		return;
	}

	vector<string> category_ids;
	main_script.OpenTablespace();
	main_script.ReadStringVector("categories", category_ids);

	for (uint32 i = 0; i < category_ids.size(); i++) {
		// ----- (2): Retrieve the list of properties for each test category
		_test_data.push_back(TestData());
		TestData& new_data = _test_data.back();

		// Read in the test category data first
		main_script.OpenTable(category_ids[i]);
		new_data.category_name = MakeUnicodeString(main_script.ReadString("name"));
		new_data.category_description = MakeUnicodeString(main_script.ReadString("description"));
		new_data.minimum_test_id = main_script.ReadUInt("min_id");
		new_data.maximum_test_id = main_script.ReadUInt("max_id");
		new_data.test_filename = main_script.ReadString("file");
		main_script.CloseTable();

		// ----- (3): Open the test file for the current category
		if (test_script.OpenFile(new_data.test_filename) == false) {
			IF_PRINT_WARNING(TEST_DEBUG) << "failed to open test file for test category: " << category_ids[i]
				<< ". The test category data will not be added to the GUI interface." << endl;
			_test_data.pop_back();
			continue;
		}
		test_script.OpenTablespace();
		test_script.OpenTable("tests");

		// ----- (4): Read the table keys (which are the test IDs) followed by the name and description of each test
		test_script.ReadTableKeys(new_data.test_ids);
		if (new_data.test_ids.empty() == true) {
			IF_PRINT_WARNING(TEST_DEBUG) << "no tests were defined for test category: " << category_ids[i] << endl;
		}
		for (uint32 j = 0; j < new_data.test_ids.size(); j++) {
			test_script.OpenTable(new_data.test_ids[j]);
			new_data.test_names.push_back(MakeUnicodeString(test_script.ReadString("name")));
			new_data.test_descriptions.push_back(MakeUnicodeString(test_script.ReadString("description")));
			test_script.CloseTable();
		}

		test_script.CloseFile();
	}

	main_script.CloseFile();

	// Remove any old test information that was loaded in the option boxes
	_category_list.ClearOptions();
	for (uint32 i = 0; i < _all_test_lists.size(); i++) {
		if (_all_test_lists[i] != NULL)
			delete _all_test_lists[i];
	}
	_all_test_lists.clear();
	_test_list = NULL;

	// ----- (5): Repopulation the category list OptionBox using the newly retrieved data
	for (uint32 i = 0; i < _test_data.size(); i++) {
		_category_list.AddOption(_test_data[i].category_name);

		// ----- (6): Create a new OptionBox and populate it with the test names for each test category
		// Add a NULL entry in the _all_test_lists container if the category had no tests defined
		if (_test_data[i].test_ids.empty() == true) {
			_all_test_lists.push_back(NULL);
			continue;
		}

		OptionBox* new_list = new OptionBox();
		new_list->SetOwner(&_test_window);
		new_list->SetPosition(50.0f, 20.0f);
		new_list->SetDimensions(580.0f, 560.0f, 1, 60, 1, 12);
		new_list->SetAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
		new_list->SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);
		new_list->SetTextStyle(TextStyle("text22"));
		new_list->SetCursorOffset(-50.0f, -20.0f);
		new_list->SetSelectMode(VIDEO_SELECT_SINGLE);
		new_list->SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);

		for (uint32 j = 0; j < _test_data[i].test_names.size(); j++) {
			new_list->AddOption(_test_data[i].test_names[j]);
		}

		_all_test_lists.push_back(new_list);
	}

	// Update the selected test category and test list to point to the first element
	_category_list.SetSelection(0);
	_test_list = _all_test_lists[0];
	_user_focus = SELECTING_CATEGORY;
	_SetDescriptionText();

	_CheckForInvalidTestID();
} // void TestMode::_ReloadTestData()



void TestMode::_CheckForInvalidTestID() {
	// Because all this method does is print out debug statements if it finds a problem, there's nothing useful for the method
	// to do if the test debug flag is disabled.
	if (TEST_DEBUG == false) {
		return;
	}

	// TODO: this method needs to be implemented
}



void TestMode::_ExecuteTest() {
	uint32 category = _category_list.GetSelection();
	uint32 test_id = _test_data[category].test_ids[_test_list->GetSelection()];
	ReadScriptDescriptor test_file;

	if (test_file.OpenFile(_test_data[category].test_filename) == false) {
		IF_PRINT_WARNING(TEST_DEBUG) << "failed to execute test because the test file could not be opened for reading: "
			<< test_file.GetFilename() << endl;
		return;
	}

	test_file.OpenTablespace();
	test_file.OpenTable("tests");
	test_file.OpenTable(test_id);

	ScriptObject exec_test = test_file.ReadFunctionPointer("ExecuteTest");
	try {
		ScriptCallFunction<void>(exec_test);
	}
	catch(luabind::error e) {
		IF_PRINT_WARNING(TEST_DEBUG) << "failed to execute test function in script file: " << test_file.GetFilename()
			<< " for test number: " << test_id << endl;
		ScriptManager->HandleLuaError(e);
	}

	test_file.CloseTable();
	test_file.CloseTable();
	test_file.CloseFile();
}



void TestMode::_SetDescriptionText() {
	_description_text.ClearText();

	if (_user_focus == SELECTING_CATEGORY) {
		_description_text.SetDisplayText(_test_data[_category_list.GetSelection()].category_description);
	}
	else if (_user_focus == SELECTING_TEST) {
		// Note that the user is not allowed to enter this focus unless there is a valid test list for the active category
		_description_text.SetDisplayText(_test_data[_category_list.GetSelection()].test_descriptions[_test_list->GetSelection()]);
	}
	else {
		_description_text.SetDisplayText("WARNING: could not set description text because the user focus was neither selecting a category nor a test.");
	}
}

} // namespace hoa_test
