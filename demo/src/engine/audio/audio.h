///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    audio.h
*** \author  Tyler Olsen - Moisés Ferrer Serra - Aaron Smith, roots@allacrost.org - byaku@allacrost.org - etherstar@allacrost.org
*** \brief   Header file for audio engine interface.
***
*** This code provides an easy-to-use API for managing all music and sounds used
*** in the game.
***
*** \note This code uses the OpenAL audio library. See http://www.openal.com/
*** ***************************************************************************/

#ifndef __AUDIO_HEADER__
#define __AUDIO_HEADER__

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
#include "audio_effects.h"

//! \brief All related audio engine code is wrapped within this namespace
namespace hoa_audio {

//! \brief The singleton pointer responsible for all audio operations.
extern GameAudio* AudioManager;

//! \brief Determines whether the code in the hoa_audio namespace should print debug statements or not.
extern bool AUDIO_DEBUG;

namespace private_audio {

//! \brief The maximum default number of audio sources that the engine tries to create
const uint16 MAX_DEFAULT_AUDIO_SOURCES = 64;

} // namespace private_audio

/** ****************************************************************************
*** \brief A singleton class that manages all audio related data and operations
***
*** This class is provided as a singleton so it is conveniently accesible where needed. Here
*** there is internal control of the audio device and available sources. Interfaces for the 2
*** main groups (sound and music) are available. Also the listener funtionality is provided
*** from here.
***
*** \note Make sure to later resume paused sounds, otherwise the sources that they hold
*** will never be released
*** ***************************************************************************/
class GameAudio : public hoa_utils::Singleton<GameAudio> {
	friend class hoa_utils::Singleton<GameAudio>;
// friend class private_audio::SoundData;
// friend class private_audio::MusicData;
	friend class AudioDescriptor;
	friend class SoundDescriptor;
	friend class MusicDescriptor;
	friend class Effects;

public:
	~GameAudio();

	/** \brief Opens all audio libraries and initializes the audio device, context, and states
	*** \return True if there were no errors during initialization
	**/
	bool SingletonInitialize();

	//! \brief Updates various parts of the audio state, such as streaming buffers
	void Update();

	float GetSoundVolume() const
		{ return _sound_volume; }

	float GetMusicVolume() const
		{ return _music_volume; }

	/** \brief Sets the global volume level for all sounds
	*** \param volume The sound volume level to set. The valid range is: [0.0 (mute), 1.0 (max volume)]
	**/
	void SetSoundVolume(float volume);

	/** \brief Sets the global volume level for all music
	*** \param volume The music volume level to set. The valid range is: [0.0 (mute), 1.0 (max volume)]
	**/
	void SetMusicVolume(float volume);

	/** \name Global Audio State Manipulation Functions
	*** \brief Performs specified operation on all sounds and music.
	***
	*** These functions will only effect audio data that is in the state(s) specified below:
	*** - PlayAudio()     <==>   all states but the playing state
	*** - PauseAudio()    <==>   playing state
	*** - ResumeAudio()   <==>   paused state
	*** - StopAudio()     <==>   all states but the stopped state
	*** - RewindAudio()   <==>   all states
	**/
	//@{
	void PauseAudio()
		{ PauseAllSounds(); PauseAllMusic(); }

	void ResumeAudio()
		{ ResumeAllSounds(); ResumeAllMusic(); }

	void StopAudio()
		{ StopAllSounds(); StopAllMusic(); }

	void RewindAudio()
		{ RewindAllSounds(); RewindAllMusic(); }
	//@}

	/** \name Global Sound State Manipulation Functions
	*** \brief Performs specified operation on all sounds
	**/
	//@{
	void PauseAllSounds();
	void ResumeAllSounds();
	void StopAllSounds();
	void RewindAllSounds();
	//@}

	/** \name Global Sound State Manipulation Functions
	*** \brief Performs specified operation on all sounds
	*** Since there is only one music source, these functions only affect that source.
	*** They are equivalent to calling the {Pause/Resume/Stop/Rewind}Music functions on
	*** the MusicDescriptor which currently has posession of the source.
	**/
	//@{
	void PauseAllMusic();
	void ResumeAllMusic();
	void StopAllMusic();
	void RewindAllMusic();
	//@}

	/** \name Three Dimensional Audio Properties Functions
	*** \brief Used to manipulate the shared 3D state members that all sounds share
	*** Refer to the OpenAL documentation to understand what effect each of these
	*** properties have (listener position, velocity, and orientation).
	*/
	//@{
	void SetListenerPosition(const float position[3]);
	void SetListenerVelocity(const float velocity[3]);
	void SetListenerOrientation(const float orientation[3]);
	
	void GetListenerPosition(float position[3]) const
		{ memcpy(position, _listener_position, sizeof(float) * 3); }

	void GetListenerVelocity(float velocity[3]) const
		{ memcpy(velocity, _listener_velocity, sizeof(float) * 3); }

	void GetListenerOrientation(float orientation[3]) const
		{ memcpy(orientation, _listener_orientation, sizeof(float) * 3); }
	//@}

	//! \name Audio Effect Functions
	//@{
	/** \brief Fades a music or sound in as it plays
	*** \param audio A reference to the music or sound to fade in
	*** \param time The amount of time that the fade should last for, in seconds
	**/
	void FadeIn(AudioDescriptor& audio, float time)
		{ _audio_effects.push_back(new private_audio::FadeInEffect(audio, time)); }

	/** \brief Fades a music or sound out as it finisheds
	*** \param audio A referenece to the music or sound to fade out
	*** \param time The amount of time that the fade should last for, in seconds
	**/
	void FadeOut(AudioDescriptor& audio, float time)
		{ _audio_effects.push_back(new private_audio::FadeOutEffect(audio, time)); }
	//@}

	/** \brief Plays a sound once with no looping
	*** \param filename The name of the sound file to play
	*** This method of playback is useful because it doesn't require any SoundDescriptor
	*** objects to be managed by the user. This is ideal for the case of scripts which
	*** wish to play a sound only once. The sound is loaded (if necessary) into the
	*** sound cache and played from there.
	**/
	void PlaySound(const std::string& filename);

	/** \name Error Detection and Processing methods
	*** Code external to the audio engine should not need to make use of the following methods,
	*** as error detection is routinely done by the engine itself.
	**/
	//@{
	/** \brief Retrieves the OpenAL error code and retains it in the _al_error_code member
	*** \return True if an OpenAL error has been detected, false if no errors were detected
	**/
	bool CheckALError()
		{ _al_error_code = alGetError(); return (_al_error_code != AL_NO_ERROR); }

	/** \brief Retrieves the OpenAL context error code and retains it in the _alc_error_code member
	*** \return True if an OpenAL context error has been detected, false if no errors were detected
	**/
	bool CheckALCError()
		{ _alc_error_code = alcGetError(_device); return (_alc_error_code != ALC_NO_ERROR); }

	ALenum GetALError()
		{ return _al_error_code; }

	ALCenum GetALCError()
		{ return _alc_error_code; }

	///! \brief Returns a string representation of the most recently fetched OpenAL error code
	const std::string CreateALErrorString();

	//! \brief Returns a string representation of the most recently fetched OpenAL context error code
	const std::string CreateALCErrorString();
	//@}

	//! \brief Prints information about the audio properties and settings of the user's machine
	void DEBUG_PrintInfo();

private:
	//! \note Constructors are kept private since this class is a singleton
	//@{
	GameAudio();
	GameAudio(const GameAudio &game_audio);
	//@}

	//! \brief The global volume level of all sounds (0.0f is mute, 1.0f is max)
	float _sound_volume;

	//! \brief The global volume level of all music (0.0f is mute, 1.0f is max)
	float _music_volume;

	//! \brief The OpenAL device currently being utilized by the audio engine
	ALCdevice* _device;

	//! \brief The current OpenAL context that the audio engine is using
	ALCcontext* _context;

	//! \brief Holds the most recently fetched OpenAL error code
	ALenum _al_error_code;

	//! \brief Holds the most recently fetched OpenAL context error code
	ALCenum _alc_error_code;

	//! \brief Contains all available audio sources
	std::vector<private_audio::AudioSource*> _source;

	//! \brief Contains the maximum number of available audio sources that can exist simultaneously
	uint16 _max_sources;

	//! \brief The listener properties used by audio which plays in a multi-dimensional space
	//@{
	float _listener_position[3];
	float _listener_velocity[3];
	float _listener_orientation[3];
	//@}

	//! \brief Holds all active audio effects
	std::list<private_audio::AudioEffect*> _audio_effects;

	//! \brief Lists of pointers to all audio descriptor objects which have been created by the user
	//@{
	std::list<SoundDescriptor*> _sound;
	std::list<MusicDescriptor*> _music;
	//@}

	/** \brief A LRU cache of sounds which are managed internally by the audio engine
	*** The purpose of this cache is to allow the user to quickly and easily play
	*** sounds without having to maintain a SoundDescriptor object in memory. This is
	*** used, for example, by script functions which simply want to play a sound to
	*** indicate an action or event has occurred. 
	***
	*** The sound cache is a LRU (least recently used) structure, meaning that if an
	*** entry needs to be evicted or replaced to make room for another, the least
	*** recently used sound is deleted from the cache.
	**/
	std::map<std::string, SoundDescriptor*> _sound_cache;

	/** \brief Acquires an available audio source that may be used
	*** \return A pointer to the available source, or NULL if no available source could be found
	*** \todo Add an algoihtm to give priority to some sounds/music over others.
	**/
	private_audio::AudioSource* _AcquireAudioSource();
}; // class GameAudio : public hoa_utils::Singleton<GameAudio>

} // namespace hoa_audio

#endif // __AUDIO_HEADER__
