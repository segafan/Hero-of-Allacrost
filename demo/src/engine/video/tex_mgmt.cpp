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
#include <set>
#include "video.h"
#include <math.h>
#include "gui.h"

using namespace std;
using namespace hoa_video::private_video;
using namespace hoa_video;
using namespace hoa_utils;

namespace hoa_video 
{


//!	\brief Converts an integer to string
void IntegerToString(std::string &s, const int32 num)
{
	static char digits[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

	if (num == 0)
	{
		s = "0";
		return;
	}

	s.clear ();
	uint32 value = abs(num);

	while (value > 0)
	{
		s = s + digits[value%10];
		value /= 10;
	}

	if (num < 0)
	{
		s = s + "-";
	}
}


//-----------------------------------------------------------------------------
// LoadImage: loads an image (static image or animated image) and returns true
//            on success
//-----------------------------------------------------------------------------

bool GameVideo::LoadImage(ImageDescriptor &id)
{
	if(id._animated)
	{
		if (id.IsGrayScale())
			return _LoadImage(dynamic_cast<AnimatedImage &>(id), true);
		else
			return _LoadImage(dynamic_cast<AnimatedImage &>(id));
	}
	else
	{
		if (id.IsGrayScale())
			return _LoadImage(dynamic_cast<StillImage &>(id), true);
		else
			return _LoadImage(dynamic_cast<StillImage &>(id));
	}
}

//-----------------------------------------------------------------------------
// _LoadImage: helper function to load an animated image
//-----------------------------------------------------------------------------
bool GameVideo::_LoadImage(AnimatedImage &id, bool grayscale)
{
	uint32 numFrames = static_cast<uint32>(id._frames.size());
	
	bool success = true;
	
	// go through all the frames and load anything that hasn't already been loaded
	for(uint32 frame = 0; frame < numFrames; ++frame)
	{
		// if the API user passes only filenames to AddFrame(), then we need to load
		// the images, but if a static image is passed directly, then we can skip
		// loading
		
		bool needToLoad = id._frames[frame]._image._elements.empty();
		
		if(needToLoad)
		{
			success &= _LoadImage(id._frames[frame]._image, grayscale);
		}
	}
	
	return success;
}

//-----------------------------------------------------------------------------
// _LoadImage: loads an image and returns it in the static image
//             On failure, returns false.
//
//             If isStatic is true, that means this is an image that is probably
//             to remain in memory for the entire game, so place it in a
//             special texture sheet reserved for things that don't change often.
//-----------------------------------------------------------------------------
bool GameVideo::_LoadImage(StillImage &id, bool grayscale)
{
	// 1. special case: if filename is empty, load a colored quad
	if(id._filename.empty())
	{
		id._elements.clear();		
		ImageElement quad(NULL, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, id._width, id._height, id._color);
		id._elements.push_back(quad);	
		return true;
	}
	
	// 2. check if an image with the same filename has already been loaded
	//    If so, point to that
	if(_images.find(id._filename + (grayscale ? string("_grayscale") : string(""))) != _images.end())
	{
		id._elements.clear();		
		
		Image *img = _images[id._filename];
		
		if(!img)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: got a NULL Image from images map in LoadImage()" << endl;
			return false;
		}

		if (img->ref_count == 0)
		{
			// if ref count is zero, it means this image was freed, but
			// not removed, so restore it
			if(!img->texture_sheet->RestoreImage(img))
				return false;
		}
		
		++(img->ref_count);
		
		if(id._width == 0.0f)
			id._width = (float) img->width;
		if(id._height == 0.0f)
			id._height = (float) img->height;
		
		ImageElement element(img, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, id._width, id._height, id._color);
		id._elements.push_back(element);
				
		return true;
	}

	// 3. If we're currently between a call of BeginTexLoadBatch() and
	//    EndTexLoadBatch(), then instead of loading right now, push it onto
	//    the batch vector so it can be processed at EndTexLoadBatch()
	if(_batchLoading)
	{
		_batchLoadImages.push_back(&id);
		return true;
	}

	// 4. If we're not batching, then load the image right away
	bool success = _LoadImageHelper(id, grayscale);
	
	if(!success)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: in LoadImage() failed to load " << id._filename << endl;
		return false;
	}
	
	return success;
}



bool GameVideo::LoadMultiImage(std::vector<StillImage> &id, const std::string filename, const uint32 rows, const uint32 cols, const float width, const float height)
{
	bool success = true;

	private_video::MultiImage image (id, filename, rows, cols, width, height, false);


	if (_batchLoading)
	{
		_batchLoadMultiImages.push_back (image);
	}
	else
	{
		success = _LoadMultiImage (image);
	}

	return success;
}

	
bool GameVideo::LoadAnimatedImage(AnimatedImage &id, const std::string filename, const uint32 rows, const uint32 cols, const float width, const float height)
{
	bool success = true;

	std::vector <StillImage> v;
	private_video::MultiImage image (v, filename, rows, cols, width, height, false);

	if (_batchLoading)
	{
		_batchLoadMultiImages.push_back (image);
	}
	else
	{
		success = _LoadMultiImage (image);
	}

	// Attach the images to the frames of the animated image
	if (success)
	{
		for (uint32 i=0; i<rows*cols; i++)
		{
			id.AddFrame (v[i], i*5);
		}
	}

	return success;
}


bool GameVideo::_LoadMultiImage (private_video::MultiImage &id)
{
	std::string filename = id.filename;
	uint32 rows = id.rows;
	uint32 cols = id.cols;
	bool grayscale = id.grayscale;
	std::vector <StillImage>& images = *id.still_images;

	if (filename.empty())
	{
		cerr << "Unexpected error loading multi image" << endl;
		return false;
	}

	std::string image_name;
	std::string s;
	uint32 current_image;
	uint32 x, y;

	images.resize (rows * cols);

	// Check if we have loaded all the sub-images
	for (x=0; x<rows; x++)
	{
		for (y=0; y<rows; y++)
		{
			image_name = filename;
			IntegerToString(s,x);
			image_name += "_X" + s;
			IntegerToString(s,y);
			image_name += "_Y" + s;
			image_name += (grayscale ? string("_grayscale") : string(""));
		
			// If this image exists, don't do anything else
			if(_images.find(image_name) == _images.end())
			{
				break;
			}
		}
	}

	// If not all the images are loaded, then load the image
	private_video::ImageLoadInfo loadInfo;
	if (x*y != rows*cols)
	{
		if(!_LoadRawImage(filename, loadInfo, grayscale))
			return false;
	}

	for (x=0; x<rows; x++)
	{
		for (y=0; y<cols; y++)
		{
			current_image = x*cols + y;
			image_name = filename;
			IntegerToString(s,x);
			image_name += "_X" + s;
			IntegerToString(s,y);
			image_name += "_Y" + s;
			image_name += (grayscale ? string("_grayscale") : string(""));

			// If the image exists, take the information from it
			if(_images.find(image_name) != _images.end())
			{
				images.at(current_image)._elements.clear();
		
				Image *img = _images[images.at(current_image)._filename];
		
				if(!img)
				{
					if(VIDEO_DEBUG)
						cerr << "VIDEO ERROR: got a NULL Image from images map in LoadImage()" << endl;
					return false;
				}

				if(img->ref_count == 0)
				{
					// if ref count is zero, it means this image was freed, but
					// not removed, so restore it
					if(!img->texture_sheet->RestoreImage(img))
					{
						return false;
					}
				}
			
				++(img->ref_count);
		
				if(images.at(current_image)._width < 0.0f)
					images.at(current_image)._width = (float) img->width;
				if(images.at(current_image)._height < 0.0f)
					images.at(current_image)._height = (float) img->height;
		
				ImageElement element(img, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f,
					images.at(current_image)._width, images.at(current_image)._height, images.at(current_image)._color);
				images.at(current_image)._elements.push_back(element);
			}
			else	// If the image is not present, take the piece from the loaded image
			{
				images.at(current_image)._filename = image_name;
				images.at(current_image)._elements.clear();
				images.at(current_image)._animated = false;
				images.at(current_image)._grayscale = grayscale;
				images.at(current_image)._is_static = false;
				if (id.height == 0.0f)
					images.at(current_image)._height = (float)((x == rows-1 && loadInfo.height%rows) ? loadInfo.height-(x*loadInfo.height/rows) : loadInfo.height/rows);
				else
					images.at(current_image)._height = id.height;
				if (id.width == 0.0f)
					images.at(current_image)._width = (float)((y == cols-1 && loadInfo.width%cols) ? loadInfo.width-(y*loadInfo.width/cols) : loadInfo.width/cols);
				else
					images.at(current_image)._width = id.width;

				private_video::ImageLoadInfo info;
				info.width = ((y == cols-1 && loadInfo.width%cols) ? loadInfo.width-(y*loadInfo.width/cols) : loadInfo.width/cols);
				info.height = ((x == rows-1 && loadInfo.height%rows) ? loadInfo.height-(x*loadInfo.height/rows) : loadInfo.height/rows);
				info.pixels = new uint8 [info.width*info.height*4];
				for (int i=0; i<info.height; i++)
				{
					memcpy ((uint8*)info.pixels+4*info.width*i, (uint8*)loadInfo.pixels+(((x*loadInfo.height/rows)+i)*loadInfo.width+y*loadInfo.width/cols)*4, 4*info.width);
				}
				
				// create an Image structure and store it our std::map of images
				Image *newImage = new Image(image_name, info.width, info.height, grayscale);

				// try to insert the image in a texture sheet
				TexSheet *sheet = _InsertImageInTexSheet(newImage, info, images.at(current_image)._is_static);
				
				if(!sheet)
				{
					// this should never happen, unless we run out of memory or there
					// is a bug in the _InsertImageInTexSheet() function
		
					if(VIDEO_DEBUG)
					cerr << "VIDEO_DEBUG: GameVideo::_InsertImageInTexSheet() returned NULL!" << endl;
		
					free(info.pixels);
					return false;
				}
				
				newImage->ref_count = 1;
				
				// store the image in our std::map
				_images[image_name] = newImage;

				// store the new image element
				ImageElement element(newImage, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f,
					images.at(current_image)._width, images.at(current_image)._height, images.at(current_image)._color);
				images.at(current_image)._elements.push_back(element);

				// finally, delete the buffer used to hold the pixel data
			}
		}
	}

	// Free loaded image, in case we used it
	if (loadInfo.pixels)
		free (loadInfo.pixels);

	return true;
}



//-----------------------------------------------------------------------------
// BeginImageLoadBatch: enables "batching mode" so when you load an image, it
//                    isn't loaded immediately but rather placed into a vector
//                    and loaded on EndTexLoadBatch().
//-----------------------------------------------------------------------------
bool GameVideo::BeginImageLoadBatch()
{
	_batchLoading = true;
	_batchLoadImages.clear();  // this should already be clear, but just in case...
	_batchLoadMultiImages.clear();

	return true;
}

//-----------------------------------------------------------------------------
// EndImageLoadBatch: ends a batch load block
//                    returns false if any of the images failed to load
//-----------------------------------------------------------------------------
bool GameVideo::EndImageLoadBatch()
{	
	_batchLoading = false;
	
	// go through vector of images waiting to be loaded and load them

	std::vector <StillImage *>::iterator iImage = _batchLoadImages.begin();
	std::vector <StillImage *>::iterator iEnd   = _batchLoadImages.end();
	
	bool success = true;
	
	while(iImage != iEnd)
	{
		StillImage *id = *iImage;
		
		if(!id)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: got a NULL StillImage in EndImageLoadBatch()!" << endl;
			success = false;
		}
		
		if(!LoadImage(*id))
			success = false;
		
		++iImage;
	}

	_batchLoadImages.clear();



	// go through vector of multi-images waiting to be loaded and load them

	std::vector<MultiImage>::iterator iMultiImage = _batchLoadMultiImages.begin();
	std::vector<MultiImage>::iterator iMultiImageEnd = _batchLoadMultiImages.end();
	
	while(iMultiImage != iMultiImageEnd)
	{		
		if(!_LoadMultiImage(*iMultiImage))
			success = false;
		
		++iMultiImage;
	}

	_batchLoadMultiImages.clear();

	
	return success;
}

//-----------------------------------------------------------------------------
// _LoadImageHelper: private function which does the dirty work of actually
//                     loading an image.
//-----------------------------------------------------------------------------
bool GameVideo::_LoadImageHelper(StillImage &id, bool grayscale)
{
	bool isStatic = id._is_static;
	
	id._elements.clear();

	private_video::ImageLoadInfo loadInfo;
	
	if(!_LoadRawImage(id._filename, loadInfo, grayscale))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: _LoadRawPixelData() failed in _LoadImageHelper()" << endl;
		return false;
	}

	// create an Image structure and store it our std::map of images
	Image *newImage = new Image(id._filename, loadInfo.width, loadInfo.height, grayscale);

	// try to insert the image in a texture sheet
	TexSheet *sheet = _InsertImageInTexSheet(newImage, loadInfo, isStatic);
	
	if(!sheet)
	{
		// this should never happen, unless we run out of memory or there
		// is a bug in the _InsertImageInTexSheet() function
		
		if(VIDEO_DEBUG)
			cerr << "VIDEO_DEBUG: GameVideo::_InsertImageInTexSheet() returned NULL!" << endl;
		
		free(loadInfo.pixels);
		return false;
	}
	
	newImage->ref_count = 1;
	
	// store the image in our std::map
	_images[id._filename] = newImage;


	// if width or height are zero, that means to use the dimensions of image
	if(id._width == 0.0f)
		id._width = (float) loadInfo.width;
	
	if(id._height == 0.0f)
		id._height = (float) loadInfo.height;

	// store the new image element
	ImageElement element(newImage, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, id._width, id._height, id._color);
	id._elements.push_back(element);

	// finally, delete the buffer used to hold the pixel data
	free(loadInfo.pixels);
	return true;
}


//-----------------------------------------------------------------------------
// _LoadRawImage: Determines which image loader to call
//-----------------------------------------------------------------------------
bool GameVideo::_LoadRawImage(const std::string & filename, private_video::ImageLoadInfo & loadInfo, bool grayscale)
{
	// Isolate the extension
	size_t extpos = filename.rfind('.');

	if(extpos == string::npos)
		return false;

	std::string extension = std::string(filename, extpos, filename.length() - extpos);

	if(extension == ".jpeg" || extension == ".jpg")
		return _LoadRawImageJpeg(filename, loadInfo, grayscale);
	if(extension == ".png")
		return _LoadRawImagePng(filename, loadInfo, grayscale) ;

	return false;
}	

//-----------------------------------------------------------------------------
// _LoadRawImagePng: Loads a PNG image to RGBA format
//-----------------------------------------------------------------------------

bool GameVideo::_LoadRawImagePng(const std::string &filename, hoa_video::private_video::ImageLoadInfo &loadInfo, bool grayscale)
{
	FILE * fp = fopen(filename.c_str(), "rb");

	if(fp == NULL)
		return false;

	unsigned char testbuffer[8];

	fread(testbuffer, 1, 8, fp);
	if(png_sig_cmp(testbuffer, 0, 8))
	{
		fclose(fp);
		return false;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 
		(png_voidp)NULL, NULL, NULL);

	if(!png_ptr)	
	{
		fclose(fp);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if(!info_ptr)
	{
		fclose(fp);
		return false;
	}

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		fclose(fp);
		return false;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);
	
	unsigned char * * row_pointers = png_get_rows(png_ptr, info_ptr);

	loadInfo.width = info_ptr->width;
	loadInfo.height = info_ptr->height;
	loadInfo.pixels = malloc(info_ptr->width * info_ptr->height * 4);

	unsigned int bpp = info_ptr->channels;

	for(unsigned int y = 0; y < info_ptr->height; y++)
	{
		for(unsigned int x = 0; x < info_ptr->width; x++)
		{
			unsigned char * imgpixel = row_pointers[y] + (x * bpp);
			unsigned char * dstpixel = ((unsigned char *)loadInfo.pixels) + ((y * info_ptr->width) + x) * 4;

			if(bpp == 4)
			{
				unsigned char r = imgpixel[0];
				unsigned char g = imgpixel[1];
				unsigned char b = imgpixel[2];
				unsigned char a = imgpixel[3];

				if (!grayscale)
				{
					dstpixel[0] = r;
					dstpixel[1] = g;
					dstpixel[2] = b;
					dstpixel[3] = a;
				}
				else
				{
					unsigned char y = static_cast<unsigned char>(0.3 * r + 0.59 * g + 0.11 * b);
					dstpixel[0] = dstpixel[1] = dstpixel[2] = y;
					dstpixel[3] = a;
				}
			}
			else if(bpp == 3)
			{
				unsigned char r = imgpixel[0];
				unsigned char g = imgpixel[1];
				unsigned char b = imgpixel[2];

				if (!grayscale)
				{
					dstpixel[0] = r;
					dstpixel[1] = g;
					dstpixel[2] = b;
					dstpixel[3] = 0xFF;
				}
				else
				{
					unsigned char y = static_cast<unsigned char>(0.3 * r + 0.59 * g + 0.11 * b);
					dstpixel[0] = dstpixel[1] = dstpixel[2] = y;
					dstpixel[3] = 0xFF;
				}
			}
			else if(bpp == 1)
			{
				png_color c = info_ptr->palette[imgpixel[0]];

				unsigned char r = c.red;
				unsigned char g = c.green;
				unsigned char b = c.blue;

				if (!grayscale)
				{
					dstpixel[0] = r;
					dstpixel[1] = g;
					dstpixel[2] = b;
					dstpixel[3] = 0xFF;
				}
				else
				{
					unsigned char y = static_cast<unsigned char>(0.3 * r + 0.59 * g + 0.11 * b);
					dstpixel[0] = dstpixel[1] = dstpixel[2] = y;
					dstpixel[3] = 0xFF;
				}
			}
		}
	}
	

	png_destroy_read_struct(&png_ptr, &info_ptr,
		(png_infopp)NULL);

	fclose(fp);
	return true;
}

//-----------------------------------------------------------------------------
// _LoadRawImageJpeg: Loads a Jpeg image to RGBA format
//-----------------------------------------------------------------------------

bool GameVideo::_LoadRawImageJpeg(const std::string &filename, hoa_video::private_video::ImageLoadInfo &loadInfo, bool grayscale)
{
	FILE * infile;
	unsigned char ** buffer;

	if((infile = fopen(filename.c_str(), "rb")) == NULL) 
		return false;

	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);

	jpeg_start_decompress(&cinfo);

	JDIMENSION row_stride = cinfo.output_width * cinfo.output_components;

	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);


	loadInfo.width = cinfo.output_width;
	loadInfo.height = cinfo.output_height;
	loadInfo.pixels = malloc(cinfo.output_width * cinfo.output_height * 4);

	unsigned int bpp = cinfo.output_components;

	for(unsigned int y = 0; y < cinfo.output_height; y++)
	{
		jpeg_read_scanlines(&cinfo, buffer, 1);

		for(unsigned int x = 0; x < cinfo.output_width; x++)
		{
			unsigned char * imgpixel = buffer[0] + (x * bpp);
			unsigned char * dstpixel = ((unsigned char *)loadInfo.pixels) + ((y * cinfo.output_width) + x) * 4;

			unsigned char r = imgpixel[0];
			unsigned char g = imgpixel[1];
			unsigned char b = imgpixel[2];

			unsigned char a = imgpixel[3];

			if (!grayscale)
			{
				dstpixel[0] = r;
				dstpixel[1] = g;
				dstpixel[2] = b;
			}
			else
			{
				unsigned char y = static_cast<unsigned char>(0.3 * r + 0.59 * g + 0.11 * b);
				dstpixel[0] = dstpixel[1] = dstpixel[2] = y;
			}

			if(bpp == 4)
				dstpixel[3] = a;
			else
				dstpixel[3] = 0xFF;
		}
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	fclose(infile);

	return true;
}


//-----------------------------------------------------------------------------
// TilesToObject: given a vector of tiles, and a 2D vector of indices into
//                those tiles, construct a single image descriptor which
//                stitches those tiles together into one image
//
// NOTE: when calling this function, make sure of the following things:
//     1. All tiles must be the SAME width and height.
//     2. The vectors must be non-empty
//     3. The indices must be within proper bounds
//     4. The indices vector has the same number of columns in every row
//     5. Remember to call DeleteImage() when you're done.
//-----------------------------------------------------------------------------

StillImage GameVideo::TilesToObject
( 
	vector<StillImage> &tiles, 
	vector< vector<uint32> > indices 
)
{	
	StillImage id;

	// figure out the width and height information
		
	int32 w, h;
	w = (int32) indices[0].size();         // how many tiles wide and high
	h = (int32) indices.size();

	float tileWidth  = tiles[0]._width;   // width and height of each tile
	float tileHeight = tiles[0]._height;
	
	id._width  = (float) w * tileWidth;   // total width/height of compound
	id._height = (float) h * tileHeight;
	
	id._is_static = tiles[0]._is_static;
	
	for(int32 y = 0; y < h; ++y)
	{
		for(int32 x = 0; x < w; ++x)
		{
			// add each tile at the correct offset
			
			float xOffset = x * tileWidth;
			float yOffset = y * tileHeight;
			
			if(!id.AddImage(tiles[indices[y][x]], xOffset, yOffset))
			{
				if(VIDEO_DEBUG)
				{
					cerr << "VIDEO ERROR: failed to AddImage in TilesToObject()!" << endl;
				}
			}
		}
	}
	
	return id;
}


//-----------------------------------------------------------------------------
// _InsertImageInTexSheet: takes an image that was loaded, finds
//                        an available texture sheet, copies it to the sheet,
//                        and returns a pointer to the texture sheet. If no
//                        available texture sheet is found, a new one is created.
//
//                        Returns NULL on failure, which should only happen if
//                        we run out of memory or bad argument is passed.
//-----------------------------------------------------------------------------
TexSheet *GameVideo::_InsertImageInTexSheet(Image *image, private_video::ImageLoadInfo & loadInfo, bool isStatic)
{
	// if it's a large image size (>512x512) then we already know it's not going
	// to fit in any of our existing texture sheets, so create a new one for it

	if(loadInfo.width > 512 || loadInfo.height > 512)
	{
		int32 roundW = RoundUpPow2(loadInfo.width);
		int32 roundH = RoundUpPow2(loadInfo.height);		
		TexSheet *sheet = _CreateTexSheet(roundW, roundH, VIDEO_TEXSHEET_ANY, false);

		// ran out of memory!
		if(!sheet)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: _CreateTexSheet() returned NULL in _InsertImageInTexSheet()!" << endl;
			return NULL;
		}
					
		if(sheet->AddImage(image, loadInfo))
			return sheet;
		else
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: AddImage() returned false for inserting large image!" << endl;	
			return NULL;
		}
	}
		
	
	// determine the type of texture sheet that should hold this image
	
	TexSheetType type;
	
	if(loadInfo.width == 32 && loadInfo.height == 32)
		type = VIDEO_TEXSHEET_32x32;
	else if(loadInfo.width == 32 && loadInfo.height == 64)
		type = VIDEO_TEXSHEET_32x64;
	else if(loadInfo.width == 64 && loadInfo.height == 64)
		type = VIDEO_TEXSHEET_64x64;
	else
		type = VIDEO_TEXSHEET_ANY;
		
	// loop through existing texture sheets and see if the image will fit in
	// any of the ones which match the type we're looking for
	
	size_t numTexSheets = _texSheets.size();
	
	for(uint32 iSheet = 0; iSheet < numTexSheets; ++iSheet)
	{
		TexSheet *sheet = _texSheets[iSheet];
		if(!sheet)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: _texSheets[iSheet] was NULL in _InsertImageInTexSheet()!" << endl;
			return NULL;
		}
		
		if(sheet->type == type && sheet->isStatic == isStatic)
		{
			if(sheet->AddImage(image, loadInfo))
			{
				// added to a sheet successfully
				return sheet;
			}
		}
	}
	
	// if it doesn't fit in any of them, create a new 512x512 and stuff it in
	
	TexSheet *sheet = _CreateTexSheet(512, 512, type, isStatic);
	if(!sheet)
	{
		// failed to create texture, ran out of memory probably
		
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: Failed to create new texture sheet in _InsertImageInTexSheet!" << endl;
		}
		
		return NULL;
	}

	// now that we have a fresh texture sheet, AddImage() should work without
	// any problem
	if(sheet->AddImage(image, loadInfo))
	{
		return sheet;
	}

	return NULL;
}


//-----------------------------------------------------------------------------
// _CreateTexSheet: creates a new texture sheet with the given parameters,
//                 adds it to our internal vector of texture sheets, and
//                 returns a pointer to it.
//                 Returns NULL on failure, which should only happen if
//                 we run out of memory or bad argument is passed.
//-----------------------------------------------------------------------------

TexSheet *GameVideo::_CreateTexSheet
(
	int32 width,
	int32 height,
	TexSheetType type,
	bool isStatic
)
{
	// validate the parameters	

	if(!IsPowerOfTwo(width) || !IsPowerOfTwo(height))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: non pow2 width and/or height passed to _CreateTexSheet!" << endl;
			
		return NULL;
	}
	
	if(type <= VIDEO_TEXSHEET_INVALID || type >= VIDEO_TEXSHEET_TOTAL)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: Invalid TexSheetType passed to _CreateTexSheet()!" << endl;
			
		return NULL;
	}
	
	GLuint texID = _CreateBlankGLTexture(width, height);	
	
	// now that we have our texture loaded, simply create a new TexSheet	
	
 	TexSheet *sheet = new TexSheet(width, height, texID, type, isStatic);
	_texSheets.push_back(sheet);
	
	return sheet;
}


//-----------------------------------------------------------------------------
// TexSheet constructor
//-----------------------------------------------------------------------------

TexSheet::TexSheet(int32 w, int32 h, GLuint texID_, TexSheetType type_, bool isStatic_)
{
	width = w;
	height = h;
	texID = texID_;
	type = type_;
	isStatic = isStatic_;
	loaded = true;
	
	if(type == VIDEO_TEXSHEET_32x32)
		texMemManager = new FixedTexMemMgr(this, 32, 32);
	else if(type == VIDEO_TEXSHEET_32x64)
		texMemManager = new FixedTexMemMgr(this, 32, 64);
	else if(type == VIDEO_TEXSHEET_64x64)
		texMemManager = new FixedTexMemMgr(this, 64, 64);
	else
		texMemManager = new VariableTexMemMgr(this);
}


//-----------------------------------------------------------------------------
// TexSheet destructor
//-----------------------------------------------------------------------------

TexSheet::~TexSheet()
{
	// delete texture memory manager
	delete texMemManager;
	
	hoa_video::GameVideo *videoManager = hoa_video::GameVideo::SingletonGetReference();
	
	if(!videoManager)
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: GameVideo::GetReference() returned NULL in TexSheet destructor!" << endl;
		}
	}
	
	// unload actual texture from memory
	videoManager->_DeleteTexture(texID);
}


//-----------------------------------------------------------------------------
// VariableTexMemMgr constructor
//-----------------------------------------------------------------------------

VariableTexMemMgr::VariableTexMemMgr(TexSheet *sheet)
{
	_texSheet    = sheet;
	_sheetWidth  = sheet->width / 16;
	_sheetHeight = sheet->height / 16;
	_blocks      = new VariableImageNode[_sheetWidth*_sheetHeight];
}


//-----------------------------------------------------------------------------
// VariableTexMemMgr destructor
//-----------------------------------------------------------------------------

VariableTexMemMgr::~VariableTexMemMgr()
{
	delete [] _blocks;	
}


bool GameVideo::_DEBUG_ShowTexSheet()
{
	// value of zero means to disable display
	if(_currentDebugTexSheet == -1)
	{
		return true;
	}
		
	// check if there aren't any texture sheets! (should never happen)
	if(_texSheets.empty())
	{
		if(VIDEO_DEBUG)		
			cerr << "VIDEO_WARNING: Called DEBUG_ShowTexture(), but there were no texture sheets" << endl;
		return false;
	}
	
	int32 numSheets = (int32) _texSheets.size();
	
	// we may go out of bounds say, if we were viewing a texture sheet and then it got
	// deleted or something. To recover, just set it to the last texture sheet
	if(_currentDebugTexSheet >= numSheets)
	{
		_currentDebugTexSheet = numSheets - 1;
	}
	
	TexSheet *sheet = _texSheets[_currentDebugTexSheet];
	
	if(!sheet)
	{
		return false;
	}
	
	int32 w = sheet->width;
	int32 h = sheet->height;
	
	Image img(sheet, string(), 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, w, h, false);


	_PushContext();	
	SetDrawFlags(VIDEO_NO_BLEND, VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	SetCoordSys(0.0f, 1024.0f, 0.0f, 760.0f);
	
	glPushMatrix();

	Move(0.0f,0.0f);
	glScalef(0.5f, 0.5f, 0.5f);

	ImageElement elem(&img, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, (float)w, (float)h);
	
	StillImage id;
	id._elements.push_back(elem);

	
	if(!DrawImage(id))
	{
		glPopMatrix();
		_PopContext();
		return false;
	}
		
	glPopMatrix();

	if(!SetFont("debug_font"))
	{
		_PopContext();
		return false;
	}
	
	char buf[200];
	
	Move(20, _coord_sys.GetTop() - 30);
	if(!DrawText("Current Texture sheet:"))
	{
		_PopContext();
		return false;
	}
	
	sprintf(buf, "  Sheet #: %d", _currentDebugTexSheet);	
	MoveRelative(0, -20);
	if(!DrawText(buf))
	{
		_PopContext();
		return false;
	}
	
	MoveRelative(0, -20);
	sprintf(buf, "  Size:    %dx%d", sheet->width, sheet->height);
	if(!DrawText(buf))
	{
		_PopContext();
		return false;
	}
	
	if(sheet->type == VIDEO_TEXSHEET_32x32)
		sprintf(buf, "  Type:    32x32");
	else if(sheet->type == VIDEO_TEXSHEET_32x64)
		sprintf(buf, "  Type:    32x64");
	else if(sheet->type == VIDEO_TEXSHEET_64x64)
		sprintf(buf, "  Type:    64x64");
	else if(sheet->type == VIDEO_TEXSHEET_ANY)
		sprintf(buf, "  Type:    Any size");
	
	MoveRelative(0, -20);
	if(!DrawText(buf))
	{
		_PopContext();
		return false;
	}
	
	sprintf(buf, "  Static:  %d", sheet->isStatic);
	MoveRelative(0, -20);
	if(!DrawText(buf))
	{
		_PopContext();
		return false;
	}

	sprintf(buf, "  TexID:   %d", sheet->texID);
	MoveRelative(0, -20);
	if(!DrawText(buf))
	{
		_PopContext();
		return false;
	}
	
	_PopContext();
	return true;
}


//-----------------------------------------------------------------------------
// _DeleteImage: decreases the reference count on an image, and deletes it
//               if zero is reached. Note that for images larger than 512x512,
//               there is no reference counting; we just delete it immediately
//               because we don't want huge textures sitting around in memory
//-----------------------------------------------------------------------------

bool GameVideo::_DeleteImage(Image *const img)
{
	if(img->width > 512 || img->height > 512)
	{
		// remove the image and texture sheet completely
		_RemoveSheet(img->texture_sheet);
		_RemoveImage(img);
	}
	else
	{
		// for smaller images, simply mark them as free in the memory manager
		--(img->ref_count);
		if(img->ref_count <= 0)
		{
			img->texture_sheet->FreeImage(img);
		}
	}
	
	return true;
}


//-----------------------------------------------------------------------------
// _RemoveSheet: removes a texture sheet from the internal std::vector
//-----------------------------------------------------------------------------

bool GameVideo::_RemoveSheet(TexSheet *sheet)
{
	if(_texSheets.empty())
	{
		return false;
	}
		
	vector<TexSheet*>::iterator iSheet = _texSheets.begin();
	vector<TexSheet*>::iterator iEnd   = _texSheets.end();
	
	// search std::vector for pointer matching sheet and remove it
	while(iSheet != iEnd)
	{
		if(*iSheet == sheet)
		{
			delete sheet;
			_texSheets.erase(iSheet);
			return true;
		}
		++iSheet;
	}
	
	// couldn't find the image
	return false;
}


//-----------------------------------------------------------------------------
// AddImage: adds a new image to a texture sheet
// NOTE: assumes that the image we're adding is still "bound" in DevIL
//-----------------------------------------------------------------------------

bool TexSheet::AddImage(Image *img, ImageLoadInfo & loadInfo)
{
	// try inserting into the texture memory manager
	bool couldInsert = texMemManager->Insert(img);	
	if(!couldInsert)
		return false;
	
	// now img contains the x, y, width, and height of the subrectangle
	// inside the texture sheet, so go ahead and copy that area
		
	TexSheet *texSheet = img->texture_sheet;
	if(!texSheet)
	{
		// technically this should never happen since Insert() returned true
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: texSheet was NULL after texMemManager->Insert() returned true" << endl;
		}
		return false;
	}

	if(!CopyRect(img->x, img->y, loadInfo))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: CopyRect() failed in TexSheet::AddImage()!" << endl;
		return false;
	}	
	
	return true;
}


//-----------------------------------------------------------------------------
// CopyRect: copies an image into a sub-rectangle of the texture
//-----------------------------------------------------------------------------

bool TexSheet::CopyRect(int32 x, int32 y, private_video::ImageLoadInfo & loadInfo)
{
	int32 error;
	
	hoa_video::GameVideo *videoManager = hoa_video::GameVideo::SingletonGetReference();
	videoManager->_BindTexture(texID);	
	
	error = glGetError();
	if(error)
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: could not bind texture in TexSheet::CopyRect()!" << endl;
		}
		return false;
	}

	glTexSubImage2D
	(
		GL_TEXTURE_2D,    // target
		0,                // level
		x,                // x offset within tex sheet
		y,                // y offset within tex sheet
		loadInfo.width,   // width in pixels of image
		loadInfo.height,  // height in pixels of image
		GL_RGBA,		  // format
		GL_UNSIGNED_BYTE, // type
		loadInfo.pixels   // pixels of the sub image
	);

	error = glGetError();
	if(error)
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: glTexSubImage2D() failed in TexSheet::CopyRect()!" << endl;
		}
		return false;
	}
	
	return true;
}


//-----------------------------------------------------------------------------
// _RemoveImage: removes an image completely from the texture sheet's
//              memory manager so that a new image can be loaded in its place
//-----------------------------------------------------------------------------

bool TexSheet::RemoveImage(Image *img)
{
	return texMemManager->Remove(img);
}


//-----------------------------------------------------------------------------
// FreeImage: sets the area taken up by the image to "free". However, the
//            image is not removed from any lists yet! It's kept around in
//            case we reload the image in the near future- in that case,
//            we can simply Restore the image instead of reloading from disk.
//-----------------------------------------------------------------------------

bool TexSheet::FreeImage(Image *img)
{
	return texMemManager->Free(img);
}

//-----------------------------------------------------------------------------
// RestoreImage: If an image is freed using FreeImage, and soon afterwards,
//               we load that image again, this function restores the image
//               without reloading the image from disk.
//-----------------------------------------------------------------------------

bool TexSheet::RestoreImage(Image *img)
{
	return texMemManager->Restore(img);
}


//-----------------------------------------------------------------------------
// DeleteImage: deletes an image descriptor (this is what the API user should call)
//-----------------------------------------------------------------------------

bool GameVideo::DeleteImage(ImageDescriptor &id)
{
	if(id._animated)
	{
		return _DeleteImage(dynamic_cast<AnimatedImage &>(id));
	}
	else
	{
		return _DeleteImage(dynamic_cast<StillImage &>(id));
	}	
}


//-----------------------------------------------------------------------------
// _DeleteImage: deletes an animated image. Actually just goes through each frame
//               and deletes that static image by calling the other _DeleteImage() function
//-----------------------------------------------------------------------------

bool GameVideo::_DeleteImage(AnimatedImage &id)
{
	int32 numFrames = id.GetNumFrames();
	bool success = true;
	
	for(int j = 0; j < numFrames; ++j)
	{
		success &= _DeleteImage(id._frames[j]._image);
	}
	
	return success;
}


//-----------------------------------------------------------------------------
// _DeleteImage: decrements the reference count for all images composing this
//              image descriptor.
//
// NOTE: for images which are 1024x1024 or higher, once their reference count
//       reaches zero, they're immediately deleted. (don't want to keep those
//       in memory if possible). For others, they're simply marked as "free"
//-----------------------------------------------------------------------------

bool GameVideo::_DeleteImage(StillImage &id)
{
	vector<ImageElement>::iterator iImage = id._elements.begin();
	vector<ImageElement>::iterator iEnd   = id._elements.end();
	
	// loop through all the images inside this descriptor
	while(iImage != iEnd)
	{
		Image *img = (*iImage).image;
		
		// only delete the image if the pointer is valid. Some ImageElements
		// have a NULL pointer because they are just colored quads
		
		if(img)
		{				
			if(img->ref_count <= 0)
			{
				if(VIDEO_DEBUG)		
					cerr << "VIDEO ERROR: Called DeleteImage() when refcount was already <= 0!" << endl;
				return false;
			}

			--(img->ref_count);
			
			if(img->ref_count == 0)
			{
				// 1. If it's on a large tex sheet (> 512x512), delete it
				// Note: We can assume that this is the only image on that texture
				//       sheet, so it's safe to delete it. (Big textures always
				//       are allocated to their own sheet, by design.)
				
				if(img->width > 512 || img->height > 512)
				{				
					_DeleteImage(img);  // overloaded DeleteImage for deleting Image
				}

				// 2. otherwise, mark it as "freed"
				
				else if(!img->texture_sheet->FreeImage(img))
				{
					if(VIDEO_DEBUG)		
						cerr << "VIDEO ERROR: Could not remove image from texture sheet!" << endl;
					return false;
				}			
			}
		}
		
		++iImage;
	}
	
	id._elements.clear();
	id._filename = "";
	id._height = id._width = 0;
	id._is_static = 0;
	
	return true;
}


//-----------------------------------------------------------------------------
// _RemoveImage: removes the image pointer from the std::map
//-----------------------------------------------------------------------------

bool GameVideo::_RemoveImage(Image *img)
{
	// nothing to do if img is null
	if(!img)
		return true;

	if(_images.empty())
	{
		return false;
	}
		
	map<string, Image*>::iterator iImage = _images.begin();
	map<string, Image*>::iterator iEnd   = _images.end();
	
	// search std::map for pointer matching img and remove it
	while(iImage != iEnd)
	{
		if(iImage->second == img)
		{
			delete img;
			_images.erase(iImage);
			return true;
		}
		++iImage;		
	}
	
	// couldn't find the image
	return false;
}


//-----------------------------------------------------------------------------
// FixedTexMemMgr constructor
//-----------------------------------------------------------------------------

FixedTexMemMgr::FixedTexMemMgr
(
	TexSheet *texSheet, 
	int32 imgW, 
	int32 imgH
)
{
	_texSheet = texSheet;
	
	// set all the dimensions	
	_sheetWidth  = texSheet->width  / imgW;
	_sheetHeight = texSheet->height / imgH;
	_imageWidth  = imgW;
	_imageHeight = imgH;
	
	// allocate the blocks array	
	int32 numBlocks = _sheetWidth * _sheetHeight;
	_blocks = new FixedImageNode[numBlocks];
	
	// initialize linked list of open blocks... which, at this point is
	// all the blocks!	
	_openListHead = &_blocks[0];
	_openListTail = &_blocks[numBlocks-1];
	
	// now initialize all the blocks to proper values		
	for(int32 i = 0; i < numBlocks - 1; ++i)
	{
		_blocks[i].next  = &_blocks[i+1];
		_blocks[i].image = NULL;
		_blocks[i].blockIndex = i;
	}

	_blocks[numBlocks-1].next  = NULL;
	_blocks[numBlocks-1].image = NULL;
	_blocks[numBlocks-1].blockIndex = numBlocks - 1;
}



//-----------------------------------------------------------------------------
// FixedTexMemMgr destructor
//-----------------------------------------------------------------------------

FixedTexMemMgr::~FixedTexMemMgr()
{
	delete [] _blocks;	
}


//-----------------------------------------------------------------------------
// Insert: inserts a new block into the texture. If there's no free blocks
//         left, return false
//-----------------------------------------------------------------------------

bool VariableTexMemMgr::Insert  (Image *img)
{
	
	// don't allow insertions into a texture bigger than 512x512... 
	// This way, if we have a 1024x1024 texture holding a fullscreen background,
	// it is always safe to remove the texture sheet from memory when the
	// background is unreferenced. That way backgrounds don't stick around in memory.

	if(_sheetWidth > 32 || _sheetHeight > 32)  // 32 blocks = 512 pixels
	{
		if(!_blocks[0].free)  // quick way to test if texsheet's occupied
			return false;
	}	
	

	// find an open block of memory. If none is found, return false
	
	int32 w = (img->width  + 15) / 16;   // width and height in blocks
	int32 h = (img->height + 15) / 16;

	int32 blockX=-1, blockY=-1;
	
	
	// this is a 100% brute force way to allocate a block, just a bunch
	// of nested loops. In practice, this actually works fine, because 
	// the allocator deals with 16x16 blocks instead of trying to worry
	// about fitting images with pixel perfect resolution.
	// Later, if this turns out to be a bottleneck, we can rewrite this
	// algorithm to something more intelligent ^_^
	for(int32 y = 0; y < _sheetHeight - h + 1; ++y)
	{
		for(int32 x = 0; x < _sheetWidth - w + 1; ++x)
		{
			int32 furthestBlocker = -1;
			
			for(int32 dy = 0; dy < h; ++dy)
			{
				for(int32 dx = 0; dx < w; ++dx)
				{
					if(!_blocks[(x+dx)+((y+dy)*_sheetWidth)].free)
					{
						furthestBlocker = x+dx;
						goto endneighborsearch;
					}					
				}
			}
			
			endneighborsearch:
			
			if(furthestBlocker == -1)
			{
				blockX = x;
				blockY = y;
				goto endsearch;
			}
		}
	}
	
	endsearch:
	
	if(blockX == -1 || blockY == -1)
		return false;	

	// check if there's already an image allocated at this block.
	// If so, we have to notify GameVideo that we're ejecting
	// this image out of memory to make place for the new one	
	
	hoa_video::GameVideo *VideoManager = hoa_video::GameVideo::SingletonGetReference();

	// update blocks
	std::set<hoa_video::private_video::Image *> removeimages;

	for(int32 y = blockY; y < blockY + h; ++y)
	{
		for(int32 x = blockX; x < blockX + w; ++x)
		{			
			int32 index = x + (y * _sheetWidth);
			// check if there's already an image at the point we're
			// trying to load at. If so, we need to tell GameVideo
			// to update its internal vector
			
			if(_blocks[index].image)
			{
				removeimages.insert(_blocks[index].image);
			}

			_blocks[index].free  = false;
			_blocks[index].image = img;

		}
	}

	for(std::set<hoa_video::private_video::Image *>::iterator itr = removeimages.begin(); itr != removeimages.end(); itr++)
	{
		Remove(*itr);
		VideoManager->_RemoveImage(*itr);
	}

		
	// calculate the actual pixel coordinates given this node's
	// block index
	
	img->x = blockX * 16;
	img->y = blockY * 16;
	
	// calculate the u,v coordinates
	
	float sheetW = (float) _texSheet->width;
	float sheetH = (float) _texSheet->height;
	
	img->u1 = float(img->x + 0.5f)               / sheetW;
	img->u2 = float(img->x + img->width - 0.5f)  / sheetW;
	img->v1 = float(img->y + 0.5f)               / sheetH;
	img->v2 = float(img->y + img->height - 0.5f) / sheetH;

	img->texture_sheet = _texSheet;
	return true;
}


//-----------------------------------------------------------------------------
// Remove: completely remove an image.
//         In other words:
//           1. find all the blocks this image owns
//           2. mark all those blocks' image pointers to NULL
//           3. set the "free" flag to true
//-----------------------------------------------------------------------------

bool VariableTexMemMgr::Remove(Image *img)
{
	return SetBlockProperties(img, true, true, true, NULL);
}


//-----------------------------------------------------------------------------
// SetBlockProperties: goes through all the blocks associated with img, and
//                     updates their "free" and "image" properties if
//                     changeFree and changeImage are true, respectively
//-----------------------------------------------------------------------------

bool VariableTexMemMgr::SetBlockProperties
(
	Image *img,
	bool changeFree,
	bool changeImage,
	bool free,
	Image *newImage
)
{
	int32 blockX = img->x / 16;          // upper-left corner in blocks
	int32 blockY = img->y / 16;
	
	int32 w = (img->width  + 15) / 16;   // width and height in blocks
	int32 h = (img->height + 15) / 16;

	for(int32 y = blockY; y < blockY + h; ++y)
	{
		for(int32 x = blockX; x < blockX + w; ++x)
		{
			if(_blocks[x+y*_sheetWidth].image == img)
			{
				if(changeFree)
					_blocks[x+y*_sheetWidth].free  = free;
				if(changeImage)
					_blocks[x+y*_sheetWidth].image = newImage;
			}
		}	
	}
	
	return true;
}


//-----------------------------------------------------------------------------
// Free: marks the blocks containing the image as free
//       NOTE: this assumes that the block isn't ALREADY free
//-----------------------------------------------------------------------------

bool VariableTexMemMgr::Free(Image *img)
{
	return SetBlockProperties(img, true, false, true, NULL);
}


//-----------------------------------------------------------------------------
// Restore: marks the blocks containing the image as non-free
//-----------------------------------------------------------------------------

bool VariableTexMemMgr::Restore(Image *img)
{
	return SetBlockProperties(img, true, false, false, NULL);
}


//-----------------------------------------------------------------------------
// Insert: inserts a new block into the texture. If there's no free blocks
//         left, returns false.
//-----------------------------------------------------------------------------

bool FixedTexMemMgr::Insert(Image *img)
{
	// whoa, nothing on the open list! (no blocks left) return false :(
	
	if(_openListHead == NULL)
		return false;
		
	// otherwise, get and remove the head of the open list	
	
	FixedImageNode *node = _openListHead;
	_openListHead = _openListHead->next;
	
	if(_openListHead == NULL)
	{
		// this must mean we just removed the last open block, so
		// set the tail to NULL as well
		_openListTail = NULL;
	}
	else
	{
		// since this is the new head, it's prev pointer should be NULL
		_openListHead->prev = NULL;
	}
	
	node->next = NULL;
	
	// check if there's already an image allocated at this block.
	// If so, we have to notify GameVideo that we're ejecting
	// this image out of memory to make place for the new one	
	
	if(node->image)
	{
		hoa_video::GameVideo *VideoManager = hoa_video::GameVideo::SingletonGetReference();
		VideoManager->_RemoveImage(node->image);
		node->image = NULL;
	}
	
	// calculate the actual pixel coordinates given this node's
	// block index
	
	img->x = _imageWidth  * (node->blockIndex % _sheetWidth);
	img->y = _imageHeight * (node->blockIndex / _sheetWidth);
	
	// calculate the u,v coordinates
	
	float sheetW = (float) _texSheet->width;
	float sheetH = (float) _texSheet->height;
	
	img->u1 = float(img->x + 0.5f)               / sheetW;
	img->u2 = float(img->x + img->width - 0.5f)  / sheetW;
	img->v1 = float(img->y + 0.5f)               / sheetH;
	img->v2 = float(img->y + img->height - 0.5f) / sheetH;

	img->texture_sheet = _texSheet;
	
	return true;
}

//-----------------------------------------------------------------------------
// CalculateBlockIndex: returns the block index used up by this image
//-----------------------------------------------------------------------------
int32 FixedTexMemMgr::_CalculateBlockIndex(Image *img)
{
	int32 blockX = img->x / _imageWidth;
	int32 blockY = img->y / _imageHeight;
	
	int32 blockIndex = blockX + _sheetWidth * blockY;
	return blockIndex;
}

//-----------------------------------------------------------------------------
// Remove: completely remove an image.
//         In other words:
//           1. mark its block's image pointer to NULL
//           2. remove it from the open list
//-----------------------------------------------------------------------------
bool FixedTexMemMgr::Remove(Image *img)
{
	// translate x,y coordinates into a block index
	int32 blockIndex = _CalculateBlockIndex(img);
	
	// check to make sure the block is actually owned by this image
	if(_blocks[blockIndex].image != img)
	{
		// error, the block that the image thinks it owns is actually not
		// owned by that image
		
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: tried to remove a fixed block not owned by this Image" << endl;
		return false;
	}
	
	// set the image to NULL to indicate that this block is completely free
	_blocks[blockIndex].image = NULL;
	
	// remove block from the open list	
	_DeleteNode(blockIndex);
	
	return true;
}

//-----------------------------------------------------------------------------
// DeleteNode: deletes a node from the open list with the given block index
//-----------------------------------------------------------------------------
void FixedTexMemMgr::_DeleteNode(int32 blockIndex)
{
	if(blockIndex < 0)
		return;
		
	if(blockIndex >= _sheetWidth * _sheetHeight)
		return;
		
	FixedImageNode *node = &_blocks[blockIndex];
		
	if(node->prev && node->next)
	{
		// node has a prev and next
		node->prev->next = node->next;
	}
	else if(node->prev)
	{
		// node is tail of the list
		node->prev->next = NULL;
		_openListTail = node->prev;
	}
	else if(node->next)
	{
		// node is head of the list
		_openListHead = node->next;
		node->next->prev = NULL;
	}
	else
	{
		// node is the only element in the list
		_openListHead = NULL;
		_openListTail = NULL;
	}
	
	// just for good measure, clear out this node's pointers	
	node->prev = NULL;
	node->next = NULL;
}

//-----------------------------------------------------------------------------
// Free: marks the block containing the image as free, i.e. on the open list
//       but leaves the image pointer intact in case we decide to restore 
//       the block later on
//
//       NOTE: this assumes that the block isn't ALREADY free
//-----------------------------------------------------------------------------
bool FixedTexMemMgr::Free(Image *img)
{
	int32 blockIndex = _CalculateBlockIndex(img);
	
	FixedImageNode *node = &_blocks[blockIndex];
	
	if(_openListTail != NULL)
	{
		// simply append to end of list
		_openListTail->next = node;
		node->prev = _openListTail;
		node->next = NULL;
		_openListTail = node;
	}
	else
	{
		// special case: empty list
		_openListHead = _openListTail = node;		
		node->next = node->prev = NULL;
	}
	
	return true;
}

//-----------------------------------------------------------------------------
// Restore: takes a block that was freed and takes it off the open list to
//          mark it as "used" again
//-----------------------------------------------------------------------------
bool FixedTexMemMgr::Restore(Image *img)
{
	int32 blockIndex = _CalculateBlockIndex(img);	
	_DeleteNode(blockIndex);	
	return true;
}

//-----------------------------------------------------------------------------
// DEBUG_NextTexSheet: increments to the next texture sheet to show with
//                     _DEBUG_ShowTexSheet().
//-----------------------------------------------------------------------------
void GameVideo::DEBUG_NextTexSheet()
{
	++_currentDebugTexSheet;
	
	if(_currentDebugTexSheet >= (int32) _texSheets.size()) 
	{
		_currentDebugTexSheet = -1;   // disable display
	}
}

//-----------------------------------------------------------------------------
// DEBUG_PrevTexSheet: cycles to the previous texturesheet to show with
//                     _DEBUG_ShowTexSheet().
//-----------------------------------------------------------------------------
void GameVideo::DEBUG_PrevTexSheet()
{
	--_currentDebugTexSheet;
	
	if(_currentDebugTexSheet < -1)
	{
		_currentDebugTexSheet = (int32) _texSheets.size() - 1;
	}
}


//-----------------------------------------------------------------------------
// ReloadTextures: reloads the texture sheets, after they have been unloaded
//                 most likely due to a change of video mode.
//                 Returns false if any of the textures fail to reload
//-----------------------------------------------------------------------------
bool GameVideo::ReloadTextures() 
{
	// reload texture sheets
	
	vector<TexSheet *>::iterator iSheet    = _texSheets.begin();
	vector<TexSheet *>::iterator iSheetEnd = _texSheets.end();

	bool success = true;

	while(iSheet != iSheetEnd)
	{
		TexSheet *sheet = *iSheet;
		
		if(sheet)
		{
			if(!sheet->Reload())
			{
				if(VIDEO_DEBUG)
					cerr << "VIDEO_ERROR: in ReloadTextures(), sheet->Reload() failed!" << endl;
				success = false;
			}
		}
		else
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: in ReloadTextures(), one of the tex sheets in the vector was NULL!" << endl;
			success = false;
		}
		
		
		++iSheet;
	}

	_DeleteTempTextures();

	if(_usesLights)
		_lightOverlay = _CreateBlankGLTexture(1024, 1024);

	// Clear all font caches
	map<string, FontProperties *>::iterator iFontProp    = _font_map.begin();
	map<string, FontProperties *>::iterator iFontPropEnd = _font_map.end();
	
	while(iFontProp != _font_map.end())
	{
		FontProperties *fp = iFontProp->second;		

		if(fp->glyph_cache)
		{
			for(std::map<uint16, FontGlyph *>::iterator glyphitr = fp->glyph_cache->begin(); glyphitr != fp->glyph_cache->end(); glyphitr++)
			{
				_DeleteTexture((*glyphitr).second->texture);
				delete (*glyphitr).second;
			}

			fp->glyph_cache->clear();
		}

		++iFontProp;
	}

	return success;
}

//-----------------------------------------------------------------------------
// UnloadTextures: frees the texture memory taken up by the texture sheets, 
//                 but leaves the lists of images intact so we can reload them
//                 Returns false if any of the textures fail to unload.
//-----------------------------------------------------------------------------
bool GameVideo::UnloadTextures() 
{
	// save temporary textures to disk, in other words textures which weren't
	// loaded to a file. This way when we recreate the GL context we will
	// be able to load them again.
	_SaveTempTextures();

	// unload texture sheets	
	vector<TexSheet *>::iterator iSheet    = _texSheets.begin();
	vector<TexSheet *>::iterator iSheetEnd = _texSheets.end();

	bool success = true;
	
	while(iSheet != iSheetEnd)
	{
		TexSheet *sheet = *iSheet;
		
		if(sheet)
		{
			if(!sheet->Unload())
			{
				if(VIDEO_DEBUG)
					cerr << "VIDEO_ERROR: in UnloadTextures(), sheet->Unload() failed!" << endl;
				success = false;
			}
		}
		else
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: in UnloadTextures(), one of the tex sheets in the vector was NULL!" << endl;
			success = false;
		}
		
		
		++iSheet;
	}

	if(_lightOverlay != 0xFFFFFFFF)
	{
		_DeleteTexture(_lightOverlay);
		_lightOverlay = 0xFFFFFFFF;
	}

	return success;
}

//-----------------------------------------------------------------------------
// _DeleteTexture: wraps call to glDeleteTexture(), adds some checking to see
//                if we deleted the last texture we bound using _BindTexture(),
//                then set the last tex ID to 0xffffffff
//-----------------------------------------------------------------------------
bool GameVideo::_DeleteTexture(GLuint texID)
{
	glDeleteTextures(1, &texID);
	
	if(_lastTexID == texID)
		_lastTexID = 0xFFFFFFFF;
	
	if(glGetError())
		return false;
		
	return true;
}

//-----------------------------------------------------------------------------
// Unload: unloads all memory used by OpenGL for this texture sheet
//         Returns false if we fail to unload, or if the sheet was already
//         unloaded
//-----------------------------------------------------------------------------
bool TexSheet::Unload()
{
	// check if we're already unloaded
	if(!loaded)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: unloading an already unloaded texture sheet" << endl;
		return false;
	}
	
	// delete the texture
	hoa_video::GameVideo *videoManager = hoa_video::GameVideo::SingletonGetReference();
	if(!videoManager->_DeleteTexture(texID))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: _DeleteTexture() failed in TexSheet::Unload()!" << endl;
		return false;
	}
	
	loaded = false;
	return true;
}

//-----------------------------------------------------------------------------
// _CreateBlankGLTexture: creates a blank texture of the given width and height
//                       and returns its OpenGL texture ID.
//                       Returns 0xffffffff on failure
//-----------------------------------------------------------------------------
GLuint GameVideo::_CreateBlankGLTexture(int32 width, int32 height)
{
	// attempt to create a GL texture with the given width and height
	// if texture creation fails, return NULL

	int32 error;
			
	GLuint texID;

	glGenTextures(1, &texID);
	error = glGetError();
	
	if(!error)   // if there's no error so far, attempt to bind texture
	{
		_BindTexture(texID);
		error = glGetError();
		
		// if the binding was successful, initialize the texture with glTexImage2D()
		if(!error)
		{		
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			error = glGetError();
		}
	}
		
	if(error != 0)   // if there's an error, delete the texture and return error code
	{
		_DeleteTexture(texID);
		
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: failed to create new texture in _CreateBlankGLTexture()." << endl;
			cerr << "  OpenGL reported the following error:" << endl << "  ";
			char *errString = (char*)gluErrorString(error);
			cerr << errString << endl;
		}
		return 0xffffffff;
	}
	
	// set clamping and filtering parameters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	
	return texID;
}

//-----------------------------------------------------------------------------
// Reload: reallocate memory with OpenGL for this texture and load all the images
//         back into it
//         Returns false if we fail to reload or if the sheet was already loaded
//-----------------------------------------------------------------------------
bool TexSheet::Reload()
{
	// check if we're already loaded
	if(loaded)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: loading an already loaded texture sheet" << endl;
		return false;
	}
	
	// create new OpenGL texture
	hoa_video::GameVideo *videoManager = hoa_video::GameVideo::SingletonGetReference();	
	GLuint tID = videoManager->_CreateBlankGLTexture(width, height);
	
	if(tID == 0xFFFFFFFF)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: _CreateBlankGLTexture() failed in TexSheet::Reload()!" << endl;
		return false;
	}
	
	texID = tID;
	
	// now the hard part: go through all the images that belong to this texture
	// and reload them again. (GameVideo's function, _ReloadImagesToSheet does this)

	if(!videoManager->_ReloadImagesToSheet(this))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: CopyImagesToSheet() failed in TexSheet::Reload()!" << endl;
		return false;
	}
	
	loaded = true;	
	return true;
}

//-----------------------------------------------------------------------------
// _ReloadImagesToSheet: helper function of the GameVideo class to
//                      TexSheet::Reload() to do the dirty work of reloading
//                      image data into the appropriate spots on the texture
//-----------------------------------------------------------------------------
bool GameVideo::_ReloadImagesToSheet(TexSheet *sheet)
{
	// delete images
	map<string, Image *>::iterator iImage     = _images.begin();
	map<string, Image *>::iterator iImageEnd  = _images.end();

	bool success = true;
	while(iImage != iImageEnd)
	{
		Image *i = iImage->second;
		if(i->texture_sheet == sheet)
		{
			ImageLoadInfo loadInfo;
			
			if(!_LoadRawImage(i->filename, loadInfo))
			{
				if(VIDEO_DEBUG)
					cerr << "VIDEO ERROR: _LoadRawImage() failed in _ReloadImagesToSheet()!" << endl;
				success = false;
			}			
			
			if(!sheet->CopyRect(i->x, i->y, loadInfo))
			{
				if(VIDEO_DEBUG)
					cerr << "VIDEO ERROR: sheet->CopyRect() failed in _ReloadImagesToSheet()!" << endl;
				success = false;
			}
		}
		++iImage;
	}
	
	return success;
}

//-----------------------------------------------------------------------------
// _SaveTempTextures: save all textures to disk which were not loaded from a file
//-----------------------------------------------------------------------------
bool GameVideo::_SaveTempTextures()
{
	map<string, Image*>::iterator iImage = _images.begin();
	map<string, Image*>::iterator iEnd   = _images.end();
	
	while(iImage != iEnd)
	{
		Image *image = iImage->second;
		
		// it's a temporary texture!!
		if(image->filename.find("TEMP_") != string::npos)
		{
			image->texture_sheet->SaveImage(image);
		}
		
		++iImage;
	}
	return true;
}

//-----------------------------------------------------------------------------
// _DeleteTempTextures: delete all the textures in the temp directory
//-----------------------------------------------------------------------------
bool GameVideo::_DeleteTempTextures()
{
	return CleanDirectory("temp");
}

//-----------------------------------------------------------------------------
// FIXME: Unused?  Wrong definition? SaveImage: saves the image to the given filename
//-----------------------------------------------------------------------------
bool TexSheet::SaveImage(Image *img)
{
	// uint8 *pixels = new uint8[width*height*4];
	GameVideo *videoManager = GameVideo::SingletonGetReference();
	videoManager->_BindTexture(texID);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	
	return false;
}


}  // namespace hoa_video

