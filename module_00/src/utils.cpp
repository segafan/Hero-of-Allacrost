///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004, 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    utils.cpp
 * \author  Tyler Olsen, roots@allacrost.org
 * \date    Last Updated: August 12th, 2005
 * \brief   Source file for Allacrost utility code.
 *****************************************************************************/

#include "utils.h"


// Includes for directory manipulation. Note, windows has its own way of 
// dealing with directories, hence the need for conditional includes

#ifdef _WIN32
#include <direct.h>
#else
#include <dirent.h>
#endif

#include <sys/stat.h>

using namespace std;

namespace hoa_utils {

bool UTILS_DEBUG = false;
const size_t UnicodeString::npos = -1;


// This will return a floating point number between -1 and 1. Its cute, really.
inline float UnitRV() {
	return 2 * (float) rand()/RAND_MAX - 1;
}



// Returns a floaing point number between 0.0 and 1.0
float RandomUnit() {
	return (static_cast<float>(rand()/RAND_MAX));
}



// This will return a random integer between lower_bound and upper_bound (inclusive)
int32 RandomNumber(int32 lower_bound, int32 upper_bound) {
	int32 range;		// The number of possible values we may return
	float result; // Our result (we cast it when we return)

	range = upper_bound - lower_bound + 1;
	if (range < 0) { // Oops, looks like someone accidentally switched the bound arguments
		if (UTILS_DEBUG) cerr << "UTILS WARNING: Call to RandomNum had bound arguments swapped." << endl;
		range = range * -1;
	}

	result = range * (float) rand()/RAND_MAX; // Compute a random floating point number in our range
	result = result + lower_bound; // Shift our range so that it is within the correct bounds;

	return static_cast<int32>(result);
}



// This returns a Gaussian random value in the range: [mean - range, mean + range]
int32 GaussianValue(int32 mean, int32 range, bool positive_value) {
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
		x = 2.0f * UnitRV() - 1.0f;            // Get the X-coordinate
		y = 2.0f * UnitRV() - 1.0f;            // Get the Y-coordinate
		r = x*x + y*y;                       // Compute the radius
	} while (r > 1.0f || r == 0.0f);         // Loop is executed 4 / pi = 1.273 times on average
	z_value = x * sqrt(-2.0f * log(r) / r); // Get the Gaussian random value with mean 0 and standard devation 1

	// Compute a Gaussian value using our own mean and standard deviation
	//result = floatnearbyintf((std_dev * z_value) + mean); this fn does rounding, but I can't get it to compile...
	result = (std_dev * z_value) + mean;

	// Reverses sign of result if we don't want a negative value returned
	if (positive_value && result < 0.0f)
		result = result * -1;

	// If we have a zero or negative range argument, we don't apply bounds to the value returned.
	if (range <= 0)
		return (int32)result;

	if (result < mean - range)      // Covers the case that we exceeded our lower bound (occurs 0.015% of the time)
		result = float( mean - range );
	else if (result > mean + range) // Covers the case that we exceeded our upper bound (occurs 0.015% of the time)
		result = float( mean + range );

	// Note: because we cast rather than round the mean + range value isn't chosen as often as mean - range
	return (int32)result;	// Cast to an int32 and return
}


// This creates a directory of the given path (e.g. img/fonts)
bool MakeDirectory(const std::string &directoryName)
{
	// don't do anything if folder already exists
	struct stat buf;
	int32 i = stat(directoryName.c_str(), &buf);	
	if(i==0)	
		return true;

	// if not then create it with mkdir(). Note that linux requires
	// file permissions to be set but windows doesn't

#ifdef _WIN32
	int32 success = mkdir(directoryName.c_str());
#else
	int32 success = mkdir(directoryName.c_str(), S_IRWXG | S_IRWXO | S_IRWXU);
#endif
	
	if(success == -1)
	{
		if(UTILS_DEBUG)
			cerr << "UTILS ERROR: could not create directory: " << directoryName.c_str() << endl;
		
		return false;
	}
	
	return true;
}


// this removes all the files in the given directory
bool CleanDirectory(const std::string &directoryName)
{
	// don't do anything if folder doesn't exist
	struct stat buf;
	int32 i = stat(directoryName.c_str(), &buf);	
	if(i!=0)	
		return true;

#ifdef _WIN32

//--- WINDOWS -------------------------------------------------------

	// get the directory of the application	
	char appPath[1024];
	GetCurrentDirectory(1024, appPath);	
	int32 appPathLen = (int32)strlen(appPath);	
	if(appPathLen <= 0)
		return false;	
	if(appPath[appPathLen-1] == '\\')    // cut off ending slash if it's there
		appPath[appPathLen-1] = '\0';
		
	string fullPath = appPath;
	
	if(directoryName[0] == '/' || directoryName[0] == '\\')
	{
		fullPath += directoryName;
	}
	else
	{
		fullPath += "/";
		fullPath += directoryName;
	}
	
	char fileFound[1024];
	WIN32_FIND_DATA info;
	HANDLE hp;
	sprintf(fileFound, "%s\\*.*", fullPath.c_str());
	hp = FindFirstFile(fileFound, &info);
	
	if(hp != INVALID_HANDLE_VALUE)
	{
		do
		{
			sprintf(fileFound, "%s\\%s", fullPath.c_str(), info.cFileName);			
			DeleteFile(fileFound);
		} while(FindNextFile(hp, &info));
	}	
	FindClose(hp);

#else

//--- LINUX / MACOS -------------------------------------------------------
	
	DIR *pDir;
	struct dirent *pEnt;
	
	pDir = opendir(directoryName.c_str());   // open the directory we want to clean
	if(!pDir)
	{
		if(UTILS_DEBUG)
			cerr << "UTILS ERROR: failed to clean directory: " << directoryName << endl;
		return false;
	}

	string baseDir = directoryName;
	if(baseDir[baseDir.length()-1] != '/')
		baseDir += "/";
	
	while((pEnt=readdir(pDir)))
	{
		string removedFile = baseDir + pEnt->d_name;
		remove(removedFile.c_str());
	}
	
	closedir(pDir);
	
#endif
	
	return true;
}


// this removes the given directory
bool RemoveDirectory(const std::string &directoryName)
{
	// don't do anything if folder doesn't exist
	struct stat buf;
	int32 i = stat(directoryName.c_str(), &buf);	
	if(i!=0)	
		return true;

	// if the folder is still there, make sure it doesn't have any files in it
	CleanDirectory(directoryName);
 
	// finally, remove the folder itself with rmdir()
	int32 success = rmdir(directoryName.c_str());
	
	if(success == -1)
	{
		if(UTILS_DEBUG)
			cerr << "UTILS ERROR: could not delete directory: " << directoryName.c_str() << endl;
		
		return false;
	}
	
	return true;
}


// converts a string to a wide string
ustring MakeWideString(const string &text)
{
	int32 length = (int32) text.length();
	uint16 *ustr = new uint16[length+1];
	ustr[length] = uint16('\0');
	
	for(int32 c = 0; c < length; ++c)
	{
		ustr[c] = uint16(text[c]);
	}
	
	ustring wstr(ustr);
	delete [] ustr;
	
	return wstr;
}


// converts a wide string to a string
string MakeByteString(const ustring &uText)
{
	int32 length = (int32) uText.length();
	
	unsigned char *str = new unsigned char[length+1];
	str[length] = '\0';
	
	for(int32 c = 0; c < length; ++c)
	{
		uint16 curChar = uText[c];
		
		if(curChar > 0xff)
			str[c] = '?';
		else
			str[c] = static_cast<unsigned char> (curChar);
	}
	
	string byteString(reinterpret_cast<char *>(str));
	delete [] str;
	
	return byteString;
}


// returns true if the given text is a number
bool IsNumber(const std::string &text)
{
	if(text.empty())
		return false;
				
	// keep track of whether decimal point is allowed. Basically it's allowed at any point
	// except once it's been used once it can't be used again
	bool isDecimalAllowed = true;
	
	size_t len = text.length();
	
	for(size_t c = 0; c < len; ++c)
	{
		// if the character is not a valid minus or plus sign, and it's not a
		// digit, and it's not a valid decimal point, then this string isn't a number
		
		bool isNumeric = (c==0 && (text[c] == '-' || text[c] == '+')) ||
		                 (isdigit(int32(text[c]))) ||
		                 (isDecimalAllowed && text[c] == '.');
		
		if(!isNumeric)
			return false;
	}
	
	return true;
}


UnicodeString::UnicodeString()
{
	_str.push_back(0);
}


UnicodeString::UnicodeString(const uint16 *s)
{
	clear();
	
	if(!s)
	{
		_str.push_back(0);
		return;
	}
	
	while(*s != 0)
	{
		_str.push_back(*s);
		++s;
	}
	
	_str.push_back(0);
}
	
	
// clear to empty string
void UnicodeString::clear()
{
	_str.clear();
}


// return true if string is empty
bool UnicodeString::empty() const
{
	return _str.empty();
}
	
	
// length of string
size_t UnicodeString::length() const
{
	// note the -1, because we assume that there is always a null terminating character
	return _str.size() - 1;
}


// length of string
size_t UnicodeString::size() const
{
	return length();
}
	

// return substring starting at pos, and continuing for n elements
UnicodeString UnicodeString::substr(size_t pos, size_t n) const
{
	size_t len = length();
	
	if(pos >= len)
		throw std::out_of_range("pos passed to substr() was too large");
		
	UnicodeString s;
	while(n > 0 && pos < len)
	{
		s += _str[pos];
		++pos;
		--n;
	}
	
	return s;
}


// add a character to end of string	
UnicodeString & UnicodeString::operator += (uint16 c)
{
	_str.push_back(c);
	
	return *this;
}


// concatenate another string onto this one
UnicodeString & UnicodeString::operator += (const UnicodeString &s)	
{
	size_t len = s.length();
	for(size_t j = 0; j < len; ++j)
	{
		_str.push_back(s[j]);
	}
	
	return *this;
}


// assign this string to another one
UnicodeString & UnicodeString::operator = (const UnicodeString &s)
{
	clear();
	
	operator += (s);
	operator += (0);
	
	return *this;
}


// finds a character within a string, starting at pos. If nothing found, returns npos
size_t UnicodeString::find(uint16 c, size_t pos) const
{
	size_t len = length();
	
	for(size_t j = pos; j < len; ++j)
	{
		if(_str[j] == c)
			return j;
	}
	
	return npos;
}


// finds a string within a string, starting at pos. If nothing found, returns npos
size_t UnicodeString::find(const UnicodeString &s, size_t pos) const
{
	size_t len = length();
	size_t total_chars = s.length();
	size_t chars_found = 0;
	
	for(size_t j = pos; j < len; ++j)
	{
		if(_str[j] == s[chars_found])
		{
			++chars_found;
			if(chars_found == total_chars)
			{
				return j - chars_found + 1;
			}
		}
		else
		{
			chars_found = 0;
		}
	}
	
	return npos;
}
	

// returns raw pointer to string
const uint16 * UnicodeString::c_str() const
{
	return &_str[0];
}

	
uint16 & UnicodeString::operator [] (size_t pos)
{
	return _str[pos];
}

const uint16 & UnicodeString::operator [] (size_t pos) const
{
	return _str[pos];
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
