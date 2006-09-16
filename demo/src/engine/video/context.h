////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    context.h
*** \author  Raj Sharma, roos@allacrost.org
*** \brief   Header file for the Context class
***
*** The Context class holds the current state of the video engine. This is
*** used so that the context can be pushed and popped, so that a function which
*** changes a lot of internal settings leaves the video engine in the same state
*** it entered in.
*** ***************************************************************************/

#ifndef __CONTEXT_HEADER__
#define __CONTEXT_HEADER__

#include <string>
#include "utils.h"
#include "coord_sys.h"
#include "color.h"
#include "screen_rect.h"

namespace hoa_video {

namespace private_video {

/** ****************************************************************************
*** \brief Represents the current graphics context
***
*** The grahpics context includes properties such as draw flags, axis
*** transformations and the current coordinate system. The context must be
*** pushed/popped by any GameVideo class function which modifies this context.
***
*** \note Transformation is actually handled separately by the OpenGL
*** transformation stack
*** ***************************************************************************/
class Context {
public:
	//! \brief
	char blend;
	//! \brief
	//@{
	char xalign;
	char yalign;
	//@}
	//! \brief
	//@{
	char xflip;
	char yflip;
	//@}
	
	//! \brief
	CoordSys coordSys;
	//! \brief
	std::string currentFont;
	//! \brief
	Color currentTextColor;
	//! \brief
	ScreenRect viewport;
	//! \brief
	ScreenRect scissorRect;
	//! \brief
	bool scissorEnabled;
}; // class Context

} // namespace private_video

} // namespace hoa_video

#endif   // __CONTEXT_HEADER__
