///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2005 by The Allacrost Project
//                       All Rights Reserved
//
// This code is licensed under the GNU GPL. It is free software and you may
// modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
/////////////////////////////////////////////////////////////////////////////// 

/*!****************************************************************************
 * \file    video.h
 * \author  Raj Sharma, rajx30@gmail.com
 * \date    Last Updated: August 15th, 2005
 * \brief   Header file for video engine interface.
 *
 * This code provides an easy-to-use API for managing all video and GUI stuff.
 *
 * \note This code uses the SDL_ttf 2.0 extension library. The documentation
 *       for it may be found here: http://jcatki.no-ip.org/SDL_ttf
 *
 * \note This code uses the DevIL library, v.1.6.7. This library can be found
 *       at http://openil.sourceforge.net
 *
 * \note Full documentation for video engine can be found at:
 *       http://www.allacrost.org/staff/user/roos/video.html
 *****************************************************************************/ 

#ifndef _VIDEO_HEADER_
#define _VIDEO_HEADER_

#include "utils.h"
#include "coord_sys.h"
#include "color.h"
#include <list>
 
//! All calls to the video engine are wrapped in this namespace.
namespace hoa_video 
{

//! Determines whether the code in the hoa_video namespace should print
extern bool VIDEO_DEBUG;

/*!
 *  \brief Linearly interpolates between initial and final value.
 *
 *  \param alpha   From 0.0 to 1.0, how much to interpolate between initial and final
 *  \param initial Initial value
 *  \param final   Final value
 */
 
float Lerp(float alpha, float initial, float final);

/*!
 *  \brief Creates a random float between a and b.
 */

float RandomFloat(float a, float b);

//! pi
const float VIDEO_PI  = 3.141592653f;

//! 2 * pi
const float VIDEO_2PI = 6.283185307f;

//! Draw flags which control x,y alignment, flipping, and blending
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


/*!***************************************************************************
 *  \brief An enumeration of shake falloff modes.which controls how quickly the
 *         shaking dies down when you do a screen shake.
 *****************************************************************************/

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

/*!***************************************************************************
 *  \brief Interpolation metods are various ways to create smoothed values
 *         between two numbers, e.g. linear interpolation
 *****************************************************************************/

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


// forward declarations
class ImageDescriptor;
class GameVideo;

//! local_video namespace, hides things which are used internally
namespace local_video
{

typedef std::string FileName;

class GUI;
class TexSheet;
class TexMemMgr;
class FixedTexMemMgr;
class VariableTexMemMgr;

struct Image;

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
 *  \brief Represents a single image within an image descriptor. Compound
 *         images are formed of multiple ImageElements.
 *****************************************************************************/

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


/*!***************************************************************************
 *  \brief represents a single image. Internally it's a reference to a
 *         sub-rect within a texture sheet.
 *****************************************************************************/

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



	TexSheet *texSheet;   //! texture sheet using this image
	FileName filename;    //! stored for every image in case it needs to be reloaded
	
	int x, y;             //! location of image within the sheet
	int width, height;    //! width and height, in pixels

	float u1, v1;         //! also store the actual uv coords. This is a bit
	float u2, v2;         //! redundant, but saves floating point calculations
	
	int refCount;         //! keep track of when this image can be deleted
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

	TexSheet(int w, int h, GLuint texID_, TexSheetType type_, bool isStatic_);
	~TexSheet();

	bool AddImage                   //! adds new image to the tex sheet
	(
		Image *img, 
		ILuint pixelData
	);
	
	bool CopyRect(ILuint pixelData, int x, int y, int w, int h);
	
	bool RemoveImage (Image *img);  //! removes an image completely
	bool FreeImage   (Image *img);  //! marks the image as free
	bool RestoreImage (Image *img); //! marks a previously freed image as "used"
	
	bool Unload();  //! unloads texture memory used by this sheet
	bool Reload();  //! reloads all the images into the sheet

	int width;
	int height;

	bool isStatic;       //! if true, images in this sheet that are unlikely to change
	TexSheetType type;   //! does it hold 32x32, 32x64, 64x64, or any kind

	TexMemMgr *_texMemManager;  //! manages which areas of the texture are free

	GLuint texID;     //! number OpenGL uses to refer to this texture
	bool loaded;
		
	friend class FixedTexMemMgr;
	friend class VariableTexMemMgr;
	friend class GameVideo;
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
 *  \brief used by the fixed-size texture manager to keep track of which blocks
 *         are owned by which images.
 *
 *  \note  The list is doubly linked to allow for O(1) removal
 *****************************************************************************/

struct FixedImageNode
{
	Image          *image;
	FixedImageNode *next;
	FixedImageNode *prev;
	
	int blockIndex;
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
	FixedTexMemMgr(TexSheet *texSheet, int imgW, int imgH);
	~FixedTexMemMgr();
	
	bool Insert  (Image *img);
	bool Remove  (Image *img);
	bool Free    (Image *img);
	bool Restore (Image *img);

private:

	int CalculateBlockIndex(Image *img);
	void DeleteNode(int blockIndex);
	
	//! store dimensions of both the texture sheet, and the images that
	//! it contains
	
	//! NOTE: the sheet dimensions are not in pixels, but images.
	//!       So, a 512x512 sheet holding 32x32 images would be 16x16
	
	int _sheetWidth;
	int _sheetHeight;
	int _imageWidth;
	int _imageHeight;
	
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


/*!***************************************************************************
 *  \brief class that lets you set up various kinds of interpolations.
 *         The basic way to use it is to set the interpolation method using
 *         SetMethod(), then call Start() with the values you want to
 *         interpolate between and the time to do it in.
 *****************************************************************************/
 
class Interpolator
{
public:

	Interpolator();

	//! call to begin an interpolation
	bool Start(float a, float b, int milliseconds);

	//! set the method of the interpolator. If you don't call it, the default
	//! is VIDEO_INTERPOLATION_LINEAR
	bool  SetMethod(InterpolationMethod method);
	
	float GetValue();              //! get the current value
	bool  Update(int frameTime);   //! call this every frame
	bool  IsFinished();            //! returns true if interpolation is done

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


/*!***************************************************************************
 *  \brief every time ShakeScreen() is called, a new ShakeForce is created
 *         to represent the force of that particular shake
 *****************************************************************************/

struct ShakeForce
{
	float initialForce;  //! initial force of the shake
	
	
	Interpolator interpolator;
	int   currentTime;   //! milliseconds that passed since this shake started
	int   endTime;       //! milliseconds that this shake was set to last for
};


/*!***************************************************************************
 *  \brief class for fading the screen. Keeps track of current color and
 *         figures out whether we should implement the fade using modulation
 *         or an overlay.
 *****************************************************************************/

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

	//! fades are either implemented with overlays or with modulation, depending
	//! on whether it's a simple fade to black or a fade to a different color.
	//! Based on that, these functions tell what overlay and modulation factors
	//! to use.
	bool  ShouldUseFadeOverlay() { return _useFadeOverlay;   }
	Color GetFadeOverlayColor()  { return _fadeOverlayColor; }
	float GetFadeModulation()    { return _fadeModulation;   }

	bool  IsFading() { return _isFading; }

private:
	
	
	Color _currentColor;  //! color the screen is currently faded to	
	Color _initialColor;  //! color we started from
	Color _finalColor;    //! color we are fading to
	int   _currentTime;   //! milliseconds that passed since this fade started
	int   _endTime;       //! milliseconds that this fade was set to last for
	bool  _isFading;      //! true if we're in the middle of a fade
	
	bool  _useFadeOverlay;
	Color _fadeOverlayColor;
	float _fadeModulation;
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
	
	int _sheetWidth;
	int _sheetHeight;
};


} // namespace local_video


/*!***************************************************************************
 *  \brief this is the class that external modules deal with when loading and
 *         drawing images.
 *
 *  \note  ImageDescriptors may be simple images or compound images. Compound
 *         images are when you stitch together multiple small images to create
 *         a large image, e.g. with TilesToObject(). Externally though,
 *         it's fine to think of compound images as just a single image.
 *****************************************************************************/

class ImageDescriptor 
{
public:

	ImageDescriptor() 
	: width(0.0f), height(0.0f), color(1.0f, 1.0f, 1.0f, 1.0f)
	{
		isStatic = false;
	}
	
	//! AddImage allows you to create compound images. You start with a 
	//! newly created ImageDescriptor, then call AddImage(), passing in
	//! all the images you want to add, along with the x, y offsets they
	//! should be positioned at.
	
	bool AddImage(const ImageDescriptor &id, float xOffset, float yOffset);
	
	void Clear()
	{
		filename.clear();
		isStatic = false;
		width = height = 0.0f;
		color = Color(1.0f, 1.0f, 1.0f, 1.0f);
		_elements.clear();
	}
	
	
	local_video::FileName filename;  //! used only as a parameter to LoadImage.

	bool  isStatic;       //! used only as a parameter to LoadImage. This tells
	                      //! whether the image being loaded is to be loaded
	                      //! into a non-volatile area of texture memory
	                      
	Color color;          //! used only as a parameter to LoadImage


	float width, height;  //! width and height of image, in pixels.
	                      //! If the ImageDescriptor is a compound, i.e. it
	                      //! contains multiple images, then the width and height
	                      //! refer to the entire compound           

private:

	//! an image descriptor represents a compound image, which is made
	//! up of multiple elements
	std::vector <local_video::ImageElement> _elements;

	friend class GameVideo;
};


/*!***************************************************************************
 *  \brief main class for all video and GUI drawing
 *****************************************************************************/

class GameVideo
{
public:
	
	SINGLETON_METHODS(GameVideo);
	
	//-- General --------------------------------------------------------------

	/*!
	 *  \brief call this once at beginning of app
	 */
	bool Initialize();

	/*!
	 *  \brief call at beginning of every frame
	 */
	bool Clear();
		
	/*!
	 *  \brief call at end of every frame
	 *
	 *  \param frameTime   milliseconds since the last frame
	 */
	bool Display(int frameTime);
	

	//-- Video settings -------------------------------------------------------
	
	//! NOTE: when you modify video setting, you must call ApplySettings()
	//!       to actually apply them

	/*!
	 *  \brief sets the current resolution to the given width and height
	 *
	 *  \note  you must call ApplySettings() to actually apply the change
	 */	
	bool SetResolution(int width, int height);	

	/*!
	 *  \brief returns true if game is in fullscreen mode
	 */	
	bool IsFullscreen();
	
	/*!
	 *  \brief sets the game to fullscreen or windowed depending on whether
	 *         true or false is passed
	 *
	 *  \note  you must call ApplySettings() to actually apply the change
	 */	
	bool SetFullscreen(bool fullscreen);
	
	/*!
	 *  \brief toggles fullscreen on and off
	 *
	 *  \note  you must call ApplySettings() to actually apply the change
	 */	
	bool ToggleFullscreen();
	
	/*!
	 *  \brief applies any changes to video settings like resolution and
	 *         fullscreen. If the changes fail, then this function returns
	 *         false, and the video settings are reset to whatever the last
	 *         working setting was.
	 */	
	bool ApplySettings();


	//-- Coordinate systems ---------------------------------------------------
	
	/*!
	 *  \brief sets the viewport, i.e. the area of the screen that gets drawn
	 *         to. The default is (0, 100, 0, 100).
	 *
	 *  \note  The arguments are percentages (0-100)
	 */
	void SetViewport
	(
		float left,
		float right,
		float bottom,
		float top
	);

	/*!
	 *  \brief sets the coordinate system. Default is (0,1024,0,768)
	 */   	                  
	void SetCoordSys
	(
		float left,
		float right,
		float bottom,
		float top
	);

	
	//-- Transformations ------------------------------------------------------

	/*!
	 *  \brief saves current draw position on the stack
	 */
	void PushState();
	
	/*!
	 *  \brief restores current draw position from stack
	 */
	void PopState ();
	
	/*!
	 *  \brief sets draw position to (x,y)
	 */
	void Move   (float x, float y);
	
	/*!
	 *  \brief moves draw position (dx, dy) units
	 */
	void MoveRel(float dx, float dy);
	
	/*!
	 *  \brief rotates images counterclockwise by 'angle' radians
	 */
	void Rotate (float angle);

	/*!
	 *  \brief sets OpenGL transform to contents of 4x4 matrix (16 values)
	 */
	void SetTransform(float m[16]);

	//-- Text -----------------------------------------------------------------

	/*!
	 *  \brief loads a font from a TTF file
	 */
	bool LoadFont
	(
		const local_video::FileName &TTF_filename, 
		const std::string &name, 
		int size
	);
	
	/*!
	 *  \brief sets current font
	 */
	bool SetFont      (const std::string &name);
	
	/*!
	 *  \brief sets current text color
	 */
	bool SetTextColor (const Color &color);
		
	/*!
	 *  \brief get name of current font
	 */
	std::string GetFont      () const;
	
	/*!
	 *  \brief get current text color
	 */
	Color       GetTextColor () const;
	
	/*!
	 *  \brief non-unicode version of DrawText(). Only use this for debug
	 *         output or other things which don't have to be localized
	 *
	 *  \param P1      Explanation
	 */
	bool DrawText(const char *const text, float x, float y);
	
	/*!
	 *  \brief unicode version of DrawText(). This should be used for
	 *         anything in the game which might need to be localized
	 *         (game dialogue, interface text, etc.)
	 */
	bool DrawText(const Uint16 *const text, float x, float y);


	//-- Images ----------------------------------------------------------------
	
	/*!
	 *  \brief loads an image	 
	 */
	bool LoadImage(ImageDescriptor &id);

	/*!
	 *  \brief captures the contents of the screen and saves it to an image
	 *         descriptor
	 */
	bool CaptureScreen(ImageDescriptor &id);

	
	/*!
	 *  \brief decreases ref count of an image
	 */
	bool DeleteImage(ImageDescriptor &id);
	
	/*!
	 *  \brief marks the start of an image loading batch. Use for loading
	 *         map tiles for instance, where many images are being loaded at
	 *         once
	 */
	bool BeginImageLoadBatch();
	
	/*!
	 *  \brief marks the end of image loading batch.
	 */
	bool EndImageLoadBatch();

	/*!
	 *  \brief unloads all texture sheets from memory when we lose the GL
	 *         context, so textures can be properly reloaded
	 */
	bool UnloadTextures();
	
	/*!
	 *  \brief reloads textures that have been unloaded after a video
	 *         settings change
	 */
	bool ReloadTextures();

	/*!
	 *  \brief sets draw flags (flip, align, blending, etc). Simply pass
	 *         in as many parameters as you want, as long as the last
	 *         parameter is a zero.
	 *         e.g. SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_CENTER, 0);
	 */
	void SetDrawFlags(int firstflag, ...);
	
	/*!
	 *  \brief draws an image which is modulated by the scene's light color
	 */	
	bool DrawImage(const ImageDescriptor &id);  
	
	/*!
	 *  \brief draws an iamge which is modulated by a custom color (not the
	 *         scene lighting)
	 */
	bool DrawImage(const ImageDescriptor &id, const Color &color);  

	/*!
	 *  \brief converts a 2D array of tiles into one big image
	 *
	 *  \param tiles   a vector of image descriptors (the tiles)
	 *  \param indices a 2D vector in row-column order (e.g. indices[y][x])
	 *         which forms a rectangular array of tiles
	 */
	ImageDescriptor TilesToObject
	( 
		std::vector<ImageDescriptor> &tiles, 
		std::vector< std::vector<uint> > indices 
	);

	void DEBUG_NextTexSheet();    // These cycle through the currently loaded
	void DEBUG_PrevTexSheet();    // texture sheets so they can be viewed on screen
	
	
	//-- Menus -----------------------------------------------------------------

	/*!
	 *  \brief sets the current menu skin (borders+fill color).
	 *
	 *  \param imgFile_UL    image file for upper-left border
	 *  \param imgFile_U     image file for upper border
	 *  \param imgFile_UR    image file for upper-right border
	 *  \param imgFile_L     image file for left border
	 *  \param imgFile_R     image file for right border
	 *  \param imgFile_BL    image file for bottom-left border
	 *  \param imgFile_B     image file for bottom border
	 *  \param imgFile_BR    image file for bottom-right border
	 *  \param fillColor     color for inner area of menu
	 */
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
	
	/*!
	 *  \brief creates an ImageDescriptor of a menu which is the given size
	 *
	 *  \param width  Width of menu, based on pixels in 1024x768 resolution
	 *  \param height Height of menu, based on pixels in 1024x768 resolution.
	 */
	bool CreateMenu(ImageDescriptor &id, float width, float height);


	//-- Lighting and fog -----------------------------------------------------
	
	/*!
	 *  \brief sets the light color for the scene
	 */
	bool SetLighting (const Color &color);

	/*!
	 *  \brief sets fog parameters
	 *
	 *  \param color  Color of the fog (alpha should be 1.0)
	 *  \param intensity  Intensity of fog from 0.0f to 1.0f
	 */	
	bool SetFog      (const Color &color, float intensity);
	
	/*!
	 *  \brief draws a halo at the given spot
	 *
	 *  \param id    image descriptor for the halo image
	 *  \param x     x coordinate of halo
	 *  \param y     y coordinate of halo
	 *  \param color color of halo
	 */
	bool DrawHalo
	(
		const ImageDescriptor &id, 
		float x, 
		float y, 
		const Color &color = Color(1.0f, 1.0f, 1.0f, 1.0f)
	);

	/*!
	 *  \brief draws a real light at the given spot
	 *
	 *  \param id    image descriptor for the light mask
	 *  \param x     x coordinate of light
	 *  \param y     y coordinate of light
	 *  \param color color of light
	 */	
	bool DrawLight
	(
		const ImageDescriptor &id, 
		float x, 
		float y, 
		const Color &color = Color(1.0f, 1.0f, 1.0f, 1.0f)
	);

	/*!
	 *  \brief call with true if this map uses real lights, otherwise call with
	 *         false
	 */	
	bool EnableRealLights(bool enable);
	

	/*!
	 *  \brief call after rendering all real lights
	 */	
	bool AccumulateLights();

	/*!
	 *  \brief call after all map images are drawn to apply lighting. All
	 *         menu and text rendering should occur AFTER this call, so that
	 *         they are not affected by lighting.
	 */	
	bool ApplyLightingOverlay();

	
	//-- Fading ---------------------------------------------------------------

	/*!
	 *  \brief fade screen to color in fadeTime number of seconds
	 */
	bool FadeScreen(const Color &color, float fadeTime);
	
	/*!
	 *  \brief returns true if screen fade is currently in progress
	 */
	bool IsFading();
	

	//-- Screen shaking -------------------------------------------------------
	
	/*!
	 *  \brief shakes the screen
	 *
	 *  \param force        Initial force of the shake
	 *  \param falloffTime  How long the shake should last for. Pass 0.0f for
	 *                      a shake that doesn't end until you stop it manually
	 *  \param falloff      Specifies the method of falloff. The default is
	 *                      VIDEO_FALLOFF_NONE.
	 */
	bool ShakeScreen
	(
		float force, 
		float falloffTime, 
		ShakeFalloff falloff = VIDEO_FALLOFF_NONE
	);	

	/*!
	 *  \brief stops all shake effects
	 */
	bool StopShaking();
	
	/*!
	 *  \brief returns true if screen is shaking
	 */
	bool IsShaking();
	

	//-- Miscellaneous --------------------------------------------------------

	/*!
	 *  \brief updates the FPS counter with the given frame time and draws the
	 *         current FPS on the screen.
	 */
	bool DrawFPS(int frameTime);
	
	/*!
	 *  \brief makes a screenshot, saves it as screenshot.jpg in the directory
	 *         of the game
	 */
	bool MakeScreenshot();
	
	/*!
	 *  \brief toggles advanced information display for video engine, shows
	 *         things like number of texture switches per frame, etc.
	 */
	bool ToggleAdvancedDisplay();

private:
	SINGLETON_DECLARE(GameVideo);
	
	// for now the game gui class is a member of video so that
	// externally people only have to deal with GameVideo.
	local_video::GUI *_gui;

	// draw flags	
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
	int  _numDrawCalls;
	bool _batchLoading;
	
	bool   _usesLights;
	GLuint _lightOverlay;

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

	Color _fogColor;
	float _fogIntensity;
	
	Color _lightColor;

	std::vector <ImageDescriptor *>    _batchLoadImages;
	
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
