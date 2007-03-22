///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    option.h
*** \author  Raj Sharma, roos@allacrost.org
*** \brief   Header file for OptionBox GUI control and supporting classes
***
*** OptionBox is a type of GUI control that allows you to create several
*** option choices, which the player can select from by using the arrow keys.
*** ***************************************************************************/

#ifndef __OPTION_HEADER__
#define __OPTION_HEADER__

#include "defs.h"
#include "utils.h"
#include "gui.h"

//! \brief All calls to the video engine are wrapped in this namespace.
namespace hoa_video {

//! \brief The number of milliseconds that the menu cursor blinks when in the blinking state
const int32 VIDEO_CURSOR_BLINK_RATE = 40;

//! \brief The number of milliseconds it takes to scroll when the cursor goes past the end of an option box
const int32 VIDEO_OPTION_SCROLL_TIME = 100;


//! \brief These are the types of events that an option box can generate
enum OptionBoxEvent {
	VIDEO_OPTION_INVALID          = -1,
	//! The selected option changed
	VIDEO_OPTION_SELECTION_CHANGE =  0,
	//! The player confirmed a selection
	VIDEO_OPTION_CONFIRM          =  1,
	//! The player pressed the cancel key
	VIDEO_OPTION_CANCEL           =  2,
	//! Two options were switched by the player
	VIDEO_OPTION_SWITCH           =  3,
	//! The player tried to exceed the top most option
	VIDEO_OPTION_BOUNDS_UP        =  4,
	//! The player tried to exceed the bottom most option
	VIDEO_OPTION_BOUNDS_DOWN      =  5,
	//! The player tried to exceed the left most option
	VIDEO_OPTION_BOUNDS_LEFT      =  6,
	//! The player tried to exceed the right most option
	VIDEO_OPTION_BOUNDS_RIGHT     =  7,
	VIDEO_OPTION_TOTAL            =  8
};


//! \brief Type identifiers for options, whether the option is text, an image, or an align flag
enum OptionElementType {
	VIDEO_OPTION_ELEMENT_INVALID      = -1,
	//! Identifies mark-up for left alignment
	VIDEO_OPTION_ELEMENT_LEFT_ALIGN   =  0,
	//! Identifies mark-up for center alignment
	VIDEO_OPTION_ELEMENT_CENTER_ALIGN =  1,
	//! Identifies mark-up for right alignment
	VIDEO_OPTION_ELEMENT_RIGHT_ALIGN  =  2,
	//! Identifies the position tag
	VIDEO_OPTION_ELEMENT_POSITION     =  3,
	//! Represents option images
	VIDEO_OPTION_ELEMENT_IMAGE        =  4,
	//! Represents option text
	VIDEO_OPTION_ELEMENT_TEXT         =  5,
	VIDEO_OPTION_ELEMENT_TOTAL        =  6
};


//! \brief For representing the visual state of the menu cursor
enum CursorState {
	VIDEO_CURSOR_STATE_INVALID  = -1,
	//! Hides the cursor so it is not drawn on the screen
	VIDEO_CURSOR_STATE_HIDDEN   =  0,
	//! Shows the cursor next to the selected option
	VIDEO_CURSOR_STATE_VISIBLE  =  1,
	//! Causes the cursor to continually blink
	VIDEO_CURSOR_STATE_BLINKING =  2,
	VIDEO_CURSOR_STATE_TOTAL    =  3
};


//! \brief Modes to control how the cursor wraps around when the cursor exceeds the list boundary
enum WrapMode {
	VIDEO_WRAP_MODE_INVALID  = -1,
	//! Cursor retains its position on the list boundary
	VIDEO_WRAP_MODE_NONE     =  0,
	//! Cursor wraps around left to right, top to bottom, when exceeding the boundary
	VIDEO_WRAP_MODE_STRAIGHT =  1,
	//! Similar to straight, but the cursor will move one row or column when it exceeds a column or row boundary
	VIDEO_WRAP_MODE_SHIFTED  =  2,
	VIDEO_WRAP_MODE_TOTAL    =  3
};


//! \brief These select modes control how confirming works when you choose options
enum SelectMode
{
	VIDEO_SELECT_INVALID = -1,
	//! Options only require a single confirmation
	VIDEO_SELECT_SINGLE  =  0,
	//! The first confirmation highlights the item, and the second confirms it.
	//! \note If you press confirm on one item and confirm again on a different item, the two items get switched.
	VIDEO_SELECT_DOUBLE  =  1,
	VIDEO_SELECT_TOTAL   =  2
};

namespace private_video {

/** ****************************************************************************
*** \brief A class which encapsulates the various contents of an option.
***
*** Contents can include text, images, mark-up tags, etc.
*** ***************************************************************************/
class OptionElement {
public:
	//! \brief A type indentifier for determining what this option represents
	OptionElementType type;

	//! \brief A simple integer value used for various purposes such as offsets
	int32 value;
};


/** ****************************************************************************
*** \brief Holds the bound coordinates for a particular "cell" in an option box.
***
*** This is used for calculations when drawing an option box
*** ***************************************************************************/
class OptionCellBounds {
public:

	//! \brief The y coordinate for the top, bottom, and center of the cell
	float y_top, y_center, y_bottom;

	//! \brief The x coordinate for the left, right, and center of the cell
	float x_left, x_center, x_right;
};


/** ****************************************************************************
*** \brief Represents one particular option in a list and all its elements
***
*** For example in a shop menu, one option might be "Mythril Knife" and contain
*** an icon of a knife, the text, "Mythril Knife", a right alignment flag, and
*** finally the text "500 drunes".
*** ***************************************************************************/
class Option {
public:
	//! \brief The elements that this option is composed of
	std::vector<OptionElement> elements;

	//! \brief Contains all pieces of text for this option
	std::vector<hoa_utils::ustring> text;

	//! \brief Contains all images used for this option
	//! \todo Allow for animated images as well
	std::vector<StillImage> images;

	//! \brief A flag to specify whether this option is disabled or not
	bool disabled;
};

} // namespace private_video

/** ****************************************************************************
*** \brief Represents rows and columns of options that the player may select
***
*** The OptionBox control is used for presenting the player with several choices,
*** of actions to take, wares to buy, etc. The class handles cursor movement
*** ***************************************************************************/
class OptionBox : public private_video::GUIControl {
public:
	OptionBox();

	~OptionBox();

	/** \brief Updates the state of the option box
	*** \param frame_time The number of milliseconds elapsed this frame
	**/
	void Update(uint32 frame_time);

	//! \brief Draws each enabled option to the screen
	void Draw();


	/** \brief Enables or disables the ability to switch the location of two options
	*** \param enable True enables switching, false disables it
	*** \todo What is the default for switching?
	**/
	void EnableSwitching(bool enable);

	//! \brief Processes the input commands for moving the cursor, selecting options, etcetra
	//@{
	void HandleUpKey();
	void HandleDownKey();
	void HandleLeftKey();
	void HandleRightKey();
	void HandleConfirmKey();
	void HandleCancelKey();
	//@}

	/** \brief Adds a new option to the OptionBox
	*** \param text for the new option
	*** \return returns false on failure (if the given text is illegal)
	**/
	bool AddOption(const hoa_utils::ustring &text);

	/** \brief enables/disables the option with the given index
	*** \param index the option to enable/disable
	*** \param enable true to enable, false to disable.  Options are enabled by default.
	**/
	bool EnableOption(int32 index, bool enable);

	//! \name Member Access Functions
	//@{
	/** \brief Sets the font that the option box will use for text
	***  \param font_name A label to a valid, already-loaded font
	**/
	void SetFont(const std::string &font_name);

	/** \brief Sets the width and height of all option cells
	*** \param horz_spacing The amount of horizontal space allocated for each cell
	*** \param vert_spacing The amount of vertical space allocated for each cell
	**/
	void SetCellSize(float horz_spacing, float vert_spacing);

	/** \brief Sets the size of the option box in terms of number of columns and rows
	*** \param columns The number of columns to allocate for the option box
	*** \param rows The number of rows to allocate for the option box
	**/
	void SetSize(int32 columns, int32 rows);

	/** \brief sets the alignment of the option text
	*** \param xalign left/right alignment of text in the cell
	*** \param yalign top/bottom alignment of text in the cell
	**/
	void SetOptionAlignment(int32 xalign, int32 yalign);

	/** \brief Sets the option selection mode (single or double confirm)
	*** \param mode The selection mode to set
	**/
	void SetSelectMode(SelectMode mode);

	/** \brief Sets the behavior for vertical wrapping of the cursor
	*** \param mode The vertical wrap behavior to set
	**/
	void SetVerticalWrapMode(WrapMode mode);

	/** \brief Sets the behavior for horizontal wrapping of the cursor
	*** \param mode The horizontal wrap behavior to set
	**/
	void SetHorizontalWrapMode(WrapMode mode);

	/** \brief Sets the state of the cursor icon
	*** \param state The cursor state to set
	**/
	void SetCursorState(CursorState state);

	/** \brief Sets the cursor offset relative to the text position
	*** \param x Horizontal offset (sign determines direction)
	*** \param y Vertical offset (sign dteremines direction)
	*** \return success/failure
	**/
	void SetCursorOffset(float x, float y);

	/** \brief Sets the currently selected option (0 to # of options - 1)
	*** \param index The desired selection index in the list of options
	**/
	void SetSelection(int32 index);

	/** \brief Sets the options to display in this option box
	*** \param format_text A vector of unicode strings which contain the text for each item, along with any formatting tags
	*** \return success/failure
	***
	*** For example: "<img/weapons/mythril.png>Mythril knife<r>500 drunes"
	**/
	bool SetOptions(const std::vector<hoa_utils::ustring>& format_text);

	/** \brief Changes the value of a particular option
	*** \param index The index of the option to change
	*** \param text The text to change the option to
	*** \return False on failure
	**/
	bool SetOptionText(int32 index, const hoa_utils::ustring &text);

	/** \brief Checks if the option box is in the process of scrolling
	*** \return True if the option box is scrolling, false if it is not
	**/
	bool IsScrolling() const;

	/** \brief returns true if the given option is enabled
	*** \param index of the option to check
	*** \return true if option is enabled, false if it's not
	**/
	bool IsEnabled(int32 index) const;

	/** \brief returns an integer which contains the code of an event that occurred, or
	 *         zero if no event occurred. This should be called every frame to see if
	 *         anything new happened, like the player confirming or canceling, etc. Do
	 *         not call it more than once per frame though, because it clears the event
	 *         flag.
	 * \return int representing an option box event (i.e. cancel, confirm, left, right, etc.)
	 */
	int32 GetEvent();

	/** \brief Returns the index of the currently selected option
	*** \return The current selection index
	**/
	int32 GetSelection() const;

	/** \brief Returns the index of the previously confirmed option when switching two options
	*** \return The index of the previously confirmed option
	**/
	int32 GetSwitchSelection() const;

	/** \brief returns the number of rows
	*** \return number of rows in option box
	**/
	int32 GetNumRows() const;

	/** \brief returns the number of columns
	*** \return number of columns in option box
	**/
	int32 GetNumColumns() const;

	/** \brief Retreives the number of options in the option box
	*** \return The number of options contained by the option box
	**/
	int32 GetNumOptions() const;
	//@}

	/** \brief Used to determine whether the option box is initialized and ready for use
	*** \param errors Used to report the list of reasons why the option box is not initialized
	*** \return True if the option box is initialized, false if it is not
	**/
	bool IsInitialized(std::string& errors);

private:
	//! after every change to any of the settings, check if the textbox is in a valid state and update this bool
	bool   _initialized;

	//! font used for the options
	std::string _font;

	//! cursor offset
	float _cursorX, _cursorY;

	//! switch cursor offset (relative to the normal cursor offset)
	float _switchCursorX, _switchCursorY;

	//! horizontal spacing
	float _hSpacing;

	//! vertical spacing
	float _vSpacing;

	//! number of columns
	int32 _numColumns;

	//! numer of rows
	int32 _numRows;

	//! horizontal alignment for text
	int32 _option_xalign;

	//! vertical alignment for text
	int32 _option_yalign;

	//! when Update() is called, blink is set to true on frames that cursor should blink (i.e. not be visible)
	bool _blink;

	//! timer used for controlling blink effect
	int32 _blinkTime;

	//! timer used for controlling scrolling effect
	int32 _scrollTime;

	//! offset we're scrolling from
	int32 _scrollStartOffset;

	//! offset we're scrolling to
	int32 _scrollEndOffset;

	//! 1 for down, -1 for up
	int32 _scrollDirection;

	//! current scroll offset
	int32 _scrollOffset;

	//! selection mode
	SelectMode _selectMode;

	//! allow switching
	bool _switching;

	//! current cursor state (blinking, visible, hidden, etc)
	CursorState _cursorState;

	//! horizontal wrapping mode
	WrapMode   _hWrapMode;

	//! vertical wrapping mode
	WrapMode   _vWrapMode;

	//! event that occurred during a frame
	int32 _event;

	//! current selection
	int32 _selection;

	//! if a switch event happens, switch selection is one of the elements being switched, and the other is _selection
	int32 _switchSelection;

	//! first selection that player confirmed on in double-confirm mode
	int32 _firstSelection;

	//! vector containing each option
	std::vector<private_video::Option> _options;

	//! how many options there are in this box
	int32 _numOptions;

	//! true if the box is currently in the middle of scrolling
	bool  _scrolling;

	//! structure containing properties of the current font like height, etc.
	FontProperties _fontProperties;

	/** \brief given an alignment and the bounds of an option cell, it sets up the correct
	 *         flags to render into that cell, and returns the x and y values where the
	 *         text should be rendered.
	 * \param xalign left/right alignement of text in option box
	 * \param yalign top/bottom alignement of text in option box
	 * \param bounds bounds of the option box, used in conjunction with xalign and yalign
	 *	      to determine x and y coordinates for the cursor
	 * \param x x position of the cursor
	 * \param y y position of the cursor
	**/
	void _SetupAlignment(int32 xalign, int32 yalign, const private_video::OptionCellBounds &bounds, float &x, float &y);

	//! \brief Removes all options from the optionbox
	void _ClearOptions();

	/** \brief helper function to parse text for an option box, and fill an Option structure
	 * \param formatString the formatted string, using the XML structure described by SetOptions()
	 * \param option which option the string corresponds to
	 * \return success/failure
	**/
	bool _ParseOption(const hoa_utils::ustring &formatString, private_video::Option &option);

	//! \brief Switches the option items specified by _selection and _switchSelection
	void _SwitchItems();

	/** \brief increments or decrements the current selection by offset
	*** \param offset amount to move in specified direction
	*** \param horizontal true if moving horizontally, false if moving vertically
	*** \return false if the selection does not change
	**/
	bool _ChangeSelection(int32 offset, bool horizontal);
}; // class OptionBox : public private_video::GUIControl

} // namespace hoa_video

#endif  // __OPTION_HEADER__
