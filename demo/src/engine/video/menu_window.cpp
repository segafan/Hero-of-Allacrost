///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

#include "utils.h"
#include "video.h"
#include "gui.h"
#include <sstream>
#include "menu_window.h"

using namespace std;
using namespace hoa_video;
using namespace hoa_video::private_video;
using hoa_utils::MakeUnicodeString;
using hoa_utils::ustring;


int32 MenuWindow::_currentMenuID = 0;
map<int32, MenuWindow *> MenuWindow::_menuMap;


namespace hoa_video
{


//-----------------------------------------------------------------------------
// MenuWindow
//-----------------------------------------------------------------------------

MenuWindow::MenuWindow() :
  _width(0),
  _height(0),
  _state(VIDEO_MENU_STATE_HIDDEN),
  _currentTime(0),
  _mode(VIDEO_MENU_INSTANT),
  _isScissored(false)
{
	_id = _currentMenuID;
	++_currentMenuID;	
	_initialized = IsInitialized(_initializeErrors);

}


//-----------------------------------------------------------------------------
// ~MenuWindow
//-----------------------------------------------------------------------------

MenuWindow::~MenuWindow()
{
	// do nothing: assume Destroy() was called!
}


//-----------------------------------------------------------------------------
// Draw: draws the menu window on the screen at the given position, and using
//       current coordinate system and alignment flags the API user has set
//-----------------------------------------------------------------------------

bool MenuWindow::Draw()
{
	// fail if menu window isn't initialized properly
	if(!_initialized)
	{
		if(VIDEO_DEBUG)
			cerr << "MenuWindow::Draw() failed because the menu window was not initialized:" << endl << _initializeErrors << endl;
		return false;		
	}

	if(_state == VIDEO_MENU_STATE_HIDDEN)
		return true;  // nothing to do

	GameVideo *video = GameVideo::SingletonGetReference();

	video->_PushContext();

	video->SetDrawFlags(_xalign, _yalign, VIDEO_BLEND, 0);
	if(_isScissored)
	{		
		ScreenRect rect = _scissorRect;
		
		if(video->IsScissoringEnabled())
		{
			rect.Intersect(video->GetScissorRect());
		}
		else
		{
			video->EnableScissoring(true);
		}

		video->SetScissorRect(rect);
	}

	video->Move(_x, _y);
	if(!video->DrawImage(_menuImage, Color::white))
	{
		if(VIDEO_DEBUG)
			cerr << "MenuMode::Draw() failed because GameVideo::DrawImage() returned false!" << endl;
		video->_PopContext();
		return false;
	}
	
	video->_PopContext();	
	
	return true;
}


//-----------------------------------------------------------------------------
// Update: increments the MenuWindow's timer for gradual show/display
//         Returns false on unexpected failure
//-----------------------------------------------------------------------------

bool MenuWindow::Update(int32 frameTime)
{
	_currentTime += frameTime;
	
	if(_currentTime >= VIDEO_MENU_SCROLL_TIME)
	{
		if(_state == VIDEO_MENU_STATE_SHOWING)
			_state = VIDEO_MENU_STATE_SHOWN;
		else if(_state == VIDEO_MENU_STATE_HIDING)
			_state = VIDEO_MENU_STATE_HIDDEN;
	}

	if(_state == VIDEO_MENU_STATE_HIDDEN || _state == VIDEO_MENU_STATE_SHOWN) 
	{
		if(_isScissored == true)
		{
			float xBuffer = (_width - _innerWidth) / 2;
			float yBuffer = (_height - _innerHeight) / 2;

			float left, right, bottom, top;
			left = 0.0f;
			right = _width;
			bottom = 0.0f;
			top = _height;

			GameVideo *video = GameVideo::SingletonGetReference();

			video->PushState();
			video->SetDrawFlags(_xalign, _yalign, 0);
			MenuWindow::CalculateAlignedRect(left, right, bottom, top);
			video->PopState();

			_scissorRect = video->CalculateScreenRect(left, right, bottom, top);

			_scissorRect.left += xBuffer;
			_scissorRect.width -= (xBuffer * 2);
			_scissorRect.top += yBuffer;
			_scissorRect.height -= (yBuffer * 2);
		}

		_isScissored = false;
		return true;
	}

	_isScissored = true;

	// how much of the menu to draw	
	float drawPortion = 1.0f;	
	
	if(_mode != VIDEO_MENU_INSTANT && _state != VIDEO_MENU_STATE_SHOWN)
	{
		float t = float(_currentTime) / float(VIDEO_MENU_SCROLL_TIME);
		if(t > 1.0f)
			t = 1.0f;
		
		if(_state == VIDEO_MENU_STATE_HIDING)
			t = 1.0f - t;
		
		drawPortion = t;
	}

	if(drawPortion != 1.0f)
	{
		if(_mode == VIDEO_MENU_EXPAND_FROM_CENTER)
		{
			GameVideo *video = GameVideo::SingletonGetReference();

			float left, right, bottom, top;
			left = 0.0f;
			right = _width;
			bottom = 0.0f;
			top = _height;

			video->PushState();
			video->SetDrawFlags(_xalign, _yalign, 0);
			MenuWindow::CalculateAlignedRect(left, right, bottom, top);
			video->PopState();
			
			float center = (top + bottom) * 0.5f;
			
			bottom = center * (1.0f - drawPortion) + bottom * drawPortion;
			top    = center * (1.0f - drawPortion) + top * drawPortion;

			_scissorRect = video->CalculateScreenRect(left, right, bottom, top);
		}
	}
	
	return true;
}


//-----------------------------------------------------------------------------
// Show: make the menu appear (either instantly or gradually depending on display
//       mode). Note this does not actually DRAW the menu, it just changes its state!
//-----------------------------------------------------------------------------

bool MenuWindow::Show()
{
	// fail if menu window isn't initialized properly
	if(!_initialized)
	{
		if(VIDEO_DEBUG)
			cerr << "MenuWindow::Draw() failed because the menu window was not initialized:" << endl << _initializeErrors << endl;
		return false;		
	}

	if(_state == VIDEO_MENU_STATE_SHOWING || _state == VIDEO_MENU_STATE_SHOWN)
	{
		// nothing to do
		return true;
	}
	
	_currentTime = 0;
	
	if(_mode == VIDEO_MENU_INSTANT)
		_state = VIDEO_MENU_STATE_SHOWN;
	else
		_state = VIDEO_MENU_STATE_SHOWING;	
	
	return true;
}


//-----------------------------------------------------------------------------
// Hide: make the menu disappear (either instantly or gradually depending on display
//       mode).
//-----------------------------------------------------------------------------

bool MenuWindow::Hide()
{
	// fail if menu window isn't initialized properly
	if(!_initialized)
	{
		if(VIDEO_DEBUG)
			cerr << "MenuWindow::Draw() failed because the menu window was not initialized:" << endl << _initializeErrors << endl;
		return false;		
	}

	if(_state == VIDEO_MENU_STATE_HIDING || _state == VIDEO_MENU_STATE_HIDDEN)
	{
		// nothing to do
		return true;
	}
	
	_currentTime = 0;
	
	if(_mode == VIDEO_MENU_INSTANT)
		_state = VIDEO_MENU_STATE_HIDDEN;
	else
		_state = VIDEO_MENU_STATE_HIDING;	
	
	return true;
}


//-----------------------------------------------------------------------------
// Create: creates the window, using the given width, height, edge flags, and display mode
//-----------------------------------------------------------------------------

bool MenuWindow::Create(float w, float h, int32 edgeVisibleFlags, int32 edgeSharedFlags)
{
	if(w <= 0 || h <= 0)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: MenuWindow::Create() failed because an invalid width or height was passed in: (width=" << w << ", height=" << h << ")" << endl;
		return false;
	}
	
	_width = w;
	_height = h;
	_edgeVisibleFlags = edgeVisibleFlags;
	_edgeSharedFlags = edgeSharedFlags;
	
	bool couldRecreate = RecreateImage();
	
	if(!couldRecreate)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: MenuWindow::Create() failed because RecreateImage() returned false" << endl;
	}
	
	// add the menu to the std::map
	
	_menuMap[_id] = this;

	_initialized = IsInitialized(_initializeErrors);
	return true;	
}


//-----------------------------------------------------------------------------
// RecreateImage: recreates the menu image descriptor. Assumes that
//                width, height, and edgeflags have proper values
//-----------------------------------------------------------------------------

bool MenuWindow::RecreateImage()
{
	GameVideo *video = GameVideo::SingletonGetReference();
	video->DeleteImage(_menuImage);
	return video->_CreateMenu(_menuImage, _width, _height, _innerWidth, _innerHeight, _edgeVisibleFlags, _edgeSharedFlags);
}


//-----------------------------------------------------------------------------
// IsInitialized: validates all members to make sure the menu is completely
//                initialized and ready to show text. If anything is wrong,
//                we return false, and fill the "errors" string with a list of
//                errors so they can be printed to the console
//-----------------------------------------------------------------------------

bool MenuWindow::IsInitialized(string &errors)
{
	bool success = true;
	
	errors.clear();
	ostringstream s;
	
	// check width
	if(_width <= 0.0f || _width > 1024.0f)
		s << "* Invalid width (" << _width << ")" << endl;
	
	// check height
	if(_height <= 0.0f || _height > 768.0f)
		s << "* Invalid height (" << _height << ")" << endl;
	
	// check display mode
	if(_mode <= VIDEO_MENU_INVALID || _mode >= VIDEO_MENU_TOTAL)
		s << "* Invalid display mode (" << _mode << ")" << endl;
		
	// check state
	if(_state <= VIDEO_MENU_STATE_INVALID || _state >= VIDEO_MENU_STATE_TOTAL)
		s << "* Invalid state (" << _state << ")" << endl;
		
	// check to see a valid image is loaded
	if(_menuImage.GetWidth() == 0)
		s << "* Menu image is not loaded" << endl;

	_initialized = success;	
	return success;
}


//-----------------------------------------------------------------------------
// Destroy: must call this when you're done with the menu
//-----------------------------------------------------------------------------

void MenuWindow::Destroy()
{
	// get rid of the entry in the std::map
	map<int32, MenuWindow *>::iterator menuIterator = _menuMap.find(_id);
	
	if(menuIterator != _menuMap.end())
	{
		_menuMap.erase(menuIterator);
	}
	else
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: could not find menu in the menu map on MenuWindow::Destroy()!" << endl;
	}
	
	GameVideo *video = GameVideo::SingletonGetReference();
	video->DeleteImage(_menuImage);
}


//-----------------------------------------------------------------------------
// SetDisplayMode
//-----------------------------------------------------------------------------

bool MenuWindow::SetDisplayMode(MenuDisplayMode mode)
{
	if(mode <= VIDEO_MENU_INVALID || mode >= VIDEO_MENU_TOTAL)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: MenuWindow::SetDisplayMode() failed because an invalid mode (" << mode << ") was passed in!" << endl;
		return false;
	}
	
	_mode = mode;
	
	_initialized = IsInitialized(_initializeErrors);
	return true;
}


//-----------------------------------------------------------------------------
// GetDisplayMode
//-----------------------------------------------------------------------------

MenuDisplayMode MenuWindow::GetDisplayMode()
{
	return _mode;
}


//-----------------------------------------------------------------------------
// GetState
//-----------------------------------------------------------------------------

MenuState MenuWindow::GetState()
{
	return _state;
}


//-----------------------------------------------------------------------------
// GetDimensions
//-----------------------------------------------------------------------------

void MenuWindow::GetDimensions(float &w, float &h)
{
	w = _width;
	h = _height;
}


//-----------------------------------------------------------------------------
// ChangeEdgeVisibleFlags
//-----------------------------------------------------------------------------

bool MenuWindow::ChangeEdgeVisibleFlags(int32 edgeVisibleFlags)
{
	_edgeVisibleFlags = edgeVisibleFlags;
	RecreateImage();

	return true;
}


//-----------------------------------------------------------------------------
// ChangeEdgeSharedFlags
//-----------------------------------------------------------------------------

bool MenuWindow::ChangeEdgeSharedFlags(int32 edgeSharedFlags)
{
	_edgeSharedFlags = edgeSharedFlags;
	RecreateImage();

	return true;
}


}  // namespace hoa_video
