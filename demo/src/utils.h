///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************(
*** \file    utils.h
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Header file for Allacrost utility code.
***
*** This code includes various utility functions that are used across different
*** parts of the code base. This file is included in every header file in the
*** Allacrost source tree.
***
*** \note Use the following macros for OS-dependent code.
***   - Windows    #ifdef _WIN32
***   - Mac OS X   #ifdef __MACH__
***   - OpenDarwin #ifdef __MACH__
***   - Linux      #ifdef __linux__
***   - Solaris    #ifdef SOLARIS
***   - BeOS       #ifdef __BEOS__
***
*** \note Use the following macros for compiler-dependent code.
***   - MSVC       #ifdef _MSC_VER
***   - g++        #ifdef __GNUC__
***
*** \note Use the following statements to determine system endianess.
***   - Big endian      if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
***   - Little endian   if (SDL_BYTEORDER == SDL_LITTLE_ENDIAN)
***
*** \note Use the following integer types throughout the entire Allacrost code.
***   - int32
***   - uint32
***   - int16
***   - uint16
***   - int8
***   - uint8
***
*** \note Use the following string types througout the entire Allacrost code.
***   - ustring   Unicode strings, meant only for text to be rendered on the screen.
***   - string    Standard C++ strings, used for all text that is not to be rendered to the screen.
***   - char*     Acceptable, but use strings instead wherever possible.
*** ***************************************************************************/

#ifndef __UTILS_HEADER__
#define __UTILS_HEADER__

#include <stdlib.h>
#include <cstdlib>
#include <cmath>
#include <string.h> // For C string manipulation functions like strcmp

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <stack>
#include <stdexcept>

#ifdef _WIN32
	// Even though Allacrost is platform independent, OpenGL on Windows requires windows.h to be included
	#include <windows.h>
	// Case-insensitive string compare is called stricmp in Windows and strcasecmp everywhere else
	#define strcasecmp stricmp
#endif

#include <SDL/SDL.h>

/** \name Allacrost Integer Types
*** \brief These are the integer types used throughout the Allacrost source code. 
*** These types are created by redefining the SDL types (we do not like SDL's type-naming conventions).
*** Use of the standard int, long, etc. is forbidden in Allacrost source code! Don't attempt to use any
*** 64-bit types either, since a large population of PCs in our target audience are not a 64-bit
*** architecture.
**/
//@{
typedef Sint32   int32;
typedef Uint32   uint32;
typedef Sint16   int16;
typedef Uint16   uint16;
typedef Sint8    int8;
typedef Uint8    uint8;
//@}

//! Contains utility code used across the entire source code
namespace hoa_utils {

//! Determines whether the code in the hoa_utils namespace should print debug statements or not.
extern bool UTILS_DEBUG;

/** \brief Rounds an unsigned integer up to the nearest power of two.
*** \param x The number to round up.
*** \return The nearest power of two rounded up from the argument.
**/
uint32 RoundUpPow2(uint32 x);

/** \brief Determines if an unsigned integer is a power of two or not.
*** \param x The number to examine.
*** \return True if the number if a power of two, false otherwise.
**/
bool IsPowerOfTwo(uint32 x);

/** \brief Determines if an integer is an odd number or not.
*** \param x The unsigned integer to examine.
*** \return True if the number is odd, false if it is not.
*** \note Using a signed integer with this function will yield the same result.
**/
bool IsOddNumber(uint32 x);

/** \brief Determines if a floating point number is within a range of two numbers.
*** \param value The floating point value to compare.
*** \param lower The minimum bound (inclusive).
*** \param upper The maximum bound (inclusive).
*** \return True if the value lies within the two bounds.
*** This function should be used in place of direct comparison of two floating point
*** values. The reason for this is that there are small variations in floating point representation
*** across systems and different rounding schemes, so its best to examine numbers within a reasonably
*** sized range. For example, if you want to detect if a number is 1.0f, try 0.999f and 1.001f for
*** the bound arguments.
**/
bool IsFloatInRange(float value, float lower, float upper);

/** ****************************************************************************
*** \brief Implements unicode strings with uint16 as the character type
***
*** This class functions identically to the std::string class provided in the C++
*** standard library. The critical difference is that each character is 2 bytes
*** (16 bits) wide instead of 1 byte (8 bits) wide so that it may implement the
*** full unicode character set.
***
*** \note This class intentionally ignores the code standard convention for class
*** names because the class objects are to be used as if they were a standard C++ type.
*** 
*** \note This class does not implement a destructor because the only data member
*** (a std::vector) will automatically destroy itself when the no-arg destructor is invoked.
***
*** \note The member functions of this class are not documented because they function
*** in the exact same manner that the C++ string class does.
***
*** \note There are some libstdc++ compatability problems with simply defining
*** basic_string<uint16>, so this class is a custom version of it.
***
*** \note Currently not all functionality of basic_string has been implemented, but
*** instead only the functions that we typically use in Allacrost. If you need a
*** basic_string function available that isn't already implemented in this class,
*** go ahead and add it yourself.
***
*** \note This class does not use wchar_t because it has poor compatibility.
*** ***************************************************************************/
class ustring {
public:
	ustring();
	ustring(const uint16*);

	static const size_t npos;

	void clear()
		{ _str.clear(); _str.push_back(0); }
	bool empty() const
		{ return _str.size() <= 1; }
	size_t length() const
		// We assume that there is always a null terminating character, hence the -1 subtracted from the size
		{ return _str.size() - 1; }
	size_t size() const
		{ return length(); }
	const uint16* c_str() const
		{ return &_str[0]; }

	size_t find(uint16 c, size_t pos = 0) const;
	size_t find(const ustring &s, size_t pos = 0) const;
	ustring substr(size_t pos = 0, size_t n = npos) const;
	
	ustring & operator + (const ustring& s);
	ustring & operator += (uint16 c);
	ustring & operator += (const ustring& s);
	ustring & operator = (const ustring& s);
	uint16 & operator [] (size_t pos)
		{ return _str[pos]; }
	const uint16 & operator [] (size_t pos) const
		{ return _str[pos]; }

private:
	//! \brief The structure containing the unicode string data.
	std::vector<uint16> _str;
}; // class ustring


//! \name String Utility Functions
//@{
/** \brief Converts an integer type into a standard string
*** \param T The integer type to convert to a string
*** \return A std::string containing the parameter in string form
**/
template <typename T>
std::string NumberToString(const T t)
{
	std::ostringstream text("");
	text << static_cast<int32>(t);
	return text.str();
}

/** \brief Determines if a string is a valid numeric string
*** \param text The string to check
*** \return A std::string containing the parameter in string form
***
*** This function will accept strings with + or - characters as the first
*** string element and strings including a single decimal point.
*** Examples of valid numeric strings are: "50", ".2350", "-252.5"
**/
bool IsStringNumeric(const std::string& text);

/** \brief Creates a ustring from a standard string
*** \param text The standard string to create the ustring equivalent for
*** \return A ustring containing the same information as the function parameter
***
*** This function is useful for hard-coding text to be displayed on the screen,
*** as unicode characters are the only characters allowed to be displayed. This
*** function serves primarily for debugging and diagnostic purposes.
**/
hoa_utils::ustring MakeUnicodeString(const std::string& text);

/** \brief Creates an starndard string from a ustring
*** \param text The ustring to create the equivalent standard string for
*** \return A string containing the same information as the ustring
***
*** This function is much less likely to be used as MakeUnicodeString.
*** Standard strings are used for resource loading (of images, sounds, etc.) so
*** this may come in use if a ustring contains file information.
**/
std::string MakeStandardString(const hoa_utils::ustring& text);
//@}

/** ****************************************************************************
*** \name Singleton class creation macros
*** \brief Used for transforming a standard class into a singleton class
***
*** To create a singleton class using these macros, perform the following steps.
***
*** 0) The header file of the class must #include "utils.h" and specify the scope hoa_utils::
*** 1) Place SINGLETON_DECLARE(class_name) in the private section of the class
*** 2) Place SINGLETON_METHODS(class_name) in the public section of the class
*** 3) Place SINGLETON_INITIALIZE(class_name) at the top of the source file of the class.
*** 4) Define the constructor and destructor for the class.
*** 5) Also define the public function bool SingletonInitialize() (from the SINGLETON_METHODS macro).
***
*** After performing these steps, your class with have 4 static functions publicly avaiable:
*** - CLASS* SingletonCreate()         creates a new singleton (if one did not already exist)
***                                    and returns a pointer to the class object
***
*** - bool SingletonInitialize()       initializes the members and data for the created singleton object
***                                    (will return false if the initialization code failed)
***
*** - void SingletonDestroy()          destroys the singleton class (as long as one currently exists)
***
*** - CLASS* SingletonGetReference()   returns a pointer to the class object
***
*** \note Do not use any other method of creating a singleton class in the code.
*** These macros are the one and only way you are allowed to create singleton classes.
***
*** \note The constructor and destructor <b>must</b> be defined. Failure to implement
*** them yields a compilation error that will look similar to the following.
*** `In function `SINGLETON::SingletonCreate()': undefined reference to `SINGLETON::SINGLETON[in-charge]()`
***
*** \note You can not create or delete instances of this class normally. The constructor, copy
*** constructor, copy assignment operator, destructor, new/new[], and delete/delete[] operators are
*** all declared in the private section of the class. Use the SingletonCreate(), SingletonDestroy(),
*** and SingletonGetReference() functions instead.
***
*** \note Usually, you shouldn't need to do much of anything in the class constructor. This is because the
*** public SingletonInitialize() function should handle the true initialization of the class. This function
*** is necessary because some singleton classes rely on the existance of one another to intialize themselves.
*** Thus, most singleton classes are first created with SingletonCreate(), and then only initialized once all
*** other singletons objects exist.
***
*** \note For most singleton classes, SingletonCreate() and SingletonDestroy() should only be called in
*** main.cpp. There may be qualified exceptions to this practice however.
***
*** \note Most, if not all, singleton classes also define a pointer to their singleton object inside the
*** source file of the class. For example, the GameAudio singleton contains the AudioManager class object
*** name inside the hoa_audio namespace. Thus, it is actually very rare that you would ever have to invoke
*** the SingletonCreate(), SingletonDestroy(), SingletonGetReference(), or SingletonInitialize() functions.
*** ***************************************************************************/
//@{
//! Place this macro in the private sector of the class definition
#define SINGLETON_DECLARE(class_name) \
	static class_name *_ref; \
	class_name(); \
	~class_name(); \
	class_name(const class_name&); \
	class_name& operator=(const class_name&);

//! Place this macro in the public sector of the class definition
#define SINGLETON_METHODS(class_name) \
	static class_name* SingletonCreate() { \
			if (_ref == NULL) { \
				_ref = new class_name(); \
			} \
			return _ref; \
	} \
	static void SingletonDestroy() { \
			if (_ref != NULL) { \
				delete _ref; \
				_ref = NULL; \
			} \
	} \
	static class_name* SingletonGetReference() { \
			return _ref; \
	} \
	bool SingletonInitialize();

//! Place this macro in the source file of the class
#define SINGLETON_INITIALIZE(class_name) \
	class_name* class_name::_ref = NULL;
//@}


/** \brief A template function that returns the number of elements in an array
*** \param array The array of elements
*** \return The number of elements in the array
**/
template <typename T, size_t N>
size_t NumberElementsArray(T (&)[N])
	{ return N; }

//! \name Random Variable Genreator Fucntions
//@{
/** \brief Creates a uniformly distributed random floating point number
*** \return A floating-point value between [0.0f, 1.0f]
**/
float RandomFloat();

/** \brief Returns a random interger value uniformally distributed between two inclusive bounds
*** \param lower_bound The lower inclusive bound
*** \param upper_bound The upper inclusive bound
*** \return An integer between [lower_bound, upper_bound]
*** \note If the user specifies a lower bound that is greater than the upper bound, the two bounds
*** are switched.
**/
int32 RandomBoundedInteger(int32 lower_bound, int32 upper_bound);

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
/** \brief Returns a Gaussian random value with specified mean and standard deviation
*** \param mean
*** \param std_dev The standard deviation of the Gaussian function (optional, default is 10.0f)
*** \param positive_value If true the function will not return a negative result (optional, default is true)
*** \return An Gaussian random integer with a mean and standard deviation as specified by the user
**/
int32 GaussianRandomValue(int32 mean, float std_dev = 10.0f, bool positive_value = true);

/******************************************************************************
 * bool Probability(uint32 chance);
 *
 *	This function calculates a random number on a given chance and returns true if the chance occurs.
 *  For example, if calling Probability(25), there is a probability of 25% on returning `true`.
 ******************************************************************************/
/** \brief Returns true/false depending on the chance
*** \param chance Value between 0..100. 0 will always return false and >=100 will always return true.
*** \return True if the chance occurs
**/
bool Probability(uint32 chance);
//@}


//! \name Sorting Functions
//@{
/** \brief Performs an insertion sort on a vector of elements
*** \param swap_vec The vector of elements to be sorted.
***
*** Insertion sort should *only* be used for vectors that are already nearly sorted, or
*** for vectors of size 10 or less. Otherwise this algorithm becomes computationally expensive
*** and you should probably choose another sorting algorithm at that point. A good example of
*** code that uses this algorithm well can be found in map.cpp, which sorts map objects every frame.
*** Because map objects change position slowly, there is usually no change or little relative change
*** in sorting order from the previous pass.
***
*** \note The type of element that is passed should have its > and = operators functionally
*** correct (if T is a class, you must overload these operators). In general, its good
*** practice if you overload all comparison operators for these types.
***
*** \note Do not invoke this function with a vector of pointers to class-type objects, as it
*** will cause a compilation error.
**/
template <typename T>
void InsertionSort(std::vector<T>& swap_vec) {
	int32 i, j;
	T value;
	for (i = 1; i < swap_vec.size(); i++) {
		value = swap_vec[i];
		for (j = i - 1; j >= 0 && swap_vec[j] > value; j--) {
			swap_vec[j+1] = swap_vec[j];
		}
		swap_vec[j+1] = value;
	}
} // void InsertionSort(std::vector<T>& swap_vec)
//@}

//! \name Directory and File Manipulation Functions
//@{
/** \brief Removes all files present in a directory
*** \param dir_name The name of the directory to clean (e.g. "img/screnshots")
*** \return True upon success, false upon failure
**/
bool CleanDirectory(const std::string& dir_name);

/** \brief Creates a directory relative to the path of the running application
*** \param dir_name The name of the directory to create (e.g. "img/screnshots")
*** \return True upon success, false upon failure
**/
bool MakeDirectory(const std::string& dir_name);


/** \brief Deletes a directory, as well as any files the directory may contain
*** \param dir_name The name of the directory to remove (e.g. "img/screnshots")
*** \return True upon success, false upon failure
**/
bool RemoveDirectory(const std::string& dir_name);
//@}

//! \name Version Checking Functions
//@{
/** \brief Checks version against a remote server
*** \return A boolean indicating whether the user is running the latest version
***
*** This function will return true to indicate that the user is using the latest 
*** version OR that an error occured.
**/
bool IsLatestVersion ();

/** \brief Gets newest version
*** \return A string containing the version information of the latest version
***
*** This should only be called after a call to IsLatestVersion()
**/
std::string GetLatestVersion ();
//@}

} // namespace hoa_utils

#endif // __UTILS_HEADER__
