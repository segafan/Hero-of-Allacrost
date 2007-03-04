///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    video.h
*** \author  Raj Sharma, roos@allacrost.org
***          Daniel Steuernol, steu@allacrost.org
*** \brief   Header file for video engine interface.
***
*** This code provides a comprehensive API for managing all drawing, rendering,
*** GUI, and other related graphical functions. This video engine is very large,
*** so the API user is advised to seek the documentation for it available on the
*** Allacrost wiki (http://wiki.allacrost.org).
***
*** In addition for its use in the game, the video engine is also actively used
*** by the Allacrost map editor GUI as a QT widget.
***
*** \note This code uses the OpenGL library for graphics rendering.
*** \note This code uses the libpng and libjpeg libraries for loading images.
*** \note This code uses the SDL_ttf 2.0 library for font rendering.
*** ***************************************************************************/

#ifndef __VIDEO_HEADER__
#define __VIDEO_HEADER__

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

// libpng and libjpeg image loader includes
#include <png.h>
extern "C" {
	#include <jpeglib.h>
}

// SDL_ttf includes
#ifdef __APPLE__
	#include <SDL_ttf/SDL_ttf.h>
#else
	#include <SDL/SDL_ttf.h>
#endif

// Various other headers of the video engine
#include "context.h"
#include "color.h"
#include "coord_sys.h"
#include "fade.h"
#include "image.h"
#include "interpolator.h"
#include "shake.h"
#include "screen_rect.h"
#include "tex_mgmt.h"
#include "text.h"
#include "particle_manager.h"
#include "particle_effect.h"
#include "gui.h"
#include "menu_window.h"
#include "option.h"
#include "textbox.h"

//! \brief All calls to the video engine are wrapped in this namespace.
namespace hoa_video {

//! \brief The singleton pointer for the engine, responsible for all video operations.
extern GameVideo *VideoManager;

//! \brief Determines whether the code in the hoa_video namespace should print
extern bool VIDEO_DEBUG;

/** \brief Creates a random float between a and b.
 * \param a lower bound of random selection
 * \param b upper bound of random selection
 * \return the randomly generated float
 */
float RandomFloat(float a, float b);


/** \brief Rotates a point (x,y) around the origin (0,0), by angle radians
 * \param x x coordinate of point to rotate
 * \param y y coordinate of point to rotate
 * \param angle amount to rotate by (in radians)
 */
void RotatePoint(float &x, float &y, float angle);


/** \brief linearly interpolation, returns a value which is (alpha*100) percent between initial and final
 *
 *  \param alpha    controls the linear interpolation
 *  \param initial  initial value
 *  \param final    final value
 * \return the linear interpolated value
 */
float Lerp(float alpha, float initial, float final);





//! animation frame period: how many milliseconds 1 frame of animation lasts for
const int32 VIDEO_ANIMATION_FRAME_PERIOD = 10;

//! \brief Draw flags to control x and y alignment, flipping, and texture blending.
enum VIDEO_DRAW_FLAGS {
	VIDEO_DRAW_FLAGS_INVALID = -1,

	//! X alignment flags
	VIDEO_X_LEFT = 1, VIDEO_X_CENTER = 2, VIDEO_X_RIGHT = 3,

	//! Y alignment flags
	VIDEO_Y_TOP = 4, VIDEO_Y_CENTER = 5, VIDEO_Y_BOTTOM = 6,

	//! X flip flags
	VIDEO_X_FLIP = 7, VIDEO_X_NOFLIP = 8,

	//! Y flip flags
	VIDEO_Y_FLIP = 9, VIDEO_Y_NOFLIP = 10,

	//! Texture blending flags
	VIDEO_NO_BLEND = 11, VIDEO_BLEND = 12, VIDEO_BLEND_ADD = 13,

	VIDEO_DRAW_FLAGS_TOTAL = 14
};


/** \brief This specifies whether the engine is to draw to an SDL window, a QT widget, etc.
*** \note The target must be set before the call to GameVideo::Initialize().
**/
enum VIDEO_TARGET {
	VIDEO_TARGET_INVALID = -1,

	//! Drawing to a SDL window
	VIDEO_TARGET_SDL_WINDOW = 0,

	//! Drawing to a QT widget
	VIDEO_TARGET_QT_WIDGET  = 1,

	VIDEO_TARGET_TOTAL = 2
};


//! \brief Specifies the stencil operation to use and describes how stencil buffer is modified.
enum VIDEO_STENCIL_OP {
	VIDEO_STENCIL_OP_INVALID = -1,

	//! Set the stencil value to one
	VIDEO_STENCIL_OP_ONE      = 0,

	//! Set the stencil value to zero
	VIDEO_STENCIL_OP_ZERO     = 1,

	//! Increase the stencil value
	VIDEO_STENCIL_OP_INCREASE = 2,

	//! Decrease the stencil value
	VIDEO_STENCIL_OP_DECREASE = 3,

	VIDEO_STENCIL_OP_TOTAL = 4
};


/** ****************************************************************************
*** \brief Manages all the video operations and serves as the API to the video engine.
***
*** This is one of the largest classes in the Allacrost engine. Because it is so
*** large, the implementation of many of the methods for this class are split up
*** into multiple .cpp source files in the video code directory.
*** *****************************************************************************/
class GameVideo {
	friend class TextBox;
	friend class OptionBox;
	friend class MenuWindow;
	friend class private_video::GUIElement;
	friend class private_video::GUI;
	friend class private_video::FixedTexMemMgr;
	friend class private_video::VariableTexMemMgr;
	friend class private_video::TexSheet;
	friend class private_video::ParticleSystem;
	friend class StillImage;
	friend class RenderedString;

public:
	SINGLETON_METHODS(GameVideo);


	//-- General --------------------------------------------------------------

	/** \brief Clears the contents of the current screen.
	*** \return success/failure
	***
	*** This method should be called at the beginning of every frame, before any
	*** draw operations are performed.
	**/
	bool Clear();

	/** \brief Clears the screen to a specific color.
	*** \param c The color to set the cleared screen to.
	*** \return success/failure
	**/
	bool Clear(const Color &c);

	/** \brief call at end of every frame
	*** \param frame_ime The number of milliseconds that have passed since the last frame.
	*** \return success/failure
	**/
	bool Display(int32 frame_time);


	/** \brief set the target, i.e. whether the video engine is being used to
	 *         draw to an SDL window, or a QT widget. (There are some important
	 *         differences, so the video engine needs to know).
	 *
	 *  \param target can be VIDEO_TARGET_QT_WIDGET, or VIDEO_TARGET_SDL_WINDOW
	 * \return success/failure
	 *
	 *  \note  If you don't call this function, the default target is SDL window
	 *         Also, note that you MUST call this before calling Initialize()
	 */
	bool SetTarget(VIDEO_TARGET target);

	//-- Video settings -------------------------------------------------------

	//! NOTE: when you modify video setting, you must call ApplySettings()
	//!       to actually apply them

	/** \brief sets the current resolution to the given width and height
	 *
	 *  \param width new screen width
	 *  \param height new screen height
	 * \return success/failure
	 *
	 *  \note  you must call ApplySettings() to actually apply the change
	 */
	bool SetResolution(int32 width, int32 height);

	/** \brief returns width, (whatever was set with SetResolution)
	 * \return width of the screen
	 */
	int32 GetWidth() { return _width; }

	/** \brief returns height, (whatever was set with SetResolution)
	 * \return height of the screen
	 */
	int32 GetHeight() { return _height; }

	/** \brief returns true if game is in fullscreen mode
	 * \return true if in fullscreen mode, false if in windowed mode
	 */
	bool IsFullscreen();

	/** \brief sets the game to fullscreen or windowed depending on whether
	 *         true or false is passed
	 *
	 *  \param fullscreen set to true if you want fullscreen, false for windowed.
	 *  \note  you must call ApplySettings() to actually apply the change
	 */
	bool SetFullscreen(bool fullscreen);

	/** \brief toggles fullscreen on and off
	 * \return success/failure
	 *  \note  you must call ApplySettings() to actually apply the change
	 */
	bool ToggleFullscreen();

	/** \brief applies any changes to video settings like resolution and
	 *         fullscreen. If the changes fail, then this function returns
	 *         false, and the video settings are reset to whatever the last
	 *         working setting was.
	 * \return success/failure
	 */
	bool ApplySettings();

	//-- Coordinate system / viewport  ------------------------------------------

	/** \brief sets the viewport, i.e. the area of the screen that gets drawn
	 *         to. The default is (0, 100, 0, 100).
	 *
	 *  \param left   left border of screen
	 *  \param right  right border of screen
	 *  \param top    top border of screen
	 *  \param bottom bottom border of screen
	 *  \note  The arguments are percentages (0-100)
	 */
	void SetViewport(float left, float right, float bottom, float top);

	/** \brief sets the coordinate system. Default is (0,1024,0,768)
	 *  \param left   left border of screen
	 *  \param right  right border of screen
	 *  \param top    top border of screen
	 *  \param bottom bottom border of screen
	 */
	void SetCoordSys(float left, float right, float bottom, float top);

	/** \brief sets the coordinate system. Default is (0,1024,0,768)
	 *  \param coordSys the coordinate system you want to set to
	 */
	void SetCoordSys(const CoordSys &coordSys);

	/** \brief enables scissoring, where you can specify a rectangle of the screen
	 *         which is affected by rendering operations. MAKE SURE to disable
	 *         scissoring as soon as you're done using the effect, or all subsequent
	 *         draw calls will get messed up
	 *
	 *  \param enable pass true to turn on scissoring, false to disable
	 */
	void EnableScissoring(bool enable);

	/** \brief returns true if scissoring's enabled
	 * \return true if enabled, false if not
	 */
	bool IsScissoringEnabled()
		{ return _scissorEnabled; }

	/** \brief sets the rectangle to use for scissorring, where you can specify an
	 *         area of the screen for draw operations to affect. Note, the coordinates
	 *         you pass in are based on the current coordinate system, not screen coords
	 * \param left coordinate for left side of rectanlge
	 * \param right coordinate for right side of rectanlge
	 * \param bottom coordinate for bottom side of rectanlge
	 * \param top coordinate for top side of rectanlge
	 */
	void SetScissorRect(float left, float right, float bottom, float top);

	/** \brief sets the rectangle to use for scissorring. This version of the function
	 *         expects a ScreenRect, in other words the coordinates have already been
	 *         transformed to integer values (pixel unit) with (0,0) as the upper left
	 *         and (w-1, h-1) as the lower right, where w and h are the current screen
	 *         dimensions
	 * \param rect rectangle to set the scissor rectangle to
	 */
	void SetScissorRect (const ScreenRect &rect);

	/** \brief returns scissor rect
	 * \return the scissor rectangle
	 */
	ScreenRect GetScissorRect()
		{ return _scissorRect; }

	/** \brief converts coordinates given relative to the current coord sys into
	 *         "screen coordinates", which are in pixel units with (0,0) as the
	 *         top left and (w-1, h-1) as the lower-right, where w and h are the
	 *         dimensions of the screen
	 * \return the screen rectangle
	 */

	ScreenRect CalculateScreenRect(float left, float right, float bottom, float top);

	//-- Transformations ------------------------------------------------------

	/** \brief saves entire state of the video engine on to the stack (all draw
	 *         flags, coordinate system, scissor rect, viewport, etc.)
	 *         This is useful for safety purposes between two major parts of code
	 *         to ensure that one part doesn't inadvertently affect the other.
	 *         However, it's a very expensive function call. If you only need to
	 *         push the current transformation, you should use PushMatrix() and
	 *         PopMatrix()
	 */
	void PushState();

	/** \brief pops the most recently pushed video engine state from the stack
	 *         and restores all of the old settings.
	 */
	void PopState ();


	/** \brief saves current modelview transformation on to the stack. In English,
	 *         that means the combined result of calls to Move/MoveRelative/Scale/Rotate
	 */
	void PushMatrix();

	/** \brief pops the modelview transformation from the stack.
	 */
	void PopMatrix();

	/** \brief sets draw position to (x,y)
	 *  \param x  x coordinate to move to
	 *  \param y  y coordinate to move to
	 */
	void Move(float x, float y);

	/** \brief moves draw position (dx, dy) units
	 *  \param dx how many units to move in x direction
	 *  \param dy how many units to move in y direction
	 */
	void MoveRelative(float dx, float dy);

	/** \brief Gets the location of the draw cursor
	* \param x stores x position of the cursor
	* \param y stores y position of the cursor
	*/
	void GetDrawPosition(float &x, float &y);

	/** \brief rotates images counterclockwise by 'angle' radians
	 *  \param angle how many radians to rotate by
	 *  \note This function should NOT be used unless you understand how transformation
	 *        matrices work in OpenGL.
	 */
	void Rotate (float angle);

	/** \brief after you call this, subsequent calls to DrawImage() result in
	 *         a scaled image
	 *
	 *  \param xScale pass 1.0 for normal horizontal scaling, 2.0 for double
	 *                  scaling, etc.
	 *  \param yScale same, except vertical scaling
	 *
	 *  \note This function should NOT be used unless you understand how transformation
	 *        matrices work in OpenGL.
	 */

	void Scale(float xScale, float yScale);

	/** \brief sets OpenGL transform to contents of 4x4 matrix (16 values)
	 *  \param array of 16 float values forming a 4x4 transformation matrix
	 */
	void SetTransform(float m[16]);

	//-- Text -----------------------------------------------------------------

	/** \brief Loads a font from a .ttf file with a specific size
	*** \param TTF_filename The filename of the .ttf file to load.
	*** \param name The name which to refer to the font (e.g. "courier24", "default", etc.)
	*** \param size The point size of the font
	*** \return success/failure
	**/
	bool LoadFont(const std::string &TTF_filename, const std::string &name, uint32 size);

	/** \brief Returns true if a font has already been successfully loaded
	*** \param name The name which to refer to the loaded font (e.g. "courier24").
	*** \return True if font is valid, false if it is not.
	**/
	bool IsValidFont(const std::string &name)
		{ return (_font_map.find(name) != _font_map.end()); }

	/** \brief Get the font properties for a previously loaded font
	*** \param fontName  The referred name of the loaded font (e.g. "courier24").
	*** \return A pointer to the FontProperties object with the requested data, or NULL if an error occurred.
	**/
	FontProperties* GetFontProperties(const std::string &fontName);

	/** \brief calculates the width of the given text if it were rendered with the given font
	 *         If an invalid font name is passed, returns -1
	 *
	 *  \param fontName  the font to use
	 *  \param text      the text string in unicode
	 * \return width of the given text
	 */
	int32 CalculateTextWidth(const std::string &fontName, const hoa_utils::ustring &text);


	/** \brief calculates the width of the given text if it were rendered with the given font
	 *         If an invalid font name is passed, returns -1
	 *
	 *  \param fontName  the font to use
	 *  \param text      the text string in multi-byte character format
	 * \return width of the given text
	 */
	int32 CalculateTextWidth(const std::string &fontName, const std::string  &text);

	/** \brief sets current font
	 *  \param name  name of the font to set to
	 * \return success/failure
	 */
	bool SetFont(const std::string &name);

	/** \brief enables/disables text shadowing
	 *  \param enable  pass true to enable, false to disable
	 */
	void EnableTextShadow(bool enable);

	/** \brief sets current text color
	 *  \param color  color to set to
	 * \return success/failure
	 */
	void SetTextColor(const Color &color)
		{ _currentTextColor = color; }

	/** \brief sets the shadow offset to use for a given font. By default, all font shadows
	 *         are slightly to the right and to the bottom of the text, by an offset of
	 *         fontHeight / 8. That doesn't always look good though, so use this function
	 *         to adjust it if you want.
	 *
	 *  \param fontName  label of the font you want to set the shadow offset for
	 *  \param x         x offset in pixels (based on 1024x768)
	 * \return success/failure
	 */
	bool SetFontShadowXOffset(const std::string &fontName, int32 x);

	/** \brief sets the shadow offset to use for a given font. By default, all font shadows
	 *         are slightly to the right and to the bottom of the text, by an offset of
	 *         fontHeight / 8. That doesn't always look good though, so use this function
	 *         to adjust it if you want.
	 *
	 *  \param fontName  label of the font you want to set the shadow offset for
	 *  \param y         y offset in pixels (based on 1024x768)
	 * \return success/failure
	 */
	bool SetFontShadowYOffset(const std::string &fontName, int32 y);

	/** \brief sets the shadow style to use for the given font
	 *
	 *  \param fontName  label of the font you want to set the shadow style for
	 *  \param style     the shadow style you want (e.g. VIDEO_TEXT_SHADOW_BLACK)
	 * \return success/failure
	 */
	bool SetFontShadowStyle(const std::string &fontName, TEXT_SHADOW_STYLE style);

	/** \brief get name of current font
	 * \return string containing the name of the font
	 */
	std::string GetFont      () const;

	/**
	 *  \brief get current text color
	 * \return the color fo the text
	 */
	Color       GetTextColor () const;

	/** \brief non-unicode version of DrawText(). Only use this for debug
	 *         output or other things which don't have to be localized
	 *
	 *  \param text   text string to draw
	 * \return success/failure
	 */
	bool DrawText(const std::string &text);

	/** \brief unicode version of DrawText(). This should be used for
	 *         anything in the game which might need to be localized
	 *         (game dialogue, interface text, etc.)
	 *
	 *  \param uText  unicode text string to draw
	 * \return success/failure
	 */
	bool DrawText(const hoa_utils::ustring &uText);

	//-- Particle effects -----------------------------------------------------------

	/** \brief add a particle effect at the given point x and y
	 *  \param filename - file containing the particle effect definition
	 *  \param x - X coordinate of the effect
	 *  \param y - Y coordinate of the effect
	 *  \param reload - reload the effect from file if it already exists
	 *  \return id corresponding to the loaded particle effect
	 *  \note  set the reload parameter to true to reload the effect definition file
	 *         every time the effect is played. This is useful if you are working on an
	 *         effect and want to see how it looks. When we actually release the game,
	 *         reload should be false since it adds some cost to the loading
	 */
	ParticleEffectID AddParticleEffect(const std::string &particleEffectFilename, float x, float y, bool reload=false);

	/** \brief draws all active particle effects
	 * \return success/failure
	 */
	bool DrawParticleEffects();

	/** \brief stops all active particle effects
	 *  \param kill_immediate  If this is true, then the particle effects die out immediately
	 *                         If it is false, then they don't immediately die, but new particles
	 *                         stop spawning
	 */
	void StopAllParticleEffects(bool kill_immediate = false);

	/** \brief get pointer to an effect given its ID
	 * \return the particle effect with the given ID
	 */
	ParticleEffect *GetParticleEffect(ParticleEffectID id);

	/** \brief get number of live particles
	 * \return the number of live particles in the system
	 */
	int32 GetNumParticles();

	//-- Images / Animation ---------------------------------------------------------

	/** \brief loads an image (static or animated image). Assumes that you have already called
	 *         all the appropriate functions to initialize the image. In the case of a static image,
	 *         this means setting its filename, and possibly other properties like width, height, and
	 *         color. In the case of an animated image, it means calling AddFrame().
	 *
	 *  \param id  image descriptor to load- either a StillImage or AnimatedImage.
	 * \return success/failure
	 */
	bool LoadImage(ImageDescriptor &id);

	/** \brief loads an image (static or animated image). Assumes that you have already called
	 *         all the appropriate functions to initialize the image. In the case of a static image,
	 *         this means setting its filename, and possibly other properties like width, height, and
	 *         color. In the case of an animated image, it means calling AddFrame().  The image is
	 *	  loaded in grayscale.
	 *
	 *  \param id  image descriptor to load- either a StillImage or AnimatedImage.
	 * \return success/failure
	 */
	bool LoadImageGrayScale(ImageDescriptor &id);

	/** \brief Loads a MultiImage in a vector of StillImages.
	 *	This function loads an image and cut it in pieces, loading each of
	 *	that on separate StillImage objects.
	 *  \param images  Vector of StillImages where the cut images will be loaded.
	 *	\param filename Name of the file to be opened to read.
	 *	\param rows Number of rows of sub-images in the MultiImage.
	 *	\param cols Number of columns of sub-images in the MultiImage.
	 *	\return success/failure
	 */
	bool LoadMultiImage(std::vector<StillImage> &images, const std::string &filename, const uint32 rows, const uint32 cols);

	/** \brief Loads a MultiImage in an AnimatedImage as frames.
	 *	This function loads an image and cut it in pieces, loading each of
	 *	that on frames of an AnimatedImage.
	 *  \param image  AnimatedImage where the cut images will be loaded.
	 *	\param filename Name of the file to be opened to read.
	 *	\param rows Number of rows of sub-images in the MultiImage.
	 *	\param rows Number of columns of sub-images in the MultiImage.
	 *	\return success/failure
	 */
	bool LoadAnimatedImage(AnimatedImage &image, const std::string &filename, const uint32 rows, const uint32 cols);

	//! \brief Saves a vector of images in a single file.
	/*	This function stores a vector of images as a single image. This is useful for creating multiimage 
	 *	images. The image can be stored in JPEG or PNG, which is decided by the filename.
	 *  \param image Vector of images to save
	 *	\param filename Name of the file to be opened to read.
	 *	\param rows Number of rows of sub-images in the MultiImage.
	 *	\param cols Number of columns of sub-images in the MultiImage.
	 *	\return success/failure
	 */
	bool SaveImage (const std::string &file_name, const std::vector<StillImage*> &image, const uint32 rows, const uint32 columns) const;

	//! \brief Saves and image into a file.
	/*	This function stores a vector of images as a single image. This is useful for creating multiimage 
	 *	images. The image can be stored in JPEG or PNG, which is decided by the filename.
	 *  \param image Image to store.
	 *	\param filename Name of the file to be opened to read.
	 *	\return success/failure
	 */
	bool SaveImage (const std::string &file_name, const StillImage &image) const;

	/** \brief captures the contents of the screen and saves it to an image
	 *         descriptor
	 *
	 *  \param id  image descriptor to capture to
	 * \return success/failure
	 */
	bool CaptureScreen(StillImage &id);

	/** \brief decreases ref count of an image (static or animated)
	 *
	 *  \param id  image descriptor to decrease the reference count of
	 * \return success/failure
	 */
	bool DeleteImage(ImageDescriptor &id);

	/** \brief unloads all texture sheets from memory when we lose the GL
	 *         context, so textures can be properly reloaded
	 * \return success/failure
	 */
	bool UnloadTextures();

	/** \brief reloads textures that have been unloaded after a video
	 *         settings change
	 * \return success/failure
	 */
	bool ReloadTextures();

	/** \brief sets draw flags (flip, align, blending, etc). Simply pass
	 *         in as many parameters as you want, as long as the last
	 *         parameter is a zero.
	 *         e.g. SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_CENTER, 0);
	 * \param firstflag the first of many draw flags
	 * \param .. specify as many draw flags as you want.  Terminate with a 0.
	 */
	void SetDrawFlags(int32 firstflag, ...);

	/** \brief draws an image which is modulated by the scene's light color
	 *
	 *  \param id  image descriptor to draw (either StillImage or AnimatedImage)
	 * \return success/failure
	 */
	bool DrawImage(const ImageDescriptor &id);

	/** \brief draws an image which is modulated by a custom color
	 *
	 *  \param id  image descriptor to draw (either StillImage or AnimatedImage)
	 * \return success/failure
	 */
	bool DrawImage(const ImageDescriptor &id, const Color &color);

	/** \brief converts a 2D array of tiles into one big image
	 *
	 *  \param tiles   a vector of image descriptors (the tiles)
	 *  \param indices a 2D vector in row-column order (e.g. indices[y][x])
	 *         which forms a rectangular array of tiles
	 * \return the image built out of the 2D array of tiles
	 */
	StillImage TilesToObject(std::vector<StillImage> &tiles, std::vector< std::vector<uint32> > indices);

	/** \brief returns the amount of animation frames that have passed since the last
	 *         call to GameVideo::Display(). This number is based on VIDEO_ANIMATION_FRAME_PERIOD,
	 *         and is used so that AnimatedImages know how many frames to increment themselves by.
	 * \return the number of nimations frames passed since last GameVideo::Display() call
	 */
	int32 GetFrameChange() { return _current_frame_diff; }

	/** \brief These cycle through the currently loaded texture sheets so they can be viewed on screen
	 */
	void DEBUG_NextTexSheet();
	void DEBUG_PrevTexSheet();

	//-- Menus -----------------------------------------------------------------

	/** \brief sets the current menu skin (borders+fill color). Assumes all four
	 *         vertices of menu interior are same color
	 *
	 *  \param imgBaseName  name of images which form the border
	 *                      For example if you pass in "/img/menus/chrome", then it will load:
	 *                          /img/menus/chrome_tl.png
	 *                          /img/menus/chrome_t.png
	 *                          /img/menus/chrome_tr.png
	 *                          /img/menus/chrome_l.png
	 *                          /img/menus/chrome_r.png
	 *                          /img/menus/chrome_bl.png
	 *                          /img/menus/chrome_b.png
	 *                          /img/menus/chrome_br.png
	 *                          /img/menus/chrome_tri_t.png
	 *                          /img/menus/chrome_tri_l.png
	 *                          /img/menus/chrome_tri_r.png
	 *                          /img/menus/chrome_tri_b.png
	 *                          /img/menus/chrome_quad.png
	 *
	 *  \param fillColor    color for inner area of menu. can be transparent
	 * \return success/failure
	 */
	bool SetMenuSkin(const std::string &imgBaseName, const Color  &fillColor);

	/** \brief sets the current menu skin (borders+fill color). This version of
	 *         SetMenuSkin() allows the 4 vertices of the interior of the menu
	 *         to have different colors so you can have gradients.
	 *
	 *  \param imgBaseName  name of images which form the border
	 *                      For example if you pass in "/img/menus/chrome", then it will load:
	 *                          /img/menus/chrome_tl.png
	 *                          /img/menus/chrome_t.png
	 *                          /img/menus/chrome_tr.png
	 *                          /img/menus/chrome_l.png
	 *                          /img/menus/chrome_r.png
	 *                          /img/menus/chrome_bl.png
	 *                          /img/menus/chrome_b.png
	 *                          /img/menus/chrome_br.png
	 *                          /img/menus/chrome_tri_t.png
	 *                          /img/menus/chrome_tri_l.png
	 *                          /img/menus/chrome_tri_r.png
	 *                          /img/menus/chrome_tri_b.png
	 *                          /img/menus/chrome_quad.png
	 *
	 *  \param fillColor_TL  color for upper left  vertex of interior
	 *  \param fillColor_TR  color for upper right vertex of interior
	 *  \param fillColor_BL  color for lower left  vertex of interior
	 *  \param fillColor_BR  color for lower right vertex of interior
	 * \return success/failure
	 */
	bool SetMenuSkin(const std::string &imgBaseName, const Color  &fillColor_TL, const Color  &fillColor_TR,
		const Color  &fillColor_BL, const Color  &fillColor_BR);

	/** \brief sets the current menu skin (borders+background image).
	 *
	 *  \param imgBaseName  name of images which form the border
	 *                      For example if you pass in "/img/menus/chrome", then it will load:
	 *                          /img/menus/chrome_tl.png
	 *                          /img/menus/chrome_t.png
	 *                          /img/menus/chrome_tr.png
	 *                          /img/menus/chrome_l.png
	 *                          /img/menus/chrome_r.png
	 *                          /img/menus/chrome_bl.png
	 *                          /img/menus/chrome_b.png
	 *                          /img/menus/chrome_br.png
	 *                          /img/menus/chrome_tri_t.png
	 *                          /img/menus/chrome_tri_l.png
	 *                          /img/menus/chrome_tri_r.png
	 *                          /img/menus/chrome_tri_b.png
	 *                          /img/menus/chrome_quad.png
	 *
	 *  \param backgroundImage image filename for the background of the menu window
	 *
	 *  \param fillColor_TL  color for upper left  vertex of interior
	 *  \param fillColor_TR  color for upper right vertex of interior
	 *  \param fillColor_BL  color for lower left  vertex of interior
	 *  \param fillColor_BR  color for lower right vertex of interior
	 * \return success/failure
	 */
	bool SetMenuSkin(const std::string &imgBaseName, const std::string &backgroundImage,
		const Color  &fillColor_TL, const Color  &fillColor_TR, const Color  &fillColor_BL, const Color  &fillColor_BR);

	/** \brief sets the current menu skin (borders+background image).
	 *
	 *  \param imgBaseName  name of images which form the border
	 *                      For example if you pass in "/img/menus/chrome", then it will load:
	 *                          /img/menus/chrome_tl.png
	 *                          /img/menus/chrome_t.png
	 *                          /img/menus/chrome_tr.png
	 *                          /img/menus/chrome_l.png
	 *                          /img/menus/chrome_r.png
	 *                          /img/menus/chrome_bl.png
	 *                          /img/menus/chrome_b.png
	 *                          /img/menus/chrome_br.png
	 *                          /img/menus/chrome_tri_t.png
	 *                          /img/menus/chrome_tri_l.png
	 *                          /img/menus/chrome_tri_r.png
	 *                          /img/menus/chrome_tri_b.png
	 *                          /img/menus/chrome_quad.png
	 *
	 *  \param backgroundImage image filename for the background of the menu window
	 *
	 *  \param fillColor       color for for interior of window
	 * \return success/failure
	 */
	bool SetMenuSkin(const std::string &imgBaseName, const std::string &backgroundImage, const Color &fillColor);

	//-- Lighting and fog -----------------------------------------------------

	/** \brief turn on the ligt color for the scene
	 * \param color the light color to use
	 * \return success/failure
	 */
	bool EnableSceneLighting(const Color &color);

	/** \brief disables scene lighting
	*/
	void DisableSceneLighting();

	/** \brief returns the scene lighting color
	 * \return the light color used in the scene
	 */
	Color &GetSceneLightingColor();

	/** \brief sets fog parameters
	 *
	 *  \param color  Color of the fog (alpha should be 1.0)
	 *  \param intensity  Intensity of fog from 0.0f to 1.0f
	 * \return success/failure
	 */
	bool EnableFog(const Color &color, float intensity);

	/** \brief disables fog
	*/
	void DisableFog();

	/** \brief draws a halo at the given spot
	 *
	 *  \param id    image descriptor for the halo image
	 *  \param x     x coordinate of halo
	 *  \param y     y coordinate of halo
	 *  \param color color of halo
	 * \return success/failure
	 */
	bool DrawHalo(const StillImage &id, float x, float y, const Color &color = Color(1.0f, 1.0f, 1.0f, 1.0f));

	/** \brief draws a real light at the given spot
	 *
	 *  \param id    image descriptor for the light mask
	 *  \param x     x coordinate of light
	 *  \param y     y coordinate of light
	 *  \param color color of light
	 * \return success/failure
	 */
	bool DrawLight(const StillImage &id, float x, float y, const Color &color = Color(1.0f, 1.0f, 1.0f, 1.0f));

	/** \brief call if this map uses real lights
	 * \return always returns true
	 */
	bool EnablePointLights();

	/**
	* \brief disables point lights
	*/
	void DisablePointLights();

	/** \brief call after rendering all real lights.  This function renders all lights
	 *		   to the lighting overlay texture, Moved into ApplyLightingOverlay
	 * \return success/failure
	 */
	//bool AccumulateLights();

	/** \brief call after all map images are drawn to apply lighting. All
	 *         menu and text rendering should occur AFTER this call, so that
	 *         they are not affected by lighting.
	 * \return success/failure
	 */
	bool ApplyLightingOverlay();

	//-- Overlays / lightning -------------------------------------------------------

	/** \brief draws a full screen overlay of the given color
	 *  \note  This is very slow, so use sparingly!
	 * \return success/failure
	 */
	bool DrawFullscreenOverlay(const Color &color);

	/** \brief call to create lightning effect
	 *  \param litFile a .lit file which contains lightning intensities stored
	 *                 as bytes (0-255).
	 * \return success/failure
	 */
	bool MakeLightning(const std::string &litFile);

	/** \brief call this every frame to draw any lightning effects. You should make
	 *         sure to place this call in an appropriate spot. In particular, you should
	 *         draw the lightning before drawing the GUI.
	 * \return success/failure
	 */
	bool DrawLightning();

	//-- Fading ---------------------------------------------------------------

	/** \brief Begins a screen fade.
	 *  \param color - color to fade to.
	 *  \param fade_time - the fade will last this number of seconds
	 *  \return True if fade was successful, false otherwise.
	 */
	void FadeScreen(const Color &color, float fade_time);

	/** \brief Determines if a fade is currently occurring.
	 *  \return True if screen fade is currently in progress, false otherwise.
	 */
	bool IsFading();

	//-- Screen shaking -------------------------------------------------------

	/** \brief shakes the screen
	 *
	 *  \param force        Initial force of the shake
	 *  \param falloffTime  How long the shake should last for. Pass 0.0f for
	 *                      a shake that doesn't end until you stop it manually
	 *  \param falloff      Specifies the method of falloff. The default is
	 *                      VIDEO_FALLOFF_NONE.
	 * \return success/failure
	 */
	bool ShakeScreen(float force, float falloffTime, ShakeFalloff falloff = VIDEO_FALLOFF_NONE);

	/**  \brief stops all shake effects
	 * \return success/failure
	 */
	bool StopShaking();

	/** \brief returns true if screen is shaking
	 * \return true if the screen is shaking, false if it's not
	 */
	bool IsShaking();

	//-- Miscellaneous --------------------------------------------------------

	/** \brief Sets a new gamma value using SDL_SetGamma()
	 *
	 *  \param value        Gamma value of 1.0f is the default value
	 */
	void SetGamma(float value);

	/** \brief Returns the gamma value
	 * \return the gamma value
	 */
	float GetGamma();

	/** \brief updates the FPS counter with the given frame time and draws the
	 *         current FPS on the screen.
	 * \return success/failure
	 */
	bool DrawFPS(int32 frameTime);

	/** \brief toggles the FPS display (on by default)
	 */
	void ToggleFPS();

	/** \brief draws a line grid. Used by map editor to draw a grid over all
	 *         the tiles. This function will start at (x,y), and go to
	 *         (xMax, yMax), with horizontal cell spacing of xstep and
	 *         vertical cell spacing of ystep. The final parameter is just the
	 *         color the lines should be drawn
	 *
	 *  \note  xMax and yMax are not inputs to the function- they are taken
	 *         from the current coord sys
	 * \param x x coordinate to start grid at
	 * \param y y coordinate to start grid at
	 * \param xstep width of grid squares
	 * \param ystep height of grid squares
	 * \param c color of the grid
	 */
	void DrawGrid(float x, float y, float xstep, float ystep, const Color &c);

	/** \brief Draws a solid rectangle of a given color.
	 * Draws a solid rectangle of a given color. For that, the lower-left corner
	 * of the rectangle has to be specified, and also its size. The parameters depends
	 * on the current Coordinate System.
	 * \param width Width of the rectangle.
	 * \param height Height of the rectangle.
	 * \param color Color to paint the rectangle.
	 */
	void DrawRectangle(const float width, const float height, const Color &color);

	/** \brief makes a screenshot, saves it as screenshot.jpg in the directory
	 *         of the game
	 * \return success/failure
	 */
	bool MakeScreenshot();

	/** \brief toggles advanced information display for video engine, shows
	 *         things like number of texture switches per frame, etc.
	 * \return always returns true
	 */
	bool ToggleAdvancedDisplay();

	/** \brief sets the default cursor to the image in the given filename
	* \param cursorImageFilename file containing the cursor image
	*/
	bool SetDefaultCursor(const std::string &cursorImageFilename);

	/** \brief returns the cursor image
	* \return the cursor image
	*/
	StillImage *GetDefaultCursor();

	/** \brief Draws a rendered string object
	 * \param string The rendered string
	 */
	bool Draw(const RenderedString &string);

	/** \brief Draws a rendered line object
	 * \param line The rendered line
	 * \param texIndex Whether main or shadow texture
	 */
	bool Draw(const RenderedLine &line, int32 texIndex);

	/** \brief Renders the given string to a drawable object
	 * \param txt The string to render
	 */
	RenderedString *RenderText(const hoa_utils::ustring &txt);

private:
	SINGLETON_DECLARE(GameVideo);

	//-- Private variables ----------------------------------------------------

	// for now the game gui class is a member of video so that
	// externally people only have to deal with GameVideo.

	//! pointer to GUI class which implements all GUI functionality
	private_video::GUI *_gui;

	//! particle manager, does dirty work of managing particle effects
	private_video::ParticleManager _particle_manager;

	//! target (QT widget or SDL window)
	VIDEO_TARGET _target;

	// draw flags

	//! blend flag which specifies normal alpha blending
	int8 _blend;

	//! x align flag which tells if images should be left, center, or right aligned
	int8 _xalign;

	//! y align flag which tells if images should be top, center, or bottom aligned
	int8 _yalign;

	//! x flip flag. true if images should be flipped horizontally
	int8 _xflip;

	//! y flip flag. true if images should be flipped vertically
	int8 _yflip;

	//! eight character name for temp files that increments every time you create a new one so they are always unique
	char _nextTempFile[9];

	//! current coordinate system
	CoordSys    _coord_sys;

	//! current viewport
	ScreenRect _viewport;

	//! current scissor rectangle
	ScreenRect _scissorRect;

	//! is scissoring enabled or not
	bool _scissorEnabled;

	//! fader class which implements screen fading
	private_video::ScreenFader _fader;

	//! advanced display flag. If true, info about the video engine is shown on screen
	bool   _advancedDisplay;

	//! fps display flag. If true, FPS is displayed
	bool   _fpsDisplay;

	//! current debug texture sheet
	int32  _currentDebugTexSheet;

	//! keep track of number of texture switches per frame
	int32  _numTexSwitches;

	//! keep track of number of draw calls per frame
	int32  _numDrawCalls;

	//! true if real lights are enabled
	bool   _usesLights;

	//! lighting overlay texture
	GLuint _lightOverlay;

	//! offsets to shake the screen by (if any)
	float  _shakeX, _shakeY;

	//! Current gamma value
	float _gamma_value;

	//! current shake forces affecting screen
	std::list<private_video::ShakeForce> _shakeForces;

    //! true if game is currently running fullscreen
	bool _fullscreen;

	//! current screen width
	int32  _width;

	//! current screen height
	int32  _height;

	// changing the video settings does not actually do anything until
	// you call ApplySettings(). Up til that point, store them in temp
	// variables so if the new settings are invalid, we can roll back.

	//! holds the desired fullscreen status (true=fullscreen, false=windowed). Not actually applied until ApplySettings() is called
	bool   _temp_fullscreen;

	//! holds the desired screen width. Not actually applied until ApplySettings() is called
	int32  _temp_width;

	//! holds the desired screen height. Not actually applied until ApplySettings() is called
	int32  _temp_height;

	//! ID of the last texture that was bound. Used to eliminate redundant binding of textures
	GLuint _lastTexID;

	//! current font name
	std::string _current_font;

	//! current text color
	Color       _currentTextColor;

	//! image which is to be used as the cursor
	StillImage _defaultMenuCursor;

	//! Image used for rendering rectangles
	StillImage _rectangle_image;

	//! if true, text shadow effect is enabled
	bool _textShadow;

	//! current fog color (set by SetFog())
	Color _fogColor;

	//! current fog intensity (set by SetFog())
	float _fogIntensity;

	//! current scene lighting color (essentially just modulates vertex colors of all the images)
	Color _lightColor;

	//! true if a lightning effect is active
	bool  _lightningActive;

	//! current time of lightning effect (time since it started)
	int32 _lightningCurTime;

	//! how many milliseconds to do the lightning effect for
	int32 _lightningEndTime;

	//! intensity data for lightning effect
	std::vector <float> _lightningData;

	//! counter to keep track of milliseconds since game started for animations
	int32 _animation_counter;

	//! keeps track of the number of frames animations should increment by for the current frame
	int32 _current_frame_diff;

	//! STL map containing all the images currently being managed by the video engine
	std::map    <std::string, private_video::Image*>   _images;

	//! vector containing all texture sheets currently being managed by the video engine
	std::vector <private_video::TexSheet *>     _texSheets;

	//! STL map containing properties for each font (includeing TTF_Font *)
	std::map    <std::string, FontProperties *> _font_map;

	//! STL map containing all loaded particle effect definitions
	std::map <std::string, ParticleEffectDef *> _particle_effect_defs;

	//! stack containing context, i.e. draw flags plus coord sys. Context is pushed and popped by any GameVideo functions that clobber these settings
	std::stack  <private_video::Context>      _contextStack;

	//-- Private methods ------------------------------------------------------

	/** \brief wraps a call to glBindTexture(), except it adds checking to eliminate redundant texture binding. Redundancy checks are already implemented by most drivers, but this is a double check "just in case"
	 *
	 *  \param texID   integer handle to the OpenGL texture
	 * \return success/failure
	 */
	bool _BindTexture(GLuint texID);

	/** \brief converts VIDEO_DRAW_LEFT or VIDEO_DRAW_RIGHT flags to a numerical offset
	* \param xalign the draw flag
	* \return the numerical offset
	*/
	int32 _ConvertXAlign(int32 xalign);

	/** \brief converts VIDEO_DRAW_TOP or VIDEO_DRAW_BOTTOM flags to a numerical offset
	* \param yalign the draw flag
	* \return the numerical offset
	*/
	int32 _ConvertYAlign(int32 yalign);

	/**  \brief creates a blank texture of the given width and height and returns integer used by OpenGL to refer to this texture. Returns 0xffffffff on failure.
	 *
	 *  \param width   desired width of the texture
	 *  \param height  desired height of the texture
	 * \return OpenGL ID for this texture or 0xffffffff for failure.
	 */
	GLuint _CreateBlankGLTexture(int32 width, int32 height);

	/** \brief creates an StillImage of a menu which is the given size
	 *
	 *  \param menu   Reference to menu to create
	 *
	 *  \param width  Width of menu, based on pixels in 1024x768 resolution
	 *  \param height Height of menu, based on pixels in 1024x768 resolution.
	 *  \param innerWidth return value for the width of the inside of the menu
	 *  \param innerHeight return value for the height of the inside of the menu
	 *  \param edgeVisibleFlags bit flags to tell which edges are visible
	 *  \param edgeSharedFlags  bit flags to tell which edges are shared with other menus
	 *
	 *  \note  this is only meant to be used by the Menu class, not by users of
	 *         the video engine.
	 * \return success/failure
	 */
	bool _CreateMenu(StillImage &menu, float width, float height, float & innerWidth, float & innerHeight,
		int32 edgeVisibleFlags, int32 edgeSharedFlags);

	/** \brief returns a filename like TEMP_abcd1234.ext, and each time you call it, it increments the
	 *         alphanumeric part of the filename. This way, during any particular run
	 *         of the game, each temp filename is guaranteed to be unique.
	 *         Assuming you create a new temp file every second, it would take 100,000 years to get
	 *         from TEMP_00000000 to TEMP_zzzzzzzz
	 *
	 *  \param extension   The extension for the temp file. Although we could just save temp files
	 *                     without an extension, that might cause stupid bugs like DevIL refusing
	 *                     to load an image because it doesn't end with .png.
	 * \return name of the generated temp file
	 */
	std::string _CreateTempFilename(const std::string &extension);

	/** \brief creates a texture sheet
	 *
	 *  \param width    width of the sheet
	 *  \param height   height of the sheet
	 *  \param type     specifies what type of images this texture sheet manages (e.g. 32x32 images, 64x64 images, any type, etc)
	 *  \param isStatic if true, this texture sheet is meant to manage images which are not expected to be loaded and unloaded very often
	 * \return the newly created texsheet
	 */
	private_video::TexSheet *_CreateTexSheet(int32 width, int32 height, private_video::TexSheetType type, bool isStatic);

	/** \brief wraps a call to glDeleteTextures(), except it adds some checking related to eliminating redundant texture binding.
	 *
	 *  \param texID   integer handle to the OpenGL texture
	 * \return success/failure
	 */
	bool _DeleteTexture(GLuint texID);

	/** \brief decreases the reference count of an image
	 *
	 *  \param image  pointer to image
	 * \return success/failure
	 */
	bool _DeleteImage(private_video::Image *const image);

	/** \brief decreases ref count of an image
	 *
	 *  \param id  image descriptor to decrease the reference count of
	 * \return success/failure
	 */
	bool _DeleteImage(StillImage &id);

	/** \brief decreases ref count of an animated image
	 *
	 *  \param id  image descriptor to decrease the reference count of
	 * \return success/failure
	 */
	bool _DeleteImage(AnimatedImage &id);

	/** \brief deletes the temporary textures from the "temp" folder that were saved
	 *         by _SaveTempTextures()
	 * \return success/failure
	 */
	bool _DeleteTempTextures();

	/** \brief draws an image element, i.e. one image within an image descriptor which may contain multiple images
	 *
	 *  \param element        pointer to the image element to draw
	 *  \param modulateColor  combination of color for this image, and our current fade color
	 * \return success/failure
	 */
	bool _DrawElement(const private_video::ImageElement &element, const Color &modulateColor);

	/** \brief draws an image element, i.e. one image within an image descriptor which may contain multiple images
	 *
	 *  \note  this version of the function accepts no color, so for cases where no fade is going on
	 *         and we don't want to modulate the image's color (which is the case indeed 99% of the time),
	 *         we can skip all the nasty modulation math for a bit of extra efficiency
	 *
	 *  \param element        pointer to the image element to draw
	 * \return success/failure
	 */
	bool _DrawElement(const private_video::ImageElement &element);

	/** \brief helper function to DrawImage() to do the actual work of doing an image
	 *
	 *  \param img static image to draw
	 * \return success/failure
	 */
	bool _DrawStillImage(const StillImage &img);

	/** \brief helper function to DrawImage() to do the actual work of drawing an image
	 *
	 *  \param img static image to draw
	 *  \param color color to modulate image by
	 * \return success/failure
	 */
	bool _DrawStillImage(const StillImage &img, const Color &color);

	/** \brief does the actual work of drawing text
	 *
	 *  \param uText  Pointer to a unicode string holding the text to draw
	 * \return success/failure
	 */
	bool _DrawTextHelper(const uint16 *const uText);

	/** \brief caches glyph info and textures for rendering
	 *
	 *  \param uText  Pointer to a unicode string holding the glyphs to cache
	 *  \param fp     Pointer to the internal FontProperties class representing the font
	 * \return success/failure
	 */
	bool _CacheGlyphs(const uint16 *uText, FontProperties *fp);

	/** \brief generates a texture for a given line
	 *
	 *  \param uText  Pointer to a unicode string holding the text to render
	 *  \param fp     Pointer to the internal FontProperties class representing the font
	 * \return the line or NULL on failure
	 */
	RenderedLine *_GenTexLine(uint16 *line, FontProperties *fp);

	/** \brief retrieves the shadow color based on the current color and shadow style
	 *
	 *  \param fp     Pointer to the internal FontProperties class representing the font
	 * \return the shadow color
	 */
	Color _GetTextShadowColor(FontProperties *fp);

	/** \brief inserts an image into a texture sheet
	 *
	 *  \param image       pointer to the image to insert
	 *  \param loadInfo	   attributes of the image to be inserted
	 * \return a new texsheet with the image in it
	 */
	private_video::TexSheet *_InsertImageInTexSheet(private_video::Image *image, private_video::ImageLoadInfo & loadInfo, bool isStatic);

	/** \brief loads an image
	 *
	 *  \param id  image descriptor to load. Can specify filename, color, width, height, and static as its parameters
	 * \return success/failure
	 */
	bool _LoadImage(StillImage &id);

	/** \brief loads an animated image. Assumes that you have called AddFrame for all the frames.
	 *
	 *  \param id  image descriptor to load
	 * \return success/failure
	 */
	bool _LoadImage(AnimatedImage &id);

	/** \brief does the actual work of loading an image
	 *
	 *  \param id  StillImage of the image to load. May specify a filename, color, width, height, and static
	 * \return success/failure
	 */
	bool _LoadImageHelper(StillImage &id);


	/**
	**/
	bool _LoadMultiImage (private_video::MultiImage &id);

	/** \brief Load raw image data from a file
	 *
	 *  \param filename   Filename of image to load
	 *  \param loadInfo   Returns with the image file attributes and pixels
	 * \return success/failure
	 */
	bool _LoadRawImage(const std::string & filename, private_video::ImageLoadInfo & loadInfo);

	/** \brief Load raw image data from a JPG file
	 *
	 *  \param filename   Filename of image to load
	 *  \param loadInfo   Returns with the image file attributes and pixels
	 * \return success/failure
	 */
	bool _LoadRawImageJpeg(const std::string & filename, private_video::ImageLoadInfo & loadInfo);

	/** \brief Load raw image data from a PNG file
	 *
	 *  \param filename   Filename of image to load
	 *  \param loadInfo   Returns with the image file attributes and pixels
	 * \return success/failure
	 */
	bool _LoadRawImagePng(const std::string & filename, private_video::ImageLoadInfo & loadInfo);

	/*! \brief Saves Raw data in a Png file
	 *	\param file_name Name of the file, without the extension
	 *	\param info Structure of the information to store
	 *	\return True if the process was carried out with no problem, false otherwise
	 */
	bool _SavePng (const std::string& file_name, hoa_video::private_video::ImageLoadInfo &info) const;

	/*! \brief Saves Raw data in a Jpeg file
	 *	\param file_name Name of the file, without the extension
	 *	\param info Structure of the information to store
	 *	\return True if the process was carried out with no problem, false otherwise
	 */
	bool _SaveJpeg (const std::string& file_name, hoa_video::private_video::ImageLoadInfo &info) const;

	/** \brief loop through all currently loaded images and if they belong to the given tex sheet, reload them into it
	 *
	 *  \param texSheet   pointer to the tex sheet whose images we want to load
	 * \return success/failure
	 */
	bool _ReloadImagesToSheet(private_video::TexSheet *texSheet);

	/** \brief removes the image from the STL map with the same pointer as the one passed in. Returns false on failure
	 *
	 *  \param imageToRemove   pointer to the image we want to remove
	 * \return success/failure
	 */

	bool _RemoveImage(private_video::Image *imageToRemove);

	/** \brief Converts a color image to a grayscale one;
	 * Converts a colored image in a grayscale one. Actually, it converts not an image, but
	 * an ImageLoadInfo structure. This is used internally when creating grayscale images.
	 * \param src Information of a color image
	 * \param dst ImageLoadInfo struct where the grayscale image will be stored
	 */
	void _ConvertImageToGrayscale(const private_video::ImageLoadInfo& src, private_video::ImageLoadInfo &dst) const;


	/** \brief Converts a RGBA buffer to a RGB one
	 * \param src Information of a RGBA buffer
	 * \param dst ImageLoadInfo struct where the RGB buffer will be stored
	 */
	void _RGBAToRGB (const private_video::ImageLoadInfo& src, private_video::ImageLoadInfo &dst) const;


	//! \brief Pass a texture (video memory) to a system memory buffer
	void _GetBufferFromTexture (hoa_video::private_video::ImageLoadInfo& buffer, hoa_video::private_video::TexSheet* texture) const;

	//! \brief Pass an image (video memory) to a system memory buffer
	void _GetBufferFromImage (hoa_video::private_video::ImageLoadInfo& buffer, hoa_video::private_video::Image* img) const;

	/** \brief removes a texture sheet from our vector of sheets and deletes it
	 *
	 *  \param sheetToRemove  pointer to the sheet we want to remove
	 * \return success/failure
	 */
	bool _RemoveSheet(private_video::TexSheet *sheetToRemove);

	/**
	 *  \brief rounds a force value to the nearest integer. Rounding is based on probability. For example the number 2.85 has an 85% chance of rounding to 3 and a 15% chance of rounding to 2
	 *
	 *  \param force  The force to round
	 * \return the rounded force value
	 */
	float _RoundForce(float force);   // rounds a force value

	/**
	 *  \brief restores coord system, draw flags, and transformations
	 */
	void _PopContext();

	/**
	 *  \brief saves coord system, draw flags, and transformations
	 */
	void _PushContext();

	/**
	 *  \brief saves temporary textures to disk, in other words, textures which were not
	 *         loaded to a file. This is used when the GL context is being destroyed,
	 *         perhaps because we are switching from windowed to fullscreen. So, we need
	 *         to save all textures to disk so we can reload them later.
	 * \return success/failure
	 */
	bool _SaveTempTextures();

	/**
	* \brief takes an x value and converts it into screen coordinates
	* \return the converted value
	*/
	int32 _ScreenCoordX(float x);

	/**
	* \brief takes an x value and converts it into screen coordinates
	* \return the converted value
	*/
	int32 _ScreenCoordY(float y);

	/**
	 *  \brief updates the shaking effect
	 *
	 *  \param frameTime  elapsed time for the current rendering frame
	 */
	void  _UpdateShake(int32 frameTime);

	/**
	 *  \brief function solely for debugging, which shows number of texture switches made during a frame,
	 *         and other statistics useful for performance tweaking, etc.
	 * \return success/failure
	 */
	bool _DEBUG_ShowAdvancedStats();

	/**
	 *  \brief function solely for debugging, which displays the currently selected texture sheet. By using DEBUG_NextTexSheet() and DEBUG_PrevTexSheet(), you can change the current texture sheet so the sheet shown by this function cycles through all currently loaded sheets.
	 * \return success/failure
	 */
	bool _DEBUG_ShowTexSheet();

	//! \brief the current draw cursor position
	float _x;
	float _y;
}; // class GameVideo

}  // namespace hoa_video

#endif // __VIDEO_HEADER__
