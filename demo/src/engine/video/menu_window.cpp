///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

#include <sstream>
#include "utils.h"
#include "video.h"
#include "gui.h"
#include "menu_window.h"

using namespace std;
using namespace hoa_video;
using namespace hoa_video::private_video;
using hoa_utils::MakeUnicodeString;
using hoa_utils::ustring;

int32 MenuWindow::_current_menu_id = 0;
map<int32, MenuWindow *> MenuWindow::_menu_map;

namespace hoa_video {

MenuWindow::MenuWindow() :
	_width(0),
	_height(0),
	_window_state(VIDEO_MENU_STATE_HIDDEN),
	_current_time(0),
	_display_mode(VIDEO_MENU_INSTANT),
	_is_scissored(false)
{
	_id = _current_menu_id;
	++_current_menu_id;
	_initialized = IsInitialized(_initialization_errors);
}



bool MenuWindow::Create(float w, float h, int32 visible_flags, int32 shared_flags) {
	if (w <= 0 || h <= 0) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: MenuWindow::Create() failed because an invalid width or height was passed in: (width=" << w << ", height=" << h << ")" << endl;
		return false;
	}

	_width = w;
	_height = h;
	_edge_visible_flags = visible_flags;
	_edge_shared_flags = shared_flags;

	if (!_RecreateImage()) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: MenuWindow::Create() failed because _RecreateImage() returned false" << endl;
		return false;
	}

	// Add the new menu to the menu map

	_menu_map[_id] = this;
	_initialized = IsInitialized(_initialization_errors);
	return true;
}



void MenuWindow::Destroy() {
	// get rid of the entry in the std::map
	map<int32, MenuWindow *>::iterator menuIterator = _menu_map.find(_id);

	if(menuIterator != _menu_map.end()) {
		_menu_map.erase(menuIterator);
	}
	else {
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: could not find menu in the menu map on MenuWindow::Destroy()!" << endl;
	}

	VideoManager->DeleteImage(_menu_image);
}



void MenuWindow::Update(uint32 frame_time) {
	_current_time += frame_time;

	if(_current_time >= VIDEO_MENU_SCROLL_TIME)
	{
		if(_window_state == VIDEO_MENU_STATE_SHOWING)
			_window_state = VIDEO_MENU_STATE_SHOWN;
		else if(_window_state == VIDEO_MENU_STATE_HIDING)
			_window_state = VIDEO_MENU_STATE_HIDDEN;
	}

	if(_window_state == VIDEO_MENU_STATE_HIDDEN || _window_state == VIDEO_MENU_STATE_SHOWN)
	{
		if(_is_scissored == true)
		{
			float xBuffer = (_width - _inner_width) / 2;
			float yBuffer = (_height - _inner_height) / 2;

			float left, right, bottom, top;
			left = 0.0f;
			right = _width;
			bottom = 0.0f;
			top = _height;

			VideoManager->PushState();
			VideoManager->SetDrawFlags(_xalign, _yalign, 0);
			MenuWindow::CalculateAlignedRect(left, right, bottom, top);
			VideoManager->PopState();

			_scissor_rect = VideoManager->CalculateScreenRect(left, right, bottom, top);

			_scissor_rect.left   += static_cast<int32> (xBuffer);
			_scissor_rect.width  -= static_cast<int32> (xBuffer * 2);
			_scissor_rect.top    += static_cast<int32> (yBuffer);
			_scissor_rect.height -= static_cast<int32> (yBuffer * 2);
		}

		_is_scissored = false;
		return;
	}

	_is_scissored = true;

	// how much of the menu to draw
	float drawPortion = 1.0f;

	if(_display_mode != VIDEO_MENU_INSTANT && _window_state != VIDEO_MENU_STATE_SHOWN)
	{
		float t = float(_current_time) / float(VIDEO_MENU_SCROLL_TIME);
		if(t > 1.0f)
			t = 1.0f;

		if(_window_state == VIDEO_MENU_STATE_HIDING)
			t = 1.0f - t;

		drawPortion = t;
	}

	if(drawPortion != 1.0f)
	{
		if(_display_mode == VIDEO_MENU_EXPAND_FROM_CENTER)
		{
			float left, right, bottom, top;
			left = 0.0f;
			right = _width;
			bottom = 0.0f;
			top = _height;

			VideoManager->PushState();
			VideoManager->SetDrawFlags(_xalign, _yalign, 0);
			MenuWindow::CalculateAlignedRect(left, right, bottom, top);
			VideoManager->PopState();

			float center = (top + bottom) * 0.5f;

			bottom = center * (1.0f - drawPortion) + bottom * drawPortion;
			top    = center * (1.0f - drawPortion) + top * drawPortion;

			_scissor_rect = VideoManager->CalculateScreenRect(left, right, bottom, top);
		}
	}

	return;
}



void MenuWindow::Draw() {
	if (!_initialized) {
		if (VIDEO_DEBUG)
			cerr << "MenuWindow::Draw() failed because the menu window was not initialized:" << endl << _initialization_errors << endl;
		return;
	}

	if (_window_state == VIDEO_MENU_STATE_HIDDEN)
		return;

	VideoManager->_PushContext();

	VideoManager->SetDrawFlags(_xalign, _yalign, VIDEO_BLEND, 0);
	if (_is_scissored) {
		ScreenRect rect = _scissor_rect;
		if (VideoManager->IsScissoringEnabled()) {
			rect.Intersect(VideoManager->GetScissorRect());
		}
		else {
			VideoManager->EnableScissoring(true);
		}
		VideoManager->SetScissorRect(rect);
	}

	VideoManager->Move(_x_position, _y_position);
	if (!VideoManager->DrawImage(_menu_image, Color::white)) {
		if (VIDEO_DEBUG)
			cerr << "MenuMode::Draw() failed because GameVideo::DrawImage() returned false!" << endl;
		VideoManager->_PopContext();
		return;
	}

	VideoManager->_PopContext();
	return;
}



void MenuWindow::Show()
{
	// fail if menu window isn't initialized properly
	if(!_initialized)
	{
		if(VIDEO_DEBUG)
			cerr << "MenuWindow::Draw() failed because the menu window was not initialized:" << endl << _initialization_errors << endl;
		return;
	}

	if(_window_state == VIDEO_MENU_STATE_SHOWING || _window_state == VIDEO_MENU_STATE_SHOWN)
	{
		// nothing to do
		return;
	}

	_current_time = 0;

	if(_display_mode == VIDEO_MENU_INSTANT)
		_window_state = VIDEO_MENU_STATE_SHOWN;
	else
		_window_state = VIDEO_MENU_STATE_SHOWING;

	return;
}



void MenuWindow::Hide() {
	if (!_initialized) {
		if (VIDEO_DEBUG)
			cerr << "MenuWindow::Draw() failed because the menu window was not initialized:" << endl << _initialization_errors << endl;
		return;
	}

	if (_window_state == VIDEO_MENU_STATE_HIDING || _window_state == VIDEO_MENU_STATE_HIDDEN) {
		// Nothing to do
		return;
	}

	_current_time = 0;

	if (_display_mode == VIDEO_MENU_INSTANT)
		_window_state = VIDEO_MENU_STATE_HIDDEN;
	else
		_window_state = VIDEO_MENU_STATE_HIDING;
}



bool MenuWindow::IsInitialized(string &errors) {
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
	if(_display_mode <= VIDEO_MENU_INVALID || _display_mode >= VIDEO_MENU_TOTAL)
		s << "* Invalid display mode (" << _display_mode << ")" << endl;

	// check state
	if(_window_state <= VIDEO_MENU_STATE_INVALID || _window_state >= VIDEO_MENU_STATE_TOTAL)
		s << "* Invalid state (" << _window_state << ")" << endl;

	// check to see a valid image is loaded
	if(_menu_image.GetWidth() == 0)
		s << "* Menu image is not loaded" << endl;

	_initialized = success;
	return success;
}



void MenuWindow::ChangeEdgeVisibleFlags(int32 flags) {
	_edge_visible_flags = flags;
	_RecreateImage();
}



void MenuWindow::ChangeEdgeSharedFlags(int32 flags) {
	_edge_shared_flags = flags;
	_RecreateImage();
}



void MenuWindow::SetDisplayMode(MenuDisplayMode mode) {
	if (mode <= VIDEO_MENU_INVALID || mode >= VIDEO_MENU_TOTAL) {
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: MenuWindow::SetDisplayMode() failed because an invalid mode (" << mode << ") was passed in!" << endl;
		return;
	}

	_display_mode = mode;
	_initialized = IsInitialized(_initialization_errors);
}



bool MenuWindow::_RecreateImage() {
	VideoManager->DeleteImage(_menu_image);
	return VideoManager->_CreateMenu(_menu_image, _width, _height, _inner_width, _inner_height, _edge_visible_flags, _edge_shared_flags);
}

}  // namespace hoa_video
