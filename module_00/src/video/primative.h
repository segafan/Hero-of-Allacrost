#ifndef __VIDEO_PRIMATIVE_DRAWING_HEADER__
#define __VIDEO_PRIMATIVE_DRAWING_HEADER__

#include "color.h"

namespace hoa_video {

/*******************************************************************************
* this at the moment is a very simple vide state, it lets you draw rectangles
* you can set the color of each corner by SetRectCornerColor, corners go 
* anticlockwise starting with bottom left. if SmoothShading is false then the
* color of the top left(last color) is used.
*******************************************************************************/
class PrimDrawVS
{
private:
	Color quadCols[4];
	int oldShadingMode;
	int curShadingMode;
public:
	void Select();
	void Deselect();

	void Rect(const float width, const float height);
	void SetRectCornerColor(const Color &c, const int i);

	void SmoothShading(const bool enable);
};

//namespace
}

#endif
