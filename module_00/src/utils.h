///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    utils.h
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 23rd, 2005
 * \brief   Header file for Allacrost utility code.
 *
 * This code includes various utility functions that are used across different
 * parts of the game. This file is included in every header file in the
 * Allacrost source tree, save for maybe some headers for the map editor code.
 *****************************************************************************/

#ifndef __UTILS_HEADER__
#define __UTILS_HEADER__

#ifdef _WIN32

// even though our game is platform independent, OpenGL on Windows requires
// windows.h to be included, otherwise all sorts of bad things happen
#include <windows.h>

#ifdef _DEBUG

// allow console logging on windows in debug mode
#define getopt(a,b,c)  (EnableDebugging("all"), -1)
#define optarg         (NULL)

#else

// disable console logging on windows in release mode
#define getopt(a,b,c)  (-1)
#define optarg         (NULL)

#endif  //#ifdef _DEBUG

typedef unsigned int uint;

#endif  //#ifdef _WIN32

#include <stdexcept>
#include <stdlib.h>
#include <cstdlib>
#include <cmath>

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <stack>

#include <SDL/SDL.h>

#include <string.h>   // C string manipulation functions like strcmp

#ifdef _WIN32
	#define strcasecmp stricmp  // for some reason, case-insensitive string compare is called
	                            // strcasecmp on linux and stricmp on windows
#endif


/*! \name Allacrost Integer Types.
 *  \brief These are the integer types you should use. Use of int, short, etc. is forbidden!
 *  
 *  These types are just renamed types from SDL, because we don't like their named types.
 */
//@{ 
typedef Sint32  int32;
typedef Uint32  uint32;
typedef Sint16  int16;
typedef Uint16  uint16;
typedef Sint8   int8;
typedef Uint8   uint8;
//@}


//! Contains utility code used across the entire 
namespace hoa_utils {

class UnicodeString;

//! unicode string class- basically like wstring except it doesn't use wchar_t which has bad compatibility
typedef UnicodeString ustring;

//! Determines whether the code in the hoa_utils namespace should print debug statements or not.
extern bool UTILS_DEBUG;

// Constants used for the GaussianValue() function
const bool UTILS_ONLY_POSITIVE = true;
const int32 UTILS_NO_BOUNDS = 0;

/******************************************************************************
	SINGLETON macros - used for turning a class into a singleton class

	The following three macros turn a normal class into a singleton class. To
	create a singleton class, perform the following steps.

	1) The class' header file must #include "utils.h"
	2) Place SINGLETON_DECLARE(class_name) in the class' private section.
	3) Place SINGLETON_METHODS(class_name) in the class' public section.
	4) Place SINGLETON_INITIALIZE(class_name) at the top of the class' source file.
	5) *REQUIRED* Implement the class' constructor and destructor (even if it does nothing).

	After performing these steps, your class with have 3 functions publicly avaiable to it:
		- _Create():			creates the new singleton and returns a pointer to the class object
		- _Destroy():			destroys the singleton class
		- _GetRefCount(): returns a pointer to the class object

	Notes and Usage:

	1) The constructor and destructor *MUST* be defined! If you do not implement
			them, then you will get a compilation error like:
			> In function `SINGLETON::_Create()': undefined reference to `SINGLETON::SINGLETON[in-charge]()

	2) You can not create or delete instances of this class normally. Ie, calling the constructor, copy
			constructor, copy assignment operator, destructor, new/new[], or delete/delete[] operators will
			result in a compilation error. Use the _Create(), _Destroy(), and _GetReference() functions instead.

	3) The only place _Create() and _Destroy() should usually be called are in loader.cpp. But if a different
			section of code detects a fatal error and needs to exit the game, _Destroy() should be called for *all*
			Singletons.

	4) FYI: _Create() and _Destroy() can be called multiple times without any problem. The only time you need
			to worry is if _Destroy() is called and then a part of the code tries to reference a pointer to the old
			singleton. Thus...

			>>> ONLY CALL _Destroy() WHEN YOU ARE EXITING OR ABORTING THE ENTIRE APPLICATION!!! <<<

	5) You can get a class object pointer like this: 'MYCLASS *test = MYCLASS::_GetReference();'
 *****************************************************************************/

// Put in the private sector of the class definition
#define SINGLETON_DECLARE(class_name) \
	static class_name *_ref; \
	class_name(); \
	~class_name(); \
	class_name(const class_name&); \
	class_name& operator=(const class_name&);

// Put in the public sector of the class definition
#define SINGLETON_METHODS(class_name) \
	static class_name* Create() { \
			if (_ref == NULL) { \
				_ref = new class_name(); \
			} \
			return _ref; \
	} \
	static void Destroy() { \
			if (_ref != NULL) { \
				delete _ref; \
				_ref = NULL; \
			} \
	} \
	static class_name* GetReference() { \
			return _ref; \
	} \
	bool Initialize();

// Put in the class' source file
#define SINGLETON_INITIALIZE(class_name) \
	class_name* class_name::_ref = NULL;




/******************************************************************************
 * float UnitRV():
 *
 *	A simple function that returns a random floating point value between [-1, 1).
 *	 Used by the GaussianValue function, but probably won't be used anywhere else.
 ******************************************************************************/
float UnitRV();

/******************************************************************************
 * float RandomUnit():
 *
 *	A simple function that returns a random floating point value between [0.0, 1.0].
 ******************************************************************************/
float RandomUnit();

/******************************************************************************
 * int RandomNum(int lower_bound, int upper_bound):
 *
 *	A simple function that returns a random interger value between [lower_bound, upper_bound].
 *	 These arguments can be postive, negative, equal, or zero. The only problem occurs if lower_bound
 *	 is greater than upper_bound, but in that case the problem is fixed and a warning is printed out
 *	 to the screen.
 ******************************************************************************/
int32 RandomNumber(int32 lower_bound, int32 upper_bound);

/******************************************************************************
 * int GaussianValue(int mean, int range, bool positive_value):
 *
 *	This function computes a random number based on a Gaussian Distribution Curve. This number will be between
 *	 mean - range and mean + range if range is greater than zero, otherwise it will return a true, unbounded
 *	 guassian random value. If positive_value is set to true, this function will only return a number that is
 *	 zero or positive.
 *
 *	Mean is (obviously) the mean, and the range represents the value for 3 standard deviations from the mean.
 *	 That means that 99.7% of the random values chosen will lay between mean - range and mean + range, if
 *	 range is a greater than or equal to zero.
 ******************************************************************************/
int32 GaussianValue(int32 mean, int32 range, bool positive_value);



/******************************************************************************
 * bool CleanDirectory(const std::string &directoryName);
 *
 *	This function erases all the files in a given directory (e.g. "img/fonts")
 ******************************************************************************/

bool CleanDirectory(const std::string &directoryName);


/******************************************************************************
 * bool MakeDirectory(const std::string &directoryName);
 *
 *	This function creates the given directory (e.g. "img/fonts")
 ******************************************************************************/


bool MakeDirectory(const std::string &directoryName);


/******************************************************************************
 * bool RemoveDirectory(const std::string &directoryName);
 *
 *	This function removes the given directory (e.g. "img/fonts")
 ******************************************************************************/

bool RemoveDirectory(const std::string &directoryName);


/******************************************************************************
 * bool MakeWideString(const std::string &text);
 *
 *	This function converts a string to a wide string. All of our GUI is based
 *  on unicode text, so this is used in cases where we want to display some
 *  non-unicode text for debugging/diagnostics purposes.
 ******************************************************************************/

hoa_utils::ustring MakeWideString(const std::string &text);


/******************************************************************************
 * bool MakeByteString(const hoa_utils::ustring &uText);
 *
 *	This function converts a wide string to a string. This is used in some places,
 *  where there is for example a filename in a wide string format which needs
 *  to be converted to a non-wide format since all our resource loading code
 *  is based on std::string
 ******************************************************************************/

std::string MakeByteString(const hoa_utils::ustring &uText);


/******************************************************************************
 * bool IsNumber(const std::string &text);
 *
 *	This function returns true if the text passed to it is a number. For example,
 *  "50", or "2350", or "-235.025" are numbers, whereas "523abc" is not :)
 ******************************************************************************/

bool IsNumber(const std::string &text);


/*!****************************************************************************
 *  \brief UnicodeString lets us use strings with uint16 as the type of character.
 *         Unfortunately there were some libstdc++ compatability problems with
 *         simply defining basic_string<uint16> so this class is basically our
 *         homebrewed version of it. Of course, not all functionality of basic_string
 *         has been implemented, just the stuff that we tend to use in practice
 *****************************************************************************/

class UnicodeString
{
public:

	UnicodeString();
	UnicodeString(const uint16 *);
	
	
	/*!
	 *  \brief clears to empty string
	 */
	void clear();
	
	
	/*!
	 *  \brief returns true if string's empty
	 */	
	bool empty() const;
	

	/*!
	 *  \brief returns length of string
	 */
	size_t length() const;
	
	
	/*!
	 *  \brief returns length of string (synonym for length())
	 */
	size_t size() const;


	/*!
	 *  \brief returns a substring, starting at element pos, and n elements long
	 *  \note  this function will only fail if pos is beyond the range of the string
	 *         In this case, the std::out_of_range exception is thrown
	 */	
	UnicodeString substr(size_t pos=0, size_t n=npos) const;
	

	UnicodeString & operator += (uint16 c);
	UnicodeString & operator += (const UnicodeString &s);	
	UnicodeString & operator = (const UnicodeString &s);
	
	
	/*!
	 *  \brief searches for a character within the string, starting at element pos.
	 *  \return npos if nothing is found, or the starting index of the substring
	 */	
	size_t find(uint16 c, size_t pos=0) const;


	/*!
	 *  \brief searches for a substring within the string, starting at element pos.
	 *  \return npos if nothing is found, or the starting index of the substring
	 */	
	size_t find(const UnicodeString &s, size_t pos=0) const;
	

	/*!
	 *  \brief npos is the largest possible value of size_t, namely -1 (0xFFFFFFFF on most machines)
	 */	
	static const size_t npos;


	/*!
	 *  \brief returns a raw pointer to the string
	 */	
	const uint16 * c_str() const;
	

	uint16 & operator [] (size_t pos);
	const uint16 & operator [] (size_t pos) const;

private:


	//! UnicodeString is just a wrapper for a vector of uint16. Note that we don't even
	//! have a destructor, because vector will destroy itself
	std::vector <uint16> _str;
};


}

#endif
