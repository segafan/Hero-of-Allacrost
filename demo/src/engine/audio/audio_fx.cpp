////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   audio_fx.cpp
*** \author Moisés Ferrer Serra, byaku@allacrost.org
*** \brief  Source for audio effects system
*** 
*** This code provides the interface for effects, as well as the private classes
*** used for build all the audio fx system. This includes a class used as interface
*** for the users (an aailable class), and private classes for handling the
*** system.
***
*** \note This code is (audio) library independent
*** ***************************************************************************/


#include "audio_fx.h"
#include "audio.h"



using namespace hoa_audio;
using namespace private_audio;




//! \brief Constructor of the class, that initialized an effect as active
IAudioFx::IAudioFx () :
_active (true)
{
}


//! \brief Update function of an effect
/*!
	Update function for an effect. It might be needed for effects that need periodic 
	work during some time (as fading). We provide an empty implementation so adding new
	effects don't need to include an empty function.
*/
void IAudioFx::Update ()
{
}













//! \brief Updates the manager.
/*!
	Updates the manager. At this time, this involves calling all the updates of the active
	effects. Inactive effects will be deleted.
*/
void AudioFxManager::Update ()
{
	// Iterate over all the registered (active) effects
	for (std::list <IAudioFx*>::iterator it=_effects.begin(); it!=_effects.end(); )
	{
		(*it)->Update ();	// Update the effect

		// If the effect is over, unregister it
		if (!(*it)->_active)
		{
			it = _effects.erase (it);
		}
		else
		{
			it++;
		}
	}
}







//! \brief Constructor of the fade out effect.
/*!
	Constructor of the fade out effect.
	\param descriptor Audio descriptor of the sound to fade.
	\param time Time that the effect will take.
*/
FadeOutFx::FadeOutFx (AudioDescriptor &descriptor, const float time) :
IAudioFx ()
,_volume (descriptor.GetVolume())
,_time (time)
,_audio_descriptor (descriptor)
{
}


//! \brief Update of the fade in effect, for gradually decrease the volume.
/*!
	Updates the volume of the sound considering the elapsed time from the last update. If the sound
	is not playing, it returns. When the volume arrives to 0.0, it stops the sound.
*/
void FadeOutFx::Update ()
{
	// If the sound is not playing, assume the effect is over
	if (_audio_descriptor.GetState() != AUDIO_STATE_PLAYING)
	{
		_active = false;
		return;
	}

	float step = 0.00025f;
	float volume = _audio_descriptor.GetVolume() - (1.0f / _time) * step;

	// If volume <= 0, mark the sound as over
	if (volume <= 0.0f)
	{
		_audio_descriptor.Stop ();				// Stop the sound
		_audio_descriptor.SetVolume (_volume);	// Restore original volume
		_active = false;
	}
	else
	{
		_audio_descriptor.SetVolume (volume);
	}
}













//! \brief Constructor of the fade in effect.
/*!
	Constructor of the fade in effect.
	\param descriptor Audio descriptor of the sound to fade.
	\param time Time that the effect will take.
*/
FadeInFx::FadeInFx (AudioDescriptor &descriptor, const float time) :
IAudioFx ()
,_volume (descriptor.GetVolume())
,_time (time)
,_audio_descriptor (descriptor)
{
	if (_audio_descriptor.GetState() != AUDIO_STATE_PLAYING)
	{
		_audio_descriptor.SetVolume (0.0f);
		_audio_descriptor.Play ();
	}
}


//! \brief Update of the fade out effect, for gradually increase the volume.
/*!
	Updates the volume of the sound considering the elapsed time from the last update. If the sound
	is not playing, it returns. When the volume arrives to the original volume of the sound, it stops 
	the sound.
*/
void FadeInFx::Update ()
{
	// If the sound is not playing, assume the effect is over
	if (_audio_descriptor.GetState() != AUDIO_STATE_PLAYING)
	{
		_active = false;
		return;
	}

	float step = 0.00025f;
	float volume = _audio_descriptor.GetVolume() + (1.0f / _time) * step;

	// If the volume is over the original value, mark the effect as over
	if (volume >= _volume)
	{
		_audio_descriptor.SetVolume (_volume);	// Restore the original volume
		_active = false;
	}
	else
	{
		_audio_descriptor.SetVolume (volume);
	}
}






//! \brief Registers a fade in effect.
/*!
	Creates an effect of type FadeIn for fading in a sound and adds it to the Fx manager. The time 
	provided is the time that the process will take for bringing the volume from 1.0 to 0.0.
	\param descriptor Descriptor of the sound to fade in.
	\param time Time that the effect will last.
*/
void Effects::FadeIn (AudioDescriptor &descriptor, const float time)
{
	private_audio::IAudioFx* fx (0);
	fx = new private_audio::FadeInFx (descriptor, time);
	if (fx)
	{
		AudioManager->_fx_manager._effects.push_back(fx);
	}
}


//! \brief Registers a fade out effect.
/*!
	Creates an effect of type FadeOut for fading out a sound and adds it to the Fx manager. The time 
	provided is the time that the process will take for bringing the volume from 0.0 to the 1.0.
	\param descriptor Descriptor of the sound to fade out.
	\param time Time that the effect will last.
*/
void Effects::FadeOut (AudioDescriptor &descriptor, const float time)
{
	private_audio::IAudioFx* fx (0);
	fx = new private_audio::FadeOutFx (descriptor, time);
	if (fx)
	{
		AudioManager->_fx_manager._effects.push_back(fx);
	}
}


//! \brief Registers a cross fade effect.
/*!
	Creates an effect of type FadeIn and FadeOut and adds them to the Fx manager. The time 
	provided is the time that the process will take for both fades.
	\param descriptor_in Descriptor of the sound to fade in.
	\param descriptor_out Descriptor of the sound to fade out.
	\param time Time that the effect will last.
*/
void Effects::CrossFade (AudioDescriptor &descriptor_in, AudioDescriptor &descriptor_out, const float time)
{
	FadeIn (descriptor_in, time);
	FadeOut (descriptor_out, time);
}

