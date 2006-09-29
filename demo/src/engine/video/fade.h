///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    fade.h
 * \author  Raj Sharma, roos@allacrost.org
 * \brief   Header file for ScreenFader class
 *
 * \note The ScreenFader class is used internally by the video engine to calculate
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
	: current_color(0.0f, 0.0f, 0.0f, 0.0f),
	  is_fading(false)
	{
		current_time = end_time = 0;
		fade_modulation = 1.0f;
		use_fade_overlay = false;
	} // constructor
	
	/** \brief Begins a screen fade.
	*** \param final - color to fade to.
	*** \param num_seconds - start fading in this number of seconds.
	*** \return True if fade was successful, false otherwise.
	**/
	bool FadeTo(const Color &final, float num_seconds);

	/** \brief Updates the ScreenFader class.
	*** \param t - how many milliseconds have passed since last update.
	*** \return True if update was successful, false otherwise.
	**/
	bool Update(int32 t);

	/** \name Fade-type Accessor Functions
	*** \brief Fades are either implemented with overlays or with modulation,
	*** depending on whether it's a simple fade to black or a fade to a different
	*** color. Based on that, these functions tell what overlay and modulation
	*** factors to use.
	**/
	//@{
	bool  ShouldUseFadeOverlay() { return use_fade_overlay;   }
	Color GetFadeOverlayColor()  { return fade_overlay_color; }
	float GetFadeModulation()    { return fade_modulation;    }
	//@}

	/** \brief Determines if a fade is currently occurring.
	*** \return True if currently doing a fade, false otherwise.
	**/
	bool  IsFading() { return is_fading; }

	//! The color the screen is currently faded to.
	Color current_color;
	//! The color we started from.
	Color initial_color;
	//! The color we are fading to.
	Color final_color;
	//! Number of milliseconds that have passed since this fade started.
	int32 current_time;
	//! Number of milliseconds that this fade was set to last for.
	int32 end_time;
	//! True if we're in the middle of a fade.
	bool  is_fading;
	
	/** \name Fade-type variables
	*** \brief Used for the type of fading done, either an overlay or modulation.
	**/
	//@{
	//! True if using an overlay, false if using modulation.
	bool  use_fade_overlay;
	//! Color of the overlay.
	Color fade_overlay_color;
	//! A float determining the degree of modulation.
	float fade_modulation;
	//@}
}; // class ScreenFader

} // namespace private_video

} // namespace hoa_video

#endif  // __FADE_HEADER__
