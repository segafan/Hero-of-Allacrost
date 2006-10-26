///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    interpolator.h
 * \author  Raj Sharma, roos@allacrost.org
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
	
	//! rise from A to B and then down to A again
	VIDEO_INTERPOLATE_EASE = 0,
	//! constant value of A
	VIDEO_INTERPOLATE_SRCA = 1,
	//! constant value of B
	VIDEO_INTERPOLATE_SRCB = 2,
	//! rises quickly at the beginning and levels out
	VIDEO_INTERPOLATE_FAST = 3,
	//! rises slowly at the beginning then shoots up
	VIDEO_INTERPOLATE_SLOW = 4,
	//! simple linear interpolation between A and B
	VIDEO_INTERPOLATE_LINEAR = 5,
	
	VIDEO_INTERPOLATE_TOTAL = 6
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

	/*!***************************************************************************
	*  \brief Constructor
	*****************************************************************************/
	Interpolator();

	/*!***************************************************************************
	*  \brief Begins interpolation
	* \param a start value of interpolation
	* \param b end value of interpolation
	* \param milliseconds amount of time to interpolate over
	* \return success/failure
	*****************************************************************************/
	bool Start(float a, float b, int32 milliseconds);

	/*!***************************************************************************
	*  \brief Sets the interpolation method.  If this is not called, VIDEO_INTERPOLATION_LINEAR
	*	     is assumed
	* \param method interpolation method to use
	* \return success/failure
	*****************************************************************************/
	bool  SetMethod(InterpolationMethod method);
	
	/*!***************************************************************************
	*  \brief Gets the current interpolated value
	* \return the current interpolated value
	*****************************************************************************/
	float GetValue();
	
	/*!***************************************************************************
	*  \brief Increments time by frameTime and updates the interpolation
	* \param frameTime amount to update the time value by
	* \return success/failure
	*****************************************************************************/
	bool  Update(int32 frameTime);
	
	/*!***************************************************************************
	*  \brief Is interpolation finished?
	* \return true if done, false if not
	*****************************************************************************/
	bool  IsFinished();

private:

	/*!***************************************************************************
	*  \brief Interpolates logarithmically.  Increases quickly then levels off.
	* \param t float value to interpolate
	* \return interpolated value
	*****************************************************************************/
	float _FastTransform(float t);
	
	/*!***************************************************************************
	*  \brief Interpolates exponentially.  Increases slowly then skyrockets.
	* \param t float value to interpolate
	* \return interpolated value
	*****************************************************************************/
	float _SlowTransform(float t);
	
	/*!***************************************************************************
	*  \brief Interpolates periodically.  Increases slowly to 1.0 then back down to 0.0
	* \param t float value to interpolate
	* \return interpolated value
	*****************************************************************************/
	float _EaseTransform(float t);

	/*!***************************************************************************
	*  \brief Verifies the interpolation method is valid.
	* \return true for valid, false for not
	*****************************************************************************/
	bool _ValidMethod();
	
	//! Interpolation method used
	InterpolationMethod _method;
	
	//! the two numbers to interpolate between
	float _a, _b;
	
	//! The current time in the interpolation
	int32   _currentTime;
	
	//! The end of the interpolation
	int32   _endTime;
	
	//! If the interpolation is finished
	bool  _finished;
	
	//! The current interpolated value
	float _currentValue;
	
}; // class Interpolator

}  // namespace hoa_video

#endif // !__INTERPOLATOR_HEADER__
