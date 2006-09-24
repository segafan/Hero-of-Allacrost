///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    video.h
 * \author  Raj Sharma, roos@allacrost.org, Daniel Steuernol, steu@allacrost.org
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
#include "defs.h"

// SDL includes
#ifdef __APPLE__
	#include <SDL_ttf/SDL_ttf.h>
#else
	#include <SDL/SDL_ttf.h>
#endif

// OpenGL includes
#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

// Image loader includes
#include <png.h>
#include <jpeglib.h>


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

 
//! All calls to the video engine are wrapped in this namespace.
namespace hoa_video 
{

class GameVideo;
//! The singleton pointer responsible for all video operations.
extern GameVideo *VideoManager;
//! Determines whether the code in the hoa_video namespace should print
extern bool VIDEO_DEBUG;



/*!
 *  \brief Creates a random float between a and b.
 */
float RandomFloat(float a, float b);


/*!
 *  \brief Rotates a point (x,y) around the origin (0,0), by angle radians
 */
void RotatePoint(float &x, float &y, float angle);


/*!
 *  \brief linearly interpolation, returns a value which is (alpha*100) percent between initial and final
 *
 *  \param alpha    controls the linear interpolation
 *  \param initial  initial value
 *  \param final    final value
 */	
float Lerp(float alpha, float initial, float final);



//! \name Constants
//@{
//! \brief PI and multiples of PI. Used in various math calculations such as interpolations
const float VIDEO_QUARTER_PI = 0.785398163f;
const float VIDEO_HALF_PI    = 1.570796326f;
const float VIDEO_PI         = 3.141592653f;
const float VIDEO_2PI        = 6.283185307f;
//@}

//! animation frame period: how many milliseconds 1 frame of animation lasts for
const int32 VIDEO_ANIMATION_FRAME_PERIOD = 10;

//! Draw flags to control x,y alignment, flipping, and blending
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
 *  \brief target must be set before the call to GameVideo::Initialize().
 *         This specifies whether we are drawing to an SDL window, a QT widget, etc.
 *****************************************************************************/

enum VIDEO_TARGET
{
	VIDEO_TARGET_INVALID = -1,

	VIDEO_TARGET_SDL_WINDOW = 0,   //! SDL window
	VIDEO_TARGET_QT_WIDGET  = 1,   //! QT widget
	
	VIDEO_TARGET_TOTAL = 2
};


/*!***************************************************************************
 *  \brief stencil operation to use, describes how stencil buffer is modified
 *****************************************************************************/

enum VIDEO_STENCIL_OP
{
	VIDEO_STENCIL_OP_INVALID = -1,
	
	VIDEO_STENCIL_OP_ONE      = 0,  //! set stencil value to one
	VIDEO_STENCIL_OP_ZERO     = 1,  //! set stencil value to zero
	VIDEO_STENCIL_OP_INCREASE = 2,  //! increase stencil value
	VIDEO_STENCIL_OP_DECREASE = 3,  //! decrease stencil value
	
	VIDEO_STENCIL_OP_TOTAL = 4
};



// forward declarations
class StillImage;
class GameVideo;
class Color;


/*!***************************************************************************
 *  \brief class for representing colors, with common operations like
 *         multiplying, adding, etc.
 *****************************************************************************/


//! private_video namespace, hides things which are used internally
namespace private_video
{

class GUI;
class TexSheet;
class TexMemMgr;
class FixedTexMemMgr;
class VariableTexMemMgr;
class Image;
class ImageLoadInfo;




} // namespace private_video




/*!****************************************************************************
 *  \brief Manages all the video audio and serves as the API to the video engine.
 *
 * \note Full documentation for video engine can be found at:
 *       http://www.allacrost.org/staff/user/roos/video.html
 *****************************************************************************/

class GameVideo
{
public:
	
	SINGLETON_METHODS(GameVideo);
	

	//-- General --------------------------------------------------------------

	/*!
	 *  \brief call at beginning of every frame
	 */
	bool Clear();


	/*!
	 *  \brief call this version of clear if you want to clear the screen
	 *         to some specific color
	 */
	bool Clear(const Color &c);
		
	/*!
	 *  \brief call at end of every frame
	 *  \param frameTime   milliseconds since the last frame
	 */
	bool Display(int32 frameTime);
	
	
	/*!
	 *  \brief set the target, i.e. whether the video engine is being used to
	 *         draw to an SDL window, or a QT widget. (There are some important
	 *         differences, so the video engine needs to know).
	 *
	 *  \param target can be VIDEO_TARGET_QT_WIDGET, or VIDEO_TARGET_SDL_WINDOW
	 *
	 *  \note  If you don't call this function, the default target is SDL window
	 *         Also, note that you MUST call this before calling Initialize()
	 */
	bool SetTarget(VIDEO_TARGET target);
	

	//-- Video settings -------------------------------------------------------
	
	//! NOTE: when you modify video setting, you must call ApplySettings()
	//!       to actually apply them

	/*!
	 *  \brief sets the current resolution to the given width and height
	 *
	 *  \param width new screen width
	 *  \param height new screen height
	 *  \note  you must call ApplySettings() to actually apply the change
	 */	
	bool SetResolution(int32 width, int32 height);	

	
	/*!
	 *  \brief returns width, (whatever was set with SetResolution)
	 */	
	int32 GetWidth() { return _width; }


	/*!
	 *  \brief returns height, (whatever was set with SetResolution)
	 */	
	int32 GetHeight() { return _height; }

	/*!
	 *  \brief returns true if game is in fullscreen mode
	 */	
	bool IsFullscreen();
	
	/*!
	 *  \brief sets the game to fullscreen or windowed depending on whether
	 *         true or false is passed
	 *
	 *  \param fullscreen set to true if you want fullscreen, false for windowed.
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


	//-- Coordinate system / viewport  ------------------------------------------
	
	/*!
	 *  \brief sets the viewport, i.e. the area of the screen that gets drawn
	 *         to. The default is (0, 100, 0, 100).
	 *
	 *  \param left   left border of screen
	 *  \param right  right border of screen
	 *  \param top    top border of screen
	 *  \param bottom bottom border of screen
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
	 *  \param left   left border of screen
	 *  \param right  right border of screen
	 *  \param top    top border of screen
	 *  \param bottom bottom border of screen
	 */   	                  
	void SetCoordSys
	(
		float left,
		float right,
		float bottom,
		float top
	);


	/*!
	 *  \brief sets the coordinate system. Default is (0,1024,0,768)
	 *  \param coordSys the coordinate system you want to set to
	 */   	                  
	void SetCoordSys
	(
		const CoordSys &coordSys
	);


	/*!
	 *  \brief enables scissoring, where you can specify a rectangle of the screen
	 *         which is affected by rendering operations. MAKE SURE to disable
	 *         scissoring as soon as you're done using the effect, or all subsequent
	 *         draw calls will get messed up
	 *
	 *  \param enable pass true to turn on scissoring, false to disable
	 */   	                  
	void EnableScissoring(bool enable);
	

	/*!
	 *  \brief returns true if scissoring's enabled
	 */   	                  
	
	bool IsScissoringEnabled() { return _scissorEnabled; }

	/*!
	 *  \brief sets the rectangle to use for scissorring, where you can specify an
	 *         area of the screen for draw operations to affect. Note, the coordinates
	 *         you pass in are based on the current coordinate system, not screen coords
	 */   	                  
	void SetScissorRect
	(
		float left,
		float right,
		float bottom,
		float top
	);


	/*!
	 *  \brief sets the rectangle to use for scissorring. This version of the function
	 *         expects a ScreenRect, in other words the coordinates have already been
	 *         transformed to integer values (pixel unit) with (0,0) as the upper left
	 *         and (w-1, h-1) as the lower right, where w and h are the current screen
	 *         dimensions
	 */   	                  
	void SetScissorRect
	(
		const ScreenRect &rect
	);


	/*!
	 *  \brief returns scissor rect
	 */   	                  
	ScreenRect GetScissorRect() { return _scissorRect; }


	/*!
	 *  \brief converts coordinates given relative to the current coord sys into
	 *         "screen coordinates", which are in pixel units with (0,0) as the
	 *         top left and (w-1, h-1) as the lower-right, where w and h are the
	 *         dimensions of the screen
	 */   	                  

	ScreenRect CalculateScreenRect(float left, float right, float bottom, float top);

	//-- Transformations ------------------------------------------------------

	/*!
	 *  \brief saves entire state of the video engine on to the stack (all draw
	 *         flags, coordinate system, scissor rect, viewport, etc.)
	 *         This is useful for safety purposes between two major parts of code
	 *         to ensure that one part doesn't inadvertently affect the other.
	 *         However, it's a very expensive function call. If you only need to
	 *         push the current transformation, you should use PushMatrix() and
	 *         PopMatrix()
	 */
	void PushState();
	
	/*!
	 *  \brief pops the most recently pushed video engine state from the stack
	 *         and restores all of the old settings.
	 */
	void PopState ();
	

	/*!
	 *  \brief saves current modelview transformation on to the stack. In English,
	 *         that means the combined result of calls to Move/MoveRelative/Scale/Rotate
	 */
	void PushMatrix();
	
	/*!
	 *  \brief pops the modelview transformation from the stack.
	 */
	void PopMatrix();

	/*!
	 *  \brief sets draw position to (x,y)
	 *  \param x  x coordinate to move to
	 *  \param y  y coordinate to move to
	 */
	void Move(float x, float y);
	
	/*!
	 *  \brief moves draw position (dx, dy) units
	 *  \param dx how many units to move in x direction
	 *  \param dy how many units to move in y direction
	 */
	void MoveRelative(float dx, float dy);

	//! \brief Gets the location of the draw cursor
	void GetDrawPosition(float &x, float &y);
	
	/*!
	 *  \brief rotates images counterclockwise by 'angle' radians
	 *  \param angle how many radians to rotate by
	 *  \note This function should NOT be used unless you understand how transformation
	 *        matrices work in OpenGL.
	 */
	void Rotate (float angle);

	
	/*!
	 *  \brief after you call this, subsequent calls to DrawImage() result in
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
	

	/*!
	 *  \brief sets OpenGL transform to contents of 4x4 matrix (16 values)
	 *  \param array of 16 float values forming a 4x4 transformation matrix
	 */
	void SetTransform(float m[16]);


	//-- Text -----------------------------------------------------------------

	/*!
	 *  \brief loads a font from a TTF file
	 *  \param TTF_filename  the filename of the TTF file to load
	 *  \param name          name of the font (e.g. "courier24")
	 *  \param size          point size of the font
	 */
	bool LoadFont
	(
		const std::string &TTF_filename, 
		const std::string &name, 
		int32 size
	);
	

	/*!
	 *  \brief returns true if the font with the given name has been successfully loaded
	 *  \param name   name of the font (e.g. "courier24")
	 */
	bool IsValidFont(const std::string &name);


	/*!
	 *  \brief given the name of a font, fill in a font properties structure with info
	 *         like the height of the font, etc.
	 *
	 *  \param fontName  name of the font, e.g. "courier24"
	 *  \param fp        reference to font properties structure to return information into
	 */
	bool GetFontProperties(const std::string &fontName, FontProperties &fp);


	/*!
	 *  \brief calculates the width of the given text if it were rendered with the given font
	 *         If an invalid font name is passed, returns -1
	 *
	 *  \param fontName  the font to use
	 *  \param text      the text string in unicode
	 */

	int32 CalculateTextWidth(const std::string &fontName, const hoa_utils::ustring &text);


	/*!
	 *  \brief calculates the width of the given text if it were rendered with the given font
	 *         If an invalid font name is passed, returns -1
	 *
	 *  \param fontName  the font to use
	 *  \param text      the text string in multi-byte character format
	 */

	int32 CalculateTextWidth(const std::string &fontName, const std::string  &text);


	/*!
	 *  \brief sets current font
	 *  \param name  name of the font to set to
	 */
	bool SetFont(const std::string &name);


	/*!
	 *  \brief enables/disables text shadowing
	 *  \param enable  pass true to enable, false to disable
	 */
	void EnableTextShadow(bool enable);

	
	/*!
	 *  \brief sets current text color
	 *  \param color  color to set to
	 */
	bool SetTextColor(const Color &color);
		

	/*!
	 *  \brief sets the shadow offset to use for a given font. By default, all font shadows
	 *         are slightly to the right and to the bottom of the text, by an offset of
	 *         fontHeight / 8. That doesn't always look good though, so use this function
	 *         to adjust it if you want.
	 *
	 *  \param fontName  label of the font you want to set the shadow offset for
	 *  \param x         x offset in pixels (based on 1024x768)
	 */
	bool SetFontShadowXOffset(const std::string &fontName, int32 x);


	/*!
	 *  \brief sets the shadow offset to use for a given font. By default, all font shadows
	 *         are slightly to the right and to the bottom of the text, by an offset of
	 *         fontHeight / 8. That doesn't always look good though, so use this function
	 *         to adjust it if you want.
	 *
	 *  \param fontName  label of the font you want to set the shadow offset for
	 *  \param y         y offset in pixels (based on 1024x768)
	 */
	bool SetFontShadowYOffset(const std::string &fontName, int32 y);


	/*!
	 *  \brief sets the shadow style to use for the given font
	 *
	 *  \param fontName  label of the font you want to set the shadow style for
	 *  \param style     the shadow style you want (e.g. VIDEO_TEXT_SHADOW_BLACK)
	 */
	bool SetFontShadowStyle(const std::string &fontName, TextShadowStyle style);


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
	 *  \param text   text string to draw
	 */
	bool DrawText(const std::string &text);
	
	/*!
	 *  \brief unicode version of DrawText(). This should be used for
	 *         anything in the game which might need to be localized
	 *         (game dialogue, interface text, etc.)
	 *
	 *  \param uText  unicode text string to draw
	 */
	bool DrawText(const hoa_utils::ustring &uText);


	//-- Particle effects -----------------------------------------------------------
	

	/*!
	 *  \brief add a particle effect at the given point x and y
	 *  \note  set the reload parameter to true to reload the effect definition file
	 *         every time the effect is played. This is useful if you are working on an
	 *         effect and want to see how it looks. When we actually release the game,
	 *         reload should be false since it adds some cost to the loading
	 */
	ParticleEffectID AddParticleEffect(const std::string &particleEffectFilename, float x, float y, bool reload=false);	


	/*!
	 *  \brief draws all active particle effects
	 */
	bool DrawParticleEffects();


	/*!
	 *  \brief stops all active particle effects
	 *  \param kill_immediate  If this is true, then the particle effects die out immediately
	 *                         If it is false, then they don't immediately die, but new particles
	 *                         stop spawning
	 */
	void StopAllParticleEffects(bool kill_immediate = false);


	/*!
	 *  \brief get pointer to an effect given its ID
	 */
	ParticleEffect *GetParticleEffect(ParticleEffectID id);


	/*!
	 *  \brief get number of live particles
	 */
	int32 GetNumParticles();


	//-- Images / Animation ---------------------------------------------------------
	

	/*!
	 *  \brief loads an image (static or animated image). Assumes that you have already called
	 *         all the appropriate functions to initialize the image. In the case of a static image,
	 *         this means setting its filename, and possibly other properties like width, height, and
	 *         color. In the case of an animated image, it means calling AddFrame().
	 *
	 *  \param id  image descriptor to load- either a StillImage or AnimatedImage.
	 */
	bool LoadImage(ImageDescriptor &id);
	bool LoadImageGrayScale(ImageDescriptor &id);


	/*!
	 *  \brief captures the contents of the screen and saves it to an image
	 *         descriptor
	 *
	 *  \param id  image descriptor to capture to
	 */
	bool CaptureScreen(StillImage &id);

	
	/*!
	 *  \brief decreases ref count of an image (static or animated)
	 *
	 *  \param id  image descriptor to decrease the reference count of
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
	void SetDrawFlags(int32 firstflag, ...);


	/*!
	 *  \brief draws an image which is modulated by the scene's light color
	 *
	 *  \param id  image descriptor to draw (either StillImage or AnimatedImage)
	 */	
	bool DrawImage(const ImageDescriptor &id);


	/*!
	 *  \brief draws an image which is modulated by a custom color
	 *
	 *  \param id  image descriptor to draw (either StillImage or AnimatedImage)
	 */	
	bool DrawImage(const ImageDescriptor &id, const Color &color);
	

	/*!
	 *  \brief converts a 2D array of tiles into one big image
	 *
	 *  \param tiles   a vector of image descriptors (the tiles)
	 *  \param indices a 2D vector in row-column order (e.g. indices[y][x])
	 *         which forms a rectangular array of tiles
	 */
	StillImage TilesToObject
	( 
		std::vector<StillImage> &tiles, 
		std::vector< std::vector<uint32> > indices 
	);


	/*!
	 *  \brief returns the amount of animation frames that have passed since the last
	 *         call to GameVideo::Display(). This number is based on VIDEO_ANIMATION_FRAME_PERIOD,
	 *         and is used so that AnimatedImages know how many frames to increment themselves by.
	 */
	int32 GetFrameChange() { return _current_frame_diff; }

	void DEBUG_NextTexSheet();    // These cycle through the currently loaded
	void DEBUG_PrevTexSheet();    // texture sheets so they can be viewed on screen
	
	
	//-- Menus -----------------------------------------------------------------

	/*!
	 *  \brief sets the current menu skin (borders+fill color). Assumes all four
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
	 */
	bool SetMenuSkin
	(
		const std::string &imgBaseName,
		const Color  &fillColor
	);

	
	/*!
	 *  \brief sets the current menu skin (borders+fill color). This version of
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
	 */
	bool SetMenuSkin(const std::string &imgBaseName, const Color  &fillColor_TL, const Color  &fillColor_TR, const Color  &fillColor_BL, const Color  &fillColor_BR);


	/*!
	 *  \brief sets the current menu skin (borders+background image).
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
	 */
	bool SetMenuSkin(const std::string &imgBaseName, const std::string &backgroundImage, const Color  &fillColor_TL, const Color  &fillColor_TR, const Color  &fillColor_BL, const Color  &fillColor_BR);



	/*!
	 *  \brief sets the current menu skin (borders+background image).
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
	 */
	bool SetMenuSkin(const std::string &imgBaseName, const std::string &backgroundImage, const Color &fillColor);


	//-- Lighting and fog -----------------------------------------------------
	
	/*!
	 *  \brief turn on or off the ligt color for the scene
	 */
	bool EnableSceneLighting(const Color &color);
	void DisableSceneLighting();


	/*!
	 *  \brief returns the scene lighting color
	 */
	Color &GetSceneLightingColor();

	/*!
	 *  \brief sets fog parameters
	 *
	 *  \param color  Color of the fog (alpha should be 1.0)
	 *  \param intensity  Intensity of fog from 0.0f to 1.0f
	 */	
	bool EnableFog(const Color &color, float intensity);
	void DisableFog();
	
	/*!
	 *  \brief draws a halo at the given spot
	 *
	 *  \param id    image descriptor for the halo image
	 *  \param x     x coordinate of halo
	 *  \param y     y coordinate of halo
	 *  \param color color of halo
	 */
	bool DrawHalo(const StillImage &id, float x, float y, const Color &color = Color(1.0f, 1.0f, 1.0f, 1.0f));

	/*!
	 *  \brief draws a real light at the given spot
	 *
	 *  \param id    image descriptor for the light mask
	 *  \param x     x coordinate of light
	 *  \param y     y coordinate of light
	 *  \param color color of light
	 */	
	bool DrawLight(const StillImage &id, float x, float y, const Color &color = Color(1.0f, 1.0f, 1.0f, 1.0f));

	/*!
	 *  \brief call with true if this map uses real lights, otherwise call with
	 *         false
	 */	
	bool EnablePointLights();
	void DisablePointLights();
	

	/*!
	 *  \brief call after rendering all real lights.  This function renders all lights
	 *		   to the lighting overlay texture, Moved into ApplyLightingOverlay
	 */	
	//bool AccumulateLights();

	/*!
	 *  \brief call after all map images are drawn to apply lighting. All
	 *         menu and text rendering should occur AFTER this call, so that
	 *         they are not affected by lighting.
	 */	
	bool ApplyLightingOverlay();


	//-- Overlays / lightning -------------------------------------------------------


	/*!
	 *  \brief draws a full screen overlay of the given color
	 *  \note  This is very slow, so use sparingly!
	 */	
	bool DrawFullscreenOverlay(const Color &color);


	/*!
	 *  \brief call to create lightning effect
	 *  \param litFile a .lit file which contains lightning intensities stored
	 *                 as bytes (0-255).
	 */	
	bool MakeLightning(const std::string &litFile);


	/*!
	 *  \brief call this every frame to draw any lightning effects. You should make
	 *         sure to place this call in an appropriate spot. In particular, you should
	 *         draw the lightning before drawing the GUI.
	 */	
	bool DrawLightning();

	
	//-- Fading ---------------------------------------------------------------

	/*!
	 *  \brief Begins a screen fade.
	 *  \param color - color to fade to.
	 *  \param fade_time - the fade will last this number of seconds
	 *  \return True if fade was successful, false otherwise.
	 */
	bool FadeScreen(const Color &color, float fade_time);
	
	/*!
	 *  \brief Determines if a fade is currently occurring.
	 *  \return True if screen fade is currently in progress, false otherwise.
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
	bool ShakeScreen(float force, float falloffTime, ShakeFalloff falloff = VIDEO_FALLOFF_NONE);	

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
	 *  \brief Sets a new gamma value using SDL_SetGamma()
	 *
	 *  \param value        Gamma value of 1.0f is the default value
	 */
	void SetGamma(float value);

	/*!
	 *  \brief Returns the gamma value
	 */
	float GetGamma();

	/*!
	 *  \brief updates the FPS counter with the given frame time and draws the
	 *         current FPS on the screen.
	 */
	bool DrawFPS(int32 frameTime);
	

	/*!
	 *  \brief toggles the FPS display (on by default)
	 */
	void ToggleFPS();
	
	
	/*!
	 *  \brief draws a line grid. Used by map editor to draw a grid over all
	 *         the tiles. This function will start at (x,y), and go to
	 *         (xMax, yMax), with horizontal cell spacing of xstep and
	 *         vertical cell spacing of ystep. The final parameter is just the
	 *         color the lines should be drawn
	 *
	 *  \note  xMax and yMax are not inputs to the function- they are taken
	 *         from the current coord sys
	 */
	void DrawGrid(float x, float y, float xstep, float ystep, const Color &c);
	

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

	bool SetDefaultCursor(const std::string &cursorImageFilename);
	StillImage *GetDefaultCursor();

private:
	SINGLETON_DECLARE(GameVideo);
	
	//-- Private variables ----------------------------------------------------

	
	// for now the game gui class is a member of video so that
	// externally people only have to deal with GameVideo.

	private_video::GUI *_gui;    //! pointer to GUI class which implements all GUI functionality

	// particle manager, does dirty work of managing particle effects
	private_video::ParticleManager _particle_manager;

	// target (QT widget or SDL window)
	VIDEO_TARGET _target;

	// draw flags	
	int8 _blend;        //! blend flag which specifies normal alpha blending
	int8 _xalign;       //! x align flag which tells if images should be left, center, or right aligned
	int8 _yalign;       //! y align flag which tells if images should be top, center, or bottom aligned
	int8 _xflip;        //! x flip flag. true if images should be flipped horizontally
	int8 _yflip;        //! y flip flag. true if images should be flipped vertically

	char _nextTempFile[9];    //! eight character name for temp files that increments every time you create a new one so they are always unique

	CoordSys    _coordSys;    //! current coordinate system
	
	ScreenRect _viewport;     //! current viewport
	ScreenRect _scissorRect;  //! current scissor rectangle
	
	bool _scissorEnabled;   //! is scissoring enabled or not
	
	private_video::ScreenFader _fader;  //! fader class which implements screen fading
	
	bool   _advancedDisplay;       //! advanced display flag. If true, info about the video engine is shown on screen
	bool   _fpsDisplay;            //! fps display flag. If true, FPS is displayed
	
	int32  _currentDebugTexSheet;  //! current debug texture sheet
	int32  _numTexSwitches;        //! keep track of number of texture switches per frame
	int32  _numDrawCalls;          //! keep track of number of draw calls per frame
	bool   _batchLoading;          //! if true, we are in batch mode
	
	bool   _usesLights;      //! true if real lights are enabled
	GLuint _lightOverlay;    //! lighting overlay texture

	float  _shakeX, _shakeY; //! offsets to shake the screen by (if any)

	float _gamma_value; //! Current gamma value
	
	std::list<private_video::ShakeForce> _shakeForces;  //! current shake forces affecting screen
		
	bool _fullscreen;     //! true if game is currently running fullscreen
	int32  _width;        //! current screen width
	int32  _height;       //! current screen height
	
	// changing the video settings does not actually do anything until
	// you call ApplySettings(). Up til that point, store them in temp
	// variables so if the new settings are invalid, we can roll back.
	
	bool   _temp_fullscreen;   //! holds the desired fullscreen status (true=fullscreen, false=windowed). Not actually applied until ApplySettings() is called
	int32  _temp_width;        //! holds the desired screen width. Not actually applied until ApplySettings() is called
	int32  _temp_height;       //! holds the desired screen height. Not actually applied until ApplySettings() is called
		
	GLuint _lastTexID;    //! ID of the last texture that was bound. Used to eliminate redundant binding of textures

	std::string _currentFont;          //! current font name
	Color       _currentTextColor;     //! current text color

	StillImage _defaultMenuCursor;  //! image which is to be used as the cursor
	
	bool _textShadow;   //! if true, text shadow effect is enabled

	Color _fogColor;        //! current fog color (set by SetFog())
	float _fogIntensity;    //! current fog intensity (set by SetFog())
	
	Color _lightColor;      //! current scene lighting color (essentially just modulates vertex colors of all the images)

	bool  _lightningActive;  //! true if a lightning effect is active
	int32 _lightningCurTime; //! current time of lightning effect (time since it started)
	int32 _lightningEndTime; //! how many milliseconds to do the lightning effect for
	
	std::vector <float> _lightningData;  //! intensity data for lightning effect

	int32 _animation_counter;   //! counter to keep track of milliseconds since game started for animations
	int32 _current_frame_diff;  //! keeps track of the number of frames animations should increment by for the current frame

	std::vector <StillImage *>    _batchLoadImages;    //! vector of images in a batch which are to be loaded
	
	std::map    <std::string, private_video::Image*>   _images;      //! STL map containing all the images currently being managed by the video engine	
	std::vector <private_video::TexSheet *>     _texSheets;          //! vector containing all texture sheets currently being managed by the video engine
	std::map    <std::string, FontProperties *> _fontMap;			 //! STL map containing properties for each font (includeing TTF_Font *)

	std::map <std::string, ParticleEffectDef *> _particle_effect_defs; //! STL map containing all loaded particle effect definitions
	
	std::stack  <private_video::Context>      _contextStack;         //! stack containing context, i.e. draw flags plus coord sys. Context is pushed and popped by any GameVideo functions that clobber these settings


	//-- Private methods ------------------------------------------------------


	/*!
	 *  \brief wraps a call to glBindTexture(), except it adds checking to eliminate redundant texture binding. Redundancy checks are already implemented by most drivers, but this is a double check "just in case"
	 *
	 *  \param texID   integer handle to the OpenGL texture
	 */	
	bool _BindTexture(GLuint texID);

	int32 _ConvertXAlign(int32 xalign);
	int32 _ConvertYAlign(int32 yalign);

	/*!
	 *  \brief creates a blank texture of the given width and height and returns integer used by OpenGL to refer to this texture. Returns 0xffffffff on failure.
	 *
	 *  \param width   desired width of the texture
	 *  \param height  desired height of the texture
	 */	
	GLuint _CreateBlankGLTexture(int32 width, int32 height);

	/*!
	 *  \brief creates an StillImage of a menu which is the given size
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
	 */
	bool _CreateMenu
	(
		StillImage &menu, 
		float width, 
		float height, 
		float & innerWidth,
		float & innerHeight,
		int32 edgeVisibleFlags, 
		int32 edgeSharedFlags
	);

	/*!
	 *  \brief returns a filename like TEMP_abcd1234.ext, and each time you call it, it increments the
	 *         alphanumeric part of the filename. This way, during any particular run
	 *         of the game, each temp filename is guaranteed to be unique.
	 *         Assuming you create a new temp file every second, it would take 100,000 years to get
	 *         from TEMP_00000000 to TEMP_zzzzzzzz
	 *
	 *  \param extension   The extension for the temp file. Although we could just save temp files
	 *                     without an extension, that might cause stupid bugs like DevIL refusing
	 *                     to load an image because it doesn't end with .png.
	 */	
	std::string _CreateTempFilename(const std::string &extension);

	/*!
	 *  \brief creates a texture sheet
	 *
	 *  \param width    width of the sheet
	 *  \param height   height of the sheet
	 *  \param type     specifies what type of images this texture sheet manages (e.g. 32x32 images, 64x64 images, any type, etc)
	 *  \param isStatic if true, this texture sheet is meant to manage images which are not expected to be loaded and unloaded very often
	 */	
	private_video::TexSheet *_CreateTexSheet
	(
		int32 width,
		int32 height,
		private_video::TexSheetType type,
		bool isStatic
	);


	/*!
	 *  \brief wraps a call to glDeleteTextures(), except it adds some checking related to eliminating redundant texture binding.
	 *
	 *  \param texID   integer handle to the OpenGL texture
	 */	
	bool _DeleteTexture(GLuint texID);


	/*!
	 *  \brief decreases the reference count of an image
	 *
	 *  \param image  pointer to image
	 */	
	bool _DeleteImage(private_video::Image *const image);
	
	/*!
	 *  \brief decreases ref count of an image
	 *
	 *  \param id  image descriptor to decrease the reference count of
	 */
	bool _DeleteImage(StillImage &id);

	/*!
	 *  \brief decreases ref count of an animated image
	 *
	 *  \param id  image descriptor to decrease the reference count of
	 */
	bool _DeleteImage(AnimatedImage &id);

	/*!
	 *  \brief deletes the temporary textures from the "temp" folder that were saved
	 *         by _SaveTempTextures()
	 */	
	bool _DeleteTempTextures();

	/*!
	 *  \brief draws an image element, i.e. one image within an image descriptor which may contain multiple images
	 *
	 *  \param element        pointer to the image element to draw
	 *  \param modulateColor  combination of color for this image, and our current fade color
	 */		 
	bool _DrawElement(const private_video::ImageElement &element, const Color &modulateColor);


	/*!
	 *  \brief draws an image element, i.e. one image within an image descriptor which may contain multiple images
	 *
	 *  \note  this version of the function accepts no color, so for cases where no fade is going on
	 *         and we don't want to modulate the image's color (which is the case indeed 99% of the time),
	 *         we can skip all the nasty modulation math for a bit of extra efficiency
	 *
	 *  \param element        pointer to the image element to draw
	 */		 
	bool _DrawElement(const private_video::ImageElement &element);

	/*!
	 *  \brief helper function to DrawImage() to do the actual work of doing an image
	 *
	 *  \param img static image to draw
	 */	
	bool _DrawStillImage(const StillImage &img);


	/*!
	 *  \brief helper function to DrawImage() to do the actual work of drawing an image
	 *
	 *  \param img static image to draw
	 *  \param color color to modulate image by
	 */	
	bool _DrawStillImage(const StillImage &img, const Color &color);  


	/*!
	 *  \brief does the actual work of drawing text
	 *
	 *  \param uText  Pointer to a unicode string holding the text to draw
	 */	
	bool _DrawTextHelper(const uint16 *const uText);

	/*!
	 *  \brief inserts an image into a texture sheet
	 *
	 *  \param image       pointer to the image to insert
	 *  \param loadInfo	   attributes of the image to be inserted
	 */	
	private_video::TexSheet *_InsertImageInTexSheet(private_video::Image *image, private_video::ImageLoadInfo & loadInfo, bool isStatic);

	/*!
	 *  \brief loads an image
	 *
	 *  \param id  image descriptor to load. Can specify filename, color, width, height, and static as its parameters
	 */
	bool _LoadImage(StillImage &id, bool grayscale = false);

	/*!
	 *  \brief loads an animated image. Assumes that you have called AddFrame for all the frames.
	 *
	 *  \param id  image descriptor to load
	 */
	bool _LoadImage(AnimatedImage &id, bool grayscale = false);

	/*!
	 *  \brief does the actual work of loading an image
	 *
	 *  \param id  StillImage of the image to load. May specify a filename, color, width, height, and static
	 */	
	bool _LoadImageHelper(StillImage &id, bool grayscale = false);

	/*!
	 *  \brief Load raw image data from a file
	 *
	 *  \param filename   Filename of image to load
	 *  \param loadInfo   Returns with the image file attributes and pixels
	 */	
	bool _LoadRawImage(const std::string & filename, private_video::ImageLoadInfo & loadInfo, bool grayscale = false);
	bool _LoadRawImageJpeg(const std::string & filename, private_video::ImageLoadInfo & loadInfo, bool grayscale);
	bool _LoadRawImagePng(const std::string & filename, private_video::ImageLoadInfo & loadInfo, bool grayscale);

	/*!
	 *  \brief loop through all currently loaded images and if they belong to the given tex sheet, reload them into it
	 *
	 *  \param texSheet   pointer to the tex sheet whose images we want to load
	 */	
	bool _ReloadImagesToSheet(private_video::TexSheet *texSheet);

	/*!
	 *  \brief removes the image from the STL map with the same pointer as the one passed in. Returns false on failure
	 *
	 *  \param imageToRemove   pointer to the image we want to remove
	 */	

	bool _RemoveImage(private_video::Image *imageToRemove);
	

	/*!
	 *  \brief removes a texture sheet from our vector of sheets and deletes it
	 *
	 *  \param sheetToRemove  pointer to the sheet we want to remove
	 */	
	bool _RemoveSheet(private_video::TexSheet *sheetToRemove);

	/*!
	 *  \brief rounds a force value to the nearest integer. Rounding is based on probability. For example the number 2.85 has an 85% chance of rounding to 3 and a 15% chance of rounding to 2
	 *
	 *  \param force  The force to round
	 */	
	float _RoundForce(float force);   // rounds a force value

	/*!
	 *  \brief restores coord system, draw flags, and transformations
	 */	
	void _PopContext();

	/*!
	 *  \brief saves coord system, draw flags, and transformations
	 */	
	void _PushContext();
	
	/*!
	 *  \brief saves temporary textures to disk, in other words, textures which were not
	 *         loaded to a file. This is used when the GL context is being destroyed,
	 *         perhaps because we are switching from windowed to fullscreen. So, we need
	 *         to save all textures to disk so we can reload them later.
	 */	
	bool _SaveTempTextures();

	int32 _ScreenCoordX(float x);
	int32 _ScreenCoordY(float y);

	/*!
	 *  \brief updates the shaking effect
	 *
	 *  \param frameTime  elapsed time for the current rendering frame
	 */	
	void  _UpdateShake(int32 frameTime);


	/*!
	 *  \brief function solely for debugging, which shows number of texture switches made during a frame,
	 *         and other statistics useful for performance tweaking, etc.
	 */	
	bool _DEBUG_ShowAdvancedStats();


	/*!
	 *  \brief function solely for debugging, which displays the currently selected texture sheet. By using DEBUG_NextTexSheet() and DEBUG_PrevTexSheet(), you can change the current texture sheet so the sheet shown by this function cycles through all currently loaded sheets.
	 */	
	bool _DEBUG_ShowTexSheet();

	//! \brief the current draw cursor position
	float _x;
	float _y;
	
	friend class TextBox;
	friend class OptionBox;
	friend class GUIElement;
	friend class MenuWindow;
	friend class private_video::GUI;	
	friend class private_video::FixedTexMemMgr;
	friend class private_video::VariableTexMemMgr;
	friend class private_video::TexSheet;
	friend class private_video::ParticleSystem;
};

}   // namespace hoa_video

#endif // !_VIDEO_HEADER_
