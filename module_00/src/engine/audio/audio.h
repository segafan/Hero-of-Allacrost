///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file    audio.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for audio engine interface.
***
*** This code provides an easy-to-use API for managing all music and sounds used
*** in the game.
***
*** \note This code uses the SDL_mixer audio library.
***
*** \note The audio engine code is temporary and, in fact, not implemented
*** very well. For example, calling the PauseMusic() function on a MusicDescriptor
*** class object will pause the music, even if the music that is playing is not the
*** music referenced by that class object (this is because SDL_mixer has only one
*** playback channel allocated for music).
*** 
*** These issues will be addressed at a later point in time, once our team
*** makes a decision on the direction to take with our audio engine (which 
*** may very well not use SDL_mixer at all). Until then, proceed with caution 
*** when using this audio engine/
*** **************************************************************************/

#ifndef __AUDIO_HEADER__
#define __AUDIO_HEADER__

#include <SDL/SDL_mixer.h>

#include "utils.h"
#include "defs.h"
#include "audio_sound.h"
#include "audio_music.h"

namespace hoa_audio {

//! The singleton pointer responsible for all audio operations.
extern GameAudio *AudioManager;
//! Determines whether the code in the hoa_audio namespace should print debug statements or not.
extern bool AUDIO_DEBUG;

/** \name Audio State Constants
*** \brief Used to determine what state a sound or music piece is in.
**/
//@{
const uint8 AUDIO_STATE_UNLOADED     = 0x01;
const uint8 AUDIO_STATE_STOPPED      = 0x02;
const uint8 AUDIO_STATE_PAUSED       = 0x04;
const uint8 AUDIO_STATE_PLAYING      = 0x08;
const uint8 AUDIO_STATE_FADING_IN    = 0x10;
const uint8 AUDIO_STATE_FADING_OUT   = 0x20;
//@}

/** \name Audio Playback Property Constants
*** \brief Used to alter the manner in which sounds or music is played.
**/
//@{
//! Pass this as the <b>loop</b> argument in an audio function and the music or sound will loop indefinitely.
const int32 AUDIO_LOOP_FOREVER = -1;
//! Pass this as the <b>loop</b> argument in an audio function and the music or sound will play only once.
const int32 AUDIO_LOOP_ONCE = 0;
//! Pass this as the <b>fade_time</b> argument in an audio function for no fading in or out of the audio.
const uint32 AUDIO_NO_FADE = 0;
//! The standard amount of time to fade in/out music (500ms). Pass this as the <b>fade_time</b> argument in an audio function.
const uint32 AUDIO_STANDARD_FADE = 500;
//@}

/** \name Audio Error Constants
*** \brief Used to determine what, if any, errors occured during audio playback
**/
//@{
const uint32 AUDIO_ERROR_NONE          = 0x00000000;
const uint32 AUDIO_ERROR_NO_DATA       = 0x00000001;
const uint32 AUDIO_ERROR_PLAY_FAILURE  = 0x00000002;
//@}

namespace private_audio {

//! The number of sound channels to open for audio mixing (music automatically has its own channel)
const uint32 SOUND_CHANNELS = 16;
//! Used in function calls for pausing audio, halting audio, or changing the volume
const int32 ALL_CHANNELS = -1;
//! When playing a sound, passing this argument will play it on any open channel
const int32 ANY_CHANNEL = -1;
//! The size (in number of bytes) of audio buffers
const int32 BUFFER_SIZE = 1024;

} // namespace private_audio

/*!****************************************************************************
 * \brief A singleton class for managing and interfacing with audio data.
 *
 * This class manages all audio data allocation and manipulation. The OpenAL sources
 * are wrapped inside this class and OpenAL buffers (which are represented by the
 * SoundDescriptor and MusicObject classes) grab these sources as they need them. The
 * buffers are stored in map structures so that audio data is not loaded when it already
 * exists.
 *
 * \note 1) Operations that load audio data should be done during parts of the game
 * when game modes are being created and destroyed. In other words, ideally you should
 * load data into SoundSource and MusicSource objects when a new game mode class object is created,
 * instead of creating them only immediately before they are needed.
 *
 * \note 2) This audio engine uses smart memory management so that loaded audio
 * data is not re-loaded if the user requests a load operation on the same
 * data. Audio data is only freed once there are no more references to the
 * data.
 *****************************************************************************/
class GameAudio {
	friend class private_audio::SoundData;
	friend class SoundDescriptor;
	friend class private_audio::MusicData;
	friend class MusicDescriptor;

public:
	SINGLETON_METHODS(GameAudio);

	/*! \brief Returns a set of error codes and also clears the error code to a no-error state.
	 *
	 *  This is the standard CheckErrors() function as defined in the Allacrost code standard.
	 *  The error code constants are listed at the top of this file in the Audio Error Codes
	 *  group.
	 */
	uint32 CheckErrors()
		{ uint32 return_code; return_code = _audio_errors; _audio_errors = AUDIO_ERROR_NONE; return return_code; }

	/*! \name Volume Member Access Functions
	 *  \brief Used for reading and modifying the volume of music/sound in the game.
	 *
	 *  These volume changes have a global effect (they modify the volume of all music/sound sources).
	 *  There is currently no support to change the volume levels of individual sound or music
	 *  sources. However, one can achieve the same effect by modifying the source properties and (in the
	 *  case of single-channel audio) manipulating distance attenuation.
	 *
	 *  \note In OpenAL, the gain property of sources is essentially the volume.
	 *
	 *  \note In addition to these functions, you can also change the gain (volume) of the listener, which
	 *  will effect the volume level of \c all audio heard in the game.
	 */
	//@{
	uint8 GetMusicVolume()
		{ return _music_volume; }
	uint8 GetSoundVolume()
		{ return _sound_volume; }
	void SetMusicVolume(uint8 vol);
	void SetSoundVolume(uint8 vol);
	//@}

	/*! \name Global Audio Manipulation Functions
	 *  \brief Performs specified operation on all active sounds and music.
	 *
	 *  These functions will only effect audio data that is in the state(s) specified below:
	 *  PauseAudio()    <==>   playing state
	 *  ResumeAudio()   <==>   paused state
	 *  StopAudio()     <==>   playing state
	 *  RewindAudio()   <==>   playing state, paused state
	 */
	//@{
	void PauseAudio();
	void ResumeAudio();
	void StopAudio();
	void RewindAudio();
	//@}

	/** \name Global Sound Manipulation Functions
	*** \brief Performs specified operation on all active sounds.
	**/
	//@{
	void PauseAllSounds();
	void ResumeAllSounds();
	void StopAllSounds();
// 	void RewindAllSounds();
	//@}

	/** \name Global Music Manipulation Functions
	*** \brief Performs specified operation on all active music.
	***
	*** Since there is only one music source, these functions only affect that source. They are equivalent
	*** to calling the {Pause/Resume/Stop/Rewind}Music functions on the MusicDescriptor which currently
	*** has posession of the source.
	**/
	//@{
	void PauseAllMusic();
	void ResumeAllMusic();
	void StopAllMusic();
	void RewindAllMusic();
	//@}

	//! Prints information related to the system's audio capabilities as reported by SDL_mixer.
	void DEBUG_PrintInfo();

private:
	SINGLETON_DECLARE(GameAudio)

	//! The volume level for music playback. Valid range is between 0 and 128.
	uint8 _music_volume;
	//! The volume level for sound playback. Valid range is between 0 and 128.
	uint8 _sound_volume;
	//! Retains all the errors that have occured on audio-related function calls, except for loading errors.
	uint32 _audio_errors;

	/** \name Audio Data Containers
	*** Sound (WAV) and music (OGG) data are stored in these container classes and referenced by the SoundDescriptor
	*** and MusicDescriptor classes. They are stored in this manner to allow for optimized memory usage (in other
	*** words, only one instance of .wav and .ogg data can be loaded in the application at any point in time). The 
	*** sound or music filename string serves as the map key for determining if the data is loaded or not. The key
	*** is a reference to the filename string (which is stored in the SoundData or MusicData class itself).
	**/
	//@{
	std::map<std::string, private_audio::MusicData*> _music_data;
	std::map<std::string, private_audio::SoundData*> _sound_data;
	//@}

	/** \name Audio Data Retrieval Functions
	*** \brief Creates and loads new audio data if the data is not already loaded.
	*** \param filename The filename of the file data to search for (not including the pathname or file extension).
	*** \return A pointer to the class object holding the new data. NULL may also be returned, indicating an error.
	***
	*** These functions are critical to ensuring efficient memory usage in the audio engine (in other words:
	*** making sure no more than one copy of audio data is loaded into the engine at any given time). When these
	*** functions are called, first the map of audio data objects is searched to see if the data is already found
	*** in there. If it is, a pointer to that object is returned. Otherwise, the function will attempt to create
	*** a new object and store that into the appropriate data object map. If that fails for some reason, the function
	*** will return NULL.
	**/
	//@{
	private_audio::SoundData* _AcquireSoundData(std::string filename);
	private_audio::MusicData* _AcquireMusicData(std::string filename);
	//@}

}; // class GameAudio

} // namespace hoa_audio

#endif /* #ifndef __AUDIO_HEADER__ */
