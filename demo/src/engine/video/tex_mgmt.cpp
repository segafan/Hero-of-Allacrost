///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    tex_mgmt.h
*** \author  Raj Sharma, roos@allacrost.org
*** \brief   Source file for texture management code
*** ***************************************************************************/

#include <cassert>
#include <cstdarg>
#include <set>
#include <fstream>
#include <math.h>

#include "utils.h"
#include "tex_mgmt.h"
#include "video.h"
#include "gui.h"

using namespace std;
using namespace hoa_utils;

namespace hoa_video {

namespace private_video {

// -----------------------------------------------------------------------------
// TexSheet class
// -----------------------------------------------------------------------------

TexSheet::TexSheet(int32 sheet_width, int32 sheet_height, GLuint sheet_id, TexSheetType sheet_type, bool sheet_static) {
	width = sheet_width;
	height = sheet_height;
	tex_id = sheet_id;
	type = sheet_type;
	is_static = sheet_static;
	loaded = true;

	if (VideoManager->_ShouldSmooth())
		Smooth();

	if (type == VIDEO_TEXSHEET_32x32)
		tex_mem_manager = new FixedTexMemMgr(this, 32, 32);
	else if (type == VIDEO_TEXSHEET_32x64)
		tex_mem_manager = new FixedTexMemMgr(this, 32, 64);
	else if (type == VIDEO_TEXSHEET_64x64)
		tex_mem_manager = new FixedTexMemMgr(this, 64, 64);
	else
		tex_mem_manager = new VariableTexMemMgr(this);
}



TexSheet::~TexSheet() {
	// Delete texture memory manager
	delete tex_mem_manager;

	// Unload actual texture from memory
	VideoManager->_DeleteTexture(tex_id);
}



bool TexSheet::Unload() {
	// Check if we're already unloaded
	if (loaded == false) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: unloading an already unloaded texture sheet" << endl;
		return false;
	}

	// Delete the texture
	if (VideoManager->_DeleteTexture(tex_id) == false) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: _DeleteTexture() failed in TexSheet::Unload()!" << endl;
		return false;
	}

	loaded = false;
	return true;
}



bool TexSheet::Reload() {
	// Check if we're already loaded
	if (loaded == true) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: loading an already loaded texture sheet" << endl;
		return false;
	}

	// Create new OpenGL texture
	GLuint id = VideoManager->_CreateBlankGLTexture(width, height);

	if (id == 0xFFFFFFFF) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: _CreateBlankGLTexture() failed in TexSheet::Reload()!" << endl;
		return false;
	}

	tex_id = id;

	// Restore texture smoothing if applied.
	bool was_smoothed = smoothed;
	smoothed = false;
	Smooth(was_smoothed);

	// Now reload all all of the images that belong to this texture
	if (VideoManager->_ReloadImagesToSheet(this) == false) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: CopyImagesToSheet() failed in TexSheet::Reload()!" << endl;
		return false;
	}

	loaded = true;
	return true;
}



bool TexSheet::AddImage(BaseImage* img, ImageLoadInfo& load_info) {
	// Try inserting into the texture memory manager
	bool could_insert = tex_mem_manager->Insert(img);
	if (could_insert == false)
		return false;

	// Now img contains the x, y, width, and height of the subrectangle
	// inside the texture sheet, so go ahead and copy that area

	TexSheet *tex_sheet = img->texture_sheet;
	if (!tex_sheet) {
		// Technically this should never happen since Insert() returned true
		if (VIDEO_DEBUG) {
			cerr << "VIDEO ERROR: texSheet was NULL after tex_mem_manager->Insert() returned true" << endl;
		}
		return false;
	}

	if (CopyRect(img->x, img->y, load_info) == false) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: CopyRect() failed in TexSheet::AddImage()!" << endl;
		return false;
	}

	return true;
}



bool TexSheet::CopyRect(int32 x, int32 y, ImageLoadInfo& load_info) {
	if (VideoManager->_BindTexture(tex_id) == false) {
		if(VIDEO_DEBUG) {
			cerr << "VIDEO ERROR: could not bind texture in TexSheet::CopyRect()!" << endl;
		}
		return false;
	}

	glTexSubImage2D
	(
		GL_TEXTURE_2D, // target
		0, // level
		x, // x offset within tex sheet
		y, // y offset within tex sheet
		load_info.width, // width in pixels of image
		load_info.height, // height in pixels of image
		GL_RGBA, // format
		GL_UNSIGNED_BYTE, // type
		load_info.pixels // pixels of the sub image
	);

	GLenum error = glGetError();
	if (error) {
		if(VIDEO_DEBUG) {
			cerr << "VIDEO ERROR: glTexSubImage2D() failed in TexSheet::CopyRect()!" << endl;
		}
		return false;
	}

	return true;
}



bool TexSheet::CopyScreenRect(int32 x, int32 y, const ScreenRect& screen_rect) {
	if (VideoManager->_BindTexture(tex_id) == false) {
		if (VIDEO_DEBUG) {
			cerr << "VIDEO ERROR: could not bind texture in TexSheet::CopyScreenRect()!" << endl;
		}
		return false;
	}

	glCopyTexSubImage2D
	(
		GL_TEXTURE_2D, // target
		0, // level
		x, // x offset within tex sheet
		y, // y offset within tex sheet
		screen_rect.left, // left starting pixel of the screen to copy
		screen_rect.top - screen_rect.height, // bottom starting pixel of the screen to copy
		screen_rect.width, // width in pixels of image
		screen_rect.height // height in pixels of image
	);

	GLenum error = glGetError();
	if (error) {
		if (VIDEO_DEBUG) {
			cerr << "VIDEO ERROR: glTexSubImage2D() failed in TexSheet::CopyScreenRect()!" << endl;
		}
		return false;
	}

	return true;
}



void TexSheet::Smooth(bool flag) {
	// In case of global smoothing, do nothing here
	if (VideoManager->_ShouldSmooth())
		return;

	// If setting has changed, set the appropriate filtering
	if (smoothed != flag) {
		smoothed = flag;
		GLenum filtering_type = smoothed ? GL_LINEAR : GL_NEAREST;

		VideoManager->_BindTexture(tex_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering_type);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering_type);
	}
}

// -----------------------------------------------------------------------------
// FixedTexMemMgr class
// -----------------------------------------------------------------------------

FixedTexMemMgr::FixedTexMemMgr(TexSheet *tex_sheet, int32 img_width, int32 img_height) {
	if (tex_sheet == NULL) {
		cerr << "VIDEO WARNING: " << __FILE__ << ":" << __FUNCTION__ << ": tex_sheet argument was NULL" << endl;
		return;
	}

	_tex_sheet = tex_sheet;

	// Set all the dimensions
	_image_width  = img_width;
	_image_height = img_height;
	_sheet_width  = tex_sheet->width  / img_width;
	_sheet_height = tex_sheet->height / img_height;

	// Allocate the blocks array
	int32 num_blocks = _sheet_width * _sheet_height;
	_blocks = new FixedImageNode[num_blocks];

	// Initialize linked list of open blocks... which, at this point is all the blocks!
	_open_list_head = &_blocks[0];
	_open_list_tail = &_blocks[num_blocks-1];

	// Now initialize all the blocks to proper values
	for (int32 i = 0; i < num_blocks - 1; ++i) {
		_blocks[i].next  = &_blocks[i+1];
		_blocks[i].image = NULL;
		_blocks[i].block_index = i;
	}

	_blocks[num_blocks-1].next  = NULL;
	_blocks[num_blocks-1].image = NULL;
	_blocks[num_blocks-1].block_index = num_blocks - 1;
}



bool FixedTexMemMgr::Insert(BaseImage *img) {
	// Whoa, nothing on the open list! (no blocks left) return false :(
	if (_open_list_head == NULL)
		return false;

	// Otherwise, get and remove the head of the open list

	FixedImageNode *node = _open_list_head;
	_open_list_head = _open_list_head->next;

	if (_open_list_head == NULL) {
		// This must mean we just removed the last open block, so
		// set the tail to NULL as well
		_open_list_tail = NULL;
	}
	else {
		// Since this is the new head, it's prev pointer should be NULL
		_open_list_head->prev = NULL;
	}

	node->next = NULL;

	// Check if there's already an image allocated at this block.
	// If so, we have to notify GameVideo that we're ejecting
	// this image out of memory to make place for the new one

	if (node->image) {
		VideoManager->_RemoveImage(node->image);
		node->image = NULL;
	}

	// Calculate the actual pixel coordinates given this node's
	// block index

	img->x = _image_width  * (node->block_index % _sheet_width);
	img->y = _image_height * (node->block_index / _sheet_width);

	// Calculate the u,v coordinates

	float sheet_width = (float) _tex_sheet->width;
	float sheet_height = (float) _tex_sheet->height;

	img->u1 = float(img->x + 0.5f)               / sheet_width;
	img->u2 = float(img->x + img->width - 0.5f)  / sheet_width;
	img->v1 = float(img->y + 0.5f)               / sheet_height;
	img->v2 = float(img->y + img->height - 0.5f) / sheet_height;

	img->texture_sheet = _tex_sheet;

	return true;
} // bool FixedTexMemMgr::Insert(BaseImage *img)



void FixedTexMemMgr::Remove(BaseImage *img) {
	// Translate x,y coordinates into a block index
	int32 block_index = _CalculateBlockIndex(img);

	// Check to make sure the block is actually owned by this image
	if (_blocks[block_index].image != img) {
		// Error, the block that the image thinks it owns is actually not
		// owned by that image

		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: tried to remove a fixed block not owned by this Image" << endl;
	}

	// Set the image to NULL to indicate that this block is completely free
	_blocks[block_index].image = NULL;

	// Remove block from the open list
	_DeleteNode(block_index);
}



void FixedTexMemMgr::Free(BaseImage *img) {
	int32 block_index = _CalculateBlockIndex(img);

	FixedImageNode *node = &_blocks[block_index];

	if (_open_list_tail != NULL) {
		// Simply append to end of list
		_open_list_tail->next = node;
		node->prev = _open_list_tail;
		node->next = NULL;
		_open_list_tail = node;
	}
	else {
		// Special case: empty list
		_open_list_head = _open_list_tail = node;
		node->next = node->prev = NULL;
	}
}



int32 FixedTexMemMgr::_CalculateBlockIndex(BaseImage *img) {
	int32 block_x = img->x / _image_width;
	int32 block_y = img->y / _image_height;

	return (block_x + _sheet_width * block_y);
}



void FixedTexMemMgr::_DeleteNode(int32 block_index) {
	if (block_index < 0)
		return;

	if (block_index >= _sheet_width * _sheet_height)
		return;

	FixedImageNode *node = &_blocks[block_index];

	if (node->prev && node->next) { // Node is somewhere in the middle of the list
		node->prev->next = node->next;
	}
	else if (node->prev) { // Node is the tail of the list
		node->prev->next = NULL;
		_open_list_tail = node->prev;
	}
	else if (node->next) { // Node is the head of the list
		
		_open_list_head = node->next;
		node->next->prev = NULL;
	}
	else {
		// Node is the only element in the list
		_open_list_head = NULL;
		_open_list_tail = NULL;
	}

	// Just for good measure, clear out this node's pointers
	node->prev = NULL;
	node->next = NULL;
}

// -----------------------------------------------------------------------------
// VariableTexMemMgr class
// -----------------------------------------------------------------------------

VariableTexMemMgr::VariableTexMemMgr(TexSheet *sheet) {
	_tex_sheet = sheet;
	_sheet_width = sheet->width / 16;
	_sheet_height = sheet->height / 16;
	_blocks = new VariableImageNode[_sheet_width*_sheet_height];
}



bool VariableTexMemMgr::Insert(BaseImage *img) {
	// Don't allow insertions into a texture bigger than 512x512...
	// This way, if we have a 1024x1024 texture holding a fullscreen background,
	// it is always safe to remove the texture sheet from memory when the
	// background is unreferenced. That way backgrounds don't stick around in memory.
	if (_sheet_width > 32 || _sheet_height > 32) { // 32 blocks = 512 pixels
		if (_blocks[0].free == false)  // Quick way to test if texsheet's occupied
			return false;
	}

	// Find an open block of memory. If none is found, return false
	// Calculate the width and height in blocks
	int32 w = (img->width + 15) / 16;
	int32 h = (img->height + 15) / 16;

	int32 block_x = -1, block_y = -1;

	// This is a 100% brute force way to allocate a block, just a bunch
	// of nested loops. In practice, this actually works fine, because
	// the allocator deals with 16x16 blocks instead of trying to worry
	// about fitting images with pixel perfect resolution.
	// Later, if this turns out to be a bottleneck, we can rewrite this
	// algorithm to something more intelligent
	for (int32 y = 0; y < _sheet_height - h + 1; y++) {
		for (int32 x = 0; x < _sheet_width - w + 1; x++) {
			int32 furthest_blocker = -1;

			for (int32 dy = 0; dy < h; dy++) {
				for (int32 dx = 0; dx < w; dx++) {
					if (_blocks[(x + dx) + ((y + dy) * _sheet_width)].free == false) {
						furthest_blocker = x+dx;
						goto endneighborsearch_GOTO;
					}
				}
			}

			endneighborsearch_GOTO:

			if (furthest_blocker == -1) {
				block_x = x;
				block_y = y;
				goto endsearch_GOTO;
			}
		}
	}

	endsearch_GOTO:

	if (block_x == -1 || block_y == -1)
		return false;

	// Check if there's already an image allocated at this block.
	// If so, we have to notify GameVideo that we're ejecting
	// this image out of memory to make place for the new one

	// Update blocks
	set<BaseImage*> remove_images;

	for(int32 y = block_y; y < block_y + h; y++) {
		for(int32 x = block_x; x < block_x + w; x++) {
			int32 index = x + (y * _sheet_width);
			// Check if there's already an image at the point we're
			// trying to load at. If so, we need to tell GameVideo
			// to update its internal vector

			if(_blocks[index].image) {
				remove_images.insert(_blocks[index].image);
			}

			_blocks[index].free  = false;
			_blocks[index].image = img;
		}
	}

	for(set<BaseImage*>::iterator i = remove_images.begin(); i != remove_images.end(); i++) {
		Remove(*i);
		VideoManager->_RemoveImage(*i);
	}


	// Calculate the actual pixel coordinates given this node's block index
	img->x = block_x * 16;
	img->y = block_y * 16;

	// Calculate the u,v coordinates

	float sheet_width = static_cast<float>(_tex_sheet->width);
	float sheet_height = static_cast<float>(_tex_sheet->height);

	img->u1 = float(img->x + 0.5f) / sheet_width;
	img->u2 = float(img->x + img->width - 0.5f) / sheet_width;
	img->v1 = float(img->y + 0.5f) / sheet_height;
	img->v2 = float(img->y + img->height - 0.5f) / sheet_height;

	img->texture_sheet = _tex_sheet;
	return true;
} // bool VariableTexMemMgr::Insert(BaseImage *img)



void VariableTexMemMgr::_SetBlockProperties(BaseImage *img, bool change_free, bool change_image, bool free, BaseImage *new_image) {
	// Calculate upper-left corner in blocks
	int32 block_x = img->x / 16;
	int32 block_y = img->y / 16;

	// Calculate width and height in blocks
	int32 w = (img->width  + 15) / 16;
	int32 h = (img->height + 15) / 16;

	for (int32 y = block_y; y < block_y + h; y++) {
		for (int32 x = block_x; x < block_x + w; x++) {
			if (_blocks[x + y * _sheet_width].image == img) {
				if (change_free)
					_blocks[x + y * _sheet_width].free = free;
				if (change_image)
					_blocks[x + y * _sheet_width].image = new_image;
			}
		}
	}
}

} // namespace private_video

using namespace hoa_video::private_video;

//-----------------------------------------------------------------------------
// DEBUG_NextTexSheet: increments to the next texture sheet to show with
//                     _DEBUG_ShowTexSheet().
//-----------------------------------------------------------------------------

void GameVideo::DEBUG_NextTexSheet()
{
	++_current_debug_TexSheet;

	if(_current_debug_TexSheet >= (int32) _tex_sheets.size())
	{
		_current_debug_TexSheet = -1;   // disable display
	}
}




//-----------------------------------------------------------------------------
// DEBUG_PrevTexSheet: cycles to the previous texturesheet to show with
//                     _DEBUG_ShowTexSheet().
//-----------------------------------------------------------------------------

void GameVideo::DEBUG_PrevTexSheet()
{
	--_current_debug_TexSheet;

	if(_current_debug_TexSheet < -1)
	{
		_current_debug_TexSheet = (int32) _tex_sheets.size() - 1;
	}
}




//-----------------------------------------------------------------------------
// ReloadTextures: reloads the texture sheets, after they have been unloaded
//                 most likely due to a change of video mode.
//                 Returns false if any of the textures fail to reload
//-----------------------------------------------------------------------------

bool GameVideo::ReloadTextures()
{
	// Reload texture sheets

	vector<TexSheet *>::iterator iSheet    = _tex_sheets.begin();
	vector<TexSheet *>::iterator iSheetEnd = _tex_sheets.end();

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

	if(_uses_lights)
		_light_overlay = _CreateBlankGLTexture(1024, 1024);

	return success;
}




//-----------------------------------------------------------------------------
// UnloadTextures: frees the texture memory taken up by the texture sheets,
//                 but leaves the lists of images intact so we can reload them
//                 Returns false if any of the textures fail to unload.
//-----------------------------------------------------------------------------

bool GameVideo::UnloadTextures()
{
	// Save temporary textures to disk, in other words textures which weren't
	// loaded to a file. This way when we recreate the GL context we will
	// be able to load them again.
	_SaveTempTextures();

	// Unload texture sheets
	vector<TexSheet *>::iterator iSheet    = _tex_sheets.begin();
	vector<TexSheet *>::iterator iSheetEnd = _tex_sheets.end();

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

	if(_light_overlay != 0xFFFFFFFF)
	{
		_DeleteTexture(_light_overlay);
		_light_overlay = 0xFFFFFFFF;
	}

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
// _DeleteTexture: wraps call to glDeleteTexture(), adds some checking to see
//                if we deleted the last texture we bound using _BindTexture(),
//                then set the last tex ID to 0xffffffff
//-----------------------------------------------------------------------------

bool GameVideo::_DeleteTexture(GLuint tex_id)
{
	glDeleteTextures(1, &tex_id);

	if(_last_tex_id == tex_id)
		_last_tex_id = 0xFFFFFFFF;

	if(glGetError())
		return false;

	return true;
}








bool GameVideo::_DEBUG_ShowTexSheet()
{
	// Value of zero means to disable display
	if(_current_debug_TexSheet == -1)
	{
		return true;
	}

	// Check if there aren't any texture sheets! (should never happen)
	if(_tex_sheets.empty())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO_WARNING: Called DEBUG_ShowTexture(), but there were no texture sheets" << endl;
		return false;
	}

	int32 num_sheets = (int32) _tex_sheets.size();

	// We may go out of bounds say, if we were viewing a texture sheet and then it got
	// deleted or something. To recover, just set it to the last texture sheet
	if(_current_debug_TexSheet >= num_sheets)
	{
		_current_debug_TexSheet = num_sheets - 1;
	}

	TexSheet *sheet = _tex_sheets[_current_debug_TexSheet];

	if(!sheet)
	{
		return false;
	}

	int32 w = sheet->width;
	int32 h = sheet->height;

	Image img(sheet, string(), "<T>", 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, w, h, false);


	PushState();
	SetDrawFlags(VIDEO_NO_BLEND, VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
	SetCoordSys(0.0f, 1024.0f, 0.0f, 760.0f);

	glPushMatrix();

	Move(0.0f,0.0f);
	glScalef(0.5f, 0.5f, 0.5f);

	ImageElement elem(&img, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, (float)w, (float)h);

	StillImage id;
	id._elements.push_back(elem);


	DrawImage(id);

	glPopMatrix();

	SetFont("debug_font");
	char buf[200];

	Move(20, _current_context.coordinate_system.GetTop() - 30);
	DrawText("Current Texture sheet:");

	sprintf(buf, "  Sheet #: %d", _current_debug_TexSheet);
	MoveRelative(0, -20);
	DrawText(buf);

	MoveRelative(0, -20);
	sprintf(buf, "  Size:    %dx%d", sheet->width, sheet->height);
	DrawText(buf);

	if(sheet->type == VIDEO_TEXSHEET_32x32)
		sprintf(buf, "  Type:    32x32");
	else if(sheet->type == VIDEO_TEXSHEET_32x64)
		sprintf(buf, "  Type:    32x64");
	else if(sheet->type == VIDEO_TEXSHEET_64x64)
		sprintf(buf, "  Type:    64x64");
	else if(sheet->type == VIDEO_TEXSHEET_ANY)
		sprintf(buf, "  Type:    Any size");

	MoveRelative(0, -20);
	DrawText(buf);

	sprintf(buf, "  Static:  %d", sheet->is_static);
	MoveRelative(0, -20);
	DrawText(buf);

	sprintf(buf, "  TexID:   %d", sheet->tex_id);
	MoveRelative(0, -20);
	DrawText(buf);

	PopState();
	return true;
}



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

			map<string,Image*>::iterator it = _images.find(filename);
			if (it == _images.end())
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
// 		if (base_img->width > 512 || base_img->height > 512) {
			// Remove the image and texture sheet completely
			// TODO: This introduces a seg fault when TexSheet::FreeImage is later called. Fix this bug!
// 			_RemoveSheet(base_img->texture_sheet);
// 			_RemoveImage(base_img);
// 		}
// 		else {
			// For smaller images, simply mark them as free 
			// in the memory manager
			base_img->texture_sheet->FreeImage(base_img);
// 		}
	}

	return true;
}




//-----------------------------------------------------------------------------
// _RemoveSheet: removes a texture sheet from the internal std::vector
//-----------------------------------------------------------------------------

bool GameVideo::_RemoveSheet(TexSheet *sheet)
{
	if(_tex_sheets.empty())
	{
		return false;
	}

	vector<TexSheet*>::iterator iSheet = _tex_sheets.begin();
	vector<TexSheet*>::iterator iEnd   = _tex_sheets.end();

	// Search std::vector for pointer matching sheet and remove it
	while(iSheet != iEnd)
	{
		if(*iSheet == sheet)
		{
			delete sheet;
			_tex_sheets.erase(iSheet);
			return true;
		}
		++iSheet;
	}

	// Couldn't find the image
	return false;
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
		if (img) {
			if (img->ref_count <= 0) {
				if (VIDEO_DEBUG)
					cerr << "VIDEO WARNING: Called DeleteImage() when refcount was already <= 0!" << endl;
				return;
			}

			(img->ref_count)--;

			if (img->ref_count == 0) {
				// 1. If it's on a large tex sheet (> 512x512), delete it
				// Note: We can assume that this is the only image on that texture
				//       sheet, so it's safe to delete it. (Big textures always
				//       are allocated to their own sheet, by design.)

				if (img->width > 512 || img->height > 512) {
					_DeleteImage(img);  // overloaded DeleteImage for deleting Image
				}

				// 2. otherwise, mark it as "freed"
				img->texture_sheet->FreeImage(img);
			}
		}
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
	TImage *timage;

	img = dynamic_cast<Image*>(base_image);
	if (img)
		return _RemoveImage(img);
	
	timage = dynamic_cast<TImage*>(base_image);
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

	if(_images.empty())
	{
		return false;
	}

	map<string, Image*>::iterator iImage = _images.begin();
	map<string, Image*>::iterator iEnd   = _images.end();

	// Search std::map for pointer matching img and remove it
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

	// Couldn't find the image
	return false;
}


//-----------------------------------------------------------------------------
// _RemoveImage: removes the timage pointer from the std::set
//-----------------------------------------------------------------------------

bool GameVideo::_RemoveImage(TImage *img)
{
	// Nothing to do if img is null
	if(!img)
		return true;

	if(_t_images.empty())
	{
		return false;
	}

	if(_GetTImage(img))
	{
		_t_images.erase(img);
		delete img;
		return true;
	}

	// Couldn't find the image
	return false;
}


//-----------------------------------------------------------------------------
// _CreateBlankGLTexture: creates a blank texture of the given width and height
//                       and returns its OpenGL texture ID.
//                       Returns 0xFFFFFFFF on failure
//-----------------------------------------------------------------------------

GLuint GameVideo::_CreateBlankGLTexture(int32 width, int32 height) {
	// Attempt to create a GL texture with the given width and height
	int32 error;

	GLuint tex_id;

	glGenTextures(1, &tex_id);
	error = glGetError();

	if(!error)   // If there's no error so far, attempt to bind texture
	{
		_BindTexture(tex_id);
		error = glGetError();

		// If the binding was successful, initialize the texture with glTexImage2D()
		if(!error)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			error = glGetError();
		}
	}

	if(error != 0)   // If there's an error, delete the texture and return error code
	{
		_DeleteTexture(tex_id);

		// This error is fatal, always show
		cerr << "VIDEO ERROR: failed to create new " << width << " by " << height << " texture in _CreateBlankGLTexture()." << endl;
		cerr << "  OpenGL reported the following error:" << endl << "  ";
		char *errString = (char*)gluErrorString(error);
		cerr << errString << endl;

		return 0xFFFFFFFF;
	}

	// Set linear texture interpolation if we're at non-natural resolution
	GLenum filtering_type = VideoManager->_ShouldSmooth() ? GL_LINEAR : GL_NEAREST;

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering_type );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering_type );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	return tex_id;
}








//-----------------------------------------------------------------------------
// ConvertImageToGrayscale: Converts an image from color to gray mode
//-----------------------------------------------------------------------------

void GameVideo::_ConvertImageToGrayscale(const ImageLoadInfo& src, ImageLoadInfo &dst) const
{
	if (!dst.width || !dst.height)	// Return if there are no pixels in the image
		return;

	// Convert the pixels to grayscale while copying
	uint8* src_pix = static_cast<uint8*>(src.pixels);
	uint8* src_end = static_cast<uint8*>(src.pixels) + (src.width * src.height * 4);
	uint8* dst_pix = static_cast<uint8*>(dst.pixels);
	uint8 value;

	for (; src_pix<src_end; src_pix+=4,dst_pix+=4)
	{
		value = static_cast<uint8>((30 * *(src_pix) + 59 * *(src_pix+1) + 11 * *(src_pix+2))*0.01f);	// Get grayscale value
		*dst_pix = *(dst_pix+1) = *(dst_pix+2) = value;		// Assign it
		*(dst_pix+3) = *(src_pix+3);					// Assign alpha value
	}
}




//-----------------------------------------------------------------------------
// RGBAToRGB: Converts a buffer from RGBA to RGB
//-----------------------------------------------------------------------------

void GameVideo::_RGBAToRGB (const private_video::ImageLoadInfo& src, private_video::ImageLoadInfo &dst) const
{
	if (!dst.width || !dst.height)	// Return if there are no pixels in the image
		return;

	uint8* pSrc = static_cast<uint8*>(src.pixels);
	uint8* pDst = static_cast<uint8*>(dst.pixels);

	int32 iSrc;
	int32 iDst;
	for (int32 i=0; i<src.height*src.width; i++)
	{
		iSrc = 4 * i;
		iDst = 3 * i;
		pDst[iDst] = pSrc[iSrc];
		pDst[iDst+1] = pSrc[iSrc+1];
		pDst[iDst+2] = pSrc[iSrc+2];
	}
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
	if(_images.find(id._filename) != _images.end())
	{
		Image *img = _images[id._filename];		// Get the image from the map

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
			if(_images.find(file_name+tags) == _images.end())
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
			if(_images.find(file_name+tags) != _images.end())
			{
				images.at(current_image)._elements.clear();

				Image *img = _images[file_name+tags];

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
				TexSheet *sheet = _InsertImageInTexSheet(new_image, info.image, images.at(current_image)._is_static);

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
				_images[file_name + tags] = new_image;

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
	TexSheet *sheet = _InsertImageInTexSheet(new_image, load_info, is_static);

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
	_images[id._filename] = new_image;

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
	VideoManager->_BindTexture(ID);
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
				VideoManager->_BindTexture(img->texture_sheet->tex_id);
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
		_RGBAToRGB (info, info);
		_SaveJpeg (file_name, info);
	}
	else
	{
		_SavePng (file_name, info);
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

	_GetBufferFromImage (buffer, img);

	if (type == JPEG)
	{
		_RGBAToRGB (buffer, buffer);
		_SaveJpeg (file_name, buffer);
	}
	else
	{
		_SavePng (file_name, buffer);
	}

	return true;
}




//----------------------------------------------------------------------------
// Pass a texture to a given buffer
//----------------------------------------------------------------------------

void GameVideo::_GetBufferFromTexture (hoa_video::private_video::ImageLoadInfo& buffer,
									   hoa_video::private_video::TexSheet* texture) const
{
	if (buffer.pixels)
		free (buffer.pixels);
	buffer.pixels = NULL;

	// Get the texture as a buffer
	buffer.height = texture->height;
	buffer.width = texture->width;
	buffer.pixels = malloc (buffer.height * buffer.width * 4);
	VideoManager->_BindTexture(texture->tex_id);
	glGetTexImage (GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.pixels);
}




//----------------------------------------------------------------------------
// Pass an Image to a given buffer
//----------------------------------------------------------------------------

void GameVideo::_GetBufferFromImage (hoa_video::private_video::ImageLoadInfo& buffer,
									 hoa_video::private_video::BaseImage* img) const
{
	// Get the texture as a buffer
	_GetBufferFromTexture (buffer, img->texture_sheet);

	// In case the image is smaller than the texture (it is just contained there), then copy the image rectangle
	if (buffer.height > img->height || buffer.width > img->width)
	{
		hoa_video::private_video::ImageLoadInfo info;
		info.width = img->width;
		info.height = img->height;
		info.pixels = malloc (img->width * img->height * 4);
		uint32 dst_bytes = info.width * 4;
		uint32 src_bytes = buffer.width * 4;
		uint32 src_offset = img->y * buffer.width * 4 + img->x * 4;
		for (int32 i=0; i<info.height; i++)
		{
			memcpy ((uint8*)info.pixels+i*dst_bytes, (uint8*)buffer.pixels+i*src_bytes+src_offset, dst_bytes);
		}

		if (buffer.pixels)
			free (buffer.pixels);

		buffer.pixels = info.pixels;
		buffer.height = info.height;
		buffer.width = info.width;
	}
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



//-----------------------------------------------------------------------------
// _InsertImageInTexSheet: takes an image that was loaded, finds
//                        an available texture sheet, copies it to the sheet,
//                        and returns a pointer to the texture sheet. If no
//                        available texture sheet is found, a new one is created.
//
//                        Returns NULL on failure, which should only happen if
//                        we run out of memory or bad argument is passed.
//-----------------------------------------------------------------------------

TexSheet *GameVideo::_InsertImageInTexSheet(BaseImage *image, private_video::ImageLoadInfo & load_info, bool is_static)
{
	// If it's a large image size (>512x512) then we already know it's not going
	// to fit in any of our existing texture sheets, so create a new one for it

	if(load_info.width > 512 || load_info.height > 512)
	{
		int32 round_width = RoundUpPow2(load_info.width);
		int32 round_height = RoundUpPow2(load_info.height);
		TexSheet *sheet = _CreateTexSheet(round_width, round_height, VIDEO_TEXSHEET_ANY, false);

		// Ran out of memory!
		if(!sheet)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: _CreateTexSheet() returned NULL in _InsertImageInTexSheet()!" << endl;
			return NULL;
		}

		if(sheet->AddImage(image, load_info))
			return sheet;
		else
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: AddImage() returned false for inserting large image!" << endl;
			return NULL;
		}
	}


	// Determine the type of texture sheet that should hold this image

	TexSheetType type;

	if(load_info.width == 32 && load_info.height == 32)
		type = VIDEO_TEXSHEET_32x32;
	else if(load_info.width == 32 && load_info.height == 64)
		type = VIDEO_TEXSHEET_32x64;
	else if(load_info.width == 64 && load_info.height == 64)
		type = VIDEO_TEXSHEET_64x64;
	else
		type = VIDEO_TEXSHEET_ANY;

	// Loop through existing texture sheets and see if the image will fit in
	// any of the ones which match the type we're looking for

	size_t num_tex_sheets = _tex_sheets.size();

	for(uint32 iSheet = 0; iSheet < num_tex_sheets; ++iSheet)
	{
		TexSheet *sheet = _tex_sheets[iSheet];
		if(!sheet)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: _texSheets[iSheet] was NULL in _InsertImageInTexSheet()!" << endl;
			return NULL;
		}

		if(sheet->type == type && sheet->is_static == is_static)
		{
			if(sheet->AddImage(image, load_info))
			{
				// Added to a sheet successfully
				return sheet;
			}
		}
	}

	// If it doesn't fit in any of them, create a new 512x512 and stuff it in

	TexSheet *sheet = _CreateTexSheet(512, 512, type, is_static);
	if(!sheet)
	{
		// Failed to create texture, ran out of memory probably

		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: Failed to create new texture sheet in _InsertImageInTexSheet!" << endl;
		}

		return NULL;
	}

	// Now that we have a fresh texture sheet, AddImage() should work without
	// any problem
	if(sheet->AddImage(image, load_info))
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
	bool is_static
)
{
	// Validate the parameters

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

	GLuint tex_id = _CreateBlankGLTexture(width, height);

	// Now that we have our texture loaded, simply create a new TexSheet

 	TexSheet *sheet = new TexSheet(width, height, tex_id, type, is_static);
	_tex_sheets.push_back(sheet);

	return sheet;
}


//-----------------------------------------------------------------------------
// _ReloadImagesToSheet: helper function of the GameVideo class to
//                      TexSheet::Reload() to do the dirty work of reloading
//                      image data into the appropriate spots on the texture
//-----------------------------------------------------------------------------

bool GameVideo::_ReloadImagesToSheet(TexSheet *sheet)
{
	// Delete images
	map<string, Image *>::iterator iImage     = _images.begin();
	map<string, Image *>::iterator iImageEnd  = _images.end();

	std::map <string, private_video::MultiImageInfo> multi_image_info;

	bool success = true;
	while(iImage != iImageEnd)
	{
		Image *i = iImage->second;

		// Check if the current image belongs to this sheet
		if(i->texture_sheet == sheet)
		{
			ImageLoadInfo load_info;

			bool is_multi_image = ( i->tags.find("<X",0) != i->filename.npos);

			if (is_multi_image)	// Check if this is a multiimage and load as it
			{
				ImageLoadInfo image;

				if (multi_image_info.find(i->filename) == multi_image_info.end())
				{
					// Load the image
					if(!_LoadRawImage(i->filename, load_info))
					{
						if(VIDEO_DEBUG)
							cerr << "VIDEO ERROR: _LoadRawImage() failed in _ReloadImagesToSheet()!" << endl;
						success = false;
					}

					// Copy the part of the image in a buffer
					image.height = i->height;
					image.width = i->width;
					image.pixels = malloc(image.height * image.width * 4);

					MultiImageInfo info;
					info.multi_image = load_info;
					info.image = image;
					multi_image_info[i->filename] = info;
				}
				else
				{
					load_info = multi_image_info[i->filename].multi_image;
					image = multi_image_info[i->filename].image;
				}

				if (!image.pixels)
				{
					if (VIDEO_DEBUG)
						cerr << "VIDEO ERROR: run out of memory in _ReloadImageToSheet()" << endl;
					success = false;
				}

				uint16 pos0, pos1;
				pos0 = i->tags.find("<X", 0);
				pos1 = i->tags.find('_', pos0);
				uint32 x = atoi( i->tags.substr(pos0+2, pos1).c_str() );
				uint32 rows = load_info.height / image.height;
				pos0 = i->tags.find("<Y", 0);
				pos1 = i->tags.find('_', pos0);
				uint32 y = atoi( i->tags.substr(pos0+2, pos1).c_str() );
				uint32 cols = load_info.width / image.width;

				for (int32 row=0; row<image.height; row++)
				{
					memcpy ((uint8*)image.pixels+4*image.width*row, (uint8*)load_info.pixels+(((x*load_info.height/rows)+row)*load_info.width+y*load_info.width/cols)*4, 4*image.width);
				}

				// Convert to grayscale if needed
				if (i->tags.find("<G>",0) != i->filename.npos)
					_ConvertImageToGrayscale(image, image);

				// Copy in the texture the image
				if(!sheet->CopyRect(i->x, i->y, image))
				{
					if(VIDEO_DEBUG)
						cerr << "VIDEO ERROR: sheet->CopyRect() failed in _ReloadImagesToSheet()!" << endl;
					success = false;
				}
			}
			else		// Load this way if it as a normal image (one image in one file)
			{
				std::string fname = i->filename;
				if (i->tags.find("<T>",0) != i->tags.npos)
				{
					fname = "img\\temp\\" + fname + ".png";
				}

				if(!_LoadRawImage(fname, load_info))
				{
					if(VIDEO_DEBUG)
						cerr << "VIDEO ERROR: _LoadRawImage() failed in _ReloadImagesToSheet()!" << endl;
					success = false;
				}

				// Convert to grayscale if needed
				if (i->tags.find("<G>",0) != i->filename.npos)
					_ConvertImageToGrayscale(load_info, load_info);

				if(!sheet->CopyRect(i->x, i->y, load_info))
				{
					if(VIDEO_DEBUG)
						cerr << "VIDEO ERROR: sheet->CopyRect() failed in _ReloadImagesToSheet()!" << endl;
					success = false;
				}

				if (load_info.pixels)
					free (load_info.pixels);
			}
		}

		++iImage;
	}

	for (map<string,MultiImageInfo>::iterator it=multi_image_info.begin(); it!=multi_image_info.end(); ++it)
	{
		free ((*it).second.multi_image.pixels);
		free ((*it).second.image.pixels);
	}

	// Regenerate all font textures
	set<TImage*>::iterator t_begin = _t_images.begin();
	set<TImage*>::iterator t_end   = _t_images.end();
	for (;t_begin != t_end; ++t_begin)
	{
		if ((*t_begin)->texture_sheet == sheet)
			(*t_begin)->Reload();
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

		// It's a temporary texture!!
		if(image->tags.find("<T>") != string::npos)
		{
			hoa_video::private_video::ImageLoadInfo buffer;

			_GetBufferFromImage (buffer, image);
			_SavePng ("img\\temp\\"+image->filename+".png", buffer);
		}

		++iImage;
	}
	return true;
}


//-----------------------------------------------------------------------------
// _AddTImage: add a TImage to the internal set
//-----------------------------------------------------------------------------

bool GameVideo::_AddTImage(TImage *timg)
{
	if (!_GetTImage(timg))
	{
		_t_images.insert(timg);
		return true;
	}
	return false;
}

}  // namespace hoa_video

