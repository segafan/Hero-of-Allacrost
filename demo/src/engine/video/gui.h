///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   gui.h
*** \author Raj Sharma, roos@allacrost.org
*** \brief  Header file for GUI code
***
*** This code implements the base structures of the video engine's GUI system.
*** ***************************************************************************/

#ifndef __GUI_HEADER__
#define __GUI_HEADER__

#include "defs.h"
#include "utils.h"
#include "screen_rect.h"
#include "image.h"
#include "text.h"

namespace hoa_video {

namespace private_video {

//! Take several samples of the FPS across frames and then average to get a steady FPS display
const int32 VIDEO_FPS_SAMPLES = 350;

//! Maximum milliseconds that the current frame time and our averaged frame time must vary before we freak out and start catching up
const int32 VIDEO_MAX_FTIME_DIFF = 4;

//! If we need to play catchup with the FPS, how many samples to take
const int32 VIDEO_FPS_CATCHUP = 20;

//! Assume this many characters per line of text when calculating display speed for textboxes
const int32 VIDEO_CHARS_PER_LINE = 30;

/** ****************************************************************************
*** \brief An abstract base class for all GUI elements (windows + controls).
*** This class contains basic functions such as Draw(), Update(), etc.
*** ***************************************************************************/
class GUIElement {
public:
	GUIElement();

	//! \note The destructor must be re-implemented in all children of this class.
	virtual ~GUIElement()
		{}

	//! \brief Draws the GUI element to the screen.
	virtual void Draw() = 0;

	/** \brief Updates the state of the element.
	*** \param frame_time The time that has elapsed since the last frame was drawn, in milliseconds
	**/
	virtual void Update(uint32 frame_time) = 0;

	/** \brief Does a self-check on all its members to see if all its members have been set to valid values.
	*** \param errors - A reference to a string to be filled with error messages if any errors are found.
	*** \return True if everything is initialized correctly, false otherwise.
	*** This is used internally to make sure we have a valid object before doing any complicated operations.
	*** If it detects any problems, it generates a list of errors and returns it by reference so they can be displayed.
	**/
	virtual bool IsInitialized(std::string &errors) = 0;

	/** \brief Sets the position of the object.
	*** \param x A reference to store the x coordinate of the object.
	*** \param y A reference to store the y coordinate of the object.
	*** \note X and y are in terms of a 1024x768 coordinate system
	**/
	void SetPosition(float x, float y)
		{ _x_position = x; _y_position = y; }

	/** \brief Sets the alignment of the element.
	*** \param xalign Valid values include VIDEO_X_LEFT, VIDEO_X_CENTER, or VIDEO_X_RIGHT.
	*** \param yalign Valid values include VIDEO_Y_TOP, VIDEO_Y_CENTER, or VIDEO_Y_BOTTOM.
	**/
	void SetAlignment(int32 xalign, int32 yalign);

	/** \brief Gets the position of the object.
	*** \param x A reference to store the x coordinate of the object.
	*** \param y A reference to store the y coordinate of the object.
	*** \note X and y are in terms of a 1024x768 coordinate system
	**/
	void GetPosition(float &x, float &y) const
		{ x = _x_position; y = _y_position; }

	/** \brief Gets the x and y alignment of the element.
	*** \param xalign - x alignment of the object
	*** \param yalign - y alignment of the object
	**/
	void GetAlignment(int32 &xalign, int32 &yalign) const
		{ xalign = _xalign; yalign = _yalign; }

	/** \brief Calculates and returns the four edges for an aligned rectangle
	*** \param left A reference where to store the coordinates of the rectangle's left edge.
	*** \param right A reference where to store the coordinates of the rectangle's right edge.
	*** \param bottom A reference where to store the coordinates of the rectangle's bttom edge.
	*** \param top A reference where to store the coordinates of the rectangle's top edge.
	***
	*** Given a rectangle specified in VIDEO_X_LEFT and VIDEO_Y_BOTTOM orientation, this function
	*** transforms the rectangle based on the video engine's alignment flags.
	**/
	virtual void CalculateAlignedRect(float &left, float &right, float &bottom, float &top);

protected:
	//! \brief Members for determining the element's draw alignment.
	int32 _xalign, _yalign;

	//! \brief The x and y position of the gui element.
	float _x_position, _y_position;

	//! \brief Used to determine if the object is in a valid state.
	//! \note This member is set after every change to any of the object's settings.
	bool  _initialized;

	//! \brief Contains the errors that need to be resolved if the object is in an invalid state (not ready for rendering).
	std::string _initialize_errors;
}; // class GUIElement


/** ****************************************************************************
*** \brief GUIControl is a type of GUI element, specifically for controls.
*** This is for functions that controls have, but menu windows don't have, such
*** as the SetOwner() function.
*** ***************************************************************************/
class GUIControl : public GUIElement {
public:
	GUIControl()
		{ _owner = NULL; }

	virtual ~GUIControl()
		{}

	/** \brief Sets the menu window which "owns" this control.
	*** \param owner_window A pointer to the menu that owns the control.
	*** \note If the control is not owned by any menu window, then set the owner to NULL.
	*** When a control is owned by a menu, it means that it obeys the menu's scissoring
	*** rectangle so that the control won't be drawn outside of the bounds of the menu.
	*** It also means that the position of the control is relative to the position of the
	*** window. (i.e. control.position += menu.position).
	 */
	virtual void SetOwner(MenuWindow *owner_window)
		{ _owner = owner_window; }

protected:
	/** \brief A pointer to the menu which owns this control.
	*** When the owner is set to NULL, the control can draw to any part of the screen
	*** (so scissoring is ignored) and drawing coordinates are not modified.
	**/
	MenuWindow *_owner;

	/** \brief Calculates and returns the four edges for an aligned rectangle
	*** \param left A reference where to store the coordinates of the rectangle's left edge.
	*** \param right A reference where to store the coordinates of the rectangle's right edge.
	*** \param bottom A reference where to store the coordinates of the rectangle's bttom edge.
	*** \param top A reference where to store the coordinates of the rectangle's top edge.
	***
	*** \note The difference between this function and the one for GUI elements is that
	*** controls must take their owner window into account.
	**/
	virtual void CalculateAlignedRect(float &left, float &right, float &bottom, float &top);
}; // class GUIControl : public GUIElement


/** ****************************************************************************
*** \brief Holds information about a menu skin (borders + interior).
***
*** There is only one active menu skin at any given time. In other words, there
*** can not have a GUI obje
***
*** \note You don't need to worry about this class and you should never create
*** any instance of it. It is only to exist as a member of the GUI class
***
*** \todo Add support for menu skin backgrounds to be tiled images versus
*** stretched images.
***
*** \todo Add support to allow menu skin backgrounds to be colors instead of
*** images.
***
*** \todo Enable menu skin images to be loaded as MultiImage image files.
*** ***************************************************************************/
class MenuSkin {
public:
	//! \brief The background image of the menu skin.
	StillImage background;

	/** \brief A 2d array that holds the border images for the menu skin
	*** The entries in this array represent the following parts:
	*** - skin[0][0]: upper left corner
	*** - skin[0][1]: top side
	*** - skin[0][2]: upper right corner
	*** - skin[1][0]: left side
	*** - skin[1][1]: center (no image, just colors)
	*** - skin[1][2]: right side
	*** - skin[2][0]: bottom left corner
	*** - skin[2][1]: bottom side
	*** - skin[2][2]: bottom right corner
	**/
	StillImage skin[3][3];

	/** \brief Border-connecting images, used when two or more MenuWindows are side by side.
	***  There are four tri-connectors and one quad-connector. tri_t would be an image for
	***  a 3-way connector on the top of a MenuWindow.
	**/
	StillImage tri_t, tri_l, tri_r, tri_b, quad;
}; // class MenuSkin


/** ****************************************************************************
*** \brief A helper class to the video engine to manage all of the GUI functionality.
***
*** There is exactly one instance of this class, which is both created and destroyed
*** by the GameVideo class. This class is essentially an extension of the GameVideo
*** class which manages the GUI system. It also happens to handle the drawing of
*** the average frames per second (FPS) on the screen.
*** ***************************************************************************/
class GUI {
public:
	//! \brief The current menu skin that all GUI objects will use.
	MenuSkin current_skin;

	//! \brief Keeps track of the sum of FPS values over the last VIDEO_FPS_SAMPLES frames.
	//! This is used to simplify the calculation of average frames per second.
	int32 total_fps;

	//! \brief A circular array of FPS samples used for calculating averaged FPS
	int32 fps_samples[VIDEO_FPS_SAMPLES];

	//! \brief An index variable to keep track of the start of the circular fps_samples array.
	int32 cur_sample;

	/** \brief The number of FPS samples currently recorded.
	*** This value should always be VIDEO_FPS_SAMPLES, unless the game has just started, in which
	*** case it could be anywhere from 0 to VIDEO_FPS_SAMPLES depending on how many frames have
	*** been displayed.
	**/
	int32 num_samples;

	GUI();

	~GUI();

	/** \brief Updates the FPS counter and draws it on the screen
	*** \param frame_time The number of milliseconds it took for the last frame.
	**/
	void DrawFPS(int32 frame_time);
	
	/** \brief Sets the current menu skin for GUI objects to use
	*** \param &img_file_* image filenames for the skin
	*** \param &fill_color_* color to fill the menu with. You can make it transparent by setting alpha
	*** \param &background_image background image of the menu
	*** \return True if successful, false otherwise.
	**/
	bool SetMenuSkin
	(
		const std::string &img_file_TL,
		const std::string &img_file_T,
		const std::string &img_file_TR,
		const std::string &img_file_L,
		const std::string &img_file_R,
		const std::string &img_file_BL, // image filenames for the borders
		const std::string &img_file_B,
		const std::string &img_file_BR,
		const std::string &img_file_tri_T,
		const std::string &img_file_tri_L,
		const std::string &img_file_tri_R,
		const std::string &img_file_tri_B,
		const std::string &img_file_quad,
		const Color &fill_color_TL, // color to fill the menu with. You can
		const Color &fill_color_TR, // make it transparent by setting alpha
		const Color &fill_color_BL,
		const Color &fill_color_BR,
		const std::string &background_image
	);

	/** \brief Creates a new image for a menu of a given width and height.
	*** \param id A reference to the StillImage object to contain the menu image
	*** \param width  desired width of menu, based on pixels in 1024x768 resolution
	*** \param height desired height of menu, based on pixels in 1024x768 resolution
	*** \param inner_width return value for the width of the inside of the menu
	*** \param inner_height return value for the height of the inside of the menu
	*** \param edge_visible_flags Specifies all the edges of the menu that should be drawn.
	*** In most cases, this should just be the default of VIDEO_MENU_EDGE_ALL. However, for
	*** example, you can make the left edge disappear by using ~VIDEO_MENU_EDGE_LEFT, or
	*** alternatively bitwise OR-ing all the other edge flags together (right, top, and bottom).
	*** \param edge_shared_flags Tells which sides of the menu window are shared with other menus.
	*** The rule of thumb is that when you want 2 menus to share a border, then you should hide one
	*** of the menu's border with the visible flags, and specify the other menu's border as shared,
	*** so it becomes the common one they both use.
	*** \return True if the menu image was successfully created, false otherwise.
	*** \note The width and height must be aligned to the border image sizes. So for example if the
	*** border artwork is all 8x8 images and you try to create a menu that is 117x69, it will get
	*** rounded to 120x72.
	**/
	bool CreateMenu(StillImage &id, float width, float height, float &inner_height, float &inner_width,
		int32 edge_visible_flags, int32 edge_shared_flags);

private:
	/** \brief Checks a menu skin to make sure its border image sizes are consistent.
	*** \param skin A reference to the menu skin to check.
	*** \return True if skin is consistent, false otherwise.
	***
	*** This function performs some simple checks to make sure that all of the images
	*** that compose a menu skin are of a consistent size.
	**/
	bool _CheckSkinConsistency(const MenuSkin &skin);
}; // class GUI

} // namespace private_video

} // namespace hoa_video

#endif // __GUI_HEADER__
