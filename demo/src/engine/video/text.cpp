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
	static const uint16 fallbackglyph = '?'; // fall back to this glyph if one does not exist

	TTF_Font * font = fp->ttf_font;
	
	SDL_Surface *initial = NULL;
	SDL_Surface *intermediary = NULL;
	int32 w,h;
	GLuint texture;

	for(const uint16 * character_ptr = uText; *character_ptr != 0; ++character_ptr)
	{
		// Reference for legibility
		const uint16 &character = *character_ptr;

		// Check if glyph already cached
		if(fp->glyph_cache->find(character) != fp->glyph_cache->end())
			continue;

		initial = TTF_RenderGlyph_Blended(font, character, color);
		
		if(!initial)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: TTF_RenderUNICODE_Blended() returned NULL in CacheGlyphs(), resorting to fallback" << endl;
			initial = TTF_RenderGlyph_Blended(font, fallbackglyph, color);
			if (!fallbackglyph)
			{
				if (VIDEO_DEBUG)
					cerr << "VIDEO ERROR: TTF_RenderUNICODE_Blended() could not render fallback glyph, aborting!" << endl;
				return false;
			}
			// TEMP
			//cerr << "VIDEO ERROR (Probably a problem from SDL_ttf): " << TTF_GetError() << endl;
			//return false;
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

		if(TTF_GlyphMetrics(font, character, &minx, &maxx, &miny, &maxy, &advance))
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

		fp->glyph_cache->insert(std::pair<uint16, FontGlyph *>(character, glyph));

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
	
	GLfloat tex_coords[8];
	GLint vertices[8];
	
	glEnableClientState ( GL_VERTEX_ARRAY );
	glEnableClientState ( GL_TEXTURE_COORD_ARRAY );
	
	glVertexPointer ( 2, GL_INT, 0, vertices );
	glTexCoordPointer ( 2, GL_FLOAT, 0, tex_coords );

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
		
		vertices[0] = minx;
		vertices[1] = miny;
		vertices[2] = minx + xhi;
		vertices[3] = miny;
		vertices[4] = minx + xhi;
		vertices[5] = miny + yhi;
		vertices[6] = minx;
		vertices[7] = miny + yhi;
		tex_coords[0] = 0.0f;
		tex_coords[1] = ty;
		tex_coords[2] = tx;
		tex_coords[3] = ty;
		tex_coords[4] = tx;
		tex_coords[5] = 0.0f;
		tex_coords[6] = 0.0f;
		tex_coords[7] = 0.0f;
		
		glColor4fv((GLfloat*)&textColor);
		glDrawArrays(GL_QUADS, 0, 4);
		/*glBegin(GL_QUADS);
		glColor4fv((GLfloat *)&textColor);

		glTexCoord2f(0.0, ty); 
		glVertex2i(minx, miny);

		glTexCoord2f(tx, ty); 
		glVertex2i(minx + xhi, miny);

		glTexCoord2f(tx, 0.0); 
		glVertex2i(minx + xhi, miny + yhi);

		glTexCoord2f(0.0, 0.0); 
		glVertex2i(minx, miny + yhi);

		glEnd();*/

		xpos += glyphinfo->advance;
	}

	glDisableClientState ( GL_VERTEX_ARRAY );
	glDisableClientState ( GL_TEXTURE_COORD_ARRAY );

	glPopMatrix();
	
	if(_fog_intensity > 0.0f)
		glEnable(GL_FOG);

	glDisable(GL_ALPHA_TEST);

	//glFinish();

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
		PushState();
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
					PopState();
					return false;
				}
				glPopMatrix();
			}

			SetTextColor(oldTextColor);

			// draw the text itself
			if(!_DrawTextHelper(buffer))
			{
				PopState();
				return false;
			}
			
			glPopMatrix();
			
			MoveRelative(0, -lineSkip * _coord_sys.GetVerticalDirection());

		} while(lastline < txt.length());
		
		PopState();
	}
		
	return true;
}


//-----------------------------------------------------------------------------
// _RenderText: Renders a given unicode string and TextStyle to a pixel array
//-----------------------------------------------------------------------------

bool GameVideo::_RenderText(hoa_utils::ustring &string, TextStyle &style, ImageLoadInfo &buffer)
{
	FontProperties * fp = _font_map[style.font];
	TTF_Font * font     = fp->ttf_font;

	if (!font)
	{
		if (VIDEO_DEBUG)
			cerr << "GameVideo::_RenderText(): font '" << style.font << "' not valid." << endl;
		return false;
	}

	ImageLoadInfo load_info;

	static const SDL_Color color = { 0xFF, 0xFF, 0xFF, 0xFF };

	SDL_Surface *initial      = NULL;
	SDL_Surface *intermediary = NULL;

	int32 line_w, line_h;

	// Minimum Y value of the line
	int32 min_y = 0;
	// Calculated line width
	int32 calc_line_width = 0;
	// Pixels left of '0' the first character extends, if any
	int32 line_start_x = 0;

	const uint16 *char_ptr;

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

	if (TTF_SizeUNICODE(font, string.c_str(), &line_w, &line_h) == -1)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: TTF_SizeUNICODE() returned NULL in _RenderText()!" << endl;
		return false;
	}

	_CacheGlyphs(string.c_str(), fp);

	for (char_ptr = string.c_str(); *char_ptr; ++char_ptr)
	{
		FontGlyph * glyphinfo = (*fp->glyph_cache)[*char_ptr];

		if (glyphinfo->top_y < min_y)
			min_y = glyphinfo->top_y;
		calc_line_width += glyphinfo->advance;
	}

	// Minimum y off by one pixel in some cases.
	min_y -= 1;

	// First character, check if it starts left of 0
	char_ptr = string.c_str();
	if (*char_ptr)
	{
		FontGlyph * first_glyphinfo = (*fp->glyph_cache)[*char_ptr];
		if (first_glyphinfo->min_x < 0)
			line_start_x = first_glyphinfo->min_x;
	}

	// TTF_SizeUNICODE underestimates line width as a 
	// result of its micro positioning
	if (calc_line_width > line_w)
		line_w = calc_line_width;

	// Adjust line sizes by negative starting offsets if present
	line_w -= line_start_x;
	line_h -= min_y;

	intermediary = SDL_CreateRGBSurface(0, line_w, line_h, 32, 
			rmask, gmask, bmask, amask);

	if (!intermediary)
	{
		if (VIDEO_DEBUG)
			cerr << "VIDEO ERROR: SDL_CreateRGBSurface() returned NULL in _RenderText()!" << endl;
		return false;
	}

	SDL_Rect surf_target;
	int32 xpos = -line_start_x;
	int32 ypos = -min_y;
	for (char_ptr = string.c_str(); *char_ptr; ++char_ptr)
	{
		FontGlyph * glyphinfo = (*fp->glyph_cache)[*char_ptr];

		initial = TTF_RenderGlyph_Blended(font, *char_ptr, color);

		if(!initial)
		{
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: TTF_RenderGlyph_Blended() returned NULL in _RenderText()!" << endl;
			return false;
		}

		surf_target.x = xpos + glyphinfo->min_x;
		surf_target.y = ypos + glyphinfo->top_y;

		if (SDL_BlitSurface(initial, NULL, intermediary, &surf_target) < 0)
		{
			SDL_FreeSurface(initial);
			SDL_FreeSurface(intermediary);
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: SDL_BlitSurface() failed in _RenderText()! (" << SDL_GetError() << ")" << endl;
			return false;
		}
		SDL_FreeSurface(initial);
		xpos += glyphinfo->advance;
	}

	SDL_LockSurface(intermediary);
	for (int j = 0; j < intermediary->w * intermediary->h; ++ j)
	{
		((uint8*)intermediary->pixels)[j*4+3] = ((uint8*)intermediary->pixels)[j*4+2];
		((uint8*)intermediary->pixels)[j*4+0] = (uint8) (style.color[0] * 0xFF);
		((uint8*)intermediary->pixels)[j*4+1] = (uint8) (style.color[1] * 0xFF);
		((uint8*)intermediary->pixels)[j*4+2] = (uint8) (style.color[2] * 0xFF);
	}

	load_info.width  = line_w;
	load_info.height = line_h;
	load_info.pixels = intermediary->pixels;

	// Prevent SDL from deleting pixel array
	intermediary->pixels = NULL;

	buffer = load_info;

	SDL_UnlockSurface(intermediary);
	SDL_FreeSurface(intermediary);

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


// *****************************************************************************
// ********************************* RenderedText *********************************
// *****************************************************************************

//-----------------------------------------------------------------------------
// RenderedText::DefaultConstructor: Constructs empty RenderedText
//-----------------------------------------------------------------------------

RenderedText::RenderedText()
: _alignment (ALIGN_CENTER) {
	Clear();
	_animated = false;
	_grayscale = false;
}

//-----------------------------------------------------------------------------
// RenderedText::ustring-Constructor: Constructs and renders RenderedText with String
//-----------------------------------------------------------------------------

RenderedText::RenderedText(const ustring &string, int8 alignment)
: _alignment (ALIGN_CENTER) {
	Clear();
	_animated = false;
	_grayscale = false;
	_string = string;
	SetAlignment(alignment);
	_Regenerate();
}

//-----------------------------------------------------------------------------
// RenderedText::sstring-Constructor: Constructs and renders RenderedText with String
//-----------------------------------------------------------------------------

RenderedText::RenderedText(const std::string &string, int8 alignment)
: _alignment (ALIGN_CENTER) {
	Clear();
	_animated = false;
	_grayscale = false;
	_string = MakeUnicodeString(string);
	SetAlignment(alignment);
	_Regenerate();
}

//-----------------------------------------------------------------------------
// RenderedText::CopyConstructor: Copies state variables and increases ref counts.
//-----------------------------------------------------------------------------

RenderedText::RenderedText(const RenderedText &other) 
{
	// Chain to overriden copy constructor
	*this = other;
}

//-----------------------------------------------------------------------------
// RenderedText::operator=: Copies state variables and increases ref counts.
//-----------------------------------------------------------------------------

void RenderedText::operator=(const RenderedText &other)
{
	// Copy values from other RenderedText manually
	_string        = other._string;
	_text_sections = other._text_sections;
	_alignment     = other._alignment;
	_width         = other._width;
	_height        = other._height;

	memcpy(_color, other._color, sizeof(_color));

	// Increase ref count for all text sections
	// as this RenderedText now also points to them
	std::vector<TImageElement>::iterator it;
	for (it = _text_sections.begin(); it != _text_sections.end(); ++it)
	{
		if (it->image)
			it->image->Add();
	}
}

//-----------------------------------------------------------------------------
// ~RenderedText: Decrements ref counts on contained timages
//-----------------------------------------------------------------------------

RenderedText::~RenderedText()
{
	_ClearImages();
}

//-----------------------------------------------------------------------------
// Clear: Decrements ref counts on contained timages, clears contained text
//-----------------------------------------------------------------------------

void RenderedText::Clear() {
	_Clear();
	_string.clear();
	_ClearImages();

	SetColor(Color::white);
}

//-----------------------------------------------------------------------------
// GetElement: Gets the array referenced generic BaseImageElement for drawing
//-----------------------------------------------------------------------------

const private_video::BaseImageElement *RenderedText::GetElement(uint32 index) const
{
	if (index >= GetNumElements())
		return NULL;
	return &_text_sections[index];
}

//-----------------------------------------------------------------------------
// GetNumElements: Gets the number of drawable elements contained
//-----------------------------------------------------------------------------

uint32 RenderedText::GetNumElements() const
{
	return _text_sections.size();
}

//-----------------------------------------------------------------------------
// SetAlignment: Sets the horizontal text alignment of a RenderedText
//-----------------------------------------------------------------------------

bool RenderedText::SetAlignment(int8 alignment)
{
	switch (alignment)
	{
		case ALIGN_LEFT:
		case ALIGN_CENTER:
		case ALIGN_RIGHT:
			if (_alignment != alignment)
			{
				_alignment = alignment;
				_Realign();
			}
			return true;
			break;
		default:
			if (VIDEO_DEBUG)
				cerr << "VIDEO: " << __FUNCTION__ << ": alignment wasn't a valid constant." << endl;
			return false;
	}
}

//-----------------------------------------------------------------------------
// SetText: Sets the contained text in a RenderedText and re-renders
//-----------------------------------------------------------------------------

void RenderedText::SetText(const ustring &string)
{
	_string = string;
	_Regenerate();
}

//-----------------------------------------------------------------------------
// SetText: std::string edition
//-----------------------------------------------------------------------------

void RenderedText::SetText(const std::string &string) {
	SetText(MakeUnicodeString(string));
}

//-----------------------------------------------------------------------------
// _ClearImages: Decrements reference counts on all contained timages
//-----------------------------------------------------------------------------

void RenderedText::_ClearImages()
{
	std::vector<TImageElement>::iterator it;
	for (it = _text_sections.begin(); it != _text_sections.end(); ++it)
	{
		if (it->image)
			VideoManager->_DeleteImage(it->image);
	}
	_text_sections.clear();
	
	_width  = 0;
	_height = 0;
}

//-----------------------------------------------------------------------------
// _Regenerate: (Re)Renders and aligns contained text
//-----------------------------------------------------------------------------

void RenderedText::_Regenerate() {
	_ClearImages();

	if(_string.empty())
	{
		return;
	}

	TextStyle style;
	style.font          = VideoManager->GetFont();
	style.shadow_enable = VideoManager->_text_shadow;
	FontProperties *fp;
	if (!VideoManager->IsValidFont(style.font)
	|| ((fp = VideoManager->GetFontProperties(style.font)) == NULL))
	{
		if(VIDEO_DEBUG)
			cerr << "RenderedText::_Regenerate(): Video engine contains invalid font." << endl;
		return;
	}

	style.color = VideoManager->GetTextColor();

	uint16 newline = '\n';
	std::vector<uint16 *> line_array;

	VideoManager->_CacheGlyphs(_string.c_str(), fp);

	const uint16 *char_iter;
	uint16 *reformatted_text = new uint16[_string.size() + 1];
	uint16 *reform_iter = reformatted_text;
	uint16 *last_line = reformatted_text;
	for (char_iter = _string.c_str(); *char_iter; ++char_iter)
	{
		if (*char_iter == newline)
		{
			*reform_iter++ = '\0';
			line_array.push_back(last_line);
			last_line = reform_iter;
		}
		else
		{
			*reform_iter++ = *char_iter;
		}
	}
	line_array.push_back(last_line);
	*reform_iter = '\0';

	std::vector<uint16 *>::iterator line_iter;

	Color old_color       = style.color;
	int32 shadow_offset_x = 0;
	int32 shadow_offset_y = 0;
	Color shadow_color    = VideoManager->_GetTextShadowColor(fp);

	float total_height = static_cast<float>( (line_array.size() - 1) * fp->line_skip );

	for (line_iter = line_array.begin(); line_iter != line_array.end(); ++line_iter)
	{
		TImage *timage = new TImage(*line_iter, style);
		if (!timage->Regenerate())
		{
			if (VIDEO_DEBUG)
			{
				cerr << "RenderedText::_Regenerate(): Failed to render TImage." << endl;
			}
		}

		// Increment the reference count
		timage->Add();
		float y_offset = total_height + _height * -VideoManager->_coord_sys.GetVerticalDirection();
		y_offset += (fp->line_skip - timage->height) * VideoManager->_coord_sys.GetVerticalDirection();
		TImageElement element(timage, 0, y_offset, 0.0f, 0.0f, 1.0f, 1.0f, static_cast<float>(timage->width), static_cast<float>(timage->height), _color);

		// if text shadows are enabled, add a shadow version
		if (style.shadow_enable && timage->style.shadow_style != VIDEO_TEXT_SHADOW_NONE)
		{
			shadow_offset_x = static_cast<int32>(VideoManager->_coord_sys.GetHorizontalDirection()) * timage->style.shadow_offset_x;
			shadow_offset_y = static_cast<int32>(VideoManager->_coord_sys.GetVerticalDirection())   * timage->style.shadow_offset_y;

			TImageElement shadow_element = element;
			shadow_element.x_offset += shadow_offset_x;
			shadow_element.y_offset += shadow_offset_y;


			// Line offsets must be set to be retained
			// after lines are aligned
			shadow_element.x_line_offset = static_cast<float>(shadow_offset_x);
			shadow_element.y_line_offset = static_cast<float>(shadow_offset_y);

			shadow_element.color[0] = shadow_color * _color[0];
			shadow_element.color[1] = shadow_color * _color[1];
			shadow_element.color[2] = shadow_color * _color[2];
			shadow_element.color[3] = shadow_color * _color[3];

			_text_sections.push_back(shadow_element);

			// Increment reference count to reflect use in shadow texture
			timage->Add();
		}

		// Add the timage to the video engine set
		VideoManager->_AddTImage(timage);

		// And to our internal vector
		_text_sections.push_back(element);

		// Set width to timage width, if wider
		if (timage->width > _width)
			_width = static_cast<float>(timage->width);

		// Increase height by the font specified line height
		_height += fp->line_skip;
	}
	delete[] reformatted_text;
	_Realign();
}

//-----------------------------------------------------------------------------
// _Realign: (Re)Aligns all text items to set alignment
//-----------------------------------------------------------------------------

void RenderedText::_Realign()
{
	std::vector<TImageElement>::iterator it;
	for (it = _text_sections.begin(); it != _text_sections.end(); ++it)
	{
		it->x_offset = _alignment * VideoManager->_coord_sys.GetHorizontalDirection() * ( (_width - it->width) / 2.0f) + it->x_line_offset;
	}
}

// *****************************************************************************
// ********************************** TImage ***********************************
// *****************************************************************************

TImage::TImage(const hoa_utils::ustring &string_, const TextStyle &style_)
:	string(string_),
	style(style_)
{
	width = 0;
	height = 0;
	grayscale = false;
	texture_sheet = NULL;
	x = 0;
	y = 0;
	u1 = 0.0f;
	v1 = 0.0f;
	u2 = 1.0f;
	v2 = 1.0f;
	ref_count = 0;
	// Use image smoothing
	smooth = true;

	LoadFontProperties();
}



TImage::TImage(TexSheet *sheet, const hoa_utils::ustring &string_, const TextStyle &style_, int32 x_, int32 y_, float u1_, float v1_,
		float u2_, float v2_, int32 width, int32 height, bool grayscale_)
:	string(string_),
	style(style_)
{
	texture_sheet = sheet;
	x = x_;
	y = y_;
	u1 = u1_;
	v1 = v1_;
	u2 = u2_;
	v2 = v2_;
	width = width;
	height = height;
	grayscale = grayscale_;
	ref_count = 0;
	// Use image smoothing
	smooth = true;

	LoadFontProperties();
}


bool TImage::LoadFontProperties()
{
	if (style.shadow_style == VIDEO_TEXT_SHADOW_INVALID)
	{
		FontProperties *fp = VideoManager->GetFontProperties(style.font);
		if (!fp)
		{
			if (VIDEO_DEBUG)
				cerr << "TImage::LoadFontProperties(): Invalid font '" << style.font << "'." << endl;
			return false;
		}
		style.shadow_style    = fp->shadow_style;
		style.shadow_offset_x = fp->shadow_x;
		style.shadow_offset_y = fp->shadow_y;
	}
	return true;
}

bool TImage::Regenerate()
{
	if (texture_sheet)
	{
		VideoManager->_DeleteImage(this);
		texture_sheet = NULL;
	}

	ImageLoadInfo buffer;

	if (!VideoManager->_RenderText(string, style, buffer))
		return false;
	
	width  = buffer.width;
	height = buffer.height;

	TexSheet *sheet = VideoManager->_InsertImageInTexSheet(this, buffer, true);
	if(!sheet)
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO_DEBUG: TImage::Regenerate(): GameVideo::_InsertImageInTexSheet() returned NULL!" << endl;

		free(buffer.pixels);
		return false;
	}

	texture_sheet = sheet;

	free(buffer.pixels);
	return true;
}

bool TImage::Reload()
{
	// Check if indeed already loaded. If not - create texture sheet entry.
	if (!texture_sheet)
		return Regenerate();

	ImageLoadInfo buffer;

	if (!VideoManager->_RenderText(string, style, buffer))
		return false;
	
	if (!texture_sheet->CopyRect(x, y, buffer))
	{
		if(VIDEO_DEBUG)
			cerr << "VIDEO ERROR: sheet->CopyRect() failed in TImage::Reload()!" << endl;
		free(buffer.pixels);
		return false;
	}
	return true;
}


TImage *GameVideo::_GetTImage(TImage *pntr)
{
	if (_t_images.find(pntr) == _t_images.end())
		return NULL;
	return pntr;
}


// *****************************************************************************
// ****************************** TImageElement *********************************
// *****************************************************************************

TImageElement::TImageElement(TImage *image_, float x_offset_, float y_offset_, float u1_, float v1_,
	float u2_, float v2_, float width_, float height_) :
	image(image_)
{
	x_offset = x_offset_;
	y_offset = y_offset_;
	u1 = u1_;
	v1 = v1_;
	u2 = u2_;
	v2 = v2_;
	width = width_;
	height = height_;
	white = true;
	one_color = true;
	blend = false;
	color[0] = Color::white;
}



TImageElement::TImageElement(TImage *image_, float x_offset_, float y_offset_, float u1_, float v1_, 
		float u2_, float v2_, float width_, float height_, Color color_[4]) :
	image(image_)
{
	x_offset = x_offset_;
	y_offset = y_offset_;
	u1 = u1_;
	v1 = v1_;
	u2 = u2_;
	v2 = v2_;
	width = width_;
	height = height_;
	color[0] = color_[0];

	// If all colors are the same, then mark it so we don't have to process all vertex colors
	if (color_[1] == color[0] && color_[2] == color[0] && color_[3] == color[0]) {
		one_color = true;

		// If all vertex colors are white, set a flag so they don't have to be processed at all
		if (color[0] == Color::white) {
			white = true;
			blend = false;
		}
		// Set blend to true if alpha < 1.0f
		else {
			blend = (color[0][3] < 1.0f);
		}
	}
	else {
		color[0] = color_[0];
		color[1] = color_[1];
		color[2] = color_[2];
		color[3] = color_[3];
		// Set blend to true if any of the four colors have an alpha value < 1.0f
		blend = (color[0][3] < 1.0f || color[1][3] < 1.0f || color[2][3] < 1.0f || color[3][3] < 1.0f);
	}
} // TImageElement::TImageElement()

BaseImage *TImageElement::GetBaseImage()
{
	return image;
}

const BaseImage *TImageElement::GetBaseImage() const
{
	return image;
}

}  // namespace hoa_video

