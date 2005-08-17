#include "utils.h"
#include <cassert>
#include <cstdarg>
#include "video.h"
#include <math.h>
#include "coord_sys.h"
#include "gui.h"

using namespace std;
using namespace hoa_video::local_video;

namespace hoa_video 
{

// controls how slow the slow transform is. The greater the number, the "slower" it is.
const float VIDEO_SLOW_TRANSFORM_POWER = 2.0f;

// controls how fast the fast transform is. The smaller the number, the "faster" it is.
const float VIDEO_FAST_TRANSFORM_POWER = 0.3f;


//-----------------------------------------------------------------------------
// Interpolator constructor
//-----------------------------------------------------------------------------

Interpolator::Interpolator()
{
	_method = VIDEO_INTERPOLATE_LINEAR;
	_currentTime = _endTime = 0;
	_a = _b  = 0.0f;
	_finished  = true;    // no interpolation in progress
	_currentValue = 0.0f;
}


//-----------------------------------------------------------------------------
// Start: begins an interpolation using a and b as inputs, in the given amount
//        of time.
//
// Note: not all interpolation methods mean "going from A to B". In the case of
//       linear, constant, fast, slow, they do start at A and go to B. But,
//       ease interpolations go from A to B and then back. And constant
//       interpolation means just staying at either A or B.
//-----------------------------------------------------------------------------

bool Interpolator::Start(float a, float b, int milliseconds)
{
	if(!ValidMethod())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: tried to start interpolation with invalid method!" << endl;
		return false;
	}
	
	if(milliseconds < 0)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: passed negative time value to Interpolator::Start()!" << endl;
		return false;
	}
	
	_a = a;
	_b = b;	
	
	_currentTime = 0;
	_endTime     = milliseconds;
	_finished    = false;
	
	Update(0);  // do initial update so we have a valid value for GetValue()
	return true;
}


//-----------------------------------------------------------------------------
// SetMethod: sets the current interpolation method. Two things will cause
//            this to fail:
//
//            1. You pass in an invalid method
//            2. You change the method while an interpolation is in progress
//-----------------------------------------------------------------------------

bool Interpolator::SetMethod(InterpolationMethod method)
{
	if(!_finished)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: tried to call SetMethod() on an interpolator that was still in progress!" << endl;
		return false;
	}
		
	if(!ValidMethod())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: passed an invalid method to Interpolator::SetMethod()!" << endl;
		return false;
	}
	
	_method = method;
	return true;
}


//-----------------------------------------------------------------------------
// GetValue: returns the current value of the interpolator. The current value
//           gets set when Update() is called so make sure to never call
//           GetValue() before updating
//-----------------------------------------------------------------------------

float Interpolator::GetValue()
{
	return _currentValue;
}


//-----------------------------------------------------------------------------
// Update: updates the interpolation by frameTime milliseconds.
//         If we reach the end of the interpolation, then IsFinished()
//         will return true.
//         This function will return false if the method is invalid.
//-----------------------------------------------------------------------------

bool Interpolator::Update(int frameTime)
{
	if(frameTime < 0)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: called Interpolator::Update() with negative frameTime!" << endl;
		return false;
	}
	
	if(!ValidMethod())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: called Interpolator::Update(), but method was invalid!" << endl;
		return false;
	}
	
	// update current time
	_currentTime += frameTime;
	
	if(_currentTime > _endTime)
	{
		_currentTime = _endTime;
		_finished    = true;
	}
	
	// calculate a value from 0.0f to 1.0f of how far we are in the interpolation	
	float t;
	
	if(_endTime == 0)
	{
		t = 1.0f;
	}
	else
	{
		t = (float)_currentTime / (float)_endTime;
	}
	
	if(t > 1.0f)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: calculated value of 't' was more than 1.0!" << endl;
		t = 1.0f;
	}
	
	// now apply a transformation based on the interpolation method
	
	switch(_method)
	{
		case VIDEO_INTERPOLATE_EASE:
			t = EaseTransform(t);			
			break;
		case VIDEO_INTERPOLATE_SRCA:
			t = 0.0f;
			break;
		case VIDEO_INTERPOLATE_SRCB:
			t = 1.0f;
			break;
		case VIDEO_INTERPOLATE_FAST:
			t = FastTransform(t);
			break;
		case VIDEO_INTERPOLATE_SLOW:
			t = SlowTransform(t);			
			break;
		case VIDEO_INTERPOLATE_LINEAR:
			// nothing to do, just use t value as it is!
			break;
		default:
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: in Interpolator::Update(), current method didn't match supported methods!" << endl;
			return false;
		}
	};
	
	_currentValue = Lerp(t, _a, _b);
	
	return true;
}


//-----------------------------------------------------------------------------
// FastTransform: rescales the range of t so that it looks like a sqrt function
//                from 0.0 to 1.0, i.e. it increases quickly then levels off
//-----------------------------------------------------------------------------

float Interpolator::FastTransform(float t)
{
	// the fast transform power is some number above 0.0 and less than 1.0
	return powf(t, VIDEO_FAST_TRANSFORM_POWER);
}


//-----------------------------------------------------------------------------
// SlowTransform: rescales the range of t so it looks like a power function
//                from 0.0 to 1.0, i.e. it increases slowly then rockets up
//-----------------------------------------------------------------------------

float Interpolator::SlowTransform(float t)
{
	// the slow transform power is a number above 1.0
	return powf(t, VIDEO_SLOW_TRANSFORM_POWER);
}


//-----------------------------------------------------------------------------
// EaseTransform: rescales the range of t so it increases slowly, rises to 1.0
//                then falls back to 0.0
//-----------------------------------------------------------------------------

float Interpolator::EaseTransform(float t)
{
	return 0.5f * (1.0f + sinf(VIDEO_2PI * (t - 0.25f)));
}


//-----------------------------------------------------------------------------
// IsFinished: returns true if the interpolator is done with the current
//             interpolation
//-----------------------------------------------------------------------------

bool Interpolator::IsFinished()
{
	return _finished;
}


//-----------------------------------------------------------------------------
// ValidMethod: private function to check that the current method is valid
//-----------------------------------------------------------------------------

bool Interpolator::ValidMethod()
{
	return (_method < VIDEO_INTERPOLATE_TOTAL && 
	        _method > VIDEO_INTERPOLATE_INVALID);	
}


}  // namespace hoa_video
