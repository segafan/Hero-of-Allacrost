///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2008 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

#include <sstream>

#include "option.h"
#include "video.h"

using namespace std;

using namespace hoa_utils;
using namespace hoa_video::private_video;

namespace hoa_video {

OptionBox::OptionBox() {
	_option_xalign = VIDEO_X_LEFT;
	_option_yalign = VIDEO_Y_TOP;
	_selection_mode = VIDEO_SELECT_SINGLE;
	_switching = false;
	_horizontal_wrap_mode = VIDEO_WRAP_MODE_NONE;
	_vertical_wrap_mode = VIDEO_WRAP_MODE_NONE;
	_cursor_state = VIDEO_CURSOR_STATE_VISIBLE;
	_event = 0;
	_selection = -1;
	_first_selection = -1;
	_number_options = 0;
	_scrolling = 0;
	_scroll_offset = 0;
	_blink_time = 0;
	_scroll_time = 0;
	_blink = false;
	_cursor_xoffset = 0.0f;
	_cursor_yoffset = 0.0f;
	_horizontal_spacing = _vertical_spacing = 0.0f;
	_number_rows = _number_columns = 0;
	_initialized = IsInitialized(_initialization_errors);
	_scissoring = false;
	_scissoring_owner = false;
}



OptionBox::~OptionBox() {
	ClearOptions();
}



void OptionBox::Update(uint32 frame_time) {
	_event = 0; // Clear all events

	_blink = ((_blink_time / VIDEO_CURSOR_BLINK_RATE) % 2) == 1;
	_blink_time += frame_time;

	if (_scrolling) {
		_scroll_time += frame_time;

		if (_scroll_time > VIDEO_OPTION_SCROLL_TIME) {
			_scroll_time = 0;
			_scrolling = false;
		}
	}
}



void OptionBox::Draw() {
	// Do nothing if the option box is not properly initialized
	if (!IsInitialized(_initialization_errors)) {
		return;
	}

	VideoManager->PushState();
	VideoManager->SetDrawFlags(_xalign, _yalign, VIDEO_BLEND, 0);

	// (1) Determine the edge dimensions of the option box
	float left, right, bottom, top;
	left = 0.0f;
	right = _number_columns * _horizontal_spacing;
	bottom = 0.0f;
	top = _number_rows * _vertical_spacing;

	CalculateAlignedRect(left, right, bottom, top);

	int32 x, y, w, h;

	x = static_cast<int32>(left < right ? left : right);
	y = static_cast<int32>(top < bottom ? top : bottom);
	w = static_cast<int32>(right - left);
	if (w < 0)
		w = -w;
	h = static_cast<int32>(top - bottom);
	if (h < 0)
		h = -h;

	//Calculate scissoring rectangle
	ScreenRect rect(x, y, w, h);

	CoordSys &cs = VideoManager->_current_context.coordinate_system;

	if( cs.GetVerticalDirection() < 0 ) {
		rect.top += static_cast<int32>( _vertical_spacing ) + ( _number_rows ); //To accomodate the 1 pixel per row offset
	}

	bool scissor = VideoManager->IsScissoringEnabled();
	if ( _owner && _scissoring && _scissoring_owner ) {
		rect.Intersect(_owner->GetScissorRect());
		if( VideoManager->IsScissoringEnabled() ) {
			rect.Intersect(VideoManager->GetScissorRect());
		}
		scissor = true;
		VideoManager->SetScissorRect(rect);
	} else if ( _scissoring && !_scissoring_owner ) {
		if( VideoManager->IsScissoringEnabled() ) {
			rect.Intersect(VideoManager->GetScissorRect());
		}
		scissor = true;
		VideoManager->SetScissorRect(rect);
	}

	VideoManager->Text()->SetDefaultFont(_font);
	VideoManager->SetDrawFlags(_option_xalign, _option_yalign, VIDEO_X_NOFLIP, VIDEO_Y_NOFLIP, VIDEO_BLEND, 0);

	int32 row_min, row_max;
	float cell_offset = 0.0f;

	if (!_scrolling) {
		row_min = _scroll_offset;
		row_max = _scroll_offset + _number_rows;
	}
	else if (_scroll_direction == -1) { // Scrolling up
		row_min = _scroll_offset;
		row_max = _scroll_offset + _number_rows + 1;

		cell_offset = cs.GetVerticalDirection() * (1.0f - (_scroll_time / static_cast<float>(VIDEO_OPTION_SCROLL_TIME))) * _vertical_spacing;
	}
	else { // Scrolling down
		row_min = _scroll_offset - 1;
		row_max = _scroll_offset + _number_rows;
		cell_offset = cs.GetVerticalDirection() * ((_scroll_time / static_cast<float>(VIDEO_OPTION_SCROLL_TIME))) * _vertical_spacing;
	}

	OptionCellBounds bounds;
	bounds.y_top = top + cell_offset;
	bounds.y_center = bounds.y_top - 0.5f * _vertical_spacing * cs.GetVerticalDirection();
	bounds.y_bottom = (bounds.y_center * 2.0f) - bounds.y_top;

	float yoff = -_vertical_spacing * cs.GetVerticalDirection();
	float xoff = _horizontal_spacing * cs.GetHorizontalDirection();
	bool finished = false;

	// Iterate through all the visible option cells and draw them
	for (int32 row = row_min; row < row_max; row++) {
		if (scissor)
			VideoManager->EnableScissoring();
		else
			VideoManager->DisableScissoring();

		bounds.x_left = left;
		bounds.x_center = bounds.x_left + (0.5f * _horizontal_spacing * cs.GetHorizontalDirection());
		bounds.x_right = (bounds.x_center * 2.0f) - bounds.x_left;

		// Draw the columns of options
		for (int32 col = 0; col < _number_columns; ++col) {
			int32 index = row * _number_columns + col;

			if (index >= _number_options || index < 0) {
				finished = true;
				break;
			}

			float left_edge = 999999.0f; // The x offset to where the text actually begins
			float x, y;

			int32 xalign = _option_xalign;
			int32 yalign = _option_yalign;
			_SetupAlignment(xalign, yalign, bounds, x, y);

			Option &op = _options.at(index);
			/*if (op.disabled)
				VideoManager->Text()->SetDefaultTextColor(Color::gray);
			else
				VideoManager->Text()->SetDefaultTextColor(Color::white);
			*/

			// Iterate through all option elements in the current option
			for (int32 element = 0; element < static_cast<int32>(op.elements.size()); element++) {
				switch (op.elements[element].type) {
					case VIDEO_OPTION_ELEMENT_LEFT_ALIGN:
					{
						xalign = VIDEO_X_LEFT;
						_SetupAlignment(xalign, _option_yalign, bounds, x, y);
						break;
					}
					case VIDEO_OPTION_ELEMENT_CENTER_ALIGN:
					{
						xalign = VIDEO_X_CENTER;
						_SetupAlignment(xalign, _option_yalign, bounds, x, y);
						break;
					}
					case VIDEO_OPTION_ELEMENT_RIGHT_ALIGN:
					{
						xalign = VIDEO_X_RIGHT;
						_SetupAlignment(xalign, _option_yalign, bounds, x, y);
						break;
					}
					case VIDEO_OPTION_ELEMENT_IMAGE:
					{
						int32 image_index = op.elements[element].value;

						if (image_index >= 0 && image_index < static_cast<int32>(op.images.size())) {
							if (op.disabled)
								op.images[image_index].Draw(Color::gray);
							else
								op.images[image_index].Draw(Color::white);

							float width = op.images[image_index].GetWidth();
							float edge = x - bounds.x_left; // edge value for VIDEO_X_LEFT
							if (xalign == VIDEO_X_CENTER)
								edge -= width * 0.5f * cs.GetHorizontalDirection();
							else if (xalign == VIDEO_X_RIGHT)
								edge -= width * cs.GetHorizontalDirection();

							if (edge < left_edge)
								left_edge = edge;

						}
						break;
					}
					case VIDEO_OPTION_ELEMENT_POSITION:
					{
						x = bounds.x_left + op.elements[element].value * cs.GetHorizontalDirection();
						VideoManager->Move(x, y);
						break;
					}
					case VIDEO_OPTION_ELEMENT_TEXT:
					{
						int32 text_index = op.elements[element].value;

						if (text_index >= 0 && text_index < static_cast<int32>(op.text.size())) {
							const ustring& text = op.text[text_index];
							float width = static_cast<float>(VideoManager->Text()->CalculateTextWidth(_font, text));
							float edge = x - bounds.x_left; // edge value for VIDEO_X_LEFT

							if (xalign == VIDEO_X_CENTER)
								edge -= width * 0.5f * cs.GetHorizontalDirection();
							else if (xalign == VIDEO_X_RIGHT)
								edge -= width * cs.GetHorizontalDirection();

							if (edge < left_edge)
								left_edge = edge;
							TextStyle ts(_font);
							if (op.disabled)
								ts.color = Color::gray;
							TextManager->Draw(text, ts);
						}

						break;
					}
					case VIDEO_OPTION_ELEMENT_INVALID:
					case VIDEO_OPTION_ELEMENT_TOTAL:
					default:
					{
						cerr << "VIDEO WARNING: In OptionBox::Draw(), invalid option element type was present" << endl;
						break;
					}
				} // switch (op.elements[element].type)
			} // for (int32 element = 0; element < static_cast<int32>(op.elements.size()); element++)

			// Should never scissor the cursor
			VideoManager->DisableScissoring();

			float cursor_offset = 0;
			if (_scrolling) {
				if (_scroll_direction == -1)// Scrolling up
					cursor_offset = -cell_offset;
				else // Scrolling down
					cursor_offset = -cell_offset + cs.GetVerticalDirection() * _vertical_spacing;
			}

			// Check if this is the index where we should draw the cursor icon for switching elements
			if (index == _first_selection && _blink == false && _cursor_state != VIDEO_CURSOR_STATE_HIDDEN) {
				_SetupAlignment(VIDEO_X_LEFT, _option_yalign, bounds, x, y);
				VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
				VideoManager->MoveRelative(_cursor_xoffset + left_edge + _cursor_xoffset, cursor_offset + _cursor_yoffset + _cursor_yoffset);
				StillImage *default_cursor = VideoManager->GetDefaultCursor();

				if (default_cursor)
					default_cursor->Draw(Color::white);
			}

			// Check if this is the index where we should draw the selection cursor icon, if it is visible
			if (index == _selection && (_blink && _cursor_state == VIDEO_CURSOR_STATE_BLINKING) == false && _cursor_state != VIDEO_CURSOR_STATE_HIDDEN) {
				_SetupAlignment(VIDEO_X_LEFT, _option_yalign, bounds, x, y);
				VideoManager->SetDrawFlags(VIDEO_BLEND, 0);
				VideoManager->MoveRelative(_cursor_xoffset + left_edge, cursor_offset + _cursor_yoffset);
				StillImage *default_cursor = VideoManager->GetDefaultCursor();

				if (default_cursor)
					default_cursor->Draw(Color::white);
			}

			bounds.x_left += xoff;
			bounds.x_center += xoff;
			bounds.x_right += xoff;
		} // for (int32 col = 0; col < _number_columns; ++col)

		if (finished)
			break;

		bounds.y_top += yoff;
		bounds.y_center += yoff;
		bounds.y_bottom += yoff;
	} // for (int32 row = row_min; row < row_max; row++)

	//VideoManager->EnableScissoring(scissoring_rollback);
	VideoManager->PopState();
} // void OptionBox::Draw()



void OptionBox::ClearOptions() {
	_options.clear();
	_number_options = 0;
}



bool OptionBox::SetOptions(const vector<ustring>& format_text) {
	_options.clear();
	_number_options = 0;

	vector<ustring>::const_iterator i = format_text.begin();
	while (i != format_text.end()) {
		const ustring &str = *i;

		Option option;

		if (_ConstructOption(str, option) == false) {
			_options.clear();
			_number_options = 0;
			if (VIDEO_DEBUG)
				cerr << "VIDEO ERROR: OptionBox::SetOptions() failed due to an invalid format string: "
					<< MakeStandardString(*i) << endl;
			return false;
		}

		_options.push_back(option);

		_number_options++;
		i++;
	}

	return true;
}



bool OptionBox::AddOption(const hoa_utils::ustring &text) {
	Option option;

	// Construct the option
	if (_ConstructOption(text, option) == false) {
		return false;
	}

	_options.push_back(option);
	_number_options++;
	return true;
}



bool OptionBox::SetOptionText(int32 index, const hoa_utils::ustring &text) {
	if (index < 0 || index >= _number_options) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: OptionBox::SetOptionText() failed because it was given an invalid index: " << index << endl;
		return false;
	}

	_ConstructOption(text, _options[index]);
	return true;
}



void OptionBox::SetSelection(int32 index) {
	if (index < 0 || index >= _number_options) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: OptionBox::SetSelection() failed because it was passed an invalid index: "
				<< index << endl;
		return;
	}
	_selection = index;

	int32 select_row = _selection / _number_columns;

	// If the new selection isn't currently being displayed, instantly scroll to it
	if (select_row < _scroll_offset || select_row > _scroll_offset + _number_rows - 1) {
		_scroll_offset = select_row - _number_rows + 1;

		int32 total_num_rows = (_number_options + _number_columns - 1) / _number_columns;

		if (_scroll_offset + _number_rows >= total_num_rows) {
			_scroll_offset = total_num_rows - _number_rows;
		}
	}
}



void OptionBox::EnableOption(int32 index, bool enable) {
	if (index < 0 || index >= _number_options) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: OptionBox::EnableOption() failed because an invalid "
				<< "option index was specified: " << index << endl;
		return;
	}

	_options[index].disabled = !enable;
}



bool OptionBox::IsOptionEnabled(int32 index) {
	if (index < 0 || index >= _number_options) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: OptionBox::IsOptionEnabled() failed because an invalid "
				<< "option index was specified: " << index << endl;
		return false;
	}

	return (!_options[index].disabled);
}



bool OptionBox::IsInitialized(string& error_messages) {
	ostringstream s;
	error_messages.clear();

	if (_number_rows <= 0)
		s << "* Invalid number of rows (" << _number_rows << ")" << endl;

	if (_number_columns <= 0)
		s << "* Invalid number of columns (" << _number_columns << ")" << endl;

	if (_horizontal_spacing <= 0.0f && _number_columns > 1)
		s << "* Invalid horizontal spacing (" << _horizontal_spacing << ")" << endl;

	if (_vertical_spacing <= 0.0f && _number_rows > 1)
		s << "* Invalid vertical spacing (" << _vertical_spacing << ")" << endl;

	if (_option_xalign < VIDEO_X_LEFT || _option_xalign > VIDEO_X_RIGHT)
		s << "* Invalid x align (" << _option_xalign << ")" << endl;

	if (_option_yalign < VIDEO_Y_TOP || _option_yalign > VIDEO_Y_BOTTOM)
		s << "* Invalid y align (" << _option_yalign << ")" << endl;

	if (_font.empty())
		s << "* Invalid font (none has been set)" << endl;

	if (_selection_mode <= VIDEO_SELECT_INVALID || _selection_mode >= VIDEO_SELECT_TOTAL)
		s << "* Invalid selection mode (" << _selection_mode << ")" << endl;

	error_messages = s.str();

	if (error_messages.empty())
		_initialized = true;
	else
		_initialized = false;

	return _initialized;
}

// -----------------------------------------------------------------------------
// Input Handling Methods
// -----------------------------------------------------------------------------

void OptionBox::HandleUpKey() {
	// Ignore input while scrolling, or if an event has already been logged
	if (_scrolling || _event)
		return;

	if (_ChangeSelection(-1, false) == false)
		_event = VIDEO_OPTION_BOUNDS_UP;
}



void OptionBox::HandleDownKey() {
	// Ignore input while scrolling, or if an event has already been logged
	if (_scrolling || _event)
		return;

	if(_ChangeSelection(1, false) == false)
		_event = VIDEO_OPTION_BOUNDS_DOWN;
}



void OptionBox::HandleLeftKey() {
	// Ignore input while scrolling, or if an event has already been logged
	if (_scrolling || _event)
		return;

	if (_ChangeSelection(-1, true) == false)
		_event = VIDEO_OPTION_BOUNDS_LEFT;
}



void OptionBox::HandleRightKey() {
	// Ignore input while scrolling, or if an event has already been logged
	if (_scrolling || _event)
		return;


	if (_ChangeSelection(1, true) == false)
		_event = VIDEO_OPTION_BOUNDS_RIGHT;
}



void OptionBox::HandleConfirmKey() {
	// Abort if an invalid option is selected
	if (_selection < 0 || _selection >= _number_options) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: In OptionBox::HandleConfirmKey(), an invalid (out of bounds) option "
				<< "was selected: " << _selection << endl;
		return;
	}

	// Ignore input while scrolling, or if an event has already been logged
	if (_scrolling || _event || _options[_selection].disabled)
		return;

	// Case #1: switch the position of two different options
	if (_switching && _first_selection >= 0 && _selection != _first_selection) {
		// Switch the location of the two selected options
		Option temp = _options[_selection];
		_options[_selection] = _options[_first_selection];
		_options[_first_selection] = temp;
		// Set _first_selection to -1, so that we know we're not in switching mode any more
		_first_selection = -1;

		_event = VIDEO_OPTION_SWITCH;
	}

	// Case #2: partial confirm (confirming the first element in a double confirm)
	else if (_selection_mode == VIDEO_SELECT_DOUBLE && _first_selection == -1) {
		// mark the item that is getting switched
		_first_selection = _selection;
	}

	// Case #3: confirm
	else {
		if (_options[_selection].disabled) {
			// TODO: do something to tell player they confirmed on a disabled option
			return;
		}

		_event = VIDEO_OPTION_CONFIRM;

		// Get out of switch mode
		_first_selection = -1;
	}
}



void OptionBox::HandleCancelKey() {
	// Ignore input while scrolling, or if an event has already been logged
	if (_scrolling || _event)
		return;

	// If we're in switch mode and the cancel key is hit, get out of switch mode.
	if (_first_selection >= 0)
		_first_selection = -1;
	else
		_event = VIDEO_OPTION_CANCEL;
}

// -----------------------------------------------------------------------------
// Member Access Functions
// -----------------------------------------------------------------------------

void OptionBox::SetFont(const std::string& font_name) {
	if (VideoManager->Text()->GetFontProperties(font_name) == NULL) {
		if(VIDEO_DEBUG)
			cerr << "VIDEO WARNING: OptionBox::SetFont() failed because VideoEngine::GetFontProperties() returned false for font: " << font_name << endl;
		return;
	}

	_font = font_name;
	_initialized = IsInitialized(_initialization_errors);
}



void OptionBox::SetCursorState(CursorState state) {
	if (state <= VIDEO_CURSOR_STATE_INVALID || state >= VIDEO_CURSOR_STATE_TOTAL) {
		if (VIDEO_DEBUG)
			cerr << "VIDEO WARNING: OptionBox::SetCursorState() failed because the argument passed was invalid: "
				<< state << endl;
		return;
	}

	_cursor_state = state;
}

// -----------------------------------------------------------------------------
// Private Methods
// -----------------------------------------------------------------------------

bool OptionBox::_ConstructOption(const ustring& format_string, Option& op) {
	// Options are enabled by default
	op.disabled = false;

	// Clear all vectors, to make certain that the Option passed in is empty
	op.elements.clear();
	op.text.clear();
	op.images.clear();

	// This is a valid case. It simply means we add an option with no tags, text, or other data.
	if (format_string.empty()) {
		return true;
	}

	// Copy the format_string into a temporary string that we can manipulate
	ustring tmp = format_string;

	while (tmp.empty() == false) {
		OptionElement new_element;

		if (tmp[0] == OPEN_TAG) { // Process a new tag
			size_t length = tmp.length();

			if (length < 3) {
				// All formatting tags are at least 3 characters long because you need the opening (<)
				// and close (>) plus stuff in the middle. So anything less than 3 characters is a problem.

				if (VIDEO_DEBUG)
					cerr << "VIDEO ERROR: OptionBox::_ConstructOption() failed because a tag opening was detected "
						<< "with an inadequate number of remaining characters to construct a full tag: "
						<< MakeStandardString(format_string) << endl;
				return false;
			}

			size_t end_position = tmp.find(END_TAG);

			if (end_position == ustring::npos) { // Did not find the end of the tag
				if (VIDEO_DEBUG)
					cerr << "VIDEO ERROR: OptionBox::_ConstructOption() failed because a matching end tag could not "
						<< "be found for an open tag: " << MakeStandardString(format_string) << endl;
				return false;
			}

			if (tmp[2] == END_TAG) { // First check if the tag is a 1-character alignment tag
				if (tmp[1] == CENTER_TAG1 || tmp[1] == CENTER_TAG2) {
					new_element.type = VIDEO_OPTION_ELEMENT_CENTER_ALIGN;
				}
				else if (tmp[1] == RIGHT_TAG1 || tmp[1] == RIGHT_TAG2) {
					new_element.type = VIDEO_OPTION_ELEMENT_RIGHT_ALIGN;
				}
				else if (tmp[1] == LEFT_TAG1 || tmp[1] == LEFT_TAG2) {
					new_element.type = VIDEO_OPTION_ELEMENT_LEFT_ALIGN;
				}
			}
			else { // The tag contains more than 1-character
				// Extract the tag string
				string tag_text = MakeStandardString(tmp.substr(1, end_position - 1));

				if (IsStringNumeric(tag_text)) { // Then this must be a positioning tag
					new_element.type  = VIDEO_OPTION_ELEMENT_POSITION;
					new_element.value = atoi(tag_text.c_str());
				}
				else { // Then this must be an image tag
					StillImage imd;
					if (imd.Load(tag_text) == false) {
						if (VIDEO_DEBUG)
							cerr << "VIDEO ERROR: OptionBox::_ConstructOption failed because of an invalid image tag: "
								<< MakeStandardString(format_string) << endl;
						return false;
					}
					new_element.type  = VIDEO_OPTION_ELEMENT_IMAGE;
					new_element.value = static_cast<int32>(op.images.size());
					op.images.push_back(imd);
				}
			}

			// Finished processing the tag so update the tmp string
			if (end_position == length - 1) { // End of string
				tmp.clear();
			}
			else {
				tmp = tmp.substr(end_position + 1, length - end_position - 1);
			}
		} // if (tmp[0] == OPEN_TAG)

		else { // If this isn't a tag, then it is raw text that should be added to the option
			new_element.type = VIDEO_OPTION_ELEMENT_TEXT;
			new_element.value = static_cast<int32>(op.text.size());

			// find the distance until the next tag
			size_t tag_begin = tmp.find(OPEN_TAG);

			if (tag_begin == ustring::npos) { // There are no more tags remaining, so extract the entire string
				op.text.push_back(tmp);
				tmp.clear();
			}
			else { // Another tag remains to be processed, so extract the text substring
				op.text.push_back(tmp.substr(0, tag_begin));
				tmp = tmp.substr(tag_begin, tmp.length() - tag_begin);
			}
		}

		op.elements.push_back(new_element);
	} // while (tmp.empty() == false)

	return true;
} // bool _ConstructOption(const ustring& format_string, Option& option)



bool OptionBox::_ChangeSelection(int32 offset, bool horizontal) {
	// Do nothing if the movement is horizontal and there is only one column with no horizontal wrap shifting
	if (horizontal == true && _number_columns == 1 && _horizontal_wrap_mode != VIDEO_WRAP_MODE_SHIFTED)
		return false;
	// Do nothing if the movement is vertical and there is only one row with no vertical wrap shifting
	if (horizontal == false && _number_rows == 1 && _vertical_wrap_mode != VIDEO_WRAP_MODE_SHIFTED)
		return false;

	// Get the row, column coordinates for the current selection
	int32 row = _selection / _number_columns;
	int32 col = _selection % _number_columns;
	bool bounds_exceeded = false;

	// Determine if the movement selection will exceed a column or row bondary
	if( (horizontal == true && ( (col + offset < 0) || (col + offset >= _number_columns) || (col + offset >= _number_options ) ) ) ||
		    (horizontal == false && ( (row + offset < 0) || (row + offset >= _number_rows) || (row + offset >= _number_options ) ) ) )
	{
		bounds_exceeded = true;
	}

	// Case #1: movement selection is within bounds
	if (bounds_exceeded == false) {
		if (horizontal)
			_selection += offset;
		else
			_selection += (offset * _number_columns);
	}

	// Case #2: movement exceeds bounds, no wrapping enabled
	else if ((horizontal == true && _horizontal_wrap_mode == VIDEO_WRAP_MODE_NONE) ||
		(horizontal == false && _vertical_wrap_mode == VIDEO_WRAP_MODE_NONE))
	{
		return false;
	}

	// Case #3: horizontal movement with wrapping enabled
	else if (horizontal == true) {
		if (col + offset < 0) { // The left boundary was exceeded
			if (_horizontal_wrap_mode == VIDEO_WRAP_MODE_STRAIGHT)
				offset += _number_columns;
			else if (_horizontal_wrap_mode == VIDEO_WRAP_MODE_SHIFTED) {
				if (_vertical_wrap_mode != VIDEO_WRAP_MODE_NONE) // Make sure vertical wrapping is accepted
					offset += _number_options;
				else
					return false;
			}
		}
		else { // The right boundary was exceeded
			if (_horizontal_wrap_mode == VIDEO_WRAP_MODE_STRAIGHT)
				offset -= _number_columns;
			else if (_horizontal_wrap_mode == VIDEO_WRAP_MODE_SHIFTED) {
				if (_vertical_wrap_mode != VIDEO_WRAP_MODE_NONE) // Make sure vertical wrapping is accepted
				{
					offset = 0;
					_selection++;
				}
				else
					return false;
			}
		}
		_selection = (_selection + offset) % _number_options;
	}

	// Case #4: vertical movement with wrapping enabled
	else {
		if (row + offset < 0) { // The top boundary was exceeded
			if (_vertical_wrap_mode == VIDEO_WRAP_MODE_STRAIGHT)
				offset += _number_options;
			else if (_vertical_wrap_mode == VIDEO_WRAP_MODE_SHIFTED) {
				if (_horizontal_wrap_mode != VIDEO_WRAP_MODE_NONE) // Make sure horizontal wrapping is accepted
					offset += (_number_columns - 1);
				else
					return false;
			}
		}
		else  { // The bottom boundary was exceeded
			if (_vertical_wrap_mode == VIDEO_WRAP_MODE_STRAIGHT) {
				if( row + offset >= _number_options )
					offset -= _number_options;
			}
			else if (_vertical_wrap_mode == VIDEO_WRAP_MODE_SHIFTED) {
				if (_horizontal_wrap_mode != VIDEO_WRAP_MODE_NONE) // Make sure horizontal wrapping is accepted
					offset -= (_number_columns - 1);
				else
					return false;
			}
		}
		_selection = (_selection + (offset * _number_columns)) % _number_options;
	}

	// If the new selection isn't currently being displayed, scroll it into view
	row = _selection / _number_columns;
	if (row < _scroll_offset || row >= _scroll_offset + _number_rows ) {
		_scrolling = true;
		_scroll_time = 0;

		if (row < _scroll_offset)
			_scroll_direction = -1 * ( _scroll_offset - row ); // scroll up
		else
			_scroll_direction = 1 * ( row - _number_rows - _scroll_offset + 1 ); // scroll down

		_scroll_offset += _scroll_direction;
	}

	_event = VIDEO_OPTION_SELECTION_CHANGE;

	return true;
} // bool OptionBox::_ChangeSelection(int32 offset, bool horizontal)



void OptionBox::_SetupAlignment(int32 xalign, int32 yalign, const OptionCellBounds& bounds, float& x, float& y) {
	VideoManager->SetDrawFlags(xalign, yalign, 0);

	switch (xalign) {
		case VIDEO_X_LEFT:
			x = bounds.x_left;
			break;
		case VIDEO_X_CENTER:
			x = bounds.x_center;
			break;
		default:
			x = bounds.x_right;
			break;
	}

	switch(yalign) {
		case VIDEO_Y_TOP:
			y = bounds.y_top;
			break;
		case VIDEO_Y_CENTER:
			y = bounds.y_center;
			break;
		default:
			y = bounds.y_bottom;
			break;
	}

	VideoManager->Move(x, y);
} // void OptionBox::_SetupAlignment(int32 xalign, int32 yalign, const OptionCellBounds& bounds, float& x, float& y)

} // namespace hoa_video
