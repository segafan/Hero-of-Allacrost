///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    audio.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \brief   Source file for audio engine interface.
 *****************************************************************************/

#ifndef __APPLE__
#include <AL/alut.h>
#else
#include <OpenAL/alut.h>
#endif // #ifndef __APPLE__
#include <vorbis/codec.h>

#include "audio.h"
#include "audio_sound.h"
#include "audio_music.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_data;

namespace hoa_audio {

GameAudio *AudioManager = NULL;
bool AUDIO_DEBUG = false;
SINGLETON_INITIALIZE(GameAudio);

namespace private_audio {

// Returns an error string corresponding to a particular error code
const string GetALErrorString(ALenum error_code) {
	switch (error_code) {
		case AL_INVALID_NAME:
			return "AL_INVALID_NAME";
		case AL_INVALID_ENUM:
			return "AL_INVALID_ENUM";
		case AL_INVALID_VALUE:
			return "AL_INVALID_VALUE";
		case AL_INVALID_OPERATION:
			return "AL_INVALID_OPERATION";
		case AL_OUT_OF_MEMORY:
			return "AL_OUT_OF_MEMORY";
		case AL_NO_ERROR:
			return "AL_NO_ERROR";
		default:
			return ("unknown AL error code: " + error_code);
	} // switch (error_code)
} // const string GetALErrorString(ALenum error_code)

// Returns an error string corresponding to a particular error code
const string GetALCErrorString(ALenum error_code) {
	switch (error_code) {
		case ALC_INVALID_DEVICE:
			return "ALC_INVALID_DEVICE";
		case ALC_INVALID_CONTEXT:
			return "ALC_INVALID_CONTEXT";
		case ALC_INVALID_ENUM:
			return "ALC_INVALID_ENUM";
		case ALC_INVALID_VALUE:
			return "ALC_INVALID_VALUE";
		case ALC_OUT_OF_MEMORY:
			return "ALC_OUT_OF_MEMORY";
		default:
			return ("unknown ALC error code: " + error_code);
	} // switch (error_code)
} // const string GetALCErrorString(ALenum error_code)

// Returns an error string corresponding to a particular error code
const string GetALUTErrorString(ALenum error_code) {
	// For OpenAL version 1.*, otherwise known as freealut
	#ifdef ALUT_API_MAJOR_VERSION
	switch (error_code) {
		case (ALUT_ERROR_NO_ERROR):
			return "ALUT_ERROR_NO_ERROR: No ALUT error found";
		case (ALUT_ERROR_OUT_OF_MEMORY):
			return "ALUT_ERROR_OUT_OF_MEMORY: ALUT ran out of memory";
		case (ALUT_ERROR_INVALID_ENUM):
			return "ALUT_ERROR_INVALID_ENUM: ALUT was given an invalid enumeration token";
		case (ALUT_ERROR_INVALID_VALUE):
			return "ALUT_ERROR_INVALID_VALUE: ALUT was given an invalid value.";
		case (ALUT_ERROR_INVALID_OPERATION):
			return "ALUT_ERROR_INVALID_OPERATION: The operation is invalid in the current ALUT state";
		case (ALUT_ERROR_NO_CURRENT_CONTEXT):
			return "ALUT_ERROR_NO_CURRENT_CONTEXT: There is no current AL context";
		case (ALUT_ERROR_AL_ERROR_ON_ENTRY):
			return "ALUT_ERROR_AL_ERROR_ON_ENTRY: There was already an AL error on entry to alutCreateBufferFromFile()";
		case (ALUT_ERROR_ALC_ERROR_ON_ENTRY):
			return "ALUT_ERROR_ALC_ERROR_ON_ENTRY: There was already an ALC error on entry to alutCreateBufferFromFile()";
		case (ALUT_ERROR_OPEN_DEVICE):
			return "ALUT_ERROR_OPEN_DEVICE: There was an error opening the ALC device";
		case (ALUT_ERROR_CLOSE_DEVICE):
			return "ALUT_ERROR_CLOSE_DEVICE: There was an error closing the ALC device";
		case (ALUT_ERROR_CREATE_CONTEXT):
			return "ALUT_ERROR_CREATE_CONTEXT: There was an error creating an ALC context";
		case (ALUT_ERROR_MAKE_CONTEXT_CURRENT):
			return "ALUT_ERROR_MAKE_CONTEXT_CURRENT: Could not change the current ALC context";
		case (ALUT_ERROR_DESTROY_CONTEXT):
			return "ALUT_ERROR_DESTROY_CONTEXT: There was an error destroying the ALC context";
		case (ALUT_ERROR_GEN_BUFFERS):
			return "ALUT_ERROR_GEN_BUFFERS: There was an error generating an AL buffer";
		case (ALUT_ERROR_BUFFER_DATA):
			return "ALUT_ERROR_BUFFER_DATA: ALUT has not been initialized";
		case (ALUT_ERROR_IO_ERROR):
			return "ALUT_ERROR_IO_ERROR: I/O error, consult errno for more details";
		case (ALUT_ERROR_UNSUPPORTED_FILE_TYPE):
			return "ALUT_ERROR_UNSUPPORTED_FILE_TYPE: Unsupported file type";
		case (ALUT_ERROR_UNSUPPORTED_FILE_SUBTYPE):
			return "ALUT_ERROR_UNSUPPORTED_FILE_SUBTYPE: Unsupported mode within an otherwise usable file type";
		case (ALUT_ERROR_CORRUPT_OR_TRUNCATED_DATA):
			return "ALUT_ERROR_CORRUPT_OR_TRUNCATED_DATA: The sound data was corrupt or truncated";
		default:
			return ("unknown ALUT error code: " + error_code);
	} // switch (error_code)

	#else
		// This function will probably never be called if ALUT_API_MAJOR_VERSION is not defined, thus this
		// statement should never be reached.
		return "ALUT Error: This ALUT version does not support error codes";
	#endif /* ALUT_API_MAJOR_VERSION */
} // const string GetALUTErrorString(ALenum error_codes)

const string GetOVErrorString(int32 error_code) {
	switch (error_code) {
		case OV_FALSE:
			return "OV_FALSE: no data available";
		case OV_HOLE:
			return "OV_HOLE: encoutered missing or corrupt data in the bitstream";
		case OV_EREAD:
			return "OV_EREAD: read error while fetching compressed data for decode";
		case OV_EFAULT:
			return "OV_EFAULT: internal inconsistency in decode state";
		case OV_EIMPL:
			return "OV_EIMPL: feature not implemented";
		case OV_EINVAL:
			return "OV_EINVAL: invalid argument, or incompletely initialized argument passed to libvorbisfile call";
		case OV_ENOTVORBIS:
			return "OV_ENOTVORBIS: the given file/data was not recognized as Ogg Vorbis data";
		case OV_EBADHEADER:
			return "OV_EBADHEADER: the file/data is apparently an Ogg Vorbis stream, but contains a corrupted or undecipherable header";
		case OV_EVERSION:
			return "OV_EVERSION: the bitstream format revision of the given stream is not supported";
		case OV_EBADLINK:
			return "OV_EBADLINK: the given link exists in the Vorbis data stream, but is not decipherable due to garbacge or corruption.";
		case OV_ENOSEEK:
			return "OV_ENOSEEK: the given stream is not seekable";
		default:
			return ("unknown OV error code: " + error_code);
	}
} // const string GetOggVorbisErrorString(int32 error_code)


// ****************************************************************************
// ************************ AudioState Class Functions ************************
// ****************************************************************************

AudioState::AudioState() {
	if (AUDIO_DEBUG) cout << "AUDIO: AudioState constructor invoked." << endl;
}

AudioState::~AudioState() {
	if (AUDIO_DEBUG) cout << "AUDIO: AudioState destructor invoked." << endl;
}

} // namespace private_audio

using namespace hoa_audio::private_audio;

// ****************************************************************************
// *********************** GameAudio Class Functions *************************
// ****************************************************************************

GameAudio::GameAudio() {
	if (AUDIO_DEBUG) cout << "AUDIO: GameAudio constructor invoked" << endl;
}


// The destructor halts all audio, frees up all allocated memory, and closes the context and device
GameAudio::~GameAudio() {
	if (AUDIO_DEBUG) cout << "AUDIO: GameAudio destructor invoked" << endl;

	// ************ (1) Delete the music source and all sound sources ****************
	delete(_music_source);
	for (list<SoundSource*>::iterator i = _sound_sources.begin(); i != _sound_sources.end(); i++) {
		delete(*i);
	}
	_sound_sources.clear();

	// ************ (2) Delete all music and sound buffers ******************
	for (std::map<string, MusicBuffer*>::iterator i = _music_buffers.begin(); i != _music_buffers.end(); i = _music_buffers.begin()) {
		delete(i->second);
		_music_buffers.erase(i);
	}
	_music_buffers.clear();
	for (std::map<string, SoundBuffer*>::iterator i = _sound_buffers.begin(); i != _sound_buffers.end(); i = _sound_buffers.begin()) {
		delete(i->second);
		_sound_buffers.erase(i);
	}
	_sound_buffers.clear();

	// ************ (3) Deactivate context and close audio device ******************
	alcMakeContextCurrent(NULL); // Disable the active context
	alcDestroyContext(_context);
	alcCloseDevice(_device);
}


// Update all of the streaming audio sources.
void GameAudio::Update() {
	_music_source->UpdateStreamQueue();

	// Later: Go through a list of registered sound descriptor objects who have streaming buffers
}


// Initializes OpenAL and creates the global audio context
bool GameAudio::Initialize() {
	ALCint highest_version = 0; // Highest version number found
   const ALCchar * best_device = NULL; // Name of the 'best' device

   // Find the highest-version device available
   const ALCchar * device_name = alcGetString(NULL, ALC_DEVICE_SPECIFIER); // Get list of all devices (terminated with two '0')
   while (*device_name != NULL) // Loop until two terminating zeroes has been read
   {
      ALCint major_v=0, minor_v=0;
      ALCdevice * _temp_device = alcOpenDevice(device_name); // Open a temporary device for reading in its version number
      if (_temp_device == NULL) // Just keep looping if we couldn't open the device
      {
         if (AUDIO_DEBUG) cout << "AUDIO: Couldn't open a temporary device: " <<  device_name << endl;
         device_name += strlen(device_name) + 1;
         continue;
      }
      ALCcontext * temp_context = alcCreateContext(_temp_device, NULL); // Create a temporary context
      if (temp_context == NULL) // Keep looping again if no context was created
      {
         if (AUDIO_DEBUG) cout << "AUDIO: Couldn't create a temporary context for device: " <<  device_name << endl;
         alcCloseDevice(_temp_device);
         device_name += strlen(device_name) + 1;
         continue;
      }
      alcMakeContextCurrent(temp_context);
      alGetError();
      alcGetIntegerv(_temp_device, ALC_MAJOR_VERSION, sizeof(ALCint), &major_v); // Get the version number
      alcGetIntegerv(_temp_device, ALC_MINOR_VERSION, sizeof(ALCint), &minor_v);
      alcMakeContextCurrent(NULL); // Disable the temporary context
      alcDestroyContext(temp_context); // Destroy the temporary context
      alcCloseDevice(_temp_device); // Close the temporary device
      if (highest_version < (major_v * 10 + minor_v)) // Check if we found a higher version
      {
         highest_version = (major_v * 10 + minor_v);
         best_device = device_name;
      }
      device_name += strlen(device_name) + 1; // Go to the next device name in the list
   }

   // Open the 'best' device we found above
   _device = alcOpenDevice(best_device);
   if (_device == NULL) {
      cerr << "AUDIO ERROR: Failed to initialize OpenAL and open a default device." << endl;
      return false;
   }

	// Create an OpenAL context
	_context = alcCreateContext(_device, NULL);
	if (_context == NULL) {
		cerr << "AUDIO ERROR: Failed to create an OpenAL context." << endl;
		alcCloseDevice(_device);
		return false;
	}
	alcMakeContextCurrent(_context);
	alGetError(); // Clear the error code

	// Only necessary to initialize ALUT for version 1.*, aka freealut
	#ifdef ALUT_API_MAJOR_VERSION
	// Initialize ALUT without a context (it has already been created)
	if (alutInitWithoutContext(NULL, NULL) == AL_FALSE) {
		cerr << "AUDIO ERROR: Failed to initialize ALUT" << endl;
		cerr << "> " << GetALUTErrorString(alutGetError()) << endl;
		return false;
	}
	#endif

	// Create a single source for music
	_music_source = new MusicSource();
	if (!_music_source->IsValid()) {
		cerr << "AUDIO ERROR: Failed to create a single source." << endl;
		alcMakeContextCurrent(NULL); // Disable the active context
		alcDestroyContext(_context);
		alcCloseDevice(_device);
		return false;
	}

	// Create as many sources as possible for sounds
	uint32 i = 1;
	while (true) {
		SoundSource *SS = new SoundSource();
		if (SS->IsValid()) {
			_sound_sources.push_back(SS);
			// Set a maximum upper limit of 64 total sources (music + sounds). Any more than that is overkill.
			if (++i >= 64) {
				break;
			}
		}
		else {
			delete(SS);
			break;
		}
	}

	if (AUDIO_DEBUG) {
		cout << "AUDIO: Allocated " << _sound_sources.size() + 1 << " audio sources." << endl;
		DEBUG_PrintInfo();
	}
	return true;
}


// Returns a pointer to the sound buffer for the file. Loads the file from memory if it isn't already. Returns NULL if there was an error.
SoundBuffer* GameAudio::_AcquireSoundBuffer(string filename) {
	// If the buffer in question is already loaded, return a pointer to it.
	if (_sound_buffers.find(filename) != _sound_buffers.end()) {
		return _sound_buffers[filename];
	}

	// The buffer is not loaded, so create a new one, place it in the pool of buffers, and return a pointer to it
	else {
		SoundBuffer *SB = new SoundBuffer(filename);
		// Return a pointer to the new buffer if it was created successfully, otherwise return NULL.
		if (SB->IsValid()) {
			_sound_buffers[filename] = SB;
			return SB;
		}
		else {
			// Note: No error code is set here because this function is only called from LoadMusic() in MusicDescriptor
			if (AUDIO_DEBUG) cerr << "AUDIO ERROR: Unable to create a new SoundBuffer" << endl;
			delete(SB);
			return NULL;
		}
	}
}

// Returns a pointer to the music buffer for the file. Loads the file from memory if it isn't already. Returns NULL if there was an error.
MusicBuffer* GameAudio::_AcquireMusicBuffer(string filename) {
	// If the buffer in question is already loaded, return a pointer to it.
	if (_music_buffers.find(filename) != _music_buffers.end()) {
		return _music_buffers[filename];
	}

	// The buffer is not loaded, so create a new one, place it in the pool of buffers, and return a pointer to it
	else {
		MusicBuffer *MB = new MusicBuffer(filename);
		// Return a pointer to the new buffer if it was created successfully, otherwise return NULL.
		if (MB->IsValid()) {
			_music_buffers[filename] = MB;
			return MB;
		}
		else {
			// Note: No error code is set here because this function is only called from LoadMusic() in MusicDescriptor
			if (AUDIO_DEBUG) cerr << "AUDIO ERROR: Unable to create a new MusicBuffer" << endl;
			delete(MB);
			return NULL;
		}
	}
}



// Returns an available sound source.
SoundSource* GameAudio::_AcquireSoundSource() {
	// Get the state of the last (oldest) element in the vector
	ALint state;
	alGetSourcei(_sound_sources.back()->source, AL_SOURCE_STATE, &state);

	if (state == AL_PLAYING) {
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: All sound sources are occupied, force stopping a source" << endl;
		// Stop this source and release it.
		alSourceStop(_sound_sources.back()->source);
		_ReleaseSoundSource(_sound_sources.back());
	}

	// Make the LRU element now the MRU element by moving it from the back of the list to the front
	_sound_sources.push_front(_sound_sources.back());
	_sound_sources.pop_back();
	return _sound_sources.front();
}

// Stops any music that is playing, unbuffers and resets the source, and returns the source pointer.
MusicSource* GameAudio::_AcquireMusicSource() {
	if (_music_source->owner != NULL) {
		_music_source->owner->StopMusic();
		_ReleaseMusicSource(_music_source);
	}
	return _music_source;
}



void GameAudio::_ReleaseSoundSource(private_audio::SoundSource* free_source) {
	if (free_source->owner == NULL) // Then the source is already released
		return;

	free_source->owner->StopSound();
	// Maybe buffered audio data needs to be removed here???
	free_source->owner->_origin = NULL;
	free_source->owner = NULL;
}

void GameAudio::_ReleaseMusicSource(private_audio::MusicSource* free_source) {
	if (free_source->owner == NULL) // Then the source is already released
		return;

	free_source->owner->StopMusic();
	// Maybe buffered audio data needs to be removed here???
	free_source->owner->_origin = NULL;
	free_source->owner = NULL;
}


void GameAudio::SetMusicVolume(float vol) {
	if (vol < 0.0f) {
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: Tried to set music volume to a negative value." << endl;
		_music_volume = 0.0f;
	}
	else if (vol > 1.0f) {
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: Tried to set music volume above maximum level (1.0)." << endl;
		_music_volume = 1.0f;
	}
	else {
		_music_volume = vol;
	}

	alSourcef(_music_source->source, AL_GAIN, _music_volume);
}

void GameAudio::SetSoundVolume(float vol) {
	if (vol < 0.0f) {
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: Tried to set sound volume to a negative value." << endl;
		_sound_volume = 0.0f;
	}
	else if (vol > 1.0f) {
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: Tried to set sound volume above maximum level (1.0)." << endl;
		_sound_volume = 1.0f;
	}
	else {
		_sound_volume = vol;
	}

	for (list<SoundSource*>::iterator i = _sound_sources.begin(); i != _sound_sources.end(); i++) {
		alSourcef((*i)->source, AL_GAIN, _sound_volume);
	}
}



void GameAudio::PauseAudio() {
	PauseAllMusic();
	PauseAllSounds();
}

void GameAudio::ResumeAudio() {
	ResumeAllMusic();
	ResumeAllSounds();
}

void GameAudio::StopAudio() {
	StopAllMusic();
	StopAllSounds();
}

void GameAudio::RewindAudio() {
	RewindAllMusic();
	RewindAllSounds();
}



void GameAudio::PauseAllSounds() {
	for (list<SoundSource*>::iterator i = _sound_sources.begin(); i != _sound_sources.end(); i++) {
		// This will only effect sources that are currently playing.
		alSourcePause((*i)->source);
	}
}

void GameAudio::ResumeAllSounds() {
	ALint state;
	for (list<SoundSource*>::iterator i = _sound_sources.begin(); i != _sound_sources.end(); i++) {
		alGetSourcei((*i)->source, AL_SOURCE_STATE, &state);

		// Only play (resume) the source if it is paused
		if (state == AL_PAUSED) {
			alSourcePlay((*i)->source);
		}
	}
}

void GameAudio::StopAllSounds() {
	for (list<SoundSource*>::iterator i = _sound_sources.begin(); i != _sound_sources.end(); i++) {
		// All sources will go to the stopped state, if they aren't there already
		alSourceStop((*i)->source);
	}
}

void GameAudio::RewindAllSounds() {
	for (list<SoundSource*>::iterator i = _sound_sources.begin(); i != _sound_sources.end(); i++) {
		// All sources will go to the initial state, if they aren't there already
		alSourceRewind((*i)->source);
	}
}

void GameAudio::PauseAllMusic() {
	if (_music_source->owner != NULL) {
		_music_source->owner->PauseMusic();
	}
}

void GameAudio::ResumeAllMusic() {
	if (_music_source->owner != NULL) {
		_music_source->owner->ResumeMusic();
	}
}

void GameAudio::StopAllMusic() {
	if (_music_source->owner != NULL) {
		_music_source->owner->StopMusic();
	}
}

void GameAudio::RewindAllMusic() {
	if (_music_source->owner != NULL) {
		_music_source->owner->RewindMusic();
	}
}


uint8 GameAudio::GetDistanceModel() {
	ALint al_model;
	al_model = alGetInteger(AL_DISTANCE_MODEL);

	switch (al_model) {
		case AL_NONE:
			return AUDIO_DISTANCE_NONE;
		#ifdef AL_VERSION_1_1
		case AL_LINEAR_DISTANCE:
			return AUDIO_DISTANCE_LINEAR;
		case AL_LINEAR_DISTANCE_CLAMPED:
			return AUDIO_DISTANCE_LINEAR_CLAMPED;
		case AL_EXPONENT_DISTANCE:
			return AUDIO_DISTANCE_EXPONENT;
		case AL_EXPONENT_DISTANCE_CLAMPED:
			return AUDIO_DISTANCE_EXPONENT_CLAMPED;
		#endif // AL_VERSION_1_1
		case AL_INVERSE_DISTANCE:
			return AUDIO_DISTANCE_INVERSE;
		case AL_INVERSE_DISTANCE_CLAMPED:
			return AUDIO_DISTANCE_INVERSE_CLAMPED;
		default:
			if (AUDIO_DEBUG) cerr << "AUDIO WARNING: engine is using an unknown distance model: " << al_model << endl;
			return 0;
	}
}


void GameAudio::SetDistanceModel(uint8 model) {
	ALenum dist_model;
	switch (model) {
		case AUDIO_DISTANCE_NONE:
			dist_model = AL_NONE;
			break;
		#ifdef AL_VERSION_1_1
		case AUDIO_DISTANCE_LINEAR:
			dist_model = AL_LINEAR_DISTANCE;
			break;
		case AUDIO_DISTANCE_LINEAR_CLAMPED:
			dist_model = AL_LINEAR_DISTANCE_CLAMPED;
			break;
		case AUDIO_DISTANCE_EXPONENT:
			dist_model = AL_EXPONENT_DISTANCE;
			break;
		case AUDIO_DISTANCE_EXPONENT_CLAMPED:
			dist_model = AL_EXPONENT_DISTANCE_CLAMPED;
			break;
		#endif // AL_VERSION_1_1
		case AUDIO_DISTANCE_INVERSE:
			dist_model = AL_INVERSE_DISTANCE;
			break;
		case AUDIO_DISTANCE_INVERSE_CLAMPED:
			dist_model = AL_INVERSE_DISTANCE_CLAMPED;
			break;
		default:
			if (AUDIO_DEBUG) cerr << "AUDIO WARNING: attempted to use an invalid distance model: " << model << endl;
			return;
	}
	alDistanceModel(dist_model);
	// Check for openal errors here
}


// Prints information about that audio settings on the user's machine
void GameAudio::DEBUG_PrintInfo() {
	cout << "*** Audio Information ***" << endl;
	cout << "Available audio devices:     " << alcGetString(_device, ALC_DEVICE_SPECIFIER) << endl;
	cout << "Default audio device:        " << alcGetString(_device, ALC_DEFAULT_DEVICE_SPECIFIER) << endl;
	cout << "OpenAL Version:              " << alGetString(AL_VERSION) << endl;
	cout << "OpenAL Renderer:             " << alGetString(AL_RENDERER) << endl;
	cout << "OpenAL Vendor:               " << alGetString(AL_VENDOR) << endl;
	cout << "OpenAL Available Extensions: " << alGetString(AL_EXTENSIONS) << endl;

	#ifdef ALUT_API_MAJOR_VERSION
	cout << "ALUT Major Version:          " << alutGetMajorVersion() << endl;
	cout << "ALUT Minor Version:          " << alutGetMinorVersion() << endl;
	#else
	cout << "ALUT Major Version:          " << "not available" << endl;
	cout << "ALUT Minor Version:          " << "not available" << endl;
	#endif

	cout << "Audio sources created:       " << (_sound_sources.size() + 1) << endl;
}

} // namespace hoa_audio
