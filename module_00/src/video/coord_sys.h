#ifndef _COORD_SYS_H_
#define _COORD_SYS_H_

namespace hoa_video {

class CoordinateSystem {
public:
	virtual void Setup(void) const=0;
};

/*******************************************************************************
* sets up orthographic projection of the specified dimensions and provides
* commands to move through it. i cant be bothered to explain what that means
* right now so if you dont know then ask
*******************************************************************************/
class OrthographicCS : public CoordinateSystem
{
private:
	float l, r, b, t;
	int layers;
public:
	OrthographicCS(float left, float right, float bottom, float top, int nl);

	void Setup(void) const;
};

//eo namespace
}

#endif
