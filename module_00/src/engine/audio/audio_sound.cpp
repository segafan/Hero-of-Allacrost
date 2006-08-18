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

#include "audio_sound.h"

using namespace std;
using namespace hoa_utils;

namespace hoa_audio {

namespace private_audio {

// ****************************************************************************
// ************************ SoundData Class Functions *************************
// ****************************************************************************

// Creates a new buffer for the filename and loads the WAV data.

SoundData::SoundData(string fname) {
	filename = fname;

	sound = Mix_LoadWAV(filename.c_str());
	if (sound == NULL) {
		cout << "AUDIO ERROR: Could not open sound file " << filename << ": " << Mix_GetError() << endl;
		reference_count = 0;
	}
	else {
		reference_count = 1;
	}
}

SoundData::~SoundData() {
	if (reference_count > 0) {
		cerr << "AUDIO WARNING: Audio engine is attempting to delete buffer " << filename
		     << ", which still has " << reference_count << " number of references to it." << endl;
	}

	Mix_FreeChunk(sound);
	sound = NULL;
}


void SoundData::RemoveReference() {
	reference_count--;
	if (reference_count == 0) {
		// TODO: delete the class object and remove it from the sound_data map, but be careful
		// delete this;
	}
}


bool SoundData::IsValid() {
	if (sound != NULL && reference_count > 0) {
		return true;
	}
	return false;
}


void SoundData::DEBUG_PrintProperties() {
	if (IsValid() == false) {
		AudioManager->_audio_errors |= AUDIO_ERROR_NO_DATA;
		cerr << "ERROR: the data for the file " << filename << " is not valid" << endl;
	}
	else {
		cout << "--------------------------------------------------------------------------------" << endl;
		cout << "Filename:      " << filename << endl;
		// These properties are not easily available with SDL_mixer
// 		cout << "Frequency:   " << property << " Hz" << endl;
// 		cout << "Bit depth:   " << property << endl;
// 		cout << "Channels:    " << property << endl;
		cout << "Size:        " << sound->alen << " bytes" << endl;
		cout << "--------------------------------------------------------------------------------" << endl;
	}
}

} // namespace private_audio

using namespace hoa_audio::private_audio;

// ****************************************************************************
// ***** SoundDescriptor Class Functions
// ****************************************************************************

// The constructor does not attempt to retrieve any resources.
SoundDescriptor::SoundDescriptor() {
	_data = NULL;
	_loop_count = 0;
	_fade_in_time = 0;
	_fade_out_time = 0;
	_play_timeout = -1;
}


// The destructor will call the AudioManager singleton to manage the buffer and source that the class was using.
SoundDescriptor::~SoundDescriptor() {
	if (_data != NULL) {
		_data->RemoveReference();
		_data = NULL;
	}
}


// This constructor will load the sound from memory, if it isn't loaded already.
bool SoundDescriptor::LoadSound(string fname) {
	if (fname == "") {
		cerr << "AUDIO WARNING: Bad filename passed in LoadSound (empty string)." << endl;
		return false;
	}

	_data = AudioManager->_AcquireSoundData(fname);
	if (_data == NULL) {
		cerr << "AUDIO ERROR: Failed to get a buffer for sound file " << fname << endl;
		return false;
	}
	return true;
}


void SoundDescriptor::FreeSound() {
	if (_data != NULL) {
		_data->RemoveReference();
		_data = NULL;
	}
}


// Retrieve the state of the audio
uint8 SoundDescriptor::GetSoundState() {
	if (_data == NULL) {
		return AUDIO_STATE_UNLOADED;
	}

	// This is needed because there is currently no guarantee if the _channel is still playing the chunk...
	if (Mix_GetChunk(_channel) != _data->sound) {
		return AUDIO_STATE_STOPPED;
	}

	switch(Mix_FadingChannel(_channel)) {
		case MIX_FADING_IN:
			return AUDIO_STATE_FADING_IN;
		case MIX_FADING_OUT:
			return AUDIO_STATE_FADING_OUT;
		default: // MIX_NO_FADING
			break;
	}
	
	if (Mix_Playing(_channel) != 0) {
		return AUDIO_STATE_PLAYING;
	}

	if (Mix_Paused(_channel) != 0) {
		return AUDIO_STATE_PAUSED;
	}

	return AUDIO_STATE_STOPPED;
}


void SoundDescriptor::PlaySound() {
	if (_data == NULL) {
		AudioManager->_audio_errors |= AUDIO_ERROR_NO_DATA;
		return;
	}

	if (_fade_in_time != 0) {
		_channel = Mix_FadeInChannelTimed(ANY_CHANNEL, _data->sound, _loop_count, _fade_in_time, _play_timeout);
	}
	else {
		_channel = Mix_PlayChannelTimed(ANY_CHANNEL, _data->sound, _loop_count, _play_timeout);
	}

	if (_channel == -1) {
		AudioManager->_audio_errors |= AUDIO_ERROR_PLAY_FAILURE;
	}
}


void SoundDescriptor::PauseSound() {
	if (_data == NULL) {
		AudioManager->_audio_errors |= AUDIO_ERROR_NO_DATA;
		return;
	}
	
	Mix_Pause(_channel);
}


void SoundDescriptor::ResumeSound() {
	if (_data == NULL) {
		AudioManager->_audio_errors |= AUDIO_ERROR_NO_DATA;
		return;
	}
	
	Mix_Resume(_channel);
}


void SoundDescriptor::StopSound() {
	if (_data == NULL) {
		AudioManager->_audio_errors |= AUDIO_ERROR_NO_DATA;
		return;
	}
	
	if (_fade_out_time != 0) {
		Mix_FadeOutChannel(_channel, _fade_out_time);
	}
	else {
		Mix_HaltChannel(_channel);
	}
}


// void SoundDescriptor::RewindSound() {}

// void SoundDescriptor::FreeSound() {}


} // namespace hoa_audio
