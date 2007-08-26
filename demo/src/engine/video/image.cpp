///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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
// ****************************** ImageLoadInfo ********************************
// *****************************************************************************

ImageLoadInfo::ImageLoadInfo() :
	width(0),
	height(0),
	pixels(NULL)
{}


ImageLoadInfo::~ImageLoadInfo() {
	if (pixels != NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "pixels member was not NULL upon object destruction" << endl;
		// TODO: fix all areas in the video engine where it frees pixels but does not set the pointer to NULL
// 		free(pixels);
// 		pixels = NULL;
	}
}


void ImageLoadInfo::ConvertToGrayscale() {
	if (width <= 0 || height <= 0) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "width and/or height members were invalid (<= 0)" << endl;
		return;
	}

	if (pixels == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "no image data (pixels == NULL)" << endl;
		return;
	}

	uint8* end_position = static_cast<uint8*>(pixels) + (width * height * 4); // "4" is because of RGBA

	for (uint8* i = static_cast<uint8*>(pixels); i < end_position; i += 4) {
		// Compute the grayscale value for this pixel based on RGB values: 0.3R + 0.59G + 0.11B
		uint8 value = static_cast<uint8>((30 * *(i) + 59 * *(i + 1) + 11 * *(i + 2)) * 0.01f);
		*i = value;
		*(i + 1) = value;
		*(i + 2) = value;
		// *(i + 3) is the alpha value and is left unmodified
	}
}


void ImageLoadInfo::RGBAToRGB() {
	if (width <= 0 || height <= 0) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "width and/or height members were invalid (<= 0)" << endl;
		return;
	}

	if (pixels == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "no image data (pixels == NULL)" << endl;
		return;
	}

	uint8* pixel_index = static_cast<uint8*>(pixels);
	uint8* pixel_source = pixel_index;

	for (int32 i = 0; i < height * width; i++, pixel_index += 4) {
		int32 index = 3 * i;
		pixel_source[index] = *pixel_index;
		pixel_source[index + 1] = *(pixel_index + 1);
		pixel_source[index + 2] = *(pixel_index + 2);
	}

	// Reduce the memory consumed by 1/4 since we no longer need to contain alpha data
	pixels = realloc(pixels, width * height * 3);
}



void ImageLoadInfo::CopyFromTexture(TexSheet* texture) {
	if (pixels != NULL)
		free(pixels);
	pixels = NULL;

	// Get the texture as a buffer
	height = texture->height;
	width = texture->width;
	pixels = malloc(height * width * 4);
	if (pixels == NULL) {
		PRINT_ERROR << "failed to malloc enough memory to copy the texture" << endl;
	}

	TextureManager->_BindTexture(texture->tex_id);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}



void ImageLoadInfo::CopyFromImage(BaseImage* img) {
	// First copy the image's entire texture sheet to memory
	CopyFromTexture(img->texture_sheet);

	// Check that the image to copy is smaller than its texture sheet (usually true).
	// If so, then copy over only the sub-rectangle area of the image from its texture
	if (height > img->height || width > img->width) {
		uint32 src_bytes = width * 4;
		uint32 dst_bytes = img->width * 4;
		uint32 src_offset = img->y * width * 4 + img->x * 4;
		void* img_pixels = malloc(img->width * img->height * 4);
		if (img_pixels == NULL) {
			PRINT_ERROR << "failed to malloc enough memory to copy the image" << endl;
			return;
		}

		for (int32 i = 0; i < img->height; i++) {
			memcpy((uint8*)img_pixels + i * dst_bytes, (uint8*)pixels + i * src_bytes + src_offset, dst_bytes);
		}

		// Delete the memory used for the texture sheet and replace it with the memory for the image
		if (pixels)
			free(pixels);

		pixels = img_pixels;
		height = img->height;
		width = img->width;
	}
}

// *****************************************************************************
// ********************************** Image ************************************
// *****************************************************************************

Image::Image(const std::string &fname, const std::string &tags_, int32 w, int32 h, bool grayscale_) :
	filename(fname),
	tags(tags_)
{
	width = w;
	height = h;
	grayscale = grayscale_;
	texture_sheet = NULL;
	x = 0;
	y = 0;
	u1 = 0.0f;
	v1 = 0.0f;
	u2 = 1.0f;
	v2 = 1.0f;
	ref_count = 0;
	smooth = false;
}



Image::Image(TexSheet *sheet, const std::string &tags_, const std::string &fname, int32 x_, int32 y_, float u1_, float v1_,
	float u2_, float v2_, int32 w, int32 h, bool grayscale_) 
{
	texture_sheet = sheet;
	filename = fname;
	tags = tags_;
	x = x_;
	y = y_;
	u1 = u1_;
	v1 = v1_;
	u2 = u2_;
	v2 = v2_;
	width = w;
	height = h;
	grayscale = grayscale_;
	ref_count = 0;
	smooth = false;
}


// *****************************************************************************
// ****************************** ImageElement *********************************
// *****************************************************************************

ImageElement::ImageElement(Image *image_, float x_offset_, float y_offset_, float u1_, float v1_,
	float u2_, float v2_, float width_, float height_) :
	image(image_)
{
	x_offset = x_offset_;
	y_offset = y_offset_;
	u1 = u1_;
	v1 = v1_;
	u2 = u2_;
	v2 = v2_;
	width = width_;
	height = height_;
	white = true;
	one_color = true;
	blend = false;
	color[0] = Color::white;
}



ImageElement::ImageElement(Image *image_, float x_offset_, float y_offset_, float u1_, float v1_, 
		float u2_, float v2_, float width_, float height_, Color color_[4]) :
	image(image_)
{
	x_offset = x_offset_;
	y_offset = y_offset_;
	u1 = u1_;
	v1 = v1_;
	u2 = u2_;
	v2 = v2_;
	width = width_;
	height = height_;
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

BaseImage *ImageElement::GetBaseImage()
{
	return image;
}

const BaseImage *ImageElement::GetBaseImage() const
{
	return image;
}

} // namespace private_video




// *****************************************************************************
// ***************************** ImageDescriptor *******************************
// *****************************************************************************

ImageDescriptor::ImageDescriptor() :
_width (0.0f),
_height (0.0f),
_is_static (false),
_grayscale (false),
_animated (false),
_loaded (false)
{
	_color[0] = _color[1] = _color[2] = _color[3] = Color::white;
}


bool ImageDescriptor::Load()
{
	return VideoManager->LoadImage(*this);
}


void ImageDescriptor::Draw()
{
	VideoManager->DrawImage(*this);
}


bool ImageDescriptor::Save(const std::string filename) const
{
	if (_animated)
		return VideoManager->SaveImage(filename, dynamic_cast<const AnimatedImage &>(*this));
	else
		return VideoManager->SaveImage(filename, dynamic_cast<const StillImage &>(*this));
}


void ImageDescriptor::_Clear()
{
	_width = 0.0f;
	_height = 0.0f;
	_is_static = false;
	_grayscale = false;
	_color[0] = _color[1] = _color[2] = _color[3] = Color::white;

	_loaded = false;
}


// *****************************************************************************
// ******************************** StillImage *********************************
// *****************************************************************************

StillImage::StillImage(const bool grayscale) {
	Clear();
	_animated = false;
	_grayscale = grayscale;
}



void StillImage::Clear() {
	_Clear();
	_filename.clear();
	_elements.clear();

	SetColor(Color::white);
}


void StillImage::SetWidth (float width)
{
	_width = width;

	for (std::vector <private_video::ImageElement>::iterator it=_elements.begin(); it<_elements.end(); ++it)
	{
		it->width = width;
	}
}


void StillImage::SetHeight (float height)
{
	_height = height;

	for (std::vector <private_video::ImageElement>::iterator it=_elements.begin(); it<_elements.end(); ++it)
	{
		it->height = height;
	}
}


void StillImage::SetDimensions (float width, float height)
{
	_width = width;
	_height = height;

	for (std::vector <private_video::ImageElement>::iterator it=_elements.begin(); it<_elements.end(); ++it)
	{
		it->width = width;
		it->height = height;
	}
}


void StillImage::EnableGrayScale() {
	// If the image is already in grayscale mode, go back
	if (_grayscale)
		return;
	
	// Mark as grayscale
	_grayscale = true;

	// If the image is not yet loaded, go back (it will be made grayscale when loading)
	if (_elements.size() == 0)
		return;

	// Turn gray all the ImageElement components
	for (uint32 i=0; i<_elements.size(); i++)
	{
		Image *img = _elements[i].image;	// Color image

		if (img == NULL)
		{
			if (VIDEO_DEBUG)
				cerr << "VIDEO ERROR: Attemp to turn to grayscale mode a NULL Image" << endl;
			continue;
		}

		// Check first if there is a grayscale version already in the map
		if (TextureManager->_images.find(img->filename + img->tags + "<G>") != TextureManager->_images.end())
		{
			_elements[i].image = TextureManager->_images[img->filename + img->tags + "<G>"];
			++(_elements[i].image->ref_count);
			continue;
		}

		// If we arrive here, it means we have to convert to grayscale the image
		hoa_video::private_video::ImageLoadInfo buffer;
		buffer.CopyFromImage(img);

		buffer.ConvertToGrayscale();
		
		Image* new_image_gray = new Image(img->filename, img->tags+"<G>", buffer.width, buffer.height, true);

		TexSheet *sheet = TextureManager->_InsertImageInTexSheet(new_image_gray, buffer, _is_static);

		if(!sheet)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO_DEBUG: GameVideo::_InsertImageInTexSheet() returned NULL!" << endl;

			delete new_image_gray;

			if (buffer.pixels) {
				free (buffer.pixels);
				buffer.pixels = NULL;
			}

			return;
		}

		new_image_gray->ref_count = 1;
		TextureManager->_images[new_image_gray->filename + new_image_gray->tags] = new_image_gray;
		_elements[i].image = new_image_gray;
	}
}



void StillImage::DisableGrayScale() {
	// If the image is already in color mode, go back
	if (!_grayscale)
		return;
	
	// Mark as not grayscale
	_grayscale = false;

	// If the image is not yet loaded, go back
	if (_elements.size() == 0)
		return;

	// Turn to color all the ImageElement components
	for (uint32 i=0; i<_elements.size(); i++)
	{
		Image *img = _elements[i].image;	// Color image

		if (img == NULL)
		{
			if (VIDEO_DEBUG)
				cerr << "VIDEO ERROR: Attemp to turn to color mode a NULL Image" << endl;
			continue;
		}

		// Check for the color mode version of the image, already in the map
		if (TextureManager->_images.find(img->filename + img->tags.substr(0,img->tags.length()-3)) == TextureManager->_images.end())
		{
			if (VIDEO_DEBUG)
				cerr << "VIDEO ERROR: Color image not found in the map, while gray one was in it" << endl;
			continue;
		}

		_elements[i].image = TextureManager->_images[img->filename + img->tags.substr(0,img->tags.length()-3)];
		--(img->ref_count);
	}
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
	// Negative offsets not allowed
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


const BaseImageElement *StillImage::GetElement(uint32 index) const
{
	if (index >= GetNumElements())
		return NULL;
	return &_elements[index];
}

uint32 StillImage::GetNumElements() const
{
	return _elements.size();
}


// *****************************************************************************
// ******************************* AnimatedImage *******************************
// *****************************************************************************

AnimatedImage::AnimatedImage(const bool grayscale) {
	Clear();
	_animated = true;
	_grayscale = grayscale;
	_number_loops = -1;
	_loop_counter = 0;
	_loops_finished = false;
}



void AnimatedImage::Clear() {
	_Clear();
	_frame_index = 0;
	_frame_counter = 0;
	_frames.clear();
	_number_loops = -1;
	_loop_counter = 0;
	_loops_finished = false;

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

	if (_loops_finished)
		return;

	// Get the amount of time that expired since the last frame
	uint32 frame_change = VideoManager->GetFrameChange();
	_frame_counter += frame_change;

	// If the frame time has expired, update the frame index and counter.
	while (_frame_counter >= _frames[_frame_index].frame_time) {
		frame_change = _frame_counter - _frames[_frame_index].frame_time;
		_frame_index++;
		if (_frame_index >= _frames.size()) {
				// Check if the animation has looping enabled and if so, increment the loop counter
				// and cease the ani_loop_countermation if the number of animation loops have finished
			if (_number_loops >= 0 && ++_loop_counter >= _number_loops) {
				_loops_finished = true;
				_frame_counter = 0;
				_frame_index--;
				return;
			}
			_frame_index = 0;
		}
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
	new_frame.frame_time = frame_time;
	new_frame.image = img;
	_frames.push_back(new_frame);

	return true;
}



bool AnimatedImage::AddFrame(const StillImage &frame, uint32 frame_time) {
	AnimationFrame new_frame;
	new_frame.image = frame;
	new_frame.frame_time = frame_time;

	// Check if the static image argument has been loaded yet.
	// If it has, then we have to increment the reference count
	uint32 num_elements = new_frame.image._elements.size();
	if (num_elements) {
		for (uint32 i = 0; i < num_elements; i++) {
			++(new_frame.image._elements[i].image->ref_count);
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
