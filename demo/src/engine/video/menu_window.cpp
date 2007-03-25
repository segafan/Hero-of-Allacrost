///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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
using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_video::private_video;


namespace hoa_video {

int32 MenuWindow::_current_menu_id = 0;
map<int32, MenuWindow*> MenuWindow::_menu_map;



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
			cerr << "VIDEO WARNING: MenuWindow::Create() failed because an invalid width or height was passed in: "
				<< "(width = " << w << ", height = " << h << ")" << endl;
		return false;
	}

	_width = w;
	_height = h;
	_edge_visible_flags = visible_flags;
	_edge_shared_flags = shared_flags;

	if (_RecreateImage() == false) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: MenuWindow::Create() failed because the window could not be recreated" << endl;
		return false;
	}

	// Add the new menu window to the menu map
	_menu_map[_id] = this;
	_initialized = IsInitialized(_initialization_errors);
	return true;
}



void MenuWindow::Destroy() {
	// Remove the entry in the std::map
	map<int32, MenuWindow*>::iterator menu_iterator = _menu_map.find(_id);

	if (menu_iterator != _menu_map.end()) {
		_menu_map.erase(menu_iterator);
	}
	else {
		if(VIDEO_DEBUG)
			cerr << "VIDEO WARNING: MenuWindow::Destroy() could not find the menu map entry" << endl;
	}

	VideoManager->DeleteImage(_menu_image);
}



void MenuWindow::Update(uint32 frame_time) {
	_current_time += frame_time;

	if (_current_time >= VIDEO_MENU_SCROLL_TIME) {
		if (_window_state == VIDEO_MENU_STATE_SHOWING)
			_window_state = VIDEO_MENU_STATE_SHOWN;
		else if (_window_state == VIDEO_MENU_STATE_HIDING)
			_window_state = VIDEO_MENU_STATE_HIDDEN;
	}

	if (_window_state == VIDEO_MENU_STATE_HIDDEN || _window_state == VIDEO_MENU_STATE_SHOWN) {
//		if (_is_scissored == true)
		{
			float x_buffer = (_width - _inner_width) / 2;
			float y_buffer = (_height - _inner_height) / 2;

			float left, right, bottom, top;
			left = 0.0f;
			right = _width;
			bottom = 0.0f;
			top = _height;

			VideoManager->PushState();
			VideoManager->SetDrawFlags(_xalign, _yalign, 0);
			CalculateAlignedRect(left, right, bottom, top);
			VideoManager->PopState();

			_scissor_rect = VideoManager->CalculateScreenRect(left, right, bottom, top);

			_scissor_rect.left   += static_cast<int32>(x_buffer);
			_scissor_rect.width  -= static_cast<int32>(x_buffer * 2);
			_scissor_rect.top    += static_cast<int32>(y_buffer);
			_scissor_rect.height -= static_cast<int32>(y_buffer * 2);
		}

		_is_scissored = false;
		return;
	}

	_is_scissored = true;

	// Holds the amount of the window that should be drawn (1.0 == 100%)
	float draw_percent = 1.0f;

	if (_display_mode != VIDEO_MENU_INSTANT && _window_state != VIDEO_MENU_STATE_SHOWN) {
		float time = static_cast<float>(_current_time) / static_cast<float>(VIDEO_MENU_SCROLL_TIME);
		if (time > 1.0f)
			time = 1.0f;

		if (_window_state == VIDEO_MENU_STATE_HIDING)
			time = 1.0f - time;

		draw_percent = time;
	}

	if (draw_percent != 1.0f) {
		if (_display_mode == VIDEO_MENU_EXPAND_FROM_CENTER) {
			float left, right, bottom, top;
			left = 0.0f;
			right = _width;
			bottom = 0.0f;
			top = _height;

			VideoManager->PushState();
			VideoManager->SetDrawFlags(_xalign, _yalign, 0);
			CalculateAlignedRect(left, right, bottom, top);
			VideoManager->PopState();

			float center = (top + bottom) * 0.5f;

			bottom = center * (1.0f - draw_percent) + bottom * draw_percent;
			top    = center * (1.0f - draw_percent) + top * draw_percent;

			_scissor_rect = VideoManager->CalculateScreenRect(left, right, bottom, top);
		}
	}
} // void MenuWindow::Update(uint32 frame_time)



void MenuWindow::Draw() {
	if (_initialized == false) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: MenuWindow::Draw() failed because the menu window was not initialized:\n" << _initialization_errors << endl;
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
	VideoManager->DrawImage(_menu_image, Color::white);

	VideoManager->_PopContext();
	return;
} // void MenuWindow::Draw()



void MenuWindow::Show() {
	if (_initialized == false) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: MenuWindow::Show() failed because the menu window was not initialized:\n" << _initialization_errors << endl;
		return;
	}

	if (_window_state == VIDEO_MENU_STATE_SHOWING || _window_state == VIDEO_MENU_STATE_SHOWN) {
		return;
	}

	_current_time = 0;

	if (_display_mode == VIDEO_MENU_INSTANT)
		_window_state = VIDEO_MENU_STATE_SHOWN;
	else
		_window_state = VIDEO_MENU_STATE_SHOWING;
} // void MenuWindow::Show()



void MenuWindow::Hide() {
	if (_initialized == false) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: MenuWindow::Hide() failed because the menu window was not initialized:\n" << _initialization_errors << endl;
		return;
	}

	if (_window_state == VIDEO_MENU_STATE_HIDING || _window_state == VIDEO_MENU_STATE_HIDDEN) {
		return;
	}

	_current_time = 0;

	if (_display_mode == VIDEO_MENU_INSTANT)
		_window_state = VIDEO_MENU_STATE_HIDDEN;
	else
		_window_state = VIDEO_MENU_STATE_HIDING;
} // void MenuWindow::Hide()



bool MenuWindow::IsInitialized(string& errors) {
	errors.clear();
	ostringstream stream;

	// Check width
	if (_width <= 0.0f || _width > 1024.0f)
		stream << "* Invalid width (" << _width << ")" << endl;

	// Check height
	if (_height <= 0.0f || _height > 768.0f)
		stream << "* Invalid height (" << _height << ")" << endl;

	// Check display mode
	if (_display_mode <= VIDEO_MENU_INVALID || _display_mode >= VIDEO_MENU_TOTAL)
		stream << "* Invalid display mode (" << _display_mode << ")" << endl;

	// Check state
	if (_window_state <= VIDEO_MENU_STATE_INVALID || _window_state >= VIDEO_MENU_STATE_TOTAL)
		stream << "* Invalid state (" << _window_state << ")" << endl;

	// Check to see a valid image is loaded
	if (_menu_image.GetWidth() == 0)
		stream << "* Menu image is not loaded" << endl;

	errors = stream.str();

	if (errors.empty()) {
		_initialized = true;
	}
	else {
		_initialized = false;
	}

	return _initialized;
} // bool MenuWindow::IsInitialized(string& errors)



void MenuWindow::ChangeEdgeVisibleFlags(int32 flags) {
	_edge_visible_flags = flags;
	_RecreateImage();
}



void MenuWindow::ChangeEdgeSharedFlags(int32 flags) {
	_edge_shared_flags = flags;
	_RecreateImage();
}



void MenuWindow::SetDisplayMode(VIDEO_MENU_DISPLAY_MODE mode) {
	if (mode <= VIDEO_MENU_INVALID || mode >= VIDEO_MENU_TOTAL) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: MenuWindow::SetDisplayMode() failed because an invalid argument was given: " << mode << endl;
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
