///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    audio.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 11th, 2005
 * \brief   Header file for audio engine interface.
 *
 * This code provides an easy-to-use API for managing all music and sounds used
 * in the game.
 *
 * \note This code uses the SDL_mixer 1.2.5 extension library. The documentation
 *       for it may be found here: http://jcatki.no-ip.org/SDL_mixer/SDL_mixer.html
 *
 * \note There is more functionality in SDL_mixer than what is included here, like
 *       stereo panning, distance attenuation, and some other effects. These
 *       features will be available in future releases of this code.
 *****************************************************************************/

#ifndef __AUDIO_HEADER__
#define __AUDIO_HEADER__

#include "utils.h"
#include <string>
#include "SDL.h"
#include <SDL/SDL_mixer.h>

//! All calls to the audio engine are wrapped in this namespace.
namespace hoa_audio {

//! Determines whether the code in the hoa_audio namespace should print debug statements or not.
extern bool AUDIO_DEBUG;

//! Pass this as the \c loop argument in an audio function and the music or sound will loop indefinitely.
const int AUDIO_LOOP_FOREVER = -1;
//! Pass this as the \c loop argument in an audio function and the music or sound will play only once.
const int AUDIO_LOOP_ONCE = 0;
//! Pass this as the \c fade_ms argument in an audio function for no fading in or out of the audio.
const int AUDIO_NO_FADE = 0;
//! The default time to fade in/out music (500ms). Pass this as the \c fade_ms argument in an audio function.
const int AUDIO_DEFAULT_FADE = 500;

//! An internal namespace to be used only within the audio engine. Don't use this namespace anywhere else!
namespace private_audio {

//! The maximum number of songs that can be loaded at any given time.
const int MAX_CACHED_MUSIC = 5;
//! The maximum number of sounds that can be loaded at any given time.
const int MAX_CACHED_SOUNDS = 50;
//! The number of sound channels to open for audio mixing (music automatically has its own channel)
const int OPEN_CHANNELS = 16;
//! Used in function calls for pausing audio, halting audio, or changing the volume
const int ALL_CHANNELS = -1;
//! When playing a sound, passing this argument will play it on any open channel
const int ANY_OPEN_CHANNEL = -1;

/*!****************************************************************************
 *  \brief Used by the GameAudio class to internally represent a sound data item.
 *
 *  You don't need to worry about this class and you should never create any instance of it.
 *****************************************************************************/
class SoundItem {
public:
	//! A unique ID number assigned for the sound item.
	unsigned int id;
	//! A pointer to the sound data loaded in memory.
	Mix_Chunk *sound;
	//! The last time that the sound was referenced, in milliseconds.
	Uint32 time;
};

/*!****************************************************************************
 *  \brief Used by the GameAudio class to internally represent a music data item.
 *
 *  You don't need to worry about this class and you should never create any instance of it.
 *****************************************************************************/
class MusicItem {
public:
	//! A unique ID number assigned for the music item.
	unsigned int id;
	//! A pointer to the sound data loaded in memory.
	Mix_Music *music;
	//! The last time that the sound was referenced, in milliseconds.
	Uint32 time;
};

} // namespace local_audio

/*!****************************************************************************
 *  \brief A container class for referring to sound objects.
 *
 *  \note If a sound is not referenced for a long time the audio engine may choose to evict
 *  it from the sound cache. However, if you try to play a sound not loaded into memory the
 *  audio engine will automatically load it and play it.
 *
 *  \note When a sound is loaded, the interal \c SoundDescriptor#id argument is the
 *  same value as the \c SoundItem#id argument in the SoundItem class used internally
 *  by the audio engine.
 *****************************************************************************/
class SoundDescriptor {
public:
	SoundDescriptor() { id = 0; }
	//! \param fn The string to set the filename member to.
	SoundDescriptor(std::string fn) { filename = fn; id = 0; }
	//! The filename for the sound, without the prefixed \c snd/ directory or \c .wav extension.
	std::string filename;
private:
	/*! \brief A unique id number for the sound.
	 *
	 *  Zero indicates it is not loaded in memory, however, the converse is not always true.
	 */
	unsigned int id;
	friend class GameAudio;
};

/*!****************************************************************************
 *  \brief A container class for referring to music objects.
 *
 *  \note If the music is not referenced for a long time the audio engine may choose to evict
 *  it from the music cache. However, if you try to play music not loaded into memory the
 *  audio engine will automatically load it and play it.
 *
 *  \note When music is loaded, the interal \c MusicDescriptor#id argument is the
 *  same value as the \c MusicItem#id argument in the MusicItem class used internally
 *  by the audio engine.
 *****************************************************************************/
class MusicDescriptor {
public:
	MusicDescriptor() { id = 0; }
	MusicDescriptor(std::string fn) { filename = fn; id = 0; }
	//! The filename for the music, without the prefixed \c mus/ directory or \c .ogg extension.
	std::string filename;
private:
	/*! \brief A unique id number for the sound.
	 *
	 *  Zero indicates it is not loaded in memory, however, the converse is not always true.
	 */
	unsigned int id;
	friend class GameAudio;
};

/*!****************************************************************************
 *  \brief Manages all the game audio and serves as the API to the audio engine.
 *
 *  The primary way you interface with this class is through the use of SoundDescriptor
 *  and MusicDescriptor class objects. You can think of them as pointers to audio data,
 *  although the details are hidden from the user of this API so its virtually impossible
 *  to cause a segmentation fault. However, be careful with setting the filenames of
 *  SoundDescriptor and MusicDescriptor because if you try to load in audio that has a
 *  misspelt filename (or refers to a missing file) this class will print out a strict
 *  error message about the issue and promptly exit the game, forcing you to fix the
 *  problem.
 *
 *  \note 1) This class is a singleton.
 *
 *  \note 2) You'll notice that there are two definitions of the FreeMusic and
 *  FreeSound functions. The private version is used by the audio code. You
 *  should use the public version (obviously).
 *
 *  \note 3) Technically LoadMusic/Sound and FreeMusic/Sound can never be called and
 *  you can still play music normally without fear of segmentation faults. *HOWEVER*,
 *  it is good practice to call the Load*() functions when you have new audio you wish
 *  to play and the Free*() functions when you are done.
 *
 *  \note 4) In a normal class, music_id and sound_id would need to be static ints,
 *  but since this is a Singleton template class, we don't need to declare them static.
 *
 *  \note 5) For those interested, this class uses a LRU replacement algorithm when the
 *  caches are full and a new audio file needs to be loaded. The cache sizes are defined
 *  by the const ints at the top of this file.
 *
 *  \note 6) Recommended fade time for playing music and sound is 500ms. Use the
 *  AUDIO_DEFAULT_FADE constant
 *****************************************************************************/
class GameAudio {
private:
	SINGLETON_DECLARE(GameAudio); // Semi-colon not needed here, will it create a warning?
	//! A boolean value that disables all audio functions when set to false.
	bool audio_on;
	//! Indicates the array index of the song currently playing in music_cache.
	int current_track;
	//! Retains the next id value to give a new music item.
	int music_id;
	//! Retains the next id value to give a new sound item.
	int sound_id;
	//! An array storing up to MAX_CACHED_MUSIC MusicItem objects loaded into memory.
	private_audio::MusicItem music_cache[private_audio::MAX_CACHED_MUSIC];
	//! An array storing up to MAX_CACHED_SOUNDS SoundItem objects loaded into memory.
	private_audio::SoundItem sound_cache[private_audio::MAX_CACHED_SOUNDS];

	/*!
	 *  \brief  Finds and allocates the first free index found in the music_cache.
	 *
	 *  If no free indeces exist, uses LRU (least recently used) replacement to evict
	 *  the oldest item in the cache.
	 *
	 *  \return The music_cache index allocated for the new music data.
	 */
	int AllocateMusicIndex();
	/*!
	 *  \brief Searches music_cache for an id that matches the function parameter.
	 *  \param mus_id The id number to search the cache for.
	 *  \return The cache index of the MusicItem with an id matching mus_id. Returns -1 if not found.
	 */
	int FindMusicIndex(unsigned int mus_id);
	/*!
	 *  \brief Frees a MusicItem in the music_cache.
	 *  \param index The index in the music_cache to free.
	 */
	void FreeMusic(int index);

	/*!
	 *  \brief  Finds and allocates the first free index found in the sound_cache.
	 *
	 *  If no free indeces exist, uses LRU (least recently used) replacement to evict
	 *  the oldest item in the cache.
	 *
	 *  \return The sound_cache index allocated for the new sound data.
	 */
	int AllocateSoundIndex();
	/*!
	 *  \brief Searches sound_cache for an id that matches the function parameter.
	 *  \param snd_id The id number to search the cache for.
	 *  \return The cache index of the SoundItem with an id matching snd_id. Returns -1 if not found.
	 */
	int FindSoundIndex(unsigned int snd_id);
	/*!
	 *  \brief Frees a SoundItem in the sound_cache.
	 *  \param index The index in the sound_cache to free.
	 */
	void FreeSound(int index);

public:
	SINGLETON_METHODS(GameAudio);
	/*!
	 *  \brief Pauses both music and sound audio until ResumeAudio() is called.
	 */
	void PauseAudio();
	/*!
	 *  \brief Un-pauses both music and sound audio.
	 */
	void ResumeAudio();

	/*!
	 *  \brief Loads a music audio file and allocates memory for its content.
	 *
	 *  The filename member of md should be set prior to calling this function.
	 *  This function modifies the id member of md, unless it is determined that
	 *  the music is already loaded in music_cache. The function will force the
	 *  game to exit if it was unsuccessful.
	 *
	 *  \param &md A reference to the MusicDescriptor to load.
	 */
	void LoadMusic(MusicDescriptor& md);
	/*!
	 *  \brief Plays the piece of music specified by the argument.
	 *
	 *  If the audio engine can not find the music referrenced by md in the music_cache,
	 *  it will be loaded in and then played. Refer to the hoa_audio namespace constants
	 *  for common values to pass into the fade_ms and loop parameters.
	 *
	 *  \param &md     The MusicDescriptor object to play.
	 *  \param fade_ms The amount of time to fade out the current music, and then fade in the new music by.
	 *  \param loop    Specifies how many times the music should be looped.
	 */
	void PlayMusic(MusicDescriptor& md, int fade_ms, int loop);
	/*!
	 *  \brief Stops the currently playing music.
	 *  \param fade_ms The amount to fade out the playing music by, in milliseconds.
	 */
	void StopMusic(int fade_ms);
	/*!
	 *  \brief Frees an item from the music_cache specified by the argument.
	 *
	 *  If the item is not found in the cache, the function will return without causing harm.
	 *
	 *  \param &md A reference to the music piece to evict from the music_cache.
	 */
	void FreeMusic(MusicDescriptor& md);
	/*!
	 *  \brief Sets the volume of the music. The valid range is 0 to 128 inclusive.
	 *
	 *  If a value is passed to this function that exceeds the acceptable bounds of the volume level,
	 *  the function will set the volume to the closest acceptable value (either 0 or 128).
	 *
	 *  \param value The numerical value to set the music volume to.
	 */
	void SetMusicVolume(int value);
	/*!
	 *  \brief Returns the current volume level of the music (0 to 128 inclusive).
	 *  \return The volume level of the music.
	 */
	int GetMusicVolume();
	/*!
	 *  \brief Prints details about what is currently stored in music_cache to standard output.
	 *  \note This function is for debugging purposes \b only! You normally should never call it.
	 */
	void PrintMusicCache();

/*!
	 *  \brief Loads a sound audio file and allocates memory for its content.
	 *
	 *  The filename member of sd should be set prior to calling this function.
	 *  This function modifies the id member of sd, unless it is determined that
	 *  the sound is already loaded in sound_cache. The function will force the
	 *  game to exit if it was unsuccessful.
	 *
	 *  \param &sd A reference to the SoundDescriptor to load.
	 */
	void LoadSound(SoundDescriptor& sd);
	/*!
	 *  \brief Plays the sound piece specified by the argument.
	 *
	 *  If the audio engine can not find the sound referrenced by sd in the music_cache,
	 *  it will be loaded in and then played. Refer to the hoa_audio namespace constants
	 *  for common values to pass into the fade_ms and loop parameters.
	 *
	 *  \param &md     The SoundDescriptor object to play.
	 *  \param fade_ms The amount of time to fade in the new sound by.
	 *  \param loop    Specifies how many times the sound should be looped.
	 */
	void PlaySound(SoundDescriptor& sd, int fade_ms, int loop);
	/*!
	 *  \brief Stops all currently playing sounds.
	 */
	void StopSound();
	/*!
	 *  \brief Frees an item from the sound_cache specified by the argument.
	 *
	 *  If the item is not found in the cache, the function will return without causing harm.
	 *
	 *  \param &sd A reference to the sound piece to evict from the sound_cache.
	 */
	void FreeSound(SoundDescriptor& sd);
	/*!
	 *  \brief Sets the sound volume. The valid range is 0 to 128 inclusive.
	 *
	 *  If a value is passed to this function that exceeds the acceptable bounds of the volume level,
	 *  the function will set the volume to the closest acceptable value (either 0 or 128).
	 *
	 *  \param value The numerical value to set the sound volume to.
	 */
	void SetSoundVolume(int value);
	/*!
	 *  \brief Returns the current volume level of the sound (0 to 128 inclusive).
	 *  \return The volume level of the sound.
	 */
	int GetSoundVolume();
	/*!
	 *  \brief Prints details about what is currently stored in sound_cache to standard output.
	 *  \note This function is for debugging purposes \b only! You normally should never call it.
	 */
	void PrintSoundCache();
}; // class GameAudio

} // namespace hoa_audio

#endif
