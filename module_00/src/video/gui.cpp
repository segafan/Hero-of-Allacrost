#include "utils.h"
#include "gui.h"

using namespace std;


namespace hoa_video
{

namespace local_video
{

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

GUI::GUI()
{
	for(int sample = 0; sample < VIDEO_FPS_SAMPLES; ++sample)
		_fpsSamples[sample] = 0;
		
	_curSample = _numSamples = 0;
	_videoManager = hoa_video::GameVideo::_GetReference();
}


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------

GUI::~GUI()
{
	for(int y = 0; y < 3; ++y)
	{
		for(int x = 0; x < 3; ++x)
		{
			_videoManager->DeleteImage(_currentSkin.skin[y][x]);
		}
	}
}


//-----------------------------------------------------------------------------
// DrawFPS: calculates the FPS based on the time the frame took, and draws it
//          To make the FPS more "steady", the FPS that's displayed on the
//          screen is actually the average over the last VIDEO_FPS_SAMPLES frames.
//-----------------------------------------------------------------------------

bool GUI::DrawFPS(int frameTime)
{
	// calculate the FPS for current frame	
	int fps = 1000;		
	if(frameTime)   
	{
		fps /= frameTime;
	}
	
	// insert newly calculated FPS into fps buffer
	_fpsSamples[_curSample] = fps;
	_curSample = (_curSample+1) % VIDEO_FPS_SAMPLES;
	
	// find the average FPS
	int totalFPS = 0;
	for(int sample = 0; sample < VIDEO_FPS_SAMPLES; ++sample)
	{
		totalFPS += _fpsSamples[sample];
	}
	
	int avgFPS = totalFPS / VIDEO_FPS_SAMPLES;

	// display to screen	
	char fpsText[16];
	sprintf(fpsText, "fps: %d", avgFPS);
	
	if( !_videoManager->SetFont("default"))
		return false;
		
	if( !_videoManager->DrawText(fpsText, 930.0f, 720.0f))
		return false;
		
	return true;
}


//-----------------------------------------------------------------------------
// SetMenuSkin:
//-----------------------------------------------------------------------------

bool GUI::SetMenuSkin
(
	const std::string &imgFile_UL,
	const std::string &imgFile_U,
	const std::string &imgFile_UR,
	const std::string &imgFile_L,
	const std::string &imgFile_R,
	const std::string &imgFile_BL,
	const std::string &imgFile_B,
	const std::string &imgFile_BR,	
	const Color  &fillColor
)
{
	int x, y;
	
	// before we actually load in the new skin, delete the old one
	
	for(y = 0; y < 3; ++y)
	{
		for(x = 0; x < 3; ++x)
		{
			_videoManager->DeleteImage(_currentSkin.skin[y][x]);
			
			// setting width/height to zero means to fall back to the
			// width and height of the images in pixels			
			_currentSkin.skin[y][x].width  = 0.0f;
			_currentSkin.skin[y][x].height = 0.0f;			
		}
	}
	
	// now load the new images
	
	_currentSkin.skin[0][0].filename = imgFile_BL;
	_currentSkin.skin[0][1].filename = imgFile_B;
	_currentSkin.skin[0][2].filename = imgFile_BR;
	_currentSkin.skin[1][0].filename = imgFile_L;
	_currentSkin.skin[1][2].filename = imgFile_R;
	_currentSkin.skin[2][0].filename = imgFile_UL;
	_currentSkin.skin[2][1].filename = imgFile_U;
	_currentSkin.skin[2][2].filename = imgFile_UR;

	// the center doesn't have an image, it's just a colored quad
	_currentSkin.skin[1][1].color    = fillColor;
	
	
	_videoManager->BeginImageLoadBatch();
	for(y = 0; y < 3; ++y)
	{
		for(x = 0; x < 3; ++x)
		{
			_videoManager->LoadImage(_currentSkin.skin[y][x]);
		}
	}
	_videoManager->EndImageLoadBatch();
	
	if(!CheckSkinConsistency(_currentSkin))
	{
		return false;			
	}
	
	return true;
}


//-----------------------------------------------------------------------------
// CheckSkinConsistency: runs some simple checks on a skin to make sure its
//                       images are properly sized. If it finds any errors
//                       it'll return false and output an error message
//-----------------------------------------------------------------------------


bool GUI::CheckSkinConsistency(const MenuSkin &s)
{
	float leftBorderSize   = _currentSkin.skin[1][0].width;
	float rightBorderSize  = _currentSkin.skin[1][2].width;
	float topBorderSize    = _currentSkin.skin[2][1].height;
	float bottomBorderSize = _currentSkin.skin[0][1].height;

	float horizontalBorderSize = leftBorderSize + rightBorderSize;
	float verticalBorderSize   = topBorderSize  + bottomBorderSize;
	
	float topWidth    = _currentSkin.skin[2][1].width;
	float bottomWidth = _currentSkin.skin[0][1].width;
	
	float leftHeight  = _currentSkin.skin[1][0].height;
	float rightHeight = _currentSkin.skin[1][2].height;
	
	
	// check #1: widths of top and bottom borders are equal
	
	if(s.skin[2][1].width != s.skin[0][1].width)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: widths of top and bottom border of skin are mismatched!" << endl;
		return false;
	}
	
	// check #2: heights of left and right borders are equal
	
	if(s.skin[1][0].height != s.skin[1][2].height)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: heights of left and right border of skin are mismatched!" << endl;
		return false;
	}

	// check #3: widths of upper-left, left, and bottom-left border are equal
	
	if( (s.skin[2][0].width != s.skin[1][0].width) ||  // UL, L
	    (s.skin[2][0].width != s.skin[0][0].width))    // UL, BL
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: upper-left, left, and lower-left of menu skin must have equal width!" << endl;
		return false;
	}
	                                           
	// check #4: widths of upper-right, right, and bottom-right border are equal
	
	if( (s.skin[2][2].width != s.skin[1][2].width) ||  // UR, R
	    (s.skin[2][2].width != s.skin[0][2].width))    // UR, BR
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: upper-right, right, and lower-right of menu skin must have equal width!" << endl;
		return false;
	}
	
	// check #5: heights of upper-left, top, and upper-right are equal

	if( (s.skin[2][0].height != s.skin[2][1].height) || // UL, U
	    (s.skin[2][0].height != s.skin[2][2].height))   // UL, UR
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: upper-left, top, and upper-right of menu skin must have equal height!" << endl;
		return false;
	}

	// check #6: heights of lower-left, bottom, and lower-right are equal
	
	if( (s.skin[0][0].height != s.skin[0][1].height) || // LL, L
	    (s.skin[0][0].height != s.skin[0][2].height))   // LL, LR
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: lower-left, bottom, and lower-right of menu skin must have equal height!" << endl;
		return false;
	}

	// phew!
	return true;
}


//-----------------------------------------------------------------------------
// CreateMenu: assuming a skin has already been loaded, this function pieces
//             together the menu borders to form a menu of the given width and
//             height and returns its image descriptor.
//
// NOTE: you may not get exactly the width and height you requested. This
//       function automatically adjusts the dimensions to minimize warping
//
// NOTE: this function assumes that the skin images actually WOULD fit together
//       if you put them next to each other. This should be an OK assumption
//       since we call CheckSkinConsistency() when we set a new skin
//-----------------------------------------------------------------------------

bool GUI::CreateMenu(hoa_video::ImageDescriptor &id, float width, float height)
{
	id.Clear();
	
	// get all the border size information
	
	float leftBorderSize   = _currentSkin.skin[1][0].width;
	float rightBorderSize  = _currentSkin.skin[1][2].width;
	float topBorderSize    = _currentSkin.skin[2][1].height;
	float bottomBorderSize = _currentSkin.skin[0][1].height;

	float horizontalBorderSize = leftBorderSize + rightBorderSize;
	float verticalBorderSize   = topBorderSize  + bottomBorderSize;
	
	float topWidth   = _currentSkin.skin[2][1].width;
	float leftHeight = _currentSkin.skin[1][0].height;
		
	// calculate how many times the top/bottom images have to be tiled
	// to make up the width of the window
	
	float innerWidth = width - horizontalBorderSize;
	
	if(innerWidth < 0.0f)
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: innerWidth was negative in CreateMenu()!" << endl;
		}
		return false;
	}
	
	float innerHeight = height - verticalBorderSize;
	
	if(innerHeight < 0.0f)
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: innerHeight was negative in CreateMenu()!" << endl;
		}
		return false;
	}
	
	// find how many times we have to tile the border images to fit the
	// dimensions given
	
	float numXTiles = innerWidth  / topWidth;
	float numYTiles = innerHeight / leftHeight;
	
	int inumXTiles = int(numXTiles);
	int inumYTiles = int(numYTiles);
	
	// ideally, numTop and friends should all divide evenly into integers,
	// but the person who called this function might have passed in a bogus
	// width and height, so we may have to extend the dimensions a little
	
	float dnumXTiles = numXTiles - inumXTiles;
	float dnumYTiles = numYTiles - inumYTiles;
	
	if(dnumXTiles > 0.001f)
	{
		float widthAdjust = (1.0f - dnumXTiles) * topWidth;
		width += widthAdjust;
		innerWidth += widthAdjust;
		++inumXTiles;		
	}
	
	if(dnumYTiles > 0.001f)
	{
		float heightAdjust = (1.0f - dnumYTiles) * topWidth;
		height += heightAdjust;
		innerHeight += heightAdjust;
		++inumYTiles;
	}
	
	// now we have all the information we need to create the menu!
	
	// first add the corners
	
	float maxX = leftBorderSize + inumXTiles * topWidth;
	float maxY = bottomBorderSize + inumYTiles * leftHeight;
	
	id.AddImage(_currentSkin.skin[0][0], 0.0f, 0.0f);   // lower left	
	id.AddImage(_currentSkin.skin[0][2], maxX,	0.0f); // lower right	            
	id.AddImage(_currentSkin.skin[2][0], 0.0f, maxY);   // upper left	
	id.AddImage(_currentSkin.skin[2][2], maxX, maxY);   // upper right

	// re-create the overlay at the correct width and height
	
	Color &skinColor = _currentSkin.skin[1][1].color;
	_videoManager->DeleteImage(_currentSkin.skin[1][1]);
	
	_currentSkin.skin[1][1].width  = innerWidth;
	_currentSkin.skin[1][1].height = innerHeight;
	_currentSkin.skin[1][1].color  = skinColor;
	_videoManager->LoadImage(_currentSkin.skin[1][1]);
	
	id.AddImage(_currentSkin.skin[1][1], leftBorderSize, bottomBorderSize);

	// iterate from left to right and fill in the horizontal borders
	
	for(int tileX = 0; tileX < inumXTiles; ++tileX)
	{
		id.AddImage(_currentSkin.skin[2][1], leftBorderSize + topWidth * tileX, maxY);
		id.AddImage(_currentSkin.skin[0][1], leftBorderSize + topWidth * tileX, 0.0f);
	}
	
	// iterate from bottom to top and fill in the vertical borders

	for(int tileY = 0; tileY < inumYTiles; ++tileY)
	{
		id.AddImage(_currentSkin.skin[1][0], 0.0f, bottomBorderSize + leftHeight * tileY);
		id.AddImage(_currentSkin.skin[1][2], maxX, bottomBorderSize + leftHeight * tileY);
	}

	return true;
}

}  // namespace local_video
}  // namespace hoa_video