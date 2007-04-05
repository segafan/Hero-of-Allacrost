///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    utils.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for Allacrost utility code.
*** ***************************************************************************/

#include "utils.h"

// Headers included for directory manipulation. Windows has its own way of
// dealing with directories, hence the need for conditional includes
#ifdef _WIN32
	#include <direct.h>
#else
	#include <dirent.h>
#endif

#include <sys/stat.h>

using namespace std;

#include "socket.h"

using namespace hoa_socket;

namespace hoa_utils {

bool UTILS_DEBUG = false;

////////////////////////////////////////////////////////////////////////////////
///// Numeric utility functions
////////////////////////////////////////////////////////////////////////////////

uint32 RoundUpPow2(uint32 x) {
	x -= 1;
	x |= x >>  1;
	x |= x >>  2;
	x |= x >>  4;
	x |= x >>  8;
	x |= x >> 16;
	return x + 1;
}



bool IsPowerOfTwo(uint32 x) {
	return ((x & (x-1)) == 0);
}



bool IsOddNumber(uint32 x) {
	#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		return (x & 0x80000000);
	#else // SDL_BYTEORDER == SDL_LITTLE_ENDIAN
		return (x & 0x00000001);
	#endif
}



bool IsFloatInRange(float value, float lower, float upper) {
	return (value >= lower && value <= upper);
}

////////////////////////////////////////////////////////////////////////////////
///// ustring Class
////////////////////////////////////////////////////////////////////////////////

const size_t ustring::npos = ~0;



ustring::ustring() {
	_str.push_back(0);
}



ustring::ustring(const uint16 *s) {
	_str.clear();

	if (!s) {
		_str.push_back(0);
		return;
	}

	while (*s != 0) {
		_str.push_back(*s);
		++s;
	}

	_str.push_back(0);
}


// Return a substring starting at pos, continuing for n elements
ustring ustring::substr(size_t pos, size_t n) const
{
	size_t len = length();

	if (pos >= len)
		throw std::out_of_range("pos passed to substr() was too large");

	ustring s;
	while (n > 0 && pos < len) {
		s += _str[pos];
		++pos;
		--n;
	}

	return s;
}


// Concatenates string to another
ustring & ustring::operator + (const ustring& s)
{
	// nothing to do for empty string
	if (s.empty())
		return *this;

	// add first character of string into the null character spot
	_str[length()] = s[0];

	// add rest of characters afterward
	size_t len = s.length();
	for (size_t j = 1; j < len; ++j) {
		_str.push_back(s[j]);
	}

	// Finish off with a null character
	_str.push_back(0);

	return *this;
}


// Adds a character to end of this string
ustring & ustring::operator += (uint16 c) {
	_str[length()] = c;
	_str.push_back(0);

	return *this;
}


// Concatenate another string on to the end of this string
ustring & ustring::operator += (const ustring &s) {
	// nothing to do for empty string
	if (s.empty())
		return *this;

	// add first character of string into the null character spot
	_str[length()] = s[0];

	// add rest of characters afterward
	size_t len = s.length();
	for (size_t j = 1; j < len; ++j) {
		_str.push_back(s[j]);
	}

	// Finish off with a null character
	_str.push_back(0);

	return *this;
}


// Will assign the current string to this string
ustring & ustring::operator = (const ustring &s) {
	clear();
	operator += (s);

	return *this;
} // ustring & ustring::operator = (const ustring &s)


// Finds a character within a string, starting at pos. If nothing is found, npos is returned
size_t ustring::find(uint16 c, size_t pos) const {
	size_t len = length();

	for (size_t j = pos; j < len; ++j) {
		if (_str[j] == c)
			return j;
	}

	return npos;
} // size_t ustring::find(uint16 c, size_t pos) const


// Finds a string within a string, starting at pos. If nothing is found, npos is returned
size_t ustring::find(const ustring &s, size_t pos) const {
	size_t len = length();
	size_t total_chars = s.length();
	size_t chars_found = 0;

	for (size_t j = pos; j < len; ++j) {
		if (_str[j] == s[chars_found]) {
			++chars_found;
			if (chars_found == total_chars) {
				return (j - chars_found + 1);
			}
		}
		else {
			chars_found = 0;
		}
	}

	return npos;
} // size_t ustring::find(const ustring &s, size_t pos) const

////////////////////////////////////////////////////////////////////////////////
///// string and ustring manipulator functions
////////////////////////////////////////////////////////////////////////////////

// Returns true if the given text is a number
bool IsStringNumeric(const string& text) {
	if (text.empty())
		return false;

	// Keep track of whether decimal point is allowed. It is allowed to be present in the text zero or one times only.
	bool decimal_allowed = true;

	size_t len = text.length();

	// Check each character of the string one at a time
	for (size_t c = 0; c < len; ++c) {
		// The only non numeric characters allowed are a - or + as the first character, and one decimal point anywhere
		bool numeric_char = (isdigit(static_cast<int32>(text[c]))) || (c==0 && (text[c] == '-' || text[c] == '+'));

		if (!numeric_char) {
			// Check if the 'bad' character is a decimal point first before labeling the string invalid
			if (decimal_allowed && text[c] == '.') {
				decimal_allowed = false; // Decimal points are now invalid for the rest of the string
			}
			else {
				return false;
			}
		}
	}

	return true;
} // bool IsStringNumeric(const string& text)


// Creates a ustring from a normal string
ustring MakeUnicodeString(const string& text) {
	int32 length = static_cast<int32>(text.length());
	uint16 *ubuff = new uint16[length+1];
	ubuff[length] = static_cast<uint16>('\0');

	for (int32 c = 0; c < length; ++c) {
		ubuff[c] = static_cast<uint16>(text[c]);
	}

	ustring new_ustr(ubuff);
	delete[] ubuff;

	return new_ustr;
} // ustring MakeUnicodeString(const string& text)


// Creates a normal string from a ustring
string MakeStandardString(const ustring& text) {
	int32 length = static_cast<int32>(text.length());

	unsigned char *strbuff = new unsigned char[length+1];
	strbuff[length] = '\0';

	for (int32 c = 0; c < length; ++c) {
		uint16 curr_char = text[c];

		if(curr_char > 0xff)
			strbuff[c] = '?';
		else
			strbuff[c] = static_cast<unsigned char>(curr_char);
	}

	string new_str(reinterpret_cast<char*>(strbuff));
	delete [] strbuff;

	return new_str;
} // string MakeStandardString(const ustring& text)

////////////////////////////////////////////////////////////////////////////////
///// Random number generator functions
////////////////////////////////////////////////////////////////////////////////

float RandomFloat() {
	return (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
}


// Returns a random integer between two inclusive bounds
int32 RandomBoundedInteger(int32 lower_bound, int32 upper_bound) {
	int32 range;  // The number of possible values we may return
	float result;

	range = upper_bound - lower_bound + 1;
	if (range < 0) { // Oops, someone accidentally switched the lower/upper bound arguments
		if (UTILS_DEBUG) cerr << "UTILS WARNING: Call to RandomNumber had bound arguments swapped." << endl;
		range = range * -1;
	}

	result = range * RandomFloat();
	result = result + lower_bound; // Shift result so that it is within the correct bounds

	return static_cast<int32>(result);
} // int32 RandomBoundedInteger(int32 lower_bound, int32 upper_bound)

// Creates a Gaussian random interger value.
// std_dev and positive_value are optional arguments with default values 10.0f and true respectively
int32 GaussianRandomValue(int32 mean, float std_dev, bool positive_value) {
	float x, y, r;  // x and y are coordinates on the unit circle
	float grv_unit; // Used to hold a Gaussian random variable on a normal distribution curve (mean 0, stand dev 1)
	float result;

	// Make sure that the standard deviation is positive
	if (std_dev < 0) {
		cerr << "UTILS WARNING: negative value for standard deviation argument in function GaussianValue" << endl;
		std_dev = -1.0f * std_dev;
	}

	// Computes a standard Gaussian random number using the the polar form of the Box-Muller transformation.
	// The algorithm computes a random point (x, y) inside the unit circle centered at (0, 0) with radius 1.
	// Then a Gaussian random variable with mean 0 and standard deviation 1 is computed by:
	//
	// x * sqrt(-2.0 * log(r) / r)
	//
	// Reference: Knuth, The Art of Computer Programming, Volume 2, p. 122

	// This loop is executed 4 / pi = 1.273 times on average
	do {
		x = 2.0f * RandomFloat() - 1.0f;     // Get a random x-coordinate [-1.0f, 1.0f]
		y = 2.0f * RandomFloat() - 1.0f;     // Get a random y-coordinate [-1.0f, 1.0f]
		r = x*x + y*y;
	} while (r > 1.0f || r == 0.0f);
	grv_unit = x * sqrt(-2.0f * log(r) / r);

	// Use the standard gaussian value to create a random number with the desired mean and standard deviation.
	result = (grv_unit * std_dev) + mean;

	// Return zero if a negative result was found and only positive values were to be returned
	if (result < 0.0f && positive_value)
		return 0;
	else
		return static_cast<int32>(result);
} // int32 GaussianValue(int32 mean, float std_dev = 6.667f, bool positive_value = false)

// Returns true/false depending on the chance
bool Probability(uint32 chance) {
	uint32 value = static_cast<uint32>(RandomBoundedInteger(1, 100));
	if (value <= chance)
		return true;
	else
		return false;
}

////////////////////////////////////////////////////////////////////////////////
///// Directory manipulation functions
////////////////////////////////////////////////////////////////////////////////

bool MakeDirectory(const std::string& dir_name) {
	// Don't do anything if the directory already exists
	struct stat buf;
	int32 i = stat(dir_name.c_str(), &buf);
	if (i == 0)
		return true;

	// Create the directory with mkdir(). Note that Windows does not require file permissions to be set, but
	// all other operating systems do.

	#ifdef _WIN32
		int32 success = mkdir(dir_name.c_str());
	#else
		int32 success = mkdir(dir_name.c_str(), S_IRWXG | S_IRWXO | S_IRWXU);
	#endif

	if (success == -1) {
		cerr << "UTILS ERROR: could not create directory: " << dir_name.c_str() << endl;
		return false;
	}

	return true;
}



bool CleanDirectory(const std::string& dir_name) {
	// Don't do anything if the directory doesn't exist
	struct stat buf;
	int32 i = stat(dir_name.c_str(), &buf);
	if (i != 0)
		return true;

	#ifdef _WIN32
		//--- WINDOWS --------------------------------------------------------------

		// Get the current directory that the Allacrost application resides in
		char app_path[1024];
		GetCurrentDirectoryA(1024, app_path);

		int32 app_path_len = static_cast<int32>(strlen(app_path));
		if (app_path_len <= 0)
			return false;
		if(app_path[app_path_len-1] == '\\')    // Remove the ending slash if one is there
			app_path[app_path_len-1] = '\0';

		string full_path = app_path;

		if (dir_name[0] == '/' || dir_name[0] == '\\') {
			full_path += dir_name;
		}
		else {
			full_path += "/";
			full_path += dir_name;
		}

		char file_found[1024];
		WIN32_FIND_DATAA info;
		HANDLE hp;
		sprintf(file_found, "%s\\*.*", full_path.c_str());
		hp = FindFirstFileA(file_found, &info);

		if (hp != INVALID_HANDLE_VALUE) {
			// Remove each file from the full_path directory
			do {
				sprintf(file_found, "%s\\%s", full_path.c_str(), info.cFileName);
				DeleteFileA(file_found);
			} while(FindNextFileA(hp, &info));
		}
		FindClose(hp);

	#else
		//--- NOT WINDOWS ----------------------------------------------------------

	DIR *parent_dir;
	struct dirent *dir_file;

	parent_dir = opendir(dir_name.c_str());   // open the directory we want to clean
	if (!parent_dir) {
		cerr << "UTILS ERROR: failed to clean directory: " << dir_name << endl;
		return false;
	}

	string base_dir = dir_name;
	if (base_dir[base_dir.length()-1] != '/')
		base_dir += "/";

	// Remove each file found in the parent directory
	while ((dir_file = readdir(parent_dir))) {
		string file_name = base_dir + dir_file->d_name;
		remove(file_name.c_str());
	}

	closedir(parent_dir);

	#endif

	return true;
}



bool RemoveDirectory(const std::string& dir_name)
{
	// Don't do anything if the directory doesn't exist
	struct stat buf;
	int32 i = stat(dir_name.c_str(), &buf);
	if (i != 0)
		return true;

	// Remove any files that still reside in the directory
	CleanDirectory(dir_name);

	// Finally, remove the folder itself with rmdir()
	int32 success = rmdir(dir_name.c_str());

	if (success == -1) {
		cerr << "UTILS ERROR: could not delete directory: " << dir_name << endl;
		return false;
	}

	return true;
}

#define VERSION_HOST "rabidtinker.mine.nu"
#define VERSION_PATH "/~alistair/allacrost-version.txt"
#define ALLACROST_MAJOR_VERSION 0
#define ALLACROST_MINOR_VERSION 1
#define ALLACROST_PATCH 0

static std::string temp_version_str;

bool IsLatestVersion ()
{
	uint32 rversionmajor;
	uint32 rversionminor;
	uint32 rpatch;
	/*rversion = atof ( system(VERSION_URL) );
	rpatch = atoi ( system(PATCH_URL) );*/
	/*FILE* fp = popen ( "curl -s " VERSION_URL, "r" );
	if (!fp)
		return true;
	fscanf ( fp, "%d.%d.%d", &rversionmajor, &rversionminor, &rpatch );
	pclose ( fp );*/
	Socket conn;
	conn.Connect ( VERSION_HOST, 80 );
	if (!conn.IsConnected()) // could not connect
		return true; // assume latest version
	conn.Write ( "GET http://%s%s\r\n", VERSION_HOST, VERSION_PATH );
	conn.IsQueued ( 300 );
	conn.ScanLine ( "%d.%d.%d", &rversionmajor, &rversionminor, &rpatch );
	conn.Disconnect();

	char vstring[255];
	sprintf ( vstring, "%d.%d.%d", rversionmajor, rversionminor, rpatch );
	temp_version_str = vstring;

	if (rversionmajor > ALLACROST_MAJOR_VERSION)
		return false;
	else if (rversionmajor == ALLACROST_MAJOR_VERSION)
	{
		if (rversionminor > ALLACROST_MINOR_VERSION)
			return false;
		else if (rversionminor == ALLACROST_MINOR_VERSION)
		{
			if (rpatch > ALLACROST_PATCH)
				return false;
		}
	}
	return true;
}

string GetLatestVersion ()
{
	return temp_version_str;
}

} // namespace utils
