#ifndef __VIDEO_IMAGES_HEADER__
#define __VIDEO_IMAGES_HEADER__

#include "video_state.h"
#include "video.h"
#include <GL/gl.h>
#include <vector>
#include <list>
#include <string>

namespace hoa_video {

class ImagesVS;

class ImageDescriptor {
public:
	ImageDescriptor() : texid(0) {}
	std::string filename;
	float width, height;

private:
	mutable GLuint texid;
	friend class ImagesVS;
};

class ImagesVS : public IVideoState
{
private:
	static std::list<ImagesVS *> ivs;
	static void Allocate(bool allocate);
	static void LoadTexture(const ImageDescriptor &id);

	bool inVideo;
	std::vector<ImageDescriptor> images;
public:
	friend class GameVideo;
	ImagesVS();
	virtual ~ImagesVS();

	int LoadImage(const ImageDescriptor &id);

	static const int AlignLeft = -1;
	static const int AlignTop = -1;
	static const int AlignCenter = 0;
	static const int AlignRight = +1;
	static const int AlignBottom = +1;
	void DrawImage(int i, bool blend = false,
				   int xalign = AlignLeft, int yalign = AlignTop,
				   bool xflip = false, bool yflip = false);

	void LoadVideo();
	void UnloadVideo();

	void Select();
	void Deselect();
};

}

#endif
