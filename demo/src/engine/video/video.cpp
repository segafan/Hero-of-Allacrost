///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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
#include <vector>

#include "utils.h"
#include "video.h"
#include "context.h"
#include "gui.h"
#include "script.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video::private_video;





template<> hoa_video::GameVideo* Singleton<hoa_video::GameVideo>::_singleton_reference = NULL;

namespace hoa_video {

GameVideo* VideoManager = NULL;
bool VIDEO_DEBUG = false;

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
Color Color::blue  (0.0f, 0.0f, 1.0f, 1.0f);
Color Color::violet(0.0f, 0.0f, 1.0f, 1.0f);
Color Color::brown (0.6f, 0.3f, 0.1f, 1.0f);



float Lerp(float alpha, float initial, float final) {
	return alpha * final + (1.0f - alpha) * initial;
}



float RandomFloat(float a, float b) {
	if (a > b) {
		float c = a;
		a = b;
		b = c;
	}

	float r = static_cast<float>(rand() % 10001);
	return a + (b - a) * r / 10000.0f;
}



void RotatePoint(float &x, float &y, float angle) {
	float old_x = x;
	float cos_angle = cosf(angle);
	float sin_angle = sinf(angle);

	x = x * cos_angle - y * sin_angle;
	y = y * cos_angle + old_x * sin_angle;
}

//-----------------------------------------------------------------------------
// GameVideo class
//-----------------------------------------------------------------------------

GameVideo::GameVideo() {
	_width = 0;
	_height = 0;
	_fullscreen = false;
	_temp_width = 0;
	_temp_height = 0;
	_temp_fullscreen = false;
	_blend = 0;
	_x_align = -1;
	_y_align = -1;
	_x_flip = 0;
	_y_flip = 0;
	_current_debug_TexSheet = -1;
	_uses_lights = false;
	_light_overlay = 0xFFFFFFFF;
	_last_tex_id = 0xFFFFFFFF;
	_num_tex_switches = 0;
	_advanced_display = false;
	_fps_display = false;
	_x_shake = 0;
	_y_shake = 0;
	_gamma_value = 1.0f;
	_fog_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
	_fog_intensity = 0.0f;
	_light_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
	_current_text_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
	_text_shadow = false;
	_coord_sys = CoordSys(0.0f, 1024.0f, 0.0f, 768.0f);
	_scissor_enabled = false;
	_viewport = ScreenRect(0, 0, 100, 100);
	_scissor_rect = ScreenRect(0, 0, 1024, 768);
	_animation_counter = 0;
	_current_frame_diff = 0;
	_lightning_active = false;
	_lightning_current_time = 0;
	_lightning_end_time = 0;
	_target = VIDEO_TARGET_SDL_WINDOW;
	_x = 0;
	_y = 0;

	if (VIDEO_DEBUG)
		cout << "VIDEO: GameVideo constructor invoked\n";

	strcpy(_next_temp_file, "00000000");
}



GameVideo::~GameVideo() {
	if (VIDEO_DEBUG)
		cout << "VIDEO: GameVideo destructor invoked" << endl;

	_particle_manager.Destroy();

	GUIManager->SingletonDestroy();

	// Remove all loaded fonts and shutdown the font library
	for (map<string, FontProperties*>::iterator i = _font_map.begin(); i!= _font_map.end(); i++) {
		FontProperties* fp = i->second;

		if (fp->ttf_font)
			TTF_CloseFont(fp->ttf_font);

		if (fp->glyph_cache) {
			for (std::map<uint16, FontGlyph*>::iterator j = fp->glyph_cache->begin(); j != fp->glyph_cache->end(); j++) {
				delete (*j).second;
			}
			delete fp->glyph_cache;
		}

		delete fp;
	}
	TTF_Quit();

	// Delete all texture sheets and images
	for (vector<TexSheet*>::iterator i = _tex_sheets.begin(); i != _tex_sheets.end(); i++) {
		delete *i;
	}

	for (map<string, Image*>::iterator i = _images.begin(); i != _images.end(); i++) {
		delete i->second;
	}
}



bool GameVideo::SingletonInitialize() {
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		cerr << "VIDEO ERROR: SDL Video Initialization failed" << endl;
		return false;
	}

	if (TTF_Init() < 0) {
			cerr << "VIDEO ERROR: SDL_ttf did not initialize! (TTF_Init() failed)" << endl;
		return false;
	}

	// Get the current system color depth and resolution
	const SDL_VideoInfo* video_info(0);
	video_info = SDL_GetVideoInfo();

	hoa_script::ReadScriptDescriptor settings_lua;
	static const char* settings_filename = "dat/config/settings.lua";

	if (settings_lua.OpenFile(settings_filename) == false) {
		fprintf(stderr, "Failed to load file '%s'.\n", settings_filename);
		exit(1);
	}

	settings_lua.OpenTable("video_settings");
	int32 settings_width = settings_lua.ReadInt("screen_resx");
	int32 settings_height = settings_lua.ReadInt("screen_resy");
	bool settings_fullscreen = settings_lua.ReadBool("full_screen");
	settings_lua.CloseTable();

	settings_lua.CloseFile();

	if (video_info) {
		// Set the resolution to be the highest possible (lower than the user one)
		if (video_info->current_w > settings_width && video_info->current_h > settings_height) {
			SetResolution (settings_width, settings_height);
		}
		else if (video_info->current_w > 1024 && video_info->current_h > 768) {
			SetResolution (1024, 768);
		}
		else if (video_info->current_w > 800 && video_info->current_h > 600) {
			SetResolution (800, 600);
		}
		else {
			SetResolution (640, 480);
		}
	}
	else {
		// Default resoltion if we could not retrieve the resolution of the user
		SetResolution(settings_width, settings_height);
	}

	SetFullscreen(settings_fullscreen);

	if (ApplySettings() == false) {
		return false;
	}

	if (LoadFont("img/fonts/tarnhalo.ttf", "debug_font", 16) == false) {
		cerr << "VIDEO ERROR: Could not load tarnhalo.ttf file!" << endl;
		return false;
	}

	// Create our default set of texture sheets
	if (_CreateTexSheet(512, 512, VIDEO_TEXSHEET_32x32, false) == NULL) {
		cerr << "VIDEO ERROR: could not create default 32x32 tex sheet!" << endl;
		return false;
	}

	if (_CreateTexSheet(512, 512, VIDEO_TEXSHEET_32x64, false) == NULL) {
		cerr << "VIDEO ERROR: could not create default 32x64 tex sheet!" << endl;
		return false;
	}

	if (_CreateTexSheet(512, 512, VIDEO_TEXSHEET_64x64, false) == NULL) {
		cerr << "VIDEO ERROR: could not create default 64x64 tex sheet!" << endl;
		return false;
	}

	if (_CreateTexSheet(512, 512, VIDEO_TEXSHEET_ANY, true) == NULL) {
		cerr << "VIDEO ERROR: could not create default static  var-sized tex sheet!" << endl;
		return false;
	}

	if (_CreateTexSheet(512, 512, VIDEO_TEXSHEET_ANY, false) == NULL) {
		cerr << "VIDEO ERROR: could not create default var-sized tex sheet!" << endl;
		return false;
	}

	// Create and initialize the GUI sub-system
	GUIManager = GUISupervisor::SingletonCreate();
	GUIManager->SingletonInitialize();

	// make a temp directory and make sure it doesn't contain any files
	// (in case the game crashed during a previous run, leaving stuff behind)

	EnableTextShadow(true);

	if (SetDefaultCursor("img/menus/cursor.png") == false) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: problem loading default menu cursor" << endl;
	}

	// Set up the screen for rendering
	Clear();
	Display(0);
	Clear();

	_rectangle_image.SetFilename("");
	if (!_rectangle_image.Load())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: quad StillImage for rendering rectangles could not be created" << endl;
		return false;
	}

	return true;
} // bool GameVideo::SingletonInitialize()



void GameVideo::MakeScreenshot(const std::string& filename) {
	private_video::ImageLoadInfo buffer;

	// Retrieve the width and height of the viewport
	GLint viewport_dimensions[4];
	glGetIntegerv(GL_VIEWPORT, viewport_dimensions);

	// Buffer to store the image before it is flipped
	buffer.width = viewport_dimensions[2];
	buffer.height = viewport_dimensions[3];
	buffer.pixels = malloc(buffer.width * buffer.height * 3);

	// Read pixel data
	glReadPixels(0, 0, buffer.width, buffer.height, GL_RGB, GL_UNSIGNED_BYTE, buffer.pixels);

	if (glGetError()) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING:" << __FILE__ << ":" << __FUNCTION__ << "glReadPixels() returned an error" << endl;

		free(buffer.pixels);
		return;
	}

	// Vertically flip the image
	void* buffer_temp = malloc(buffer.width * buffer.height * 3);
	for (int32 i=0; i < buffer.height; ++i) {
		memcpy((uint8*)buffer_temp + i * buffer.width * 3, (uint8*)buffer.pixels + (buffer.height - i - 1) * buffer.width * 3, buffer.width * 3);
	}
	free(buffer.pixels);
	buffer.pixels = buffer_temp;

	_SaveJpeg(filename, buffer);
	free(buffer.pixels);
}



void GameVideo::SetCoordSys(const CoordSys& coordinate_system) {
	_coord_sys = coordinate_system;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(_coord_sys.GetLeft(), _coord_sys.GetRight(), _coord_sys.GetBottom(), _coord_sys.GetTop(), -1, 1);

 	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}



void GameVideo::SetDrawFlags(int32 first_flag, ...) {
	int32 flag = first_flag;
	va_list args;

	va_start(args, first_flag);
	while (flag != 0) {
		switch (flag) {
		case VIDEO_X_LEFT: _x_align = -1; break;
		case VIDEO_X_CENTER: _x_align = 0; break;
		case VIDEO_X_RIGHT: _x_align = 1; break;

		case VIDEO_Y_TOP: _y_align = 1; break;
		case VIDEO_Y_CENTER: _y_align = 0; break;
		case VIDEO_Y_BOTTOM: _y_align = -1; break;

		case VIDEO_X_NOFLIP: _x_flip = 0; break;
		case VIDEO_X_FLIP: _x_flip = 1; break;

		case VIDEO_Y_NOFLIP: _y_flip = 0; break;
		case VIDEO_Y_FLIP: _y_flip = 1; break;

		case VIDEO_NO_BLEND: _blend = 0; break;
		case VIDEO_BLEND: _blend = 1; break;
		case VIDEO_BLEND_ADD: _blend = 2; break;

		default:
			if (VIDEO_DEBUG)
				cerr << "VIDEO WARNING: " << __FILE__ << ":" << __FUNCTION__ << ": Unknown flag in argument list: " << flag << endl;
			break;
		}
		flag = va_arg(args, int32);
	}
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
		if  (!UnloadTextures())
		{
			cerr << "VIDEO: Failed to delete OpenGL textures during a context change" << endl;
		}

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

		// set up multisampling
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 2);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

		// set up vsync
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

		if (!SDL_SetVideoMode(_temp_width, _temp_height, 0, flags)) {
		// RGB values of 1 for each and 8 for depth seemed to be sufficient.
		// 565 and 16 here because it works with them on this computer.
		// NOTE from prophile: this ought to be changed to 5558
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);

			// kill multisampling
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

			// cancel vsync
			SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);

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

		// turn off writing to the depth buffer
		glDepthMask ( GL_FALSE );

		_width      = _temp_width;
		_height     = _temp_height;
		_fullscreen = _temp_fullscreen;

		ReloadTextures();

		EnableFog(_fog_color, _fog_intensity);

		return true;
	} // if (_target == VIDEO_TARGET_SDL_WINDOW)

	// Used by the Allacrost editor, which uses QT4
	else if (_target == VIDEO_TARGET_QT_WIDGET) {
		_width = _temp_width;
		_height = _temp_height;
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

void GameVideo::SetViewport(float left, float right, float bottom, float top) {
	assert(left < right);
	assert(bottom < top);

	int32 l = int32(left * _width * .01f);
	int32 b = int32(bottom * _height * .01f);
	int32 r = int32(right * _width * .01f);
	int32 t = int32(top * _height * .01f);

	if (l < 0)
		l = 0;
	if (b < 0)
		b = 0;
	if (r > _width)
		r = _width;
	if (t > _height)
		t = _height;

	_viewport = ScreenRect(l, b, r - l + 1, t - b + 1);
	glViewport(l, b, r - l + 1, t - b + 1);
}



void GameVideo::Clear() {
	if (_uses_lights)
		Clear(_light_color);
	else
		Clear(Color::black);
}



void GameVideo::Clear(const Color &c) {
	SetViewport(0,100,0,100);
	glClearColor(c[0], c[1], c[2], c[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	_num_tex_switches = 0;

	if (glGetError() && VIDEO_DEBUG) {
		cerr << "VIDEO WARNING:" << __FILE__ << ":" << __FUNCTION__ << ": glGetError returned true" << endl;
	}
}

//-----------------------------------------------------------------------------
// AccumulateLights: if real lights are enabled, then you must call DrawLight()
//                   for each light, and then call AccumulateLights() to
//                   save the lighting information into the overlay
//-----------------------------------------------------------------------------
/*bool GameVideo::AccumulateLights()
{
	if(_light_overlay != 0xFFFFFFFF)
	{
		_BindTexture(_light_overlay);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 1024, 1024, 0);
	}

	return true;
}*/



void GameVideo::Display(int32 frame_time) {
	// update particle effects
	_particle_manager.Update(frame_time);

	// update shaking effect
	_PushContext();
	SetCoordSys(0, 1024, 0, 768);
	_UpdateShake(frame_time);

	// update lightning timer
	_lightning_current_time += frame_time;

	if(_lightning_current_time > _lightning_end_time)
		_lightning_active = false;

	// show an overlay over the screen if we're fading
	if (_fader.ShouldUseFadeOverlay()) {
		Color c = _fader.GetFadeOverlayColor();
		StillImage fade_overlay;
		fade_overlay.SetDimensions(1024.0f, 768.0f);
		fade_overlay.SetColor(c);
		LoadImage(fade_overlay);
		SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_BOTTOM, 0);
		PushState();
		Move(0, 0);
		DrawImage(fade_overlay);
		PopState();
		DeleteImage(fade_overlay);
	}

	// this must be called before DrawFPS and all, because we only
	// want to count texture switches related to the game itself, not the
	// ones used to draw debug text and things like that.

	if(_advanced_display)
		_DEBUG_ShowAdvancedStats();

	if(_fps_display)
		DrawFPS(frame_time);

	if(!_DEBUG_ShowTexSheet())
	{
		if(VIDEO_DEBUG)
		{
			// keep track of whether we've already shown this error.
			// If we've shown it once, stop showing it so we don't clog up
			// the debug output with the same message 1000 times
			static bool has_failed = false;

			if(!has_failed)
			{
				cerr << "VIDEO ERROR: _DEBUG_ShowTexSheet() failed\n";
				has_failed = true;
			}
		}
	}

	_PopContext();

	SDL_GL_SwapBuffers();

	_fader.Update(frame_time);

	// update animation timers

	int32 old_frame_index = _animation_counter / VIDEO_ANIMATION_FRAME_PERIOD;
	_animation_counter += frame_time;
	int32 current_frame_index = _animation_counter / VIDEO_ANIMATION_FRAME_PERIOD;

	_current_frame_diff = current_frame_index - old_frame_index;
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
	sprintf(text, "Switches: %d\nParticles: %d", _num_tex_switches, _particle_manager.GetNumParticles());

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
	GLint matrix_mode;
	glGetIntegerv(GL_MATRIX_MODE, &matrix_mode);
	assert(matrix_mode == GL_MODELVIEW);
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
	GLint matrix_mode;
	glGetIntegerv(GL_MATRIX_MODE, &matrix_mode);
	assert(matrix_mode == GL_MODELVIEW);
#endif
	glTranslatef(tx, ty, 0);
	_x += tx;
	_y += ty;
}


//-----------------------------------------------------------------------------
// Rotate: rotates the coordinate axes anticlockwise by acAngle degrees, think
//         about this CARFULLY before you call it
//-----------------------------------------------------------------------------

void GameVideo::Rotate(float ac_angle)
{
#ifndef NDEBUG
	GLint matrix_mode;
	glGetIntegerv(GL_MATRIX_MODE, &matrix_mode);
	assert(matrix_mode == GL_MODELVIEW);
#endif
	glRotatef(ac_angle, 0, 0, 1);
}


//-----------------------------------------------------------------------------
// Scale: scales the coordinate axes by xScale and yScale respectively
//-----------------------------------------------------------------------------

void GameVideo::Scale(float x, float y)
{
#ifndef NDEBUG
	GLint matrix_mode;
	glGetIntegerv(GL_MATRIX_MODE, &matrix_mode);
	assert(matrix_mode == GL_MODELVIEW);
#endif

	glScalef(x, y, 1.0f);
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
// Menu Methods
//-----------------------------------------------------------------------------

bool GameVideo::LoadMenuSkin(std::string skin_name, std::string border_image, std::string background_image, bool make_default)
{
	return GUIManager->LoadMenuSkin(skin_name, border_image, background_image,
		Color::clear, Color::clear, Color::clear, Color::clear, make_default);
}



bool GameVideo::LoadMenuSkin(std::string skin_name, std::string border_image, Color background_color, bool make_default)
{
	return GUIManager->LoadMenuSkin(skin_name, border_image, "",
		background_color, background_color, background_color, background_color, make_default);
}



bool GameVideo::LoadMenuSkin(std::string skin_name, std::string border_image, Color top_left, Color top_right,
	Color bottom_left, Color bottom_right, bool make_default)
{
	return GUIManager->LoadMenuSkin(skin_name, border_image, "",
		top_left, top_right, bottom_left, bottom_right, make_default);
}



bool GameVideo::LoadMenuSkin(std::string skin_name, std::string border_image, std::string background_image,
	Color background_color, bool make_default)
{
	return GUIManager->LoadMenuSkin(skin_name, border_image, background_image,
		background_color, background_color, background_color, background_color, make_default);
}



bool GameVideo::LoadMenuSkin(std::string skin_name, std::string border_image, std::string background_image,
	Color top_left, Color top_right, Color bottom_left, Color bottom_right, bool make_default)
{
	return GUIManager->LoadMenuSkin(skin_name, border_image, background_image,
		top_left, top_right, bottom_left, bottom_right, make_default);
}

//-----------------------------------------------------------------------------
// _BindTexture: wraps call to glBindTexture(), plus some extra checking to
//              discard the call if we try to bind the same texture twice
//-----------------------------------------------------------------------------

bool GameVideo::_BindTexture(GLuint tex_id)
{
	if(tex_id != _last_tex_id)
	{
		_last_tex_id = tex_id;
		glBindTexture(GL_TEXTURE_2D, tex_id);
		++_num_tex_switches;
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
	_advanced_display = !_advanced_display;
	return true;
}



//-----------------------------------------------------------------------------
// EnableSceneLighting: sets lighting parameters for the scene, actually just a color
//              unless we change the lighting system later on.
//              NOTE: the color's alpha value (i.e. color[3]) must be 1.0f
//-----------------------------------------------------------------------------
bool GameVideo::EnableSceneLighting(const Color &color)
{
	_light_color = color;

	if(color[3] != 1.0f)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: color passed to EnableSceneLighting() had alpha other than 1.0f!" << endl;
		_light_color[3] = 1.0f;
	}

	return true;
}

//-----------------------------------------------------------------------------
// DisableSceneLighting: Turn off scene lighting
//-----------------------------------------------------------------------------
void GameVideo::DisableSceneLighting()
{
	_light_color = Color::white;
}

//-----------------------------------------------------------------------------
// GetLighting: returns the scene lighting color
//-----------------------------------------------------------------------------

Color &GameVideo::GetSceneLightingColor()
{
	return _light_color;
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
	_fog_color = color;
	_fog_intensity = intensity;

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
	_fog_intensity = 0.0f;
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
	_light_overlay = _CreateBlankGLTexture(1024, 1024);

	_uses_lights = true;

	return true;
}

//-----------------------------------------------------------------------------
// DisablePointLights: call when done with point lights
//-----------------------------------------------------------------------------
void GameVideo::DisablePointLights()
{
	if(_light_overlay != 0xFFFFFFFF)
	{
		_DeleteTexture(_light_overlay);
	}

	_light_overlay = 0xFFFFFFFF;

	_uses_lights = false;
}


//-----------------------------------------------------------------------------
// ApplyLightingOverlay: call after all map images are drawn to apply lighting.
//                       All menu and text rendering should occur AFTER this
//                       call, so that they are not affected by lighting.
//-----------------------------------------------------------------------------
bool GameVideo::ApplyLightingOverlay()
{
	if(_light_overlay != 0xFFFFFFFF)
	{
		// Copy light overlay to opengl texture
		_BindTexture(_light_overlay);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 1024, 1024, 0);

		CoordSys temp_CoordSys = _coord_sys;

		SetCoordSys(0.0f, 1.0f, 0.0f, 1.0f);
		float xlo = 0.0f, ylo = 0.0f, xhi = 1.0f, yhi = 1.0f;
		glEnable(GL_TEXTURE_2D);

		float mx = _width / 1024.0f;
		float my = _height / 1024.0f;

		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_COLOR, GL_ZERO);

		_BindTexture(_light_overlay);

		GLfloat vertices[8] = { xlo, ylo, xhi, ylo, xhi, yhi, xlo, yhi };
		GLfloat tex_coords[8] = { 0.0f, 0.0f, mx, 0.0f, mx, my, 0.0f, my };

		glEnableClientState ( GL_VERTEX_ARRAY );
		glEnableClientState ( GL_TEXTURE_COORD_ARRAY );

		glVertexPointer ( 2, GL_FLOAT, 0, vertices );
		glTexCoordPointer ( 2, GL_FLOAT, 0, tex_coords );

		glDrawArrays ( GL_QUADS, 0, 4 );

		glDisableClientState ( GL_TEXTURE_COORD_ARRAY );
		glDisableClientState ( GL_VERTEX_ARRAY );

		/*glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(xlo, ylo); //bl
		glTexCoord2f(mx, 0.0f);
		glVertex2f(xhi, ylo); //br
		glTexCoord2f(mx, my);
		glVertex2f(xhi, yhi);//tr
		glTexCoord2f(0.0f, my);
		glVertex2f(xlo, yhi);//tl
		glEnd();*/
		SetCoordSys(temp_CoordSys.GetLeft(), temp_CoordSys.GetRight(), temp_CoordSys.GetBottom(), temp_CoordSys.GetTop());
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
	GLint viewport_dimensions[4];
	glGetIntegerv(GL_VIEWPORT, viewport_dimensions);

	// Set up the screen rectangle to copy
	int32 width  = viewport_dimensions[2];
	int32 height = viewport_dimensions[3];
	ScreenRect    screen_rect(0, height, width, height);

	//TEMP TAGS
	// create an Image structure and store it our std::map of images
	if (id._filename == "")
		id._filename = "captured_screen";
	Image *new_image = new Image(id._filename, "<T>", width, height, false);

	// Try to create a texture sheet of screen size (rounded up)
	TexSheet *sheet = _CreateTexSheet(RoundUpPow2(width), RoundUpPow2(height), VIDEO_TEXSHEET_ANY, false);

	if(!sheet)
	{
		// This should never happen, unless we run out of memory or the
		// size of the screen texture is larger than the max available
		if (VIDEO_DEBUG)
			cerr << "VIDEO_DEBUG: GameVideo::_InsertImageInTexSheet() returned NULL!" << endl;

		return false;
	}
	if (!sheet->tex_mem_manager->Insert(new_image))
	{
		if (VIDEO_DEBUG)
			cerr << "VIDEO_DEBUG: TexMemMgr->Insert(image) returned NULL in " << __FUNCTION__ << endl;
		_RemoveSheet(sheet);
		return false;
	}

	if (!sheet->CopyScreenRect(0, 0, screen_rect))
	{
		if (VIDEO_DEBUG)
			cerr << "VIDEO_DEBUG: TexSheet->CopyScreenRect() failed in " << __FUNCTION__ << endl;
		_RemoveSheet(sheet);
		return false;
	}

	new_image->ref_count = 1;

	// if width or height are zero, that means to use the dimensions of image
	if(id._width == 0.0f)
		id._width  = (float)width;

	if(id._height == 0.0f)
		id._height = (float)height;

	// Store the new image element (flipped y)
	ImageElement element(new_image, 0, 0, 0.0f, 1.0f, 1.0f, 0.0f, id._width, id._height, id._color);
	id._elements.push_back(element);

	// Store the image in our std::map
	_images[id._filename] = new_image;

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
	_fps_display = !_fps_display;
}


//-----------------------------------------------------------------------------
// _CreateTempFilename
//-----------------------------------------------------------------------------

std::string GameVideo::_CreateTempFilename(const std::string &extension)
{
	// figure out the temp filename to return
	string file_name = "/tmp/allacrost";
	file_name += _next_temp_file;
	file_name += extension;

	// increment the 8-character temp name
	// Note: assume that the temp name is currently set to
	//       a valid name


	for(int32 digit = 7; digit >= 0; --digit)
	{
		++_next_temp_file[digit];

		if(_next_temp_file[digit] > 'z')
		{
			if(digit==0)
			{
				if(VIDEO_DEBUG)
					cerr << "VIDEO ERROR: _nextTempFile went past 'zzzzzzzz'" << endl;
				return file_name;
			}

			_next_temp_file[digit] = '0';
		}
		else
		{
			if(_next_temp_file[digit] > '9' && _next_temp_file[digit] < 'a')
				_next_temp_file[digit] = 'a';

			// if the digit did not overflow, then we don't need to carry over
			break;
		}
	}

	return file_name;
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
	c.x_align   = _x_align;
	c.y_align   = _y_align;
	c.x_flip    = _x_flip;
	c.y_flip    = _y_flip;

	c.viewport = _viewport;
	c.scissor_rectangle = _scissor_rect;
	c.scissoring_enabled = _scissor_enabled;

	c.font      = _current_font;
	c.text_color = _current_text_color;

	_context_stack.push(c);
}


//-----------------------------------------------------------------------------
// _PopContext: pops transformation, coordsys, and draw flags so that
//              GameVideo doesn't trample on client's settings
//-----------------------------------------------------------------------------

void GameVideo::_PopContext()
{
	// restore context information and pop it from stack
	private_video::Context c = _context_stack.top();
	SetCoordSys(c.coordinate_system);
	_blend  = c.blend;
	_x_align = c.x_align;
	_y_align = c.y_align;
	_x_flip  = c.x_flip;
	_y_flip  = c.y_flip;

	_current_font      = c.font;
	_current_text_color = c.text_color;

	_viewport = c.viewport;
	_scissor_rect = c.scissor_rectangle;
	_scissor_enabled = c.scissoring_enabled;
	_context_stack.pop();

	// restore modelview transformation
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glViewport(_viewport.left, _viewport.top, _viewport.width, _viewport.height);

	if(_scissor_enabled)
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor( static_cast<GLint>( (_scissor_rect.left / static_cast<float>(VIDEO_STANDARD_RES_WIDTH) ) * _viewport.width ),
		           static_cast<GLint>( ( _scissor_rect.top / static_cast<float>(VIDEO_STANDARD_RES_HEIGHT) ) * _viewport.height ),
		           static_cast<GLsizei>( ( _scissor_rect.width / static_cast<float>(VIDEO_STANDARD_RES_WIDTH) ) * _viewport.width ),
		           static_cast<GLsizei>( ( _scissor_rect.height / static_cast<float>(VIDEO_STANDARD_RES_HEIGHT) ) * _viewport.height ) );
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
	}
}


//-----------------------------------------------------------------------------
// _ConvertYAlign: convert a value like VIDEO_Y_BOTTOM to an offset like -1
//-----------------------------------------------------------------------------

int32 GameVideo::_ConvertYAlign(int32 y_align)
{
	switch(y_align)
	{
		case VIDEO_Y_BOTTOM:
			return -1;
		case VIDEO_Y_CENTER:
			return 0;
		default:
			return 1;
	}
}


//-----------------------------------------------------------------------------
// _ConvertXAlign: convert a value like VIDEO_X_LEFT to an offset like -1
//-----------------------------------------------------------------------------

int32 GameVideo::_ConvertXAlign(int32 x_align)
{
	switch(x_align)
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
bool GameVideo::SetDefaultCursor(const std::string &cursor_image_filename)
{
	_default_menu_cursor.SetFilename(cursor_image_filename);
	return LoadImage(_default_menu_cursor);
}


//-----------------------------------------------------------------------------
// GetDefaultCursor: sets the gefault menu cursor, returns NULL if none is set
//-----------------------------------------------------------------------------
StillImage *GameVideo::GetDefaultCursor()
{
	if(_default_menu_cursor.GetWidth() != 0.0f)  // cheap test if image is valid
		return &_default_menu_cursor;
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
// EnableScissoring: enable/disable scissoring
//-----------------------------------------------------------------------------
void GameVideo::EnableScissoring(bool enable)
{
	_scissor_enabled = enable;

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
	_scissor_rect = CalculateScreenRect(left, right, bottom, top);
	glScissor( static_cast<GLint>( (_scissor_rect.left / static_cast<float>(VIDEO_STANDARD_RES_WIDTH) ) * _viewport.width ),
	           static_cast<GLint>( ( _scissor_rect.top / static_cast<float>(VIDEO_STANDARD_RES_HEIGHT) ) * _viewport.height ),
	           static_cast<GLsizei>( ( _scissor_rect.width / static_cast<float>(VIDEO_STANDARD_RES_WIDTH) ) * _viewport.width ),
	           static_cast<GLsizei>( ( _scissor_rect.height / static_cast<float>(VIDEO_STANDARD_RES_HEIGHT) ) * _viewport.height ) );
}


//-----------------------------------------------------------------------------
// SetScissorRect: set the scissoring rectangle
//-----------------------------------------------------------------------------
void GameVideo::SetScissorRect(const ScreenRect &rect)
{
	_scissor_rect = rect;
	glScissor( static_cast<GLint>( (_scissor_rect.left / static_cast<float>(VIDEO_STANDARD_RES_WIDTH) ) * _viewport.width ),
		   static_cast<GLint>( ( _scissor_rect.top / static_cast<float>(VIDEO_STANDARD_RES_HEIGHT) ) * _viewport.height ),
	           static_cast<GLsizei>( ( _scissor_rect.width / static_cast<float>(VIDEO_STANDARD_RES_WIDTH) ) * _viewport.width ),
	           static_cast<GLsizei>( ( _scissor_rect.height / static_cast<float>(VIDEO_STANDARD_RES_HEIGHT) ) * _viewport.height ) );
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
// _ShouldSmooth: call to check if textures should be smoothed to display
//                nicely on non-natural (non 1024x768) resolutions
//-----------------------------------------------------------------------------
bool GameVideo::_ShouldSmooth() {
	return ( _width  != VIDEO_STANDARD_RES_WIDTH ||
		 _height != VIDEO_STANDARD_RES_HEIGHT);
}


//-----------------------------------------------------------------------------
// MakeLightning: creates a lightning effect
//-----------------------------------------------------------------------------

bool GameVideo::MakeLightning(const std::string &lit_file)
{
	FILE *fp = fopen(lit_file.c_str(), "rb");
	if(!fp)
		return false;

	int32 data_size;
	if(!fread(&data_size, 4, 1, fp))
	{
		fclose(fp);
		return false;
	}

	// since this file was created on windows, it uses little endian byte order
	// Check if this processor uses big endian, and reorder bytes if so.

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		data_size = ((data_size & 0xFF000000) >> 24) |
		           ((data_size & 0x00FF0000) >> 8) |
		           ((data_size & 0x0000FF00) << 8) |
		           ((data_size & 0x000000FF) << 24);
	#endif

	uint8 *data = new uint8[data_size];

	if(!fread(data, data_size, 1, fp))
	{
		delete [] data;
		fclose(fp);
		return false;
	}

	fclose(fp);

	_lightning_data.clear();

	for(int32 j = 0; j < data_size; ++j)
	{
		float f = float(data[j]) / 255.0f;
		_lightning_data.push_back(f);
	}

	delete [] data;

	_lightning_active = true;
	_lightning_current_time = 0;
	_lightning_end_time = data_size * 1000 / 100;

	return true;
}


//-----------------------------------------------------------------------------
// DrawLightning: draws lightning effect on the screen using a fullscreen overlay
//-----------------------------------------------------------------------------

bool GameVideo::DrawLightning()
{
	if(!_lightning_active)
		return true;

	// convert milliseconds elapsed into data points elapsed

	float t = _lightning_current_time * 100.0f / 1000.0f;

	int32 rounded_t = static_cast<int32>(t);
	t -= rounded_t;

	// get 2 separate data points and blend together (linear interpolation)

	float data1 = _lightning_data[rounded_t];
	float data2 = _lightning_data[rounded_t+1];

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

void GameVideo::DrawGrid(float x, float y, float x_step, float y_step, const Color &c)
{
	PushState();

	Move(0, 0);

	float x_Max = _coord_sys.GetRight();
	float y_Max = _coord_sys.GetBottom();

	std::vector<GLfloat> vertices;
	int numvertices = 0;
	for (; x <= x_Max; x += x_step)
	{
		vertices.push_back(x);
		vertices.push_back(_coord_sys.GetBottom());
		vertices.push_back(x);
		vertices.push_back(_coord_sys.GetTop());
		numvertices += 2;
	}
	for (; y < y_Max; y += y_step)
	{
		vertices.push_back(_coord_sys.GetLeft());
		vertices.push_back(y);
		vertices.push_back(_coord_sys.GetRight());
		vertices.push_back(y);
		numvertices += 2;
	}
	glColor4fv(&c[0]);
	glEnableClientState ( GL_VERTEX_ARRAY );
	glVertexPointer ( 2, GL_FLOAT, 0, &(vertices[0]) );
	glDrawArrays ( GL_LINES, 0, numvertices );
	glDisableClientState ( GL_VERTEX_ARRAY );
	/*glBegin(GL_LINES);
	glColor4fv(&c[0]);

	for(; x <= x_Max; x += x_step)
	{
		glVertex2f(x, _coord_sys.GetBottom());
		glVertex2f(x, _coord_sys.GetTop());
	}

	for(; y <= y_Max; y += y_step)
	{
		glVertex2f(_coord_sys.GetLeft(), y);
		glVertex2f(_coord_sys.GetRight(), y);
	}

	glEnd();*/
	PopState();
}

//-----------------------------------------------------------------------------
// Draws a solid rectangle of a given color.
//-----------------------------------------------------------------------------
void GameVideo::DrawRectangle(const float width, const float height, const Color &color)
{
	_rectangle_image._elements[0].width = width;
	_rectangle_image._elements[0].height = height;

	VideoManager->DrawImage(_rectangle_image, color);
}

}  // namespace hoa_video
