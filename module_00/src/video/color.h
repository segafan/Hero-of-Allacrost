#ifndef __VIDEO_COLOR_HEADER__
#define __VIDEO_COLOR_HEADER__

namespace hoa_video {

/*******************************************************************************
* This class represents a color, it will be used for drawing graphical
* primatives amongst other things. the things you might want to concern
* yourself with are
*
* 1) Color() - creates Colour being the default (0,0,0,1) ie black
* 2) Color(r,g,b,a) -  constructs the given color, values are clamped into
*    [0,1] with 0 being off and 1 being full intensity
* 3) operator[] - use this to get/set the color components, 
*    indicies 0 1 2 3 map to r g b a
*******************************************************************************/
class Color
{
private:
	float color[4];
	
	void VerifyColor();
public:
	Color();
	Color(float r, float g, float b, float a);

	void Use();
	float &operator[](int i);
};

//end of namespace
}

#endif
