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

/*******************************************************************************
 * template<class T> class Singleton: A generic template for making classes singletons
 *	- Written by Kevin Martin
 *
 * Create the singleton initially by 
 *
 * Singleton<type> t;
 *
 * then as long as t remains in scope then any other Singleton<type> created 
 * will act as pointers to the inital one. If the initial one goes out of scope
 * then it will be destroyed and the next one will create a new object. If the
 * main one goes out of scope while another is still in scope then your screwed,
 * but if it does then your using it in a way it isnt inended
 *
 * To make sure a class is a singleton give it private constructor/
 * operator new/etc and use SINGLETON2 to give Singleton<type> permission to
 * construct it.
 *
 * SINGLETON1 is required in an implementation file to set up the static members
 * of Singleton<type>
 *******************************************************************************/
template<class T> class Singleton {
private:
	static T *obj;
	static Singleton<T> *createInst;
public:
	Singleton() {
		if(obj == NULL) {
			obj = new T;
			createInst = this;
		}
	}
	
	~Singleton() {
		if(obj != NULL && createInst == this) {
			delete obj;
			obj=NULL;
			createInst=NULL;
		}
	}

	T &operator*() const {
		return *obj;
	}

	T *operator->() const {
		return obj; 
	}
};


 
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

// #defines used for the Singleton Template class
#define SINGLETON1(type) type *hoa_utils::Singleton<type>::obj=NULL; hoa_utils::Singleton<type> *hoa_utils::Singleton<type>::createInst=NULL;
#define SINGLETON2(type) friend class hoa_utils::Singleton<type>;

#endif 
