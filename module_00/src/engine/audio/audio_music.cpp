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

#include "audio.h"
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

	file_handle = fopen(("mus/" + filename + ".ogg").c_str(), "rb");
	if (file_handle == NULL) {
		cerr << "AUDIO ERROR: Could not open music file: music/" << filename << ".ogg" << endl;
		return;
	}

	int32 result;
	result = ov_open(file_handle, &file_stream, NULL, 0);
	if (result < 0) {
		fclose(file_handle);
		cerr << "AUDIO ERROR: Failed to open Vorbig Ogg file: music/" << filename << ".ogg.  Error message: ";
		switch (result) {
			case OV_EREAD:
				cout << "A read from media returned an error." << endl;
				break;
			case OV_ENOTVORBIS:
				cout << "The bitstream is not Vorbis data." << endl;
				break;
			case OV_EVERSION:
				cout << "Vorbis version mismatch." << endl;
				break;
			case OV_EBADHEADER:
				cout << "Invalid Vorbis bitstream header." << endl;
				break;
			case OV_EFAULT:
				cout << "Internal logic fault (possible heap/stack corruption)" << endl;
				break;
			default:
				cout << "Unknown error." << endl;
				break;
			}
		return;
	}

	file_info = ov_info(&file_stream, -1);
	file_comment = ov_comment(&file_stream, -1);
	if (file_info->channels == 1)
		format = AL_FORMAT_MONO16;
	else
		format = AL_FORMAT_STEREO16;

	alGenBuffers(2, buffers);
}

MusicBuffer::~MusicBuffer() {
	if (reference_count != 0) {
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: Deleting a music buffer with a non-zero reference count" << endl;
	}
	alDeleteBuffers(2, buffers);

	ov_clear(&file_stream); // This call also closes the file handle
	file_handle = NULL;
	file_info = NULL;
	file_comment = NULL;

	// Remove the element from the sound buffer map
	AudioManager->_music_buffers.erase(AudioManager->_music_buffers.find(filename));
}

bool MusicBuffer::IsValid() {
	if (alIsBuffer(buffers[0]) == AL_TRUE && alIsBuffer(buffers[1]) == AL_TRUE) {
		return true;
	}
	else {
		return false;
	}
}

// Remove a reference to this buffer object and delete it if there are no more references.
void MusicBuffer::RemoveReference() {
	reference_count--;
	if (reference_count == 0) {
		delete this;
	}
}

// Refills the specified buffer with more audio data
void MusicBuffer::RefillBuffer(ALuint buff) {
	char data[MUSIC_BUFFER_SIZE];
	int32 size = 0;
	int32 bitstream;
	int32 result;

	while (size < static_cast<int32>(MUSIC_BUFFER_SIZE)) {
		// ov_read function args: OggVorbis_File, char* buffer, int buffer_length, endianness (1 = big, 0 = little),
		//                        int data_size (bytes, 1 or 2), int signed (1 = signed, 0 = unsigned), int* bitstream_number
		result = ov_read(&file_stream, data + size, MUSIC_BUFFER_SIZE - size, UTILS_SYSTEM_ENDIAN, 2, 1, &bitstream);
		if (result > 0) {
			size += result;
		}
		else if (result == 0) { // Indicates EOF
			break;
		}
		else { // (result < 0) == error
			if (AUDIO_DEBUG) cerr << "AUDIO ERROR: Failure while streaming music data into buffer. Error message: ";
			switch (result) {
				case OV_EREAD:
					if (AUDIO_DEBUG) cerr << "read error from ogg file." << endl;
					break;
				case OV_ENOTVORBIS:
					if (AUDIO_DEBUG) cerr << "music file does not contain vorbis data." << endl;
					break;
				case OV_EVERSION:
					if (AUDIO_DEBUG) cerr << "vorbis version mismatch." << endl;
					break;
				case OV_EBADHEADER:
					if (AUDIO_DEBUG) cerr << "invalid vorbis header." << endl;
					break;
				case OV_EFAULT:
					if (AUDIO_DEBUG) cerr << "internal logic fault (possibly heap/stack corruption)." << endl;
					break;
				case OV_EINVAL:
					if (AUDIO_DEBUG) cerr << "invalidation error." << endl;
					break;
				default:
					if (AUDIO_DEBUG) cerr << "unknown ogg error; error code: " << result << endl;
					break;
			}
			break;
		}
	}

	if (size == 0) { // No data was buffered, either because of an error or EOF
		return;
	}

	alBufferData(buff, format, data, size, file_info->rate);
	// Check for OpenAL errors here
}

void MusicBuffer::DEBUG_PrintProperties() {
	cout << ">>> MusicBuffer Properties <<<" << endl;
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
	while(number_queued > 0) {
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

	int32 num_processed;

	alGetSourcei(source, AL_BUFFERS_PROCESSED, &num_processed);
	while(num_processed > 0) {
		ALuint buff;
		alSourceUnqueueBuffers(source, 1, &buff); // Unqueues one of the buffers in the MusicBuffer object
		num_processed--;
		// check for errors here
		owner->_data->RefillBuffer(buff);
		alSourceQueueBuffers(source, 1, &buff); // Requeues one of the buffers in the Musicbuffer object
		// check for errors here
	}
}

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
	if (_data != NULL) {
		_data->RemoveReference();
		_data = NULL;
	}
	_data = AudioManager->_AcquireMusicBuffer(fname);
	if (_data == NULL) {
		return false;
	}
	return true;
}


// Releases the music source (if it currently holds it) and removes the reference to the buffer it holds.
void MusicDescriptor::FreeMusic() {
	if (_origin != NULL) {
		StopMusic();
		ALuint buff;
		alSourceUnqueueBuffers(_origin->source, 1, &buff);
		alSourceUnqueueBuffers(_origin->source, 1, &buff);
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

	_data->RefillBuffer(_data->buffers[0]);
	_data->RefillBuffer(_data->buffers[1]);

	alSourceQueueBuffers(_origin->source, 2, _data->buffers);
	alSourcePlay(_origin->source);
}

void MusicDescriptor::PauseMusic() {
	if (_origin == NULL) {
		return;
	}

	alSourcePause(_origin->source);
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
}


void MusicDescriptor::StopMusic() {
	if (_origin == NULL) {
		return;
	}

	alSourceStop(_origin->source);
}

void MusicDescriptor::RewindMusic() {
	if (_origin == NULL) {
		return;
	}

	alSourceRewind(_origin->source);
}

} // namespace hoa_audio
