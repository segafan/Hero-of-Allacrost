///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    tex_mgmt.h
 * \author  Raj Sharma, roos@allacrost.org
 * \brief   Header file for texture management code
 *
 * We use texture management so that at runtime, we can load many small
 * images (e.g. tiles) and stick them together into larger textures called
 * "texture sheets". This improves performance, because then we don't have to
 * constantly switch textures while rendering.
 *
 * This file contains several classes:
 *
 * TexSheet: physically represents an OpenGL texture in memory, plus a pointer
 *           to a texture management class which keeps track of which images
 *           are in that texture sheet.
 *
 * TexMemMgr: abstract base class, used by TexSheet. The job of a texture
 *            memory manager is to allocate sub-rectangles within the sheet
 *            so we can stuff images into it.
 *
 * FixedTexMemMgr: texture memory manager for fixed-size images, i.e. 32x32.
 *                 This class can do all its operations in constant (O(1)) time
 *                 because it knows in advance that all textures are the same size
 *
 * FixedImageNode: this represents a single allocation within the fixed-size
 *                 texture memory manager
 *
 * VariableTexMemMgr: this manages memory for any size images. This works fairly
 *                    well in practice, but it generally has a fair amount of wasted
 *                    space, because the images don't always "fit together" perfectly
 *                    on the sheet.
 *
 * VariableImageNode: a single alloaction within the variable-size texture memory
 *                    manager
 *****************************************************************************/ 


#ifndef __TEX_MGMT_HEADER__
#define __TEX_MGMT_HEADER__

#include "utils.h"

// OpenGL includes
#include <GL/gl.h>
#include <GL/glu.h>


namespace hoa_video
{

namespace private_video
{

class Image;


/*!***************************************************************************
 *  \brief Represents the different image sizes that a texture sheet can hold
 *****************************************************************************/

enum TexSheetType
{
	VIDEO_TEXSHEET_INVALID = -1,
	
	VIDEO_TEXSHEET_32x32,
	VIDEO_TEXSHEET_32x64,
	VIDEO_TEXSHEET_64x64,
	VIDEO_TEXSHEET_ANY,
	
	VIDEO_TEXSHEET_TOTAL
};


/*!***************************************************************************
 *  \brief base class for texture memory managers. It is used by TextureSheet
 *         to manage which areas of the texture are available and which are used.
 *****************************************************************************/

class TexMemMgr
{
public:
	
	virtual ~TexMemMgr() {}

	virtual bool Insert  (Image *img)=0;
	virtual bool Remove  (Image *img)=0;
	virtual bool Free    (Image *img)=0;
	virtual bool Restore (Image *img)=0;
	
};


/*!***************************************************************************
 *  \brief An actual OpenGL texture which can be used for storing multiple
 *         smaller images in it, to save on texture switches.
 *
 *  \note  This is called TexSheet instead of Texture, so it's clear that
 *         this doesn't represent an image that you would draw on the screen,
 *         but simply a "container" for smaller images.
 *****************************************************************************/

class TexSheet
{
public:

	TexSheet(int32 w, int32 h, GLuint texID_, TexSheetType type_, bool isStatic_);
	~TexSheet();

	bool AddImage                   //! adds new image to the tex sheet
	(
		Image *img
	);
	
	bool SaveImage(Image *img);
	
	bool CopyRect(int32 x, int32 y, int32 w, int32 h);
	
	bool RemoveImage (Image *img);  //! removes an image completely
	bool FreeImage   (Image *img);  //! marks the image as free
	bool RestoreImage (Image *img); //! marks a previously freed image as "used"
	
	bool Unload();  //! unloads texture memory used by this sheet
	bool Reload();  //! reloads all the images into the sheet

	int32 width;
	int32 height;

	bool isStatic;       //! if true, images in this sheet that are unlikely to change
	TexSheetType type;   //! does it hold 32x32, 32x64, 64x64, or any kind

	TexMemMgr *texMemManager;  //! manages which areas of the texture are free

	GLuint texID;     //! number OpenGL uses to refer to this texture
	bool loaded;		
};


/*!***************************************************************************
 *  \brief used by the fixed-size texture manager to keep track of which blocks
 *         are owned by which images.
 *
 *  \note  The list is doubly linked to allow for O(1) removal
 *****************************************************************************/

class FixedImageNode
{
public:
	Image          *image;
	FixedImageNode *next;
	FixedImageNode *prev;
	
	int32 blockIndex;
};


/*!***************************************************************************
 *  \brief used to manage textures which are designated for fixed image sizes.
 *         For example, a 512x512 sheet that only holds 32x32 tiles.
 *
 *  \note  The texture sheet's size must be divisible by the size of the images
 *         that it holds. For example, you can't create a 256x256 sheet which
 *         holds tiles which are 17x93.
 *****************************************************************************/

class FixedTexMemMgr : public TexMemMgr
{
public:
	FixedTexMemMgr(TexSheet *texSheet, int32 imgW, int32 imgH);
	~FixedTexMemMgr();
	
	bool Insert  (Image *img);
	bool Remove  (Image *img);
	bool Free    (Image *img);
	bool Restore (Image *img);

private:

	int32 CalculateBlockIndex(Image *img);
	void DeleteNode(int32 blockIndex);
	
	//! store dimensions of both the texture sheet, and the images that
	//! it contains
	
	//! NOTE: the sheet dimensions are not in pixels, but images.
	//!       So, a 512x512 sheet holding 32x32 images would be 16x16
	
	int32 _sheetWidth;
	int32 _sheetHeight;
	int32 _imageWidth;
	int32 _imageHeight;
	
	TexSheet *_texSheet;
	
	//! The open list keeps track of which blocks of memory are
	//! open. Note that we track blocks with BOTH an array and a list.
	//! Although it takes up more memory, this makes ALL operations
	//! dealing with the blocklist O(1) so performance is awesome.
	//! Memory isn't too bad either, since blocklist is fairly small.
	//! The tail pointer is also kept so that we can add newly
	//! freed blocks to the end of the list- that way, essentially
	//! blocks that are freed are given a little bit of time from the
	//! time they're freed to the time they're removed, in case they
	//! are loaded again in the near future
	
	FixedImageNode *_openListHead;
	FixedImageNode *_openListTail;
	
	//! this is our actual array of blocks which is indexed like a 2D
	//! array. For example, blocks[x+y*width]->image would tell us
	//! which image is currently allocated at spot (x,y)
	
	FixedImageNode *_blocks;
};


/*!***************************************************************************
 *  \brief this is how we keep track of which images are used/freed in the
 *         variable texture mem manager
 *****************************************************************************/

class VariableImageNode
{
public:
	VariableImageNode()
	{
		image = NULL;
		free  = true;
	}

	Image *image;
	bool   free;
};


/*!***************************************************************************
 *  \brief used to manage a texture sheet where the size of the images it
 *         will contain are unknown
 *
 *  \note  For the sake of reducing the time it takes to allocate an image,
 *         this class rounds image dimensions up to powers of 16. So although
 *         it's fine to add any-sized images to a variable texture sheet,
 *         some space will be wasted due to rounding.
 *****************************************************************************/

class VariableTexMemMgr : public TexMemMgr
{
public:
	
	VariableTexMemMgr(TexSheet *sheet);
	~VariableTexMemMgr();

	bool Insert  (Image *img);
	bool Remove  (Image *img);
	bool Free    (Image *img);
	bool Restore (Image *img);

private:

	bool SetBlockProperties
	(
		Image *img, 
		bool changeFree, 
		bool changeImage, 
		bool free, 
		Image *newImage
	);

	TexSheet *_texSheet;
	VariableImageNode *_blocks;
	
	//! Sheet's dimensions
	//! NOTE: these aren't in pixels, but in "blocks" of 16x16. So,
	//!       a 512x512 sheet would be 32x32 in blocks
	
	int32 _sheetWidth;
	int32 _sheetHeight;
};

}  // namespace private_video
}  // namespace hoa_video

#endif   // !__TEX_MGMT_HEADER__
