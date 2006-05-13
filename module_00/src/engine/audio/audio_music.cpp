///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    audio_music.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \brief   Source file for music-related code in the audio engine.
 *****************************************************************************/

#include "audio_music.h"

using namespace std;
using namespace hoa_utils;

namespace hoa_audio {

namespace private_audio {

// ****************************************************************************
// *********************** MusicBuffer Class Functions ************************
// ****************************************************************************

MusicBuffer::MusicBuffer(string fname) {
	filename = fname;
	reference_count = 1;
	FILE* file_handle = NULL;

	file_handle = fopen(("mus/" + filename + ".ogg").c_str(), "rb");
	if (file_handle == NULL) {
		cerr << "AUDIO ERROR: Could not open music file: music/" << filename << ".ogg" << endl;
		return;
	}

	int32 result;
	result = ov_open(file_handle, &file_stream, NULL, 0);
	if (result != 0) {
		fclose(file_handle);
		cerr << "AUDIO ERROR: Failed to open Ogg Vorbis file: music/" << filename << ".ogg." << endl;;
		GetOVErrorString(result);
// 		reference_count = 0;
		return;
	}

	if (ov_info(&file_stream, -1)->channels == 1) {
		data_format = AL_FORMAT_MONO16;
	}
	else {
		data_format = AL_FORMAT_STEREO16;
	}
	file_info = ov_info(&file_stream, -1);
	file_comment = ov_comment(&file_stream, -1);

	alGenBuffers(MUSIC_BUFFER_COUNT, buffers);
	ALenum error_code = alGetError();
	
	if (error_code != AL_NO_ERROR) {
		cerr << "AUDIO: error generating music buffers: " << alGetString(error_code) << endl;
// 		reference_count = 0;
	}
	
	if (AUDIO_DEBUG)
		DEBUG_PrintProperties();
}



MusicBuffer::~MusicBuffer() {
	if (reference_count != 0) {
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: Deleting a music buffer with a non-zero reference count" << endl;
	}
	
	alDeleteBuffers(MUSIC_BUFFER_COUNT, buffers);

	ov_clear(&file_stream);
	file_info = NULL;
	file_comment = NULL;

	// Remove the element from the sound buffer map
	AudioManager->_music_buffers.erase(AudioManager->_music_buffers.find(filename));
}



bool MusicBuffer::IsValid() {
	for (uint8 i = 0; i < MUSIC_BUFFER_COUNT; i++) {
		if (alIsBuffer(buffers[i]) == AL_FALSE) {
			return false;
		}
	}
	
	return true;
}



// Remove a reference to this buffer object and delete it if there are no more references.
void MusicBuffer::RemoveReference() {
	reference_count--;
	if (reference_count == 0) {
		delete this;
	}
}



// Refills the specified buffer with more audio data
void MusicBuffer::RefillBuffer(ALuint buffer) {
	char data[MUSIC_BUFFER_SIZE];
	int32 size = 0;
	int32 bitstream;
	int32 bytes_read;

	while (size < static_cast<int32>(MUSIC_BUFFER_SIZE)) {
		// ov_read function args: OggVorbis_File, char* buffer, int buffer_length, endianness (1 = big, 0 = little),
		//                        int data_size (bytes, 1 or 2), int signed (1 = signed, 0 = unsigned), int* bitstream_number
		bytes_read = ov_read(&file_stream, data + size, MUSIC_BUFFER_SIZE - size, UTILS_SYSTEM_ENDIAN, 2, 1, &bitstream);
		if (bytes_read > 0) {
			size += bytes_read;
		}
		else if (bytes_read == 0) { // Indicates end-of-file
			break;
		}
		else { // indicates an error occured
			if (AUDIO_DEBUG) {
				cerr << "AUDIO ERROR: Failure while streaming music data into buffer." 
				     << GetOVErrorString(bytes_read) << endl;
			}
			return;
		}
	}

	if (size == 0) { // No data was buffered, either because of an error or EOF
// 		cout << "MusicBuffer::RefillBuffer: returning with no audio data buffered" << endl;
		return;
	}
	else {
// 		cout << "MusicBuffer::RefillBuffer: buffering data to buffer " << buffer << " with " << size << " bytes" << endl;
		alBufferData(buffer, data_format, data, size, file_info->rate);
		ALenum error_code = alGetError();
		if (error_code != AL_NO_ERROR) {
			cerr << GetALErrorString(error_code) << endl;
		}
	}
}

void MusicBuffer::DEBUG_PrintProperties() {
	cout << "Vendor:          " << file_comment->vendor << endl;
	cout << "Version:         " << file_info->version << endl;
	cout << "Channels:        " << file_info->channels << endl;
	cout << "Rate:            " << file_info->rate << endl;
	cout << "Bitrate Upper:   " << file_info->bitrate_upper << endl;
	cout << "Bitrate Nominal: " << file_info->bitrate_nominal << endl;
	cout << "Bitrate Lower:   " << file_info->bitrate_lower << endl;
	cout << "Bitrate Window:  " << file_info->bitrate_window << endl;
	cout << "Comments: " << endl;
	for (uint32 i = 0; i < static_cast<uint32>(file_comment->comments); i++) {
		cout << "> " << file_comment->user_comments[i] << endl;
	}
}

// ****************************************************************************
// *********************** MusicSource Class Functions ************************
// ****************************************************************************

// MusicSources are only created in the GameAudio::Initialize() function
MusicSource::MusicSource() {
	owner = NULL;
	alGenSources(1, &source);
	if (IsValid()) {
		// Turn off attenuation for this source
		alSourcef(source, AL_ROLLOFF_FACTOR, 0.0f);
	}
}



// MusicSources are only destroyed by the GameAudio destructor
MusicSource::~MusicSource() {
	if (IsValid()) {
		alDeleteSources(1, &source);
	}
}



bool MusicSource::IsValid() {
	if (alIsSource(source) == AL_TRUE) {
		return true;
	}
	else {
		return false;
	}
}



void MusicSource::EmptyStreamQueue() {
	int32 number_queued; // The number of buffers queued in the source

	alGetSourcei(source, AL_BUFFERS_QUEUED, &number_queued);
	while (number_queued > 0) {
		ALuint buff;
		alSourceUnqueueBuffers(source, 1, &buff);
		number_queued--;
		// Check for errors here
	}
}



void MusicSource::UpdateStreamQueue() {
	if (owner == NULL) { // If nothing owns this source, it certainly has no data to stream in
		return;
	}
	
	if (!is_playing) {
		return;
	}

	ALint src_info;
	ALenum error_code;

	alGetSourcei(source, AL_BUFFERS_PROCESSED, &src_info);
	while (src_info > 0) { // Refill each buffer that has been processed
		ALuint buffer;
		alSourceUnqueueBuffers(source, 1, &buffer); // Unqueues one of the buffers in the MusicBuffer object
		src_info--;
		owner->_data->RefillBuffer(buffer);
		alSourceQueueBuffers(source, 1, &buffer); // Requeues one of the buffers in the Musicbuffer object
		
		error_code = alGetError();
		if (error_code != AL_NO_ERROR) {
			cerr << GetALErrorString(error_code) << endl;
		}
	} // while (src_info > 0)
	
	// Check for buffer underrun condition
	alGetSourcei(source, AL_SOURCE_STATE, &src_info);
	if (src_info != AL_PLAYING) {
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: music buffer under-run occured, re-playing source" << endl;
		alSourcePlay(source) ;
	}
} // void MusicSource::UpdateStreamQueue()

} // namespace private_audio

using namespace hoa_audio::private_audio;

// ****************************************************************************
// ********************* MusicDescriptor Class Functions **********************
// ****************************************************************************

// The no-arg constructor does not attempt to retrieve any resources.
MusicDescriptor::MusicDescriptor() {
	_origin = NULL;
	_data = NULL;
}

// The destructor will call the AudioManager singleton to manage the buffer and source that the class was using.
MusicDescriptor::~MusicDescriptor() {
	if (_data != NULL) {
		_data->RemoveReference();
		_data = NULL;
	}
	
	// TODO: Fix me later
	if (_origin != NULL) {
		_origin = NULL;
	}
}



bool MusicDescriptor::LoadMusic(std::string fname) {
	// If the music descriptor is using buffered audio data, reset the audio data reference.
	if (_data != NULL) {
		_data->RemoveReference();
		_data = NULL;
	}
	
	_data = AudioManager->_AcquireMusicBuffer(fname);
	if (_data == NULL) {
		return false;
	}
	else {
		return true;
	}
}


// Releases the music source (if it currently holds it) and removes the reference to the buffer it holds.
void MusicDescriptor::FreeMusic() {
	if (_origin != NULL) {
		StopMusic();
		ALuint buffer;
		alSourceUnqueueBuffers(_origin->source, 1, &buffer);
		alSourceUnqueueBuffers(_origin->source, 1, &buffer);
		_origin->owner = NULL; // Releases hold of the source
		_origin = NULL; // To make sure we don't try to use the source again
	}

	if (_data != NULL) {
		_data->RemoveReference();
		_data = NULL;
	}
}


void MusicDescriptor::AllocateSource() {
	if (_origin != NULL) { // Then nothing needs to be done
		return;
	}
	// If another music piece is playing, this call will stop it.
	_origin = AudioManager->_AcquireMusicSource();
}

void MusicDescriptor::PlayMusic() {
	if (_data == NULL) { // Can't play music without data
		return;
	}

	if (_origin == NULL) { // Always check whether we have the source or not and if not, get it.
		_origin = AudioManager->_AcquireMusicSource();
		if (_origin == NULL) {
			AudioManager->_audio_errors |= AUDIO_SOURCE_ACQUISITION_FAILURE;
			if (AUDIO_DEBUG) cerr << "AUDIO ERROR: Failure to acquire a music source." << endl;
			return;
		}
		else {
			_origin->owner = this;
		}
	}

	// If the music is already playing and OpenAL attempt to play it again, it will rewind it and then begin playing.
	// We do not want this, so instead calling this function while the music is already playing is effectively a no-op.
	ALenum state;
	alGetSourcei(_origin->source, AL_SOURCE_STATE, &state);
	if (state == AL_PLAYING) {
		return;
	}

	for (uint32 i = 0; i < MUSIC_BUFFER_COUNT; i++) {
		_data->RefillBuffer(_data->buffers[i]);
	}

	alSourceQueueBuffers(_origin->source, MUSIC_BUFFER_COUNT, _data->buffers);
	alSourcePlay(_origin->source);
	_origin->is_playing = true;
}

void MusicDescriptor::PauseMusic() {
	if (_origin == NULL) {
		return;
	}

	alSourcePause(_origin->source);
	_origin->is_playing = false;
}

void MusicDescriptor::ResumeMusic() {
	if (_origin == NULL) {
		// note to self: might want to try acquiring a source in this case...?
		return;
	}

	// The music must be paused to resume(play) it again. Otherwise this function
	// could generate incorrect behavior in certain scenarios.
	ALenum state;
	alGetSourcei(_origin->source, AL_SOURCE_STATE, &state);
	if (state != AL_PAUSED) {
		return;
	}

	alSourcePlay(_origin->source);
	_origin->is_playing = true;
}


void MusicDescriptor::StopMusic() {
	if (_origin == NULL) {
		return;
	}

	alSourceStop(_origin->source);
	_origin->is_playing = false;
}

void MusicDescriptor::RewindMusic() {
	if (_origin == NULL) {
		return;
	}

	alSourceRewind(_origin->source);
}

} // namespace hoa_audio
