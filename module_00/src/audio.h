/* 
 * audio.h
 *	Hero of Allacrost header file for audio interface
 *	(C) 2004 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */
 
/*
 *	This code uses the SDL_mixer 1.2.5 extension library. The documentation (which is pretty well 
 *	 written) can be found here: http://jcatki.no-ip.org/SDL_mixer/SDL_mixer.html. 
 *
 *	The GameAudio class provides an easy-to-use API for managing all music and sound used in the game. 
 *	 There should only be one instance of this class, globally defined in hoa_loader.h. Don't create 
 *	 another object for this class unless you want to invite disaster.
 */
 
/* Note from author: There is more functionality to SDL_mixer than what I have included here, like stereo
 *	panning, distance attenuation, and some other effects. If you have a particular piece of audio you wish
 *	to play that this code doesn't cover, e-mail me and I will gladly add it: roots@allacrost.org
 */

#ifndef __AUDIO_HEADER__
#define __AUDIO_HEADER__ 
 
#include <string>
#include "SDL.h"
#include <SDL/SDL_mixer.h>
#include "utils.h"


namespace hoa_audio {

class GameAudio;

const bool AUDIO_DEBUG = true;

// AUDIO_LOOP_FOREVER = Pass this into a loop argument and the music or sound will loop indefinitely
const int AUDIO_LOOP_FOREVER = -1;
// AUDIO_LOOP_ONCE = Pass this into a loop argument and the music or sound will play only once
const int AUDIO_LOOP_ONCE = 0;
// AUDIO_NO_FADE = Pass this as any fade_ms argument
const int AUDIO_NO_FADE = 0;
// AUDIO_DEFAULT_FADE = The default time to fade in/out music (500ms)
const int AUDIO_DEFAULT_FADE = 500;


namespace local_audio {

// MAX_CACHED_MUSIC = the maximum number of songs that can be loaded at any given time
const int MAX_CACHED_MUSIC = 5;
// MAX_CACHED_SOUNDS = the maximum number of sounds that can be loaded at any given time
const int MAX_CACHED_SOUNDS = 50;
// OPEN_CHANNELS = the number of sound channels to open for audio mixing (music automatically has its own channel)
const int OPEN_CHANNELS = 16;
// ALL_CHANNELS = used in function calls for pausing audio, halting audio, or changing the volume
const int ALL_CHANNELS = -1;
// ANY_OPEN_CHANNEL = when playing a sound, passing this argument will play it on any open channel
const int ANY_OPEN_CHANNEL = -1;



/******************************************************************************
 * SoundItem, MusicItem structs - Used by the audio code. You don't need to worry
 *	about these structs and you should never create any instances of them.
 *****************************************************************************/
typedef struct SoundItem {
	unsigned int id;	// The unique ID for the sound
	Mix_Chunk *sound; // A pointer to the sound loaded in memory
	Uint32 time;			// The last time that the sound was referenced, in milliseconds (used for LRU calculation)
} SoundItem;

typedef struct MusicItem {
	unsigned int id;	// The unique ID for the song
	Mix_Music *music; // A pointer to the music loaded in memory
	Uint32 time;			// The last time that the song was referenced, in milliseconds (used for LRU calculation)
} MusicItem;

} // namespace local_audio

/******************************************************************************
 * SoundDescriptor, MusicDescritor structs - A container for associating filenames
 *	with loaded audio via a unique ID. When you create an instance of this struct,
 *	initialize id to zero. After that, >>>NEVER<<< modify the id value, otherwise
 *	you could cause the wrong audio file to play.
 *****************************************************************************/
class SoundDescriptor {
	private:  unsigned int id;		// Unique sound ID. Zero indicates it is not loaded in memory
	public: std::string filename; // The filename for the sound (without 'snd/' or '.wav')
	friend class GameAudio;
};

class MusicDescriptor {
	private: unsigned int id;			// Unique music ID. Zero indicates it is not loaded in memory
	public: std::string filename; // The filename for the sound (without 'music/' or '.ogg')
	friend class GameAudio;
};






/******************************************************************************
 * GameAudio class - Manages all the game audio. 
 *	>>>This class is a singleton<<<
 *
 *	members: audio_on	 : a boolean value that disables all audio functions when set to false
 *				 current_track: indicates the array index of the song currently playing in music_cache
 *					 music_id	 : retains next id value to give a new music item
 *					 sound_id	 : retains next id value to give a new sound item
 *					 sound_cache: an array storing up to MAX_CACHED_SOUNDS SoundItem objects loaded into memory
 *					 music_cache: an array storing up to MAX_CACHED_MUSIC MusicItem objects loaded into memory
 *
 *	functions: GameAudio(): 
 *							 Initializes SDL Audio subsystem, SDL mixer system, and class member values.
 *						 ~GameAudio():
 *							 Frees up all music and sound in the caches and shutsdown the SDL mixer system.
 *						 int AllocateMusicIndex():
 *							 Returns the first free index in the music_cache. If none exist, uses LRU replacement.
 *						 int FindMusicIndex(unsigned int mus_id):
 *							 Searches music_cache for an id that matches mus_id. Returns -1 if not found.
 *						 void FreeMusic(int index):
 *							 Frees the item at music_cache[index].
 *						 int AllocateSoundIndex():
 *							 Returns the first free index in the sound_cache. If none exist, uses LRU replacement.
 *						 int FindSoundIndex(unsigned int snd_id):
 *							 Searches sound_cache for an id that matches snd_id. Returns -1 if not found.
 *						 void FreeSound(int index):
 *							 Frees the item at sound_cache[index].
 *
 *						 void PauseAudio():
 *							 Pauses both music and sound audio until ResumeAudio() is called.
 *						 void ResumeAudio():
 *							 Unpauses both music and sound audio.
 *
 *						 void LoadMusic(MusicDescriptor& md): 
 *							 Loads the md.filename audio file. Sets up md.id. Exits the game if unsuccessful.
 *						 void PlayMusic(MusicDescriptor& md, int fade_ms, int loop): 
 *							 Plays the file referenced to by md. If it is not in the music_cache, it loads it in.
 *							 fade_ms is the amount of milliseconds for music to fade out and back in. 0 means no fade.
 *						 void StopMusic(bool fade):
 *							 Quite simply, it stops the music. The music fades out if the argument is true.
 *						 void FreeMusic(MusicDescriptor& md): 
 *							 Frees a MusicItem object from the music_cache whose id matches md.id.
 *						 void SetMusicVolume(int val):
 *							 Sets the music volume to val. Valid range is 0 to 128 inclusive.
 *						 int GetMusicVolume():
 *							 Returns the value of music_vol.
 *						 void PrintMusicCache():
 *							 This function is for debugging purposes ONLY. It prints the contents of the music_cache.
 *
 *						 void LoadSound(SoundDescriptor& sd): 
 *							 Loads the sd.filename audio file. Sets up sd.id. Exits the game if unsuccessful.
 *						 void PlaySound(SoundDescriptor& sd, int fade_ms, int loop): 
 *							 Plays the file referenced to by sd. If it is not in the cache, it loads it in.
 *						 void StopSound():
 *							 Stops all sounds currently playing on all channels.
 *						 void FreeSound(SoundDescriptor& sd): 
 *							 Frees a SoundItem object from the sound_cache whose id matches sd.id.
 *						 void SetSoundVolume(int val):
 *							 Sets the sound volume to val. Valid range is 0 to 128 inclusive.
 *						 int GetSoundVolume(int val):
 *							 Returns the value of sound_vol.
 *						 void PrintSoundCache(): 
 *							 This function is for debugging purposes ONLY. It prints the contents of the sound_cache.
 *
 *	notes: 1) You'll notice that there are two definitions of the FreeMusic and FreeSound functions. 
 *						The private version is used by the audio code. You should use the public version (obviously).
 *
 *				 2) Technically LoadMusic/Sound and FreeMusic/Sound can never be called and you can still play music
 *						normally without fear of segmentation faults. *HOWEVER*, it is good practice to call the Load
 *						functions when you have new audio you want to play and the Free functions when you are done.
 *
 *				 3) In a normal class, music_id and sound_id would need to be static ints, but since this is a
 *						Singleton template class, we don't need to declare them static.
 *
 *				 4) For those interested, this class uses a LRU replacement algorithm when the caches are full and
 *						a new audio file needs to be loaded. The cache sizes are defined by the const ints at the top
 *						of this file. 
 *
 *				 5) Recommended fade time for playing music and sound is 500ms.
 *****************************************************************************/
class GameAudio {
	bool audio_on;
	int current_track;
	int music_id;
	int sound_id;
	local_audio::MusicItem music_cache[local_audio::MAX_CACHED_MUSIC];
	local_audio::SoundItem sound_cache[local_audio::MAX_CACHED_SOUNDS];

	GameAudio();
	GameAudio( const GameAudio& ga ) {}
	GameAudio& operator=( const GameAudio& ga ) {}
	SINGLETON2(GameAudio);	
	
	int AllocateMusicIndex();
	int FindMusicIndex(unsigned int mus_id);
	void FreeMusic(int index);
	
	int AllocateSoundIndex();
	int FindSoundIndex(unsigned int snd_id);
	void FreeSound(int index);
public:		
	~GameAudio();
	void PauseAudio();
	void ResumeAudio();
	
	void LoadMusic(MusicDescriptor& md);
	void PlayMusic(MusicDescriptor& md, int fade_ms, int loop);
	void StopMusic(int fade_ms);
	void FreeMusic(MusicDescriptor& md);
	void SetMusicVolume(int value);
	int GetMusicVolume();
	void PrintMusicCache();

	void LoadSound(SoundDescriptor& sd);
	void PlaySound(SoundDescriptor& sd, int fade_ms, int loop);
	void StopSound();
	void FreeSound(SoundDescriptor& sd);
	void SetSoundVolume(int value);
	int GetSoundVolume();
	void PrintSoundCache();

};

} // namespace hoa_audio

#endif
