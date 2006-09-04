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
	
	VIDEO_TEXT_SHADOW_NONE,       //! no text shadow, even if shadows are enabled
	VIDEO_TEXT_SHADOW_DARK,       //! shadowed area is darkened (default)
	VIDEO_TEXT_SHADOW_LIGHT,      //! shadowed area is lightened
	VIDEO_TEXT_SHADOW_BLACK,      //! shadowed area is completely black
	VIDEO_TEXT_SHADOW_COLOR,      //! shadowed area is the color of the text, but less alpha
	VIDEO_TEXT_SHADOW_INVCOLOR,   //! shadowed area is the inverse of the text's color (e.g. white text, black shadow)
	
	VIDEO_TEXT_SHADOW_TOTAL
	
};

/*!***************************************************************************
 *  \brief this structure holds properties about a particular font glyph
 *****************************************************************************/


class FontGlyph
{
public:	
	GLuint texture;		//! index of the GL texture for this glyph
	
	int32 width;		//! width of the glyph in pixels
	int32 height;		//! height of the glyph in pixels

	float minx;			//! minx of the glyph in pixels (see TTF_GlyphMetrics)
	float miny;			//! miny of the glyph in pixels (see TTF_GlyphMetrics)

	float tx;			//! maxx of the glyph in texture space (see TTF_GlyphMetrics)
	float ty;			//! maxy of the glyph in texture space (see TTF_GlyphMetrics)

	int advance;
};

/*!***************************************************************************
 *  \brief this structure holds properties about fonts
 *****************************************************************************/

class FontProperties
{
public:
	int32 height;   //! maximum height of all glyphs
	int32 lineskip; //! SDL_ttf's recommended amount of spacing between lines
	int32 ascent;   //! height above baseline of font
	int32 descent;  //! height below baseline of font
	int32 shadowX;  //! x offset of text shadow
	int32 shadowY;  //! y offset of text shadow
	
	TextShadowStyle shadowStyle;  //! style of text shadow

	TTF_Font * ttf_font; //! pointer fot STL's TTF Font structure

	std::map<uint16, FontGlyph *> * glyphcache; //! cache of all glyphs used in this font
};

}  // namespace hoa_video


#endif
