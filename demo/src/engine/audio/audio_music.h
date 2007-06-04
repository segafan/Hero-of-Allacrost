///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    audio_music.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for music-related code in the audio engine.
***
*** The classes in this file are used for management and processing of all music
*** data and playback.
***
*** \note This code uses the SDL_mixer audio library.
*** **************************************************************************/

#ifndef __AUDIO_MUSIC_HEADER__
#define __AUDIO_MUSIC_HEADER__

#include "utils.h"
#include "defs.h"
#include "audio.h"

namespace hoa_audio {

namespace private_audio {

/** ***************************************************************************
*** \brief An internal class used to manage music data information.
***
*** This class manages information about music data loaded into the application.
*** Objects of this class are managed internally by the audio engine and are
*** never referred to by the user.
*** **************************************************************************/
class MusicData {
public:
	MusicData(const std::string & fname);
	~MusicData();

	//! The filename of the audio data the buffer holds.
	std::string filename;
	//! The number of MusicDescriptor objects that refer to this MusicData object.
	uint8 reference_count;
	//! A pointer to the chunk of sound data loaded in memory.
	Mix_Music *music;
	//! Is this piece of music currently playing
	bool playing;
	
	//! Returns true if all OpenAL buffers are valid.
	bool IsValid();
	//! Removes a single reference to this buffer. If the reference count becomes zero, the buffer is destroyed.
	void RemoveReference();

	//! Displays the properties of the buffered data to standard output.
	void DEBUG_PrintProperties();
};

} // namespace private_audio

class MusicDescriptor {
	friend class GameAudio;
public:
	MusicDescriptor();
	~MusicDescriptor();

	//! Returns a const reference to the filename of the buffer that the source points to.
	const std::string &GetFilename()
	{ if (_data != NULL) return _data->filename; throw std::runtime_error("MusicDescriptor: Can't get the filename reference!"); }
	/** \brief Loads the music file from memory.
	*** \param fname The name of the file to load, without path information or file extension attached.
	**/
	bool LoadMusic(const std::string & fname);
	/** \brief Removes a reference to the music data.
	*** \note It is perfectly fine to call this function when the music is playing. If that is
	*** the case, the music will first be stopped, and then freed.
	**/
	void FreeMusic();

	/** \name Standard Music Operations
	*** \brief The basic functions that can change the state of music playback.
	*** \note Calling these functions when the music is already in a certain state results in a no-op.
	**/
	//@{
	void PlayMusic();
	void PauseMusic();
	void ResumeMusic();
	void StopMusic();
	void RewindMusic();
	void SeekMusic(float seconds);
	//@}

	/** \brief Retrieves the state of the music
	*** \return A value indicating the state. Refer to the audio state constants in audio.h.
	**/
	uint8 GetMusicState();

	//! Returns true if this piece of music is currently playing
	bool IsPlaying();

	/** \name Set Music Playback Properties Functions
	*** \param The value to set the playback property in question.
	**/
	//@{
	void SetLoopCount(int32 loops)
		{ _loop_count = loops; }
	void SetFadeInTime(uint32 fade_time)
		{ _fade_in_time = fade_time; }
	void SetFadeOutTime(uint32 fade_time)
		{ _fade_out_time = fade_time; }
	void SetPlayTimeout(uint32 timeout)
		{ _play_timeout = timeout; }
	//@}

	//! Displays the properties of the music descriptor's buffer.
	void DEBUG_dataProperties()
		{ if (_data != NULL) _data->DEBUG_PrintProperties(); }
private:
	//! A pointer to the music data that is used.
	private_audio::MusicData *_data;
	/** \brief The number of loops to play the sound for
	*** By default this member is set to -1, meaning it plays indefinitely until it is explicitly stopped.
	***  - A value of 0 indicates no looping (plays the sound once and then stops)
	***  - A value of -1 indicates infinite looping (until the user tells the sound to stop)
	**/
	int32 _loop_count;
	/** \brief The number of milliseconds to fade in the sound when playing begins
	*** A value of zero means that no fade in is done (this is the default).
	**/
	uint32 _fade_in_time;
	/** \brief The number of milliseconds to fade out the sound when the sound is stopped
	*** A value of zero means that no fade out is done (this is the default).
	**/
	uint32 _fade_out_time;
	/** \brief The number of milliseconds to play a sound before timing out and stopping the sound
	*** A value of -1 indicates no time out occurs (this is the default)
	**/
	int32 _play_timeout;
}; // class MusicDescriptor

} // namespace hoa_audio

#endif /* #ifndef __AUDIO_MUSIC_HEADER__ */
