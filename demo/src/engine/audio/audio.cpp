///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    audio.cpp
*** \author  Tyler Olsen - Moisés Ferrer Serra - Aaron Smith, roots@allacrost.org - byaku@allacrost.org - etherstar@allacrost.org
*** \brief   Implementation of the audio engine singleton.
***
*** The code included here implements the interface of the audio singleton.
***
*** \note This code uses the OpenAL audio library. See http://www.openal.com/
*** ***************************************************************************/

#include "audio.h"
#include "system.h"

#include <iostream>

using namespace std;
using namespace hoa_utils;
using namespace hoa_system;
using namespace hoa_audio::private_audio;


template<> hoa_audio::GameAudio* Singleton<hoa_audio::GameAudio>::_singleton_reference = 0;


namespace hoa_audio {

GameAudio* AudioManager = NULL;
bool AUDIO_DEBUG = false;



GameAudio::GameAudio () :
	_sound_volume(1.0f),
	_music_volume(1.0f),
	_device(0),
	_context(0),
	_max_sources(MAX_DEFAULT_AUDIO_SOURCES)
{}




bool GameAudio::SingletonInitialize() {
	const ALCchar* best_device = 0; // Will store the name of the 'best' device for audio playback
	ALCint highest_version = 0; // The highest version number found
	CheckALError(); // Clears errors
	CheckALCError(); // Clears errors

	// Find the highest-version device available
	const ALCchar* device_name = alcGetString(0, ALC_DEVICE_SPECIFIER); // Get list of all devices (terminated with two '0')
	if (CheckALCError() == true) {
		PRINT_WARNING << "failed to retreive the list of available audio devices: " << CreateALCErrorString() << endl;
	}

	while (*device_name != 0) { // Check all the detected devices
		ALCint major_v = 0, minor_v = 0;

		// Open a temporary device for reading in its version number
		ALCdevice* temp_device = alcOpenDevice(device_name);
		if (CheckALCError() || temp_device == NULL) { // If we couldn't open the device, just move on to the next
			IF_PRINT_WARNING(AUDIO_DEBUG) << "couldn't open device for version checking: " << device_name << endl;
			device_name += strlen(device_name) + 1;
			continue;
		}

		// Create a temporary context for the device
		ALCcontext *temp_context = alcCreateContext(temp_device, 0);
		if (CheckALCError() || temp_context == NULL) { // If we couldn't create the context, move on to the next device
			IF_PRINT_WARNING(AUDIO_DEBUG) << "couldn't create a temporary context for device: " << device_name << endl;
			alcCloseDevice(temp_device);
			device_name += strlen(device_name) + 1;
			continue;
		}

		// Retrieve the version number for the device
		alcMakeContextCurrent(temp_context);

		alcGetIntegerv(temp_device, ALC_MAJOR_VERSION, sizeof(ALCint), &major_v);
		alcGetIntegerv(temp_device, ALC_MINOR_VERSION, sizeof(ALCint), &minor_v);
		alcMakeContextCurrent(0); // Disable the temporary context
		alcDestroyContext(temp_context); // Destroy the temporary context
		alcCloseDevice(temp_device); // Close the temporary device

		// Check if a higher version device was found
		if (highest_version < (major_v * 10 + minor_v)) {
			highest_version = (major_v * 10 + minor_v);
			best_device = device_name;
		}
		device_name += strlen(device_name) + 1; // Go to the next device name in the list
	} // while (*device_name != 0)

	// Open the 'best' device we found above. If no devices were previously found,
	// it will try opening the default one (0)
	_device = alcOpenDevice(best_device);
	if (CheckALCError() || _device == NULL) {
		PRINT_ERROR << "failed to open an OpenAL audio device: " << CreateALCErrorString() << endl;
		return false;
	}

	// Create an OpenAL context
	_context = alcCreateContext(_device, NULL);
	if (CheckALCError() || _context == NULL) {
		PRINT_ERROR << "failed to create an OpenAL context: " << CreateALCErrorString()<< endl;
		alcCloseDevice(_device);
		return false;
	}
	
	alcMakeContextCurrent(_context);
	CheckALError(); // Clear errors
	CheckALCError(); // Clear errors

	// Create as many sources as possible (we fix an upper bound of _max_sources)
	ALuint source;
	for (uint16 i = 0; i < _max_sources; i++) {
		alGenSources(1, &source);
		if (CheckALError() == true) {
			_max_sources = i;
			break;
		}
		_source.push_back(new private_audio::AudioSource);
		_source.back()->source = source;
	}

	if (_max_sources == 0) {
		PRINT_ERROR << "failed to create at least one OpenAL audio source" << endl;
		return false;
	}

	return true;
} // bool GameAudio::SingletonInitialize()



GameAudio::~GameAudio() {
	// Delete all entries in the sound cache
	for (map<string, SoundDescriptor*>::iterator i = _sound_cache.begin(); i != _sound_cache.end(); i++) {
		delete i->second;
	}
	_sound_cache.clear();

	// Delete all audio sources
	for (vector<AudioSource*>::iterator i = _source.begin(); i != _source.end(); i++) {
		alDeleteSources(1, &(*i)->source);
	}
	_source.clear();
	
	// Delete all audio buffers
	for (vector<AudioBuffer*>::iterator i = _buffer.begin(); i != _buffer.end(); i++) {
		if ((*i)->IsValid()) {
			alDeleteBuffers(1, &(*i)->buffer);
		}
	}
	_buffer.clear();

	alcMakeContextCurrent(0);
	alcDestroyContext(_context);
	alcCloseDevice(_device);
}



void GameAudio::Update () {
	for (vector<AudioSource*>::iterator i = _source.begin(); i != _source.end(); i++) {
		if ((*i)->owner != NULL) {
			(*i)->owner->_Update();
		}
	}

	_fx_manager.Update();
}



void GameAudio::SetSoundVolume(float volume) {
	if (volume < 0.0f) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "tried to set sound volume less than 0.0f: " << volume << endl;
		_sound_volume = 0.0f;
	}
	else if (volume > 1.0f) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "tried to set sound volume greater than 1.0f: " << volume << endl;
		_sound_volume = 1.0f;
	}
	else {
		_sound_volume = volume;
	}

	for (list<SoundDescriptor*>::iterator i = _sound.begin(); i != _sound.end(); i++) {
		(*i)->SetVolume(_sound_volume);
	}
}



void GameAudio::SetMusicVolume(float volume) {
	if (volume < 0.0f) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "tried to set music volume less than 0.0f: " << volume << endl;
		_music_volume = 0.0f;
	}
	else if (volume > 1.0f) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "tried to set music volume greater than 1.0f: " << volume << endl;
		_music_volume = 1.0f;
	}
	else {
		_music_volume = volume;
	}

	for (list<MusicDescriptor*>::iterator i = _music.begin(); i != _music.end(); i++) {
		(*i)->SetVolume(_music_volume);
	}
}



void GameAudio::PauseAllSounds() {
	for (list<SoundDescriptor*>::iterator i = _sound.begin(); i != _sound.end(); i++) {
		(*i)->Pause();
	}
}



void GameAudio::ResumeAllSounds() {
	for (list<SoundDescriptor*>::iterator i = _sound.begin(); i != _sound.end(); i++) {
		(*i)->Resume();
	}
}



void GameAudio::StopAllSounds() {
	for (list<SoundDescriptor*>::iterator i = _sound.begin(); i != _sound.end(); i++) {
		(*i)->Stop();
	}
}



void GameAudio::RewindAllSounds() {
	for (list<SoundDescriptor*>::iterator i =_sound.begin(); i != _sound.end(); i++) {
		(*i)->Rewind();
	}
}



void GameAudio::PauseAllMusic() {
	for (list<MusicDescriptor*>::iterator i = _music.begin(); i != _music.end(); i++) {
		(*i)->Pause();
	}
}



void GameAudio::ResumeAllMusic() {
	for (list<MusicDescriptor*>::iterator i = _music.begin(); i != _music.end(); i++) {
		(*i)->Resume();
	}
}



void GameAudio::StopAllMusic() {
	for (list<MusicDescriptor*>::iterator i = _music.begin(); i != _music.end(); i++) {
		(*i)->Stop();
	}
}



void GameAudio::RewindAllMusic() {
	for (list<MusicDescriptor*>::iterator i = _music.begin(); i != _music.end(); i++) {
		(*i)->Rewind();
	}
}



void GameAudio::SetListenerPosition(const float position[3]) {
	alListenerfv(AL_POSITION, position);
	memcpy(_listener_position, position, sizeof(float) * 3);
}



void GameAudio::SetListenerVelocity(const float velocity[3]) {
	alListenerfv(AL_VELOCITY, velocity);
	memcpy(_listener_velocity, velocity, sizeof(float) * 3);
}



void GameAudio::SetListenerOrientation(const float orientation[3]) {
	alListenerfv(AL_ORIENTATION, orientation);
	memcpy(_listener_orientation, orientation, sizeof(float) * 3);
}



void GameAudio::PlaySound(const std::string& filename) {
	SoundDescriptor* new_sound = new SoundDescriptor();

	if (new_sound->LoadAudio(filename) == false) {
		IF_PRINT_WARNING(AUDIO_DEBUG) << "could not load new audio file into sound cache: " << filename << endl;
		delete new_sound;
		return;
	}

	_sound_cache.insert(make_pair(filename, new_sound));
	new_sound->Play();
}



const std::string GameAudio::CreateALErrorString() {
	switch (_al_error_code) {
		case AL_NO_ERROR:
			return "AL_NO_ERROR";
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
		default:
			return ("Unknown AL error code: " + NumberToString(_al_error_code));
	}
}



const std::string GameAudio::CreateALCErrorString() {
	switch (_alc_error_code) {
		case ALC_NO_ERROR:
			return "ALC_NO_ERROR";
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
			return ("Unknown ALC error code: " + NumberToString(_alc_error_code));
	}
}



void GameAudio::DEBUG_PrintInfo() {
	const ALCchar* c;

	cout << "*** Audio Information ***" << endl;

	cout << "Maximum number of sources:   " << _max_sources << endl;
	cout << "Available audio devices:     " << alcGetString (_device, ALC_DEVICE_SPECIFIER) << endl;
	cout << "Default audio device:        " << alcGetString (_device, ALC_DEFAULT_DEVICE_SPECIFIER) << endl;
	cout << "OpenAL Version:              " << alGetString (AL_VERSION) << endl;
	cout << "OpenAL Renderer:             " << alGetString (AL_RENDERER) << endl;
	cout << "OpenAL Vendor:               " << alGetString (AL_VENDOR) << endl;

	CheckALError();

	cout << "Available OpenAL Extensions:" << endl;
	c = alGetString(AL_EXTENSIONS);
	bool new_extension = true;
	while (c[0]) {
		if (new_extension) {
			cout << " - ";
			new_extension = false;
			continue;
		}
		else if (c[0] == ' ') {
			cout << endl;
			new_extension = true;
			c++;
			continue;
		}
		
		cout << c[0];
		c++;
	}
}



private_audio::AudioSource* GameAudio::_AcquireAudioSource () {
	for (vector<AudioSource*>::iterator i = _source.begin(); i != _source.end(); i++) {
		if ((*i)->owner == NULL) {
			return *i;
		}
	}

	return NULL;
}

} // namespace hoa_audio
