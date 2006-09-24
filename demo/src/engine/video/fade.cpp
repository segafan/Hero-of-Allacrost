///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

#include "utils.h"
#include <cassert>
#include <cstdarg>
#include "video.h"
#include <math.h>
#include "gui.h"

using namespace std;
using namespace hoa_video::private_video;

namespace hoa_video 
{

//-----------------------------------------------------------------------------
// FadeTo: Begins a fade to the given color in num_seconds
//         returns false if invalid parameter is passed
//-----------------------------------------------------------------------------
bool ScreenFader::FadeTo(const Color &final, float num_seconds)
{
	if(num_seconds < 0.0f)
		return false;
	
	_initial_color = _current_color;
	_final_color   = final;
	
	_current_time = 0;
	_end_time     = int32(num_seconds * 1000);  // convert seconds to milliseconds here
	
	_is_fading = true;
	
	// figure out if this is a simple fade or if an overlay is required
	// A simple fade is defined as a fade from clear to black, from black
	// to clear, or from somewhere between clear and black to either clear
	// or black. More simply, it's a fade where both the initial and final
	// color's RGB values are zeroed out

	_use_fade_overlay = true;	

	if( (_initial_color[0] == 0.0f &&
	     _initial_color[1] == 0.0f &&
	     _initial_color[2] == 0.0f &&
	     _final_color[0]   == 0.0f &&
	     _final_color[1]   == 0.0f &&
	     _final_color[2]   == 0.0f))
	{
		_use_fade_overlay = false;
	}
	else
	{
		_fade_modulation = 1.0f;
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
	if(!_is_fading)
		return true;
				
	if(_current_time >= _end_time)
	{
		_current_color = _final_color;
		_is_fading     = false;
		
		if(_use_fade_overlay)
		{
			// check if we have faded to black or clear. If so, we can use modulation
			if(_final_color[3] == 0.0f ||
			  (_final_color[0] == 0.0f &&
			   _final_color[1] == 0.0f &&
			   _final_color[2] == 0.0f))
			{
				_use_fade_overlay = false;
				_fade_modulation = 1.0f - _final_color[3];
			}
		}
		else
			_fade_modulation = 1.0f - _final_color[3];
	}
	else
	{
		// calculate the new interpolated color
		float a = (float)_current_time / (float)_end_time;

		_current_color[3] = Lerp(a, _initial_color[3], _final_color[3]);

		// if we are fading to or from clear, then only the alpha should get
		// interpolated.
		if(_final_color[3] == 0.0f)
		{
			_current_color[0] = _initial_color[0];
			_current_color[1] = _initial_color[1];
			_current_color[2] = _initial_color[2];
		}
		if(_initial_color[3] == 0.0f)
		{
			_current_color[0] = _final_color[0];
			_current_color[1] = _final_color[1];
			_current_color[2] = _final_color[2];
		}
		else
		{
			_current_color[0] = Lerp(a, _initial_color[0], _final_color[0]);
			_current_color[1] = Lerp(a, _initial_color[1], _final_color[1]);
			_current_color[2] = Lerp(a, _initial_color[2], _final_color[2]);
		}
		
		if(!_use_fade_overlay)
			_fade_modulation = 1.0f - _current_color[3];
		else
			_fade_overlay_color = _current_color;
	}

	_current_time += t;
	return true;
}



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
