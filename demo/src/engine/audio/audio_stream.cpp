////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   audio_stream.cpp
*** \author Moisés Ferrer Serra, byaku@allacrost.org
*** \brief  Implementation of the streaming audio classes
*** 
*** This code implements the functionality for more advanced streaming. That 
*** streaming class takes care of looping and customized looping.
***
*** \note This code is not dependent on any (audio) library.
*** ***************************************************************************/


#include "audio_stream.h"
#include "audio_input.h"
#include <cstring>
#include <cctype>


using namespace hoa_audio::private_audio;


//! \brief Constructor of the class.
/*!
	Initializes the stream.
	\param filename Name of the file from where the streaming will be performed.
	\param mode Mode of streaming (0 for streaming from memory, 1 for streaming from file).
	\param loop Flag to enable looping.
*/
Stream::Stream (const std::string &filename, const int mode, const bool loop) :
_looping (loop),
_audio_input (0),
_cursor (0),
_loop_start (0),
_loop_end (0),
_end_of_stream (false)
{
	if (filename.size() <= 3)		// Name of file is at least 3 letters (so the extension is in there)
	{
		return;
	}

	IAudioInput* input (0);

	std::string audio_extension = filename.substr (filename.size()-3, 3);

	// Convert to uppercase
	for (std::string::iterator str=audio_extension.begin(); str<audio_extension.end(); str++)
	{
		*str = std::toupper (*str);
	}

	// Based on the extension of the file, load properly one
	if (!audio_extension.compare ("WAV"))
	{
		input = new WavFile (filename);
	}
	else if (!audio_extension.compare ("OGG"))
	{
		input = new OggFile (filename);
	}

	input->Initialize ();

	// If streaming mode is from memory
	if (mode == STREAM_FILE)
	{
		_audio_input = input;
	}
	else if (mode == STREAM_MEMORY)
	{
		uint8* buffer = new uint8 [input->GetDataSize()];
		bool end;
		input->Read (buffer, input->GetSamples(), end);
		_audio_input = new AudioMemory (input->GetSamplesPerSecond(), input->GetBitsPerSample(), input->GetChannels(), input->GetSamples(), buffer);
		delete[] buffer;
	}

	_loop_end = GetSamples ();
}


Stream::~Stream ()
{
	if (_audio_input)
	{
		delete _audio_input;
	}
}


//! \brief Fills the buffer.
/*!
	Fills the buffer with data readed from the stream.
	\return Number of samples readed
	\param buffer Buffer where the data will be loaded.
	\param size Number of samples to be read.
*/
uint32 Stream::FillBuffer (uint8* buffer, const uint32 size)
{
	if (!_audio_input)
	{
		return 0;
	}

	uint32 readed (0);
	uint32 read;

	if (_looping)
	{
		while (readed < size)
		{
			if (_cursor == _loop_end || _cursor == GetSamples())
			{
				_audio_input->Seek (_loop_start);
				_cursor = _loop_start;
			}
			read = (size-readed < _loop_end-_cursor) ? size-readed : _loop_end-_cursor;
			readed += _audio_input->Read (buffer+readed*GetSampleSize(), read, _end_of_stream);
			_cursor += readed;
		}
		_end_of_stream = false;
		return readed;
	}
	else
	{
		while (readed < size)
		{
			read = (size-readed < GetSamples()-_cursor) ? size-readed : GetSamples()-_cursor;
			readed += _audio_input->Read (buffer+readed*GetSampleSize(), read, _end_of_stream);
			_cursor += readed;
			if (_end_of_stream)
			{
//				_audio_input->Seek (0);
				return readed;
			}
		}
		return readed;
	}

	return 0;

/*
	if (_loop_start == 0 && _loop_end == GetSamples())	// If no special looping structure is presented, do normal looping
	{
		readed = _audio_input->Read (buffer, size, end);

		while (readed < size)
		{
			if (_looping)		// If there is looping, rewind to the beginning and continue filling the buffer
			{
				_audio_input->Seek (0);
				readed += _audio_input->Read (buffer+(readed*_audio_input->GetChannels()*_audio_input->GetBitsPerSample()/8), size-readed, end);
			}
			else			// If there is no looping, fill with silence
			{
				memset (buffer+(readed*_audio_input->GetChannels()*_audio_input->GetBitsPerSample()/8), 0, (size-readed)*(_audio_input->GetChannels()*_audio_input->GetBitsPerSample()/8));
				silence = true;
				readed = size;
			}
		}
	}
	else
	{
		while (readed < size)
		{
			if (_looping)		// If there is looping, rewind to the beginning and continue filling the buffer
			{
				_audio_input->Seek (0);
				readed += _audio_input->Read (buffer+(readed*_audio_input->GetChannels()*_audio_input->GetBitsPerSample()/8), size-readed, end);
			}
			else			// If there is no looping, fill with silence
			{
				memset (buffer+(readed*_audio_input->GetChannels()*_audio_input->GetBitsPerSample()/8), 0, (size-readed)*(_audio_input->GetChannels()*_audio_input->GetBitsPerSample()/8));
				silence = true;
				readed = size;
			}
		}
	}*/
}


void Stream::Seek (const uint32 sample)
{
	if (sample >= 0 && sample < GetSamples())
	{
		_audio_input->Seek (sample);
		_cursor = sample;
		_end_of_stream = false;
	}
}



//! \brief Sets the value for the start position of the loop.
/*!
	Sets the value for the start position of the loop. If an invalid position
	is provided, no operation is performed.
*/
void Stream::SetLoopStart (const uint32 sample)
{
	if (sample >= 0 && sample < GetSamples())
	{
		_loop_start = sample;
	}
}


//! \brief Sets the value for the start position of the loop.
/*!
	Sets the value for the end position of the loop. If an invalid position
	is provided, no operation is performed.
*/
void Stream::SetLoopEnd (const uint32 sample)
{
	if (sample >= 0 && sample < GetSamples())
	{
		_loop_end = sample;
	}
}

