///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2013 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    test.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for test game mode
*** **************************************************************************/

#ifndef __TEST_HEADER__
#define __TEST_HEADER__

#include "defs.h"
#include "utils.h"

#include "mode_manager.h"
#include "gui.h"

/** \brief Namespace containing code used only for testing purposes
*** \note Normally no other code should need to use this namespace.
**/
namespace hoa_test {

//! \brief Determines whether the code in the hoa_test namespace should print debug statements or not.
extern bool TEST_DEBUG;



namespace private_test {

//! \brief Used to define an invalid test identifier
const uint32 INVALID_TEST = 0;

//! \brief The path and name of the Lua file where the test directory list is stored
const std::string TEST_MAIN_FILENAME = "dat/test/test_main.lua";

/** ****************************************************************************
*** \brief A container class to hold data about a related set of tests
***
*** This container is populated with data read from two Lua files. The main test
*** file contains the test category name, category description, min/max test IDs,
*** and the test filename. The file for the test is then used to read the
*** test ids, test names, and test descriptions.
*** ***************************************************************************/
class TestData {
public:
	//! \brief The name of the category that will displayed in the test menu
	hoa_utils::ustring category_name;

	//! \brief The text that describes the category
	hoa_utils::ustring category_description;

	//! \brief Defines the range of possible test ID numbers
	uint32 minimum_test_id, maximum_test_id;

	//! \brief The name of the Lua file that contains the code to execute all of the tests
	std::string test_filename;

	//! \brief Holds all of the IDs for the test in question
	std::vector<uint32> test_ids;

	//! \brief The names of all tests contained within this category
	std::vector<hoa_utils::ustring> test_names;

	//! \brief The descriptions for all tests contained within this category
	std::vector<hoa_utils::ustring> test_descriptions;
};

} // namespace private_test


/** ****************************************************************************
*** \brief A game mode used for debugging and testing purposes
***
*** This is a game mode that players will not encounter during the game. The mode manages
*** a simple GUI interface that lists all of the available tests that can be run and allows
*** the user to select from among those tests. The available tests are defined in TEST_MAIN_FILENAME.
*** 
*** The way to activate test mode is either through running the program executable with the -t/--test option,
*** or through the Ctrl+T meta key when the game is in BootMode. When starting TestMode via the command-line,
*** the user may optionally include a test ID number to immediately begin running a specific test. Whenever an
*** instance of TestMode exists on the game stack, the Ctrl+T command will clear the game stack of any other modes
*** and return TestMode to be the active game mode. Ctrl+T will otherwise be ignored if the active game mode is
*** not BootMode or no TestMode instance is found on the game stack.
*** 
*** Once in TestMode, the GUI will display three windows. The vertical window on the left side lists all of the
*** test categories. The vertical window on the right side lists all of the available tests for the selected category.
*** And the horizontal window on the bottom of the screen is used to display information text about the selected
*** category or test.
*** ***************************************************************************/
class TestMode : public hoa_mode_manager::GameMode {
public:
	TestMode();

	/** \brief Creates a TestMode instance and immediately begins the specified test
	*** \param test_number The id number of the test to begin executing
	*** \note If the test_number is invalid, a warning will be printed and TestMode will run as normal
	**/
	TestMode(uint32 test_number);

	~TestMode();

	//! \brief Resets appropriate class members. Called whenever TestMode is made the active game mode
	void Reset();

	//! \brief Updates the GUI objects and processes user input
	void Update();

	//! \brief Draws the GUI objects to the screen
	void Draw();

private:
	//! \brief Defines the places where the user input may be focused
	typedef enum {
		SELECTING_CATEGORY,
		SELECTING_TEST
	} UserFocus;

	//! \brief When true, the test defined by _test_number will be executed immediately when the game mode becomes active
	bool _run_test_immediately;
	
	//! \brief The number of the test to execute
	uint32 _test_number;

	//! \brief Where the user focus is currently at, used to update the mode state appropriately
	UserFocus _user_focus;

	//! \brief Contains all of the data that will be displayed in the TestMode GUI. Each element represents one category of test data
	std::vector<private_test::TestData> _test_data;

	// ---------- GUI Objects

	//! \brief Used to display information in the test window when a test category contains no tests
	hoa_video::TextImage _no_tests_message;

	//! \brief Vertical window on the left side of the screen. Used to display the _category_list OptionBox
	hoa_gui::MenuWindow _category_window;

	//! \brief Vertical window on the right side of the screen. Used to display the _test_list OptionBox
	hoa_gui::MenuWindow _test_window;

	//! \brief Horizontal window on the bottom of the screen. Used to display the _description_text TextBox
	hoa_gui::MenuWindow _description_window;

	//! \brief The list of selectabled quit options presented to the user while the mode is in the quit state
	hoa_gui::OptionBox _category_list;

	//! \brief The lists of available tests for each test category
	std::vector<hoa_gui::OptionBox*> _all_test_lists;

	//! \brief A pointer to the list of all tests for the selected category in _all_test_lists
	hoa_gui::OptionBox* _test_list;

	//! \brief Holds the descriptive text of the highlighted test category or test
	hoa_gui::TextBox _description_text;

	// ---------- Methods

	//! \brief Defines the static properties of the various GUI objects
	void _Initialize();
	
	//! \brief Clears out and reloads all test data
	void _ReloadTestData();

	/** \brief Checks each test ID and test ID range for any potential problems
	***
	*** This is called at the end of _ReloadTestData as a means to test the integrity of that data. The function checks for two things.
	*** First, it ensures that the test ID ranges for each test category do not overlap. Second, it checks to see that each defined test
	*** ID lies within the valid range of it's category. If either of these checks fail, warning messages will be printed to the console
	*** (if TEST_DEBUG is enabled), but no corrective action will take place.
	**/
	void _CheckForInvalidTestID();

	/** \brief Runs the Lua function to execute the test that is identified by _test_number
	*** \note This may result in a new game mode being added to the stack, making TestMode no longer active
	**/
	void _ExecuteTest();

	//! \brief Clears and updates the description text to reflect the currently selected test or test category
	void _SetDescriptionText();
}; // class PauseMode : public hoa_mode_manager::GameMode

} // namespace hoa_test

#endif // __TEST_HEADER__