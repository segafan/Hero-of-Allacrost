////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   audio_descriptor.cpp
*** \author Mois�s Ferrer Serra, byaku@allacrost.org
*** \brief  Source for audio descriptors, sources and buffers
***
*** This code provides the funcionality for load sounds and music in the engine.
*** it provides all the funtions available for them, as basic ones (play, stop,...)
*** seeking and more.
***
*** \note This code uses the OpenAL audio library. See http://www.openal.com/
*** ***************************************************************************/


#include "audio.h"
#include "audio_descriptor.h"

#include <iostream>
#include <string>

using namespace hoa_audio::private_audio;
using namespace hoa_audio;





SoundBuffer::SoundBuffer () :
buffer (0)
{
	alGenBuffers(1, &buffer);
	if (ALError ())
	{
		buffer = 0;
	}
}


SoundBuffer::~SoundBuffer ()
{
	if (IsValid())
	{
		alDeleteBuffers(1, &buffer);
		ALError ();
	}
}


//! \brief Fills an OpenAL buffer.
/*!
	Fills an OpenAL buffer with provided data.
	\param data Buffer with the raw data.
	\param format Format of the buffer (mono/stereo and 8/16 bits per sample).
	\param size Size in bytes of the buffer.
	\param frequency Frequency of the data (samples per second).
*/
void SoundBuffer::FillBuffer (uint8* data, const ALenum format, const uint32 size, const uint32 frequency)
{
	alBufferData (buffer, format, data, size, frequency);
	ALError ();
}


//! \brief Checks if the buffer is a valid OpenAL buffer.
/*!
	Checks if the buffer is a valid OpenAL buffer.
*/
bool SoundBuffer::IsValid () const
{
	bool valid = alIsBuffer (buffer) == AL_TRUE;
	ALError ();
	
	return valid;
}











SoundSrc::SoundSrc () :
source (0),
attached (false),
owner (0)
{
}


SoundSrc::~SoundSrc ()
{
	// Stop the soure before leaving
	if (IsValid())
	{
		alSourceStop (source);
		ALError ();
	}
}


//! \brief Checks if the source is a valid OpenAL source.
/*!
	Checks if the source is a valid OpenAL source.
*/
bool SoundSrc::IsValid () const
{
	bool valid = alIsSource(source) == AL_TRUE;
	ALError ();

	return valid;
}


//! \brief Plays a source.
/*!
	Start playing a source. If the source is already playing, it does nothing.
*/
void SoundSrc::Play ()
{
	if (IsValid())
	{
		alSourcePlay (source);
		ALError ();
	}
}


//! \brief Stops a source.
/*!
	Stops playing a source. If the source is already stopped, it does nothing.
*/
void SoundSrc::Stop ()
{
	if (IsValid())
	{
		alSourceStop (source);
		ALError ();
	}
}


//! \brief Pauses a source.
/*!
	Pauses a source. If the source is already pauses, it does nothing.
*/
void SoundSrc::Pause ()
{
	if (IsValid())
	{
		alSourcePause (source);
		ALError ();
	}
}


//! \brief Resumes a paused source.
/*!
	Resumes a paused source. If the source is not paused, it does nothing.
*/ 
void SoundSrc::Resume ()
{
	if (IsValid())
	{
		alSourcePlay (source);
		ALError ();
	}
}


//! \brief Rewinds a source.
/*!
	Rewinds a source.
*/ 
void SoundSrc::Rewind ()
{
	if (IsValid())
	{
		alSourceRewind (source);
		ALError ();
	}
}
















AudioDescriptor::AudioDescriptor () :
_buffer (0),
_source (0),
_state (AUDIO_STATE_UNLOADED),
_stream (0),
_data (0),
_looping (false),
_samples (0),
_time (0),
_offset (0),
_samples_per_second (0),
_volume (1.0f),
_stream_buffer_size (16384)
{
	_position[0] = 0.0f;
	_position[1] = 0.0f;
	_position[2] = 0.0f;

	_velocity[0] = 0.0f;
	_velocity[1] = 0.0f;
	_velocity[2] = 0.0f;

	_direction[0] = 0.0f;
	_direction[1] = 1.0f;
	_direction[2] = 0.0f;
}


AudioDescriptor::~AudioDescriptor ()
{
	FreeSound ();
}


//! \brief Prepares a sound to be played.
/*!
	Prepares a sound to be played. The action taken depends on the kind of load 
	type selected. For static sounds, a OpenAL buffers is filled. For streaming,
	the file/memory is prepared for that.
	\param file_name Name of the file to use.
	\param load_type Type of load (0=static, 1=memory streaming, 2=file streaming).
	\param stream_buffer_size Size of streaming buffer, used if streaming is enabled.
*/
void AudioDescriptor::LoadSound (const std::string &file_name, AUDIO_LOAD load_type, const uint32 stream_buffer_size)
{
	FreeSound ();

	switch (load_type)
	{
		// Load static sound
		case AUDIO_LOAD_STATIC:
		{
			// For static sounds just 1 buffer is needed. We create it dynamically as an array, so later we can delete it
			// with a call of delete[], as in the streaming case
			_buffer = new SoundBuffer [1];

			// Load the sound and get its parameters
			Stream stream (file_name, STREAM_FILE, false);
			_samples = stream.GetSamples();
			_time = stream.GetTime ();
			_samples_per_second = stream.GetSamplesPerSecond ();
			uint8* buffer = new uint8 [stream.GetDataSize()];
			uint32 size (0);
			stream.FillBuffer (buffer, stream.GetSamples());
			if (stream.GetBitsPerSample() == 8)
			{
				if (stream.GetChannels() == 1)
				{
					_format = AL_FORMAT_MONO8;
				}
				else
				{
					_format = AL_FORMAT_STEREO8;
				}
			}
			else
			{
				if (stream.GetChannels() == 1)
				{
					_format = AL_FORMAT_MONO16;
				}
				else
				{
					_format = AL_FORMAT_STEREO16;
				}
			}

			// Pass the data to the OpenAl buffer
			_buffer->FillBuffer (buffer, _format, stream.GetDataSize(), stream.GetSamplesPerSecond());
			delete[] buffer;
			buffer = NULL;

			// Get a source
			_source = AudioManager->_AcquireSoundSrc ();
			// _source = GameAudio::SingletonGetReference()->_AcquireSoundSrc ();
			if (_source)
			{
				alSourcei (_source->source, AL_BUFFER, _buffer->buffer);
				ALError ();
				_source->owner = this;
			}

			break;
		}

		// Load memory streamed sound
		case AUDIO_LOAD_STREAM_MEMORY:
		{
			_stream = new Stream (file_name, STREAM_MEMORY, _looping);
			_buffer = new SoundBuffer [2];		// For streaming we will use 2 buffers

			_samples = _stream->GetSamples();
			_time = _stream->GetTime ();
			_samples_per_second = _stream->GetSamplesPerSecond ();
			_stream_buffer_size = stream_buffer_size;

			_data = new uint8 [_stream_buffer_size*_stream->GetSampleSize()];
			uint32 size (0);			
			if (_stream->GetBitsPerSample() == 8)
			{
				if (_stream->GetChannels() == 1)
				{
					_format = AL_FORMAT_MONO8;
				}
				else
				{
					_format = AL_FORMAT_STEREO8;
				}
			}
			else
			{
				if (_stream->GetChannels() == 1)
				{
					_format = AL_FORMAT_MONO16;
				}
				else
				{
					_format = AL_FORMAT_STEREO16;
				}
			}

			// Get a source
			_source = AudioManager->_AcquireSoundSrc ();
			// _source = GameAudio::SingletonGetReference()->_AcquireSoundSrc ();
			if (_source)
			{
				_source->owner = this;
			}

			// Fill the buffers and queue them on the source
			PrepareStreamingBuffers ();

			break;
		}

		// Load file streamed sound
		case AUDIO_LOAD_STREAM_FILE:
		{
			_stream = new Stream (file_name, STREAM_FILE, _looping);
			_buffer = new SoundBuffer [2];		// For streaming, use 2 buffers

			_samples = _stream->GetSamples();
			_time = _stream->GetTime ();
			_samples_per_second = _stream->GetSamplesPerSecond ();
			_stream_buffer_size = stream_buffer_size;

			_data = new uint8 [_stream_buffer_size*_stream->GetSampleSize()];
			uint32 size (0);			
			if (_stream->GetBitsPerSample() == 8)
			{
				if (_stream->GetChannels() == 1)
				{
					_format = AL_FORMAT_MONO8;
				}
				else
				{
					_format = AL_FORMAT_STEREO8;
				}
			}
			else
			{
				if (_stream->GetChannels() == 1)
				{
					_format = AL_FORMAT_MONO16;
				}
				else
				{
					_format = AL_FORMAT_STEREO16;
				}
			}

			// Get a source
			// _source = GameAudio::SingletonGetReference()->_AcquireSoundSrc ();
			_source = AudioManager->_AcquireSoundSrc ();
			
			if (_source)
			{
				_source->owner = this;
			}

			// Fill the buffers and queue them on the source
			PrepareStreamingBuffers ();

			break;
		}

		default:
			return;
	}
}


//! \brief Frees resources and reset parameters.
/*!
	Frees resources and reset parameters.
*/
void AudioDescriptor::FreeSound ()
{
	_samples = 0;
	_time = 0;
	_samples_per_second = 0;
	_offset = 0;
	_looping = false;
	_volume = 1.0f;

	// If the sound is still attached to a sound, reset to the default parameters the source
	if (_source)
	{
		Stop ();
		alSourcei (_source->source, AL_LOOPING, AL_FALSE);
		ALError ();
		alSourcef (_source->source, AL_GAIN, 1.0f);
		ALError ();
		alSourcei (_source->source, AL_SAMPLE_OFFSET, 0);
		ALError ();
		alSourcei (_source->source, AL_BUFFER, 0);
		ALError ();
		_source->attached = false;
		_source = 0;
	}

	if (_buffer)
	{
		delete[] _buffer;
		_buffer = 0;
	}

	if (_stream)
	{
		delete _stream;
		_stream = 0;
	}
}


//! \brief Plays a sound.
/*!
	Plays a sound. If the sound is playing, it does nothing.
*/
void AudioDescriptor::Play ()
{
	if (_state == AUDIO_STATE_PLAYING)
	{
		return;
	}

	if (_stream && _stream->GetEndOfStream())
	{
		_stream->Seek (_offset);
		PrepareStreamingBuffers ();
	}
	_source->Play ();
	_state = AUDIO_STATE_PLAYING;
}


//! \brief Stops a sound.
/*!
	Stops a sound. If the sound is already stopped, it does nothing.
*/
void AudioDescriptor::Stop ()
{
	if (_state == AUDIO_STATE_STOPPED)
	{
		return;
	}

	if (_source)
	{
		_source->Stop ();
		_state = AUDIO_STATE_STOPPED;
	}
}


//! \brief Pauses a sound.
/*!
	Pauses a sound. If the sound is paused, it does nothing.
*/
void AudioDescriptor::Pause ()
{
	if (_state == AUDIO_STATE_PAUSED)
	{
		return;
	}

	if (_source)
	{
		_source->Pause ();
		_state = AUDIO_STATE_PAUSED;
	}
}


//! \brief Resumes a sound.
/*!
	Resumes a sound. If the sound is not paused, it does nothing.
*/
void AudioDescriptor::Resume ()
{
	if (_state != AUDIO_STATE_PAUSED)
	{
		return;
	}

	if (_source)
	{
		_source->Resume ();
		_state = AUDIO_STATE_PLAYING;
	}
}


//! \brief Rewinds a sound.
/*!
	Rewinds a sound.
*/
void AudioDescriptor::Rewind ()
{
	if (_source)
	{
		_source->Rewind ();
	}
}


//! \brief Indicates wether the sound is looping or not.
/*!
	Indicates wether the sound is looping or not.
	\return True if the sound is looping or false if not.
*/
bool AudioDescriptor::IsLooping () const
{
	return _looping;
}


//! \brief Enables/disables looping.
/*!
	Enables/disables looping for a sound.
	\param loop True to enable looping, false to disable it.
*/
void AudioDescriptor::SetLooping (const bool loop)
{
	if (_looping == loop)
	{
		return;
	}

	_looping = loop;
	if (_stream)
	{
		_stream->SetLooping (_looping);
	}
	else
	{
		if (_source)
		{
			if (_source->IsValid())
			{
				if (_looping)
				{
					alSourcei (_source->source, AL_LOOPING, AL_TRUE);
					ALError ();
				}
				else
				{
					alSourcei (_source->source, AL_LOOPING, AL_FALSE);
					ALError ();
				}
			}
		}
	}
}


//! \brief Sets the start loop point, if enabled
/*!
	Sets the start loop point, if the audio is being streamed.
	\param loop_start Sample position for the start loop point.
*/
void AudioDescriptor::SetLoopStart (const uint32 loop_start)
{
	if (_stream)
	{
		_stream->SetLoopStart (loop_start);
	}
}


//! \brief Sets the end loop point, if enabled
/*!
	Sets the end loop point, if the audio is being streamed.
	\param loop_end Sample position for the end loop point.
*/
void AudioDescriptor::SetLoopEnd (const uint32 loop_end)
{
	if (_stream)
	{
		_stream->SetLoopEnd (loop_end);
	}
}



//! \brief Gets the state of the sound.
/*!
	Gets the state of the sound.
	\return The state of a sound (0=unloaded, 1=stopped, 2=paused, 3=playing).
*/
uint8 AudioDescriptor::GetState () const
{
	return _state;
}


//! \brief Seeks to a given sample position.
/*!
	Seeks to a given sample position. If the position is out of the stream, no action is done.
	\param sample Position to seek.
*/
void AudioDescriptor::SeekSample (const uint32 sample)
{
	if (sample < 0 || sample>=_samples)
	{
		return;
	}

	_offset = sample;

	if (_stream)
	{
		_stream->Seek (_offset);
		PrepareStreamingBuffers ();
	}
	else
	{
		alSourcei (_source->source, AL_SAMPLE_OFFSET, _offset);
		ALError ();
	}
}
	

//! \brief Seeks to a given time.
/*!
	Seeks to a given time. The position is approximated, so it is aligned with a proper sample position.
	If the position is out of the stream, no action is done.
	\param second Time to seek.
*/
void AudioDescriptor::SeekSecond (const float second)
{
	uint32 pos ((uint32)(second*_samples_per_second));
	if (pos < 0 || pos>=_samples)
	{
		return;
	}

	_offset = pos;
	if (_stream)
	{
		_stream->Seek (_offset);
		PrepareStreamingBuffers ();
	}
	else
	{
		alSourcei (_source->source, AL_SAMPLE_OFFSET, _offset);
		ALError ();
	}
}


//! \brief Sets the volume.
/*!
	Sets the volume. The volume will be clamped 1o 0.0 and 1.0.
	\param volume Volume, in range [0.0,1.0].
*/
void AudioDescriptor::SetVolume (const float volume)
{
	// Clamp volume so it is always in the range [0.0,1.0]
	_volume = (volume < 0.0f) ? 0.0f : ( (volume>1.0f) ?  1.0f : volume);
}
	

//! \brief Gets the current volume of the sound.
/*!
	Returns the volume of the sound
	\return The volume of the sound.
*/
float AudioDescriptor::GetVolume () const
{
	return _volume;
}


//! \brief Updates the audio.
/*!
	Performs any periodic operation, if needed.
*/
void AudioDescriptor::Update ()
{
	// Update stream
	if (_stream)
	{
		if (_state == AUDIO_STATE_PLAYING)
		{
			ALint queued (0);
			alGetSourcei (_source->source, AL_BUFFERS_QUEUED, &queued);
			ALError ();

			// If there are no more buffers and the end of stream was reached, stop the sound
			if (!queued && _stream->GetEndOfStream())
			{
				_state = AUDIO_STATE_STOPPED;
				return;
			}

			ALint played (0);
			alGetSourcei (_source->source, AL_BUFFERS_PROCESSED, &played);
			ALError ();

			// If some buffers were processed, attemp to refill them
			if (played > 0)
			{
				while (played > 0)
				{
					ALuint buffers_played;
					alSourceUnqueueBuffers (_source->source, 1, &buffers_played);
					ALError ();
					
					uint32 size = _stream->FillBuffer (_data, _stream_buffer_size);
					if (size > 0)	// If there is no data for filling, don't do it
					{
						alBufferData (buffers_played, _format, _data, size*_stream->GetSampleSize(), _stream->GetSamplesPerSecond());
						ALError ();
						alSourceQueueBuffers (_source->source, 1, &buffers_played);
						ALError ();
					}
					
//					std::cout << "(" << buffers_played << ") ";
//					std::cout.flush ();
					alGetSourcei (_source->source, AL_BUFFERS_PROCESSED, &played);
					ALError ();
				}

				// This ensures that if a streaming sound is stopped because no data was queued in 
				// enough time (i.e, a lag in the execution path with short streaming buffers), it
				// will be replayed. If a playing source with queued buffers ends the buffers, then is
				// automatically stopped. This will avoid that, but replaying the sound
				ALint state;
				alGetSourcei (_source->source, AL_SOURCE_STATE, &state);
				ALError ();
				if (state != AL_PLAYING)
				{
					alSourcePlay (_source->source);
					ALError();
				}
			}
		}
	}
}


//! \brief Prepares streaming buffers when used for first time or after a seeking operation.
/*!
	Prepares streaming buffers when used for first time or after a seeking operation. This is a 
	special case, since the already queued buffers must be unqueued, and the new ones must be
	refilled.
*/
void AudioDescriptor::PrepareStreamingBuffers ()
{
	bool playing (false);

	if (_source)
	{
		if (_state == AUDIO_STATE_PLAYING)
		{
			playing = true;
			Stop ();
		}
		alSourcei (_source->source, AL_BUFFER, 0);
		ALError ();
	}

	uint32 read = _stream->FillBuffer (_data, _stream_buffer_size);
	if (read > 0)
	{
		_buffer[0].FillBuffer (_data, _format, read*_stream->GetSampleSize(), _stream->GetSamplesPerSecond());
		if (_source)
		{
			alSourceQueueBuffers (_source->source, 1, &_buffer[0].buffer);
			ALError ();
		}
	}

	read = _stream->FillBuffer (_data, _stream_buffer_size);
	if (read > 0)
	{
		_buffer[1].FillBuffer (_data, _format, read*_stream->GetSampleSize(), _stream->GetSamplesPerSecond());
		if (_source)
		{
			alSourceQueueBuffers (_source->source, 1, &_buffer[1].buffer);
			ALError ();
		}
	}

	if (playing)
	{
		Play ();
	}
}


//! \brief Sets the position of the sound (3D).
/*!
	Sets the position of the sound (3D).
	\param position Array with the position information.
*/
void AudioDescriptor::SetPosition (const float position[3])
{
	memcpy (_position, position, sizeof(float)*3);
	if (_source)
	{
		if (_source->IsValid())
		{
			alSourcefv (_source->source, AL_POSITION, _position);
			ALError ();
		}
	}
}


//! \brief Sets the velocity of the sound (3D).
/*!
	Sets the velocity of the sound (3D).
	\param velocity Array with the velocity information.
*/
void AudioDescriptor::SetVelocity (const float velocity[3])
{
	memcpy (_velocity, velocity, sizeof(float)*3);
	if (_source)
	{
		if (_source->IsValid())
		{
			alSourcefv (_source->source, AL_VELOCITY, _position);
			ALError ();
		}
	}
}


//! \brief Sets the direction of the sound (3D).
/*!
	Sets the direction of the sound (3D).
	\param direction Array with the direction information.
*/
void AudioDescriptor::SetDirection (const float direction[3])
{
	memcpy (_direction, direction, sizeof(float)*3);
	if (_source)
	{
		if (_source->IsValid())
		{
			alSourcefv (_source->source, AL_DIRECTION, _direction);
			ALError ();
		}
	}
}


//! \brief Gets the position of the sound (3D).
/*!
	Gets the position of the sound (3D).
	\param position Array where the position information will be stored.
*/
void AudioDescriptor::GetPosition (float position[3]) const
{
	memcpy (&position, _position, sizeof(float)*3);
}


//! \brief Gets the velocity of the sound (3D).
/*!
	Gets the velocity of the sound (3D).
	\param velocity Array where the velocity information will be stored.
*/
void AudioDescriptor::GetVelocity (float velocity[3]) const
{
	memcpy (&velocity, _velocity, sizeof(float)*3);
}


//! \brief Gets the direction of the sound (3D).
/*!
	Gets the direction of the sound (3D).
	\param direction Array where the direction information will be stored.
*/
void AudioDescriptor::GetDirection (float direction[3]) const
{
	memcpy (&direction, _direction, sizeof(float)*3);
}


//! \brief Prints information about the audio descriptor.
/*!
	It prints information about the loaded sound, and also information
	about the streaming properties, if it is enabled.
*/
void AudioDescriptor::DEBUG_PrintInfo ()
{
	std::cout << "*** Audio Descriptor Information ***" << std::endl;

	std::cout << "*** Sound information ***" << std::endl;

	switch (_format)
	{
		case AL_FORMAT_MONO8:
			std::cout << "Channels:                     " << 1 << std::endl;
			std::cout << "Bits Per Samples:             " << 8 << std::endl;
			break;
		case AL_FORMAT_MONO16:
			std::cout << "Channels:                     " << 1 << std::endl;
			std::cout << "Bits Per Samples:             " << 16 << std::endl;
			break;
		case AL_FORMAT_STEREO8:
			std::cout << "Channels:                     " << 2 << std::endl;
			std::cout << "Bits Per Samples:             " << 8 << std::endl;
			break;
		case AL_FORMAT_STEREO16:
			std::cout << "Channels:                     " << 2 << std::endl;
			std::cout << "Bits Per Samples:             " << 16 << std::endl;
			break;
	}
	std::cout << "Frequency:                    " << _samples_per_second << std::endl;
	std::cout << "Samples:                      " << _samples << std::endl;
	std::cout << "Time:                         " << _time << std::endl;

	if (_stream)
	{
		std::cout << "Load audio type:              Streamed sound" << std::endl;
		std::cout << "Stream buffer size (samples): " << _stream_buffer_size << std::endl;
	}
	else
	{
		std::cout << "Load audio type:              Static loaded sound" << std::endl;
	}

	std::cout << "*** OpenAL information ***" << std::endl;
	if (_source)
	{
		std::cout << "Played through source:        " << _source->source << std::endl;
	}
	if (_stream)
	{
		if (_buffer)
		{
			std::cout << "Using buffers:                " << _buffer[0].buffer << " y " << _buffer[1].buffer << std::endl;
		}
	}
	else
	{
		if (_buffer)
		{
			std::cout << "Using buffers:                " << _buffer[0].buffer << std::endl;
		}
	}
}








//! \brief Costructor of the Sound Descriptor
/*!
	Registers the sound in the list of sounds descriptors of the global class.
*/
SoundDescriptor::SoundDescriptor ()
{
	AudioManager->_sound.push_back (this);
}


//! \brief Destructor of the Sound Descriptor
/*!
	Deregisters the sound in the list of music descriptors of the global class.
*/
SoundDescriptor::~SoundDescriptor ()
{
	for (std::list<SoundDescriptor*>::iterator it=AudioManager->_sound.begin();
		it!=AudioManager->_sound.end(); it++)
	{
		if (*it == this)
		{
			AudioManager->_sound.erase (it);
			return;
		}
	}
}


//! \brief Sets the volume of a sound.
/*!
	Sets the volume of the sound, in the range [0.0,1.0]. This value will be modulated by the 
	volume value of the sound group.
	\param volume Volume of the sound.
*/
void SoundDescriptor::SetVolume (const float volume)
{
	AudioDescriptor::SetVolume (volume);

	float sound_volume = _volume * AudioManager->GetSoundVolume ();
	
	if (_source)
	{
		if (_source->IsValid())
		{
			alSourcef (_source->source, AL_GAIN, sound_volume);
			ALError ();
		}
	}
}





//! \brief Costructor of the Music Descriptor
/*!
	Registers the music in the list of music descriptors of the global class.
*/
MusicDescriptor::MusicDescriptor ()
{
	AudioManager->_music.push_back (this);
}


//! \brief Destructor of the Music Descriptor
/*!
	Deregisters the music in the list of music descriptors of the global class.
*/
MusicDescriptor::~MusicDescriptor ()
{
	for (std::list<MusicDescriptor*>::iterator it=AudioManager->_music.begin();
		it!=AudioManager->_music.end(); it++)
	{
		if (*it == this)
		{
			AudioManager->_music.erase (it);
			return;
		}
	}
}


//! \brief Sets the volume of the music.
/*!
	Sets the volume of the music, in the range [0.0,1.0]. This value will be modulated by the 
	volume value of the music group.
	\param volume Volume of the sound.
*/
void MusicDescriptor::SetVolume (const float volume)
{
	AudioDescriptor::SetVolume (volume);

	float sound_volume  = _volume * AudioManager->GetMusicVolume ();

	if (_source)
	{
		if (_source->IsValid())
		{
			alSourcef (_source->source, AL_GAIN, sound_volume);
			ALError ();
		}
	}
}
