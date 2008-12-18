///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

#include "video.h"
#include "gui.h"

using namespace std;
using namespace hoa_utils;

template<> hoa_video::GUISupervisor* Singleton<hoa_video::GUISupervisor>::_singleton_reference = NULL;

namespace hoa_video {

GUISupervisor* GUIManager = NULL;

namespace private_video {

// *****************************************************************************
// ******************************* GUIElement **********************************
// *****************************************************************************

GUIElement::GUIElement() :
	_xalign(VIDEO_X_LEFT),
	_yalign(VIDEO_Y_TOP),
	_x_position(0.0f),
	_y_position(0.0f),
	_width(0.0f),
	_height(0.0f),
	_initialized(false)
{}



void GUIElement::SetDimensions(float w, float h) {
	if (w <= 0.0f) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "invalid width argument: " << w << endl;
		return;
	}

	if (h <= 0.0f) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "invalid height argument: " << h << endl;
		return;
	}

	_width = w;
	_height = h;
}



void GUIElement::SetAlignment(int32 xalign, int32 yalign) {
	if (_xalign != VIDEO_X_LEFT && _xalign != VIDEO_X_CENTER && _xalign != VIDEO_X_RIGHT) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "invalid xalign value: " << xalign << endl;
		return;
	}

	if (_yalign != VIDEO_Y_TOP && _yalign != VIDEO_Y_CENTER && _yalign != VIDEO_Y_BOTTOM) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "invalid yalign value: " << yalign << endl;
		return;
	}

	_xalign = xalign;
	_yalign = yalign;
}



void GUIElement::CalculateAlignedRect(float& left, float& right, float& bottom, float& top) {
	float width = right - left;
	float height = top - bottom;

	if (width < 0.0f)
		width = -width;

	if (height < 0.0f)
		height = -height;

	if (VideoManager->_current_context.coordinate_system.GetVerticalDirection() < 0.0f)
		top = -top;

	if (VideoManager->_current_context.coordinate_system.GetHorizontalDirection() < 0.0f)
		right = -right;

	float x_off, y_off;

	x_off = _x_position + ((VideoManager->_current_context.x_align + 1) * width)  * 0.5f *
		-VideoManager->_current_context.coordinate_system.GetHorizontalDirection();
	y_off = _y_position + ((VideoManager->_current_context.y_align + 1) * height) * 0.5f *
		-VideoManager->_current_context.coordinate_system.GetVerticalDirection();

	left   += x_off;
	right  += x_off;
	top    += y_off;
	bottom += y_off;
} // void GUIElement::CalculateAlignedRect(float &left, float &right, float &bottom, float &top)



void GUIElement::_DEBUG_DrawOutline() {
	float left = 0.0f;
	float right = _width;
	float bottom = 0.0f;
	float top = _height;

	VideoManager->Move(0.0f, 0.0f);
	CalculateAlignedRect(left, right, bottom, top);
	VideoManager->DrawRectangleOutline(left, right, bottom, top, 3, alpha_black);
	VideoManager->DrawRectangleOutline(left, right, bottom, top, 1, alpha_white);
}

// *****************************************************************************
// ******************************* GUIControl **********************************
// *****************************************************************************

void GUIControl::CalculateAlignedRect(float &left, float &right, float &bottom, float &top) {
	GUIElement::CalculateAlignedRect(left, right, bottom, top);

	// calculate the position offsets due to the owner window
	if (_owner) {
		// first, calculate the owner menu's rectangle
		float menu_left, menu_right, menu_bottom, menu_top;
		float menu_height, menu_width;

		_owner->GetDimensions(menu_width, menu_height);
		menu_left = 0.0f;
		menu_right = menu_width;
		menu_bottom = 0.0f;
		menu_top = menu_height;
		VideoManager->PushState();

		int32 xalign, yalign;
		_owner->GetAlignment(xalign, yalign);

		VideoManager->SetDrawFlags(xalign, yalign, 0);
		_owner->CalculateAlignedRect(menu_left, menu_right, menu_bottom, menu_top);
		VideoManager->PopState();

		// now, depending on the alignment of the control, add an offset
		if (menu_left < menu_right) {
			left += menu_left;
			right += menu_left;
		}
		else {
			left += menu_right;
			right += menu_right;
		}

		if (menu_top < menu_bottom) {
			top += menu_top;
			bottom += menu_top;
		}
		else {
			top += menu_bottom;
			bottom += menu_bottom;
		}
	} // if (_owner)
}



void GUIControl::_DEBUG_DrawOutline() {
	float left = 0.0f;
	float right = _width;
	float bottom = 0.0f;
	float top = _height;

	VideoManager->Move(0.0f, 0.0f);
	CalculateAlignedRect(left, right, bottom, top);
	VideoManager->DrawRectangleOutline(left, right, bottom, top, 3, alpha_black);
	VideoManager->DrawRectangleOutline(left, right, bottom, top, 1, alpha_white);
}

} // namespace private_video

using namespace private_video;

// *****************************************************************************
// ****************************** GUISupervisor ********************************
// *****************************************************************************

GUISupervisor::GUISupervisor() {
	_fps_sum = 0;
	_current_sample = 0;
	_number_samples = 0;

	for (uint32 sample = 0; sample < FPS_SAMPLES; sample++)
		 _fps_samples[sample] = 0;
}



GUISupervisor::~GUISupervisor() {
	// Determine if any MenuWindows have not yet been deleted, and delete them if they exist
	if (_menu_windows.empty() == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "there were undestroyed MenuWindows in GUISupervisor destructor" << endl;
		std::map<uint32, MenuWindow*> window_copies = _menu_windows;
		for (std::map<uint32, MenuWindow*>::iterator i = window_copies.begin(); i != window_copies.end(); i++) {
			i->second->Destroy();
		}
		window_copies.clear();
	}

	// Delete all menu skins which are still active
	_menu_skins.clear();
}



bool GUISupervisor::SingletonInitialize() {
	if (ImageDescriptor::LoadMultiImageFromElementGrid(_scroll_arrows, "img/menus/scroll_arrows.png", 2, 4) == true)
		return true;
	else
		return false;
}



bool GUISupervisor::LoadMenuSkin(std::string skin_name, std::string border_image, std::string background_image, bool make_default)
{
	return LoadMenuSkin(skin_name, border_image, background_image,
		Color::clear, Color::clear, Color::clear, Color::clear, make_default);
}



bool GUISupervisor::LoadMenuSkin(std::string skin_name, std::string border_image, Color background_color, bool make_default)
{
	return LoadMenuSkin(skin_name, border_image, "", background_color, background_color,
		background_color, background_color, make_default);
}



bool GUISupervisor::LoadMenuSkin(std::string skin_name, std::string border_image, Color top_left, Color top_right,
	Color bottom_left, Color bottom_right, bool make_default)
{
	return LoadMenuSkin(skin_name, border_image, "", top_left, top_right,
		bottom_left, bottom_right, make_default);
}



bool GUISupervisor::LoadMenuSkin(std::string skin_name, std::string border_image, std::string background_image,
	Color background_color, bool make_default)
{
	return LoadMenuSkin(skin_name, border_image, background_image, background_color,
		background_color, background_color, background_color, make_default);
}



bool GUISupervisor::LoadMenuSkin(string skin_name, string border_image, string background_image,
	Color top_left, Color top_right, Color bottom_left, Color bottom_right, bool make_default)
{
	// ----- (1) Check that the skin_name is not already used by another skin
	if (_menu_skins.find(skin_name) != _menu_skins.end()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "the skin name " << skin_name << " is already used by another skin" << endl;
		return false;
	}

	_menu_skins.insert(make_pair(skin_name, MenuSkin()));
	MenuSkin& new_skin = _menu_skins[skin_name];

	// ----- (2) Load the MultiImage containing the borders of the skin.
	std::vector<StillImage> skin_borders;
	if (ImageDescriptor::LoadMultiImageFromElementGrid(skin_borders, border_image, 3, 6) == false) {
		_menu_skins.erase(skin_name);
		return false;
	}

	// Copy the borders over to the new MenuSkin and delete the unused images
	new_skin.borders[0][0] = skin_borders[0];
	new_skin.borders[0][1] = skin_borders[1];
	new_skin.borders[0][2] = skin_borders[2];
	new_skin.borders[1][0] = skin_borders[6];
	new_skin.borders[1][2] = skin_borders[8];
	new_skin.borders[2][0] = skin_borders[12];
	new_skin.borders[2][1] = skin_borders[13];
	new_skin.borders[2][2] = skin_borders[14];

	new_skin.connectors[0] = skin_borders[4];
	new_skin.connectors[1] = skin_borders[16];
	new_skin.connectors[2] = skin_borders[9];
	new_skin.connectors[3] = skin_borders[11];
	new_skin.connectors[4] = skin_borders[10];

	// Set the four background colors for the vertices of the middle image
	new_skin.borders[1][1].SetVertexColors(top_left, top_right, bottom_left, bottom_right);

	// The skin borders at indeces: 3, 5, 7, 15, and 17 are not used, and will be discarded when
	// they go out of scope (ie when this function returns)

	// ----- (3) Load the background image, if one has been specified
	if (background_image != "") {
		if (new_skin.background.Load(background_image) == false) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "the background image file could not be loaded" << endl;
			_menu_skins.erase(skin_name);
			return false;
		}
	}

	// ----- (4) Determine if this new skin should be made the default skin
	if (make_default == true || _menu_skins.size() == 1) {
		_default_skin = &new_skin;
	}

	return true;
} // bool GUISupervisor::LoadMenuSkin(string skin_name, string border_image, string background_image, ...)



void GUISupervisor::SetDefaultMenuSkin(std::string& skin_name) {
	if (_menu_skins.find(skin_name) == _menu_skins.end()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "the skin name " << skin_name << " was not registered" << endl;
		return;
	}

	_default_skin = &_menu_skins[skin_name];
}



void GUISupervisor::DeleteMenuSkin(std::string& skin_name) {
	if (_menu_skins.find(skin_name) == _menu_skins.end()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "the skin name " << skin_name << " was not registered" << endl;
		return;
	}

	MenuSkin* dead_skin = &_menu_skins[skin_name];

	map<uint32, MenuWindow*>::iterator i = _menu_windows.begin();
	while (i != _menu_windows.end()) {
		if (dead_skin == i->second->_skin) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "the MenuSkin \"" << skin_name << "\" was not deleted because a MenuWindow object was found to be using it" << endl;
			return;
		}
		++i;
	}

	_menu_skins.erase(skin_name);
}



void GUISupervisor::_AddMenuWindow(MenuWindow* new_window) {
	if (_menu_windows.find(new_window->_id) != _menu_windows.end()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed because there already existed a window with the same ID" << endl;
		return;
	}
	_menu_windows.insert(std::make_pair(new_window->_id, new_window));
}



void GUISupervisor::_RemoveMenuWindow(MenuWindow* old_window) {
	map<uint32, MenuWindow*>::iterator i = _menu_windows.find(old_window->_id);

	if (i != _menu_windows.end()) {
		_menu_windows.erase(i);
	}
	else {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "did not find a corresponding entry in the menu windows map" << endl;
	}
}



void GUISupervisor::_DrawFPS(uint32 frame_time) {
	VideoManager->Text()->SetDefaultTextColor(Color::white);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_X_NOFLIP, VIDEO_Y_NOFLIP, VIDEO_BLEND, 0);

	// Calculate the FPS for the current frame
	uint32 current_fps = 1000;
	if (frame_time) {
		current_fps /= frame_time;
	}

	// The number of times to insert the current FPS sample into the fps_samples array
	uint32 number_insertions;

	if (_number_samples == 0) {
		// If the FPS display is uninitialized, set the entire FPS array to the current FPS
		_number_samples = FPS_SAMPLES;
		number_insertions = FPS_SAMPLES;
	}
	else if (current_fps >= 500) {
		 // If the game is going at 500 fps or faster, 1 insertion is enough
		number_insertions = 1;
	}
	else {
		// Find if there's a discrepancy between the current frame time and the averaged one.
		// If there's a large difference, add extra samples so the FPS display "catches up" more quickly.
		float avg_frame_time = 1000.0f * FPS_SAMPLES / _fps_sum;
		int32 time_difference = static_cast<int32>(avg_frame_time) - static_cast<int32>(frame_time);

		if (time_difference < 0)
			time_difference = -time_difference;

		if (time_difference <= static_cast<int32>(MAX_FTIME_DIFF))
			number_insertions = 1;
		else
			number_insertions = FPS_CATCHUP; // Take more samples to catch up to the current FPS
	}

	// Insert the current_fps samples into the fps_samples array for the number of times specified
	for (uint32 j = 0; j < number_insertions; j++) {
		_fps_sum -= _fps_samples[_current_sample];
		_fps_sum += current_fps;
		_fps_samples[_current_sample] = current_fps;
		_current_sample = (_current_sample + 1) % FPS_SAMPLES;
	}

	uint32 avg_fps = _fps_sum / FPS_SAMPLES;

	// The text to display to the screen
	char fps_text[16];
	sprintf(fps_text, "FPS: %d", avg_fps);

	VideoManager->Text()->SetDefaultFont("debug_font");
	VideoManager->Move(930.0f, 720.0f); // Upper right hand corner of the screen
	VideoManager->Text()->Draw(fps_text);
} // void GUISupervisor::_DrawFPS(uint32 frame_time)

} // namespace hoa_video
