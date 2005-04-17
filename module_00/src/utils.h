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

#include <stdlib.h>
#include <iostream>
#include <cstdlib>
#include <cmath> 

namespace hoa_utils {

const bool UTILS_DEBUG = false;

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
	5) Replace 'class_name' in the above 3 with the name of the class you are writing.
	5) *REQUIRED* Implement the constructor and destructor in the class' source file.

	After performing these steps, your class with have 3 functions publicly avaiable to it:
		- _Create():			returns a pointer to the class object
		- _Destory():			removes a reference to the class object
		- _GetRefCount(): returns the current number of references to this class. Used for
											debugging purposes only

	Notes and Usage:

	1) The constructor and destructor *MUST* be defined! If you do not implement
			them, then you will get a compilation error like:
			> In function `SINGLETON::_Create()': undefined reference to `SINGLETON::SINGLETON[in-charge]()

	2) You can not create or delete instances of this class normally. Ie, calling the constructor, copy
			constructor, copy assignment operator, destructor, new/new[], or delete/delete[] operators will
			result in a compilation error. Use the _Create() and _Destroy() functions instead.

	3) Be *VERY* careful with the _Create() and _Destroy() functions. Every _Create() call needs a
			_Destroy() call, otherwise there will be memory leaks.

	4) You can get a class object pointer like this: 'MYCLASS *test = MYCLASS::_Create();'

	5) After you are finished, invoke the following: 'test->_Delete();' *or* 'MYCLASS::_Delete();'
			Either was is fine. Make sure to set 'test = NULL;' so you don't accidentally reference it
			again (that would be dangerous...)
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
			if (_ref != 0) { \
				delete _ref; \
				_ref = NULL; \
			} \
	} \
	static class_name* _GetReference() { \
			return _ref; \
	}

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
