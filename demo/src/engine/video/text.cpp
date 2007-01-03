///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    text.cpp
*** \author  Raj Sharma, roos@allacrost.org
*** \brief   Source file for text rendering
***
*** This code makes use of the SDL_ttf font library for representing fonts,
*** font glyphs, and text.
*** ***************************************************************************/

#include <cassert>
#include <cstdarg>
#include <math.h>
#include "utils.h"
#include "video.h"
#include "gui.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_video::private_video;

namespace hoa_video {

bool GameVideo::LoadFont(const string &filename, const string &name, uint32 size) {
	// Return true if the font is already loaded
	if (_font_map.find(filename) != _font_map.end()) {
		return true;
	}

	// Attempt to load the font
	TTF_Font *font = TTF_OpenFont(filename.c_str(), size);
	
	if (!font) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: TTF_OpenFont() failed for font file: " << filename.c_str() << endl;
		return false;
	}

	// Create a new FontProperties object for the font, and add it to the font map
	FontProperties *fp = new FontProperties;
	_font_map[name] = fp;

	// Set all of the font's properties
	fp->ttf_font = font;
	fp->height = TTF_FontHeight(font);
	fp->line_skip = TTF_FontLineSkip(font);
	fp->ascent = TTF_FontAscent(font);
	fp->descent = TTF_FontDescent(font);

	// Set default shadow: x to be 1/8th of the font's height (or 1 pixel), y to be -x
	fp->shadow_x = max(fp->height / 8, 1);
	fp->shadow_y = -fp->shadow_x;

	// Set default shadow style and create the glyph cache
	fp->shadow_style = VIDEO_TEXT_SHADOW_DARK;
	fp->glyph_cache = new std::map<uint16, FontGlyph*>;
		
	return true;
} // bool GameVideo::LoadFont(const string &filename, const string &name, int32 size)



FontProperties* GameVideo::GetFontProperties(const std::string &font_name) {
	if (!IsValidFont(font_name)) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: GetFontProperties() failed becase an invalid font name was passed: " << font_name << endl;
		return NULL;
	}
	
	return _font_map[font_name];
}



bool GameVideo::SetFont(const std::string &name) {
	// check if font is loaded before setting it
	if ( _font_map.find(name) == _font_map.end())
		return false;
		
	_current_font = name;
	return true;
}


//-----------------------------------------------------------------------------
// GetFont: returns the name of the current font (e.g. "verdana18")
//-----------------------------------------------------------------------------

std::string GameVideo::GetFont() const
{
	return _current_font;
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

	if(_font_map.empty())
		return false;
		
	// empty string, do nothing
	if(*uText == 0)
		return true;
		
	SDL_Color color;
	
	if(_font_map.find(_current_font) == _font_map.end())
		return false;
	
	FontProperties * fp = _font_map[_current_font];
	TTF_Font * font = fp->ttf_font;
	
	color.r = 255;
	color.g = 255;
	color.b = 255;
	
	
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);

	SDL_Surface *initial = NULL;
	SDL_Surface *intermediary = NULL;
	int32 w,h;
	GLuint texture;

	CoordSys &cs = _coord_sys;

	// Figure out which glyphs are not already cached
	std::vector<uint16> newglyphs;
	for(const uint16 * glyph = uText; *glyph != 0; glyph++)
	{
		if(fp->glyph_cache->find(*glyph) != fp->glyph_cache->end())
			continue;

		newglyphs.push_back(*glyph);
	}
	

	for(size_t glyphindex = 0; glyphindex < newglyphs.size(); glyphindex++)
	{
		
		initial = TTF_RenderGlyph_Blended(font, newglyphs[glyphindex], color);
		
		if(!initial)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: TTF_RenderUNICODE_Blended() returned NULL in _DrawTextHelper()!" << endl;
			return false;
		}

		w = RoundUpPow2(initial->w + 1);
		h = RoundUpPow2(initial->h + 1);	

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

		SDL_LockSurface(intermediary);
		for(int j = 0; j < w * h ; ++ j)
		{
			((uint8*)intermediary->pixels)[j*4+3] = ((uint8*)intermediary->pixels)[j*4+2];
			
			((uint8*)intermediary->pixels)[j*4+0] = 
			((uint8*)intermediary->pixels)[j*4+1] = 
			((uint8*)intermediary->pixels)[j*4+2] = 0xff;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, 
					 GL_UNSIGNED_BYTE, intermediary->pixels );
		SDL_UnlockSurface(intermediary);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	


		if(glGetError())
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: glGetError() true after glTexImage2D() in _DrawTextHelper!" << endl;
			return false;
		}

		int minx, maxx;
		int miny, maxy;
		int advance;

		if(TTF_GlyphMetrics(font, newglyphs[glyphindex], &minx, &maxx, &miny, &maxy, &advance))
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: glGetError() true after glTexImage2D() in _DrawTextHelper!" << endl;
			return false;
		}

		FontGlyph * glyph = new FontGlyph;
		glyph->texture = texture;
		glyph->min_x = minx;
		glyph->min_y = miny;
		glyph->width = initial->w + 1;
		glyph->height = initial->h + 1;
		glyph->max_x = (float)(((double)initial->w + 1) / ((double)w));
		glyph->max_y = (float)(((double)initial->h + 1) / ((double)h));
		glyph->advance = advance;

		fp->glyph_cache->insert(std::pair<uint16, FontGlyph *>(newglyphs[glyphindex], glyph));

		SDL_FreeSurface(initial);
		SDL_FreeSurface(intermediary);


	}
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_FOG);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);

	glPushMatrix();

	int fontwidth;
	int fontheight;
	if(TTF_SizeUNICODE(fp->ttf_font, uText, &fontwidth, &fontheight) != 0)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: TTF_SizeUNICODE() returned NULL in _DrawTextHelper()!" << endl;
		return false;
	}

	float xoff = ((_xalign+1) * fontwidth) * .5f * -cs.GetHorizontalDirection();
	float yoff = ((_yalign+1) * fontheight) * .5f * -cs.GetVerticalDirection();

	MoveRelative(xoff, yoff);

	float modulation = _fader.GetFadeModulation();
	Color textColor = _currentTextColor * modulation;

	int xpos = 0;

	for(const uint16 * glyph = uText; *glyph != 0; glyph++)
	{
		FontGlyph * glyphinfo = (*fp->glyph_cache)[*glyph];
		
		int xhi = glyphinfo->width; 
		int yhi = glyphinfo->height;
		
		if(cs.GetHorizontalDirection() < 0.0f)
			xhi = -xhi;
		if(cs.GetVerticalDirection() < 0.0f)
			yhi = -yhi;
			
		float tx, ty;
		tx = glyphinfo->max_x;
		ty = glyphinfo->max_y;

		int minx, miny;
		minx = glyphinfo->min_x * (int)cs.GetHorizontalDirection() + xpos;
		miny = glyphinfo->min_y * (int)cs.GetVerticalDirection();
		
		_BindTexture(glyphinfo->texture);

		if(glGetError())
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: glGetError() true after 2nd call to glBindTexture() in _DrawTextHelper!" << endl;
			return false;
		}
		
		glBegin(GL_QUADS);
		glColor4fv((GLfloat *)&textColor);

		glTexCoord2f(0.0, ty); 
		glVertex2i(minx, miny);

		glTexCoord2f(tx, ty); 
		glVertex2i(minx + xhi, miny);

		glTexCoord2f(tx, 0.0); 
		glVertex2i(minx + xhi, miny + yhi);

		glTexCoord2f(0.0, 0.0); 
		glVertex2i(minx, miny + yhi);

		glEnd();

		xpos += glyphinfo->advance;
	}


	glPopMatrix();
	
	if(_fogIntensity > 0.0f)
		glEnable(GL_FOG);

	glDisable(GL_ALPHA_TEST);

	glFinish();

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
	ustring wstr = MakeUnicodeString(txt);
	return DrawText(wstr);
}


//-----------------------------------------------------------------------------
// DrawText: unicode version
//-----------------------------------------------------------------------------

bool GameVideo::DrawText(const ustring &txt)
{
	if(txt.empty())
	{
		// Previously, if an empty string was passed, it was considered an error
		// However, it happens often enough in practice that now we just return true
		// without doing anything.
		
		return true;
	}

	if(_font_map.find(_current_font) == _font_map.end())
	{
		if(VIDEO_DEBUG)
			cerr << "GameVideo::DrawText() failed because font passed was either not loaded or improperly loaded!\n" <<
			        "  *fontname: " << _current_font << endl;
		return false;
	}

	FontProperties * fp = _font_map[_current_font];
	TTF_Font * font = fp->ttf_font;
	
	if(font)
	{
		_PushContext();
		int32 lineSkip = fp->line_skip;
		
		// Optimization: something seems to be wrong with ustring, using a buffer instead
		uint16 buffer[2048];
		uint16 newline('\n');

		size_t lastline = 0;
		
		do
		{
			size_t nextline;
			for(nextline = lastline; nextline < txt.length(); nextline++)
			{
				if(txt[nextline] == newline)
					break;

				buffer[nextline - lastline] = txt[nextline];
			}
			buffer[nextline - lastline] = 0;
			lastline = nextline + 1;

			glPushMatrix();
			
			Color oldTextColor = _currentTextColor;

			// if text shadows are enabled, draw the shadow
			if(_textShadow && fp->shadow_style != VIDEO_TEXT_SHADOW_NONE)
			{
				Color textColor;
				
				switch(fp->shadow_style)
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
							cerr << "VIDEO ERROR: Unknown text shadow style (" << fp->shadow_style << ") found in GameVideo::DrawText()!" << endl;
						break;
					}
				};
				SetTextColor(textColor);
				
				
				glPushMatrix();
				MoveRelative(+_coord_sys.GetHorizontalDirection() * fp->shadow_x, 0.0f);
				MoveRelative(0.0f, _coord_sys.GetVerticalDirection() * fp->shadow_y);
				
				if(!_DrawTextHelper(buffer))
				{
					_PopContext();
					return false;
				}
				glPopMatrix();
			}

			SetTextColor(oldTextColor);

			// draw the text itself
			if(!_DrawTextHelper(buffer))
			{
				_PopContext();
				return false;
			}
			
			glPopMatrix();
			
			MoveRelative(0, -lineSkip * _coord_sys.GetVerticalDirection());

		} while(lastline < txt.length());
		
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
	if(-1 == TTF_SizeUNICODE(_font_map[fontName]->ttf_font, text.c_str(), &w, NULL))
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
	if(-1 == TTF_SizeText(_font_map[fontName]->ttf_font, text.c_str(), &w, NULL))
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
	if(_font_map.find(fontName) == _font_map.end())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: GameVideo::SetFontShadowXOffset() failed for font (" << fontName << ") because the font's properties could not be found!" << endl;
		return false;
	}
	
	FontProperties *fp = _font_map[fontName];
	
	if(!fp)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: GameVideo::SetFontShadowXOffset() failed for font (" << fontName << ") because the FontProperties pointer was NULL!" << endl;
		return false;
	}

	fp->shadow_x = x;
	return true;
}


//-----------------------------------------------------------------------------
// SetFontShadowYOffset: sets y offset to use for font shadow
//-----------------------------------------------------------------------------

bool GameVideo::SetFontShadowYOffset(const std::string &fontName, int32 y)
{
	if(_font_map.find(fontName) == _font_map.end())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: GameVideo::SetFontShadowYOffset() failed for font (" << fontName << ") because the font's properties could not be found!" << endl;
		return false;
	}

	FontProperties *fp = _font_map[fontName];
	
	if(!fp)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: GameVideo::SetFontShadowYOffset() failed for font (" << fontName << ") because the FontProperties pointer was NULL!" << endl;
		return false;
	}
	
	fp->shadow_y = y;
	return true;
}


//-----------------------------------------------------------------------------
// SetFontShadowStyle: sets the shadow style for the given font
//-----------------------------------------------------------------------------

bool GameVideo::SetFontShadowStyle(const std::string &fontName, TEXT_SHADOW_STYLE style)
{
	if(_font_map.find(fontName) == _font_map.end())
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: GameVideo::SetFontShadowYOffset() failed for font (" << fontName << ") because the font's properties could not be found!" << endl;
		return false;
	}

	FontProperties *fp = _font_map[fontName];
	
	if(!fp)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: GameVideo::SetFontShadowYOffset() failed for font (" << fontName << ") because the FontProperties pointer was NULL!" << endl;
		return false;
	}
	
	fp->shadow_style = style;
	return true;
}



}  // namespace hoa_video
