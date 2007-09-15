///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    image_base.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for image base classes
*** ***************************************************************************/

#include <cstdarg>
#include <math.h>

#include "utils.h"
#include "image_base.h"
#include "video.h"
#include "gui.h"

using namespace std;
using namespace hoa_utils;

namespace hoa_video {

namespace private_video {

// -----------------------------------------------------------------------------
// ImageMemory class
// -----------------------------------------------------------------------------

ImageMemory::ImageMemory() :
	width(0),
	height(0),
	pixels(NULL),
	rgb_format(false)
{}



ImageMemory::~ImageMemory() {
	if (pixels != NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "pixels member was not NULL upon object destruction" << endl;
		// TODO: fix all areas in the video engine where it frees pixels but does not set the pointer to NULL
// 		free(pixels);
// 		pixels = NULL;
	}
}



bool ImageMemory::LoadImage(const string& filename) {
	if (pixels != NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "pixels member was not NULL upon function invocation" << endl;
		free(pixels);
		pixels = NULL;
	}

	// Isolate the extension
	size_t ext_position = filename.rfind('.');

	if (ext_position == string::npos) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "could not decipher file extension for filename: " << filename << endl;
		return false;
	}

	string extension = string(filename, ext_position, filename.length() - ext_position);

	// NOTE: We could technically try uppercase forms of the file extension, or also include the .jpeg extension name,
	// but Allacrost's file standard states that only the .png and .jpg image file extensions are suppported.
	if (extension == ".png")
		return _LoadPngImage(filename);
	else if (extension == ".jpg")
		return _LoadJpgImage(filename);

	IF_PRINT_WARNING(VIDEO_DEBUG) << "unsupported file extension: \"" << extension << "\" for filename: " << filename << endl;
	return false;
}



bool ImageMemory::SaveImage(const string& filename, bool png_image) {
	if (pixels == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "pixels member was NULL upon function invocation for file: " << filename << endl;
		return false;
	}

	if (png_image) {
		return _SavePngImage(filename);
	}
	else {
		// JPG images don't have alpha information, so we must convert the data to RGB format first
		if (rgb_format == false)
			RGBAToRGB();
		return _SaveJpgImage(filename);
	}
}



void ImageMemory::ConvertToGrayscale() {
	if (width <= 0 || height <= 0) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "width and/or height members were invalid (<= 0)" << endl;
		return;
	}

	if (pixels == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "no image data (pixels == NULL)" << endl;
		return;
	}

	uint8 format_bytes = (rgb_format ? 3 : 4);
	uint8* end_position = static_cast<uint8*>(pixels) + (width * height * format_bytes);

	for (uint8* i = static_cast<uint8*>(pixels); i < end_position; i += format_bytes) {
		// Compute the grayscale value for this pixel based on RGB values: 0.30R + 0.59G + 0.11B
		uint8 value = static_cast<uint8>((30 * *(i) + 59 * *(i + 1) + 11 * *(i + 2)) * 0.01f);
		*i = value;
		*(i + 1) = value;
		*(i + 2) = value;
		// *(i + 3) for RGBA is the alpha value and is left unmodified
	}
}


void ImageMemory::RGBAToRGB() {
	if (width <= 0 || height <= 0) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "width and/or height members were invalid (<= 0)" << endl;
		return;
	}

	if (pixels == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "no image data (pixels == NULL)" << endl;
		return;
	}

	if (rgb_format == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "image data was said to already be in RGB format" << endl;
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
	rgb_format = true;
}



void ImageMemory::CopyFromTexture(TexSheet* texture) {
	if (pixels != NULL)
		free(pixels);
	pixels = NULL;

	// Get the texture as a buffer
	height = texture->height;
	width = texture->width;
	pixels = malloc(height * width * (rgb_format ? 3 : 4));
	if (pixels == NULL) {
		PRINT_ERROR << "failed to malloc enough memory to copy the texture" << endl;
	}

	TextureManager->_BindTexture(texture->tex_id);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}



void ImageMemory::CopyFromImage(BaseImageTexture* img) {
	// First copy the image's entire texture sheet to memory
	CopyFromTexture(img->texture_sheet);

	// Check that the image to copy is smaller than its texture sheet (usually true).
	// If so, then copy over only the sub-rectangle area of the image from its texture
	if (height > img->height || width > img->width) {
		uint8 format_bytes = (rgb_format ? 3 : 4);
		uint32 src_bytes = width * format_bytes;
		uint32 dst_bytes = img->width * format_bytes;
		uint32 src_offset = img->y * width * format_bytes + img->x * format_bytes;
		void* img_pixels = malloc(img->width * img->height * format_bytes);
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

		height = img->height;
		width = img->width;
		pixels = img_pixels;
	}
}



bool ImageMemory::_LoadPngImage(const string& filename) {
	//! \todo Someone who understands the libpng library needs to add some comments to this function
	FILE* fp = fopen(filename.c_str(), "rb");

	if (fp == NULL)
		return false;

	uint8 test_buffer[8];

	fread(test_buffer, 1, 8, fp);
	if (png_sig_cmp(test_buffer, 0, 8)) {
		fclose(fp);
		return false;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);

	if (png_ptr == NULL) {
		fclose(fp);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, (png_infopp)NULL);
		fclose(fp);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, NULL, (png_infopp)NULL);
		fclose(fp);
		return false;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);

	uint8** row_pointers = png_get_rows(png_ptr, info_ptr);

	width = info_ptr->width;
	height = info_ptr->height;
	pixels = malloc(info_ptr->width * info_ptr->height * 4);

	if (pixels == NULL) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose(fp);
		PRINT_ERROR << "failed to malloc sufficient memory for .png file: " << filename << endl;
		return false;
	}
	
	uint32 bpp = info_ptr->channels;
	uint8* img_pixel = NULL;
	uint8* dst_pixel = NULL;

	if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE) {
		png_color c;
		for (uint32 y = 0; y < info_ptr->height; y++) {
			for (uint32 x = 0; x < info_ptr->width; x++) {
				img_pixel = row_pointers[y] + (x * bpp);
				dst_pixel = ((uint8*)pixels) + ((y * info_ptr->width) + x) * 4;
				c = info_ptr->palette[img_pixel[0]];

				dst_pixel[0] = c.red;
				dst_pixel[1] = c.green;
				dst_pixel[2] = c.blue;
				dst_pixel[3] = 0xFF;
			}
		}
	}
	else if (bpp == 1) {
		for (uint32 y = 0; y < info_ptr->height; y++) {
			for (uint32 x = 0; x < info_ptr->width; x++) {
				img_pixel = row_pointers[y] + (x * bpp);
				dst_pixel = ((uint8*)pixels) + ((y * info_ptr->width) + x) * 4;
				dst_pixel[0] = img_pixel[0];
				dst_pixel[1] = img_pixel[0];
				dst_pixel[2] = img_pixel[0];
				dst_pixel[3] = 0xFF;
			}
		}
	}
	else if (bpp == 3) {
		for (uint32 y = 0; y < info_ptr->height; y++) {
			for (uint32 x = 0; x < info_ptr->width; x++) {
				img_pixel = row_pointers[y] + (x * bpp);
				dst_pixel = ((uint8*)pixels) + ((y * info_ptr->width) + x) * 4;
				dst_pixel[0] = img_pixel[0];
				dst_pixel[1] = img_pixel[1];
				dst_pixel[2] = img_pixel[2];
				dst_pixel[3] = 0xFF;
			}
		}
	}
	else if (bpp == 4) {
		for (uint32 y = 0; y < info_ptr->height; y++) {
			for (uint32 x = 0; x < info_ptr->width; x++) {
				img_pixel = row_pointers[y] + (x * bpp);
				dst_pixel = ((uint8*)pixels) + ((y * info_ptr->width) + x) * 4;
				dst_pixel[0] = img_pixel[0];
				dst_pixel[1] = img_pixel[1];
				dst_pixel[2] = img_pixel[2];
				dst_pixel[3] = img_pixel[3];
			}
		}
	}
	else {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose(fp);
		PRINT_ERROR << "failed to load .png file (bytes per pixel not supported): " << filename << endl;
		return false;
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	fclose(fp);

	rgb_format = false;
	return true;
} // bool ImageMemory::_LoadPngImage(const string& filename)



bool ImageMemory::_LoadJpgImage(const string& filename) {
	//! \todo Someone who understands the libjpeg library needs to add some comments to this function
	FILE* fp;
	uint8** buffer;

	if ((fp = fopen(filename.c_str(), "rb")) == NULL)
		return false;

	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);

	jpeg_start_decompress(&cinfo);

	JDIMENSION row_stride = cinfo.output_width * cinfo.output_components;

	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	width = cinfo.output_width;
	height = cinfo.output_height;
	pixels = malloc(cinfo.output_width * cinfo.output_height * 3);

	uint32 bpp = cinfo.output_components;
	uint8* img_pixel = NULL;
	uint8* dst_pixel = NULL;

	if (bpp == 3) {
		for (uint32 y = 0; y < cinfo.output_height; y++) {
			jpeg_read_scanlines(&cinfo, buffer, 1);

			for(uint32 x = 0; x < cinfo.output_width; x++) {
				img_pixel = buffer[0] + (x * bpp);
				dst_pixel = ((uint8 *)pixels) + ((y * cinfo.output_width) + x) * 3;

				dst_pixel[0] = img_pixel[0];
				dst_pixel[1] = img_pixel[1];
				dst_pixel[2] = img_pixel[2];
			}
		}
	}
	else if (bpp == 4) {
		for (uint32 y = 0; y < cinfo.output_height; y++) {
			jpeg_read_scanlines(&cinfo, buffer, 1);

			for (uint32 x = 0; x < cinfo.output_width; x++) {
				img_pixel = buffer[0] + (x * bpp);
				dst_pixel = ((uint8 *)pixels) + ((y * cinfo.output_width) + x) * 3;

				dst_pixel[0] = img_pixel[0];
				dst_pixel[1] = img_pixel[1];
				dst_pixel[2] = img_pixel[2];
			}
		}
	}
	else {
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		fclose(fp);
		PRINT_ERROR << "failed to load .jpg file (bytes per pixel not supported): " << filename << endl;
		return false;
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	fclose(fp);
	rgb_format = true;
	return true;
} // bool ImageMemory::_LoadJpgImage(const string& filename)



bool ImageMemory::_SavePngImage(const std::string& filename) const {
	//! \todo Someone who understands the libpng library needs to add some comments to this function
	FILE* fp = fopen(filename.c_str(), "wb");

	if (fp == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "could not open file: " << filename << endl;
		return false;
	}

	if (rgb_format == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "attempting to save RGB format image data as a RGBA format PNG image" << endl;
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);

	if (!png_ptr) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "png_create_write_struct() failed for file: " << filename << endl;
		fclose(fp);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "png_create_info_struct() failed for file: " << filename << endl;
		png_destroy_write_struct(&png_ptr, NULL);
		fclose(fp);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "setjmp returned non-zero for file: " << filename << endl;
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return false;
	}

	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_byte** row_pointers = new png_byte*[height];
	int32 bytes_per_row = width * 4;
	for (int32 i = 0; i < height; i++) {
		row_pointers[i] = (png_byte*)pixels + bytes_per_row * i;
	}

	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	delete[] row_pointers;

	return true;
} // bool ImageMemory::_SavePngImage(const std::string& filename) const



bool ImageMemory::_SaveJpgImage(const std::string& filename) const {
	//! \todo Someone who understands the libjpeg library needs to add some comments to this function
	FILE* fp = fopen(filename.c_str(), "wb");
	if (fp == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "could not open file: " << filename << endl;
		return false;
	}

	if (rgb_format == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "attempting to save non-RGB format pixel data as a RGB format JPG image" << endl;
	}

	jpeg_compress_struct cinfo;
	jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	cinfo.in_color_space = JCS_RGB;
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;

	jpeg_set_defaults(&cinfo);
	jpeg_stdio_dest(&cinfo, fp);
	jpeg_start_compress(&cinfo, TRUE);

	JSAMPROW row_pointer; // A pointer to a single row
	uint32 row_stride = width * 3; // The physical row width in the buffer (RGB)

	// Note that the lines have to be stored from top to bottom
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer = (uint8*)pixels + cinfo.next_scanline * row_stride;
	    jpeg_write_scanlines(&cinfo, &row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	fclose(fp);
	return true;
} // bool ImageMemory::_SaveJpgImage(const std::string& file_name) const

// -----------------------------------------------------------------------------
// BaseImageTexture class
// -----------------------------------------------------------------------------

BaseImageTexture::BaseImageTexture() :
	texture_sheet(NULL),
	width(0),
	height(0),
	x(0),
	y(0),
	u1(0.0f),
	v1(0.0f),
	u2(0.0f),
	v2(0.0f),
	smooth(false),
	ref_count(0)
{}



BaseImageTexture::BaseImageTexture(int32 width_, int32 height_) :
	texture_sheet(NULL),
	width(width_),
	height(height_),
	x(0),
	y(0),
	u1(0.0f),
	v1(0.0f),
	u2(0.0f),
	v2(0.0f),
	smooth(false),
	ref_count(0)
{}



BaseImageTexture::BaseImageTexture(TexSheet* texture_sheet_, int32 width_, int32 height_) :
	texture_sheet(texture_sheet_),
	width(width_),
	height(height_),
	x(0),
	y(0),
	u1(0.0f),
	v1(0.0f),
	u2(0.0f),
	v2(0.0f),
	smooth(false),
	ref_count(0)
{}



BaseImageTexture::~BaseImageTexture() {
	if (ref_count > 0) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "destructor invoked when the object had a reference count greater than zero: " << ref_count << endl;
	}
}

// -----------------------------------------------------------------------------
// ImageTexture class
// -----------------------------------------------------------------------------

ImageTexture::ImageTexture(const string& filename_, const string& tags_, int32 width_, int32 height_) :
	BaseImageTexture(width_, height_),
	filename(filename_),
	tags(tags_)
{}



ImageTexture::ImageTexture(TexSheet* texture_sheet_, const string& filename_, const string& tags_, int32 width_, int32 height_) :
	BaseImageTexture(texture_sheet_, width_, height_),
	filename(filename_),
	tags(tags_)
{}



ImageTexture::~ImageTexture() {
	if (TextureManager->_images.empty()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "TextureManager _images container was empty upon destructor invocation" << endl;
		return;
	}

	// Search the map of images in the TextureManager for this instance and remove it
	for (map<string, ImageTexture*>::iterator i = (TextureManager->_images.begin()); i != (TextureManager->_images.end()); i++) {
		if (i->second == this) {
			TextureManager->_images.erase(i);
			break;
		}
	}
}

// -----------------------------------------------------------------------------
// BaseImageElement class
// -----------------------------------------------------------------------------

BaseImageElement::BaseImageElement(float width_, float height_, float x_offset_, float y_offset_,
	float u1_, float v1_, float u2_, float v2_) :
	width(width_),
	height(height_),
	x_offset(x_offset_),
	y_offset(y_offset_),
	u1(u1_),
	v1(v1_),
	u2(u2_),
	v2(v2_),
	blend(false),
	unichrome_vertices(true),
	white_vertices(true),
	base_image(NULL)
{
	color[0] = Color::white;
	color[1] = Color::white;
	color[2] = Color::white;
	color[3] = Color::white;
}



BaseImageElement::BaseImageElement(float width_, float height_, float x_offset_, float y_offset_,
	float u1_, float v1_, float u2_, float v2_, Color color_[4]) :
	width(width_),
	height(height_),
	x_offset(x_offset_),
	y_offset(y_offset_),
	u1(u1_),
	v1(v1_),
	u2(u2_),
	v2(v2_),
	blend(false),
	unichrome_vertices(false),
	white_vertices(false),
	base_image(NULL)
{
	color[0] = color_[0];
	color[1] = color_[1];
	color[2] = color_[2];
	color[3] = color_[3];
	
	// If all colors are the same, then mark it so we don't have to process all vertex colors
	if (color[0] == color[1] && color[0] == color[2] && color[0] == color[3]) {
		unichrome_vertices = true;

		// If all vertex colors are white, set a flag so they don't have to be processed at all
		if (color[0] == Color::white) {
			white_vertices = true;
			blend = false;
		}
		// Set blend to true if the alpha is less than 1.0f (opaque)
		else {
			blend = (color[0][3] < 1.0f);
		}
	}
	else {
		// Set blend to true if any of the four colors have an alpha value less than 1.0f (opaque)
		blend = (color[0][3] < 1.0f || color[1][3] < 1.0f || color[2][3] < 1.0f || color[3][3] < 1.0f);
	}
}



BaseImageElement::~BaseImageElement() {
	if (base_image != NULL) {
		if (base_image->RemoveReference() == true) {
			// If the image exceeds 512 in either width or height, it has an un-shared texture sheet, which we
			// should now delete that the image is being removed
			if (base_image->width > 512 || base_image->height > 512) {
				// Remove the image and texture sheet completely
				// TODO: This introduces a seg fault when TexSheet::FreeImage is later called. Fix this bug!
				base_image->texture_sheet->RemoveImage(base_image);
				TextureManager->_RemoveSheet(base_image->texture_sheet);
			}
			else {
				// Otherise simply mark the image as free in the texture sheet
				base_image->texture_sheet->FreeImage(base_image);
			}
		}
	}
}



BaseImageElement::BaseImageElement(const BaseImageElement& copy) :
	width(copy.width),
	height(copy.height),
	x_offset(copy.x_offset),
	y_offset(copy.y_offset),
	u1(copy.u1),
	v1(copy.v1),
	u2(copy.u2),
	v2(copy.v2),
	//color(copy.color),
	blend(copy.blend),
	unichrome_vertices(copy.unichrome_vertices),
	white_vertices(copy.white_vertices),
	base_image(copy.base_image)
{
	color[0] = copy.color[0];
	color[1] = copy.color[1];
	color[2] = copy.color[2];
	color[3] = copy.color[3];
	if (base_image != NULL)
		base_image->AddReference();
}



BaseImageElement& BaseImageElement::operator=(const BaseImageElement& copy) {
	// Handle the case were a dumbass assigns an object to itself
	if (this == &copy) {
		return *this;
	}

	width = copy.width;
	height = copy.height;
	x_offset = copy.x_offset;
	y_offset = copy.y_offset;
	u1 = copy.u1;
	v1 = copy.v1;
	u2 = copy.u2;
	v2 = copy.v2;
	color[0] = copy.color[0];
	color[1] = copy.color[1];
	color[2] = copy.color[2];
	color[3] = copy.color[3];
	blend = copy.blend;
	unichrome_vertices = copy.unichrome_vertices;
	white_vertices = copy.white_vertices;

	// Update the reference to the old image texture and possibly free it from texture memory
	// Case 1: We previously pointed to a valid image texture and the copy does not point to the same texture
	if (base_image != NULL && copy.base_image != base_image) {
		if (base_image->RemoveReference() == true) {
			// If the image exceeds 512 in either width or height, it has an un-shared texture sheet, which we
			// should now delete that the image is being removed
			if (base_image->width > 512 || base_image->height > 512) {
				// Remove the image and texture sheet completely
				// TODO: This introduces a seg fault when TexSheet::FreeImage is later called. Fix this bug!
				base_image->texture_sheet->RemoveImage(base_image);
				TextureManager->_RemoveSheet(base_image->texture_sheet);
			}
			else {
				// Otherise simply mark the image as free in the texture sheet
				base_image->texture_sheet->FreeImage(base_image);
			}
		}
	}
	// Case 2: The original image texture was NULL and the copy is not NULL, so increment the reference
	else if (copy.base_image != NULL) {
		copy.base_image->AddReference();
	}

	base_image = copy.base_image;

	return *this;
}



void BaseImageElement::Draw(const Color* color_array) const {
	// If no color array was passed, use the element's own vertex colors
	if (color_array == NULL) {
		color_array = color;
	}

	// Array of the four vertexes defined on the 2D plane for glDrawArrays()
	static const float vert_coords[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
	};

	// Set blending parameters
	if (VideoManager->_current_context.blend) {
		glEnable(GL_BLEND);
		if (VideoManager->_current_context.blend == 1)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal blending
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending
	}
	else {
		// If blending isn't in the draw flags, don't use blending UNLESS the given image element has translucent vertex colors
		if (blend == false)
			glDisable(GL_BLEND);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal blending
	}

	// If we have a valid image texture poiner, setup texture coordinates and the texture coordinate array for glDrawArrays()
	if (base_image != NULL) {
		// Set the texture coordinates
		float s0, s1, t0, t1;

		s0 = base_image->u1 + u1 * (base_image->u2 - base_image->u1);
		s1 = base_image->u1 + u2 * (base_image->u2 - base_image->u1);
		t0 = base_image->v1 + v1 * (base_image->v2 - base_image->v1);
		t1 = base_image->v1 + v2 * (base_image->v2 - base_image->v1);

		// Swap x texture coordinates if x flipping is enabled
		if (VideoManager->_current_context.x_flip) {
			float temp = s0;
			s0 = s1;
			s1 = temp;
		}

		// Swap y texture coordinates if y flipping is enabled
		if (VideoManager->_current_context.y_flip) {
			float temp = t0;
			t0 = t1;
			t1 = temp;
		}

		// Place the texture coordinates in a 4x2 array mirroring the structure of the vertex array for use in glDrawArrays().
		float tex_coords[] = {
			s0, t1,
			s1, t1,
			s1, t0,
			s0, t0,
		};

		// Enable texturing and bind texture
		glEnable(GL_TEXTURE_2D);
		TextureManager->_BindTexture(base_image->texture_sheet->tex_id);
		base_image->texture_sheet->Smooth(base_image->smooth);

		// Enable and setup the texture coordinate array
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);

		if (unichrome_vertices == true) {
			glColor4fv((GLfloat*)color_array[0].GetColors());
		}
		else {
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4, GL_FLOAT, 0, (GLfloat*)color_array);
		}
	} // if (img != NULL)

	// Otherwise there is no image, so we're drawing pure color on the vertices
	else {
		// Use a single call to glColor for unichrome images, or a setup a gl color array for multiple colors
		if (unichrome_vertices == true) {
			glColor4fv((GLfloat*)color_array[0].GetColors());
			glDisableClientState(GL_COLOR_ARRAY);
		}
		else {
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4, GL_FLOAT, 0, (GLfloat*)color_array);
		}

		// Disable texturing as we're using pure colour
		glDisable(GL_TEXTURE_2D);
	}

	// Use a vertex array to draw all of the vertices
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vert_coords);
	glDrawArrays(GL_QUADS, 0, 4);

	if (VideoManager->_current_context.blend)
		glDisable(GL_BLEND);

	if (VideoManager->CheckGLError() == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "an OpenGL error occurred: " << VideoManager->CreateGLErrorString() << endl;
	}
} // void BaseImageElement::Draw(const Color* color_array) const


void BaseImageElement::DEBUG_PrintInfo() {
	cout << "BaseImageElement properties:" << endl;
	cout << "* width:                " << width << endl;
	cout << "* height:               " << height << endl;
	cout << "* x offset:             " << x_offset << endl;
	cout << "* y offset:             " << y_offset << endl;
	cout << "* uv coords (1, 2):      (" << u1 << "," << v1 << ") (" << u2 << ", " << v2 << ")" << endl;
	cout << "* colors, RGBA format:  " << endl;
	cout << "  * TL                  " << color[0].GetRed() << ", " << color[0].GetGreen() << ", " << color[0].GetBlue() << ", " << color[0].GetAlpha() << endl;
	cout << "  * TR                  " << color[1].GetRed() << ", " << color[1].GetGreen() << ", " << color[1].GetBlue() << ", " << color[1].GetAlpha() << endl;
	cout << "  * BL                  " << color[2].GetRed() << ", " << color[2].GetGreen() << ", " << color[2].GetBlue() << ", " << color[2].GetAlpha() << endl;
	cout << "  * BR:                 " << color[3].GetRed() << ", " << color[3].GetGreen() << ", " << color[3].GetBlue() << ", " << color[3].GetAlpha() << endl;
	cout << "* blend:                " << (blend ? "true" : "false") << endl;
	cout << "* unichrome vertices:   " << (unichrome_vertices ? "true" : "false") << endl;
	cout << "* white vertices:       " << (white_vertices ? "true" : "false") << endl;
	cout << endl;
}

// -----------------------------------------------------------------------------
// ImageElement class
// -----------------------------------------------------------------------------

ImageElement::ImageElement(ImageTexture* image_, float width_, float height_, float x_offset_, float y_offset_,
		float u1_, float v1_, float u2_, float v2_) :
	BaseImageElement(width_, height_, x_offset_, y_offset_, u1_, v1_, u2_, v2_),
	image(image_)
{
	base_image = image_;
	if (base_image != NULL)
		base_image->AddReference();
}



ImageElement::ImageElement(ImageTexture *image_, float width_, float height_, float x_offset_, float y_offset_, 
		float u1_, float v1_, float u2_, float v2_, Color color_[4]) :
	BaseImageElement(width_, height_, x_offset_, y_offset_, u1_, v1_, u2_, v2_, color_),
	image(image_)
{
	base_image = image_;
	if (base_image != NULL)
		base_image->AddReference();
}

} // namespace private_video

} // namespace hoa_video
