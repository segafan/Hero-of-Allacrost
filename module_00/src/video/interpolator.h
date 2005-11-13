///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
/////////////////////////////////////////////////////////////////////////////// 

/*!****************************************************************************
 * \file    interpolator.h
 * \author  Raj Sharma, roos@allacrost.org
 * \date    Last Updated: November 7th, 2005
 * \brief   Header file for Interpolator class
 *
 * The Interpolator class can interpolate between a single and final value
 * using various methods (linear, fast, slow, etc.)
 *****************************************************************************/ 

#ifndef __INTERPOLATOR_HEADER__
#define __INTERPOLATOR_HEADER__


#include "utils.h"

namespace hoa_video
{


/*!***************************************************************************
 *  \brief Interpolation metods are various ways to create smoothed values
 *         between two numbers, e.g. linear interpolation
 *****************************************************************************/

enum InterpolationMethod
{
	VIDEO_INTERPOLATE_INVALID = -1,
	
	VIDEO_INTERPOLATE_EASE,   //! rise from A to B and then down to A again
	VIDEO_INTERPOLATE_SRCA,   //! constant value of A
	VIDEO_INTERPOLATE_SRCB,   //! constant value of B
	VIDEO_INTERPOLATE_FAST,   //! rises quickly at the beginning and levels out
	VIDEO_INTERPOLATE_SLOW,   //! rises slowly at the beginning then shoots up
	VIDEO_INTERPOLATE_LINEAR, //! simple linear interpolation between A and B
	
	VIDEO_INTERPOLATE_TOTAL
};


/*!***************************************************************************
 *  \brief class that lets you set up various kinds of interpolations.
 *         The basic way to use it is to set the interpolation method using
 *         SetMethod(), then call Start() with the values you want to
 *         interpolate between and the time to do it in.
 *****************************************************************************/
 
class Interpolator
{
public:

	Interpolator();

	//! call to begin an interpolation
	bool Start(float a, float b, int32 milliseconds);

	//! set the method of the interpolator. If you don't call it, the default
	//! is VIDEO_INTERPOLATION_LINEAR
	bool  SetMethod(InterpolationMethod method);
	
	float GetValue();              //! get the current value
	bool  Update(int32 frameTime);   //! call this every frame
	bool  IsFinished();            //! returns true if interpolation is done

private:

	float FastTransform(float t);
	float SlowTransform(float t);
	float EaseTransform(float t);

	bool ValidMethod();
	
	InterpolationMethod _method;
	
	float _a, _b;
	int32   _currentTime;
	int32   _endTime;
	bool  _finished;
	float _currentValue;
};

}  // namespace hoa_video

#endif // !__INTERPOLATOR_HEADER__
