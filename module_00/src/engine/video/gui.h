///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    gui.h
 * \author  Raj Sharma, roos@allacrost.org
 * \brief   Header file for GUI code
 *
 * This code implements the details of the GUI system, and is included in the
 * video engine as a private member.
 *****************************************************************************/ 



#ifndef _GUI_HEADER_
#define _GUI_HEADER_

#include "utils.h"
#include "screen_rect.h"
#include "image.h"
#include "text.h"
 
 
//! All calls to the video engine are wrapped in this namespace.
namespace hoa_video
{


namespace private_video
{
class GUI;
}


class MenuWindow;
class GameVideo;

//! Determines whether the code in the hoa_video namespace should print debug statements or not.
extern bool VIDEO_DEBUG;


/*!****************************************************************************
 *  \brief GUIElement is the base class for all GUI elements (windows + controls). 
 *         It contains some basic things like Draw(), Update(), etc.
 *****************************************************************************/

class GUIElement
{
public:
	
	GUIElement();
	virtual ~GUIElement() {}

	
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
	 *  \note  x and y are in terms of a 1024x768 coordinate system
	 */
	void SetPosition(float x, float y) { _x = x; _y = y; }


	/*!
	 *  \brief gets the position of the text box
	 *
	 *  \note  x and y are in terms of a 1024x768 coordinate system
	 */

	void GetPosition(float &x, float &y) { x = _x; y = _y; }

	
	/*!
	 *  \brief sets the alignment of the element
	 *
	 *  \param xalign can be VIDEO_X_LEFT, VIDEO_X_CENTER, or VIDEO_X_RIGHT
	 *  \param yalign can be VIDEO_Y_TOP, VIDEO_Y_CENTER, or VIDEO_Y_BOTTOM
	 *
	 *  \return false if invalid value is passed
	 */
	bool SetAlignment(int32 xalign, int32 yalign);
	

	/*!
	 *  \brief gets the x and y alignment of the element
	 */
	void GetAlignment(int32 &xalign, int32 &yalign);


	/*!
	 *  \brief given a rectangle specified in VIDEO_X_LEFT and VIDEO_Y_BOTTOM
	 *         orientation, this function transforms the rectangle based on
	 *         the video engine's alignment flags.
	 */
	virtual void CalculateAlignedRect(float &left, float &right, float &bottom, float &top);

protected:

	int32 _xalign, _yalign;          //! alignment (left, center, right, etc)
	float _x, _y;	                 //! position of the control
	bool   _initialized;             //! after every change to any of the settings, check if the object is in a valid state and update this bool
	std::string _initializeErrors;   //! if the object is in an invalid state (not ready for rendering), then this string contains the errors that need to be resolved
	
};



/*!****************************************************************************
 *  \brief GUIControl is a type of GUI element, specifically for controls.
 *         This is for functions that controls have, but menu windows don't have,
 *         such as the SetOwner() function
 *****************************************************************************/

class GUIControl : public GUIElement
{
public:
	
	GUIControl() { _owner = NULL; }
	virtual ~GUIControl() 	{}

	/*!
	 *  \brief sets the "owner" of the menu window. When a control is owned
	 *         by a menu, it means that it obeys the menu's scissoring rectangle,
	 *         so it won't draw outside of the bounds of the menu. It also means
	 *         that the position of the control is relative to the position of the
	 *         window. (i.e. control.position += menu.position).
	 *
	 *  \param ownerWindow  pointer to the menu that owns the control. If the
	 *         control is not owned by any menu window, then pass NULL. In this case,
	 *         the control can draw to any part of the screen (so scissoring is ignored)
	 *         and coordinates are not modified.
	 */
	virtual void SetOwner(MenuWindow *ownerWindow) { _owner = ownerWindow; }

protected:

	virtual void CalculateAlignedRect(float &left, float &right, float &bottom, float &top);

	MenuWindow *_owner;
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
	
	StillImage skin[3][3], tri_t, tri_l, tri_r, tri_b, quad;
	
	// background image
	StillImage background;
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
		const Color &fillColor_BR,
		const std::string &backgroundImage
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
		StillImage &id, 
		float width, 
		float height, 
		int32 edgeVisibleFlags, 
		int32 edgeSharedFlags
	);

private:

	//! current skin
	MenuSkin _currentSkin;

	//! pointer to video manager
	GameVideo *_videoManager;
	
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
