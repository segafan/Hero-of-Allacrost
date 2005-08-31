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
// FadeTo: Begins a fade to the given color in numSeconds
//         returns true if invalid parameter is passed
//-----------------------------------------------------------------------------

bool ScreenFader::FadeTo(const Color &final, float numSeconds)
{
	if(numSeconds < 0.0f)
		return false;
	
	_initialColor = _currentColor;
	_finalColor   = final;
	
	_currentTime = 0;
	_endTime     = int32(numSeconds * 1000);  // convert seconds to milliseconds here
	
	_isFading = true;
	
	// figure out if this is a simple fade or if an overlay is required
	// A simple fade is defined as a fade from clear to black, from black
	// to clear, or from somewhere between clear and black to either clear
	// or black. More simply, it's a fade where both the initial and final
	// color's RGB values are zeroed out

	_useFadeOverlay = true;	

	if( (_initialColor.color[0] == 0.0f && 
	     _initialColor.color[1] == 0.0f &&
	     _initialColor.color[2] == 0.0f &&
	     _finalColor.color[0]   == 0.0f && 
	     _finalColor.color[1]   == 0.0f && 
	     _finalColor.color[2]   == 0.0f))
	{
		_useFadeOverlay = false;
	}
	else
	{
		_fadeModulation = 1.0f;
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
	if(!_isFading)
		return true;
				
	if(_currentTime >= _endTime)
	{
		_currentColor   = _finalColor;
		_isFading       = false;
		
		if(_useFadeOverlay)
		{
			// check if we have faded to black or clear. If so, we can use modulation
			if(_finalColor[3] == 0.0f ||
			  (_finalColor[0] == 0.0f &&
			   _finalColor[1] == 0.0f &&
			   _finalColor[2] == 0.0f))
			{
				_useFadeOverlay = false;
				_fadeModulation = 1.0f - _finalColor[3];
			}
		}
		else
			_fadeModulation = 1.0f - _finalColor[3];
	}
	else
	{
		// calculate the new interpolated color
		float a = (float)_currentTime / (float)_endTime;

		_currentColor.color[3] = _Lerp(a, _initialColor.color[3], _finalColor.color[3]);

		
		// if we are fading to or from clear, then only the alpha should get
		// interpolated.
		if(_finalColor.color[3] == 0.0f)
		{
			_currentColor.color[0] = _initialColor.color[0];
			_currentColor.color[1] = _initialColor.color[1];
			_currentColor.color[2] = _initialColor.color[2];
		}
		if(_initialColor.color[3] == 0.0f)
		{
			_currentColor.color[0] = _finalColor.color[0];
			_currentColor.color[1] = _finalColor.color[1];
			_currentColor.color[2] = _finalColor.color[2];
		}
		else
		{
			_currentColor.color[0] = _Lerp(a, _initialColor.color[0], _finalColor.color[0]);
			_currentColor.color[1] = _Lerp(a, _initialColor.color[1], _finalColor.color[1]);
			_currentColor.color[2] = _Lerp(a, _initialColor.color[2], _finalColor.color[2]);
		}
		
		if(!_useFadeOverlay)
			_fadeModulation = 1.0f - _currentColor.color[3];
		else
			_fadeOverlayColor = _currentColor;
	}

	_currentTime += t;
	return true;
}


//-----------------------------------------------------------------------------
// FadeScreen: sets up a fade to the given color over "fadeTime" number of seconds
//-----------------------------------------------------------------------------

bool GameVideo::FadeScreen(const Color &color, float fadeTime)
{
	return _fader.FadeTo(color, fadeTime);
}


//-----------------------------------------------------------------------------
// IsFading: returns true if screen is in the middle of a fade
//-----------------------------------------------------------------------------

bool GameVideo::IsFading()
{
	return _fader.IsFading();
}


}  // namespace hoa_video
