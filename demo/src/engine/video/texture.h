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
*** \brief   Header file for texture management code
***
*** We use texture management so that at runtime, we can load many small images
*** (e.g. tiles) and stick them together into larger textures called "texture
*** sheets". This improves performance, because then we don't have to constantly
*** switch textures while rendering.
***
*** This file contains several classes:
***
*** - <b>TexSheet</b>: physically represents an OpenGL texture in memory, plus
***  a pointer to a texture management class which keeps track of which images
***  are in that texture sheet.
***
*** - <b>TexMemMgr</b>: abstract base class for texture memory management. The
*** job of a texture memory manager is to allocate sub-rectangles within the
*** sheet so we can stuff images into it.
***
*** - <b>FixedTexMemMgr</b>: texture memory manager for fixed-size images,
*** i.e. 32x32. This class can do all its operations in constant (O(1)) time
*** because it knows in advance that all textures are the same size.
***
*** - <b>FixedImageNode</b>: this represents a single allocation within the fixed-size
*** texture memory manager
***
*** - <b>VariableTexMemMgr</b>: this manages memory for variable-sized images.
*** This works fairly well in practice, but it generally has a fair amount of
*** wasted space, because the images don't always "fit together" perfectly on
*** the sheet.
***
*** - <b>VariableImageNode</b>: a single alloaction within the variable-size 
*** texture memory manager
*** ***************************************************************************/

#ifndef __TEXTURE_HEADER__
#define __TEXTURE_HEADER__

#include "defs.h"
#include "utils.h"

// OpenGL includes
#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

namespace hoa_video {

namespace private_video {

//! \brief Used to indicate an invalid texture ID
const GLuint INVALID_TEXTURE_ID = 0xFFFFFFFF;

//! \brief Represents the different image sizes that a texture sheet can hold
enum TexSheetType {
	VIDEO_TEXSHEET_INVALID = -1,
	
	VIDEO_TEXSHEET_32x32 = 0,
	VIDEO_TEXSHEET_32x64 = 1,
	VIDEO_TEXSHEET_64x64 = 2,
	VIDEO_TEXSHEET_ANY = 3,
	
	VIDEO_TEXSHEET_TOTAL = 4
};

/** ****************************************************************************
*** \brief An abstract base class for texture memory managerment
***
*** This class is used by texture sheets to manage which areas of the sheet are
*** available and which are occupied.
*** ***************************************************************************/
class TexMemMgr {
public:
	virtual ~TexMemMgr()
		{}

	/** \brief Inserts a new block into the texture
	*** \param img A pointer to the image to insert
	*** \return Success/failure
	*/
	virtual bool Insert(BaseImageTexture *img) = 0;
	
	/** \brief Removes a block from the texture
	*** \param img A pointer to the image to remove
	**/
	virtual void Remove(BaseImageTexture *img) = 0;
	
	/** \brief Marks a block as free
	*** \param img A pointer to the image to free
	**/
	virtual void Free(BaseImageTexture *img) = 0;
	
	/** \brief Marks a block previously freed as used
	*** \param img A pointer to the image to restore
	**/
	virtual void Restore(BaseImageTexture *img) = 0;
}; // class TexMemMgr


/** ****************************************************************************
*** \brief An OpenGL texture which can store multiple smaller images in itself
***
*** The purpose of this is to save computation resources on texture switches,
*** so that an increased performance can be achieved.
***
*** \note This is called TexSheet instead of Texture, so that it is clear that
*** this doesn't represent an image that you would draw on the screen, but
*** simply a "container" for smaller images.
*** ***************************************************************************/
class TexSheet {
public:
	/** \brief Constructs a new texture sheet
	*** \param sheet_width The width of the sheet
	*** \param sheet_height The height of the sheet
	*** \param sheet_id The OpenGL texture ID value for the sheet
	*** \param sheet_type The type of texture data that the texture sheet should hold
	*** \param sheet_static Whether the sheet should be labeled static or not
	**/
	TexSheet(int32 sheet_width, int32 sheet_height, GLuint sheet_id, TexSheetType sheet_type, bool sheet_static);

	~TexSheet();

	/** \brief Unloads all texture memory used by OpenGL for this sheet
	*** \return Success/failure
	**/
	bool Unload();
	
	/** \brief Reloads all the images into the sheet and reallocates OpenGL memory
	*** \return Success/failure
	**/
	bool Reload();

	/** \brief Adds a new image to the tex sheet
	*** \param img A pointer to the new image to add
	*** \param load_info The image loading info
	*** \return Success/failure
	**/
	bool AddImage(BaseImageTexture *img, ImageMemory& load_info);

	/** \brief Removes an image completely from the texture sheet's memory manager
	*** \param img The image to remove
	**/
	void RemoveImage(BaseImageTexture *img)
		{ num_textures--; tex_mem_manager->Remove(img); }
	
	/** \brief Marks the image as free
	*** \param img The image to mark as free
	*** \note Marking an image as free does not delete it. The image may be later
	*** restored from the free state so that it does not have to be re-fetched
	*** from the hard disk.
	**/
	void FreeImage(BaseImageTexture *img)
	 	{ num_textures--; tex_mem_manager->Free(img); }
	
	/** \brief Restores an image which was previously freed
	*** \param img The image to mark as used
	**/
	void RestoreImage(BaseImageTexture *img)
		{ num_textures++; tex_mem_manager->Restore(img); }

	/** \brief Copies an image into a sub-rectangle of the texture
	*** \param x X coordinate of rectangle to copy image to
	*** \param y Y coordinate of rectangle to copy image to
	*** \param load_info The image loading info
	*** \return Success/failure
	**/
	bool CopyRect(int32 x, int32 y, private_video::ImageMemory& load_info);

	/** \brief Copies an portion of the screen into a sub-rectangle of the texture
	*** \param x X coordinate of rectangle to copy screen to
	*** \param y Y coordinate of rectangle to copy screen to
	*** \param screen_rect The portion of the screen
	*** \return Success/failure
	**/
	bool CopyScreenRect(int32 x, int32 y, const ScreenRect &screen_rect);

	/** \brief Enables (GL_LINEAR) or disables (GL_NEAREST) smoothing on this texsheet
	*** \param flag True enables smoothing while false disables it. Default value is true.
	**/
	void Smooth(bool flag = true);

	/** \brief Draws the entire texture sheet to the screen
	*** This is primarily used for debugging, as it draws all images contained within the texture to the screen.
	*** It ignores any blending or lighting properties that are enabled in the VideoManager
	**/
	void Draw() const;

	//! \brief The number of textures that are stored within this sheet
	uint32 num_textures;

	//! \brief The width of the texsheet
	int32 width;
	
	//! \brief The height of the texsheet
	int32 height;

	//! \brief The interger that OpenGL uses to refer to this texture
	GLuint tex_id;

	//! \brief The type (dimensions) of images that this texture sheet holds
	TexSheetType type;

	//! \brief A pointer to the manager which controls which areas of the texture are free or occupied
	TexMemMgr* tex_mem_manager;

	//! \brief If true, images in this sheet that are unlikely to change
	bool is_static;
	
	//! \brief Flag indicating if texture sheet is loaded or not
	bool loaded;

	//! \brief True if this texture sheet is currently set to GL_LINEAR
	bool smoothed;
}; // class TexSheet


/** ****************************************************************************
*** \brief Used by the fixed-size texture manager to keep track of which blocks are owned by which images.
***
*** \note The list is doubly linked to allow for O(1) removal
*** ***************************************************************************/
class FixedImageNode {
public:
	//! \brief The image that belongs to the block
	BaseImageTexture* image;
	
	//! \brief The next node in the list
	FixedImageNode* next;
	
	//! \brief The previous node in the list
	FixedImageNode* prev;
	
	//! \brief The block index
	int32 block_index;
}; // class FixedImageNode


/** ****************************************************************************
*** \brief Used to manage texture sheets which are designated for fixed image sizes.
***
*** An example where this class would be used would be a 512x512 pixel sheet that
*** only holds 32x32 pixel tiles. The texture sheet's size must be divisible by
*** the size of the images that it holds. For example, you can't create a 256x256
*** sheet which holds tiles which are 17x93.
*** ***************************************************************************/
class FixedTexMemMgr : public TexMemMgr {
public:
	/** \brief Constructs a new memory manager for a texture sheet
	*** \param tex_sheet The texture sheet which this object will manage
	*** \param img_width The width of the images which will be stored in this sheet
	*** \param img_height The height of the images which will be stored in this sheet
	**/
	FixedTexMemMgr(TexSheet* tex_sheet, int32 img_width, int32 img_height);

	~FixedTexMemMgr()
		{ delete[] _blocks; }

	//! \name Methods inherited from TexMemMgr
	//@{
	bool Insert(BaseImageTexture *img);
	
	void Remove(BaseImageTexture *img);
	
	void Free(BaseImageTexture *img);
	
	void Restore(BaseImageTexture *img)
		{ _DeleteNode(_CalculateBlockIndex(img)); }
	//@}

private:
	/** \brief Grabs the block index based off of the image
	*** \param img The image to look for
	*** \return The block index for that image
	**/
	int32 _CalculateBlockIndex(BaseImageTexture *img);
	
	/** \brief Grabs the block index based off of the image
	*** \param block_index The node in the list to delete
	**/
	void _DeleteNode(int32 block_index);

	//! \brief Sheet width, in number of images
	int32 _sheet_width;

	//! \brief Sheet height, in number of images
	int32 _sheet_height;

	//! \brief Images width, in pixels
	int32 _image_width;

	//! \brief Images height, in pixels
	int32 _image_height;

	//! \brief The texture sheet which is managed by this class object
	TexSheet* _tex_sheet;
	
	/** \brief Head of list of open memory blocks.
	*** The open list keeps track of which blocks of memory are open. Note that
	*** we track blocks with both an array and a list. Although it takes up
	*** more memory, this makes all operations dealing with the blocklist
	*** O(1) so that performance is awesome. Memory isn't too bad either,
	*** since the block list is fairly small.
	**/
	FixedImageNode* _open_list_head;

	/** \brief Tail of list of open memory blocks.
	*** The open list keeps track of which blocks of memory are open. The tail
	*** pointer is also kept so that we can add newly freed blocks to the end
	*** of the list. That way, essentially blocks that are freed are given a
	*** little bit of time from the time they're freed to the time they're
	*** removed, in case they are loaded again in the near future.
	**/
	FixedImageNode* _open_list_tail;
	
	/** \brief This is our actual array of blocks which is indexed like a 2D array.
	*** For example, blocks[x + y * width]->image would tell us which image is
	*** currently allocated at spot (x,y).
	**/
	FixedImageNode* _blocks;
}; // class FixedTexMemMgr : public TexMemMgr


/** ****************************************************************************
*** \brief Keeps track of which images are used/freed in the variable texture mem manager
*** ***************************************************************************/
class VariableImageNode {
public:
	VariableImageNode() :
		image(NULL), free(true) {}

	//! \brief A pointer to the image
	BaseImageTexture* image;
	
	//! \brief Set to true if the image is freed
	bool free;
}; // class VariableImageNode


/** ****************************************************************************
*** \brief Used to manage texture sheets for variable image sizes
***
*** For the sake of reducing the time it takes to allocate an image, this class rounds image
*** dimensions up to powers of 16. So although it's fine to add any-sized images to a variable
*** texture sheet, some space will be wasted for images which are not multiples of 16 pixels.
*** ***************************************************************************/
class VariableTexMemMgr : public TexMemMgr {
public:
	VariableTexMemMgr(TexSheet* sheet);

	~VariableTexMemMgr()
		{ delete[] _blocks; }

	//! \name Methods inherited from TexMemMgr
	//@{
	bool Insert(BaseImageTexture *img);

	void Remove(BaseImageTexture *img)
		{ _SetBlockProperties(img, true, true, true, NULL); }

	void Free(BaseImageTexture *img)
		{ _SetBlockProperties(img, true, false, true, NULL); }

	void Restore(BaseImageTexture *img)
		{ _SetBlockProperties(img, true, false, false, NULL); }
	//@}

private:
	/** \brief Goes through all the blocks associated with an image, and updates their "free" and "image" properties.
	*** This actually will happen when changeFree and changeImage are true, respectively
	*** \param img The image to use for the block
	*** \param change_free The block's free status has changed
	*** \param change_image The block's image has changed
	*** \param free The block's free status
	*** \param new_image The new image to use if changeImage is true
	**/
	void _SetBlockProperties(BaseImageTexture* img, bool change_free, bool change_image, bool free, BaseImageTexture* new_image);

	//! \brief The texhseet it's using
	TexSheet* _tex_sheet;
	
	//! \brief It's list of blocks
	VariableImageNode* _blocks;
		
	//! \brief Sheet width in number of blocks
	int32 _sheet_width;

	//! \brief Sheet height in number of blocks
	int32 _sheet_height;
}; // class VariableTexMemMgr : public TexMemMgr

}  // namespace private_video

}  // namespace hoa_video

#endif // __TEXTURE_HEADER__
