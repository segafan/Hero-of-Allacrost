#include <GL/gl.h>
#include <cassert>
#include "ortho.h"

namespace hoa_video {

OrthographicVS::OrthographicVS(const float w, const float h, const int l) : width(w), height(h), layers(l) {

}

void OrthographicVS::Select() {
	//calculate and set projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, 0, layers);

	//most functions of this vs expect this matrix mode
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//and this blending function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void OrthographicVS::Deselect() {

}

/*******************************************************************************
* move directly to layer l and reset to (x,y) = (0,0)
*******************************************************************************/
void OrthographicVS::SelectLayer(const int l) const {
	//i know the matrix mode will be modelview but it just feels very very bad
	//not to call it
	//XXX -- I changed this to assert the right matrix mode -- this will make
	//it easier to catch spotty code. --CamelJockey
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif
	glLoadIdentity();
	glTranslatef(0, 0, -l);
}

/*******************************************************************************
* move relativly x+=rx, y+=ry
*******************************************************************************/
void OrthographicVS::Move(const float rx, const float ry) const {
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif
	glTranslatef(rx, ry, 0);
}

/*******************************************************************************
* rotates the coordinate axes anticlockwise by acAngle degrees, think about 
* this CARFULLY before you call it
*******************************************************************************/
void OrthographicVS::Rotate(const float acAngle) const {
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif
	glRotatef(acAngle, 0, 0, 1);
}

/*******************************************************************************
* saves your current position in a stack, bewarned this stack is small ~32
* so use it wisely
*******************************************************************************/
void OrthographicVS::PushState() const {
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif
	glPushMatrix();
}

/*******************************************************************************
* restores last position, read PushState()
*******************************************************************************/
void OrthographicVS::PopState() const {
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif
	glPopMatrix();
}

//namespace
}
