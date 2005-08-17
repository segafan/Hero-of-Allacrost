#include "utils.h"
#include <cassert>
#include <cstdarg>
#include "video.h"
#include <math.h>
#include "coord_sys.h"
#include "gui.h"

using namespace std;
using namespace hoa_video::local_video;

namespace hoa_video 
{

bool VIDEO_DEBUG = false;

SINGLETON_INITIALIZE(GameVideo);


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
// GameVideo
//-----------------------------------------------------------------------------

GameVideo::GameVideo() 
: _width(0), 
  _height(0),
  _fullscreen(false),
  _temp_width(0),
  _temp_height(0),
  _temp_fullscreen(false),
  _blend(0), 
  _xalign(-1), 
  _yalign(-1), 
  _xflip(0), 
  _yflip(0),
  _currentDebugTexSheet(-1),
  _batchLoading(false),
  _usesLights(false),
  _lightOverlay(0xFFFFFFFF),
  _gui(NULL),
  _lastTexID(0xFFFFFFFF),
  _numTexSwitches(0),
  _advancedDisplay(false),
  _shakeX(0),
  _shakeY(0),
  _fogColor(1.0f, 1.0f, 1.0f, 1.0f),
  _fogIntensity(0.0f),
  _lightColor(1.0f, 1.0f, 1.0f, 1.0f)
{
	if (VIDEO_DEBUG) 
		cout << "VIDEO: GameVideo constructor invoked\n";
}


//-----------------------------------------------------------------------------
// Initialize: called to actually initialize the video engine
//-----------------------------------------------------------------------------

bool GameVideo::Initialize()
{
	if(VIDEO_DEBUG)
		cout << "VIDEO: Initializing SDL subsystem\n";
		
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) 
	{
		fprintf(stderr, "Barf! SDL Video Initialization failed!\n");
		exit(1);
	}

	if(VIDEO_DEBUG)
		cout << "VIDEO: setting video mode\n";

	SetResolution(1024, 768);
	SetFullscreen(false);
	
	if(!ApplySettings())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ChangeMode() failed in GameVideo::Initialize()!" << endl;
		return false;
	}

	if(VIDEO_DEBUG)
		cout << "VIDEO: Initializing IL\n";


	// initialize DevIL
	ilInit();
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	
	if(!ilEnable(IL_ORIGIN_SET))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: SERIOUS PROBLEM! ilEnable(IL_ORIGIN_SET) failed in GameVideo::Initialize()!" << endl;
		return false;
	}

	if(VIDEO_DEBUG)
		cout << "VIDEO: Initializing ILU\n";
	
	iluInit();     // assume this function works since iluInit() doesn't return error codes! :(
	
	if(VIDEO_DEBUG)
		cout << "VIDEO: Initializing ILUT\n";

	if(!ilutRenderer(ILUT_OPENGL))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: SERIOUS PROBLEM! ilutRenderer(ILUT_OPENGL) failed in GameVideo::Initialize()!" << endl;
		// don't return false, since it's possible to play game w/o ilutRenderer
	}

	if(VIDEO_DEBUG)
		cout << "VIDEO: Initializing SDL_ttf\n";

	// initialize SDL_ttf
	if(TTF_Init() < 0)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: SDL_ttf did not initialize! (TTF_Init() failed)" << endl;
		return false;
	}

	if(VIDEO_DEBUG)
		cout << "VIDEO: Loading default font\n";
	

	if(!LoadFont("img/fonts/cour.ttf", "default", 18))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: Could not load cour.ttf file!" << endl;
		return false;
	}

	if(VIDEO_DEBUG)
		cout << "VIDEO: Creating texture sheets\n";

	
	// create our default texture sheets

	if(!CreateTexSheet(512, 512, VIDEO_TEXSHEET_32x32, false))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: could not create default 32x32 tex sheet!" << endl;
		return false;
	}
	
	if(!CreateTexSheet(512, 512, VIDEO_TEXSHEET_32x64, false))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: could not create default 32x64 tex sheet!" << endl;
		return false;
	}

	if(!CreateTexSheet(512, 512, VIDEO_TEXSHEET_64x64, false))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: could not create default 64x64 tex sheet!" << endl;
		return false;
	}

	if(!CreateTexSheet(512, 512, VIDEO_TEXSHEET_ANY,   true))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: could not create default static  var-sized tex sheet!" << endl;
		return false;
	}

	if(!CreateTexSheet(512, 512, VIDEO_TEXSHEET_ANY,   false))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: could not create default var-sized tex sheet!" << endl;
		return false;
	}

	if(VIDEO_DEBUG)
		cout << "VIDEO: Erasing the screen\n";

	_gui = new GUI;

	if(!Clear())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: first call to Clear() in GameVideo::Initialize() failed!" << endl;
		return false;
	}
	
	if(!Display(0))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: Display() in GameVideo::Initialize() failed!" << endl;
		return false;
	}	

	if(!Clear())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: second call to Clear() in GameVideo::Initialize() failed!" << endl;
		return false;
	}

	if(VIDEO_DEBUG)
		cout << "VIDEO: GameVideo::Initialize() returned successfully" << endl;
	
	return true;
}


//-----------------------------------------------------------------------------
// MakeScreenshot: create a screenshot and save as screenshot.jpg
//-----------------------------------------------------------------------------

bool GameVideo::MakeScreenshot()
{
	if(VIDEO_DEBUG)
		cout << "VIDEO: Entering MakeScreenshot()" << endl;

	ILuint screenshot;
	ilGenImages(1, &screenshot);
	
	if(ilGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ilGenImages() failed in MakeScreenshot()!" << endl;
		return false;
	}
	
	ilBindImage(screenshot);
	if(ilGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ilBindImage() failed in MakeScreenshot()!" << endl;
		return false;
		ilDeleteImages(1, &screenshot);
	}
	
	if(!ilEnable(IL_FILE_OVERWRITE))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ilEnable() failed in MakeScreenshot()!" << endl;
		return false;		
		ilDeleteImages(1, &screenshot);
	}
	
	if(!ilutGLScreen())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ilutGLScreen() failed in MakeScreenshot()!" << endl;
		ilDeleteImages(1, &screenshot);
		return false;
	}

	if(!ilSaveImage("screenshot.jpg"))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ilSaveImage() failed in MakeScreenshot()!" << endl;
		ilDeleteImages(1, &screenshot);
		return false;
	}

	ilDeleteImages(1, &screenshot);
	if(ilGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ilDeleteImages() failed in MakeScreenshot()!" << endl;
		return false;
	}

	if(VIDEO_DEBUG)
		cout << "VIDEO: Exiting MakeScreenshot() successfully (JPG file saved)" << endl;

	return true;
}


//-----------------------------------------------------------------------------
// ~GameVideo
//-----------------------------------------------------------------------------

GameVideo::~GameVideo() 
{ 
	if (VIDEO_DEBUG) 
		cout << "VIDEO: GameVideo destructor invoked" << endl;
	
	// delete GUI
	delete _gui;
		
	// delete TTF fonts
	map<string, TTF_Font *>::iterator iFont    = _fontMap.begin();
	map<string, TTF_Font *>::iterator iFontEnd = _fontMap.end();
	
	while(iFont != _fontMap.end())
	{
		TTF_Font *font = iFont->second;
		
		if(font)
		{
			TTF_CloseFont(font);
		}
		
		++iFont;
	}
	
	// uninitialize SDL_ttf
	TTF_Quit();
	
	// uninitiailize DevIL
	ilShutDown();
	
	// delete texture sheets
	vector<TexSheet *>::iterator iSheet      = _texSheets.begin();
	vector<TexSheet *>::iterator iSheetEnd   = _texSheets.end();

	while(iSheet != iSheetEnd)
	{
		delete *iSheet;
		++iSheet;
	}
	
	// delete images
	map<FileName, Image *>::iterator iImage     = _images.begin();
	map<FileName, Image *>::iterator iImageEnd  = _images.end();

	while(iImage != iImageEnd)
	{
		delete iImage->second;
		++iImage;
	}
	
	
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
	_coordSys = CoordSys(left, right, bottom, top);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glOrtho(_coordSys._left, _coordSys._right, _coordSys._bottom, _coordSys._top, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


//-----------------------------------------------------------------------------
// SetDrawFlags: used for controlling various flags like blending, flipping, etc.
//-----------------------------------------------------------------------------

void GameVideo::SetDrawFlags(int firstflag, ...)
{
	int n;
	int flag;
	va_list args;

	va_start(args, firstflag);
	for (n=0;;n++) 
	{
		flag = (n==0) ? firstflag : va_arg(args, int);
		switch (flag) {
		case 0: goto done;

		case VIDEO_X_LEFT: _xalign=-1; break;
		case VIDEO_X_CENTER: _xalign=0; break;
		case VIDEO_X_RIGHT: _xalign=1; break;

		case VIDEO_Y_TOP: _yalign=-1; break;
		case VIDEO_Y_CENTER: _yalign=0; break;
		case VIDEO_Y_BOTTOM: _yalign=1; break;

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
	// Losing GL context, so unload images first
	UnloadTextures();

	int flags = SDL_OPENGL;
	
	if(_temp_fullscreen)
		flags |= SDL_FULLSCREEN;

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	if (!SDL_SetVideoMode(_temp_width, _temp_height, 0, flags)) 
	{	
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: SDL_SetVideoMode() failed in ApplySettings()!" << endl;

		_temp_fullscreen = _fullscreen;
		_temp_width      = _width;
		_temp_height     = _height;

		if(_width > 0)   // quick test to see if we already had a valid video mode
		{
			ReloadTextures();					
		}
		return false;		
	} 

	_width      = _temp_width;
	_height     = _temp_height;
	_fullscreen = _temp_fullscreen;

	ReloadTextures();
	
	SetFog(_fogColor, _fogIntensity);
	
	return true;
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

	int l=int(left*_width*.01f);
	int b=int(bottom*_height*.01f);
	int r=int(right*_width*.01f);
	int t=int(top*_height*.01f);

	if (l<0) l=0;
	if (b<0) b=0;
	if (r>_width) r=_width;
	if (t>_height) t=_height;

	glViewport(l, b, r-l+1, t-b+1);
}


//-----------------------------------------------------------------------------
// Clear: clear the screen to black, it doesnt clear other buffers, that can be 
//        done by videostates that use them
//-----------------------------------------------------------------------------

bool GameVideo::Clear() 
{
	SetViewport(0,100,0,100);
	
	if(_usesLights)
	{
		glClearColor
		(
			_lightColor.color[0], 
			_lightColor.color[1], 
			_lightColor.color[2], 
			_lightColor.color[3]
		);
	}
	else
		glClearColor(0,0,0,1);	
	
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

bool GameVideo::AccumulateLights()
{
	if(_lightOverlay != 0xFFFFFFFF)
	{
		BindTexture(_lightOverlay);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 1024, 1024, 0);
	}
	
	return true;
}


//-----------------------------------------------------------------------------
// Display: if running in double buffered mode then flip the other buffer to the 
//          screen
//-----------------------------------------------------------------------------

bool GameVideo::Display(int frameTime) 
{
	// update shaking effect
	CoordSys oldSys = _coordSys;
	SetCoordSys(0, 1024, 0, 768);

	UpdateShake(frameTime);	
	
	// show an overlay over the screen if we're fading
	if(_fader.ShouldUseFadeOverlay())
	{
		Color c = _fader.GetFadeOverlayColor();
		ImageDescriptor fadeOverlay;
		fadeOverlay.width  = 1024.0f;
		fadeOverlay.height = 768.0f;
		fadeOverlay.color  = c;
		LoadImage(fadeOverlay);		
		SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, 0);
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
		DEBUG_ShowTexSwitches();

	DrawFPS(frameTime);
		
	if(!DEBUG_ShowTexSheet())
	{
		if(VIDEO_DEBUG)
		{
			// keep track of whether we've already shown this error.
			// If we've shown it once, stop showing it so we don't clog up
			// the debug output with the same message 1000 times
			static bool hasFailed = false;
			
			if(!hasFailed)
			{
				cerr << "VIDEO ERROR: DEBUG_ShowTexSheet() failed\n";
				hasFailed = true;
			}
		}
	}

	SetCoordSys(oldSys._left, oldSys._right, oldSys._bottom, oldSys._top);

	SDL_GL_SwapBuffers();
	
	_fader.Update(frameTime);	
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

bool GameVideo::SetResolution(int width, int height)
{
	if(width <= 0 || height <= 0)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: invalid width and/or height passed to SetResolution!" << endl;
		return false;
	}
	
	_temp_width  = width;
	_temp_height = height;
	return true;
}


//-----------------------------------------------------------------------------
// DEBUG_ShowTexSwitches: display how many times we switched textures during
//                        the current frame
//-----------------------------------------------------------------------------

bool GameVideo::DEBUG_ShowTexSwitches()
{
	// display to screen	
	char text[16];
	sprintf(text, "Switches: %d", _numTexSwitches);
	
	if( !SetFont("default"))
		return false;
		
	if( !DrawText(text, 876.0f, 690.0f))
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
}

//-----------------------------------------------------------------------------
// MoveRel: 
//-----------------------------------------------------------------------------

void GameVideo::MoveRel(float tx, float ty) 
{
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif
	glTranslatef(tx, ty, 0);
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
// PushState: saves your current position in a stack, bewarned this stack is 
//            small ~32 so use it wisely
//-----------------------------------------------------------------------------

void GameVideo::PushState() {
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif
	glPushMatrix();
}


//-----------------------------------------------------------------------------
// PopState: restores last position, read PushState()
//-----------------------------------------------------------------------------

void GameVideo::PopState() 
{
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif
	glPopMatrix();
}



//-----------------------------------------------------------------------------
// SetMenuSkin
//-----------------------------------------------------------------------------

bool GameVideo::SetMenuSkin
(
	const std::string &imgFile_UL,
	const std::string &imgFile_U,
	const std::string &imgFile_UR,
	const std::string &imgFile_L,
	const std::string &imgFile_R,
	const std::string &imgFile_BL,
	const std::string &imgFile_B,
	const std::string &imgFile_BR,	
	const Color  &fillColor
)
{
	return _gui->SetMenuSkin
	(
		imgFile_UL,
		imgFile_U,
		imgFile_UR,
		imgFile_L,
		imgFile_R,
		imgFile_BL,
		imgFile_B,
		imgFile_BR,
		fillColor
	);
}


//-----------------------------------------------------------------------------
// BindTexture: wraps call to glBindTexture(), plus some extra checking to
//              discard the call if we try to bind the same texture twice
//-----------------------------------------------------------------------------

bool GameVideo::BindTexture(GLuint texID)
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
// CreateMenu
//-----------------------------------------------------------------------------

bool GameVideo::CreateMenu(ImageDescriptor &id, float width, float height)
{
	return _gui->CreateMenu(id, width, height);
}


//-----------------------------------------------------------------------------
// SetLighting: sets lighting parameters for the scene, actually just a color
//              unless we change the lighting system later on.
//              NOTE: the color's alpha value (i.e. color[3]) must be 1.0f
//-----------------------------------------------------------------------------

bool GameVideo::SetLighting(const Color &color)
{
	_lightColor = color;

	if(color.color[3] != 1.0f)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: color passed to SetLighting() had alpha other than 1.0f!" << endl;
		_lightColor.color[3] = 1.0f;		
	}

	return true;
}


//-----------------------------------------------------------------------------
// SetFog: sets fog parameters. Fog color is usually gray, and intensity can
//         be from 0.0 (no fog) to 1.0 (entire screen is gray)
//         To turn off fog, just call this function with intensity of 0.0f
//-----------------------------------------------------------------------------

bool GameVideo::SetFog(const Color &color, float intensity)
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
		glFogfv(GL_FOG_COLOR, (GLfloat *)color.color);
	}
	
	return true;	
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
// EnableRealLights: call with true if this map uses real lights
//-----------------------------------------------------------------------------

bool GameVideo::EnableRealLights(bool enable)
{
	if(enable)
	{
		_lightOverlay = CreateBlankGLTexture(1024, 1024);
	}
	else
	{
		if(_lightOverlay != 0xFFFFFFFF)
		{
			DeleteTexture(_lightOverlay);
		}
		
		_lightOverlay = 0xFFFFFFFF;
	}

	_usesLights = enable;
	
	return true;	
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
		CoordSys tempCoordSys = _coordSys;

		SetCoordSys(0.0f, 1.0f, 0.0f, 1.0f);
		float xlo = 0.0f, ylo = 0.0f, xhi = 1.0f, yhi = 1.0f;
		glEnable(GL_TEXTURE_2D);
		
		float mx = _width / 1024.0f;
		float my = _height / 1024.0f;

		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		
		BindTexture(_lightOverlay);
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
		SetCoordSys(tempCoordSys._left, tempCoordSys._right, tempCoordSys._bottom, tempCoordSys._top);
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

bool GameVideo::CaptureScreen(ImageDescriptor &id)
{
	if(VIDEO_DEBUG)
		cout << "VIDEO: Entering CaptureScreen()" << endl;

	if(_images.find("!!screenie!!") != _images.end())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: Tried to CaptureScreen() even though captured image was already in memory!" << endl;
		return false;
	}

	ILuint screenshot;
	ilGenImages(1, &screenshot);
	
	if(ilGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ilGenImages() failed in CaptureScreen()!" << endl;
		return false;
	}
	
	ilBindImage(screenshot);
	if(ilGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ilBindImage() failed in CaptureScreen()!" << endl;
		ilDeleteImages(1, &screenshot);
		return false;
	}

	if(!ilutGLScreen())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ilutGLScreen() failed in CaptureScreen()!" << endl;
		ilDeleteImages(1, &screenshot);			
		return false;
	}
	
	// flip the image because our game uses coordinate system where
	// bigger y is higher on the screen
	iluFlipImage();	

	id._elements.clear();
	
	int w, h;
	
	w = ilGetInteger(IL_IMAGE_WIDTH);
	h = ilGetInteger(IL_IMAGE_HEIGHT);

	// create an Image structure and store it our std::map of images
	Image *newImage = new Image("!!screenie!!", w, h);

	// try to insert the image in a texture sheet
	int x, y;
	TexSheet *sheet = InsertImageInTexSheet(newImage, screenshot, x, y, w, h, false);

	if(!sheet)
	{
		// this should never happen, unless we run out of memory or there
		// is a bug in the InsertImageInTexSheet() function
		
		if(VIDEO_DEBUG)
			cerr << "VIDEO_DEBUG: GameVideo::InsertImageInTexSheet() returned NULL!" << endl;
		
		ilDeleteImages(1, &screenshot);
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
	ilDeleteImages(1, &screenshot);
	
	if(ilGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ilGetError() true after ilDeleteImages() in CaptureScreen()!" << endl;
		return false;
	}

	if(VIDEO_DEBUG)
		cout << "VIDEO: Exited CaptureScreen() successfully!" << endl;
	return true;
}



}  // namespace hoa_video
