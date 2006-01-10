///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    audio_music.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Created: January 3rd, 2006
 * \brief   Header file for music-related code in the audio engine.
 *
 * The classes in this file are used for management and processing of all sound
 * Vorbis Ogg data.
 *
 * \note This code uses the OpenAL audio library. See http://www.openal.com/
 *
 * \note This code uses the Vorbis Ogg audio libraries. See http://www.vorbis.org/
 *****************************************************************************/

#ifndef __AUDIO_MUSIC_HEADER__
#define __AUDIO_MUSIC_HEADER__

#include "utils.h"
#include "defs.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>

namespace hoa_audio {

namespace private_audio {

//! The size (in bytes) to make
const uint32 MUSIC_BUFFER_SIZE = 32768;

/*!****************************************************************************
 * \brief An internal class used to manage music data information.
 *
 * This class serves as a wrapper to raw audio data information loaded by OpenAL.
 * Objects of this class are managed internally by the GameAudio class and are
 * never seen by the user.
 *
 * \note 1) It is assumed that all music loaded into this class is dual-channel,
 * and hence effects such as distance attenuation can be performed on it.
 *****************************************************************************/
class MusicBuffer {
public:
	MusicBuffer(std::string fname);
	~MusicBuffer();

	//! The filename of the audio data the buffer holds.
	std::string filename;
	//! The number of MusicDescriptor objects that refer to this object
	uint8 reference_count;

	//! File pointer used by vorbis ogg calls
	FILE* file_handle;
	//! A handle for the streaming ogg data
	OggVorbis_File file_stream;
	//! Various data about the open vorbis file.
	vorbis_info* file_info;
	//! Comments left in the file by its creator.
	vorbis_comment* file_comment;

	//! The buffers which will hold the streaming ogg data.
	ALuint buffers[2];
	//! Format for the data (number of channels and bit-width).
	ALenum format;

	//! Returns true if all OpenAL buffers are valid.
	bool IsValid();
	//! Removes a single reference to this buffer. If the reference count becomes zero, the buffer is destroyed.
	void RemoveReference();
	//! This function refills a buffer with the next segment in an audio stream.
	//! \param buff The buffer to fill with audio data. This will always be one of the elements of buffers[].
	void RefillBuffer(ALuint buff);

	//! Displays the properties of the buffered data to standard output.
	void DEBUG_PrintProperties();
};

class MusicSource {
public:
	MusicSource();
	~MusicSource();

	//! The OpenAL source that this class object maintains.
	ALuint source;
	//! A pointer to the MusicDescriptor that currently owns this source.
	MusicDescriptor *owner;

	//! Returns true if OpenAL determines that the source is valid.
	bool IsValid();
	//! Removes all pending buffers from the source.
	void EmptyStreamQueue();
	//! Updates the buffers that are being stream-fed data for the source to output.
	void UpdateStreamQueue();
	//! Displays the properties of the music source to standard output.
	void DEBUG_PrintProperties() {}
};

} // namespace private_audio

class MusicDescriptor {
	friend class GameAudio;
	friend class private_audio::MusicBuffer;
	friend class private_audio::MusicSource;
private:
	//! A pointer to the music buffer that is used.
	private_audio::MusicBuffer *_data;
	//! A pointer to the music source that is used.
	private_audio::MusicSource *_origin;

public:
	MusicDescriptor();
	~MusicDescriptor();

	const std::string &GetFilename()
		{ if (_data != NULL) return _data->filename; }
	//! Loads the music file from memory.
	//! \param fname The name of the file to load, without path information or file extension attached.
	bool LoadMusic(std::string fname);
	/*! \brief Frees the audio data associated with this object.
	 *  This function will do the following: delete the MusicBuffer it refers to (as long as nothing else
	 *  is refering to the same buffer), release and re-initialize the music source it holds.
	 */
	void FreeMusic();
	/*! \brief Grabs a music source for the descriptor to use.
	 *  \note Since there is only one music source, calling this can be dangerous because another MusicDescriptor
	 *  may currently be using the source. If that is the case, calling this function will stop the currently
	 *  playing music abruptly.
	 */
	void AllocateSource();

	//! \name Standard Music Functions
	//! \brief Common methods for standard manipulation of music data.
	//! \note When the music is paused, both PlayMusic and ResumeMusic perform the same function.
	//@{
	void PlayMusic();
	void PauseMusic();
	void ResumeMusic();
	void StopMusic();
	void RewindMusic();
	//@}

	//! Displays the properties of the music descriptor's buffer.
	void DEBUG_dataProperties()
		{ if (_data != NULL) _data->DEBUG_PrintProperties(); }
	//! Displays the properties of the music descriptor's source.
	void DEBUG_originProperties()
		{ if (_origin != NULL) _origin->DEBUG_PrintProperties(); }
}; // class MusicDescriptor

} // namespace hoa_audio

#endif
