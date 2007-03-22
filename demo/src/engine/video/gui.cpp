///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

#include "utils.h"
#include "gui.h"
#include "video.h"

using namespace std;

namespace hoa_video {

namespace private_video {

// *****************************************************************************
// ******************************* GUIElement **********************************
// *****************************************************************************

GUIElement::GUIElement() :
	_xalign(VIDEO_X_LEFT),
	_yalign(VIDEO_Y_TOP),
	_x_position(0.0f),
	_y_position(0.0f),
	_initialized(false)
{}



void GUIElement::SetAlignment(int32 xalign, int32 yalign) {
	if (_xalign != VIDEO_X_LEFT && _xalign != VIDEO_X_CENTER && _xalign != VIDEO_X_RIGHT) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: Invalid xalign value (" << xalign << ") passed to GUIElement::SetAlignment()" << endl;
		return;
	}

	if (_yalign != VIDEO_Y_TOP && _yalign != VIDEO_Y_CENTER && _yalign != VIDEO_Y_BOTTOM) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: Invalid yalign value (" << yalign << ") passed to GUIElement::SetAlignment()" << endl;
		return;
	}

	_xalign = xalign;
	_yalign = yalign;
}



void GUIElement::CalculateAlignedRect(float &left, float &right, float &bottom, float &top) {
	float width  = right - left;
	float height = top - bottom;

	if (width < 0.0f)
		width = -width;

	if (height < 0.0f)
		height = -height;

	if (VideoManager->_coord_sys.GetVerticalDirection() < 0.0f)
		top = -top;

	if (VideoManager->_coord_sys.GetHorizontalDirection() < 0.0f)
		right = -right;

	float xoff, yoff;

	xoff = _x_position + ((VideoManager->_xalign + 1) * width)  * 0.5f * -VideoManager->_coord_sys.GetHorizontalDirection();
	yoff = _y_position + ((VideoManager->_yalign + 1) * height) * 0.5f * -VideoManager->_coord_sys.GetVerticalDirection();

	left   += xoff;
	right  += xoff;

	top    += yoff;
	bottom += yoff;
} // void GUIElement::CalculateAlignedRect(float &left, float &right, float &bottom, float &top)

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

// *****************************************************************************
// *********************************** GUI *************************************
// *****************************************************************************

GUI::GUI() {
	for (int32 sample = 0; sample < VIDEO_FPS_SAMPLES; ++sample)
		 fps_samples[sample] = 0;

	total_fps = 0;

	cur_sample = num_samples = 0;
}



GUI::~GUI() {
	for (uint32 y = 0; y < 3; ++y) {
		for (uint32 x = 0; x < 3; ++x) {
			VideoManager->DeleteImage(current_skin.skin[y][x]);
		}
	}

	VideoManager->DeleteImage(current_skin.background);
}


//-----------------------------------------------------------------------------
// DrawFPS: calculates the FPS based on the time the frame took, and draws it
//          To make the FPS more "steady", the FPS that's displayed on the
//          screen is actually the average over the last VIDEO_FPS_SAMPLES frames.
//-----------------------------------------------------------------------------
void GUI::DrawFPS(int32 frame_time)
{
	VideoManager->SetTextColor(Color::white);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_X_NOFLIP, VIDEO_Y_NOFLIP, VIDEO_BLEND, 0);

	// calculate the FPS for current frame
	int32 fps = 1000;
	if (frame_time) {
		fps /= frame_time;
	}


	int32 n_iter;

	if (num_samples == 0)
	{
		// If the FPS display is uninitialized, set the entire FPS array to the
		// current FPS

		num_samples = n_iter = VIDEO_FPS_SAMPLES;
	}
	else if (frame_time < 2)
		n_iter = 1;    // if the game is going at 500 fps or faster, 1 iteration is enough
	else
	{
		// find if there's a discrepancy between the current frame time and the
		// averaged one. If there's a large one, it's likely that we just switched
		// modes (e.g. going from boot screen to map), so add extra samples so the
		// FPS display "catches up" more quickly

		float avg_frame_time = 1000.0f * VIDEO_FPS_SAMPLES / total_fps;
		int32 diff_time = ((int32)avg_frame_time - frame_time);

		if (diff_time < 0)
			diff_time = -diff_time;

		if (diff_time <= VIDEO_MAX_FTIME_DIFF)
			n_iter = 1;
		else
			n_iter = VIDEO_FPS_CATCHUP;  // catch up faster
	}


	for (int32 j = 0; j < n_iter; ++j)
	{
		// insert newly calculated FPS into fps buffer

		if (cur_sample < 0 || cur_sample >= VIDEO_FPS_SAMPLES)
		{
			if (VIDEO_DEBUG)
				cerr << "VIDEO ERROR: out of bounds _curSample in DrawFPS()!" << endl;
			return;
		}

		total_fps -= fps_samples[cur_sample];
		total_fps += fps;
		fps_samples[cur_sample] = fps;
		cur_sample = (cur_sample+1) % VIDEO_FPS_SAMPLES;
	}

	// find the average FPS
	int32 avg_FPS = total_fps / VIDEO_FPS_SAMPLES;

	// display to screen
	char fps_text[16];
	sprintf(fps_text, "fps: %d", avg_FPS);

	if (!VideoManager->SetFont("debug_font"))
	{
		return;
	}

	VideoManager->Move(930.0f, 720.0f);
	VideoManager->DrawText(fps_text);
} // void GUI::DrawFPS(int32 frame_time)


//-----------------------------------------------------------------------------
// SetMenuSkin:
//-----------------------------------------------------------------------------
bool GUI::SetMenuSkin
(
	const std::string &img_file_TL,
	const std::string &img_file_T,
	const std::string &img_file_TR,
	const std::string &img_file_L,
	const std::string &img_file_R,
	const std::string &img_file_BL,
	const std::string &img_file_B,
	const std::string &img_file_BR,
	const std::string &img_file_Tri_T,
	const std::string &img_file_Tri_L,
	const std::string &img_file_Tri_R,
	const std::string &img_file_Tri_B,
	const std::string &img_file_Quad,
	const Color &fill_color_TL,
	const Color &fill_color_TR,
	const Color &fill_color_BL,
	const Color &fill_color_BR,
	const std::string &background_image
)
{
	int32 x, y;

	// before we actually load in the new skin, delete the old one

	for (y = 0; y < 3; ++y)
	{
		for (x = 0; x < 3; ++x)
		{
			VideoManager->DeleteImage(current_skin.skin[y][x]);

			// setting width/height to zero means to fall back to the
			// width and height of the images in pixels
			current_skin.skin[y][x].SetDimensions(0.0f, 0.0f);
		}
	}

	VideoManager->DeleteImage(current_skin.tri_t);
	VideoManager->DeleteImage(current_skin.tri_l);
	VideoManager->DeleteImage(current_skin.tri_r);
	VideoManager->DeleteImage(current_skin.tri_b);
	VideoManager->DeleteImage(current_skin.quad);
	VideoManager->DeleteImage(current_skin.background);

	current_skin.tri_t.SetDimensions(0.0f, 0.0f);
	current_skin.tri_l.SetDimensions(0.0f, 0.0f);
	current_skin.tri_r.SetDimensions(0.0f, 0.0f);
	current_skin.tri_b.SetDimensions(0.0f, 0.0f);
	current_skin.quad.SetDimensions(0.0f, 0.0f);

	map<int32, MenuWindow *>::iterator i_menu = MenuWindow::_menu_map.begin();

	while (i_menu != MenuWindow::_menu_map.end())
	{
		MenuWindow *menu = i_menu->second;
		VideoManager->DeleteImage(menu->_menu_image);
		++i_menu;
	}

	// now load the new images

	current_skin.skin[0][0].SetFilename(img_file_TL);
	current_skin.skin[0][1].SetFilename(img_file_T);
	current_skin.skin[0][2].SetFilename(img_file_TR);
	current_skin.skin[1][0].SetFilename(img_file_L);
	current_skin.skin[1][2].SetFilename(img_file_R);
	current_skin.skin[2][0].SetFilename(img_file_BL);
	current_skin.skin[2][1].SetFilename(img_file_B);
	current_skin.skin[2][2].SetFilename(img_file_BR);

	current_skin.tri_t.SetFilename(img_file_Tri_T);
	current_skin.tri_l.SetFilename(img_file_Tri_L);
	current_skin.tri_r.SetFilename(img_file_Tri_R);
	current_skin.tri_b.SetFilename(img_file_Tri_B);
	current_skin.quad.SetFilename(img_file_Quad);

	current_skin.background.SetFilename(background_image);

	// the center doesn't have an image, it's just a colored quad
	current_skin.skin[1][1].SetVertexColors
	(
		fill_color_TL,
		fill_color_TR,
		fill_color_BL,
		fill_color_BR
	);

	for (y = 0; y < 3; ++y)
	{
		for (x = 0; x < 3; ++x)
		{
			VideoManager->LoadImage(current_skin.skin[y][x]);
		}
	}

	VideoManager->LoadImage(current_skin.tri_t);
	VideoManager->LoadImage(current_skin.tri_l);
	VideoManager->LoadImage(current_skin.tri_r);
	VideoManager->LoadImage(current_skin.tri_b);
	VideoManager->LoadImage(current_skin.quad);

	VideoManager->LoadImage(current_skin.background);

	if (!_CheckSkinConsistency(current_skin))
	{
		return false;
	}

	i_menu = MenuWindow::_menu_map.begin();

	while (i_menu != MenuWindow::_menu_map.end())
	{
		MenuWindow *menu = i_menu->second;
		menu->_RecreateImage();
		++i_menu;
	}

	return true;
} // SetMenuSkin(...)





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
bool GUI::CreateMenu(StillImage &id, float width, float height, float& inner_width, float& inner_height,
	int32 edge_visible_flags, int32 edge_shared_flags) {
	id.Clear();

	// get all the border size information
	float left_border_size   = current_skin.skin[1][0].GetWidth();
	float right_border_size  = current_skin.skin[1][2].GetWidth();
	float top_border_size    = current_skin.skin[2][1].GetHeight();
	float bottom_border_size = current_skin.skin[0][1].GetHeight();

	float horizontal_border_size = left_border_size + right_border_size;
	float vertical_border_size   = top_border_size  + bottom_border_size;

	float top_width   = current_skin.skin[2][1].GetWidth();
	float left_height = current_skin.skin[1][0].GetHeight();

	// calculate how many times the top/bottom images have to be tiled
	// to make up the width of the window
	inner_width = width - horizontal_border_size;

	if (inner_width < 0.0f)
	{
		if (VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: inner_width was negative in CreateMenu()!" << endl;
		}
		return false;
	}

	inner_height = height - vertical_border_size;

	if (inner_height < 0.0f)
	{
		if (VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: inner_height was negative in CreateMenu()!" << endl;
		}
		return false;
	}

	// true if there is a background image
	bool background_loaded = current_skin.background.GetWidth();

	// find how many times we have to tile the border images to fit the
	// dimensions given
	float num_x_tiles = inner_width  / top_width;
	float num_y_tiles = inner_height / left_height;

	int32 inum_x_tiles = int32(num_x_tiles);
	int32 inum_y_tiles = int32(num_y_tiles);

	// ideally, num_top and friends should all divide evenly into integers,
	// but the person who called this function might have passed in a bogus
	// width and height, so we may have to extend the dimensions a little
	float dnum_x_tiles = num_x_tiles - inum_x_tiles;
	float dnum_y_tiles = num_y_tiles - inum_y_tiles;

	if (dnum_x_tiles > 0.001f)
	{
		float width_adjust = (1.0f - dnum_x_tiles) * top_width;
		width += width_adjust;
		inner_width += width_adjust;
		++inum_x_tiles;
	}

	if (dnum_y_tiles > 0.001f)
	{
		float height_adjust = (1.0f - dnum_y_tiles) * top_width;
		height += height_adjust;
		inner_height += height_adjust;
		++inum_y_tiles;
	}

	// now we have all the information we need to create the menu!

	// re-create the overlay at the correct width and height
	Color c[4];
	current_skin.skin[1][1].GetVertexColor(c[0], 0);
	current_skin.skin[1][1].GetVertexColor(c[1], 1);
	current_skin.skin[1][1].GetVertexColor(c[2], 2);
	current_skin.skin[1][1].GetVertexColor(c[3], 3);

	VideoManager->DeleteImage(current_skin.skin[1][1]);
	current_skin.skin[1][1].SetDimensions(left_border_size, top_border_size);
	current_skin.skin[1][1].SetVertexColors(c[0], c[1], c[2], c[3]);
	VideoManager->LoadImage(current_skin.skin[1][1]);


	// if a valid background image is loaded (nonzero width), then tile the interior
	if (background_loaded)
	{
		current_skin.background.SetVertexColors(c[0], c[1], c[2], c[3]);

		float width = current_skin.background.GetWidth();
		float height = current_skin.background.GetHeight();

		float min_x = 0;
		float min_y = 0;

		float max_x = inner_width + horizontal_border_size;
		float max_y = inner_height + vertical_border_size;

		if (edge_visible_flags & VIDEO_MENU_EDGE_LEFT)
			min_x += (left_border_size / 2);
		if (edge_visible_flags & VIDEO_MENU_EDGE_BOTTOM)
			min_y += (bottom_border_size / 2);

		if (edge_visible_flags & VIDEO_MENU_EDGE_RIGHT)
			max_x -= (right_border_size / 2);
		if (edge_visible_flags & VIDEO_MENU_EDGE_TOP)
			max_y -= (top_border_size / 2);

		for (float y = min_y ; y < max_y; y += height)
		{
			for (float x = min_x; x < max_x; x += width)
			{
				float u = 1.0, v = 1.0;

				if(x + width > max_x)
					u = (max_x - x) / width;
				if(y + height > max_y)
					v = (max_y - y) / height;

				id.AddImage(current_skin.background, x, y, 0, 0, u, v);
			}
		}
	} // if (background_loaded) ...
	else
	{
		// re-create the overlay at the correct width and height
		VideoManager->DeleteImage(current_skin.skin[1][1]);
		current_skin.skin[1][1].SetDimensions(inner_width, inner_height);
		current_skin.skin[1][1].SetVertexColors(c[0], c[1], c[2], c[3]);
		VideoManager->LoadImage(current_skin.skin[1][1]);

		id.AddImage(current_skin.skin[1][1], left_border_size, bottom_border_size);
	}

	// first add the corners
	float max_x = left_border_size + inum_x_tiles * top_width;
	float max_y = bottom_border_size + inum_y_tiles * left_height;
	float min_x = 0.0f;
	float min_y = 0.0f;

	// lower left
	if (edge_visible_flags & VIDEO_MENU_EDGE_LEFT && edge_visible_flags & VIDEO_MENU_EDGE_BOTTOM)
	{
		if (edge_shared_flags & VIDEO_MENU_EDGE_LEFT && edge_shared_flags & VIDEO_MENU_EDGE_BOTTOM)
			id.AddImage(current_skin.quad, min_x, min_y);
		else if (edge_shared_flags & VIDEO_MENU_EDGE_LEFT)
			id.AddImage(current_skin.tri_b, min_x, min_y);
		else if (edge_shared_flags & VIDEO_MENU_EDGE_BOTTOM)
			id.AddImage(current_skin.tri_l, min_x, min_y);
		else
			id.AddImage(current_skin.skin[2][0], min_x, min_y);
	}
	else if (edge_visible_flags & VIDEO_MENU_EDGE_LEFT)
		id.AddImage(current_skin.skin[1][0], min_x, min_y);
	else if (edge_visible_flags & VIDEO_MENU_EDGE_BOTTOM)
		id.AddImage(current_skin.skin[0][1], min_x, min_y);
	else if (!background_loaded)
		id.AddImage(current_skin.skin[1][1], min_x, min_y);

	// lower right
	if (edge_visible_flags & VIDEO_MENU_EDGE_RIGHT && edge_visible_flags & VIDEO_MENU_EDGE_BOTTOM)
	{
		if (edge_shared_flags & VIDEO_MENU_EDGE_RIGHT && edge_shared_flags & VIDEO_MENU_EDGE_BOTTOM)
			id.AddImage(current_skin.quad, max_x, min_y);
		else if (edge_shared_flags & VIDEO_MENU_EDGE_RIGHT)
			id.AddImage(current_skin.tri_b, max_x, min_y);
		else if (edge_shared_flags & VIDEO_MENU_EDGE_BOTTOM)
			id.AddImage(current_skin.tri_r, max_x, min_y);
		else
			id.AddImage(current_skin.skin[2][2], max_x, min_y);
	}
	else if (edge_visible_flags & VIDEO_MENU_EDGE_RIGHT)
		id.AddImage(current_skin.skin[1][2], max_x, min_y);
	else if (edge_visible_flags & VIDEO_MENU_EDGE_BOTTOM)
		id.AddImage(current_skin.skin[2][1], max_x, min_y);
	else if (!background_loaded)
		id.AddImage(current_skin.skin[1][1], max_x, min_y);

	// upper left
	if (edge_visible_flags & VIDEO_MENU_EDGE_LEFT && edge_visible_flags & VIDEO_MENU_EDGE_TOP)
	{
		if (edge_shared_flags & VIDEO_MENU_EDGE_LEFT && edge_shared_flags & VIDEO_MENU_EDGE_TOP)
			id.AddImage(current_skin.quad, min_x, max_y);
		else if (edge_shared_flags & VIDEO_MENU_EDGE_LEFT)
			id.AddImage(current_skin.tri_t, min_x, max_y);
		else if (edge_shared_flags & VIDEO_MENU_EDGE_TOP)
			id.AddImage(current_skin.tri_l, min_x, max_y);
		else
			id.AddImage(current_skin.skin[0][0], min_x, max_y);
	}
	else if (edge_visible_flags & VIDEO_MENU_EDGE_LEFT)
		id.AddImage(current_skin.skin[1][0], min_x, max_y);
	else if (edge_visible_flags & VIDEO_MENU_EDGE_TOP)
		id.AddImage(current_skin.skin[0][1], min_x, max_y);
	else if (!background_loaded)
		id.AddImage(current_skin.skin[1][1], min_x, max_y);

	// upper right
	if (edge_visible_flags & VIDEO_MENU_EDGE_TOP && edge_visible_flags & VIDEO_MENU_EDGE_RIGHT)
	{
		if (edge_shared_flags & VIDEO_MENU_EDGE_RIGHT && edge_shared_flags & VIDEO_MENU_EDGE_TOP)
			id.AddImage(current_skin.quad, max_x, max_y);
		else if (edge_shared_flags & VIDEO_MENU_EDGE_RIGHT)
			id.AddImage(current_skin.tri_t, max_x, max_y);
		else if (edge_shared_flags & VIDEO_MENU_EDGE_TOP)
			id.AddImage(current_skin.tri_r, max_x, max_y);
		else
			id.AddImage(current_skin.skin[0][2], max_x, max_y);
	}
	else if (edge_visible_flags & VIDEO_MENU_EDGE_TOP)
		id.AddImage(current_skin.skin[0][1], max_x, max_y);
	else if (edge_visible_flags & VIDEO_MENU_EDGE_RIGHT)
		id.AddImage(current_skin.skin[1][2], max_x, max_y);
	else if (!background_loaded)
		id.AddImage(current_skin.skin[1][1], max_x, max_y);

	// iterate from left to right and fill in the horizontal borders
	for (int32 tile_x = 0; tile_x < inum_x_tiles; ++tile_x)
	{
		if (edge_visible_flags & VIDEO_MENU_EDGE_TOP)
			id.AddImage(current_skin.skin[0][1], left_border_size + top_width * tile_x, max_y);
		else if (!background_loaded)
			id.AddImage(current_skin.skin[1][1], left_border_size + top_width * tile_x, max_y);

		if (edge_visible_flags & VIDEO_MENU_EDGE_BOTTOM)
			id.AddImage(current_skin.skin[2][1], left_border_size + top_width * tile_x, 0.0f);
		else if (!background_loaded)
			id.AddImage(current_skin.skin[1][1], left_border_size + top_width * tile_x, 0.0f);
	}

	// iterate from bottom to top and fill in the vertical borders
	for (int32 tile_y = 0; tile_y < inum_y_tiles; ++tile_y)
	{
		if (edge_visible_flags & VIDEO_MENU_EDGE_LEFT)
			id.AddImage(current_skin.skin[1][0], 0.0f, bottom_border_size + left_height * tile_y);
		else if (!background_loaded)
			id.AddImage(current_skin.skin[1][1], 0.0f, bottom_border_size + left_height * tile_y);

		if (edge_visible_flags & VIDEO_MENU_EDGE_RIGHT)
			id.AddImage(current_skin.skin[1][2], max_x, bottom_border_size + left_height * tile_y);
		else if (!background_loaded)
			id.AddImage(current_skin.skin[1][1], max_x, bottom_border_size + left_height * tile_y);
	}

	return true;
} // CreateMenu(...)


bool GUI::_CheckSkinConsistency(const MenuSkin &s) {
	// check #1: widths of top and bottom borders are equal
	if (s.skin[2][1].GetWidth() != s.skin[0][1].GetWidth())
	{
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: widths of top and bottom border of skin are mismatched!" << endl;
		return false;
	}

	// check #2: heights of left and right borders are equal
	if (s.skin[1][0].GetHeight() != s.skin[1][2].GetHeight())
	{
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: heights of left and right border of skin are mismatched!" << endl;
		return false;
	}

	// check #3: widths of upper-left, left, and bottom-left border are equal
	if ( (s.skin[2][0].GetWidth() != s.skin[1][0].GetWidth()) ||  // UL, L
	     (s.skin[2][0].GetWidth() != s.skin[0][0].GetWidth()))    // UL, BL
	{
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: upper-left, left, and lower-left of menu skin must have equal width!" << endl;
		return false;
	}

	// check #4: widths of upper-right, right, and bottom-right border are equal
	if ( (s.skin[2][2].GetWidth() != s.skin[1][2].GetWidth()) ||  // UR, R
	     (s.skin[2][2].GetWidth() != s.skin[0][2].GetWidth()))    // UR, BR
	{
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: upper-right, right, and lower-right of menu skin must have equal width!" << endl;
		return false;
	}

	// check #5: heights of upper-left, top, and upper-right are equal
	if ( (s.skin[2][0].GetHeight() != s.skin[2][1].GetHeight()) || // UL, U
	     (s.skin[2][0].GetHeight() != s.skin[2][2].GetHeight()))   // UL, UR
	{
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: upper-left, top, and upper-right of menu skin must have equal height!" << endl;
		return false;
	}

	// check #6: heights of lower-left, bottom, and lower-right are equal
	if ( (s.skin[0][0].GetHeight() != s.skin[0][1].GetHeight()) || // LL, L
	     (s.skin[0][0].GetHeight() != s.skin[0][2].GetHeight()))   // LL, LR
	{
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: lower-left, bottom, and lower-right of menu skin must have equal height!" << endl;
		return false;
	}

	// phew!
	return true;
} // bool GUI::_CheckSkinConsistency(const MenuSkin &s)

} // namespace private_video

} // namespace hoa_video
