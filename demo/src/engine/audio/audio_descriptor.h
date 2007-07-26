////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   audio_descriptor.h
*** \author Moisés Ferrer Serra, byaku@allacrost.org
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

#include "audio_input.h"
#include "audio_stream.h"


namespace hoa_audio
{

class GameAudio;
class AudioDescriptor;


namespace private_audio
{



//! \brief Class wrapping an OpenAL buffer.
/*!
	Class wrapping an OpenAL buffer.
*/
class SoundBuffer
{
friend class GameAudio;

public:
	ALuint buffer;	//!< \brief Id of an OpenAL buffer

public:
	SoundBuffer ();
	~SoundBuffer ();

	void FillBuffer (uint8* data, const ALenum format, const uint32 size, const uint32 frequency);

	bool IsValid () const;
};




//! \brief Class wrapping an OpenAL source.
/*!
	Class wrapping an OpenAL source.
*/
class SoundSource
{
public:
	ALuint source;				//!< \brief OpenAL Id of the source.
	int16 source_pos;			//!< \brief Position of the source on the vector of the strucure of the GameAudio object.
	bool attached;				//!< \brief Flag indicating if the source is attached to a descriptor.
	AudioDescriptor* owner;		//!< \brief Pointer to the descriptor associated to this source.

public:
	SoundSource ();
	~SoundSource ();

	bool IsValid () const;

	void Play ();
	void Stop ();
	void Pause ();
	void Resume ();
	void Rewind ();
};


} // End of namespace private_audio



//! \brief Feasible states of any audio descriptor in the system.
/*!
	Set of feasible states that audio descriptors can be.
*/
enum AUDIO_STATE
{
	AUDIO_STATE_UNLOADED	= 0,	//! \brief Sound not loaded.
	AUDIO_STATE_STOPPED		= 1,	//! \brief Stopped sound.
	AUDIO_STATE_PAUSED		= 2,	//! \brief Paused sound.
	AUDIO_STATE_PLAYING		= 3		//! \brief Sound playing.
};

//! \brief Available methods for loading a sound.
/*!
	These are the ways a sound acn be loaded. A load can be load statically, so it is
	completely loaded in memory, or streaming (from file or from memory).
*/
enum AUDIO_LOAD
{
	AUDIO_LOAD_STATIC			= 0,	//!< \brief Load sound statically.
	AUDIO_LOAD_STREAM_MEMORY	= 1,	//!< \brief Load sound for streaming from memory.
	AUDIO_LOAD_STREAM_FILE		= 2		//!< \brief Load sound for streaming from file.
};




//! \brief Class that provides the funcionality for managing sounds.
/*!
	Class that provides the funcionality for managing sounds. This includes the basic 
	functionality such play, stop, pause, seeking, and also 3D functions.
*/
class AudioDescriptor
{
protected:
	private_audio::SoundSource* _source;	//!< \brief Pointer to the OpenAL source attached to the sound.
	private_audio::SoundBuffer* _buffer;	//!< \brief Pointer to the OpenAL buffers attached to the sound (1 for static sounds, 2 for streamed ones).
	AUDIO_STATE _state;						//!< \brief Current state of the sound (playing, stopped, ...).
	private_audio::Stream* _stream;			//!< \brief Pointer to the stream object (0 if the sound is static).
	uint8* _data;							//!< \brief Pointer for loaded data on memory.
	ALenum _format;							//!< \brief Format of the sound in OpenAL format (mono/stereo, 8/16 bits per second).
	bool _looping;							//!< \brief Flag for indicating if the sound is looping.
	float _time;							//!< \brief Time of the sound in seconds.
	uint32 _samples;						//!< \brief Number of samples of the sound.
	uint32 _offset;							//!< \brief Last seeked position (0 by default), in samples.
	uint32 _samples_per_second;				//!< \brief Samples per second of the sound.
	float _volume;							//!< \brief Volume of the sound.
	uint32 _stream_buffer_size;				//!< \brief Size of streaming buffer, used if using streaming.

	// 3D
	float _position[3];						//!< \brief Position of the sound.
	float _velocity[3];						//!< \brief Velocity of the sound.
	float _direction[3];					//!< \brief Direction of the sound.

public:
	AudioDescriptor ();
	virtual ~AudioDescriptor ();

	void LoadSound (const std::string &file_name, AUDIO_LOAD load_type=AUDIO_LOAD_STATIC, const uint32 stream_buffer_size=16384);
	void FreeSound ();

	void Play ();
	void Stop ();
	void Pause ();
	void Resume ();
	void Rewind ();

	void SeekSample (const uint32 sample);
	void SeekSecond (const float second);

	bool IsLooping () const;
	void SetLooping (const bool loop);
	void SetLoopStart (const uint32 loop_start);
	void SetLoopEnd (const uint32 loop_end);

	uint8 GetState () const;

	virtual void SetVolume (const float volume);
	float GetVolume () const;

	//! \name 3D functions
	//! \brief Functions for spatial sounds.
	/*!
		Set of functions used to manipulate the 3d features of a sound. These features will just 
		take effect if a sound is in mono format (not stereo).
	*/
	//@{
	void SetPosition (const float position[3]);
	void SetVelocity (const float velocity[3]);
	void SetDirection (const float direction[3]);
	void GetPosition (float position[3]) const;
	void GetVelocity (float velocity[3]) const;
	void GetDirection (float direction[3]) const;
	//@}

	void Update ();

	void DEBUG_PrintInfo ();

private:
	void PrepareStreamingBuffers ();
};




//! \brief Class that provides the funcionality for managing sounds.
/*!
	Class that provides the funcionality for managing sounds (in sound/music group).
*/
class SoundDescriptor : public AudioDescriptor
{
public:
	SoundDescriptor ();
	~SoundDescriptor ();
	virtual void SetVolume (const float volume);
};




//! \brief Class that provides the funcionality for managing music.
/*!
	Class that provides the funcionality for managing music (in sound/music group).
*/
class MusicDescriptor : public AudioDescriptor
{
public:
	MusicDescriptor ();
	~MusicDescriptor ();
	virtual void SetVolume (const float volume);
};





} // End of namespace hoa_audio


#endif

