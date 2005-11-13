///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    particle_manager.h
 * \author  Raj Sharma, roos@allacrost.org
 * \date    Last Updated: November 6th, 2005
 * \brief   Header file for particle manager
 *
 * The particle manager is very simple. Every time you want to draw an effect,
 * you call AddEffect() with a pointer to the effect definition structure.
 * Then every frame, call Update() and Draw() to draw all the effects.
 *****************************************************************************/

#ifndef __PARTICLE_MANAGER_HEADER__
#define __PARTICLE_MANAGER_HEADER__

#include "utils.h"


typedef int32 ParticleEffectID;

const ParticleEffectID VIDEO_INVALID_EFFECT = -1;

namespace hoa_video
{

class ParticleEffectDef;
class ParticleEffect;

namespace private_video
{

#include "utils.h"


/*!***************************************************************************
 *  \brief ParticleManager, used internally by video engine to store/update/draw 
 *         all particle effects.
 *****************************************************************************/

class ParticleManager
{
public:
	
	ParticleManager()  { _current_id = 0; }


	/*!
	 *  \brief loads an effect definition from a particle file
	 */		
	ParticleEffectDef *LoadEffect(const std::string &filename);

	
	/*!
	 *  \brief creates a new instance of an effect at (x,y), given its definition.
	 *         The effect is added to the internal std::map, _effects, and is now
	 *         included in calls to Draw() and Update()
	 */			
	ParticleEffectID AddEffect(const ParticleEffectDef *def, float x, float y);	


	/*!
	 *  \brief draws all active effects
	 */		
	bool Draw();


	/*!
	 *  \brief updates all active effects
	 */		
	bool Update(int32 frame_time);


	/*!
	 *  \brief stops all effects
	 *
	 *  \param kill_immediate If this is true, the effects are immediately killed. If
	 *                        it isn't true, then we stop the effects from emitting
	 *                        new particles, and allow them to live until all the active
	 *                        particles fizzle out.
	 */		
	void StopAll(bool kill_immediate = false);
	
	
	/*!
	 *  \brief Converts a particle effect id into a ParticleEffect pointer.
	 *         The pointers that this function returns are valid only up until
	 *         the next call to Update(), so they should never be stored. Just use
	 *         them for the current frame and then throw them away.
	 */			
	ParticleEffect *GetEffect(ParticleEffectID id);


	/*!
	 *  \brief returns the total number of particles among all active effects
	 */		
	int32 GetNumParticles();
	
	
	/*!
	 *  \brief destroys the system. Called by GameVideo's destructor
	 */			
	void Destroy();

private:

	//! Helper function to initialize a new ParticleEffect from its definition.
	//! Used by AddEffect()
	ParticleEffect *_CreateEffect(const ParticleEffectDef *def);

	//! All the effects currently being managed. An std::map is used so that
	//! we can convert easily between an id and a pointer
	std::map<ParticleEffectID, ParticleEffect *> _effects;
	
	//! The next time we create an effect, its id will be _current_id
	int32 _current_id;
	
	//! Total number of particles among all the active effects. This is updated
	//! during each call to Update(), so that when GetNumParticles() is called,
	//! we can just return this value instead of having to calculate it
	int32 _num_particles;
};

}  // namespace private_video
}  // namespace hoa_video

#endif // !__PARTICLE_MANAGER_HEADER
