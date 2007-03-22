///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    audio_sound.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for sound-related code in the audio engine.
***
*** The classes in this file are used for management and processing of all sound
*** data.
***
*** \note This code uses the SDL_mixer audio library.
*** **************************************************************************/

#ifndef __AUDIO_SOUND_HEADER__
#define __AUDIO_SOUND_HEADER__

#include "utils.h"
#include "defs.h"
#include "audio.h"

namespace hoa_audio {

namespace private_audio {

/** ***************************************************************************
*** \brief An internal class used to manage sound data information.
***
*** This class manages information about sound data loaded into the application.
*** Objects of this class are managed internally by the audio engine and are
*** never referred to by the user.
*** **************************************************************************/
class SoundData {
public:
	SoundData(std::string fname);
	~SoundData();

	//! The filename of the audio data the buffer holds.
	std::string filename;
	//! The number of SoundDescriptors referring to this buffer.
	int8 reference_count;
	//! A pointer to the chunk of sound data loaded in memory.
	Mix_Chunk *sound;

	//! Returns true if the OpenAL buffer is valid.
	bool IsValid();
	//! Removes a single reference to this buffer. If the reference count becomes zero, the class object is destroyed.
	void RemoveReference();
	//! Displays the properties of the data to standard output.
	void DEBUG_PrintProperties();
};

} // namespace private_audio

/*!****************************************************************************
 * \brief Manages sound data loaded from memory.
 *
 * The purpose of this class is to provide the API user with an easy-to-use
 * interface for manipulating sound data.This class holds all the properties
 * about a given sound. Including its location, velocity, whether or not it
 * loops, and numerous other properties.
 *****************************************************************************/
class SoundDescriptor {
	friend class GameAudio;
public:
	SoundDescriptor();
	~SoundDescriptor();
	
	//! Returns a const reference to the filename of the buffer that the source points to.
	const std::string &GetFilename()
		{ if (_data != NULL) return _data->filename; }
	/** \brief Loads new sound data from a file.
	*** \param fname The name of the file, without path information or a file extension.
	*** \return Returns false if there was an error loading the sound.
	**/
	bool LoadSound(std::string fname);
	/** \brief Removes a reference to the sound data.
	*** \note It is perfectly fine to call this function when a sound is playing. If that is
	*** the case, the sound will first be stopped, and then freed.
	**/
	void FreeSound();

	/** \name Standard Sound Operations
	*** \brief The basic functions that can change the state of sound playback.
	*** \note Calling these functions when the sound is already in a certain state results in a no-op.
	**/
	//@{
	void PlaySound();
	void PauseSound();
	void ResumeSound();
	void StopSound();
	// Rewinding and seeking of sounds is not supported in SDL_mixer
// 	void RewindSound();
// 	void SeekSound(float seconds);
	//@}

	/** \brief Retrieves the state of the sound
	*** \return A value indicating the state. Refer to the audio state constants in audio.h.
	**/
	uint8 GetSoundState();

	/** \name Set Sound Playback Properties Functions
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

	/** \name Retreive Sound Playback Properties Functions
	*** \return The value of the playback property that was queried
	**/
	//@{
	int32 GetLoopCount()
		{ return _loop_count; }
	uint32 GetFadeInTime()
		{ return _fade_in_time; }
	uint32 GetFadeOutTime()
		{ return _fade_out_time; }
	int32 GetPlayTimeout()
		{ return _play_timeout; }
	//@}

	//! Displays the properties of the sound descriptor's buffer.
	void DEBUG_DataProperties()
		{ if (_data != NULL) _data->DEBUG_PrintProperties(); }
private:
	//! A pointer to the sound data that is used.
	private_audio::SoundData *_data;
	/** \brief The audio channel that the sound is playing on
	*** This member is needed to perform specific queries on the playing channel
	*** \note This is actually quite a dangerous member to keep around because once a sound stops playing,
	*** the channel number is no longer valid and this member is not updated to reflect that. You should
	*** only rely on this member being correct <b>if</b> the sound is currently playing.
	**/
	int32 _channel;
	/** \brief The number of loops to play the sound for
	*** By default this member is set to zero, meaning it plays only once.
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
}; // class SoundDescriptor

} // namespace hoa_audio

#endif /* #ifndef __AUDIO_SOUND_HEADER__ */
