////////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
////////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file   audio_stream.h
*** \author Moisés Ferrer Serra, byaku@allacrost.org
*** \brief  Header file for class for streaming audio from diferent sources
*** 
*** This code implements the functionality for more advanced streaming.
***
*** \note This code is not dependent on any (audio) library.
*** ***************************************************************************/


#ifndef __AUDIO_STREAM_HEADER__
#define __AUDIO_STREAM_HEADER__

#ifdef __MACH__
	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>
#else
	#include "al.h"
	#include "alc.h"
#endif

#include "audio_input.h"


namespace hoa_audio
{


namespace private_audio
{


//! \brief Availble streaming modes
/*
	Available streaming modes
*/
enum STREAM_MODE	
{
	STREAM_MEMORY	= 0,	//!< \brief Streaming from memory
	STREAM_FILE		= 1		//!< \brief Streaming from file
};




//! \brief Class for managing simple streaming based on audio input objects.
/*!
	This class manages properly streaming on input audio objects. It can handle
	looping properly.

	\note This class will be extended to manage customized looping
*/
class Stream
{
protected:
	bool _looping;				//!< \brief Flag for looping sound.
	uint32 _loop_start;			//!< \brief Cursor (sample) for the start position of the loop.
	uint32 _loop_end;			//!< \brief Cursor (sample) for the end position of the loop.
	IAudioInput* _audio_input;	//!< \brief Pointer to an input audio object.
	uint32 _cursor;				//!< \brief Sample position from the next read operation will be performed.
	bool _end_of_stream;		//!< \brief True if the end of stream was reached, false otherwise.

public:
	Stream (const std::string &filename, const int mode=STREAM_FILE, const bool loop=true);
	~Stream ();

	uint32 FillBuffer (uint8* buffer, const uint32 size);
	void Seek (const uint32 sample);

	//! \brief Gets the state of stream.
	/*!
		This functions returns true if the stream finished, or false otherwise. It is
		used to stop soundss when the stream is over. A looping sound will never reach
		the end of stream.
		\return True if the end of stream was reached, false otherwise.
	*/
	inline bool GetEndOfStream ()
	{
		return _end_of_stream;
	}


	//! \brief Enables/disables looping.
	/*!
		Ths function enables/disables looping.
		\param loop True to enable looping, false to disable it.
	*/
	inline void SetLooping (const bool loop)
	{
		_looping = loop;
		if (loop)
		{
			_end_of_stream = false;
		}
	}


	void SetLoopStart (const uint32 sample);
	void SetLoopEnd (const uint32 sample);

	//! \brief Gets the state of the looping.
	/*!
		Gets the looping state.
		\return True if looping is enabled, false if it is not.
	*/
	inline bool IsLooping () const
	{
		return _looping;
	}

	//! \name Accessors of the data of the class
	//! \brief Accessors for retrieving audio parameters.
	/*!
		Accessors for retrieving audio parameters. That includes samples per second, bits per sample,
		channels, data size and other derived variables. These values can not be assigned.
	*/
	//@{
	inline uint8 GetBitsPerSample () const
	{
		return _audio_input->GetBitsPerSample();
	}

	inline uint32 GetSamplesPerSecond () const
	{
		return _audio_input->GetSamplesPerSecond ();
	}

	inline uint16 GetChannels () const
	{
		return _audio_input->GetChannels ();
	}

	inline uint32 GetDataSize () const
	{
		return _audio_input->GetDataSize ();
	}

	inline uint32 GetSamples () const
	{
		return _audio_input->GetSamples ();
	}

	inline uint16 GetSampleSize () const
	{
		return _audio_input->GetSampleSize ();
	}

	inline float GetTime () const
	{
		return _audio_input->GetTime ();
	}
	//}@
};



} // End of namespace private_audio


} // End of namespace hoa_audio


#endif
