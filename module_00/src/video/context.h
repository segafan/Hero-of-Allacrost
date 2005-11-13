///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
/////////////////////////////////////////////////////////////////////////////// 

/*!****************************************************************************
 * \file    context.h
 * \author  Raj Sharma, roos@allacrost.org
 * \date    Last Updated: November 7th, 2005
 * \brief   Header file for Context class
 *
 * The Context class holds the current state of the video engine. This is
 * used so that the context can be pushed and popped, so that a function which
 * changes a lot of internal settings leaves the video engine in the same state
 * it entered in.
 *****************************************************************************/ 


#ifndef __CONTEXT_HEADER__
#define __CONTEXT_HEADER__

#include "utils.h"
#include "coord_sys.h"
#include "color.h"
#include "screen_rect.h"
#include <string>

using std::string;

namespace hoa_video
{

namespace private_video
{

/*!***************************************************************************
 *  \brief Represents the graphics "context", i.e. draw flags, transformation
 *         and coord sys. Must be pushed and popped by any GameVideo functions
 *         which modify any of the context
 *
 *         Note: transformation is actually not a part of this struct since
 *               it is handled separately by the OpenGL transformation stack
 *****************************************************************************/

class Context
{
public:
	char blend, xalign, yalign, xflip, yflip;
	CoordSys coordSys;
	std::string currentFont;
	Color currentTextColor;
	ScreenRect viewport;
	ScreenRect scissorRect;
	bool scissorEnabled;
};

} // namespace private_video
} // namespace hoa_video

#endif   // !__CONTEXT_HEADER__
