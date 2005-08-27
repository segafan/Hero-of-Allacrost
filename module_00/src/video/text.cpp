#include "utils.h"
#include <cassert>
#include <cstdarg>
#include "video.h"
#include <math.h>
#include "gui.h"

using namespace std;
using namespace hoa_video::private_video;

namespace hoa_video 
{

extern uint32 RoundUpPow2(uint32 x);

//-----------------------------------------------------------------------------
// LoadFont: loads a font of a given size. The name parameter is a string which
//           you use to refer to the font when calling SetFont().
//
//   Example:  gamevideo->LoadFont( "fonts/arial.ttf", "arial36", 36 );
//-----------------------------------------------------------------------------

bool GameVideo::LoadFont(const string &filename, const string &name, int32 size)
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
// _DrawTextHelper: since there are two DrawText functions (one for unicode and
//                 one for non-unicode), this private function is used to
//                 do all the work so that code doesn't have to be duplicated.
//                 Either text or uText is valid string and the other is NULL.
//-----------------------------------------------------------------------------

bool GameVideo::_DrawTextHelper
( 
	const char    *text, 
	const wchar_t *uText, 
	float x, 
	float y 
)
{
	if(_fontMap.empty())
		return false;
		
	SetCoordSys(0,1024,0,768);
	SDL_Rect location;
	SDL_Color color;
	location.x = (int32)x;
	location.y = (int32)y;
	
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
	int32 w,h;
	GLuint texture;
	
	
	if( uText )
	{
		initial = TTF_RenderUNICODE_Blended(font, (uint16 *) uText, color);
		
		if(!initial)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: TTF_RenderUNICODE_Blended() returned NULL in _DrawTextHelper()!" << endl;
			return false;
		}
	}
	else
	{
		initial = TTF_RenderText_Blended(font, text, color);

		if(!initial)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: TTF_RenderText_Blended() returned NULL in _DrawTextHelper()!\n(current font = " << _currentFont << ", text={" << text << "}" << endl;
			return false;
		}
	}
		
	w = RoundUpPow2(initial->w);
	h = RoundUpPow2(initial->h);


	CoordSys &cs = _coordSys;
	float xoff = ((_xalign+1) * initial->w) * .5f * (cs._left < cs._right ? -1 : +1);
	float yoff = ((_yalign+1) * initial->h) * .5f * (cs._bottom < cs._top ? -1 : +1);
	
	location.x += (int32)xoff;
	location.y += (int32)yoff;
	
	intermediary = SDL_CreateRGBSurface(0, w, h, 32, 
			0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

	if(!intermediary)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: SDL_CreateRGBSurface() returned NULL in _DrawTextHelper()!" << endl;
		return false;
	}


	if(SDL_BlitSurface(initial, 0, intermediary, 0) < 0)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: SDL_BlitSurface() failed in _DrawTextHelper()!" << endl;
		return false;
	}


	glGenTextures(1, &texture);
	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() true after glGenTextures() in _DrawTextHelper!" << endl;
		return false;
	}
	
	_BindTexture(texture);
	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() true after glBindTexture() in _DrawTextHelper!" << endl;
		return false;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, 
	             GL_UNSIGNED_BYTE, intermediary->pixels );

	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() true after glTexImage2D() in _DrawTextHelper!" << endl;
		return false;
	}

	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	

	glEnable(GL_TEXTURE_2D);
	_BindTexture(texture);
	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() true after 2nd call to glBindTexture() in _DrawTextHelper!" << endl;
		return false;
	}

	glColor3f(1.0f, 1.0f, 1.0f);

	glDisable(GL_FOG);

	
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
	
	if(_fogIntensity > 0.0f)
		glEnable(GL_FOG);

	glFinish();
	
	SDL_FreeSurface(initial);
	SDL_FreeSurface(intermediary);
		
	if(!_DeleteTexture(texture))
	{
		if(VIDEO_DEBUG)
			cerr << "glGetError() true after glDeleteTextures() in _DrawTextHelper!" << endl;
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// DrawText: non unicode version
//-----------------------------------------------------------------------------

bool GameVideo::DrawText(const string &txt, float x, float y)
{		
	if(txt.empty())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: tried to draw empty text string!" << endl;		
	}


	TTF_Font *font = _fontMap[_currentFont];

	_PushContext();

	if(font)
	{
		int32 lineSkip = TTF_FontLineSkip(_fontMap[GetFont()]);	
		// use a temporary string since the code changes it
		string text = txt;
		
		do
		{
			size_t newlinePos = text.find("\n");
			if(newlinePos != string::npos)
			{
				if(!_DrawTextHelper(text.substr(0, newlinePos).c_str(), NULL, x, y))
				{
					_PopContext();
					return false;
				}
				text = text.substr(newlinePos+1, text.length()-newlinePos);
				y -= lineSkip;
			}
			else
			{
				if(!_DrawTextHelper(text.c_str(), NULL, x, y))
				{
					_PopContext();
					return false;
				}
				text = "";
			}
		} while(text != "");
	}
	
	_PopContext();

	return true;
}


//-----------------------------------------------------------------------------
// DrawText: unicode version
//-----------------------------------------------------------------------------

bool GameVideo::DrawText(const wstring &txt, float x, float y)
{
	TTF_Font *font = _fontMap[_currentFont];
	
	uint32 newline = uint32('\n');
	
	if(font)
	{
		_PushContext();
		int32 lineSkip = TTF_FontLineSkip(_fontMap[GetFont()]);	
		
		// temporary so we can mess with it
		wstring text = txt;
		
		do
		{
			size_t newlinePos = text.find(newline);
			if(newlinePos != string::npos)
			{
				if(!_DrawTextHelper(NULL, text.substr(0, newlinePos).c_str(), x, y))
				{
					_PopContext();
					return false;
				}
				text = text.substr(newlinePos+1, text.length()-newlinePos);
				y -= lineSkip;
			}
			else
			{
				if(!_DrawTextHelper(NULL, text.c_str(), x, y))
				{
					_PopContext();
					return false;
				}
				text.clear();
			}
		} while(!text.empty());
		
		_PopContext();
	}
		
	return true;
}


}  // namespace hoa_video
