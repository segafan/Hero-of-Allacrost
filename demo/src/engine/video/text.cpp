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
*** \author  Lindsay Roberts, linds@allacrost.org
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

template<> hoa_video::TextSupervisor* Singleton<hoa_video::TextSupervisor>::_singleton_reference = NULL;

namespace hoa_video {

TextSupervisor* TextManager = NULL;

namespace private_video {

// -----------------------------------------------------------------------------
// TextImageTexture class
// -----------------------------------------------------------------------------

TextImageTexture::TextImageTexture(const hoa_utils::ustring& string_, const TextStyle& style_) :
	BaseTexture(),
	string(string_),
	style(style_)
{
	// Enable image smoothing for text
	smooth = true;
	LoadFontProperties();
}



TextImageTexture::~TextImageTexture() {
	if (TextureManager->_IsTextImageRegistered(this)) {
		TextureManager->_text_images.erase(this);
		return;
	}
}



void TextImageTexture::LoadFontProperties() {
	if (style.shadow_style == VIDEO_TEXT_SHADOW_INVALID) {
		FontProperties *fp = TextManager->GetFontProperties(style.font);
		if (fp == NULL) {
			IF_PRINT_WARNING(VIDEO_DEBUG) << "invalid font '" << style.font << "'" << endl;
		}
		style.shadow_style = fp->shadow_style;
		style.shadow_offset_x = fp->shadow_x;
		style.shadow_offset_y = fp->shadow_y;
	}
}



bool TextImageTexture::Regenerate() {
	if (texture_sheet) {
		texture_sheet->RemoveTexture(this);
		TextureManager->_RemoveSheet(texture_sheet);
		texture_sheet = NULL;
	}

	ImageMemory buffer;

	if (TextManager->_RenderText(string, style, buffer) == false)
		return false;

	width = buffer.width;
	height = buffer.height;

	TexSheet* sheet = TextureManager->_InsertImageInTexSheet(this, buffer, true);
	if (sheet == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "call to TextureManager::_InsertImageInTexSheet() returned NULL" << endl;
		free(buffer.pixels);
		buffer.pixels = NULL;
		return false;
	}

	texture_sheet = sheet;
	free(buffer.pixels);
	buffer.pixels = NULL;

	return true;
}



bool TextImageTexture::Reload() {
	// Regenerate text image if it is not already loaded in a texture sheet
	if (texture_sheet == NULL)
		return Regenerate();

	ImageMemory buffer;

	if (TextManager->_RenderText(string, style, buffer) == false)
		return false;

	if (texture_sheet->CopyRect(x, y, buffer) == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "call to TextureSheet::CopyRect() failed" << endl;
		free(buffer.pixels);
		buffer.pixels = NULL;
		return false;
	}

	free(buffer.pixels);
	buffer.pixels = NULL;
	return true;
}

// -----------------------------------------------------------------------------
// TextImageElement class
// -----------------------------------------------------------------------------

TextImageElement::TextImageElement(TextImageTexture *image_, float x_offset_, float y_offset_, float u1_, float v1_,
	float u2_, float v2_, float width_, float height_) :
	image(image_),
	width(width_),
	height(height_),
	u1(u1_),
	v1(v1_),
	u2(u2_),
	v2(v2_),
	x_offset(x_offset_),
	y_offset(y_offset_),
	x_line_offset(0.0f),
	y_line_offset(0.0f),
	blend(false),
	unichrome_vertices(true)
{
	color[0] = Color::white;
	color[1] = Color::white;
	color[2] = Color::white;
	color[3] = Color::white;
}



TextImageElement::TextImageElement(TextImageTexture *image_, float x_offset_, float y_offset_, float u1_, float v1_,
		float u2_, float v2_, float width_, float height_, Color color_[4]) :
	image(image_),
	width(width_),
	height(height_),
	u1(u1_),
	v1(v1_),
	u2(u2_),
	v2(v2_),
	x_offset(x_offset_),
	y_offset(y_offset_),
	x_line_offset(0.0f),
	y_line_offset(0.0f),
	blend(false),
	unichrome_vertices(true)
{
	color[0] = color_[0];
	color[1] = color_[1];
	color[2] = color_[2];
	color[3] = color_[3];

	// TODO: check the color argument to determine value to set blend and unichrome_vertices
}

} // namespace private_video

// -----------------------------------------------------------------------------
// RenderedText class
// -----------------------------------------------------------------------------

RenderedText::RenderedText() :
	ImageDescriptor(),
	_alignment(ALIGN_CENTER)
{
	Clear();
	_grayscale = false;
}



RenderedText::RenderedText(const ustring& string, TextStyle style, int8 alignment) :
	ImageDescriptor(),
	_string(string),
	_style(style)
{
	_grayscale = false;
	Clear();
	SetAlignment(alignment);
	_Regenerate();
}



RenderedText::RenderedText(const string& string, TextStyle style, int8 alignment) :
	ImageDescriptor(),
	_string(MakeUnicodeString(string)),
	_style(style)
{
	_grayscale = false;
	Clear();
	SetAlignment(alignment);
	_Regenerate();
}



RenderedText::RenderedText(const RenderedText& copy) {
	// Copy all RenderedText members
	_string = copy._string;
	_text_sections = copy._text_sections;
	_alignment = copy._alignment;
	_style = copy._style;

	// Increment the reference count for all TextImageTexture objects
	std::vector<TextImageElement>::iterator i;
	for (i = _text_sections.begin(); i != _text_sections.end(); ++i) {
		if (i->image != NULL)
			i->image->AddReference();
	}
}



RenderedText &RenderedText::operator=(const RenderedText& copy) {
	// Handle the case were a dumbass assigns an object to itself
	if (this == &copy) {
		return *this;
	}

	// Copy all RenderedText members
	_string = copy._string;
	_text_sections = copy._text_sections;
	_alignment = copy._alignment;
	_style = copy._style;

	// Increment the reference count for all TextImageTexture objects
	std::vector<TextImageElement>::iterator i;
	for (i = _text_sections.begin(); i != _text_sections.end(); ++i) {
		if (i->image != NULL)
			i->image->AddReference();
	}

	return *this;
}



void RenderedText::Clear() {
	ImageDescriptor::Clear();
	_string.clear();
	_ClearImages();
}



void RenderedText::Draw() const {
	// TODO
}



void RenderedText::Draw(const Color& draw_color) const {
	// TODO
}



void RenderedText::SetAlignment(int8 alignment) {
	switch (alignment) {
		case ALIGN_LEFT:
		case ALIGN_CENTER:
		case ALIGN_RIGHT:
			if (_alignment != alignment) {
				_alignment = alignment;
				_Realign();
			}
			return;
		default:
			IF_PRINT_WARNING(VIDEO_DEBUG) << "bad value for alignment argument: " << alignment << endl;
			return;
	}
}



void RenderedText::_Regenerate() {
	_ClearImages();

	if (_string.empty())
		return;

	FontProperties* fp = TextManager->GetFontProperties(_style.font);
	if (TextManager->IsFontValid(_style.font) == false || fp == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "invalid font or font properties" << endl;
		return;
	}

	const uint16 newline = '\n';
	std::vector<uint16*> line_array;

	TextManager->_CacheGlyphs(_string.c_str(), fp);

	const uint16 *char_iter;
	uint16 *reformatted_text = new uint16[_string.size() + 1];
	uint16 *reform_iter = reformatted_text;
	uint16 *last_line = reformatted_text;
	for (char_iter = _string.c_str(); *char_iter; ++char_iter) {
		if (*char_iter == newline) {
			*reform_iter++ = '\0';
			line_array.push_back(last_line);
			last_line = reform_iter;
		}
		else {
			*reform_iter++ = *char_iter;
		}
	}
	line_array.push_back(last_line);
	*reform_iter = '\0';

	std::vector<uint16 *>::iterator line_iter;

	Color old_color = _style.color;
	int32 shadow_offset_x = 0;
	int32 shadow_offset_y = 0;
	Color shadow_color = TextManager->_GetTextShadowColor(fp);

	float total_height = static_cast<float>((line_array.size() - 1) * fp->line_skip);

	for (line_iter = line_array.begin(); line_iter != line_array.end(); ++line_iter) {
		TextImageTexture *timage = new TextImageTexture(*line_iter, _style);
		if (!timage->Regenerate()) {
			if (VIDEO_DEBUG) {
				cerr << "RenderedText::_Regenerate(): Failed to render TextImageTexture" << endl;
			}
		}

		// Increment the reference count
		timage->AddReference();
		float y_offset = total_height + _height * -VideoManager->_current_context.coordinate_system.GetVerticalDirection();
		y_offset += (fp->line_skip - timage->height) * VideoManager->_current_context.coordinate_system.GetVerticalDirection();
		TextImageElement element(timage, 0, y_offset, 0.0f, 0.0f, 1.0f, 1.0f, static_cast<float>(timage->width), static_cast<float>(timage->height), _color);

		// if text shadows are enabled, add a shadow version
		if (timage->style.shadow_style != VIDEO_TEXT_SHADOW_NONE) {
			shadow_offset_x = static_cast<int32>(VideoManager->_current_context.coordinate_system.GetHorizontalDirection()) * timage->style.shadow_offset_x;
			shadow_offset_y = static_cast<int32>(VideoManager->_current_context.coordinate_system.GetVerticalDirection())   * timage->style.shadow_offset_y;

			TextImageElement shadow_element = element;
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
			timage->AddReference();
		}

		TextureManager->_RegisterTextImage(timage);

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
} // void RenderedText::_Regenerate()



void RenderedText::_Realign() {
	vector<TextImageElement>::iterator i;
	for (i = _text_sections.begin(); i != _text_sections.end(); ++i) {
		i->x_offset = _alignment * VideoManager->_current_context.coordinate_system.GetHorizontalDirection() *
			((_width - i->width) / 2.0f) + i->x_line_offset;
	}
}

// -----------------------------------------------------------------------------
// TextSupervisor class
// -----------------------------------------------------------------------------

TextSupervisor::TextSupervisor()
{}



TextSupervisor::~TextSupervisor() {
	// Remove all loaded fonts and shutdown the font library
	for (map<string, FontProperties*>::iterator i = _font_map.begin(); i!= _font_map.end(); i++) {
		FontProperties* fp = i->second;

		if (fp->ttf_font)
			TTF_CloseFont(fp->ttf_font);

		if (fp->glyph_cache) {
			for (std::map<uint16, FontGlyph*>::iterator j = fp->glyph_cache->begin(); j != fp->glyph_cache->end(); j++) {
				delete (*j).second;
			}
			delete fp->glyph_cache;
		}

		delete fp;
	}

	TTF_Quit();
}



bool TextSupervisor::SingletonInitialize() {
	if (TTF_Init() < 0) {
		PRINT_ERROR << "SDL_ttf initialization failed" << endl;
		return false;
	}

	if (LoadFont("img/fonts/tarnhalo.ttf", "debug_font", 16) == false) {
		PRINT_ERROR << "could not load the debug font" << endl;
		TTF_Quit();
		return false;
	}

	return true;
}



bool TextSupervisor::LoadFont(const string& filename, const string& font_name, uint32 size, TEXT_SHADOW_STYLE style,
	int32 x_offset, int32 y_offset, bool make_default)
{
	// Make sure that the font name is not already taken
	if (IsFontValid(font_name) == true) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "a font with the desired reference name already existed: " << font_name << endl;
		return false;
	}

	if (size == 0) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "attempted to load a font of point size zero" << font_name << endl;
		return false;
	}

	// Attempt to load the font
	TTF_Font* font = TTF_OpenFont(filename.c_str(), size);
	if (font == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "TTF_OpenFont() failed to load the font file: " << filename << endl;
		return false;
	}

	// Create a new FontProperties object for this font
	FontProperties* fp = new FontProperties;

	// Set all of the font's properties
	fp->ttf_font = font;
	fp->height = TTF_FontHeight(font);
	fp->line_skip = TTF_FontLineSkip(font);
	fp->ascent = TTF_FontAscent(font);
	fp->descent = TTF_FontDescent(font);

	// Set default shadow. If both offsets are zero, set the shadow offset to be 1/8th the height of the font
	if (x_offset == 0 && y_offset == 0) {
		fp->shadow_x = max(fp->height / 8, 1);
		fp->shadow_y = -fp->shadow_x;
	}
	else {
		fp->shadow_x = x_offset;
		fp->shadow_y = y_offset;
	}

	// Set the shadow style and create the glyph cache
	fp->shadow_style = style;
	fp->glyph_cache = new std::map<uint16, FontGlyph*>;

	_font_map[font_name] = fp;

	return true;
} // bool TextSupervisor::LoadFont(...)



void TextSupervisor::SetDefaultFont(const std::string& font_name) {
	if (_font_map.find(font_name) == _font_map.end()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed because no font existed by the name: " << font_name << endl;
		return;
	}

	VideoManager->_current_context.font = font_name;
}



const string& TextSupervisor::GetDefaultFont() const {
	return VideoManager->_current_context.font;
}



FontProperties* TextSupervisor::GetFontProperties(const std::string& font_name) {
	if (IsFontValid(font_name) == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed becase argument was invalid for font name: " << font_name << endl;
		return NULL;
	}

	return _font_map[font_name];
}



void TextSupervisor::SetFontShadowStyle(const std::string& font_name, TEXT_SHADOW_STYLE style) {
	if (IsFontValid(font_name) == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed because the properties could not be found for font: " << font_name << endl;
		return;
	}

	FontProperties* font = _font_map[font_name];
	if (font == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed because the properties were NULL for for font: " << font_name << endl;
		return;
	}

	font->shadow_style = style;
}



void TextSupervisor::SetFontShadowOffsets(const std::string& font_name, int32 x, int32 y) {
	if (IsFontValid(font_name) == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "invalid font_name argument: " << font_name << endl;
		return;
	}

	FontProperties *font = _font_map[font_name];
	if (font == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed to retrieve font properties for font: " << font_name << endl;
		return;
	}

	font->shadow_x = x;
	font->shadow_y = y;
}



Color TextSupervisor::GetDefaultTextColor() const {
	return VideoManager->_current_context.text_color;
}



void TextSupervisor::SetDefaultTextColor(const Color& color) {
	VideoManager->_current_context.text_color = color;
}



void TextSupervisor::Draw(const ustring& text) {
	Draw(text, _default_style);
}


void TextSupervisor::Draw(const ustring& text, const TextStyle& style) {
	if (text.empty()) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "empty string was passed to function" << endl;
		return;
	}

	if (IsFontValid(style.font) == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed because font was invalid: " << style.font << endl;
		return;
	}

	FontProperties* fp = _font_map[style.font];
	if (fp == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed because font properties were invalid for font: " << style.font << endl;
		return;
	}

	TTF_Font* font = fp->ttf_font;
	if (font == NULL) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "failed because TTF_Font was invalid in font proproperties for font: " << style.font << endl;
		return;
	}

	VideoManager->PushState();

	int32 line_skip = fp->line_skip;
	// NOTE Optimization: something seems to be wrong with ustring, using a buffer instead
	uint16 buffer[2048];
	uint16 newline('\n');
	size_t last_line = 0;

	do {
		size_t next_line;
		for (next_line = last_line; next_line < text.length(); next_line++) {
			if (text[next_line] == newline)
				break;

			buffer[next_line - last_line] = text[next_line];
		}
		buffer[next_line - last_line] = 0;
		last_line = next_line + 1;

		glPushMatrix();

		// If text shadows are enabled, draw the shadow first
		if (fp->shadow_style != VIDEO_TEXT_SHADOW_NONE) {
			Color shadow_color = _GetTextShadowColor(fp);

			glPushMatrix();
			VideoManager->MoveRelative(VideoManager->_current_context.coordinate_system.GetHorizontalDirection() * fp->shadow_x, 0.0f);
			VideoManager->MoveRelative(0.0f, VideoManager->_current_context.coordinate_system.GetVerticalDirection() * fp->shadow_y);

			if (_DrawTextHelper(buffer, fp, shadow_color) == false) {
				VideoManager->PopState();
				return;
			}

			glPopMatrix();
		}

		// Now draw the text itself
		if (_DrawTextHelper(buffer, fp, style.color) == false) {
			VideoManager->PopState();
			return;
		}

		glPopMatrix();
		VideoManager->MoveRelative(0, -line_skip * VideoManager->_current_context.coordinate_system.GetVerticalDirection());

	} while (last_line < text.length());

	VideoManager->PopState();
} // void TextSupervisor::Draw(const ustring& text)



int32 TextSupervisor::CalculateTextWidth(const std::string& font_name, const hoa_utils::ustring& text) {
	if (IsFontValid(font_name) == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "font name argument was invalid: " << font_name << endl;
		return -1;
	}

	int32 width;
	if (TTF_SizeUNICODE(_font_map[font_name]->ttf_font, text.c_str(), &width, NULL) == -1) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "call to TTF_SizeUNICODE failed with TTF error: " << TTF_GetError() << endl;
		return -1;
	}

	return width;
}



int32 TextSupervisor::CalculateTextWidth(const std::string& font_name, const std::string& text) {
	if (IsFontValid(font_name) == false) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "font name argument was invalid: " << font_name << endl;
		return -1;
	}

	int32 width;
	if (TTF_SizeText(_font_map[font_name]->ttf_font, text.c_str(), &width, NULL) == -1) {
		IF_PRINT_WARNING(VIDEO_DEBUG) << "call to TTF_SizeText failed with TTF error: " << TTF_GetError() << endl;
		return -1;
	}

	return width;
}



bool TextSupervisor::_CacheGlyphs(const uint16 *uText, FontProperties *fp) {
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
		if(VideoManager->CheckGLError())
		{
			SDL_FreeSurface(initial);
			SDL_FreeSurface(intermediary);
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: glGetError() true after glGenTextures() in CacheGlyphs()!" << endl;
			return false;
		}

		TextureManager->_BindTexture(texture);
		if(VideoManager->CheckGLError())
		{
			SDL_FreeSurface(initial);
			SDL_FreeSurface(intermediary);
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: glGetError() true after glBindTexture() in CacheGlyphs()!" << endl;
			return false;
		}

		SDL_LockSurface(intermediary);

		uint32 num_bytes = w * h * 4;

		for (uint32 j = 0; j < num_bytes; j += 4)
		{
			((uint8*)intermediary->pixels)[j+3] = ((uint8*)intermediary->pixels)[j+2];

			((uint8*)intermediary->pixels)[j+0] = 0xff;
			((uint8*)intermediary->pixels)[j+1] = 0xff;
			((uint8*)intermediary->pixels)[j+2] = 0xff;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA,
					 GL_UNSIGNED_BYTE, intermediary->pixels );
		SDL_UnlockSurface(intermediary);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		if(VideoManager->CheckGLError())
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
} // TextSupervisor::CacheGlyphs()

//-----------------------------------------------------------------------------
// _DrawTextHelper: since there are two DrawText functions (one for unicode and
//                 one for non-unicode), this private function is used to
//                 do all the work so that code doesn't have to be duplicated.
//                 Either text or uText is valid string and the other is NULL.
//-----------------------------------------------------------------------------

bool TextSupervisor::_DrawTextHelper(const uint16 *uText, FontProperties *fp, Color color) {
	if(_font_map.empty())
		return false;

	// empty string, do nothing
	if (*uText == 0)
		return true;

	if (!fp)
		return false;

	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);

	CoordSys &cs = VideoManager->_current_context.coordinate_system;

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

	float xoff = ((VideoManager->_current_context.x_align+1) * fontwidth) * .5f * -cs.GetHorizontalDirection();
	float yoff = ((VideoManager->_current_context.y_align+1) * fontheight) * .5f * -cs.GetVerticalDirection();

	VideoManager->MoveRelative(xoff, yoff);

	float modulation = VideoManager->_screen_fader.GetFadeModulation();
	Color textColor = color * modulation;

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

		TextureManager->_BindTexture(glyphinfo->texture);

		if(VideoManager->CheckGLError())
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

		xpos += glyphinfo->advance;
	}

	glDisableClientState ( GL_VERTEX_ARRAY );
	glDisableClientState ( GL_TEXTURE_COORD_ARRAY );

	glPopMatrix();

	if (VideoManager->_fog_intensity > 0.0f)
		glEnable(GL_FOG);

	glDisable(GL_ALPHA_TEST);

	return true;
}



//-----------------------------------------------------------------------------
// _RenderText: Renders a given unicode string and TextStyle to a pixel array
//-----------------------------------------------------------------------------

bool TextSupervisor::_RenderText(hoa_utils::ustring &string, TextStyle &style, ImageMemory &buffer)
{
	FontProperties * fp = _font_map[style.font];
	TTF_Font * font     = fp->ttf_font;

	if (!font)
	{
		if (VIDEO_DEBUG)
			cerr << "TextSupervisor::_RenderText(): font '" << style.font << "' not valid." << endl;
		return false;
	}

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

	uint8 *intermed_buf = (uint8 *) calloc(line_w * line_h, 4);
	intermediary = SDL_CreateRGBSurfaceFrom(intermed_buf, line_w, line_h, 32, line_w * 4, rmask, gmask, bmask, amask);

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
			free(intermed_buf);
			if(VIDEO_DEBUG)
				cerr << "VIDEO ERROR: SDL_BlitSurface() failed in _RenderText()! (" << SDL_GetError() << ")" << endl;
			return false;
		}
		SDL_FreeSurface(initial);
		xpos += glyphinfo->advance;
	}

	SDL_LockSurface(intermediary);

	uint8 color_mult[] = {
		(uint8) style.color[0] * 0xFF,
		(uint8) style.color[1] * 0xFF,
		(uint8) style.color[2] * 0xFF,
	};

	uint32 num_bytes = intermediary->w * intermediary->h * 4;
	for (uint32 j = 0; j < num_bytes; j += 4)
	{
		((uint8*)intermediary->pixels)[j+3] = ((uint8*)intermediary->pixels)[j+2];
		((uint8*)intermediary->pixels)[j+0] = color_mult[0];
		((uint8*)intermediary->pixels)[j+1] = color_mult[1];
		((uint8*)intermediary->pixels)[j+2] = color_mult[2];
	}

	buffer.width = line_w;
	buffer.height = line_h;
	buffer.pixels = intermed_buf;

	SDL_UnlockSurface(intermediary);
	SDL_FreeSurface(intermediary);

	return true;
}



Color TextSupervisor::_GetTextShadowColor(FontProperties *fp) {
	Color shadow_color;

	if (fp->shadow_style != VIDEO_TEXT_SHADOW_NONE) {
		switch( fp->shadow_style) {
			case VIDEO_TEXT_SHADOW_DARK:
				shadow_color = Color::black;
				shadow_color[3] = VideoManager->_current_context.text_color[3] * 0.5f;
				break;
			case VIDEO_TEXT_SHADOW_LIGHT:
				shadow_color = Color::white;
				shadow_color[3] = VideoManager->_current_context.text_color[3] * 0.5f;
				break;
			case VIDEO_TEXT_SHADOW_BLACK:
				shadow_color = Color::black;
				shadow_color[3] = VideoManager->_current_context.text_color[3];
				break;
			case VIDEO_TEXT_SHADOW_COLOR:
				shadow_color = VideoManager->_current_context.text_color;
				shadow_color[3] = VideoManager->_current_context.text_color[3] * 0.5f;
				break;
			case VIDEO_TEXT_SHADOW_INVCOLOR:
				shadow_color = Color(1.0f - VideoManager->_current_context.text_color[0], 1.0f - VideoManager->_current_context.text_color[1],
					1.0f - VideoManager->_current_context.text_color[2], VideoManager->_current_context.text_color[3] * 0.5f);
				break;
			default:
				if(VIDEO_DEBUG)
					cerr << "VIDEO ERROR: Unknown text shadow style (" << fp->shadow_style << ") -  TextSupervisor::_GetTextShadowColor()" << endl;
				break;
		}
	}

	return shadow_color;
}

}  // namespace hoa_video
