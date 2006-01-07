///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    audio.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: October 29th, 2005
 * \brief   Header file for audio engine interface.
 *
 * This code provides an easy-to-use API for managing all music and sounds used
 * in the game.
 *
 * \note This code uses the OpenAL audio library. See http://www.openal.com/
 *****************************************************************************/

#ifndef __AUDIO_HEADER__
#define __AUDIO_HEADER__

#include "utils.h"
#include "defs.h"
#include "audio_sound.h"
#include "audio_music.h"
#include <string>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

namespace hoa_audio {

extern GameAudio *AudioManager;
extern bool AUDIO_DEBUG;

//! \name Audio State Constants
//! \brief Used to determine what state a sound or music piece is in.
//@{
const uint8 AUDIO_STATE_UNLOADED            = 0x01;
const uint8 AUDIO_STATE_STOPPED             = 0x02;
const uint8 AUDIO_STATE_PAUSED              = 0x04;
const uint8 AUDIO_STATE_PLAYING             = 0x08;
//@}

//! \name Audio Distance Model Constants
//! \brief Used to set the distance model used by OpenAL.
//! \note The default distance model is AUDIO_DISTANCE_INVERSE_CLAMPED
//@{
const uint8 AUDIO_DISTANCE_NONE             = 0x01;
const uint8 AUDIO_DISTANCE_LINEAR           = 0x02;
const uint8 AUDIO_DISTANCE_LINEAR_CLAMPED   = 0x04;
const uint8 AUDIO_DISTANCE_INVERSE          = 0x08;
const uint8 AUDIO_DISTANCE_INVERSE_CLAMPED  = 0x10;
const uint8 AUDIO_DISTANCE_EXPONENT         = 0x20;
const uint8 AUDIO_DISTANCE_EXPONENT_CLAMPED = 0x40;
//@}

//! \name Audio Error Codes
//@{
//! \brief These are error codes that the API user can query and handle as they wish.
//! \note The default distance model is AUDIO_INVERSE_DISTANCE_CLAMPED
const uint32 AUDIO_NO_ERRORS                  = 0x00000000;
const uint32 AUDIO_OUT_OF_MEMORY              = 0x00000001;
const uint32 AUDIO_INVALID_OPERATION          = 0x00000002;
//! Indicates that too many sounds are being played concurrently.
const uint32 AUDIO_SOURCE_OVERUSAGE           = 0x00000000;
const uint32 AUDIO_SOURCE_ACQUISITION_FAILURE = 0x00000000;
//@}

namespace private_audio {

//! \name {Sound/Music}Descriptor Property Constants
//@{
//! \brief Constants used interally to check whether a property is in a default state or not.
const uint16 SOURCE_BAD                = 0x0000;
const uint16 SOURCE_OK                 = 0x0001;
const uint16 SOURCE_LOOP               = 0x0002;
const uint16 SOURCE_GAIN               = 0x0004;
const uint16 SOURCE_PITCH              = 0x0008;
const uint16 SOURCE_MIN_GAIN           = 0x0010;
const uint16 SOURCE_MAX_GAIN           = 0x0020;
const uint16 SOURCE_MAX_DISTANCE       = 0x0040;
const uint16 SOURCE_REFERENCE_DISTANCE = 0x0080;
const uint16 SOURCE_ROLLOFF_FACTOR     = 0x0100;
const uint16 SOURCE_RELATIVE           = 0x0200;
const uint16 SOURCE_CONE_INNER_ANGLE   = 0x0400;
const uint16 SOURCE_CONE_OUTER_ANGLE   = 0x0800;
const uint16 SOURCE_CONE_OUTER_GAIN    = 0x1000;
//@}

//! Converts the OpenAL enum error codes into a string.
std::string GetALErrorString(ALenum err);
//! Converts the OpenALC enum error codes into a string.
std::string GetALCErrorString(ALenum err);

/*!****************************************************************************
 * \brief An internal class used for retaining audio state information.
 *
 * When a game mode is made to be the new active game mode of the stack, sometimes
 * we will wish to retain information about the audio state. This is so that when
 * we restore the previously active state again, the audio can resume as if no interruption
 * had occured.
 *
 * This class takes a snapshot of the audio state and saves the following information:
 *  - The listener properties
 *  - The attenutation distance model
 *  - Which sources are assigned to which buffers
 *  - The source properties
 *  - The position of each audio source that was playing when the call was made.
 *****************************************************************************/
class AudioState {
public:
	AudioState();
	~AudioState();
private:
	ALenum _distance_model;
	ALfloat _listener_gain;
	ALfloat _listener_position[3];
	ALfloat _listener_velocity[3];
	ALfloat _listener_orientation[6];
	friend class GameAudio;
};

} // namespace private_audio

/*!****************************************************************************
 * \brief A singleton class for managing and interfacing with audio data.
 *
 * This class manages all audio data allocation and manipulation. The OpenAL sources
 * are wrapped inside this class and OpenAL buffers (which are represented by the
 * SoundDescriptor and MusicObject classes) grab these sources as they need them. The
 * buffers are stored in map structures so that audio data is not loaded when it already
 * exists.
 *
 * \note 1) Operations that load audio data should be done during parts of the game
 * when game modes are being created and destroyed. In other words, ideally you should
 * load data into SoundSource and MusicSource objects when a new game mode class object is created,
 * instead of creating them only immediately before they are needed.
 *
 * \note 2) This audio engine uses smart memory management so that loaded audio
 * data is not re-loaded if the user requests a load operation on the same
 * data. Audio data is only freed once there are no more references to the
 * data.
 *****************************************************************************/
class GameAudio {
	friend class AudioState;
	friend class private_audio::SoundBuffer;
	friend class private_audio::SoundSource;
	friend class SoundDescriptor;
	friend class private_audio::MusicBuffer;
	friend class private_audio::MusicSource;
	friend class MusicDescriptor;
private:
	SINGLETON_DECLARE(GameAudio)

	//! The audio device opened and being operated on by OpenAL.
	ALCdevice *_device;
	//! The OpenAL context using the device.
	ALCcontext *_context;

	//! The volume (gain) for the music source. Valid range is between 0.0f and 1.0f.
	float _music_volume;
	//! The volume (gain) for all sound sources. Valid range is between 0.0f and 1.0f.
	float _sound_volume;

	//! Retains all the errors that have occured on audio-related function calls, except for loading errors.
	uint32 _audio_errors;

	//! \name Containers for Audio Data
	//! \brief STL maps are used to hold both music and sounds.
	//@{
	std::map<std::string, private_audio::MusicBuffer*> _music_buffers;
	std::map<std::string, private_audio::SoundBuffer*> _sound_buffers;
	//@}

	//! The single source reserved for game music.
	private_audio::MusicSource *_music_source;
	//! All of the sources that are reserved for sound data.
	std::list<private_audio::SoundSource*> _sound_sources;

	// //! A map of the audio states we have saved. Probably will never be more than four or five elements.
	// std::map<uint32, private_audio::AudioState> _saved_states;

	/*! \name Buffer Retrieval Functions
	 *  \brief Creates and loads new buffer data if the data is not already loaded.
	 *  \param filename The filename of the file data to search for (not including the pathname or file extension).
	 *  \return A pointer to the class object holding the new data. NULL may also be returned, indicating an error.
	 *
	 *  These functions are critical to ensuring efficient memory usage in the audio engine (in other words:
	 *  making sure no more than one copy of audio data is loaded into the engine at any given time). When these
	 *  functions are called, first the map of buffer objects is searched to see if the data is already found
	 *  in there. If it is, a pointer to that object is returned. Otherwise, the function will attempt to create
	 *  a new object and store that into the appropriate buffer object map. If that fails for some reason, the function
	 *  will return NULL.
	 */
	//@{
	private_audio::SoundBuffer* _AcquireSoundBuffer(std::string filename);
	private_audio::MusicBuffer* _AcquireMusicBuffer(std::string filename);
	//@}

	/*! \name Source Acquisition Functions
	 *  \brief Retrieves an audio source that may be used for
	 *  \return A pointer to a sound source that may be allocated. NULL may also be returned, indicating an error.
	 */
	//@{
	private_audio::SoundSource* _AcquireSoundSource();
	private_audio::MusicSource* _AcquireMusicSource();
	//@}

	/*! \name Source Release Functions
	 *  \brief Releases an audio source from being allocated to a descriptor object.
	 *  \param free_source A pointer to the audio source to free.
	 */
	//@{
	void _ReleaseSoundSource(private_audio::SoundSource* free_source);
	void _ReleaseMusicSource(private_audio::MusicSource* free_source);
	//@}

public:
	SINGLETON_METHODS(GameAudio);

	/*! \brief Updates all streaming audio queues.
	 *
	 *  The purpose of this function is to refill buffers that are part of a streaming audio source. It
	 *  is vital to prevent the player from hearing jumps or skips in the audio
	 *
	 *  \note This function is only called from one location: the main game loop. It should not be
	 *  called anywhere else.
	 */
	void Update();

	/*! \brief Returns a set of error codes and also clears the error code to a no-error state.
	 *
	 *  This is the standard CheckErrors() function as defined in the Allacrost code standard.
	 *  The error code constants are listed at the top of this file in the Audio Error Codes
	 *  group.
	 */
	uint32 CheckErrors()
		{ uint32 return_code; return_code = _audio_errors; _audio_errors = AUDIO_NO_ERRORS; return return_code; }

	/*! \name Volume Member Access Functions
	 *  \brief Used for reading and modifying the volume of music/sound in the game.
	 *
	 *  These volume changes have a global effect (they modify the volume of all music/sound sources).
	 *  There is currently no support to change the volume levels of individual sound or music
	 *  sources. However, one can achieve the same effect by modifying the source properties and (in the
	 *  case of single-channel audio) manipulating distance attenuation.
	 *
	 *  \note In OpenAL, the gain property of sources is essentially the volume.
	 *
	 *  \note In addition to these functions, you can also change the gain (volume) of the listener, which
	 *  will effect the volume level of \c all audio heard in the game.
	 */
	//@{
	float GetMusicVolume()
		{ return _music_volume; }
	float GetSoundVolume()
		{ return _sound_volume; }
	void SetMusicVolume(float vol);
	void SetSoundVolume(float vol);
	//@}

	/*! \name Global Audio Manipulation Functions
	 *  \brief Performs specified operation on all active sounds and music.
	 *
	 *  These functions will only effect audio data that is in the state(s) specified below:
	 *  PauseAudio()    <==>   playing state
	 *  ResumeAudio()   <==>   paused state
	 *  StopAudio()     <==>   playing state
	 *  RewindAudio()   <==>   playing state, paused state
	 */
	//@{
	void PauseAudio();
	void ResumeAudio();
	void StopAudio();
	void RewindAudio();
	//@}

	/*! \name Global Sound Manipulation Functions
	 *  \brief Performs specified operation on all active sounds.
	 */
	//@{
	//! \note Make sure to resume these sounds, otherwise the sources that they hold will never be released!
	void PauseAllSounds();
	void ResumeAllSounds();
	void StopAllSounds();
	void RewindAllSounds();
	//@}

	/*! \name Global Music Manipulation Functions
	 *  \brief Performs specified operation on all active music.
	 *
	 *  Since there is only one music source, these functions only affect that source. They are equivalent
	 *  to calling the {Pause/Resume/Stop/Rewind}Music functions on the MusicDescriptor which currently
	 *  has posession of the source.
	 */
	//@{
	void PauseAllMusic();
	void ResumeAllMusic();
	void StopAllMusic();
	void RewindAllMusic();
	//@}

	/*! \name Listener Property Access Functions
	 *  \brief Functions used for reading and  modifying the properties of the listener.
	 */
	//@{
// 	void GetListenerPosition(float[3] pos)
// 		{ alGetListenerfv(AL_POSITION, pos); }
// 	void GetListenerVelocity(float[3] vel)
// 		{ alGetListenerfv(AL_VELOCITY, vel); }
// 	float GetListenerGain(float gain)
// 		{ float g; alGetListenerf(AL_GAIN, &g); return g; }
// 	void GetListenerOrientation(float[6] ori)
// 		{ alGetListenerfv(AL_ORIENTATION, ori); }
// 	void SetListenerPosition(float[3] pos)
// 		{ alListenerfv(AL_POSITION, pos); }
// 	void SetListenerVelocity(float[3] vel)
// 		{ alListenerfv(AL_VELOCITY, vel); }
// 	void SetListenerGain(float gain)
// 		{ alListenerf(AL_GAIN, gain); }
// 	void SetListenerOrientation(float[6] ori)
// 		{ alListenerfv(AL_ORIENTATION, ori); }
	//@}

	/*! \brief Gets a value indicating what distance model is currently being used by OpenAL.
	 *  \return A value representing the distance model. Refer to the Audio Distance Model Constants.
	 */
	uint8 GetDistanceModel();
	/*! \brief Changes the distance model that will be used by OpenAL.
	 *  \param model A value representing the distance model. Refer to the Audio Distance Model Constants.
	 *  \note The new distance model will immediately take effect after this call.
	 */
	void SetDistanceModel(uint8 model);



	/*! \brief Saves the audio state onto an internal stack.
	 *  \return
	 *  The state data that is saved includes: properties of the listener, the distance model used for
	 *  attenuation, and which SoundDescriptors and MusicObjects are allocated to which sources.
	 */
	// uint32 SaveAudioState();
	//! Restores the audio state properties from the stack.
	//! \param state_id The identification number of the state to retrieve.
	// void RestoreAudioState(uint32 state_id);

	//! Prints information related to the system's audio capabilities as seen by OpenAL.
	void DEBUG_PrintInfo();
}; // class GameAudio

} // namespace hoa_audio

#endif
