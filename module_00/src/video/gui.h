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

//! Determines whether the code in the hoa_video namespace should print debug statements or not.
extern bool VIDEO_DEBUG;


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
	
	VIDEO_TEXT_INSTANT,
	VIDEO_TEXT_CHAR,
	VIDEO_TEXT_FADELINE,
	VIDEO_TEXT_FADECHAR,
	VIDEO_TEXT_REVEAL,
	VIDEO_TEXT_FADEREVEAL,
	
	VIDEO_TEXT_TOTAL
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

class TextBox
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
	bool SetAlignment(int xalign, int yalign);


	/*!
	 *  \brief get the alignment for text
	 *
	 *  \param xalign x alignment, e.g. VIDEO_X_LEFT
	 *  \param yalign y alignment, e.g. VIDEO_Y_TOP
	 */
	void GetAlignment(int &xalign, int &yalign);


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
	 *         textbox object before doing any complicated operations. If it detects
	 *         any problems, it generates a list of errors and returns it by reference
	 *         so they can be displayed
	 *
	 *  \param errors reference to a string to be filled if any errors are found
	 */
	bool IsInitialized(std::string &errors);


private:

	bool   _initialized;             //! after every change to any of the settings, check if the textbox is in a valid state and update this bool
	std::string _initializeErrors;   //! if the textbox is in an invalid state (not ready for rendering text), then this string contains the errors that need to be resolved

	float _x, _y;                    //! position of the text box
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
	
	// skin[0][0]: bottom left
	// skin[0][1]: bottom
	// skin[0][2]: bottom right
	// skin[1][0]: left
	// skin[1][1]: center (no image, just colors)
	// skin[1][2]: right
	// skin[2][0]: upper left
	// skin[2][1]: top
	// skin[2][2]: upper right
	
	hoa_video::ImageDescriptor skin[3][3];
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
	 *  \note  Width and height must be aligned to the border image sizes. So for example
	 *         if your border artwork is all 8x8 images and you try to create a menu that
	 *         is 117x69, it will get rounded to 120x72.
	 */	
	bool CreateMenu(ImageDescriptor &id, float width, float height);

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
