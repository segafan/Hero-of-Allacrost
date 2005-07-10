/* 
 * audio.cpp
 *	Audio management module for Hero of Allacrost
 *	(C) 2004 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */
 
/*
 *	This code heavily uses the SDL_mixer 1.2.5 extension library. The documentation can be found here: 
 *	 http://jcatki.no-ip.org/SDL_mixer/SDL_mixer.html. The GameAudio class provides an easy-to-use API 
 *	 for managing all music and sound used in the game. It is a singleton class.
 */


#include <iostream>
#include "audio.h"

using namespace std;
using namespace hoa_audio::local_audio;
using namespace hoa_utils;

namespace hoa_audio {

bool AUDIO_DEBUG = false;
SINGLETON_INITIALIZE(GameAudio);

// The constructor initializes variables and the audio systems.
GameAudio::GameAudio() {
	if (AUDIO_DEBUG) cout << "AUDIO: GameAudio constructor" << endl;
	current_track = -1; // No track playing since we haven't loaded any music
	music_id = 1;
	sound_id = 1;
	
	for (int i = 0; i < MAX_CACHED_MUSIC; i++) {
		music_cache[i].id = 0;
		music_cache[i].music = NULL;
		music_cache[i].time = 0;		 
	}
	for (int i = 0; i < MAX_CACHED_SOUNDS; i++) {
		sound_cache[i].id = 0;
		sound_cache[i].sound = NULL;
		sound_cache[i].time = 0;			 
	}
	
	// Notice that we still continue the game even if audio initialization fails.
		
	if(SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) { // Really bad!
		cerr << "AUDIO ERROR: Could not initalize SDL audio subsystem: " << SDL_GetError() << endl;
		return;
	}

	// Open 22.05KHz, signed 16bit, system byte order, stereo audio, using 1024 byte chunks
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1) {
		audio_on = false;
		cerr << "AUDIO ERROR: Could not initialize mixer audio: " << Mix_GetError() << endl;
	}
	else {
		audio_on = true;
		Mix_AllocateChannels(OPEN_CHANNELS);
	}	 
}



// The destructor halts all music, halts all sounds, frees every item in both caches, and then closes the audio
GameAudio::~GameAudio() {
	if (AUDIO_DEBUG) cout << "AUDIO: GameAudio destructor invoked." << endl;
	Mix_HaltMusic();
	Mix_HaltChannel(ALL_CHANNELS);	
	
	// Close all open music and sounds
	for (int i = 0; i < MAX_CACHED_MUSIC; i++) {
		if (music_cache[i].music != NULL)
		 FreeMusic(i);
	}
	for (int i = 0; i < MAX_CACHED_SOUNDS; i++) {
		if (sound_cache[i].sound != NULL)
		 FreeSound(i);
	}

	Mix_CloseAudio();
}



// Returns a free music cache index. If there are no free indeces, uses LRU replacement.
int GameAudio::AllocateMusicIndex() {
	Uint32 lru; // represents the time of the oldest item
	int index;	// represents the array index of the oldest item

	for (int i = 0; i < MAX_CACHED_MUSIC; i++) { // Traverse thru the music cache looking for free indeces
		if (music_cache[i].id == 0) // Found a free location, so return its index
			return i;
	}
		
	// No free music cache locations were found, so use LRU replacement
	if (current_track != 0) {
		lru = music_cache[0].time;
		index = 0;
	}
	else {
		lru = sound_cache[1].time;
		index = 1;		
	}
		
	for (int i = 1; i < MAX_CACHED_MUSIC; i++) { // Go thru the music cache trying to find older items
		if (music_cache[i].time < lru && i != current_track) { // Found a non-active item with an older time
			lru = music_cache[i].time; // Replace lru and index with the attributes of the older element
			index = i;
		}
	}
	FreeMusic(index); // Free the least recently used (LRU) element (= oldest element)
	return index;		 // Return the index of the newly freed array location
}



// Returns the cache index that stores the same id as the argument. Returns -1 if not found.
int GameAudio::FindMusicIndex(unsigned int mus_id) {
	if (mus_id == 0) // This music hasn't been loaded, so its clearly not in the cache
		return -1;

	for (int i = 0; i < MAX_CACHED_MUSIC; i++) { // Traverse thru the music cache
		if (music_cache[i].id == mus_id) // Found the item, return its index!
			return i;
	}
	return -1; // We couldn't find the item in the cache. Return a bad value
}



// Frees the cache location at 'index'
void GameAudio::FreeMusic(int index) {
	// Make sure that our index is within the array bounds
	if (index >= 0 && index < MAX_CACHED_MUSIC) {
		if (music_cache[index].music != NULL)	// Only free the memory pointed to by 'music' if it isn't NULL
			Mix_FreeMusic(music_cache[index].music);
		music_cache[index].id = 0;
		music_cache[index].music = NULL;
		music_cache[index].time = 0;
	}
}



// Returns a free sound cache index. If there are no free indeces, uses LRU replacement.
int GameAudio::AllocateSoundIndex() {
	Uint32 lru;		// represents the time of the oldest item
	int index = 0; // represents the array index of the oldest item

	for (int i = 0; i < MAX_CACHED_SOUNDS; i++) { // Traverse thru the music cache looking for free indeces
		if (sound_cache[i].sound == NULL) // Found a free location, so return its index
			return i;
	}
		
	// No free sound cache locations were found, so use LRU replacement
	lru = sound_cache[0].time;	// Set time equal to the zero'th element's time (remember index = 0)

	for (int i = 1; i < MAX_CACHED_SOUNDS; i++) { // Go thru the sound cache trying to find older itmes
		if (sound_cache[i].time < lru) { // Found an item with an older time, so replace 'lru' and 'index'
			lru = sound_cache[i].time; // Replace lru and index with the new least recently used element.
			index = i;
		}
	}
	FreeSound(index); // Free the least recently used (LRU) element (= oldest element)
	return index; // Return the index of the newly freed array location
}



// Returns the cache index that stores the same id as the argument. Returns -1 if not found.
int GameAudio::FindSoundIndex(unsigned int snd_id) {
	if (snd_id == 0) // The sound hasn't been loaded, so its clearly not in the cache
		return -1;

	for (int i = 0; i < MAX_CACHED_SOUNDS; i++) { // Traverse thru the music cache
		if (sound_cache[i].id == snd_id) // Found the item, return its index!
			return i;
	}
	return -1; // We couldn't find the item in the cache. Return a bad value
}


// Frees the sound_cache location at 'index'
void GameAudio::FreeSound(int index) {	
	// Make sure that our index is within the array bounds
	if (index >= 0 && index < MAX_CACHED_SOUNDS) {
		if (sound_cache[index].sound != NULL) // Only free the memory pointed to by 'sound' if it isn't NULL
			Mix_FreeChunk(sound_cache[index].sound);
		sound_cache[index].id = 0;
		sound_cache[index].sound = NULL;
		sound_cache[index].time = 0;
	}
}



// Pauses both sound and music
void GameAudio::PauseAudio() {
	if (audio_on == false) // If audio wasn't properly initialized, we do nothing here
		return;
		
	if (Mix_Paused(ALL_CHANNELS) == 0) // Sound channels are not paused, so pause them now
		Mix_Pause(ALL_CHANNELS);
	if (Mix_PausedMusic() == 0) // Music is not paused, so pause it now
		Mix_PauseMusic();
}



// Resumes (unpauses) both sound and music
void GameAudio::ResumeAudio() {
	if (audio_on == false) // If audio wasn't properly initialized, we do nothing here
		return;
		
	if (Mix_Paused(ALL_CHANNELS) != 0) // Sound channels are all paused, so resume them now
		Mix_Resume(ALL_CHANNELS);
	if (Mix_PausedMusic() != 0) // Music is paused, so resume it now
		Mix_ResumeMusic();
}



// Loads a new song into the music_cache. Exits program if an error occurs.
void GameAudio::LoadMusic(MusicDescriptor& md) {
	int location; // location is the array index in the music_cache that the song will be loaded to
	Mix_Music *new_mus; // new_mus is the pointer to the music data we are trying to load
	
	if (audio_on == false) // Do nothing if audio init went bad
		return;

	location = FindMusicIndex(md.id); // Check if the file is already in the cache
	if (location != -1) // File is already loaded. Return and be angry that you wasted your time
		return;
	
	string load_name = "mus/" + md.filename + ".ogg"; // Create full path to filename
	new_mus = Mix_LoadMUS(load_name.c_str()); // We need to convert the string to a C-type string
	if (new_mus == NULL) { // typo in filename arg or missing/corrupt file are the most likely errors to cause this
		cerr << "AUDIO ERROR: Could not load " << load_name << ". " << Mix_GetError() << endl;
		cerr << "* Likely causes of this error are a typo in the filename argument passed or a missing or corrupt" 
		     <<	"data file.\n* The game is exiting, please fix this error immediately!" << endl;
		exit(1);
	}
	
	location = AllocateMusicIndex();
	MusicItem new_song;							 // new_song is the new music_cache item we wish to create
	new_song.id = music_id;
	new_song.music = new_mus;
	new_song.time = SDL_GetTicks();	 // Set the last refereneced time equal to now				
	music_cache[location] = new_song; // Store the new_song into the music_cache

	md.id = music_id; // Setup the id in the MusicDescriptor passed
	
	music_id++; // This can generate an overflow after 4,294,967,295. Roughly 50 days if it occurs 1000 times/sec.
}



// Plays the music. loop can be: loop = AUDIO_LOOP_FOREVER (-1), loop = AUDIO_LOOP_ONCE (0) ...
void GameAudio::PlayMusic(MusicDescriptor& md, int fade_ms, int loop) {
	int location; // location is the music_cache index of the file we wish to play

	if (audio_on == false) // If audio wasn't initialized properly, how can we play anything? 
		return;
		
	if (current_track != -1) { // If already playing, return and be pissed at the programmer who called you
		if (md.id == music_cache[current_track].id)
			return;
	}
		
	location = FindMusicIndex(md.id); // Find where the file is located in the cache, if its even there
	if (location == -1) { // File is not in cache, so lets load it now
		LoadMusic(md);
		location = FindMusicIndex(md.id);
	}
		
	if (Mix_PlayingMusic()) { // Then we need to stop playing the current song
		if (fade_ms != 0)
			Mix_FadeOutMusic(fade_ms); // Fade the current track out
		else
			Mix_HaltMusic(); // Stop the music immediately with no fade
	}
	
	if (fade_ms != 0) 
		Mix_FadeInMusic(music_cache[location].music, loop, fade_ms); // Fade next track in
	else
		Mix_PlayMusic(music_cache[location].music, loop); // Forget this fading business, just play the damn song!
		
	// Update current track and last access time for the file
	current_track = location;
	music_cache[current_track].time = SDL_GetTicks();
}



// I don't think this will ever be used unless we need sudden dramatic silence ;) But it's here just in case.
void GameAudio::StopMusic(int fade_ms) {
	if (audio_on && Mix_PlayingMusic()) { // Don't stop the music unless it's playing
		if (fade_ms != 0)
			Mix_FadeOutMusic(fade_ms);
		Mix_HaltMusic();
		current_track = -1;
	}
	return;
}



// Frees the music based on the md.id value of the argument
void GameAudio::FreeMusic(MusicDescriptor& md) {
	int location = FindMusicIndex(md.id); // Find the index matching the filename
	if (location == -1) // It's not in the cache so it has already been evicted
		return;
	
	// Now free the element at 'location'
	if (music_cache[location].music != NULL)	// Only free the memory pointed to by 'sound' if it isn't NULL
		Mix_FreeMusic(music_cache[location].music);
	music_cache[location].id = 0;
	music_cache[location].music = NULL;
	music_cache[location].time = 0;
	md.id = 0;
	return;
}



// Changes the music volume. The value argument should be between 0 and 128
void GameAudio::SetMusicVolume(int value) {
	if (audio_on == false) // If audio wasn't properly initialized, we do nothing here
		return;
		
	if (value > MIX_MAX_VOLUME) {
		Mix_VolumeMusic(MIX_MAX_VOLUME); // Set music volume to maximum level
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: Tried to set music volume past max level: " << value << endl;
	}
	else if (value < 0) {
		Mix_VolumeMusic(0); // Turn music off (set volume to 0)
		if (AUDIO_DEBUG) cerr << "AUDIO WARNING: Tried to set music volume past min level: " << value << endl;
	}
	else { // Set the music volume to the new value
		Mix_VolumeMusic(value);
	}
}



// *Used for debugging purposes ONLY* Prints the contents of the music cache.
void GameAudio::PrintMusicCache() {
	cout << "AUDIO: Printing music cache" << endl;
	for (int i = 0; i < MAX_CACHED_MUSIC; i++) { // Loop thru the music cache
		if (music_cache[i].music == NULL) // There is no memory allocated for this index
			continue; // We aren't interested in cache locations that hold no data. Look at the next location

		cout << "*** music_cache[" << i << "] ***\n";
		cout << " id				 : " << music_cache[i].id << "\n";
		cout << " last access: " << music_cache[i].time << endl;
	}
}



// Loads a new sound into the sound_cache. Exits the program if a load error occurs.
void GameAudio::LoadSound(SoundDescriptor& sd) {
	int location;		 // location is the array index in the sound_cache where the sound will be stored
	Mix_Chunk *new_chunk; // new_chunk is a pointer to the sound data we are loading in
	
	if (audio_on == false) // Do nothing if audio init went bad
		return;

	location = FindSoundIndex(sd.id); // Check if the file is already loaded in the cache
	if (location != -1) // File is already loaded. Return and curse the idiot that called you!
		return;	 
				
	string load_name = "snd/" + sd.filename + ".wav"; // Create full path to sound filename
	new_chunk = Mix_LoadWAV(load_name.c_str()); // We need to convert the string to a C-type string
	if (new_chunk == NULL) {
		cerr << "AUDIO ERROR: Could not load " << load_name << ". " << Mix_GetError() << endl;
		cerr << " Likely causes of this error are a typo in the filename or a missing or corrupt"
		     << "data file.\n The game is exiting, please fix this error immediately!" << endl;
		exit(1);
	}
	
	location = AllocateSoundIndex();	// Find a location for the new sound in the cache
	SoundItem new_snd;							 // new_snd is the new item we want to add to the sound_cache
	new_snd.id = sound_id;
	new_snd.sound = new_chunk;	
	new_snd.time = SDL_GetTicks();	 // Set the last reference time equal to right now
	sound_cache[location] = new_snd; // Store the new_snd into the sound_cache
	
	sd.id = sound_id;
	
	sound_id++; // This can generate an overflow after 4,294,967,295. Roughly 50 days if it occurs 1000 times/sec.
}



// Plays a sound (duh). If the sound isn't in the cache, it will automatically be loaded. 
void GameAudio::PlaySound(SoundDescriptor& sd, int fade_ms, int loop) {
	int location;
	
	if (audio_on == false)	// Check for bad audio initialization or bad filename
		return;	
	
	location = FindSoundIndex(sd.id); // Look for the sound in the cache and return its index.
	if (location == -1) { // The sound isn't in the cache, so load it
		LoadSound(sd); // The program aborts if we can't load the file here
		location = FindSoundIndex(sd.id);
	}
	
	if (fade_ms == 0) {
		if (Mix_PlayChannel(ANY_OPEN_CHANNEL, sound_cache[location].sound, loop) == -1) // There was an error playing
			cerr << "AUDIO ERROR: Could not play snd/" << sd.filename << ".wav. " << Mix_GetError() << endl;
	}
	else {
		if (Mix_FadeInChannel(ANY_OPEN_CHANNEL, sound_cache[location].sound, loop, fade_ms) == -1) // Error
			cerr << "AUDIO ERROR: Could not play snd/" << sd.filename << ".wav. " << Mix_GetError() << endl;
	}
	
	sound_cache[location].time = SDL_GetTicks(); // Update last access time
}



// Quite simply, stops all sounds. A function I don't think will ever be used, but its here just in case.
void GameAudio::StopSound() {
	if (audio_on && (Mix_Playing(ALL_CHANNELS) != 0)) // Don't stop sound unless it's playing
		Mix_HaltChannel(ALL_CHANNELS);									// Halt all sound channels
	return;
}



// Frees the sound based on the sd->id value of the argument
void GameAudio::FreeSound(SoundDescriptor& sd) {
	int location = FindSoundIndex(sd.id); // Find the index matching the filename
	if (location == -1) // It's not in the cache so its already been evicted
		return;
	
	// Now free the element at 'location'
	if (sound_cache[location].sound != NULL) {// Only free the memory pointed to by 'sound' if it isn't NULL
			Mix_FreeChunk(sound_cache[location].sound);
		sound_cache[location].id = 0;
		sound_cache[location].sound = NULL;
		sound_cache[location].time = 0;
		sd.id = 0;
	}
}



// Changes the sound volume. The argument should be between 0 and 128
void GameAudio::SetSoundVolume(int value) {
	if (audio_on == false) // If audio wasn't properly initialized, we do nothing here
		return;
		
	if (value >= MIX_MAX_VOLUME) {
		Mix_Volume(ALL_CHANNELS, MIX_MAX_VOLUME); // Set sound volume to maximum level
	}
	else if (value <= 0) {
		Mix_Volume(ALL_CHANNELS, 0); // Turn sound off (set volume to 0)
	}
	else {
		Mix_Volume(ALL_CHANNELS, value); // Set the sound volume to the new value
	}
}



// *Used for debugging purposes ONLY* Prints the contents of the sound cache
void GameAudio::PrintSoundCache() {
	cout << "AUDIO: Printing sound cache" << endl;
	for (int i = 0; i < MAX_CACHED_SOUNDS; i++) { // Loop thru the sound cache
		if (sound_cache[i].sound == NULL) // There is no memory allocated for this index
			continue; // We aren't interested in cache locations that hold no data. Look at the next location
			
		cout << "*** sound_cache[" << i << "] ***\n";
		cout << " id				 : " << sound_cache[i].id << "\n";
		cout << " last access: " << sound_cache[i].time << endl;
	}	
}

} // namespace hoa_audio
