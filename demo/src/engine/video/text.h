///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef __TEXT_HEADER__
#define __TEXT_HEADER__


#include "utils.h"

typedef struct _TTF_Font TTF_Font;


namespace hoa_video
{

/*!***************************************************************************
 *  \brief Styles for text shadows
 *****************************************************************************/

enum TextShadowStyle
{
	VIDEO_TEXT_SHADOW_INVALID = -1,
	
	//! no text shadow, even if shadows are enabled
	VIDEO_TEXT_SHADOW_NONE = 0,
	
	//! shadowed area is darkened (default)
	VIDEO_TEXT_SHADOW_DARK = 1,
	
	//! shadowed area is lightened
	VIDEO_TEXT_SHADOW_LIGHT = 2,
	
	//! shadowed area is completely black
	VIDEO_TEXT_SHADOW_BLACK = 3,
	
	//! shadowed area is the color of the text, but less alpha
	VIDEO_TEXT_SHADOW_COLOR = 4,
	
	//! shadowed area is the inverse of the text's color (e.g. white text, black shadow)
	VIDEO_TEXT_SHADOW_INVCOLOR = 5,
	
	VIDEO_TEXT_SHADOW_TOTAL = 6
	
};

/*!***************************************************************************
 *  \brief this structure holds properties about a particular font glyph
 *****************************************************************************/


class FontGlyph
{
public:	

	//! index of the GL texture for this glyph
	GLuint texture;
	
	//! width of the glyph in pixels
	int32 width;
	
	//! height of the glyph in pixels
	int32 height;

	//! minx of the glyph in pixels (see TTF_GlyphMetrics)
	int minx;
	
	//! miny of the glyph in pixels (see TTF_GlyphMetrics)
	int miny;

	//! maxx of the glyph in texture space (see TTF_GlyphMetrics)
	float tx;
	
	//! maxy of the glyph in texture space (see TTF_GlyphMetrics)
	float ty;

	//! space between glyphs
	int advance;
};


/*!***************************************************************************
 *  \brief this structure holds properties about fonts
 *****************************************************************************/

class FontProperties
{
public:

	//! maximum height of all glyphs
	int32 height;
	
	//! SDL_ttf's recommended amount of spacing between lines
	int32 lineskip;
	
	//! height above baseline of font
	int32 ascent;
	
	//! height below baseline of font
	int32 descent;
	
	//! x offset of text shadow
	int32 shadowX;
	
	//! y offset of text shadow
	int32 shadowY;
	
	//! style of text shadow
	TextShadowStyle shadowStyle;

	//! pointer fot STL's TTF Font structure
	TTF_Font * ttf_font;

	//! cache of all glyphs used in this font
	std::map<uint16, FontGlyph *> * glyphcache;
};

}  // namespace hoa_video


#endif
