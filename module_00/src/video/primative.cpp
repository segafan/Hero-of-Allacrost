#include <GL/gl.h>
#include "color.h"
#include "primative.h"

namespace hoa_video {

//read comments for base function
void PrimDrawVS::Select() {
	//we ought to preserve anything we will unintentionally destroy
	glGetIntegerv(GL_SHADE_MODEL, &oldShadingMode);
	glShadeModel(curShadingMode);
}

//read comments for base function
void PrimDrawVS::Deselect() {
	//restore anything we preserved
	glShadeModel(oldShadingMode);
}

/*******************************************************************************
* draws a rectangle centered at the current position using the chosen colors
* of the desired width and height, pretty simple really
*******************************************************************************/
void PrimDrawVS::Rect(const float width, const float height) {
	float x=width/2;
	float y=height/2;

	glBegin(GL_QUADS);
		quadCols[0].Use();
		glVertex2f(-x,-y);

		quadCols[1].Use();
		glVertex2f(x,-y);

		quadCols[2].Use();
		glVertex2f(x,y);

		quadCols[3].Use();
		glVertex2f(-x,y);
	glEnd();
}

/*******************************************************************************
* sets the color of corner i to c. i goes round anticlockwise starting with
* bottom left corner
*******************************************************************************/
void PrimDrawVS::SetRectCornerColor(const Color &c, int i) {
	if (i<0) i=0;
	if (i>3) i=3;

	quadCols[i]=c;
}

/*******************************************************************************
* chooses whether we do silly overused shading, or boring dull shading, take
* your pick
*******************************************************************************/
void PrimDrawVS::SmoothShading(const bool enable) {
	if (enable) {
		curShadingMode=GL_SMOOTH;
	} else {
		curShadingMode=GL_FLAT;
	}
	glShadeModel(curShadingMode);
}

//end of namespace
}
