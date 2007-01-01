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

#ifndef __GUI_HEADER__
#define __GUI_HEADER__

#include "defs.h"
#include "utils.h"
#include "screen_rect.h"
#include "image.h"
#include "text.h"
 
//! All calls to the video engine are wrapped in this namespace.
namespace hoa_video {

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

	/*!
	 *  \brief Destructor
	 *  \note This function must be reimplemented in all children of this class.
	 */
	virtual ~GUIElement() {}

	
	/*!
	 *  \brief Draws a control.
	 *  \return True if the draw was successful, false otherwise.
	 *
	 *  \note This function must be reimplemented in all children of this class.
	 */
	virtual void Draw() = 0;


	/*!
	 *  \brief Updates the control.
	 *
	 *  \param frame_time - time elapsed during this frame, in milliseconds
	 *  \return True if update was successful, false otherwise.
	 *
	 *  \note This function must be reimplemented in all children of this class.
	 */
	virtual void Update(uint32 frame_time) = 0;


	/*!
	 *  \brief Does a self-check on all its members to see if all its members have been
	 *         set to valid values. This is used internally to make sure we have a valid
	 *         object before doing any complicated operations. If it detects
	 *         any problems, it generates a list of errors and returns it by reference
	 *         so they can be displayed.
	 *
	 *  \param errors - reference to a string to be filled if any errors are found
	 *  \return True if everything is initialized correctly, false otherwise.
	 *
	 *  \note This function must be reimplemented in all children of this class.
	 */
	virtual bool IsInitialized(std::string &errors) = 0;


	/*!
	 *  \brief Sets the position of the object.
	 *  \param x - x coordinate of the object
	 *  \param y - y coordinate of the object
	 *
	 *  \note x and y are in terms of a 1024x768 coordinate system
	 */
	void SetPosition(float x, float y) { _x = x; _y = y; }


	/*!
	 *  \brief Gets the position of the object.
	 *  \param x - x coordinate of the object
	 *  \param y - y coordinate of the object
	 *
	 *  \note x and y are in terms of a 1024x768 coordinate system
	 */
	void GetPosition(float &x, float &y) { x = _x; y = _y; }

	
	/*!
	 *  \brief Sets the alignment of the element.
	 *
	 *  \param xalign can be VIDEO_X_LEFT, VIDEO_X_CENTER, or VIDEO_X_RIGHT
	 *  \param yalign can be VIDEO_Y_TOP, VIDEO_Y_CENTER, or VIDEO_Y_BOTTOM
	 *
	 *  \return false if invalid value is passed
	 */
	void SetAlignment(int32 xalign, int32 yalign);
	

	/*!
	 *  \brief gets the x and y alignment of the element
	 *  \param xalign - x alignment of the object
	 *  \param yalign - y alignment of the object
	 */
	void GetAlignment(int32 &xalign, int32 &yalign);


	/*!
	 *  \brief given a rectangle specified in VIDEO_X_LEFT and VIDEO_Y_BOTTOM
	 *         orientation, this function transforms the rectangle based on
	 *         the video engine's alignment flags.
	 *
	 *  \note This function must be reimplemented in all children of this class.
	 */
	virtual void CalculateAlignedRect(float &left, float &right, float &bottom, float &top);

protected:
	//! alignment (left, center, right, etc)
	int32 _xalign, _yalign;
	//! position of the control
	float _x, _y;
	//! after every change to any of the settings, check if the object is in a valid state and update this bool
	bool  _initialized;
	//! if the object is in an invalid state (not ready for rendering), then this string contains the errors that need to be resolved
	std::string _initialize_errors;
}; // class GUIElement



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
	 *  \param owner_window  pointer to the menu that owns the control. If the
	 *         control is not owned by any menu window, then pass NULL. In this case,
	 *         the control can draw to any part of the screen (so scissoring is ignored)
	 *         and coordinates are not modified.
	 *
	 *  \note  This function must be reimplemented in all children of this class.
	 */
	virtual void SetOwner(MenuWindow *owner_window) { _owner = owner_window; }

protected:
	/*!
	 *  \brief Transforms a rectangle based on the coordinate system and alignment
	 *         flags.
	 *
	 *  \param left   - coordinate of left edge of rectangle
	 *  \param right  - coordinate of right edge of rectangle
	 *  \param bottom - coordinate of bottom edge of rectangle
	 *  \param top    - coordinate of top edge of rectangle
	 *
	 *  \note This function must be reimplemented in all children of this class.
	 */
	virtual void CalculateAlignedRect(float &left, float &right, float &bottom, float &top);

	//! A pointer to the menu which owns this control.
	MenuWindow *_owner;
}; // class GUIControl


//! An internal namespace to be used only within the video engine. Don't use this namespace anywhere else!
namespace private_video
{

//! Take several samples of the FPS across frames and then average to get a steady FPS display
const int32 VIDEO_FPS_SAMPLES = 350;

//! Maximum milliseconds that the current frame time and our averaged frame time must vary before we freak out and start catching up
const int32 VIDEO_MAX_FTIME_DIFF = 4;

//! If we need to play catchup with the FPS, how many samples to take
const int32 VIDEO_FPS_CATCHUP = 20;

//! Assume this many characters per line of text when calculating display speed for textboxes
const int32 VIDEO_CHARS_PER_LINE = 30;


/*!****************************************************************************
 *  \brief Holds information about a menu skin (borders + interior)
 *
 *  \note You don't need to worry about this class and you should never create
 *        any instance of it.
 *****************************************************************************/
class MenuSkin
{
public:
	/*! \brief This 2d array holds the skin for the menu.
	 *         skin[0][0]: upper left 
	 *         skin[0][1]: top
	 *         skin[0][2]: upper right
	 *         skin[1][0]: left
	 *         skin[1][1]: center (no image, just colors)
	 *         skin[1][2]: right
	 *         skin[2][0]: bottom left
	 *         skin[2][1]: bottom
	 *         skin[2][2]: bottom right
	 */
	StillImage skin[3][3];

	/*! \brief These are border-connecting images, used when having two or more
	 *         MenuWindows side by side.
	 *
	 *  \note  For example, tri_t would be an image for a 3-way connector on the top
	 *         of a MenuWindow. quad is an image for a 4-way connector.
	 */
	//@{
	StillImage tri_t;
	StillImage tri_l;
	StillImage tri_r;
	StillImage tri_b;
	StillImage quad;
	//@}
	
	//! Background image of the menu skin.
	StillImage background;
}; // class MenuSkin


/*!****************************************************************************
 *  \brief Basically a helper class to the video engine, to manage all of the
 *         GUI functionality.
 *  \note  You do not have to ever directly use this class.
 *****************************************************************************/
class GUI
{
public:
	GUI();
	~GUI();

	/*!
	 *  \brief Updates the FPS counter and draws it on the screen
	 *
	 *  \param frame_time  The number of milliseconds it took for the last frame.
	 *
	 *  \return True if successful, false otherwise.
	 */	
	bool DrawFPS(int32 frame_time);
	
	/*!
	 *  \brief Sets the current menu skin, so any menus subsequently created
	 *         by CreateMenu() use this skin.
	 *  \param &img_file_* image filenames for the skin
	 *  \param &fill_color_* color to fill the menu with. You can make it transparent by setting alpha
	 *  \param &background_image background image of the menu
	 *  \return True if successful, false otherwise.
	 */	
	bool SetMenuSkin
	(
		const std::string &img_file_TL,
		const std::string &img_file_T,
		const std::string &img_file_TR,
		const std::string &img_file_L,
		const std::string &img_file_R,
		const std::string &img_file_BL,  // image filenames for the borders
		const std::string &img_file_B,
		const std::string &img_file_BR,		
		const std::string &img_file_tri_T,
		const std::string &img_file_tri_L,
		const std::string &img_file_tri_R,
		const std::string &img_file_tri_B,
		const std::string &img_file_quad,
		const Color &fill_color_TL,     // color to fill the menu with. You can
		const Color &fill_color_TR,     // make it transparent by setting alpha
		const Color &fill_color_BL,
		const Color &fill_color_BR,
		const std::string &background_image
	);


	/*!
	 *  \brief creates a new menu descriptor of the given width and height
	 *
	 *  \param width  desired width of menu, based on pixels in 1024x768 resolution
	 *  \param height desired height of menu, based on pixels in 1024x768 resolution
	 *  \param inner_width return value for the width of the inside of the menu
	 *  \param inner_height return value for the height of the inside of the menu
	 *
	 *  \param edge_visible_flags specifies all the edges of the menu that should be drawn.
	 *         In most cases, this should just be the default of VIDEO_MENU_EDGE_ALL.
	 *         However, for example, you can make the left edge disappear by using
	 *         ~VIDEO_MENU_EDGE_LEFT, or alternatively bitwise OR-ing all the other
	 *         edge flags together (right, top, and bottom)
	 *
	 *  \param edge_shared_flags tells which sides of the menu window are shared with
	 *         other menus. The rule of thumb is that when you want 2 menus to share
	 *         a border, then you should hide one of the menu's border with the
	 *         visible flags, and specify the other menu's border as shared, so it
	 *         becomes the common one they both use.
	 *
	 *  \return True if menu was successfully created, false otherwise.
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
		float & inner_height,
		float & inne_width,
		int32 edge_visible_flags, 
		int32 edge_shared_flags
	);

	//! Current skin
	MenuSkin current_skin;
	//! A pointer to the video manager
	GameVideo *video_manager;
	//! Keeps track of the sum of FPS values over the last VIDEO_FPS_SAMPLES frames. Used to simplify averaged FPS calculations
	int32 total_FPS;
	//! A circular array of FPS samples used in calculating averaged FPS
	int32 fps_samples[VIDEO_FPS_SAMPLES];
	//! An index variable to keep track of the start of the circular array
	int32 cur_sample;
	//! The number of FPS samples currently recorded. This value should always be VIDEO_FPS_SAMPLES, unless the game has just started, in which case it could be anywhere from 0 to VIDEO_FPS_SAMPLES depending on how many frames have been displayed.
	int32 num_samples;

	/*!
	 *  \brief Checks a menu skin to make sure its border image sizes are consistent. If it finds any mistakes it will return false, and also spit out debug error messages if VIDEO_DEBUG is true.
	 *  \note This function is used only internally by the SetMenuSkin() function.
	 *
	 *  \param skin The skin you want to check
	 *  \return True if skin is consisten, false otherwise.
	 */	
	bool CheckSkinConsistency(const MenuSkin &skin);
}; // class GUI

} // namespace private_video

} // namespace hoa_video

#endif // __GUI_HEADER__
