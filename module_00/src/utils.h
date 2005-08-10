/* 
 * hoa_utils.h
 *	Header file for the Hero of Allacrost utility functions
 *	(C) 2004 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */

#ifndef __UTILS_HEADER__
#define __UTILS_HEADER__

#ifdef _WIN32

// even though our game is platform independent, OpenGL on Windows requires
// windows.h to be included, otherwise all sorts of bad things happen
#include <windows.h>

// the following defines basically disable command processing on Windows,
// because there's no getopt() function. Some time in the future, I (Raj)
// will fix this by using Cygwin or MingW or whatever...

#ifdef _DEBUG

#define getopt(a,b,c)  (EnableDebugging("all"), -1)
#define optarg         (NULL)


#else

#define getopt(a,b,c)  (-1)
#define optarg         (NULL)

#endif


#endif

#include <stdlib.h>
#include <iostream>
#include <cstdlib>
#include <cmath> 

#include <vector>
#include <string>
#include <map>
#include <stack>
#include <list>


#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <GL/gl.h>
#include <GL/glu.h>

#define ILUT_USE_OPENGL

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>


#ifdef _WIN32
typedef Uint32  uint;  // linux GCC has uint, but not all compilers
#endif

namespace hoa_utils {

extern bool UTILS_DEBUG;

// Constants used for the GaussianValue() function
const bool UTILS_ONLY_POSITIVE = true;
const int UTILS_NO_BOUNDS = 0;

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
	static class_name* _Create() { \
			if (_ref == NULL) { \
				_ref = new class_name(); \
			} \
			return _ref; \
	} \
	static void _Destroy() { \
			if (_ref != NULL) { \
				delete _ref; \
				_ref = NULL; \
			} \
	} \
	static class_name* _GetReference() { \
			return _ref; \
	} \

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
int RandomNum(int lower_bound, int upper_bound);

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
int GaussianValue(int mean, int range, bool positive_value);

}

#endif 
