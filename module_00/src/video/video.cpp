#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <cassert>
#include <cstdarg>
#include "video.h"

namespace hoa_video {

SINGLETON1(GameVideo);

GameVideo::GameVideo() : _width(0), _height(0), _setUp(false),
	_blend(0), _xalign(-1), _yalign(-1), _xflip(0), _yflip(0)
{
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Barf! SDL Video Initialization failed!\n");
		exit(1);
	}
	ilInit();
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	ilEnable(IL_ORIGIN_SET);
	iluInit();

	Clear();
	Render();
	Clear();
}

void
GameVideo::SetCoordSys(float x_left, float x_right,
					   float y_bottom, float y_top, int nl)
{
	_left = x_left;
	_right = x_right;
	_bottom = y_bottom;
	_top = y_top;
	_nl = nl;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(x_left, x_right, y_bottom, y_top, -1e-4, nl+1e-4);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

static unsigned
RoundUpPow2(unsigned x)
{
	x -= 1;
	x |= x >>  1;
	x |= x >>  2;
	x |= x >>  4;
	x |= x >>  8;
	x |= x >> 16;
	return x + 1;
}

bool GameVideo::LoadTexture(ImageDescriptor &id) {
	//load image -- check return value?
	ILuint img;
	unsigned w,h;
	ilGenImages(1, &img);
	ilBindImage(img);
	if (!ilLoadImage((char *)id.filename.c_str())) {
		ilDeleteImages(1, &img);
		return false;
	}
	w = ilGetInteger(IL_IMAGE_WIDTH);
	h = ilGetInteger(IL_IMAGE_HEIGHT);
	if (((w*h) & ((w*h) - 1)) != 0) {
		// round texture size up one size if needed.
		w = RoundUpPow2(w);
		h = RoundUpPow2(h);
		iluImageParameter(ILU_FILTER, ILU_BILINEAR);
		iluScale(w, h, 1);
	}
	glGenTextures(1, &id.texid);
	glBindTexture(GL_TEXTURE_2D, id.texid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_LINEAR_MIPMAP_NEAREST); // bilinear for now
	if (ilGetInteger(IL_IMAGE_FORMAT) == IL_RGBA)
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA8, w, h,
						  GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());
	else
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, w, h,
						  GL_RGB, GL_UNSIGNED_BYTE, ilGetData());
	ilDeleteImages(1, &img);
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

void GameVideo::SetDrawFlags(int firstflag, ...)
{
	int n;
	int flag;
	va_list args;

	va_start(args, firstflag);
	for (n=0;;n++) {
		flag = (n==0) ? firstflag : va_arg(args, int);
		switch (flag) {
		case 0: goto done;

		case VIDEO_X_LEFT: _xalign=-1; break;
		case VIDEO_X_CENTER: _xalign=0; break;
		case VIDEO_X_RIGHT: _xalign=1; break;

		case VIDEO_Y_TOP: _yalign=-1; break;
		case VIDEO_Y_CENTER: _yalign=0; break;
		case VIDEO_Y_BOTTOM: _yalign=1; break;

		case VIDEO_X_NOFLIP: _xflip=0; break;
		case VIDEO_X_FLIP: _xflip=1; break;

		case VIDEO_Y_NOFLIP: _yflip=0; break;
		case VIDEO_Y_FLIP: _yflip=1; break;

		case VIDEO_NO_BLEND: _blend=0; break;
		case VIDEO_BLEND: _blend=1; break;
		case VIDEO_BLEND_ADD: _blend=2; break;

		default:
			fprintf(stderr, "Unknown flag %d passed to SetDrawFlags()\n", flag);
		}
	}
done:
	va_end(args);
}

void GameVideo::DrawImage(const ImageDescriptor &id) {
	int s0,s1,t0,t1;
	float xoff,yoff;
	float xlo,xhi,ylo,yhi;

	if (_xflip) { s0=1, s1=0; } else { s0=0; s1=1; }
	if (_yflip) { t0=1, t1=0; } else { t0=0, t1=1; }

	if (_xflip)
		xlo=id.width, xhi=0;
	else
		xlo=0, xhi=id.width;
	if (_left > _right) { xlo=-xlo; xhi=-xhi; }

	if (_yflip)
		ylo=id.height, yhi=0;
	else
		ylo=0, yhi=id.height;
	if (_bottom > _top) { ylo=-ylo; yhi=-yhi; }

	xoff = ((_xalign+1) * id.width) * .5f * (_left < _right ? -1 : +1);
	yoff = ((_yalign+1) * id.height) * .5f * (_bottom < _top ? -1 : +1);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, id.texid);
	if (_blend) {
		glEnable(GL_BLEND);
		if (_blend == 1)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive
	}

	glPushMatrix();
	glTranslatef(xoff, yoff, 0);
	glBegin(GL_QUADS);
		glTexCoord2i(s0, t1);
		glVertex2f(xlo, ylo); //bl

		glTexCoord2i(s1, t1);
		glVertex2f(xhi, ylo); //br

		glTexCoord2i(s1, t0);
		glVertex2f(xhi, yhi);//tr

		glTexCoord2i(s0, t0);
		glVertex2f(xlo, yhi);//tl
	glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	if (_blend)
		glDisable(GL_BLEND);
}

bool GameVideo::LoadImage(ImageDescriptor &id) {
	if (!LoadTexture(id))
		return false;
	_images.push_back(&id);
	return true;
}

void GameVideo::DeleteImage(ImageDescriptor &id) {
	unsigned n=_images.size();
	for (unsigned i=0; i<n; i++)
		if (_images[i] == &id) {
			glDeleteTextures(1, &id.texid);
			_images[i]=_images[_images.size()-1];
			_images.resize(_images.size()-1);
			break;
		}
}

void GameVideo::ReloadImages() {
	unsigned n=_images.size();
	for (unsigned i=0; i<n; i++) {
		if (_images[i]->texid!=0)
			glDeleteTextures(1, &_images[i]->texid);
		LoadTexture(*_images[i]);
	}
}

void GameVideo::UnloadImages() {
	unsigned n=_images.size();
	for (unsigned i=0; i<n; i++) {
		if (_images[i]->texid != 0) {
			glDeleteTextures(1,&_images[i]->texid);
			_images[i]->texid=0;
		}
	}
}

void GameVideo::DeleteImages() {
	UnloadImages();
	_images.clear();
}

/*******************************************************************************
* change the video mode, thats about it
*******************************************************************************/
bool GameVideo::ChangeMode(const SDL_Rect &s) {
	// Yea, so we need to reload our textures but we just lost our old GL
	// context. See ReloadImages() call below.
	UnloadImages();

	//probably gonna change the struct later
	int desWidth = s.w;
	int desHeight = s.h;
	int desFlags = SDL_OPENGL;

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); // XXX
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	if (SDL_SetVideoMode(desWidth, desHeight, 0, desFlags) == NULL) {
		_setUp=false;
		_width=0;
		_height=0;
	} else {
		_setUp=true;
		_width=desWidth;
		_height=desHeight;

		ReloadImages();
	}

	return _setUp;
}

/*******************************************************************************
* set the rectangle of the screen onto which all drawing maps to, the arguments
* are percentages so 0, 0, 100, 100 would mean the whole screen
*******************************************************************************/
void GameVideo::SetViewport(float left, float bottom, float right, float top) {
	assert(left < right);
	assert(bottom < top);

	int l=int(left*_width*.01f);
	int b=int(bottom*_height*.01f);
	int r=int(right*_width*.01f);
	int t=int(top*_height*.01f);

	if (l<0) l=0;
	if (b<0) b=0;
	if (r>_width) r=_width;
	if (t>_height) t=_height;

	glViewport(l, b, r-l+1, t-b+1);
}

/*******************************************************************************
* clear the screen to black, it doesnt clear other buffers, that can be done
* by videostates that use them
*******************************************************************************/
void GameVideo::Clear() {
	SetViewport(0,0,100,100);
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT);
}

/*******************************************************************************
* if running in double buffered mode then flip the other buffer to the screen
*******************************************************************************/
void GameVideo::Render() {
	SDL_GL_SwapBuffers();
	Clear();
}

/*******************************************************************************
* move directly to layer l and reset to (x,y) = (0,0)
*******************************************************************************/
void GameVideo::SelectLayer(int l) {
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
void GameVideo::Move(float tx, float ty) {
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif
	glLoadIdentity();
	glTranslatef(tx, ty, 0);
}

void GameVideo::MoveRel(float tx, float ty) {
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif
	glTranslatef(tx, ty, 0);
}

/*******************************************************************************
* rotates the coordinate axes anticlockwise by acAngle degrees, think about 
* this CARFULLY before you call it
*******************************************************************************/
void GameVideo::Rotate(float acAngle) {
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
void GameVideo::PushState() {
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
void GameVideo::PopState() {
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif
	glPopMatrix();
}
}
