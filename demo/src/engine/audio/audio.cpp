///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
*** \file    audio.cpp
*** \author  Tyler Olsen - Mois�s Ferrer - Aaron Smith, roots@allacrost.org - byaku@allacrost.org - etherstar@allacrost.org
*** \brief   Implementation of the audio engine singleton.
***
*** The code included here implements the interface of the audio singleton.
***
*** \note This code uses the OpenAL audio library. See http://www.openal.com/
******************************************************************************/


#include "audio.h"
// #include "audio_music.h"
#include "system.h"

#include <iostream>

using namespace std;
using namespace hoa_utils;
using namespace hoa_system;
using namespace hoa_audio;
using namespace private_audio;




bool hoa_audio::private_audio::ALError ()
{
	return alGetError() != AL_NO_ERROR;

//	static ALenum error_code;
//	error_code = alGetError();
//	PrintALError (error_code);
//	return error_code != AL_NO_ERROR;
}

bool hoa_audio::private_audio::ALCError ()
{
	return alGetError() != AL_NO_ERROR;

//	static ALCenum error_code;
//	error_code = alcGetError (0);
//	PrintALError (error_code);
//	return error_code != ALC_NO_ERROR;
}

void hoa_audio::private_audio::PrintALError (ALenum error_code)
{
//	std::cout << GetALErrorString (error_code) << std::endl;
	GetALErrorString (error_code);
}

void hoa_audio::private_audio::PrintALCError (ALCenum error_code)
{
//	std::cout << GetALCErrorString (error_code) << std::endl;
	GetALCErrorString (error_code);
}



//! \brief Returns an error string corresponding to a particular AL error code.
/*!
	Returns an error string corresponding to a particular error code.
	\param error_code AL error code.
	\return String corresponding to the error code.
*/
const std::string hoa_audio::private_audio::GetALErrorString (ALenum error_code)
{
	switch (error_code)
	{
		case AL_INVALID_NAME:
			return "AL_INVALID_NAME";
		case AL_INVALID_ENUM:
			return "AL_INVALID_ENUM";
		case AL_INVALID_VALUE:
			return "AL_INVALID_VALUE";
		case AL_INVALID_OPERATION:
			return "AL_INVALID_OPERATION";
		case AL_OUT_OF_MEMORY:
			return "AL_OUT_OF_MEMORY";
		case AL_NO_ERROR:
			return "AL_NO_ERROR";
		default:
			return ("unknown AL error code: " + error_code);
	}
}


//! \brief Returns an error string corresponding to a particular ALC error code.
/*!
	Returns an error string corresponding to a particular ALC error code.
	\param error_code ALC error code.
	\return String corresponding to the error code.
*/
const std::string hoa_audio::private_audio::GetALCErrorString (ALCenum error_code)
{
	switch (error_code)
	{
		case ALC_INVALID_DEVICE:
			return "ALC_INVALID_DEVICE";
		case ALC_INVALID_CONTEXT:
			return "ALC_INVALID_CONTEXT";
		case ALC_INVALID_ENUM:
			return "ALC_INVALID_ENUM";
		case ALC_INVALID_VALUE:
			return "ALC_INVALID_VALUE";
		case ALC_OUT_OF_MEMORY:
			return "ALC_OUT_OF_MEMORY";
		case ALC_NO_ERROR:
			return "ALC_NO_ERROR";
		default:
			return ("unknown ALC error code: " + error_code);
	}
}


template<> hoa_audio::GameAudio* Singleton<hoa_audio::GameAudio>::_singleton_reference = 0;

namespace hoa_audio {

GameAudio* AudioManager = NULL;
bool AUDIO_DEBUG = false;



GameAudio::GameAudio () :
_sound_volume (1.0f),
_music_volume (1.0f),
_device (0),
_context (0),
_max_sources (64)
{}




bool GameAudio::SingletonInitialize ()
{
	const ALCchar *best_device = 0;		// Name of the 'best' device
	ALCint highest_version = 0;			// Highest version number found
	
	// Find the highest-version device available
	const ALCchar *device_name = alcGetString (0, ALC_DEVICE_SPECIFIER);	// Get list of all devices (terminated with two '0')
	if (!ALCError ())
	{
		while (*device_name != 0)	// Check all the detected devices
		{
			ALCint major_v=0, minor_v=0;
			ALCdevice *_temp_device = alcOpenDevice(device_name); // Open a temporary device for reading in its version number

			if (ALCError () || !_temp_device)	 // Just keep looping if we couldn't open the device
			{
				if (AUDIO_DEBUG)
				{
					std::cout << "AUDIO: Couldn't open a temporary device: " <<  device_name << std::endl;	
				}
				device_name += strlen(device_name) + 1;
				continue;
			}

			ALCcontext *temp_context = alcCreateContext(_temp_device, 0);	// Create a temporary context

			if (ALCError() || !temp_context)	// Keep looping again if no context was created
			{
				if (AUDIO_DEBUG)
				{
					std::cout << "AUDIO: Couldn't create a temporary context for device: " <<  device_name << std::endl;
				}
				alcCloseDevice(_temp_device);
				device_name += strlen(device_name) + 1;
				continue;
			}

			alcMakeContextCurrent(temp_context);
			ALCError ();

			alcGetIntegerv(_temp_device, ALC_MAJOR_VERSION, sizeof(ALCint), &major_v); // Get the version number
			ALCError ();
			alcGetIntegerv(_temp_device, ALC_MINOR_VERSION, sizeof(ALCint), &minor_v);
			ALCError ();
			alcMakeContextCurrent(0); // Disable the temporary context
			ALCError ();
			alcDestroyContext(temp_context); // Destroy the temporary context
			ALCError ();
			alcCloseDevice(_temp_device); // Close the temporary device
			ALCError ();
			if (highest_version < (major_v * 10 + minor_v))		// Check if we found a higher version
			{
				highest_version = (major_v * 10 + minor_v);
				best_device = device_name;
			}
			device_name += strlen(device_name) + 1; // Go to the next device name in the list
		}
	}

	// Open the 'best' device we found above. If could not found anything, will try opening the default one (0)
	_device = alcOpenDevice(best_device);
	if (ALCError () || !_device)
	{
		std::cerr << "AUDIO ERROR: Failed to initialize OpenAL and open a default device." << std::endl;
		return false;
	}

	// Create an OpenAL context
	_context = alcCreateContext(_device, NULL);
	if (ALCError () || !_context)
	{
		std::cerr << "AUDIO ERROR: Failed to create an OpenAL context." << std::endl;
		alcCloseDevice(_device);
		ALCError ();
		return false;
	}
	alcMakeContextCurrent(_context);
	ALCError ();

	// Create as many sources as possible (we fix an upper bound of _max_sources)
	ALuint source;
	for (int i(0); i<_max_sources; i++)
	{
		alGenSources (1, &source);
		if (ALError ())
		{
			_max_sources = i;
			break;
		}
		_source.push_back (new private_audio::SoundSrc);
		_source.back()->source = source;
		_source.back()->source_pos = i;
	}

	return true;
}


//! \brief Destructor, responsible for cleaning up.
/*!
	Clean up resources.
*/
GameAudio::~GameAudio ()
{
	// Stop and delete the sources
	for (std::vector <private_audio::SoundSrc*>::iterator it=_source.begin(); it<_source.end(); it++)
	{
		if ((*it)->IsValid())
		{
			(*it)->Stop ();
			alDeleteSources (1, &(*it)->source);
			ALError ();
		}
	}
	
	// Delete the buffers
	for (std::vector <private_audio::SoundBuffer*>::iterator it=_buffer.begin(); it<_buffer.end(); it++)
	{
		if ((*it)->IsValid())
		{
			alDeleteBuffers (1, &(*it)->buffer);
			ALError ();
		}
	}

	// Delete the persistant sounds.
	for (std::map<std::string, SoundDescriptor*>::iterator it = _persistantSounds.begin();
		it != _persistantSounds.end(); it++)
	{
		if (it->second != 0)
		{
			it->second->FreeSound(); // Likely redundant due to deletion of sources/buffers, but can't hurt.
			delete (*it).second;
			_persistantSounds[it->first] = 0;
		}
	}
	_persistantSounds.clear();

	alcMakeContextCurrent (0);
	ALCError ();
	alcDestroyContext (_context);
	ALCError ();
	alcCloseDevice (_device);
	ALCError ();
}


// FIXME: What is this?
GameAudio::GameAudio (const GameAudio &game_audio)
{
}

//! \brief Updates the audio information.
/*!
	Updates the audio information, such as updating the streaming buffers.
*/
void GameAudio::Update ()
{
//	alcSuspendContext (_context);

	for (std::vector <private_audio::SoundSrc*>::iterator it=_source.begin(); it<_source.end(); it++)
	{
		if ((*it)->attached)
		{
			(*it)->owner->Update ();
		}
	}

	// Update the effects
	_fx_manager.Update ();

//	alcProcessContext (_context);
}


void GameAudio::PlaySound(const std::string& filename) {
	SoundDescriptor* new_sound = new SoundDescriptor();

//	Apparently, LoadSound returns void, so we'll try doing it this way.
	new_sound->LoadSound(filename);
/*	if (new_sound->LoadSound(filename) == false) {
		if (AUDIO_DEBUG)
			cerr << "AUDIO ERROR: GameAudio::PlaySound() failed because the sound file could not be loaded" << endl;
		delete(new_sound);
	}
*/

//	new_sound->PlaySound();
	new_sound->Play();
//	_temp_sounds.push_front(new_sound);
}


//! \brief Gets the sound volume.
/*!
	Gets the sound volume.
	\return The sound volume.
*/
float GameAudio::GetSoundVolume () const
{
	return _sound_volume;
}


//! \brief Gets the music volume.
/*!
	Gets the music volume.
	\return The music volume.
*/
float GameAudio::GetMusicVolume () const
{
	return _music_volume;
}


//! \brief Sets the sound volume.
/*!
	Sets the sound volume. This volume will modulate every sound volume.
	\param volume Sets the sound volume, in the range [0.0,1.0].
*/
void GameAudio::SetSoundVolume (const float volume)
{
	_sound_volume = (volume < 0.0f) ? 0.0f : ( (volume>1.0f) ?  1.0f : volume);

	for (std::list<SoundDescriptor*>::iterator it=_sound.begin(); it!=_sound.end(); it++)
	{
		(*it)->SetVolume((*it)->GetVolume());
	}
}


//! \brief Sets the music volume.
/*!
	Sets the music volume. This volume will modulate every music volume.
	\param volume Sets the music volume, in the range [0.0,1.0].
*/
void GameAudio::SetMusicVolume (const float volume)
{
	_music_volume = (volume < 0.0f) ? 0.0f : ( (volume>1.0f) ?  1.0f : volume);

	for (std::list<MusicDescriptor*>::iterator it=_music.begin(); it!=_music.end(); it++)
	{
		(*it)->SetVolume((*it)->GetVolume());
	}
}


//! \brief Pauses all sounds and music.
/*!
	Pauses all sounds and music.
*/
void GameAudio::PauseAudio ()
{
	PauseAllSounds ();
	PauseAllMusic ();
}


//! \brief Resumes all sounds and music.
/*!
	Resumes all sounds and music.
*/
void GameAudio::ResumeAudio ()
{
	ResumeAllSounds ();
	ResumeAllMusic ();
}


//! \brief Stops all sounds and music.
/*!
	Stops all sounds and music.
*/
void GameAudio::StopAudio ()
{
	StopAllSounds ();
	StopAllMusic ();
}


//! \brief Rewinds all sounds and music.
/*!
	Rewinds all sounds and music.
*/
void GameAudio::RewindAudio ()
{
	RewindAllSounds ();
	RewindAllMusic ();
}


//! \brief Pauses all sounds.
/*!
	Pauses all sounds.
*/
void GameAudio::PauseAllSounds ()
{
	for (std::list<SoundDescriptor*>::iterator it=_sound.begin(); it!=_sound.end(); it++)
	{
		(*it)->Pause ();
	}
}


//! \brief Resumes all sounds.
/*!
	Resumes all sounds.
*/
void GameAudio::ResumeAllSounds ()
{
	for (std::list<SoundDescriptor*>::iterator it=_sound.begin(); it!=_sound.end(); it++)
	{
		(*it)->Resume ();
	}
}


//! \brief Stops all sounds.
/*!
	Stops all sounds.
*/
void GameAudio::StopAllSounds ()
{
	for (std::list<SoundDescriptor*>::iterator it=_sound.begin(); it!=_sound.end(); it++)
	{
		(*it)->Stop ();
	}
}


//! \brief Rewinds all sounds.
/*!
	Rewinds all sounds.
*/
void GameAudio::RewindAllSounds ()
{
	for (std::list<SoundDescriptor*>::iterator it=_sound.begin(); it!=_sound.end(); it++)
	{
		(*it)->Rewind ();
	}
}


//! \brief Pauses all music.
/*!
	Pauses all music.
*/
void GameAudio::PauseAllMusic ()
{
	for (std::list<MusicDescriptor*>::iterator it=_music.begin(); it!=_music.end(); it++)
	{
		(*it)->Pause ();
	}
}


//! \brief Resumes all music.
/*!
	Resumes all music.
*/
void GameAudio::ResumeAllMusic ()
{
	for (std::list<MusicDescriptor*>::iterator it=_music.begin(); it!=_music.end(); it++)
	{
		(*it)->Resume ();
	}
}


//! \brief Stops all music.
/*!
	Stops all music.
*/
void GameAudio::StopAllMusic ()
{
	for (std::list<MusicDescriptor*>::iterator it=_music.begin(); it!=_music.end(); it++)
	{
		(*it)->Stop ();
	}
}


//! \brief Rewinds all music.
/*!
	Rewinds all music.
*/
void GameAudio::RewindAllMusic ()
{
	for (std::list<MusicDescriptor*>::iterator it=_music.begin(); it!=_music.end(); it++)
	{
		(*it)->Rewind ();
	}
}


//! \brief Sets the listener position (3D).
/*!
	Sets the listener position.
	\param position Array containing the position information.
*/
void GameAudio::SetListenerPosition (const float position[3])
{
	alListenerfv (AL_POSITION, position);
	ALError ();
	memcpy (_listener_position, position, sizeof(float)*3);
}


//! \brief Sets the listener velocity (3D).
/*!
	Sets the listener vevlocity.
	\param velocity Array containing the velocity information.
*/
void GameAudio::SetListenerVelocity (const float velocity[3])
{
	alListenerfv (AL_VELOCITY, velocity);
	ALError ();
	memcpy (_listener_velocity, velocity, sizeof(float)*3);
}


//! \brief Sets the listener orientation (3D).
/*!
	Sets the listener orientation.
	\param orientation Array containing the orientation information.
*/
void GameAudio::SetListenerOrientation (const float orientation[3])
{
	alListenerfv (AL_ORIENTATION, orientation);
	ALError ();
	memcpy (_listener_orientation, orientation, sizeof(float)*3);
}


//! \brief Gets the listener position (3D).
/*!
	Gets the listener position.
	\param position Array to hold the position information.
*/
void GameAudio::GetListenerPosition (float position[3]) const
{
	memcpy (position, _listener_position, sizeof(float)*3);
}


//! \brief Gets the listener velocity (3D).
/*!
	Gets the listener velocity.
	\param velocity Array to hold the velocity information.
*/
void GameAudio::GetListenerVelocity (float velocity[3]) const
{
	memcpy (velocity, _listener_velocity, sizeof(float)*3);
}


//! \brief Gets the listener orientation (3D).
/*!
	Gets the listener orientation.
	\param orientation Array to hold the orientation information.
*/
void GameAudio::GetListenerOrientation (float orientation[3]) const
{
	memcpy (orientation, _listener_orientation, sizeof(float)*3);
}


//! \brief Prints information about that audio settings on the user's machine.
/*!
	Prints information about that audio settings on the user's machine.
*/
void GameAudio::DEBUG_PrintInfo()
{
	const ALCchar* c;

	std::cout << "*** Audio Information ***" << std::endl;

	std::cout << "Maximum number of sources:   " << _max_sources << std::endl;

	std::cout << "Available audio devices:     " << alcGetString (_device, ALC_DEVICE_SPECIFIER) << std::endl;
	ALCError ();
	
	std::cout << "Default audio device:        " << alcGetString (_device, ALC_DEFAULT_DEVICE_SPECIFIER) << std::endl;
	ALCError ();
	
	std::cout << "OpenAL Version:              " << alGetString (AL_VERSION) << std::endl;
	ALError ();
	
	std::cout << "OpenAL Renderer:             " << alGetString (AL_RENDERER) << std::endl;
	ALError ();
	
	std::cout << "OpenAL Vendor:               " << alGetString (AL_VENDOR) << std::endl;
	ALError ();

	std::cout << "OpenAL Available Extensions:" << std::endl;
	c = alGetString (AL_EXTENSIONS);
	ALError ();
	bool new_extension = true;
	while (c[0])
	{
		if (new_extension)
		{
			std::cout << "- ";
			new_extension = false;
			continue;
		}
		else if (c[0] == ' ')
		{
			std::cout << std::endl;
			new_extension = true;
			c++;
			continue;
		}
		
		std::cout << c[0];
		c++;
	}
}

void GameAudio::PlayPersistantSound(const std::string soundName)
{
	// Get a reference 
	SoundDescriptor * sound = 0;
	std::map<std::string, SoundDescriptor*>::iterator it = _persistantSounds.find(soundName);
	if (it == _persistantSounds.end())
	{
		// No sound was found in the map, create a new sound and add it to the
		// persistant sounds map.
		sound = new SoundDescriptor();
		sound->LoadSound(soundName, hoa_audio::AUDIO_LOAD_STREAM_MEMORY);
		_persistantSounds[soundName] = sound;
	}
	else
	{
		// The sound was in the map, get a reference to the pointer.
		sound = it->second;
	}

	// Play the sound.
	sound->SetLooping(false);
	sound->Play();
}


//! \brief Provides an available source, if there is one.
/*!
	Provides an available source, if any.
	\return A pointer to the available source.
	\todo Add an algoihtm to give priority to some sounds/music over others.
*/
private_audio::SoundSrc* GameAudio::_AcquireSoundSrc ()
{
	for (std::vector<private_audio::SoundSrc*>::iterator it=_source.begin(); it<_source.end(); it++)
	{
		if (!(*it)->attached)
		{
			(*it)->attached = true;
			return *it;
		}
	}

	return 0;
}

//! \brief Initialization of static member data.
std::map<std::string, SoundDescriptor*> GameAudio::_persistantSounds;

} // namespace hoa_audio
