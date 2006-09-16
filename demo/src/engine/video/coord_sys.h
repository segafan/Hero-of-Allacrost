////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    coord_sys.h
*** \author  Raj Sharma, roos@allacrost.org
*** \brief   Header file for CoordSys class
*** ***************************************************************************/

#ifndef __COORD_SYS_HEADER__
#define __COORD_SYS_HEADER__

#include "utils.h"

namespace hoa_video {

/** ****************************************************************************
*** \brief Determines the drawing coordinates
***
*** The CoordSys structure holds a "coordinate system" defined by a rectangle
*** (left, right, bottom, and top) which determines how drawing coordinates
*** are mapped to the screen.
***
*** \note The default coordinate system is (0, 1024, 0, 768), which is the same
*** as the game's default 1024x768 resolution.
*** ***************************************************************************/
class CoordSys {
	friend class GUIElement;
	friend class TextBox;
	friend class OptionBox;
	friend class GameVideo;

public:
	CoordSys()
		{}
	CoordSys(float left, float right, float bottom, float top)
		{ _left = left; _right = right; _bottom = bottom; _top = top;
			if(_right > _left) _rightDir = 1.0f; else _rightDir = -1.0f;
			if(_top > _bottom) _upDir = 1.0f; else _upDir = -1.0f;
		}
	
private:
	//! \brief This is 1.0f if increasing y coordinates are up, otherwise it's -1.0f.
	float _upDir;
	//! \brief this is 1.0f if increasing x coordinates are right, otherwise -1.0f.
	float _rightDir;

	//! \brief The values of the four sides of the screen that determine the drawing coordinates
	//@{
	float _left;
	float _right;
	float _bottom;
	float _top;
	//@}
}; // class CoordSys

}  // namespace hoa_video

#endif   // __COORD_SYS_HEADER__
