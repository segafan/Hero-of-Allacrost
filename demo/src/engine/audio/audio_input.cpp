////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   audio_input.cpp
*** \author Mois�s Ferrer Serra - Aaron Smith, byaku@allacrost.org - etherstar@allacrost.org.
*** \brief  Source for the classes that provide input for sounds
*** 
*** This code provides classes for loading sounds (WAV and OGG). It also
*** provides the functionality for basic streaming operations, both from memory and from 
*** file
***
*** \note This code uses Ogg-vorbis library for loading Ogg files
*** ***************************************************************************/


#include "audio_input.h"
#include <string>
#include <iostream>


using namespace hoa_audio::private_audio;


/********************************/
/*			IAudioInput			*/
/********************************/

IAudioInput::IAudioInput () :
_samples_per_second  (0)
,_bits_per_sample (0)
,_channels (0)
,_samples (0)
,_data_size (0)
,_sample_size (0)
,_time (0)
{
}





/********************************/
/*			AudioFile			*/
/********************************/


//! \brief Constructor of the class.
/*!
	Constructor of the class. Specify the name of the audio file to be used.
	\param file_name File name of the audio file to open.
*/
IAudioFile::IAudioFile (const std::string file_name) :
IAudioInput ()
,_file_name (file_name)
{
}






/********************************/
/*			AudioMemory			*/
/********************************/

//! \brief Constructor of the class.
/*!
	Constructor of the class. It needs the parameters of the audio to interpret the raw audio data.
	\param samples_per_second Samples per second.
	\param bits_per_sample Bits per sample.
	\param channels Number of channels (1=mono, 2=stereo).
	\param samples Number of samples in the data.
	\param data Pointer to the raw PCM data.
*/
AudioMemory::AudioMemory (const uint32 samples_per_second, const uint8 bits_per_sample,
						  const uint16 channels, const uint32 samples, uint8* data) :
IAudioInput ()
,_buffer (0)
,_cursor (0)
{
	_samples_per_second = samples_per_second;
	_bits_per_sample = bits_per_sample;
	_channels = channels;
	_samples = samples;

	_sample_size = _bits_per_sample * _channels / 8;
	_time = (float)samples / (float)_samples_per_second;
	_data_size = _samples * _sample_size;

	_buffer = new uint8 [_data_size];
	memcpy (_buffer, data, _data_size);
}


AudioMemory::~AudioMemory ()
{
	if (_buffer)
	{
		delete[] _buffer;
	}
}


bool AudioMemory::Initialize ()
{
	return true;
}


void AudioMemory::Seek (const uint32 cursor)
{
	if (cursor >= 0 && cursor < _samples)
	{
		_cursor = cursor;
	}
}


uint32 AudioMemory::Read (uint8* buffer, const uint32 size, bool &end)
{
	// Clamp the number of samples to read in case there are not enough because of end of stream
	uint32 read ((_samples - _cursor >= size) ? size : _samples - _cursor);

	// Copy the data in the buffer and move the read cursor
	memcpy (buffer, _buffer + _cursor*_sample_size, read*_sample_size);
	_cursor += read;

	end = (_cursor == _samples);

	return read;
}





/********************************/
/*			WavFile				*/
/********************************/

//! \brief Constructor of the class.
/*!
	Constructor of the class.
	\param file_name Name of the WAV file.
*/
WavFile::WavFile (const std::string &file_name) :
IAudioFile (file_name),
_file_in ()
{
}


WavFile::~WavFile ()
{
	_file_in.close ();
};


bool WavFile::Initialize ()
{
	//! \todo Remake these function so more complex WAV files can be loaded

	uint32 size;

	char buffer[4];

	_file_in.open (_file_name.c_str(), std::ios::binary);
	if (_file_in.fail())
	{
		_file_in.clear();
		return false;
	}

	// Check Chunk ID to be "RIFF" -- 4 bytes
	_file_in.read (buffer, 4);
	if (buffer[0] != 'R' && buffer[1] != 'I' && buffer[2] != 'F' && buffer[3] != 'F')
	{
		return false;
	}

	// Get chunk size (file size - 8) -- 4 bytes
	_file_in.read (buffer, 4);
	memcpy (&size, buffer, 4);
#ifdef __BIG_ENDIAN__
	// Swap the bytes for the big endian hardware.
	uint8 e1, e2, e3, e4;
	e1 = size & 255;
  	e2 = (size >> 8) & 255;
  	e3 = (size >> 16) & 255;
  	e4 = (size >> 24) & 255;
  	size = ((uint32)e1 << 24) + ((uint32)e2 << 16) + ((uint32)e3 << 8) + e4;
#endif

	// Check format to be "WAVE" -- 4 bytes
	_file_in.read (buffer, 4);
	if (buffer[0] != 'W' && buffer[1] != 'A' && buffer[2] != 'V' && buffer[3] != 'E')
	{
		return false;
	}

	// Check SubChunk ID to be "fmt " -- 4 bytes
	_file_in.read (buffer, 4);
	if (buffer[0] != 'f' && buffer[1] != 'm' && buffer[2] != 't' && buffer[3] != ' ')
	{
		return false;
	}

	// Check subchunk size (to be 16) -- 4 bytes
	_file_in.read (buffer, 4);
	memcpy (&size, buffer, 4);
#ifdef __BIG_ENDIAN__
	// Swap the bytes for the big endian hardware.
	uint8 e1, e2, e3, e4;
	e1 = size & 255;
  	e2 = (size >> 8) & 255;
  	e3 = (size >> 16) & 255;
  	e4 = (size >> 24) & 255;
  	size = ((uint32)e1 << 24) + ((uint32)e2 << 16) + ((uint32)e3 << 8) + e4;
#endif
	if (size != 16)
	{
		return false;
	}

	// Check audio format (now just PCM supported) -- 2 bytes
	_file_in.read (buffer, 2);
	size = 0;
	memcpy (&size, buffer, 2);
#ifdef __BIG_ENDIAN__
	uint8 e1, e2;
	e1 = size & 255;
	e2 = (size>> 8) * 255;
	size = (e1 << 8) + e2;
#endif
	if (size != 1)		// PCM == 1
	{
		return false;
	}

	// Get the number of channels (now just mono and stereo supported) -- 2 bytes
	_file_in.read (buffer, 2);
	memcpy (&_channels, buffer, 2);
#ifdef __BIG_ENDIAN__
	uint8 e1, e2;
	e1 = _channels & 255;
	e2 = (_channels >> 8) * 255;
	_channels = (e1 << 8) + e2;
#endif
	if (_channels != 1 && _channels != 2)
	{
		return false;
	}

	// Get sample rate (usually 11025, 22050, 44100) -- 4 bytes
	_file_in.read (buffer, 4);
	memcpy (&_samples_per_second, buffer, 4);
#ifdef __BIG_ENDIAN__
	uint8 e1, e2, e3, e4;
	e1 = _samples_per_second & 255;
  	e2 = (_samples_per_second >> 8) & 255;
  	e3 = (_samples_per_second >> 16) & 255;
  	e4 = (_samples_per_second >> 24) & 255;
  	_samples_per_second = ((uint32)e1 << 24) + ((uint32)e2 << 16) + ((uint32)e3 << 8) + e4;
#endif

	// Get byte rate -- 4 bytes
	uint32 byte_rate;
	_file_in.read (buffer, 4);
	memcpy (&byte_rate, buffer, 4);
#ifdef __BIG_ENDIAN__
	uint8 e1, e2, e3, e4;
	e1 = byte_rate & 255;
  	e2 = (byte_rate >> 8) & 255;
  	e3 = (byte_rate >> 16) & 255;
  	e4 = (byte_rate >> 24) & 255;
  	byte_rate = ((uint32)e1 << 24) + ((uint32)e2 << 16) + ((uint32)e3 << 8) + e4;
#endif

	// Get block alignement (channels * bits_per_sample / 8) -- 2 bytes
	_file_in.read (buffer, 2);
	memcpy (&_sample_size, buffer, 2);
#ifdef __BIG_ENDIAN__
	uint8 e1, e2;
	e1 = _sample_size & 255;
	e2 = (_sample_size >> 8) * 255;
	_sample_size = (e1 << 8) + e2;
#endif

	// Get bits per sample -- 2 bytes
	_file_in.read (buffer, 2);
	memcpy (&_bits_per_sample, buffer, 2);
#ifdef __BIG_ENDIAN__
	uint8 e1, e2;
	e1 = _bits_per_sample & 255;
	e2 = (_bits_per_sample >> 8) * 255;
	_bits_per_sample = (e1 << 8) + e2;
#endif

	// Check subchunck 2 ID (to be "data") -- 4 bytes
	_file_in.read (buffer, 4);
	if (buffer[0] != 'd' && buffer[1] != 'a' && buffer[2] != 't' && buffer[3] != 'a')
	{
		return false;
	}
	
	// Check subchunck 2 size -- 4 bytes
	_file_in.read (buffer, 4);
	memcpy (&_data_size, buffer, 4);
#ifdef __BIG_ENDIAN__
	uint8 e1, e2, e3, e4;
	e1 = _data_size & 255;
  	e2 = (_data_size >> 8) & 255;
  	e3 = (_data_size >> 16) & 255;
  	e4 = (_data_size >> 24) & 255;
  	_data_size = ((uint32)e1 << 24) + ((uint32)e2 << 16) + ((uint32)e3 << 8) + e4;
#endif

	_data_init = _file_in.tellg ();

	_samples = _data_size / _sample_size;
	_time = (float)_samples / (float)_samples_per_second;

	return true;
}


void WavFile::Seek (uint32 cursor)
{
	std::streamoff sample (cursor * _sample_size);

	if (static_cast <uint32>(sample) >= 0 && static_cast <uint32>(sample) < _data_size)
	{
		_file_in.clear();
		_file_in.seekg (sample+static_cast<std::streamoff>(_data_init), std::ios_base::beg);
	}
}


uint32 WavFile::Read (uint8* buffer, const uint32 size, bool &end)
{
	_file_in.read (reinterpret_cast<char*>(buffer), size*_sample_size);

	uint32 read (_file_in.gcount() / _sample_size);
	end = (read != size);

	return read;
}




OggFile::OggFile (const std::string &file_name) :
IAudioFile (file_name)
,_read_buffer_position (0)
,_read_buffer_size (0)
{
}


OggFile::~OggFile ()
{
	ov_clear (&_vorbis_file);
}


bool OggFile::Initialize ()
{
	// Win32 requires a special loading method in order load ogg files
	// properly when dynamically linking vorbis libs.
#ifdef WIN32
	// The Win32 workaround is to use the ov_open_callbacks function.

	// Callbacks struct defining open, closing, seeking and location behaviors.
	ov_callbacks callbacks = 
	{
		(size_t (*)(void *, size_t, size_t, void *))  fread,
		(int (*)(void *, ogg_int64_t, int)) _FseekWrap,
		(int (*)(void *)) fclose, 
		(long (*)(void *)) ftell
	};

	FILE* file = fopen (_file_name.c_str(), "rb");

	if (ov_open_callbacks(file, &_vorbis_file, NULL, 0, callbacks) < 0)
	{
		fclose(file);
		std::cout << "Input does not appear to be an Ogg bitstream" << std::endl;
		return false;
	}

#else
	// File loading code for non Win32 platforms.  Much simpler.
	FILE* file = fopen (_file_name.c_str(), "rb");

	if (ov_open(file, &_vorbis_file, NULL, 0) < 0)
	{
		fclose(file);
		std::cout << "Input does not appear to be an Ogg bitstream" << std::endl;
		return false;
	}
#endif

	_channels = _vorbis_file.vi->channels;
	_samples_per_second = _vorbis_file.vi->rate;
	_bits_per_sample = 16;
	_samples = (uint32)ov_pcm_total (&_vorbis_file, -1);
	_time = (float)ov_time_total (&_vorbis_file, -1);
	_sample_size = _channels * _bits_per_sample / 8;
	_data_size = _samples * _sample_size;

	return true;
}


void OggFile::Seek (const uint32 cursor)
{
	if (ov_seekable (&_vorbis_file))
	{
		ov_pcm_seek (&_vorbis_file, cursor);
	};

	// Reset the temporal buffer by setting the position to 0
	_read_buffer_position = 0;
	_read_buffer_size = 0;
}


uint32 OggFile::Read (uint8* buffer, const uint32 size, bool &end)
{
	int current_section;
	uint32 read (0);

	end = false;

	// First get data from the temporal buffer (if there is in it)
	if (_read_buffer_size > 0)
	{
		if (_read_buffer_size > size*_sample_size)
		{
			read = size*_sample_size;
		}
		else
		{
			read = _read_buffer_size;
		}
		memcpy (buffer, _read_buffer+_read_buffer_position, read);
		_read_buffer_size -= read;
		_read_buffer_position += read;
	}

	while (read < size*_sample_size && !end)
	{
		_read_buffer_position = 0;
		
		
#ifdef  __BIG_ENDIAN__
        uint32 ret = ov_read (&_vorbis_file, (char*)_read_buffer, 4096, 1, _bits_per_sample/8, 1, &current_section);
#else
        uint32 ret = ov_read (&_vorbis_file, (char*)_read_buffer, 4096, 0, _bits_per_sample/8, 1, &current_section);
#endif 

		// If nothing could be read, the EOF was reached
		if (ret == 0)
		{
			end = true;
		}
		else if (ret < 0)
		{
			//! \todo Care about errors when reading OGG
		}
		else
		{
			//! \todo Care about differences of sample rate when reading OGG

			_read_buffer_size = ret;
			ret = ((size*_sample_size)-read > ret) ? ret : (size*_sample_size)-read;
			memcpy (buffer+read, _read_buffer+_read_buffer_position, ret);
			read += ret;
			_read_buffer_size -= ret;
			_read_buffer_position += ret;
		}
	}

	return read / _sample_size;
}

int OggFile::_FseekWrap(FILE *f,ogg_int64_t off,int whence)
{
    if (f == NULL)
	{
		return(-1);
	}
	else
	{
		return fseek(f,static_cast<long>(off),whence);
	}
}

