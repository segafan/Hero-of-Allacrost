///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    audio_sound.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \brief   Sound file for sound-related code in the audio engine.
 *****************************************************************************/

#include <AL/alut.h>

#include "audio_sound.h"

using namespace std;
using namespace hoa_utils;

namespace hoa_audio {

namespace private_audio {

// ****************************************************************************
// *********************** SoundBuffer Class Functions ************************
// ****************************************************************************

// Creates a new buffer for the filename and loads the WAV data.

SoundBuffer::SoundBuffer(string fname) {
	filename = fname;
	
	// Differences between OpenAL 1.* and earlier versions require this function
	// to be written in very different ways depending on the OpenAL version.

	// For OpenAL version 1.*, otherwise known as freealut
	#ifdef ALUT_API_MAJOR_VERSION
	buffer = alutCreateBufferFromFile(("snd/" + filename + ".wav").c_str());
	if (buffer == AL_NONE) {
		cout << "AUDIO ERROR: ALUT could not load WAV file " << "snd/" + filename + ".wav" << endl;
		cout << "> " << GetALUTErrorString(alutGetError()) << endl;
		reference_count = 0;
		return;
	}
	reference_count = 1;
	
	// For older versions of OpenAL prior to 1.*
	#else
	ALenum format;
	ALsizei size;
	ALvoid *data;
	ALsizei freq;
	ALboolean loop;
	ALbyte *file;
	ALenum error_check;
	
	// (1) Generate a new OpenAL buffer and set the _reference_count to one if successful.
	alGenBuffers(1, &buffer);
	error_check = alGetError();
	if (error_check != AL_NO_ERROR) {
		cout << "AUDIO ERROR: OpenAL could not generate buffer, returned error code: "
		     << GetALErrorString(error_check) << endl;
		return;
	}
	reference_count = 1;

	// (2) Load the WAV file from main memory.
	file = (ALbyte*)(("snd/" + fname + ".wav").c_str());
	alutLoadWAVFile(file, &format, &data, &size, &freq, &loop);
	error_check = alGetError();
	if (error_check != AL_NO_ERROR) {
		cout << "AUDIO ERROR: OpenAL could not load WAV file " << filename << ". Returned error code: "
		     << GetALErrorString(error_check) << endl;
		alDeleteBuffers(1, &buffer);
		reference_count = 0;
		return;
	}

	// (3) Allocate the WAV data into the newly created buffer.
	alBufferData(buffer, format, data, size, freq);
	error_check = alGetError();
	if (error_check != AL_NO_ERROR) {
		cout << "AUDIO ERROR: OpenAL failed to send WAV data of file " << filename << " to buffer."
		     << " Returned error code: " << GetALErrorString(error_check) << endl;
		alDeleteBuffers(1, &buffer);
		reference_count = 0;
		return;
	}

	// (4) Unload the WAV file since we don't need it anymore.
	alutUnloadWAV(format, data, size, freq);
	error_check = alGetError();
	if (error_check != AL_NO_ERROR) {
		cout << "AUDIO ERROR: OpenAL failed to unload WAV file " << filename << ". Returned error code: "
		     << GetALErrorString(error_check) << endl;
		alDeleteBuffers(1, &buffer);
		reference_count = 0;
		return;
	}
	#endif
}

SoundBuffer::~SoundBuffer() {
	if (reference_count != 0) {
		cerr << "AUDIO WARNING: Audio engine is attempting to delete buffer " << filename 
		     << ", which still has " << reference_count << " number of references to it." << endl;
	}
	alDeleteBuffers(1, &buffer);
	// Remove the element from the sound buffer map
	AudioManager->_sound_buffers.erase(AudioManager->_sound_buffers.find(filename));
}

// Remove a reference to this buffer object and delete it if there are no more references.
void SoundBuffer::RemoveReference() {
	reference_count--;
	if (reference_count == 0) {
		delete this;
	}
}

bool SoundBuffer::IsValid() {
	if (alIsBuffer(buffer) == AL_TRUE) {
		return true;
	}
	return false;
}

void SoundBuffer::DEBUG_PrintProperties() {
	cout << ">>> SoundBuffer Properties <<<" << endl;
	if (alIsBuffer(buffer) == AL_FALSE) {
		cout << "ERROR: the buffer in question is not valid" << endl;
	}
	else {
		ALint property;
		cout << "> Filename:   snd/" << filename << ".wav" << endl;
		alGetBufferi(buffer, AL_FREQUENCY, &property);
		cout << "> Frequency:  " << property << "(Hz)" << endl;
		alGetBufferi(buffer, AL_BITS, &property);
		cout << "> Bit depth:  " << property << endl;
		alGetBufferi(buffer, AL_CHANNELS, &property);
		cout << "> Channels:   " << property << endl;
		alGetBufferi(buffer, AL_SIZE, &property);
		cout << "> Size:       " << property << "(bytes)" << endl;
	}
}

// ****************************************************************************
// *********************** SoundSource Class Functions ************************
// ****************************************************************************

// SoundSources are only created in the GameAudio::Initialize() function
SoundSource::SoundSource() {
	owner = NULL;
	alGenSources(1, &source);
}

// SoundSources are only destroyed by the GameAudio destructor
SoundSource::~SoundSource() {
	if (IsValid()) {
		alSourceStop(source);
		alDeleteSources(1, &source);
	}
}

bool SoundSource::IsValid() {
	if (alIsSource(source) == AL_TRUE) {
		return true;
	}
	else {
		return false;
	}
}

void SoundSource::DEBUG_PrintProperties() {
	ALint i_property;
	ALfloat f_property;
	ALfloat fv_property[3];

	cout << ">>> SoundSource Properties <<<" << endl;

	alGetSourcef(source, AL_GAIN, &f_property);
	cout << "> Gain:                " << f_property << endl;
	alGetSourcef(source, AL_PITCH, &f_property);
	cout << "> Pitch:               " << f_property << endl;
	alGetSourcef(source, AL_MIN_GAIN, &f_property);
	cout << "> Min Gain:            " << f_property << endl;
	alGetSourcef(source, AL_MAX_GAIN, &f_property);
	cout << "> Max Gain:            " << f_property << endl;

	// Max distance only works with inverse clamped distance model
	alGetSourcef(source, AL_MAX_DISTANCE, &f_property);
	cout << "> Max Distance:        " << f_property << endl;
	alGetSourcef(source, AL_ROLLOFF_FACTOR, &f_property);
	cout << "> Roll-off Factor:     " << f_property << endl;
	alGetSourcef(source, AL_REFERENCE_DISTANCE, &f_property);
	cout << "> Reference Distance:  " << f_property << endl;
	alGetSourcei(source, AL_SOURCE_RELATIVE, &i_property);
	cout << "> Source Relative:     ";
	if (i_property == AL_TRUE)
		cout << "true" << endl;
	else
		cout << "false" << endl;

	alGetSourcefv(source, AL_POSITION, fv_property);
	cout << "> Position:            " << "[" << fv_property[0] << ", " << fv_property[1] << ", " << fv_property[2] << "] ([X,Y,Z])" << endl;
	alGetSourcefv(source, AL_DIRECTION, fv_property);
	cout << "> Direction:           " << "[" << fv_property[0] << ", " << fv_property[1] << ", " << fv_property[2] << "] ([X,Y,Z])" << endl;
	alGetSourcefv(source, AL_VELOCITY, fv_property);
	cout << "> Velocity:            " << "[" << fv_property[0] << ", " << fv_property[1] << ", " << fv_property[2] << "] ([X,Y,Z])" << endl;
	alGetSourcef(source, AL_CONE_OUTER_GAIN, &fv_property[0]);
	alGetSourcef(source, AL_CONE_INNER_ANGLE, &fv_property[1]);
	alGetSourcef(source, AL_CONE_OUTER_ANGLE, &fv_property[2]);
	cout << "> Cone Properties:     " << "[" << fv_property[0] << " / " << fv_property[1] << " / " << fv_property[2]
	     << "] ([Outer Gain / Inner Angle / Outer Angle])" << endl;

	alGetSourcei(source, AL_SOURCE_TYPE, &i_property);
	cout << "> Source Type:         ";
	switch (i_property) {
// 		case AL_STATIC:
// 			cout << "static" << endl;
		case AL_STREAMING:
			cout << "streaming" << endl;
			break;
// 		case AL_UNDETERMINED:
		default:
			cout << "undetermined" << endl;
			break;
	}
	alGetSourcei(source, AL_LOOPING, &i_property);
	cout << "> Source Loops:        ";
	if (i_property == AL_TRUE)
		cout << "true" << endl;
	else
		cout << "false" << endl;
	alGetSourcei(source, AL_SOURCE_STATE, &i_property);
	cout << "> Source State:        ";
	switch (i_property) {
		case AL_INITIAL:
			cout << "initial" << endl;
			break;
		case AL_STOPPED:
			cout << "stopped" << endl;
			break;
		case AL_PLAYING:
			cout << "playing" << endl;
			break;
		case AL_PAUSED:
			cout << "paused" << endl;
			break;
		default: // this case should never occur
			cout << "unknown" << endl;
			break;
	}
	alGetSourcei(source, AL_BUFFERS_QUEUED, &i_property);
	cout << "> Buffers Queued:      " << i_property << endl;
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &i_property);
	cout << "> Buffers Processed:   " << i_property << endl;
//	alGetSourcef(source, AL_SEC_OFFSET, &fv_property[0]);
//	alGetSourcef(source, AL_SAMPLE_OFFSET, &fv_property[1]);
//	alGetSourcef(source, AL_BYTE_OFFSET, &fv_property[2]);
//	cout << "> Playback Offset:   " << fv_property[0] << "/" << fv_property[1] << "/" << fv_property[2] << " (Seconds/Samples/Bytes)" << endl;
} // SoundSource::DEBUG_PrintProperties()

} // namespace private_audio

using namespace hoa_audio::private_audio;

// ****************************************************************************
// ***** SoundDescriptor Class Functions
// ****************************************************************************

// The constructor does not attempt to retrieve any resources.
SoundDescriptor::SoundDescriptor() {
	_origin = NULL;
	_data = NULL;
}


// The destructor will call the AudioManager singleton to manage the buffer and source that the class was using.
SoundDescriptor::~SoundDescriptor() {
	if (_data != NULL) {
		_data->RemoveReference();
		_data = NULL;
	}

	if (_origin != NULL) {
		AudioManager->_ReleaseSoundSource(_origin);
		_origin = NULL;
	}
}


// This constructor will load the sound from memory, if it isn't loaded already.
bool SoundDescriptor::LoadSound(string fname) {
	if (fname == "") {
		cerr << "AUDIO WARNING: Bad filename passed in LoadSound (empty string)." << endl;
		return false;
	}

	_data = AudioManager->_AcquireSoundBuffer(fname);
	if (_data == NULL) {
		cerr << "AUDIO ERROR: Failed to get a buffer for sound file snd/" << fname << ".wav" << endl;
		return false;
	}
	return true;
}


// Retrieve the state of the audio
uint8 SoundDescriptor::GetState() {
	if (_data == NULL)
		return AUDIO_STATE_UNLOADED;
	if (_origin == NULL)
		return AUDIO_STATE_STOPPED;

	ALenum state;
	alGetSourcei(_origin->source, AL_SOURCE_STATE, &state);
	switch (state) {
		case AL_PLAYING:
			return AUDIO_STATE_PLAYING;
		case AL_PAUSED:
			return AUDIO_STATE_PAUSED;
		case AL_STOPPED:
		case AL_INITIAL:
		default:
			return AUDIO_STATE_STOPPED;
	}
}


void SoundDescriptor::PlaySound() {
	if (_data == NULL) {
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: Attempted to play a sound that had no audio data." << endl;
		return;
	}

	if (HasSource() == false) {
		AllocateSource();
		if (_origin == NULL) {
			if (AUDIO_DEBUG) cerr << "AUDIO WARNING: Failed to allocate a sound source for playing a sound." << endl;
			return;
		}
	}

	alSourcePlay(_origin->source);
}

void SoundDescriptor::PauseSound() {
	if (_origin == NULL) { // If there is no source, there won't be a buffer either.
		return;
	}
	alSourcePause(_origin->source);
}

void SoundDescriptor::StopSound() {
	if (_origin == NULL) { // If there is no source, there won't be a buffer either.
		return;
	}
	alSourceStop(_origin->source);
}

void SoundDescriptor::RewindSound() {
	if (_origin == NULL) { // If there is no source, there won't be a buffer either.
		return;
	}
	alSourceRewind(_origin->source);
}


//
void SoundDescriptor::FreeSound() {
	if (_data != NULL) {
		_data->RemoveReference();
		_data = NULL;
	}

	if (_origin != NULL) {
		AudioManager->_ReleaseSoundSource(_origin);
		_origin = NULL;
	}
}



void SoundDescriptor::SetLooping(bool loop) {
	if (_origin == NULL) {
		return;
	}
	if (loop == _looping) {
		return;
	}

	_looping = loop;
	if (_looping) {
		alSourcei(_origin->source, AL_LOOPING, AL_TRUE);
		cout << "SET LOOP!" << endl;
	}
	else {
		alSourcei(_origin->source, AL_LOOPING, AL_FALSE);
	}
}



void SoundDescriptor::AllocateSource() {
	if (_origin != NULL || _data == NULL) {
		return;
	}
	_origin = AudioManager->_AcquireSoundSource();
	if (_origin == NULL) {
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: Failed to allocate sound source" << endl;
		return;
	}
	alSourcei(_origin->source, AL_BUFFER, _data->buffer);
// 	alSourcef(_origin->source,  AL_PITCH,    1.0f     );
// 	alSourcef(_origin->source,  AL_GAIN,     1.0f     );
// 	alSourcefv(_origin->source, AL_POSITION, SourcePos);
// 	alSourcefv(_origin->source, AL_VELOCITY, SourceVel);
// 	alSourcei(_origin->source,  AL_LOOPING,  loop     ););
}

} // namespace hoa_audio
