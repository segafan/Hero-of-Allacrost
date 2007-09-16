///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    image.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for image classes
*** ***************************************************************************/

#include <cstdarg>
#include <math.h>

#include "utils.h"
#include "image.h"
#include "video.h"
#include "gui.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_video::private_video;

namespace hoa_video {

// -----------------------------------------------------------------------------
// ImageDescriptor class
// -----------------------------------------------------------------------------

ImageDescriptor::ImageDescriptor() :
	_width(0.0f),
	_height(0.0f),
	_is_static(false),
	_grayscale(false),
	_loaded(false)
{
	_color[0] = _color[1] = _color[2] = _color[3] = Color::white;
}



void ImageDescriptor::Clear() {
	_width = 0.0f;
	_height = 0.0f;
	_is_static = false;
	_grayscale = false;
	_loaded = false;
	_color[0] = _color[1] = _color[2] = _color[3] = Color::white;
}



void ImageDescriptor::GetImageInfo(const std::string& filename, uint32 &rows, uint32& cols, uint32& bpp) {
	// Isolate the file extension
	size_t ext_position = filename.rfind('.');

	if (ext_position == string::npos) {
		throw Exception("could not decipher file extension for filename: " + filename, __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	std::string extension = std::string(filename, ext_position, filename.length() - ext_position);

	if (extension == ".png")
		_GetPngImageInfo(filename, rows, cols, bpp);
	else if (extension == ".jpg")
		_GetJpgImageInfo(filename, rows, cols, bpp);
	else
		throw Exception("unsupported image file extension \"" + extension + "\" for filename: " + filename, __FILE__, __LINE__, __FUNCTION__);
}



bool ImageDescriptor::LoadMultiImageFromElementSize(vector<StillImage>& images, const string& filename,
	const uint32 elem_width, const uint32 elem_height)
{
	// First retrieve the dimensions of the multi image (in pixels)
	uint32 img_height, img_width, bpp;
	try {
		GetImageInfo(filename, img_height, img_width, bpp);
	}
	catch (Exception e) {
		if (VIDEO_DEBUG)
			cerr << e.ToString() << endl;
		return false;
	}

	// Make sure that the element height and width divide evenly into the height and width of the multi image
	if ((img_height % elem_height) != 0 || (img_width % elem_width) != 0) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "multi image size not evenly divisible by element size for multi image file: " << filename << endl;
		return false;
	}

	// Determine the number of rows and columns of element images inside the multi image
	uint32 grid_rows = img_height / elem_height;
	uint32 grid_cols = img_width / elem_width;

	// If necessary, resize the images vector so that it is the same size as the number of element images which
	// we will soon extract from the multi image
	if (images.size() != grid_rows * grid_cols) {
		images.resize(grid_rows * grid_cols);
	}

	// If the width or height of the StillImages in the images vector were not specified (set to the default 0.0f),
	// then set those sizes to the element width and height arguments (which are in number of pixels)
	for (vector<StillImage>::iterator i = images.begin(); i < images.end(); i++) {
		if (IsFloatEqual(i->_height, 0.0f) == true)
			i->_height = static_cast<float>(elem_height);
		if (IsFloatEqual(i->_width, 0.0f) == true)
			i->_width = static_cast<float>(elem_width);
	}

	return _LoadMultiImage(images, filename, grid_rows, grid_cols);
} // bool ImageDescriptor::LoadMultiImageFromElementSize(...)



bool ImageDescriptor::LoadMultiImageFromElementGrid(vector<StillImage>& images, const string& filename,
		const uint32 grid_rows, const uint32 grid_cols)
{
	// First retrieve the dimensions of the multi image (in pixels)
	uint32 img_height, img_width, bpp;
	try {
		GetImageInfo(filename, img_height, img_width, bpp);
	}
	catch (Exception e) {
		if (VIDEO_DEBUG)
			cerr << e.ToString() << endl;
		return false;
	}

	// Make sure that the number of grid rows and columns divide evenly into the image size
	if ((img_height % grid_rows) != 0 || (img_width % grid_cols) != 0) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "multi image size not evenly divisible by grid rows or columsn for multi image file: " << filename << endl;
		return false;
	}

	// If necessary, resize the images vector so that it is the same size as the number of element images which
	// we will soon extract from the multi image
	if (images.size() != grid_rows * grid_cols) {
		images.resize(grid_rows * grid_cols);
	}

	// If the width or height of the StillImages in the images vector were not specified (set to the default 0.0f),
	// then set those sizes to the element width and height arguments (which are in number of pixels)
	float elem_width = static_cast<float>(img_width) / static_cast<float>(grid_cols);
	float elem_height = static_cast<float>(img_height) / static_cast<float>(grid_rows);
	for (vector<StillImage>::iterator i = images.begin(); i < images.end(); i++) {
		if (IsFloatEqual(i->_height, 0.0f) == true)
			i->_height = static_cast<float>(elem_height);
		if (IsFloatEqual(i->_width, 0.0f) == true)
			i->_width = static_cast<float>(elem_width);
	}

	return _LoadMultiImage(images, filename, grid_rows, grid_cols);
} // bool ImageDescriptor::LoadMultiImageFromElementGrid(...)



bool ImageDescriptor::SaveMultiImage(const vector<StillImage*>& images, const string& filename,
	const uint32 grid_rows, const uint32 grid_columns)
{
	// Check there are elements to store
	if (images.empty()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "images vector argument was empty when saving file: " << filename << endl;
		return false;
	}

	// Check if the number of images is compatible with the number of rows and columns
	if (images.size() < grid_rows * grid_columns) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "images vector argument did not contain enough images to save for file: " << filename << endl;
		return false;
	}
	else if (images.size() > grid_rows * grid_columns) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "images vector argument had a size greater than the number of images to save for file: " << filename << endl;
		// NOTE: no return false for this case because we have enough images to continue
	}

	// Check that all the images have only a single ImageElement
	for (uint32 i = 0 ; i < images.size(); i++) {
		if (images[i]->_elements.size() != 1) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "a StillImage to be saved contained multiple ImageElements when saving file: " << filename << endl;
			return false;
		}
	}

	// Check that all the images are non-NULL and are of the same size
	int32 img_width = images[0]->_elements[0].image->width;
	int32 img_height = images[0]->_elements[0].image->height;
	for (uint32 i = 0; i < images.size(); i++) {
		if (images[i] == NULL || images[i]->_elements[0].image == NULL) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "NULL StillImage or ImageElement was present in images vector argument when saving file: " << filename << endl;
			return false;
		}
		if (images[i]->_elements[0].image->width != img_width || images[i]->_elements[0].image->height != img_height) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "images contained in vector argument did not share the same dimensions" << endl;
			return false;
		}
	}

	// Isolate the filename's extension and determine the type of image file we're saving
	bool is_png_image;
	size_t ext_position = filename.rfind('.');
	if (ext_position == string::npos) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to decipher file extension for filename: " << filename << endl;
		return false;
	}

	string extension = string(filename, ext_position, filename.length() - ext_position);

	if (extension == ".png")
		is_png_image = true;
	else if (extension == ".jpg")
		is_png_image = false;
	else {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "unsupported file extension: \"" << extension << "\" for filename: " << filename << endl;
		return false;
	}

	// Structure for the image buffer to save
	ImageMemory save;

	save.height = grid_rows * img_height;
	save.width = grid_columns * img_width;
	save.pixels = malloc(save.width * save.height * 4);

	if (save.pixels == NULL) {
		PRINT_ERROR << "failed to malloc enough memory to save new image file: " << filename << endl;
		return false;
	}

	// Initially, we need to grab the Image pointer of the first StillImage, the texture ID of its TextureSheet owner,
	// and malloc enough memory for the entire sheet so that we can copy over the texture sheet from video memory to
	// system memory. 
	ImageTexture* img = const_cast<ImageTexture*>(images[0]->_elements[0].image);
	GLuint tex_id = img->texture_sheet->tex_id;

	ImageMemory texture;
	texture.width = img->texture_sheet->width;
	texture.height = img->texture_sheet->height;
	texture.pixels = malloc(texture.width * texture.height * 4);

	if (texture.pixels == NULL) {
		PRINT_ERROR << "failed to malloc enough memory to save new image file: " << filename << endl;
		free(save.pixels);
		return false;
	}

	TextureManager->_BindTexture(tex_id);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.pixels);

	uint32 i = 0; // i is used to count through the images vector to get the image to save
	for (uint32 x = 0; x < grid_rows; x++) {
		for (uint32 y = 0; y < grid_columns; y++) {
			img = const_cast<ImageTexture*>(images[i]->_elements[0].image);

			// Check if this image has a different texture ID than the last. If it does, we need to re-grab the texture
			// memory for the texture sheet that the new image is contained within and store it in the texture.pixels
			// buffer, which is CPU system memory.
			if (tex_id != img->texture_sheet->tex_id) {
				// Get new texture ID
				TextureManager->_BindTexture(img->texture_sheet->tex_id);
				tex_id = img->texture_sheet->tex_id;

				// If the new texture is bigger, reallocate memory
				if (texture.height * texture.width < img->texture_sheet->height * img->texture_sheet->width) {
					free(texture.pixels);
					texture.width = img->texture_sheet->width;
					texture.height = img->texture_sheet->height;
					texture.pixels = realloc(texture.pixels, texture.width * texture.height * 4);
					if (texture.pixels == NULL) {
						PRINT_ERROR << "failed to malloc enough memory to save new image file: " << filename << endl;
						free(save.pixels);
						return false;
					}
				}
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.pixels);
			}

			// Determine the part of the texture that we are interested in (the part that contains the current image we're saving)
			uint32 src_offset = img->y * texture.width * 4 + img->x * 4;
			uint32 dst_offset = x * img_height * img_width * grid_columns * 4 + y * img_width * 4;
			uint32 src_bytes = texture.width * 4;
			uint32 dst_bytes = img_width * grid_columns * 4;
			uint32 copy_bytes = img_width * 4;

			// Copy each row of image pixels over the the save.pixels buffer one at a time
			for (int32 j = 0; j < img_height; j++) {
				memcpy((uint8*)save.pixels + j * dst_bytes + dst_offset, (uint8*)texture.pixels + j * src_bytes + src_offset, copy_bytes);
			}

			i++;
		} // for (uint32 y = 0; y < grid_columns; y++)
	} // for (uint32 x = 0; x < grid_rows; x++)

	// save.pixels now contains all the image data we wish to save, so write it out to the new image file
	bool success = true;
	success = save.SaveImage(filename, is_png_image);
	free(save.pixels);
	free(texture.pixels);

	return success;
} // bool ImageDescriptor::SaveMultiImage(...)



void ImageDescriptor::_GetPngImageInfo(const std::string& filename, uint32& rows, uint32& cols, uint32& bpp) {
	//! \todo Someone who understands libpng needs to write some comments in this function
	FILE* fp = fopen(filename.c_str(), "rb");

	if (fp == NULL) {
		throw Exception("failed to open file: " + filename, __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	uint8 test_buffer[8];

	fread(test_buffer, 1, 8, fp);
	if (png_sig_cmp(test_buffer, 0, 8)) {
		throw Exception("png_sig_cmp() failed for file: " + filename, __FILE__, __LINE__, __FUNCTION__);
		fclose(fp);
		return;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);

	if (png_ptr == NULL) {
		fclose(fp);
		return;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, NULL, (png_infopp)NULL);
		fclose(fp);
		throw Exception("png_create_info_struct() returned NULL for file: " + filename, __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, NULL, (png_infopp)NULL);
		fclose(fp);
		throw Exception("setjmp returned non-zero value for file: " + filename, __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);

	cols = info_ptr->width;
	rows = info_ptr->height;
	bpp = info_ptr->channels * 8;

	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	fclose(fp);
} // void ImageDescriptor::_GetPngImageInfo(const std::string& filename, uint32& rows, uint32& cols, uint32& bpp)



void ImageDescriptor::_GetJpgImageInfo(const std::string& filename, uint32& rows, uint32& cols, uint32& bpp) {
	//! \todo Someone who understands libjpeg needs to write some comments in this function
	FILE* fp = fopen(filename.c_str(), "rb");

	if (fp == NULL) {
		throw Exception("failed to open file: " + filename, __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);

	cols = cinfo.output_width;
	rows = cinfo.output_height;
	bpp = cinfo.output_components;

	jpeg_destroy_decompress(&cinfo);

	fclose(fp);
} // void ImageDescriptor::_GetJpgImageInfo(const std::string& filename, uint32& rows, uint32& cols, uint32& bpp)



bool ImageDescriptor::_LoadMultiImage(vector<StillImage>& images, const string &filename,
	const uint32 grid_rows, const uint32 grid_cols)
{
	std::string tags;
	uint32 current_image;
	uint32 x, y;

	bool need_load = false;

	// Check if we have loaded all the sub-images. If any single sub-image is not loaded, then
	// we must re-load the entire multi-image file
	for (x = 0; x < grid_rows && need_load == false; x++) {
		for (y = 0; y < grid_cols && need_load == false; y++) {
			tags = "<X" + NumberToString(x) + "_" + NumberToString(grid_rows) + ">";
			tags += "<Y" + NumberToString(y) + "_" + NumberToString(grid_cols) + ">";

			// This sub image was not found, so exit this loop immediately
			if (TextureManager->_images.find(filename + tags) == TextureManager->_images.end()) {
				need_load = true;
			}
		}
	}

	// If not all the images are loaded, then load the multi image from disk and create enough memory to copy
	// over individual sub-image elements from it
	ImageMemory multi_image;
	ImageMemory sub_image;
	if (need_load) {
		if (multi_image.LoadImage(filename) == false) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to load multi image file: " << filename << endl;
			return false;
		}

		sub_image.width = multi_image.width / grid_cols;
		sub_image.height = multi_image.height / grid_rows;
		sub_image.pixels = malloc(sub_image.width * sub_image.height * 4);
		if (sub_image.pixels == NULL) {
			PRINT_ERROR << "failed to malloc memory for multi image file: " << filename << endl;
			free(multi_image.pixels);
			multi_image.pixels = NULL;
			return false;
		}
	}

	// One by one, get the subimages
	for (x = 0; x < grid_rows; x++) {
		for (y = 0; y < grid_cols; y++) {
			tags = "<X" + NumberToString(x) + "_" + NumberToString(grid_rows) + ">";
			tags += "<Y" + NumberToString(y) + "_" + NumberToString(grid_cols) + ">";

			current_image = x * grid_cols + y;

			// If this image already exists in a texture sheet somewhere, add a reference to it
			// and add a new ImageElement to the current StillImage
			if (TextureManager->_images.find(filename + tags) != TextureManager->_images.end()) {
				images.at(current_image)._elements.clear();
				ImageTexture* img = TextureManager->_images[filename + tags];

				if (img == NULL) {
					IF_PRINT_WARNING(VIDEO_DEBUG) << "a NULL image was found in the TextureManager's _images container" << endl;

					free(multi_image.pixels);
					free(sub_image.pixels);
					multi_image.pixels = NULL;
					sub_image.pixels = NULL;
					return false;
				}

				if (img->ref_count == 0) {
					// If ref count is zero, it means this image was freed, but not removed, so restore it
					img->texture_sheet->RestoreImage(img);
				}

				ImageElement element(img, images.at(current_image)._width, images.at(current_image)._height,
					0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, images.at(current_image)._color);
				images.at(current_image)._elements.push_back(element);
			}

			// We have to first extract this image from the larger multi image and add it to a texture sheet.
			// Then we can add the image data to the StillImage being constructed
			else {
				images.at(current_image)._filename = filename;

				for (int32 i = 0; i < sub_image.height; i++) {
					memcpy((uint8*)sub_image.pixels + 4 * sub_image.width * i, (uint8*)multi_image.pixels + (((x * multi_image.height / grid_rows) + i) *
						multi_image.width + y * multi_image.width / grid_cols) * 4, 4 * sub_image.width);
				}

				ImageTexture* new_image = new ImageTexture(filename, tags, sub_image.width, sub_image.height);

				// Try to insert the image in a texture sheet
				TexSheet* sheet = TextureManager->_InsertImageInTexSheet(new_image, sub_image, images.at(current_image)._is_static);

				if (sheet == NULL) {
					IF_PRINT_WARNING(VIDEO_DEBUG) << "call to TextureController::_InsertImageInTexSheet failed" << endl;

					free(multi_image.pixels);
					free(sub_image.pixels);
					multi_image.pixels = NULL;
					sub_image.pixels = NULL;
					delete new_image;
					return false;
				}

				TextureManager->_images[filename + tags] = new_image;

				// store the new image element
				ImageElement element(new_image, images.at(current_image)._width, images.at(current_image)._height,
					0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, images.at(current_image)._color);
				images.at(current_image)._elements.push_back(element);
			}

			// Finally, do a grayscale conversion for the image if grayscale mode is enabled
			if (images.at(current_image)._grayscale) {
				images.at(current_image).EnableGrayScale();
			}
		} // for (y = 0; y < grid_cols; y++)
	} // for (x = 0; x < grid_rows; x++)

	// Make sure to free all dynamically allocated memory
	if (multi_image.pixels) {
		free(multi_image.pixels);
		multi_image.pixels = NULL;
	}
	if (sub_image.pixels) {
		free(sub_image.pixels);
		sub_image.pixels = NULL;
 	}

	return true;
} // bool ImageDescriptor::_LoadMultiImage(...)

// -----------------------------------------------------------------------------
// StillImage class
// -----------------------------------------------------------------------------

StillImage::StillImage(const bool grayscale) {
	Clear();
	_grayscale = grayscale;
}



void StillImage::Clear() {
	ImageDescriptor::Clear();
	_filename.clear();
	_elements.clear();
}



bool StillImage::Load(const string& filename) {
	// Delete everything previously stored in here
	_elements.clear();
	_filename = filename;

	// TEMP: This is a temporary hack to support procedural images by using empty filenames. It should be removed later
	// 1. Special case: if filename is empty, load a colored quad
	if (filename.empty()) {
		ImageElement quad(NULL, _width, _height, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, _color);
		_elements.push_back(quad);
		return true;
	}

	// 2. Check if an image with the same filename has already been loaded. If so, point to that and increment its reference
	map<string, ImageTexture*>::iterator i = TextureManager->_images.find(_filename);
	if (i != TextureManager->_images.end()) {
		ImageTexture* img = i->second;

		if (img == NULL) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "recovered a NULL image inside the TextureManager's image map: " << _filename << endl;
			return false;
		}

		// If the following condition is true, it means this image was freed but not removed from the texture sheet
		// So, we must restore it
		if (img->ref_count == 0) {
			img->texture_sheet->RestoreImage(img);
		}

		if (IsFloatEqual(_width, 0.0f))
			_width = static_cast<float>(img->width);
		if (IsFloatEqual(_height, 0.0f))
			_height = static_cast<float>(img->height);

		ImageElement element(img, _width, _height, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, _color);
		_elements.push_back(element);

		return true;
	}

	// 3. The image file needs to be loaded from disk
	ImageMemory img_data;

	if (img_data.LoadImage(_filename) == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "call to ImageMemory::LoadImage() failed for file: " << _filename << endl;
		return false;
	}

	// Create a new texture image and store it in a texture sheet. If the _grayscale member of this class is true,
	// we first load the color copy of the image to a texture sheet. Then we'll convert the image data to grayscale
	// and save that image data to texture memory as well
	ImageTexture* new_image = new ImageTexture(_filename, "", img_data.width, img_data.height);

	if (TextureManager->_InsertImageInTexSheet(new_image, img_data, _is_static) == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "call to TextureController::_InsertImageInTexSheet() failed for file: " << _filename << endl;

		delete new_image;
		free(img_data.pixels);
		img_data.pixels = NULL;
		return false;
	}

	TextureManager->_images[_filename] = new_image;

	// If width or height members are zero, set them to the dimensions of the image data (which are in number of pixels)
	if (IsFloatEqual(_width, 0.0f) == true)
		_width = static_cast<float>(img_data.width);

	if (IsFloatEqual(_height, 0.0f) == true)
		_height = static_cast<float>(img_data.height);

	// Check whether we also need to create a grayscale version of this image or not
	if (_grayscale == false) {
		_elements.push_back(ImageElement(new_image, _width, _height, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, _color));
		free(img_data.pixels);
		
		img_data.pixels = NULL;
		
		
		
		return true;
	}

	// If we reached this point, we must now create a grayscale version of this image
	img_data.ConvertToGrayscale();
	ImageTexture* gray_image = new ImageTexture(_filename, "<G>", img_data.width, img_data.height);
	if (TextureManager->_InsertImageInTexSheet(gray_image, img_data, _is_static) == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "call to TextureController::_InsertImageInTexSheet() failed for file: " << _filename
			<< ", could not enable grayscale mode" << endl;

		TextureManager->_images.erase(_filename);
		new_image->RemoveReference();
		delete new_image;
		free(img_data.pixels);
		img_data.pixels = NULL;
		return false;
	}

	TextureManager->_images[_filename + "<G>"] = gray_image;
	_elements.push_back(ImageElement(gray_image, _width, _height, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, _color));
	free(img_data.pixels);
	img_data.pixels = NULL;
	return true;
} // bool StillImage::Load(const string& filename)



void StillImage::Draw() const {
	// If real lighting is enabled, draw images normally since the light overlay
	// will take care of the modulation. If not, (i.e. no overlay is being used)
	// then pass the light color so the vertex colors can do the modulation
	if (VideoManager->_uses_lights == false && (VideoManager->_light_color != Color::white))
		Draw(VideoManager->_light_color);
	else
		Draw(Color::white);
}



void StillImage::Draw(const Color& draw_color) const {
	// Don't draw anything if this image is completely transparent (invisible)
	if (IsFloatEqual(draw_color[3], 0.0f) == true) {
		return;
	}
	
	float modulation = VideoManager->_screen_fader.GetFadeModulation();
	Color fade_color(modulation, modulation, modulation, 1.0f);
	
	float x_shake = VideoManager->_x_shake * (VideoManager->_current_context.coordinate_system.GetRight() -
		VideoManager->_current_context.coordinate_system.GetLeft()) / 1024.0f;
	float y_shake = VideoManager->_y_shake * (VideoManager->_current_context.coordinate_system.GetTop() -
		VideoManager->_current_context.coordinate_system.GetBottom()) / 768.0f;

	float x_align_offset = ((VideoManager->_current_context.x_align + 1) * _width) * 0.5f * -
		VideoManager->_current_context.coordinate_system.GetHorizontalDirection();
	float y_align_offset = ((VideoManager->_current_context.y_align + 1) * _height) * 0.5f * -
		VideoManager->_current_context.coordinate_system.GetVerticalDirection();

	// Save the draw cursor position as we move to draw each element
	glPushMatrix();

	VideoManager->MoveRelative(x_align_offset, y_align_offset);

	bool skip_modulation = (draw_color == Color::white && IsFloatEqual(modulation, 1.0f));

	// If we're modulating, calculate the fading color now
	if (skip_modulation == false)
		fade_color = draw_color * fade_color;
	
	for (uint32 i = 0; i < _elements.size(); ++i) {
		float x_off, y_off;

		if (VideoManager->_current_context.x_flip) {
			x_off = _width - _elements[i].x_offset - _elements[i].width;
		}
		else {
			x_off = _elements[i].x_offset;
		}
		
		if (VideoManager->_current_context.y_flip) {
			y_off = _height - _elements[i].y_offset - _elements[i].height;
		}
		else {
			y_off = _elements[i].y_offset;
		}

		x_off += x_shake;
		y_off += y_shake;

		glPushMatrix();
		VideoManager->MoveRelative(x_off * VideoManager->_current_context.coordinate_system.GetHorizontalDirection(),
			y_off * VideoManager->_current_context.coordinate_system.GetVerticalDirection());
		
		float x_scale = _elements[i].width;
		float y_scale = _elements[i].height;
		
		if (VideoManager->_current_context.coordinate_system.GetHorizontalDirection() < 0.0f)
			x_scale = -x_scale;
		if (VideoManager->_current_context.coordinate_system.GetVerticalDirection() < 0.0f)
			y_scale = -y_scale;
		
		glScalef(x_scale, y_scale, 1.0f);

		if (skip_modulation)
			_elements[i].Draw();
		else {
			Color modulated_colors[4];
			modulated_colors[0] = _elements[i].color[0] * fade_color;
			modulated_colors[1] = _elements[i].color[1] * fade_color;
			modulated_colors[2] = _elements[i].color[2] * fade_color;
			modulated_colors[3] = _elements[i].color[3] * fade_color;
			_elements[i].Draw(modulated_colors);
		}
		glPopMatrix();
	}
	glPopMatrix();
} // void StillImage::Draw(const Color& draw_color) const



bool StillImage::Save(const string& filename) const {
	if (_elements.empty()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "attempted to save an image that contained no image elements" << endl;
		return false;
	}

	if (_elements.size() > 1) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "support for the saving of compound (multi-element) images is not supported yet" << endl;
		return false;
	}

	// Isolate the file extension
	size_t ext_position = filename.rfind('.');
	bool is_png_image;

	if (ext_position == string::npos) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "could not decipher file extension for file: " << filename << endl;
		return false;
	}

	string extension = string(filename, ext_position, filename.length() - ext_position);

	if (extension == ".png")
		is_png_image = true;
	else if (extension == ".jpg")
		is_png_image = false;
	else {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "unsupported file extension \"" << extension << "\" for file: " << filename << endl;
		return false;
	}

	ImageMemory buffer;
	ImageTexture* img = const_cast<ImageTexture*>(_elements[0].image);
	buffer.CopyFromImage(img);
	return buffer.SaveImage(filename, is_png_image);
} // bool StillImage::Save(const string& filename)



void StillImage::SetWidth(float width) {
	// Case 1: No image elements loaded, just change the internal width
	if (_elements.empty() == true) {
		_width = width;
		return;
	}

	// Case 2: we only have one image element to change its width
	if (_elements.size() == 1) {
		_width = width;
		_elements[0].width = width;
		return;
	}

	// Case 3: For composite images, we must set the width of each element appropriately
	// That is, scale its width relative to the width of the composite image
	if (IsFloatEqual(_width, 0.0f) == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "internal width was 0.0f when trying to re-size multiple image elements" << endl;
		return;
	}

	for (vector<ImageElement>::iterator i = _elements.begin(); i < _elements.end(); i++) {
		if (IsFloatEqual(i->width, 0.0f) == false)
			i->width = width * (_width / i->width);
	}
	_width = width;
}



void StillImage::SetHeight(float height) {
	// Case 1: No image elements loaded, just change the internal height
	if (_elements.empty() == true) {
		_height = height;
		return;
	}

	// Case 2: we only have one image element to change its height
	if (_elements.size() == 1) {
		_height = height;
		_elements[0].height = height;
		return;
	}

	// Case 3: For composite images, we must set the width of each element appropriately
	// That is, scale its width relative to the width of the composite image
	if (IsFloatEqual(_height, 0.0f) == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "internal height was 0.0f when trying to re-size multiple image elements" << endl;
		return;
	}

	for (vector<ImageElement>::iterator i = _elements.begin(); i < _elements.end(); i++) {
		if (IsFloatEqual(i->height, 0.0f) == false)
			i->height = height * (_height / i->height);
	}
	_height = height;
}



void StillImage::EnableGrayScale() {
	if (_grayscale) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "grayscale mode was already enabled" << endl;
		return;
	}
	
	// Mark as grayscale
	_grayscale = true;

	// If no image element is yet loaded, we are done (during the loading phase, grayscale will automatically be enabled)
	if (_elements.empty() == true) {
		return;
	}

	// Enable grayscale on each image element
	for (uint32 i = 0; i < _elements.size(); i++) {
		ImageTexture* img = _elements[i].image;

		if (img == NULL) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "discovered a NULL image element at position: " << i << endl;
			continue;
		}

		// Check if there is a grayscale version of this image in texture memory already and if so, we
		// only need to change the ImageElement to the grayscale one and add a new reference to it
		if (TextureManager->_images.find(img->filename + img->tags + "<G>") != TextureManager->_images.end()) {
			// NOTE: We do not decrement the reference to the colored image, because we want to guarantee that
			// it remains referenced in texture memory while its grayscale counterpart is being used
			_elements[i].image = TextureManager->_images[img->filename + img->tags + "<G>"];
			continue;
		}

		// Create a copy of the image, convert it to grayscale, and add the grayed image copy to texture memory
		ImageMemory gray_img;
		gray_img.CopyFromImage(img);
		gray_img.ConvertToGrayscale();
		
		ImageTexture* new_img = new ImageTexture(img->filename, img->tags + "<G>", gray_img.width, gray_img.height);

		if (TextureManager->_InsertImageInTexSheet(new_img, gray_img, _is_static) == NULL) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to insert new grayscale image into texture sheet" << endl;
			delete new_img;

			if (gray_img.pixels) {
				free(gray_img.pixels);
				gray_img.pixels = NULL;
			}

			return;
		}

		TextureManager->_images[new_img->filename + new_img->tags] = new_img;
		_elements[i].image = new_img;

		if (gray_img.pixels) {
			free(gray_img.pixels);
			gray_img.pixels = NULL;
		}
	}
} // void StillImage::EnableGrayScale()



void StillImage::DisableGrayScale() {
	if (_grayscale == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "grayscale mode was already disabled" << endl;
		return;
	}

	_grayscale = false;

	// If no image elements are yet loaded, we're finished
	if (_elements.empty() == true) {
		return;
	}

	// For each ImageElement, return to using the non-grayscaled version, which should be located somewhere
	// in texture memory already (if it is not, this is an error).
	for (uint32 i = 0; i < _elements.size(); i++) {
		ImageTexture* img = _elements[i].image; // This points to the grayscale image element

		if (img == NULL) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "discovered a NULL image element at position: " << i << endl;
			continue;
		}

		// Make sure that the non-grayscale version of the image is located in texture memory. To get the map
		// string entry for this image, we crop the last 3 letters of the grayscale tag which should contain <G>
		if (TextureManager->_images.find(img->filename + img->tags.substr(0, img->tags.length() - 3)) == TextureManager->_images.end()) {
			PRINT_WARNING << "non-grayscale version of image was not found in texture memory" << endl;
			continue;
		}

		_elements[i].image = TextureManager->_images[img->filename + img->tags.substr(0, img->tags.length() - 3)];

		// Decrement the grayscale reference and if there are no references left, remove it from texture memory
		if (img->RemoveReference() == true) {
			img->texture_sheet->FreeImage(img);
		}
	}
} // void StillImage::DisableGrayScale()



void StillImage::AddImage(const StillImage& img, float x_offset, float y_offset, float u1, float v1, float u2, float v2) {
	if (x_offset < 0.0f || y_offset < 0.0f) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "negative x or y offset passed to function" << endl;
		return;
	}
	
	if (img._elements.empty() == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "StillImage argument had no image elements" << endl;
		return;
	}

	// Modify the _filename member to reflect this new image element addition (if its the only element, copy the filename)
	if (_elements.empty()) {
		_filename = img._filename;
	} else {
		_filename = "";
	}

	// Add each new image element to this image
	for (uint32 i = 0; i < img._elements.size(); ++i) {
		ImageElement elem_copy = img._elements[i];
		elem_copy.x_offset += x_offset;
		elem_copy.y_offset += y_offset;
		elem_copy.u1 = u1;
		elem_copy.v1 = v1;
		elem_copy.u2 = u2;
		elem_copy.v2 = v2;
		elem_copy.width *= (elem_copy.u2 - elem_copy.u1);
		elem_copy.height *= (elem_copy.v2 - elem_copy.v1);
		_elements.push_back(elem_copy);

		// Recalculate the width and height of the composite StillImage now that a new element has been added to it
		float max_x = elem_copy.x_offset + elem_copy.width;
		if (max_x > _width)
			_width = max_x;
			
		float max_y = elem_copy.y_offset + elem_copy.height;
		if (max_y > _height)
			_height = max_y;
	}
} // void StillImage::AddImage(const StillImage& img, float x_offset, float y_offset, float u1, float v1, float u2, float v2)



void StillImage::ConstructCompositeImage(const std::vector<StillImage>& tiles, const std::vector<std::vector<uint32> >& indeces) {
	if (tiles.empty() == true || indeces.empty() == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "either the tiles or indeces vector function arguments were empty" << endl;
		return;
	}

	for (uint32 i = 1; i < tiles.size(); i++) {
		if (tiles[0]._width != tiles[i]._width || tiles[0]._height != tiles[i]._height) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "images within the tiles argument had unequal dimensions" << endl;
			return;
		}
	}

	for (uint32 i = 1; i < indeces.size(); i++) {
		if (indeces[0].size() != indeces[i].size()) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "the row sizes in the indices 2D vector argument did not match" << endl;
			return;
		}
	}

	Clear();

	// Set the members of the composite image that we are about to construct
	_width  = static_cast<float>(indeces[0].size()) * tiles[0]._width;
	_height = static_cast<float>(indeces.size()) * tiles[0]._height;
	_is_static = tiles[0]._is_static;

	// Add each tile at the image at the appropriate offset
	for (uint32 y = 0; y < indeces.size(); ++y) {
		for (uint32 x = 0; x < indeces[0].size(); ++x) {
			// NOTE: we did not check that all the entries in indeces were within the
			// correct range for the size of the tiles vector, so this may cause a out-of-bounds
			// run-time error.
			AddImage(tiles[indeces[y][x]], x * tiles[0]._width, y * tiles[0]._height);
		}
	}
} // void ConstructCompositeImage(const std::vector<StillImage>& tiles, const std::vector<std::vector<uint32> >& indeces)



void ImageDescriptor::DEBUG_PrintInfo() {
	cout << "ImageDescriptor properties:" << endl;
	cout << "* width:                " << _width << endl;
	cout << "* height:               " << _height << endl;
	cout << "* colors, RGBA format:  " << endl;
	cout << "  * TL                  " << _color[0].GetRed() << ", " << _color[0].GetGreen() << ", " << _color[0].GetBlue() << ", " << _color[0].GetAlpha() << endl;
	cout << "  * TR                  " << _color[1].GetRed() << ", " << _color[1].GetGreen() << ", " << _color[1].GetBlue() << ", " << _color[1].GetAlpha() << endl;
	cout << "  * BL                  " << _color[2].GetRed() << ", " << _color[2].GetGreen() << ", " << _color[2].GetBlue() << ", " << _color[2].GetAlpha() << endl;
	cout << "  * BR:                 " << _color[3].GetRed() << ", " << _color[3].GetGreen() << ", " << _color[3].GetBlue() << ", " << _color[3].GetAlpha() << endl;
	cout << "* static:               " << (_is_static ? "true" : "false") << endl;
	cout << "* grayscale:            " << (_grayscale ? "true" : "false") << endl;
	cout << endl;
}

// -----------------------------------------------------------------------------
// AnimatedImage class
// -----------------------------------------------------------------------------

AnimatedImage::AnimatedImage(const bool grayscale) {
	Clear();
	_grayscale = grayscale;
}



AnimatedImage::AnimatedImage(float width, float height, bool grayscale) {
	Clear();
	_width = width;
	_height = height;
	_grayscale = grayscale;
}



void AnimatedImage::Clear() {
	ImageDescriptor::Clear();
	_frame_index = 0;
	_frame_counter = 0;
	_frames.clear();
	_number_loops = -1;
	_loop_counter = 0;
	_loops_finished = false;
}



bool AnimatedImage::LoadFromFrameSize(const string& filename, const vector<uint32>& timings, const uint32 frame_width, const uint32 frame_height, const uint32 trim) {
	// Make the multi image call
	// TODO: Handle the case where the _grayscale member is true so all frames are loaded in grayscale format
	vector<StillImage> image_frames;
	if (ImageDescriptor::LoadMultiImageFromElementSize(image_frames, filename, frame_width, frame_height) == false) {
		return false;
	}

	if (trim >= image_frames.size()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "attempt to trim away more frames than requested to load for file: " << filename << endl;
		return false;
	}

	if (timings.size() < (image_frames.size() - trim)) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "not enough timing data to fill frames grid when loading file: " << filename << endl;
		return false;
	}

	_frames.clear();
	ResetAnimation();

	// Add the loaded frame image and timing information
	for (uint32 i = 0; i < image_frames.size() - trim; i++) {
		_frames.push_back(AnimationFrame());
		image_frames[i].SetDimensions(_width, _height);
		_frames.back().image = image_frames[i];
		_frames.back().frame_time = timings[i];
		if (timings[i] == 0) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "added a frame time value of zero when loading file: " << filename << endl;
		}
	}

	return true;
} // bool AnimatedImage::LoadFromFrameSize(...)



bool AnimatedImage::LoadFromFrameGrid(const string& filename, const vector<uint32>& timings, const uint32 frame_rows, const uint32 frame_cols, const uint32 trim) {
	if (trim >= frame_rows * frame_cols) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "attempt to trim away more frames than requested to load for file: " << filename << endl;
		return false;
	}

	if (timings.size() < (frame_rows * frame_cols - trim)) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "not enough timing data to fill frames grid when loading file: " << filename << endl;
		return false;
	}

	_frames.clear();
	ResetAnimation();

	// Make the multi image call
	// TODO: Handle the case where the _grayscale member is true so all frames are loaded in grayscale format
	vector<StillImage> image_frames;
	if (ImageDescriptor::LoadMultiImageFromElementGrid(image_frames, filename, frame_rows, frame_cols) == false) {
		return false;
	}

	// Add the loaded frame image and timing information
	for (uint32 i = 0; i < frame_rows * frame_cols - trim; i++) {
		_frames.push_back(AnimationFrame());
		image_frames[i].SetDimensions(_width, _height);
		_frames.back().image = image_frames[i];
		_frames.back().frame_time = timings[i];
		if (timings[i] == 0) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "added zero frame time for an image frame when loading file: " << filename << endl;
		}
	}

	return true;
} // bool AnimatedImage::LoadFromFrameGrid(...)



void AnimatedImage::Draw() const {
	if (_frames.empty()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "no frames were loaded into the AnimatedImage object" << endl;
		return;
	}

	_frames[_frame_index].image.Draw();
}



void AnimatedImage::Draw(const Color& draw_color) const {
	if (_frames.empty()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "no frames were loaded into the AnimatedImage object" << endl;
		return;
	}

	_frames[_frame_index].image.Draw(draw_color);
}



bool AnimatedImage::Save(const std::string& filename, uint32 grid_rows, uint32 grid_cols) const {
	vector<StillImage*> image_frames;
	for (uint32 i = 0; i < _frames.size(); i++) {
		image_frames.push_back(const_cast<StillImage*>(&(_frames[i].image)));
	}

	if (grid_rows == 0 || grid_cols == 0) {
		return ImageDescriptor::SaveMultiImage(image_frames, filename, 1, _frames.size());
	}
	else {
		return ImageDescriptor::SaveMultiImage(image_frames, filename, grid_rows, grid_cols);
	}
}



void AnimatedImage::EnableGrayScale() {
	if (_grayscale == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "grayscale mode was already enabled when function was invoked" << endl;
		return;
	}

	_grayscale = true;
	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i].image.EnableGrayScale();
	}
}



void AnimatedImage::DisableGrayScale() {
	if (_grayscale == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "grayscale mode was already disabled when function was invoked" << endl;
		return;
	}

	_grayscale = false;
	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i].image.DisableGrayScale();
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
} // void AnimatedImage::Update()



bool AnimatedImage::AddFrame(const string& frame, uint32 frame_time) {
	StillImage img;
	img.SetStatic(_is_static);
	img.SetVertexColors(_color[0], _color[1], _color[2], _color[3]);
	if (img.Load(frame, _width, _height) == false) {
		return false;
	}
	
	AnimationFrame new_frame;
	new_frame.frame_time = frame_time;
	new_frame.image = img;
	_frames.push_back(new_frame);
	return true;
}



bool AnimatedImage::AddFrame(const StillImage& frame, uint32 frame_time) {
	if (frame.GetNumElements() == 0) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "StillImage argument did not contain any image elements" << endl;
		return false;
	}

	AnimationFrame new_frame;
	new_frame.image = frame;
	new_frame.frame_time = frame_time;
	
	_frames.push_back(new_frame);
	return true;
}



void AnimatedImage::SetWidth(float width) {
	_width = width;

	for (uint32 i = 0; i < _frames.size(); ++i) {
		_frames[i].image.SetWidth(width);
	}
}



void AnimatedImage::SetHeight(float height) {
	_height = height;

	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i].image.SetHeight(height);
	}
}



void AnimatedImage::SetDimensions(float width, float height) {
	_width = width;
	_height = height;

	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i].image.SetDimensions(width, height);
	}
}



void AnimatedImage::SetColor(const Color &color) {
	_color[0] = color;
	_color[1] = color;
	_color[2] = color;
	_color[3] = color;

	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i].image.SetColor(color);
	}
}



void AnimatedImage::SetVertexColors(const Color &tl, const Color &tr, const Color &bl, const Color &br) {
	_color[0] = tl;
	_color[1] = tr;
	_color[2] = bl;
	_color[3] = br;

	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i].image.SetVertexColors(tl, tr, bl, br);
	}
}

}  // namespace hoa_video
