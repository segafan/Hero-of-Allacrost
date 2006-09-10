///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    particle_system.h
 * \author  Raj Sharma, roos@allacrost.org
 * \brief   Header file for particle system
 *
 * This file contains two classes: ParticleSystemDef, and ParticleSystem.
 *
 * ParticleSystemDef is a "definition" class, meaning that it holds information 
 * about a particle system, like its lifetime, the emitter, and other properties.
 *
 * ParticleSystem is an "instance" class, meaning that it holds information about
 * a particle system which is currently being drawn on screen.
 *
 * This way, if you have 100 explosions for example, the properties of the
 * effect are stored only once, and the only thing that gets repeated 100 times
 * is instance-specific stuff like positions of vertices, etc.
 *****************************************************************************/

#ifndef __PARTICLE_SYSTEM_HEADER__
#define __PARTICLE_SYSTEM_HEADER__

#include "utils.h"
#include "particle.h"
#include "particle_emitter.h"
#include "video.h"

namespace hoa_video
{

namespace private_video
{

class ParticleKeyframe;


/*!***************************************************************************
 *  \brief when we change a property of an effect, it affects all of the
 *         systems contained within that effect. So, this structure contains
 *         any relevant parameters that particle systems need to know about.
 *****************************************************************************/

class EffectParameters
{
public:

	//! orientation of the effect, called with ParticleEffect::SetOrientation()
	float orientation;
	
	//! attraction point, particles gravitate towards this
	float attractor_x, attractor_y;
};


class ParticleSystemDef
{
public:


	//! Is this system supposed to be displayed
	bool _enabled;
	
	//! Each system contains 1 emitter, which mainly determines where to shoot particles out from
	//! and how fast to shoot them out
	ParticleEmitter _emitter;
	
	//! Array of keyframes, which specify how particle properties vary over time. This array must
	//! contain at least 1 keyframe (in that case, the properties are all held constant)
	std::vector <ParticleKeyframe *> _keyframes;

	//! How to blend the particles: VIDEO_NO_BLEND, VIDEO_BLEND, or VIDEO_BLEND_ADD
	//! For most effects, we want VIDEO_BLEND_ADD
	int32 _blend_mode;
	
	//! How many seconds the system should live for before it dies out. This is only
	//! meaningful if our emitter mode is EMITTER_MODE_ONE_SHOT. The other modes
	//! will simply keep playing until the effect is destroyed or stopped, except for
	//! EMITTER_MODE_BURST, which spits out a bunch of particles at the beginning and
	//! then dies as soon as all of those particles die
	float _system_lifetime;
	
	//! How long each particle should live for before it dies	
	float _particle_lifetime;
	
	//! Random variation added to particle lifetime
	float _particle_lifetime_variation;
	
	//! Maximum number of particles this system can have at one time
	int32 _max_particles;
	
	//! A number below 1.0 (but generally pretty close to 1.0). A damp of .99 means that
	//! each second, particle velocity drops by 1%
	float _damping;	
	
	//! Random variation added to damping
	float _damping_variation;
	
	//! constant acceleration, good example is gravity. Note that down is in the positive y
	//! direction since we are using screen coordinates
	float _acceleration_x;
	float _acceleration_y;
	
	float _acceleration_variation_x;
	float _acceleration_variation_y;
	
	//! wind velocity, more generally any velocity which is added to each particle's velocity
	float _wind_velocity_x;
	float _wind_velocity_y;
	
	//! wind velocity variation
	float _wind_velocity_variation_x;
	float _wind_velocity_variation_y;
		
	//! true if we should use wave motion for this system
	bool  _wave_motion_used;
	
	//! wavelength. For example a wavelength of 5 means that it takes 5 seconds to go from
	//! one point on the sinusoidal curve to the next
	float _wave_length;
	
	//! Random variation added to wave length
	float _wave_length_variation;
	
	//! wave amplitude- the distance from the peak to the bottom of the sinusoidal curve
	float _wave_amplitude;
	
	//! Random variation added to wave amplitude
	float _wave_amplitude_variation;
		
	//! tangential acceleration, how particle accelerates tangential to vector from particle
	//! to center of emitter region. Positive is clockwise.
	float _tangential_acceleration;
	
	//! tangential acceleration variation
	float _tangential_acceleration_variation;
	
	//! radial acceleration. Positive means particles accelerate away from the emitter, negative
	//! means they accelerate back towards it	
	float _radial_acceleration;
	
	//! radial acceleration variation
	float _radial_acceleration_variation;

	//! if true, we use a user defined attractor instead of the emitter position for radial
	//! acceleration. The user defined attractor is set using ParticleEffect::SetAttractorPoint()
	bool _user_defined_attractor;
	
	//! how quickly the "pull" of an attractor falls off as a particle gets further away from it.
	//! For example if falloff is 10^-3, and a particle is 500 pixels away from the attractor, 
	//! then the radial acceleration is lessened by (500 * 10^-3) = .5, or 50%
	float _attractor_falloff;	
			
	//! True if ANY of the keyframes for the particles contain non-zero rotations
	//! This is used by the Draw() function so it knows whether it needs to factor in
	//! rotations when determining the particle vertices (since this adds a lot of extra
	//! computation)
	bool _rotation_used;	
	
	//! True if you want particles to rotate to face the same direction they are going.
	//! So for example a particle going straight up will not be rotated, but a particle
	//! that is going 45 degrees northeast will be rotated 45 degrees clockwise
	bool _rotate_to_velocity;
	
	
	//! true if speed scaling is used. Note that speed scaling can only be used if
	//! rotate to velocity is also used.
	bool _speed_scale_used;
	
	
	//! this number is multiplied by a particle's speed to come up with a scale of how much
	//! to stretch a particle in its direction of motion
	float _speed_scale;
	

	//! if you use speed_scale, this can cause particles to become extremely tiny at low speeds
	//! this variable allows you to set a minimum bound on the scaling due to speed
	float _min_speed_scale;


	//! if you use speed_scale, this can cause particles to become extremely large at high speeds
	//! this variable allows you to set a maximum bound on the scaling due to speed
	float _max_speed_scale;

	
	//! True if alpha blending should be used to create smooth transitions between animation
	//! frames
	bool _smooth_animation;

	//! True if this system should not actually write to the screen, but instead modify the
	//! stencil buffer. Every time a pixel passes the alpha test, the stencil buffer will be
	//! modified, according to _stencil_op
	bool _modify_stencil;
	
	//! if modify stencil is true, then the operation to use when the alpha test passes
	//! can either be VIDEO_STENCIL_OP_INCREASE, VIDEO_STENCIL_OP_ONE, VIDEO_STENCIL_OP_ZERO, or
	//! VIDEO_STENCIL_OP_DECREASE. The stencil test we use is "equal to 1"
	
	VIDEO_STENCIL_OP _stencil_op;
	
	
	//! if this is true, then we only draw in areas where the stencil buffer contains a 1.
	//! Note that _use_stencil and _modify_stencil cannot both be 1 at the same time	
	bool _use_stencil;


	//! Value from 0 to 1.0f, tells how strongly to modulate the particle colors by the scene
	//! lighting. If 0, then it's not weighted at all. If 1, then we multiply the particle color
	//! by the scene lighting. If it's somewhere between 0 and 1, then we calculate a color which
	//! is somewhere between white and the scene lighting color, and modulate with that.
	float _scene_lighting;

	
	//! true if particles' initial angle should be randomized. If false, then all particles
	//! have an angle of zero when they spawn
	bool _random_initial_angle;

	
	//! Array telling how long each animation should last for
	std::vector <int32>       _animation_frame_times;
	

	//! Array of filenames for each frame of animation
	std::vector <std::string> _animation_frame_filenames;
};



class ParticleSystem
{
public:

	ParticleSystem();
	

	/*!
	 *  \brief initializes this particle system as an instance of the
	 *         type of particle system specified by the ParticleSystemDef
	 */		
	bool Create(const ParticleSystemDef *sys_def);


	/*!
	 *  \brief draws the system
	 */	
	bool Draw();
	

	/*!
	 *  \brief updates the system
	 */		
	bool Update(float frame_time, const EffectParameters &params);
	
	
	/*!
	 *  \brief destroys the system
	 */		
	void Destroy();


	/*!
	 *  \brief returns true if system is still alive
	 */		
	bool IsAlive() const;
	

	/*!
	 *  \brief returns true if system has been stopped by a call to Stop()
	 */		
	bool IsStopped() const;
	
	
	/*!
	 *  \brief stops the system, i.e. makes it stop emitting new particles
	 */		
	void Stop();


	/*!
	 *  \brief returns how many particles are alive in this system
	 */	
	int32 GetNumParticles() const;


	/*!
	 *  \brief returns the number of seconds since this system was created
	 */	
	float GetAge() const;

private:


	/*!
	 *  \brief helper function to update properties of particles
	 */	
	void _UpdateParticles(float t, const EffectParameters &params);
	

	/*!
	 *  \brief helper function to kill off any particles that have died
	 *
	 *  \param num_particles this is an optimization that lets us avoid
	 *         killing particles. Killing particles is expensive, because
	 *         it leaves holes in our array, so we have to shuffle data
	 *         around to fill those holes. So instead, say we have 10
	 *         particles to kill this frame, and we also want to emit 8
	 *         particles. Then, instead of killing 10 particles, we only
	 *         kill 2, and for the other 8, we respawn them immediately.
	 */		
	void _KillParticles(int32 &num_particles, const EffectParameters &params);
	
	
	/*!
	 *  \brief helper function that emits whatever particles still need to be
	 *         emitted after calling _KillParticles
	 */		
	void _EmitParticles(int32 num_particles, const EffectParameters &params);
	

	/*!
	 *  \brief helper function to move a particle from element src to element dest
	 *         in the array. This is required any time we kill a particle, because
	 *         killing particles leaves a hole in the array
	 */	
	void _MoveParticle(int32 src, int32 dest);
	
	
	/*!
	 *  \brief creates a new particle at element i in the particle array
	 */		
	void _RespawnParticle(int32 i, const EffectParameters &params);


	//! The system definition, contains information like the emitter properties, lifetime of
	//! particles, particle keyframes, etc. Basically everything which isn't instance-specific
	const ParticleSystemDef *_system_def;

	//! Animation for each particle. If it's non-animated, it just has 1 frame
	hoa_video::AnimatedImage _animation;

	//! Total number of particles that this system can have
	int32 _max_particles;
	
	//! Number of active particles in this system. (The size of the vectors may be larger, since
	//! we might set a particle quota for the system which is higher than what's actually there.)
	int32 _num_particles;

	//! The array of particle vertices. Note that this array contains FOUR vertices per particle.
	//! This is used for rendering the particles with OpenGL
	std::vector <ParticleVertex>   _particle_vertices;
	std::vector <Color>            _particle_colors;
	std::vector <ParticleTexCoord> _particle_texcoords;
	
	//! This array holds everything except positions and colors. The reason we keep positions and
	//! colors separate is so that they can be efficiently fed to OpenGL for rendering.
	std::vector <Particle>     _particles;
	
	//! if stopped is true, no new particles should be emitted
	bool _stopped;
	
	//! alive gets set to false when the number of active particles drops to zero
	bool _alive;
	
	//! age of the system, since it was created
	float _age;
	
	//! last time the system was updated (based on the system's age)
	float _last_update_time;
};

}  // namespace private_video
}  // namespace hoa_video

#endif
