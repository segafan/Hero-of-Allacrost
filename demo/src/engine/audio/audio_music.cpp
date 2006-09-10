///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    audio_music.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for music-related code in the audio engine.
*** **************************************************************************/

#include "audio_music.h"

using namespace std;
using namespace hoa_utils;

namespace hoa_audio {

namespace private_audio {

// ****************************************************************************
// ************************ MusicData Class Functions *************************
// ****************************************************************************

MusicData::MusicData(const std::string & fname) :
	filename(fname),
	playing(false)
{
	music = Mix_LoadMUS(filename.c_str());
	if (music == NULL) {
		cout << "AUDIO ERROR: Could not open music file " << filename << ": " << Mix_GetError() << endl;
		reference_count = 0;
	}
	else {
		reference_count = 1;
	}
}


MusicData::~MusicData() {
	if (reference_count > 0) {
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: freeing music data with a non-zero reference count" << endl;
	}
	
	Mix_FreeMusic(music);
	music = NULL;
}


bool MusicData::IsValid() {
	if (music != NULL && reference_count > 0) {
		return true;
	}
	return false;
}


// Remove a reference to this buffer object and delete it if there are no more references.
void MusicData::RemoveReference() {
	reference_count--;
	if (reference_count == 0) {
		// TODO: delete the class object and remove it from the music_data map, but be careful
		// delete this;
	}
}


void MusicData::DEBUG_PrintProperties() {
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
// 		cout << "Size:        " << _chunk->alen << " bytes" << endl;
		cout << "--------------------------------------------------------------------------------" << endl;
	}
}


} // namespace private_audio

using namespace hoa_audio::private_audio;

// ****************************************************************************
// ********************* MusicDescriptor Class Functions **********************
// ****************************************************************************


MusicDescriptor::MusicDescriptor() :
	_data(NULL),
	_loop_count(-1),
	_fade_in_time(0),
	_fade_out_time(0),
	_play_timeout(-1)
{
}


MusicDescriptor::~MusicDescriptor() {
	if (_data != NULL) {
		_data->RemoveReference();
		_data = NULL;
	}
}


bool MusicDescriptor::LoadMusic(const std::string & fname) {
	// If the music descriptor is already using other audio data, remove a reference to that data
	if (_data != NULL) {
		_data->RemoveReference();
		_data = NULL;
	}
	
	_data = AudioManager->_AcquireMusicData(fname);
	if (_data == NULL) {
		return false;
	}
	else {
		return true;
	}
}

void MusicDescriptor::FreeMusic() {
	if (_data != NULL) {
		_data->RemoveReference();
		_data = NULL;
	}
}


void MusicDescriptor::PlayMusic() {
	

	if (_data == NULL) {
		return;
	}

	if (GetMusicState() != AUDIO_STATE_STOPPED) {
		AudioManager->StopAllMusic();
	}

	if (Mix_FadeInMusic(_data->music, _loop_count, _fade_in_time) == -1) {
			cerr << "AUDIO ERROR: Could not play music" << endl;
	}

	_data->playing = true;
}


void MusicDescriptor::PauseMusic() {
	if (_data == NULL) {
		AudioManager->_audio_errors |= AUDIO_ERROR_NO_DATA;
		return;
	}

	Mix_PauseMusic();

	_data->playing = false;
}


void MusicDescriptor::ResumeMusic() {
	if (_data == NULL) {
		AudioManager->_audio_errors |= AUDIO_ERROR_NO_DATA;
		return;
	}

	Mix_ResumeMusic();

	_data->playing = true;
}


void MusicDescriptor::StopMusic() {
	if (_data == NULL) {
		AudioManager->_audio_errors |= AUDIO_ERROR_NO_DATA;
		return;
	}

	Mix_FadeOutMusic(_fade_out_time);

	_data->playing = false;
}


void MusicDescriptor::RewindMusic() {
	if (_data == NULL) {
		AudioManager->_audio_errors |= AUDIO_ERROR_NO_DATA;
		return;
	}

	Mix_RewindMusic();
}


void MusicDescriptor::SeekMusic(float seconds) {
	if (_data == NULL) {
		AudioManager->_audio_errors |= AUDIO_ERROR_NO_DATA;
		return;
	}

	Mix_SetMusicPosition(seconds);
}


uint8 MusicDescriptor::GetMusicState() {
	if (_data == NULL) {
		return AUDIO_STATE_UNLOADED;
	}

	switch(Mix_FadingMusic()) {
		case MIX_FADING_IN:
			return AUDIO_STATE_FADING_IN;
		case MIX_FADING_OUT:
			return AUDIO_STATE_FADING_OUT;
		default: // MIX_NO_FADING
			break;
	}
	
	if (Mix_PlayingMusic() != 0) {
		return AUDIO_STATE_PLAYING;
	}

	if (Mix_PausedMusic() != 0) {
		return AUDIO_STATE_PAUSED;
	}

	return AUDIO_STATE_STOPPED;
}


bool MusicDescriptor::IsPlaying()
{
	if (_data != NULL) {
		return _data->playing;
	}
	else
	{
		return false;
	}
}


} // namespace hoa_audio
