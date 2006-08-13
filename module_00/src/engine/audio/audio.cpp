///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    audio.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for audio engine interface.
*** **************************************************************************/

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

	Mix_HaltMusic();
	Mix_HaltChannel(ALL_CHANNELS);

	// Delete all sound and music data
	for (std::map<string, SoundData*>::iterator i = _sound_data.begin(); i != _sound_data.end(); i++) {
		delete i->second;
	}
	_sound_data.clear();
	
	for (std::map<string, MusicData*>::iterator i = _music_data.begin(); i != _music_data.end(); i++) {
		delete i->second;
	}
	_music_data.clear();

	Mix_CloseAudio();
}


// Initializes OpenAL and creates the global audio context
bool GameAudio::Initialize() {
	if (AUDIO_DEBUG) cout << "AUDIO: GameAudio constructor" << endl;

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
		cerr << "AUDIO ERROR: Could not initalize SDL audio subsystem: " << SDL_GetError() << endl;
		return false;
	}

	// Open 22.05KHz, signed 16bit, system byte order, stereo audio, using 1024 byte chunks
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, BUFFER_SIZE) == -1) {
		cerr << "AUDIO ERROR: Could not initialize mixer audio: " << Mix_GetError() << endl;
		return false;
	}
	
	Mix_AllocateChannels(SOUND_CHANNELS);
	return true;
}


// Returns a pointer to the sound buffer for the file. Loads the file from memory if it isn't already. Returns NULL if there was an error.
SoundData* GameAudio::_AcquireSoundData(string filename) {
	// If the data in question is already loaded, return a pointer to it.
	if (_sound_data.find(filename) != _sound_data.end()) {
		return _sound_data[filename];
	}

	// The data is not loaded, so load it, place it in the data map, and return a pointer to it
	else {
		SoundData *SD = new SoundData(filename);
		// Return a pointer to the new data if it was created successfully, otherwise return NULL.
		if (SD->IsValid()) {
			_sound_data[filename] = SD;
			return SD;
		}
		else {
			// Note: No error code is set here because this function is only called from LoadMusic() in MusicDescriptor
			if (AUDIO_DEBUG) cerr << "AUDIO ERROR: Unable to create a new SoundData" << endl;
			delete(SD);
			return NULL;
		}
	}
}

// Returns a pointer to the music buffer for the file. Loads the file from memory if it isn't already. Returns NULL if there was an error.
MusicData* GameAudio::_AcquireMusicData(string filename) {
	// If the buffer in question is already loaded, return a pointer to it.
	if (_music_data.find(filename) != _music_data.end()) {
		return _music_data[filename];
	}

	// The data is not loaded, so load it, place it in the data map, and return a pointer to it
	else {
		MusicData *MD = new MusicData(filename);
		// Return a pointer to the new data if it was created successfully, otherwise return NULL.
		if (MD->IsValid()) {
			_music_data[filename] = MD;
			return MD;
		}
		else {
			// Note: No error code is set here because this function is only called from LoadMusic() in MusicDescriptor
			if (AUDIO_DEBUG) cerr << "AUDIO ERROR: Unable to create a new MusicData" << endl;
			delete(MD);
			return NULL;
		}
	}
}


void GameAudio::SetMusicVolume(float vol) {
	if (vol > 1.0f) {
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: Tried to set music volume above maximum level" << endl;
		_music_volume = 1.0f;
	}
	else if (vol < 0.0f)
	{
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: Tried to set music volume below minimum level" << endl;
		_music_volume = 0.0f;
	}
	else {
		_music_volume = vol;
	}

	uint8 volume = static_cast<uint8>(_music_volume * 128.0f); // scale for SDL_Mixer
	Mix_VolumeMusic(volume);
}


void GameAudio::SetSoundVolume(float vol) {
	if (vol > 1.0f) {
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: Tried to set sound volume above maximum level" << endl;
		_sound_volume = 1.0f;
	}
	else if (vol < 0.0f)
	{
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: Tried to set sound volume below minimum level" << endl;
		_sound_volume = 0.0f;
	}
	else {
		_sound_volume = vol;
	}

	uint8 volume = static_cast<uint8>(_sound_volume * 128.0f); // scale for SDL_Mixer
	Mix_Volume(ALL_CHANNELS, volume);
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
// 	RewindAllSounds();
}


void GameAudio::PauseAllSounds() {
	Mix_Pause(ALL_CHANNELS);
}


void GameAudio::ResumeAllSounds() {
	Mix_Resume(ALL_CHANNELS);
}


void GameAudio::StopAllSounds() {
	Mix_HaltChannel(ALL_CHANNELS);
}


// void GameAudio::RewindAllSounds() {}


void GameAudio::PauseAllMusic() {
	Mix_PauseMusic();
	for (std::map<string, MusicData*>::iterator i = _music_data.begin(); i != _music_data.end(); i++) {
		i->second->playing = false;
	}
}


void GameAudio::ResumeAllMusic() {
	Mix_ResumeMusic();
	for (std::map<string, MusicData*>::iterator i = _music_data.begin(); i != _music_data.end(); i++) {
		i->second->playing = true;
	}
}


void GameAudio::StopAllMusic() {
	Mix_HaltMusic();
	for (std::map<string, MusicData*>::iterator i = _music_data.begin(); i != _music_data.end(); i++) {
		i->second->playing = false;
	}
}


void GameAudio::RewindAllMusic() {
	Mix_RewindMusic();
}


// Prints information about that audio settings on the user's machine
void GameAudio::DEBUG_PrintInfo() {
	cout << "*** Audio Information ***" << endl;
	cout << "SDL_mixer version: " << MIX_MAJOR_VERSION << '.' << MIX_MINOR_VERSION << '.' << MIX_PATCHLEVEL << endl;
	cout << "Number of mixing channels:   " << MIX_CHANNELS << endl;
	cout << "Number of playback channels: " << MIX_DEFAULT_CHANNELS << endl;
	cout << "Playback frequency: " << MIX_DEFAULT_FREQUENCY << endl;
	cout << "Playback format: " << MIX_DEFAULT_FORMAT << endl;
}

} // namespace hoa_audio
