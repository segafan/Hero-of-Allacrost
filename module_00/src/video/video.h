/* 
	Video.h
	
 	Hero of Allacrost video class code.
 	
 	Here's a forum post with some rough documentation:
 	http://www.allacrost.org/forum/viewtopic.php?p=5295#5295
*/


#ifndef _VIDEO_HEADER_
#define _VIDEO_HEADER_

#include "utils.h"
#include "coord_sys.h"
#include "color.h"
 

namespace hoa_video 
{

extern bool VIDEO_DEBUG;

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

enum
{
	VIDEO_X_LEFT = 1,
	VIDEO_X_CENTER,
	VIDEO_X_RIGHT,
	VIDEO_Y_TOP,
	VIDEO_Y_CENTER,
	VIDEO_Y_BOTTOM,
	VIDEO_X_FLIP,
	VIDEO_X_NOFLIP,
	VIDEO_Y_FLIP,
	VIDEO_Y_NOFLIP,
	VIDEO_NO_BLEND,
	VIDEO_BLEND,
	VIDEO_BLEND_ADD
};

class ImageDescriptor;
class GameVideo;

namespace local_video
{

typedef std::string FileName;

class GUI;
class TexSheet;
class TexMemMgr;
class FixedTexMemMgr;
class VariableTexMemMgr;

struct Image;

//-----------------------------------------------------------------------------
// TexSheet_Type: what size of images this texture sheet is allowed to hold
//-----------------------------------------------------------------------------

enum TexSheetType
{
	VIDEO_TEXSHEET_INVALID = -1,
	
	VIDEO_TEXSHEET_32x32,
	VIDEO_TEXSHEET_32x64,
	VIDEO_TEXSHEET_64x64,
	VIDEO_TEXSHEET_ANY,
	
	VIDEO_TEXSHEET_TOTAL
};


//-----------------------------------------------------------------------------
// ImageElement: multiple image elements make up a compound image, which is
//               what an image descriptor represents.
//-----------------------------------------------------------------------------

struct ImageElement
{
	ImageElement
	(
		Image *image_, 
		float xOffset_, 
		float yOffset_, 
		float width_, 
		float height_,
		Color color_
	)
	{
		image   = image_;
		xOffset = xOffset_;
		yOffset = yOffset_;
		width   = width_;
		height  = height_;
		color   = color_;
	}
	
	Image * image;

	float xOffset;
	float yOffset;
	
	float width;
	float height;
	
	Color color;
};


//-----------------------------------------------------------------------------
// Image: represents a single image. Internally what it does is contains a
//        reference to a sub-rectangle of a texture sheet which holds many images.
//
// NOTE: it is a struct, and that's fine because Image is never directly exposed
//       outside of the video engine
//-----------------------------------------------------------------------------

struct Image
{
	Image
	(
		const FileName &fname,
		int w, 
		int h
	) 
	: filename(fname)
	{ 
		width    = w;
		height   = h;
		refCount = 0;
	}

	Image
	(
		TexSheet *sheet,
		const FileName &fname,
		int x_,
		int y_,
		int w, 
		int h,
		float u1_,
		float v1_,
		float u2_,
		float v2_
	) 
	: filename(fname)
	{
		texSheet = sheet;
		x = x_;
		y = y_;
		width = w;
		height = h;
		u1 = u1_;
		u2 = u2_;
		v1 = v1_;
		v2 = v2_;
		refCount = 0;
	}



	TexSheet *texSheet;   // texture sheet using this image
	FileName filename;    // stored for every image in case it needs to be reloaded
	
	int x, y;             // location of image within the sheet
	int width, height;    // width and height, in pixels

	float u1, v1;         // also store the actual uv coords. This is a bit
	float u2, v2;         // redundant, but saves floating point calculations
	
	int refCount;         // keep track of when this image can be deleted
};


//-----------------------------------------------------------------------------
// TexSheet: an actual OpenGL texture, which might be used for storing
//           multiple smaller images in it, to save on switching textures
//           during rendering.
//
// NOTE: I named this TexSheet instead of Texture, so it's clear that this
//       doesn't represent one image, but rather just a collection of images
//       which are placed into one texture.
//-----------------------------------------------------------------------------

class TexSheet
{
public:

	TexSheet(int w, int h, GLuint texID_, TexSheetType type_, bool isStatic_);
	~TexSheet();

	bool AddImage                   // adds new image to the tex sheet
	(
		Image *img, 
		ILuint pixelData
	);  
	
	bool RemoveImage (Image *img);  // removes an image completely
	bool FreeImage   (Image *img);  // marks the image as free
	bool RestoreImage (Image *img); // marks a previously freed image as "used"

	int width;
	int height;

	bool isStatic;       // if true, images in this sheet that are unlikely to change
	TexSheetType type;   // does it hold 32x32, 32x64, 64x64, or any kind

	TexMemMgr *_texMemManager;  // manages which areas of the texture are free

	GLuint texID;     // number OpenGL uses to refer to this texture
	
	friend class FixedTexMemMgr;
	friend class VariableTexMemMgr;
	friend class GameVideo;
};


//-----------------------------------------------------------------------------
// TexMemMgr: base class for texture memory manager. It is used by TextureSheet
//            to manage which areas of the texture are free and which are used.
//-----------------------------------------------------------------------------

class TexMemMgr
{
public:
	
	virtual ~TexMemMgr() {}

	virtual bool Insert  (Image *img)=0;
	virtual bool Remove  (Image *img)=0;
	virtual bool Free    (Image *img)=0;
	virtual bool Restore (Image *img)=0;
	
};


//-----------------------------------------------------------------------------
// FixedImageNode: used by the fixed-size texture manager to keep track of which
//                 blocks are owned by which images
//                 The list is doubly linked to allow for O(1) removal
//-----------------------------------------------------------------------------

struct FixedImageNode
{
	Image          *image;
	FixedImageNode *next;
	FixedImageNode *prev;
	
	int blockIndex;
};


//-----------------------------------------------------------------------------
// FixedTexMemMgr: used to manage textures which are designated for one specific
//                 image size. For example, we could create a TextureSheet
//                 which is 512x512 and holds ONLY 32x32 tiles.
//
// NOTE: The texture sheet's size must be divisible by the size of the images
//       it holds. For example, you can't create a 256x256 sheet which manages
//       tiles which are 17x93.
//-----------------------------------------------------------------------------

class FixedTexMemMgr : public TexMemMgr
{
public:
	FixedTexMemMgr(TexSheet *texSheet, int imgW, int imgH);
	~FixedTexMemMgr();
	
	bool Insert  (Image *img);
	bool Remove  (Image *img);
	bool Free    (Image *img);
	bool Restore (Image *img);

private:

	int CalculateBlockIndex(Image *img);
	void DeleteNode(int blockIndex);
	
	// store dimensions of both the texture sheet, and the images that
	// it contains
	
	// NOTE: the sheet dimensions are not in pixels, but images.
	//       So, a 512x512 sheet holding 32x32 images would be 16x16
	
	int _sheetWidth;
	int _sheetHeight;
	int _imageWidth;
	int _imageHeight;
	
	TexSheet *_texSheet;
	
	// The open list keeps track of which blocks of memory are
	// open. Note that we track blocks with BOTH an array and a list.
	// Although it takes up more memory, this makes ALL operations
	// dealing with the blocklist O(1) so performance is awesome.
	// Memory isn't too bad either, since blocklist is fairly small.
	// The tail pointer is also kept so that we can add newly
	// freed blocks to the end of the list- that way, essentially
	// blocks that are freed are given a little bit of time from the
	// time they're freed to the time they're removed, in case they
	// are loaded again in the near future
	
	FixedImageNode *_openListHead;
	FixedImageNode *_openListTail;
	
	// this is our actual array of blocks which is indexed like a 2D
	// array. For example, blocks[x+y*width]->image would tell us
	// which image is currently allocated at spot (x,y)
	
	FixedImageNode *_blocks;
};


//-----------------------------------------------------------------------------
// VariableImageNode: this is how we keep track of which images are used/freed
//                    in the variable texture mem manager.
//-----------------------------------------------------------------------------

struct VariableImageNode
{
	VariableImageNode()
	{
		image = NULL;
		free  = true;
	}

	Image *image;
	bool   free;
};


//-----------------------------------------------------------------------------
// VariableTexMemMgr: used to manage a texture sheet where the size of the
//                    images that it will contain are unknown.
//
// NOTE: for the sake of reducing the time it takes to allocate a texture,
//       this class treats images as if their dimensions are rounded up to
//       the nearest multiple of 16. So in English, what this means is that
//       a little space gets wasted if you allocate images whose dimensions
//       aren't multiples of 16. (Hopefully not enough to worry about)
//-----------------------------------------------------------------------------

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
	
	// Sheet's dimensions
	// NOTE: these aren't in pixels, but in "blocks" of 16x16. So,
	//       a 512x512 sheet would be 32x32 in blocks
	
	int _sheetWidth;
	int _sheetHeight;
};


} // namespace local_video


//-----------------------------------------------------------------------------
// ImageDescriptor: this is the class that external modules deal with when
//                  loading and drawing images.
//
// NOTE: As of August 2005, image descriptors can be composed of multiple 
//       smaller images stitched together, thus we store a vector of image handles
//       along with their offsets from the top
//-----------------------------------------------------------------------------

class ImageDescriptor 
{
public:

	ImageDescriptor() 
	: width(0.0f), height(0.0f), color(1.0f, 1.0f, 1.0f, 1.0f)
	{
		isStatic = false;
	}
	
	// AddImage allows you to create compound images. You start with a 
	// newly created ImageDescriptor, then call AddImage(), passing in
	// all the images you want to add, along with the x, y offsets they
	// should be positioned at.
	
	bool AddImage(const ImageDescriptor &id, float xOffset, float yOffset);
	
	void Clear()
	{
		filename.clear();
		isStatic = false;
		width = height = 0.0f;
		color = Color(1.0f, 1.0f, 1.0f, 1.0f);
		_elements.clear();
	}
	
	
	local_video::FileName filename;  // used only as a parameter to LoadImage.

	bool  isStatic;       // used only as a parameter to LoadImage. This tells
	                      // whether the image being loaded is to be loaded
	                      // into a non-volatile area of texture memory
	                      
	Color color;          // used only as a parameter to LoadImage


	float width, height;  // width and height of image, in pixels.
	                      // If the ImageDescriptor is a compound, i.e. it
	                      // contains multiple images, then the width and height
	                      // refer to the entire compound           

private:

	// an image descriptor represents a compound image, which is made
	// up of multiple elements
	std::vector <local_video::ImageElement> _elements;

	friend class GameVideo;
};


//-----------------------------------------------------------------------------
// GameVideo: main class for all rendering options
//-----------------------------------------------------------------------------

class GameVideo
{
public:
	
	SINGLETON_METHODS(GameVideo);
	
	// General

	bool Initialize();
	bool Clear();
	bool Display();


	// Coordinate systems
	
	bool ChangeMode  (const SDL_Rect &s);
	void SetViewport (float left, float top, float right, float bottom);
	
	void SetCoordSys (float left, float right, float top, float bottom, int layer);	

	
	// Transformations

	void PushState();
	void PopState ();
	
	void Move   (float tx, float ty);
	void MoveRel(float tx, float ty);
	void Rotate (float angle);


	// Fonts

	bool LoadFont
	(
		const local_video::FileName &filename, 
		const std::string &name, 
		int size
	);
	
	bool SetFont      (const std::string &name);
	bool SetTextColor (const Color &color);
	
	std::string GetFont      () const;
	Color       GetTextColor () const;
	
	bool DrawText(const char   *const text, float x, float y);
	bool DrawText(const Uint16 *const text, float x, float y);


	// Images

	bool LoadImage(ImageDescriptor &id);
	bool DeleteImage(ImageDescriptor &id);
	
	bool BeginImageLoadBatch();
	bool EndImageLoadBatch();

	bool UnloadImages();
	bool ReloadImages();
	bool DeleteImages();

	void SetDrawFlags(int firstflag, ...);
	bool DrawImage(const ImageDescriptor &id);

	void SelectLayer(int l);
	
	void DEBUG_NextTexSheet();
	void DEBUG_PrevTexSheet();
	
	ImageDescriptor TilesToObject
	( 
		std::vector<ImageDescriptor> &tiles, 
		std::vector< std::vector<uint> > indices 
	);
	
	// Menus


	bool SetMenuSkin
	(
		const std::string &imgFile_UL,  // image filenames for the borders
		const std::string &imgFile_U,
		const std::string &imgFile_UR,
		const std::string &imgFile_L,
		const std::string &imgFile_R,
		const std::string &imgFile_BL,
		const std::string &imgFile_B,
		const std::string &imgFile_BR,
		
		const Color  &fillColor         // color to fill the menu with. You can
		                                // make it transparent by setting alpha
	);

	
	// width and height are based on pixels, in 1024x768 resolution		
	bool CreateMenu(ImageDescriptor &id, float width, float height);
	
	
	// Miscellaneous
	
	bool DrawFPS(int frameTime);
	bool MakeScreenshot();

private:
	SINGLETON_DECLARE(GameVideo);
	
	// for now the game gui class is a member of video so that
	// externally people only have to deal with GameVideo.
	local_video::GUI *_gui;
	
	int  _width;
	int  _height;
	bool _setUp;
	char _blend;
	char _xalign;
	char _yalign;
	char _xflip;
	char _yflip;

	CoordSys _coordSys;
	
	int  _currentDebugTexSheet;
	bool _batching;

	std::string _currentFont;
	Color       _currentTextColor;

	std::vector <ImageDescriptor *>                            _batchImages;
	std::map    <local_video::FileName, local_video::Image*>   _images;
	std::vector <local_video::TexSheet *>                      _texSheets;
	std::map    <std::string, TTF_Font *>                      _fontMap;


	bool DrawTextHelper( 
		const char   *const text, 
		const Uint16 *const uText, 
		float x, 
		float y
	);

	local_video::TexSheet *CreateTexSheet
	(
		int width,
		int height,
		local_video::TexSheetType type,
		bool isStatic
	);

	local_video::TexSheet *InsertImageInTexSheet
	(
		local_video::Image *image,
		ILuint pixelData, 
		int &x, 
		int &y,
		int w,
		int h,
		bool isStatic
	);
		
	
	bool LoadImageImmediate(ImageDescriptor &id, bool isStatic);

	bool RemoveImage(local_video::Image *);     // removes image from std::map
	bool RemoveSheet(local_video::TexSheet *);  // removes sheet from std::vector

	bool DeleteImage    (local_video::Image    *const);
	bool DeleteTexSheet (local_video::TexSheet *const);
	bool DrawElement
	(
		const local_video::Image *const, 
		float w, 
		float h, 
		const Color &color
	);

	bool DEBUG_ShowTexSheet();
	
	friend class local_video::FixedTexMemMgr;
	friend class local_video::VariableTexMemMgr;
};

}

#endif // !_VIDEO_HEADER_
