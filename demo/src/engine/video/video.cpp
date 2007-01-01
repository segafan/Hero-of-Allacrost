///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
 * \file    video.cpp
 * \author  Raj Sharma, roos@allacrost.org
 * \brief   Source file for video engine interface.
 *****************************************************************************/ 


#include <cassert>
#include <cstdarg>
#include <math.h>

#include "utils.h"
#include "video.h"
#include "context.h"
#include "gui.h"

using namespace std;
using namespace hoa_video::private_video;
using namespace hoa_utils;

namespace hoa_video 
{

GameVideo *VideoManager = NULL;
bool VIDEO_DEBUG = false;

SINGLETON_INITIALIZE(GameVideo);


//-----------------------------------------------------------------------------
// Static variables
//-----------------------------------------------------------------------------

Color Color::clear (0.0f, 0.0f, 0.0f, 0.0f);
Color Color::white (1.0f, 1.0f, 1.0f, 1.0f);
Color Color::gray  (0.5f, 0.5f, 0.5f, 1.0f);
Color Color::black (0.0f, 0.0f, 0.0f, 1.0f);
Color Color::red   (1.0f, 0.0f, 0.0f, 1.0f);
Color Color::orange(1.0f, 0.4f, 0.0f, 1.0f);
Color Color::yellow(1.0f, 1.0f, 0.0f, 1.0f);
Color Color::green (0.0f, 1.0f, 0.0f, 1.0f);
Color Color::aqua  (0.0f, 1.0f, 1.0f, 1.0f);
Color Color::blue  (1.0f, 0.0f, 1.0f, 1.0f);
Color Color::violet(1.0f, 0.0f, 1.0f, 1.0f);
Color Color::brown (0.6f, 0.3f, 0.1f, 1.0f);


//-----------------------------------------------------------------------------
// Lerp: linearly interpolates value between initial and final
//-----------------------------------------------------------------------------

float Lerp(float alpha, float initial, float final)
{
	return alpha * final + (1.0f-alpha) * initial;
}


//-----------------------------------------------------------------------------
// RandomFloat: returns a random float between a and b
//-----------------------------------------------------------------------------

float RandomFloat(float a, float b)
{
	if(a == b)
		return a;
	
	if(a > b)
	{
		float c = a;  // swap
		a = b;
		b = c;
	}
	
	float r = float(rand()%10001);
	
	return a + (b - a) * r / 10000.0f;
}


//-----------------------------------------------------------------------------
// RotatePoint: rotates (x,y) around origin by angle radians
//-----------------------------------------------------------------------------

void RotatePoint(float &x, float &y, float angle)
{
	float old_x = x;
	float cos_angle = cosf(angle);
	float sin_angle = sinf(angle);
	
	x = x * cos_angle - y * sin_angle;
	y = y * cos_angle + old_x * sin_angle;
}

//-----------------------------------------------------------------------------
// GameVideo
//-----------------------------------------------------------------------------

GameVideo::GameVideo()
{
	_width = 0; 
	_height = 0;
	_fullscreen = false;
	_temp_width = 0;
	_temp_height = 0;
	_temp_fullscreen = false;
	_blend = 0; 
	_xalign = -1; 
	_yalign = -1; 
	_xflip = 0; 
	_yflip = 0;
	_currentDebugTexSheet = -1;
	_batchLoading = false;
	_usesLights = false;
	_lightOverlay = 0xFFFFFFFF;
	_gui = NULL;
	_lastTexID = 0xFFFFFFFF;
	_numTexSwitches = 0;
	_advancedDisplay = false;
	_fpsDisplay = false;
	_shakeX = 0;
	_shakeY = 0;
	_gamma_value = 1.0f;
	_fogColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
	_fogIntensity = 0.0f;
	_lightColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
	_currentTextColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
	_textShadow = false;
	_coord_sys = CoordSys(0.0f, 1024.0f, 0.0f, 768.0f);
	_scissorEnabled = false;
	_viewport = ScreenRect(0, 0, 100, 100);
	_scissorRect = ScreenRect(0, 0, 1024, 768);
	_animation_counter = 0;
	_current_frame_diff = 0;
	_lightningActive = false;
	_lightningCurTime = 0;
	_lightningEndTime = 0;
	_target = VIDEO_TARGET_SDL_WINDOW;
	_x = 0;
	_y = 0;

	if (VIDEO_DEBUG) 
		cout << "VIDEO: GameVideo constructor invoked\n";
	
	strcpy(_nextTempFile, "00000000");	
}


//-----------------------------------------------------------------------------
// SingletonInitialize: called to actually initialize the video engine
//-----------------------------------------------------------------------------

bool GameVideo::SingletonInitialize()
{
	if(VIDEO_DEBUG)
		cout << "VIDEO: Initializing SDL subsystem\n";
		
	// Set the window title and icon name
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) 
	{
		fprintf(stderr, "Barf! SDL Video Initialization failed!\n");
		exit(1);
	}

	// initialize SDL_ttf
	if(TTF_Init() < 0)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: SDL_ttf did not initialize! (TTF_Init() failed)" << endl;
		return false;
	}

	if(VIDEO_DEBUG)
		cout << "VIDEO: setting video mode\n";

	// Get the current system color depth and resolution
	const SDL_VideoInfo* video_info (0);
	video_info = SDL_GetVideoInfo ();

	if (video_info)
	{
		// Set the resolution to be the highest possible (lower than the user one)
		if (video_info->current_w > 1280 && video_info->current_h > 1024)
		{
			SetResolution (1280, 1024);
		}
		else if (video_info->current_w > 1024 && video_info->current_h > 768)
		{
			SetResolution (1024, 768);
		}
		else if (video_info->current_w > 800 && video_info->current_h > 600)
		{
			SetResolution (800, 600);
		}
		else
		{
			SetResolution (640, 480);
		}
	}
	else
	{
		// Default resoltion if we could not retrieve the resolution of the user
		SetResolution(1024, 768);
	}

	SetFullscreen(false);
	
	if(!ApplySettings())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ChangeMode() failed in GameVideo::SingletonInitialize()!" << endl;
		return false;
	}

	if(VIDEO_DEBUG)
		cout << "VIDEO: Loading default font\n";

	if(!LoadFont("img/fonts/tarnhalo.ttf", "debug_font", 16))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: Could not load tarnhalo.ttf file!" << endl;
		return false;
	}

	if(VIDEO_DEBUG)
		cout << "VIDEO: Creating texture sheets\n";

	
	// create our default texture sheets

	if(!_CreateTexSheet(512, 512, VIDEO_TEXSHEET_32x32, false))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: could not create default 32x32 tex sheet!" << endl;
		return false;
	}
	
	if(!_CreateTexSheet(512, 512, VIDEO_TEXSHEET_32x64, false))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: could not create default 32x64 tex sheet!" << endl;
		return false;
	}

	if(!_CreateTexSheet(512, 512, VIDEO_TEXSHEET_64x64, false))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: could not create default 64x64 tex sheet!" << endl;
		return false;
	}

	if(!_CreateTexSheet(512, 512, VIDEO_TEXSHEET_ANY,   true))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: could not create default static  var-sized tex sheet!" << endl;
		return false;
	}

	if(!_CreateTexSheet(512, 512, VIDEO_TEXSHEET_ANY,   false))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: could not create default var-sized tex sheet!" << endl;
		return false;
	}

	// create the GUI
	_gui = new private_video::GUI;

	// make a temp directory and make sure it doesn't contain any files
	// (in case the game crashed during a previous run, leaving stuff behind)

	// enable text shadows
	EnableTextShadow(true);

	// set default menu cursor
	
	if(VIDEO_DEBUG)
		cout << "VIDEO: Setting default menu cursor" << endl;
	
	if(!SetDefaultCursor("img/menus/cursor.png"))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: problem loading default menu cursor" << endl;
	}
	

	if(VIDEO_DEBUG)
		cout << "VIDEO: Erasing the screen\n";


	// set up the screen for rendering
	if(!Clear())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: first call to Clear() in GameVideo::SingletonInitialize() failed!" << endl;
		return false;
	}
	
	if(!Display(0))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: Display() in GameVideo::SingletonInitialize() failed!" << endl;
		return false;
	}	

	if(!Clear())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: second call to Clear() in GameVideo::SingletonInitialize() failed!" << endl;
		return false;
	}

	if(VIDEO_DEBUG)
		cout << "VIDEO: GameVideo::SingletonInitialize() returned successfully" << endl;
	
	
	return true;
}


//-----------------------------------------------------------------------------
// MakeScreenshot: create a screenshot and save as screenshot.jpg
//-----------------------------------------------------------------------------

bool GameVideo::MakeScreenshot()
{
	if(VIDEO_DEBUG)
		cout << "VIDEO: Entering MakeScreenshot()" << endl;

	// Retrieve width/height of the viewport
	GLint viewportDims[4];
	glGetIntegerv(GL_VIEWPORT, viewportDims);

	// Buffer to store the image before it is flipped
	void * buffer = malloc(viewportDims[2] * viewportDims[3] * 3);

	// Read pixel data
	glReadPixels(0, 0, viewportDims[2], viewportDims[3], GL_RGB, GL_UNSIGNED_BYTE, buffer);

	if(glGetError())
	{	
		if(VIDEO_DEBUG)
			cerr << "VIDEO_DEBUG: glReadPixels() returned an error inside GameVideo::CaptureScreen!" << endl;
		
		free(buffer);
		return false;
	}


	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	FILE * outfile;	
	if((outfile = fopen("screenshot.jpg", "wb")) == NULL) 
	{	
		cerr << "Could not open screenshot.jpg for writing!" << endl;
		free(buffer);
		return false;
	}

	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = viewportDims[2]; 	
	cinfo.image_height = viewportDims[3];
	cinfo.input_components = 3;		
	cinfo.in_color_space = JCS_RGB; 	

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 70, TRUE);
	jpeg_start_compress(&cinfo, TRUE);

	JSAMPLE ** row_pointers = (JSAMPLE **)malloc(sizeof(JSAMPLE *) * viewportDims[3]);

	for(int32 line = 0; line < viewportDims[3]; line++)
	{
		row_pointers[line] = ((unsigned char *)buffer) + ((viewportDims[3] - line - 1) * viewportDims[2] * 3);
	}

	jpeg_write_scanlines(&cinfo, row_pointers, viewportDims[3]);
	jpeg_finish_compress(&cinfo);
	fclose(outfile);

	jpeg_destroy_compress(&cinfo);

	free(buffer);
	free(row_pointers);

	return true;
}


//-----------------------------------------------------------------------------
// ~GameVideo
//-----------------------------------------------------------------------------

GameVideo::~GameVideo() 
{ 
	if (VIDEO_DEBUG) 
		cout << "VIDEO: GameVideo destructor invoked" << endl;

	// destroy particle manager
	_particle_manager.Destroy();	
	
	// delete GUI
	delete _gui;
		
	// delete font properties
	
	map<string, FontProperties *>::iterator iFontProp    = _font_map.begin();
	map<string, FontProperties *>::iterator iFontPropEnd = _font_map.end();
	
	while(iFontProp != _font_map.end())
	{
		FontProperties *fp = iFontProp->second;		

		if(fp->ttf_font)
			TTF_CloseFont(fp->ttf_font);

		if(fp->glyph_cache)
		{
			for(std::map<uint16, FontGlyph *>::iterator glyphitr = fp->glyph_cache->begin(); glyphitr != fp->glyph_cache->end(); glyphitr++)
			{
				// Possibly de-init the GL texture
				delete (*glyphitr).second;
			}

			delete fp->glyph_cache;
		}

		delete fp;		
		++iFontProp;
	}
	
		
	// uninitialize SDL_ttf
	TTF_Quit();
	
	// delete texture sheets
	vector<TexSheet *>::iterator iSheet      = _texSheets.begin();
	vector<TexSheet *>::iterator iSheetEnd   = _texSheets.end();

	while(iSheet != iSheetEnd)
	{
		delete *iSheet;
		++iSheet;
	}
	
	// delete images
	map<string, Image *>::iterator iImage     = _images.begin();
	map<string, Image *>::iterator iImageEnd  = _images.end();

	while(iImage != iImageEnd)
	{
		delete iImage->second;
		++iImage;
	}
}


//-----------------------------------------------------------------------------
// SetCoordSys: sets the current coordinate system
//-----------------------------------------------------------------------------

void GameVideo::SetCoordSys(const CoordSys &coordSys)
{
	_coord_sys = coordSys;
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	
	glOrtho(_coord_sys.GetLeft(), _coord_sys.GetRight(), _coord_sys.GetBottom(), _coord_sys.GetTop(), -1, 1);
	
	// Removed this code bleow
 	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


//-----------------------------------------------------------------------------
// SetCoordSys: sets the current coordinate system
//-----------------------------------------------------------------------------

void GameVideo::SetCoordSys
(
	float left, 
	float right, 
	float bottom, 
	float top
)
{
	SetCoordSys(CoordSys(left, right, bottom, top));	
}


//-----------------------------------------------------------------------------
// SetDrawFlags: used for controlling various flags like blending, flipping, etc.
//-----------------------------------------------------------------------------

void GameVideo::SetDrawFlags(int32 firstflag, ...)
{
	int32 n;
	int32 flag;
	va_list args;

	va_start(args, firstflag);
	for (n=0;;n++) 
	{
		flag = (n==0) ? firstflag : va_arg(args, int32);
		switch (flag) {
		case 0: goto done;

		case VIDEO_X_LEFT: _xalign=-1; break;
		case VIDEO_X_CENTER: _xalign=0; break;
		case VIDEO_X_RIGHT: _xalign=1; break;

		case VIDEO_Y_TOP: _yalign=1; break;
		case VIDEO_Y_CENTER: _yalign=0; break;
		case VIDEO_Y_BOTTOM: _yalign=-1; break;

		case VIDEO_X_NOFLIP: _xflip=0; break;
		case VIDEO_X_FLIP: _xflip=1; break;

		case VIDEO_Y_NOFLIP: _yflip=0; break;
		case VIDEO_Y_FLIP: _yflip=1; break;

		case VIDEO_NO_BLEND: _blend=0; break;
		case VIDEO_BLEND: _blend=1; break;
		case VIDEO_BLEND_ADD: _blend=2; break;

		default:
			if(VIDEO_DEBUG)
				cerr << "Unknown flag " << flag << " passed to SetDrawFlags()\n";
			break;
		}
	}
done:
	va_end(args);
}


//-----------------------------------------------------------------------------
// ApplySettings: after you change the resolution and/or fullscreen settings,
//                calling this function actually applies those settings
//-----------------------------------------------------------------------------

bool GameVideo::ApplySettings()
{
	// Used by the game Hero of Allacrost, an SDL application
	if (_target == VIDEO_TARGET_SDL_WINDOW) {
		// Losing GL context, so unload images first
		UnloadTextures();

		int32 flags = SDL_OPENGL;
		
		if (_temp_fullscreen == true) {
			flags |= SDL_FULLSCREEN;
		}

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		if (!SDL_SetVideoMode(_temp_width, _temp_height, 0, flags)) {
		// RGB values of 1 for each and 8 for depth seemed to be sufficient.
		// 565 and 16 here because it works with them on this computer. 
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5); 
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);

			if (!SDL_SetVideoMode(_temp_width, _temp_height, 0, flags)) {
				if (VIDEO_DEBUG) {
					cerr << "VIDEO ERROR: SDL_SetVideoMode() failed with error: " << SDL_GetError() << endl;
				}
				_temp_fullscreen = _fullscreen;
				_temp_width      = _width;
				_temp_height     = _height;

				if (_width > 0) { // Test to see if we already had a valid video mode
					ReloadTextures();
				}
				return false;
			}
		}

		_width      = _temp_width;
		_height     = _temp_height;
		_fullscreen = _temp_fullscreen;

		ReloadTextures();
		
		EnableFog(_fogColor, _fogIntensity);
		
		return true;
	} // if (_target == VIDEO_TARGET_SDL_WINDOW)

	// Used by the Allacrost editor, which uses QT4
	else if (_target == VIDEO_TARGET_QT_WIDGET)
	{
		_width      = _temp_width;
		_height     = _temp_height;
		_fullscreen = _temp_fullscreen;
		
		return true;
	}
	
	return false;
}


//-----------------------------------------------------------------------------
// SetViewport: set the rectangle of the screen onto which all drawing maps to, 
//              the arguments are percentages so 0, 0, 100, 100 would mean the 
//              whole screen
//-----------------------------------------------------------------------------

void GameVideo::SetViewport(float left, float right, float bottom, float top) 
{
	assert(left < right);
	assert(bottom < top);

	int32 l=int32(left*_width*.01f);
	int32 b=int32(bottom*_height*.01f);
	int32 r=int32(right*_width*.01f);
	int32 t=int32(top*_height*.01f);

	if (l<0) l=0;
	if (b<0) b=0;
	if (r>_width) r=_width;
	if (t>_height) t=_height;

	_viewport = ScreenRect(l, b, r-l+1, t-b+1);
	glViewport(l, b, r-l+1, t-b+1);
}


//-----------------------------------------------------------------------------
// Clear: clear the screen to black, it doesnt clear other buffers, that can be 
//        done by videostates that use them
//-----------------------------------------------------------------------------

bool GameVideo::Clear() 
{
	if(_usesLights)
		return Clear(_lightColor);
	else
		return Clear(Color::black);
}



//-----------------------------------------------------------------------------
// Clear: clear the screen to given color, it doesnt clear other buffers, that can be 
//        done by videostates that use them
//-----------------------------------------------------------------------------

bool GameVideo::Clear(const Color &c) 
{
	SetViewport(0,100,0,100);
	glClearColor(c[0], c[1], c[2], c[3]);
	glClear(GL_COLOR_BUFFER_BIT);
		
	_numTexSwitches = 0;
	
	if(glGetError())
		return false;
		
	return true;
}

//-----------------------------------------------------------------------------
// AccumulateLights: if real lights are enabled, then you must call DrawLight()
//                   for each light, and then call AccumulateLights() to
//                   save the lighting information into the overlay
//-----------------------------------------------------------------------------
/*bool GameVideo::AccumulateLights()
{
	if(_lightOverlay != 0xFFFFFFFF)
	{
		_BindTexture(_lightOverlay);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 1024, 1024, 0);
	}
	
	return true;
}*/


//-----------------------------------------------------------------------------
// Display: if running in double buffered mode then flip the other buffer to the 
//          screen
//-----------------------------------------------------------------------------

bool GameVideo::Display(int32 frameTime) 
{
	// update particle effects
	_particle_manager.Update(frameTime);

	// update shaking effect	
	_PushContext();
	SetCoordSys(0, 1024, 0, 768);
	_UpdateShake(frameTime);
	
	// update lightning timer
	_lightningCurTime += frameTime;
	
	if(_lightningCurTime > _lightningEndTime)
		_lightningActive = false;
	
	// show an overlay over the screen if we're fading
	if(_fader.ShouldUseFadeOverlay())
	{
		Color c = _fader.GetFadeOverlayColor();
		StillImage fadeOverlay;
		fadeOverlay.SetDimensions(1024.0f, 768.0f);
		fadeOverlay.SetColor(c);
		LoadImage(fadeOverlay);		
		SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
		PushState();
		Move(0, 0);
		DrawImage(fadeOverlay);		
		PopState();
		DeleteImage(fadeOverlay);
	}

	// this must be called before DrawFPS and all, because we only
	// want to count texture switches related to the game itself, not the
	// ones used to draw debug text and things like that.
	
	if(_advancedDisplay)
		_DEBUG_ShowAdvancedStats();

	if(_fpsDisplay)
		DrawFPS(frameTime);
		
	if(!_DEBUG_ShowTexSheet())
	{
		if(VIDEO_DEBUG)
		{
			// keep track of whether we've already shown this error.
			// If we've shown it once, stop showing it so we don't clog up
			// the debug output with the same message 1000 times
			static bool hasFailed = false;
			
			if(!hasFailed)
			{
				cerr << "VIDEO ERROR: _DEBUG_ShowTexSheet() failed\n";
				hasFailed = true;
			}
		}
	}

	_PopContext();

	SDL_GL_SwapBuffers();
	
	_fader.Update(frameTime);	
	
	// update animation timers
	
	int32 oldFrameIndex = _animation_counter / VIDEO_ANIMATION_FRAME_PERIOD;
	_animation_counter += frameTime;
	int32 currentFrameIndex = _animation_counter / VIDEO_ANIMATION_FRAME_PERIOD;
	
	_current_frame_diff = currentFrameIndex - oldFrameIndex;
	
	return true;
}


//-----------------------------------------------------------------------------
// IsFullscreen: returns true if we're in fullscreen mode, false if windowed
//-----------------------------------------------------------------------------

bool GameVideo::IsFullscreen()
{
	return _fullscreen;
}


//-----------------------------------------------------------------------------
// SetFullscreen: if you pass in true, makes the game fullscreen, otherwise
//                makes it windowed. Returns false on failure.
// NOTE: to actually apply the change, call ApplySettings()
//-----------------------------------------------------------------------------

bool GameVideo::SetFullscreen(bool fullscreen)
{
	_temp_fullscreen = fullscreen;
	return true;	
}


//-----------------------------------------------------------------------------
// ToggleFullscreen: if game is currently windowed, makes it fullscreen and
//                   vica versa. Returns false on failure.
// NOTE: to actually apply the change, call ApplySettings()
//-----------------------------------------------------------------------------

bool GameVideo::ToggleFullscreen()
{
	return SetFullscreen(!_temp_fullscreen);
}


//-----------------------------------------------------------------------------
// SetResolution: sets the resolution
// NOTE: to actually apply the change, call ApplySettings()
//-----------------------------------------------------------------------------

bool GameVideo::SetResolution(int32 width, int32 height)
{
	if (width <= 0 || height <= 0)
	{
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: invalid width and/or height passed to SetResolution!" << endl;
		return false;
	}
	
	_temp_width  = width;
	_temp_height = height;
	return true;
}


//-----------------------------------------------------------------------------
// _DEBUG_ShowAdvancedStats: display # of tex switches and other useful runtime
//                           statistics
//-----------------------------------------------------------------------------

bool GameVideo::_DEBUG_ShowAdvancedStats()
{
	// display to screen	
	char text[50];
	sprintf(text, "Switches: %d\nParticles: %d", _numTexSwitches, _particle_manager.GetNumParticles());
	
	if( !SetFont("debug_font"))
		return false;
		
	Move(896.0f, 690.0f);
	if( !DrawText(text))
		return false;
			
	return true;
}



//-----------------------------------------------------------------------------
// Move: move relativly x+=rx, y+=ry
//-----------------------------------------------------------------------------

void GameVideo::Move(float tx, float ty) 
{
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif
	glLoadIdentity();
	glTranslatef(tx, ty, 0);
	_x = tx;
	_y = ty;
}

//-----------------------------------------------------------------------------
// MoveRelative: 
//-----------------------------------------------------------------------------
void GameVideo::MoveRelative(float tx, float ty) 
{
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif
	glTranslatef(tx, ty, 0);
	_x += tx;
	_y += ty;
}

//-----------------------------------------------------------------------------
// GetDrawPosition: Get the location that the draw cursor is currently located at.
//-----------------------------------------------------------------------------
void GameVideo::GetDrawPosition(float &x, float &y)
{
	x = _x;
	y = _y;
}

//-----------------------------------------------------------------------------
// Rotate: rotates the coordinate axes anticlockwise by acAngle degrees, think 
//         about this CARFULLY before you call it
//-----------------------------------------------------------------------------

void GameVideo::Rotate(float acAngle) 
{
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif
	glRotatef(acAngle, 0, 0, 1);
}


//-----------------------------------------------------------------------------
// Scale: scales the coordinate axes by xScale and yScale respectively
//-----------------------------------------------------------------------------

void GameVideo::Scale(float xScale, float yScale)
{
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif

	glScalef(xScale, yScale, 1.0f);
}



//-----------------------------------------------------------------------------
// PushState: saves your current position in a stack, bewarned this stack is 
//            small ~32 so use it wisely
//-----------------------------------------------------------------------------

void GameVideo::PushState() 
{
	_PushContext();
}


//-----------------------------------------------------------------------------
// PopState: restores last position, read PushState()
//-----------------------------------------------------------------------------

void GameVideo::PopState() 
{
	_PopContext();
}


//-----------------------------------------------------------------------------
// SetMenuSkin: sets the menu skin to use the menu border images starting with
//              imageBaseName, and with an interior of fill color
//-----------------------------------------------------------------------------

bool GameVideo::SetMenuSkin
(
	const std::string &imgBaseName,	
	const Color  &fillColor
)
{
	return SetMenuSkin(imgBaseName, fillColor, fillColor, fillColor, fillColor);
}


//-----------------------------------------------------------------------------
// SetMenuSkin: sets the menu skin to use the menu border images starting with
//              imageBaseName, and with an interior whose 4 vertices' colors
//              are given by fillColor_TL, fillColor_TR, fillColor_BL and
//              fillColor_BR
//-----------------------------------------------------------------------------

bool GameVideo::SetMenuSkin
(
	const std::string &imgBaseName,
	const Color  &fillColor_TL,
	const Color  &fillColor_TR,
	const Color  &fillColor_BL,
	const Color  &fillColor_BR
)
{
	return _gui->SetMenuSkin
	(
		imgBaseName + "_tl.png",
		imgBaseName + "_t.png",
		imgBaseName + "_tr.png",
		imgBaseName + "_l.png",
		imgBaseName + "_r.png",
		imgBaseName + "_bl.png",
		imgBaseName + "_b.png",
		imgBaseName + "_br.png",
		imgBaseName + "_tri_t.png",
		imgBaseName + "_tri_l.png",
		imgBaseName + "_tri_r.png",
		imgBaseName + "_tri_b.png",
		imgBaseName + "_quad.png",
		
		fillColor_TL,
		fillColor_TR,
		fillColor_BL,
		fillColor_BR,
		
		""     // no background image
	);
}


//-----------------------------------------------------------------------------
// SetMenuSkin: sets the menu skin to use the menu border images starting with
//              imageBaseName, and with a background image
//-----------------------------------------------------------------------------

bool GameVideo::SetMenuSkin
(
	const std::string &imgBaseName,
	const std::string &backgroundImage,
	const Color  &fillColor_TL,
	const Color  &fillColor_TR,
	const Color  &fillColor_BL,
	const Color  &fillColor_BR	
)
{
	return _gui->SetMenuSkin
	(
		imgBaseName + "_tl.png",
		imgBaseName + "_t.png",
		imgBaseName + "_tr.png",
		imgBaseName + "_l.png",
		imgBaseName + "_r.png",
		imgBaseName + "_bl.png",
		imgBaseName + "_b.png",
		imgBaseName + "_br.png",
		imgBaseName + "_tri_t.png",
		imgBaseName + "_tri_l.png",
		imgBaseName + "_tri_r.png",
		imgBaseName + "_tri_b.png",
		imgBaseName + "_quad.png",
		fillColor_TL,
		fillColor_TR,
		fillColor_BL,
		fillColor_BR,
		backgroundImage
	);
}


//-----------------------------------------------------------------------------
// SetMenuSkin: sets the menu skin to use the menu border images starting with
//              imageBaseName, a background image, and with an interior of fill color
//-----------------------------------------------------------------------------

bool GameVideo::SetMenuSkin
(
	const std::string &imgBaseName,
	const std::string &backgroundImage,
	const Color  &fillColor
)
{
	return SetMenuSkin(imgBaseName, backgroundImage, fillColor, fillColor, fillColor, fillColor);
}


//-----------------------------------------------------------------------------
// _BindTexture: wraps call to glBindTexture(), plus some extra checking to
//              discard the call if we try to bind the same texture twice
//-----------------------------------------------------------------------------

bool GameVideo::_BindTexture(GLuint texID)
{
	if(texID != _lastTexID)
	{
		_lastTexID = texID;
		glBindTexture(GL_TEXTURE_2D, texID);
		++_numTexSwitches;
	}
	
	if(glGetError())
		return false;
	
	return true;
}


//-----------------------------------------------------------------------------
// ToggleAdvancedDisplay: toggles advanced display. When advanced display is
//                        enabled, you can see things like how many texture
//                        switches occurred during the current frame, etc.
//-----------------------------------------------------------------------------

bool GameVideo::ToggleAdvancedDisplay()
{
	_advancedDisplay = !_advancedDisplay;
	return true;
}



//-----------------------------------------------------------------------------
// _CreateMenu: creates menu image descriptor
//-----------------------------------------------------------------------------
bool GameVideo::_CreateMenu(StillImage &menu, float width, float height, float & innerWidth, float & innerHeight, int32 edgeVisibleFlags, int32 edgeSharedFlags)
{
	return _gui->CreateMenu(menu, width, height, innerWidth, innerHeight, edgeVisibleFlags, edgeSharedFlags);
}


//-----------------------------------------------------------------------------
// EnableSceneLighting: sets lighting parameters for the scene, actually just a color
//              unless we change the lighting system later on.
//              NOTE: the color's alpha value (i.e. color[3]) must be 1.0f
//-----------------------------------------------------------------------------
bool GameVideo::EnableSceneLighting(const Color &color)
{
	_lightColor = color;

	if(color[3] != 1.0f)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: color passed to EnableSceneLighting() had alpha other than 1.0f!" << endl;
		_lightColor[3] = 1.0f;
	}

	return true;
}

//-----------------------------------------------------------------------------
// DisableSceneLighting: Turn off scene lighting
//-----------------------------------------------------------------------------
void GameVideo::DisableSceneLighting()
{
	_lightColor = Color::white;
}

//-----------------------------------------------------------------------------
// GetLighting: returns the scene lighting color
//-----------------------------------------------------------------------------

Color &GameVideo::GetSceneLightingColor()
{
	return _lightColor;
}



//-----------------------------------------------------------------------------
// EnableFog: sets fog parameters. Fog color is usually gray, and intensity can
//         be from 0.0 (no fog) to 1.0 (entire screen is gray)
//         To turn off fog, just call this function with intensity of 0.0f
//-----------------------------------------------------------------------------
bool GameVideo::EnableFog(const Color &color, float intensity)
{
	// check if intensity is within bounds. If not, clamp it but display an
	// error message
	
	if(intensity < 0.0f)
	{
		intensity = 0.0f;
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: negative intensity passed to SetFog()" << endl;
	}
	else if(intensity > 1.0f)
	{
		intensity = 1.0f;
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: intensity larger than 1.0f passed to SetFog()" << endl;
	}
	
	// set the parameters
	_fogColor = color;
	_fogIntensity = intensity;
	
	// apply the new settings with OpenGL
	if(intensity == 0.0f)
	{
		glDisable(GL_FOG);
	}
	else
	{
		glEnable(GL_FOG);
		glHint(GL_FOG_HINT, GL_DONT_CARE);
		glFogf(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_START, 0.0f - intensity);
		glFogf(GL_FOG_END, 1.0f - intensity);
		glFogfv(GL_FOG_COLOR, (GLfloat *)color.GetColors());
	}
	
	return true;	
}

//-----------------------------------------------------------------------------
// DisableFog: Turns off fog
//-----------------------------------------------------------------------------
void GameVideo::DisableFog() 
{
	glDisable(GL_FOG);
	_fogIntensity = 0.0f;
}


//-----------------------------------------------------------------------------
// SetTransform: set current OpenGL modelview matrix to the 4x4 matrix (16 values)
//               that's passed in
//-----------------------------------------------------------------------------
void GameVideo::SetTransform(float m[16])
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(m);
}

//-----------------------------------------------------------------------------
// EnablePointLights: call if this map uses point lights
//-----------------------------------------------------------------------------
bool GameVideo::EnablePointLights()
{
	_lightOverlay = _CreateBlankGLTexture(1024, 1024);

	_usesLights = true;
	
	return true;	
}

//-----------------------------------------------------------------------------
// DisablePointLights: call when done with point lights
//-----------------------------------------------------------------------------
void GameVideo::DisablePointLights()
{
	if(_lightOverlay != 0xFFFFFFFF)
	{
		_DeleteTexture(_lightOverlay);
	}
		
	_lightOverlay = 0xFFFFFFFF;

	_usesLights = false;
}


//-----------------------------------------------------------------------------
// ApplyLightingOverlay: call after all map images are drawn to apply lighting. 
//                       All menu and text rendering should occur AFTER this 
//                       call, so that they are not affected by lighting.
//-----------------------------------------------------------------------------
bool GameVideo::ApplyLightingOverlay()
{
	if(_lightOverlay != 0xFFFFFFFF)
	{
		// Copy light overlay to opengl texture
		_BindTexture(_lightOverlay);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 1024, 1024, 0);

		CoordSys tempCoordSys = _coord_sys;

		SetCoordSys(0.0f, 1.0f, 0.0f, 1.0f);
		float xlo = 0.0f, ylo = 0.0f, xhi = 1.0f, yhi = 1.0f;
		glEnable(GL_TEXTURE_2D);
		
		float mx = _width / 1024.0f;
		float my = _height / 1024.0f;

		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		
		_BindTexture(_lightOverlay);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);	
		glVertex2f(xlo, ylo); //bl
		glTexCoord2f(mx, 0.0f);
		glVertex2f(xhi, ylo); //br
		glTexCoord2f(mx, my);
		glVertex2f(xhi, yhi);//tr
		glTexCoord2f(0.0f, my);
		glVertex2f(xlo, yhi);//tl
		glEnd();		
		SetCoordSys(tempCoordSys.GetLeft(), tempCoordSys.GetRight(), tempCoordSys.GetBottom(), tempCoordSys.GetTop());
	}
		
	return true;
}


//-----------------------------------------------------------------------------
// CaptureScreen: captures the contents of the screen and saves it to an image
//                descriptor. Note that this function ignores the filename
//                and isStatic properties of the image descriptor. It will
//                obey the color property though, in case you want to create
//                a funky colored image of the screen...
//
//                NOTE: this function assumes that you will never have two
//                      captured screens loaded at the same time. If it detects
//                      that you try to capture the screen even though a
//                      capture is already in memory, it is considered an error
//-----------------------------------------------------------------------------

bool GameVideo::CaptureScreen(StillImage &id)
{

	if(VIDEO_DEBUG)
		cout << "VIDEO: Entering CaptureScreen()" << endl;

	// Retrieve width/height of the viewport
	GLint viewportDims[4];
	glGetIntegerv(GL_VIEWPORT, viewportDims);

	// Set up the loadInfo struct
	ImageLoadInfo loadInfo;
	loadInfo.width = viewportDims[2];
	loadInfo.height = viewportDims[3];
	loadInfo.pixels = malloc(viewportDims[2] * viewportDims[3] * 4);

	// Buffer to store the image before it is flipped
	void * buffer = malloc(viewportDims[2] * viewportDims[3] * 4);

	// Read pixel data
	glReadPixels(0, 0, viewportDims[2], viewportDims[3], GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	if(glGetError())
	{	
		if(VIDEO_DEBUG)
			cerr << "VIDEO_DEBUG: glReadPixels() returned an error inside GameVideo::CaptureScreen!" << endl;
		
		free(buffer);
		free(loadInfo.pixels);
		return false;
	}

	// Flip the image
	for(int32 line = 0; line < viewportDims[3]; line++)
	{
		unsigned char * srcline = ((unsigned char *)buffer) + (line * viewportDims[2] * 4);
		unsigned char * destline = ((unsigned char *)loadInfo.pixels) + ((viewportDims[3] - line - 1) * viewportDims[2] * 4);

		memcpy(destline, srcline, viewportDims[2] * 4);
	}

	// Free the buffer
	free(buffer);

	// create an Image structure and store it our std::map of images
	Image *newImage = new Image(id._filename, loadInfo.width, loadInfo.height, false);

	// try to insert the image in a texture sheet
	TexSheet *sheet = _InsertImageInTexSheet(newImage, loadInfo, true);
	
	if(!sheet)
	{
		// this should never happen, unless we run out of memory or there
		// is a bug in the _InsertImageInTexSheet() function
		
		if(VIDEO_DEBUG)
			cerr << "VIDEO_DEBUG: GameVideo::_InsertImageInTexSheet() returned NULL!" << endl;
		
		free(loadInfo.pixels);
		return false;
	}
	
	newImage->ref_count = 1;
	
	// if width or height are zero, that means to use the dimensions of image
	if(id._width == 0.0f)
		id._width = (float) loadInfo.width;
	
	if(id._height == 0.0f)
		id._height = (float) loadInfo.height;

	// store the new image element
	ImageElement  element(newImage, 0, 0, id._width, id._height, 0.0f, 0.0f, 1.0f, 1.0f, id._color);
	id._elements.push_back(element);

	// finally, delete the buffer used to hold the pixel data
	free(loadInfo.pixels);

	return true;
}


//-----------------------------------------------------------------------------
// SetGamma: Sets a new gamma value using SDL_SetGamma()
//-----------------------------------------------------------------------------
void GameVideo::SetGamma(float value)
{
	_gamma_value = value;

	// Limit min/max gamma
	if (_gamma_value > 2.0f)
	{
		if(VIDEO_DEBUG) cout << "VIDEO: Tried to set gamma over 2.0f!" << endl;
		_gamma_value = 2.0f;
	}
	else if (_gamma_value < 0.0f)
	{
		if(VIDEO_DEBUG) cout << "VIDEO: Tried to set gamma below 0.0f!" << endl;
		_gamma_value = 0.0f;
	}

	SDL_SetGamma(_gamma_value, _gamma_value, _gamma_value);
}


//-----------------------------------------------------------------------------
// Returns the gamma value
//-----------------------------------------------------------------------------
float GameVideo::GetGamma()
{
	return _gamma_value;
}


//-----------------------------------------------------------------------------
// ToggleFPS: toggles the FPS display
//-----------------------------------------------------------------------------

void GameVideo::ToggleFPS()
{
	_fpsDisplay = !_fpsDisplay;
}


//-----------------------------------------------------------------------------
// _CreateTempFilename
//-----------------------------------------------------------------------------

string GameVideo::_CreateTempFilename(const string &extension)
{
	// figure out the temp filename to return
	string filename = "/tmp/allacrost_TEMP_";
	filename += _nextTempFile;
	filename += extension;
	
	// increment the 8-character temp name
	// Note: assume that the temp name is currently set to
	//       a valid name
	
	
	for(int digit = 7; digit >= 0; --digit)
	{
		++_nextTempFile[digit];		
		
		if(_nextTempFile[digit] > 'z')
		{
			if(digit==0)
			{
				if(VIDEO_DEBUG)
					cerr << "VIDEO ERROR: _nextTempFile went past 'zzzzzzzz'" << endl;
				return filename;
			}
			
			_nextTempFile[digit] = '0';
		}
		else
		{
			if(_nextTempFile[digit] > '9' && _nextTempFile[digit] < 'a')
				_nextTempFile[digit] = 'a';
		
			// if the digit did not overflow, then we don't need to carry over
			break;
		}		
	}	
	
	return filename;
}


//-----------------------------------------------------------------------------
// _PushContext: pushes transformation, coordsys, and draw flags so that
//               GameVideo doesn't trample on client's settings
//-----------------------------------------------------------------------------

void GameVideo::_PushContext()
{
	// push current modelview transformation
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	// save context information
	private_video::Context c;
	c.coordinate_system = _coord_sys;
	c.blend    = _blend;
	c.x_align   = _xalign;
	c.y_align   = _yalign;
	c.x_flip    = _xflip;
	c.y_flip    = _yflip;
	
	c.viewport = _viewport;
	c.scissor_rectangle = _scissorRect;
	c.scissoring_enabled = _scissorEnabled;
	
	c.font      = _current_font;
	c.text_color = _currentTextColor;
	
	_contextStack.push(c);
}


//-----------------------------------------------------------------------------
// _PopContext: pops transformation, coordsys, and draw flags so that
//              GameVideo doesn't trample on client's settings
//-----------------------------------------------------------------------------

void GameVideo::_PopContext()
{
	// restore context information and pop it from stack
	private_video::Context c = _contextStack.top();
	SetCoordSys(c.coordinate_system);
	_blend  = c.blend;
	_xalign = c.x_align;
	_yalign = c.y_align;
	_xflip  = c.x_flip;
	_yflip  = c.y_flip;
	
	_current_font      = c.font;
	_currentTextColor = c.text_color;
	
	_viewport = c.viewport;
	_scissorRect = c.scissor_rectangle;
	_scissorEnabled = c.scissoring_enabled;
	_contextStack.pop();

	// restore modelview transformation
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glViewport(_viewport.left, _viewport.top, _viewport.width, _viewport.height);
	
	if(_scissorEnabled)
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor(_scissorRect.left, _scissorRect.top, _scissorRect.width, _scissorRect.height);
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
	}
}


//-----------------------------------------------------------------------------
// _ConvertYAlign: convert a value like VIDEO_Y_BOTTOM to an offset like -1
//-----------------------------------------------------------------------------

int32 GameVideo::_ConvertYAlign(int32 yalign)
{
	switch(yalign)
	{
		case VIDEO_Y_BOTTOM:
			return -1;
		case VIDEO_X_CENTER:
			return 0;
		default:
			return 1;
	}
}


//-----------------------------------------------------------------------------
// _ConvertXAlign: convert a value like VIDEO_X_LEFT to an offset like -1
//-----------------------------------------------------------------------------

int32 GameVideo::_ConvertXAlign(int32 xalign)
{
	switch(xalign)
	{
		case VIDEO_X_LEFT:
			return -1;
		case VIDEO_X_CENTER:
			return 0;
		default:
			return 1;
	}
}

//-----------------------------------------------------------------------------
// SetDefaultCursor: sets the default menu cursor, returns false if it fails
//-----------------------------------------------------------------------------
bool GameVideo::SetDefaultCursor(const std::string &cursorImageFilename)
{
	_defaultMenuCursor.SetFilename(cursorImageFilename);
	return LoadImage(_defaultMenuCursor);
}


//-----------------------------------------------------------------------------
// GetDefaultCursor: sets the gefault menu cursor, returns NULL if none is set
//-----------------------------------------------------------------------------
StillImage *GameVideo::GetDefaultCursor()
{
	if(_defaultMenuCursor.GetWidth() != 0.0f)  // cheap test if image is valid
		return &_defaultMenuCursor;
	else
		return NULL;
}


//-----------------------------------------------------------------------------
// PushMatrix: pushes the current model view transformation on to stack
//             Note, this assumes that glMatrixMode(GL_MODELVIEW) has been called
//-----------------------------------------------------------------------------
void GameVideo::PushMatrix()
{
	glPushMatrix();
}

//-----------------------------------------------------------------------------
// PopMatrix: pops the current model view transformation from stack
//            Note, this assumes that glMatrixMode(GL_MODELVIEW) has been called
//-----------------------------------------------------------------------------
void GameVideo::PopMatrix()
{
	glPopMatrix();
}

//-----------------------------------------------------------------------------
// Intersect: intersects this rect with another, and stores the resulting
//            rectangle. If the two rectangles don't intersect, then all
//            member variables are zero
//-----------------------------------------------------------------------------
void ScreenRect::Intersect(const ScreenRect &rect)
{
	left   = max(left,   rect.left);
	top    = max(top,    rect.top);

	int32 right  = min(left + width - 1,  rect.left + rect.width - 1);
	int32 bottom = min(top + height - 1, rect.top + rect.height - 1);
	
	if(left > right || top > bottom)
	{
		left = right = width = height = 0;
	}
	else
	{
		width  = right - left + 1;
		height = bottom - top + 1;
	}
}

//-----------------------------------------------------------------------------
// EnableScissoring: enable/disable scissoring
//-----------------------------------------------------------------------------
void GameVideo::EnableScissoring(bool enable)
{
	_scissorEnabled = enable;
	
	if(enable)
		glEnable(GL_SCISSOR_TEST);
	else
		glDisable(GL_SCISSOR_TEST);
}


//-----------------------------------------------------------------------------
// SetScissorRect: set the scissoring rectangle, coordinates relative to the
//                 current coord sys
//-----------------------------------------------------------------------------
void GameVideo::SetScissorRect(float left, float right, float bottom, float top)
{
	_scissorRect = CalculateScreenRect(left, right, bottom, top);
	glScissor(_scissorRect.left, _scissorRect.top, _scissorRect.width, _scissorRect.height);	
}


//-----------------------------------------------------------------------------
// SetScissorRect: set the scissoring rectangle
//-----------------------------------------------------------------------------
void GameVideo::SetScissorRect(const ScreenRect &rect)
{
	_scissorRect = rect;
	glScissor(rect.left, rect.top, rect.width, rect.height);
}

//-----------------------------------------------------------------------------
// CalculateScreenRect: calculate a rectangle in screen coordinates given one
//                       using our current coordinate system
//-----------------------------------------------------------------------------
ScreenRect GameVideo::CalculateScreenRect(float left, float right, float bottom, float top)
{
	ScreenRect rect;
	
	int32 scr_left    = _ScreenCoordX(left);
	int32 scr_right   = _ScreenCoordX(right);
	int32 scr_bottom  = _ScreenCoordY(bottom);
	int32 scr_top     = _ScreenCoordY(top);
	
	int32 temp;
	if(scr_left > scr_right)
	{
		temp = scr_left;
		scr_left = scr_right;
		scr_right = temp;
	}
	
	if(scr_top > scr_bottom)
	{
		temp = scr_top;
		scr_top = scr_bottom;
		scr_bottom = temp;
	}

	rect.top    = scr_top;
	rect.left   = scr_left;	
	rect.width  = scr_right - scr_left;
	rect.height = scr_bottom - scr_top;
	
	return rect;
}


//-----------------------------------------------------------------------------
// _ScreenCoordX
//-----------------------------------------------------------------------------

int32 GameVideo::_ScreenCoordX(float x)
{
	float percent;
	if(_coord_sys.GetLeft() < _coord_sys.GetRight())
		percent = (x - _coord_sys.GetLeft()) / (_coord_sys.GetRight() - _coord_sys.GetLeft());
	else
		percent = (x - _coord_sys.GetRight()) / (_coord_sys.GetLeft() - _coord_sys.GetRight());
	
	return int32(percent * float(_width));
}


//-----------------------------------------------------------------------------
// _ScreenCoordY
//-----------------------------------------------------------------------------

int32 GameVideo::_ScreenCoordY(float y)
{
	float percent;
	if(_coord_sys.GetTop() < _coord_sys.GetBottom())
		percent = (y - _coord_sys.GetTop()) / (_coord_sys.GetBottom() - _coord_sys.GetTop());
	else
		percent = (y - _coord_sys.GetBottom()) / (_coord_sys.GetTop() - _coord_sys.GetBottom());
	
	return int32(percent * float(_height));
}


//-----------------------------------------------------------------------------
// MakeLightning: creates a lightning effect
//-----------------------------------------------------------------------------

bool GameVideo::MakeLightning(const std::string &litFile)
{
	FILE *fp = fopen(litFile.c_str(), "rb");
	if(!fp)
		return false;
	
	int32 dataSize;
	if(!fread(&dataSize, 4, 1, fp))
	{
		fclose(fp);
		return false;
	}

	// since this file was created on windows, it uses little endian byte order
	// Check if this processor uses big endian, and reorder bytes if so.

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		dataSize = ((dataSize & 0xFF000000) >> 24) | 
		           ((dataSize & 0x00FF0000) >> 8) | 
		           ((dataSize & 0x0000FF00) << 8) |
		           ((dataSize & 0x000000FF) << 24);
	#endif

	uint8 *data = new uint8[dataSize];
	
	if(!fread(data, dataSize, 1, fp))
	{
		delete [] data;
		fclose(fp);
		return false;
	}
	
	fclose(fp);
	
	_lightningData.clear();
	
	for(int32 j = 0; j < dataSize; ++j)
	{
		float f = float(data[j]) / 255.0f;
		_lightningData.push_back(f);
	}
	
	delete [] data;	
	
	_lightningActive = true;
	_lightningCurTime = 0;
	_lightningEndTime = dataSize * 1000 / 100;
	
	return true;
}


//-----------------------------------------------------------------------------
// DrawLightning: draws lightning effect on the screen using a fullscreen overlay
//-----------------------------------------------------------------------------

bool GameVideo::DrawLightning()
{
	if(!_lightningActive)
		return true;
	
	// convert milliseconds elapsed into data points elapsed
	
	float t = _lightningCurTime * 100.0f / 1000.0f;
	
	int32 roundedT = static_cast<int32>(t);
	t -= roundedT;
	
	// get 2 separate data points and blend together (linear interpolation)
	
	float data1 = _lightningData[roundedT];
	float data2 = _lightningData[roundedT+1];

	float intensity = data1 * (1-t) + data2 * t;

	DrawFullscreenOverlay(Color(1.0f, 1.0f, 1.0f, intensity));
	
	return true;
}


//-----------------------------------------------------------------------------
// DrawFullscreenOverlay
//-----------------------------------------------------------------------------

bool GameVideo::DrawFullscreenOverlay(const Color &color)
{
	PushState();
	
	SetCoordSys(0.0f, 1.0f, 0.0f, 1.0f);
	SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, VIDEO_BLEND, 0);
	Move(0.0f, 0.0f);
	StillImage img;
	img.SetDimensions(1.0f, 1.0f);
	LoadImage(img);	
	DrawImage(img, color);	
	
	PopState();
	
	return true;
}


//-----------------------------------------------------------------------------
// SetTarget: lets video engine know if it's drawing to an SDL window or a
//            QT widget
//-----------------------------------------------------------------------------

bool GameVideo::SetTarget(VIDEO_TARGET target)
{
	if(target <= VIDEO_TARGET_INVALID || target >= VIDEO_TARGET_TOTAL)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: tried to set video engine to invalid target (" << target << ")" << endl;
		return false;
	}

	_target = target;	
	return true;
}


//-----------------------------------------------------------------------------
// DrawGrid: draws a grid going from (x,y) to (maxX, maxY), with horizontal and
//           vertical grid spacing of xstep and ystep, with a color 'c'
//-----------------------------------------------------------------------------

void GameVideo::DrawGrid(float x, float y, float xstep, float ystep, const Color &c)
{
	PushState();
	
	Move(0, 0);
	glBegin(GL_LINES);
	glColor4fv(&c[0]);
	
	float xMax = _coord_sys.GetRight();
	float yMax = _coord_sys.GetBottom();
	
	for(; x <= xMax; x += xstep)
	{
		glVertex2f(x, _coord_sys.GetBottom());
		glVertex2f(x, _coord_sys.GetTop());
	}

	for(; y <= yMax; y += ystep)
	{
		glVertex2f(_coord_sys.GetLeft(), y);
		glVertex2f(_coord_sys.GetRight(), y);
	}
	
	glEnd();
	PopState();
}



}  // namespace hoa_video
