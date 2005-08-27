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


//! An internal namespace to be used only within the video engine. Don't use this namespace anywhere else!
namespace private_video
{

// !take several samples of the FPS across frames and then average to get a steady FPS display
const int32 VIDEO_FPS_SAMPLES = 350;

// !maximum milliseconds that the current frame time and our averaged frame time must vary before we freak out and start catching up
const int32 VIDEO_MAX_FTIME_DIFF = 4;

// !if we need to play catchup with the FPS, how many samples to take
const int32 VIDEO_FPS_CATCHUP = 20;


/*!****************************************************************************
 *  \brief holds information about a menu skin (borders + interior)
 *
 *  You don't need to worry about this class and you should never create any instance of it.
 *****************************************************************************/

struct MenuSkin
{
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
