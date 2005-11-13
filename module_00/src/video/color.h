///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
/////////////////////////////////////////////////////////////////////////////// 

/*!****************************************************************************
 * \file    color.h
 * \author  Raj Sharma, roos@allacrost.org
 * \date    Last Updated: November 7th, 2005
 * \brief   Header file for Color class
 *
 * The Color class encapsulates an array of 4 floats, and allows you to do
 * basic operations like adding and multiplying colors.
 *****************************************************************************/ 


#ifndef __COLOR_HEADER__
#define __COLOR_HEADER__


#include "utils.h"

namespace hoa_video
{

class Color
{
public:

	// default colors
	
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

	float color[4];
	
	bool operator == (const Color &c) const
	{
		return color[0] == c.color[0] &&
		       color[1] == c.color[1] &&
		       color[2] == c.color[2] &&
		       color[3] == c.color[3];
	}

	Color operator + (const Color &c) const
	{
		Color col = Color(color[0] + c.color[0],
		                  color[1] + c.color[1],
		                  color[2] + c.color[2],
		                  color[3] + c.color[3]);
		                    
		if(col[0] > 1.0f) col[0] = 1.0f;
		if(col[1] > 1.0f) col[1] = 1.0f;
		if(col[2] > 1.0f) col[2] = 1.0f;
		if(col[3] > 1.0f) col[3] = 1.0f;
		return col;
	}

	Color operator *= (const Color &c)
	{
		return Color(color[0] * c.color[0],
		             color[1] * c.color[1],
		             color[2] * c.color[2],
		             color[3] * c.color[3]);
	}


	Color operator * (const Color &c) const
	{
		return Color(color[0] * c.color[0],
		             color[1] * c.color[1],
		             color[2] * c.color[2],
		             color[3] * c.color[3]);
	}
	
	Color operator * (float f) const
	{
		return Color(color[0] * f,
		             color[1] * f,
		             color[2] * f,
		             color[3]);
	}

	Color()
	{
		color[0]=0.0f;
		color[1]=0.0f;
		color[2]=0.0f;
		color[3]=1.0f;		
	}
	
	Color(float r, float g, float b, float a)
	{
		color[0]=r;
		color[1]=g;
		color[2]=b;
		color[3]=a;
	}

	float &operator[](int32 i)
	{
		// no bounds check for efficiency!
		return color[i];
	}

	const float &operator[](int32 i) const
	{
		// no bounds check for efficiency!
		return color[i];
	}
};

}  // namespace hoa_video

#endif  // !__COLOR_HEADER__
