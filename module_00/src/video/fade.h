///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
/////////////////////////////////////////////////////////////////////////////// 

/*!****************************************************************************
 * \file    fade.h
 * \author  Raj Sharma, roos@allacrost.org
 * \date    Last Updated: November 7th, 2005
 * \brief   Header file for ScreenFader class
 *
 * The ScreenFader class is used internally by the video engine to calculate
 * how much to fade the screen by.
 *****************************************************************************/ 

#ifndef __FADE_HEADER__
#define __FADE_HEADER__


#include "utils.h"

namespace hoa_video
{

namespace private_video
{


/*!***************************************************************************
 *  \brief class for fading the screen. Keeps track of current color and
 *         figures out whether we should implement the fade using modulation
 *         or an overlay.
 *****************************************************************************/

class ScreenFader
{
public:
	
	ScreenFader()
	: _currentColor(0.0f, 0.0f, 0.0f, 0.0f),
	  _isFading(false)
	{
		_currentTime = _endTime = 0;
		_fadeModulation = 1.0f;
		_useFadeOverlay = false;
	}
	
	bool FadeTo(const Color &final, float numSeconds);
	bool Update(int32 t);

	//! fades are either implemented with overlays or with modulation, depending
	//! on whether it's a simple fade to black or a fade to a different color.
	//! Based on that, these functions tell what overlay and modulation factors
	//! to use.
	bool  ShouldUseFadeOverlay() { return _useFadeOverlay;   }
	Color GetFadeOverlayColor()  { return _fadeOverlayColor; }
	float GetFadeModulation()    { return _fadeModulation;   }

	bool  IsFading() { return _isFading; }

private:
	
	
	Color _currentColor;  //! color the screen is currently faded to	
	Color _initialColor;  //! color we started from
	Color _finalColor;    //! color we are fading to
	int32   _currentTime;   //! milliseconds that passed since this fade started
	int32   _endTime;       //! milliseconds that this fade was set to last for
	bool  _isFading;      //! true if we're in the middle of a fade
	
	bool  _useFadeOverlay;
	Color _fadeOverlayColor;
	float _fadeModulation;
};

} // namespace private_video
} // namespace hoa_video

#endif  // !__FADE_HEADER__
