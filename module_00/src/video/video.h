#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <SDL/SDL.h>
#include <GL/gl.h>
#include <vector>
#include "utils.h"

namespace hoa_video {

class GameVideo;

class ImageDescriptor {
public:
	ImageDescriptor() : texid(0) {}
	std::string filename;
	float width, height;

private:
	GLuint texid;
	friend class GameVideo;
};

const int VIDEO_X_LEFT = 1;
const int VIDEO_X_CENTER = 2;
const int VIDEO_X_RIGHT = 3;
const int VIDEO_Y_TOP = 4;
const int VIDEO_Y_CENTER = 5;
const int VIDEO_Y_BOTTOM = 6;
const int VIDEO_X_FLIP = 7;
const int VIDEO_X_NOFLIP = 8;
const int VIDEO_Y_FLIP = 9;
const int VIDEO_Y_NOFLIP = 10;
const int VIDEO_NO_BLEND = 11;
const int VIDEO_BLEND = 12;
const int VIDEO_BLEND_ADD = 13;

/*******************************************************************************
* manages video, the functions are pretty self explanatory, but if unsure
* read the documentation in video.cpp
*******************************************************************************/
class GameVideo
{
private:
	SINGLETON_DECLARE(GameVideo);
	//current screen info
	int _width, _height;
	bool _setUp;

	float _left, _right, _bottom, _top;
	int _nl;

	char _blend;
	char _xalign;
	char _yalign;
	char _xflip;
	char _yflip;

	// textures
	std::vector<ImageDescriptor*> _images;
	static bool LoadTexture(ImageDescriptor &id);
public:
	SINGLETON_METHODS(GameVideo);
	bool ChangeMode(const SDL_Rect &s);
	void SetViewport(float left, float top, float right, float bottom);
	void SetCoordSys(float x_left, float x_right,
					 float y_bottom, float y_top, int nl);

	bool LoadImage(ImageDescriptor &id);
	void DeleteImage(ImageDescriptor &id);

	// We need this because after changing the video mode, I _believe_ SDL
	// creates a new GL context, hence all textures must be reloaded.

	// This function deletes all the texture objects allocated through
	// LoadImage(), but does not remove the references to ImageDescriptor
	// objects.
	void UnloadImages();
	// This function will reload all textures.
	void ReloadImages();
	// This will remove all images.
	void DeleteImages();

	void SetDrawFlags(int firstflag, ...);
	void DrawImage(const ImageDescriptor &id);

	void Clear();
	void Render();

	void SelectLayer(int l);
	void Move(float tx, float ty);
	void MoveRel(float tx, float ty);
	void Rotate(float angle);
	void PushState();
	void PopState();
};

}

#endif // !_VIDEO_H_
