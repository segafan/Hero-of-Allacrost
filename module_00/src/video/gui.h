/* 
	gui.h
	
 	Hero of Allacrost GUI class code.
*/


#ifndef _GUI_HEADER_
#define _GUI_HEADER_

#include "utils.h"
#include "video.h"
 

namespace hoa_video
{

extern bool VIDEO_DEBUG;


namespace local_video
{

// take several samples of the FPS across frames and then average to get a
// steady FPS display

const int VIDEO_FPS_SAMPLES = 30;


//-----------------------------------------------------------------------------
// MenuSkin: holds information about the visual appearance of menus
//-----------------------------------------------------------------------------

struct MenuSkin
{
	// this 2d array holds the skin for the menu.
	
	// skin[0][0]: bottom left
	// skin[0][1]: bottom
	// skin[0][2]: bottom right
	// skin[1][0]: left
	// skin[1][1]: center (only set skin[1][1].color. Don't load an image)
	// skin[1][2]: right
	// skin[2][0]: upper left
	// skin[2][1]: top
	// skin[2][2]: upper right
	
	hoa_video::ImageDescriptor skin[3][3];
};



//-----------------------------------------------------------------------------
// GUI: main class for all GUI operations
//-----------------------------------------------------------------------------

class GUI
{
public:

	GUI();
	~GUI();
	bool DrawFPS(int frameTime);
	
	bool SetMenuSkin
	(
		const std::string &imgFile_UL,  // image filenames for the borders
		const std::string &imgFile_U,
		const std::string &imgFile_UR,
		const std::string &imgFile_L,
		const std::string &imgFile_R,
		const std::string &imgFile_BL,
		const std::string &imgFile_B,
		const std::string &imgFile_BR,
		
		const Color  &fillColor         // color to fill the menu with. You can
		                                // make it transparent by setting alpha
	);

	// width and height are based on pixels, in 1024x768 resolution		
	bool CreateMenu(ImageDescriptor &id, float width, float height);

private:

	bool CheckSkinConsistency(const MenuSkin &skin);
	MenuSkin _currentSkin;

	hoa_video::GameVideo *_videoManager;
	
	int _fpsSamples[VIDEO_FPS_SAMPLES];
	int _curSample;
	int _numSamples;
};

} // namespace local_video

} // namespace hoa_video

#endif // !_GUI_HEADER_
