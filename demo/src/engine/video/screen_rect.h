///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    screen_rect.h
 * \author  Raj Sharma, roos@allacrost.org
 * \brief   Header file for ScreenRect class
 *
 * The ScreenRect class is used for storing rectangles with pixel coordinates
 *****************************************************************************/ 


#ifndef __SCREEN_RECT_HEADER__
#define __SCREEN_RECT_HEADER__

#include "defs.h"
#include "utils.h"

namespace hoa_video
{

/*!***************************************************************************
 *  \brief a rectangle structure, used in storing the current scissoring
 *         or viewport rectangles, etc. It is based on standard screen coordinates,
 *         where (0,0) is the top-left and the unit is 1 pixel (hence int32 coordinates)
 *****************************************************************************/

class ScreenRect
{
public:	
	ScreenRect() {}
	ScreenRect(int32 l, int32 t, int32 w, int32 h)
	: left(l), top(t), width(w), height(h)
	{
	}
	

	/*!
	 *  \brief intersects this rectangle with the one passed in, and modifies this
	 *         rect to be the intersection of the two. (The intersection of two
	 *         rectangles is of course, another smaller rectangle). Note that
	 *         if the two rectangles don't intersect at all, then a "zero rectangle"
	 *         results where left, top, width, and height are all zero.
	 * \param rect the rectangle to intersect with
	 */	
	
	void Intersect(const ScreenRect &rect);	
	
	int32 left, top, width, height;
};

}  // namespace hoa_video

#endif   //! __SCREEN_RECT_HEADER__
