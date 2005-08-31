#include "utils.h"
#include <cassert>
#include <cstdarg>
#include "video.h"
#include <math.h>
#include "gui.h"

using namespace std;
using namespace hoa_video::private_video;
using hoa_utils::MakeWideString;
using hoa_utils::ustring;

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
	// quit if font is already loaded
	if( _fontMap.find(filename) != _fontMap.end() )
	{
		return true;
	}

	// load the font
	TTF_Font *font = TTF_OpenFont(filename.c_str(), size);
	
	if(!font)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: TTF_OpenFont() failed for filename:\n" << filename.c_str() << endl;
		return false;
	}
	
	_fontMap[name] = font;


	// figure out all the font's properties
	
	FontProperties *fp = new FontProperties;

	fp->height    = TTF_FontHeight(font);
	fp->lineskip  = TTF_FontLineSkip(font);
	fp->ascent    = TTF_FontAscent(font);
	fp->descent   = TTF_FontDescent(font);
	
	fp->shadowX   = max(fp->height / 8, 1);
	fp->shadowY   = -fp->shadowX;
	
	fp->shadowStyle = VIDEO_TEXT_SHADOW_DARK;

	_fontProperties[name] = fp;
		
	return true;
}


//-----------------------------------------------------------------------------
// IsValidFont: returns true if the font with the given name has been
//              successfully loaded
//-----------------------------------------------------------------------------

bool GameVideo::IsValidFont(const std::string &name)
{
	// simple check, if it's in the std::map of fonts, it must be loaded
	return (_fontMap.find(name) != _fontMap.end());
}


//-----------------------------------------------------------------------------
// GetFontProperties: given font name, return properties into fp
//-----------------------------------------------------------------------------

bool GameVideo::GetFontProperties(const std::string &fontName, FontProperties &fp)
{
	if(!IsValidFont(fontName))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: GameVideo::GetFontProperties() failed becase an invalid font name was passed:" << endl << "(" << fontName << ")" << endl;
		return false;
	}

	if(_fontProperties.find(fontName) == _fontProperties.end())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: GameVideo::GetFontProperties() failed because although the font was found in fontMap, its properties could not be found" << endl;
		return false;
	}
	
	fp = *(_fontProperties[fontName]);
	
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
	const uint16 *uText
)
{
	if(_fontMap.empty())
		return false;
		
	SDL_Color color;
	
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
	/*
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
	*/	
	w = RoundUpPow2(initial->w);
	h = RoundUpPow2(initial->h);


	CoordSys &cs = _coordSys;

	float xoff = ((_xalign+1) * initial->w) * .5f * -cs._rightDir;
	float yoff = ((_yalign+1) * initial->h) * .5f * -cs._upDir;

    uint32 rmask, gmask, bmask, amask;

		// SDL interprets each pixel as a 32-bit number, so our masks must depend
		// on the endianness (byte order) of the machine
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
	#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
	#endif	

	intermediary = SDL_CreateRGBSurface(0, w, h, 32, 
			rmask, gmask, bmask, amask);

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

	for(int j = 0; j < w * h ; ++ j)
	{
		((uint8*)intermediary->pixels)[j*4+3] = ((uint8*)intermediary->pixels)[j*4+2];
		
		((uint8*)intermediary->pixels)[j*4+0] = 
		((uint8*)intermediary->pixels)[j*4+1] = 
		((uint8*)intermediary->pixels)[j*4+2] = 0xff;
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

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_ONE, GL_ONE);
	//glBlendFunc(GL_DST_COLOR, GL_ZERO);
	//glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);
	_BindTexture(texture);
	if(glGetError())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: glGetError() true after 2nd call to glBindTexture() in _DrawTextHelper!" << endl;
		return false;
	}

//	glColor3f(1.0f, 1.0f, 1.0f);

	glDisable(GL_FOG);

	glPushMatrix();
	
	MoveRelative(xoff, yoff);
	
	glBegin(GL_QUADS);
	if(_coordSys._upDir > 0.0f)
	{
		glColor4fv((GLfloat *)&_currentTextColor);
		glTexCoord2f(0.0f, 1.0f); 
		glVertex2f(0.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); 
		glVertex2f((float)w, 0.0f);
		glTexCoord2f(1.0f, 0.0f); 
		glVertex2f((float)w, (float)h);
		glTexCoord2f(0.0f, 0.0f); 
		glVertex2f(0.0f, (float)h);
	}
	else
	{
		glColor4fv((GLfloat *)&_currentTextColor);
		glTexCoord2f(0.0f, 1.0f); 
		glVertex2f(0.0f, (float)h);
		glTexCoord2f(1.0f, 1.0f); 
		glVertex2f((float)w, (float)h);
		glTexCoord2f(1.0f, 0.0f); 
		glVertex2f((float)w, 0.0f);
		glTexCoord2f(0.0f, 0.0f); 
		glVertex2f(0.0f, 0.0f);
	}	
	glEnd();

	glPopMatrix();
	
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
// Note: this simply converts to unicode and calls the unicode version.
//       This performance penalty is acceptable because any text that the player
//       sees will be unicode text anyways. Non-unicode is only used for
//       debugging text.
//-----------------------------------------------------------------------------

bool GameVideo::DrawText(const string &txt)
{
	ustring wstr = MakeWideString(txt);
	return DrawText(wstr);
}


//-----------------------------------------------------------------------------
// DrawText: unicode version
//-----------------------------------------------------------------------------

bool GameVideo::DrawText(const ustring &txt)
{
	if(txt.empty())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: tried to draw empty unicode text string!" << endl;		
	}

	FontProperties fp;
	if(!GetFontProperties(_currentFont, fp))
	{
		if(VIDEO_DEBUG)
			cerr << "GameVideo::DrawText() failed because font passed was either not loaded or improperly loaded!\n" <<
			        "  *fontname: " << _currentFont << endl;
		return false;
	}

	TTF_Font *font = _fontMap[_currentFont];
	
	uint32 newline = uint32('\n');
	
	if(font)
	{
		_PushContext();
		int32 lineSkip = fp.lineskip;
		
		// temporary so we can mess with it
		ustring text = txt;
		
		do
		{
			size_t newlinePos = text.find(newline);
			ustring textToDraw;
			
			if(newlinePos != string::npos)
			{
				// if there's a newline, draw the text up to the newline			
				textToDraw = text.substr(0, newlinePos);
				text = text.substr(newlinePos+1, text.length()-newlinePos);
			}
			else
			{
				// if there's no newline, draw the entire string
				textToDraw = text;
				text.clear();
			}

			glPushMatrix();
			
			Color oldTextColor = _currentTextColor;

			// if text shadows are enabled, draw the shadow
			if(_textShadow && fp.shadowStyle != VIDEO_TEXT_SHADOW_NONE)
			{
				Color textColor;
				
				switch(fp.shadowStyle)
				{
					case VIDEO_TEXT_SHADOW_DARK:
						textColor = Color::black;
						textColor[3] = oldTextColor[3] * 0.5f;
						break;
					case VIDEO_TEXT_SHADOW_LIGHT:
						textColor = Color::white;
						textColor[3] = oldTextColor[3] * 0.5f;
						break;
					case VIDEO_TEXT_SHADOW_BLACK:
						textColor = Color::black;
						textColor[3] = oldTextColor[3];
						break;
					case VIDEO_TEXT_SHADOW_COLOR:
						textColor = oldTextColor;
						textColor[3] = oldTextColor[3] * 0.5f;
						break;
					case VIDEO_TEXT_SHADOW_INVCOLOR:
						textColor = Color(1.0f - oldTextColor[0], 1.0f - oldTextColor[1], 1.0f - oldTextColor[2], oldTextColor[3] * 0.5f);
						break;
					default:
					{
						if(VIDEO_DEBUG)
							cerr << "VIDEO ERROR: Unknown text shadow style (" << fp.shadowStyle << ") found in GameVideo::DrawText()!" << endl;
						break;
					}
				};
				SetTextColor(textColor);
				
				
				glPushMatrix();
				MoveRelative(+_coordSys._rightDir * fp.shadowX, 0.0f);
				MoveRelative(0.0f, _coordSys._upDir * fp.shadowY);
				
				if(!_DrawTextHelper(textToDraw.c_str()))
				{
					_PopContext();
					return false;
				}
				glPopMatrix();
			}

			SetTextColor(oldTextColor);

			// draw the text itself
			if(!_DrawTextHelper(textToDraw.c_str()))
			{
				_PopContext();
				return false;
			}
			
			glPopMatrix();
			
			MoveRelative(0, -lineSkip * _coordSys._upDir);

		} while(!text.empty());
		
		_PopContext();
	}
		
	return true;
}


//-----------------------------------------------------------------------------
// CalculateTextWidth: return the width of the given text using the given font
//-----------------------------------------------------------------------------

int32 GameVideo::CalculateTextWidth(const std::string &fontName, const hoa_utils::ustring &text)
{
	if(!IsValidFont(fontName))
		return -1;
		
	int32 w;	
	if(-1 == TTF_SizeUNICODE(_fontMap[fontName], text.c_str(), &w, NULL))
		return -1;
		
	return w;
}


//-----------------------------------------------------------------------------
// CalculateTextWidth: non-unicode version
//-----------------------------------------------------------------------------

int32 GameVideo::CalculateTextWidth(const std::string &fontName, const std::string  &text)
{
	if(!IsValidFont(fontName))
		return -1;

	int32 w;	
	if(-1 == TTF_SizeText(_fontMap[fontName], text.c_str(), &w, NULL))
		return -1;
		
	return w;
}


//-----------------------------------------------------------------------------
// EnableTextShadow: enable/disable text shadow effect
//-----------------------------------------------------------------------------

void GameVideo::EnableTextShadow(bool enable)
{
	_textShadow = enable;
}


//-----------------------------------------------------------------------------
// SetFontShadowXOffset: sets x offset to use for font shadow
//-----------------------------------------------------------------------------

bool GameVideo::SetFontShadowXOffset(const std::string &fontName, int32 x)
{
	if(_fontProperties.find(fontName) == _fontProperties.end())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: GameVideo::SetFontShadowXOffset() failed for font (" << fontName << ") because the font's properties could not be found!" << endl;
		return false;
	}
	
	FontProperties *fp = _fontProperties[fontName];
	
	if(!fp)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: GameVideo::SetFontShadowXOffset() failed for font (" << fontName << ") because the FontProperties pointer was NULL!" << endl;
		return false;
	}

	fp->shadowX = x;
	return true;
}


//-----------------------------------------------------------------------------
// SetFontShadowYOffset: sets y offset to use for font shadow
//-----------------------------------------------------------------------------

bool GameVideo::SetFontShadowYOffset(const std::string &fontName, int32 y)
{
	if(_fontProperties.find(fontName) == _fontProperties.end())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: GameVideo::SetFontShadowYOffset() failed for font (" << fontName << ") because the font's properties could not be found!" << endl;
		return false;
	}

	FontProperties *fp = _fontProperties[fontName];
	
	if(!fp)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: GameVideo::SetFontShadowYOffset() failed for font (" << fontName << ") because the FontProperties pointer was NULL!" << endl;
		return false;
	}
	
	fp->shadowY = y;
	return true;
}


//-----------------------------------------------------------------------------
// SetFontShadowStyle: sets the shadow style for the given font
//-----------------------------------------------------------------------------

bool GameVideo::SetFontShadowStyle(const std::string &fontName, TextShadowStyle style)
{
	if(_fontProperties.find(fontName) == _fontProperties.end())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: GameVideo::SetFontShadowYOffset() failed for font (" << fontName << ") because the font's properties could not be found!" << endl;
		return false;
	}

	FontProperties *fp = _fontProperties[fontName];
	
	if(!fp)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: GameVideo::SetFontShadowYOffset() failed for font (" << fontName << ") because the FontProperties pointer was NULL!" << endl;
		return false;
	}
	
	fp->shadowStyle = style;
	return true;
}



}  // namespace hoa_video
