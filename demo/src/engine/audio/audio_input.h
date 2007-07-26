////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   audio_input.h
*** \author Mois�s Ferrer Serra, byaku@allacrost.org
*** \brief  Header file for classes that provide input for sounds
*** 
*** This code provides classes for loading sounds (WAV and OGG). It also
*** provides the functionality for basic streaming operations, both from memory and from 
*** file
***
*** \note This code uses Ogg-vorbis library for loading Ogg files
*** ***************************************************************************/


#ifndef __AUDIO_INPUT_HEADER__
#define __AUDIO_INPUT_HEADER__

/*
typedef int int32;
typedef unsigned int uint32;
typedef short int int16;
typedef unsigned short int uint16;
typedef char int8;
typedef unsigned char uint8;
*/

#ifdef __MACH__
	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>
#else
	#include "al.h"
	#include "alc.h"
#endif

#include "utils.h"
#include <string>
#include <fstream>
#include <vorbis/vorbisfile.h>


//! \brief Namespace for the audio engine
namespace hoa_audio
{

//! \brief Private namespace for the audio engine
namespace private_audio
{

//! \brief Interface for audio input objects
/*!
	This class is an interface for objects that work as an audio source in the
	engine. All the classes that will be in that group (audio input) will 
	derive from this one and will have to implement all the pure virtual functions.
	These functions are the real interface of the class.
*/
class IAudioInput
{
private:

protected:
	uint32 _samples_per_second;		//!< \brief Samples per second (tipically 11025, 22050, 44100).
	uint8 _bits_per_sample;			//!< \brief Bits per sample (tipically 8 or 16).
	uint16 _channels;				//!< \brief Channels of the sound (1=mono, 2=stereo).
	uint32 _samples;				//!< \brief Samples of the audio piece.

	// These are derived variables, defined becase are frequently required
	uint32 _data_size;				//!< \brief Size of the audio in bytes.
	uint16 _sample_size;			//!< \brief Size of a sample in bytes (_bits_per_sample * _channels / 8).
	float _time;					//!< \brief Time of the audio piece (_samples / _samples_per_second).

public:
	IAudioInput ();
	virtual ~IAudioInput () {  };

	//! \brief Initialize the input stream.
	/*!
		This function is responsible for prepairing the stream from where the data is going to be read.
		For instance, if it is a file, it will be responsible of getting a file descriptor, open the 
		file and seek the data. If the input is from memory, this function will set the values of all
		the variables, so the data can be properly interpreted.
		\return True if the stream was successfully opened and is ready to read, false otherwise.
	*/
	virtual bool Initialize () = 0;

	//! \brief Seeks the stream in the sample specified.
	/*!
		Seeks the stream in the sample specified. If the position is out of the 
		stream, the read cursor is not modified, so posterior calls to a Read function will 
		work as if no Seek call was done.
		\param cursor Sample position to place the read cursor.
	*/
	virtual void Seek (const uint32 cursor) = 0;

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
	virtual uint32 Read (uint8* buffer, const uint32 size, bool &end) = 0;

	//! \name Accessors of the data of the class
	//! \brief Accessors for retrieving audio parameters.
	/*!
		Accessors for retrieving audio parameters. That includes samples per second, bits per sample,
		channels, data size and other derived variables. These values can not be assigned.
	*/
	//@{
	inline uint32 GetSamplesPerSecond () const
	{
		return _samples_per_second;
	}

	inline uint8 GetBitsPerSample () const
	{
		return _bits_per_sample;
	}

	inline uint16 GetChannels () const
	{
		return _channels;
	}

	inline uint32 GetSamples () const
	{
		return _samples;
	}

	inline uint32 GetDataSize () const
	{
		return _data_size;
	}

	inline float GetTime () const
	{
		return _time;
	}

	inline uint16 GetSampleSize () const
	{
		return _sample_size;
	}
	//}@
};




//! \brief Interface for audio files (for input).
/*!
	This class is an interface for audio files that will be read. Any supported
	audio file will be implemented in a new class derivated from this one.
*/
class IAudioFile : public IAudioInput
{
protected:
	std::string _file_name;		//!< \brief File name

public:
	IAudioFile (const std::string file_name);
	~IAudioFile () {  };

	//! \brief Gets the names of the file.
	/*!
		Gets the name of the file.
	*/
	inline const std::string &GetFileName () const
	{
		return _file_name;
	}
};




//! \brief Class for managing input from WAV files.
/*!
	Class for managing input from WAV files.
*/
class WavFile : public IAudioFile
{
private:
	std::ifstream _file_in;		//!< \brief Input stream for the file.
	std::streampos _data_init;	//!< \brief Offset from the begining of the file where the data begins.

public:
	WavFile (const std::string &file_name);
	~WavFile ();

	bool Initialize ();
	void Seek (const uint32 cursor);
	uint32 Read (uint8* buffer, const uint32 size, bool &end);
};





//! \brief Class for managing input from OGG files.
/*!
	Class for managing input from OGG files.
*/
class OggFile : public IAudioFile
{
private:
	OggVorbis_File _vorbis_file;		//!< \brief Vorbis file information.
	unsigned char _read_buffer[4096];	//!< \brief Temporal buffer for reading data.
	uint16 _read_buffer_position;		//!< \brief Position of previous read data (for the emporal buffer).
	uint16 _read_buffer_size;			//!< \brief Size of available data on the buffer (for the emporal buffer).

	//! \brief Reads up to the specified number of samples.
	/*!
		Required for opening ogg files on the win32 platform. Provides a safe wrap of the fseek function. 
		\param f Pointer to the FILE object which represents the stream. 
		\param off Number of bytes to offset from the stream's origin.
		\param whence The position to read, added to the off paramater to determine actual seek position.
		\return Zero if successful, the nonzer fseek error code otherwise.
	*/
	static int _FseekWrap(FILE * f, ogg_int64_t off, int whence);

public:
	OggFile (const std::string &file_name);
	~OggFile ();

	bool Initialize ();
	void Seek (const uint32 cursor);
	uint32 Read (uint8* buffer, const uint32 size, bool &end);
};





//! \brief Class for managing input from memory.
/*!
	Class for managing input from memory.
*/
class AudioMemory : public IAudioInput
{
private:
	uint8* _buffer;		//!< \brief Data buffer where the audio is stored.
	uint32 _cursor;		//!< \brief Position of the cursor where the next read operation will be performed.

public:
	AudioMemory (const uint32 samples_per_second, const uint8 bits_per_sample,
				 const uint16 channels, const uint32 samples, uint8* data);
	~AudioMemory ();

	bool Initialize ();
	void Seek (const uint32 cursor);
	uint32 Read (uint8* buffer, const uint32 size, bool &end);
};


} // End of namespace private_audio


} // End of namespace hoa_audio


#endif

