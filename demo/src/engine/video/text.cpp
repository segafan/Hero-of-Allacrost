///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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
	return _current_text_color;
}

//-----------------------------------------------------------------------------
// _GenTexLine: renders a text line to a texture
//-----------------------------------------------------------------------------
RenderedLine *GameVideo::_GenTexLine(uint16 *line, FontProperties *fp)
{
	if (!fp)
		return NULL;

	// Array is { texid, shadowTexid }
	GLuint texid[2] = { 0, 0 };

	if (!*line)
		return new RenderedLine(texid, 0, 0, 0, 0, 0, 0);
	
	static const SDL_Color color = { 0xFF, 0xFF, 0xFF, 0xFF };

	TTF_Font * font = fp->ttf_font;

	SDL_Surface *initial      = NULL;
	SDL_Surface *intermediary = NULL;
	// If we are shadowing, use an array
	// to store shadow colored versions
	// of text pixels.
	uint8       *shadowPixels = NULL;
	int32 lineW,    lineH;
	int32 textureW, textureH;

	// Number of textures we're rendering
	uint8 numTextures = 0;

	// Minimum Y value of the line
	int32 minY = 0;
	// Calculated line width
	int32 calcLineWidth = 0;

	Color shadowColor;

	uint16 *charPtr;

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		static const int rmask = 0xff000000;
		static const int gmask = 0x00ff0000;
		static const int bmask = 0x0000ff00;
		static const int amask = 0x000000ff;
	#else
		static const int rmask = 0x000000ff;
		static const int gmask = 0x0000ff00;
		static const int bmask = 0x00ff0000;
		static const int amask = 0xff000000;
	#endif	

	if (TTF_SizeUNICODE(font, line, &lineW, &lineH) == -1)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: TTF_SizeUNICODE() returned NULL in _GenTexLine()!" << endl;
		return NULL;
	}

	for (charPtr = line; *charPtr; ++charPtr)
	{
		FontGlyph * glyphinfo = (*fp->glyph_cache)[*charPtr];
		int curMinY = glyphinfo->top_y;
		if (curMinY < minY)
			minY = curMinY;
		calcLineWidth += glyphinfo->advance;
	}

	lineH -= minY;
	// TTF_SizeUNICODE underestimates line width as a 
	// result of its micro positioning
	if (calcLineWidth > lineW)
		lineW = calcLineWidth;

	textureW = RoundUpPow2(lineW + 1);
	textureH = RoundUpPow2(lineH + 1);

	intermediary = SDL_CreateRGBSurface(0, textureW, textureH, 32, 
			rmask, gmask, bmask, amask);

	if(!intermediary)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: SDL_CreateRGBSurface() returned NULL in _GenTexLine()!" << endl;
		return NULL;
	}

	SDL_Rect surfTarget;
	int xpos = 0;
	int ypos = -minY;
	for (charPtr = line; *charPtr; ++charPtr)
	{
		FontGlyph * glyphinfo = (*fp->glyph_cache)[*charPtr];

		initial = TTF_RenderGlyph_Blended(font, *charPtr, color);

		if(!initial)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: TTF_RenderGlyph_Blended() returned NULL in _GenTexLine()!" << endl;
			return NULL;
		}

		surfTarget.x = xpos + glyphinfo->min_x;
		surfTarget.y = ypos + glyphinfo->top_y;

		if(SDL_BlitSurface(initial, NULL, intermediary, &surfTarget) < 0)
		{
			SDL_FreeSurface(initial);
			SDL_FreeSurface(intermediary);
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: SDL_BlitSurface() failed in _GenTexLine()! (" << SDL_GetError() << ")" << endl;
			return NULL;
		}
		SDL_FreeSurface(initial);
		xpos += glyphinfo->advance;
	}

	if (_text_shadow)
	{
		shadowPixels = new uint8[intermediary->w * intermediary->h * 4];
		shadowColor  = _GetTextShadowColor(fp);
	}

	SDL_LockSurface(intermediary);
	for(int j = 0; j < intermediary->w * intermediary->h ; ++ j)
	{
		if (_text_shadow)
		{
			shadowPixels[j*4+3] = ((uint8*)intermediary->pixels)[j*4+2];
			shadowPixels[j*4+0] = (uint8) (shadowColor[0] * 0xFF);
			shadowPixels[j*4+1] = (uint8) (shadowColor[1] * 0xFF);
			shadowPixels[j*4+2] = (uint8) (shadowColor[2] * 0xFF);
		}
		((uint8*)intermediary->pixels)[j*4+3] = ((uint8*)intermediary->pixels)[j*4+2];
		((uint8*)intermediary->pixels)[j*4+0] = (uint8) (_current_text_color[0] * 0xFF);
		((uint8*)intermediary->pixels)[j*4+1] = (uint8) (_current_text_color[1] * 0xFF);
		((uint8*)intermediary->pixels)[j*4+2] = (uint8) (_current_text_color[2] * 0xFF);
	}

	numTextures = _text_shadow ? 2 : 1;

	glGenTextures(numTextures, texid);

	uint8 *texturePointers[2] =
	{ 
		(uint8 *)intermediary->pixels, 
		shadowPixels 
	};

	for (int j = 0; j < numTextures; ++j)
	{
		_BindTexture(texid[j]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	

		glTexImage2D(GL_TEXTURE_2D, 0, 4, textureW, textureH, 0, GL_RGBA, 
			     GL_UNSIGNED_BYTE, texturePointers[j] );

		GLenum err;
		if((err = glGetError()) != 0)
		{
		        if(VIDEO_DEBUG)
		   	     cerr << "VIDEO ERROR: _GenTexLine: glError found after glTexImage2D (" << gluErrorString(err) << ")" << endl;

			SDL_FreeSurface(intermediary);

			if (shadowPixels)
				delete shadowPixels;

		        return NULL;
		}

	}

	SDL_UnlockSurface(intermediary);
	SDL_FreeSurface(intermediary);

	if (shadowPixels)
		delete shadowPixels;

	return new RenderedLine(texid, lineW, textureW, lineH, textureH, 0, minY);
}

//-----------------------------------------------------------------------------
// _CacheGlyphs: renders unicode characters to the internal glyph cache
//-----------------------------------------------------------------------------

bool GameVideo::_CacheGlyphs
(
	const uint16 *uText,
	FontProperties *fp 
)
{
	if (!fp)
		return false;

	static const SDL_Color color = { 0xFF, 0xFF, 0xFF, 0xFF };

	TTF_Font * font = fp->ttf_font;
	
	SDL_Surface *initial = NULL;
	SDL_Surface *intermediary = NULL;
	int32 w,h;
	GLuint texture;

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
				cerr << "VIDEO ERROR: TTF_RenderUNICODE_Blended() returned NULL in CacheGlyphs()!" << endl;
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
			SDL_FreeSurface(initial);
			SDL_FreeSurface(intermediary);
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: SDL_CreateRGBSurface() returned NULL in CacheGlyphs()!" << endl;
			return false;
		}


		if(SDL_BlitSurface(initial, 0, intermediary, 0) < 0)
		{
			SDL_FreeSurface(initial);
			SDL_FreeSurface(intermediary);
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: SDL_BlitSurface() failed in CacheGlyphs()!" << endl;
			return false;
		}

		glGenTextures(1, &texture);
		if(glGetError())
		{
			SDL_FreeSurface(initial);
			SDL_FreeSurface(intermediary);
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: glGetError() true after glGenTextures() in CacheGlyphs()!" << endl;
			return false;
		}
		
		_BindTexture(texture);
		if(glGetError())
		{
			SDL_FreeSurface(initial);
			SDL_FreeSurface(intermediary);
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: glGetError() true after glBindTexture() in CacheGlyphs()!" << endl;
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
			SDL_FreeSurface(initial);
			SDL_FreeSurface(intermediary);
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: glGetError() true after glTexImage2D() in CacheGlyphs()!" << endl;
			return false;
		}

		int minx, maxx;
		int miny, maxy;
		int advance;

		if(TTF_GlyphMetrics(font, newglyphs[glyphindex], &minx, &maxx, &miny, &maxy, &advance))
		{
			SDL_FreeSurface(initial);
			SDL_FreeSurface(intermediary);
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: glGetError() true after glTexImage2D() in CacheGlyphs()!" << endl;
			return false;
		}

		FontGlyph * glyph = new FontGlyph;
		glyph->texture = texture;
		glyph->min_x = minx;
		glyph->min_y = miny;
		glyph->top_y = fp->ascent - maxy;
		glyph->width = initial->w + 1;
		glyph->height = initial->h + 1;
		glyph->max_x = (float)(((double)initial->w + 1) / ((double)w));
		glyph->max_y = (float)(((double)initial->h + 1) / ((double)h));
		glyph->advance = advance;

		fp->glyph_cache->insert(std::pair<uint16, FontGlyph *>(newglyphs[glyphindex], glyph));

		SDL_FreeSurface(initial);
		SDL_FreeSurface(intermediary);
	}
	return true;
} // GameVideo::CacheGlyphs()

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
		
	if(_font_map.find(_current_font) == _font_map.end())
		return false;
	
	FontProperties * fp = _font_map[_current_font];
	
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);

	CoordSys &cs = _coord_sys;

	_CacheGlyphs(uText, fp);

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

	float xoff = ((_x_align+1) * fontwidth) * .5f * -cs.GetHorizontalDirection();
	float yoff = ((_y_align+1) * fontheight) * .5f * -cs.GetVerticalDirection();

	MoveRelative(xoff, yoff);

	float modulation = _fader.GetFadeModulation();
	Color textColor = _current_text_color * modulation;

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
	
	if(_fog_intensity > 0.0f)
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
			
			Color oldTextColor = _current_text_color;

			// if text shadows are enabled, draw the shadow
			if(_text_shadow && fp->shadow_style != VIDEO_TEXT_SHADOW_NONE)
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
// RenderText: Renders the given unicode string.
//-----------------------------------------------------------------------------
RenderedString *GameVideo::RenderText(const ustring &txt)
{
	if(txt.empty())
	{
		// Previously, if an empty string was passed, it was considered an error
		// However, it happens often enough in practice that now we just return true
		// without doing anything.
		
		return NULL;
	}

	if(_font_map.find(_current_font) == _font_map.end())
	{
		if(VIDEO_DEBUG)
			cerr << "GameVideo::DrawText() failed because font passed was either not loaded or improperly loaded!\n" <<
				"  *fontname: " << _current_font << endl;
		return NULL;
	}

	FontProperties * fp = _font_map[_current_font];
	TTF_Font * font = fp->ttf_font;
	
	if(!font)
	{
		return NULL;
	}

	uint16 newline('\n');
	int32 lineSkip = fp->line_skip;
	std::vector<uint16 *> lineArray;

	_CacheGlyphs(txt.c_str(), fp);

	const uint16 *charIter;
	// Set lineStart value to one additional to total height of line
	// a reasonable maximum value.
	uint16 *reformattedText = new uint16[txt.size() + 1];
	uint16 *reformIter = reformattedText;
	uint16 *lastLine = reformattedText;
	for (charIter = txt.c_str(); *charIter; ++charIter)
	{
		if (*charIter == newline)
		{
			*reformIter++ = '\0';
			lineArray.push_back(lastLine);
			lastLine = reformIter;
		}
		else
		{
			*reformIter++ = *charIter;
		}
	}
	lineArray.push_back(lastLine);
	*reformIter = '\0';

	std::vector<uint16 *>::iterator lineIter;


	Color oldColor = _current_text_color;
	int32 shadowOffsetX = 0;
	int32 shadowOffsetY = 0;
	// if text shadows are enabled, draw the shadow
	if(_text_shadow && fp->shadow_style != VIDEO_TEXT_SHADOW_NONE)
	{
		shadowOffsetX = static_cast<int32>(_coord_sys.GetHorizontalDirection()) * fp->shadow_x;
		shadowOffsetY = static_cast<int32>(_coord_sys.GetVerticalDirection()) * fp->shadow_y;
	}

	RenderedString *rendStr = new RenderedString(lineSkip, shadowOffsetX, shadowOffsetY);

	for (lineIter = lineArray.begin(); lineIter != lineArray.end(); ++lineIter)
	{
		RenderedLine *line = NULL;
		if ((line = _GenTexLine(*lineIter, fp)) == NULL)
		{
			if(VIDEO_DEBUG)
				cerr << "Failed to generate line texture for " << MakeStandardString(ustring(*lineIter)) << "." << endl;
			delete rendStr;
			return NULL;
		}
		rendStr->Add(line);
	}
	delete[] reformattedText;
		
	return rendStr;
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
	_text_shadow = enable;
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

//-----------------------------------------------------------------------------
// _GetTextShadowColor: gets the current text shadow color
//-----------------------------------------------------------------------------
Color GameVideo::_GetTextShadowColor(FontProperties *fp)
{
	Color shadowColor;
	if(_text_shadow && fp->shadow_style != VIDEO_TEXT_SHADOW_NONE)
	{
		switch(fp->shadow_style)
		{
			case VIDEO_TEXT_SHADOW_DARK:
				shadowColor = Color::black;
				shadowColor[3] = _current_text_color[3] * 0.5f;
				break;
			case VIDEO_TEXT_SHADOW_LIGHT:
				shadowColor = Color::white;
				shadowColor[3] = _current_text_color[3] * 0.5f;
				break;
			case VIDEO_TEXT_SHADOW_BLACK:
				shadowColor = Color::black;
				shadowColor[3] = _current_text_color[3];
				break;
			case VIDEO_TEXT_SHADOW_COLOR:
				shadowColor = _current_text_color;
				shadowColor[3] = _current_text_color[3] * 0.5f;
				break;
			case VIDEO_TEXT_SHADOW_INVCOLOR:
				shadowColor = Color(1.0f - _current_text_color[0], 1.0f - _current_text_color[1], 1.0f - _current_text_color[2], _current_text_color[3] * 0.5f);
				break;
			default:
			{
				if(VIDEO_DEBUG)
					cerr << "VIDEO ERROR: Unknown text shadow style (" << fp->shadow_style << ") -  GameVideo::_GetTextShadowColor()" << endl;
				break;
			}
		}
	}
	return shadowColor;
}

//-----------------------------------------------------------------------------
// ~RenderedLine: deletes the line
//-----------------------------------------------------------------------------

RenderedLine::~RenderedLine()
{
}

//-----------------------------------------------------------------------------
// RenderedLine: constructs a new rendered line
//-----------------------------------------------------------------------------

RenderedLine::RenderedLine(GLuint *tex, int32 lineWidth, int32 texWidth, int32 lineHeight, int32 texHeight, int32 xOffset, int32 yOffset)
	: height(lineHeight), width(lineWidth),
	  x_offset(xOffset),  y_offset(yOffset)
{
	u = ((float)lineWidth  + 1.0f) / (float)texWidth;
	v = ((float)lineHeight + 1.0f) / (float)texHeight;
	if (tex)
		memcpy(texture, tex, sizeof(GLuint) * NUM_TEXTURES);
	else
		memset(texture, 0,   sizeof(GLuint) * NUM_TEXTURES);
}

//-----------------------------------------------------------------------------
// RenderedString::Draw: draws a rendered string
//-----------------------------------------------------------------------------
bool RenderedString::Draw() const
{
	return VideoManager->Draw(*this);
}


//-----------------------------------------------------------------------------
// RenderedLine::Draw: draws a rendered lineline
//-----------------------------------------------------------------------------
bool GameVideo::Draw(const RenderedLine &line, int32 texIndex)
{
	if (texIndex >= RenderedLine::NUM_TEXTURES
	||  texIndex < 0)
		return false;

	// Empty strings are 0 texture ids
	if (!line.texture[texIndex])
		return false;

	if (!_BindTexture(line.texture[texIndex]))
	{
		if ( VIDEO_DEBUG )
		{
			cerr << "Failed to bind texture for line draw." << endl;
		}
		return false;
	}

	float halfW = line.width  * 0.5f;
	float halfH = line.height * 0.5f;

	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, line.v); 
	glVertex2f(- halfW, - halfH);

	glTexCoord2f(line.u, line.v); 
	glVertex2f(+ halfW, - halfH);

	glTexCoord2f(line.u, 0.0f); 
	glVertex2f(+ halfW, + halfH);

	glTexCoord2f(0.0f, 0.0f); 
	glVertex2f(- halfW, + halfH);

	glEnd();

	return true;
}


//-----------------------------------------------------------------------------
// RenderedString::Add: adds a line to a rendered string
//-----------------------------------------------------------------------------
bool RenderedString::Add(RenderedLine *str)
{
	lines.push_back(str);
	if (str->width > _width)
		_width = str->width;
	return true;
}

//-----------------------------------------------------------------------------
// RenderedString: constructs a new empty rendered string
//-----------------------------------------------------------------------------
RenderedString::RenderedString(int32 line_skip, int32 shadowX, int32 shadowY)
	: _width(0), _line_skip(line_skip),
	  _shadow_xoff(shadowX), _shadow_yoff(shadowY)
	{}

//-----------------------------------------------------------------------------
// ~RenderedString: deletes all contained lines
//-----------------------------------------------------------------------------
RenderedString::~RenderedString()
{
	std::vector<RenderedLine *>::iterator line;
	for (line = lines.begin(); line != lines.end(); ++line)
	{
		VideoManager->_DeleteTexture((*line)->texture[RenderedLine::MAIN_TEXTURE]);
		VideoManager->_DeleteTexture((*line)->texture[RenderedLine::SHADOW_TEXTURE]);
		delete *line;
	}
}

//-----------------------------------------------------------------------------
// Draw: draws a rendered string
//-----------------------------------------------------------------------------
bool GameVideo::Draw(const RenderedString &string)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_FOG);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);

	float xoff = ((_x_align) * string.GetWidth()) * .5f * -_coord_sys.GetHorizontalDirection();

	std::vector<RenderedLine*>::const_iterator it;

	glPushMatrix();
	MoveRelative(xoff, 0.0f);
	for (it = string.lines.begin(); it != string.lines.end(); ++it)
	{
		const RenderedLine &line = *(*it);
		if (line.texture[RenderedLine::SHADOW_TEXTURE])
		{
			glPushMatrix();
			MoveRelative(string.GetShadowX(), string.GetShadowY());
			Draw(line, RenderedLine::SHADOW_TEXTURE);
			glPopMatrix();
		}
		Draw(line, RenderedLine::MAIN_TEXTURE);
		MoveRelative(0.0f, -_coord_sys.GetVerticalDirection() * string.GetLineSkip());
	}
	glPopMatrix();
	return true;
}

}  // namespace hoa_video

