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
*** \author  Lindsay Roberts, linds@allacrost.org
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

//! \brief The singleton pointer for the instance of the text supervisor
extern TextSupervisor* TextManager;

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

	//! \brief The height above and below baseline of font
	int32 ascent, descent;

	//! \brief The x and y offsets of the text shadow.
	int32 shadow_x, shadow_y;

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
	TextStyle() :
		shadow_style(VIDEO_TEXT_SHADOW_INVALID)
	{}

	//! \brief The string font name
	std::string font;

	//! \brief The x offset of the shadow
	int32 shadow_offset_x;

	//! \brief The y offset of the shadow
	int32 shadow_offset_y;

	//! \brief The enum representing the shadow style
	TEXT_SHADOW_STYLE shadow_style;

	//! \brief The text color
	Color color;
};

namespace private_video {

/** ****************************************************************************
*** \brief Represents an image of rendered text stored in a texture sheet
*** A text specific subclass of the BaseImage subclass, contains a
*** string and style needed to render a piece of text.
*** ***************************************************************************/
class TextImageTexture : public private_video::BaseImageTexture {
public:
	/** \brief Constructor defaults image as the first one in a texture sheet.
	*** \note The actual sheet where the image is located will be determined later.
	**/
	TextImageTexture(const hoa_utils::ustring& string_, const TextStyle& style_);

	~TextImageTexture();

	// ---------- Public members

	//! \brief The string represented
	hoa_utils::ustring string;

	//! \brief The font/color/etc.
	TextStyle style;

	// ---------- Public methods

	//! \brief Loads the properties of the internal font
	bool LoadFontProperties();

	//! \brief Generate a text texture and add to a texture sheet
	bool Regenerate();

	//! \brief Reload texture to an already assigned texture sheet
	bool Reload();

private:
	TextImageTexture(const TextImageTexture& copy);
	TextImageTexture& operator=(const TextImageTexture& copy);
}; // class TextImageTexture : public private_video::BaseImage


/** ****************************************************************************
*** \brief A text specific subclass of the BaseImageElement subclass.
*** ***************************************************************************/
class TextImageElement : public private_video::BaseImageElement {
public:
	//! \brief Constructor defaulting the element to have white vertices and disabled blending.
	TextImageElement(TextImageTexture* image_, float x_offset_, float y_offset_, float u1_, float v1_,
		float u2_, float v2_, float width_, float height_);

	TextImageElement(TextImageTexture* image_, float x_offset_, float y_offset_, float u1_, float v1_,
		float u2_, float v2_, float width_, float height_, Color color_[4]);

	// ---------- Public members

	//! \brief The image that is being referenced by this object.
	TextImageTexture* image;

	//! \brief The x offset from the line proper
	float x_line_offset;

	//! \brief The y offset from the line proper
	float y_line_offset;
}; // class TextImageElement : public private_video::BaseImageElement

} // namespace private_video

/** ****************************************************************************
*** \brief Represents a rendered text string
*** RenderedText is a compound image containing each line of a text string.
*** ***************************************************************************/
class RenderedText : public ImageDescriptor {
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

	void Draw() const {}

	void Draw(const Color& draw_color) const {}

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
	bool IsAnimated() const
		{ return false; }
	
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
	std::vector<private_video::TextImageElement> _text_sections;

	//! \brief The horizontal text alignment
	int8 _alignment;
}; // class RenderedText : public ImageDescriptor



class TextSupervisor : public hoa_utils::Singleton<TextSupervisor> {
	friend class GameVideo;
	friend class TextureController;

	friend class private_video::TextImageTexture;
	friend class RenderedText;

public:
	TextSupervisor();

	~TextSupervisor();

	bool SingletonInitialize();

	//! \name Font manipulation methods
	//@{
	/** \brief Loads a font file from disk with a specific size and name
	*** \param font_filename The filename of the font file to load
	*** \param font_name The name which to refer to the font after it is loaded
	*** \param size The point size to set the font after it is loaded
	*** \param style The default shadow style for this font (default value = VIDEO_TEXT_SHADOW_NONE)
	*** \param x_offset The x shadow offset for this font in number of pixels (default value = 0)
	*** \param y_offset The y shadow offset for this font in number of pixels (default value = 0)
	*** \param make_default If set to true, this font will be made the default font if it is loaded successfully (default value = false)
	*** \return True if the font was successfully loaded, or false if there was an error
	***
	*** \note If both the x and y offset arguments are zero, the offset value will instead be set to 1/8th of the font's height
	*** to the right in the x direction, and down in the y direction. This is done because a shadow with a (0, 0) offset is not
	*** valid (the shadow will be completely obscured by the text itself).
	**/
	bool LoadFont(const std::string& filename, const std::string& font_name, uint32 size, TEXT_SHADOW_STYLE style = VIDEO_TEXT_SHADOW_NONE,
		 int32 x_offset = 0, int32 y_offset = 0, bool make_default = false);

	/** \brief Removes a loaded font from memory and frees up associated resources
	*** \param font_name The reference name of the font to unload
	***
	*** If the argument name is invalid (i.e. no font with that reference name exists), this method will do
	*** nothing more than print out a warning message if running in debug mode.
	**/
// 	void FreeFont(const std::string& font_name);

	/** \brief Returns true if a font of a certain reference name exists
	*** \param font_name The reference name of the font to check
	*** \return True if font name is valid, false if it is not.
	**/
	bool IsFontValid(const std::string& font_name)
		{ return (_font_map.find(font_name) != _font_map.end()); }

	/** \brief Gets the name of the current default font used for text rendering
	*** \note If there are no fonts loaded, this method will return an empty string
	**/
	const std::string& GetDefaultFont() const;

	/** \brief Sets the default font to use for text rendering
	*** \param font_name The name of the pre-loaded font to set as the default
	***
	*** If the argument does not have valid font data associated with it, no
	*** change will be made and a warning message will be printed if debug
	*** mode is enabled.
	**/
	void SetDefaultFont(const std::string& font_name);

	/** \brief Sets the default shadow style to use for a specified font
	*** \param font_name The reference name of the font to set the shadow style for
	*** \param style The shadow style desired for this font
	**/
	void SetFontShadowStyle(const std::string& font_name, TEXT_SHADOW_STYLE style);

	/** \brief Sets the default x and y shadow offsets to use for a specified font
	*** \param font_name The reference name of the font to set the shadow offsets for
	*** \param x The x offset in number of pixels
	*** \param y The y offset in number of pixels
	***
	*** By default, all font shadows are slightly to the right and to the bottom of the text,
	*** by an offset of one eight of the font's height.
	**/
	void SetFontShadowOffsets(const std::string& font_name, int32 x, int32 y);

	// NOTE: Make this function private
	/** \brief Get the font properties for a previously loaded font
	*** \param font_name The name reference of the loaded font
	*** \return A pointer to the FontProperties object with the requested data, or NULL if the properties could not be fetched.
	**/
	FontProperties* GetFontProperties(const std::string& font_name);
	//@}

	//! \brief Text manipulation methods
	//@{
	//! \brief Returns the current default color for rendering text
	Color GetDefaultTextColor() const;

	/** \brief Sets the default color to render text in
	*** \param color The color to set the text to
	**/
	void SetDefaultTextColor(const Color& color);

	/** \brief Renders and draws a string of text to the screen
	*** \param text The text string to draw in unicode format
	**/
	void Draw(const hoa_utils::ustring& text);
// 	void Draw(const hoa_utils::ustring& text, std::string font_name);
// 	void Draw(const hoa_utils::ustring& text, Color& color);
// 	void Draw(const hoa_utils::ustring& text, std::string font_name);
// 	void Draw(const hoa_utils::ustring& text, std::string font_name, Color& color);

	/** \brief Renders and draws a string of text to the screen
	*** \param text The text string to draw in standard format
	**/
	void Draw(const std::string& text)
		{ Draw(hoa_utils::MakeUnicodeString(text)); }

	/** \brief Calculates what the width would be of a rendered string of text
	*** \param font_name The reference name of the font to use for the calculation
	*** \param text The text string in unicode format
	*** \return The width of the text as it would be rendered, or -1 if there was an error
	 */
	int32 CalculateTextWidth(const std::string& font_name, const hoa_utils::ustring& text);

	/** \brief calculates the width of the given text if it were rendered with the given font
	*** \param font_name The reference name of the font to use for the calculation
	*** \param text The text string in standard format
	*** \return The width of the text as it would be rendered, or -1 if there was an error
	**/
	int32 CalculateTextWidth(const std::string& font_name, const std::string& text);
	//@}

private:
	//! \brief The default font, set to empty string if no fonts are loaded
	std::string _default_font;

	//! \brief The default color to render text in
	Color _default_text_color;

	//! STL map containing properties for each font (includeing TTF_Font *)
	std::map<std::string, FontProperties*> _font_map;

	/** \brief does the actual work of drawing text
	 *
	 *  \param uText  Pointer to a unicode string holding the text to draw
	 * \return success/failure
	 */
	bool _DrawTextHelper(const uint16 *const uText);

	/** Renders a given unicode string and TextStyle to a pixel array
	 * \param string The ustring to render
	 * \param style  The text style to render
	 * \param buffer The pixel array to render to
	 */
	bool _RenderText(hoa_utils::ustring& string, TextStyle& style, private_video::ImageMemory& buffer);

	/** \brief caches glyph info and textures for rendering
	 *
	 *  \param uText  Pointer to a unicode string holding the glyphs to cache
	 *  \param fp     Pointer to the internal FontProperties class representing the font
	 * \return success/failure
	 */
	bool _CacheGlyphs(const uint16 *uText, FontProperties *fp);

	/** \brief retrieves the shadow color based on the current color and shadow style
	 *
	 *  \param fp     Pointer to the internal FontProperties class representing the font
	 * \return the shadow color
	 */
	Color _GetTextShadowColor(FontProperties *fp);
}; // class TextSupervisor : public hoa_utils::Singleton

}  // namespace hoa_video

#endif // __TEXT_HEADER__
