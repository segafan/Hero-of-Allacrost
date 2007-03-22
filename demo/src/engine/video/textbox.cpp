///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2007 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

#include <sstream>

#include "utils.h"
#include "video.h"
#include "textbox.h"
#include "menu_window.h"

using namespace std;
using namespace hoa_utils;
using namespace hoa_video;
using namespace hoa_video::private_video;

namespace hoa_video {

TextBox::TextBox() {
	_finished = false;
	_current_time = 0;
	_mode = VIDEO_TEXT_INVALID;
	_display_speed = 0.0f;
	_num_chars = 0;
	_initialized = IsInitialized(_initialization_errors);
	_width = _height = 0.0f;
	_text_xalign = VIDEO_X_LEFT;
	_text_yalign = VIDEO_Y_BOTTOM;
}



TextBox::~TextBox() {
	// Does nothing since TextBox doesn't allocate any memory
}



void TextBox::ClearText() {
	_finished = true;
	_text.clear();
	_num_chars = 0;
}



void TextBox::Update(uint32 time) {
	_current_time += time;

	if (_text.empty() == false && _current_time > _end_time)
		_finished = true;
}



void TextBox::Draw() {
	if (_text.empty())
		return;

	if (_initialized == false) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: TextBox::Draw() failed because the textbox was not initialized:\n" << _initialization_errors << endl;
		return;
	}

	// Take the following steps to draw the text:
	//  (1): Save the video engine's context and enable appropriate draw settings
	//  (2): Determine the coordinates of the textbox rectangle to draw
	//  (3): Create a ScreenRect for the textbox area and apply scissoring if its enabled
	//  (4): Determine the text draw position from the alignment flags
	//  (5): Draw each line of text to the screen
	//  (6): Restore the original video engine context
	VideoManager->_PushContext();

	VideoManager->SetDrawFlags(_xalign, _yalign, VIDEO_BLEND, 0);
	VideoManager->SetFont(_font);

	// Stores the positions of the four sides of the rectangle
	float left   = 0.0f;
	float right  = _width;
	float bottom = 0.0f;
	float top    = _height;

	CalculateAlignedRect(left, right, bottom, top);

	// Create a screen rectangle for the position and apply any scissoring
	int32 x, y, w, h;

	x = static_cast<int32>(left < right ? left : right);
	y = static_cast<int32>(top < bottom ? top : bottom);
	w = static_cast<int32>(right - left);
	h = static_cast<int32>(top - bottom);

	if (w < 0)
		w = -w;
	if (h < 0)
		h = -h;

	ScreenRect rect(x, y, w, h);

	if (_owner) {
		rect.Intersect(_owner->GetScissorRect());
	}
	rect.Intersect(VideoManager->GetScissorRect());
	VideoManager->EnableScissoring(_owner || VideoManager->IsScissoringEnabled());
	if (VideoManager->IsScissoringEnabled()) {
		VideoManager->SetScissorRect(rect);
	}

	// Holds the height of the text to be drawn
	float text_height = static_cast<float>(_CalculateTextHeight());
	// Holds the x and y position where the text should be drawn
	float text_xpos, text_ypos;

	// Determine the vertical position of the text based on the alignment
	if (_yalign == VIDEO_Y_TOP) {
		text_ypos = top;
	}
	else if (_yalign == VIDEO_Y_CENTER) {
		text_ypos = top - (VideoManager->_coord_sys.GetVerticalDirection() * (_height - text_height) * 0.5f);
	}
	else { // (_yalign == VIDEO_Y_BOTTOM)
		text_ypos = top - (VideoManager->_coord_sys.GetVerticalDirection() * (_height - text_height));
	}

	// Determine the horizontal position of the text based on the alignment
	if (_text_xalign == VIDEO_X_LEFT) {
		text_xpos = left;
	}
	else if (_text_xalign == VIDEO_X_CENTER) {
		text_xpos = (left + right) * 0.5f;
	}
	else { // (_text_xalign == VIDEO_X_RIGHT)
		text_xpos = right;
	}

	// Set the draw cursor, draw flags, and draw the text
	VideoManager->Move(0.0f, text_ypos);
	VideoManager->SetDrawFlags(VIDEO_X_LEFT, VIDEO_Y_TOP, VIDEO_BLEND, 0);
	_DrawTextLines(text_xpos, text_ypos, rect);

	VideoManager->_PopContext();
} // void TextBox::Draw()



void TextBox::SetDimensions(float w, float h) {
	if (w <= 0.0f || w > 1024.0f) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: TextBox::SetDimensions() failed, invalid width: " << w << endl;
		return;
	}

	if (h <= 0.0f || h > 768.0f) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: TextBox::SetDimensions() failed, invalid height: " << h << endl;
		return;
	}

	_width = w;
	_height = h;

	_initialized = IsInitialized(_initialization_errors);
}




void TextBox::SetTextAlignment(int32 xalign, int32 yalign) {
	_text_xalign = xalign;
	_text_yalign = yalign;

	_initialized = IsInitialized(_initialization_errors);
}



void TextBox::SetFont(const string& font_name) {
	_font_properties = VideoManager->GetFontProperties(font_name);
	if (_font_properties == NULL) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: TextBox::SetFont() failed because it was passed an invalid font name: " << font_name << endl;
		return;
	}

	_font = font_name;
	_initialized = IsInitialized(_initialization_errors);
}



void TextBox::SetDisplayMode(const TEXT_DISPLAY_MODE &mode) {
	if (mode <= VIDEO_TEXT_INVALID || mode >= VIDEO_TEXT_TOTAL) {
		cerr << "VIDEO WARNING: TextBox::SetDisplayMode() failed because of an invalid mode argument: " << mode << endl;
		return;
	}

	_mode = mode;
	_initialized = IsInitialized(_initialization_errors);
}



void TextBox::SetDisplaySpeed(float display_speed) {
	if (display_speed <= 0.0f) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: TextBox::SetDisplaySpeed() failed due to an invalid display speed: " << display_speed << endl;
		return;
	}

	_display_speed = display_speed;
	_initialized = IsInitialized(_initialization_errors);
}



void TextBox::SetDisplayText(const ustring& text) {
	if (text.empty()) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: empty string passed to TextBox::SetDisplayText()!" << endl;
		return;
	}

	if (_initialized == false) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: TextBox::SetDisplayText() failed because the textbox was not initialized:\n" << _initialization_errors << endl;
		return;
	}

	// (1): Go through the text ustring and determine where the newline characters can be found, examining one line at a time and adding it to the _text vector.
	size_t newline_pos;
	ustring temp_str = text;

	_text.clear();
	_num_chars = 0;

	while (true) {
		newline_pos = temp_str.find(NEWLINE_CHARACTER);

		// If the end of the string has been reached, add the new line and exit
		if (newline_pos == ustring::npos) {
			_AddLine(temp_str);
			break;
		}
		// Otherwise, add the new line segment and proceed to find the next
		else {
			_AddLine(temp_str.substr(0, newline_pos));
			temp_str = temp_str.substr(newline_pos + 1, temp_str.length() - newline_pos);
		}
	}

	// (2): Calculate the height of the text and check it against the height of the textbox.
	int32 text_height = _CalculateTextHeight();

	if (text_height > _height) {
		if (VIDEO_DEBUG) {
			cerr << "VIDEO WARNING: In TextBox::SetDisplayText() tried to display text of height (";
			cerr << text_height << ") in a window of lower height (" << _height << ")" << endl;
		}
	}

	// Reset the timer since new text has been set
	_current_time = 0;

	// (3): Determine how much time the text will take to display depending on the display mode, speed, and size of the text
	switch(_mode) {
		case VIDEO_TEXT_INSTANT:
			_end_time = 0;
			break;

		case VIDEO_TEXT_CHAR:     // All three of these modes display one character at a time
		case VIDEO_TEXT_FADECHAR:
		case VIDEO_TEXT_REVEAL:
			// We desire the total number of milliseconds to render the string.
			// Display speed is in character per second, so cancel the character term and multiply by 1000 to get ms
			_end_time = static_cast<int32>(1000.0f * _num_chars / _display_speed);
			break;

		case VIDEO_TEXT_FADELINE:   // Displays one line at a time
			// Instead of _num_chars in the other calculation, we use number of lines times VIDEO_CHARS_PER_LINE
			_end_time = static_cast<int32>(1000.0f * (_text.size() * VIDEO_CHARS_PER_LINE) / _display_speed);
			break;

		default:
			_end_time = 0;
			if (VIDEO_DEBUG)
				cerr << "VIDEO WARNING: unknown display mode was set in TextBox::SetDisplayText(): " << _mode << endl;
			break;
	};

	// (4): Set finished to true only if the display mode is VIDEO_TEXT_INSTANT
	if (_mode == VIDEO_TEXT_INSTANT)
		_finished = true;
	else
		_finished = false;
} // void TextBox::SetDisplayText(const ustring& text)



void TextBox::SetDisplayText(const string &text) {
	ustring wstr = MakeUnicodeString(text);
	SetDisplayText(wstr);
}



bool TextBox::IsInitialized(string& errors) {
	errors.clear();
	ostringstream stream;

	// Check width
	if (_width <= 0.0f || _width > 1024.0f)
		stream << "* Invalid width (" << _width << ")" << endl;

	// Check height
	if (_height <= 0.0f || _height > 768.0f)
		stream << "* Invalid height (" << _height << ")" << endl;

	// Check alignment flags
	if (_text_xalign < VIDEO_X_LEFT || _text_xalign > VIDEO_X_RIGHT)
		stream << "* Invalid x alignment (" << _text_xalign << ")" << endl;

	if (_text_yalign < VIDEO_Y_TOP || _text_yalign > VIDEO_Y_BOTTOM)
		stream << "* Invalid y alignment (" << _text_yalign << ")" << endl;

	// Check font
	if (_font.empty())
		stream << "* Invalid font: no font has been set" << endl;
	else if (VideoManager->IsValidFont(_font) == false)
		stream << "* Invalid font: " << _font << endl;

	// Check display speed
	if (_display_speed <= 0.0f)
		stream << "* Invalid display speed (" << _display_speed << ")" << endl;

	// Check display mode
	if (_mode <= VIDEO_TEXT_INVALID || _mode >= VIDEO_TEXT_TOTAL)
		stream << "* Invalid display mode (" << _mode << ")" << endl;

	errors = stream.str();

	if (errors.empty()) {
		_initialized = true;
	}
	else {
		_initialized = false;
	}

	return _initialized;
} // bool TextBox::IsInitialized(string& errors)



int32 TextBox::_CalculateTextHeight() {
	if (_text.empty())
		return 0;
	else
		return _font_properties->height + _font_properties->line_skip * (static_cast<int32>(_text.size()) - 1);
}



void TextBox::_AddLine(const ustring& line) {
	// perform word wrapping in a loop until all the text is added

	ustring temp_line = line;

	while (temp_line.empty() == false) {
		int32 text_width = VideoManager->CalculateTextWidth(_font, line);

		// If the text can fit in the text box, add the whole line and return
		if (text_width < _width) {
			_text.push_back(temp_line);
			_num_chars += static_cast<int32>(temp_line.size());
			return;
		}

		// Otherwise, find the maximum number of words which can fit and make that substring a line
		// Word boundaries are found by calling the _IsCharacterBreakable() method
		ustring wrapped_line;
		int32 num_wrapped_chars = 0;
		int32 last_breakable_index = -1;
		int32 line_length = static_cast<int32>(temp_line.length());

		while (num_wrapped_chars < line_length) {
			wrapped_line += temp_line[num_wrapped_chars];

			if (_IsBreakableChar(temp_line[num_wrapped_chars])) {
				int32 text_width = VideoManager->CalculateTextWidth(_font, wrapped_line);

				if (text_width < _width) {
					// We haven't gone past the breaking point: mark this as a possible breaking point
					last_breakable_index = num_wrapped_chars;
				}
				else {
					// We exceeded the maximum width, so go back to the previous breaking point.
					// If there was no previous breaking point (== -1), then just break it off at
					// the current character position.
					if (last_breakable_index != -1)
						num_wrapped_chars = last_breakable_index;
					break;
				}
			} // (_IsBreakableChar(temp_line[num_wrapped_chars]))
			++num_wrapped_chars;
		} // while (num_wrapped_chars < line_length)

		// Figure out the number of characters in the wrapped line and construct the wrapped line
		text_width = VideoManager->CalculateTextWidth(_font, wrapped_line);
		if (text_width >= _width && last_breakable_index != -1) {
			num_wrapped_chars = last_breakable_index;
		}
		wrapped_line = temp_line.substr(0, num_wrapped_chars);

		// Add the new wrapped line to the text.
		_text.push_back(wrapped_line);
		_num_chars += static_cast<int32>(wrapped_line.size());

		// If there is no more text remaining, we are finished.
		if (num_wrapped_chars == line_length)
			return;
		// Otherwise, we need to grab the rest of the text that remains to be added and loop again
		else
			temp_line = temp_line.substr(num_wrapped_chars + 1, line_length - num_wrapped_chars);
	} // while (temp_line.empty() == false)
} // void TextBox::_AddLine(const ustring& line)



bool TextBox::_IsBreakableChar(uint16 character) {
	if (character == 0x20)
		return true;

	return false;
}



void TextBox::_DrawTextLines(float text_x, float text_y, ScreenRect scissor_rect) {
	int32 num_chars_drawn = 0;
// 	// A quick
// 	TEXT_DISPLAY_MODE mode = _mode;
//
// 	if (_finished)
// 		mode = VIDEO_TEXT_INSTANT;

	// Calculate the fraction of the text to display
	float percent_complete;
	if (_finished)
		percent_complete = 1.0f;
	else
		percent_complete = static_cast<float>(_current_time) / static_cast<float>(_end_time);

	// Iterate through the loop for every line of text and draw it
	for (int32 line = 0; line < static_cast<int32>(_text.size()); ++line) {
		// (1): Calculate the x draw offset for this line and move to that position
		float line_width = static_cast<float>(VideoManager->CalculateTextWidth(_font, _text[line]));
		int32 x_align = VideoManager->_ConvertXAlign(_text_xalign);
		float x_offset = text_x + ((x_align + 1) * line_width) * 0.5f * VideoManager->_coord_sys.GetHorizontalDirection();
		VideoManager->MoveRelative(x_offset, 0.0f);

		int32 line_size = static_cast<int32>(_text[line].size());

		// (2): Draw the text depending on the display mode and whether or not the gradual display is finished
		if (_finished || _mode == VIDEO_TEXT_INSTANT) {
			VideoManager->DrawText(_text[line]);
		}

		else if (_mode == VIDEO_TEXT_CHAR) {
			// Determine which character is currently being rendered
			int32 cur_char = static_cast<int32>(percent_complete * _num_chars);

			// If the current character to draw is after this line, render the entire line
			if (num_chars_drawn + line_size < cur_char) {
				VideoManager->DrawText(_text[line]);
			}
			// The current character to draw is on this line: figure out which characters on this line should be drawn
			else {
				int32 num_completed_chars = cur_char - num_chars_drawn;
				if (num_completed_chars > 0) {
					ustring substring = _text[line].substr(0, num_completed_chars);
					VideoManager->DrawText(substring);
				}
			}
		} // else if (_mode == VIDEO_TEXT_CHAR)

		else if (_mode == VIDEO_TEXT_FADECHAR) {
			// Figure out which character is currently being rendered
			float fade_cur_char = percent_complete * _num_chars;
			int32 cur_char = static_cast<int32>(fade_cur_char);
			float cur_percent = fade_cur_char - cur_char;

			// If the current character to draw is after this line, draw the whole line
			if (num_chars_drawn + line_size <= cur_char) {
				VideoManager->DrawText(_text[line]);
			}
			// The current character is on this line: draw any previous characters on this line as well as the current character
			else {
				int32 num_completed_chars = cur_char - num_chars_drawn;

				// Continue only if this line has at least one character that should be drawn
				if (num_completed_chars >= 0) {
					ustring substring;

					// Draw any fully completed characters at full opacity
					if (num_completed_chars > 0) {
						substring = _text[line].substr(0, num_completed_chars);
						VideoManager->DrawText(substring);
					}

					// Draw the current character that is being faded in at the appropriate alpha level
					Color old_color = VideoManager->GetTextColor();
					Color new_color = old_color;
					new_color[3] *= cur_percent;

					VideoManager->SetTextColor(new_color);
					VideoManager->MoveRelative(static_cast<float>(VideoManager->CalculateTextWidth(_font, substring)), 0.0f);
					VideoManager->DrawText(_text[line].substr(num_completed_chars, 1));
					VideoManager->SetTextColor(old_color);
				}
			}
		} // else if (_mode == VIDEO_TEXT_FADECHAR)

		else if (_mode == VIDEO_TEXT_FADELINE) {
			// Deteremine which line is currently being rendered
			float fade_lines = percent_complete * _text.size();
			int32 lines = static_cast<int32>(fade_lines);
			float cur_percent = fade_lines - lines;

			// If this line comes before the line being rendered, simply draw the line and be done with it
			if (line < lines) {
				VideoManager->DrawText(_text[line]);
			}
			// Otherwise if this is the line being rendered, determine the amount of alpha for the line being faded in and draw it
			else if (line == lines) {
				Color old_color = VideoManager->GetTextColor();
				Color new_color = old_color;
				new_color[3] *= cur_percent;

				VideoManager->SetTextColor(new_color);
				VideoManager->DrawText(_text[line]);
				VideoManager->SetTextColor(old_color);
			}
		} // else if (_mode == VIDEO_TEXT_FADELINE)

		else if (_mode == VIDEO_TEXT_REVEAL) {
			// Determine which character is currently being rendered
			float fade_cur_char = percent_complete * _num_chars;
			int32 cur_char = static_cast<int32>(fade_cur_char);
			float cur_percent = fade_cur_char - cur_char;
			int32 num_completed_chars = cur_char - num_chars_drawn;

			// If the current character comes after this line, simply render the entire line
			if (num_chars_drawn + line_size <= cur_char) {
				VideoManager->DrawText(_text[line]);
			}
			// If the line contains the current character, draw all previous characters as well as the current one
			else if (num_completed_chars >= 0) {
				ustring substring;

				// If there are already completed characters on this line, draw them in full
				if (num_completed_chars > 0) {
					substring = _text[line].substr(0, num_completed_chars);
					VideoManager->DrawText(substring);
				}

				// Now draw the current character from the line, partially scissored according to the amount that is complete
				ustring cur_char_string = _text[line].substr(num_completed_chars, 1);

				// Create a rectangle for the current character, in window coordinates
				int32 char_x, char_y, char_w, char_h;
				char_x = static_cast<int32>(x_offset + VideoManager->_coord_sys.GetHorizontalDirection()
					* VideoManager->CalculateTextWidth(_font, substring));
				char_y = static_cast<int32>(text_y - VideoManager->_coord_sys.GetVerticalDirection()
					* (_font_properties->height + _font_properties->descent));

				if (VideoManager->_coord_sys.GetHorizontalDirection() < 0.0f)
					char_y = static_cast<int32>(VideoManager->_coord_sys.GetBottom()) - char_y;

				if (VideoManager->_coord_sys.GetVerticalDirection() < 0.0f)
					char_x = static_cast<int32>(VideoManager->_coord_sys.GetLeft()) - char_x;

				char_w = VideoManager->CalculateTextWidth(_font, cur_char_string);
				char_h = _font_properties->height;

				// Multiply the width by percentage done to determine the scissoring dimensions
				char_w = static_cast<int32>(cur_percent * char_w);
				VideoManager->MoveRelative(VideoManager->_coord_sys.GetHorizontalDirection()
					* VideoManager->CalculateTextWidth(_font, substring), 0.0f);

				// Construct the scissor rectangle using the character dimensions and draw the revealing character
				VideoManager->PushState();
				ScreenRect char_scissor_rect(char_x, char_y, char_w, char_h);
				scissor_rect.Intersect(char_scissor_rect);
				VideoManager->EnableScissoring(true);
				VideoManager->SetScissorRect(scissor_rect);
				VideoManager->DrawText(cur_char_string);
				VideoManager->PopState();
			}
			// In the else case, the current character is before the line, so we don't draw anything for this line at all
		} // else if (_mode == VIDEO_TEXT_REVEAL)

		else {
			// Invalid display mode: just render the text instantly
			VideoManager->DrawText(_text[line]);
			if (VIDEO_DEBUG) {
				cerr << "VIDEO WARNING: In TextBox::_DrawTextLines, an unsupported text display mode was active: " << _mode << endl;
			}
		}

		// (3): Prepare to draw the next line and move the draw cursor appropriately
		num_chars_drawn += line_size;
// 		VideoManager->MoveRelative(-xOffset, _font_properties.line_skip * -cs._vertical_direction);
		text_y += _font_properties->line_skip * -VideoManager->_coord_sys.GetVerticalDirection();
		VideoManager->Move(0.0f, text_y);
	} // for (int32 line = 0; line < static_cast<int32>(_text.size()); ++line)
} // void TextBox::_DrawTextLines(float text_x, float text_y, ScreenRect scissor_rect)


}  // namespace hoa_video
