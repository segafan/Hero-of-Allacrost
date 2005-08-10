/*
	===========================================================================
	Color class
	===========================================================================
	
	This class represents a color.
	
	Some things to note:
	
	1) Color() - creates Colour being the default (0,0,0,1) ie black
	2) Color(r,g,b,a) -  constructs the given color, values are clamped into
	   [0,1] with 0 being off and 1 being full intensity
	3) operator[] - use this to get/set the color components,
	   indicies 0 1 2 3 map to r g b a
*/


#ifndef __VIDEO_COLOR_HEADER__
#define __VIDEO_COLOR_HEADER__

#include "utils.h"

namespace hoa_video {

class Color
{
public:
	float color[4];
	
	bool operator == (const Color &c)
	{
		return color[0] == c.color[0] &&
		       color[1] == c.color[1] &&
		       color[2] == c.color[2] &&
		       color[3] == c.color[3];
	}
	
	void VerifyColor();

	Color();
	Color(float r, float g, float b, float a);

	void Use();
	float &operator[](int i);
};

//end of namespace
}

#endif
