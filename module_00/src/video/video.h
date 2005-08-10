/* 
	Video.h
	
 	Hero of Allacrost video class code.
 	
 	Documentation at:
 	http://www.allacrost.org/staff/user/roos/video.html
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
// Functions
//-----------------------------------------------------------------------------

float Lerp(float alpha, float initial, float final);
float RandomFloat(float a, float b);


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------


const float VIDEO_PI  = 3.141592653f;
const float VIDEO_2PI = 6.283185307f;

// draw flags
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


// shake falloff: when you call ShakeScreen(), this controls how quickly the
//                shaking dies down
enum ShakeFalloff
{
	VIDEO_FALLOFF_INVALID = -1,
	
	VIDEO_FALLOFF_NONE,     // shake remains at constant force
	VIDEO_FALLOFF_EASE,     // shake starts out small, builds up, then dies down
	VIDEO_FALLOFF_LINEAR,   // shake strength decreases linear til the end
	VIDEO_FALLOFF_GRADUAL,  // shake decreases slowly and drops off at the end
	VIDEO_FALLOFF_SUDDEN,   // shake suddenly falls off, for "impacts" like meteors
	
	VIDEO_FALLOFF_TOTAL
};


// interpolation methods: given two numbers A and B, these define ways to 
//                        interpolate values between them.
enum InterpolationMethod
{
	VIDEO_INTERPOLATE_INVALID = -1,
	
	VIDEO_INTERPOLATE_EASE,   // rise from A to B and then down to A again
	VIDEO_INTERPOLATE_SRCA,   // constant value of A
	VIDEO_INTERPOLATE_SRCB,   // constant value of B
	VIDEO_INTERPOLATE_FAST,   // rises quickly at the beginning and levels out
	VIDEO_INTERPOLATE_SLOW,   // rises slowly at the beginning then shoots up
	VIDEO_INTERPOLATE_LINEAR, // simple linear interpolation between A and B
	
	VIDEO_INTERPOLATE_TOTAL
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
	
	bool CopyRect(ILuint pixelData, int x, int y, int w, int h);
	
	bool RemoveImage (Image *img);  // removes an image completely
	bool FreeImage   (Image *img);  // marks the image as free
	bool RestoreImage (Image *img); // marks a previously freed image as "used"
	
	bool Unload();  // unloads texture memory used by this sheet
	bool Reload();  // reloads all the images into the sheet

	int width;
	int height;

	bool isStatic;       // if true, images in this sheet that are unlikely to change
	TexSheetType type;   // does it hold 32x32, 32x64, 64x64, or any kind

	TexMemMgr *_texMemManager;  // manages which areas of the texture are free

	GLuint texID;     // number OpenGL uses to refer to this texture
	bool loaded;
		
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
// Interpolator: class that lets you set up various kinds of interpolations
//               The basic way you use it is that you set the interpolator's
//               method with SetMethod(), and call Start() with the values to 
//               interpolate, and the amount of time to do it in.
//-----------------------------------------------------------------------------

class Interpolator
{
public:

	Interpolator();

	// call to begin an interpolation
	bool Start(float a, float b, int milliseconds);

	// set the method of the interpolator. If you don't call it, the default
	// is VIDEO_INTERPOLATION_LINEAR
	bool  SetMethod(InterpolationMethod method);
	
	float GetValue();              // get the current value
	bool  Update(int frameTime);   // call this every frame
	bool  IsFinished();            // returns true if interpolation is done

private:

	float FastTransform(float t);
	float SlowTransform(float t);
	float EaseTransform(float t);

	bool ValidMethod();
	
	InterpolationMethod _method;
	
	float _a, _b;
	int   _currentTime;
	int   _endTime;
	bool  _finished;
	float _currentValue;
};


//-----------------------------------------------------------------------------
// ShakeForce: every time ShakeScreen() is called, a new ShakeForce is created
//             to represent the force of that particular shake
//-----------------------------------------------------------------------------

struct ShakeForce
{
	float initialForce;  // initial force of the shake
	
	
	Interpolator interpolator;
	int   currentTime;   // milliseconds that passed since this shake started
	int   endTime;       // milliseconds that this shake was set to last for
};


//-----------------------------------------------------------------------------
// ScreenFader: class for fading the screen
//-----------------------------------------------------------------------------

class ScreenFader
{
public:
	
	ScreenFader()
	: _currentColor(0.0f, 0.0f, 0.0f, 0.0f),
	  _isFading(false)
	{
		_currentTime = _endTime = 0;
		_fadeModulation = 1.0f;
		_useFadeOverlay = false;
	}
	
	bool FadeTo(const Color &final, float numSeconds);
	bool Update(int t);

	// fades are either implemented with overlays or with modulation, depending
	// on whether it's a simple fade to black or a fade to a different color.
	// Based on that, these functions tell what overlay and modulation factors
	// to use.
	bool  ShouldUseFadeOverlay() { return _useFadeOverlay;   }
	Color GetFadeOverlayColor()  { return _fadeOverlayColor; }
	float GetFadeModulation()    { return _fadeModulation;   }

	bool  IsFading() { return _isFading; }

private:
	
	
	Color _currentColor;  // color the screen is currently faded to	
	Color _initialColor;  // color we started from
	Color _finalColor;    // color we are fading to
	int   _currentTime;   // milliseconds that passed since this fade started
	int   _endTime;       // milliseconds that this fade was set to last for
	bool  _isFading;      // true if we're in the middle of a fade
	
	bool  _useFadeOverlay;
	Color _fadeOverlayColor;
	float _fadeModulation;
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
	
	//-- General --------------------------------------------------------------

	bool Initialize();               // call this once at beginning of app
	bool Clear();                    // call at beginning of every frame
	bool Display(int frameTime);     // call at end of every frame
	

	//-- Video settings -------------------------------------------------------
	
	// NOTE: when you modify video setting, you must call ApplySettings()
	//       to actually apply them
	
	bool SetResolution(int width, int height);	
	bool IsFullscreen();
	bool SetFullscreen(bool fullscreen);
	bool ToggleFullscreen();
	
	bool ApplySettings();


	//-- Coordinate systems ---------------------------------------------------
		
	void SetViewport     // set the viewport, i.e. the area of the screen that
	(                    // gets drawn to, in terms of percentages
		float left,      // e.g. (0,100,0,100) <--default
		float right,
		float bottom,
		float top
	);
	                  	                  
	void SetCoordSys     // set the coordinate system
	(                    // e.g. (0,1024,0,768)
		float left,
		float right,
		float bottom,
		float top
	);

	
	//-- Transformations ------------------------------------------------------

	void PushState();    // save the current draw position on a stack
	void PopState ();    // restore the last draw position from the stack
	
	void Move   (float x, float y);    // set the draw position to (x, y)
	void MoveRel(float dx, float dy);  // move the draw position (dx, dy) units
	void Rotate (float angle);         // rotation for images (counter clockwise)


	//-- Text -----------------------------------------------------------------

	bool LoadFont     // load a font from a TTF file
	(
		const local_video::FileName &TTF_filename, 
		const std::string &name, 
		int size
	);
	
	bool SetFont      (const std::string &name);  // set current font
	bool SetTextColor (const Color &color);       // set current text color
	
	std::string GetFont      () const;            // get name of current font
	Color       GetTextColor () const;            // get current text color
	
	// NON-UNICODE version of DrawText(), only use this for debug output
	bool DrawText(const char *const text, float x, float y);
	
	// Unicode version of DrawText(), this should be used for any text which
	// might need to be localized (game dialogue, interface text, etc.)
	bool DrawText(const Uint16 *const text, float x, float y);


	//-- Images ----------------------------------------------------------------

	bool LoadImage(ImageDescriptor &id);    // load an image
	bool DeleteImage(ImageDescriptor &id);  // decrease ref count of image!
	
	bool BeginImageLoadBatch();   // if you are making many LoadImage() calls,
	bool EndImageLoadBatch();     // wrap that block of code with these 2 functions

	bool UnloadTextures();        // used when changing video modes
	bool ReloadTextures();

	void SetDrawFlags(int firstflag, ...);      // set draw flags (flip, align, etc.)
	
	bool DrawImage(const ImageDescriptor &id);  // draw image at current draw position

	void DEBUG_NextTexSheet();    // These cycle through the currently loaded
	void DEBUG_PrevTexSheet();    // texture sheets so they can be viewed on screen
	
	ImageDescriptor TilesToObject // converts a 2d array of tiles into one big image
	( 
		std::vector<ImageDescriptor> &tiles, 
		std::vector< std::vector<uint> > indices 
	);
	
	//-- Menus -----------------------------------------------------------------

	bool SetMenuSkin       // sets the current menu skin (borders + fill color)
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

	
	// create an imagedescriptor of a menu which is the given size
	// width and height are based on pixels, in 1024x768 resolution		
	bool CreateMenu(ImageDescriptor &id, float width, float height);
	
	
	//-- Fading ---------------------------------------------------------------

	// fade screen to given color in fadeTime number of seconds
	bool FadeScreen(const Color &color, float fadeTime);
	
	// returns true if a fade is currently being performed
	bool IsFading();
	

	//-- Screen shaking -------------------------------------------------------
	
	// shake the screen with a given force that lasts for falloffTime seconds
	// for a shake that keeps going until you stop it, pass 0.0f for falloffTime
	bool ShakeScreen
	(
		float force, 
		float falloffTime, 
		ShakeFalloff falloff = VIDEO_FALLOFF_NONE
	);	
	bool StopShaking(); // stop all shake effects
	bool IsShaking();   // returns true if any screen shaking is happening
	

	//-- Miscellaneous --------------------------------------------------------

	bool DrawFPS(int frameTime);
	bool MakeScreenshot();
	
	bool ToggleAdvancedDisplay();

private:
	SINGLETON_DECLARE(GameVideo);
	
	// for now the game gui class is a member of video so that
	// externally people only have to deal with GameVideo.
	local_video::GUI *_gui;
	
	char _blend;
	char _xalign;
	char _yalign;
	char _xflip;
	char _yflip;

	CoordSys    _coordSys;
	local_video::ScreenFader _fader;
	
	bool _advancedDisplay;
	
	int  _currentDebugTexSheet;
	int  _numTexSwitches;
	bool _batching;

	float _shakeX, _shakeY;   // offsets to shake the screen by (if any)
	std::list<local_video::ShakeForce> _shakeForces;
	
	bool _fullscreen;
	int  _width;
	int  _height;
	
	bool _temp_fullscreen;    // changing the video settings does not actually do anything until
	int  _temp_width;         // you call ApplySettings(). Up til that point, store them in temp
	int  _temp_height;        // variables so if the new settings are invalid, we can roll back.
	
	
	// requests new texture from OpenGL of given width, height. returns 0xffffffff on failure
	GLuint CreateBlankGLTexture(int width, int height);

	GLuint _lastTexID;

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
		
	float Lerp(float alpha, float initial, float final);
	
	bool LoadImageImmediate(ImageDescriptor &id, bool isStatic);

	// use DevIL to load an image and return the raw image data
	bool LoadRawPixelData(const std::string &filename, ILuint &pixelData, uint &width, uint &height);

	// loop through all currently loaded images and if they belong to the given tex sheet,
	// reload them into it
	bool ReloadImagesToSheet(local_video::TexSheet *);

	bool BindTexture(GLuint texID);
	bool DeleteTexture(GLuint texID);

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

	float RoundForce(float force);   // rounds a force value
	void UpdateShake(int frameTime); // this must be called every frame internally to update shake effects

	bool DEBUG_ShowTexSwitches();
	bool DEBUG_ShowTexSheet();
	
	friend class local_video::FixedTexMemMgr;
	friend class local_video::VariableTexMemMgr;
	friend class local_video::TexSheet;
};

}   // namespace hoa_video

#endif // !_VIDEO_HEADER_
