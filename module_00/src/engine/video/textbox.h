///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    textbox.h
 * \author  Raj Sharma, roos@allacrost.org
 * \brief   Header file for TextBox class
 *
 * The TextBox class is a GUI control which lets you define a rectangular area
 * of the screen to display text in.
 *****************************************************************************/ 


#ifndef __TEXTBOX_HEADER__
#define __TEXTBOX_HEADER__

#include "utils.h"
#include "gui.h"

//! All calls to the video engine are wrapped in this namespace.
namespace hoa_video
{

class GUI;
class TextBox;


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
	 *  \brief sets the width and height of the text box. Returns false and prints an error message
	 *         if the width or height are negative or larger than 1024 or 768 respectively
	 *
	 *  \note  w and h are in terms of a 1024x768 coordinate system
	 */
	bool SetDimensions(float w, float h);


	/*!
	 *  \brief gets the width and height of the text box. Returns false if SetDimensions() hasn't
	 *         been called yet
	 *
	 *  \note  w and h are in terms of a 1024x768 coordinate system
	 */
	void GetDimensions(float &w, float &h);


	/*!
	 *  \brief set the alignment for text. Returns false if invalid value is passed
	 *
	 *  \param xalign x alignment, e.g. VIDEO_X_LEFT
	 *  \param yalign y alignment, e.g. VIDEO_Y_TOP
	 */
	bool SetTextAlignment(int32 xalign, int32 yalign);


	/*!
	 *  \brief get the alignment for text
	 *
	 *  \param xalign x alignment, e.g. VIDEO_X_LEFT
	 *  \param yalign y alignment, e.g. VIDEO_Y_TOP
	 */
	void GetTextAlignment(int32 &xalign, int32 &yalign);


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

	float _width, _height;            //! dimensions of the text box

	float _displaySpeed;              //! characters per second to display text
						   
	int32 _text_xalign, _text_yalign; //! alignment flags for text
	int32 _numChars;                  //! hold the number of characters for the entire text
						   
	bool    _finished;                //! true if the text being drawn by ShowText() is done displaying in the case of gradual rendering
	int32   _currentTime;             //! milliseconds that passed since ShowText() was called
	int32   _endTime;                 //! milliseconds from the time since ShowText() was called until the text display will be complete
								   
	std::string    _font;             //! font used for this textbox
	FontProperties _fontProperties;   //! structure containing properties of the current font like height, etc.

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
	 *  \param scissorRect  scissor rectangle used for the textbox
	 */
	void _DrawTextLines(float textX, float textY, ScreenRect scissorRect);
};



} // namespace hoa_video


#endif  // !__TEXTBOX_HEADER__
