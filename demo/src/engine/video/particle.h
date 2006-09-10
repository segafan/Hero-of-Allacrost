///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    particle.h
 * \author  Raj Sharma, roos@allacrost.org
 * \brief   Header file for particle data
 *
 * This file contains structure(s) for representing a single particle. In theory,
 * we should be able to just have one structure which holds all properties for
 * a particle, but in fact we have two: one for the position, and one for everything
 * else. The reason is that having the positions separated is more efficient
 * for rendering.
 *****************************************************************************/

#ifndef __PARTICLE_HEADER__
#define __PARTICLE_HEADER__

#include "utils.h"
#include "color.h"
#include "particle_keyframe.h"


namespace hoa_video
{

namespace private_video
{


/*!***************************************************************************
 *  \brief this is used in the vertex array for DrawArrays(). Every time
 *         the particle system is rendered, we need to iterate through all the
 *         particles in the system, and use the position, size, and rotation
 *         to generate 4 ParticleVertex's. This is pretty expensive, but
 *         unfortunately it's necessary since the positions change every frame.
 *****************************************************************************/

class ParticleVertex
{
public:

	//! position of 1 vertex of the particle's quad
	float _x;
	float _y;
};


/*!***************************************************************************
 *  \brief this is used in texture coordinate array for DrawArrays()
 *         Unless animated particles are used, this can be generated just once
 *****************************************************************************/

class ParticleTexCoord
{
public:

	//! texture coordinates for 1 vertex of the particle's quad
	float _t0;
	float _t1;
};


/*!***************************************************************************
 *  \brief this is the structure we use to represent a particle
 *****************************************************************************/


class Particle
{
public:

	//! position
	float  _x;
	float  _y;
	
	//! size
	float  _size_x;
	float  _size_y;
	
	//! velocity
	float  _velocity_x;
	float  _velocity_y;
	
	// store the combined velocity (particle + wind + wave) so we only have
	// to calculate it once
	float _combined_velocity_x;
	float _combined_velocity_y;
	
	//! color
	Color  _color;
		
	//! current rotation angle
	float  _rotation_angle;
	
	//! rotation speed
	float  _rotation_speed;
	
	//! seconds since particle was spawned
	float  _time;
	
	//! lifetime (when the particle is supposed to die)
	float  _lifetime;
	
	//! this is 2 * pi / wavelength. The reason we store this weird
	//! number instead of the wavelength is because that's what we
	//! will ultimately plug into the sin function
	float _wave_length_coefficient;	
	
	//! half the amplitude of the wave. We store half the amplitude
	//! instead of the whole amplitude because that's what gets multiplied
	//! with the sin function
	float _wave_half_amplitude;

	//! acceleration, i.e. change in velocity per second. The most common use
	//! for this is for simulating gravity. If you have multiple constant
	//! forces acting on particles, then this vector should be the sum of
	//! those forces.
	float _acceleration_x;
	float _acceleration_y;
	
	//! tangential acceleration- just like normal acceleration, except it
	//! is applied in the tangent direction. positive = clockwise.
	float _tangential_acceleration;
	
	//! radial acceleration- acceleration towards (negative) or away (positive)
	//! from an attractor. Note that the default attractor is the emitter position.
	//! The client can set an attractor for the entire effect by calling
	//! ParticleEffect::SetAttractor(x,y)
	float _radial_acceleration;
	
	//! wind velocity. this gets added to the particle's velocity each frame.
	//! note that different particles might also have a slightly different wind
	//! velocity, if the system has some wind velocity variation	
	float _wind_velocity_x;
	float _wind_velocity_y;
	
	//! damping- the particle's velocity gets multiplied by this value each second.
	//! So for example, a damping of .6 means that a particle slows down by 40% each
	//! second.
	float _damping;	
	
	// when a particle is created, it is given a rotation direction: either
	// 1 (clockwise) or -1 (counterclockwise)
	float _rotation_direction;
		
	// property variations	
	float _current_size_variation_x;
	float _current_size_variation_y;
	float _next_size_variation_x;
	float _next_size_variation_y;	
	float _current_rotation_speed_variation;
	float _next_rotation_speed_variation;	
	Color _current_color_variation;
	Color _next_color_variation;
	
	// keep track of current and next keyframes
	ParticleKeyframe *_current_keyframe;
	ParticleKeyframe *_next_keyframe;
};

}
}

#endif  //!__PARTICLE_HEADER__
