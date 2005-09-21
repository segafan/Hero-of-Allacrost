///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
/////////////////////////////////////////////////////////////////////////////// 

/*!****************************************************************************
 * \file    gui.h
 * \author  Raj Sharma, rajx30@gmail.com
 * \date    Last Updated: August 23rd, 2005
 * \brief   Header file for GUI code
 *
 * This code implements the details of the GUI system, and is included in the
 * video engine as a private member.
 *****************************************************************************/ 



#ifndef _GUI_HEADER_
#define _GUI_HEADER_

#include "utils.h"
#include "video.h"
 
 
//! All calls to the video engine are wrapped in this namespace.
namespace hoa_video
{

class GUI;

//! Determines whether the code in the hoa_video namespace should print debug statements or not.
extern bool VIDEO_DEBUG;


// !how often the menu cursor blinks (assuming it's in blinking state), in milliseconds
const int32 VIDEO_CURSOR_BLINK_RATE = 40;


// !how many milliseconds it takes to scroll when the cursor goes past the end of an option box
const int32 VIDEO_OPTION_SCROLL_TIME = 100;


// !how many milliseconds it takes for a menu to scroll in or out of view
const int32 VIDEO_MENU_SCROLL_TIME = 200;


//! \name Menu edge bitflags
//@{
/*!
	* \brief flags to control the drawing of each edge of the menu. For example,
	*        if you want to show a menu with its left edge hidden, then you'd
	*        pass in all the flags except VIDEO_MENU_EDGE_LEFT to the Create()
	*        function, or alternatively you could pass the complement of that
	*        bit flag (~VIDEO_MENU_EDGE_LEFT)
	*/

const int32 VIDEO_MENU_EDGE_LEFT   = 0x1;
const int32 VIDEO_MENU_EDGE_RIGHT  = 0x2;
const int32 VIDEO_MENU_EDGE_TOP    = 0x4;
const int32 VIDEO_MENU_EDGE_BOTTOM = 0x8;
const int32 VIDEO_MENU_EDGE_ALL    = 0xF;
//@}


/*!****************************************************************************
 *  \brief These menu display modes control how the menu appears or disappears.
 *         It can happen instantly, or it can animate showing/hiding
 *   VIDEO_MENU_INSTANT: appears/disappears instantly
 *   VIDEO_MENU_EXPAND_FROM_CENTER: starts as a thin horizontal line at center and expands out
 *****************************************************************************/
 
enum MenuDisplayMode
{
	VIDEO_MENU_INVALID = -1,
	
	VIDEO_MENU_INSTANT            = 0,
	VIDEO_MENU_EXPAND_FROM_CENTER = 1,
	
	VIDEO_MENU_TOTAL = 2
};


/*!****************************************************************************
 *  \brief These menu states tell you whether a menu is fully hidden/shown,
 *         or in the process of changing state.
 *   VIDEO_MENU_STATE_SHOWN: menu is fully shown
 *   VIDEO_MENU_STATE_SHOWING: menu is still scrolling on to the screen
 *   VIDEO_MENU_STATE_HIDING: menu is scrolling out, but not completely hidden yet
 *   VIDEO_MENU_STATE_HIDDEN: menu is fully hidden
 *****************************************************************************/

enum MenuState
{
	VIDEO_MENU_STATE_INVALID = -1,
	
	VIDEO_MENU_STATE_SHOWN   = 0,
	VIDEO_MENU_STATE_SHOWING = 1,
	VIDEO_MENU_STATE_HIDING  = 2,
	VIDEO_MENU_STATE_HIDDEN  = 3,
	
	VIDEO_MENU_STATE_TOTAL = 4
};


/*!****************************************************************************
 *  \brief These text display modes control how the text is rendered:
 *   VIDEO_TEXT_INSTANT: render the text instantly
 *   VIDEO_TEXT_CHAR: render the text one character at a time
 *   VIDEO_TEXT_FADELINE: fade each line in one at a time
 *   VIDEO_TEXT_FADECHAR: fades in each character at a time
 *   VIDEO_TEXT_REVEAL: goes left to right and reveals the text one pixel column at a time
 *   VIDEO_TEXT_FADEREVEAL: like REVEAL, except as text gets revealed it fades in
 *****************************************************************************/

enum TextDisplayMode
{
	VIDEO_TEXT_INVALID = -1,
	
	VIDEO_TEXT_INSTANT    = 0,
	VIDEO_TEXT_CHAR       = 1,
	VIDEO_TEXT_FADELINE   = 2,
	VIDEO_TEXT_FADECHAR   = 3,
	VIDEO_TEXT_REVEAL     = 4,
	VIDEO_TEXT_FADEREVEAL = 5,
	
	VIDEO_TEXT_TOTAL = 6
};


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
 *  \brief GUIControl is the base class for all GUI controls. It contains
 *         some basic things like Draw(), Update(), etc.
 *****************************************************************************/

class GUIControl
{
public:
	
	virtual ~GUIControl() {}

	
	/*!
	 *  \brief draws a control
	 */

	virtual bool Draw() = 0;


	/*!
	 *  \brief updates the control
	 *
	 *  \param frameTime time elapsed during this frame, in milliseconds
	 */

	virtual bool Update(int32 frameTime) = 0;


	/*!
	 *  \brief does a self-check on all its members to see if all its members have been
	 *         set to valid values. This is used internally to make sure we have a valid
	 *         object before doing any complicated operations. If it detects
	 *         any problems, it generates a list of errors and returns it by reference
	 *         so they can be displayed
	 *
	 *  \param errors reference to a string to be filled if any errors are found
	 */
	virtual bool IsInitialized(std::string &errors) = 0;


	/*!
	 *  \brief sets the position of the object
	 *
	 *  \note  x and y are in terms of coordinate system defined by (0, 1024, 0, 768)
	 */
	virtual void SetPosition(float x, float y) = 0;

protected:

	float _x, _y;	                 //! position of the control
	bool   _initialized;             //! after every change to any of the settings, check if the object is in a valid state and update this bool
	std::string _initializeErrors;   //! if the object is in an invalid state (not ready for rendering), then this string contains the errors that need to be resolved
	
	
	/*!
	 *  \brief given a rectangle specified in VIDEO_X_LEFT and VIDEO_Y_BOTTOM
	 *         orientation, this function transforms the rectangle based on
	 *         the video engine's alignment flags.
	 */
	
	void _CalculateAlignedRect(float &left, float &right, float &bottom, float &top);
};


/*!****************************************************************************
 *  \brief Menus are basically "windows", like those blue rectangular ones
 *         in Final Fantasy games
 *****************************************************************************/

class MenuWindow : public GUIControl
{
public:

	MenuWindow();
	~MenuWindow();
	
	/*!
	 *  \brief draws menu window on screen
	 */
	bool Draw();


	/*!
	 *  \brief updates the menu window, used for gradual show/hide effects
	 *
	 *  \param frameTime time elapsed during this frame, in milliseconds
	 */
	bool Update(int32 frameTime);


	/*!
	 *  \brief causes the menu to be visible. Depending on the display mode,
	 *         the menu might show instantly or gradually. You can check for when
	 *         the menu is fully shown by checking if GetState() returns
	 *         VIDEO_MENU_STATE_SHOWN (Until then, it is VIDOE_MENU_STATE_SHOWING)
	 *
	 *  \note  The time it takes for the menu to show is VIDEO_MENU_SCROLL_TIME
	 */
	bool Show();


	/*!
	 *  \brief hides the menu. Depending on the display mode, the menu might hide
	 *         instantly or gradually. If it's gradual, you should still continue
	 *         calling Draw() even after you call Hide() until it's fully hidden.
	 *         You can check if it's fully hidden by checking if GetState()
	 *         returns VIDEO_MENU_STATE_HIDDEN. (Until then, it will be 
	 *         VIDEO_MENU_STATE_HIDING)
	 *
	 *  \note  The time it takes for the menu to show is VIDEO_MENU_SCROLL_TIME
	 */
	bool Hide();
	

	/*!
	 *  \brief does a self-check on all its members to see if all its members have been
	 *         set to valid values. This is used internally to make sure we have a valid
	 *         object before doing any complicated operations. If it detects
	 *         any problems, it generates a list of errors and returns it by reference
	 *         so they can be displayed
	 *
	 *  \param errors reference to a string to be filled if any errors are found
	 */
	bool IsInitialized(std::string &errors);


	/*!
	 *  \brief sets the position of the object
	 *
	 *  \note  x and y are in terms of coordinate system defined by (0, 1024, 0, 768)
	 */
	void SetPosition(float x, float y);


	/*!
	 *  \brief sets the width and height of the menu. Returns false and prints an error message
	 *         if the width or height are negative or larger than 1024 or 768 respectively
	 *
	 *  \param w width in pixels
	 *  \param h height in pixels
	 *  \param edgeVisibleFlags a combination of bitflags, VIDEO_MENU_EDGE_LEFT, etc. This tells which edges are visible. (A non-visible edge means that the border gets stripped off)
	 *  \param edgeSharedFlags  a combination of bitflags, VIDEO_MENU_EDGE_LEFT, etc. This tells which edges are shared with other menus so they can use the appropriate connector images
	 *
	 *  \note  this MUST be called before you try drawing it
	 */
	bool Create(float w, float h, int32 edgeVisibleFlags = VIDEO_MENU_EDGE_ALL, int32 edgeSharedFlags = 0);


	/*!
	 *  \brief after the call to Create(), if the edge flags have to change for
	 *         some reason, call this function. Note that it is somewhat expensive
	 *         since it has to recreate the image descriptor
	 *
	 *  \param edgeVisibleFlags bit flags to specify which edges are visible
	 */
	bool ChangeEdgeVisibleFlags(int32 edgeVisibleFlags);


	/*!
	 *  \brief after the call to Create(), if the edge flags have to change for
	 *         some reason, call this function. Note that it is somewhat expensive
	 *         since it has to recreate the image descriptor
	 *
	 *  \param edgeSharedFlags bit flags to specify which edges are shared
	 */
	bool ChangeEdgeSharedFlags(int32 edgeSharedFlags);


	/*!
	 *  \brief you MUST call this when you are done using a menu. Failure to do so
	 *         may result in problems like texture memory not being freed.
	 */
	void Destroy();


	/*!
	 *  \brief gets the width and height of the menu. Returns false if SetDimensions() hasn't
	 *         been called yet
	 *
	 *  \note  w and h are in terms of coordinate system defined by (0, 1024, 0, 768) 
	 */
	void GetDimensions(float &w, float &h);


	/*!
	 *  \brief sets the current menu display mode, e.g. instantly appearing,
	 *         or expanding from the center outward, etc.
	 *
	 *  \param mode  menu display mode to use, e.g. VIDEO_MENU_INSTANT, 
	 *               VIDEO_MENU_EXPAND_FROM_CENTER, etc.
	 */
	bool SetDisplayMode(MenuDisplayMode mode);
	
	
	/*!
	 *  \brief get the current menu display mode set for this menu
	 */
	MenuDisplayMode GetDisplayMode();	
	
	/*!
	 *  \brief get the current state for this menu (hidden, shown, hiding, showing)
	 */
	MenuState GetState();

private:
	
	/*!
	 *  \brief used to recreate the menu image descriptor when the menu is created
	 *         for the first time, or if the menu skin changes
	 */
	bool RecreateImage();

	static int32 _currentMenuID;                    //! hand out new IDs to each menu that is created
	static std::map<int32, MenuWindow *> _menuMap;  //! keep a registered std::map of menus in case they need to be updated when the skin changes
	
	int32 _id;                       //! id of the menu, used to register and unregister it with the std::map when it is constructed/destructed
	float _width, _height;           //! dimensions
	int32 _edgeVisibleFlags;         //! flags used to tell which edges are visible 
	int32 _edgeSharedFlags;          //! flags used to tell which edges are shared
	
	MenuState _state;                //! menu state (hidden, shown, hiding, showing)
	int32  _currentTime;             //! milliseconds that passed since menu was shown
	ImageDescriptor _menuImage;      //! image descriptor of the menu
	MenuDisplayMode _mode;           //! text display mode (one character at a time, fading in, instant, etc.)
	
	friend class private_video::GUI;
};


/*!****************************************************************************
 *  \brief Although we have a DrawText() function, for any non-trivial text
 *         display, the TextBox class is used. This class provides a couple
 *         of things which aren't handled by DrawText(), namely word wrapping,
 *         and "gradual display" like drawing one character at a time, or fading
 *         each line of text in individually.
 *
 *  \note  The alignment flags affect the textbox as a whole, not the actual text
 *
 *  \note  This class is based on UNICODE text. If you try to use it for regular
 *         text, that's fine but it will store it internally as wide strings
 *****************************************************************************/

class TextBox : public GUIControl
{
public:

	TextBox();
	~TextBox();


	/*!
	 *  \brief must be called every frame to update the gradual text display
	 */
	bool Update(int32 frameTime);


	/*!
	 *  \brief renders the textbox. Note that it is not affected by draw flags or coord sys settings,
	 *         it uses whatever has been set for it using the Set*() calls
	 */
	bool Draw();


	/*!
	 *  \brief sets the position of the text box
	 *
	 *  \note  x and y are in terms of coordinate system defined by (0, 1024, 0, 768)
	 */
	void SetPosition(float x, float y);


	/*!
	 *  \brief gets the position of the text box
	 *
	 *  \note  x and y are in terms of coordinate system defined by (0, 1024, 0, 768)	 
	 */

	void GetPosition(float &x, float &y);


	/*!
	 *  \brief sets the width and height of the text box. Returns false and prints an error message
	 *         if the width or height are negative or larger than 1024 or 768 respectively
	 *
	 *  \note  w and h are in terms of coordinate system defined by (0, 1024, 0, 768) 
	 */
	bool SetDimensions(float w, float h);


	/*!
	 *  \brief gets the width and height of the text box. Returns false if SetDimensions() hasn't
	 *         been called yet
	 *
	 *  \note  w and h are in terms of coordinate system defined by (0, 1024, 0, 768) 
	 */
	void GetDimensions(float &w, float &h);


	/*!
	 *  \brief set the alignment for text. Returns false if invalid value is passed
	 *
	 *  \param xalign x alignment, e.g. VIDEO_X_LEFT
	 *  \param yalign y alignment, e.g. VIDEO_Y_TOP
	 */
	bool SetAlignment(int32 xalign, int32 yalign);


	/*!
	 *  \brief get the alignment for text
	 *
	 *  \param xalign x alignment, e.g. VIDEO_X_LEFT
	 *  \param yalign y alignment, e.g. VIDEO_Y_TOP
	 */
	void GetAlignment(int32 &xalign, int32 &yalign);


	/*!
	 *  \brief sets the font for this textbox. Returns false on failure
	 *
	 *  \param fontName the label associated with the font when you called LoadFont()
	 */

	bool SetFont(const std::string &fontName);


	/*!
	 *  \brief gets the font for this textbox
	 */

	std::string GetFont();


	/*!
	 *  \brief sets the current text display mode, e.g. one character at a time,
	 *         fading the text from left to right, etc.
	 *
	 *  \param mode  display mode to use, e.g. VIDEO_TEXT_CHAR for one character
	 *               at a time
	 */
	bool SetDisplayMode(const TextDisplayMode &mode);
	
	
	/*!
	 *  \brief get the current text display mode set for this textbox.
	 *
	 *  \param mode  display mode to use, e.g. VIDEO_TEXT_CHAR for one character
	 *               at a time
	 */
	TextDisplayMode GetDisplayMode();
	

	/*!
	 *  \brief sets the current text display speed
	 *
	 *  \param displaySpeed The display speed, always based on characters per
	 *                      second. If the current display mode is one line at a time,
	 *                      then the display speed is based on VIDEO_CHARS_PER_LINE 
	 *                      characters per line, so for example, a display speed of 10 
	 *                      would mean 3 seconds per line if VIDEO_CHARS_PER_LINE is 30.
	 *                      
	 *
	 *  \note  This has no effect for textboxes using the VIDEO_TEXT_INSTANT
	 *         display mode.
	 */
	bool SetDisplaySpeed(float displaySpeed);
	
	
	/*!
	 *  \brief get the current text display speed, in characters per second
	 */
	float GetDisplaySpeed();


	/*!
	 *  \brief returns true if this textbox is finished scrolling text
	 *
	 *  \note  If you create a textbox but don't draw any text on it, the
	 *         finished property is false. Only after text is drawn to it
	 *         does this return true
	 */
	bool IsFinished();


	/*!
	 *  \brief if text is in the middle of scrolling, this forces it to complete.
	 *         This is useful if a player gets impatient while text is scrolling
	 *         to the screen. Returns false if we're not in the middle of a text
	 *         render operation.
	 */
	bool ForceFinish();


	/*!
	 *  \brief sets the text for this box to the string passed in.
	 *
	 *  \note  if you use a gradual text display mode like VIDEO_TEXT_CHAR, then
	 *         the text will be displayed gradually and when it's done displaying,
	 *         IsFinished() will return true.
	 *
	 *  \note  this function checks the text passed in if it's too big for the
	 *         textbox and inserts newlines where appropriate. If the text is so
	 *         big that it can't fit even with word wrapping, an error is printed
	 *         to the console if debugging is turned on, and false is returned
	 *
	 *  \param text  text to draw
	 */
	bool ShowText(const hoa_utils::ustring &text);


	/*!
	 *  \brief non-unicode version of ShowText(). See the unicode version for more
	 *         details.
	 */
	bool ShowText(const std::string &text);


	/*!
	 *  \brief returns the text currently being displayed by textbox
	 */
	hoa_utils::ustring GetText();


	/*!
	 *  \brief clears the textbox so it's not displaying anything
	 */
	bool Clear();


	/*!
	 *  \brief returns true if this text box is empty (either because ShowText() has never been called, or because Clear() was called)
	 */
	bool IsEmpty();

	/*!
	 *  \brief does a self-check on all its members to see if all its members have been
	 *         set to valid values. This is used internally to make sure we have a valid
	 *         object before doing any complicated operations. If it detects
	 *         any problems, it generates a list of errors and returns it by reference
	 *         so they can be displayed
	 *
	 *  \param errors reference to a string to be filled if any errors are found
	 */
	bool IsInitialized(std::string &errors);

private:

	float _width, _height;           //! dimensions of the text box

	float _displaySpeed;             //! characters per second to display text
						   
	int32 _xalign, _yalign;          //! alignment flags for text
	int32 _numChars;                 //! hold the number of characters for the entire text
						   
	bool    _finished;               //! true if the text being drawn by ShowText() is done displaying in the case of gradual rendering
	int32   _currentTime;            //! milliseconds that passed since ShowText() was called
	int32   _endTime;                //! milliseconds from the time since ShowText() was called until the text display will be complete
	
	std::string    _font;            //! font used for this textbox
	FontProperties _fontProperties;  //! structure containing properties of the current font like height, etc.

	TextDisplayMode _mode;                 //! text display mode (one character at a time, fading in, instant, etc.)
	std::vector<hoa_utils::ustring> _text; //! array of strings, one for each line


	/*!
	 *  \brief returns the height of the text when it's rendered with the current font
	 */
	int32 _CalculateTextHeight();

	
	/*!
	 *  \brief returns true if the given unicode character can be interrupted for a word wrap.
	 *         For example in English, you can do a word wrap wherever there is a space (code 0x20).
	 *         Other languages might have space characters corresponding to other unicode values
	 *
	 *  \param character the character you want to check
	 */
	bool _IsBreakableChar(uint16 character);


	/*!
	 *  \brief adds a new line of text to the _text vector. If the line is too long to fit in
	 *         the width of the textbox, automatically split it into multiple lines (i.e. word wrap)
	 *
	 *  \param line unicode text string to add as a new line
	 */
	void _AddLine(const hoa_utils::ustring &line);
	

	/*!
	 *  \brief does dirtywork of drawing the text, taking the display mode into account
	 *
	 *  \param textX x value to use depending on the alignment
	 *  \param textY y value to use depending on the alignment
	 */
	void _DrawTextLines(float textX, float textY);
};


/*!****************************************************************************
 *  \brief with an option, you can have text, images, alignment tags, or
 *         position tags. An OptionElement encapsulates each of these things
 *****************************************************************************/

class OptionElement
{
public:

	OptionElementType type;    //! type of option element
	int32 value;               //! value, like an offset for a position tag, etc.
};


/*!****************************************************************************
 *  \brief holds the bounds for a particular "cell" in an option box. This is
 *         used for calculations when drawing an option box
 *****************************************************************************/

class OptionCellBounds
{
public:
	float cellYTop;     //! y coordinate of top of cell
	float cellYCenter;  //! y coordinate of center of cell
	float cellYBottom;  //! y coordinate of bottom of cell
	
	float cellXLeft;    //! x coordinate of left of cell
	float cellXCenter;  //! x coordinate of center of cell
	float cellXRight;	//! x coordinate of right of cell
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
	
	std::vector<OptionElement>      elements;  //! vector of option elements
	std::vector<hoa_utils::ustring> text;      //! vector of text
	std::vector<ImageDescriptor>    images;    //! vector of images
	
	bool disabled;   //! flag to specify whether this option is disabled or not
};


/*!****************************************************************************
 *  \brief The OptionBox control is used for basically showing several choices
 *         that the player can choose by moving the cursor to the choice they
 *         want and pressing the confirm key.
 *****************************************************************************/

class OptionBox : public GUIControl
{
public:
	
	OptionBox();
	~OptionBox();
	
	/*!
	 *  \brief updates the option box control
	 *
	 *  \param frameTime number of milliseconds elapsed this frame
	 */
	
	bool Update(int32 frameTime);


	/*!
	 *  \brief draws the control
	 */
		
	bool Draw();
	
	
	/*!
	 *  \brief sets the font for this control
	 *
	 *  \fontName label to a valid, already-loaded font
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
	 *  \brief sets position of the control
	 */

	void SetPosition(float x, float y);


	/*!
	 *  \brief sets the cell width and height
	 */

	void SetCellSize(float hSpacing, float vSpacing);
	

	/*!
	 *  \brief sets the size of the box in terms of number of columns and rows
	 */

	void SetSize(int32 columns, int32 rows);


	/*!
	 *  \brief sets the alignment of the option text
	 */

	void SetOptionAlignment(int32 xalign, int32 yalign);


	/*!
	 *  \brief sets the selection mode (single or double confirm mode)
	 */

	void SetSelectMode(SelectMode mode);


	/*!
	 *  \brief enables/disables switching, where player can confirm on one item, then
	 *         confirm on another item to switch them
	 */

	void EnableSwitching(bool enable);


	/*!
	 *  \brief sets the behavior to use for vertical wrapping
	 */

	void SetVerticalWrapMode(WrapMode mode);


	/*!
	 *  \brief sets the behavior to use for horizontal wrapping
	 */

	void SetHorizontalWrapMode(WrapMode mode);


	/*!
	 *  \brief sets the cursor state to be visible, hidden, or blinking
	 */

	bool SetCursorState(CursorState state);


	/*!
	 *  \brief sets the cursor offset relative to the text positions
	 */

	bool SetCursorOffset(float x, float y);	


	/*!
	 *  \brief sets the current selection (0 to _numOptions-1)
	 */

	bool SetSelection(int32 index);


	/*!
	 *  \brief sets the options to display in this option box
	 *
	 *  \param formatText a vector of unicode strings which contain the text
	 *         for each item, along with any formatting tags
	 *
	 *         For example: "<img/weapons/mythril.png>Mythril knife<r>500 Gil"
	 */

	bool SetOptions(const std::vector<hoa_utils::ustring> &formatText);


	/*!
	 *  \brief enables/disables the option with the given index
	 */

	bool EnableOption(int32 index, bool enable);


	/*!
	 *  \brief sorts the option list alphabetically
	 */

	bool Sort();


	/*!
	 *  \brief returns true if the option box is in the middle of scrolling
	 */

	bool IsScrolling() const;


	/*!
	 *  \brief returns an integer which contains the code of an event that occurred, or
	 *         zero if no event occurred. This should be called every frame to see if 
	 *         anything new happened, like the player confirming or canceling, etc. Do
	 *         not call it more than once per frame though, because it clears the event
	 *         flag.
	 */

	int32 GetEvent();


	/*!
	 *  \brief returns the index of the currently selected option
	 */

	int32 GetSelection() const;


	/*!
	 *  \brief if double-confirm mode is enabled and one item has been confirmed but
	 *         we're waiting for the player to confirm the other, then GetSwitchSelection()
	 *         returns the index of the already-confirmed item
	 */

	int32 GetSwitchSelection() const;


	/*!
	 *  \brief returns the number of rows
	 */

	int32 GetNumRows() const;


	/*!
	 *  \brief returns the number of columns
	 */

	int32 GetNumColumns() const;


	/*!
	 *  \brief returns the number of options that were set using SetOptions()
	 */

	int32 GetNumOptions() const;


	/*!
	 *  \brief used mostly internally to determine if the option box is initialized.
	 *         If not, then "errors" is filled with a list of reasons why it is not
	 *         initialized.
	 */

	bool IsInitialized(std::string &errors);

private:

	/*!
	 *  \brief given an alignment and the bounds of an option cell, it sets up the correct
	 *         flags to render into that cell, and returns the x and y values where the
	 *         text should be rendered.
	 */

	void _SetupAlignment(int32 xalign, int32 yalign, const OptionCellBounds &bounds, float &x, float &y);


	/*!
	 *  \brief clears the list of options
	 */

	void _ClearOptions();


	/*!
	 *  \brief helper function to add a new option, used by SetOptions().
	 */

	bool _AddOption(const hoa_utils::ustring &formatString);


	/*!
	 *  \brief switches the option items specified by _selection and _switchSelection
	 */

	void _SwitchItems();


	/*!
	 *  \brief increments or decrements the current selection by offset
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


	bool   _initialized;                  //! after every change to any of the settings, check if the textbox is in a valid state and update this bool
	std::string _initializeErrors;        //! if the option box is in an invalid state (not ready for drawing), then this string contains the errors that need to be resolved
	std::string _font;                    //! font used for the options
	float _cursorX, _cursorY;             //! cursor offset
	float _switchCursorX, _switchCursorY; //! switch cursor offset (relative to the normal cursor offset)
	
	float _hSpacing;            //! horizontal spacing
	float _vSpacing;            //! vertical spacing
	int32 _numColumns;          //! number of columns
	int32 _numRows;             //! numer of rows
	int32 _xalign;              //! horizontal alignment for text
	int32 _yalign;              //! vertical alignment for text
	
	bool _blink;                //! when Update() is called, blink is set to true on frames that cursor should blink (i.e. not be visible)
							 
	int32 _blinkTime;           //! timer used for controlling blink effect
	int32 _scrollTime;          //! timer used for controlling scrolling effect
	
	int32 _scrollStartOffset;   //! offset we're scrolling from
	int32 _scrollEndOffset;     //! offset we're scrolling to
	
	int32 _scrollDirection;     //! 1 for down, -1 for up
	
	int32 _scrollOffset;        //! current scroll offset
							 
	SelectMode _selectMode;     //! selection mode
	bool _switching;            //! allow switching
							 
	CursorState _cursorState;   //! current cursor state (blinking, visible, hidden, etc)
							 
	WrapMode   _hWrapMode;      //! horizontal wrapping mode
	WrapMode   _vWrapMode;      //! vertical wrapping mode
	
	int32 _event;                    //! event that occurred during a frame
	int32 _selection;                //! current selection
	int32 _switchSelection;          //! if a switch event happens, switch selection is one of the elements being switched, and the other is _selection
	int32 _firstSelection;           //! first selection that player confirmed on in double-confirm mode
	
	std::vector <Option> _options;   //! vector containing each option
	int32 _numOptions;               //! how many options there are in this box
	bool  _scrolling;                //! true if the box is currently in the middle of scrolling
	FontProperties _fontProperties;  //! structure containing properties of the current font like height, etc.
};



//! An internal namespace to be used only within the video engine. Don't use this namespace anywhere else!
namespace private_video
{

// !take several samples of the FPS across frames and then average to get a steady FPS display
const int32 VIDEO_FPS_SAMPLES = 350;

// !maximum milliseconds that the current frame time and our averaged frame time must vary before we freak out and start catching up
const int32 VIDEO_MAX_FTIME_DIFF = 4;

// !if we need to play catchup with the FPS, how many samples to take
const int32 VIDEO_FPS_CATCHUP = 20;

// !assume this many characters per line of text when calculating display speed for textboxes
const int32 VIDEO_CHARS_PER_LINE = 30;


/*!****************************************************************************
 *  \brief holds information about a menu skin (borders + interior)
 *
 *  You don't need to worry about this class and you should never create any instance of it.
 *****************************************************************************/

class MenuSkin
{
public:
	// this 2d array holds the skin for the menu.
	
	// skin[0][0]: upper left 
	// skin[0][1]: top
	// skin[0][2]: upper right
	// skin[1][0]: left
	// skin[1][1]: center (no image, just colors)
	// skin[1][2]: right
	// skin[2][0]: bottom left
	// skin[2][1]: bottom
	// skin[2][2]: bottom right
	
	hoa_video::ImageDescriptor skin[3][3], tri_t, tri_l, tri_r, tri_b, quad;
};


/*!****************************************************************************
 *  \brief Basically a helper class to the video engine, to manage all of the
 *         GUI functionality. You do not have to ever directly use this class.
 *****************************************************************************/

class GUI
{
public:

	GUI();
	~GUI();

	/*!
	 *  \brief updates the FPS counter and draws it on the screen
	 *
	 *  \param frameTime  The number of milliseconds it took for the last frame.
	 */	
	bool DrawFPS(int32 frameTime);
	
	/*!
	 *  \brief sets the current menu skin, so any menus subsequently created
	 *         by CreateMenu() use this skin.
	 */	
	bool SetMenuSkin
	(
		const std::string &imgFile_TL,
		const std::string &imgFile_T,
		const std::string &imgFile_TR,
		const std::string &imgFile_L,
		const std::string &imgFile_R,
		const std::string &imgFile_BL,  // image filenames for the borders
		const std::string &imgFile_B,
		const std::string &imgFile_BR,		
		const std::string &imgFile_Tri_T,
		const std::string &imgFile_Tri_L,
		const std::string &imgFile_Tri_R,
		const std::string &imgFile_Tri_B,
		const std::string &imgFile_Quad,		
		const Color &fillColor_TL,     // color to fill the menu with. You can
		const Color &fillColor_TR,     // make it transparent by setting alpha
		const Color &fillColor_BL,
		const Color &fillColor_BR
	);


	/*!
	 *  \brief creates a new menu descriptor of the given width and height
	 *
	 *  \param width  desired width of menu, based on pixels in 1024x768 resolution
	 *  \param height desired height of menu, based on pixels in 1024x768 resolution
	 *
	 *  \param edgeVisibleFlags specifies all the edges of the menu that should be drawn.
	 *         In most cases, this should just be the default of VIDEO_MENU_EDGE_ALL.
	 *         However, for example, you can make the left edge disappear by using
	 *         ~VIDEO_MENU_EDGE_LEFT, or alternatively bitwise OR-ing all the other
	 *         edge flags together (right, top, and bottom)
	 *
	 *  \param edgeSharedFlags tells which sides of the menu window are shared with
	 *         other menus. The rule of thumb is that when you want 2 menus to share
	 *         a border, then you should hide one of the menu's border with the
	 *         visible flags, and specify the other menu's border as shared, so it
	 *         becomes the common one they both use.
	 *
	 *  \note  Width and height must be aligned to the border image sizes. So for example
	 *         if your border artwork is all 8x8 images and you try to create a menu that
	 *         is 117x69, it will get rounded to 120x72.
	 */	
	bool CreateMenu
	(
		ImageDescriptor &id, 
		float width, 
		float height, 
		int32 edgeVisibleFlags, 
		int32 edgeSharedFlags
	);

private:

	//! current skin
	MenuSkin _currentSkin;

	//! pointer to video manager
	hoa_video::GameVideo *_videoManager;
	
	//! keeps track of the sum of FPS values over the last VIDEO_FPS_SAMPLES frames. Used to simplify averaged FPS calculations
	int32 _totalFPS;
	
	//! circular array of FPS samples used in calculating averaged FPS
	int32 _fpsSamples[VIDEO_FPS_SAMPLES];
	
	//! index variable to keep track of the start of the circular array
	int32 _curSample;
	
	//! number of FPS samples currently recorded. This value should always be VIDEO_FPS_SAMPLES, unless the game has just started, in which case it could be anywhere from 0 to VIDEO_FPS_SAMPLES depending on how many frames have been displayed.
	int32 _numSamples;

	/*!
	 *  \brief checks a menu skin to make sure its border image sizes are consistent. If it finds any mistakes it will return false, and also spit out debug error messages if VIDEO_DEBUG is true.
	 *         Note this function is used only internally by the SetMenuSkin() function.
	 *
	 *  \param skin    The skin you want to check
	 */	
	bool _CheckSkinConsistency(const MenuSkin &skin);
};

} // namespace private_video

} // namespace hoa_video

#endif // !_GUI_HEADER_
