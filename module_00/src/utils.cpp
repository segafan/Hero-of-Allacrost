/* 
 * hoa_utils.cpp
 *	General Purpose Utility functions for Hero of Allacrost
 *	(C) 2004 by Tyler Olsen
 *
 *	This code is licensed under the GNU GPL. It is free software and you may modify it 
 *	 and/or redistribute it under the terms of this license. See http://www.gnu.org/copyleft/gpl.html
 *	 for details.
 */

#include "utils.h"

using namespace std;

namespace hoa_utils {

// This will return a floating point number between -1 and 1. Its cute, really.
float UnitRV() {
	return 2 * (float) rand()/RAND_MAX - 1;
}	 



// This will return a random integer between lower_bound and upper_bound (inclusive)
int RandomNum(int lower_bound, int upper_bound) {
	int range;		// The number of possible values we may return
	float result; // Our result (we cast it when we return)
	
	range = upper_bound - lower_bound + 1;
	if (range < 0) { // Oops, looks like someone accidentally switched the bound arguments
		if (UTILS_DEBUG) cerr << "WARNING: Call to RandomNum had bound arguments swapped." << endl;
		range = range * -1;
	}
	
	result = range * (float) rand()/RAND_MAX; // Compute a random floating point number in our range
	result = result + lower_bound; // Shift our range so that it is within the correct bounds;
	
	return (int)result;
}



// This returns a Gaussian random value in the range: [mean - range, mean + range]
int GaussianValue(int mean, int range, bool positive_value) {
	float x, y, r; // x and y are points on the unit circle. r is the radius
	float z_value; // This is a Gaussian random variable on a normal distribution curve (mean 0, stand dev 1)
	float result;	// This is the resulting Gaussian random variable we want to return
	float std_dev; // This is the standard deviation, equal to 1/3 of range

	if (range <= 0)		 // Use the default range of +/- 20% of the mean (= 6.667 standard deviation)
		std_dev = (float) (mean * 0.20 / 3);
	else								// Set standard deviation equal to one third of the range
		std_dev = (float) (range / 3);
	
	if (std_dev < 0)		// Make sure that we have a positive standard deviation
		std_dev = -1 * std_dev;
	
	// Computes a standard Gaussian random deviate using the polar method. The polar method is to 
	//	compute a random point (x, y) inside the unit circle centered at (0, 0) with radius 1. Then
	//	
	//	 x * Math.sqrt(-2.0 * Math.log(r) / r)
	//	 
	//	is a Gaussian random variable with mean 0 and standard deviation 1. 
	//	 Reference: Knuth, The Art of Computer Programming, Volume 2, p. 122
	do {
		x = 2.0 * UnitRV() - 1.0;						// Get the X-coordinate
		y = 2.0 * UnitRV() - 1.0;						// Get the Y-coordinate
		r = x*x + y*y;											 // Compute the radius
	} while (r > 1.0 || r == 0.0);				 // Loop is executed 4 / pi = 1.273 times on average	
	z_value = x * sqrt(-2.0 * log(r) / r); // Get the Gaussian random value with mean 0 and standard devation 1
	
	// Compute a Gaussian value using our own mean and standard deviation
	//result = floatnearbyintf((std_dev * z_value) + mean); this fn does rounding, but I can't get it to compile...
	result = (std_dev * z_value) + mean;
	
	// Reverses sign of result if we don't want a negative value returned
	if (positive_value && result < 0.0)
		result = result * -1;
	
	// If we have a zero or negative range argument, we don't apply bounds to the value returned.
	if (range <= 0) 
		return (int)result;
	
	if (result < mean - range)			// Covers the case that we exceeded our lower bound (occurs 0.015% of the time)
		result = mean - range;
	else if (result > mean + range) // Covers the case that we exceeded our upper bound (occurs 0.015% of the time)
		result = mean + range;

	// Note: because we cast rather than round the mean + range value isn't chosen as often as mean - range
	return (int)result;	// Cast to an int and return
}

} // namespace utils



// For those interested in seeing the results of some sample runs. Don't forget to #include <ctime>
/*int main() {
	int tmp;
	int v15 = 0, v16 = 0, v17 = 0, v18 = 0, v19 = 0, v20 = 0, v21 = 0, v22 = 0, v23 = 0;
	int v24 = 0, v25 = 0, v26 = 0, v27 = 0, v28 = 0, v29 = 0, v30 = 0, v31 = 0, v32 = 0, v33 = 0, v34 = 0, v35 = 0;
	
	srand(time(NULL));
	for (int i = 0; i < 100000; i++) {
		tmp = GaussianValue(25, 10, false);
		switch (tmp) {
			case 15: v15++; break;
			case 16: v16++; break;
			case 17: v17++; break;
			case 18: v18++; break;
			case 19: v19++; break;
			case 20: v20++; break;
			case 21: v21++; break;
			case 22: v22++; break;
			case 23: v23++; break;
			case 24: v24++; break;
			case 25: v25++; break;
			case 26: v26++; break;
			case 27: v27++; break;
			case 28: v28++; break;
			case 29: v29++; break;
			case 30: v30++; break;
			case 31: v31++; break;
			case 32: v32++; break;
			case 33: v33++; break;
			case 34: v34++; break;
			case 35: v35++; break;
			default: cout << "ERROR: Exceeded range on trial " << i << " value " << tmp << endl;
		}
	}
	
	cout << "RESULTS FOR GAUSSIAN DISTRIBUTION:" << endl;
	cout << v15 << ' ' << v16 << ' ' << v17 << ' ' << v18 << ' ' << v19 << ' ' << v20 << ' ' << v21 << ' ';
	cout << v22 << ' ' << v23 << ' ' << v24 << '*' << v25 << '*' << v26 << ' ' << v27 << ' ' << v28 << ' ';
	cout << v29 << ' ' << v30 << ' ' << v31 << ' ' << v32 << ' ' << v33 << ' ' << v34 << ' ' << v35 << endl;
	
	int lower;
	int upper;
	
	v15 = 0; v16 = 0; v17 = 0; v18 = 0; v19 = 0; v20 = 0; v21 = 0; v22 = 0; v23 = 0; v24 = 0; v25 = 0; 
	v26 = 0; v27 = 0; v28 = 0; v29 = 0; v30 = 0; v31 = 0; v32 = 0; v33 = 0; v34 = 0; v35 = 0;
	lower = 15;
	upper = 35; 
	for (int i = 0; i < 100000; i++) {
		tmp = RandomNum(lower, upper);
		switch (tmp) {
			case 15: v15++; break;
			case 16: v16++; break;
			case 17: v17++; break;
			case 18: v18++; break;
			case 19: v19++; break;
			case 20: v20++; break;
			case 21: v21++; break;
			case 22: v22++; break;
			case 23: v23++; break;
			case 24: v24++; break;
			case 25: v25++; break;
			case 26: v26++; break;
			case 27: v27++; break;
			case 28: v28++; break;
			case 29: v29++; break;
			case 30: v30++; break;
			case 31: v31++; break;
			case 32: v32++; break;
			case 33: v33++; break;
			case 34: v34++; break;
			case 35: v35++; break;
			default: cout << "ERROR: Exceeded range on trial " << i << " value " << tmp << endl;
		}
	}

	cout << "RESULTS FOR RANDOM NUMBER RANGE 15-35:" << endl;
	cout << v15 << ' ' << v16 << ' ' << v17 << ' ' << v18 << ' ' << v19 << ' ' << v20 << ' ' << v21 << ' ';
	cout << v22 << ' ' << v23 << ' ' << v24 << '*' << v25 << '*' << v26 << ' ' << v27 << ' ' << v28 << ' ';
	cout << v29 << ' ' << v30 << ' ' << v31 << ' ' << v32 << ' ' << v33 << ' ' << v34 << ' ' << v35 << endl;
	return 0;
}*/
