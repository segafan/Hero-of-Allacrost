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

#include "defs.h"
#include "utils.h"

typedef struct _TTF_Font TTF_Font;

namespace hoa_video {

//! \brief Styles for setting the type of text shadows.
enum TEXT_SHADOW_STYLE {
	VIDEO_TEXT_SHADOW_INVALID = -1,

	//! \brief No text shadow is drawn, even if shadows are enabled.
	VIDEO_TEXT_SHADOW_NONE = 0,
	//! \brief Shadowed area is darkened (this is the default).
	VIDEO_TEXT_SHADOW_DARK = 1,
	//! \brief Shadowed area is lightened.
	VIDEO_TEXT_SHADOW_LIGHT = 2,
	//! \brief Shadowed area is completely black.
	VIDEO_TEXT_SHADOW_BLACK = 3,
	//! \brief Shadowed area is the same color of the text, but has less alpha.
	VIDEO_TEXT_SHADOW_COLOR = 4,
	//! \brief Shadowed area is the inverse of the text's color (e.g. white text, black shadow).
	VIDEO_TEXT_SHADOW_INVCOLOR = 5,

	VIDEO_TEXT_SHADOW_TOTAL = 6
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
	//! \brief The top y value of the glyph.
	int top_y;
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
	TEXT_SHADOW_STYLE shadow_style;
	//! \brief A pointer to SDL_TTF's font structure.
	TTF_Font* ttf_font;
	//! \brief A pointer to a cache which holds all of the glyphs used in this font.
	std::map<uint16, FontGlyph*>* glyph_cache;
}; // class FontProperties

/** ****************************************************************************
*** \brief A structure holding a single rendered line.
*** ***************************************************************************/
class RenderedLine {
private:
        //! \brief Disabled copy constructor
        RenderedLine(const RenderedLine &other);
        //! \brief Disabled assignment operator
        RenderedLine &operator=(const RenderedLine &other);
public:
	//! \brief Num textures constant enum
	enum
	{
		MAIN_TEXTURE   = 0,
		SHADOW_TEXTURE = 1,
		NUM_TEXTURES   = 2,
	};
	//! \brief The height of the line
	int32 height;
	//! \brief V texture coordinate of the line.
	float v;
	//! \brief The width of the line
	int32 width;
	//! \brief U texture coordinate of the line.
	float u;
	//! \brief The GL texture
	GLuint texture[NUM_TEXTURES];
	//! \brief The X offset to draw on
	int32 x_offset;
	//! \brief The Y offset to draw on
	int32 y_offset;
	//! \brief Create a RenderedLine
	RenderedLine(GLuint *tex, 
		     int32 lineWidth,  int32 texWidth,
		     int32 lineHeight, int32 texHeight, 
		     int32 xOffset,    int32 yOffset);
	//! \brief Deletes textures
	~RenderedLine();
}; // class RenderedLine


/** ****************************************************************************
*** \brief A structure which a rendered string
*** ***************************************************************************/
class RenderedString {
private:
	//! \brief The total width of this text block.
	int32 _width;
	//! \brief SDL_ttf's recommended amount of spacing between lines.
	int32 _line_skip;
	//! \brief X offset of the shadow texture
	int32 _shadow_xoff;
	//! \brief Y offset of the shadow texture
	int32 _shadow_yoff;
public:
	//! \brief Vector of line textures
	std::vector<RenderedLine*> lines;
	//! \brief Constructs empty string.
	RenderedString(int32 line_skip, int32 shadowX = 0, int32 shadowY = 0);
	//! \brief Deletes textures
	~RenderedString();
	//! \brief Draw the string
	bool Draw() const;
	//! \brief Add a line to the string
	bool Add(RenderedLine *line);
	//! \brief Get the current line skip
	int32 GetLineSkip() const { return _line_skip; };
	//! \brief Get the line width
	int32 GetWidth() const { return _width; };
	//! \brief Get the shadow X offset
	int32 GetShadowX() const { return _shadow_xoff; };
	//! \brief Get the shadow X offset
	int32 GetShadowY() const { return _shadow_yoff; };
	//! \brief Set the shadow X offset
	void SetShadowX(int32 xoff) { _shadow_xoff = xoff; };
	//! \brief Set the shadow X offset
	void SetShadowY(int32 yoff) { _shadow_yoff = yoff; };
}; // class RenderedString

}  // namespace hoa_video


#endif
