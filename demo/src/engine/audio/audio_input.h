////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   audio_input.h
*** \author Moisés Ferrer Serra, byaku@allacrost.org
*** \brief  Header file for classes that provide input for sounds
***
*** This code provides classes for loading sounds (WAV and OGG). It also
*** provides the functionality for basic streaming operations, both from memory
*** and from a file.
***
*** \note This code uses the Ogg/Vorbis libraries for loading Ogg files. WAV
*** files are loaded via custom loading code
*** ***************************************************************************/

#ifndef __AUDIO_INPUT_HEADER__
#define __AUDIO_INPUT_HEADER__

#ifdef __MACH__
	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>
#else
	#include "al.h"
	#include "alc.h"
#endif

#include <vorbis/vorbisfile.h>
#include <fstream>

#include "defs.h"
#include "utils.h"

namespace hoa_audio {

namespace private_audio {

//! \brief Interface for audio input objects
/*!
	This class is an interface for objects that work as an audio source in the
	engine. All the classes that will be in that group (audio input) will 
	derive from this one and will have to implement all the pure virtual functions.
	These functions are the real interface of the class.
*/
class AudioInput {
public:
	AudioInput ();

	virtual ~AudioInput ()
		{};

	//! \brief Initialize the input stream.
	/*!
		This function is responsible for prepairing the stream from where the data is going to be read.
		For instance, if it is a file, it will be responsible of getting a file descriptor, open the 
		file and seek the data. If the input is from memory, this function will set the values of all
		the variables, so the data can be properly interpreted.
		\return True if the stream was successfully opened and is ready to read, false otherwise.
	*/
	virtual bool Initialize() = 0;

	//! \brief Seeks the stream in the sample specified.
	/*!
		Seeks the stream in the sample specified. If the position is out of the 
		stream, the read cursor is not modified, so posterior calls to a Read function will 
		work as if no Seek call was done.
		\param cursor Sample position to place the read cursor.
	*/
	virtual void Seek(uint32 cursor) = 0;

	//! \brief Reads up to the specified number of samples.
	/*!
		Reads up to the specified number of samples, and stores it in a buffer. It can store from 0 to
		the spcified number of samples. The buffer must hold enough information for the data.
		\param buffer Buffer to store the data. After a read operation, the read cursor is displaced 
		automatically.
		\param size Number of samples to be read.
		\param end The function will set this to true if the end of stream is reached, false otherwise.
		\return Number of samples readed.
	*/
	virtual uint32 Read(uint8* buffer, uint32 size, bool& end) = 0;

	//! \name Accessors of the data of the class
	//! \brief Accessors for retrieving audio parameters.
	/*!
		Accessors for retrieving audio parameters. That includes samples per second, bits per sample,
		channels, data size and other derived variables. These values can not be assigned.
	*/
	//@{
	uint32 GetSamplesPerSecond() const
		{ return _samples_per_second; }

	uint8 GetBitsPerSample() const
		{ return _bits_per_sample; }

	uint16 GetChannels() const
		{ return _channels; }

	uint32 GetSamples() const
		{ return _samples; }

	uint32 GetDataSize() const
		{ return _data_size; }

	float GetTime() const
		{ return _time; }

	uint16 GetSampleSize() const
		{ return _sample_size; }
	//@}

protected:
	//! \brief Samples per second (typically 11025, 22050, 44100).
	uint32 _samples_per_second;

	//! \brief Bits per sample (typically 8 or 16).
	uint8 _bits_per_sample;

	//! \brief Channels of the sound (1=mono, 2=stereo).
	uint16 _channels;

	//! \brief Samples of the audio piece.
	uint32 _samples;

	//! \brief Size of the audio in bytes.
	uint32 _data_size;

	//! \brief Size of a sample in bytes (_bits_per_sample * _channels / 8).
	uint16 _sample_size;

	//! \brief Time of the audio piece (_samples / _samples_per_second).
	float _time;
}; // class AudioInput


/** ****************************************************************************
*** \brief Manages input extraced from .wav files
***
*** Wav files are usually used for sounds. This class implements its own custom
*** wav file parser/loader to interpret the data from the file into meaningful
*** audio data.
*** ***************************************************************************/
class WavFile : public AudioInput {
public:
	WavFile(const std::string& file_name) :
		AudioInput(), _file_name(file_name) {}

	~WavFile()
		{ if (_file_input) _file_input.close(); }

	//! \brief Inherited functions from AudioInput class
	//@{
	//! \todo Enable this function to handle loading of more complex WAV files
	bool Initialize();

	void Seek(uint32 cursor);

	uint32 Read(uint8* buffer, uint32 size, bool& end);
	//@}

	const std::string& GetFileName() const
		{ return _file_name; }

private:
	//! \brief The name of the audio file operated on by this class
	std::string _file_name;

	//! \brief The input I/O stream for the file
	std::ifstream _file_input;

	//! \brief The offset to where the data begins in the file (past the header information)
	std::streampos _data_init;
}; // class WavFile : public AudioInput





//! \brief Class for managing input from OGG files.
/*!
	Class for managing input from OGG files.
*/
/** ****************************************************************************
*** \brief Represents an OpenAL buffer
***
*** A buffer in OpenAL is simply a structure which contains raw audio data.
*** Buffers must be attached to an OpenAL source in order to play. OpenAL
*** suppports an infinte number of buffers (as long as there is enough memory).
*** ***************************************************************************/
class OggFile : public AudioInput {
public:
	OggFile(const std::string &file_name) :
		AudioInput(), _file_name(file_name), _read_buffer_position(0), _read_buffer_size(0) {}

	~OggFile()
		{ ov_clear(&_vorbis_file); }

	//! \brief Inherited functions from AudioInput class
	//@{
	bool Initialize();

	void Seek(uint32 cursor);

	uint32 Read(uint8* buffer, uint32 size, bool& end);
	//@}

	const std::string& GetFileName() const
		{ return _file_name; }

private:
	//! \brief The name of the audio file operated on by this class
	std::string _file_name;

	//!< \brief Vorbis file information.
	OggVorbis_File _vorbis_file;		

	//!< \brief Temporal buffer for reading data.
	unsigned char _read_buffer[4096];	

	//!< \brief Position of previous read data (for the emporal buffer).
	uint16 _read_buffer_position;		

	//!< \brief Size of available data on the buffer (for the emporal buffer).
	uint16 _read_buffer_size;			

	//! \brief Reads up to the specified number of samples.
	/*!
		Required for opening ogg files on the win32 platform. Provides a safe wrap of the fseek function. 
		\param f Pointer to the FILE object which represents the stream. 
		\param off Number of bytes to offset from the stream's origin.
		\param whence The position to read, added to the off paramater to determine actual seek position.
		\return Zero if successful, the nonzer fseek error code otherwise.
	*/
	static int _FileSeekWrapper(FILE* file, ogg_int64_t off, int whence);
}; // class OggFile : public AudioInput


/** ****************************************************************************
*** \brief Manages audio input data that is stored in memory
***
*** 
*** ***************************************************************************/
class AudioMemory : public AudioInput {
public:
//! \brief Constructor of the class.
/*!
	Constructor of the class. It needs the parameters of the audio to interpret the raw audio data.
	\param samples_per_second Samples per second.
	\param bits_per_sample Bits per sample.
	\param channels Number of channels (1=mono, 2=stereo).
	\param samples Number of samples in the data.
	\param data Pointer to the raw PCM data.
*/
	AudioMemory(uint32 samples_per_second, uint8 bits_per_sample, uint16 channels, uint32 samples, uint8* data);

	~AudioMemory();

	//! \brief Inherited functions from AudioInput class
	//@{
	//! \note Audio memory does not need to be initialized, as that is done by the class constructor
	bool Initialize()
		{ return true; }

	void Seek(uint32 cursor);

	uint32 Read(uint8* buffer, uint32 size, bool& end);
	//@}

private:
	//!< \brief Data buffer where the audio is stored.
	uint8* _buffer;

	//!< \brief Position of the cursor where the next read operation will be performed.
	uint32 _cursor;
}; // class AudioMemory : public AudioInput

} // namespace private_audio

} // namespace hoa_audio

#endif // __AUDIO_INPUT_HEADER__
