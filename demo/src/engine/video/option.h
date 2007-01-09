///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    option.h
 * \author  Raj Sharma, roos@allacrost.org
 * \brief   Header file for OptionBox class
 *
 * The OptionBox class is a GUI control which allows you to create several
 * choices, which the player can select from by using the arrow keys
 *****************************************************************************/ 

#ifndef __OPTION_HEADER__
#define __OPTION_HEADER__

#include "defs.h"
#include "utils.h"
#include "gui.h"

//! All calls to the video engine are wrapped in this namespace.
namespace hoa_video
{


//! how often the menu cursor blinks (assuming it's in blinking state), in milliseconds
const int32 VIDEO_CURSOR_BLINK_RATE = 40;

//! how many milliseconds it takes to scroll when the cursor goes past the end of an option box
const int32 VIDEO_OPTION_SCROLL_TIME = 100;


/*!****************************************************************************
 *  \brief These are the types of events that an option box can generate
 *   VIDEO_OPTION_SELECTION_CHANGE: the selection changed
 *   VIDEO_OPTION_CONFIRM:          the player confirmed an option
 *   VIDEO_OPTION_CANCEL:           the player pressed the cancel key
 *   VIDEO_OPTION_SWITCH:           two elements were just switched
 *   VIDEO_OPTION_BOUNDS_UP:        player tried to go past top of option box
 *   VIDEO_OPTION_BOUNDS_DOWN:      player tried to go past bottom of option box
 *   VIDEO_OPTION_BOUNDS_LEFT:      player tried to go past left of option box
 *   VIDEO_OPTION_BOUNDS_RIGHT:     player tried to go past right of option box
 *****************************************************************************/

enum OptionBoxEvent
{
	VIDEO_OPTION_INVALID = -1,
	
	VIDEO_OPTION_NO_EVENT = 0,
	
	VIDEO_OPTION_SELECTION_CHANGE = 0,
	VIDEO_OPTION_CONFIRM          = 1,
	VIDEO_OPTION_CANCEL           = 2,
	VIDEO_OPTION_SWITCH           = 3,
	VIDEO_OPTION_BOUNDS_UP        = 4,
	VIDEO_OPTION_BOUNDS_DOWN      = 5,
	VIDEO_OPTION_BOUNDS_LEFT      = 6,
	VIDEO_OPTION_BOUNDS_RIGHT     = 7,
	
	VIDEO_OPTION_TOTAL = 8
};


/*!****************************************************************************
 *  \brief When you create an option, it's more than just text- it can contain
 *         alignment tags, position tags, or even images. These are called
 *         "option elements":
 *   VIDEO_OPTION_ELEMENT_LEFT_ALIGN: left align tag
 *   VIDEO_OPTION_ELEMENT_CENTER_ALIGN: center align tag
 *   VIDEO_OPTION_ELEMENT_RIGHT_ALIGN: right align tag
 *   VIDEO_OPTION_ELEMENT_POSITION: position tag
 *   VIDEO_OPTION_ELEMENT_IMAGE: image
 *   VIDEO_OPTION_ELEMENT_TEXT: text
 *****************************************************************************/

enum OptionElementType
{
	VIDEO_OPTION_ELEMENT_INVALID = -1,
	
	VIDEO_OPTION_ELEMENT_LEFT_ALIGN   = 0,
	VIDEO_OPTION_ELEMENT_CENTER_ALIGN = 1,
	VIDEO_OPTION_ELEMENT_RIGHT_ALIGN  = 2,	

	VIDEO_OPTION_ELEMENT_POSITION = 3,	
	VIDEO_OPTION_ELEMENT_IMAGE    = 4,
	VIDEO_OPTION_ELEMENT_TEXT     = 5,

	VIDEO_OPTION_ELEMENT_TOTAL = 6
};


/*!****************************************************************************
 *  \brief The visual state of the menu cursor
 *   VIDEO_CURSOR_STATE_HIDDEN: causes cursor to not be displayed
 *   VIDEO_CURSOR_STATE_VISIBLE: causes cursor to be displayed
 *   VIDEO_CURSOR_STATE_BLINKING: causes cursor to continually blink
 *****************************************************************************/

enum CursorState
{
	VIDEO_CURSOR_STATE_INVALID = -1,
	
	VIDEO_CURSOR_STATE_HIDDEN   = 0,
	VIDEO_CURSOR_STATE_VISIBLE  = 1,
	VIDEO_CURSOR_STATE_BLINKING = 2,
	
	VIDEO_CURSOR_STATE_TOTAL = 3
};


/*!****************************************************************************
 *  \brief Modes to control how the cursor wraps around when the player goes
 *         too far to one side of an option box
 *   VIDEO_WRAP_MODE_NONE:     if cursor goes off the right edge, it stays
 *                             where it is
 *   VIDEO_WRAP_MODE_STRAIGHT: if the cursor goes off the right edge, it appears
 *                             on the left side, on the same row
 *   VIDEO_WRAP_MODE_SHIFTED:  if the cursor goes off the right edge, it appears
 *                             on the left side, but one row down
 *****************************************************************************/

enum WrapMode
{
	VIDEO_WRAP_MODE_INVALID = -1,
	
	VIDEO_WRAP_MODE_NONE     = 0,
	VIDEO_WRAP_MODE_STRAIGHT = 1,
	VIDEO_WRAP_MODE_SHIFTED  = 2,
	
	VIDEO_WRAP_MODE_TOTAL = 3
};


/*!****************************************************************************
 *  \brief These select modes control how confirming works when you choose options
 *   VIDEO_SELECT_SINGLE: just confirm on an item once
 *   VIDEO_SELECT_DOUBLE: confirm once to highlight an item, then again to actually
 *                        confirm. If you press confirm on one item and then again
 *                        on a *different* item, then the two items get switched
 *****************************************************************************/

enum SelectMode
{
	VIDEO_SELECT_INVALID = -1,
	
	VIDEO_SELECT_SINGLE = 0,
	VIDEO_SELECT_DOUBLE = 1,
	
	VIDEO_SELECT_TOTAL = 2
};


/*!****************************************************************************
 *  \brief with an option, you can have text, images, alignment tags, or
 *         position tags. An OptionElement encapsulates each of these things
 *****************************************************************************/

class OptionElement
{
public:

	//! type of option element
	OptionElementType type;
	
	//! value, like an offset for a position tag, etc.
	int32 value;
};


/*!****************************************************************************
 *  \brief holds the bounds for a particular "cell" in an option box. This is
 *         used for calculations when drawing an option box
 *****************************************************************************/

class OptionCellBounds
{
public:

	//! y coordinate of top of cell
	float cellYTop;
	
	//! y coordinate of center of cell
	float cellYCenter;
	
	//! y coordinate of bottom of cell
	float cellYBottom;
	
	//! x coordinate of left of cell
	float cellXLeft;
	
	//! x coordinate of center of cell
	float cellXCenter;
	
	//! x coordinate of right of cell
	float cellXRight;
};


/*!****************************************************************************
 *  \brief represents one particular option in a list. For example
 *         in a shop, one option might be "Mythril knife", and it contains
 *         an icon of a knife, the text, "Mythril knife", and then a
 *         right alignment flag, and at the end, "500 Gil"
 *****************************************************************************/

class Option
{
public:
	
	//! vector of option elements
	std::vector<OptionElement>      elements;
	
	//! vector of text
	std::vector<hoa_utils::ustring> text;
	
	//! vector of images
	std::vector<StillImage>    images;
	
	//! flag to specify whether this option is disabled or not
	bool disabled;
};


/*!****************************************************************************
 *  \brief The OptionBox control is used for basically showing several choices
 *         that the player can choose by moving the cursor to the choice they
 *         want and pressing the confirm key.
 *****************************************************************************/

class OptionBox : public private_video::GUIControl
{
public:
	
	/*!
	 *  \brief Constructor
	 */
	OptionBox();
	
	/*!
	 *  \brief Destructor
	 */
	~OptionBox();
	
	/*!
	 *  \brief updates the option box control
	 *  \param frameTime number of milliseconds elapsed this frame
	 * \return success/failure
	 */
	
	void Update(uint32 frameTime);


	/*!
	 *  \brief draws the control
	 * \return success/failure
	 */
		
	void Draw();
	
	
	/*!
	 *  \brief sets the font for this control
	 *  \param fontName label to a valid, already-loaded font
	 * \return success/failure
	 */
	
	bool SetFont(const std::string &fontName);


	/*!
	 *  \brief handles left key press
	 */

	void HandleLeftKey();


	/*!
	 *  \brief handles up key press
	 */

	void HandleUpKey();


	/*!
	 *  \brief handles down key press
	 */

	void HandleDownKey();


	/*!
	 *  \brief handles right key press
	 */

	void HandleRightKey();


	/*!
	 *  \brief handles confirm key press
	 */

	void HandleConfirmKey();


	/*!
	 *  \brief handles cancel key press
	 */

	void HandleCancelKey();


	/*!
	 *  \brief sets the cell width and height
	 * \param hSpacing horizontal spacing between cells
	 * \param vSpacing vertical spacing between cells
	 */

	void SetCellSize(float hSpacing, float vSpacing);
	

	/*!
	 *  \brief sets the size of the box in terms of number of columns and rows
	 * \param columns number of columns in the options box
	 * \param rows number of rows in the options box
	 */

	void SetSize(int32 columns, int32 rows);


	/*!
	 *  \brief sets the alignment of the option text
	 * \param xalign left/right alignment of text in the cell
	 * \param yalign top/bottom alignment of text in the cell
	 */

	void SetOptionAlignment(int32 xalign, int32 yalign);


	/*!
	 *  \brief sets the selection mode (single or double confirm mode)
	 * \param mode the select mode
	 */

	void SetSelectMode(SelectMode mode);


	/*!
	 *  \brief enables/disables switching, where player can confirm on one item, then
	 *         confirm on another item to switch them
	 * \param enable true for enabling switching, false for no
	 */

	void EnableSwitching(bool enable);


	/*!
	 *  \brief sets the behavior to use for vertical wrapping
	 * \param mode the wrap mode
	 */

	void SetVerticalWrapMode(WrapMode mode);


	/*!
	 *  \brief sets the behavior to use for horizontal wrapping
	 * \param mode the wrap mode
	 */

	void SetHorizontalWrapMode(WrapMode mode);


	/*!
	 *  \brief sets the cursor state to be visible, hidden, or blinking
	 * \param state the cursor state
	 * \return success/failure
	 */

	bool SetCursorState(CursorState state);


	/*!
	 *  \brief sets the cursor offset relative to the text positions
	 * \param x left/right offset
	 * \param y top/bottom offset
	 * \return success/failure
	 */

	bool SetCursorOffset(float x, float y);	


	/*!
	 *  \brief sets the current selection (0 to _numOptions-1)
	 * \param index the desired selection
	 * \return success/failure...could fail if SetOptions() was not called
	 */

	bool SetSelection(int32 index);


	/*!
	 *  \brief sets the options to display in this option box
	 *
	 *  \param formatText a vector of unicode strings which contain the text
	 *         for each item, along with any formatting tags
	 *
	 *         For example: "<img/weapons/mythril.png>Mythril knife<r>500 Gil"
	 * \return success/failure
	 */

	bool SetOptions(const std::vector<hoa_utils::ustring> &formatText);


	/*!
	 *  \brief changes the text of a particular option
	 *
	 *  \param index which option to change
	 *  \param text the text to change the option to
	 *
	 *  \return returns false on failure, for example if the index passed in is
	 *          invalid or if SetOptions() has never been called to initially
	 *          populate the options box
	 */

	bool SetOptionText(int32 index, const hoa_utils::ustring &text);

	/*!
	 *  \brief Adds a new option to the OptionBox
	 *
	 *  \param text for the new option
	 *
	 *  \return returns false on failure (if the given text is illegal)
	 */
	bool AddOption(const hoa_utils::ustring &text);


	/*!
	 *  \brief enables/disables the option with the given index
	 * \param index the option to enable/disable
	 * \param enable true to enable, false to disable.  Options are enabled by default.
	 */

	bool EnableOption(int32 index, bool enable);


	/*!
	 *  \brief sorts the option list alphabetically
	 * \return success/failure
	 */

	bool Sort();


	/*!
	 *  \brief returns true if the option box is in the middle of scrolling
	 * \return true if scrolling option box, false if not
	 */

	bool IsScrolling() const;

	
	/*!
	 *  \brief returns true if the given option is enabled
	 *  \param index of the option to check
	 * \return true if option is enabled, false if it's not
	 */
	bool IsEnabled(int32 index) const;


	/*!
	 *  \brief returns an integer which contains the code of an event that occurred, or
	 *         zero if no event occurred. This should be called every frame to see if 
	 *         anything new happened, like the player confirming or canceling, etc. Do
	 *         not call it more than once per frame though, because it clears the event
	 *         flag.
	 * \return int representing an option box event (i.e. cancel, confirm, left, right, etc.)
	 */

	int32 GetEvent();


	/*!
	 *  \brief returns the index of the currently selected option
	 * \return the current selection
	 */

	int32 GetSelection() const;


	/*!
	 *  \brief if double-confirm mode is enabled and one item has been confirmed but
	 *         we're waiting for the player to confirm the other, then GetSwitchSelection()
	 *         returns the index of the already-confirmed item
	 * \return index of already confirmed option
	 */

	int32 GetSwitchSelection() const;


	/*!
	 *  \brief returns the number of rows
	 * \return number of rows in option box
	 */

	int32 GetNumRows() const;


	/*!
	 *  \brief returns the number of columns
	 * \return number of columns in option box
	 */

	int32 GetNumColumns() const;


	/*!
	 *  \brief returns the number of options that were set using SetOptions()
	 * \return number of options in option box
	 */

	int32 GetNumOptions() const;


	/*!
	 *  \brief used mostly internally to determine if the option box is initialized.
	 *         If not, then "errors" is filled with a list of reasons why it is not
	 *         initialized.
	 * \param errors string to hold any error info produced by this function
	 * \return true if initialized, false if not
	 */

	bool IsInitialized(std::string &errors);

private:

	/*!
	 *  \brief given an alignment and the bounds of an option cell, it sets up the correct
	 *         flags to render into that cell, and returns the x and y values where the
	 *         text should be rendered.
	 * \param xalign left/right alignement of text in option box
	 * \param yalign top/bottom alignement of text in option box
	 * \param bounds bounds of the option box, used in conjunction with xalign and yalign
	 *	      to determine x and y coordinates for the cursor
	 * \param x x position of the cursor
	 * \param y y position of the cursor
	 */

	void _SetupAlignment(int32 xalign, int32 yalign, const OptionCellBounds &bounds, float &x, float &y);


	/*!
	 *  \brief clears the list of options
	 */

	void _ClearOptions();


	/*!
	 *  \brief helper function to parse text for an option box, and fill an Option structure
	 * \param formatString the formatted string, using the XML structure described by SetOptions()
	 * \param option which option the string corresponds to
	 * \return success/failure
	 */

	bool _ParseOption(const hoa_utils::ustring &formatString, Option &option);


	/*!
	 *  \brief switches the option items specified by _selection and _switchSelection
	 */

	void _SwitchItems();


	/*!
	 *  \brief increments or decrements the current selection by offset
	 * \param offset amount to move in specified direction
	 * \param horizontal true if moving horizontally, false if moving vertically
	 *  \return false if the selection does not change
	 */

	bool _ChangeSelection(int32 offset, bool horizontal);


	/*!
	 *  \brief plays the confirm sound
	 */

	void _PlayConfirmSound();


	/*!
	 *  \brief returns the height of the text when it's rendered with the current font
	 */

	void _PlayNoConfirmSound();


	/*!
	 *  \brief plays the select sound
	 */

	void _PlaySelectSound();


	/*!
	 *  \brief plays the switch sound
	 */

	void _PlaySwitchSound();


	//! after every change to any of the settings, check if the textbox is in a valid state and update this bool
	bool   _initialized;
	
	//! if the option box is in an invalid state (not ready for drawing), then this string contains the errors that need to be resolved
	std::string _initializeErrors;
	
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
	std::vector <Option> _options;
	
	//! how many options there are in this box
	int32 _numOptions;
	
	//! true if the box is currently in the middle of scrolling
	bool  _scrolling;
	
	//! structure containing properties of the current font like height, etc.
	FontProperties _fontProperties;
	
}; // class OptionBox : public private_video::GUIControl


} // namespace hoa_video


#endif  // !__OPTION_HEADER__
