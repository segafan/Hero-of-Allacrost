#include "utils.h"
#include <cassert>
#include <cstdarg>
#include "video.h"
#include <math.h>
#include "coord_sys.h"
#include "gui.h"

using namespace std;
using namespace hoa_video::local_video;
using namespace hoa_video;

namespace hoa_video 
{

//-----------------------------------------------------------------------------
// RoundUpPow2: rounds up a number to the nearest power of 2
//-----------------------------------------------------------------------------

uint RoundUpPow2(unsigned x)
{
	x -= 1;
	x |= x >>  1;
	x |= x >>  2;
	x |= x >>  4;
	x |= x >>  8;
	x |= x >> 16;
	return x + 1;
}


//-----------------------------------------------------------------------------
// IsPowerOfTwo: returns true if given number is a power of 2
//-----------------------------------------------------------------------------

bool IsPowerOfTwo(uint x)
{
	return ((x & (x-1)) == 0);
}


//-----------------------------------------------------------------------------
// LoadImage: loads an image and returns it in the image descriptor
//            On failure, returns false.
//
//            If isStatic is true, that means this is an image that is probably
//            to remain in memory for the entire game, so place it in a
//            special texture sheet reserved for things that don't change often.
//-----------------------------------------------------------------------------

bool GameVideo::LoadImage(ImageDescriptor &id)
{
	// 1. special case: if filename is empty, load a colored quad
	
	if(id.filename.empty())
	{
		id._elements.clear();		
		ImageElement quad(NULL, 0.0f, 0.0f, id.width, id.height, id.color);
		id._elements.push_back(quad);		
		return true;
	}
	
	
	// 2. check if an image with the same filename has already been loaded
	//    If so, point to that
	
	if(_images.find(id.filename) != _images.end())
	{
		id._elements.clear();		
		
		Image *img = _images[id.filename];
		
		if(!img)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: got a NULL Image from images map in LoadImage()" << endl;
			return false;
		}

		if(img->refCount == 0)
		{
			// if ref count is zero, it means this image was freed, but
			// not removed, so restore it
			if(!img->texSheet->RestoreImage(img))
				return false;
		}
		
		++img->refCount;
		
		if(id.width == 0.0f)
			id.width = (float) img->width;
		if(id.height == 0.0f)
			id.height = (float) img->height;
		
		ImageElement element(img, 0, 0, id.width, id.height, id.color);		
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
	
	return LoadImageImmediate(id, id.isStatic);
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

	std::vector <ImageDescriptor *>::iterator iImage = _batchLoadImages.begin();
	std::vector <ImageDescriptor *>::iterator iEnd   = _batchLoadImages.end();
	
	bool success = true;
	
	while(iImage != iEnd)
	{
		ImageDescriptor *id = *iImage;
		
		if(!id)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: got a NULL ImageDescriptor in EndImageLoadBatch()!" << endl;
			success = false;
		}
		
		if(!LoadImage(*id))
			success = false;
		
		++iImage;
	}

	_batchLoadImages.clear();	
	
	return success;
}



//-----------------------------------------------------------------------------
// LoadImageImmediate: private function which does the dirty work of actually
//                     loading an image.
//-----------------------------------------------------------------------------

bool GameVideo::LoadImageImmediate(ImageDescriptor &id, bool isStatic)
{
	id._elements.clear();

	ILuint pixelData;
	uint w, h;
	
	if(!LoadRawPixelData(id.filename, pixelData, w, h))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: LoadRawPixelData() failed in LoadImageImmediate()" << endl;
		return false;
	}


	// create an Image structure and store it our std::map of images
	Image *newImage = new Image(id.filename, w, h);

	// try to insert the image in a texture sheet
	int x, y;
	TexSheet *sheet = InsertImageInTexSheet(newImage, pixelData, x, y, w, h, isStatic);
	
	if(!sheet)
	{
		// this should never happen, unless we run out of memory or there
		// is a bug in the InsertImageInTexSheet() function
		
		if(VIDEO_DEBUG)
			cerr << "VIDEO_DEBUG: GameVideo::InsertImageInTexSheet() returned NULL!" << endl;
		
		ilDeleteImages(1, &pixelData);
		return false;
	}
	
	newImage->refCount = 1;
	
	// store the image in our std::map
	_images[id.filename] = newImage;


	// if width or height are zero, that means to use the dimensions of image
	if(id.width == 0.0f)
		id.width = (float) w;
	
	if(id.height == 0.0f)
		id.height = (float) h;

	// store the new image element
	ImageElement element(newImage, 0, 0, id.width, id.height, id.color);
	id._elements.push_back(element);

	// finally, delete the buffer DevIL used to load the image
	ilDeleteImages(1, &pixelData);
	
	if(ilGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ilGetError() true after ilDeleteImages() in LoadImageImmediate()!" << endl;
		return false;
	}
	
	return true;
}


//-----------------------------------------------------------------------------
// LoadRawPixelData: uses DevIL to load the given filename.
//                   Returns the DevIL texture ID, width and height
//                   Upon exit, leaves this image as the currently "bound" image
//-----------------------------------------------------------------------------

bool GameVideo::LoadRawPixelData(const string &filename, ILuint &pixelData, uint &w, uint &h)
{
	// load the image using DevIL

	ilGenImages(1, &pixelData);
	
	if(ilGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "ilGetError() true after ilGenImages() in LoadImageImmediate()!" << endl;
		return false;
	}
	
	ilBindImage(pixelData);
	
	if(ilGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "ilGetError() true after ilBindImage() in LoadImageImmediate()!" << endl;
		return false;
	}

	if (!ilLoadImage((char *)filename.c_str())) 
	{
		ilDeleteImages(1, &pixelData);
		return false;
	}
	
	// find width and height
	
	w = ilGetInteger(IL_IMAGE_WIDTH);
	h = ilGetInteger(IL_IMAGE_HEIGHT);
	
	return true;
}	


//-----------------------------------------------------------------------------
// AddImage: this is the function that gives us the ability to form
//           "compound images". Call AddImage() on an existing image
//           descriptor to place a new image at the desired offsets.
//
// NOTE: it is an error to pass in negative offsets
//
// NOTE: when you create a compound image descriptor with AddImage(),
//       remember to call DeleteImage() on it when you're done. Even though
//       it's not loading any new image from disk, it increases the ref counts.
//-----------------------------------------------------------------------------

bool ImageDescriptor::AddImage
(
	const ImageDescriptor &id, 
	float xOffset, 
	float yOffset
)
{
	if(xOffset < 0.0f || yOffset < 0.0f)
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: passed negative offsets to AddImage()!" << endl;
		}
		
		return false;
	}
	
	size_t numElements = id._elements.size();
	if(numElements == 0)
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: passed in an uninitialized image descriptor to AddImage()!" << endl;
		}
		
		return false;
	}
	
	for(uint iElement = 0; iElement < numElements; ++iElement)
	{
		// add the new image element to our descriptor
		ImageElement elem = id._elements[iElement];
		elem.xOffset += xOffset;
		elem.yOffset += yOffset;
		
		if(elem.image)
		{
			++(elem.image->refCount);
		}
		
		_elements.push_back(elem);

		// recalculate width and height of the descriptor as a whole
		// This assumes that there are no negative offsets
		float maxX = elem.xOffset + elem.width;
		if(maxX > width)
			width = maxX;
			
		float maxY = elem.yOffset + elem.height;
		if(maxY > height)
			height = maxY;		
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

ImageDescriptor GameVideo::TilesToObject
( 
	vector<ImageDescriptor> &tiles, 
	vector< vector<uint> > indices 
)
{	
	ImageDescriptor id;

	// figure out the width and height information
		
	int w, h;
	w = (int) indices[0].size();         // how many tiles wide and high
	h = (int) indices.size();

	float tileWidth  = tiles[0].width;   // width and height of each tile
	float tileHeight = tiles[0].height;
	
	id.width  = (float) w * tileWidth;   // total width/height of compound
	id.height = (float) h * tileHeight;
	
	id.isStatic = tiles[0].isStatic;
	
	for(int y = 0; y < h; ++y)
	{
		for(int x = 0; x < w; ++x)
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
// InsertImageInTexSheet: takes an image that was loaded with DevIL, finds
//                        an available texture sheet, copies it to the sheet,
//                        and returns a pointer to the texture sheet. If no
//                        available texture sheet is found, a new one is created.
//
//                        Returns NULL on failure, which should only happen if
//                        we run out of memory or bad argument is passed.
//-----------------------------------------------------------------------------

TexSheet *GameVideo::InsertImageInTexSheet
(
	Image *image,
	ILuint pixelData, 
	int &x, 
	int &y,
	int w,
	int h,
	bool isStatic
)
{
	// if it's a large image size (>512x512) then we already know it's not going
	// to fit in any of our existing texture sheets, so create a new one for it

	if(w > 512 || h > 512)
	{
		int roundW = RoundUpPow2(w);
		int roundH = RoundUpPow2(h);		
		TexSheet *sheet = CreateTexSheet(roundW, roundH, VIDEO_TEXSHEET_ANY, false);

		// ran out of memory!
		if(!sheet)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: CreateTexSheet() returned NULL in InsertImageInTexSheet()!" << endl;
			return NULL;
		}
					
		if(sheet->AddImage(image, pixelData))
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
	
	if(w == 32 && h == 32)
		type = VIDEO_TEXSHEET_32x32;
	else if(w == 32 && h == 64)
		type = VIDEO_TEXSHEET_32x64;
	else if(w == 64 && h == 64)
		type = VIDEO_TEXSHEET_64x64;
	else
		type = VIDEO_TEXSHEET_ANY;
		
	// loop through existing texture sheets and see if the image will fit in
	// any of the ones which match the type we're looking for
	
	size_t numTexSheets = _texSheets.size();
	
	for(uint iSheet = 0; iSheet < numTexSheets; ++iSheet)
	{
		TexSheet *sheet = _texSheets[iSheet];
		if(!sheet)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: _texSheets[iSheet] was NULL in InsertImageInTexSheet()!" << endl;
			return NULL;
		}
		
		if(sheet->type == type && sheet->isStatic == isStatic)
		{
			if(sheet->AddImage(image, pixelData))
			{
				// added to a sheet successfully
				return sheet;
			}
		}
	}
	
	// if it doesn't fit in any of them, create a new 512x512 and stuff it in
	
	TexSheet *sheet = CreateTexSheet(512, 512, type, isStatic);
	if(!sheet)
	{
		// failed to create texture, ran out of memory probably
		
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: Failed to create new texture sheet in InsertImageInTexSheet!" << endl;
		}
		
		return NULL;
	}

	// now that we have a fresh texture sheet, AddImage() should work without
	// any problem
	if(sheet->AddImage(image, pixelData))
	{
		return sheet;
	}

	return NULL;
}


//-----------------------------------------------------------------------------
// CreateTexSheet: creates a new texture sheet with the given parameters,
//                 adds it to our internal vector of texture sheets, and
//                 returns a pointer to it.
//                 Returns NULL on failure, which should only happen if
//                 we run out of memory or bad argument is passed.
//-----------------------------------------------------------------------------

TexSheet *GameVideo::CreateTexSheet
(
	int width,
	int height,
	TexSheetType type,
	bool isStatic
)
{
	// validate the parameters	

	if(!IsPowerOfTwo(width) || !IsPowerOfTwo(height))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: non pow2 width and/or height passed to CreateTexSheet!" << endl;
			
		return NULL;
	}
	
	if(type <= VIDEO_TEXSHEET_INVALID || type >= VIDEO_TEXSHEET_TOTAL)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: Invalid TexSheetType passed to CreateTexSheet()!" << endl;
			
		return NULL;
	}
	
	GLuint texID = CreateBlankGLTexture(width, height);	
	
	// now that we have our texture loaded, simply create a new TexSheet	
	
	TexSheet *sheet = new TexSheet(width, height, texID, type, isStatic);
	_texSheets.push_back(sheet);
	
	return sheet;
}


//-----------------------------------------------------------------------------
// TexSheet constructor
//-----------------------------------------------------------------------------

TexSheet::TexSheet(int w, int h, GLuint texID_, TexSheetType type_, bool isStatic_)
{
	width = w;
	height = h;
	texID = texID_;
	type = type_;
	isStatic = isStatic_;
	loaded = true;
	
	if(type == VIDEO_TEXSHEET_32x32)
		_texMemManager = new FixedTexMemMgr(this, 32, 32);
	else if(type == VIDEO_TEXSHEET_32x64)
		_texMemManager = new FixedTexMemMgr(this, 32, 64);
	else if(type == VIDEO_TEXSHEET_64x64)
		_texMemManager = new FixedTexMemMgr(this, 64, 64);
	else
		_texMemManager = new VariableTexMemMgr(this);
}


//-----------------------------------------------------------------------------
// TexSheet destructor
//-----------------------------------------------------------------------------

TexSheet::~TexSheet()
{
	// delete texture memory manager
	delete _texMemManager;
	
	hoa_video::GameVideo *videoManager = hoa_video::GameVideo::_GetReference();
	
	if(!videoManager)
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: GameVideo::_GetReference() returned NULL in TexSheet destructor!" << endl;
		}
	}
	
	// unload actual texture from memory
	videoManager->DeleteTexture(texID);
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


bool GameVideo::DEBUG_ShowTexSheet()
{
	// value of zero means to disable display
	if(_currentDebugTexSheet == -1)
		return true;
		
	// check if there aren't any texture sheets! (should never happen)
	if(_texSheets.empty())
	{
		if(VIDEO_DEBUG)		
			cerr << "VIDEO_WARNING: Called DEBUG_ShowTexture(), but there were no texture sheets" << endl;
		return false;
	}
	
	
	int numSheets = (int) _texSheets.size();
	
	// we may go out of bounds say, if we were viewing a texture sheet and then it got
	// deleted or something. To recover, just set it to the last texture sheet
	if(_currentDebugTexSheet >= numSheets)
	{
		_currentDebugTexSheet = numSheets - 1;
	}
	
	TexSheet *sheet = _texSheets[_currentDebugTexSheet];
	
	if(!sheet)
		return false;
	
	int w = sheet->width;
	int h = sheet->height;
	
	Image img( sheet, string(), 0, 0, w, h, 0.0f, 0.0f, 1.0f, 1.0f );

	int blend = _blend;
	int xalign = _xalign;
	int yalign = _yalign;
	
	SetDrawFlags(VIDEO_NO_BLEND, VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
	
	glPushMatrix();

	Move(0.0f,0.0f);
	glScalef(0.5f, 0.5f, 0.5f);
	
	if(!DrawElement(&img, (float)w, (float)h, Color(1.0f, 1.0f, 1.0f, 1.0f)))
		return false;
		
	glPopMatrix();
	
	_blend = blend;
	_xalign = xalign;
	_yalign = yalign;

	if(!SetFont("default"))
		return false;
	
	char buf[200];
	
	if(!DrawText("Current Texture sheet:", 20, _coordSys._top - 30))
		return false;
	
	sprintf(buf, "  Sheet #: %d", _currentDebugTexSheet);	
	if(!DrawText(buf, 20, _coordSys._top - 50))
		return false;
	
	sprintf(buf, "  Size:    %dx%d", sheet->width, sheet->height);
	if(!DrawText(buf, 20, _coordSys._top - 70))
		return false;
	
	if(sheet->type == VIDEO_TEXSHEET_32x32)
		sprintf(buf, "  Type:    32x32");
	else if(sheet->type == VIDEO_TEXSHEET_32x64)
		sprintf(buf, "  Type:    32x64");
	else if(sheet->type == VIDEO_TEXSHEET_64x64)
		sprintf(buf, "  Type:    64x64");
	else if(sheet->type == VIDEO_TEXSHEET_ANY)
		sprintf(buf, "  Type:    Any size");
	
	if(!DrawText(buf, 20, _coordSys._top - 90))
		return false;
	
	sprintf(buf, "  Static:  %d", sheet->isStatic);
	if(!DrawText(buf, 20, _coordSys._top - 110))
		return false;

	sprintf(buf, "  TexID:   %d", sheet->texID);
	if(!DrawText(buf, 20, _coordSys._top - 130))
		return false;
	
	return true;
}


//-----------------------------------------------------------------------------
// DeleteImage: decreases the reference count on an image, and deletes it
//              if zero is reached. Note that for images larger than 512x512,
//              there is no reference counting; we just delete it immediately
//              because we don't want huge textures sitting around in memory
//-----------------------------------------------------------------------------

bool GameVideo::DeleteImage(Image *const img)
{
	if(img->width > 512 || img->height > 512)
	{
		// remove the image and texture sheet completely
		DeleteTexSheet(img->texSheet);
		RemoveImage(img);
	}
	else
	{
		// for smaller images, simply mark them as free in the memory manager
		--img->refCount;
		if(img->refCount <= 0)
		{
			img->texSheet->FreeImage(img);
		}
	}
	
	return true;
}


//-----------------------------------------------------------------------------
// DeleteTexSheet: deletes a texture sheet with given pointer
//-----------------------------------------------------------------------------

bool GameVideo::DeleteTexSheet (TexSheet *const sheet)
{
	RemoveSheet(sheet);
	return true;
}


//-----------------------------------------------------------------------------
// RemoveSheet: removes a texture sheet from the internal std::vector
//-----------------------------------------------------------------------------

bool GameVideo::RemoveSheet(TexSheet *sheet)
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

bool TexSheet::AddImage(Image *img, ILuint pixelData)
{
	// try inserting into the texture memory manager
	bool couldInsert = _texMemManager->Insert(img);	
	if(!couldInsert)
		return false;
	
	// now img contains the x, y, width, and height of the subrectangle
	// inside the texture sheet, so go ahead and copy that area
		
	TexSheet *texSheet = img->texSheet;
	if(!texSheet)
	{
		// technically this should never happen since Insert() returned true
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: texSheet was NULL after texMemManager->Insert() returned true" << endl;
		}
		return false;
	}

	if(!CopyRect(pixelData, img->x, img->y, img->width, img->height))
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

bool TexSheet::CopyRect(ILuint pixelData, int x, int y, int w, int h)
{
	int error;
	
	hoa_video::GameVideo *videoManager = hoa_video::GameVideo::_GetReference();
	videoManager->BindTexture(texID);	
	
	error = glGetError();
	if(error)
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: could not bind texture in TexSheet::CopyRect()!" << endl;
		}
		return false;
	}
	
	ILubyte *pixels = ilGetData();
	
	GLenum format = ilGetInteger(IL_IMAGE_FORMAT);

	glTexSubImage2D
	(
		GL_TEXTURE_2D,    // target
		0,                // level
		x,                // x offset within tex sheet
		y,                // y offset within tex sheet
		w,                // width in pixels of image
		h,                // height in pixels of image
		format,           // format
		GL_UNSIGNED_BYTE, // type
		pixels            // pixels of the sub image
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
// RemoveImage: removes an image completely from the texture sheet's
//              memory manager so that a new image can be loaded in its place
//-----------------------------------------------------------------------------

bool TexSheet::RemoveImage(Image *img)
{
	return _texMemManager->Remove(img);
}


//-----------------------------------------------------------------------------
// FreeImage: sets the area taken up by the image to "free". However, the
//            image is not removed from any lists yet! It's kept around in
//            case we reload the image in the near future- in that case,
//            we can simply Restore the image instead of reloading from disk.
//-----------------------------------------------------------------------------

bool TexSheet::FreeImage(Image *img)
{
	return _texMemManager->Free(img);
}

//-----------------------------------------------------------------------------
// RestoreImage: If an image is freed using FreeImage, and soon afterwards,
//               we load that image again, this function restores the image
//               without reloading the image from disk.
//-----------------------------------------------------------------------------

bool TexSheet::RestoreImage(Image *img)
{
	return _texMemManager->Restore(img);
}


//-----------------------------------------------------------------------------
// DeleteImage: decrements the reference count for all images composing this
//              image descriptor.
//
// NOTE: for images which are 1024x1024 or higher, once their reference count
//       reaches zero, they're immediately deleted. (don't want to keep those
//       in memory if possible). For others, they're simply marked as "free"
//-----------------------------------------------------------------------------

bool GameVideo::DeleteImage(ImageDescriptor &id)
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
			if(img->refCount <= 0)
			{
				if(VIDEO_DEBUG)		
					cerr << "VIDEO ERROR: Called DeleteImage() when refcount was already <= 0!" << endl;
				return false;
			}

			--img->refCount;	
			
			if(img->refCount == 0)
			{
				// 1. If it's on a large tex sheet (> 512x512), delete it
				// Note: We can assume that this is the only image on that texture
				//       sheet, so it's safe to delete it. (Big textures always
				//       are allocated to their own sheet, by design.)
				
				if(img->width > 512 || img->height > 512)
				{				
					DeleteImage   (img);  // overloaded DeleteImage for deleting Image
				}

				// 2. otherwise, mark it as "freed"
				
				else if(!img->texSheet->FreeImage(img))
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
	id.filename.clear();
	id.height = id.width = 0;
	id.isStatic = 0;
	
	return true;
}


//-----------------------------------------------------------------------------
// RemoveImage: removes the image pointer from the std::map
//-----------------------------------------------------------------------------

bool GameVideo::RemoveImage(Image *img)
{
	// nothing to do if img is null
	if(!img)
		return true;

	if(_images.empty())
	{
		return false;
	}
		
	map<FileName, Image*>::iterator iImage = _images.begin();
	map<FileName, Image*>::iterator iEnd   = _images.end();
	
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
	int imgW, 
	int imgH
)
{
	_texSheet = texSheet;
	
	// set all the dimensions	
	_sheetWidth  = texSheet->width  / imgW;
	_sheetHeight = texSheet->height / imgH;
	_imageWidth  = imgW;
	_imageHeight = imgH;
	
	// allocate the blocks array	
	int numBlocks = _sheetWidth * _sheetHeight;
	_blocks = new FixedImageNode[numBlocks];
	
	// initialize linked list of open blocks... which, at this point is
	// all the blocks!	
	_openListHead = &_blocks[0];
	_openListTail = &_blocks[numBlocks-1];
	
	// now initialize all the blocks to proper values		
	for(int i = 0; i < numBlocks - 1; ++i)
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
	
	int w = (img->width  + 15) / 16;   // width and height in blocks
	int h = (img->height + 15) / 16;

	int blockX=-1, blockY=-1;
	
	
	// this is a 100% brute force way to allocate a block, just a bunch
	// of nested loops. In practice, this actually works fine, because 
	// the allocator deals with 16x16 blocks instead of trying to worry
	// about fitting images with pixel perfect resolution.
	// Later, if this turns out to be a bottleneck, we can rewrite this
	// algorithm to something more intelligent ^_^
	for(int y = 0; y < _sheetHeight - h + 1; ++y)
	{
		for(int x = 0; x < _sheetWidth - w + 1; ++x)
		{
			int furthestBlocker = -1;
			
			for(int dy = 0; dy < h; ++dy)
			{
				for(int dx = 0; dx < w; ++dx)
				{
					if(!_blocks[(x+dx)+(y+dy)*_sheetWidth].free)
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
	
	hoa_video::GameVideo *VideoManager = hoa_video::GameVideo::_GetReference();

	// update blocks
	for(int y = blockY; y < blockY + h; ++y)
	{
		int index = y*_sheetHeight + blockX;
		for(int x = blockX; x < blockX + w; ++x)
		{			
			// check if there's already an image at the point we're
			// trying to load at. If so, we need to tell GameVideo
			// to update its internal vector
			
			if(_blocks[index].image)
			{
				VideoManager->RemoveImage(_blocks[index].image);
			}


			_blocks[index].free  = false;
			_blocks[index].image = img;
			
			++index;
		}
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

	img->texSheet = _texSheet;
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
	int blockX = img->x / 16;          // upper-left corner in blocks
	int blockY = img->y / 16;
	
	int w = (img->width  + 15) / 16;   // width and height in blocks
	int h = (img->height + 15) / 16;

	for(int y = blockY; y < blockY + h; ++y)
	{
		for(int x = blockX; x < blockX + w; ++x)
		{
			if(changeFree)
				_blocks[x+y*_sheetWidth].free  = free;
			if(changeImage)
				_blocks[x+y*_sheetWidth].image = newImage;
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
		hoa_video::GameVideo *VideoManager = hoa_video::GameVideo::_GetReference();
		VideoManager->RemoveImage(node->image);
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

	img->texSheet = _texSheet;
	
	return true;
}


//-----------------------------------------------------------------------------
// CalculateBlockIndex: returns the block index used up by this image
//-----------------------------------------------------------------------------

int FixedTexMemMgr::CalculateBlockIndex(Image *img)
{
	int blockX = img->x / _imageWidth;
	int blockY = img->y / _imageHeight;
	
	int blockIndex = blockX + _sheetWidth * blockY;
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
	int blockIndex = CalculateBlockIndex(img);
	
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
	DeleteNode(blockIndex);
	
	return true;
}


//-----------------------------------------------------------------------------
// DeleteNode: deletes a node from the open list with the given block index
//-----------------------------------------------------------------------------

void FixedTexMemMgr::DeleteNode(int blockIndex)
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
	int blockIndex = CalculateBlockIndex(img);
	
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
	int blockIndex = CalculateBlockIndex(img);	
	DeleteNode(blockIndex);	
	return true;
}


//-----------------------------------------------------------------------------
// DEBUG_NextTexSheet: increments to the next texture sheet to show with
//                     DEBUG_ShowTexSheet().
//-----------------------------------------------------------------------------

void GameVideo::DEBUG_NextTexSheet()
{
	++_currentDebugTexSheet;
	
	if(_currentDebugTexSheet >= (int) _texSheets.size()) 
	{
		_currentDebugTexSheet = -1;   // disable display
	}
}


//-----------------------------------------------------------------------------
// DEBUG_PrevTexSheet: cycles to the previous texturesheet to show with
//                     DEBUG_ShowTexSheet().
//-----------------------------------------------------------------------------

void GameVideo::DEBUG_PrevTexSheet()
{
	--_currentDebugTexSheet;
	
	if(_currentDebugTexSheet < -1)
	{
		_currentDebugTexSheet = (int) _texSheets.size() - 1;
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

	if(_usesLights)
		_lightOverlay = CreateBlankGLTexture(1024, 1024);

	return success;
}


//-----------------------------------------------------------------------------
// UnloadTextures: frees the texture memory taken up by the texture sheets, 
//                 but leaves the lists of images intact so we can reload them
//                 Returns false if any of the textures fail to unload.
//-----------------------------------------------------------------------------

bool GameVideo::UnloadTextures() 
{
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
		DeleteTexture(_lightOverlay);
		_lightOverlay = 0xFFFFFFFF;
	}

	return success;
}

//-----------------------------------------------------------------------------
// DeleteTexture: wraps call to glDeleteTexture(), adds some checking to see
//                if we deleted the last texture we bound using BindTexture(),
//                then set the last tex ID to 0xffffffff
//-----------------------------------------------------------------------------

bool GameVideo::DeleteTexture(GLuint texID)
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
	hoa_video::GameVideo *videoManager = hoa_video::GameVideo::_GetReference();
	if(!videoManager->DeleteTexture(texID))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: DeleteTexture() failed in TexSheet::Unload()!" << endl;
		return false;
	}
	
	loaded = false;
	return true;
}


//-----------------------------------------------------------------------------
// CreateBlankGLTexture: creates a blank texture of the given width and height
//                       and returns its OpenGL texture ID.
//                       Returns 0xffffffff on failure
//-----------------------------------------------------------------------------

GLuint GameVideo::CreateBlankGLTexture(int width, int height)
{
	// attempt to create a GL texture with the given width and height
	// if texture creation fails, return NULL

	int error;
			
	GLuint texID;

	glGenTextures(1, &texID);
	error = glGetError();
	
	if(!error)   // if there's no error so far, attempt to bind texture
	{
		BindTexture(texID);
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
		DeleteTexture(texID);
		
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: failed to create new texture in CreateBlankGLTexture()." << endl;
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
	hoa_video::GameVideo *videoManager = hoa_video::GameVideo::_GetReference();	
	GLuint tID = videoManager->CreateBlankGLTexture(width, height);
	
	if(tID == 0xFFFFFFFF)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: CreateBlankGLTexture() failed in TexSheet::Reload()!" << endl;
		return false;
	}
	
	texID = tID;
	
	// now the hard part: go through all the images that belong to this texture
	// and reload them again. (GameVideo's function, ReloadImagesToSheet does this)

	if(!videoManager->ReloadImagesToSheet(this))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: CopyImagesToSheet() failed in TexSheet::Reload()!" << endl;
		return false;
	}
	
	loaded = true;	
	return true;
}


//-----------------------------------------------------------------------------
// ReloadImagesToSheet: helper function of the GameVideo class to
//                      TexSheet::Reload() to do the dirty work of reloading
//                      image data into the appropriate spots on the texture
//-----------------------------------------------------------------------------

bool GameVideo::ReloadImagesToSheet(TexSheet *sheet)
{
	// delete images
	map<FileName, Image *>::iterator iImage     = _images.begin();
	map<FileName, Image *>::iterator iImageEnd  = _images.end();

	bool success = true;
	while(iImage != iImageEnd)
	{
		Image *i = iImage->second;
		if(i->texSheet == sheet)
		{
			ILuint pixelData;
			uint w, h;
			
			if(!LoadRawPixelData(i->filename, pixelData, w, h))
			{
				if(VIDEO_DEBUG)
					cerr << "VIDEO ERROR: LoadRawPixelData() failed in ReloadImagesToSheet()!" << endl;
				success = false;
			}			
			
			if(!sheet->CopyRect(pixelData, i->x, i->y, w, h))
			{
				if(VIDEO_DEBUG)
					cerr << "VIDEO ERROR: sheet->CopyRect() failed in ReloadImagesToSheet()!" << endl;
				success = false;
			}
		}
		++iImage;
	}
	
	return success;
}


}  // namespace hoa_video
