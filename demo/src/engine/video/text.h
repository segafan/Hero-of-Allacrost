///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
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
*** \brief A class encompassing all options for a text style
*** ***************************************************************************/
class TextStyle {
public:
	//! \brief The string font name
	std::string font;

	//! \brief Whether shadowing is enabled
	bool shadow_enable;

	//! \brief The x offset of the shadow
	int32 shadow_offset_x;

	//! \brief The y offset of the shadow
	int32 shadow_offset_y;

	//! \brief The enum representing the shadow style
	TEXT_SHADOW_STYLE shadow_style;

	//! \brief The text color
	Color		  color;

	TextStyle() :
		shadow_style(VIDEO_TEXT_SHADOW_INVALID) {}
	
};


/** ****************************************************************************
*** A text specific subclass of the BaseImage subclass, contains a
*** string and style needed to render a piece of text.
*** ***************************************************************************/
class TextImage : public private_video::BaseImage {
public:
	//! \brief The string represented
	hoa_utils::ustring string;

	//! \brief The font/color/etc.
	TextStyle          style;

	/** \brief Constructor defaults image as the first one in a texture sheet.
	*** \note The actual sheet where the image is located will be determined later.
	**/
	TextImage(const hoa_utils::ustring &string_, const TextStyle &style_);

	//! \brief Constructor where image coordinates are specified, along with texture coords and the texture sheet.
	TextImage(private_video::TexSheet *sheet, const hoa_utils::ustring &string_, const TextStyle &style_, int32 x_, int32 y_, float u1_, float v1_,
		float u2_, float v2_, int32 width, int32 height, bool grayscale_);

	TextImage & operator=(TextImage& rhs)
		{ return *this; }

	//! Loads the properties of the internal font
	bool LoadFontProperties();

	//! Generate a text texture and add to a texture sheet
	bool Regenerate();

	//! Reload texture to an already assigned texture sheet
	bool Reload();
};

/** ****************************************************************************
*** \brief A text specific subclass of the BaseImageElement subclass.
*** ***************************************************************************/
class TextImageElement : public private_video::BaseImageElement
{
public:
	//! \brief The image that is being referenced by this object.
	TextImage* image;

	//! \brief X offset from the line proper
	float x_line_offset;

	//! \brief Y offset from the line proper
	float y_line_offset;



	/** \brief Constructor specifying a specific image element.
	*** Multiple elements can be stacked to form one compound image
	**/
	TextImageElement(TextImage *image_, float x_offset_, float y_offset_, float u1_, float v1_,
		float u2_, float v2_, float width_, float height_, Color color_[4]);
	
	//! \brief Constructor defaulting the element to have white vertices and disabled blending.
	TextImageElement(TextImage *image_, float x_offset_, float y_offset_, float u1_, float v1_,
		float u2_, float v2_, float width_, float height_);

	/** Helper function to get abstract drawable
	 *  image type.
	 */
	virtual private_video::BaseImage *GetBaseImage()
		{ return image; }

	/** Helper function to get abstract drawable
	 *  image type (const version).
	 */
	virtual const private_video::BaseImage *GetBaseImage() const
		{ return image; }
};


/** ****************************************************************************
*** \brief Represents a rendered text string
*** RenderedText is a compound image containing each line of a text string.
*** ***************************************************************************/
class RenderedText : public ImageListDescriptor {
	friend class GameVideo;
public:
	enum align {
		ALIGN_LEFT   = 0,
		ALIGN_CENTER = 1,
		ALIGN_RIGHT  = 2,
	};

	//! \brief Construct empty text object
	RenderedText();

	//! \brief Constructs rendered string of specified ustring
	RenderedText(const hoa_utils::ustring &string, int8 alignment = ALIGN_CENTER);

	//! \brief Constructs rendered string of specified std::string
	RenderedText(const std::string        &string, int8 alignment = ALIGN_CENTER);

	//! \brief Copy constructor increases contained reference counts
	RenderedText(const RenderedText &other);

	//! \brief Clone operator increases contained reference counts
	RenderedText &operator=(const RenderedText &other);

	//! \brief Destructs RenderedText, lowering reference counts on all contained timages.
	~RenderedText();

	//! \brief Clears the image by resetting its properties
	void Clear();

	//! \name Class Member Set Functions
	//@{
	//! \brief Sets the text contained
	void SetText(const hoa_utils::ustring &string);

	//! \brief Sets the text (std::string version)
	void SetText(const std::string &string);

	/** \brief Sets horizontal text alignment
	 *  \param alignment The alignment - ALIGN_CENTER,
	 *         ALIGN_LEFT or ALIGN_RIGHT.
	 */
	bool SetAlignment(int8 alignment);

	//! \brief Sets width of the image
	virtual void SetWidth(float width)
		{ _width = width; }

	//! \brief Sets height of the image
	virtual void SetHeight(float height)
		{ _height = height; }

	//! \brief Sets the dimensions (width + height) of the image.
	virtual void SetDimensions(float width, float height)
		{ _width  = width; _height = height; }

	//! \brief Sets image to static/animated
	virtual void SetStatic(bool is_static)
		{ _is_static = is_static; }

	//! \brief Sets the color for the image (for all four verteces).
	void SetColor(const Color &color)
		{ _color[0] = _color[1] = _color[2] = _color[3] = color; }

	/** \brief Sets individual vertex colors in the image.
	*** \param tl top left vertex color
	*** \param tr top right vertex color
	*** \param bl bottom left vertex color
	*** \param br bottom right vertex color
	**/
	void SetVertexColors(const Color &tl, const Color &tr, const Color &bl, const Color &br)
		{ _color[0] = tl; _color[1] = tr; _color[2] = bl; _color[3] = br; }
	//@}

	//! \name Class Member Get Functions
	//@{
	//! \brief Returns the filename of the image.
	hoa_utils::ustring GetString() const
		{ return _string; }

	/** \brief Returns the color of a particular vertex
	*** \param c The Color object to place the color in.
	*** \param index The vertex index of the color to fetch
	*** \note If an invalid index value is used, the function will return with no warning.
	**/
	void GetVertexColor(Color &c, uint8 index)
		{ if (index > 3) return; else c = _color[index]; }

	//@}

	//! \brief Virtual method to retrieve a drawable base class element
	virtual const private_video::BaseImageElement *GetElement(uint32 index) const;

	//! \brief Virtual method to retrieve number of drawable base class elements
	virtual uint32 GetNumElements() const;

private:
	/** \brief Clears all rendered text, decreasing ref count.
	**/
	void _ClearImages();

	/** \brief Regenerates the string textures.
	**/
	void _Regenerate();

	/** \brief Realigns text to a horizontal alignment
	**/
	void _Realign();
	
	/** \brief The string this was constructed from (for reloading)
	**/
	hoa_utils::ustring _string;

	/** The TextImage elements representing rendered text
	**  portions, usually lines.
	**/
	std::vector<TextImageElement> _text_sections;

	//! \brief The horizontal text alignment
	int8 _alignment;
}; // class RenderedText : public ImageDescriptor


}  // namespace hoa_video

#endif // __TEXT_HEADER__
