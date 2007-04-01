///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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

using namespace std;
using namespace hoa_utils;
using namespace hoa_video::private_video;


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
	_current_time = _end_time = 0;
	_a = _b  = 0.0f;
	_finished  = true;    // no interpolation in progress
	_current_value = 0.0f;
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

bool Interpolator::Start(float a, float b, int32 milliseconds)
{
	if(!_ValidMethod())
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

	_current_time = 0;
	_end_time     = milliseconds;
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

	if(!_ValidMethod())
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
	return _current_value;
}


//-----------------------------------------------------------------------------
// Update: updates the interpolation by frameTime milliseconds.
//         If we reach the end of the interpolation, then IsFinished()
//         will return true.
//         This function will return false if the method is invalid.
//-----------------------------------------------------------------------------

bool Interpolator::Update(int32 frame_time)
{
	if(frame_time < 0)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: called Interpolator::Update() with negative frameTime!" << endl;
		return false;
	}

	if(!_ValidMethod())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: called Interpolator::Update(), but method was invalid!" << endl;
		return false;
	}

	// update current time
	_current_time += frame_time;

	if(_current_time > _end_time)
	{
		_current_time = _end_time;
		_finished    = true;
	}

	// calculate a value from 0.0f to 1.0f of how far we are in the interpolation
	float t;

	if(_end_time == 0)
	{
		t = 1.0f;
	}
	else
	{
		t = (float)_current_time / (float)_end_time;
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
			t = _EaseTransform(t);
			break;
		case VIDEO_INTERPOLATE_SRCA:
			t = 0.0f;
			break;
		case VIDEO_INTERPOLATE_SRCB:
			t = 1.0f;
			break;
		case VIDEO_INTERPOLATE_FAST:
			t = _FastTransform(t);
			break;
		case VIDEO_INTERPOLATE_SLOW:
			t = _SlowTransform(t);
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

	_current_value = Lerp(t, _a, _b);

	return true;
}


//-----------------------------------------------------------------------------
// _FastTransform: rescales the range of t so that it looks like a sqrt function
//                from 0.0 to 1.0, i.e. it increases quickly then levels off
//-----------------------------------------------------------------------------

float Interpolator::_FastTransform(float t)
{
	// the fast transform power is some number above 0.0 and less than 1.0
	return powf(t, VIDEO_FAST_TRANSFORM_POWER);
}


//-----------------------------------------------------------------------------
// _SlowTransform: rescales the range of t so it looks like a power function
//                from 0.0 to 1.0, i.e. it increases slowly then rockets up
//-----------------------------------------------------------------------------

float Interpolator::_SlowTransform(float t)
{
	// the slow transform power is a number above 1.0
	return powf(t, VIDEO_SLOW_TRANSFORM_POWER);
}


//-----------------------------------------------------------------------------
// _EaseTransform: rescales the range of t so it increases slowly, rises to 1.0
//                then falls back to 0.0
//-----------------------------------------------------------------------------

float Interpolator::_EaseTransform(float t)
{
	return 0.5f * (1.0f + sinf(UTILS_2PI * (t - 0.25f)));
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
// _ValidMethod: private function to check that the current method is valid
//-----------------------------------------------------------------------------

bool Interpolator::_ValidMethod()
{
	return (_method < VIDEO_INTERPOLATE_TOTAL &&
	        _method > VIDEO_INTERPOLATE_INVALID);
}


}  // namespace hoa_video
