///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    audio_sound.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \brief   Header file for sound-related code in the audio engine.
 *
 * The classes in this file are used for management and processing of all sound
 * (WAV) data.
 *
 * \note This code uses the OpenAL audio library. See http://www.openal.com/
 *****************************************************************************/

#ifndef __AUDIO_SOUND_HEADER__
#define __AUDIO_SOUND_HEADER__

#include "utils.h"
#include "defs.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

namespace hoa_audio {

namespace private_audio {

/*!****************************************************************************
 * \brief An internal class used to manage sound data information.
 *
 * This class serves as a wrapper to raw audio data information loaded by OpenAL.
 * Objects of this class are managed internally by the GameAudio class and are
 * never seen by the user.
 *
 * \note 1) There is no need to save the filename of the buffer in this class,
 * because the filename is used as the look-up key in the map container of the
 * GameAudio class. See the documentation of that class for more detail.
 *
 * \note 2) Buffers with more than one channel are automatically treated as
 * ambient sounds by OpenAL (that is, no 3D spatialization).
 *****************************************************************************/
class SoundBuffer {
public:
	SoundBuffer(std::string fname);
	~SoundBuffer();

	//! The filename of the audio data the buffer holds.
	std::string filename;
	//! The number of SoundDescriptors referring to this buffer.
	uint8 reference_count;
	//! The buffer used by OpenAL
	ALuint buffer;

	//! Returns true if the OpenAL buffer is valid.
	bool IsValid();
	//! Removes a single reference to this buffer. If the reference count becomes zero, the buffer is destroyed.
	void RemoveReference();
	//! Displays the properties of the buffered data to standard output.
	void DEBUG_PrintProperties();
};

/*!****************************************************************************
 * \brief An internal class used to manage sound sources.
 *
 * Objects of this class are managed internally by the audio code and are
 * never exposed to the user. This class manages an OpenAL source. A source is
 * just as it sounds: a source from which sound eminates from.
 *
 * One of the tricky issues about OpenAL is that on some platforms (mainly Windows),
 * only a small number of sources can exist at the same time (usually between 16-64,
 * depending on the system and sound card in question).This means that sources need to
 * be shared among different audio buffers. Therefore when the audio engine is being
 * initialized, as many sources as possible are created.
 *
 *
 * \note 1) On Linux and other operating systems, the limit on simulatenous sources
 * is non-existant. Therefore, we limit the number of existing sources to 64 for all
 * cases.
 *****************************************************************************/
class SoundSource {
public:
	SoundSource();
	~SoundSource();

	//! The OpenAL source that this class object maintains.
	ALuint source;
	//! The properties that have been modified from their defaults.
	uint16 properties;
	//! A pointer to the SoundDescriptor that currently owns this source.
	SoundDescriptor *owner;

	//! Returns true if OpenAL determines that the source is valid.
	bool IsValid();
	//! Displays the properties of the sound source to standard output.
	void DEBUG_PrintProperties();
};

} // namespace private_audio

/*!****************************************************************************
 * \brief Manages sound data loaded from memory.
 *
 * The purpose of this class is to provide the API user with an easy-to-use
 * interface for manipulating sound data.This class holds all the properties
 * about a given sound. Including its location, velocity, whether or not it
 * loops, and numerous other properties.
 *****************************************************************************/
class SoundDescriptor {
	friend class GameAudio;
private:
	//! A pointer to the sound buffer that is used.
	private_audio::SoundBuffer *_data;
	//! A pointer to the sound source that is used.
	private_audio::SoundSource *_origin;

	bool _looping;
// 	float[3] _position;
// 	float[3] _velocity;

// 	// float _gain;
// 	float _pitch;
// 	float _min_gain;
// 	float _max_gain;
// 	float _max_distance;
// 	float _ref_distance;
// 	float _rolloff_factor;
// 	bool _relative;
// 	float _cone_inner_angle;
// 	float _cone_outer_angle;
// 	float _cone_outer_gain;

public:
	SoundDescriptor();
	~SoundDescriptor();
	//! Returns a const reference to the filename of the buffer that the source points to.
	const std::string &GetFilename()
		{ if (_data != NULL) return _data->filename; }
	/*! \brief Loads new sound data from a file.
	 *  \param fname The name of the file, without path information or a file extension.
	 *  \return Returns false if there was an error loading the sound.
	 *
	 *  If the sound file is already loaded in the game, this function will not load it again.
	 */
	bool LoadSound(std::string fname);

	/*! \brief Retrieves the state of the sound (unloaded, stopped, paused, or playing).
	 *  \return A value indicating the state. Refer to the audio state constants in audio.h.
	 */
	uint8 GetState();

	/*! \name Basic Sound Operations
	 *  \brief The five basic functions that can manipulate sound.
	 *  \note Calling these functions when the audio is in a certain state results in a no-op.
	 *  For example, calling PlaySound() when the sound is already playing.
	 */
	//@{
	void PlaySound();
	void PauseSound();
	void ResumeSound();
	void StopSound();
	void RewindSound();
	//@}

	/*! \brief Releases the sound source and removes a reference to the sound buffer data.
	 *  \note It's perfectly fine to call this function when a sound is playing. If that is
	 *  the case, the sound will first be stopped, and then freed.
	 */
	void FreeSound();

	/*! \name Looping Member Accessing Functions
	 *  \brief Allows user to activate or examine the ability for the sound to loop.
	 *  \note By default, a loaded sound will not loop.
	 */
	//@{
	void SetLooping(bool loop);
	bool IsLooping()
		{ return _looping; }
	//@}

	/*! \name Seeking Functions
	 *  \brief Seeks to the given seconds, samples, or bytes specified by the argument.
	 *
	 *  If the sound object is currently playing, these functions will seek to that position and
	 *  continue playing. Calling these functions when the sound object is not playing will begin playing
	 *  from that position on the next call to PlaySound(). Specifying a bad (out-of-range) value will do
	 *  nothing, but generates an error code.
	 */
	//@{
// 	void SeekSeconds(float seconds);
// 	void SeekSamples(float samples);
// 	void SeekBytes(float bytes);
	//@}

	/*! \brief Grabs a sound source for this object.
	 *
	 *  Usually you will not need to call this function. When you play a sound piece, a source is
	 *  automatically allocated for it. It is here only for optimization reasons, such as grabbing
	 *  sources for multiple sounds shortly before playing them so there is less overhead in
	 *  beginning to play a sound.
	 */
	void AllocateSource();
	/*! \brief Determines if the SoundDescriptor currently has position of a source.
	 *  \return True if the object has position of a source.
	 */
	bool HasSource()
		{ if (_origin == NULL) return false; else return true; }

	/*! \name Property Set Functions for Soundobjects
	 *  \brief Sets the various properties of a sound.
	 */
	//@{
// 	void SetPosition(float position[3])
// 		{ if (_origin != NULL) alSourcefv(_origin->source, AL_POSITION, position); }
// 	void SetVelocity(float velocity[3])
// 		{ if (_origin != NULL) alSourcefv(_origin->source, AL_VELOCITY, velocity); }
// 	void SetLoop(bool loop)
// 		{ if (_origin == NULL) return; ALint aloop; if (loop) aloop = AL_TRUE else aloop = AL_FALSE; alSourcei(_origin->source, AL_LOOPING, &aloop); }
// 	// void SetGain(float gain);
// 	void SetPitch(float pitch)
// 		{ if (_origin != NULL) alSourcef(_origin->source, AL_PITCH, &pitch); }
// 	void SetMinGain(float min_gain)
// 		{ if (_origin != NULL) alSourcef(_origin->source, AL_MIN_GAIN, &min_gain); }
// 	void SetMaxGame(float max_gain)
// 		{ if (_origin != NULL) alSourcef(_origin->source, AL_MAX_GAIN, &max_gain); }
// 	void SetMaxDistance(float max_distance)
// 		{ if (_origin != NULL) alSourcef(_origin->source, AL_MAX_DISTANCE, &max_distance); }
// 	void SetReferenceDistance(float ref_distance[3])
// 		{ if (_origin != NULL) alSourcefv(_origin->source, AL_REFERENCE_DISTANCE, ref_distance); }
// 	void SetRollOffFactor(float roll_factor)
// 		{ if (_origin != NULL) alSourcef(_origin->source, AL_ROLLOFF_FACTOR, &roll_factor); }
// 	void SetRelative(bool relative)
// 		{ if (_origin == NULL) return; ALint rel; if (relative) rel = AL_TRUE else rel = AL_FALSE; alSourcei(_origin->source, AL_LOOPING, &rel); }
// 	void SetConeInnerAngle(float angle)
// 		{ if (_origin != NULL) alSourcef(_origin->source, AL_CONE_INNER_ANGLE, &angle); }
// 	void SetConeOuterAngle(float angle)
// 		{ if (_origin != NULL) alSourcef(_origin->source, AL_CONE_OUTER_ANGLE, &angle); }
// 	void SetConeOuterGain(float gain)
// 		{ if (_origin != NULL) alSourcef(_origin->source, AL_CONE_OUTER_GAIN, &gain); }
	//@}



	//! Displays the properties of the sound descriptor's buffer.
	void DEBUG_dataProperties()
		{ if (_data != NULL) _data->DEBUG_PrintProperties(); }
	//! Displays the properties of the sound descriptor's source.
	void DEBUG_originProperties()
		{ if (_origin != NULL) _origin->DEBUG_PrintProperties(); }
}; // class SoundDescriptor

} // namespace hoa_audio

#endif
