#ifndef __VIDEO_ORTHO_VIDEO_STATE_HEADER__
#define __VIDEO_ORTHO_VIDEO_STATE_HEADER__

#include "video_state.h"

namespace hoa_video {

/*******************************************************************************
* sets up orthographic projection of the specified dimensions and provides
* commands to move through it. i cant be bothered to explain what that means
* right now so if you dont know then ask
*******************************************************************************/
class OrthographicVS : public IVideoState
{
private:
	float width, height;
	int layers;
public:
	OrthographicVS(const float w, const float h, const int l);

	void Select();
	void Deselect();

	void SelectLayer(const int l) const;
	void Move(const float rx, const float ry) const;
	void Rotate(const float acAngle) const;
	void PushState() const;
	void PopState() const;
};

//eo namespace
}

#endif
