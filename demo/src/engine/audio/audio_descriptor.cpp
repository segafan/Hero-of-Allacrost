////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   audio_descriptor.cpp
*** \author Moisés Ferrer Serra, byaku@allacrost.org
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

using namespace std;
using namespace hoa_audio::private_audio;

namespace hoa_audio {

namespace private_audio {

////////////////////////////////////////////////////////////////////////////////
// AudioBuffer class methods
////////////////////////////////////////////////////////////////////////////////

AudioBuffer::AudioBuffer() :
	buffer(0)
{
	alGenBuffers(1, &buffer);
	if (AudioManager->CheckALError()) {
		buffer = 0;
	}
}



AudioBuffer::~AudioBuffer() {
	if (IsValid()) {
		alDeleteBuffers(1, &buffer);
	}
}

////////////////////////////////////////////////////////////////////////////////
// AudioSource class methods
////////////////////////////////////////////////////////////////////////////////

AudioSource::~AudioSource() {
	if (IsValid()) {
		alSourceStop(source);
		alDeleteSources(1, &source);
	}
	else {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "OpenAL source was invalid upon destruction" << endl;
	}
}



void AudioSource::Reset() {
	owner = NULL;

	if (IsValid() == false) {
		return;
	}

	alSourcei(source, AL_LOOPING, AL_FALSE);
	alSourcef(source, AL_GAIN, 1.0f);
	alSourcei(source, AL_SAMPLE_OFFSET, 0);
	alSourcei(source, AL_BUFFER, 0);
}

} // namespace private_audio

////////////////////////////////////////////////////////////////////////////////
// AudioDescriptor class methods
////////////////////////////////////////////////////////////////////////////////

AudioDescriptor::AudioDescriptor () :
	_state(AUDIO_STATE_UNLOADED),
	_buffer(NULL),
	_source(NULL),
	_stream(NULL),
	_data(NULL),
	_samples_per_second(0),
	_looping(false),
	_time(0.0f),
	_samples(0),
	_offset(0),
	_volume(1.0f),
	_stream_buffer_size(0)
{
	_position[0] = 0.0f;
	_position[1] = 0.0f;
	_position[2] = 0.0f;
	_velocity[0] = 0.0f;
	_velocity[1] = 0.0f;
	_velocity[2] = 0.0f;
	_direction[0] = 0.0f;
	_direction[1] = 0.0f;
	_direction[2] = 0.0f;
}



bool AudioDescriptor::LoadAudio(const string& file_name, AUDIO_LOAD load_type, uint32 stream_buffer_size) {
	// Clean out any audio resources being used before trying to set new ones
	FreeAudio();

	if (load_type == AUDIO_LOAD_STATIC) {
		// For static sounds just 1 buffer is needed. We create it dynamically as an array here,
		// so that later we can delete it with a call of delete[], same as in the streaming case
		_buffer = new AudioBuffer[1];

		// Load the sound and get its parameters
		AudioStream stream(file_name, STREAM_FILE, false);
		_samples = stream.GetSamples();
		_time = stream.GetTime();
		_samples_per_second = stream.GetSamplesPerSecond();
		_data = new uint8[stream.GetDataSize()];
		stream.FillBuffer(_data, stream.GetSamples());
		if (stream.GetBitsPerSample() == 8) {
			if (stream.GetChannels() == 1) {
				_format = AL_FORMAT_MONO8;
			}
			else {
				_format = AL_FORMAT_STEREO8;
			}
		}
		else { // 16 bits per sample
			if (stream.GetChannels() == 1) {
				_format = AL_FORMAT_MONO16;
			}
			else {
				_format = AL_FORMAT_STEREO16;
			}
		}

		// Pass the data to the OpenAL buffer
		_buffer->FillBuffer(_data, _format, stream.GetDataSize(), stream.GetSamplesPerSecond());
		delete[] _data;
		_data = NULL;

		// Attempt to acquire a source for the new audio to use
		_source = AudioManager->_AcquireAudioSource ();
		if (_source == NULL) {
			IF_PRINT_WARNING(AUDIO_DEBUG) << "could not acquire audio source for new audio file: " << file_name << endl;
			return true;
		}

		alSourcei(_source->source, AL_BUFFER, _buffer->buffer);
		_source->owner = this;
	} // if (load_type == AUDIO_LOAD_STATIC)

	else if (load_type == AUDIO_LOAD_STREAM_MEMORY) {
		_stream = new AudioStream(file_name, STREAM_MEMORY, _looping);
		_buffer = new AudioBuffer[2]; // For streaming we need to use 2 buffers

		_samples = _stream->GetSamples();
		_time = _stream->GetTime();
		_samples_per_second = _stream->GetSamplesPerSecond();
		_stream_buffer_size = stream_buffer_size;

		_data = new uint8[_stream_buffer_size * _stream->GetSampleSize()];
		if (_stream->GetBitsPerSample() == 8) {
			if (_stream->GetChannels() == 1) {
				_format = AL_FORMAT_MONO8;
			}
			else {
				_format = AL_FORMAT_STEREO8;
			}
		}
		else {
			if (_stream->GetChannels() == 1) {
				_format = AL_FORMAT_MONO16;
			}
			else {
				_format = AL_FORMAT_STEREO16;
			}
		}

		// Attempt to acquire a source for the new audio to use
		_source = AudioManager->_AcquireAudioSource ();
		if (_source == NULL) {
			IF_PRINT_WARNING(AUDIO_DEBUG) << "could not acquire audio source for new audio file: " << file_name << endl;
			return true;
		}

		_source->owner = this;
		// Fill the buffers and queue them on the source
		_PrepareStreamingBuffers();
	} // else if (load_type == AUDIO_LOAD_STREAM_MEMORY) {

		// Load file streamed sound
	else if (load_type == AUDIO_LOAD_STREAM_FILE) {
		_stream = new AudioStream(file_name, STREAM_FILE, _looping);
		_buffer = new AudioBuffer[2]; // For streaming we need to use 2 buffers

		_samples = _stream->GetSamples();
		_time = _stream->GetTime ();
		_samples_per_second = _stream->GetSamplesPerSecond ();
		_stream_buffer_size = stream_buffer_size;

		_data = new uint8[_stream_buffer_size * _stream->GetSampleSize()];

		if (_stream->GetBitsPerSample() == 8) {
			if (_stream->GetChannels() == 1) {
				_format = AL_FORMAT_MONO8;
			}
			else {
				_format = AL_FORMAT_STEREO8;
			}
		}
		else {
			if (_stream->GetChannels() == 1) {
				_format = AL_FORMAT_MONO16;
			}
			else {
				_format = AL_FORMAT_STEREO16;
			}
		}

		// Attempt to acquire a source for the new audio to use
		_source = AudioManager->_AcquireAudioSource ();
		if (_source == NULL) {
			IF_PRINT_WARNING(AUDIO_DEBUG) << "could not acquire audio source for new audio file: " << file_name << endl;
			return true;
		}

		_source->owner = this;
		// Fill the buffers and queue them on the source
		_PrepareStreamingBuffers();
	} // else if (load_type == AUDIO_LOAD_STREAM_FILE)

	else {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "unknown load_type argument passed: " << load_type << endl;
		return false;
	}

	return true;
} // bool AudioDescriptor::LoadAudio(const string& file_name, AUDIO_LOAD load_type, uint32 stream_buffer_size)



void AudioDescriptor::FreeAudio() {
	_state = AUDIO_STATE_UNLOADED;
	_samples = 0;
	_time = 0;
	_samples_per_second = 0;
	_offset = 0;
	_looping = false;
	_volume = 1.0f;

	// If the sound is still attached to a sound, reset to the default parameters the source
	if (_source != NULL) {
		_source->Reset();
		_source = NULL;
	}

	if (_buffer != NULL) {
		delete[] _buffer;
		_buffer = NULL;
	}

	if (_stream != NULL) {
		delete _stream;
		_stream = NULL;
	}
}



void AudioDescriptor::Play() {
	if (_state == AUDIO_STATE_PLAYING)
		return;

	if (_source == NULL) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "did not have access to valid AudioSource" << endl;
		return;
	}

	if (_stream && _stream->GetEndOfStream()) {
		_stream->Seek(_offset);
		_PrepareStreamingBuffers();
	}

	alSourcePlay(_source->source);
	_state = AUDIO_STATE_PLAYING;
}



void AudioDescriptor::Stop() {
	if (_state == AUDIO_STATE_STOPPED)
		return;

	if (_source == NULL) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "did not have access to valid AudioSource" << endl;
		return;
	}

	alSourceStop(_source->source);
	_state = AUDIO_STATE_STOPPED;
}



void AudioDescriptor::Pause() {
	if (_state == AUDIO_STATE_PAUSED)
		return;

	if (_source == NULL) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "did not have access to valid AudioSource" << endl;
		return;
	}

	alSourcePause(_source->source);
	_state = AUDIO_STATE_PAUSED;
}



void AudioDescriptor::Resume() {
	if (_state != AUDIO_STATE_PAUSED)
		return;

	if (_source == NULL) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "did not have access to valid AudioSource" << endl;
		return;
	}

	alSourcePlay(_source->source);
	_state = AUDIO_STATE_PLAYING;
}



void AudioDescriptor::Rewind() {
	if (_source == NULL) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "did not have access to valid AudioSource" << endl;
		return;
	}

	alSourceRewind(_source->source);
}



void AudioDescriptor::SetLooping(bool loop) {
	if (_looping == loop)
		return;

	_looping = loop;
	if (_stream != NULL) {
		_stream->SetLooping(_looping);
	}
	else if (_source != NULL) {
		if (_looping)
			alSourcei(_source->source, AL_LOOPING, AL_TRUE);
		else
			alSourcei (_source->source, AL_LOOPING, AL_FALSE);
	}
}



void AudioDescriptor::SetLoopStart(uint32 loop_start) {
	if (_stream == NULL) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "the audio data was not loaded with streaming properties, this operation is not permitted" << endl;
		return;
	}
	_stream->SetLoopStart (loop_start);
}



void AudioDescriptor::SetLoopEnd(uint32 loop_end) {
	if (_stream == NULL) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "the audio data was not loaded with streaming properties, this operation is not permitted" << endl;
		return;
	}
	_stream->SetLoopEnd (loop_end);
}



void AudioDescriptor::SeekSample(uint32 sample) {
	if (sample >= _samples) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "failed because requested seek time fell outside the valid range of samples: " << sample << endl;
		return;
	}

	_offset = sample;

	if (_stream) {
		_stream->Seek(_offset);
		_PrepareStreamingBuffers();
	}
	else if (_source != NULL) {
		alSourcei(_source->source, AL_SAMPLE_OFFSET, _offset);
	}
}



void AudioDescriptor::SeekSecond(float second) {
	if (second < 0.0f) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "function received invalid argument that was less than 0.0f: " << second << endl;
		return;
	}

	uint32 pos = static_cast<uint32>(second * _samples_per_second);
	if (pos >= _samples) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "failed because requested seek time fell outside the valid range of samples: " << pos << endl;
		return;
	}

	_offset = pos;
	if (_stream) {
		_stream->Seek(_offset);
		_PrepareStreamingBuffers();
	}
	else if (_source != NULL) {
		alSourcei(_source->source, AL_SEC_OFFSET, _offset);
	}
}



void AudioDescriptor::SetVolume(float volume) {
	if (volume < 0.0f) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "tried to set volume less than 0.0f: " << volume << endl;
		_volume = 0.0f;
	}
	else if (volume > 1.0f) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "tried to set volume greater than 1.0f: " << volume << endl;
		_volume = 1.0f;
	}
	else {
		_volume = volume;
	}
}



void AudioDescriptor::SetPosition(const float position[3]) {
	if (_format != AL_FORMAT_MONO8 && _format != AL_FORMAT_MONO16) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "audio is stereo channel and will not be effected by function call" << endl;
		return;
	}

	memcpy(_position, position, sizeof(float) * 3);
	if (_source != NULL) {
		alSourcefv(_source->source, AL_POSITION, _position);
	}
}



void AudioDescriptor::SetVelocity(const float velocity[3]) {
	if (_format != AL_FORMAT_MONO8 && _format != AL_FORMAT_MONO16) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "audio is stereo channel and will not be effected by function call" << endl;
		return;
	}

	memcpy(_velocity, velocity, sizeof(float) * 3);
	if (_source != NULL) {
		alSourcefv(_source->source, AL_VELOCITY, _position);
	}
}



void AudioDescriptor::SetDirection(const float direction[3]) {
	if (_format != AL_FORMAT_MONO8 && _format != AL_FORMAT_MONO16) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "audio is stereo channel and will not be effected by function call" << endl;
		return;
	}
	
	memcpy(_direction, direction, sizeof(float) * 3);
	if (_source != NULL) {
		alSourcefv(_source->source, AL_DIRECTION, _direction);
	}
}



void AudioDescriptor::DEBUG_PrintInfo() {
	cout << "*** Audio Descriptor Information ***" << endl;

	uint8 num_channels = 0;
	uint8 bits_per_sample = 0;
	switch (_format) {
		case AL_FORMAT_MONO8:
			num_channels = 1;
			bits_per_sample = 8;
			break;
		case AL_FORMAT_MONO16:
			num_channels = 1;
			bits_per_sample = 16;
			break;
		case AL_FORMAT_STEREO8:
			num_channels = 1;
			bits_per_sample = 8;
			break;
		case AL_FORMAT_STEREO16:
			num_channels = 2;
			bits_per_sample = 16;
			break;
		default:
			IF_PRINT_WARNING(AUDIO_DEBUG) << "unknown audio format: " << _format << endl;
			break;
	}

	cout << "Channels:           " << num_channels << endl;
	cout << "Bits Per Samples:   " << bits_per_sample << endl;
	cout << "Frequency:          " << _samples_per_second << endl;
	cout << "Samples:            " << _samples << endl;
	cout << "Time:               " << _time << endl;

	if (_stream) {
		cout << "Load audio type:              streamed" << endl;
		cout << "Stream buffer size (samples): " << _stream_buffer_size << endl;
	}
	else {
		cout << "Load audio type:              static" << endl;
	}
} // void AudioDescriptor::DEBUG_PrintInfo()



void AudioDescriptor::_Update() {
	// Only streaming audio that is playing requires periodic updates
	if (_stream == NULL || _state != AUDIO_STATE_PLAYING)
		return;

	ALint queued = 0;
	alGetSourcei(_source->source, AL_BUFFERS_QUEUED, &queued);

	// If there are no more buffers and the end of stream was reached, stop the sound
	if (queued != 0 && _stream->GetEndOfStream()) {
		_state = AUDIO_STATE_STOPPED;
		return;
	}

	ALint buffers_processed = 0;
	alGetSourcei(_source->source, AL_BUFFERS_PROCESSED, &buffers_processed);
	// If any buffers have finished playing, attempt to refill them
	if (buffers_processed > 0) {
		while (buffers_processed > 0) {
			ALuint buffer_finished;
			alSourceUnqueueBuffers(_source->source, 1, &buffer_finished);

			uint32 size = _stream->FillBuffer(_data, _stream_buffer_size);
			if (size > 0) { // Make sure that there is data available to fill
				alBufferData(buffer_finished, _format, _data, size * _stream->GetSampleSize(), _stream->GetSamplesPerSecond());
				alSourceQueueBuffers(_source->source, 1, &buffer_finished);
			}

			alGetSourcei(_source->source, AL_BUFFERS_PROCESSED, &buffers_processed);
		}

		// This ensures that if a streaming audio piece is stopped because the buffers ran out
		// of audio data for the source to play, the audio will be automatically replayed again.
		ALint state;
		alGetSourcei(_source->source, AL_SOURCE_STATE, &state);
		if (state != AL_PLAYING) {
			alSourcePlay(_source->source);
		}
	}
} // void AudioDescriptor::_Update()



void AudioDescriptor::_PrepareStreamingBuffers() {
	if (_stream == NULL) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "_stream pointer was NULL, meaning this function should never have been called" << endl;
		return;
	}

	if (_source == NULL) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "failed because no source was available for this object to utilize" << endl;
		return;
	}

	bool was_playing = false;
	AudioManager->CheckALError(); // Clear error code

	// Stop the audio if it is playing and detatch the buffer from the source
	if (_state == AUDIO_STATE_PLAYING) {
		was_playing = true;
		Stop();
	}
	alSourcei(_source->source, AL_BUFFER, 0);

	// Refill the first buffer
	uint32 read = _stream->FillBuffer(_data, _stream_buffer_size);
	if (read > 0) {
		_buffer[0].FillBuffer(_data, _format, read * _stream->GetSampleSize(), _stream->GetSamplesPerSecond());
		if (_source != NULL)
			alSourceQueueBuffers(_source->source, 1, &_buffer[0].buffer);
	}

	// Refill the second buffer
	read = _stream->FillBuffer(_data, _stream_buffer_size);
	if (read > 0) {
		_buffer[1].FillBuffer(_data, _format, read * _stream->GetSampleSize(), _stream->GetSamplesPerSecond());
		if (_source != NULL)
			alSourceQueueBuffers(_source->source, 1, &_buffer[1].buffer);
	}

	if (AudioManager->CheckALError()) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "OpenAL error detected: " << AudioManager->CreateALErrorString();
	}

	if (was_playing) {
		Play();
	}
}

////////////////////////////////////////////////////////////////////////////////
// SoundDescriptor class methods
////////////////////////////////////////////////////////////////////////////////

SoundDescriptor::SoundDescriptor() {
	AudioManager->_sound.push_back(this);
}



SoundDescriptor::~SoundDescriptor() {
	for (list<SoundDescriptor*>::iterator i = AudioManager->_sound.begin(); i != AudioManager->_sound.end(); i++) {
		if (*i == this) {
			AudioManager->_sound.erase(i);
			return;
		}
	}

	IF_PRINT_WARNING(AUDIO_DEBUG) << "class object was not found in AudioManager's list of registered sounds" << endl;
}



void SoundDescriptor::SetVolume(float volume) {
	AudioDescriptor::SetVolume(volume);

	float sound_volume = _volume * AudioManager->GetSoundVolume();
	
	if (_source) {
			alSourcef(_source->source, AL_GAIN, sound_volume);
	}
}

////////////////////////////////////////////////////////////////////////////////
// MusicDescriptor class methods
////////////////////////////////////////////////////////////////////////////////

MusicDescriptor::MusicDescriptor() {
	AudioManager->_music.push_back(this);
}



MusicDescriptor::~MusicDescriptor() {
	for (list<MusicDescriptor*>::iterator i = AudioManager->_music.begin(); i !=AudioManager->_music.end(); i++) {
		if (*i == this) {
			AudioManager->_music.erase(i);
			return;
		}
	}

	IF_PRINT_WARNING(AUDIO_DEBUG) << "class object was not found in AudioManager's list of registered music" << endl;
}



void MusicDescriptor::SetVolume(float volume) {
	AudioDescriptor::SetVolume(volume);

	float sound_volume = _volume * AudioManager->GetMusicVolume();

	if (_source) {
		alSourcef(_source->source, AL_GAIN, sound_volume);
	}
}

} // namespace hoa_audio
