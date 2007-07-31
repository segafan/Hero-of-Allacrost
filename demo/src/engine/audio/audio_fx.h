////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   audio_fx.h
*** \author Moisés Ferrer Serra, byaku@allacrost.org
*** \brief  Header file for audio effects
*** 
*** This code provides the interface for effects, as well as the private classes
*** used for build all the audio fx system
***
*** \note This code is (audio) library independent
*** ***************************************************************************/


#ifndef __AUDIO_FX_HEADER__
#define __AUDIO_FX_HEADER__

#ifdef __MACH__
	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>
#else
	#include "al.h"
	#include "alc.h"
#endif

#include "defs.h"
#include "utils.h"

#include "audio_descriptor.h"

namespace hoa_audio
{

namespace private_audio
{

//! \brief Interface for Effect objects.
/*!
	The purpose for this class is to be the base for all Fx classes. Therefore it is possible
	to keep all of them in the FxManager easily, when needed. Then an Update can be called for 
	all of them.
*/
class IAudioFx
{
public:
	bool _active;	//!< \brief Flag for indicating that an effect becames inactive.

public:
	IAudioFx ();
	virtual ~IAudioFx () {  };
	virtual void Update ();
};






//! \brief Audio effects manager for updating the effects.
/*!
	The manager holds effects and calls them when its update function is called.
*/
class AudioFxManager 
{
public:
	std::list <IAudioFx*> _effects;	//!< \brief List of registered effects.

public:
	void Update ();
};







//! \brief Fade in effect.
/*!
	Fade in effect. It brings a sound from 0.0 to the current volume of the sound. It sets the the volume to 0.0
	and starts playing the sound (in case it is not et playng).
*/
class FadeInFx : public IAudioFx
{
private:
	float _volume;		//!< \brief Volume of the sound when registered.
	float _time;		//!< \brief Time that the effect takes.
	AudioDescriptor& _audio_descriptor;		//!< \brief Descriptor of the sound to modify.

public:
	FadeInFx (AudioDescriptor &descriptor, const float time);
	void Update ();
};







//! \brief Fade out effect.
/*!
	Fade out effect. It brings a sound from the current volume of the sound to 0.0. It sets the the volume to the 
	initial volume at the end. If the sound is not playing, the effect is not applied. When the sound reaches 0.0,
	it is stopped and the volume is restored.
*/
class FadeOutFx : public IAudioFx
{
private:
	float _volume;		//!< \brief Volume of the sound when registered.
	float _time;		//!< \brief Time that the effect takes.
	AudioDescriptor& _audio_descriptor;		//!< \brief Descriptor of the sound to modify.

public:
	FadeOutFx (AudioDescriptor &descriptor, const float time);
	void Update ();
};



}	// End of namespace private_audio



//! \brief Class for accessing the effects
/*!
	This class gives the user an easy interface for access the effects. All the effects should
	be called from a function in here.
*/
class Effects
{
public:
	static void FadeIn (AudioDescriptor &descriptor, const float time);
	static void FadeOut (AudioDescriptor &descriptor, const float time);
	static void CrossFade (AudioDescriptor &descriptor_in, AudioDescriptor &descriptor_out, const float time);
};


}	// End of namespace hoa_audio


#endif
