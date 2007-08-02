////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   audio_descriptor.h
*** \author Mois�s Ferrer Serra, byaku@allacrost.org
*** \brief  Header file for audio descriptors, sources and buffers
***
*** This code provides the interface for the sound and music descriptors, that
*** are the units for load and manage sounds in the engine.
***
*** \note This code uses the OpenAL audio library. See http://www.openal.com/
*** ***************************************************************************/

#ifndef __AUDIO_DESCRIPTOR_HEADER__
#define __AUDIO_DESCRIPTOR_HEADER__

#ifdef __MACH__
	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>
#else
	#include "al.h"
	#include "alc.h"
#endif

#include "defs.h"
#include "utils.h"

#include "audio_input.h"
#include "audio_stream.h"

namespace hoa_audio {

//! \brief The set of states that AudioDescriptor class objects may be in
enum AUDIO_STATE {
	//! Audio data is not loaded
	AUDIO_STATE_UNLOADED   = 0,
	//! Audio is loaded, but is stopped
	AUDIO_STATE_STOPPED    = 1,
	//! Audio is loaded and is presently playing
	AUDIO_STATE_PLAYING    = 2,
	//! Audio is loaded and was playing, but is now paused
	AUDIO_STATE_PAUSED     = 3
};

//! \brief The possible ways for that a piece of audio data may be loaded
enum AUDIO_LOAD {
	AUDIO_LOAD_STATIC           = 0, //!< \brief Load sound statically.
	AUDIO_LOAD_STREAM_MEMORY    = 1, //!< \brief Load sound for streaming from memory.
	AUDIO_LOAD_STREAM_FILE      = 2 //!< \brief Load sound for streaming from file.
};

namespace private_audio {

//! \brief The default size for streaming buffers
const uint32 DEFAULT_BUFFER_SIZE = 16384;

/** ****************************************************************************
*** \brief Represents an OpenAL buffer
***
*** A buffer in OpenAL is simply a structure which contains raw audio data.
*** Buffers must be attached to an OpenAL source in order to play. OpenAL
*** suppports an infinte number of buffers (as long as there is enough memory).
*** ***************************************************************************/
class AudioBuffer {
	friend class GameAudio;

public:
	AudioBuffer();

	~AudioBuffer();

	/** \brief Fills an OpenAL buffer with raw audio data
	*** \param data A pointer to the raw data to fill the buffer with
	*** \param format The format of the buffer data (mono/stereo, 8/16 bits per sample)
	*** \param size The size of the data in number of bytes
	*** \param frequency The audio frequency of the data in samples per second
	**/
	void FillBuffer(uint8* data, ALenum format, uint32 size, uint32 frequency)
		{ alBufferData(buffer, format, data, size, frequency); }

	//! \brief Returns true if this class object holds a reference to a valid OpenAL buffer
	bool IsValid() const
		{ return alIsBuffer(buffer) == AL_TRUE; }

	//! \brief The ID of the OpenAL buffer
	ALuint buffer;
}; // class AudioBuffer


/** ****************************************************************************
*** \brief Represents an OpenAL source
***
*** A source in OpenAL is just what it sounds like it is: a source of audio
*** playback. Sources have their own set of properties like position, velocity,
*** etc. Those properties are not managed by this class, but rather by the
*** AudioDescriptor to which the source is attached. OpenAL (or rather, the
*** audio hardware) only allows a limited number of audio sources to exist at
*** one time, so we can't create a source for every piece of audio that is
*** loaded by the game. Therefore, we create as many sources as we can (up to
*** MAX_DEFAULT_AUDIO_SOURCES) and have the audio descriptors share between
*** sources as they need them.
***
*** \note OpenAL sources are created and by the GameAudio class, not within the
*** AudioSource constructor. The sources are, however, deleted by the destructor.
*** ***************************************************************************/
class AudioSource {
public:
	AudioSource() :
		source(0), owner(NULL) {}

	~AudioSource();

	//! \brief Returns true if this class object holds a reference to a valid OpenAL source
	bool IsValid() const
		{ return (alIsSource(source) == AL_TRUE); }

	//! \brief Resets the default properties of the OpenAL sources and removes the owner
	void Reset();

	//! \brief The ID of the OpenAL source
	ALuint source;

	//! \brief Pointer to the descriptor associated to this source.
	AudioDescriptor* owner;
}; // class AudioSource

} // namespace private_audio


//! \brief Class that provides the funcionality for managing sounds.
/*!
	Class that provides the funcionality for managing sounds. This includes the basic
	functionality such play, stop, pause, seeking, and also 3D functions.
*/
/** ****************************************************************************
*** \brief An abstract class for representing a piece of audio
***
*** This class takes the OpenAL buffer and source concepts and ties them
*** together. This class enables playback, streaming, 3D source positioning,
*** and many other features for manipulating a piece of audio. Sounds and
*** music are defined by classes which derive from this class.
***
*** \note Some features of this class are only available if the audio is loaded
*** in a streaming manner.
*** ***************************************************************************/
class AudioDescriptor {
	friend class GameAudio;

public:
	AudioDescriptor();

	virtual ~AudioDescriptor()
		{ FreeAudio(); }

	/** \brief Loads a new piece of audio data from a file
	*** \param filename The name of the file that contains the new audio data (should have a .wav or .ogg file extension)
	*** \param load_type The type of loading to perform (default == AUDIO_LOAD_STATIC)
	*** \param stream_buffer_size If the loading type is streaming, the buffer size to use (default == DEFAULT_BUFFER_SIZE)
	*** \return True if the audio was succesfully loaded, false if there was an error
	***
	*** The action taken by this function depends on the load type selected. For static sounds, a single OpenAL buffer is
	*** filled. For streaming, the file/memory is prepared.
	*/
	bool LoadAudio(const std::string& filename, AUDIO_LOAD load_type = AUDIO_LOAD_STATIC, uint32 stream_buffer_size = private_audio::DEFAULT_BUFFER_SIZE);

	//! \brief Frees all data resources and resets class parameters
	void FreeAudio();

	const std::string GetFilename() const
		{ if (_input == NULL) return ""; else return _input->GetFilename(); }

	//! \brief Returns true if this audio represents a sound, false if the audio represents a music piece
	virtual bool IsSound() const = 0;

	AUDIO_STATE GetState() const
		{ return _state; }

	/** \name Audio State Manipulation Functions
	*** \brief Performs specified operation on the audio
	***
	*** These functions will only take effect when the audio is in the state(s) specified below:
	*** - PlayAudio()     <==>   all states but the playing state
	*** - PauseAudio()    <==>   playing state
	*** - ResumeAudio()   <==>   paused state
	*** - StopAudio()     <==>   all states but the stopped state
	*** - RewindAudio()   <==>   all states
	**/
	//@{
	void Play();
	void Stop();
	void Pause();
	void Resume();
	void Rewind();
	//@}

	bool IsLooping() const
		{ return _looping; }

	/** \brief Enables/disables looping for this audio
	*** \param loop True to enable looping, false to disable it.
	**/
	void SetLooping(bool loop);

	/** \brief Sets the starting loop point, used for customized looping
	*** \param loop_start The sample position for the start loop point
	*** \note This function is only valid if the audio has been loaded with streaming support
	**/
	void SetLoopStart(uint32 loop_start);

	/** \brief Sets the ending loop point, used for customized looping
	*** \param loop_start The sample position for the end loop point
	*** \note This function is only valid if the audio has been loaded with streaming support
	**/
	void SetLoopEnd(uint32 loop_end);

	/** \brief Seeks to the requested sample position
	*** \param sample The sample position to seek to
	**/
	void SeekSample(uint32 sample);

	/** \brief Seeks to the requested playback time
	*** \param second The time to seek to, in seconds (e.g. 4.5f == 4.5 second mark)
	*** \note The position is aligned with a proper sample position, so the seek is not fully
	*** accurate.
	**/
	void SeekSecond(float second);

	//! \brief Returns the volume level for this audio
	float GetVolume() const
		{ return _volume; }

	/** \brief Sets the volume for this particular audio piece
	*** \param volume The volume level to set, ranging from [0.0f, 1.0f]
	**/
	virtual void SetVolume(float volume);

	/** \name Functions for 3D Spatial Audio
	*** These functions manipulate and retrieve the 3d properties of the audio. Note that only audio which
	*** are mono channel will be affected by these methods. Stereo channel audio will see no difference.
	**/
	//@{
	void SetPosition(const float position[3]);
	void SetVelocity(const float velocity[3]);
	void SetDirection(const float direction[3]);

	void GetPosition(float position[3]) const
		{ memcpy(&position, _position, sizeof(float) * 3); }

	void GetVelocity(float velocity[3]) const
		{ memcpy(&velocity, _velocity, sizeof(float) * 3); }

	void GetDirection(float direction[3]) const
		{ memcpy(&direction, _direction, sizeof(float) * 3); }
	//@}

	//! \brief Prints various properties about the audio data managed by this class
	void DEBUG_PrintInfo();

protected:
	//! \brief The current state of the audio (playing, stopped, etc.)
	AUDIO_STATE _state;

	//! \brief A pointer to the buffer(s) being used by the audio (1 buffer for static sounds, 2 for streamed ones)
	private_audio::AudioBuffer* _buffer;

	//! \brief A pointer to the source object being used by the audio
	private_audio::AudioSource* _source;

	//! \brief A pointer to the input object that manages the data
	private_audio::AudioInput* _input;

	//! \brief A pointer to the stream object (set to NULL if the audio was loaded statically)
	private_audio::AudioStream* _stream;

	//! \brief A pointer to where the data is streamed to
	uint8* _data;

	//! \brief The format of the audio (mono/stereo, 8/16 bits per second).
	ALenum _format;

	//! \brief Samples per second of the sound.
// 	uint32 _samples_per_second;

	//! \brief Flag for indicating if the audio should loop or not
	bool _looping;

	//! \brief The total play time of the audio, in seconds
// 	float _time;

	//! \brief The total number of samples of the audio
// 	uint32 _samples;

	//! \brief The audio position that was last seeked, in samples.
	uint32 _offset;

	//! \brief The volume of the sound, ranging from 0.0f to 1.0f
	float _volume;

	//! \brief Size of the streaming buffer, if the audio was loaded for streaming
	uint32 _stream_buffer_size;

	//! \brief The 3D orientation properties of the audio
	//@{
	float _position[3];
	float _velocity[3];
	float _direction[3];
	//@}

private:
	/** \brief Updates the audio during playback
	*** This function is only useful for streaming audio that is currently in the play state. If either of these two
	*** conditions are not met, the function will return since it has nothing to do.
	**/
	void _Update();

	/** \brief Prepares streaming buffers when used for first time or after a seeking operation.
	*** This is a special case, since the already queued buffers must be unqueued, and the new
	*** ones must be refilled. This function should only be called for streaming audio.
	**/
	void _PrepareStreamingBuffers();
}; // class AudioDescriptor


/** ****************************************************************************
*** \brief An class for representing a piece of sound audio
***
*** Sounds are almost always in the .wav file format. T
*** ***************************************************************************/
class SoundDescriptor : public AudioDescriptor {
public:
	SoundDescriptor();

	~SoundDescriptor();

	bool IsSound() const
		{ return true; }

//! \brief Sets the volume of a sound.
/*!
	Sets the volume of the sound, in the range [0.0,1.0]. This value will be modulated by the
	volume value of the sound group.
	\param volume Volume of the sound.
*/
	void SetVolume(float volume);
}; // class SoundDescriptor : public AudioDescriptor


/** ****************************************************************************
*** \brief A class for representing a piece of music audio
***
*** Music is almost always in the .ogg file format.
***
*** \note Looping is enabled for music by default
*** ***************************************************************************/
class MusicDescriptor : public AudioDescriptor {
public:
	MusicDescriptor();

	~MusicDescriptor();

	bool IsSound() const
		{ return false; }

//! \brief Sets the volume of the music.
/*!
	Sets the volume of the music, in the range [0.0,1.0]. This value will be modulated by the
	volume value of the music group.
	\param volume Volume of the sound.
*/
	void SetVolume(float volume);
}; // class MusicDescriptor : public AudioDescriptor

} // namespace hoa_audio

#endif
