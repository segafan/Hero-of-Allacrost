#include <GL/gl.h>
#include "color.h"

namespace hoa_video {

/*******************************************************************************
* I defined operator[] to allow modification of the color, so i need to call
* this to check the color isnt invalid. it clamps all values in [0,1]
*******************************************************************************/
void Color::VerifyColor() {
	for (int i=0;i<4;i++) {
		if (color[i]<0) color[i]=0;
		if (color[i]>1) color[i]=1;
	}
}

/*******************************************************************************
* creates a default color - black
*******************************************************************************/
Color::Color() {
	Color(0, 0, 0, 1);
}

/*******************************************************************************
* creates the color specified by rgba
*******************************************************************************/
Color::Color(float r, float g, float b, float a) {
	color[0]=r;
	color[1]=g;
	color[2]=b;
	color[3]=a;
	VerifyColor();
}

/*******************************************************************************
* DONT CALL THIS
* 
* selects the color into opengl, used by other video code
*
* DONT CALL THIS
*******************************************************************************/
void Color::Use() {
	VerifyColor();
	glColor4fv(color);
}

/*******************************************************************************
* allows you to get/modify the color, the components 0 1 2 3 are r g b a
* respectivly - dont do anything stupid like int &x=Color::[0] because i am 
* free to change how anything works at any time and i need to make sure the
* color values are valid
*******************************************************************************/
float &Color::operator[](int i) {
	VerifyColor();
	if (i<0) {
		//should throw out of bounds exception
		//or log it
		//for now return color[0];
		i=0;
	}

	if (i>3) {
		//should throw out of bounds exception
		//or log it
		//for now return color[3];
		i=3;
	}

	return color[i];
}
	
//end of namespace
}
