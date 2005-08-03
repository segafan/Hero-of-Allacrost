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
// GameVideo
//-----------------------------------------------------------------------------

GameVideo::GameVideo() 
: _width(0), 
  _height(0), 
  _setUp(false),
  _blend(0), 
  _xalign(-1), 
  _yalign(-1), 
  _xflip(0), 
  _yflip(0),
  _currentDebugTexSheet(-1),
  _batching(false),
  _gui(NULL)
{
	if (VIDEO_DEBUG) fprintf(stdout, "VIDEO: GameVideo constructor invoked\n");

}

bool GameVideo::Initialize()
{
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) 
	{
		fprintf(stderr, "Barf! SDL Video Initialization failed!\n");
		exit(1);
	}

	SDL_Rect mode = {0,0,1024,768}; // TEMPORARY
	ChangeMode(mode);	

	// initialize DevIL
	ilInit();
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	ilEnable(IL_ORIGIN_SET);
	iluInit();
	ilutRenderer(ILUT_OPENGL);

	// initialize SDL_ttf
	TTF_Init();

	LoadFont("img/fonts/cour.ttf", "default", 18);
	
	// create our default texture sheets

	CreateTexSheet(512, 512, VIDEO_TEXSHEET_32x32, false);
	CreateTexSheet(512, 512, VIDEO_TEXSHEET_32x64, false);
	CreateTexSheet(512, 512, VIDEO_TEXSHEET_64x64, false);
	CreateTexSheet(512, 512, VIDEO_TEXSHEET_ANY,   true);
	CreateTexSheet(512, 512, VIDEO_TEXSHEET_ANY,   false);

	Clear();
	Display();
	Clear();

	_gui = new GUI;
		
	return true;
}

bool GameVideo::MakeScreenshot()
{
	//ilutGLScreenie();
	
	ILuint screenshot;
	ilGenImages(1, &screenshot);
	ilBindImage(screenshot);
	
	ilEnable(IL_FILE_OVERWRITE);
	ilutGLScreen();
	ilSaveImage("screenshot.jpg");
	ilDeleteImages(1, &screenshot);
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
	CoordSys tempCoordSys = _coordSys;
	
	SetCoordSys(0,1024,0,768,0);
	SDL_Rect location;
	SDL_Color color;
	location.x = (int)x;
	location.y = (int)y;
	
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
	}
	else
	{
		initial = TTF_RenderText_Blended(font, text, color);
	}
		
	w = RoundUpPow2(initial->w);
	h = RoundUpPow2(initial->h);
	
	intermediary = SDL_CreateRGBSurface(0, w, h, 32, 
			0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

	SDL_BlitSurface(initial, 0, intermediary, 0);
	
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, 
			GL_UNSIGNED_BYTE, intermediary->pixels );
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
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
	glDeleteTextures(1, &texture);

	SetCoordSys( tempCoordSys._left, tempCoordSys._right, tempCoordSys._bottom, tempCoordSys._top, tempCoordSys._layer );

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
	if (VIDEO_DEBUG) fprintf(stdout, "VIDEO: GameVideo destructor invoked\n"); 
	
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
	float top, 
	int layer
)
{
	_coordSys = CoordSys(left, right, bottom, top, layer);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glOrtho(_coordSys._left, _coordSys._right, _coordSys._bottom, _coordSys._top, -1e-4, _coordSys._layer+1e-4);
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
		return false;
	
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
	for (n=0;;n++) {
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
			fprintf(stderr, "Unknown flag %d passed to SetDrawFlags()\n", flag);
		}
	}
done:
	va_end(args);
}


//-----------------------------------------------------------------------------
// DrawImage: draws an image given the image descriptor
//-----------------------------------------------------------------------------

void GameVideo::DrawImage(const ImageDescriptor &id)
{
	size_t numElements = id._elements.size();
	
	for(uint iElement = 0; iElement < numElements; ++iElement)
	{		
		glPushMatrix();
		MoveRel((float)id._elements[iElement].xOffset, (float)id._elements[iElement].yOffset);
		DrawElement
		(
			id._elements[iElement].image, 
			id._elements[iElement].width, 
			id._elements[iElement].height,
			id._elements[iElement].color
		);
		glPopMatrix();
	}
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

		if(img->refCount == 0)
		{
			// if ref count is zero, it means this image was freed, but
			// not removed, so restore it
			img->texSheet->RestoreImage(img);
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

void GameVideo::BeginImageLoadBatch()
{
	_batching = true;
	_batchImages.clear();  // this should already be clear, but just in case...
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

	// load the image using DevIL

	ILuint pixelData;
	uint w,h;
	ilGenImages(1, &pixelData);
	ilBindImage(pixelData);

	if (!ilLoadImage((char *)id.filename.c_str())) 
	{
		ilDeleteImages(1, &pixelData);
		return false;
	}
	
	// find width and height
	
	w = ilGetInteger(IL_IMAGE_WIDTH);
	h = ilGetInteger(IL_IMAGE_HEIGHT);
	

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
			return NULL;
					
		if(sheet->AddImage(image, pixelData))
			return sheet;
		else
			return NULL;
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
	
	// attempt to create a GL texture with the given width and height
	// if texture creation fails, return NULL

	int error;
			
	GLuint texID;

	glGenTextures(1, &texID);
	error = glGetError();
	
	if(!error)
	{
		glBindTexture(GL_TEXTURE_2D, texID);
		error = glGetError();
		
		if(!error)
		{		
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			error = glGetError();
		}
	}
		
	if(error != 0)
	{
		glDeleteTextures(1, &texID);
		
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: failed to create new texture in CreateTexSheet()." << endl;
			cerr << "  OpenGL reported the following error:" << endl << "  ";
			char *errString = (char*)gluErrorString(error);
			cerr << errString << endl;
		}
		return NULL;
	}
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	
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
	
	// unload actual texture from memory
	glDeleteTextures(1, &texID);
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


void GameVideo::DEBUG_ShowTexSheet()
{
	// value of zero means to disable display
	if(_currentDebugTexSheet == -1)
		return;
		
	// check if there aren't any texture sheets! (should never happen)
	if(_texSheets.empty())
	{
		if(VIDEO_DEBUG)		
			cerr << "VIDEO_WARNING: Called DEBUG_ShowTexture(), but there were no texture sheets" << endl;
		return;
	}
	
	
	CoordSys tempCoordSys = _coordSys;
	SetCoordSys(0.0f, 1024.0f, 0.0f, 768.0f, 1);
	
	TexSheet *sheet = _texSheets[_currentDebugTexSheet];
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
	
	DrawElement(&img, (float)w, (float)h, Color(1.0f, 1.0f, 1.0f, 1.0f));
	glPopMatrix();
	
	_blend = blend;
	_xalign = xalign;
	_yalign = yalign;

	SetFont("default");
	
	char buf[200];
	
	DrawText("Current Texture sheet:", 20, _coordSys._top - 30);
	
	sprintf(buf, "  Sheet #: %d", _currentDebugTexSheet);	
	DrawText(buf, 20, _coordSys._top - 50);
	
	sprintf(buf, "  Size:    %dx%d", sheet->width, sheet->height);
	DrawText(buf, 20, _coordSys._top - 70);
	
	if(sheet->type == VIDEO_TEXSHEET_32x32)
		sprintf(buf, "  Type:    32x32");
	else if(sheet->type == VIDEO_TEXSHEET_32x64)
		sprintf(buf, "  Type:    32x64");
	else if(sheet->type == VIDEO_TEXSHEET_64x64)
		sprintf(buf, "  Type:    64x64");
	else if(sheet->type == VIDEO_TEXSHEET_ANY)
		sprintf(buf, "  Type:    Any size");
	
	DrawText(buf, 20, _coordSys._top - 90);
	
	sprintf(buf, "  Static:  %d", sheet->isStatic);
	DrawText(buf, 20, _coordSys._top - 110);

	sprintf(buf, "  TexID:   %d", sheet->texID);
	DrawText(buf, 20, _coordSys._top - 130);

		
	SetCoordSys(tempCoordSys._left, tempCoordSys._right, tempCoordSys._bottom, tempCoordSys._top, tempCoordSys._layer);
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
//-----------------------------------------------------------------------------

bool TexSheet::AddImage(Image *img, ILuint pixelData)
{
	int error;

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
	
	GLuint texID = img->texSheet->texID;
	
	glBindTexture(GL_TEXTURE_2D, texID);
	
	error = glGetError();
	if(error)
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: could not bind texture in TexSheet::AddImage()!" << endl;
		}
		return false;
	}
	
	ILubyte *pixels = ilGetData();
	
	GLenum format = ilGetInteger(IL_IMAGE_FORMAT);

	glTexSubImage2D
	(
		GL_TEXTURE_2D,    // target
		0,                // level
		img->x,           // x offset
		img->y,           // y offset
		img->width,       // width
		img->height,      // height
		format,           // format
		GL_UNSIGNED_BYTE, // type
		pixels            // pixels of the sub image
	);

	error = glGetError();
	if(error)
	{
		if(VIDEO_DEBUG)
		{
			cerr << "VIDEO ERROR: glTexSubImage2D() failed in TexSHeet::AddImage()!" << endl;
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

void GameVideo::DrawElement(const Image *const img, float w, float h, const Color &color)
{
	if(color.color[3] == 0.0f)
	{
		// do nothing, alpha is 0
		return;
	}
	
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
		glBindTexture(GL_TEXTURE_2D, img->texSheet->texID);
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
// ReloadImages: reloads images if the gl context is lost
//-----------------------------------------------------------------------------

void GameVideo::ReloadImages() 
{
/* TODO: reimplement this
	unsigned n= (unsigned) _images.size();
	
	for (unsigned i=0; i<n; i++) 
	{
		if (_images[i]->texid!=0)
			glDeleteTextures(1, &_images[i]->texid);
			
		LoadTexture(*_images[i]);
	}
	*/
}


//-----------------------------------------------------------------------------
// UnloadImages: frees the memory taken up by the images, but leaves the
//               list of images alone in case they need to be reloaded
//-----------------------------------------------------------------------------

void GameVideo::UnloadImages() 
{
/*  REIMPLEMENT THIS
	unsigned n= (unsigned) _images.size();
	for (unsigned i=0; i<n; i++) {
		if (_images[i]->texid != 0) {
			glDeleteTextures(1,&_images[i]->texid);
			_images[i]->texid=0;
		}
	}
*/
}


//-----------------------------------------------------------------------------
// DeleteImages: deletes all the images completely (dangerous!)
//-----------------------------------------------------------------------------

void GameVideo::DeleteImages() 
{
	UnloadImages();
	_images.clear();
}


//-----------------------------------------------------------------------------
// DrawFPS: draws current frames per second
//-----------------------------------------------------------------------------

bool GameVideo::DrawFPS(int frameTime)
{
	return _gui->DrawFPS(frameTime);
}


//-----------------------------------------------------------------------------
// ChangeMode: changes the mode, that's about it
//-----------------------------------------------------------------------------

bool GameVideo::ChangeMode(const SDL_Rect &s) 
{
	// Yea, so we need to reload our textures but we just lost our old GL
	// context. See ReloadImages() call below.
	UnloadImages();

	//probably gonna change the struct later
	int desWidth = s.w;
	int desHeight = s.h;
	int desFlags = SDL_OPENGL;

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); // XXX
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	if (SDL_SetVideoMode(desWidth, desHeight, 0, desFlags) == NULL) {
		_setUp=false;
		_width=0;
		_height=0;
	} else {
		_setUp=true;
		_width=desWidth;
		_height=desHeight;

		ReloadImages();
	}

	return _setUp;
}


//-----------------------------------------------------------------------------
// SetViewport: set the rectangle of the screen onto which all drawing maps to, 
//              the arguments are percentages so 0, 0, 100, 100 would mean the 
//              whole screen
//-----------------------------------------------------------------------------

void GameVideo::SetViewport(float left, float bottom, float right, float top) {
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

void GameVideo::Clear() 
{
	SetViewport(0,0,100,100);
	glClearColor(0,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT);
}


//-----------------------------------------------------------------------------
// Display: if running in double buffered mode then flip the other buffer to the 
//          screen
//-----------------------------------------------------------------------------

void GameVideo::Display() 
{
	DEBUG_ShowTexSheet();
	SDL_GL_SwapBuffers();
}


//-----------------------------------------------------------------------------
// SelectLayer: move directly to layer l and reset to (x,y) = (0,0)
//-----------------------------------------------------------------------------

void GameVideo::SelectLayer(int l) 
{
#ifndef NDEBUG
	GLint matrixMode;
	glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
	assert(matrixMode == GL_MODELVIEW);
#endif
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -1.0f);
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
// CreateMenu
//-----------------------------------------------------------------------------

bool GameVideo::CreateMenu(ImageDescriptor &id, float width, float height)
{
	return _gui->CreateMenu(id, width, height);
}





}



