///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    audio.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 23rd, 2005
 * \brief   Source file for audio engine interface.
 *****************************************************************************/

#include "audio.h"

using namespace std;
using namespace hoa_utils;

namespace hoa_audio {

GameAudio *AudioManager = NULL;
bool AUDIO_DEBUG = false;
SINGLETON_INITIALIZE(GameAudio);

namespace private_audio {

// Returns an error string corresponding to a particular error code
string GetALErrorString(ALenum err) {
	switch(err) {
		case AL_INVALID_NAME:
			return "AL_INVALID_NAME";
			break;
		case AL_INVALID_ENUM:
			return "AL_INVALID_ENUM";
			break;
		case AL_INVALID_VALUE:
			return "AL_INVALID_VALUE";
			break;
		case AL_INVALID_OPERATION:
			return "AL_INVALID_OPERATION";
			break;
		case AL_OUT_OF_MEMORY:
			return "AL_OUT_OF_MEMORY";
			break;
		default:
			return "AL_NO_ERROR";
	}
}

// Returns an error string corresponding to a particular error code
string GetALCErrorString(ALenum err) {
	switch(err) {
		case ALC_INVALID_DEVICE:
			return "ALC_INVALID_DEVICE";
			break;
		case ALC_INVALID_CONTEXT:
			return "ALC_INVALID_CONTEXT";
			break;
		case ALC_INVALID_ENUM:
			return "ALC_INVALID_ENUM";
			break;
		case ALC_INVALID_VALUE:
			return "ALC_INVALID_VALUE";
			break;
		case ALC_OUT_OF_MEMORY:
			return "ALC_OUT_OF_MEMORY";
			break;
		default:
			return "AL_NO_ERROR";
	}
}

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

	// ************ (2) Delete all audio buffers ******************
	for (std::map<string, MusicBuffer*>::iterator i = _music_buffers.begin(); i != _music_buffers.end(); i++) {
		delete(i->second);
	}
	_music_buffers.clear();
	for (std::map<string, SoundBuffer*>::iterator i = _sound_buffers.begin(); i != _sound_buffers.end(); i++) {
		delete(i->second);
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
	_device = alcOpenDevice(NULL); // Opens the default device
	if (_device == NULL) {
		cerr << "AUDIO ERROR: Failed to initialize OpenAL and open a default device." << endl;
		return false;
	}

	_context = alcCreateContext(_device, NULL);
	if (_context == NULL) {
		cerr << "AUDIO ERROR: Failed to create an OpenAL context." << endl;
		alcCloseDevice(_device);
		return false;
	}
	alcMakeContextCurrent(_context);
	alGetError(); // Clear the error code

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

	if (AUDIO_DEBUG) cout << "AUDIO: Allocated " << _sound_sources.size() + 1 << " audio sources." << endl;
	return true;
}


// Returns a pointer to the sound buffer for the file. Loads the file from memory if it isn't already. Returns NULL if there was an error.
SoundBuffer* GameAudio::_AcquireSoundBuffer(string filename) {
	if (_sound_buffers.find(filename) == _sound_buffers.end()) { // Then the sound isn't in the buffer map, so load it.
		SoundBuffer *SB = new SoundBuffer(filename);
		if (!SB->IsValid()) {
			// No error code is set here, because this function is only called from LoadSound() in SoundDescriptor
			if (AUDIO_DEBUG) cerr << "AUDIO ERROR: Unable to create a new SoundBuffer" << endl;
			delete(SB);
			return NULL;
		}
		else {
			_sound_buffers[filename] = SB;
			return SB;
		}
	}
	else { // The file is already loaded, so return a pointer to its buffer
		return _sound_buffers[filename];
	}
}

// Returns a pointer to the music buffer for the file. Loads the file from memory if it isn't already. Returns NULL if there was an error.
MusicBuffer* GameAudio::_AcquireMusicBuffer(string filename) {
	if (_music_buffers.find(filename) == _music_buffers.end()) { // Then the music isn't in the buffer map, so load it.
		MusicBuffer *MB = new MusicBuffer(filename);
		if (!MB->IsValid()) {
			// No error code is set here, because this function is only called from LoadMusic() in MusicDescriptor
			if (AUDIO_DEBUG) cerr << "AUDIO ERROR: Unable to create a new MusicBuffer" << endl;
			delete(MB);
			return NULL;
		}
		else {
			_music_buffers[filename] = MB;
			return MB;
		}
	}
	else { // The file is already loaded, so return a pointer to its buffer
		return _music_buffers[filename];
	}
}



// Returns an available sound source.
SoundSource* GameAudio::_AcquireSoundSource() {
	// Get the state of the last (oldest) element in the vector
	ALint state;
	alGetSourcei(_sound_sources.back()->source, AL_SOURCE_STATE, &state);

	if (state == AL_PLAYING) {
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: All sources are occupied!" << endl;
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
// 		case AL_LINEAR:
// 			return AUDIO_DISTANCE_LINEAR;
// 		case AL_LINEAR_CLAMPED:
// 			return AUDIO_DISTANCE_LINEAR_CLAMPED;
		case AL_INVERSE_DISTANCE:
			return AUDIO_DISTANCE_INVERSE;
		case AL_INVERSE_DISTANCE_CLAMPED:
			return AUDIO_DISTANCE_INVERSE_CLAMPED;
// 		case AL_EXPONENT:
// 			return AUDIO_DISTANCE_EXPONENT;
// 		case AL_EXPONENT_CLAMPED:
// 			return AUDIO_DISTANCE_EXPONENT_CLAMPED;
		default:
			if (AUDIO_DEBUG) cerr << "AUDIO WARNING: using unknown OpenAL distance model: " << al_model << endl;
			return 0;
	}
}


void GameAudio::SetDistanceModel(uint8 model) {
	ALenum dist_model;
	switch (model) {
		case AUDIO_DISTANCE_NONE:
			dist_model = AL_NONE;
			break;
// 		case AUDIO_DISTANCE_LINEAR:
// 			dist_model = AL_LINEAR;
// 			break;
// 		case AUDIO_DISTANCE_LINEAR_CLAMPED:
// 			dist_model = AL_LINEAR_CLAMPED;
// 			break;
		case AUDIO_DISTANCE_INVERSE:
			dist_model = AL_INVERSE_DISTANCE;
			break;
		case AUDIO_DISTANCE_INVERSE_CLAMPED:
			dist_model = AL_INVERSE_DISTANCE_CLAMPED;
			break;
// 		case AUDIO_DISTANCE_EXPONENT:
// 			dist_model = AL_EXPONENT;
// 			break;
// 		case AUDIO_DISTANCE_EXPONENT_CLAMPED:
// 			dist_model = AL_EXPONENT_CLAMPED;
// 			break;
		default:
			if (AUDIO_DEBUG) cerr << "AUDIO WARNING: attempted to set an invalid distance model: " << model << endl;
			return;
	}
	alDistanceModel(dist_model);
	// Check for openal errors here
}


// Prints information about that audio settings on the user's machine
void GameAudio::DEBUG_PrintInfo() {
	cout << "*** Audio Information ***" << endl;
	cout << "List of available audio devices: " << alcGetString(_device, ALC_DEVICE_SPECIFIER) << endl;
	cout << "Default audio device:            " << alcGetString(_device, ALC_DEFAULT_DEVICE_SPECIFIER) << endl;
	cout << "OpenAL Version:                  " << alGetString(AL_VERSION) << endl;
	cout << "OpenAL Renderer:                 " << alGetString(AL_RENDERER) << endl;
	cout << "OpenAL Vendor:                   " << alGetString(AL_VENDOR) << endl;
	cout << "OpenAL Available Extensions:     " << alGetString(AL_EXTENSIONS) << endl;
	cout << "Available number of sources:     " << (_sound_sources.size() + 1) << endl;
}

} // namespace hoa_audio
