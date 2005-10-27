#include "utils.h"
#include "gui.h"

using namespace std;


namespace hoa_video
{


namespace private_video
{

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

GUI::GUI()
{
	for(int32 sample = 0; sample < VIDEO_FPS_SAMPLES; ++sample)
		_fpsSamples[sample] = 0;
		
	_totalFPS      = 0;
	
	_curSample = _numSamples = 0;
	_videoManager = hoa_video::GameVideo::GetReference();
	
	if(!_videoManager)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: in GUI constructor, got NULL for GameVideo reference!" << endl;
	}
}


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------

GUI::~GUI()
{
	for(int32 y = 0; y < 3; ++y)
	{
		for(int32 x = 0; x < 3; ++x)
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

bool GUI::DrawFPS(int32 frameTime)
{
	_videoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_X_NOFLIP, VIDEO_Y_NOFLIP, VIDEO_BLEND, 0);
	
	// calculate the FPS for current frame	
	int32 fps = 1000;		
	if(frameTime)   
	{
		fps /= frameTime;
	}	
	
	
	int32 nIter;
	
	if(_numSamples == 0)
	{
		// If the FPS display is uninitialized, set the entire FPS array to the
		// current FPS
		
		_numSamples = nIter = VIDEO_FPS_SAMPLES;
	}
	else if(frameTime < 2)
		nIter = 1;    // if the game is going at 500 fps or faster, 1 iteration is enough
	else
	{
		// find if there's a discrepancy between the current frame time and the
		// averaged one. If there's a large one, it's likely that we just switched
		// modes (e.g. going from boot screen to map), so add extra samples so the
		// FPS display "catches up" more quickly
		
		float avgFrameTime = 1000.0f * VIDEO_FPS_SAMPLES / _totalFPS;
		int32 diffTime = ((int32)avgFrameTime - frameTime);
		
		if(diffTime < 0)
			diffTime = -diffTime;
		
		if(diffTime <= VIDEO_MAX_FTIME_DIFF)
			nIter = 1;
		else
			nIter = VIDEO_FPS_CATCHUP;  // catch up faster
	}
		
	
	for(int32 j = 0; j < nIter; ++j)
	{	
		// insert newly calculated FPS into fps buffer
		
		if(_curSample < 0 || _curSample >= VIDEO_FPS_SAMPLES)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: out of bounds _curSample in DrawFPS()!" << endl;
			return false;
		}		
		
		_totalFPS -= _fpsSamples[_curSample];
		_totalFPS += fps;		
		_fpsSamples[_curSample] = fps;
		_curSample = (_curSample+1) % VIDEO_FPS_SAMPLES;
	}		
		
	// find the average FPS

	int32 avgFPS = _totalFPS / VIDEO_FPS_SAMPLES;

	// display to screen	 
	char fpsText[16];
	sprintf(fpsText, "fps: %d", avgFPS);
	
	if( !_videoManager->SetFont("debug_font"))
	{
		return false;
	}
	
	_videoManager->Move(930.0f, 720.0f);
	if( !_videoManager->DrawText(fpsText))
	{
		return false;
	}		
	
	return true; 
}


//-----------------------------------------------------------------------------
// SetMenuSkin:
//-----------------------------------------------------------------------------

bool GUI::SetMenuSkin
(
	const std::string &imgFile_TL,
	const std::string &imgFile_T,
	const std::string &imgFile_TR,	
	const std::string &imgFile_L,
	const std::string &imgFile_R,
	const std::string &imgFile_BL,
	const std::string &imgFile_B,
	const std::string &imgFile_BR,
	const std::string &imgFile_Tri_T,
	const std::string &imgFile_Tri_L,
	const std::string &imgFile_Tri_R,
	const std::string &imgFile_Tri_B,
	const std::string &imgFile_Quad,		
	const Color &fillColor_TL,
	const Color &fillColor_TR,
	const Color &fillColor_BL,
	const Color &fillColor_BR

)
{
	int32 x, y;
	
	// before we actually load in the new skin, delete the old one
	
	for(y = 0; y < 3; ++y)
	{
		for(x = 0; x < 3; ++x)
		{
			_videoManager->DeleteImage(_currentSkin.skin[y][x]);
			
			// setting width/height to zero means to fall back to the
			// width and height of the images in pixels			
			_currentSkin.skin[y][x].SetDimensions(0.0f, 0.0f);
		}
	}
	
	_videoManager->DeleteImage(_currentSkin.tri_t);
	_videoManager->DeleteImage(_currentSkin.tri_l);
	_videoManager->DeleteImage(_currentSkin.tri_r);
	_videoManager->DeleteImage(_currentSkin.tri_b);
	_videoManager->DeleteImage(_currentSkin.quad);
	_currentSkin.tri_t.SetDimensions(0.0f, 0.0f);
	_currentSkin.tri_l.SetDimensions(0.0f, 0.0f);
	_currentSkin.tri_r.SetDimensions(0.0f, 0.0f);
	_currentSkin.tri_b.SetDimensions(0.0f, 0.0f);
	_currentSkin.quad.SetDimensions(0.0f, 0.0f);


	map<int32, MenuWindow *>::iterator iMenu = MenuWindow::_menuMap.begin();
	
	while(iMenu != MenuWindow::_menuMap.end())
	{
		MenuWindow *menu = iMenu->second;
		_videoManager->DeleteImage(menu->_menuImage);
		++iMenu;
	}
	
	// now load the new images
	
	_currentSkin.skin[0][0].SetFilename(imgFile_TL);
	_currentSkin.skin[0][1].SetFilename(imgFile_T);
	_currentSkin.skin[0][2].SetFilename(imgFile_TR);
	_currentSkin.skin[1][0].SetFilename(imgFile_L);
	_currentSkin.skin[1][2].SetFilename(imgFile_R);
	_currentSkin.skin[2][0].SetFilename(imgFile_BL);
	_currentSkin.skin[2][1].SetFilename(imgFile_B);
	_currentSkin.skin[2][2].SetFilename(imgFile_BR);

	_currentSkin.tri_t.SetFilename(imgFile_Tri_T);
	_currentSkin.tri_l.SetFilename(imgFile_Tri_L);
	_currentSkin.tri_r.SetFilename(imgFile_Tri_R);
	_currentSkin.tri_b.SetFilename(imgFile_Tri_B);
	_currentSkin.quad.SetFilename(imgFile_Quad);

	// the center doesn't have an image, it's just a colored quad
	_currentSkin.skin[1][1].SetVertexColors
	(
		fillColor_TL,
		fillColor_TR,
		fillColor_BL,
		fillColor_BR
	);
		
	_videoManager->BeginImageLoadBatch();
	for(y = 0; y < 3; ++y)
	{
		for(x = 0; x < 3; ++x)
		{
			_videoManager->LoadImage(_currentSkin.skin[y][x]);
		}
	}

	_videoManager->LoadImage(_currentSkin.tri_t);
	_videoManager->LoadImage(_currentSkin.tri_l);
	_videoManager->LoadImage(_currentSkin.tri_r);
	_videoManager->LoadImage(_currentSkin.tri_b);
	_videoManager->LoadImage(_currentSkin.quad);

	_videoManager->EndImageLoadBatch();
	
	if(!_CheckSkinConsistency(_currentSkin))
	{
		return false;			
	}

	iMenu = MenuWindow::_menuMap.begin();
	
	while(iMenu != MenuWindow::_menuMap.end())
	{
		MenuWindow *menu = iMenu->second;
		menu->RecreateImage();
		++iMenu;
	}

	
	return true;
}


//-----------------------------------------------------------------------------
// _CheckSkinConsistency: runs some simple checks on a skin to make sure its
//                        images are properly sized. If it finds any errors
//                        it'll return false and output an error message
//-----------------------------------------------------------------------------


bool GUI::_CheckSkinConsistency(const MenuSkin &s)
{
	// check #1: widths of top and bottom borders are equal
	
	if(s.skin[2][1].GetWidth() != s.skin[0][1].GetWidth())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: widths of top and bottom border of skin are mismatched!" << endl;
		return false;
	}
	
	// check #2: heights of left and right borders are equal
	
	if(s.skin[1][0].GetHeight() != s.skin[1][2].GetHeight())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: heights of left and right border of skin are mismatched!" << endl;
		return false;
	}

	// check #3: widths of upper-left, left, and bottom-left border are equal
	
	if( (s.skin[2][0].GetWidth() != s.skin[1][0].GetWidth()) ||  // UL, L
	    (s.skin[2][0].GetWidth() != s.skin[0][0].GetWidth()))    // UL, BL
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: upper-left, left, and lower-left of menu skin must have equal width!" << endl;
		return false;
	}
	                                           
	// check #4: widths of upper-right, right, and bottom-right border are equal
	
	if( (s.skin[2][2].GetWidth() != s.skin[1][2].GetWidth()) ||  // UR, R
	    (s.skin[2][2].GetWidth() != s.skin[0][2].GetWidth()))    // UR, BR
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: upper-right, right, and lower-right of menu skin must have equal width!" << endl;
		return false;
	}
	
	// check #5: heights of upper-left, top, and upper-right are equal

	if( (s.skin[2][0].GetHeight() != s.skin[2][1].GetHeight()) || // UL, U
	    (s.skin[2][0].GetHeight() != s.skin[2][2].GetHeight()))   // UL, UR
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: upper-left, top, and upper-right of menu skin must have equal height!" << endl;
		return false;
	}

	// check #6: heights of lower-left, bottom, and lower-right are equal
	
	if( (s.skin[0][0].GetHeight() != s.skin[0][1].GetHeight()) || // LL, L
	    (s.skin[0][0].GetHeight() != s.skin[0][2].GetHeight()))   // LL, LR
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
//       since we call _CheckSkinConsistency() when we set a new skin
//-----------------------------------------------------------------------------

bool GUI::CreateMenu
(
	hoa_video::StaticImage &id, 
	float width, 
	float height, 
	int32 edgeVisibleFlags,
	int32 edgeSharedFlags
)
{
	id.Clear();
	
	// get all the border size information
	
	float leftBorderSize   = _currentSkin.skin[1][0].GetWidth();
	float rightBorderSize  = _currentSkin.skin[1][2].GetWidth();
	float topBorderSize    = _currentSkin.skin[2][1].GetHeight();
	float bottomBorderSize = _currentSkin.skin[0][1].GetHeight();

	float horizontalBorderSize = leftBorderSize + rightBorderSize;
	float verticalBorderSize   = topBorderSize  + bottomBorderSize;
	
	float topWidth   = _currentSkin.skin[2][1].GetWidth();
	float leftHeight = _currentSkin.skin[1][0].GetHeight();
		
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
	
	int32 inumXTiles = int32(numXTiles);
	int32 inumYTiles = int32(numYTiles);
	
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


	// re-create the overlay at the correct width and height
	
	Color c[4];
	_currentSkin.skin[1][1].GetVertexColor(c[0], 0);
	_currentSkin.skin[1][1].GetVertexColor(c[1], 1);
	_currentSkin.skin[1][1].GetVertexColor(c[2], 2);
	_currentSkin.skin[1][1].GetVertexColor(c[3], 3);
	
	_videoManager->DeleteImage(_currentSkin.skin[1][1]);	
	_currentSkin.skin[1][1].SetDimensions(leftBorderSize, topBorderSize);
	_currentSkin.skin[1][1].SetVertexColors(c[0], c[1], c[2], c[3]);
	_videoManager->LoadImage(_currentSkin.skin[1][1]);
	
	
	// first add the corners
	
	float maxX = leftBorderSize + inumXTiles * topWidth;
	float maxY = bottomBorderSize + inumYTiles * leftHeight;
	float minX = 0.0f;
	float minY = 0.0f;	

	// lower left
	if(edgeVisibleFlags & VIDEO_MENU_EDGE_LEFT && edgeVisibleFlags & VIDEO_MENU_EDGE_BOTTOM)
	{
		if(edgeSharedFlags & VIDEO_MENU_EDGE_LEFT && edgeSharedFlags & VIDEO_MENU_EDGE_BOTTOM)
			id.AddImage(_currentSkin.quad, minX, minY);
		else if(edgeSharedFlags & VIDEO_MENU_EDGE_LEFT)
			id.AddImage(_currentSkin.tri_b, minX, minY);
		else if(edgeSharedFlags & VIDEO_MENU_EDGE_BOTTOM)
			id.AddImage(_currentSkin.tri_l, minX, minY);
		else
			id.AddImage(_currentSkin.skin[2][0], minX, minY);
	}
	else if(edgeVisibleFlags & VIDEO_MENU_EDGE_LEFT)	
		id.AddImage(_currentSkin.skin[1][0], minX, minY);
	else if(edgeVisibleFlags & VIDEO_MENU_EDGE_BOTTOM)
		id.AddImage(_currentSkin.skin[0][1], minX, minY);
	else
		id.AddImage(_currentSkin.skin[1][1], minX, minY);
	
	
	// lower right
	if(edgeVisibleFlags & VIDEO_MENU_EDGE_RIGHT && edgeVisibleFlags & VIDEO_MENU_EDGE_BOTTOM)	
	{
		if(edgeSharedFlags & VIDEO_MENU_EDGE_RIGHT && edgeSharedFlags & VIDEO_MENU_EDGE_BOTTOM)
			id.AddImage(_currentSkin.quad, maxX, minY);            
		else if(edgeSharedFlags & VIDEO_MENU_EDGE_RIGHT)
			id.AddImage(_currentSkin.tri_b, maxX, minY);            
		else if(edgeSharedFlags & VIDEO_MENU_EDGE_BOTTOM)
			id.AddImage(_currentSkin.tri_r, maxX, minY);            
		else
			id.AddImage(_currentSkin.skin[2][2], maxX, minY);            
	}
	else if(edgeVisibleFlags & VIDEO_MENU_EDGE_RIGHT)
		id.AddImage(_currentSkin.skin[1][2], maxX, minY);
	else if(edgeVisibleFlags & VIDEO_MENU_EDGE_BOTTOM)
		id.AddImage(_currentSkin.skin[2][1], maxX, minY);
	else
		id.AddImage(_currentSkin.skin[1][1], maxX, minY);

	// upper left
	if(edgeVisibleFlags & VIDEO_MENU_EDGE_LEFT && edgeVisibleFlags & VIDEO_MENU_EDGE_TOP)
	{
		if(edgeSharedFlags & VIDEO_MENU_EDGE_LEFT && edgeSharedFlags & VIDEO_MENU_EDGE_TOP)
			id.AddImage(_currentSkin.quad, minX, maxY);
		else if(edgeSharedFlags & VIDEO_MENU_EDGE_LEFT)
			id.AddImage(_currentSkin.tri_t, minX, maxY);
		else if(edgeSharedFlags & VIDEO_MENU_EDGE_TOP)
			id.AddImage(_currentSkin.tri_l, minX, maxY);
		else
			id.AddImage(_currentSkin.skin[0][0], minX, maxY);
	}
	else if(edgeVisibleFlags & VIDEO_MENU_EDGE_LEFT)
		id.AddImage(_currentSkin.skin[1][0], minX, maxY);
	else if(edgeVisibleFlags & VIDEO_MENU_EDGE_TOP)
		id.AddImage(_currentSkin.skin[0][1], minX, maxY);
	else
		id.AddImage(_currentSkin.skin[1][1], minX, maxY);

	// upper right
	if(edgeVisibleFlags & VIDEO_MENU_EDGE_TOP && edgeVisibleFlags & VIDEO_MENU_EDGE_RIGHT)	
	{		
		if(edgeSharedFlags & VIDEO_MENU_EDGE_RIGHT && edgeSharedFlags & VIDEO_MENU_EDGE_TOP)
			id.AddImage(_currentSkin.quad, maxX, maxY);
		else if(edgeSharedFlags & VIDEO_MENU_EDGE_RIGHT)
			id.AddImage(_currentSkin.tri_t, maxX, maxY);
		else if(edgeSharedFlags & VIDEO_MENU_EDGE_TOP)
			id.AddImage(_currentSkin.tri_r, maxX, maxY);
		else
			id.AddImage(_currentSkin.skin[0][2], maxX, maxY);
	}
	else if(edgeVisibleFlags & VIDEO_MENU_EDGE_TOP)
		id.AddImage(_currentSkin.skin[0][1], maxX, maxY);
	else if(edgeVisibleFlags & VIDEO_MENU_EDGE_RIGHT)
		id.AddImage(_currentSkin.skin[1][2], maxX, maxY);
	else
		id.AddImage(_currentSkin.skin[1][1], maxX, maxY);

	// iterate from left to right and fill in the horizontal borders
	
	for(int32 tileX = 0; tileX < inumXTiles; ++tileX)
	{
		if(edgeVisibleFlags & VIDEO_MENU_EDGE_TOP)
			id.AddImage(_currentSkin.skin[0][1], leftBorderSize + topWidth * tileX, maxY);
		else
			id.AddImage(_currentSkin.skin[1][1], leftBorderSize + topWidth * tileX, maxY);

		if(edgeVisibleFlags & VIDEO_MENU_EDGE_BOTTOM)
			id.AddImage(_currentSkin.skin[2][1], leftBorderSize + topWidth * tileX, 0.0f);
		else
			id.AddImage(_currentSkin.skin[1][1], leftBorderSize + topWidth * tileX, 0.0f);
	}
	
	// iterate from bottom to top and fill in the vertical borders

	for(int32 tileY = 0; tileY < inumYTiles; ++tileY)
	{
		if(edgeVisibleFlags & VIDEO_MENU_EDGE_LEFT)
			id.AddImage(_currentSkin.skin[1][0], 0.0f, bottomBorderSize + leftHeight * tileY);
		else
			id.AddImage(_currentSkin.skin[1][1], 0.0f, bottomBorderSize + leftHeight * tileY);
		
		if(edgeVisibleFlags & VIDEO_MENU_EDGE_RIGHT)
			id.AddImage(_currentSkin.skin[1][2], maxX, bottomBorderSize + leftHeight * tileY);
		else
			id.AddImage(_currentSkin.skin[1][1], maxX, bottomBorderSize + leftHeight * tileY);
	}


	// re-create the overlay at the correct width and height
	
	_videoManager->DeleteImage(_currentSkin.skin[1][1]);
	_currentSkin.skin[1][1].SetDimensions(innerWidth, innerHeight);
	_currentSkin.skin[1][1].SetVertexColors(c[0], c[1], c[2], c[3]);
	_videoManager->LoadImage(_currentSkin.skin[1][1]);
	
	id.AddImage(_currentSkin.skin[1][1], leftBorderSize, bottomBorderSize);


	return true;
}

}  // namespace private_video


//-----------------------------------------------------------------------------
// CalculateAlignedRect: transforms a rectangle based on the coordinate system
//                       and alignment flags
//-----------------------------------------------------------------------------

void GUIElement::CalculateAlignedRect(float &left, float &right, float &bottom, float &top)
{
	float width  = right - left;
	float height = top - bottom;

	if(width < 0.0f)
		width = -width;
	
	if(height < 0.0f)
		height = -height;
	
	GameVideo *video = GameVideo::GetReference();	
	CoordSys &cs = video->_coordSys;
	
	if(cs._upDir < 0.0f)
		top = -top;
		
	if(cs._rightDir < 0.0f)
		right = -right;
	
	float xoff, yoff;
	
	xoff = _x + ((video->_xalign + 1) * width)  * 0.5f * -cs._rightDir;
	yoff = _y + ((video->_yalign + 1) * height) * 0.5f * -cs._upDir;
	
	left   += xoff;
	right  += xoff;
	
	top    += yoff;
	bottom += yoff;		
}


//-----------------------------------------------------------------------------
// CalculateAlignedRect: calculates the rectangle for a GUI control. The difference
//                        between this function and the one for GUI elements is that
//                        controls must take their owner window into account
//-----------------------------------------------------------------------------

void GUIControl::CalculateAlignedRect(float &left, float &right, float &bottom, float &top)
{
	GUIElement::CalculateAlignedRect(left, right, bottom, top);

	// calculate the position offsets due to the owner window
	if(_owner)
	{
		// first, calculate the owner menu's rectangle
		float menu_left, menu_right, menu_bottom, menu_top;
		float menu_height, menu_width;
		
		GameVideo *video = GameVideo::GetReference();
		_owner->GetDimensions(menu_width, menu_height);
		menu_left = 0.0f;
		menu_right = menu_width;
		menu_bottom = 0.0f;
		menu_top = menu_height;
		video->PushState();
		
		int32 xalign, yalign;
		_owner->GetAlignment(xalign, yalign);
		
		video->SetDrawFlags(xalign, yalign, 0);
		_owner->CalculateAlignedRect(menu_left, menu_right, menu_bottom, menu_top);
		video->PopState();
		
		// now, depending on the alignment of the control, add an offset
	
		if(menu_left < menu_right)
		{
			left += menu_left;
			right += menu_left;
		}
		else
		{
			left += menu_right;
			right += menu_right;
		}
		
		if(menu_top < menu_bottom)
		{
			top += menu_top;
			bottom += menu_top;
		}
		else
		{
			top += menu_bottom;
			bottom += menu_bottom;
		}		

	}
}



//-----------------------------------------------------------------------------
// SetAlignment: sets the x, y alignment. Returns false if invalid value is passed
//-----------------------------------------------------------------------------

bool GUIElement::SetAlignment(int32 xalign, int32 yalign)
{
	if(_xalign != VIDEO_X_LEFT && _xalign != VIDEO_X_CENTER && _xalign != VIDEO_X_RIGHT)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: Invalid xalign value (" << xalign << ") passed to GUIElement::SetAlignment()" << endl;
		return false;
	}
	
	if(_yalign != VIDEO_Y_TOP && _yalign != VIDEO_Y_CENTER && _yalign != VIDEO_Y_BOTTOM)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: Invalid yalign value (" << yalign << ") passed to GUIElement::SetAlignment()" << endl;
		return false;
	}
	
	_xalign = xalign;
	_yalign = yalign;
	
	return true;
}


//-----------------------------------------------------------------------------
// GetAlignment: returns the x, y alignment
//-----------------------------------------------------------------------------

void GUIElement::GetAlignment(int32 &xalign, int32 &yalign)
{
	xalign = _xalign;
	yalign = _yalign;
}


//-----------------------------------------------------------------------------
// GUIElement constructor
//-----------------------------------------------------------------------------

GUIElement::GUIElement()
: _xalign(VIDEO_X_LEFT),
  _yalign(VIDEO_Y_TOP),
  _x(0.0f),
  _y(0.0f),
  _initialized(false)
{
}


}  // namespace hoa_video
