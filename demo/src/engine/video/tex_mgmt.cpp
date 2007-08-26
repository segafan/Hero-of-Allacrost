///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    texture_controller.cpp
*** \author  Tyler Olsen, roots@allacrost.org
*** \brief   Source file for texture management code
*** ***************************************************************************/

// #include <cassert>
// #include <cstdarg>
// #include <set>
// #include <fstream>
// #include <math.h>

#include "utils.h"
#include "tex_mgmt.h"
#include "video.h"
#include "gui.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_video::private_video;

template<> hoa_video::TextureController* Singleton<hoa_video::TextureController>::_singleton_reference = NULL;

namespace hoa_video {

TextureController* TextureManager = NULL;



TextureController::TextureController() :
	_last_tex_id(INVALID_TEXTURE_ID),
	_debug_current_sheet(-1),
	_debug_num_tex_switches(0)
{}



TextureController::~TextureController() {
	for (vector<TexSheet*>::iterator i = _tex_sheets.begin(); i != _tex_sheets.end(); i++) {
		delete *i;
	}

	for (map<string, Image*>::iterator i = _images.begin(); i != _images.end(); i++) {
		delete i->second;
	}
}



bool TextureController::SingletonInitialize() {
	// Create a default set of texture sheets
	if (_CreateTexSheet(512, 512, VIDEO_TEXSHEET_32x32, false) == NULL) {
		PRINT_ERROR << "could not create default 32x32 texture sheet" << endl;
		return false;
	}
	if (_CreateTexSheet(512, 512, VIDEO_TEXSHEET_32x64, false) == NULL) {
		PRINT_ERROR << "could not create default 32x64 texture sheet" << endl;
		return false;
	}
	if (_CreateTexSheet(512, 512, VIDEO_TEXSHEET_64x64, false) == NULL) {
		PRINT_ERROR << "could not create default 64x64 texture sheet" << endl;
		return false;
	}
	if (_CreateTexSheet(512, 512, VIDEO_TEXSHEET_ANY, true) == NULL) {
		PRINT_ERROR << "could not create default static variable sized texture sheet" << endl;
		return false;
	}
	if (_CreateTexSheet(512, 512, VIDEO_TEXSHEET_ANY, false) == NULL) {
		PRINT_ERROR << "could not create default variable sized tex sheet" << endl;
		return false;
	}

	return true;
}




bool TextureController::UnloadTextures() {
	bool success = true;

	// Save temporary textures to disk, in other words textures which were not
	// loaded from a file. This way when we recreate the GL context we will
	// be able to load them again.
	if (_SaveTempTextures() == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "call to _SaveTempTextures() failed" << endl;
		success = false;
	}

	// Unload all texture sheets
	vector<TexSheet*>::iterator i = _tex_sheets.begin();
	while (i != _tex_sheets.end()) {
		if (*i != NULL) {
			if ((*i)->Unload() == false) {
				IF_PRINT_WARNING(VIDEO_DEBUG) << "a TextureSheet::Unload() call failed" << endl;
				success = false;
			}
		}
		else {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "a NULL TextureSheet was found in the _tex_sheets container" << endl;
			success = false;
		}

		i++;
	}

	// Delete the light overlay texture
	if (VideoManager->_light_overlay != INVALID_TEXTURE_ID) {
		_DeleteTexture(VideoManager->_light_overlay);
		VideoManager->_light_overlay = INVALID_TEXTURE_ID;
	}

	// Clear all font caches
	map<string, FontProperties*>::iterator j = VideoManager->_font_map.begin();
	while (j != VideoManager->_font_map.end()) {
		FontProperties *fp = j->second;

		if (fp->glyph_cache) {
			for (map<uint16, FontGlyph*>::iterator k = fp->glyph_cache->begin(); k != fp->glyph_cache->end(); k++) {
				_DeleteTexture((*k).second->texture);
				delete (*k).second;
			}

			fp->glyph_cache->clear();
		}

		j++;
	}

	return success;
} // bool TextureController::UnloadTextures()



bool TextureController::ReloadTextures() {
	bool success = true;
	vector<TexSheet*>::iterator i = _tex_sheets.begin();

	while (i != _tex_sheets.end()) {
		if (*i != NULL) {
			if ((*i)->Reload() == false) {
				IF_PRINT_WARNING(VIDEO_DEBUG) << "a TextureSheet::Reload() call failed" << endl;
				success = false;
			}
		}
		else {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "a NULL TextureSheet was found in the _tex_sheets container" << endl;
			success = false;
		}

		i++;
	}

	_DeleteTempTextures();

	if (VideoManager->_uses_lights)
		VideoManager->_light_overlay = _CreateBlankGLTexture(1024, 1024);

	return success;
}



void TextureController::DEBUG_NextTexSheet() {
	_debug_current_sheet++;

	if (_debug_current_sheet >= static_cast<int32>(_tex_sheets.size()))
		_debug_current_sheet = -1;  // Disables texture sheet display
}



void TextureController::DEBUG_PrevTexSheet() {
	_debug_current_sheet--;

	if (_debug_current_sheet < -1)
		_debug_current_sheet = static_cast<int32>(_tex_sheets.size()) - 1;
}



GLuint TextureController::_CreateBlankGLTexture(int32 width, int32 height) {
	GLuint tex_id;
	glGenTextures(1, &tex_id);

	if (VideoManager->CheckGLError()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "an OpenGL error was detected: " << VideoManager->CreateGLErrorString() << endl;
		_DeleteTexture(tex_id);
		return INVALID_TEXTURE_ID;
	}

	_BindTexture(tex_id); // NOTE: this call makes another call to VideoManager->CheckGLError()

	// If the binding was successful, initialize the texture with glTexImage2D()
	if (VideoManager->GetGLError() == GL_NO_ERROR) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	if (VideoManager->CheckGLError()) {
		PRINT_ERROR << "failed to create new texture. OpenGL reported the following error: " << VideoManager->CreateGLErrorString() << endl;
		_DeleteTexture(tex_id);
		return INVALID_TEXTURE_ID;
	}

	// Set linear texture interpolation only if we are at a non-natural resolution
	GLenum filtering_type = VideoManager->_ShouldSmooth() ? GL_LINEAR : GL_NEAREST;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering_type);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering_type);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	return tex_id;
}



void TextureController::_BindTexture(GLuint tex_id) {
	if (tex_id != _last_tex_id) {
		_last_tex_id = tex_id;
		glBindTexture(GL_TEXTURE_2D, tex_id);
		_debug_num_tex_switches++;
	}

	if (VideoManager->CheckGLError()) {
		PRINT_WARNING << "an OpenGL error was detected: " << VideoManager->CreateGLErrorString() << endl;
	}
}



void TextureController::_DeleteTexture(GLuint tex_id) {
	glDeleteTextures(1, &tex_id);

	if (_last_tex_id == tex_id)
		_last_tex_id = INVALID_TEXTURE_ID;

	if (VideoManager->CheckGLError()) {
		PRINT_WARNING << "an OpenGL error was detected: " << VideoManager->CreateGLErrorString() << endl;
	}
}



bool TextureController::_SaveTempTextures() {
	bool success = true;

	for (map<string, Image*>::iterator i = _images.begin(); i != _images.end(); i++) {
		Image *image = i->second;

		// Check that this is a temporary texture and if so, save it to disk as a .png file
		if (image->tags.find("<T>") != string::npos) {
			hoa_video::private_video::ImageLoadInfo buffer;
			buffer.CopyFromImage(image);
			if (VideoManager->_SavePng("img/temp/" + image->filename + ".png", buffer) == false) {
				success = false;
				IF_PRINT_WARNING(VIDEO_DEBUG) << "call to _SavePng() failed" << endl;
			}
		}
	}
	return success;
}



TexSheet* TextureController::_CreateTexSheet(int32 width, int32 height, TexSheetType type, bool is_static) {
	// Validate that the function arguments are appropriate values
	if (!IsPowerOfTwo(width) || !IsPowerOfTwo(height)) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "non power-of-two width and/or height argument" << endl;
		return NULL;
	}

	if (type <= VIDEO_TEXSHEET_INVALID || type >= VIDEO_TEXSHEET_TOTAL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "invalid TexSheetType argument" << endl;
		return NULL;
	}

	// Create a blank texture for the sheet to use
	GLuint tex_id = _CreateBlankGLTexture(width, height);
	if (tex_id == INVALID_TEXTURE_ID) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to create a new blank OpenGL texture" << endl;
		return NULL;
	}

 	TexSheet *sheet = new TexSheet(width, height, tex_id, type, is_static);
	_tex_sheets.push_back(sheet);
	return sheet;
}



void TextureController::_RemoveSheet(TexSheet* sheet) {
	if (sheet == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "NULL argument passed to function" << endl;
		return;
	}

	if (_tex_sheets.empty()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "no texture sheets were loaded when function was called" << endl;
		return;
	}

	vector<TexSheet*>::iterator i = _tex_sheets.begin();

	while(i != _tex_sheets.end()) {
		if (*i == sheet) {
			delete sheet;
			_tex_sheets.erase(i);
			return;
		}
		i++;
	}

	IF_PRINT_WARNING(VIDEO_DEBUG) << "could not find texture sheet to delete" << endl;
}



TexSheet* TextureController::_InsertImageInTexSheet(BaseImage *image, private_video::ImageLoadInfo & load_info, bool is_static) {
	// Image sizes larger than 512 in either dimension require their own texture sheet
	if (load_info.width > 512 || load_info.height > 512) {
		int32 round_width = RoundUpPow2(load_info.width);
		int32 round_height = RoundUpPow2(load_info.height);
		TexSheet* sheet = _CreateTexSheet(round_width, round_height, VIDEO_TEXSHEET_ANY, false);

		// Ran out of memory!
		if (sheet == NULL) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "could not create new texture sheet for image" << endl;
			return NULL;
		}

		if (sheet->AddImage(image, load_info) == true)
			return sheet;
		else {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "TexSheet::AddImage returned false when trying to insert a large image" << endl;
			return NULL;
		}
	}

	// Determine the type of texture sheet that should hold this image
	TexSheetType type;

	if (load_info.width == 32 && load_info.height == 32)
		type = VIDEO_TEXSHEET_32x32;
	else if (load_info.width == 32 && load_info.height == 64)
		type = VIDEO_TEXSHEET_32x64;
	else if (load_info.width == 64 && load_info.height == 64)
		type = VIDEO_TEXSHEET_64x64;
	else
		type = VIDEO_TEXSHEET_ANY;

	// Look through all existing texture sheets and see if the image will fit in any of the ones which
	// match the type and static status that we are looking for
	for (uint32 i = 0; i < _tex_sheets.size(); i++) {
		TexSheet* sheet = _tex_sheets[i];
		if (sheet == NULL) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "found a NULL texture sheet in the _tex_sheets container" << endl;
			continue;
		}

		if (sheet->type == type && sheet->is_static == is_static) {
			if (sheet->AddImage(image, load_info) == true) {
				return sheet;
			}
		}
	}

	// We couldn't add it to any existing sheets, so we must create a new one for it
	TexSheet *sheet = _CreateTexSheet(512, 512, type, is_static);
	if (sheet == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to create a new texture sheet for image" << endl;
		return NULL;
	}

	// AddImage should always work here. If not, there is a serious problem
	if (sheet->AddImage(image, load_info)) {
		return sheet;
	}
	else {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "all attempts to add image to a texture sheet have failed" << endl;
		return NULL;
	}
} // TexSheet* TextureController::_InsertImageInTexSheet(BaseImage *image, private_video::ImageLoadInfo & load_info, bool is_static)



bool TextureController::_ReloadImagesToSheet(TexSheet* sheet) {
	// Delete images
	std::map<string, private_video::MultiImageInfo> multi_image_info;

	bool success = true;
	for (map<string, Image *>::iterator i = _images.begin(); i != _images.end(); i++) {
		// Only operate on images which belong to the requested TexSheet
		if (i->second->texture_sheet != sheet) {
			continue;
		}

		Image* img = i->second;
		ImageLoadInfo load_info;
		bool is_multi_image = (img->tags.find("<X", 0) != img->filename.npos);

		// Multi Images require a different reloading process
		if (is_multi_image) {
			ImageLoadInfo image;

			if (multi_image_info.find(img->filename) == multi_image_info.end()) {
				// Load the image
				if (VideoManager->_LoadRawImage(img->filename, load_info) == false) {
					IF_PRINT_WARNING(VIDEO_DEBUG) << "call to _LoadRawImage() failed" << endl;
					success = false;
					continue;
				}

				// Copy the part of the image in a buffer
				image.height = img->height;
				image.width = img->width;
				image.pixels = malloc(image.height * image.width * 4);

				if (image.pixels == NULL) {
					IF_PRINT_WARNING(VIDEO_DEBUG) << "call to malloc returned NULL" << endl;
					success = false;
					continue;
				}

				MultiImageInfo info;
				info.multi_image = load_info;
				info.image = image;
				multi_image_info[img->filename] = info;
			}
			else {
				load_info = multi_image_info[img->filename].multi_image;
				image = multi_image_info[img->filename].image;
			}

			uint16 pos0, pos1; // Used to find the start and end positions of a sub-string
			uint32 x, y; //
			uint32 rows, cols;
			
			pos0 = img->tags.find("<X", 0);
			pos1 = img->tags.find('_', pos0);
			x = atoi(img->tags.substr(pos0 + 2, pos1).c_str());
			
			pos0 = img->tags.find("<Y", 0);
			pos1 = img->tags.find('_', pos0);
			y = atoi(img->tags.substr(pos0 + 2, pos1).c_str());

			rows = load_info.height / image.height;
			cols = load_info.width / image.width;

			for (int32 row = 0; row < image.height; row++) {
				memcpy((uint8*)image.pixels + 4 * image.width * row, (uint8*)load_info.pixels + (((x * load_info.height / rows) + row)
					* load_info.width + y * load_info.width / cols) * 4, 4 * image.width);
			}

			// Convert to grayscale if needed
			if (img->tags.find("<G>", 0) != img->filename.npos)
				image.ConvertToGrayscale();

			// Copy the image into the texture sheet
			if (sheet->CopyRect(img->x, img->y, image) == false) {
				IF_PRINT_WARNING(VIDEO_DEBUG) << "call to TexSheet::CopyRect() failed" << endl;
				success = false;
			}
		} // if (is_multi_image)

		// Reload a normal image file
		else {
			std::string fname = img->filename;

			// Check if it is a temporary image, and if so retrieve it from the img/temp directory
			if (img->tags.find("<T>", 0) != img->tags.npos) {
				fname = "img/temp/" + fname + ".png";
			}

			if (VideoManager->_LoadRawImage(fname, load_info) == false) {
				IF_PRINT_WARNING(VIDEO_DEBUG) << "call to _LoadRawImage() failed" << endl;
				success = false;
			}

			// Convert to grayscale if needed
			if (img->tags.find("<G>", 0) != img->filename.npos)
				load_info.ConvertToGrayscale();

			if (sheet->CopyRect(img->x, img->y, load_info) == false) {
				IF_PRINT_WARNING(VIDEO_DEBUG) << "call to TexSheet::CopyRect() failed" << endl;
				success = false;
			}

			if (load_info.pixels)
				free(load_info.pixels);
		}
	} // for (map<string, Image *>::iterator i = _images.begin(); i != _images.end(); i++)

	for (map<string,MultiImageInfo>::iterator i = multi_image_info.begin(); i != multi_image_info.end(); ++i) {
		free(i->second.multi_image.pixels);
		free(i->second.image.pixels);
	}

	// Regenerate all font textures
	for (set<TextImage*>::iterator i = _text_images.begin(); i != _text_images.end(); i++) {
		if ((*i)->texture_sheet == sheet) {
			if ((*i)->Reload() == false) {
				IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to reload a TextImage" << endl;
				success = false;
			}
		}
	}

	return success;
} // bool TextureController::_ReloadImagesToSheet(TexSheet* sheet)



void TextureController::_RegisterTextImage(TextImage* img) {
	if (_IsTextImageRegistered(img) == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "attempted to register an already registered TextImage" << endl;
		return;
	}
	_text_images.insert(img);
}



void TextureController::_DEBUG_ShowTexSheet() {
	// Value less than zero means we shouldn't show any texture sheets
	if (_debug_current_sheet < 0) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "function was called when _debug_current_sheet was not a positive value" << endl;
		return;
	}

	if (_tex_sheets.empty()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "there were no texture sheets available to show" << endl;
		return;
	}

	// If we were viewing a particular texture sheet and it happened to get deleted, we change
	// to look at a different sheet
	int32 num_sheets = static_cast<uint32>(_tex_sheets.size());

	if (_debug_current_sheet >= num_sheets) {
		_debug_current_sheet = num_sheets - 1;
	}

	TexSheet *sheet = _tex_sheets[_debug_current_sheet];
	if (sheet == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "discovered a NULL texture sheet in _tex_sheets container" << endl;
		return;
	}

	int32 w = sheet->width;
	int32 h = sheet->height;

	Image img(sheet, string(), "<T>", 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, w, h, false);


	VideoManager->PushState();
	VideoManager->SetDrawFlags(VIDEO_NO_BLEND, VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	VideoManager->SetCoordSys(0.0f, 1024.0f, 0.0f, 760.0f);

	glPushMatrix();
	VideoManager->Move(0.0f,0.0f);
	glScalef(0.5f, 0.5f, 0.5f);

	ImageElement elem(&img, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, static_cast<float>(w), static_cast<float>(h));
	StillImage id;
	id._elements.push_back(elem);

	VideoManager->DrawImage(id);
	glPopMatrix();

	VideoManager->SetFont("debug_font");
	char buf[200];

	VideoManager->Move(20, VideoManager->_current_context.coordinate_system.GetTop() - 30);
	VideoManager->DrawText("Current Texture sheet:");

	sprintf(buf, "  Sheet #: %d", _debug_current_sheet);
	VideoManager->MoveRelative(0, -20);
	VideoManager->DrawText(buf);

	VideoManager->MoveRelative(0, -20);
	sprintf(buf, "  Size:    %dx%d", sheet->width, sheet->height);
	VideoManager->DrawText(buf);

	if (sheet->type == VIDEO_TEXSHEET_32x32)
		sprintf(buf, "  Type:    32x32");
	else if (sheet->type == VIDEO_TEXSHEET_32x64)
		sprintf(buf, "  Type:    32x64");
	else if (sheet->type == VIDEO_TEXSHEET_64x64)
		sprintf(buf, "  Type:    64x64");
	else if (sheet->type == VIDEO_TEXSHEET_ANY)
		sprintf(buf, "  Type:    Any size");
	else
		sprintf(buf, "  Type:    Unknown");

	VideoManager->MoveRelative(0, -20);
	VideoManager->DrawText(buf);

	sprintf(buf, "  Static:  %d", sheet->is_static);
	VideoManager->MoveRelative(0, -20);
	VideoManager->DrawText(buf);

	sprintf(buf, "  TexID:   %d", sheet->tex_id);
	VideoManager->MoveRelative(0, -20);
	VideoManager->DrawText(buf);

	VideoManager->PopState();
} // void TextureController::_DEBUG_ShowTexSheet()



//-----------------------------------------------------------------------------
// _DeleteImage: decreases the reference count on an image, and deletes it
//               if zero is reached. Note that for images larger than 512x512,
//               there is no reference counting; we just delete it immediately
//               because we don't want huge textures sitting around in memory
//-----------------------------------------------------------------------------

bool GameVideo::_DeleteImage(BaseImage *const base_img) {
	// If the image is grayscale, also perform a delete for the color image one
	if (base_img->grayscale) {
		Image *img = dynamic_cast<Image *>(base_img);
		if (img)
		{
			// The filename of the color image is the grayscale one but without "_grayscale" (10 characters) at the end
			string filename (img->filename,0,img->filename.length()-10);

			map<string,Image*>::iterator it = TextureManager->_images.find(filename);
			if (it == TextureManager->_images.end())
			{
				if (VIDEO_DEBUG)
					cerr << "Attemp to delete a color copy didn't work" << endl;
				return false;
			}

			_DeleteImage(it->second);
		}
		else
		{
			if (VIDEO_DEBUG)
				cerr << "_DeleteImage(): Error: Grayscale BaseImage* was not of dynamic type Image*." << endl;
			return false;
		}
	}

	if (base_img->Remove()) {
 		if (base_img->width > 512 || base_img->height > 512) {
			// Remove the image and texture sheet completely
			// TODO: This introduces a seg fault when TexSheet::FreeImage is later called. Fix this bug!
 			_RemoveImage(base_img);
 			TextureManager->_RemoveSheet(base_img->texture_sheet);
 		}
 		else {
			// For smaller images, simply mark them as free 
			// in the memory manager
			base_img->texture_sheet->FreeImage(base_img);
 		}
	}

	return true;
}



void GameVideo::DeleteImage(ImageDescriptor &id) {
	if (id._animated) {
		_DeleteImage(dynamic_cast<AnimatedImage&>(id));
	}
	else {
		_DeleteImage(dynamic_cast<StillImage&>(id));
	}
}



void GameVideo::_DeleteImage(AnimatedImage &id) {
	for (uint32 i = 0; i < id.GetNumFrames(); i++) {
		_DeleteImage(id._frames[i].image);
	}
}

//-----------------------------------------------------------------------------
// _DeleteImage: decrements the reference count for all images composing this
//              image descriptor.
//
// NOTE: for images which are 1024x1024 or higher, once their reference count
//       reaches zero, they're immediately deleted. (don't want to keep those
//       in memory if possible). For others, they're simply marked as "free"
//-----------------------------------------------------------------------------

void GameVideo::_DeleteImage(StillImage &id) {
	// Loop through all the images inside this descriptor
	for (vector<ImageElement>::iterator i = id._elements.begin(); i != id._elements.end(); i++) {
		Image *img = (*i).image;

		// Only delete the image if the pointer is valid. Some ImageElements
		// have a NULL pointer because they are just colored quads
		if (img)
			_DeleteImage(img);

	}

	id._elements.clear();
	id._filename = "";
	id._height = id._width = 0;
	id._is_static = 0;
}

//-----------------------------------------------------------------------------
// _RemoveImage(BaseImage*): removes the child image class from its specific storage method
//-----------------------------------------------------------------------------

bool GameVideo::_RemoveImage(BaseImage *base_image)
{
	Image  *img;
	TextImage *timage;

	img = dynamic_cast<Image*>(base_image);
	if (img)
		return _RemoveImage(img);
	
	timage = dynamic_cast<TextImage*>(base_image);
	if (timage)
		return _RemoveImage(timage);
	
	if (VIDEO_DEBUG)
		cerr << "_RemoveImage(BaseImage*): Dynamic cast failed for all accepted types." << endl;

	return false;
}


//-----------------------------------------------------------------------------
// _RemoveImage(Image*): removes the image pointer from the std::map
//-----------------------------------------------------------------------------

bool GameVideo::_RemoveImage(Image *img)
{
	// Nothing to do if img is null
	if(!img)
		return true;

	if(TextureManager->_images.empty())
	{
		return false;
	}

	map<string, Image*>::iterator iImage = TextureManager->_images.begin();
	map<string, Image*>::iterator iEnd   = TextureManager->_images.end();

	// Search std::map for pointer matching img and remove it
	while(iImage != iEnd)
	{
		if(iImage->second == img)
		{
			delete img;
			TextureManager->_images.erase(iImage);
			return true;
		}
		++iImage;
	}

	// Couldn't find the image
	return false;
}


//-----------------------------------------------------------------------------
// _RemoveImage: removes the timage pointer from the std::set
//-----------------------------------------------------------------------------

bool GameVideo::_RemoveImage(TextImage *img)
{
	// Nothing to do if img is null
	if(!img)
		return true;

	if(TextureManager->_text_images.empty())
	{
		return false;
	}

	if(TextureManager->_IsTextImageRegistered(img))
	{
		TextureManager->_text_images.erase(img);
		delete img;
		return true;
	}

	// Couldn't find the image
	return false;
}



//-----------------------------------------------------------------------------
// GetImageInfo: Gets cols, rows and bpp from an image file
//-----------------------------------------------------------------------------
bool GameVideo::GetImageInfo (const std::string& file_name, uint32 &rows, uint32& cols, uint32& bpp)
{
	// Isolate the extension
	size_t extpos = file_name.rfind('.');

	if(extpos == string::npos)
	{
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: image file extension not specified when calling to GetImageInfo" << endl;
		return false;
	}

	std::string extension = std::string(file_name, extpos, file_name.length() - extpos);

	if(extension == ".jpeg" || extension == ".jpg")
		return _GetImageInfoJpeg(file_name, rows, cols, bpp);
	if(extension == ".png")
		return _GetImageInfoPng(file_name, rows, cols, bpp);

	if (VIDEO_DEBUG)
		cerr << "VIDEO ERROR: image file extension not recognized when calling to GetImageInfo" << endl;

	return false;
}


//-----------------------------------------------------------------------------
// GetImageInfoPng: Gets cols, rows and bpp from a PNG image file
//-----------------------------------------------------------------------------
bool GameVideo::_GetImageInfoPng (const std::string& file_name, uint32 &rows, uint32& cols, uint32& bpp)
{
	FILE * fp = fopen(file_name.c_str(), "rb");

	if(fp == NULL)
		return false;

	uint8 test_buffer[8];

	fread(test_buffer, 1, 8, fp);
	if(png_sig_cmp(test_buffer, 0, 8))
	{
		fclose(fp);
		return false;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);

	if(!png_ptr)
	{
		fclose(fp);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if(!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, NULL, (png_infopp)NULL);
		fclose(fp);
		return false;
	}

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, NULL, (png_infopp)NULL);
		fclose(fp);
		return false;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);

	cols = info_ptr->width;
	rows = info_ptr->height;
	bpp = info_ptr->channels * 8;

	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	fclose (fp);

	return true;
}


//-----------------------------------------------------------------------------
// GetImageInfoJpeg: Gets cols, rows and bpp from a JPEG image file
//-----------------------------------------------------------------------------
bool GameVideo::_GetImageInfoJpeg (const std::string& file_name, uint32 &rows, uint32& cols, uint32& bpp)
{
	FILE * infile;

	if((infile = fopen(file_name.c_str(), "rb")) == NULL)
		return false;

	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);

	cols = cinfo.output_width;
	rows = cinfo.output_height;
	bpp = cinfo.output_components;

	jpeg_destroy_decompress(&cinfo);

	fclose(infile);

	return true;
}


//-----------------------------------------------------------------------------
// LoadImage: loads an image (static image or animated image) and returns true
//            on success
//-----------------------------------------------------------------------------

bool GameVideo::LoadImage(ImageDescriptor &id)
{
	if(id._animated)
	{
		// First load the image
		if (!_LoadImage(dynamic_cast<AnimatedImage &>(id)))
		{
			return false;
		}

		// Then turn it grayscale if needed
		if (id.IsGrayScale())
		{
			dynamic_cast<AnimatedImage &>(id).EnableGrayScale();
		}
	}
	else
	{
		// First load the image
		if (!_LoadImage(dynamic_cast<StillImage &>(id)))
		{
			return false;
		}

		// Then turn it grayscale if needed
		if (id.IsGrayScale())
		{
			dynamic_cast<StillImage &>(id).EnableGrayScale();
		}
	}

	return true;
}




//-----------------------------------------------------------------------------
// _LoadImage: helper function to load an animated image
//-----------------------------------------------------------------------------

bool GameVideo::_LoadImage(AnimatedImage &id)
{
	uint32 num_frames = static_cast<uint32>(id._frames.size());

	bool success = true;

	// Go through all the frames and load anything that hasn't already been loaded
	for(uint32 frame = 0; frame < num_frames; ++frame)
	{
		// If the API user passes only filenames to AddFrame(), then we need to load
		// the images, but if a static image is passed directly, then we can skip
		// loading

		bool need_to_load = id._frames[frame].image._elements.empty();

		if(need_to_load)
		{
			success &= _LoadImage(id._frames[frame].image);
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

bool GameVideo::_LoadImage(StillImage &id)
{
	// Delete everything previously stored in here
	id._elements.clear();

	// 1. Special case: if filename is empty, load a colored quad
	if(id._filename.empty())
	{
		ImageElement quad(NULL, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, id._width, id._height, id._color);
		id._elements.push_back(quad);
		return true;
	}

	// 2. Check if an image with the same filename has already been loaded
	//    If so, point to that
	if(TextureManager->_images.find(id._filename) != TextureManager->_images.end())
	{
		Image *img = TextureManager->_images[id._filename];		// Get the image from the map

		if(!img)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: got a NULL Image from images map in LoadImage()" << endl;
			return false;
		}

		if (img->ref_count == 0)
		{
			// If ref count is zero, it means this image was freed, but
			// not removed, so restore it
			img->texture_sheet->RestoreImage(img);

		}

		img->Add();		// Increment the reference counter of the Image

		if(id._width == 0.0f)
			id._width = (float) img->width;
		if(id._height == 0.0f)
			id._height = (float) img->height;

		ImageElement element(img, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, id._width, id._height, id._color);
		id._elements.push_back(element);

		return true;
	}

	// 3. Load the image right away
	bool success = _LoadImageHelper(id);

	if(!success)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: in LoadImage() failed to load " << id._filename << endl;
		return false;
	}

	return success;
}




//-----------------------------------------------------------------------------
// LoadMultiImageFromNumberElements: Opens an image file and breaks it in many StillImages
//-----------------------------------------------------------------------------

bool GameVideo::LoadMultiImageFromNumberElements(std::vector<StillImage> &images, const std::string &file_name, const uint32 rows, const uint32 cols)
{
	// Get the size of the image
	uint32 image_rows, image_cols, bpp;
	GetImageInfo(file_name, image_rows, image_cols, bpp);

	// If all the images are not the same size, then there is an error
	if ((image_rows % rows) || (image_cols % cols))
	{
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: loading multi image, the size of the image (file) is not multiple of the size of the images (StillImage)" << endl;
		return false;
	}

	// If the images vector has not the same size of the required elements, redimension the vector
	if (images.size()!= rows*cols)
	{
		images.resize(rows*cols);
	}

	// If the width and height was not specified (0.0f), then assume the size of the image element
	float width ((float)image_cols / (float)cols);
	float height ((float)image_rows / (float)rows);
	for (std::vector<StillImage>::iterator it=images.begin(); it<images.end(); ++it)
	{
		if (it->_height == 0.0f)
			it->_height = height;
		if (it->_width == 0.0f)
			it->_width = width;
	}

	// Load the images
	return _LoadMultiImage (images, file_name, rows, cols);
}



//-----------------------------------------------------------------------------
// LoadMultiImageFromElementsSize: Opens an image file and breaks it in many frames of an AnimateImage
//-----------------------------------------------------------------------------
bool GameVideo::LoadMultiImageFromElementsSize(std::vector<StillImage> &images, const std::string &file_name, const uint32 width, const uint32 height)
{
	// Get the size of the image
	uint32 image_rows, image_cols, bpp;
	GetImageInfo(file_name, image_rows, image_cols, bpp);

	// If all the images are not the same size, then there is an error
	if ((image_rows % height) || (image_cols % width))
	{
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: loading multi image, the size of the image (file) is not multiple of the size of the images (StillImage)" << endl;
		return false;
	}

	uint32 rows (image_rows / height);
	uint32 cols (image_cols / width);

	// If the images vector has not the same size of the required elements, redimension the vector
	// Using this multi image loader probably means tat the images vector is empty
	if (images.size() != rows*cols)
	{
		images.resize(rows*cols);
	}

	// If the width and height was not specified (0.0f), then assume the size of the image element
	for (std::vector<StillImage>::iterator it=images.begin(); it<images.end(); ++it)
	{
		if (it->_height == 0.0f)
			it->_height = (float)height;
		if (it->_width == 0.0f)
			it->_width = (float)width;
	}

	// Load the images
	return _LoadMultiImage (images, file_name, rows, cols);
}


//-----------------------------------------------------------------------------
// _LoadMultiImage: Breaks an image in several StillImages
//-----------------------------------------------------------------------------
bool GameVideo::_LoadMultiImage (std::vector <StillImage>& images, const std::string &file_name, const uint32& rows,
								 const uint32& cols)
{
	std::string tags;
	std::string s;
	uint32 current_image;
	uint32 x, y;

	bool need_load = false;

	// Check if we have loaded all the sub-images
	for (x=0; x<rows && !need_load; x++)
	{
		for (y=0; y<cols && !need_load; y++)
		{
			tags = "";
			DataToString(s,x);
			tags += "<X" + s + "_";
			DataToString(s,rows);
			tags += s + ">";
			DataToString(s,y);
			tags += "<Y" + s + "_";
			DataToString(s,cols);
			tags += s + ">";

			// If this image doesn't exist, don't do anything else.
			// We will have to load the image file
			if(TextureManager->_images.find(file_name+tags) == TextureManager->_images.end())
			{
				need_load = true;
			}
		}
	}

	// If not all the images are loaded, then load the big image from disk
	private_video::MultiImageInfo info;
	if (need_load)
	{
		if(!_LoadRawImage(file_name, info.multi_image))
			return false;

		info.image.width = info.multi_image.width / cols;
		info.image.height = info.multi_image.height / rows;
		info.image.pixels = (uint8 *) malloc(info.image.width*info.image.height*4);
	}

	// One by one, get the subimages
	for (x=0; x<rows; x++)
	{
		for (y=0; y<cols; y++)
		{
			DataToString(s,x);
			tags = "<X" + s + "_";
			DataToString(s,rows);
			tags += s + ">";
			DataToString(s,y);
			tags += "<Y" + s + "_";
			DataToString(s,cols);
			tags += s + ">";

			current_image = x*cols + y;

			// If the image exists, take the information from it
			if(TextureManager->_images.find(file_name+tags) != TextureManager->_images.end())
			{
				images.at(current_image)._elements.clear();

				Image *img = TextureManager->_images[file_name+tags];

				if (!img)
				{
					if(VIDEO_DEBUG)
						cerr << "VIDEO ERROR: got a NULL Image from images map in LoadImage()" << endl;

					if (info.multi_image.pixels)
						free (info.multi_image.pixels);
					if (info.image.pixels)
						free (info.image.pixels);
					return false;
				}

				if (img->ref_count == 0)
				{
					// If ref count is zero, it means this image was freed, but
					// not removed, so restore it
					img->texture_sheet->RestoreImage(img);
				}

				++(img->ref_count);

				ImageElement element(img, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f,
					images.at(current_image)._width, images.at(current_image)._height, images.at(current_image)._color);
				images.at(current_image)._elements.push_back(element);
			}
			else	// If the image is not present, take the piece from the loaded image
			{
				images.at(current_image)._filename = file_name;
				images.at(current_image)._animated = false;

				for (int32 i=0; i<info.image.height; i++)
				{
					memcpy ((uint8*)info.image.pixels+4*info.image.width*i, (uint8*)info.multi_image.pixels+(((x*info.multi_image.height/rows)+i)*info.multi_image.width+y*info.multi_image.width/cols)*4, 4*info.image.width);
				}

				// Create an Image structure and store it our std::map of images
				Image *new_image = new Image(file_name, tags, info.image.width, info.image.height, false);

				// Try to insert the image in a texture sheet
				TexSheet *sheet = TextureManager->_InsertImageInTexSheet(new_image, info.image, images.at(current_image)._is_static);

				if (!sheet)
				{
					// This should never happen, unless we run out of memory or there
					// is a bug in the _InsertImageInTexSheet() function

					if(VIDEO_DEBUG)
						cerr << "VIDEO_DEBUG: GameVideo::_InsertImageInTexSheet() returned NULL!" << endl;

					if (info.multi_image.pixels)
						free (info.multi_image.pixels);
					if (info.image.pixels)
						free (info.image.pixels);
					return false;
				}

				new_image->ref_count = 1;

				// store the image in our std::map
				TextureManager->_images[file_name + tags] = new_image;

				// store the new image element
				ImageElement element(new_image, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, images.at(current_image)._width, images.at(current_image)._height, images.at(current_image)._color);
				images.at(current_image)._elements.push_back(element);
			}

			// If the image is in grayscale, convert it
			if (images.at(current_image)._grayscale)
			{
				images.at(current_image).EnableGrayScale();
			}
		}
	}

	// Free loaded image, in case we used it
	if (info.multi_image.pixels)
		free (info.multi_image.pixels);
	if (info.image.pixels)
		free (info.image.pixels);

	return true;
}


//-----------------------------------------------------------------------------
// LoadAnimatedImageFromNumberElements: Opens an image file and breaks it in many frames of an AnimateImage
//-----------------------------------------------------------------------------

bool GameVideo::LoadAnimatedImageFromNumberElements(AnimatedImage &id, const std::string &filename, const uint32 rows, const uint32 cols)
{
	// If the number of frames and the number of sub-images doesn't match, return
	if (id.GetNumFrames() != rows*cols)
	{
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: The animated image don't have enough frames to hold the data" << endl;
		return false;
	}

	// Get the vector of images
	std::vector <StillImage> v;
	for (uint32 frame=0; frame<id.GetNumFrames(); ++frame)
	{
		v.push_back(id.GetFrame(frame));
	}

	// Load the frames via LoadMultiImage
	bool success = LoadMultiImageFromNumberElements(v, filename, rows, cols);

	// Put the loaded frame images back into the AnimatedImage
	if (success == true) {
		for (uint32 i = 0; i < v.size(); i++) {
			id._frames[i].image = v[i];
		}
	}

	return success;
}


bool GameVideo::LoadAnimatedImageFromElementsSize(AnimatedImage &image, const std::string &file_name, const uint32 rows, const uint32 cols)
{
	return false;
}


//-----------------------------------------------------------------------------
// _LoadImageHelper: private function which does the dirty work of actually
//                     loading an image.
//-----------------------------------------------------------------------------

bool GameVideo::_LoadImageHelper(StillImage &id)
{
	bool is_static = id._is_static;

	id._elements.clear();

	private_video::ImageLoadInfo load_info;

	if(!_LoadRawImage(id._filename, load_info))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: _LoadRawPixelData() failed in _LoadImageHelper()" << endl;
		return false;
	}

	// Create an Image structure and store it our std::map of images (for the color copy, always present)
	Image *new_image = new Image(id._filename, "", load_info.width, load_info.height, false);

	// Try to insert the image in a texture sheet
	TexSheet *sheet = TextureManager->_InsertImageInTexSheet(new_image, load_info, is_static);

	if(!sheet)
	{
		// This should never happen, unless we run out of memory or there
		// is a bug in the _InsertImageInTexSheet() function

		if(VIDEO_DEBUG)
			cerr << "VIDEO_DEBUG: GameVideo::_InsertImageInTexSheet() returned NULL!" << endl;

		delete new_image;
		free (load_info.pixels);
		return false;
	}

	new_image->ref_count = 1;

	// Store the image in our std::map
	TextureManager->_images[id._filename] = new_image;

	// If width or height are zero, that means to use the dimensions of image
	if(id._width == 0.0f)
		id._width = (float) load_info.width;

	if(id._height == 0.0f)
		id._height = (float) load_info.height;

	ImageElement element(new_image, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, id._width, id._height, id._color);
	id._elements.push_back(element);

	// Finally, delete the buffer used to hold the pixel data
	if (load_info.pixels)
		free (load_info.pixels);

	return true;
}




//-----------------------------------------------------------------------------
// _LoadRawImage: Determines which image loader to call
//-----------------------------------------------------------------------------

bool GameVideo::_LoadRawImage(const std::string & filename, private_video::ImageLoadInfo & load_info)
{
	// Isolate the extension
	size_t extpos = filename.rfind('.');

	if(extpos == string::npos)
		return false;

	std::string extension = std::string(filename, extpos, filename.length() - extpos);

	if(extension == ".jpeg" || extension == ".jpg")
		return _LoadRawImageJpeg(filename, load_info);
	if(extension == ".png")
		return _LoadRawImagePng(filename, load_info);

	return false;
}




//-----------------------------------------------------------------------------
// _LoadRawImagePng: Loads a PNG image to RGBA format
//-----------------------------------------------------------------------------

bool GameVideo::_LoadRawImagePng(const std::string &filename, hoa_video::private_video::ImageLoadInfo &load_info)
{
	FILE * fp = fopen(filename.c_str(), "rb");

	if(fp == NULL)
		return false;

	uint8 test_buffer[8];

	fread(test_buffer, 1, 8, fp);
	if(png_sig_cmp(test_buffer, 0, 8))
	{
		fclose(fp);
		return false;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL, NULL, NULL);

	if(!png_ptr)
	{
		fclose(fp);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if(!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, NULL, (png_infopp)NULL);
		fclose(fp);
		return false;
	}

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, NULL, (png_infopp)NULL);
		fclose(fp);
		return false;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);

	uint8** row_pointers = png_get_rows(png_ptr, info_ptr);

	load_info.width = info_ptr->width;
	load_info.height = info_ptr->height;
	load_info.pixels = malloc (info_ptr->width * info_ptr->height * 4);

	uint32 bpp = info_ptr->channels;

	uint8* img_pixel;
	uint8* dst_pixel;

	if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_color c;
		for(uint32 y = 0; y < info_ptr->height; y++)
		{
			for(uint32 x = 0; x < info_ptr->width; x++)
			{
				img_pixel = row_pointers[y] + (x * bpp);
				dst_pixel = ((uint8 *)load_info.pixels) + ((y * info_ptr->width) + x) * 4;
				c = info_ptr->palette[img_pixel[0]];

				dst_pixel[0] = c.red;
				dst_pixel[1] = c.green;
				dst_pixel[2] = c.blue;
				dst_pixel[3] = 0xFF;
			}
		}
	}
	else if (bpp == 1)
	{
		for(uint32 y = 0; y < info_ptr->height; y++)
		{
			for(uint32 x = 0; x < info_ptr->width; x++)
			{
				img_pixel = row_pointers[y] + (x * bpp);
				dst_pixel = ((uint8 *)load_info.pixels) + ((y * info_ptr->width) + x) * 4;
				dst_pixel[0] = img_pixel[0];
				dst_pixel[1] = img_pixel[0];
				dst_pixel[2] = img_pixel[0];
				dst_pixel[3] = 0xFF;
			}
		}
	}
	else if (bpp == 3)
	{
		for(uint32 y = 0; y < info_ptr->height; y++)
		{
			for(uint32 x = 0; x < info_ptr->width; x++)
			{
				img_pixel = row_pointers[y] + (x * bpp);
				dst_pixel = ((uint8 *)load_info.pixels) + ((y * info_ptr->width) + x) * 4;
				dst_pixel[0] = img_pixel[0];
				dst_pixel[1] = img_pixel[1];
				dst_pixel[2] = img_pixel[2];
				dst_pixel[3] = 0xFF;
			}
		}
	}
	else if (bpp == 4)
	{
		for(uint32 y = 0; y < info_ptr->height; y++)
		{
			for(uint32 x = 0; x < info_ptr->width; x++)
			{
				img_pixel = row_pointers[y] + (x * bpp);
				dst_pixel = ((uint8 *)load_info.pixels) + ((y * info_ptr->width) + x) * 4;
				dst_pixel[0] = img_pixel[0];
				dst_pixel[1] = img_pixel[1];
				dst_pixel[2] = img_pixel[2];
				dst_pixel[3] = img_pixel[3];
			}
		}
	}
	else
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose (fp);
		if (VIDEO_DEBUG)
			cerr << "Game Video: Fail in loading Png file (bytes per pixel not supported)" << endl;
		return false;
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	fclose(fp);

	return true;
}




//-----------------------------------------------------------------------------
// _LoadRawImageJpeg: Loads a Jpeg image to RGBA format
//-----------------------------------------------------------------------------

bool GameVideo::_LoadRawImageJpeg(const std::string &filename, hoa_video::private_video::ImageLoadInfo &load_info)
{
	FILE * infile;
	uint8** buffer;

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

	load_info.width = cinfo.output_width;
	load_info.height = cinfo.output_height;
	load_info.pixels = malloc (cinfo.output_width * cinfo.output_height * 4);

	uint32 bpp = cinfo.output_components;

	uint8* img_pixel;
	uint8* dst_pixel;

	if (bpp == 3)
	{
		for(uint32 y = 0; y < cinfo.output_height; y++)
		{
			jpeg_read_scanlines(&cinfo, buffer, 1);

			for(uint32 x = 0; x < cinfo.output_width; x++)
			{
				img_pixel = buffer[0] + (x * bpp);
				dst_pixel = ((uint8 *)load_info.pixels) + ((y * cinfo.output_width) + x) * 4;

				dst_pixel[0] = img_pixel[0];
				dst_pixel[1] = img_pixel[1];
				dst_pixel[2] = img_pixel[2];
				dst_pixel[3] = 0xFF;
			}
		}

	}
	else if (bpp == 4)
	{
		for(uint32 y = 0; y < cinfo.output_height; y++)
		{
			jpeg_read_scanlines(&cinfo, buffer, 1);

			for(uint32 x = 0; x < cinfo.output_width; x++)
			{
				img_pixel = buffer[0] + (x * bpp);
				dst_pixel = ((uint8 *)load_info.pixels) + ((y * cinfo.output_width) + x) * 4;

				dst_pixel[0] = img_pixel[0];
				dst_pixel[1] = img_pixel[1];
				dst_pixel[2] = img_pixel[2];
				dst_pixel[3] = img_pixel[3];
			}
		}
	}
	else
	{
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		if (VIDEO_DEBUG)
			cerr << "Game Video: Fail in loading Png file (bytes per pixel not supported)" << endl;
		return false;
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	fclose(infile);

	return true;
}




//-----------------------------------------------------------------------------
// _SavePng: Stores a buffer in Png format
//-----------------------------------------------------------------------------

bool GameVideo::_SavePng (const std::string& file_name, hoa_video::private_video::ImageLoadInfo &info) const
{
	FILE * fp = fopen(file_name.c_str(), "wb");

	if(fp == NULL)
		return false;

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		(png_voidp)NULL, NULL, NULL);

	if(!png_ptr)
	{
		fclose(fp);
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if(!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, NULL);
		fclose(fp);
		return false;
	}

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return false;
	}

	png_init_io(png_ptr, fp);

	png_set_IHDR (png_ptr, info_ptr, info.width, info.height, 8, PNG_COLOR_TYPE_RGB_ALPHA,
				  PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_byte** row_pointers = new png_byte* [info.height];
	int32 bytes_per_row = info.width * 4;
	for (int32 i=0; i<info.height; i++)
	{
		row_pointers[i] = (png_byte*)info.pixels + bytes_per_row * i;
	}
	png_set_rows(png_ptr, info_ptr, row_pointers);

	png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	png_write_image(png_ptr, row_pointers);

	png_write_end(png_ptr, info_ptr);

	png_destroy_write_struct (&png_ptr, &info_ptr);

	delete[] row_pointers;

	return true;
}




//-----------------------------------------------------------------------------
// _SaveJpeg: Stores a buffer in Jpeg file
//-----------------------------------------------------------------------------

bool GameVideo::_SaveJpeg (const std::string& file_name, hoa_video::private_video::ImageLoadInfo &info) const
{
	FILE * outfile;
	if((outfile = fopen(file_name.c_str(), "wb")) == NULL)
	{
		if (VIDEO_DEBUG)
			cerr << "Game Video: could not save '" << file_name.c_str() << "'" << endl;
		return false;
	}

	jpeg_compress_struct cinfo;
	jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	cinfo.in_color_space = JCS_RGB;
	cinfo.image_width = info.width;
	cinfo.image_height = info.height;
	cinfo.input_components = 3;
	jpeg_set_defaults (&cinfo);

	jpeg_stdio_dest(&cinfo, outfile);

	jpeg_start_compress (&cinfo, TRUE);

	JSAMPROW row_pointer;				// Pointer to a single row
	uint32 row_stride = info.width * 3;	// Physical row width in buffer

	// Note that the lines have to be stored from top to bottom
	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer = (uint8*)info.pixels + cinfo.next_scanline * row_stride;
	    jpeg_write_scanlines(&cinfo, &row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	fclose(outfile);

	return true;
}




//-----------------------------------------------------------------------------
// SaveImage: Saves a vector of images in a single file
//-----------------------------------------------------------------------------

bool GameVideo::SaveImage (const std::string &file_name, const std::vector<StillImage*> &image,
						   const uint32 rows, const uint32 columns) const
{
	enum eLoadType
	{
		NONE	= 0,
		JPEG	= 1,
		PNG		= 2
	} type (NONE);

	// Isolate the extension
	size_t extpos = file_name.rfind('.');

	if(extpos == string::npos)
		return false;

	std::string extension = std::string(file_name, extpos, file_name.length() - extpos);

	if(extension == ".jpeg" || extension == ".jpg")
		type = JPEG;
	if(extension == ".png")
		type = PNG;

	if (type == NONE)
	{
		if (VIDEO_DEBUG)
			cerr << "Game Video: Don't know which format to use for storage of an image" << endl;
		return false;
	}

	// Check there are elements to store
	if (image.empty())
	{
		if (VIDEO_DEBUG)
			cerr << "Game Video: Attempt to store no image" << endl;
		return false;
	}

	// Check if the number of images is compatible with the number of rows and columns
	if (image.size() != rows*columns)
	{
		if (VIDEO_DEBUG)
			cerr << "Game Video: Can't store " << image.size() << " in " << rows << " rows and " << columns << " columns" << endl;
		return false;
	}

	// Check all the images have just 1 ImageElement
	for (uint32 i=0 ; i<image.size(); i++)
	{
		if (image[i]->_elements.size() != 1)
		{
			if (VIDEO_DEBUG)
				cerr << "Game Video: one of the images didn't have 1 ImageElement" << endl;
			return false;
		}
	}

	// Check all the images are of the same size
	int32 width = image[0]->_elements[0].image->width;
	int32 height = image[0]->_elements[0].image->height;
	for (uint32 i = 0; i < image.size(); i++)
	{
		if (!image[i]->_elements[0].image || image[i]->_elements[0].image->width != width ||
			image[i]->_elements[0].image->height != height)
		{
			if (VIDEO_DEBUG)
				cerr << "Game Video: not all the images where of the same size" << endl;
			return false;
		}
	}

	// Structure for the image buffer to save
	hoa_video::private_video::ImageLoadInfo info;
	info.height = rows*height;
	info.width = columns*width;
	info.pixels = malloc (info.width * info.height * 4);

	hoa_video::private_video::Image* img;
	GLuint ID;
	hoa_video::private_video::ImageLoadInfo texture;

	// Do that for the first image (on later ones, maybe we don't need to get again
	// the texture, since it might be the same)
	img = const_cast<Image*>(image[0]->_elements[0].image);
	ID = img->texture_sheet->tex_id;
	texture.width = img->texture_sheet->width;
	texture.height = img->texture_sheet->height;
	texture.pixels = malloc (texture.width * texture.height * 4);
	TextureManager->_BindTexture(ID);
	glGetTexImage (GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.pixels);

	uint32 i=0;
	for (uint32 x=0; x<rows; x++)
	{
		for (uint32 y=0; y<columns; y++)
		{
			img = const_cast<Image*>(image[i]->_elements[0].image);
			if (ID != img->texture_sheet->tex_id)
			{
				// Get new texture ID
				TextureManager->_BindTexture(img->texture_sheet->tex_id);
				ID = img->texture_sheet->tex_id;

				// If the new texture is bigger, reallocate memory
				if (texture.height * texture.width < img->texture_sheet->height * img->texture_sheet->width * 4)
				{
					free (texture.pixels);
					texture.width = img->texture_sheet->width;
					texture.height = img->texture_sheet->height;
					texture.pixels = malloc (texture.width * texture.height * 4);
				}
				glGetTexImage (GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.pixels);
			}

			// Copy the part of the texture we are interested in
			uint32 copy_bytes = width * 4;
			uint32 dst_offset = x*height*width*columns*4 + y*width*4;
			uint32 dst_bytes = width*columns*4;
			uint32 src_bytes = texture.width * 4;
			uint32 src_offset = img->y * texture.width * 4 + img->x * 4;
			for (int32 j = 0; j < height; j++)
			{
				memcpy ((uint8*)info.pixels+j*dst_bytes+dst_offset, (uint8*)texture.pixels+j*src_bytes+src_offset, copy_bytes);
			}

			i++;
		}
	}

	// Store the resultant buffer
	if (type == JPEG)
	{
		info.RGBAToRGB();
		_SaveJpeg(file_name, info);
	}
	else
	{
		_SavePng(file_name, info);
	}

	if (info.pixels)
		free (info.pixels);

	if (texture.pixels)
		free (texture.pixels);

	return true;
}




//-----------------------------------------------------------------------------
// SaveImage: Saves an AnimatedImage as a multiimage
//-----------------------------------------------------------------------------

bool GameVideo::SaveImage (const std::string &file_name, const AnimatedImage &image) const
{
	int32 frame_count = dynamic_cast<const AnimatedImage &>(image).GetNumFrames();
	std::vector <StillImage*> frames;
	frames.reserve (frame_count);

	for (int32 frame=0; frame<frame_count; frame++)
	{
		frames.push_back(dynamic_cast<const AnimatedImage &>(image).GetFrame(frame));
	}

	return SaveImage (file_name, frames, 1, frame_count);
}




//-----------------------------------------------------------------------------
// SaveImage: Saves an image in a file
//-----------------------------------------------------------------------------

bool GameVideo::SaveImage (const std::string &file_name, const StillImage &image) const
{
	enum eLoadType
	{
		NONE	= 0,
		JPEG	= 1,
		PNG		= 2
	} type (NONE);

	// Isolate the extension
	size_t extpos = file_name.rfind('.');

	if(extpos == string::npos)
		return false;

	std::string extension = std::string(file_name, extpos, file_name.length() - extpos);

	if(extension == ".jpeg" || extension == ".jpg")
		type = JPEG;
	if(extension == ".png")
		type = PNG;

	if (type == NONE)
	{
		if (VIDEO_DEBUG)
			cerr << "Game Video: Don't know which format to use for storage of an image" << endl;
		return false;
	}

	// Check there are elements to store
	if (image._elements.empty())
	{
		if (VIDEO_DEBUG)
			cerr << "Game Video: Attempt to store empty image" << endl;
		return false;
	}

	// Still can't store compound images
	if (image._elements.size() > 1)
	{
		if (VIDEO_DEBUG)
			cerr << "Game Video: Compound images can't be stored yet" << endl;
		return false;
	}

	hoa_video::private_video::ImageLoadInfo buffer;
	hoa_video::private_video::Image* img = const_cast<Image*>(image._elements[0].image);

	buffer.CopyFromImage(img);

	if (type == JPEG)
	{
		buffer.RGBAToRGB();
		_SaveJpeg (file_name, buffer);
	}
	else
	{
		_SavePng (file_name, buffer);
	}

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

StillImage GameVideo::TilesToObject (std::vector<StillImage> &tiles, std::vector< std::vector<uint32> > indices)
{
	StillImage id;

	// Figure out the width and height information

	int32 w, h;
	w = (int32) indices[0].size();         // How many tiles wide and high
	h = (int32) indices.size();

	float tile_width  = tiles[0]._width;   // Width and height of each tile
	float tile_height = tiles[0]._height;

	id._width  = (float) w * tile_width;   // Total width/height of compound
	id._height = (float) h * tile_height;

	id._is_static = tiles[0]._is_static;

	for(int32 y = 0; y < h; ++y)
	{
		for(int32 x = 0; x < w; ++x)
		{
			// Add each tile at the correct offset

			float x_offset = x * tile_width;
			float y_offset = y * tile_height;

			if(!id.AddImage(tiles[indices[y][x]], x_offset, y_offset))
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

}  // namespace hoa_video
