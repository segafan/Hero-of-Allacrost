///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
/////////////////////////////////////////////////////////////////////////////// 

/*!****************************************************************************
 * \file    coord_sys.h
 * \author  Raj Sharma, roos@allacrost.org
 * \date    Last Updated: November 7th, 2005
 * \brief   Header file for CoordSys class
 *
 * The CoordSys class holds information about a coordinate system.
 *****************************************************************************/ 


#ifndef __COORD_SYS_HEADER__
#define __COORD_SYS_HEADER__


#include "utils.h"


namespace hoa_video
{

/*!***************************************************************************
 *  \brief the CoordSys structure holds a "coordinate system", which is
 *         defined by rectangle (left, right, bottom, and top) which determines
 *         how coordinates are mapped to the screen. The default coordinate
 *         system is (0,1024,0,768). As another example, if you wanted to make
 *         it so that the screen coordinates ranged from 0 to 1, you could
 *         set the coordinate system to (0,1,0,1)
 *****************************************************************************/

class CoordSys
{
public:

	CoordSys() {}

	CoordSys(float left, float right, float bottom, float top)
	{
		_left   = left;
		_right  = right;
		_bottom = bottom;
		_top    = top;
		
		if(_right > _left)
			_rightDir = 1.0f;
		else
			_rightDir = -1.0f;
			
		if(_top > _bottom)
			_upDir = 1.0f;
		else
			_upDir = -1.0f;
	}
	
private:

	float _upDir;     //! this is 1.0f if increasing y coordinates are up, otherwise it's -1.0f.
	float _rightDir;  //! this is 1.0f if increasing x coordinates are right, otherwise -1.0f. Pretty much any SANE coordinate system has a rightVec of 1.0f

	float _left;
	float _right;
	float _bottom;
	float _top;

	friend class GUIElement;
	friend class TextBox;
	friend class OptionBox;
	friend class GameVideo;
};

}  // namespace hoa_video

#endif   // !__COORD_SYS_HEADER__
