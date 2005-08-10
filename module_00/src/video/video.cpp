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

// time between screen shake updates in milliseconds
const int VIDEO_TIME_BETWEEN_SHAKE_UPDATES = 50;  

// controls how slow the slow transform is. The greater the number, the "slower" it is.
const float VIDEO_SLOW_TRANSFORM_POWER = 2.0f;

// controls how fast the fast transform is. The smaller the number, the "faster" it is.
const float VIDEO_FAST_TRANSFORM_POWER = 0.3f;



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
  _batching(false),
  _gui(NULL),
  _lastTexID(0xFFFFFFFF),
  _numTexSwitches(0),
  _advancedDisplay(false),
  _shakeX(0),
  _shakeY(0)
{
	if (VIDEO_DEBUG) 
		cout << "VIDEO: GameVideo constructor invoked\n";
}

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
// FadeTo: Begins a fade to the given color in numSeconds
//         returns true if invalid parameter is passed
//-----------------------------------------------------------------------------

bool ScreenFader::FadeTo(const Color &final, float numSeconds)
{
	if(numSeconds < 0.0f)
		return false;
	
	_initialColor = _currentColor;
	_finalColor   = final;
	
	_currentTime = 0;
	_endTime     = int(numSeconds * 1000);  // convert seconds to milliseconds here
	
	_isFading = true;
	
	// figure out if this is a simple fade or if an overlay is required
	// A simple fade is defined as either a fade from (x,x,x,x)->(0,0,0,1) or from
	// (0,0,0,1)->(x,x,x,x). In other words, fading into or out of black.

	_useFadeOverlay = true;	

	Color black(0.0f, 0.0f, 0.0f, 1.0f);

	if( (_initialColor.color[0] == _initialColor.color[1] && 
	     _initialColor.color[0] == _initialColor.color[2] &&
	     _initialColor.color[0] == _initialColor.color[3] &&
	     _finalColor == black ) ||
	     
	    (_finalColor.color[0] == _finalColor.color[1] && 
	     _finalColor.color[0] == _finalColor.color[2] &&
	     _finalColor.color[0] == _finalColor.color[3] &&
	     _initialColor == black))	
	{
		_useFadeOverlay = false;
	}
	else
	{
		_fadeModulation = 1.0f;
	}
	
	Update(0);  // do initial update
	return true;
}



//-----------------------------------------------------------------------------
// Update: updates screen fader- figures out new interpolated fade color,
//         whether to fade using overlays or modulation, etc.
//-----------------------------------------------------------------------------

bool ScreenFader::Update(int t)
{
	if(!_isFading)
		return true;
				
	if(_currentTime >= _endTime)
	{
		_currentColor   = _finalColor;
		_isFading       = false;
		
		if(_useFadeOverlay)
		{
			// check if we have faded to black or clear. If so, we can use modulation
			if(_finalColor[3] == 0.0f ||
			  (_finalColor[0] == 0.0f &&
			   _finalColor[1] == 0.0f &&
			   _finalColor[2] == 0.0f))
			{
				_useFadeOverlay = false;
				_fadeModulation = 1.0f - _finalColor[3];
			}
		}
		else
			_fadeModulation = 1.0f - _finalColor[3];
	}
	else
	{
		// calculate the new interpolated color
		float a = (float)_currentTime / (float)_endTime;

		_currentColor.color[3] = Lerp(a, _initialColor.color[3], _finalColor.color[3]);

		
		// if we are fading to or from clear, then only the alpha should get
		// interpolated.
		if(_finalColor.color[3] == 0.0f)
		{
			_currentColor.color[0] = _initialColor.color[0];
			_currentColor.color[1] = _initialColor.color[1];
			_currentColor.color[2] = _initialColor.color[2];
		}
		if(_initialColor.color[3] == 0.0f)
		{
			_currentColor.color[0] = _finalColor.color[0];
			_currentColor.color[1] = _finalColor.color[1];
			_currentColor.color[2] = _finalColor.color[2];
		}
		else
		{
			_currentColor.color[0] = Lerp(a, _initialColor.color[0], _finalColor.color[0]);
			_currentColor.color[1] = Lerp(a, _initialColor.color[1], _finalColor.color[1]);
			_currentColor.color[2] = Lerp(a, _initialColor.color[2], _finalColor.color[2]);
		}
		
		if(!_useFadeOverlay)
			_fadeModulation = 1.0f - _currentColor.color[3];
		else
			_fadeOverlayColor = _currentColor;
	}

	_currentTime += t;
	return true;
}


//-----------------------------------------------------------------------------
// FadeScreen: sets up a fade to the given color over "fadeTime" number of seconds
//-----------------------------------------------------------------------------

bool GameVideo::FadeScreen(const Color &color, float fadeTime)
{
	return _fader.FadeTo(color, fadeTime);
}


//-----------------------------------------------------------------------------
// IsFading: returns true if screen is in the middle of a fade
//-----------------------------------------------------------------------------

bool GameVideo::IsFading()
{
	return _fader.IsFading();
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
	}
	
	if(!ilEnable(IL_FILE_OVERWRITE))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ilEnable() failed in MakeScreenshot()!" << endl;
		return false;		
	}
	
	if(!ilutGLScreen())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ilutGLScreen() failed in MakeScreenshot()!" << endl;
		return false;
	}

	if(!ilSaveImage("screenshot.jpg"))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ilSaveImage() failed in MakeScreenshot()!" << endl;
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
// SetFont: sets the current font. The name parameter is the name that was
//          passed to LoadFont() when it was loaded
//-----------------------------------------------------------------------------

bool GameVideo::SetFont(const std::string &name)
{
	// check if font is loaded before setting it
	if( _fontMap.find(name) == _fontMap.end())
		return false;
		
	_currentFont = name;
	return true;
}


//-----------------------------------------------------------------------------
// SetTextColor: sets the color to use when rendering text
//-----------------------------------------------------------------------------

bool GameVideo::SetTextColor (const Color &color)
{
	_currentTextColor = color;
	return true;
}


//-----------------------------------------------------------------------------
// GetFont: returns the name of the current font (e.g. "verdana18")
//-----------------------------------------------------------------------------

std::string GameVideo::GetFont() const
{
	return _currentFont;
}


//-----------------------------------------------------------------------------
// GetTextColor: returns the current text color
//-----------------------------------------------------------------------------

Color GameVideo::GetTextColor () const
{
	return _currentTextColor;
}


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
// DrawTextHelper: since there are two DrawText functions (one for unicode and
//                 one for non-unicode), this private function is used to
//                 do all the work so that code doesn't have to be duplicated.
//                 Either text or uText is valid string and the other is NULL.
//-----------------------------------------------------------------------------

bool GameVideo::DrawTextHelper
( 
	const char   *text, 
	const Uint16 *uText, 
	float x, 
	float y 
)
{
	if(_fontMap.empty())
		return false;
		
	CoordSys tempCoordSys = _coordSys;
	
	SetCoordSys(0,1024,0,768);
	SDL_Rect location;
	SDL_Color color;
	location.x = (int)x;
	location.y = (int)y;
	
	if(_fontMap.find(_currentFont) == _fontMap.end())
		return false;
	
	TTF_Font *font = _fontMap[_currentFont];
	
	color.r = 255;
	color.g = 255;
	color.b = 255;
	
	
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);

	SDL_Surface *initial;
	SDL_Surface *intermediary;
	int w,h;
	GLuint texture;
	
	
	if( uText )
	{
		initial = TTF_RenderUNICODE_Blended(font, uText, color);
		
		if(!initial)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: TTF_RenderUNICODE_Blended() returned NULL in DrawTextHelper()!" << endl;
			return false;
		}
	}
	else
	{
		initial = TTF_RenderText_Blended(font, text, color);

		if(!initial)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: TTF_RenderText_Blended() returned NULL in DrawTextHelper()!" << endl;
			return false;
		}
	}
		
	w = RoundUpPow2(initial->w);
	h = RoundUpPow2(initial->h);
	
	intermediary = SDL_CreateRGBSurface(0, w, h, 32, 
			0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

	if(!intermediary)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: SDL_CreateRGBSurface() returned NULL in DrawTextHelper()!" << endl;
		return false;
	}


	if(SDL_BlitSurface(initial, 0, intermediary, 0) < 0)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: SDL_BlitSurface() failed in DrawTextHelper()!" << endl;
		return false;
	}


	glGenTextures(1, &texture);
	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() true after glGenTextures() in DrawTextHelper!" << endl;
		return false;
	}
	
	BindTexture(texture);
	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() true after glBindTexture() in DrawTextHelper!" << endl;
		return false;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, 
	             GL_UNSIGNED_BYTE, intermediary->pixels );

	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() true after glTexImage2D() in DrawTextHelper!" << endl;
		return false;
	}

	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	

	glEnable(GL_TEXTURE_2D);
	BindTexture(texture);
	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() true after 2nd call to glBindTexture() in DrawTextHelper!" << endl;
		return false;
	}

	glColor3f(1.0f, 1.0f, 1.0f);
	
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 1.0f); 
	glVertex2f((float)location.x, (float)location.y);
	glTexCoord2f(1.0f, 1.0f); 
	glVertex2f((float)location.x + w, (float)location.y);
	glTexCoord2f(1.0f, 0.0f); 
	glVertex2f((float)location.x + w, (float)location.y + h);
	glTexCoord2f(0.0f, 0.0f); 
	glVertex2f((float)location.x, (float)location.y + h);

	glEnd();
	
	glFinish();
	
	SDL_FreeSurface(initial);
	SDL_FreeSurface(intermediary);
		
	if(!DeleteTexture(texture))
	{
		if(VIDEO_DEBUG)
			cerr << "glGetError() true after glDeleteTextures() in DrawTextHelper!" << endl;
		return false;
	}

	SetCoordSys( tempCoordSys._left, tempCoordSys._right, tempCoordSys._bottom, tempCoordSys._top);

	return true;
}


//-----------------------------------------------------------------------------
// DrawText: non unicode version
//-----------------------------------------------------------------------------

bool GameVideo::DrawText(const char *text, float x, float y)
{
	return DrawTextHelper(text, NULL, x, y);
}


//-----------------------------------------------------------------------------
// DrawText: unicode version
//-----------------------------------------------------------------------------

bool GameVideo::DrawText(const Uint16 *text, float x, float y)
{
	return DrawTextHelper(NULL, text, x, y);
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
// LoadFont: loads a font of a given size. The name parameter is a string which
//           you use to refer to the font when calling SetFont().
//
//   Example:  gamevideo->LoadFont( "fonts/arial.ttf", "arial36", 36 );
//-----------------------------------------------------------------------------

bool GameVideo::LoadFont(const string &filename, const string &name, int size)
{
	if( _fontMap.find(filename) != _fontMap.end() )
	{
		// font is already loaded, nothing to do
		return true;
	}

	TTF_Font *font = TTF_OpenFont(filename.c_str(), size);
	
	if(!font)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: TTF_OpenFont() failed for filename:\n" << filename.c_str() << endl;
		return false;
	}
	
	_fontMap[name] = font;
	return true;
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
// DrawImage: draws an image given the image descriptor
//-----------------------------------------------------------------------------

bool GameVideo::DrawImage(const ImageDescriptor &id)
{
	size_t numElements = id._elements.size();
	
	for(uint iElement = 0; iElement < numElements; ++iElement)
	{		
		glPushMatrix();
		MoveRel((float)id._elements[iElement].xOffset, (float)id._elements[iElement].yOffset);
		
		// include screen shaking effects
		MoveRel(_shakeX * (_coordSys._right - _coordSys._left) / 1024.0f, 
		        _shakeY * (_coordSys._top   - _coordSys._bottom) / 768.0f);  
		
		if(!DrawElement(
			id._elements[iElement].image, 
			id._elements[iElement].width, 
			id._elements[iElement].height,
			id._elements[iElement].color
		))
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: DrawElement() failed in DrawImage()!" << endl;
			return false;
		}
		glPopMatrix();
	}
	return true;
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

	if(_batching)
	{
		_batchImages.push_back(&id);
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
	_batching = true;
	_batchImages.clear();  // this should already be clear, but just in case...

	return true;
}

//-----------------------------------------------------------------------------
// EndImageLoadBatch: ends a batch load block
//                    returns false if any of the images failed to load
//-----------------------------------------------------------------------------

bool GameVideo::EndImageLoadBatch()
{	
	_batching = false;
	
	// go through vector of images waiting to be loaded and load them

	std::vector <ImageDescriptor *>::iterator iImage = _batchImages.begin();
	std::vector <ImageDescriptor *>::iterator iEnd   = _batchImages.end();
	
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

	_batchImages.clear();	
	
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
	
	GameVideo *videoManager = GameVideo::_GetReference();
	
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
	
	GameVideo *videoManager = GameVideo::_GetReference();
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
// DrawElement: draws an image element. This is only used privately.
//-----------------------------------------------------------------------------

bool GameVideo::DrawElement(const Image *const img, float w, float h, const Color &c)
{
	Color color = c;
	
	if(color.color[3] == 0.0f)
	{
		// do nothing, alpha is 0
		return true;
	}

	color.color[0] *= _fader.GetFadeModulation();
	color.color[1] *= _fader.GetFadeModulation();
	color.color[2] *= _fader.GetFadeModulation();
	
	float s0,s1,t0,t1;
	float xoff,yoff;
	float xlo,xhi,ylo,yhi;

	CoordSys &cs = _coordSys;
	
	if(img)
	{
		s0 = img->u1;
		s1 = img->u2;
		t0 = img->v1;
		t1 = img->v2;
	}

	if (_xflip) 
	{ 
		s0=1-s0; 
		s1=1-s1; 
	} 

	if (_yflip) 
	{ 
		t0=1-t0;
		t1=1-t1;
	} 

	if (_xflip)
	{
		xlo = (float) w;
		xhi = 0.0f;
	}
	else
	{
		xlo = 0.0f;
		xhi = (float) w;
	}
	if (cs._left > cs._right) 
	{ 
		xlo = (float) -xlo; 
		xhi = (float) -xhi; 
	}

	if (_yflip)
	{
		ylo=(float) h;
		yhi=0.0f;
	}
	else
	{
		ylo=0.0f;
		yhi=(float) h;
	}
	
	if (cs._bottom > cs._top) 
	{ 
		ylo=(float) -ylo; 
		yhi=(float) -yhi; 
	}

	xoff = ((_xalign+1) * w) * .5f * (cs._left < cs._right ? -1 : +1);
	yoff = ((_yalign+1) * h) * .5f * (cs._bottom < cs._top ? -1 : +1);

	if(img)
	{
		glEnable(GL_TEXTURE_2D);
		BindTexture(img->texSheet->texID);
	}	
	
	if (_blend || color.color[3] < 1.0f) 
	{
		glEnable(GL_BLEND);
		if (_blend == 1)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive
	}
	else
	{
		glDisable(GL_BLEND);
	}

	glPushMatrix();
	
	glTranslatef(xoff, yoff, 0);
	glBegin(GL_QUADS);
		glColor4fv(&(color.color[0]));
		
		if(img)
			glTexCoord2f(s0, t1);
		
		glVertex2f(xlo, ylo); //bl

		if(img)
			glTexCoord2f(s1, t1);

		glVertex2f(xhi, ylo); //br

		if(img)
			glTexCoord2f(s1, t0);

		glVertex2f(xhi, yhi);//tr

		if(img)
			glTexCoord2f(s0, t0);

		glVertex2f(xlo, yhi);//tl

	glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	if (_blend)
		glDisable(GL_BLEND);	
		
	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() returned true in DrawElement()!" << endl;
		return false;
	}		
		
	return true;
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


	return success;
}


//-----------------------------------------------------------------------------
// DrawFPS: draws current frames per second
//-----------------------------------------------------------------------------

bool GameVideo::DrawFPS(int frameTime)
{
	return _gui->DrawFPS(frameTime);
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
	return true;
}


//-----------------------------------------------------------------------------
// SetViewport: set the rectangle of the screen onto which all drawing maps to, 
//              the arguments are percentages so 0, 0, 100, 100 would mean the 
//              whole screen
//-----------------------------------------------------------------------------

void GameVideo::SetViewport(float left, float right, float bottom, float top) {
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
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT);
	
	_numTexSwitches = 0;
	
	if(glGetError())
		return false;
		
	return true;
}


//-----------------------------------------------------------------------------
// Display: if running in double buffered mode then flip the other buffer to the 
//          screen
//-----------------------------------------------------------------------------

bool GameVideo::Display(int frameTime) 
{
	// show an overlay over the screen if we're fading

	CoordSys oldSys = _coordSys;
	SetCoordSys(0, 1024, 0, 768);

	UpdateShake(frameTime);
	
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
	GameVideo *videoManager = GameVideo::_GetReference();
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
	GameVideo *videoManager = GameVideo::_GetReference();	
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


//-----------------------------------------------------------------------------
// Interpolator constructor
//-----------------------------------------------------------------------------

Interpolator::Interpolator()
{
	_method = VIDEO_INTERPOLATE_LINEAR;
	_currentTime = _endTime = 0;
	_a = _b  = 0.0f;
	_finished  = true;    // no interpolation in progress
	_currentValue = 0.0f;
}


//-----------------------------------------------------------------------------
// Start: begins an interpolation using a and b as inputs, in the given amount
//        of time.
//
// Note: not all interpolation methods mean "going from A to B". In the case of
//       linear, constant, fast, slow, they do start at A and go to B. But,
//       ease interpolations go from A to B and then back. And constant
//       interpolation means just staying at either A or B.
//-----------------------------------------------------------------------------

bool Interpolator::Start(float a, float b, int milliseconds)
{
	if(!ValidMethod())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: tried to start interpolation with invalid method!" << endl;
		return false;
	}
	
	if(milliseconds < 0)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: passed negative time value to Interpolator::Start()!" << endl;
		return false;
	}
	
	_a = a;
	_b = b;	
	
	_currentTime = 0;
	_endTime     = milliseconds;
	_finished    = false;
	
	Update(0);  // do initial update so we have a valid value for GetValue()
	return true;
}


//-----------------------------------------------------------------------------
// SetMethod: sets the current interpolation method. Two things will cause
//            this to fail:
//
//            1. You pass in an invalid method
//            2. You change the method while an interpolation is in progress
//-----------------------------------------------------------------------------

bool Interpolator::SetMethod(InterpolationMethod method)
{
	if(!_finished)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: tried to call SetMethod() on an interpolator that was still in progress!" << endl;
		return false;
	}
		
	if(!ValidMethod())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: passed an invalid method to Interpolator::SetMethod()!" << endl;
		return false;
	}
	
	_method = method;
	return true;
}


//-----------------------------------------------------------------------------
// GetValue: returns the current value of the interpolator. The current value
//           gets set when Update() is called so make sure to never call
//           GetValue() before updating
//-----------------------------------------------------------------------------

float Interpolator::GetValue()
{
	return _currentValue;
}


//-----------------------------------------------------------------------------
// Update: updates the interpolation by frameTime milliseconds.
//         If we reach the end of the interpolation, then IsFinished()
//         will return true.
//         This function will return false if the method is invalid.
//-----------------------------------------------------------------------------

bool Interpolator::Update(int frameTime)
{
	if(frameTime < 0)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: called Interpolator::Update() with negative frameTime!" << endl;
		return false;
	}
	
	if(!ValidMethod())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: called Interpolator::Update(), but method was invalid!" << endl;
		return false;
	}
	
	// update current time
	_currentTime += frameTime;
	
	if(_currentTime > _endTime)
	{
		_currentTime = _endTime;
		_finished    = true;
	}
	
	// calculate a value from 0.0f to 1.0f of how far we are in the interpolation	
	float t;
	
	if(_endTime == 0)
	{
		t = 1.0f;
	}
	else
	{
		t = (float)_currentTime / (float)_endTime;
	}
	
	if(t > 1.0f)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: calculated value of 't' was more than 1.0!" << endl;
		t = 1.0f;
	}
	
	// now apply a transformation based on the interpolation method
	
	switch(_method)
	{
		case VIDEO_INTERPOLATE_EASE:
			t = EaseTransform(t);			
			break;
		case VIDEO_INTERPOLATE_SRCA:
			t = 0.0f;
			break;
		case VIDEO_INTERPOLATE_SRCB:
			t = 1.0f;
			break;
		case VIDEO_INTERPOLATE_FAST:
			t = FastTransform(t);
			break;
		case VIDEO_INTERPOLATE_SLOW:
			t = SlowTransform(t);			
			break;
		case VIDEO_INTERPOLATE_LINEAR:
			// nothing to do, just use t value as it is!
			break;
		default:
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: in Interpolator::Update(), current method didn't match supported methods!" << endl;
			return false;
		}
	};
	
	_currentValue = Lerp(t, _a, _b);
	
	return true;
}


//-----------------------------------------------------------------------------
// FastTransform: rescales the range of t so that it looks like a sqrt function
//                from 0.0 to 1.0, i.e. it increases quickly then levels off
//-----------------------------------------------------------------------------

float Interpolator::FastTransform(float t)
{
	// the fast transform power is some number above 0.0 and less than 1.0
	return powf(t, VIDEO_FAST_TRANSFORM_POWER);
}


//-----------------------------------------------------------------------------
// SlowTransform: rescales the range of t so it looks like a power function
//                from 0.0 to 1.0, i.e. it increases slowly then rockets up
//-----------------------------------------------------------------------------

float Interpolator::SlowTransform(float t)
{
	// the slow transform power is a number above 1.0
	return powf(t, VIDEO_SLOW_TRANSFORM_POWER);
}


//-----------------------------------------------------------------------------
// EaseTransform: rescales the range of t so it increases slowly, rises to 1.0
//                then falls back to 0.0
//-----------------------------------------------------------------------------

float Interpolator::EaseTransform(float t)
{
	return 0.5f * (1.0f + sinf(VIDEO_2PI * (t - 0.25f)));
}


//-----------------------------------------------------------------------------
// IsFinished: returns true if the interpolator is done with the current
//             interpolation
//-----------------------------------------------------------------------------

bool Interpolator::IsFinished()
{
	return _finished;
}


//-----------------------------------------------------------------------------
// ValidMethod: private function to check that the current method is valid
//-----------------------------------------------------------------------------

bool Interpolator::ValidMethod()
{
	return (_method < VIDEO_INTERPOLATE_TOTAL && 
	        _method > VIDEO_INTERPOLATE_INVALID);	
}


//-----------------------------------------------------------------------------
// ShakeScreen: shakes the screen with a given force and shake method
//              If you want it to shake until you manually stop it, then
//              pass in VIDEO_FALLOFF_NONE and 0.0f for falloffTime
//-----------------------------------------------------------------------------

bool GameVideo::ShakeScreen(float force, float falloffTime, ShakeFalloff falloffMethod)
{
	// check inputs
	if(force < 0.0f)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: passed negative force to ShakeScreen()!" << endl;
		return false;
	}

	if(falloffTime < 0.0f)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: passed negative falloff time to ShakeScreen()!" << endl;
		return false;
	}

	if(falloffMethod <= VIDEO_FALLOFF_INVALID || falloffMethod >= VIDEO_FALLOFF_TOTAL)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: passed invalid shake method to ShakeScreen()!" << endl;
		return false;
	}
	
	if(falloffTime == 0.0f && falloffMethod != VIDEO_FALLOFF_NONE)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: ShakeScreen() called with 0.0f (infinite), but falloff method was not VIDEO_FALLOFF_NONE!" << endl;
		return false;
	}
		
	// create the shake force structure
	
	int milliseconds = int(falloffTime * 1000);
	ShakeForce s;
	s.currentTime  = 0;
	s.endTime      = milliseconds;
	s.initialForce = force;
	
	
	// set up the interpolation
	switch(falloffMethod)
	{
		case VIDEO_FALLOFF_NONE:
			s.interpolator.SetMethod(VIDEO_INTERPOLATE_SRCA);
			s.interpolator.Start(force, 0.0f, milliseconds);
			break;
		
		case VIDEO_FALLOFF_EASE:
			s.interpolator.SetMethod(VIDEO_INTERPOLATE_EASE);
			s.interpolator.Start(0.0f, force, milliseconds);
			break;
		
		case VIDEO_FALLOFF_LINEAR:
			s.interpolator.SetMethod(VIDEO_INTERPOLATE_LINEAR);
			s.interpolator.Start(force, 0.0f, milliseconds);
			break;
			
		case VIDEO_FALLOFF_GRADUAL:
			s.interpolator.SetMethod(VIDEO_INTERPOLATE_SLOW);
			s.interpolator.Start(force, 0.0f, milliseconds);
			break;
			
		case VIDEO_FALLOFF_SUDDEN:
			s.interpolator.SetMethod(VIDEO_INTERPOLATE_FAST);
			s.interpolator.Start(force, 0.0f, milliseconds);
			break;
		
		default:
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: falloff method passed to ShakeScreen() was not supported!" << endl;
			return false;
		}		
	};
	
	// add the shake force to GameVideo's list
	_shakeForces.push_front(s);
	
	return true;
}


//-----------------------------------------------------------------------------
// StopShaking: removes ALL shaking on the screen
//-----------------------------------------------------------------------------

bool GameVideo::StopShaking()
{
	_shakeForces.clear();
	_shakeX = _shakeY = 0.0f;
	return true;
}


//-----------------------------------------------------------------------------
// IsShaking: returns true if the screen has any shake effect applied to it
//-----------------------------------------------------------------------------

bool GameVideo::IsShaking()
{
	return !_shakeForces.empty();
}


//-----------------------------------------------------------------------------
// RoundForce: rounds a force to an integer. Whether to round up or round down
//             is based on the fractional part. A force of 1.37 has a 37%
//             chance of being a 2, else it's a 1
//             This is necessary because otherwise a shake force of 0.5f
//             would get rounded to zero all the time even though there is some
//             force
//-----------------------------------------------------------------------------

float GameVideo::RoundForce(float force)
{
	int fractionPct = int(force * 100) - (int(force) * 100);
	
	int r = rand()%100;
	if(fractionPct > r)
		force = ceilf(force);
	else
		force = floorf(force);
		
	return force;
}



//-----------------------------------------------------------------------------
// UpdateShake: called once per frame to update the the shake effects
//              and update the shake x,y offsets
//-----------------------------------------------------------------------------

void GameVideo::UpdateShake(int frameTime)
{
	if(_shakeForces.empty())
	{
		_shakeX = _shakeY = 0;
		return;
	}

	// first, update all the shake effects and calculate the net force, i.e.
	// the sum of the forces of all the shakes
	
	float netForce = 0.0f;
	
	list<ShakeForce>::iterator iShake = _shakeForces.begin();
	list<ShakeForce>::iterator iEnd   = _shakeForces.end();
	
	while(iShake != iEnd)
	{
		ShakeForce &s = *iShake;
		s.currentTime += frameTime;

		if(s.endTime != 0 && s.currentTime >= s.endTime)
		{
			iShake = _shakeForces.erase(iShake);
		}
		else
		{
			s.interpolator.Update(frameTime);
			netForce += s.interpolator.GetValue();
			++iShake;	
		}
	}	

	// cap the max update frequency
	
	static int timeTilNextUpdate = 0;		
	timeTilNextUpdate -= frameTime;
	
	if(timeTilNextUpdate > 0)
		return;
	
	timeTilNextUpdate = VIDEO_TIME_BETWEEN_SHAKE_UPDATES;


	// now that we have our force (finally), calculate the proper shake offsets
	// note that this doesn't produce a radially symmetric distribution of offsets
	// but I think it's not noticeable so... :)
	
	_shakeX = RoundForce(RandomFloat(-netForce, netForce));
	_shakeY = RoundForce(RandomFloat(-netForce, netForce));	
}



}  // namespace hoa_video
