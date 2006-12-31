///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

#include "utils.h"
#include <cassert>
#include <cstdarg>
#include "video.h"
#include <math.h>
#include "gui.h"

using namespace std;
using namespace hoa_video::private_video;
using namespace hoa_video;
using namespace hoa_utils;

namespace hoa_video {

namespace private_video {

// *****************************************************************************
// ********************************** Image ************************************
// *****************************************************************************

Image::Image(const std::string &fname, int32 w, int32 h, bool grayscale_) :
	filename(fname),
	width(w),
	height(h),
	grayscale(grayscale_)
{
	texture_sheet = NULL;
	x = 0;
	y = 0;
	u1 = 0.0f;
	v1 = 0.0f;
	u2 = 1.0f;
	v2 = 1.0f;
	ref_count = 0;
}



Image::Image(TexSheet *sheet, const std::string &fname, int32 x_, int32 y_, float u1_, float v1_,
	float u2_, float v2_, int32 w, int32 h, bool grayscale_) :
	texture_sheet(sheet),
	filename(fname),
	x(x_),
	y(y_),
	u1(u1_),
	v1(v1_),
	u2(u2_),
	v2(v2_),
	width(w),
	height(h),
	grayscale(grayscale_)
{
	ref_count = 0;
}

// *****************************************************************************
// ****************************** ImageElement *********************************
// *****************************************************************************

ImageElement::ImageElement(Image *image_, float x_offset_, float y_offset_, float u1_, float v1_,
	float u2_, float v2_, float width_, float height_) :
	image(image_),
	x_offset(x_offset_),
	y_offset(y_offset_),
	u1(u1_),
	v1(v1_),
	u2(u2_),
	v2(v2_),
	width(width_),
	height(height_)
{
	white = true;
	one_color = true;
	blend = false;
	color[0] = Color::white;
}



ImageElement::ImageElement(Image *image_, float x_offset_, float y_offset_, float u1_, float v1_, 
		float u2_, float v2_, float width_, float height_, Color color_[4]) :
	image(image_),
	x_offset(x_offset_),
	y_offset(y_offset_),
	u1(u1_),
	v1(v1_),
	u2(u2_),
	v2(v2_),
	width(width_),
	height(height_)
{
	color[0] = color_[0];

	// If all colors are the same, then mark it so we don't have to process all vertex colors
	if (color_[1] == color[0] && color_[2] == color[0] && color_[3] == color[0]) {
		one_color = true;

		// If all vertex colors are white, set a flag so they don't have to be processed at all
		if (color[0] == Color::white) {
			white = true;
			blend = false;
		}
		// Set blend to true if alpha < 1.0f
		else {
			blend = (color[0][3] < 1.0f);
		}
	}
	else {
		color[0] = color_[0];
		color[1] = color_[1];
		color[2] = color_[2];
		color[3] = color_[3];
		// Set blend to true if any of the four colors have an alpha value < 1.0f
		blend = (color[0][3] < 1.0f || color[1][3] < 1.0f || color[2][3] < 1.0f || color[3][3] < 1.0f);
	}
} // ImageElement::ImageElement()

// *****************************************************************************
// ******************************* MultiImage **********************************
// *****************************************************************************

MultiImage::MultiImage(AnimatedImage& id, const std::string& filename, const uint32 rows, const uint32 cols,
	const float width, const float height, const bool grayscale) :
	filename(filename),
	rows(rows),
	cols(cols),
	width(width),
	height(height),
	grayscale(grayscale),
	animated_image(&id),
	still_images(0)
{}

MultiImage::MultiImage(std::vector <StillImage>& id, const std::string& filename, const uint32 rows, const uint32 cols,
	const float width, const float height, const bool grayscale) :
	filename(filename),
	rows(rows),
	cols(cols),
	width(width),
	height(height),
	grayscale(grayscale),
	animated_image(0),
	still_images(&id)
{}

} // namespace private_video

// *****************************************************************************
// ***************************** ImageDescriptor *******************************
// *****************************************************************************

bool ImageDescriptor::Load() {
	return VideoManager->LoadImage(*this);
}



void ImageDescriptor::Draw() {
	VideoManager->DrawImage(*this);
}

// *****************************************************************************
// ******************************** StillImage *********************************
// *****************************************************************************

StillImage::StillImage(bool grayscale) {
	Clear();
	_animated = false;
	_grayscale = grayscale;
}



void StillImage::Clear() {
	_filename.clear();
	_width = 0.0f;
	_height = 0.0f;
	_is_static = false;
	_elements.clear();
	SetColor(Color::white);
}



void StillImage::EnableGrayScale() {
	this->_grayscale = true;
	this->Load();
}



void StillImage::DisableGrayScale() {
	this->_grayscale = false;
	this->Load();
}

//------------------------------------------------------------------------------
// AddImage: this is the function that gives us the ability to form "compound images".
// Call AddImage() on an existing image descriptor to place a new image at the desired offsets.
//
// NOTE: It is an error to pass in negative offsets to this function
//
// NOTE: When you create a compound image descriptor with AddImage(), remember to call DeleteImage()
// on it when you're done. Even though it's not loading any new image from disk, it increases the
// reference count.
//------------------------------------------------------------------------------
bool StillImage::AddImage(const StillImage &id, float x_offset, float y_offset, float u1, float v1, float u2, float v2)
{
	if (x_offset < 0.0f || y_offset < 0.0f) {
		if (VIDEO_DEBUG) 
			cerr << "VIDEO ERROR: passed negative offsets to StillImage::AddImage()" << endl;
		return false;
	}
	
	size_t num_elements = id._elements.size();
	if (num_elements == 0) {
		if (VIDEO_DEBUG) 
			cerr << "VIDEO ERROR: passed in an uninitialized image descriptor to StillImage::AddImage()!" << endl;
		
		return false;
	}
	
	for (uint32 i = 0; i < num_elements; ++i) {
		// Add the new image element to the descriptor
		ImageElement elem = id._elements[i];
		elem.x_offset += x_offset;
		elem.y_offset += y_offset;
		elem.u1 = u1;
		elem.v1 = v1;
		elem.u2 = u2;
		elem.v2 = v2;
		
		elem.width *= (elem.u2 - elem.u1);
		elem.height *= (elem.v2 - elem.v1);

		// TODO: This needs a comment here
		if (elem.image) {
			++(elem.image->ref_count);
		}
		_elements.push_back(elem);

		// Recalculate the width and height of the descriptor as a whole. (This assumes that there are no negative offsets.)
		float max_x = elem.x_offset + elem.width;
		if (max_x > _width)
			_width = max_x;
			
		float max_y = elem.y_offset + elem.height;
		if (max_y > _height)
			_height = max_y;
	}
	
	return true;	
} // bool StillImage::AddImage()

// *****************************************************************************
// ******************************* AnimatedImage *******************************
// *****************************************************************************

AnimatedImage::AnimatedImage(bool grayscale) {
	Clear();
	_animated = true;
	_grayscale = grayscale;
}



void AnimatedImage::Clear() {
	_frame_index = 0;
	_frame_counter = 0;
	_frames.clear();
	_is_static = false;
	_width = 0.0f;
	_height = 0.0f;
	SetColor(Color::white);
}



void AnimatedImage::EnableGrayScale() {
	// Enable gray scale on all frames
	StillImage *img;
	for (uint32 i = 0; i < _frames.size(); i++) {
		img = GetFrame(i);
		img->EnableGrayScale();
	}
}



void AnimatedImage::DisableGrayScale() {
	// Disable gray scale on all frames
	StillImage *img;
	for (uint32 i = 0; i < _frames.size(); i++) {
		img = GetFrame(i);
		img->DisableGrayScale();
	}
}



void AnimatedImage::Update() {
	if (_frames.size() <= 1)
		return;

	// Get the amount of time that expired since the last frame
	uint32 frame_change = VideoManager->GetFrameChange();
	_frame_counter += frame_change;

	// If the frame time has expired, update the frame index and counter
	while (_frame_counter >= _frames[_frame_index]._frame_time) {
		frame_change = _frame_counter - _frames[_frame_index]._frame_time;
		_frame_index = (_frame_index + 1) % _frames.size();
		_frame_counter = frame_change;
	}
}



bool AnimatedImage::AddFrame(const std::string &frame, uint32 frame_time) {
	StillImage img;
	img.SetFilename(frame);	
	img.SetDimensions(_width, _height);
	img.SetVertexColors(_color[0], _color[1], _color[2], _color[3]);
	img.SetStatic(_is_static);
	
	AnimationFrame new_frame;
	new_frame._frame_time = frame_time;
	new_frame._image = img;
	_frames.push_back(new_frame);

	return true;
}



bool AnimatedImage::AddFrame(const StillImage &frame, uint32 frame_time) {
	AnimationFrame new_frame;
	new_frame._image = frame;
	new_frame._frame_time = frame_time;

	// Check if the static image argument has been loaded yet.
	// If it has, then we have to increment the reference count
	uint32 num_elements = new_frame._image._elements.size();
	if (num_elements) {
		for (uint32 i = 0; i < num_elements; i++) {
			++(new_frame._image._elements[i].image->ref_count);
		}
	}
	
	_frames.push_back(new_frame);
	return true;
}



void AnimatedImage::SetWidth(float width) {
	_width = width;

	// Update the width of each frame image
	StillImage *img;
	for (uint32 i = 0; i < _frames.size(); ++i) {
		img = GetFrame(i);
		img->SetWidth(width);
	}
}



void AnimatedImage::SetHeight(float height) {
	_height = height;

	// Update the height of each frame image
	StillImage *img;
	for (uint32 i = 0; i < _frames.size(); i++) {
		img = GetFrame(i);
		img->SetHeight(height);
	}
}



void AnimatedImage::SetDimensions(float width, float height) {
	_width = width;
	_height = height;

	// Update the width and height of each frame image
	StillImage *img;
	for (uint32 i = 0; i < _frames.size(); i++) {
		img = GetFrame(i);
		img->SetDimensions(width, height);
	}
}



void AnimatedImage::SetColor(const Color &color)
{
	_color[0] = color;
	_color[1] = color;
	_color[2] = color;
	_color[3] = color;

	// Update the color of each frame image
	StillImage *img;
	for (uint32 i = 0; i < _frames.size(); i++) {
		img = GetFrame(i);
		img->SetColor(color);
	}
}



void AnimatedImage::SetVertexColors(const Color &tl, const Color &tr, const Color &bl, const Color &br) {
	_color[0] = tl;
	_color[1] = tr;
	_color[2] = bl;
	_color[3] = br;

	// Update the vertex colors of each frame image
	StillImage *img;
	for (uint32 i = 0; i < _frames.size(); i++) {
		img = GetFrame(i);
		img->SetVertexColors(tl, tr, bl, br);
	}
}

}  // namespace hoa_video
