////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    color.h
*** \author  Raj Sharma, roos@allacrost.org
*** \brief   Header file for the Color class
*** ***************************************************************************/

#ifndef __COLOR_HEADER__
#define __COLOR_HEADER__

#include "defs.h"
#include "utils.h"

namespace hoa_video {

/** ****************************************************************************
*** \brief Representation of a RGBA color
***
*** This class encapsulates an array of 4 floats, and allows you to do basic
*** operations like adding and multiplying colors.
*** ***************************************************************************/
class Color {
public:
	/** \brief Default colors for ease of use
	*** These are defined in the file video.cpp. All colors are opaque (0.0f alpha value)
	**/
	//@{
	static Color clear;
	static Color white;
	static Color gray;
	static Color black;
	static Color red;
	static Color orange;
	static Color yellow;
	static Color green;
	static Color aqua;
	static Color blue;
	static Color violet;
	static Color brown;
	//@}

	Color()
		{ colors_[0] = 0.0f; colors_[1] = 0.0f; colors_[2] = 0.0f; colors_[3] = 1.0f; }
	Color(float r, float g, float b, float a)
		{ colors_[0] = r; colors_[1] = g; colors_[2] = b; colors_[3] = a; }

	//! \brief Overloaded Operators
	//@{
	bool operator == (const Color &c) const
		{ return colors_[0] == c.colors_[0] && colors_[1] == c.colors_[1] && colors_[2] == c.colors_[2] && colors_[3] == c.colors_[3]; }
	bool operator != (const Color &c) const
		{ return colors_[0] != c.colors_[0] || colors_[1] != c.colors_[1] || colors_[2] != c.colors_[2] || colors_[3] != c.colors_[3]; }
	Color operator + (const Color &c) const
		{
			Color col = Color(colors_[0] + c.colors_[0], colors_[1] + c.colors_[1], colors_[2] + c.colors_[2], colors_[3] + c.colors_[3]);
			if(col[0] > 1.0f) col[0] = 1.0f;
			if(col[1] > 1.0f) col[1] = 1.0f;
			if(col[2] > 1.0f) col[2] = 1.0f;
			if(col[3] > 1.0f) col[3] = 1.0f;
			return col;
		}
	Color operator *= (const Color &c)
		{ return Color(colors_[0] * c.colors_[0], colors_[1] * c.colors_[1], colors_[2] * c.colors_[2], colors_[3] * c.colors_[3]); }
	Color operator * (const Color &c) const
		{ return Color(colors_[0] * c.colors_[0], colors_[1] * c.colors_[1], colors_[2] * c.colors_[2], colors_[3] * c.colors_[3]); }
	Color operator * (float f) const
		{ return Color(colors_[0] * f, colors_[1] * f, colors_[2] * f, colors_[3]); }
	/** \note No checking of array bounds are done here for efficiency reasons. If safety is a concern, use the
	*** class member access functions instead.
	**/
	float &operator[](int32 i)
		{ return colors_[i]; }
	/** \note No checking of array bounds are done here for efficiency reasons. If safety is a concern, use the
	*** class member access functions instead.
	**/
	const float &operator[](int32 i) const
		{ return colors_[i]; }
	//@}

	//! \brief Class member access functions
	//@{
	const float* GetColors() const
		{ return colors_; }
	float GetRed() const
		{ return colors_[0]; }
	float GetGreen() const
		{ return colors_[1]; }
	float GetBlue() const
		{ return colors_[2]; }
	float GetAlpha() const
		{ return colors_[3]; }

	void SetRed(float r)
		{ colors_[0] = r; if (colors_[0] > 1.0f) colors_[0] = 1.0f; else if (colors_[0] < 0.0f) colors_[0] = 0.0f; }
	void SetGreen(float g)
		{ colors_[1] = g; if (colors_[1] > 1.0f) colors_[1] = 1.0f; else if (colors_[1] < 0.0f) colors_[1] = 0.0f; }
	void SetBlue(float b)
		{ colors_[2] = b; if (colors_[2] > 1.0f) colors_[2] = 1.0f; else if (colors_[2] < 0.0f) colors_[2] = 0.0f; }
	void SetAlpha(float a)
		{ colors_[3] = a; if (colors_[3] > 1.0f) colors_[3] = 1.0f; else if (colors_[3] < 0.0f) colors_[3] = 0.0f; }
	//@}

private:
	/** \brief The four RGBA values that represent the color
	*** These values range from 0.0 to 1.0. The indeces of the array represent:
	*** red, green, blue, and alpha in that order.
	**/
	float colors_[4];
}; // class Color

}  // namespace hoa_video

#endif  // __COLOR_HEADER__
