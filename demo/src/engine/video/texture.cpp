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

// #include <cassert>
// #include <cstdarg>
// #include <set>
// #include <fstream>
// #include <math.h>

#include "utils.h"
#include "texture.h"
#include "video.h"

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
	TextureManager->_DeleteTexture(tex_id);
}



bool TexSheet::Unload() {
	if (loaded == false) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: unloading an already unloaded texture sheet" << endl;
		return false;
	}

	TextureManager->_DeleteTexture(tex_id);
	loaded = false;
	return true;
}



bool TexSheet::Reload() {
	if (loaded == true) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: loading an already loaded texture sheet" << endl;
		return false;
	}

	// Create new OpenGL texture
	GLuint id = TextureManager->_CreateBlankGLTexture(width, height);

	if (id == INVALID_TEXTURE_ID) {
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
	if (TextureManager->_ReloadImagesToSheet(this) == false) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: CopyImagesToSheet() failed in TexSheet::Reload()!" << endl;
		return false;
	}

	loaded = true;
	return true;
}



bool TexSheet::AddImage(BaseImageTexture* img, ImageMemory& load_info) {
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



bool TexSheet::CopyRect(int32 x, int32 y, ImageMemory& load_info) {
	TextureManager->_BindTexture(tex_id);

	glTexSubImage2D
	(
		GL_TEXTURE_2D, // target
		0, // level
		x, // x offset within tex sheet
		y, // y offset within tex sheet
		load_info.width, // width in pixels of image
		load_info.height, // height in pixels of image
		(load_info.rgb_format ? GL_RGB : GL_RGBA), // format
		GL_UNSIGNED_BYTE, // type
		load_info.pixels // pixels of the sub image
	);

	if (VideoManager->CheckGLError() == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "an OpenGL error occured: " << VideoManager->CreateGLErrorString() << endl;
		return false;
	}

	return true;
}



bool TexSheet::CopyScreenRect(int32 x, int32 y, const ScreenRect& screen_rect) {
	TextureManager->_BindTexture(tex_id);

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

	if (VideoManager->CheckGLError() == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "an OpenGL error occured: " << VideoManager->CreateGLErrorString() << endl;
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

		TextureManager->_BindTexture(tex_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering_type);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering_type);
	}
}



void TexSheet::Draw() const {
	// The vertex coordinate array to use (assumes glScale() has been appropriately set)
	static const float vertex_coords[] = {
		0.0f, 0.0f, // Upper left
		1.0f, 0.0f, // Upper right
		1.0f, 1.0f, // Lower right
		0.0f, 1.0f, // Lower left
	};

	// The texture coordinate array to use (specifies the coordinates encompassing the entire texture)
	static const float texture_coords[] = {
		0.0f, 1.0f, // Upper right
		1.0f, 1.0f, // Lower right
		1.0f, 0.0f, // Lower left
		0.0f, 0.0f, // Upper left
	};

	// Enable texturing and bind the texture
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	TextureManager->_BindTexture(tex_id);

	// Enable and setup the texture coordinate array
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texture_coords);

	// Use a vertex array to draw all of the vertices
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, vertex_coords);
	glDrawArrays(GL_QUADS, 0, 4);

	if (VideoManager->CheckGLError() == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "an OpenGL error occurred: " << VideoManager->CreateGLErrorString() << endl;
	}
} // void TexSheet::Draw() const

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



bool FixedTexMemMgr::Insert(BaseImageTexture *img) {
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
		Remove(node->image);
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
} // bool FixedTexMemMgr::Insert(BaseImageTexture *img)



void FixedTexMemMgr::Remove(BaseImageTexture *img) {
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



void FixedTexMemMgr::Free(BaseImageTexture *img) {
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



int32 FixedTexMemMgr::_CalculateBlockIndex(BaseImageTexture *img) {
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



bool VariableTexMemMgr::Insert(BaseImageTexture *img) {
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
	set<BaseImageTexture*> remove_images;

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

	for(set<BaseImageTexture*>::iterator i = remove_images.begin(); i != remove_images.end(); i++) {
		Remove(*i);
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
} // bool VariableTexMemMgr::Insert(BaseImageTexture *img)



void VariableTexMemMgr::_SetBlockProperties(BaseImageTexture *img, bool change_free, bool change_image, bool free, BaseImageTexture *new_image) {
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

} // namespace hoa_video
