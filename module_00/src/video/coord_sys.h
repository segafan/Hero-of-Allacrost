/*
	===========================================================================
	CoordSys structure
	===========================================================================
	
	Holds a coordinate system
*/

#ifndef _COORD_SYS_HEADER_
#define _COORD_SYS_HEADER_

#include "utils.h"

struct CoordSys
{
	CoordSys() 
	{
		_left  = _top    = 0.0f;
		_right = _bottom = 1.0f;
		
		_layer  = 1;
	}
	
	CoordSys(float left, float right, float bottom, float top, int layer)
	{
		_left   = left;
		_right  = right;
		_bottom = bottom;
		_top    = top;
		_layer   = layer;
	}
	
	float _left;
	float _right;
	float _bottom;
	float _top;
	int   _layer;
};

#endif
