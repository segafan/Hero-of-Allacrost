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

TexSheet::TexSheet(int32 sheet_width, int32 sheet_height, GLuint sheet_id, TexSheetType sheet_type, bool sheet_static) :
	width(sheet_width),
	height(sheet_height),
	tex_id(sheet_id),
	type(sheet_type),
	is_static(sheet_static),
	smoothed(false),
	loaded(true)
{
	Smooth();
}



TexSheet::~TexSheet() {
	// Unload OpenGL texture from memory
	TextureManager->_DeleteTexture(tex_id);
}



bool TexSheet::Unload() {
	if (loaded == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "attempted to unload an already unloaded texture sheet" << endl;
		return false;
	}

	TextureManager->_DeleteTexture(tex_id);
	tex_id = INVALID_TEXTURE_ID;
	loaded = false;
	return true;
}



bool TexSheet::Reload() {
	if (loaded == true) {
		if (VIDEO_DEBUG)
			IF_PRINT_WARNING(VIDEO_DEBUG) << "attempted to load an already loaded texture sheet" << endl;
		return false;
	}

	// Create new OpenGL texture
	GLuint id = TextureManager->_CreateBlankGLTexture(width, height);

	if (id == INVALID_TEXTURE_ID) {
		PRINT_ERROR << "call to TextureController::_CreateBlankGLTexture() failed" << endl;
		return false;
	}

	tex_id = id;

	// Restore texture smoothing if applied.
	bool was_smoothed = smoothed;
	smoothed = false;
	Smooth(was_smoothed);

	// Reload all of the images that belong to this texture
	if (TextureManager->_ReloadImagesToSheet(this) == false) {
		PRINT_ERROR << "call to TextureController::_ReloadImagesToSheet() failed" << endl;
		return false;
	}

	loaded = true;
	return true;
}



bool TexSheet::CopyRect(int32 x, int32 y, ImageMemory& data) {
	TextureManager->_BindTexture(tex_id);

	glTexSubImage2D(
		GL_TEXTURE_2D, // target
		0, // level
		x, // x offset within tex sheet
		y, // y offset within tex sheet
		data.width, // width in pixels of image
		data.height, // height in pixels of image
		(data.rgb_format ? GL_RGB : GL_RGBA), // format
		GL_UNSIGNED_BYTE, // type
		data.pixels // pixels of the sub image
	);

	if (VideoManager->CheckGLError() == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "an OpenGL error occured: " << VideoManager->CreateGLErrorString() << endl;
		return false;
	}

	return true;
}



bool TexSheet::CopyScreenRect(int32 x, int32 y, const ScreenRect& screen_rect) {
	TextureManager->_BindTexture(tex_id);

	glCopyTexSubImage2D(
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



void TexSheet::DEBUG_Draw() const {
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
} // void TexSheet::DEBUG_Draw() const

// -----------------------------------------------------------------------------
// FixedTexSheet class
// -----------------------------------------------------------------------------

FixedTexSheet::FixedTexSheet(int32 sheet_width, int32 sheet_height, GLuint sheet_id, TexSheetType sheet_type, bool sheet_static, int32 img_width, int32 img_height) :
	TexSheet(sheet_width, sheet_height, sheet_id, sheet_type, sheet_static),
	_texture_width(img_width),
	_texture_height(img_height)
{
	// Set all the dimensions
	_block_width  = width / _texture_width;
	_block_height = height / _texture_height;

	// Allocate the blocks array
	int32 num_blocks = _block_width * _block_height;
	_blocks = new FixedTexNode[num_blocks];

	// Construct the linked list of open blocks
	_open_list_head = &_blocks[0];
	_open_list_tail = &_blocks[num_blocks - 1];

	for (int32 i = 0; i < num_blocks - 1; i++) {
		_blocks[i].image = NULL;
		_blocks[i].next = &_blocks[i + 1];
		_blocks[i].block_index = i;
	}

	_open_list_tail->image = NULL;
	_open_list_tail->next = NULL;
	_open_list_tail->block_index = num_blocks - 1;
}



FixedTexSheet::~FixedTexSheet() {
	if (GetNumberTextures() != 0)
		IF_PRINT_WARNING(VIDEO_DEBUG) << "texture sheet being deleted when it has a non-zero allocated texture count: " << GetNumberTextures() << endl;

	delete[] _blocks;
}



bool FixedTexSheet::AddTexture(BaseTexture* img, ImageMemory& data) {
	if (InsertTexture(img) == false)
		return false;

	// Copy the pixel data for the texture over
	if (CopyRect(img->x, img->y, data) == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "VIDEO ERROR: CopyRect() failed in TexSheet::AddImage()!" << endl;
		return false;
	}

	return true;
} // bool FixedTexSheet::AddTexture(BaseTexture *img)



bool FixedTexSheet::InsertTexture(BaseTexture* img) {
	if (img == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "NULL pointer was given as function argument" << endl;
		return false;
	}

	// Retrieve the node from the head of the list to use for this texture
	FixedTexNode* node = _RemoveOpenNode();
	if (node == NULL) // This condition indicates that there are no remaining free nodes on the open list
		return false; 

	// Check if there's already an image allocated at this block (an image was freed earlier, but not removed)
	// If so, we must now remove it from memory
	if (node->image != NULL) {
		// TODO: TextureManager needs to have the image element removed from its map containers
		node->image = NULL;
	}

	// Calculate the texture's pixel coordinates in the sheet given this node's block index
	img->x = _texture_width * (node->block_index % _block_width);
	img->y = _texture_height * (node->block_index / _block_width);

	// Calculate the u,v coordinates
	float sheet_width = static_cast<float>(width);
	float sheet_height = static_cast<float>(height);

	img->u1 = static_cast<float>(img->x + 0.5f) / sheet_width;
	img->u2 = static_cast<float>(img->x + img->width - 0.5f) / sheet_width;
	img->v1 = static_cast<float>(img->y + 0.5f) / sheet_height;
	img->v2 = static_cast<float>(img->y + img->height - 0.5f) / sheet_height;

	img->texture_sheet = this;
	node->image = img;
	return true;
} // bool FixedTexSheet::InsertTexture(BaseTexture* img)



void FixedTexSheet::RemoveTexture(BaseTexture *img) {
	if (img == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "NULL pointer was given as function argument" << endl;
		return;
	}

	int32 block_index = _CalculateBlockIndex(img);

	// Check to make sure the block is actually owned by this image
	if (_blocks[block_index].image != img) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "tried to remove a fixed block not owned by the image" << endl;
		return;
	}

	_blocks[block_index].image = NULL;
	_AddOpenNode(&_blocks[block_index]);
}



void FixedTexSheet::FreeTexture(BaseTexture *img) {
	if (img == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "NULL pointer was given as function argument" << endl;
		return;
	}

	int32 block_index = _CalculateBlockIndex(img);

	// Check to make sure the block is actually owned by this image
	if (_blocks[block_index].image != img) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "tried to remove a fixed block not owned by the image" << endl;
		return;
	}

	// Unliked the RemoveTexture call, we do not set the block's image to NULL here
	_AddOpenNode(&_blocks[block_index]);
}



void FixedTexSheet::RestoreTexture(BaseTexture *img) {
	if (img == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "NULL pointer was given as function argument" << endl;
		return;
	}

	// Go through the list of open nodes and find the node with this image
	FixedTexNode* last = NULL;
	FixedTexNode* now = _open_list_head;

	while (now != NULL) {
		// If we found the texture, update the list so that the containing node temporarily becomes 
		// the head of the open list, then remove that node from the list head
		if (now->image == img) {
			if (last != NULL) {
				last = now->next;
				now->next = _open_list_head;
				_open_list_head = now;
			}

			_RemoveOpenNode();
			return;
		}

		last = now;
		now = now->next;
	}

	IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to restore, texture was not found in open list" << endl;
}



uint32 FixedTexSheet::GetNumberTextures() {
	uint32 num_blocks = 0;

	for (int32 i = 0; i < _block_width * _block_height; i++) {
		if (_blocks[i].image != NULL) {
			num_blocks++;
		}
	}

	return num_blocks;
}



int32 FixedTexSheet::_CalculateBlockIndex(BaseTexture* img) {
	int32 block_x = img->x / _texture_width;
	int32 block_y = img->y / _texture_height;

	return (block_x + _block_width * block_y);
}



void FixedTexSheet::_AddOpenNode(FixedTexNode* node) {
	if (_open_list_tail != NULL) {
		_open_list_tail->next = node;
		_open_list_tail = node;
		_open_list_tail->next = NULL;
	}
	else {
		_open_list_head = node;
		_open_list_tail = node;
		_open_list_tail->next = NULL;
	}
}



FixedTexNode* FixedTexSheet::_RemoveOpenNode() {
	if (_open_list_head == NULL)
		return NULL;

	FixedTexNode* node = _open_list_head;
	_open_list_head = _open_list_head->next;
	node->next = NULL;

	// This condition means we just removed the last open block, so set the tail pointer to NULL as well
	if (_open_list_head == NULL) {
		_open_list_tail = NULL;
	}

	return node;
}

// -----------------------------------------------------------------------------
// VariableTexSheet class
// -----------------------------------------------------------------------------

VariableTexSheet::VariableTexSheet(int32 sheet_width, int32 sheet_height, GLuint sheet_id, TexSheetType sheet_type, bool sheet_static) :
	TexSheet(sheet_width, sheet_height, sheet_id, sheet_type, sheet_static)
{
	_block_width = width / 16;
	_block_height = height / 16;
	_blocks = new VariableTexNode[_block_width * _block_height];
}



VariableTexSheet::~VariableTexSheet() {
	if (GetNumberTextures() != 0)
		IF_PRINT_WARNING(VIDEO_DEBUG) << "texture sheet being deleted when it has a non-zero allocated texture count: " << GetNumberTextures() << endl;

	delete[] _blocks;
}



bool VariableTexSheet::AddTexture(BaseTexture* img, ImageMemory& data) {
	if (InsertTexture(img) == false)
		return false;

	// Copy the pixel data for the texture over
	if (CopyRect(img->x, img->y, data) == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "VIDEO ERROR: CopyRect() failed in TexSheet::AddImage()!" << endl;
		return false;
	}

	return true;
} // bool VariableTexSheet::Insert(BaseTexture *img)



bool VariableTexSheet::InsertTexture(BaseTexture* img) {
	if (img == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "NULL pointer was given as function argument" << endl;
		return false;
	}

	// Don't allow insertions into a texture bigger than 512x512...
	// This way, if we have a 1024x1024 texture holding a fullscreen background,
	// it is always safe to remove the texture sheet from memory when the
	// background is unreferenced. That way backgrounds don't stick around in memory.
	if (_block_width > 32 || _block_height > 32) { // 32 blocks = 512 pixels
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
	for (int32 y = 0; y < _block_height - h + 1; y++) {
		for (int32 x = 0; x < _block_width - w + 1; x++) {
			int32 furthest_blocker = -1;

			for (int32 dy = 0; dy < h; dy++) {
				for (int32 dx = 0; dx < w; dx++) {
					if (_blocks[(x + dx) + ((y + dy) * _block_width)].free == false) {
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
	set<BaseTexture*> remove_images;

	for(int32 y = block_y; y < block_y + h; y++) {
		for(int32 x = block_x; x < block_x + w; x++) {
			int32 index = x + (y * _block_width);
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

	for(set<BaseTexture*>::iterator i = remove_images.begin(); i != remove_images.end(); i++) {
		RemoveTexture(*i);
	}


	// Calculate the actual pixel coordinates given this node's block index
	img->x = block_x * 16;
	img->y = block_y * 16;

	// Calculate the u,v coordinates

	float sheet_width = static_cast<float>(width);
	float sheet_height = static_cast<float>(height);

	img->u1 = float(img->x + 0.5f) / sheet_width;
	img->u2 = float(img->x + img->width - 0.5f) / sheet_width;
	img->v1 = float(img->y + 0.5f) / sheet_height;
	img->v2 = float(img->y + img->height - 0.5f) / sheet_height;

	img->texture_sheet = this;

	return true;
} // bool VariableTexSheet::InsertTexture(BaseTexture* img)



uint32 VariableTexSheet::GetNumberTextures() {
	set<BaseTexture*> textures;

	for (int32 i = 0; i < _block_width * _block_height; i++) {
		if (_blocks[i].image != NULL && textures.find(_blocks[i].image) != textures.end()) {
			textures.insert(_blocks[i].image);
		}
	}

	return textures.size();
}



void VariableTexSheet::_SetBlockProperties(BaseTexture* img, bool change_free, bool change_image, bool free, BaseTexture* new_image) {
	if (img == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "NULL pointer was given as function argument" << endl;
		return;
	}

	// Calculate upper-left corner in blocks
	int32 block_x = img->x / 16;
	int32 block_y = img->y / 16;

	// Calculate width and height in blocks
	int32 w = (img->width  + 15) / 16;
	int32 h = (img->height + 15) / 16;

	for (int32 y = block_y; y < block_y + h; y++) {
		for (int32 x = block_x; x < block_x + w; x++) {
			if (_blocks[x + y * _block_width].image == img) {
				if (change_free)
					_blocks[x + y * _block_width].free = free;
				if (change_image)
					_blocks[x + y * _block_width].image = new_image;
			}
		}
	}
}

} // namespace private_video

} // namespace hoa_video
