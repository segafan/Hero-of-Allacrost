#include <GL/gl.h>
#include <cassert>
#include "coord_sys.h"

namespace hoa_video {

OrthographicCS::OrthographicCS(float left, float right,
							   float bottom, float top, int nlayers)
	: l(left), r(right), b(bottom), t(top), layers(nlayers) {}

void OrthographicCS::Setup() const {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(l, r, b, t, -1e-4, layers+1e-4);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//namespace
}
