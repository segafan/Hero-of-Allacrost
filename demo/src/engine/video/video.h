///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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

#include <set>

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
extern GameVideo* VideoManager;

//! \brief Determines whether the code in the hoa_video namespace should print
extern bool VIDEO_DEBUG;

/** \brief Creates a random float between a and b.
*** \param a lower bound of random selection
*** \param b upper bound of random selection
*** \return the randomly generated float
**/
float RandomFloat(float a, float b);

/** \brief Rotates a point (x,y) around the origin (0,0), by angle radians
*** \param x x coordinate of point to rotate
*** \param y y coordinate of point to rotate
*** \param angle amount to rotate by (in radians)
**/
void RotatePoint(float &x, float &y, float angle);

/** \brief linearly interpolation, returns a value which is (alpha*100) percent between initial and final
***
***  \param alpha    controls the linear interpolation
***  \param initial  initial value
***  \param final    final value
*** \return the linear interpolated value
**/
float Lerp(float alpha, float initial, float final);

//! animation frame period: how many milliseconds 1 frame of animation lasts for
const int32 VIDEO_ANIMATION_FRAME_PERIOD = 10;

//! \brief Draw flags to control x and y alignment, flipping, and texture blending.
enum VIDEO_DRAW_FLAGS {
	VIDEO_DRAW_FLAGS_INVALID = -1,

	//@{
	//! X alignment flags
	VIDEO_X_LEFT = 1, VIDEO_X_CENTER = 2, VIDEO_X_RIGHT = 3,
	//@}

	//@{
	//! Y alignment flags
	VIDEO_Y_TOP = 4, VIDEO_Y_CENTER = 5, VIDEO_Y_BOTTOM = 6,
	//@}

	//@{
	//! X flip flags
	VIDEO_X_FLIP = 7, VIDEO_X_NOFLIP = 8,
	//@}

	//@{
	//! Y flip flags
	VIDEO_Y_FLIP = 9, VIDEO_Y_NOFLIP = 10,
	//@}

	//@{
	//! Texture blending flags
	VIDEO_NO_BLEND = 11, VIDEO_BLEND = 12, VIDEO_BLEND_ADD = 13,
	//@}

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

//! \brief Standard natural resolution sizes for allacrost images.
enum {
	VIDEO_STANDARD_RES_WIDTH  = 1024,
	VIDEO_STANDARD_RES_HEIGHT = 768
};

/** ****************************************************************************
*** \brief Manages all the video operations and serves as the API to the video engine.
***
*** This is one of the largest classes in the Allacrost engine. Because it is so
*** large, the implementation of many of the methods for this class are split up
*** into multiple .cpp source files in the video code directory.
*** *****************************************************************************/
class GameVideo : public hoa_utils::Singleton<GameVideo> {
	friend class hoa_utils::Singleton<GameVideo>;
	friend class TextBox;
	friend class OptionBox;
	friend class MenuWindow;
	friend class private_video::GUIElement;
	friend class private_video::GUISupervisor;
	friend class private_video::FixedTexMemMgr;
	friend class private_video::VariableTexMemMgr;
	friend class private_video::TexSheet;
	friend class private_video::ParticleSystem;
	friend class StillImage;
	friend class TImage;
	friend class RenderedText;

public:
	~GameVideo();

	bool SingletonInitialize();


	//-- General --------------------------------------------------------------

	/** \brief Clears the contents of the current screen.
	***
	*** This method should be called at the beginning of every frame, before any
	*** draw operations are performed. Note that it only clears the color buffer,
	*** not any of the other OpenGL buffers.
	**/
	void Clear();

	/** \brief Clears the screen to a specific color.
	*** \param c The color to set the cleared screen to.
	**/
	void Clear(const Color &c);

	/** \brief call at end of every frame
	*** \param frame_time The number of milliseconds that have passed since the last frame.
	**/
	void Display(int32 frame_time);


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
	void SetTarget(VIDEO_TARGET target);

	//-- Video settings -------------------------------------------------------

	//! NOTE: when you modify video setting, you must call ApplySettings()
	//!       to actually apply them

	/** \brief sets the current resolution to the given width and height
	 *  \param width new screen width
	 *  \param height new screen height
	 *
	 *  \note  you must call ApplySettings() to actually apply the change
	 */
	void SetResolution(uint32 width, uint32 height)
		{ _temp_width = width; _temp_height = height; }

	/** \brief returns width, (whatever was set with SetResolution)
	 * \return width of the screen
	 */
	int32 GetWidth() const
		{ return _width; }

	/** \brief returns height, (whatever was set with SetResolution)
	 * \return height of the screen
	 */
	int32 GetHeight() const
		{ return _height; }

	/** \brief return a reference to the coordinates system
	 *  \return Reference to the coordinates system
	 */
	const CoordSys& GetCoordSys() const
		{ return _coord_sys; }

	/** \brief returns true if game is in fullscreen mode
	 * \return true if in fullscreen mode, false if in windowed mode
	 */
	bool IsFullscreen() const
		{ return _fullscreen; }

	/** \brief Return the pixel size expressed coordinate system units
	 *  \return Pixel size expressed in cordinate system units
	 *  \param x Horizontal resolution
	 *  \param x Vertical resolution
	 */
	void GetPixelSize(double &x, double &y)
	{
		x = (_coord_sys.GetRight() - _coord_sys.GetLeft()) / _width;
		y = (_coord_sys.GetTop() - _coord_sys.GetBottom()) / _height;
	}

	/** \brief sets the game to fullscreen or windowed depending on whether
	 *         true or false is passed
	 *
	 *  \param fullscreen set to true if you want fullscreen, false for windowed.
	 *  \note  you must call ApplySettings() to actually apply the change
	 */
	void SetFullscreen(bool fullscreen)
		{ _temp_fullscreen = fullscreen; }

	/** \brief toggles fullscreen on and off
	 * \return success/failure
	 *  \note  you must call ApplySettings() to actually apply the change
	 */
	void ToggleFullscreen()
		{ SetFullscreen(!_temp_fullscreen); }

	/** \brief applies any changes to video settings like resolution and
	 *         fullscreen. If the changes fail, then this function returns
	 *         false, and the video settings are reset to whatever the last
	 *         working setting was.
	 * \return success/failure
	 */
	bool ApplySettings();

	//-- Coordinate system / viewport  ------------------------------------------

	/** \brief Sets the viewport, i.e. the area of the screen that gets drawn to.
	*** \param left Left border of screen
	*** \param right Right border of screen
	*** \param top Top border of screen
	*** \param bottom Bottom border of screen
	***
	*** The arguments are percentages, and the default viewport is (0, 100,
	*** 0, 100), which covers the entire screen.
	**/
	void SetViewport(float left, float right, float bottom, float top);

	/** \brief Sets the coordinate system to use. Default is (0, 1024, 0, 768)
	 *  \param left   left border of screen
	 *  \param right  right border of screen
	 *  \param top    top border of screen
	 *  \param bottom bottom border of screen
	 */
	void SetCoordSys(float left, float right, float bottom, float top)
		{ SetCoordSys(CoordSys(left, right, bottom, top)); }

	/** \brief Sets the coordinate system. Default is (0, 1024, 0, 768)
	*** \param coordinate_system the coordinate system you want to set to
	**/
	void SetCoordSys(const CoordSys& coordinate_system);

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
		{ return _scissor_enabled; }

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
	void SetScissorRect(const ScreenRect &rect);

	/** \brief returns scissor rect
	 * \return the scissor rectangle
	 */
	ScreenRect GetScissorRect()
		{ return _scissor_rect; }

	/** \brief converts coordinates given relative to the current coord sys into
	 *         "screen coordinates", which are in pixel units with (0,0) as the
	 *         top left and (w-1, h-1) as the lower-right, where w and h are the
	 *         dimensions of the screen
	 * \return the screen rectangle
	 */

	ScreenRect CalculateScreenRect(float left, float right, float bottom, float top);

	//-- Transformations ------------------------------------------------------

	/** \brief Saves relevant state of the video engine on to an internal stack
	*** The contents saved include: all draw flags, the coordinate system,
	*** the scissor rectangle, the viewport, etc.). This is useful for safety
	*** purposes between two major parts of code to ensure that one part doesn't
	*** inadvertently affect the other.
	***
	*** \note This is a very expensive function call. If you only need to push
	*** the current transformation, you should use PushMatrix() and PopMatrix().
	***
	*** \note The size of the stack is small (around 32 entries), so you should
	*** try and limit the maximum number of pushed state entries so that this
	*** limit is not exceeded
	**/
	void PushState();

	//! \brief Restores the most recently pushed video engine state
	void PopState ();


	/** \brief saves current modelview transformation on to the stack. In English,
	 *         that means the combined result of calls to Move/MoveRelative/Scale/Rotate
	 *   \note Assumes that glMatrixMode(GL_MODELVIEW) has been called
	 */
	void PushMatrix()
		{ glPushMatrix(); }

	/** \brief pops the modelview transformation from the stack.
	*** \note Assumes that glMatrixMode(GL_MODELVIEW) has been called
	 */
	void PopMatrix()
		{ glPopMatrix(); }

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
	void GetDrawPosition(float &x, float &y) const
		{ x = _x; y = _y; }

	/** \brief rotates images counterclockwise by 'angle' radians
	 *  \param angle how many radians to rotate by
	 *  \note This function should NOT be used unless you understand how transformation
	 *        matrices work in OpenGL.
	 */
	void Rotate (float angle);

	/** \brief after you call this, subsequent calls to DrawImage() result in
	 *         a scaled image
	 *
	 *  \param x pass 1.0 for normal horizontal scaling, 2.0 for double
	 *                  scaling, etc.
	 *  \param y same, except vertical scaling
	 *
	 *  \note This function should NOT be used unless you understand how transformation
	 *        matrices work in OpenGL.
	 */

	void Scale(float x, float y);

	/** \brief sets OpenGL transform to contents of 4x4 matrix (16 values)
	 *  \param m of 16 float values forming a 4x4 transformation matrix
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
	*** \param font_name  The referred name of the loaded font (e.g. "courier24").
	*** \return A pointer to the FontProperties object with the requested data, or NULL if an error occurred.
	**/
	FontProperties* GetFontProperties(const std::string &font_name);

	/** \brief calculates the width of the given text if it were rendered with the given font
	 *         If an invalid font name is passed, returns -1
	 *
	 *  \param font_name  the font to use
	 *  \param text      the text string in unicode
	 * \return width of the given text
	 */
	int32 CalculateTextWidth(const std::string &font_name, const hoa_utils::ustring &text);


	/** \brief calculates the width of the given text if it were rendered with the given font
	 *         If an invalid font name is passed, returns -1
	 *
	 *  \param font_name  the font to use
	 *  \param text      the text string in multi-byte character format
	 * \return width of the given text
	 */
	int32 CalculateTextWidth(const std::string &font_name, const std::string  &text);

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
		{ _current_text_color = color; }

	/** \brief sets the shadow offset to use for a given font. By default, all font shadows
	 *         are slightly to the right and to the bottom of the text, by an offset of
	 *         fontHeight / 8. That doesn't always look good though, so use this function
	 *         to adjust it if you want.
	 *
	 *  \param font_name  label of the font you want to set the shadow offset for
	 *  \param x         x offset in pixels (based on 1024x768)
	 * \return success/failure
	 */
	bool SetFontShadowXOffset(const std::string &font_name, int32 x);

	/** \brief sets the shadow offset to use for a given font. By default, all font shadows
	 *         are slightly to the right and to the bottom of the text, by an offset of
	 *         fontHeight / 8. That doesn't always look good though, so use this function
	 *         to adjust it if you want.
	 *
	 *  \param font_name  label of the font you want to set the shadow offset for
	 *  \param y         y offset in pixels (based on 1024x768)
	 * \return success/failure
	 */
	bool SetFontShadowYOffset(const std::string &font_name, int32 y);

	/** \brief sets the shadow style to use for the given font
	 *
	 *  \param font_name  label of the font you want to set the shadow style for
	 *  \param style     the shadow style you want (e.g. VIDEO_TEXT_SHADOW_BLACK)
	 * \return success/failure
	 */
	bool SetFontShadowStyle(const std::string &font_name, TEXT_SHADOW_STYLE style);

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
	 *  \param particle_effect_filename - file containing the particle effect definition
	 *  \param x - X coordinate of the effect
	 *  \param y - Y coordinate of the effect
	 *  \param reload - reload the effect from file if it already exists
	 *  \return id corresponding to the loaded particle effect
	 *  \note  set the reload parameter to true to reload the effect definition file
	 *         every time the effect is played. This is useful if you are working on an
	 *         effect and want to see how it looks. When we actually release the game,
	 *         reload should be false since it adds some cost to the loading
	 */
	ParticleEffectID AddParticleEffect(const std::string &particle_effect_filename, float x, float y, bool reload=false);

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

	/*! \brief Get information of an image file
	 *	\param file_name Name of the file, without the extension
	 *	\param rows Rows of the image
	 *	\param cols Columns of the image
	 *	\param bpp Bits per pixel of the image
	 *	\return True if the process was carried out with no problem, false otherwise
	 */
	bool GetImageInfo (const std::string& file_name, uint32 &rows, uint32& cols, uint32& bpp);

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
	 *	that on separate StillImage objects. It uses the number of stored elements as input parameters.
	 *  \param images  Vector of StillImages where the cut images will be loaded.
	 *	\param filename Name of the file.
	 *	\param rows Number of rows of sub-images in the MultiImage.
	 *	\param cols Number of columns of sub-images in the MultiImage.
	 *	\return success/failure
	 */
	bool LoadMultiImageFromNumberElements(std::vector<StillImage> &images, const std::string &filename, const uint32 rows, const uint32 cols);

	/** \brief Loads a MultiImage in a vector of StillImages.
	 *	This function loads an image and cut it in pieces, loading each of
	 *	that on separate StillImage objects. It uses the size of the stored elements as input parameters.
	 *  \param images  Vector of StillImages where the cut images will be loaded.
	 *	\param filename Name of the file.
	 *	\param width Width of a stored element.
	 *	\param height Height of a stored element.
	 *	\return success/failure
	 */
	bool LoadMultiImageFromElementsSize(std::vector<StillImage> &images, const std::string &filename, const uint32 width, const uint32 height);

	/** \brief Loads a MultiImage in an AnimatedImage as frames.
	 *	This function loads an image and cut it in pieces, loading each of
	 *	that on frames of an AnimatedImage.
	 *  \param image AnimatedImage where the cut images will be loaded.
	 *	\param file_name Name of the file.
	 *	\param rows Number of rows of sub-images in the MultiImage.
	 *	\param cols Number of columns of sub-images in the MultiImage.
	 *	\return success/failure
	 */
	bool LoadAnimatedImageFromNumberElements(AnimatedImage &image, const std::string &file_name, const uint32 rows, const uint32 cols);

	/** \brief Loads a MultiImage in an AnimatedImage as frames.
	 *	This function loads an image and cut it in pieces, loading each of
	 *	that on frames of an AnimatedImage.
	 *  \param image AnimatedImage where the cut images will be loaded.
	 *	\param file_name Name of the file.
	 *	\param rows Number of rows of sub-images in the MultiImage.
	 *	\param cols Number of columns of sub-images in the MultiImage.
	 *	\return success/failure
	 */
	bool LoadAnimatedImageFromElementsSize(AnimatedImage &image, const std::string &file_name, const uint32 rows, const uint32 cols);

	//! \brief Saves a vector of images in a single file.
	/*	This function stores a vector of images as a single image. This is useful for creating multiimage 
	 *	images. The image can be stored in JPEG or PNG, which is decided by the filename.
	 *  \param image Vector of images to save
	 *	\param file_name Name of the file.
	 *	\param rows Number of rows of sub-images in the MultiImage.
	 *	\param cols Number of columns of sub-images in the MultiImage.
	 *	\return success/failure
	 */
	bool SaveImage (const std::string &file_name, const std::vector<StillImage*> &image, const uint32 rows, const uint32 columns) const;

	//! \brief Saves a StillImage into a file.
	/*	The image can be stored in JPEG or PNG, which is decided by the filename.
	 *  \param image StillImage to store.
	 *	\param file_name Name of the file.
	 *	\return success/failure
	 */
	bool SaveImage (const std::string &file_name, const StillImage &image) const;

	//! \brief Saves an AnimatedImage into a file.
	/*	The image can be stored in JPEG or PNG, which is decided by the filename.
	 *  It will be stored as a multiimage, with each frame aligned in one row and as many columns as frames.
	 *  \param image AnimatedImage to store.
	 *	\param file_name Name of the file.
	 *	\return success/failure
	 */
	bool SaveImage (const std::string &file_name, const AnimatedImage &image) const;


	/** \brief decreases ref count of an image (static or animated)
	*** \param id  image descriptor to decrease the reference count of
	**/
	void DeleteImage(ImageDescriptor &id);

	/** \brief captures the contents of the screen and saves it to an image
	 *         descriptor
	 *
	 *  \param id  image descriptor to capture to
	 * \return success/failure
	 */
	bool CaptureScreen(StillImage &id);

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
	 * \param first_flag the first of many draw flags
	 * \param .. specify as many draw flags as you want.  Terminate with a 0.
	 */
	void SetDrawFlags(int32 first_flag, ...);

	/** \brief draws an image which is modulated by the scene's light color
	 *
	 *  \param id  image descriptor to draw (either StillImage or AnimatedImage)
	 */
	void DrawImage(const ImageDescriptor &id);

	/** \brief draws an image which is modulated by a custom color
	 *
	 *  \param id  image descriptor to draw (either StillImage or AnimatedImage)
	 *  \param color Color used for modulating the image
	 */
	void DrawImage(const ImageDescriptor &id, const Color &color);

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

	/** \name Methods for loading of menu skins
	***
	*** These methods all attempt to load a menu skin. The differences between these implementations are
	*** whether the skin includes a background image, single background color, multiple background colors,
	*** or some combination thereof.
	***
	*** \param skin_name The name that will be used to refer to the skin after it is successfully loaded
	*** \param border_image The filename for the multi-image that contains the menu's border images
	*** \param background_image The filename for the skin's background image (optional)
	*** \param top_left Sets the background color for the top left portion of the skin
	*** \param top_right Sets the background color for the top right portion of the skin
	*** \param bottom_left Sets the background color for the bottom left portion of the skin
	*** \param bottom_right Sets the background color for the bottom right portion of the skin
	*** \param make_default If this skin should be the default menu skin to be used, set this argument to true
	*** \return True if the skin was loaded successfully, or false in case of an error
	***
	*** A few notes about this function:
	*** - If you set a background image, any background colors will not be visible unless the background image has some transparency
	*** - If no other menu skins are loaded when this function is called, the default skin will automatically be set to this skin,
	***   regardless of the value of the make_default parameter.
	**/
	//@{
	//! \brief A background image with no background colors
	bool LoadMenuSkin(std::string skin_name, std::string border_image, std::string background_image, bool make_default = false);

	//! \brief No background image with a single background color
	bool LoadMenuSkin(std::string skin_name, std::string border_image, Color background_color, bool make_default = false);

	//! \brief No background image with multiple background colors
	bool LoadMenuSkin(std::string skin_name, std::string border_image, Color top_left, Color top_right,
		Color bottom_left, Color bottom_right, bool make_default = false);

	//! \brief A background image with a single background color
	bool LoadMenuSkin(std::string skin_name, std::string border_image, std::string background_image,
		Color background_color, bool make_default = false);

	//! \brief A background image with multiple background colors
	bool LoadMenuSkin(std::string skin_name, std::string border_image, std::string background_image,
		Color top_left, Color top_right, Color bottom_left, Color bottom_right, bool make_default = false);
	//@}

	//! \brief Returns true if there is a menu skin avialable corresponding to the argument name
	bool IsMenuSkinAvailable(std::string& skin_name) const
		{ if (private_video::GUIManager->GetMenuSkin(skin_name) == NULL) return false; else return true; }

	/** \brief Sets the default menu skin to use
	*** \param skin_name The name of the already loaded menu skin that should be made the default skin
	***
	*** If the skin_name does not refer to a valid skin, a warning message will be printed and no change
	*** will occur.
	*** \note This method will <b>not</b> change the skins of any existing menu windows.
	**/
	void SetDefaultMenuSkin(std::string& skin_name)
		{ private_video::GUIManager->SetDefaultMenuSkin(skin_name); }

	/** \brief Deletes a menu skin that has been loaded
	*** \param skin_name The name of the loaded menu skin that should be removed
	***
	*** Before you call this function, you must delete any and all MenuWindow objects which make use of this skin,
	*** or change the skin used by those objects. Failing to do so will result in a warning message being printed
	*** and the skin will not be deleted. The function will also print a warning message and exit if it could not
	*** find a skin referred to by the argument name.
	**/
	void DeleteMenuSkin(std::string& skin_name)
		{ private_video::GUIManager->DeleteMenuSkin(skin_name); }

	//-- Lighting and fog -----------------------------------------------------

	/** \brief turn on the ligt color for the scene
	 * \param color the light color to use
	 */
	void EnableSceneLighting(const Color& color);

	/** \brief disables scene lighting
	*/
	void DisableSceneLighting();

	/** \brief returns the scene lighting color
	 * \return the light color used in the scene
	 */
	const Color& GetSceneLightingColor()
		{ return _light_color; }

	/** \brief Initializes and enables fog parameters
	*** \param color The color of the fog (the alpha value should be 1.0)
	*** \param intensity Intensity of fog from 0.0f to 1.0f. 0.0f will disable fog.
	**/
	void EnableFog(const Color& color, float intensity);

	//! \brief Disables the fog effect
	void DisableFog();

	/** \brief draws a halo at the given spot
	 *
	 *  \param id    image descriptor for the halo image
	 *  \param x     x coordinate of halo
	 *  \param y     y coordinate of halo
	 *  \param color color of halo
	 */
	void DrawHalo(const StillImage &id, float x, float y, const Color &color = Color(1.0f, 1.0f, 1.0f, 1.0f));

	/** \brief draws a real light at the given spot
	 *
	 *  \param id    image descriptor for the light mask
	 *  \param x     x coordinate of light
	 *  \param y     y coordinate of light
	 *  \param color color of light
	 */
	void DrawLight(const StillImage &id, float x, float y, const Color &color = Color(1.0f, 1.0f, 1.0f, 1.0f));

	//! \brief Prepares the 
	void EnablePointLights();

	/**
	* \brief disables point lights
	*/
	void DisablePointLights();

	/** \brief call after all map images are drawn to apply lighting. All
	 *         menu and text rendering should occur AFTER this call, so that
	 *         they are not affected by lighting.
	 */
	void ApplyLightingOverlay();

	//-- Overlays / lightning -------------------------------------------------------

	/** \brief draws a full screen overlay of the given color
	 *  \note  This is very slow, so use sparingly!
	 */
	void DrawFullscreenOverlay(const Color& color);

	/** \brief call to create lightning effect
	 *  \param lit_file a .lit file which contains lightning intensities stored
	 *                 as bytes (0-255).
	 * \return success/failure
	 */
	bool MakeLightning(const std::string& lit_file);

	/** \brief call this every frame to draw any lightning effects. You should make
	 *         sure to place this call in an appropriate spot. In particular, you should
	 *         draw the lightning before drawing the GUI. The lightning is drawn by
	 *         using a fullscreen overlay.
	 */
	void DrawLightning();

	//-- Fading ---------------------------------------------------------------

	/** \brief Begins a screen fade.
	*** \param color The color to fade the screen to
	*** \param time The fading process will take this number of milliseconds
	**/
	void FadeScreen(const Color& color, uint32 time)
		{ _fader.BeginFade(color, time); }

	//! \brief Returns true if a screen fade is currently in progress
	bool IsFading()
		{ return _fader.IsFading(); }

	//-- Screen shaking -------------------------------------------------------

	/** \brief Adds a new shaking effect to the screen
	***
	*** \param force The initial force of the shake
	*** \param falloff_time The number of milliseconds that the effect should last for. 0 indicates infinite time.
	*** \param falloff_method Specifies the method of falloff. The default is VIDEO_FALLOFF_NONE.
	*** \note If you want to manually control when the shaking stops, set the falloff_time to zero
	*** and the falloff_method to VIDEO_FALLOFF_NONE.
	**/
	void ShakeScreen(float force, uint32 falloff_time, ShakeFalloff falloff_method = VIDEO_FALLOFF_NONE);

	//! \brief Terminates all current screen shake effects
	void StopShaking()
		{ _shake_forces.clear(); _x_shake = 0.0f; _y_shake = 0.0f; }

	//! \brief Returns true if the screen is shaking
	bool IsShaking()
		{ return (_shake_forces.empty() == false); }

	//-- Miscellaneous --------------------------------------------------------

	/** \brief Sets a new gamma value using SDL_SetGamma()
	 *
	 *  \param value        Gamma value of 1.0f is the default value
	 */
	void SetGamma(float value);

	/** \brief Returns the gamma value
	 * \return the gamma value
	 */
	float GetGamma() const
		{ return _gamma_value; }

	/** \brief Updates the FPS counter and draws the current average FPS to the screen
	*** The number of milliseconds that have expired since the last frame was drawn
	**/
	void DrawFPS(uint32 frame_time);

	/** \brief toggles the FPS display (on by default)
	 */
	void ToggleFPS()
		{ _fps_display = !_fps_display; }

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
	 * \param x_step width of grid squares
	 * \param y_step height of grid squares
	 * \param c color of the grid
	 */
	void DrawGrid(float x, float y, float x_step, float y_step, const Color &c);

	/** \brief Draws a solid rectangle of a given color.
	 * Draws a solid rectangle of a given color. For that, the lower-left corner
	 * of the rectangle has to be specified, and also its size. The parameters depends
	 * on the current Coordinate System.
	 * \param width Width of the rectangle.
	 * \param height Height of the rectangle.
	 * \param color Color to paint the rectangle.
	 */
	void DrawRectangle(float width, float height, const Color& color);

	/** \brief Takes a screenshot and saves the image to a file
	*** \param filename The name of the file, if any, to save the screenshot as. Default is "screenshot.jpg"
	**/
	void MakeScreenshot(const std::string& filename = "screenshot.jpg");

	/** \brief toggles advanced information display for video engine, shows
	 *         things like number of texture switches per frame, etc.
	 */
	void ToggleAdvancedDisplay()
		{ _advanced_display = !_advanced_display; }

	/** \brief sets the default cursor to the image in the given filename
	* \param cursor_image_filename file containing the cursor image
	*/
	bool SetDefaultCursor(const std::string &cursor_image_filename);

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
	 * \param tex_index Whether main or shadow texture
	 */
	bool Draw(const RenderedLine& line, int32 tex_index);

	/** \brief Renders the given string to a drawable object
	 * \param txt The string to render
	 */
	RenderedString* RenderText(const hoa_utils::ustring& txt);

	/** Renders a given unicode string and TextStyle to a pixel array
	 * \param string The ustring to render
	 * \param style  The text style to render
	 * \param buffer The pixel array to render to
	 */
	bool _RenderText(hoa_utils::ustring& string, TextStyle& style, private_video::ImageLoadInfo& buffer);

private:
	GameVideo();

	//-- Private variables ----------------------------------------------------

	//! particle manager, does dirty work of managing particle effects
	private_video::ParticleManager _particle_manager;

	//! \brief The type of window target that the video manager will operate on (SDL window or QT widget)
	VIDEO_TARGET _target;

	// draw flags

	//! blend flag which specifies normal alpha blending
	int8 _blend;

	//! x align flag which tells if images should be left, center, or right aligned
	int8 _x_align;

	//! y align flag which tells if images should be top, center, or bottom aligned
	int8 _y_align;

	//! x flip flag. true if images should be flipped horizontally
	int8 _x_flip;

	//! y flip flag. true if images should be flipped vertically
	int8 _y_flip;

	//! eight character name for temp files that increments every time you create a new one so they are always unique
	char _next_temp_file[9];

	//! current coordinate system
	CoordSys    _coord_sys;

	//! current viewport
	ScreenRect _viewport;

	//! current scissor rectangle
	ScreenRect _scissor_rect;

	//! is scissoring enabled or not
	bool _scissor_enabled;

	//! fader class which implements screen fading
	private_video::ScreenFader _fader;

	//! advanced display flag. If true, info about the video engine is shown on screen
	bool _advanced_display;

	//! fps display flag. If true, FPS is displayed
	bool _fps_display;

	//! current debug texture sheet
	int32 _current_debug_TexSheet;

	//! keep track of number of texture switches per frame
	int32 _num_tex_switches;

	//! keep track of number of draw calls per frame
	int32 _num_draw_calls;

	//! true if real lights are enabled
	bool _uses_lights;

	//! lighting overlay texture
	GLuint _light_overlay;

	//! X offset to shake the screen by (if any)
	float  _x_shake;
	
	//! Y offset to shake the screen by (if any)
	float _y_shake;

	//! Current gamma value
	float _gamma_value;

	//! current shake forces affecting screen
	std::list<private_video::ShakeForce> _shake_forces;

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
	GLuint _last_tex_id;

	//! current font name
	std::string _current_font;

	//! current text color
	Color _current_text_color;

	//! image which is to be used as the cursor
	StillImage _default_menu_cursor;

	//! Image used for rendering rectangles
	StillImage _rectangle_image;

	//! if true, text shadow effect is enabled
	bool _text_shadow;

	//! current fog color (set by SetFog())
	Color _fog_color;

	//! current fog intensity (set by SetFog())
	float _fog_intensity;

	//! current scene lighting color (essentially just modulates vertex colors of all the images)
	Color _light_color;

	//! true if a lightning effect is active
	bool  _lightning_active;

	//! current time of lightning effect (time since it started)
	int32 _lightning_current_time;

	//! how many milliseconds to do the lightning effect for
	int32 _lightning_end_time;

	//! intensity data for lightning effect
	std::vector <float> _lightning_data;

	//! counter to keep track of milliseconds since game started for animations
	int32 _animation_counter;

	//! keeps track of the number of frames animations should increment by for the current frame
	int32 _current_frame_diff;

	//! STL map containing all the images currently being managed by the video engine
	std::map<std::string, private_video::Image*> _images;

	//! STL map containing all the text images currently being managed by the video engine
	std::set<hoa_video::TImage*> _t_images;

	//! vector containing all texture sheets currently being managed by the video engine
	std::vector<private_video::TexSheet*> _tex_sheets;

	//! STL map containing properties for each font (includeing TTF_Font *)
	std::map<std::string, FontProperties*> _font_map;

	//! STL map containing all loaded particle effect definitions
	std::map<std::string, ParticleEffectDef*> _particle_effect_defs;

	//! stack containing context, i.e. draw flags plus coord sys. Context is pushed and popped by any GameVideo functions that clobber these settings
	std::stack<private_video::Context> _context_stack;

	//-- Private methods ------------------------------------------------------

	/** \brief wraps a call to glBindTexture(), except it adds checking to eliminate redundant texture binding. Redundancy checks are already implemented by most drivers, but this is a double check "just in case"
	 *
	 *  \param tex_ID   integer handle to the OpenGL texture
	 * \return success/failure
	 */
	bool _BindTexture(GLuint tex_ID);

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
	 *  \param is_static if true, this texture sheet is meant to manage images which are not expected to be loaded and unloaded very often
	 * \return the newly created texsheet
	 */
	private_video::TexSheet *_CreateTexSheet(int32 width, int32 height, private_video::TexSheetType type, bool is_static);

	/** \brief wraps a call to glDeleteTextures(), except it adds some checking related to eliminating redundant texture binding.
	 *
	 *  \param tex_ID   integer handle to the OpenGL texture
	 * \return success/failure
	 */
	bool _DeleteTexture(GLuint tex_ID);

	/** \brief decreases the reference count of an image
	*** \param image  pointer to image
	*** \return success/failure
	**/
	bool _DeleteImage(private_video::BaseImage *const image);

	/** \brief decreases ref count of an image
	*** \param id  image descriptor to decrease the reference count of
	**/
	void _DeleteImage(StillImage &id);

	/** \brief decreases ref count of an animated image
	*** \param id  image descriptor to decrease the reference count of
	**/
	void _DeleteImage(AnimatedImage &id);

	/** \brief deletes the temporary textures from the "temp" folder that were saved
	 *         by _SaveTempTextures()
	 * \return success/failure
	 */
	bool _DeleteTempTextures()
		{ return hoa_utils::CleanDirectory("img\\temp"); }

	/** \brief draws an image element, i.e. one image within an image descriptor which may contain multiple images
	 *
	 *  \note  this function takes an array of vertex colors. in cases where
	 *         modulation is required, copy the element.color array and modulate
	 *         it, else just pass element.color as this argument.
	 *
	 *  \param element        pointer to the image element to draw
	 *  \param color_array    pointer to an 4 element array of vertex colors,
	 *                        modulated if necessary
	 */
	void _DrawElement(const private_video::BaseImageElement &element, const Color *color_array);

	/** \brief helper function to DrawImage() to do the actual work of doing an image
	 *
	 *  \param img static image to draw
	 */
	void _DrawStillImage(const ImageListDescriptor &img);

	/** \brief helper function to DrawImage() to do the actual work of drawing an image
	 *
	 *  \param img static image to draw
	 *  \param color color to modulate image by
	 */
	void _DrawStillImage(const ImageListDescriptor &img, const Color &color);

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
	 *  \param line  Pointer to a unicode string holding the text to render
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
	 *  \param image     pointer to the image to insert
	 *  \param load_info attributes of the image to be inserted
	 *  \param is_static Wether an image is static or not  
	 * \return a new texsheet with the image in it
	 */
	private_video::TexSheet *_InsertImageInTexSheet(private_video::BaseImage *image, private_video::ImageLoadInfo & load_info, bool is_static);

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

	/** \brief Adds a TImage to the internal set
	 *
	 * \param timg The pointer to add
	 *
	 * \return True if successful, false if already present
	 */
	bool _AddTImage(TImage *timg);

	/** \brief Gets a TImage from the internal set
	 *
	 * \param pntr The pointer in the set.
	 *
	 * \return The pointer itself if successful, or NULL
	 */
	TImage *_GetTImage(TImage *pntr);


	/** \brief Decrements the reference count of a TImage
	 *   and deletes from set if necessary.
	 *
	 *  \param img The pointer (and key of the set)
	 */
	bool _RemoveImage(TImage *img);


	/** \brief Decrements the reference count of an Image
	 *   of the base class and deletes from storage if necessary.
	 *
	 *  \param base_img The pointer (and key of the set/map)
	 */
	bool _RemoveImage(private_video::BaseImage *base_image);

	/** \brief Loads an image file in several StillImages
	 *  \param images Vector of StillImages to be loaded
	 *  \param file_name Name of the image file to read
	 *  \param rows Number of rows of StillImages
	 *  \param cols Number of columns of StillImages
	 */
	bool _LoadMultiImage (std::vector <StillImage>& images, const std::string &file_name, const uint32& rows, const uint32& cols);

	/** \brief Load raw image data from a file
	 *
	 *  \param file_name   Filename of image to load
	 *  \param load_info   Returns with the image file attributes and pixels
	 * \return success/failure
	 */
	bool _LoadRawImage(const std::string & file_name, private_video::ImageLoadInfo & load_info);

	/** \brief Load raw image data from a JPG file
	 *
	 *  \param file_name   Filename of image to load
	 *  \param load_info   Returns with the image file attributes and pixels
	 * \return success/failure
	 */
	bool _LoadRawImageJpeg(const std::string & file_name, private_video::ImageLoadInfo & load_info);

	/** \brief Load raw image data from a PNG file
	 *
	 *  \param file_name   Filename of image to load
	 *  \param load_info   Returns with the image file attributes and pixels
	 * \return success/failure
	 */
	bool _LoadRawImagePng(const std::string & file_name, private_video::ImageLoadInfo & load_info);

	/*! \brief Saves Raw data in a Png file
	 *	\param file_name Name of the file, without the extension
	 *	\param info Structure of the information to store
	 *	\return True if the process was carried out with no problem, false otherwise
	 */
	bool _SavePng(const std::string& file_name, hoa_video::private_video::ImageLoadInfo &info) const;

	/*! \brief Saves Raw data in a Jpeg file
	 *	\param file_name Name of the file, without the extension
	 *	\param info Structure of the information to store
	 *	\return True if the process was carried out with no problem, false otherwise
	 */
	bool _SaveJpeg(const std::string& file_name, hoa_video::private_video::ImageLoadInfo &info) const;

	/*! \brief Get information of a Png file
	 *	\param file_name Name of the file, without the extension
	 *	\param rows Rows of the image
	 *	\param cols Columns of the image
	 *	\param bpp Bits per pixel of the image
	 *	\return True if the process was carried out with no problem, false otherwise
	 */
	bool _GetImageInfoPng(const std::string& file_name, uint32& rows, uint32& cols, uint32& bpp);

	/*! \brief Get information of a Jpeg file
	 *	\param file_name Name of the file, without the extension
	 *	\param rows Rows of the image
	 *	\param cols Columns of the image
	 *	\param bpp Bits per pixel of the image
	 *	\return True if the process was carried out with no problem, false otherwise
	 */
	bool _GetImageInfoJpeg(const std::string& file_name, uint32& rows, uint32& cols, uint32& bpp);

	/** \brief loop through all currently loaded images and if they belong to the given tex sheet, reload them into it
	 *
	 *  \param tex_sheet   pointer to the tex sheet whose images we want to load
	 * \return success/failure
	 */
	bool _ReloadImagesToSheet(private_video::TexSheet* tex_sheet);

	/** \brief removes the image from the STL map with the same pointer as the one passed in. Returns false on failure
	 *
	 *  \param image_to_remove   pointer to the image we want to remove
	 * \return success/failure
	 */

	bool _RemoveImage(private_video::Image *image_to_remove);

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
	/*!
	 *  \param buffer Buffer where the pixels of the texture will be stored
	 *  \param texture TexSheet to be copied
	*/
	void _GetBufferFromTexture (hoa_video::private_video::ImageLoadInfo& buffer, hoa_video::private_video::TexSheet* texture) const;

	//! \brief Pass an image (video memory) to a system memory buffer
	/*!
	 *  \param buffer Buffer where the pixels of the image will be stored
	 *  \param img Image to be copied
	*/
	void _GetBufferFromImage (hoa_video::private_video::ImageLoadInfo& buffer, hoa_video::private_video::BaseImage* img) const;

	/** \brief removes a texture sheet from our vector of sheets and deletes it
	 *
	 *  \param sheet_to_remove  pointer to the sheet we want to remove
	 * \return success/failure
	 */
	bool _RemoveSheet(private_video::TexSheet *sheet_to_remove);

	/** \brief Rounds a force value to the nearest integer based on probability. 
	*** \param force  The force to round
	*** \return the rounded force value
	*** \note For example, a force value of 2.85 has an 85% chance of rounding to 3 and a 15% chance of rounding to 2. This rounding
	*** methodology is necessary because for force values less than 1 (e.g. 0.5f), the shake force would always round down to zero
	*** even though there is positive force.
	**/
	float _RoundForce(float force);

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

	/** \brief Updates all active shaking effects
	*** \param frame_time The number of milliseconds that have elapsed for the current rendering frame
	**/
	void _UpdateShake(uint32 frame_time);

	//! Whether textures should be smoothed for non natural resolution.
	bool _ShouldSmooth()
		{ return ( _width != VIDEO_STANDARD_RES_WIDTH || _height != VIDEO_STANDARD_RES_HEIGHT); }

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
