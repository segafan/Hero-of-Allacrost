///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2006 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software 
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    text.h
*** \author  Raj Sharma, roos@allacrost.org
*** \brief   Header file for text rendering
***
*** This code makes use of the SDL_ttf font library for representing fonts,
*** font glyphs, and text.
*** ***************************************************************************/

#ifndef __TEXT_HEADER__
#define __TEXT_HEADER__

#include "utils.h"

typedef struct _TTF_Font TTF_Font;

namespace hoa_video {

//! \brief Styles for setting the type of text shadows.
enum TextShadowStyle {
	//! \brief No text shadow, even if shadows are enabled
	VIDEO_TEXT_SHADOW_NONE = 0,
	//! \brief Shadowed area is darkened (default)
	VIDEO_TEXT_SHADOW_DARK = 1,
	//! \brief Shadowed area is lightened
	VIDEO_TEXT_SHADOW_LIGHT = 2,
	//! \brief Shadowed area is completely black
	VIDEO_TEXT_SHADOW_BLACK = 3,
	//! \brief Shadowed area is the color of the text, but less alpha
	VIDEO_TEXT_SHADOW_COLOR = 4,
	//! \brief Shadowed area is the inverse of the text's color (e.g. white text, black shadow)
	VIDEO_TEXT_SHADOW_INVCOLOR = 5
};

/** ****************************************************************************
*** \brief A structure to hold properties about a particular font glyph
*** ***************************************************************************/
class FontGlyph {
public:
	//! \brief The index of the GL texture for this glyph.
	GLuint texture;
	//! \brief The width of the glyph in pixels.
	int32 width;
	//! \brief The height of the glyph in pixels.
	int32 height;
	//! \brief The minx of the glyph in pixels (see TTF_GlyphMetrics).
	int min_x;
	//! \brief The miny of the glyph in pixels (see TTF_GlyphMetrics).
	int min_y;
	//! \brief The maxx of the glyph in texture space (see TTF_GlyphMetrics).
	float max_x;
	//! \brief The maxy of the glyph in texture space (see TTF_GlyphMetrics).
	float max_y;
	//! \brief The amount of space between glyphs.
	int32 advance;
}; // class FontGlyph


/** ****************************************************************************
*** \brief A structure which holds properties about fonts
*** ***************************************************************************/
class FontProperties {
public:
	//! \brief The maximum height of all of the glyphs for this font.
	int32 height;
	//! \brief SDL_ttf's recommended amount of spacing between lines.
	int32 line_skip;
	//! \brief The height above baseline of font
	int32 ascent;
	//! \brief The height below baseline of font
	int32 descent;
	//! \brief The x offset of the text shadow.
	int32 shadow_x;
	//! \brief The y offset of the text shadow.
	int32 shadow_y;
	//! \brief The style of the text shadow.
	TextShadowStyle shadow_style;
	//! \brief A pointer to SDL_TTF's font structure.
	TTF_Font* ttf_font;
	//! \brief A pointer to a cache which holds all of the glyphs used in this font.
	std::map<uint16, FontGlyph*>* glyph_cache;
}; // class FontProperties

}  // namespace hoa_video


#endif
