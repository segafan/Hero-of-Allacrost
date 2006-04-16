///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    menu_window.h
 * \author  Raj Sharma, roos@allacrost.org
 * \brief   Header file for menu window class
 *
 * The menu window class is a GUI control used for creating/displaying
 * those funky blue menu windows all RPGs seem to have.
 *****************************************************************************/ 


#ifndef __MENU_WINDOW_HEADER__
#define __MENU_WINDOW_HEADER__

#include "utils.h"
#include "gui.h"
#include "screen_rect.h"
#include "image.h"


//! All calls to the video engine are wrapped in this namespace.
namespace hoa_video
{

class GUI;
class MenuWindow;


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
 *  \brief Menus are basically "windows", like those blue rectangular ones
 *         in Final Fantasy games
 *****************************************************************************/

class MenuWindow : public GUIElement
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
	 *  \note  w and h are in terms of a 1024x768 coordinate system
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


	/*!
	 *  \brief get the current screen rectangle for scissoring used by this menu.
	 *         This is mainly used so that controls owned by a menu obey the parent
	 *         window's scissor rectangle
	 */
	ScreenRect GetScissorRect() { return _scissorRect; }

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
	StillImage _menuImage;      //! image descriptor of the menu
	MenuDisplayMode _mode;           //! text display mode (one character at a time, fading in, instant, etc.)
	
	bool       _isScissored;         //! true if scissoring needs to be used
	ScreenRect _scissorRect;         //! rectangle used for scissoring, set during each call to Update()
	friend class private_video::GUI;
};


} // namespace hoa_video


#endif  // !__MENU_WINDOW_HEADER__
