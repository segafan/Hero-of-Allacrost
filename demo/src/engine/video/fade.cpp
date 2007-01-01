///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////


#include <cassert>
#include <cstdarg>
#include <math.h>
#include "utils.h"
#include "fade.h"
#include "video.h"
#include "gui.h"

using namespace std;
using namespace hoa_video::private_video;

namespace hoa_video {

namespace private_video {

//-----------------------------------------------------------------------------
// FadeTo: Begins a fade to the given color in num_seconds
//         returns false if invalid parameter is passed
//-----------------------------------------------------------------------------
bool ScreenFader::FadeTo(const Color &final, float num_seconds)
{
	if(num_seconds < 0.0f)
		return false;
	
	initial_color = current_color;
	final_color   = final;
	
	current_time = 0;
	end_time     = int32(num_seconds * 1000);  // convert seconds to milliseconds here
	
	is_fading = true;
	
	// figure out if this is a simple fade or if an overlay is required
	// A simple fade is defined as a fade from clear to black, from black
	// to clear, or from somewhere between clear and black to either clear
	// or black. More simply, it's a fade where both the initial and final
	// color's RGB values are zeroed out

	use_fade_overlay = true;	

	if( (initial_color[0] == 0.0f &&
	     initial_color[1] == 0.0f &&
	     initial_color[2] == 0.0f &&
	     final_color[0]   == 0.0f &&
	     final_color[1]   == 0.0f &&
	     final_color[2]   == 0.0f))
	{
		use_fade_overlay = false;
	}
	else
	{
		fade_modulation = 1.0f;
	}
	
	Update(0);  // do initial update
	return true;
}



//-----------------------------------------------------------------------------
// Update: updates screen fader- figures out new interpolated fade color,
//         whether to fade using overlays or modulation, etc.
//-----------------------------------------------------------------------------
bool ScreenFader::Update(int32 t)
{
	if(!is_fading)
		return true;
				
	if(current_time >= end_time)
	{
		current_color = final_color;
		is_fading     = false;
		
		if(use_fade_overlay)
		{
			// check if we have faded to black or clear. If so, we can use modulation
			if(final_color[3] == 0.0f ||
			  (final_color[0] == 0.0f &&
			   final_color[1] == 0.0f &&
			   final_color[2] == 0.0f))
			{
				use_fade_overlay = false;
				fade_modulation = 1.0f - final_color[3];
			}
		}
		else
			fade_modulation = 1.0f - final_color[3];
	}
	else
	{
		// calculate the new interpolated color
		float a = (float)current_time / (float)end_time;

		current_color[3] = Lerp(a, initial_color[3], final_color[3]);

		// if we are fading to or from clear, then only the alpha should get
		// interpolated.
		if(final_color[3] == 0.0f)
		{
			current_color[0] = initial_color[0];
			current_color[1] = initial_color[1];
			current_color[2] = initial_color[2];
		}
		if(initial_color[3] == 0.0f)
		{
			current_color[0] = final_color[0];
			current_color[1] = final_color[1];
			current_color[2] = final_color[2];
		}
		else
		{
			current_color[0] = Lerp(a, initial_color[0], final_color[0]);
			current_color[1] = Lerp(a, initial_color[1], final_color[1]);
			current_color[2] = Lerp(a, initial_color[2], final_color[2]);
		}
		
		if(!use_fade_overlay)
			fade_modulation = 1.0f - current_color[3];
		else
			fade_overlay_color = current_color;
	}

	current_time += t;
	return true;
}

} // namespace private_video

//-----------------------------------------------------------------------------
// FadeScreen: sets up a fade to the given color over "fade_time" number of seconds
//-----------------------------------------------------------------------------
bool GameVideo::FadeScreen(const Color &color, float fade_time)
{
	return _fader.FadeTo(color, fade_time);
}



//-----------------------------------------------------------------------------
// IsFading: returns true if screen is in the middle of a fade
//-----------------------------------------------------------------------------
bool GameVideo::IsFading()
{
	return _fader.IsFading();
}

}  // namespace hoa_video
