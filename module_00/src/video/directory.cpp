#include "utils.h"
#include "video.h"
#include <sys/stat.h>

// windows has it's own stupid way of dealing with directories, hence the
// need for conditional including

#ifdef _WIN32
#include <direct.h>
#else
#include <dirent.h>
#endif

using namespace std;
using namespace hoa_video;

//! All calls to the video engine are wrapped in this namespace.
namespace hoa_video 
{


//-----------------------------------------------------------------------------
// MakeDirectory
//-----------------------------------------------------------------------------

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
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: could not create directory: " << directoryName.c_str() << endl;
		
		return false;
	}
	
	return true;
}


//-----------------------------------------------------------------------------
// CleanDirectory: deletes all the files in a folder. This function is a bit
//                 ugly since it has to be done differently on Windows than on
//                 other operating systems
//-----------------------------------------------------------------------------

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
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: failed to clean directory: " << directoryName << endl;
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


//-----------------------------------------------------------------------------
// RemoveDirectory
//-----------------------------------------------------------------------------

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
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: could not delete directory: " << directoryName.c_str() << endl;
		
		return false;
	}
	
	return true;
}

} // namespace hoa_video
