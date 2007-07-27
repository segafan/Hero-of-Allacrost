///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    audio.h
 * \author  Tyler Olsen - Mois�s Ferrer - Aaron Smith, roots@allacrost.org - byaku@allacrost.org - etherstar@allacrost.org
 * \brief   Header file for audio engine interface.
 *
 * This code provides an easy-to-use API for managing all music and sounds used
 * in the game.
 *
 * \note This code uses the OpenAL audio library. See http://www.openal.com/
 *****************************************************************************/


#ifndef __AUDIO_HEADER__
#define __AUDIO_HEADER__


#ifdef __MACH__
	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>
#else
	#include "al.h"
	#include "alc.h"
#endif

#include "audio_descriptor.h"
#include "audio_fx.h"
#include "utils.h"
#include <vector>
#include <string>
#include <map>


namespace hoa_audio
{

//! The singleton pointer responsible for all audio operations.
extern GameAudio* AudioManager;
//! Determines whether the code in the hoa_audio namespace should print debug statements or not.
extern bool AUDIO_DEBUG;

namespace private_audio
{

bool ALError ();
bool ALCError ();
const std::string GetALErrorString (ALenum error_code);
const std::string GetALCErrorString (ALenum error_code);
void PrintALError (ALenum error_code);
void PrintALCError (ALenum error_code);

} // End of namespace private_audio

//! \brief Singleton class for managing the audio engine.
/*!
	This class is provided as a singleton so it is conveniently accesible where needed. Here 
	there is internal control of the audio device and available sources. Interfaces for the 2 
	main groups (sound and music) are available. Also the listener funtionality is provided 
	from here.
*/
class GameAudio : public hoa_utils::Singleton<GameAudio>
{
friend class hoa_utils::Singleton<GameAudio>;
// friend class private_audio::SoundData;
// friend class private_audio::MusicData;
//! \brief Access granted for retrieving sources easily.
friend class AudioDescriptor;
//! \brief Access granted for registering and deregistering sounds.
friend class SoundDescriptor;
//! \brief Access granted for registering and deregistering music.
friend class MusicDescriptor;
//! \brief Access granted for easy access to the FxManager.
friend class Effects;

private:
	GameAudio();
	GameAudio(const GameAudio &game_audio);

	float _sound_volume;		//!< \brief Volume of the sound.
	float _music_volume;		//!< \brief Volume of the music.
	ALCdevice* _device;			//!< \brief Pointer to an OpenAL device.
	ALCcontext* _context;		//!< \brief Pointer to an OpenAL context.
	std::vector <private_audio::SoundBuffer*> _buffer;	//!< \brief Vector of OpenAL buffers.
	std::vector <private_audio::SoundSrc*> _source;	//!< \brief Vector of OpenAL sources.

	int16 _max_sources;		//!< \brief Maximum number of availables sources.

	float _listener_position[3];	//!< \brief Position of the listener.
	float _listener_velocity[3];	//!< \brief Velociy of the listener.
	float _listener_orientation[3];	//!< \brief Orientation of the listener.

	private_audio::AudioFxManager _fx_manager;	//!< \brief Effects Manager for keeping tack of the effects.

	std::list <SoundDescriptor*> _sound;
	std::list <MusicDescriptor*> _music;

	static std::map<std::string, SoundDescriptor*> _persistantSounds; //!< \brief Holds the list of persistant sounds.

public:
	~GameAudio();

	bool SingletonInitialize ();

	float GetSoundVolume () const;
	float GetMusicVolume () const;

	void SetSoundVolume (const float volume);
	void SetMusicVolume (const float volume);

	/*! \brief Plays a sound once
	 *  \param filename The name of the sound file to play
	 *  
	 *  This method of playback is useful because it doesn't require any SoundDescriptor objects to be managed by the user.
	 *  This is ideal for the case of scripts which wish to play a sound only once.
	 */
	void PlaySound(const std::string& filename);

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
	void PauseAudio ();
	void ResumeAudio ();
	void StopAudio ();
	void RewindAudio ();
	//@}

	/*! \name Global Sound Manipulation Functions
	 *  \brief Performs specified operation on all active sounds.
	 */
	//@{
	//! \note Make sure to resume these sounds, otherwise the sources that they hold will never be released!
	void PauseAllSounds ();
	void ResumeAllSounds ();
	void StopAllSounds ();
	void RewindAllSounds ();
	//@}

	/*! \name Global Music Manipulation Functions
	 *  \brief Performs specified operation on all active music.
	 *
	 *  Since there is only one music source, these functions only affect that source. They are equivalent
	 *  to calling the {Pause/Resume/Stop/Rewind}Music functions on the MusicDescriptor which currently
	 *  has posession of the source.
	 */
	//@{
	void PauseAllMusic ();
	void ResumeAllMusic ();
	void StopAllMusic ();
	void RewindAllMusic ();
	//@}

	//! \name 3D functions
	//! \brief Functions for spatial sounds.
	/*!
		Set of functions used to manipulate the 3d features of the listener.
	*/
	//@{
	void SetListenerPosition (const float position[3]);
	void SetListenerVelocity (const float velocity[3]);
	void SetListenerOrientation (const float orientation[3]);
	void GetListenerPosition (float position[3]) const;
	void GetListenerVelocity (float velocity[3]) const;
	void GetListenerOrientation (float orientation[3]) const;
	//@}

	void Update ();

	void DEBUG_PrintInfo();

	//! \brief Returns a reference to the effects manager
	/*!
		This is used by the effects class to register the effect in the manager. Then, if needed, they can
		be updated.
		\return A reference to the effects manager.
	*/
	inline private_audio::AudioFxManager& GetFxManager ()
	{
		return _fx_manager;
	}

	//! \name Audio Effects functions.
	//! \brief Functions for effects applied to sounds.
	/*!
		Set of functions used to manipulate the 3d features of the listener.
	*/
	//@{
	void FadeIn (AudioDescriptor &descriptor, const float time);
	void FadeOut (AudioDescriptor &descriptor, const float time);
	void CrossFade (AudioDescriptor &descriptor_in, AudioDescriptor &descriptor_out, const float time);
	//@}

	//! \brief Plays a sound that persists beyond the current scope.
	/*!
		Plays a sound that has been stored for persistance.  If the sound has not been previously stored
		it will attempted to be loaded and played.
 		\param The name of the sound file to play.
		\return True if the specified file could be loaded/played. False otherwise.
	*/
	static void PlayPersistantSound(const std::string soundName);

private:
	private_audio::SoundSrc* _AcquireSoundSrc ();
};


} // End of namespace hoa_audio


#endif /* #ifndef __AUDIO_HEADER__ */
