#include "utils.h"
#include <cassert>
#include <cstdarg>
#include "video.h"
#include <math.h>
#include "coord_sys.h"
#include "gui.h"

using namespace std;
using namespace hoa_video::local_video;

namespace hoa_video 
{

// time between screen shake updates in milliseconds
const int VIDEO_TIME_BETWEEN_SHAKE_UPDATES = 50;  

//-----------------------------------------------------------------------------
// ShakeScreen: shakes the screen with a given force and shake method
//              If you want it to shake until you manually stop it, then
//              pass in VIDEO_FALLOFF_NONE and 0.0f for falloffTime
//-----------------------------------------------------------------------------

bool GameVideo::ShakeScreen(float force, float falloffTime, ShakeFalloff falloffMethod)
{
	// check inputs
	if(force < 0.0f)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: passed negative force to ShakeScreen()!" << endl;
		return false;
	}

	if(falloffTime < 0.0f)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: passed negative falloff time to ShakeScreen()!" << endl;
		return false;
	}

	if(falloffMethod <= VIDEO_FALLOFF_INVALID || falloffMethod >= VIDEO_FALLOFF_TOTAL)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: passed invalid shake method to ShakeScreen()!" << endl;
		return false;
	}
	
	if(falloffTime == 0.0f && falloffMethod != VIDEO_FALLOFF_NONE)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ShakeScreen() called with 0.0f (infinite), but falloff method was not VIDEO_FALLOFF_NONE!" << endl;
		return false;
	}
		
	// create the shake force structure
	
	int milliseconds = int(falloffTime * 1000);
	ShakeForce s;
	s.currentTime  = 0;
	s.endTime      = milliseconds;
	s.initialForce = force;
	
	
	// set up the interpolation
	switch(falloffMethod)
	{
		case VIDEO_FALLOFF_NONE:
			s.interpolator.SetMethod(VIDEO_INTERPOLATE_SRCA);
			s.interpolator.Start(force, 0.0f, milliseconds);
			break;
		
		case VIDEO_FALLOFF_EASE:
			s.interpolator.SetMethod(VIDEO_INTERPOLATE_EASE);
			s.interpolator.Start(0.0f, force, milliseconds);
			break;
		
		case VIDEO_FALLOFF_LINEAR:
			s.interpolator.SetMethod(VIDEO_INTERPOLATE_LINEAR);
			s.interpolator.Start(force, 0.0f, milliseconds);
			break;
			
		case VIDEO_FALLOFF_GRADUAL:
			s.interpolator.SetMethod(VIDEO_INTERPOLATE_SLOW);
			s.interpolator.Start(force, 0.0f, milliseconds);
			break;
			
		case VIDEO_FALLOFF_SUDDEN:
			s.interpolator.SetMethod(VIDEO_INTERPOLATE_FAST);
			s.interpolator.Start(force, 0.0f, milliseconds);
			break;
		
		default:
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: falloff method passed to ShakeScreen() was not supported!" << endl;
			return false;
		}		
	};
	
	// add the shake force to GameVideo's list
	_shakeForces.push_front(s);
	
	return true;
}


//-----------------------------------------------------------------------------
// StopShaking: removes ALL shaking on the screen
//-----------------------------------------------------------------------------

bool GameVideo::StopShaking()
{
	_shakeForces.clear();
	_shakeX = _shakeY = 0.0f;
	return true;
}


//-----------------------------------------------------------------------------
// IsShaking: returns true if the screen has any shake effect applied to it
//-----------------------------------------------------------------------------

bool GameVideo::IsShaking()
{
	return !_shakeForces.empty();
}


//-----------------------------------------------------------------------------
// RoundForce: rounds a force to an integer. Whether to round up or round down
//             is based on the fractional part. A force of 1.37 has a 37%
//             chance of being a 2, else it's a 1
//             This is necessary because otherwise a shake force of 0.5f
//             would get rounded to zero all the time even though there is some
//             force
//-----------------------------------------------------------------------------

float GameVideo::RoundForce(float force)
{
	int fractionPct = int(force * 100) - (int(force) * 100);
	
	int r = rand()%100;
	if(fractionPct > r)
		force = ceilf(force);
	else
		force = floorf(force);
		
	return force;
}


//-----------------------------------------------------------------------------
// UpdateShake: called once per frame to update the the shake effects
//              and update the shake x,y offsets
//-----------------------------------------------------------------------------

void GameVideo::UpdateShake(int frameTime)
{
	if(_shakeForces.empty())
	{
		_shakeX = _shakeY = 0;
		return;
	}

	// first, update all the shake effects and calculate the net force, i.e.
	// the sum of the forces of all the shakes
	
	float netForce = 0.0f;
	
	list<ShakeForce>::iterator iShake = _shakeForces.begin();
	list<ShakeForce>::iterator iEnd   = _shakeForces.end();
	
	while(iShake != iEnd)
	{
		ShakeForce &s = *iShake;
		s.currentTime += frameTime;

		if(s.endTime != 0 && s.currentTime >= s.endTime)
		{
			iShake = _shakeForces.erase(iShake);
		}
		else
		{
			s.interpolator.Update(frameTime);
			netForce += s.interpolator.GetValue();
			++iShake;	
		}
	}	

	// cap the max update frequency
	
	static int timeTilNextUpdate = 0;		
	timeTilNextUpdate -= frameTime;
	
	if(timeTilNextUpdate > 0)
		return;
	
	timeTilNextUpdate = VIDEO_TIME_BETWEEN_SHAKE_UPDATES;


	// now that we have our force (finally), calculate the proper shake offsets
	// note that this doesn't produce a radially symmetric distribution of offsets
	// but I think it's not noticeable so... :)
	
	_shakeX = RoundForce(RandomFloat(-netForce, netForce));
	_shakeY = RoundForce(RandomFloat(-netForce, netForce));	
}


}  // namespace hoa_video
