///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    shake.h
 * \author  Raj Sharma, roos@allacrost.org
 * \date    Last Updated: November 7th, 2005
 * \brief   Header file for ShakeForce class
 *
 * The ShakeForce class holds information about a screen shake, and is used
 * by the video engine to keep track of how to shake the screen
 *****************************************************************************/ 

#ifndef __SHAKE_HEADER__
#define __SHAKE_HEADER__

#include "defs.h"
#include "utils.h"


namespace hoa_video
{


/*!***************************************************************************
 *  \brief An enumeration of shake falloff modes.which controls how quickly the
 *         shaking dies down when you do a screen shake.
 *****************************************************************************/

enum ShakeFalloff
{
	VIDEO_FALLOFF_INVALID = -1,
	
	//! shake remains at constant force
	VIDEO_FALLOFF_NONE = 0,
	
	//! shake starts out small, builds up, then dies down
	VIDEO_FALLOFF_EASE = 1,
	
	//! shake strength decreases linear til the end
	VIDEO_FALLOFF_LINEAR = 2,
	
	//! shake decreases slowly and drops off at the end
	VIDEO_FALLOFF_GRADUAL = 3,
	
	//! shake suddenly falls off, for "impacts" like meteors
	VIDEO_FALLOFF_SUDDEN = 4,
	
	VIDEO_FALLOFF_TOTAL = 5
};


namespace private_video
{

/*!***************************************************************************
 *  \brief every time ShakeScreen() is called, a new ShakeForce is created
 *         to represent the force of that particular shake
 *****************************************************************************/

class ShakeForce
{
public:

	//! initial force of the shake
	float initialForce;
	
	//! used to interpolate the shake
	Interpolator interpolator;
	
	//! milliseconds that passed since this shake started
	int32   currentTime;
	
	//! milliseconds that this shake was set to last for
	int32   endTime;
};


}  // namespace private_video
}  // namespace hoa_video

#endif  // !__SHAKE_HEADER__
